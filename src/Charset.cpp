/*
 * nPOP
 *
 * charset.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2012 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <mlang.h>

extern "C" {
#include "Charset.h"
#include "General.h"
#include "Memory.h"
#include "String.h"
extern TCHAR *AppDir;
}

/* Define */

/* Struct */
typedef struct _CHARSET {
	WCHAR *name;
	WCHAR chars[256];
	_CHARSET *next;
} CHARSET;

/* Global Variables */
MIMECPINFO* mimeCPInfos;
ULONG lFetchedCelt;
CHARSET *LoadedCharsets = NULL;

/* Local Function Prototypes */
static void codepage_init(void);
static void codepage_free(void);
static CHARSET *charset_load(WCHAR *charset);

/*
 * charset_init - OLE‚Ì‰Šú‰»
 */
void charset_init()
{
#ifndef _WIN32_WCE
	CoInitialize(NULL);
#else
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif
}

/*
 * charset_uninit - OLE‚Ì‰ğ•ú
 */
void charset_uninit()
{
	if (mimeCPInfos != NULL) {
		codepage_free();
	}
	CoUninitialize();
	CHARSET *next, *lcs = LoadedCharsets;
	while (lcs) {
		next = lcs->next;
		mem_free((void **)&lcs->name);
		mem_free((void **)&lcs);
		lcs = next;
	}
}

/*
 * codepage_init - ƒR[ƒhƒy[ƒWî•ñ‚Ì‰Šú‰»
 */
static void codepage_init(void)
{
	IMultiLanguage *pMultiLanguage;
	IEnumCodePage *spEnumCodePage;
	HRESULT hr = S_OK;

	if (mimeCPInfos != NULL) {
		codepage_free();
	}
	CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_ALL, __uuidof(IMultiLanguage), (void**)&pMultiLanguage);
	if (pMultiLanguage == NULL) {
		return;
	}
	hr = pMultiLanguage->EnumCodePages(MIMECONTF_SAVABLE_MAILNEWS, &spEnumCodePage);
	if (FAILED(hr)) {
		pMultiLanguage->Release();
		return;
	}
	UINT nCodePage = 0;
	hr = pMultiLanguage->GetNumberOfCodePageInfo(&nCodePage);
	if (FAILED(hr)) {
		pMultiLanguage->Release();
		return;
	}
	// —ñ‹“‚·‚é
	mimeCPInfos = (MIMECPINFO*)::CoTaskMemAlloc(sizeof(MIMECPINFO) * nCodePage);
	hr = spEnumCodePage->Next(nCodePage, mimeCPInfos, &lFetchedCelt);
	if (FAILED(hr)) {
		::CoTaskMemFree(mimeCPInfos);
		mimeCPInfos = NULL;
		pMultiLanguage->Release();
		return;
	}
	pMultiLanguage->Release();
}

/*
 * codepage_free - ƒR[ƒhƒy[ƒWî•ñ‚Ì‰ğ•ú
 */
static void codepage_free(void)
{
	if (mimeCPInfos != NULL) {
		::CoTaskMemFree(mimeCPInfos);
		mimeCPInfos = NULL;
	}
	lFetchedCelt = 0;
}

/*
 * charset_to_cp - Charset‚©‚çCodePage‚ğæ“¾
 */
DWORD charset_to_cp(const BYTE charset)
{
#ifdef UNICODE
	return CP_int;
#else
	if (mimeCPInfos == NULL) {
		codepage_init();
	}
	for (int i = 0; i < (int)lFetchedCelt; i++) {
		if (mimeCPInfos[i].bGDICharset == charset) {
			return mimeCPInfos[i].uiFamilyCodePage;
		}
	}
	return CP_int;
#endif
}

/*
 * charset_encode - ƒGƒ“ƒR[ƒh
 */
char *charset_encode(WCHAR *charset, WCHAR *buf, UINT len)
{
	IMultiLanguage *pMultiLanguage;
	MIMECSETINFO mimeInfo;
	DWORD mode = 0;
	DWORD encoding;
	char *ret;
	UINT src_len = len;
	UINT ret_len;

	CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_ALL, __uuidof(IMultiLanguage), (void**)&pMultiLanguage);
	if (pMultiLanguage == NULL) {
		return NULL;
	}
	mimeInfo.uiCodePage = 0;
	mimeInfo.uiInternetEncoding = 0;
	pMultiLanguage->GetCharsetInfo(charset, &mimeInfo);
	encoding = (mimeInfo.uiInternetEncoding == 0) ? mimeInfo.uiCodePage : mimeInfo.uiInternetEncoding;

	ret_len = 0;
	if (pMultiLanguage->ConvertStringFromUnicode(&mode, encoding, buf, &src_len, NULL, &ret_len) != S_OK) {
		pMultiLanguage->Release();
		return NULL;
	}
	ret = (char *)mem_alloc(sizeof(char) * (ret_len + 1));
	if (ret_len != 0 &&
		pMultiLanguage->ConvertStringFromUnicode(&mode, encoding, buf, &src_len, ret, &ret_len) != S_OK) {
		mem_free((void **)&ret);
		pMultiLanguage->Release();
		return NULL;
	}
	*(ret + ret_len) = '\0';
	pMultiLanguage->Release();
	return ret;
}

/*
 * charset_decode - ƒfƒR[ƒh
 */
WCHAR *charset_decode(WCHAR *charset, char *buf, UINT len)
{
	IMultiLanguage *pMultiLanguage;
	MIMECSETINFO mimeInfo;
	DWORD mode = 0;
	DWORD encoding;
	WCHAR *wret = NULL;
	UINT ret_len;
	BOOL is_8859_15 = FALSE;

	CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_ALL, __uuidof(IMultiLanguage), (void**)&pMultiLanguage);
	if (pMultiLanguage != NULL) {
		HRESULT gci;
		mimeInfo.uiCodePage = 0;
		mimeInfo.uiInternetEncoding = 0;
		gci = pMultiLanguage->GetCharsetInfo(charset, &mimeInfo);
		if (gci != S_OK && lstrcmpiW(charset, L"ISO-8859-15") == 0) {
			// for some reason, iso-8859-15 is often not available
			gci = pMultiLanguage->GetCharsetInfo(L"ISO-8859-1", &mimeInfo);
			if (gci == S_OK) {
				is_8859_15 = TRUE;
			}
		}
		if (gci == S_OK) {
			encoding = (mimeInfo.uiInternetEncoding == 0) ? mimeInfo.uiCodePage : mimeInfo.uiInternetEncoding;

			ret_len = 0;
			if (pMultiLanguage->ConvertStringToUnicode(&mode, encoding, buf, &len, NULL, &ret_len) == S_OK) {
				wret = (WCHAR *)mem_alloc(sizeof(WCHAR) * (ret_len + 1));
			}
		}
	}
	if (wret != NULL) {
		if (pMultiLanguage->ConvertStringToUnicode(&mode, encoding, buf, &len, wret, &ret_len) == S_OK) {
			*(wret + ret_len) = L'\0';
			if (is_8859_15 == TRUE) {
				// handle the 8 differences between iso-8859-1 and iso-8859-15
				WCHAR *p;
				for (p = wret; *p != L'\0'; p++) {
					switch (*p) {
						case 0x00A4: // ¤ becomes €
							*p = 0x20AC;
							break;
						case 0x00A6: // ¦ becomes Š
							*p = 0x0160;
							break;
						case 0x00A8: // ¨ becomes š
							*p = 0x0161;
							break;
						case 0x00B4: // ´ becomes 
							*p = 0x017D;
							break;
						case 0x00B8: // ¸ becomes 
							*p = 0x017E;
							break;
						case 0x00BC: // ¼ becomes Œ
							*p = 0x0152;
							break;
						case 0x00BD: // ½ becomes œ
							*p = 0x0153;
							break;
						case 0x00BE: // ¾ becomes Ÿ
							*p = 0x0178;
							break;
					}
				}
			}
		} else {
			mem_free((void **)&wret);
			wret = NULL;
		}
	}
	pMultiLanguage->Release();
	if (wret == NULL) {
		CHARSET *lcs = LoadedCharsets;
		while (lcs) {
			if (lstrcmpiW(lcs->name, charset) == 0) {
				break;
			}
			lcs = lcs->next;
		}
		if (lcs == NULL) {
			lcs = charset_load(charset);
			if (lcs != NULL) {
				lcs->next = LoadedCharsets;
				LoadedCharsets = lcs;
			}
		}
		if (lcs) {
			UINT i;
			ret_len = strlen(buf);
			wret = (WCHAR *)mem_alloc(sizeof(WCHAR) * (ret_len + 1));
			for (i = 0; i < ret_len; i++) {
				wret[i] = lcs->chars[ (unsigned char) buf[i] ];
			}
			*(wret + ret_len) = L'\0';
		}
	}
	return wret;
}

static int hex_val(int c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return -1; // invalid
}

/*
 * charset_load - look for file charset.txt
 *                format assumed to follow examples at
 *                ftp://ftp.unicode.org/Public/MAPPINGS/
 *                0x00 0x0000 # comment\n
 */

static CHARSET *charset_load(WCHAR *charset) {
	TCHAR fpath[BUF_SIZE];
	CHARSET *ret = NULL;
	long len;
	BOOL ok = TRUE;

	wsprintf(fpath, TEXT("%sResource\\%s.txt"), AppDir, charset);
	len = file_get_size(fpath);
	if (len > 256 * 8) { // must be at least this long
		char *buf, *p;
		buf	= file_read(fpath, len);
		if (buf != NULL) {
			ret = (CHARSET *)mem_calloc(sizeof(CHARSET));
		}
		for (p = buf; p && ok; p++) {
			while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
			if (*p == '\0') break;
			if (*p == '#') { // comment (until newline)
				while (*p != '\0' && *p != '\n') p++;
			} else {
				int pos=-1, val=-1, a, b, c, d;
				if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X') && *(p+2) && *(p+3)) {
					a = hex_val(*(p+2));
					b = hex_val(*(p+3));
					if (a >= 0 && b >= 0) {
						pos = 16 * a + b;
					}
				}
				p+=4;
				while (*p == ' ' || *p == '\t') p++;
				if (*p == '0' && (*(p+1) == 'x' || *(p+1) == 'X')
					&& *(p+2) && *(p+3) && *(p+4) && *(p+5)) {
					a = hex_val(*(p+2));
					b = hex_val(*(p+3));
					c = hex_val(*(p+4));
					d = hex_val(*(p+5));
					if (a >= 0 && b >= 0 && c >= 0 && d >= 0) {
						val = ((((a * 16) + b) * 16) + c) * 16 + d;
					}
				}
				if (val >= 0 && val < 0xFFFE && pos >= 0 && pos < 256) {
					ret->chars[pos] = val;
					while(*p != '\0' && *p != '\n') p++;
				} else {
					ok = FALSE;
					break;
				}
			}
		}
		mem_free((void**)&buf);
		if (ok) {
			ret->name = (WCHAR *)mem_alloc(sizeof(WCHAR) * (lstrlenW(charset) + 1));
			lstrcpyW(ret->name, charset);
		} else {
			mem_free((void**)&ret);
			ret = NULL;
		}
	}
	return ret;
}

/*
 * charset_enum - ƒLƒƒƒ‰ƒNƒ^ƒZƒbƒg‚Ì—ñ‹“
 */
HRESULT charset_enum(HWND hWnd)
{
	TCHAR lst_buf[256];

	if (mimeCPInfos == NULL) {
		codepage_init();
	}
	for (int i = 0; i < (int)lFetchedCelt; i++) {
		if (*mimeCPInfos[i].wszBodyCharset == L'_') {
			continue;
		}
#ifdef UNICODE
		for (int j = 0; j < SendMessage(hWnd, CB_GETCOUNT, 0, 0); j++) {
			SendMessage(hWnd, CB_GETLBTEXT, j, (LPARAM)lst_buf);
			if (lstrcmpi(mimeCPInfos[i].wszBodyCharset, lst_buf) == 0) {
				SendMessage(hWnd, CB_DELETESTRING, j, 0);
				break;
			}
		}
		SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)mimeCPInfos[i].wszBodyCharset);
#else
		char *buf;

		buf = alloc_wchar_to_char(CP_int, mimeCPInfos[i].wszBodyCharset);
		if (buf == NULL) {
			return E_FAIL;
		}
		for (int j = 0; j < SendMessage(hWnd, CB_GETCOUNT, 0, 0); j++) {
			SendMessage(hWnd, CB_GETLBTEXT, j, (LPARAM)lst_buf);
			if (lstrcmpi(buf, lst_buf) == 0) {
				SendMessage(hWnd, CB_DELETESTRING, j, 0);
				break;
			}
		}
		SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)buf);
		mem_free((void **)&buf);
#endif
	}
	return S_OK;
}

/*
 * set_default_encode - ƒfƒtƒHƒ‹ƒg‚ÌƒGƒ“ƒR[ƒh‚ğİ’è
 */
void set_default_encode(const UINT cp, TCHAR **HeadCharset, TCHAR **BodyCharset)
{
	if (mimeCPInfos == NULL) {
		codepage_init();
	}
	for (int i = 0; i < (int)lFetchedCelt; i++) {
		if (*mimeCPInfos[i].wszBodyCharset == L'_') {
			continue;
		}
		if (mimeCPInfos[i].uiFamilyCodePage == cp || mimeCPInfos[i].uiCodePage == cp) {
#ifdef UNICODE
			if (*HeadCharset == NULL) {
				*HeadCharset = alloc_copy_t(mimeCPInfos[i].wszHeaderCharset);
			}
			if (*BodyCharset == NULL) {
				*BodyCharset = alloc_copy_t(mimeCPInfos[i].wszBodyCharset);
			}
#else
			if (*HeadCharset == NULL) {
				*HeadCharset = alloc_wchar_to_char(CP_int, mimeCPInfos[i].wszHeaderCharset);
			}
			if (*BodyCharset == NULL) {
				*BodyCharset = alloc_wchar_to_char(CP_int, mimeCPInfos[i].wszBodyCharset);
			}
#endif
			return;
		}
	}
}
/* End of source */
