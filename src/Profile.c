/*
 * nPOP
 *
 * Profile.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2011 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <tchar.h>

#include "Memory.h"
#include "Profile.h"
#include "Strtbl.h"
#include "String.h"

/* Define */
#define BUF_SIZE				256
#define ALLOC_SIZE				10

#ifndef CopyMemory
#define CopyMemory				memcpy
#endif

/* Global Variables */
typedef struct _KEY {
	TCHAR key_name[BUF_SIZE];
	int hash;
	TCHAR *string;
	BOOL comment_flag;
} KEY_INFO;

typedef struct _SECTION {
	TCHAR section_name[BUF_SIZE];
	int hash;
	KEY_INFO *key_info;
	int key_count;
	int key_size;
} SECTION_INFO;

static SECTION_INFO *section_info;
static int section_count;
static int section_size;

/* Local Function Prototypes */
static BOOL trim(TCHAR *buf);
static int str2hash(const TCHAR *str);
static BOOL write_ascii_file(const HANDLE hFile, const TCHAR *buf, const int len);
static BOOL check_file(const TCHAR *fname);

static BOOL section_add(const TCHAR *section_name);
static int section_find(const TCHAR *section_name);
static BOOL key_add(SECTION_INFO *si, const TCHAR *key_name, const TCHAR *str, const BOOL comment_flag);
static int key_find(const SECTION_INFO *si, const TCHAR *key_name);
static BOOL profile_write_data(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *str);

/*
 * trim - remove spaces and tabs from both ends of buf
 */
static BOOL trim(TCHAR *buf)
{
	TCHAR *p, *r;

	// 前後の空白を除いたポインタを取得
	for (p = buf; (*p == TEXT(' ') || *p == TEXT('\t')) && *p != TEXT('\0'); p++)
		;
	for (r = buf + lstrlen(buf) - 1; r > p && (*r == TEXT(' ') || *r == TEXT('\t')); r--)
		;
	*(r + 1) = TEXT('\0');

	// 元の文字列にコピー
	lstrcpy(buf, p);
	return TRUE;
}

/*
 * str2hash - 文字列のハッシュ値を取得
 */
static int str2hash(const TCHAR *str)
{
#define to_lower(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)
	int hash = 0;

	for (; *str != TEXT('\0'); str++) {
		if (*str != TEXT(' ')) {
			hash = ((hash << 1) + to_lower(*str));
		}
	}
	return hash;
}

/*
 * write_ascii_file - マルチバイトに変換して保存
 */
static BOOL write_ascii_file(const HANDLE hFile, const TCHAR *buf, const int len)
{
#ifdef UNICODE
	char *str;
	DWORD ret;
	int clen;

	clen = WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
	if ((str = (char *)mem_alloc(clen + 1)) == NULL) {
		return FALSE;
	}
	WideCharToMultiByte(CP_UTF8, 0, buf, -1, str, clen, NULL, NULL);
	if (WriteFile(hFile, str, clen - 1, &ret, NULL) == FALSE) {
		mem_free(&str);
		return FALSE;
	}
	mem_free(&str);
	return TRUE;
#else
	DWORD ret;

	return WriteFile(hFile, buf, len, &ret, NULL);
#endif
}

/*
 * check_file - ファイルが存在するかチェック
 */
static BOOL check_file(const TCHAR *fname)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if ((hFindFile = FindFirstFile(fname, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);

	if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		return TRUE;
	}
	return FALSE;
}

/*
 * section_add - セクションの追加
 */
static BOOL section_add(const TCHAR *section_name)
{
	SECTION_INFO *tmp_section;

	if (section_name == NULL || *section_name == TEXT('\0')) {
		return FALSE;
	}

	if (section_size < section_count + 1) {
		// 再確保
		section_size += ALLOC_SIZE;
		if ((tmp_section = (SECTION_INFO *)mem_calloc(sizeof(SECTION_INFO) * section_size)) == NULL) {
			return FALSE;
		}
		if (section_info != NULL) {
			CopyMemory(tmp_section, section_info, sizeof(SECTION_INFO) * section_count);
			mem_free(&section_info);
		}
		section_info = tmp_section;
	}
	// セクション追加
	str_cpy_n_t((section_info + section_count)->section_name, section_name, BUF_SIZE);
	trim((section_info + section_count)->section_name);
	(section_info + section_count)->hash = str2hash((section_info + section_count)->section_name);

	section_count++;
	return TRUE;
}

/*
 * section_find - セクションの検索
 */
static int section_find(const TCHAR *section_name)
{
	int hash;
	int i;

	if (section_info == NULL || section_name == NULL || *section_name == TEXT('\0')) {
		return -1;
	}

	hash = str2hash(section_name);
	for (i = 0; i < section_count; i++) {
		if ((section_info + i)->hash != hash) {
			continue;
		}
		if (lstrcmpi((section_info + i)->section_name, section_name) == 0) {
			return i;
		}
	}
	return -1;
}

/*
 * key_add - キーの追加
 */
static BOOL key_add(SECTION_INFO *si, const TCHAR *key_name, const TCHAR *str, const BOOL comment_flag)
{
	KEY_INFO *tmp_key;
	int index = -1;

	if (key_name == NULL || *key_name == TEXT('\0') || str == NULL) {
		return FALSE;
	}

	if (si->key_size < si->key_count + 1) {
		// 再確保
		si->key_size += ALLOC_SIZE;
		if ((tmp_key = (KEY_INFO *)mem_calloc(sizeof(KEY_INFO) * si->key_size)) == NULL) {
			return FALSE;
		}
		if (si->key_info != NULL) {
			CopyMemory(tmp_key, si->key_info, sizeof(KEY_INFO) * si->key_count);
			mem_free(&si->key_info);
		}
		si->key_info = tmp_key;
	}
	// キー追加
	(si->key_info + si->key_count)->string = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(str) + 1));
	if ((si->key_info + si->key_count)->string == NULL) {
		return FALSE;
	}
	str_cpy_n_t((si->key_info + si->key_count)->key_name, key_name, BUF_SIZE);
	trim((si->key_info + si->key_count)->key_name);
	if (comment_flag == FALSE) {
		(si->key_info + si->key_count)->hash = str2hash((si->key_info + si->key_count)->key_name);
	} else {
		(si->key_info + si->key_count)->hash = 0;
	}
	lstrcpy((si->key_info + si->key_count)->string, str);
	(si->key_info + si->key_count)->comment_flag = comment_flag;

	si->key_count++;
	return TRUE;
}

/*
 * key_find - キーの検索
 */
static int key_find(const SECTION_INFO *si, const TCHAR *key_name)
{
	int hash;
	int i;

	if (si->key_info == NULL || key_name == NULL || *key_name == TEXT('\0')) {
		return -1;
	}

	hash = str2hash(key_name);
	for (i = 0; i < si->key_count; i++) {
		if ((si->key_info + i)->comment_flag == TRUE ||
			(si->key_info + i)->hash != hash) {
			continue;
		}
		if (lstrcmpi((si->key_info + i)->key_name, key_name) == 0) {
			return i;
		}
	}
	return -1;
}

/*
 * profile_create - just set up memory
 */
BOOL profile_create(void) {
	if ((section_info = (SECTION_INFO *)mem_calloc(sizeof(SECTION_INFO) * ALLOC_SIZE)) == NULL) {
		return FALSE;
	}
	section_count = 1;
	section_size = ALLOC_SIZE;
	return TRUE;
}

/*
 * profile_initialize - 初期化
 */
BOOL profile_initialize(const TCHAR *file_path, const BOOL pw_only)
{
	HANDLE hFile;
	TCHAR *buf;
	char *cbuf;
	DWORD size_low, size_high;
	DWORD ret;
	long file_size;
#ifdef UNICODE
	int i, CP_ini;
	long len;
	char *p;
#endif

	// ファイルを開く
	hFile = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		if (check_file(file_path) == FALSE) {
			return TRUE;
		}
		return FALSE;
	}
	// 確保するサイズの取得
	if ((size_low = GetFileSize(hFile, &size_high)) == 0xFFFFFFFF) {
		CloseHandle(hFile);
		return FALSE;
	}
	file_size = (long)size_low;

	// 読み取る領域の確保
	if ((cbuf = (char *)mem_alloc(file_size + 1)) == NULL) {
		CloseHandle(hFile);
		return FALSE;
	}
	// ファイルを読み込む
	if (ReadFile(hFile, cbuf, size_low, &ret, NULL) == FALSE) {
		mem_free(&cbuf);
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	*(cbuf + file_size) = '\0';


#ifdef UNICODE
	// check if old format: nPOP uses CP_ACP (and has no Version)
	// nPOPuk used CP_ACP for Version < 3000
	// (except for a beta of 2.17 that supported op.Codepage)
	// Starting with Version = 3000, nPOPuk used CP_UTF8 (and ignored op.Codepage)
	CP_ini = CP_ACP;
	i = 0;
	for (p = cbuf; *p != '\0'; p++) {
		if (*p == '[') {
			if (i == 0) i = 1;
			else break; // left [GENERAL] for next section
		}
		if (str_cmp_n(p, "Version=", 8) == 0) {
			p += 8;
			if (atoi(p) >= 3000) {
				CP_ini = CP_UTF8;
				break;
			}
		} else if (str_cmp_n(p, "Codepage=", 9) == 0) {
			p += 9;
			if (*p == '\"') p++;
			if (str_cmp_n(p, "CP_ACP", 6) == 0) {
				CP_ini = CP_ACP;
			}if (str_cmp_n(p, "CP_UTF8", 7) == 0) {
				CP_ini = CP_UTF8;
			} else {
				CP_ini = atoi(p);
			}
		}
		while (*p != '\0') {
			if (*p == '\r' && *(p+1) == '\n') {
				p += 2;
				break;
			}
			p++;
		}
	}
	len = MultiByteToWideChar(CP_ini, 0, cbuf, -1, NULL, 0);
	if ((buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1))) == NULL) {
		mem_free(&cbuf);
		return FALSE;
	}
	MultiByteToWideChar(CP_ini, 0, cbuf, -1, buf, len);
	file_size = len;
	mem_free(&cbuf);
#else
	buf = cbuf;
#endif

	if (profile_create() == FALSE) {
		mem_free(&buf);
		return FALSE;
	}

	profile_parse(buf, file_size, pw_only);
	mem_free(&buf);
	return TRUE;
}
/*
 * profile_parse - parse string (from file or editor window)
 */
void profile_parse(TCHAR *buf, long len, BOOL general_only)
{
	TCHAR *p, *r, *s;
	TCHAR tmp[BUF_SIZE];
	BOOL Done = FALSE;
	int i;

	p = buf;
	while ((len > (p - buf)) && *p != TEXT('\0') && Done == FALSE) {
		for (r = p; (len > (r - buf)) && (*r != TEXT('\r') && *r != TEXT('\n')); r++)
			;

		switch (*p) {
		case TEXT('['):
			// セクションの追加
			if (p == r || *(r - 1) != TEXT(']')) {
				break;
			}
			*(r - 1) = TEXT('\0');
			if (general_only && section_count > 1) {
				Done = TRUE;
			} else {
				section_add(p + 1);
			}
			break;

		case TEXT('\r'):
		case TEXT('\n'):
			break;

		default:
			if (section_info == NULL || p == r) {
				break;
			}
			if (*p == TEXT('#') || *p == TEXT(';')) {
				// コメント
				for (s = tmp, i = 0; p < r && i < BUF_SIZE; p++, s++, i++) {
					*s = *p;
				}
				*s = TEXT('\0');
				key_add((section_info + section_count - 1), tmp, TEXT(""), TRUE);
			} else {
				// キーの追加
				for (s = tmp, i=0; p < r && i < BUF_SIZE; p++, s++, i++) {
					if (*p == TEXT('=')) {
						break;
					}
					*s = *p;
				}
				*s = TEXT('\0');
				if (*p == TEXT('=')) {
					p++;
				}
				*r = TEXT('\0');
				key_add((section_info + section_count - 1), tmp, p, FALSE);
			}
			if (len > (r - buf)) {
				r++;
			}
		}
		p = r;
		for (; (len > (p - buf)) && (*p == TEXT('\r') || *p == TEXT('\n')); p++)
			;
	}
	return;
}

/*
 * profile_flush - バッファをファイルに書き込む
 */
BOOL profile_flush(const TCHAR *file_path, TCHAR **general_only)
{
	HANDLE hFile;
	TCHAR *buf, *p, *name;
	int len;
	int i, j;

	if (section_info == NULL) {
		return FALSE;
	}

	// compute string length
	if (general_only) {
		len = lstrlen(STR_WARN_EDIT_RISK);
	} else {
		len = 0;
	}
	for (i = 0; i < section_count; i++) {
		if ((section_info + i)->key_info == NULL) {
			continue;
		}
		name = (section_info + i)->section_name;
		if (general_only && lstrcmp(name, GENERAL) != 0) {
			continue;
		}
		// セクション名
		if (i != 0) {
			len += lstrlen(name) + 4;
		}
		for (j = 0; j < (section_info + i)->key_count; j++) {
			if (*((section_info + i)->key_info + j)->key_name == TEXT('\0')) {
				continue;
			}
			// キー名
			len += lstrlen(((section_info + i)->key_info + j)->key_name);
			if (((section_info + i)->key_info + j)->comment_flag == FALSE) {
				len++;
				if (((section_info + i)->key_info + j)->string != NULL) {
					// 文字列
					len += lstrlen(((section_info + i)->key_info + j)->string);
				}
			}
			len += 2;
		}
		len += 2;
	}

	// allocate string
	if ((p = buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1))) == NULL) {
		return FALSE;
	}
	if (general_only) {
		lstrcpy(p, STR_WARN_EDIT_RISK);
		p += lstrlen(p);
	}
	// build string
	for (i = 0; i < section_count; i++) {
		if ((section_info + i)->key_info == NULL) {
			continue;
		}
		name = (section_info + i)->section_name;
		if (general_only && lstrcmp(name, GENERAL) != 0) {
			continue;
		}
		// セクション名
		if (i != 0) {
			*(p++) = TEXT('[');
			lstrcpy(p, name);
			p += lstrlen(p);
			*(p++) = TEXT(']');
			*(p++) = TEXT('\r');
			*(p++) = TEXT('\n');
		}
		for (j = 0; j < (section_info + i)->key_count; j++) {
			if (*((section_info + i)->key_info + j)->key_name == TEXT('\0')) {
				continue;
			}
			// キー名
			lstrcpy(p, ((section_info + i)->key_info + j)->key_name);
			p += lstrlen(p);
			if (((section_info + i)->key_info + j)->comment_flag == FALSE) {
				*(p++) = TEXT('=');
				if (((section_info + i)->key_info + j)->string != NULL) {
					// 文字列
					lstrcpy(p, ((section_info + i)->key_info + j)->string);
					p += lstrlen(p);
				}
			}
			*(p++) = TEXT('\r');
			*(p++) = TEXT('\n');
		}
		*(p++) = TEXT('\r');
		*(p++) = TEXT('\n');
	}
	*p = TEXT('\0');

	if (general_only) {
		// only requesting a string containing the [GENERAL] section
		*general_only = buf;
		return TRUE;
	}

	hFile = CreateFile(file_path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		mem_free(&buf);
		return FALSE;
	}
	if (write_ascii_file(hFile, buf, len) == FALSE) {
		mem_free(&buf);
		CloseHandle(hFile);
		return FALSE;
	}
	mem_free(&buf);
	FlushFileBuffers(hFile);
	CloseHandle(hFile);
	return TRUE;
}

/*
 * profile_free - 設定情報の解放
 */
void profile_free(void)
{
	int i, j;

	if (section_info == NULL) {
		return;
	}
	for (i = 0; i < section_count; i++) {
		if ((section_info + i)->key_info == NULL) {
			continue;
		}
		// キーの解放
		for (j = 0; j < (section_info + i)->key_count; j++) {
			if (((section_info + i)->key_info + j)->string != NULL) {
				mem_free(&((section_info + i)->key_info + j)->string);
			}
		}
		mem_free(&(section_info + i)->key_info);
	}
	mem_free(&section_info);
	section_info = NULL;
	section_count = 0;
	section_size = 0;
}

/*
 * profile_get_string - 文字列の取得
 */
long profile_get_string(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *default_str, TCHAR *ret, const long size)
{
	TCHAR *buf, *p;
	int section_index;
	int key_index;
	int len;

	// セクションの検索
	if ((section_index = section_find(section_name)) == -1) {
		str_cpy_n_t(ret, default_str, size);
		return lstrlen(ret);
	}

	// キーの検索
	key_index = key_find((section_info + section_index), key_name);
	if (key_index == -1 || ((section_info + section_index)->key_info + key_index)->string == NULL) {
		str_cpy_n_t(ret, default_str, size);
		return lstrlen(ret);
	}

	// 内容の取得
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(((section_info + section_index)->key_info + key_index)->string) + 1));
	if (buf != NULL) {
		lstrcpy(buf, ((section_info + section_index)->key_info + key_index)->string);
		trim(buf);
		p = (*buf == TEXT('\"')) ? buf + 1 : buf;
		if ((len = lstrlen(p)) > 0 && *(p + len - 1) == TEXT('\"')) {
			*(p + len - 1) = TEXT('\0');
		}
		str_cpy_n_t(ret, p, size);
		mem_free(&buf);
	} else {
		str_cpy_n_t(ret, ((section_info + section_index)->key_info + key_index)->string, size);
	}
	return lstrlen(ret);
}

/*
 * profile_alloc_string - バッファを確保して文字列の取得
 */
TCHAR *profile_alloc_string(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *default_str)
{
	TCHAR *buf;
	int section_index;
	int key_index;
	int len;

	// セクションの検索
	if ((section_index = section_find(section_name)) == -1) {
		if ((buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(default_str) + 1))) != NULL) {
			lstrcpy(buf, default_str);
			return buf;
		}
		return NULL;
	}

	// キーの検索
	key_index = key_find((section_info + section_index), key_name);
	if (key_index == -1 || ((section_info + section_index)->key_info + key_index)->string == NULL) {
		if ((buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(default_str) + 1))) != NULL) {
			lstrcpy(buf, default_str);
			return buf;
		}
		return NULL;
	}

	// 内容の取得
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(((section_info + section_index)->key_info + key_index)->string) + 1));
	if (buf != NULL) {
		lstrcpy(buf, ((section_info + section_index)->key_info + key_index)->string);
		trim(buf);
		if (*buf == TEXT('\"')) {
			lstrcpy(buf, buf + 1);
			if ((len = lstrlen(buf)) > 0 && *(buf + len - 1) == TEXT('\"')) {
				*(buf + len - 1) = TEXT('\0');
			}
		}
		return buf;
	}
	return NULL;
}

/*
 * profile_free_string - バッファの解放
 */
void profile_free_string(TCHAR *buf)
{
	mem_free(&buf);
}

/*
 * profile_get_int - 数値の取得
 */
int profile_get_int(const TCHAR *section_name, const TCHAR *key_name, const int default_str)
{
	TCHAR *buf, *p;
	int section_index;
	int key_index;
	int ret;
	int len;

	// セクションの検索
	if ((section_index = section_find(section_name)) == -1) {
		return default_str;
	}

	// キーの検索
	key_index = key_find((section_info + section_index), key_name);
	if (key_index == -1 || ((section_info + section_index)->key_info + key_index)->string == NULL) {
		return default_str;
	}

	// 内容の取得
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(((section_info + section_index)->key_info + key_index)->string) + 1));
	if (buf != NULL) {
		lstrcpy(buf, ((section_info + section_index)->key_info + key_index)->string);
		trim(buf);
		p = (*buf == TEXT('\"')) ? buf + 1 : buf;
		if ((len = lstrlen(p)) > 0 && *(p + len - 1) == TEXT('\"')) {
			*(p + len - 1) = TEXT('\0');
		}
		ret = _ttoi(p);
		mem_free(&buf);
	} else {
		ret = _ttoi(((section_info + section_index)->key_info + key_index)->string);
	}
	return ret;
}

/*
 * profile_write_data - データの書き込み
 */
static BOOL profile_write_data(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *str)
{
	int section_index;
	int key_index;
	int j;

	if (section_name == NULL) {
		return FALSE;
	}

	if (section_info == NULL) {
		// セクションの確保
		if ((section_info = (SECTION_INFO *)mem_calloc(sizeof(SECTION_INFO) * ALLOC_SIZE)) == NULL) {
			return FALSE;
		}
		section_count = 1;
		section_size = ALLOC_SIZE;
	}

	// セクションの検索
	if ((section_index = section_find(section_name)) == -1) {
		// セクションの追加
		if (section_add(section_name) == FALSE) {
			return FALSE;
		}
		section_index = section_count - 1;
	}

	if (key_name == NULL) {
		if ((section_info + section_index)->key_info != NULL) {
			// キーの削除
			for (j = 0; j < (section_info + section_index)->key_count; j++) {
				if (((section_info + section_index)->key_info + j)->string != NULL) {
					mem_free(&((section_info + section_index)->key_info + j)->string);
				}
			}
			mem_free(&(section_info + section_index)->key_info);
			(section_info + section_index)->key_info = NULL;
			(section_info + section_index)->key_count = 0;
			(section_info + section_index)->key_size = 0;
		}
		return TRUE;
	}

	// キーの検索
	if ((key_index = key_find((section_info + section_index), key_name)) == -1) {
		// キーの追加
		return key_add((section_info + section_index), key_name, str, FALSE);
	} else {
		// 内容の変更
		if (((section_info + section_index)->key_info + key_index)->string != NULL) {
			mem_free(&((section_info + section_index)->key_info + key_index)->string);
		}
		if (str == NULL) {
			*((section_info + section_index)->key_info + key_index)->key_name = TEXT('\0');
			((section_info + section_index)->key_info + key_index)->string = NULL;
			return TRUE;
		}
		((section_info + section_index)->key_info + key_index)->string = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(str) + 1));
		if (((section_info + section_index)->key_info + key_index)->string == NULL) {
			return FALSE;
		}
		lstrcpy(((section_info + section_index)->key_info + key_index)->string, str);
	}
	return TRUE;
}

/*
 * profile_write_string - 文字列の書き込み
 */
BOOL profile_write_string(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *str)
{
	TCHAR *buf, *p;
	BOOL ret;

	if (str == NULL || *str == TEXT('\0')) {
		return profile_write_data(section_name, key_name, TEXT(""));
	}

	if ((buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(str) + 3))) == NULL) {
		return profile_write_data(section_name, key_name, str);
	}
	p = buf;
	*(p++) = TEXT('"');
	lstrcpy(p, str);
	p += lstrlen(p);
	*(p++) = TEXT('"');
	*(p++) = TEXT('\0');
	ret = profile_write_data(section_name, key_name, buf);
	mem_free(&buf);
	return ret;
}

/*
 * profile_write_int - 数値の書き込み
 */
BOOL profile_write_int(const TCHAR *section_name, const TCHAR *key_name, const int num)
{
	TCHAR ret[BUF_SIZE];

	wsprintf(ret, TEXT("%d"), num);
	return profile_write_data(section_name, key_name, ret);
}

/*
 * profile_delete_key - delete specific key from profile section (GJC)
 */
BOOL profile_delete_key(const TCHAR *section_name, const TCHAR *key_name)
{
	int section_index;
	int key_index;

	if ((section_index = section_find(section_name)) == -1) {
		return FALSE;
	}
	if ((key_index = key_find((section_info + section_index), key_name)) == -1) {
		return FALSE;
	}
	if (((section_info + section_index)->key_info + key_index)->string != NULL) {
		mem_free(&((section_info + section_index)->key_info + key_index)->string);
	}
	*((section_info + section_index)->key_info + key_index)->key_name = TEXT('\0');
	((section_info + section_index)->key_info + key_index)->string = NULL;
	return TRUE;
}

/*
 * profile_find_section - check if section exists (GJC)
 */
BOOL profile_find_section(const TCHAR *section_name)
{
	if (section_find(section_name) == -1) {
		return FALSE;
	}
	return TRUE;
}

/*
 * profile_clear_section - delete all keys from profile section (GJC)
 */
BOOL profile_clear_section(const TCHAR *section_name)
{
	SECTION_INFO *section_to_del;
	int j;
	int section_index = section_find(section_name);
	if (section_index == -1) {
		return FALSE;
	}
	section_to_del = (section_info + section_index);
	if (section_to_del->key_info == NULL) {
		return FALSE;
	}

	for (j = 0; j < section_to_del->key_count; j++) {
		*(section_to_del->key_info + j)->key_name = TEXT('\0');
		if ((section_to_del->key_info + j)->string != NULL) {
			mem_free(&(section_to_del->key_info + j)->string);
		}
	}
	mem_free(&section_to_del->key_info);
	section_to_del->key_info = NULL;
	section_to_del->key_count = 0;
	section_to_del->key_size = 0;

	return TRUE;
}

/* End of source */
