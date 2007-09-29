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

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * create_font - ÉtÉHÉìÉgÇçÏê¨Ç∑ÇÈ
 */
HFONT create_font(HWND hWnd, TCHAR *FontName, int FontSize, int Charset)
{
	LOGFONT lf;
	HDC hdc;

	ZeroMemory(&lf, sizeof(LOGFONT));

	hdc = GetDC(hWnd);
	lf.lfHeight = -(int)((FontSize * GetDeviceCaps(hdc,LOGPIXELSY)) / 72);
	ReleaseDC(hWnd, hdc);

	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = 0;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = Charset;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lstrcpy(lf.lfFaceName, FontName);
	return CreateFontIndirect((CONST LOGFONT *)&lf);
}
/* End of source */
