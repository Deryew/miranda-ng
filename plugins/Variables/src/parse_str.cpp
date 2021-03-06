/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

static wchar_t *parseCaps(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	wchar_t *res = mir_wstrdup(ai->argv.w[1]);
	wchar_t *cur = res;
	CharLower(res);
	*cur = (wchar_t)CharUpper((LPTSTR)*cur);
	cur++;
	while (*cur != 0) {
		if ((*cur == ' ') && (*(cur + 1) != 0)) {
			cur++;
			if (IsCharLower(*cur))
				*cur = (wchar_t)CharUpper((LPTSTR)*cur);
		}
		else {
			cur++;
			if (IsCharUpper(*cur))
				*cur = (wchar_t)CharLower((LPTSTR)*cur);
		}
	}
	return res;
}

static wchar_t *parseCaps2(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	wchar_t *res = mir_wstrdup(ai->argv.w[1]);
	wchar_t *cur = res;
	*cur = (wchar_t)CharUpper((LPTSTR)*cur);
	cur++;
	while (*cur != 0) {
		if ((*cur == ' ') && (*(cur + 1) != 0)) {
			cur++;
			if (IsCharLower(*cur))
				*cur = (wchar_t)CharUpper((LPTSTR)*cur);
		}
		else cur++;
	}
	return res;
}

static wchar_t *parseCrlf(ARGUMENTSINFO *ai)
{
	ai->flags |= AIF_DONTPARSE;
	return mir_wstrdup(L"\r\n");
}

static wchar_t *parseEolToCrlf(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	wchar_t *res = mir_wstrdup(ai->argv.w[1]);
	wchar_t *cur = res;
	do {
		cur = wcschr(cur, '\n');
		if ((cur == nullptr) || ((cur > res) && (*(cur - 1) == '\r')))
			continue;

		log_debug(cur);
		int loc = cur - res;
		res = (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + 2)*sizeof(wchar_t));
		cur = res + loc;
		memmove(cur + 2, cur + 1, (mir_wstrlen(cur + 1) + 1)*sizeof(wchar_t));
		memcpy(cur, L"\r\n", 2 * sizeof(wchar_t));
		cur += 2;
	}
	while (cur != nullptr);

	return res;
}

static wchar_t *parseFixeol(ARGUMENTSINFO *ai)
{
	wchar_t *szReplacement;
	if (ai->argc == 2)
		szReplacement = L"(...)";
	else if (ai->argc == 3)
		szReplacement = ai->argv.w[2];
	else
		return nullptr;

	wchar_t *cur = ai->argv.w[1];
	while (mir_wstrcmp(cur, L"\r\n") && *cur != '\n' && *cur != 0)
		cur++;

	if (*cur == '\0')
		return mir_wstrdup(ai->argv.w[1]);

	cur--;
	wchar_t *res = (wchar_t*)mir_alloc((cur - ai->argv.w[1] + mir_wstrlen(szReplacement) + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return res;

	memset(res, 0, (((cur - ai->argv.w[1]) + 1) * sizeof(wchar_t)));
	wcsncpy(res, ai->argv.w[1], cur - ai->argv.w[1]);
	mir_wstrcat(res, szReplacement);
	return res;
}

static wchar_t *parseFixeol2(ARGUMENTSINFO *ai)
{
	wchar_t *szReplacement;
	switch (ai->argc) {
	case 2:	szReplacement = L" ";	break;
	case 3:  szReplacement = ai->argv.w[2];  break;
	default: return nullptr;
	}

	wchar_t *res = mir_wstrdup(ai->argv.w[1]);
	for (size_t pos = 0; pos < mir_wstrlen(res); pos++) {
		wchar_t *cur = res + pos;
		wchar_t *szEol = nullptr;
		if (!wcsncmp(cur, L"\r\n", mir_wstrlen(L"\r\n")))
			szEol = L"\r\n";

		if (*cur == '\n')
			szEol = L"\n";

		if (szEol != nullptr) {
			if (mir_wstrlen(szReplacement) > mir_wstrlen(szEol)) {
				res = (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + mir_wstrlen(szReplacement) - mir_wstrlen(szEol) + 1)*sizeof(wchar_t));
				cur = res + pos;
			}
			memmove(cur + mir_wstrlen(szReplacement), cur + mir_wstrlen(szEol), (mir_wstrlen(cur + mir_wstrlen(szEol)) + 1)*sizeof(wchar_t));
			memcpy(cur, szReplacement, mir_wstrlen(szReplacement)*sizeof(wchar_t));
			pos += mir_wstrlen(szReplacement) - 1;
		}
	}
	return (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + 1)*sizeof(wchar_t));
}

static wchar_t *parseInsert(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return nullptr;

	unsigned int pos = ttoi(ai->argv.w[3]);
	if (pos > mir_wstrlen(ai->argv.w[1]))
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((mir_wstrlen(ai->argv.w[1]) + mir_wstrlen(ai->argv.w[2]) + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((mir_wstrlen(ai->argv.w[1]) + mir_wstrlen(ai->argv.w[2]) + 1) * sizeof(wchar_t)));
	wcsncpy(res, ai->argv.w[1], pos);
	mir_wstrcpy(res + pos, ai->argv.w[2]);
	mir_wstrcpy(res + pos + mir_wstrlen(ai->argv.w[2]), ai->argv.w[1] + pos);
	return res;
}

static wchar_t *parseLeft(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	int len = ttoi(ai->argv.w[2]);
	if (len < 0)
		return nullptr;

	len = min(len, (signed int)mir_wstrlen(ai->argv.w[1]));
	wchar_t *res = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((len + 1) * sizeof(wchar_t)));
	wcsncpy(res, ai->argv.w[1], len);
	return res;
}

static wchar_t *parseLen(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	return itot((int)mir_wstrlen(ai->argv.w[1]));
}

static wchar_t *parseLineCount(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	int count = 1;
	wchar_t *cur = ai->argv.w[1];
	while (cur < (ai->argv.w[1] + mir_wstrlen(ai->argv.w[1]))) {
		if (!wcsncmp(cur, L"\r\n", 2)) {
			count++;
			cur++;
		}
		else if (*cur == '\n')
			count++;

		cur++;
	}

	return itot(count);
}

static wchar_t *parseLower(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	wchar_t *res = mir_wstrdup(ai->argv.w[1]);
	if (res == nullptr)
		return nullptr;

	return CharLower(res);
}

static wchar_t *parseLongest(ARGUMENTSINFO *ai)
{
	if (ai->argc < 2)
		return nullptr;

	unsigned int iLong = 1;
	for (unsigned int i = 2; i < ai->argc; i++)
	if (mir_wstrlen(ai->argv.w[i]) > mir_wstrlen(ai->argv.w[iLong]))
		iLong = i;

	return mir_wstrdup(ai->argv.w[iLong]);
}

static wchar_t *parseNoOp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	return mir_wstrdup(ai->argv.w[1]);
}

static wchar_t *parsePad(ARGUMENTSINFO *ai)
{
	wchar_t padchar;
	switch (ai->argc) {
	case 3:  padchar = ' ';  break;
	case 4:  padchar = *ai->argv.w[3];  break;
	default: return nullptr;
	}

	int padding = ttoi(ai->argv.w[2]);
	if (padding < 0)
		return nullptr;

	unsigned int addcount = max(padding - (signed int)mir_wstrlen(ai->argv.w[1]), 0);
	wchar_t *res = (wchar_t*)mir_alloc((addcount + mir_wstrlen(ai->argv.w[1]) + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((addcount + mir_wstrlen(ai->argv.w[1]) + 1) * sizeof(wchar_t)));
	wchar_t *cur = res;
	for (unsigned int i = 0; i < addcount; i++)
		*cur++ = padchar;

	mir_wstrcat(res, ai->argv.w[1]);
	return res;
}

static wchar_t *parsePadright(ARGUMENTSINFO *ai)
{
	wchar_t padchar;
	switch (ai->argc) {
	case 3: padchar = ' ';  break;
	case 4: padchar = *ai->argv.w[3]; break;
	default: return nullptr;
	}

	int padding = ttoi(ai->argv.w[2]);
	if (padding < 0)
		return nullptr;

	unsigned int addcount = max(padding - (signed int)mir_wstrlen(ai->argv.w[1]), 0);
	wchar_t *res = (wchar_t*)mir_alloc((addcount + mir_wstrlen(ai->argv.w[1]) + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((addcount + mir_wstrlen(ai->argv.w[1]) + 1) * sizeof(wchar_t)));
	mir_wstrcpy(res, ai->argv.w[1]);
	wchar_t *cur = res + mir_wstrlen(ai->argv.w[1]);
	for (unsigned int i = 0; i < addcount; i++)
		*cur++ = padchar;

	return res;
}

static wchar_t *parsePadcut(ARGUMENTSINFO *ai)
{
	wchar_t padchar;
	switch (ai->argc) {
	case 3: padchar = ' ';   break;
	case 4: padchar = *ai->argv.w[3]; break;
	default: return nullptr;
	}

	int padding = ttoi(ai->argv.w[2]);
	if (padding < 0)
		return nullptr;

	int addcount = max(padding - (signed int)mir_wstrlen(ai->argv.w[1]), 0);
	wchar_t *res = (wchar_t*)mir_alloc((padding + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((padding + 1) * sizeof(wchar_t)));
	wchar_t *cur = res;
	for (int i = 0; i < addcount; i++)
		*cur++ = padchar;

	if (padding > addcount)
		wcsncpy(res + addcount, ai->argv.w[1], padding - addcount);

	return res;
}

static wchar_t *parsePadcutright(ARGUMENTSINFO *ai)
{
	wchar_t padchar;
	switch (ai->argc) {
	case 3:  padchar = ' ';  break;
	case 4:  padchar = *ai->argv.w[3]; break;
	default: return nullptr;
	}

	int padding = ttoi(ai->argv.w[2]);
	if (padding < 0)
		return nullptr;

	int addcount = max(padding - (signed int)mir_wstrlen(ai->argv.w[1]), 0);
	wchar_t *res = (wchar_t*)mir_alloc((padding + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((padding + 1) * sizeof(wchar_t)));
	wchar_t *cur = res + padding - addcount;
	for (int i = 0; i < addcount; i++)
		*cur++ = padchar;

	if (padding > addcount)
		wcsncpy(res, ai->argv.w[1], padding - addcount);

	return res;
}

static wchar_t *parseRepeat(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	int count = ttoi(ai->argv.w[2]);
	if (count < 0)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((count * mir_wstrlen(ai->argv.w[1]) + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((count * mir_wstrlen(ai->argv.w[1]) + 1) * sizeof(wchar_t)));
	for (int i = 0; i < count; i++)
		mir_wstrcat(res, ai->argv.w[1]);

	return res;
}

static wchar_t *parseReplace(ARGUMENTSINFO *ai)
{
	if ((ai->argc < 4) || (ai->argc % 2 != 0))
		return nullptr;

	wchar_t *cur;

	size_t pos = 0;
	wchar_t *res = mir_wstrdup(ai->argv.w[1]);
	for (size_t i = 2; i < ai->argc; i += 2) {
		if (mir_wstrlen(ai->argv.w[i]) == 0)
			continue;

		for (pos = 0; pos<mir_wstrlen(res); pos++) {
			cur = res + pos;
			if (!wcsncmp(cur, ai->argv.w[i], mir_wstrlen(ai->argv.w[i]))) {
				if (mir_wstrlen(ai->argv.w[i + 1]) > mir_wstrlen(ai->argv.w[i])) {
					res = (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + mir_wstrlen(ai->argv.w[i + 1]) - mir_wstrlen(ai->argv.w[i]) + 1)*sizeof(wchar_t));
					cur = res + pos;
				}
				memmove(cur + mir_wstrlen(ai->argv.w[i + 1]), cur + mir_wstrlen(ai->argv.w[i]), (mir_wstrlen(cur + mir_wstrlen(ai->argv.w[i])) + 1)*sizeof(wchar_t));
				memcpy(cur, ai->argv.w[i + 1], mir_wstrlen(ai->argv.w[i + 1])*sizeof(wchar_t));
				pos += mir_wstrlen(ai->argv.w[i + 1]) - 1;
			}
		}
		res = (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + 1)*sizeof(wchar_t));
	}

	return res;
}

static wchar_t *parseRight(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	int len = ttoi(ai->argv.w[2]);
	if (len < 0)
		return nullptr;

	len = min(len, (signed int)mir_wstrlen(ai->argv.w[1]));
	wchar_t *res = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((len + 1)*sizeof(wchar_t)));
	wcsncpy(res, ai->argv.w[1] + mir_wstrlen(ai->argv.w[1]) - len, len);
	return res;
}

/*
	string, display size, scroll amount
	*/
static wchar_t *parseScroll(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return nullptr;

	if (mir_wstrlen(ai->argv.w[1]) == 0)
		return mir_wstrdup(ai->argv.w[1]);

	size_t move = ttoi(ai->argv.w[3]) % mir_wstrlen(ai->argv.w[1]);
	size_t display = ttoi(ai->argv.w[2]);
	if (display > mir_wstrlen(ai->argv.w[1]))
		display = (unsigned)mir_wstrlen(ai->argv.w[1]);

	wchar_t *res = (wchar_t*)mir_alloc((2 * mir_wstrlen(ai->argv.w[1]) + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((2 * mir_wstrlen(ai->argv.w[1]) + 1) * sizeof(wchar_t)));
	mir_wstrcpy(res, ai->argv.w[1]);
	mir_wstrcat(res, ai->argv.w[1]);
	memmove(res, res + move, (mir_wstrlen(res + move) + 1)*sizeof(wchar_t));
	*(res + display) = 0;
	res = (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + 1)*sizeof(wchar_t));

	return res;
}

static wchar_t *parseShortest(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return nullptr;

	int iShort = 1;
	for (unsigned i = 2; i < ai->argc; i++)
	if (mir_wstrlen(ai->argv.w[i]) < mir_wstrlen(ai->argv.w[iShort]))
		iShort = i;

	return mir_wstrdup(ai->argv.w[iShort]);
}

static wchar_t *parseStrchr(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	wchar_t *c = wcschr(ai->argv.w[1], *ai->argv.w[2]);
	if (c == nullptr || *c == 0)
		return mir_wstrdup(L"0");

	return itot(c - ai->argv.w[1] + 1);
}

static wchar_t *parseStrcmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	if (mir_wstrcmp(ai->argv.w[1], ai->argv.w[2]))
		ai->flags |= AIF_FALSE;

	return mir_wstrdup(L"");
}

static wchar_t *parseStrmcmp(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return nullptr;

	ai->flags |= AIF_FALSE;
	for (unsigned i = 2; i < ai->argc; i++) {
		if (!mir_wstrcmp(ai->argv.w[1], ai->argv.w[i])) {
			ai->flags &= ~AIF_FALSE;
			break;
		}
	}

	return mir_wstrdup(L"");
}

static wchar_t *parseStrncmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return nullptr;

	int n = ttoi(ai->argv.w[3]);
	if (n <= 0)
		return nullptr;

	if (wcsncmp(ai->argv.w[1], ai->argv.w[2], n))
		ai->flags |= AIF_FALSE;

	return mir_wstrdup(L"");
}

static wchar_t *parseStricmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	if (mir_wstrcmpi(ai->argv.w[1], ai->argv.w[2]))
		ai->flags |= AIF_FALSE;

	return mir_wstrdup(L"");
}

static wchar_t *parseStrnicmp(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return nullptr;

	int n = ttoi(ai->argv.w[3]);
	if (n <= 0)
		return nullptr;

	if (wcsnicmp(ai->argv.w[1], ai->argv.w[2], n))
		ai->flags |= AIF_FALSE;

	return mir_wstrdup(L"");
}

static wchar_t *parseStrrchr(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	wchar_t *c = wcsrchr(ai->argv.w[1], *ai->argv.w[2]);
	if ((c == nullptr) || (*c == 0))
		return mir_wstrdup(L"0");

	return itot(c - ai->argv.w[1] + 1);
}

static wchar_t *parseStrstr(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	wchar_t *c = wcsstr(ai->argv.w[1], ai->argv.w[2]);
	if ((c == nullptr) || (*c == 0))
		return mir_wstrdup(L"0");

	return itot(c - ai->argv.w[1] + 1);
}

static wchar_t *parseSubstr(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return nullptr;

	int to, from = max(ttoi(ai->argv.w[2]) - 1, 0);
	if (ai->argc > 3)
		to = min(ttoi(ai->argv.w[3]), (int)mir_wstrlen(ai->argv.w[1]));
	else
		to = (int)mir_wstrlen(ai->argv.w[1]);

	if (to < from)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((to - from + 1)*sizeof(wchar_t));
	memset(res, 0, ((to - from + 1) * sizeof(wchar_t)));
	wcsncpy(res, ai->argv.w[1] + from, to - from);
	return res;
}

static wchar_t *parseSelect(ARGUMENTSINFO *ai)
{
	if (ai->argc <= 1)
		return nullptr;

	int n = ttoi(ai->argv.w[1]);
	if ((n > (signed int)ai->argc - 2) || n <= 0)
		return nullptr;

	return mir_wstrdup(ai->argv.w[n + 1]);
}

static wchar_t *parseSwitch(ARGUMENTSINFO *ai)
{
	if (ai->argc % 2 != 0)
		return nullptr;

	for (unsigned i = 2; i < ai->argc; i += 2)
	if (!mir_wstrcmp(ai->argv.w[1], ai->argv.w[i]))
		return mir_wstrdup(ai->argv.w[i + 1]);

	return nullptr;
}

static wchar_t *parseTrim(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	wchar_t *scur = ai->argv.w[1];
	while (*scur == ' ')
		scur++;

	wchar_t *ecur = ai->argv.w[1] + mir_wstrlen(ai->argv.w[1]) - 1;
	while ((*ecur == ' ') && (ecur > ai->argv.w[1]))
		ecur--;

	if (scur >= ecur)
		return mir_wstrdup(L"");

	wchar_t *res = (wchar_t*)mir_alloc((ecur - scur + 2)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((ecur - scur + 2) * sizeof(wchar_t)));
	wcsncpy(res, scur, ecur - scur + 1);

	return res;
}

static wchar_t *parseTab(ARGUMENTSINFO *ai)
{
	int count = 1;
	if ((ai->argc == 2) && (mir_wstrlen(ai->argv.w[1]) > 0))
		count = ttoi(ai->argv.w[1]);

	if (count < 0)
		return nullptr;

	wchar_t *res = (wchar_t*)mir_alloc((count + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, (count + 1)*sizeof(wchar_t));
	wchar_t *cur = res;
	for (int i = 0; i < count; i++)
		*cur++ = '\t';

	return res;
}

static wchar_t *parseUpper(ARGUMENTSINFO *ai)
{
	if (ai->argc != 2)
		return nullptr;

	wchar_t *res = mir_wstrdup(ai->argv.w[1]);
	if (res == nullptr)
		return nullptr;

	return CharUpper(res);
}

static wchar_t *getNthWord(wchar_t *szString, int w)
{
	if (szString == nullptr)
		return nullptr;

	int count = 0;
	wchar_t *scur = szString;
	while (*scur == ' ')
		scur++;

	count++;
	while ((count < w) && (scur < szString + mir_wstrlen(szString))) {
		if (*scur == ' ') {
			while (*scur == ' ')
				scur++;

			count++;
		}
		if (count < w)
			scur++;
	}
	if (count != w)
		return nullptr;

	wchar_t *ecur = scur;
	while ((*ecur != ' ') && (*ecur != 0))
		ecur++;

	wchar_t *res = (wchar_t*)mir_alloc((ecur - scur + 1)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((ecur - scur + 1) * sizeof(wchar_t)));
	wcsncpy(res, scur, ecur - scur);
	return res;
}

static wchar_t *parseWord(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3 || ai->argc > 4)
		return nullptr;

	wchar_t *res = nullptr;
	int to, from = ttoi(ai->argv.w[2]);
	if (ai->argc == 4) {
		if (mir_wstrlen(ai->argv.w[3]) > 0)
			to = ttoi(ai->argv.w[3]);
		else
			to = 100000; // rework
	}
	else to = from;

	if ((from == 0) || (to == 0) || (from > to))
		return nullptr;

	for (int i = from; i <= to; i++) {
		wchar_t *szWord = getNthWord(ai->argv.w[1], i);
		if (szWord == nullptr)
			return res;

		if (res != nullptr) {
			wchar_t *pres = (wchar_t*)mir_realloc(res, (mir_wstrlen(res) + mir_wstrlen(szWord) + 2)*sizeof(wchar_t));
			if (pres != nullptr) {
				res = pres;
				mir_wstrcat(res, L" ");
				mir_wstrcat(res, szWord);
			}
			mir_free(szWord);
		}
		else res = szWord;
	}

	return res;
}

static wchar_t *parseExtratext(ARGUMENTSINFO *ai)
{
	if (ai->argc > 1)
		return nullptr;

	ai->flags |= AIF_DONTPARSE;
	return mir_wstrdup(ai->fi->szExtraText.w);
}

void registerStrTokens()
{
	registerIntToken(MIR_CAPS, parseCaps, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("converts each first letter of a word to uppercase, all others to lowercase"));
	registerIntToken(MIR_CAPS2, parseCaps2, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("converts each first letter of a word to uppercase"));
	registerIntToken(MIR_CRLF, parseCrlf, TRF_FUNCTION, LPGEN("String Functions") "\t()\t" LPGEN("inserts 'end of line' character"));
	registerIntToken(MIR_EXTRATEXT, parseExtratext, TRF_FIELD, LPGEN("String Functions") "\t" LPGEN("depends on calling plugin"));
	registerIntToken(MIR_EOL2CRLF, parseEolToCrlf, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("replace all occurrences of \\n (Unix) by \\r\\n (Windows)"));
	registerIntToken(MIR_FIXEOL, parseFixeol, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("cuts x after the first line and appends y (y is optional)"));
	registerIntToken(MIR_FIXEOL2, parseFixeol2, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("replaces all end of line characters by y (y is optional)"));
	registerIntToken(MIR_INSERT, parseInsert, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("inserts string y at position z in string x"));
	registerIntToken(MIR_LEFT, parseLeft, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("trims x to length y, keeping first y characters"));
	registerIntToken(MIR_LEN, parseLen, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("length of x"));
	registerIntToken(MIR_LINECOUNT, parseLineCount, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("the number of lines in string x"));
	registerIntToken(MIR_LONGEST, parseLongest, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,...)\t" LPGEN("the longest string of the arguments"));
	registerIntToken(MIR_LOWER, parseLower, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("converts x to lowercase"));
	registerIntToken(MIR_NOOP, parseNoOp, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("no operation, x as given"));
	registerIntToken(MIR_PAD, parsePad, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("pads x to length y prepending character z (z is optional)"));
	registerIntToken(MIR_PADRIGHT, parsePadright, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("pads x to length y appending character z (z is optional)"));
	registerIntToken(MIR_PADCUT, parsePadcut, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("pads x to length y prepending character z, or cut if x is longer (z is optional)"));
	registerIntToken(MIR_PADCUTRIGHT, parsePadcutright, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("pads x to length y appending character z, or cut if x is longer (z is optional)"));
	registerIntToken(MIR_REPEAT, parseRepeat, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("repeats x y times"));
	registerIntToken(MIR_REPLACE, parseReplace, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z,...)\t" LPGEN("replace all occurrences of y in x with z, multiple y and z arguments allowed"));
	registerIntToken(MIR_RIGHT, parseRight, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("trims x to length y, keeping last y characters"));
	registerIntToken(MIR_SCROLL, parseScroll, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("moves string x, z characters to the left and trims it to y characters"));
	registerIntToken(MIR_STRCMP, parseStrcmp, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("TRUE if x equals y"));
	registerIntToken(MIR_STRMCMP, parseStrmcmp, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,...)\t" LPGEN("TRUE if x equals any of the following arguments"));
	registerIntToken(MIR_STRNCMP, parseStrncmp, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("TRUE if the first z characters of x equal y"));
	registerIntToken(MIR_STRICMP, parseStricmp, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("TRUE if x equals y, ignoring case"));
	registerIntToken(MIR_STRNICMP, parseStrnicmp, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("TRUE if the first z characters of x equal y, ignoring case"));
	registerIntToken(MIR_SHORTEST, parseShortest, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,...)\t" LPGEN("the shortest string of the arguments"));
	registerIntToken(MIR_STRCHR, parseStrchr, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("location of first occurrence of character y in string x"));
	registerIntToken(MIR_STRRCHR, parseStrrchr, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("location of last occurrence of character y in string x"));
	registerIntToken(MIR_STRSTR, parseStrstr, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y)\t" LPGEN("location of first occurrence of string y in x"));
	registerIntToken(MIR_SUBSTR, parseSubstr, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("substring of x starting from position y to z"));
	registerIntToken(MIR_SELECT, parseSelect, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,...)\t" LPGEN("the xth string of the arguments"));
	registerIntToken(MIR_SWITCH, parseSwitch, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z,...)\t" LPGEN("z if y equals x, multiple y and z arguments allowed"));
	registerIntToken(MIR_TRIM, parseTrim, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("removes white spaces in before and after x"));
	registerIntToken(MIR_TAB, parseTab, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("inserts x tab characters (x is optional)"));
	registerIntToken(MIR_UPPER, parseUpper, TRF_FUNCTION, LPGEN("String Functions") "\t(x)\t" LPGEN("converts x to upper case"));
	registerIntToken(MIR_WORD, parseWord, TRF_FUNCTION, LPGEN("String Functions") "\t(x,y,z)\t" LPGEN("words (separated by white spaces) number y to z from string x (z is optional)"));
}
