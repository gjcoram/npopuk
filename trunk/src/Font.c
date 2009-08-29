/*
 * nPOP
 *
 * Font.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include <windows.h>

#include "Memory.h"
#include "String.h"
#include "font.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * font_create_or_copy - create (if fi->name) or copy default (but resize)
 */
HFONT font_create_or_copy(HWND hWnd, HDC hdc, FONT_INFO *fi)
{
	HFONT retfont = NULL;
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(LOGFONT));

	if (fi->name != NULL && *fi->name != TEXT('\0')) {
		lf.lfHeight = -(int)((fi->size * GetDeviceCaps(hdc,LOGPIXELSY)) / 72);
		lf.lfWidth = 0;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = fi->weight;
		lf.lfItalic = fi->italic;
		lf.lfUnderline = FALSE;
		lf.lfStrikeOut = FALSE;
		lf.lfCharSet = fi->charset;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		lstrcpy(lf.lfFaceName, fi->name);
		retfont = CreateFontIndirect((CONST LOGFONT *)&lf);
	}

	if (retfont == NULL) {
#ifdef _WIN32_WCE
		if (GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf) == 0) {
#else
		if (GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf) == 0) {
#endif
			return NULL;
		}
		lf.lfHeight = -(int)((fi->size * GetDeviceCaps(hdc,LOGPIXELSY)) / 72);
		retfont = CreateFontIndirect((CONST LOGFONT *)&lf);
	}
	return retfont;
}

/*
 * font_get_charset - フォントのキャラセット取得
 */
int font_get_charset(const HDC hdc)
{
	TEXTMETRIC tm;

	GetTextMetrics(hdc, &tm);
	return tm.tmCharSet;
}

/*
 * font_select - フォントの選択
 */
#ifndef _WIN32_WCE
BOOL font_select(const HWND hWnd, FONT_INFO *fi)
{
	CHOOSEFONT cf;
	LOGFONT lf;
	HDC hdc;

	// フォント情報の作成
	ZeroMemory(&lf, sizeof(LOGFONT));
	hdc = GetDC(NULL);
	lf.lfHeight = -(int)((fi->size * GetDeviceCaps(hdc, LOGPIXELSY)) / 72);
	ReleaseDC(NULL, hdc);
	lf.lfWeight = fi->weight;
	lf.lfItalic = fi->italic;
	lf.lfCharSet = fi->charset;
	lstrcpy(lf.lfFaceName, fi->name);
	// フォント選択ダイアログを表示
	ZeroMemory(&cf, sizeof(CHOOSEFONT));
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	cf.nFontType = SCREEN_FONTTYPE;
	if (ChooseFont(&cf) == FALSE) {
		return FALSE;
	}
	// 設定取得
	mem_free(&fi->name);
	fi->name = alloc_copy_t(lf.lfFaceName);
	fi->weight = lf.lfWeight;
	fi->italic = lf.lfItalic;
	fi->charset = lf.lfCharSet;
	fi->size = cf.iPointSize / 10;
	return TRUE;
}
#endif
/* End of source */
