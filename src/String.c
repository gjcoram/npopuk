/*
 * nPOP
 *
 * String.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2008 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include <windows.h>

#include "Memory.h"
#include "String.h"

/* Define */
#define to_lower_t(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)
#define to_lower(c)			((c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c)

/* Global Variables */
FINDPARTS *FindParts = NULL;
extern void ErrorMessage(HWND hWnd, TCHAR *buf);

/* Local Function Prototypes */
static int str_len_n(const TCHAR *buf, int len);

/*
 * a2i - 数字の文字列を数値(int)に変換する
 */
int a2i(const char *str)
{
	int num = 0;
	int m = 1;

	if (*str == '-') {
		m = -1;
		str++;
	} else if (*str == '+') {
		str++;
	}

	for (; *str >= '0' && *str <= '9'; str++) {
		num = 10 * num + (*str - '0');
	}
	return num * m;
}

#ifdef UNICODE
/*
 * a2i_t - TCHAR to int
 */
int a2i_t(const TCHAR *str)
{
	int num = 0;
	int m = 1;

	if (*str == TEXT('-')) {
		m = -1;
		str++;
	} else if (*str == TEXT('+')) {
		str++;
	}

	for (; *str >= TEXT('0') && *str <= TEXT('9'); str++) {
		num = 10 * num + (*str - TEXT('0'));
	}
	return num * m;
}
#endif

/*
 * delete_ctrl_char - コントロール文字を削除する
 */
void delete_ctrl_char(TCHAR *buf)
{
	TCHAR *p, *r;

	for (p = r = buf; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\t')) {
			*(r++) = TEXT(' ');
		} else if (*p != TEXT('\r') && *p != TEXT('\n')) {
			*(r++) = *p;
		}
	}
	*r = TEXT('\0');
}

/*
 * alloc_copy_t - バッファを確保して文字列をコピーする
 */
TCHAR *alloc_copy_t(const TCHAR *buf)
{
	TCHAR *ret;

	if (buf == NULL) {
		return NULL;
	}
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(buf) + 1));
	if (ret != NULL) {
		lstrcpy(ret, buf);
	}
	return ret;
}

/*
 * alloc_copy - バッファを確保して文字列をコピーする
 */
#ifdef UNICODE
char *alloc_copy(const char *buf)
{
	char *ret;

	if (buf == NULL) {
		return NULL;
	}
	ret = (char *)mem_alloc(sizeof(char) * (tstrlen(buf) + 1));
	if (ret != NULL) {
		tstrcpy(ret, buf);
	}
	return ret;
}
#endif

/*
 * alloc_tchar_to_char - メモリを確保して TCHAR を char に変換する
 */
#ifdef UNICODE
char *alloc_tchar_to_char(TCHAR *str)
{
	char *cchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = tchar_to_char_size(str);
	cchar = (char *)mem_alloc(len + 1);
	if (cchar == NULL) {
		return NULL;
	}
	tchar_to_char(str, cchar, len);
	return cchar;
}
#endif

/*
 * alloc_char_to_tchar - メモリを確保して char を TCHAR に変換する
 */
#ifdef UNICODE
TCHAR *alloc_char_to_tchar(const char *str)
{
	TCHAR *tchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = char_to_tchar_size(str);
	tchar = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (tchar == NULL) {
		return NULL;
	}
	char_to_tchar(str, tchar, len);
	return tchar;
}
#endif

/*
 * alloc_wchar_to_char - ワイド文字をASCII文字に変換
 */
#ifndef UNICODE
char *alloc_wchar_to_char(const UINT cp, WCHAR *str)
{
	char *cchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = WideCharToMultiByte(cp, 0, str, -1, NULL, 0, NULL, NULL);
	cchar = (char *)mem_alloc(len + 1);
	if (cchar == NULL) {
		return NULL;
	}
	WideCharToMultiByte(cp, 0, str, -1, cchar, len, NULL, NULL);
	return cchar;
}
#endif

/*
 * alloc_char_to_wchar - ASCII文字をワイド文字に変換
 */
#ifndef UNICODE
WCHAR *alloc_char_to_wchar(const UINT cp, char *str)
{
	WCHAR *tchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = MultiByteToWideChar(cp, 0, str, -1, NULL, 0);
	tchar = (WCHAR *)mem_alloc(sizeof(WCHAR) * (len + 1));
	if (tchar == NULL) {
		return NULL;
	}
	MultiByteToWideChar(cp, 0, str, -1, tchar, len);
	return tchar;
}
#endif

/*
 * str_join_t - 文字列を連結して最後の文字のアドレスを返す
 */
TCHAR * __cdecl str_join_t(TCHAR *ret, ... )
{
	va_list buf;
	TCHAR *str;

	va_start(buf, ret);

	str = va_arg(buf, TCHAR *);
	while (str != (TCHAR *)-1) {
		if (str != NULL) {
			while (*(ret++) = *(str++));
			ret--;
		}
		str = va_arg(buf, TCHAR *);
	}

	va_end(buf);
	return ret;
}

/*
 * str_join - 文字列を連結して最後の文字のアドレスを返す
 */
#ifdef UNICODE
char * __cdecl str_join(char *ret, ... )
{
	va_list buf;
	char *str;

	va_start(buf, ret);

	str = va_arg(buf, char *);
	while (str != (char *)-1) {
		if (str != NULL) {
			while (*(ret++) = *(str++));
			ret--;
		}
		str = va_arg(buf, char *);
	}

	va_end(buf);
	return ret;
}
#endif

/*
 * str_cpy_t - 文字列をコピーして最後の文字のアドレスを返す
 */
TCHAR *str_cpy_t(TCHAR *ret, TCHAR *buf)
{
	if (buf == NULL) {
		*ret = TEXT('\0');
		return ret;
	}
 	while (*(ret++) = *(buf++));
	ret--;
	return ret;
}

/*
 * str_cpy - 文字列をコピーして最後の文字のアドレスを返す
 */
#ifdef UNICODE
char *str_cpy(char *ret, char *buf)
{
	if (buf == NULL) {
		*ret = '\0';
		return ret;
	}
	while (*(ret++) = *(buf++));
	ret--;
	return ret;
}
#endif

/*
 * str_cpy_n_t - 指定された文字数まで文字列をコピーする
 */
void str_cpy_n_t(TCHAR *ret, TCHAR *buf, int len)
{
	if (len <= 0) return;
	while (--len && (*(ret++) = *(buf++)));
	*ret = TEXT('\0');
}

/*
 * str_cpy_n - 指定された文字数まで文字列をコピーする
 */
#ifdef UNICODE
void str_cpy_n(char *ret, char *buf, int len)
{
	while (--len && (*(ret++) = *(buf++)));
	*ret = '\0';
}
#endif

/*
 * str_cpy_f_t - 指定の文字までの文字列をコピーする
 */
TCHAR *str_cpy_f_t(TCHAR *ret, TCHAR *buf, TCHAR c)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != c && *p != TEXT('\0'); p++, r++) {
#ifndef UNICODE
		// 2バイトコードの場合は2バイト進める
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			*(r++) = *(p++);
		}
#endif
		*r = *p;
	}
	*r = TEXT('\0');
	return ((*p == c) ? p + 1 : p);
}

/*
 * str_cpy_f - 指定の文字までの文字列をコピーする
 */
#ifdef UNICODE
char *str_cpy_f(char *ret, char *buf, char c)
{
	char *p, *r;

	for (p = buf, r = ret; *p != c && *p != '\0'; p++, r++) {
		*r = *p;
	}
	*r = '\0';
	return ((*p == c) ? p + 1 : p);
}
#endif

/*
 * str_cat_n - 指定された文字数まで文字列を追加する
 */
void str_cat_n(TCHAR *ret, char *buf, int len)
{
	TCHAR *p, *r;
	int i;

#ifdef UNICODE
	TCHAR *tBuf;

	p = tBuf = alloc_char_to_tchar(buf);
	if (p == NULL) {
		return;
	}
#else
	p = buf;
#endif

	i = lstrlen(ret);
	r = ret + i;
	while (*p != TEXT('\0') && i < len) {
		*(r++) = *(p++);
		i++;
	}
	*r = TEXT('\0');

#ifdef UNICODE
	mem_free(&tBuf);
#endif
}

/*
 * str_cmp_i_t - 文字列の大文字小文字を区別しない比較を行う (TCHAR)
 */
int str_cmp_i_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (to_lower_t(*buf1) == to_lower_t(*buf2)) {
		if (*buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

/*
 * str_cmp_i - 文字列の大文字小文字を区別しない比較を行う
 */
#ifdef UNICODE
int str_cmp_i(const char *buf1, const char *buf2)
{
	while (to_lower(*buf1) == to_lower(*buf2)) {
		if (*buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
#endif

/*
 * str_cmp_ni_t - 文字列の大文字小文字を区別しない比較を行う (TCHAR)
 */
int str_cmp_ni_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (to_lower_t(*buf1) == to_lower_t(*buf2)) {
		len--;
		if (len <= 0 || *buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

/*
 * str_cmp_n_t - case-sensitive TCHAR compare (GJC)
 */
int str_cmp_n_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (*buf1 == *buf2) {
		len--;
		if (len <= 0 || *buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

#ifdef UNICODE
/*
 * str_cmp_ni - 文字列の大文字小文字を区別しない比較を行う
 */
int str_cmp_ni(const char *buf1, const char *buf2, int len)
{
	while (to_lower(*buf1) == to_lower(*buf2)) {
		len--;
		if (len <= 0 || *buf1 == '\0') {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
/*
 * str_cmp_n_t - case-sensitive char compare
 */
int str_cmp_n(const char *buf1, const char *buf2, int len)
{
	while (*buf1 == *buf2) {
		len--;
		if (len <= 0 || *buf1 == '\0') {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
#endif

/*
 * str_match_t - 2つの文字列をワイルドカード(*)を使って比較を行う
 */
BOOL str_match_t(const TCHAR *ptn, const TCHAR *str)
{
	switch (*ptn) {
	case TEXT('\0'):
		return (*str == TEXT('\0'));
	case TEXT('*'):
		if (*(ptn + 1) == TEXT('\0')) {
			return TRUE;
		}
		if (str_match_t(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != TEXT('\0')) {
			str++;
			if (str_match_t(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case TEXT('?'):
		return (*str != TEXT('\0')) && str_match_t(ptn + 1, str + 1);
	default:
		while (to_lower_t(*ptn) == to_lower_t(*str)) {
			if (*ptn == TEXT('\0')) {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == TEXT('*') || *ptn == TEXT('?')) {
				return str_match_t(ptn, str);
			}
		}
		return FALSE;
	}
}

/*
 * str_match - 2つの文字列をワイルドカード(*)を使って比較を行う
 */
#ifdef UNICODE
BOOL str_match(const char *ptn, const char *str)
{
	switch (*ptn) {
	case '\0':
		return (*str == '\0');
	case '*':
		if (*(ptn + 1) == '\0') {
			return TRUE;
		}
		if (str_match(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != '\0') {
			str++;
			if (str_match(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case '?':
		return (*str != '\0') && str_match(ptn + 1, str + 1);
	default:
		while (to_lower(*ptn) == to_lower(*str)) {
			if (*ptn == '\0') {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == '*' || *ptn == '?') {
				return str_match(ptn, str);
			}
		}
		return FALSE;
	}
}
#endif

/*
 * str_len_n - 比較用の文字列の長さを取得する
 */
static int str_len_n(const TCHAR *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (*buf == TEXT('\0')) {
			break;
		}
		buf++;
	}
	return i;
}

/*
 * str_find - find pattern ptn in string str
 *          - or wildcard-pattern (parsed into FINDPARTS) fp in str
 */
TCHAR *str_find(TCHAR *ptn, TCHAR *str, int case_flag, FINDPARTS *fp, int *len)
{
	TCHAR *p;
	int len1, len2, len3;
	DWORD lcid;
	if (fp != NULL) {
		ptn = fp->str;
	}
	if (ptn && *ptn != TEXT('\0')) {
		len1 = lstrlen(ptn);
	} else {
		len1 = 0;
	}

	*len = -1;
	lcid = MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT);
	for (p = str; *p != TEXT('\0'); p++) {
		int match;
		if (len1 == 0) {
			match = 2;
			len2 = 0;
		} else {
			len2 = str_len_n(p, len1);
			match = CompareString(lcid, (case_flag) ? 0 : NORM_IGNORECASE, p, len2, ptn, len1);
		}
		if (match == 2) {
			if (fp == NULL) {
				*len = len2;
				break;
			} else if (fp->next == NULL) {
				len3 = lstrlen(p);
				if (fp->wild == TEXT('*')) {
					*len = len3;
					break;
				} else if (fp->wild == TEXT('?') && len3 > len2) {
					*len = len2 + 1;
#ifndef UNICODE
					if (IsDBCSLeadByte((BYTE)*(p+(*len))) == TRUE && *(p+(*len)+1) != TEXT('\0')) {
						*len++;
					}
#endif
					break;
				} else if (fp->wild == 0 || fp->wild == TEXT('\0')) {
					*len = len2;
					break;
				}
			} else {
				// fp->next != NULL
				TCHAR *r, *s;
				len3 = 0;
				if (fp->wild == TEXT('*')) {
					r = str_find(ptn, p+len2, case_flag, fp->next, &len3);
					if (*r != TEXT('\0')) {
						len3 += (r-p);
						*len = len3;
						break;
					}
				} else if (fp->wild == TEXT('?')) {
					r = p + len2 + 1;
#ifndef UNICODE
					if (IsDBCSLeadByte((BYTE)*r) == TRUE && *(r+1) != TEXT('\0')) {
						r++;
					}
#endif
					s = fp->next->str;
					if (s && *s != TEXT('\0')) {
						int len4 = lstrlen(s);
						len3 = str_len_n(r, len4);
						match = CompareString(lcid, (case_flag) ? 0 : NORM_IGNORECASE, r, len3, s, len4);
					} else {
						// original pattern must have had "??"
						match = 2;
					}
					if (match == 2) {
						if (fp->next->next != NULL || fp->next->wild != 0) {
							// this recursively deals with fp->next->wild and fp->next->next
							r = str_find(ptn, r, case_flag, fp->next, &len3);
						} // else r = r
						if (*r != TEXT('\0')) {
							len3 += (r-p);
							*len = len3;
							break;
						}
					} else {
						len3 = 0;
					}
				} else { 
					// no wildcard? then why is fp->next not null?
					ErrorMessage(NULL, TEXT("Wildcard parsing error"));
					len3 = 1;
				}
				if (len3 > 0) {
					*len = len3;
					break;
				}
			}
		}
#ifndef UNICODE
		// 2バイトコードの場合は2バイト進める
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
		}
#endif
	}
	return p;
}

/*
 * word_find_ni_t - search for pattern in all words of string
 */
BOOL word_find_ni_t(const TCHAR *ptn, const TCHAR *str, const int len)
{
	const TCHAR *p;
	p = str;
	while (*p != '\0') {
		while(!IS_ALNUM_UM_T(*p)) p++;
		if (str_cmp_ni_t(ptn, p, len) == 0) {
			return TRUE;
		}
		while (*p != TEXT(' ') && *p != TEXT('\0')) p++;
	}
	return FALSE;
}

void findparts_free()
{
	if (FindParts != NULL) {
		FINDPARTS *fp = FindParts, *fpn;
		while (fp) {
			mem_free(&(fp->str));
			fpn = fp->next;
			mem_free(&fp);
			fp = fpn;
		}
		FindParts = NULL;
	}
}

/*
 * ParseFindString - convert "abc*def??ghi" to {"abc",'*'},{"def",'?'},{"",'?'},{"ghi",'\0'}
 */
BOOL ParseFindString(TCHAR *str)
{
	FINDPARTS *fp;
	TCHAR *p, *t, *tmp;
	int in_str = 1;
	BOOL ret = TRUE;

	findparts_free();
	FindParts = fp = (FINDPARTS *)mem_calloc(sizeof(FINDPARTS));
	tmp = t = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(str) + 1));
	if (fp == NULL || tmp == NULL) {
		mem_free(&tmp);
		return FALSE;
	}
	*t = TEXT('\0');
	for (p = str; *p != TEXT('\0'); p++) {
		BOOL is_lead = FALSE;
#ifdef UNICODE
		if (*p == TEXT('\\') && *(p + 1) != TEXT('\0')) {
#else
		is_lead = IsDBCSLeadByte((BYTE)*p);
		if (( is_lead == TRUE || *p == TEXT('\\')) && *(p + 1) != TEXT('\0')) {
#endif
			if (!in_str) {
				if ((fp->next = (FINDPARTS *)mem_calloc(sizeof(FINDPARTS))) == NULL) {
					ret = FALSE;
					break;
				}
				fp = fp->next;
				in_str = 1;
			}
			if (is_lead == TRUE) {
				*(t++) = *(p++);
			} else {
				// de-escape: \? becomes ?
				p++;
			}
			*(t++) = *p;
		} else if (*p == TEXT('*') || *p == TEXT('?')) {
			if (in_str) {
				// end of a string
				*t = TEXT('\0');
				fp->str = alloc_copy_t(tmp);
				t = tmp;
			} else if (fp->wild != TEXT('*') || *p != TEXT('*')) {
				// two wilds in sequence
				// ** is the same as *, but ?* is different from *
				// "a?*e" matches "axe" but not "aerie"
				if ((fp->next = (FINDPARTS *)mem_calloc(sizeof(FINDPARTS))) == NULL) {
					ret = FALSE;
					break;
				}
				fp = fp->next;
				fp->str = 0;
			}
			fp->wild = *p;
			in_str = 0;
		} else {
			if (!in_str) {
				if ((fp->next = (FINDPARTS *)mem_calloc(sizeof(FINDPARTS))) == NULL) {
					ret = FALSE;
					break;
				}
				fp = fp->next;
				in_str = 1;
			}
			*(t++) = *p;
		}
	}
	if (ret && in_str) {
		// end of a string
		*t = TEXT('\0');
		fp->str = alloc_copy_t(tmp);
	}
	fp->next = NULL;
	mem_free(&tmp);

	return ret;
}

/* End of source */