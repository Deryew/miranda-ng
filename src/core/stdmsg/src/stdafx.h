/*

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commctrl.h>
#include <vssym32.h>

#include <Initguid.h>
#include <Oleacc.h>
#include <Uxtheme.h>

#include <malloc.h>
#include <time.h>

#include "resource.h"

#include <win2k.h>

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_button.h>
#include <m_clist.h>
#include <m_clc.h>
#include <m_options.h>
#include <m_protosvc.h>
#include <m_utils.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_userinfo.h>
#include <m_history.h>
#include <m_addcontact.h>
#include <m_chat_int.h>
#include <m_message.h>
#include <m_file.h>
#include <m_icolib.h>
#include <m_fontservice.h>
#include <m_timezones.h>
#include <m_avatars.h>
#include <m_metacontacts.h>
#include <m_ieview.h>
#include <m_smileyadd.h>
#include <m_popup.h>

#include "cmdlist.h"
#include "msgs.h"
#include "globals.h"
#include "richutil.h"
#include "version.h"

#define EM_SUBCLASSED (WM_USER+0x101)
#define EM_ACTIVATE   (WM_USER+0x102)

extern HINSTANCE g_hInst;
extern HCURSOR hCurSplitNS, hCurSplitWE, hCurHyperlinkHand;
extern HANDLE hHookWinEvt, hHookWinPopup, hHookWinWrite;
extern CREOleCallback reOleCallback;

/////////////////////////////////////////////////////////////////////////////////////////

struct TABLIST
{
	wchar_t *pszID;
	char *pszModule;
	TABLIST *next;
};

struct MODULEINFO : public GCModuleInfoBase
{
	int OnlineIconIndex;
	int OfflineIconIndex;
};

struct SESSION_INFO : public GCSessionInfoBase
{
	int iX, iY;
};

struct LOGSTREAMDATA : public GCLogStreamDataBase {};

struct GlobalLogSettings : public GlobalLogSettingsBase
{
	int   iX, iY;
	bool  bTabsEnable, TabsAtBottom, TabCloseOnDblClick, TabRestore;

	HFONT MessageAreaFont;
	COLORREF MessageAreaColor;
};

extern GlobalLogSettings g_Settings;
extern SESSION_INFO g_TabSession;
extern CHAT_MANAGER saveCI;
extern TABLIST *g_TabList;
extern HMENU g_hMenu;
extern HIMAGELIST hIconsList;

extern HINSTANCE g_hInst;
extern BOOL SmileyAddInstalled, PopupInstalled;

//main.c
void LoadIcons(void);
void Unload_ChatModule();
void Load_ChatModule();

// log.c
void Log_StreamInEvent(HWND hwndDlg, LOGINFO* lin, SESSION_INFO *si, BOOL bRedraw, BOOL bPhaseTwo);
void ValidateFilename(wchar_t * filename);
char* Log_CreateRtfHeader(MODULEINFO * mi);

// window.c
INT_PTR CALLBACK RoomWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int GetTextPixelSize(wchar_t* pszText, HFONT hFont, BOOL bWidth);

BOOL SM_SetTabbedWindowHwnd(SESSION_INFO *si, HWND hwnd);
SESSION_INFO* SM_GetPrevWindow(SESSION_INFO *si);
SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si);

// options.c
void AddIcons(void);
HICON LoadIconEx(const char *pszIcoLibName, bool big);
HANDLE GetIconHandle(const char *pszIcolibName);

// services.c
void ShowRoom(SESSION_INFO *si, WPARAM wp, BOOL bSetForeground);

// tools.c
int  GetColorIndex(const char* pszModule, COLORREF cr);
void CheckColorsInModule(const char* pszModule);
int  GetRichTextLength(HWND hwnd);
UINT CreateGCMenu(HWND hwndDlg, HMENU *hMenu, int iIndex, POINT pt, SESSION_INFO *si, wchar_t* pszUID, wchar_t* pszWordText);
void DestroyGCMenu(HMENU *hMenu, int iIndex);
bool LoadMessageFont(LOGFONT *lf, COLORREF *colour);

// message.c
char* Message_GetFromStream(HWND hwndDlg, SESSION_INFO *si);

void NotifyLocalWinEvent(MCONTACT hContact, HWND hwnd, unsigned int type);
BOOL TabM_AddTab(const wchar_t *pszID, const char* pszModule);
BOOL TabM_RemoveAll(void);

#pragma comment(lib,"comctl32.lib")
