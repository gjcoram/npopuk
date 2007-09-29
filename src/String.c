/*
 * nPOP
 *
 * String.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include <windows.h>

#include "Memory.h"
#include "String.h"

/* Define */

/* Global Variables */

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
 * alloc_copy - バッファを確保して文字列をコピーする
 */
TCHAR *alloc_copy(const TCHAR *buf)
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
TCHAR *alloc_char_to_tchar(char *str)
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
 * str_join - 文字列を連結して最後の文字のアドレスを返す
 */
TCHAR * __cdecl str_join(TCHAR *ret, ... )
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
 * str_cpy_f - 指定の文字までの文字列をコピーする
 */
TCHAR *str_cpy_f(TCHAR *ret, TCHAR *buf, TCHAR c)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != c && *p != TEXT('\0'); p++, r++) {
		*r = *p;
	}
	*r = TEXT('\0');
	return ((*p == c) ? p + 1 : p);
}

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
 * str_cmp_i - 文字列の大文字小文字を区別しない比較を行う
 */
#ifdef UNICODE
int str_cmp_i(const char *buf1, const char *buf2)
{
	TCHAR *str1, *str2;
	int ret;
	int llen;

	llen = char_to_tchar_size(buf1);
	str1 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str1 == NULL) {
		return -1;
	}
	char_to_tchar(buf1, str1, llen);

	llen = char_to_tchar_size(buf2);
	str2 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str2 == NULL) {
		return -1;
	}
	char_to_tchar(buf2, str2, llen);

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		NORM_IGNORECASE, str1, -1, str2, -1) - 2;

	mem_free(&str1);
	mem_free(&str2);
	return ret;
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
 * str_cmp_ni_t - 文字列の大文字小文字を区別しない比較を行う (TCHAR)
 */
int str_cmp_ni_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	int ret;
	int len1, len2;

	len1 = str_len_n(buf1, len);
	len2 = str_len_n(buf2, len);

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		NORM_IGNORECASE, buf1, len1, buf2, len2);
	return ret - 2;
}

/*
 * str_cmp_ni - 文字列の大文字小文字を区別しない比較を行う
 */
#ifdef UNICODE
int str_cmp_ni(const char *buf1, const char *buf2, int len)
{
	TCHAR *str1, *str2;
	int ret;
	int llen;

	llen = ((unsigned int)len < tstrlen(buf1)) ? len : tstrlen(buf1);
	str1 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str1 == NULL) {
		return -1;
	}
	char_to_tchar(buf1, str1, llen);
	*(str1 + llen) = TEXT('\0');

	llen = ((unsigned int)len < tstrlen(buf2)) ? len : tstrlen(buf2);
	str2 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str2 == NULL) {
		return -1;
	}
	char_to_tchar(buf2, str2, llen);
	*(str2 + llen) = TEXT('\0');

	ret = str_cmp_ni_t(str1, str2, len);

	mem_free(&str1);
	mem_free(&str2);
	return ret;
}
#endif

/*
 * str_match_t - 2つの文字列をワイルドカード(*)を使って比較を行う
 */
BOOL str_match_t(const TCHAR *ptn, const TCHAR *str)
{
#define to_lower_t(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)
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
#define to_lower(c)		((c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c)
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
 * str_find - 文字列内に含まれる文字列を検索して位置を返す
 */
TCHAR *str_find(TCHAR *ptn, TCHAR *str, int case_flag)
{
	TCHAR *p;
	int len1, len2;

	len1 = lstrlen(ptn);
	for (p = str; *p != '\0'; p++) {
		len2 = str_len_n(p, len1);
		if (CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
			(case_flag) ? 0 : NORM_IGNORECASE, p, len2, ptn, len1) == 2) {
			break;
		}
#ifndef UNICODE
		// 2バイトコードの場合は2バイト進める
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
		}
#endif
	}
	return p;
}
/* End of source */
