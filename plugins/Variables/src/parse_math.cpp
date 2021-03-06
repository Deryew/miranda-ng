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

static wchar_t *parseAdd(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return nullptr;

	int result = 0;
	for (unsigned int i = 1; i < ai->argc; i++)
		result += ttoi(ai->argv.w[i]);

	return itot(result);
}

static wchar_t *parseDiv(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	int val1 = ttoi(ai->argv.w[1]);
	int val2 = ttoi(ai->argv.w[2]);
	if (val2 == 0)
		return nullptr;

	return itot(val1 / val2);
}

static wchar_t *parseHex(ARGUMENTSINFO *ai)
{
	unsigned int i;
	wchar_t szVal[34];

	if (ai->argc != 3)
		return nullptr;

	int val = ttoi(ai->argv.w[1]);
	int padding = ttoi(ai->argv.w[2]);
	mir_snwprintf(szVal, L"%x", val);
	unsigned int zeros = max(padding - (signed int)mir_wstrlen(szVal), 0);
	wchar_t *res = (wchar_t*)mir_alloc((zeros + mir_wstrlen(szVal) + 3)*sizeof(wchar_t));
	if (res == nullptr)
		return nullptr;

	memset(res, 0, ((zeros + mir_wstrlen(szVal) + 3) * sizeof(wchar_t)));
	mir_wstrcpy(res, L"0x");
	for (i = 0; i < zeros; i++)
		*(res + 2 + i) = '0';

	mir_wstrcat(res, szVal);
	return res;
}

static wchar_t *parseMod(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	int val1 = ttoi(ai->argv.w[1]);
	int val2 = ttoi(ai->argv.w[2]);
	if (val2 == 0)
		return nullptr;

	return itot(val1 % val2);
}

static wchar_t *parseMul(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return nullptr;

	int result = ttoi(ai->argv.w[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		result *= ttoi(ai->argv.w[i]);

	return itot(result);
}

static wchar_t *parseMuldiv(ARGUMENTSINFO *ai)
{
	if (ai->argc != 4)
		return nullptr;

	if (ttoi(ai->argv.w[3]) == 0)
		return nullptr;

	return itot((ttoi(ai->argv.w[1])*ttoi(ai->argv.w[2])) / ttoi(ai->argv.w[3]));
}

static wchar_t *parseMin(ARGUMENTSINFO *ai)
{
	if (ai->argc < 2)
		return nullptr;

	int minVal = ttoi(ai->argv.w[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		minVal = min(ttoi(ai->argv.w[i]), minVal);

	return itot(minVal);
}

static wchar_t *parseMax(ARGUMENTSINFO *ai)
{
	if (ai->argc < 2)
		return nullptr;

	int maxVal = ttoi(ai->argv.w[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		maxVal = max(ttoi(ai->argv.w[i]), maxVal);

	return itot(maxVal);
}

static wchar_t *parseNum(ARGUMENTSINFO *ai)
{
	if (ai->argc != 3)
		return nullptr;

	int val = ttoi(ai->argv.w[1]);
	int padding = ttoi(ai->argv.w[2]);
	wchar_t *szVal = itot(val);
	if (szVal == nullptr)
		return nullptr;

	unsigned zeros = max(padding - (signed int)mir_wstrlen(szVal), 0);
	wchar_t *res = (wchar_t*)mir_alloc((zeros + mir_wstrlen(szVal) + 1)*sizeof(wchar_t));
	if (res == nullptr) {
		mir_free(szVal);
		return nullptr;
	}

	memset(res, 0, ((zeros + mir_wstrlen(szVal) + 1) * sizeof(wchar_t)));
	wchar_t *cur = res;
	for (unsigned i = 0; i < zeros; i++)
		*cur++ = '0';

	mir_wstrcat(res, szVal);
	mir_free(szVal);

	return res;
}

static wchar_t *parseRand(ARGUMENTSINFO *)
{
	return itot(rand());
}

static wchar_t *parseSub(ARGUMENTSINFO *ai)
{
	if (ai->argc < 3)
		return nullptr;

	int result = ttoi(ai->argv.w[1]);
	for (unsigned i = 2; i < ai->argc; i++)
		result -= ttoi(ai->argv.w[i]);

	return itot(result);
}

void registerMathTokens()
{
	registerIntToken(ADD, parseAdd, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y ,...)\t" LPGEN("x + y + ..."));
	registerIntToken(DIV, parseDiv, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y)\t" LPGEN("x divided by y"));
	registerIntToken(HEX, parseHex, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y)\t" LPGEN("converts decimal value x to hex value and padds to length y"));
	registerIntToken(MOD, parseMod, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y)\t" LPGEN("x modulo y (remainder of x divided by y)"));
	registerIntToken(MUL, parseMul, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y)\t" LPGEN("x times y"));
	registerIntToken(MULDIV, parseMuldiv, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y,z)\t" LPGEN("x times y divided by z"));
	registerIntToken(MIN, parseMin, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y,...)\t" LPGEN("minimum value of (decimal) arguments"));
	registerIntToken(MAX, parseMax, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y,...)\t" LPGEN("maximum value of (decimal) arguments"));
	registerIntToken(NUM, parseNum, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y)\t" LPGEN("pads decimal value x to length y with zeros"));
	registerIntToken(RAND, parseRand, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t()\t" LPGEN("random number"));
	registerIntToken(SUB, parseSub, TRF_FUNCTION, LPGEN("Mathematical Functions") "\t(x,y,...)\t" LPGEN("x - y - ..."));
	srand((unsigned int)GetTickCount());
}
