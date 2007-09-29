/**************************************************************************

	nPOP

	Font.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"


/******************************************************************************

	CreateEditFont

	ÉtÉHÉìÉgÇçÏê¨Ç∑ÇÈ

******************************************************************************/

HFONT CreateEditFont(HWND hWnd, TCHAR *FontName, int FontSize, int Charset)
{
	LOGFONT lf;
	HDC hdc;

	tZeroMemory(&lf, sizeof(LOGFONT));

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
	TStrCpy(lf.lfFaceName, FontName);
	return CreateFontIndirect((CONST LOGFONT *)&lf);
}
/* End of source */
