/*
 * nPOP
 *
 * String.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2012 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

#ifndef _INC_STRING_H
#define _INC_STRING_H

/* Include Files */
#include <windows.h>

/* Define */
#ifdef UNICODE
#define tchar_to_char_size(wbuf) (WCtoMB(CP_int, 0, wbuf, -1, NULL, 0, NULL, NULL))
#else
#define tchar_to_char_size(wbuf) (lstrlen(wbuf) + 1)
#endif

#ifdef UNICODE
#define tchar_to_char(wbuf, ret, len) (WCtoMB(CP_int, 0, wbuf, -1, ret, len, NULL, NULL))
#else
#define tchar_to_char(wbuf, ret, len) (str_cpy_n_t(ret, wbuf, len))
#endif

#ifdef UNICODE
#define char_to_tchar_size(buf) (MBtoWC(CP_int, 0, buf, -1, NULL, 0))
#else
#define char_to_tchar_size(buf) (lstrlen(buf) + 1)
#endif

#ifdef UNICODE
#define char_to_tchar(buf, wret, len) (MBtoWC(CP_int, 0, buf, -1, wret, len))
#else
#define char_to_tchar(buf, wret, len) (str_cpy_n_t(wret, buf, len))
#endif

#ifndef CP_UTF8 // also found in General.h
#define ADD_UTF8_CONVERTERS
#define CP_UTF8 65001
#endif
#ifdef ADD_UTF8_CONVERTERS
int WCtoMB(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte,
		LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
int MBtoWC(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
#else
#define WCtoMB WideCharToMultiByte
#define MBtoWC MultiByteToWideChar
#endif

#ifdef UNICODE
#define tstrcpy		strcpy
#else
#define tstrcpy		lstrcpy
#endif

#ifdef UNICODE
#define tstrcat		strcat
#else
#define tstrcat		lstrcat
#endif

#ifdef UNICODE
#define tstrcmp		strcmp
#else
#define tstrcmp		lstrcmp
#endif

#ifdef UNICODE
#define tstrlen		strlen
#else
#define tstrlen		lstrlen
#endif

/* Struct */

/* Global Variables */
extern int CP_int;

/* Function Prototypes */
int a2i(const char *str);
#ifdef UNICODE
int a2i_t(const TCHAR *str);
#endif
void delete_ctrl_char(TCHAR *buf);

TCHAR *alloc_copy_t(const TCHAR *buf);
#ifdef UNICODE
char *alloc_copy(const char *buf);
#else
#define alloc_copy alloc_copy_t
#endif
#ifdef UNICODE
char *alloc_tchar_to_char(TCHAR *str);
TCHAR *alloc_char_to_tchar(const char *str);
#else
#define alloc_tchar_to_char alloc_copy_t
#define alloc_char_to_tchar alloc_copy_t
#endif
#ifndef UNICODE
char *alloc_wchar_to_char(const UINT cp, WCHAR *str);
WCHAR *alloc_char_to_wchar(const UINT cp, char *str);
#endif

TCHAR * __cdecl str_join_t(TCHAR *ret, ... );
#ifdef UNICODE
char * __cdecl str_join(char *ret, ... );
#else
#define str_join str_join_t
#endif

TCHAR *str_cpy_t(TCHAR *ret, const TCHAR *buf);
#ifdef UNICODE
char *str_cpy(char *ret, const char *buf);
#else
#define str_cpy str_cpy_t
#endif
void str_cpy_n_t(TCHAR *ret, const TCHAR *buf, int len);
#ifdef UNICODE
void str_cpy_n(char *ret, const char *buf, int len);
#else
#define str_cpy_n str_cpy_n_t
#endif
TCHAR *str_cpy_f_t(TCHAR *ret, TCHAR *buf, TCHAR c);
#ifdef UNICODE
char *str_cpy_f(char *ret, char *buf, char c);
#else
#define str_cpy_f str_cpy_f_t
#endif
void str_cat_n(TCHAR *ret, char *buf, int len);

int str_cmp_i_t(const TCHAR *buf1, const TCHAR *buf2, int len);
#ifdef UNICODE
int str_cmp_i(const char *buf1, const char *buf2);
#else
#define str_cmp_i lstrcmpi
#endif
int str_cmp_ni_t(const TCHAR *buf1, const TCHAR *buf2, int len);
int str_cmp_n_t(const TCHAR *buf1, const TCHAR *buf2, int len);
#ifdef UNICODE
int str_cmp_ni(const char *buf1, const char *buf2, int len);
int str_cmp_n(const char *buf1, const char *buf2, int len);
#else
#define str_cmp_ni str_cmp_ni_t
#define str_cmp_n str_cmp_n_t
#endif

BOOL str_match_t(const TCHAR *ptn, const TCHAR *str);
#ifdef UNICODE
BOOL str_match(const char *ptn, const char *str);
#else
#define str_match str_match_t
#endif

typedef struct _FINDPARTS {
	TCHAR *str;
	TCHAR wild;
	struct _FINDPARTS *next;
} FINDPARTS;

void findparts_free();
BOOL ParseFindString(TCHAR *str);

TCHAR *str_find(TCHAR *ptn, TCHAR *str, int case_flag, FINDPARTS *fp, int *len);

BOOL word_find_ni_t(const TCHAR *ptn, const TCHAR *str, const int len);

// IS_ALNUM_PM: alpha, num, +, - (for timezone)
// IS_ALNUM_UM: alpha, num, _, - (for e-mail addresses)
// foo = isalnum('À'); (a) causes a run-time violation and (b) returns 0
#define IS_ALPHA(c)			((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= 'À' && c <= 'ÿ' && c != '×' && c!= '÷'))
#define IS_NUM(c)			(c >= '0' && c <= '9')
#define IS_ALNUM_PM(c)		(IS_NUM(c) || IS_ALPHA(c) || c == '+' || c == '-')
#define IS_ALPHA_T(c)		((c >= TEXT('a') && c <= TEXT('z')) || (c >= TEXT('A') && c <= TEXT('Z')) || (c >= TEXT('À') && c <= TEXT('ÿ') && c != TEXT('×') && c != TEXT('÷')))
#define IS_NUM_T(c)			(c >= TEXT('0') && c <= TEXT('9'))
#define IS_ALNUM_PM_T(c)	(IS_NUM_T(c) || IS_ALPHA_T(c) || c == TEXT('+') || c == TEXT('-'))
#define IS_ALNUM_UM_T(c)	(IS_NUM_T(c) || IS_ALPHA_T(c) || c == TEXT('_') || c == TEXT('-'))
#define IS_PAREN_QUOTE_T(c)	(c == TEXT('(') || c == TEXT(')') || c == TEXT('<') || c == TEXT('>') || c == TEXT('\"') || c == TEXT('\''))
#endif

#ifdef ADD_UTF8_CONVERTERS
// CP_UTF8 is not available on WinCE3 and below
// These functions provide replacement functionality
int ConvertUTF16toUTF8(const TCHAR *wstr, char *dest, int len, BOOL strict);
int ConvertUTF8toUTF16(const unsigned char *str, TCHAR *dest, int len, BOOL strict);
#endif

/* End of source */
