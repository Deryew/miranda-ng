#include "..\stdafx.h"
#include "google_api.h"

CGDriveService::CGDriveService(const char *protoName, const wchar_t *userName)
	: CCloudService(protoName, userName)
{
}

CGDriveService* CGDriveService::Init(const char *moduleName, const wchar_t *userName)
{
	CGDriveService *proto = new CGDriveService(moduleName, userName);
	Services.insert(proto);
	return proto;
}

int CGDriveService::UnInit(CGDriveService *proto)
{
	Services.remove(proto);
	delete proto;
	return 0;
}

const char* CGDriveService::GetModuleName() const
{
	return "/Google";
}

int CGDriveService::GetIconId() const
{
	return IDI_GDRIVE;
}

bool CGDriveService::IsLoggedIn()
{
	ptrA token(db_get_sa(NULL, GetAccountName(), "TokenSecret"));
	if (!token || token[0] == 0)
		return false;
	time_t now = time(nullptr);
	time_t expiresIn = db_get_dw(NULL, GetAccountName(), "ExpiresIn");
	return now < expiresIn;
}

void CGDriveService::Login()
{
	ptrA token(db_get_sa(NULL, GetAccountName(), "TokenSecret"));
	ptrA refreshToken(db_get_sa(NULL, GetAccountName(), "RefreshToken"));
	if (token && refreshToken && refreshToken[0]) 	{
		GDriveAPI::RefreshTokenRequest request(refreshToken);
		NLHR_PTR response(request.Send(m_hConnection));
		
		JSONNode root = GetJsonResponse(response);

		JSONNode node = root.at("access_token");
		db_set_s(NULL, GetAccountName(), "TokenSecret", node.as_string().c_str());

		node = root.at("expires_in");
		time_t expiresIn = time(nullptr) + node.as_int();
		db_set_dw(NULL, GetAccountName(), "ExpiresIn", expiresIn);
		
		return;
	}
	
	COAuthDlg dlg(this, GOOGLE_AUTH, RequestAccessTokenThread);
	dlg.DoModal();
}

void CGDriveService::Logout()
{
	mir_forkthreadex(RevokeAccessTokenThread, this);
}

unsigned CGDriveService::RequestAccessTokenThread(void *owner, void *param)
{
	HWND hwndDlg = (HWND)param;
	CGDriveService *service = (CGDriveService*)owner;

	if (service->IsLoggedIn())
		service->Logout();

	char requestToken[128];
	GetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, requestToken, _countof(requestToken));

	GDriveAPI::GetAccessTokenRequest request(requestToken);
	NLHR_PTR response(request.Send(service->m_hConnection));

	if (response == nullptr || response->resultCode != HTTP_CODE_OK) {
		const char *error = response && response->dataLength
			? response->pData
			: service->HttpStatusToError(response ? response->resultCode : 0);

		Netlib_Logf(service->m_hConnection, "%s: %s", service->GetAccountName(), error);
		//ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);
		return 0;
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty()) {
		Netlib_Logf(service->m_hConnection, "%s: %s", service->GetAccountName(), service->HttpStatusToError(response->resultCode));
		//ShowNotification(TranslateT("server does not respond"), MB_ICONERROR);
		return 0;
	}

	JSONNode node = root.at("error_description");
	if (!node.isnull()) {
		ptrW error_description(mir_a2u_cp(node.as_string().c_str(), CP_UTF8));
		Netlib_Logf(service->m_hConnection, "%s: %s", service->GetAccountName(), service->HttpStatusToError(response->resultCode));
		//ShowNotification((wchar_t*)error_description, MB_ICONERROR);
		return 0;
	}

	node = root.at("access_token");
	db_set_s(NULL, service->GetAccountName(), "TokenSecret", node.as_string().c_str());

	node = root.at("expires_in");
	time_t expiresIn = time(nullptr) + node.as_int();
	db_set_dw(NULL, service->GetAccountName(), "ExpiresIn", expiresIn);

	node = root.at("refresh_token");
	db_set_s(NULL, service->GetAccountName(), "RefreshToken", node.as_string().c_str());

	SetDlgItemTextA(hwndDlg, IDC_OAUTH_CODE, "");

	EndDialog(hwndDlg, 1);

	return 0;
}

unsigned CGDriveService::RevokeAccessTokenThread(void *param)
{
	CGDriveService *service = (CGDriveService*)param;

	ptrA token(db_get_sa(NULL, service->GetAccountName(), "TokenSecret"));
	GDriveAPI::RevokeAccessTokenRequest request(token);
	NLHR_PTR response(request.Send(service->m_hConnection));

	return 0;
}

void CGDriveService::HandleJsonError(JSONNode &node)
{
	JSONNode error = node.at("error");
	if (!error.isnull()) {
		json_string tag = error.at(".tag").as_string();
		throw Exception(tag.c_str());
	}
}

void CGDriveService::UploadFile(const char *parentId, const char *name, const char *data, size_t size, char *fileId)
{
	ptrA token(db_get_sa(NULL, GetAccountName(), "TokenSecret"));
	GDriveAPI::UploadFileRequest request(token, parentId, name, data, size);
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	JSONNode node = root.at("id");
	mir_strcpy(fileId, node.as_string().c_str());
}

void CGDriveService::CreateUploadSession(const char *parentId, const char *name, char *uploadUri)
{
	ptrA token(db_get_sa(NULL, GetAccountName(), "TokenSecret"));
	GDriveAPI::CreateUploadSessionRequest request(token, parentId, name);
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		for (int i = 0; i < response->headersCount; i++)
		{
			if (mir_strcmpi(response->headers[i].szName, "Location"))
				continue;

			mir_strcpy(uploadUri, response->headers[i].szValue);
			return;
		}
	}

	HttpResponseToError(response);
}

void CGDriveService::UploadFileChunk(const char *uploadUri, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize, char *fileId)
{
	GDriveAPI::UploadFileChunkRequest request(uploadUri, chunk, chunkSize, offset, fileSize);
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (response->resultCode == HTTP_CODE_PERMANENT_REDIRECT)
		return;

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		JSONNode root = GetJsonResponse(response);
		JSONNode node = root.at("id");
		mir_strcpy(fileId, node.as_string().c_str());
		return;
	}

	HttpResponseToError(response);
}

void CGDriveService::CreateFolder(const char *path, char *folderId)
{
	ptrA token(db_get_sa(NULL, GetAccountName(), "TokenSecret"));
	GDriveAPI::CreateFolderRequest request(token, path);
	NLHR_PTR response(request.Send(m_hConnection));

	JSONNode root = GetJsonResponse(response);
	JSONNode node = root.at("id");
	mir_strcpy(folderId, node.as_string().c_str());
}

void CGDriveService::CreateSharedLink(const char *itemId, char *url)
{
	ptrA token(db_get_sa(NULL, GetAccountName(), "TokenSecret"));
	GDriveAPI::GrantPermissionsRequest request(token, itemId);
	NLHR_PTR response(request.Send(m_hConnection));

	HandleHttpError(response);

	if (HTTP_CODE_SUCCESS(response->resultCode)) {
		CMStringA sharedUrl(CMStringDataFormat::FORMAT, GDRIVE_SHARE, itemId);
		mir_strcpy(url, sharedUrl);
		return;
	}

	HttpResponseToError(response);
}

UINT CGDriveService::Upload(FileTransferParam *ftp)
{
	try {
		if (!IsLoggedIn())
			Login();

		if (!IsLoggedIn()) {
			ftp->SetStatus(ACKRESULT_FAILED);
			return ACKRESULT_FAILED;
		}

		char folderId[32] = { 0 };
		if (ftp->IsFolder()) {
			T2Utf folderName(ftp->GetFolderName());

			CreateFolder(folderName, folderId);
			char link[MAX_PATH];

			CreateSharedLink(folderId, link);
			ftp->AppendFormatData(L"%s\r\n", ptrW(mir_utf8decodeW(link)));
			ftp->AddSharedLink(link);
		}

		ftp->FirstFile();
		do
		{
			T2Utf fileName(ftp->GetCurrentRelativeFilePath());
			uint64_t fileSize = ftp->GetCurrentFileSize();

			char fileId[32];

			size_t chunkSize = ftp->GetCurrentFileChunkSize();
			mir_ptr<char>chunk((char*)mir_calloc(chunkSize));

			if (chunkSize == fileSize)
			{
				ftp->CheckCurrentFile();
				size_t size = ftp->ReadCurrentFile(chunk, chunkSize);

				UploadFile(folderId, fileName, chunk, size, fileId);

				ftp->Progress(size);
			}
			else
			{
				char uploadUri[1024];
				CreateUploadSession(uploadUri, folderId, fileName);

				uint64_t offset = 0;
				double chunkCount = ceil(double(fileSize) / chunkSize);
				while (chunkCount > 0) {
					ftp->CheckCurrentFile();
					size_t size = ftp->ReadCurrentFile(chunk, chunkSize);

					UploadFileChunk(uploadUri, chunk, size, offset, fileSize, fileId);

					offset += size;
					ftp->Progress(size);
				}
			}

			if (!ftp->IsFolder()) {
				char link[MAX_PATH];
				CreateSharedLink(fileId, link);
				ftp->AppendFormatData(L"%s\r\n", ptrW(mir_utf8decodeW(link)));
				ftp->AddSharedLink(link);
			}
		} while (ftp->NextFile());
	}
	catch (Exception &ex) {
		Netlib_Logf(m_hConnection, "%s: %s", MODULE, ex.what());
		ftp->SetStatus(ACKRESULT_FAILED);
		return ACKRESULT_FAILED;
	}

	ftp->SetStatus(ACKRESULT_SUCCESS);
	return ACKRESULT_SUCCESS;
}
