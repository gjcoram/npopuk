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
#include "General.h"
#include "Memory.h"
#include "global.h"
#include "md5.h"

/* Define */
#ifdef UNICODE
#define CSTEP				1
#else
#define CSTEP				2
#endif

#define MAILADDR_START		1
#define COMMENT_START		2
#define MAILADDR_END		3
#define COMMENT_END			4

#define DEF_TIME_ZONE		900

#define NULLCHECK_STRLEN(m)	((m != NULL) ? lstrlen(m) : 0)
#define IsBreakAlNum(c)		((c >= TEXT('a') && c <= TEXT('z')) || \
								(c >= TEXT('A') && c <= TEXT('Z')) || \
								c == TEXT('\'') || \
								(c >= TEXT('0') && c <= TEXT('9')))

/* Global Variables */
extern OPTION op;

/* Local Function Prototypes */
static int GetStrLen(const TCHAR *buf, int len);
static TCHAR *StrNextContentT(TCHAR *p);
#ifdef UNICODE
static char *StrNextContent(char *p);
#else
#define StrNextContent StrNextContentT
#endif
static void ReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag);
static void sReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag);
static BOOL URLHeadToItem(TCHAR *str, TCHAR *head, TCHAR **buf);
static TCHAR *GetNextQuote(TCHAR *buf, TCHAR qStr);

/*
 * AllocTcharToChar - ���������m�ۂ��� TCHAR �� char �ɕϊ�����
 */
#ifdef UNICODE
char *AllocTcharToChar(TCHAR *str)
{
	char *cchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = TcharToCharSize(str);
	cchar = (char *)mem_alloc(len + 1);
	if (cchar == NULL) {
		return NULL;
	}
	TcharToChar(str, cchar, len);
	return cchar;
}
#endif

/*
 * AllocCharToTchar - ���������m�ۂ��� char �� TCHAR �ɕϊ�����
 */
#ifdef UNICODE
TCHAR *AllocCharToTchar(char *str)
{
	TCHAR *tchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = CharToTcharSize(str);
	tchar = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (tchar == NULL) {
		return NULL;
	}
	CharToTchar(str, tchar, len);
	return tchar;
}
#endif

/*
 * a2i - �����̕�����𐔒l(int)�ɕϊ�����
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
 * DelCtrlChar - �R���g���[���������폜����
 */
void DelCtrlChar(TCHAR *buf)
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
 * AllocCopy - �o�b�t�@���m�ۂ��ĕ�������R�s�[����
 */
TCHAR *AllocCopy(const TCHAR *buf)
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
 * TStrJoin - �������A�����čŌ�̕����̃A�h���X��Ԃ�
 */
TCHAR * __cdecl TStrJoin(TCHAR *ret, ... )
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
 * TStrCpy - ��������R�s�[���čŌ�̕����̃A�h���X��Ԃ�
 */
TCHAR *TStrCpy(TCHAR *ret, TCHAR *buf)
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
 * StrCpy - ��������R�s�[���čŌ�̕����̃A�h���X��Ԃ�
 */
#ifdef UNICODE
char *StrCpy(char *ret, char *buf)
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
 * StrCpyN - �w�肳�ꂽ�������Ȃŕ�������R�s�[����
 */
#ifdef UNICODE
void StrCpyN(char *ret, char *buf, int len)
{
	while (--len && (*(ret++) = *(buf++)));
	*ret = '\0';
}
#endif

/*
 * TStrCpyN - �w�肳�ꂽ�������Ȃŕ�������R�s�[����
 */
void TStrCpyN(TCHAR *ret, TCHAR *buf, int len)
{
	while (--len && (*(ret++) = *(buf++)));
	*ret = TEXT('\0');
}

/*
 * TStrCpyF - �w��̕����܂ł̕�������R�s�[����
 */
TCHAR *TStrCpyF(TCHAR *ret, TCHAR *buf, TCHAR c)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != c && *p != TEXT('\0'); p++, r++) {
		*r = *p;
	}
	*r = TEXT('\0');
	return ((*p == c) ? p + 1 : p);
}

/*
 * StrCatN - �w�肳�ꂽ�������܂ŕ������ǉ�����
 */
void StrCatN(TCHAR *ret, char *buf, int len)
{
	TCHAR *p, *r;
	int i;

#ifdef UNICODE
	TCHAR *tBuf;

	p = tBuf = AllocCharToTchar(buf);
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
 * TStrCmp - ������̑啶������������ʂ����r���s��
 */
int TStrCmp(const TCHAR *buf1, const TCHAR *buf2)
{
	int ret;

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		0, buf1, -1, buf2, -1);
	return ret - 2;
}

/*
 * TStrCmpI - ������̑啶������������ʂ��Ȃ���r���s�� (TCHAR)
 */
int TStrCmpI(const TCHAR *buf1, const TCHAR *buf2)
{
	int ret;

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		NORM_IGNORECASE, buf1, -1, buf2, -1);
	return ret - 2;
}

/*
 * StrCmpI - ������̑啶������������ʂ��Ȃ���r���s��
 */
#ifdef UNICODE
int StrCmpI(const char *buf1, const char *buf2)
{
	TCHAR *str1, *str2;
	int ret;
	int llen;

	llen = CharToTcharSize(buf1);
	str1 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str1 == NULL) {
		return -1;
	}
	CharToTchar(buf1, str1, llen);

	llen = CharToTcharSize(buf2);
	str2 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str2 == NULL) {
		return -1;
	}
	CharToTchar(buf2, str2, llen);

	ret = TStrCmpI(str1, str2);

	mem_free(&str1);
	mem_free(&str2);
	return ret;
}
#endif

/*
 * GetStrLen - ��r�p�̕�����̒������擾����
 */
static int GetStrLen(const TCHAR *buf, int len)
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
 * TStrCmpNI - ������̑啶������������ʂ��Ȃ���r���s�� (TCHAR)
 */
int TStrCmpNI(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	int ret;
	int len1, len2;

	len1 = GetStrLen(buf1, len);
	len2 = GetStrLen(buf2, len);

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		NORM_IGNORECASE, buf1, len1, buf2, len2);
	return ret - 2;
}

/*
 * StrCmpNI - ������̑啶������������ʂ��Ȃ���r���s��
 */
#ifdef UNICODE
int StrCmpNI(const char *buf1, const char *buf2, int len)
{
	TCHAR *str1, *str2;
	int ret;
	int llen;

	llen = ((unsigned int)len < tstrlen(buf1)) ? len : tstrlen(buf1);
	str1 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str1 == NULL) {
		return -1;
	}
	CharToTchar(buf1, str1, llen);
	*(str1 + llen) = TEXT('\0');

	llen = ((unsigned int)len < tstrlen(buf2)) ? len : tstrlen(buf2);
	str2 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (llen + 1));
	if (str2 == NULL) {
		return -1;
	}
	CharToTchar(buf2, str2, llen);
	*(str2 + llen) = TEXT('\0');

	ret = TStrCmpNI(str1, str2, len);

	mem_free(&str1);
	mem_free(&str2);
	return ret;
}
#endif

/*
 * StrMatch - 2�̕���������C���h�J�[�h(*)���g���Ĕ�r���s��
 */
BOOL StrMatch(const TCHAR *ptn, const TCHAR *str)
{
#define ToLower(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)
	switch (*ptn) {
	case TEXT('\0'):
		return (*str == TEXT('\0'));
	case TEXT('*'):
		if (StrMatch(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != TEXT('\0')) {
			str++;
			if (StrMatch(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case TEXT('?'):
		return (*str != TEXT('\0')) && StrMatch(ptn + 1, str + 1);
	default:
		while (ToLower(*ptn) == ToLower(*str)) {
			if (*ptn == TEXT('\0')) {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == TEXT('*') || *ptn == TEXT('?')) {
				return StrMatch(ptn, str);
			}
		}
		return FALSE;
	}
}

/*
 * StrFind - ��������Ɋ܂܂�镶������������Ĉʒu��Ԃ�
 */
TCHAR *StrFind(TCHAR *ptn, TCHAR *str, int CaseFlag)
{
	TCHAR *p;
	int len1, len2;

	len1 = lstrlen(ptn);
	for (p = str; *p != '\0'; p++) {
		len2 = GetStrLen(p, len1);
		if (CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
			(CaseFlag) ? 0 : NORM_IGNORECASE, p, len2, ptn, len1) == 2) {
			break;
		}
#ifndef UNICODE
		// 2�o�C�g�R�[�h�̏ꍇ��2�o�C�g�i�߂�
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
		}
#endif
	}
	return p;
}

/*
 * StrNextContentT - �w�b�_���̎��̃R���e���c�̐擪�Ɉړ����� (TCHAR)
 */
static TCHAR *StrNextContentT(TCHAR *p)
{
	while (1) {
		for (; !(*p == TEXT('\r') && *(p + 1) == TEXT('\n')) && *p != TEXT('\0'); p++);
		if (*p == TEXT('\0')) {
			break;
		}
		p += 2;
		if (*p == TEXT(' ') || *p == TEXT('\t')) {
			continue;
		}
		break;
	}
	return p;
}

/*
 * GetHeaderStringPointT - �w�b�_����w��̍��ڂ̃R���e���c�̈ʒu���擾 (TCHAR)
 */
TCHAR *GetHeaderStringPointT(TCHAR *buf, TCHAR *str)
{
	TCHAR *p;
	int len;

	len = lstrlen(str);
	p = buf;
	while (1) {
		if (CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
			NORM_IGNORECASE, p, len, str, len) != 2) {
			// ���̃R���e���c�Ɉړ�����
			p = StrNextContentT(p);
			if (*p == TEXT('\0') || (*p == TEXT('\r') && *(p + 1) == TEXT('\n'))) {
				break;
			}
			continue;
		}

		p += len;
		for (; *p == TEXT(' ') || *p == TEXT('\t'); p++);
		return p;
	}
	return NULL;
}

/*
 * GetHeaderStringSizeT - �w�b�_����w��̍��ڂ̃R���e���c�̃T�C�Y���擾 (TCHAR)
 */
int GetHeaderStringSizeT(TCHAR *buf, BOOL CrLfFlag)
{
	TCHAR *p;
	int i;

	p = buf;
	i = 0;
	while (*p != TEXT('\0')) {
		if (*p == TEXT('\r') && *(p + 1) == TEXT('\n')) {
			p += 2;
			if (*p != TEXT(' ') && *p != TEXT('\t')) {
				break;
			}
			if (CrLfFlag == TRUE) {
				i += 2;
			} else {
				p++;
				continue;
			}
		}
		p++;
		i++;
	}
	return i;
}

/*
 * GetHeaderStringT - �w�b�_����w��̍��ڂ̃R���e���c���擾 (TCHAR)
 */
BOOL GetHeaderStringT(TCHAR *buf, TCHAR *ret, BOOL CrLfFlag)
{
	TCHAR *p, *r;

	p = buf;
	r = ret;
	while (*p != TEXT('\0')) {
		if (*p == TEXT('\r') && *(p + 1) == TEXT('\n')) {
			p += 2;
			if (*p != TEXT(' ') && *p != TEXT('\t')) {
				break;
			}
			if (CrLfFlag == TRUE) {
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
			} else {
				p++;
				continue;
			}
		}
		*(r++) = *(p++);
	}
	*r = TEXT('\0');
	return TRUE;
}

/*
 * StrNextContent - �w�b�_���̎��̃R���e���c�̐擪�Ɉړ����� (TCHAR)
 */
#ifdef UNICODE
static char *StrNextContent(char *p)
{
	while (1) {
		for (; !(*p == '\r' && *(p + 1) == '\n') && *p != '\0'; p++);
		if (*p == '\0') {
			break;
		}
		p += 2;
		if (*p == ' ' || *p == '\t') {
			continue;
		}
		break;
	}
	return p;
}
#endif

/*
 * GetHeaderStringPoint - �w�b�_����w��̍��ڂ̃R���e���c�̈ʒu���擾
 */
#ifdef UNICODE
char *GetHeaderStringPoint(char *buf, char *str)
{
	char *p;
	int len;

	len = tstrlen(str);
	p = buf;
	while (1) {
		if (StrCmpNI(p, str, len) != 0) {
			// ���̃R���e���c�Ɉړ�����
			p = StrNextContent(p);
			if (*p == '\0' || (*p == '\r' && *(p + 1) == '\n')) {
				break;
			}
			continue;
		}

		p += len;
		for (; *p == ' ' || *p == '\t'; p++);
		return p;
	}
	return NULL;
}
#endif

/*
 * GetHeaderStringSize - �w�b�_����w��̍��ڂ̃R���e���c�̃T�C�Y���擾
 */
#ifdef UNICODE
int GetHeaderStringSize(char *buf, BOOL CrLfFlag)
{
	char *p;
	int i;

	p = buf;
	i = 0;
	while (*p != '\0') {
		if (*p == '\r' && *(p + 1) == '\n') {
			p += 2;
			if (*p != ' ' && *p != '\t') {
				break;
			}
			if (CrLfFlag == TRUE) {
				i += 2;
			} else {
				p++;
				continue;
			}
		}
		p++;
		i++;
	}
	return i;
}
#endif

/*
 * GetHeaderString - �w�b�_����w��̍��ڂ̃R���e���c���擾
 */
#ifdef UNICODE
BOOL GetHeaderString(char *buf, char *ret, BOOL CrLfFlag)
{
	char *p, *r;

	p = buf;
	r = ret;
	while (*p != '\0') {
		if (*p == '\r' && *(p + 1) == '\n') {
			p += 2;
			if (*p != ' ' && *p != '\t') {
				break;
			}
			if (CrLfFlag == TRUE) {
				*(r++) = '\r';
				*(r++) = '\n';
			} else {
				p++;
				continue;
			}
		}
		*(r++) = *(p++);
	}
	*r = '\0';
	return TRUE;
}
#endif

/*
 * TrimMessageId - �����񂩂�Message-ID�݂̂𒊏o����
 */
void TrimMessageId(char *buf)
{
	char *p, *r;

	if (buf == NULL) {
		return;
	}

	for (p = buf; *p != '<' && *p != '\0'; p++);
	if (*p == '\0') {
		return;
	}

	for (r = p; *r != '>' && *r != '\0'; r++);
	if (*r == '\0') {
		return;
	}
	*(r + 1) = '\0';

	if (p != buf) {
		tstrcpy(buf, p);
	}
}

/*
 * GetReferencesSize - References��ۑ��p�ɕϊ������Ƃ��̃T�C�Y�̎擾
 */
int GetReferencesSize(char *p, BOOL Flag)
{
	char *t, *t2;

	t2 = t = p;
	while (*p != '\0') {
		if (*p == '>') {
			p++;
			for (; *p == ' ' || *p == '\t'; p++);
			if (*p == '\0') {
				break;
			}
			t2 = t;
			t = p;
		} else {
			p++;
		}
	}
	return tstrlen(((Flag == TRUE) ? t2 : t));
}

/*
 * ConvReferences - References��ۑ��p�ɕϊ�����
 */
BOOL ConvReferences(char *p, char *r, BOOL Flag)
{
	char *t, *t2;

	t2 = t = p;
	while (*p != '\0') {
		if (*p == '>') {
			p++;
			for (; *p == ' ' || *p == '\t'; p++);
			if (*p == '\0') {
				break;
			}
			t2 = t;
			t = p;
		} else {
			p++;
		}
	}
	tstrcpy(r, ((Flag == TRUE) ? t2 : t));
	TrimMessageId(r);
	return TRUE;
}

/*
 * GetBodyPointaT - Body�ʒu�̂��擾 (TCHAR)
 */
TCHAR *GetBodyPointaT(TCHAR *buf)
{
	TCHAR *p = buf;

	while (1) {
		for (; *p != TEXT('\0') && !(*p == TEXT('\r') && *(p + 1) == TEXT('\n')); p++);
		if (*p == TEXT('\0')) {
			break;
		}
		p += 2;
		if (*p == TEXT('\r') || *(p + 1) == TEXT('\n')) {
			return (p + 2);
		}
	}
	return NULL;
}

/*
 * GetBodyPointa - Body�ʒu�̂��擾
 */
#ifdef UNICODE
char *GetBodyPointa(char *buf)
{
	char *p = buf;

	while (1) {
		for (; *p != '\0' && !(*p == '\r' && *(p + 1) == '\n'); p++);
		if (*p == '\0') {
			break;
		}
		p += 2;
		if (*p == '\r' || *(p + 1) == '\n') {
			return (p + 2);
		}
	}
	return NULL;
}
#endif

/*
 * DateAdd - ���t�̌v�Z���s��
 */
void DateAdd(SYSTEMTIME *sTime, char *tz)
{
	TIME_ZONE_INFORMATION tmzi;
	SYSTEMTIME DiffSystemTime;
	FILETIME fTime;
	FILETIME DiffFileTime;
	__int64 LInt;
	__int64 DiffLInt;
	static int tmz = -1;
	int itz;
	int diff;
	DWORD ret;
	int f = 1;

	if (sTime == NULL || tmz == -1) {
		// �^�C���]�[���̎擾
		tmz = (op.TimeZone != NULL && *op.TimeZone != TEXT('\0')) ? _ttoi(op.TimeZone) : DEF_TIME_ZONE;
		tmz = tmz * 60 / 100;

		if (op.TimeZone == NULL || *op.TimeZone == TEXT('\0')) {
			ret = GetTimeZoneInformation(&tmzi);
			if (ret != 0xFFFFFFFF) {
				switch (ret) {
				case TIME_ZONE_ID_STANDARD:
					tmzi.Bias += tmzi.StandardBias;
					break;

				case TIME_ZONE_ID_DAYLIGHT:
					tmzi.Bias += tmzi.DaylightBias;
					break;
				}
				tmz = tmzi.Bias * -1;
			}
		}
		// �^�C���]�[���̐ݒ�̂�
		if (sTime == NULL) return;
	}

	// �ϊ����̃^�C���]�[����'��'�ɕϊ�
	if (*tz != '+' && *tz != '-') {
		if (StrCmpI(tz, "GMT") == 0 || StrCmpI(tz, "UT") == 0) {
			// �W������
			itz = 0;
		} else if (StrCmpI(tz, "EDT") == 0) {
			itz = -4;
		} else if (StrCmpI(tz, "EST") == 0 || StrCmpI(tz, "CDT") == 0) {
			itz = -5;
		} else if (StrCmpI(tz, "CST") == 0 || StrCmpI(tz, "MDT") == 0) {
			itz = -6;
		} else if (StrCmpI(tz, "MST") == 0 || StrCmpI(tz, "PDT") == 0) {
			itz = -7;
		} else if (StrCmpI(tz, "PST") == 0) {
			itz = -8;
		} else {
			return;
		}
	} else {
		itz = a2i(tz + 1);
		if (*tz == '-') {
			itz *= -1;
		}
	}
	itz = (itz / 100) * 60 + itz - (itz / 100) * 100;

	// ���̌v�Z
	diff = tmz - itz;
	if (diff == 0) {
		return;
	}
	if (diff < 0) {
		diff *= -1;
		f = -1;
	}

	// �v�Z����l��FileTime�ɕϊ�
	ZeroMemory(&DiffFileTime, sizeof(FILETIME));
	if (FileTimeToSystemTime(&DiffFileTime, &DiffSystemTime) == FALSE) {
		return;
	}
	DiffSystemTime.wHour += diff / 60;
	DiffSystemTime.wMinute += diff % 60;
	if (SystemTimeToFileTime(&DiffSystemTime, &DiffFileTime) == FALSE) {
		return;
	}
	CopyMemory(&DiffLInt, &DiffFileTime, sizeof(FILETIME));

	// FileTime�ɕϊ����ē��t�̌v�Z���s��
	if (SystemTimeToFileTime(sTime, &fTime) == FALSE) {
		return;
	}
	CopyMemory(&LInt, &fTime, sizeof(FILETIME));
	LInt += DiffLInt * f;
	CopyMemory(&fTime, &LInt, sizeof(FILETIME));
	FileTimeToSystemTime(&fTime, sTime);
}

/*
 * FormatDateConv - �t�H�[�}�b�g�ɂ��������ē��t�̓W�J���s��
 */
static int FormatDateConv(char *format, char *buf, SYSTEMTIME *gTime)
{
#define IS_ALPHA(c)		((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define IS_NUM(c)		(c >= '0' && c <= '9')
#define IS_ALNUM(c)		(IS_NUM(c) || IS_ALPHA(c) || c == '+' || c == '-')
	int i;
	char *p, *r, *t;
	char tmp[BUF_SIZE];
	char tz[BUF_SIZE];
	char month[12][4] = {
		{"Jan"}, {"Feb"}, {"Mar"}, {"Apr"},
		{"May"}, {"Jun"}, {"Jul"}, {"Aug"},
		{"Sep"}, {"Oct"}, {"Nov"}, {"Dec"},
	};

	ZeroMemory(gTime, sizeof(SYSTEMTIME));
	*tz = '\0';
	p = format;
	r = buf;
	while (*p != '\0') {
		switch (*p) {
		case 'w':	// �j��
			for (; IS_ALPHA(*r); r++);
			break;

		case 'd':	// ��
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wDay = a2i(tmp);
			if (gTime->wDay == 0) {
				return -1;
			}
			break;

		case 'M':	// ��(���l)
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wMonth = a2i(tmp);
			if (gTime->wMonth == 0) {
				return -1;
			}
			break;

		case 'm':	// ��
			for (t = tmp; IS_ALPHA(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			for (i = 0; i < 12; i++) {
				if (StrCmpNI(*(month + i), tmp, tstrlen(tmp) + 1) == 0) {
					break;
				}
			}
			if (i >= 12) {
				return -1;
			}
			gTime->wMonth = i + 1;
			break;

		case 'y':	// �N
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wYear = a2i(tmp);
			if (gTime->wYear < 1900) {
				if (gTime->wYear > 70) {
					gTime->wYear += 1900;
				} else {
					gTime->wYear += 2000;
				}
			}
			break;

		case 'h':	// ��
		case 'H':	// ��
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wHour = a2i(tmp);
			break;

		case 'n':	// ��
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wMinute = a2i(tmp);
			break;

		case 's':	// �b
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wSecond = a2i(tmp);
			break;

		case 't':	// TZ
			for (t = tz; IS_ALNUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			break;

		case '*':	// Not next char
			for (; *r != '\0' && *r != *(p + 1); r++);
			break;

		case ' ':	// Space
			for (; *r == ' '; r++);
			break;

		default:
			if (*p == *r) {
				r++;
			} else {
				return -1;
			}
			break;
		}
		p++;
	}
	DateAdd(gTime, tz);
	return 0;
}

/*
 * DateConv - ���t�`���̕ϊ����s�� (RFC 822, RFC 2822)
 */
int DateConv(char *buf, char *ret)
{
	SYSTEMTIME gTime;
	TCHAR fDay[BUF_SIZE];
	TCHAR fTime[BUF_SIZE];
#ifdef UNICODE
	TCHAR wret[BUF_SIZE];
#endif
	TCHAR *fmt;
	int i;

	if (tstrlen(buf) >= BUF_SIZE) {
		StrCpyN(ret, buf, BUF_SIZE - 1);
		return -1;
	}

	// ����������� SYSTEMTIME �ɕϊ�
	i = FormatDateConv("w, d m y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w, d-m-y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w, d m y h:n t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w d m y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w m d h:n:s y t", buf, &gTime);
	if (i == -1) {
		tstrcpy(ret, buf);
		return -1;
	}

	fmt = (op.DateFormat != NULL && *op.DateFormat != TEXT('\0')) ? op.DateFormat : NULL;
	if (GetDateFormat(0, 0, &gTime, fmt, fDay, BUF_SIZE - 1) == 0) {
		tstrcpy(ret, buf);
		return -1;
	}
	fmt = (op.TimeFormat != NULL && *op.TimeFormat != TEXT('\0')) ? op.TimeFormat : NULL;
	if (GetTimeFormat(0, 0, &gTime, fmt, fTime, BUF_SIZE - 1) == 0) {
		tstrcpy(ret, buf);
		return -1;
	}

#ifdef UNICODE
	TStrJoin(wret, fDay, TEXT(" "), fTime, (TCHAR *)-1);
	TcharToChar(wret, ret, BUF_SIZE);
#else
	TStrJoin(ret, fDay, TEXT(" "), fTime, (TCHAR *)-1);
#endif
	return 0;
}

/*
 * SortDateConv - �\�[�g�p�̓��t�`���ϊ����s��
 */
int SortDateConv(char *buf, char *ret)
{
	SYSTEMTIME gTime;
	TCHAR fDay[BUF_SIZE];
	TCHAR fTime[BUF_SIZE];
#ifdef UNICODE
	TCHAR wret[BUF_SIZE];
	char fmt[BUF_SIZE];
#endif
	char tmp[BUF_SIZE];
	char *p, *r;
	char c;
	int i;

	if (tstrlen(buf) >= BUF_SIZE ||
		op.DateFormat == NULL || *op.DateFormat == TEXT('\0') ||
		op.TimeFormat == NULL || *op.TimeFormat == TEXT('\0')) {
		StrCpyN(ret, buf, BUF_SIZE - 1);
		return -1;
	}

	// �`���̐���
	r = tmp;
#ifdef UNICODE
	TcharToChar(op.DateFormat, fmt, BUF_SIZE);
	p = fmt;
#else
	p = op.DateFormat;
#endif
	while (*p != '\0') {
		c = *(p++);
		switch (c) {
		case 'y':
		case ' ':
			*(r++) = c;
			for (; *p == c; p++);
			break;

		case 'M':
			if (StrCmpNI(p, "MMM", 3) == 0) {
				*(r++) = 'm';
			} else {
				*(r++) = 'M';
			}
			for (; *p == c; p++);
			break;

		case 'd':
			if (StrCmpNI(p, "dd", 2) == 0 || StrCmpNI(p, "ddd", 3) == 0) {
				*(r++) = '*';
			} else {
				*(r++) = 'd';
			}
			for (; *p == c; p++);
			break;

		case 'g':
			*(r++) = '*';
			for (; *p == c; p++);
			break;

		default:
			*(r++) = c;
			break;
		}
	}
	*(r++) = ' ';

#ifdef UNICODE
	TcharToChar(op.TimeFormat, fmt, BUF_SIZE);
	p = fmt;
#else
	p = op.TimeFormat;
#endif
	while (*p != '\0') {
		c = *(p++);
		switch (c) {
		case 'H':
		case 'h':
		case 's':
		case ' ':
			*(r++) = c;
			for (; *p == c; p++);
			break;

		case 'm':
			*(r++) = 'n';
			for (; *p == c; p++);
			break;

		case 't':
			*(r++) = '*';
			for (; *p == c; p++);
			break;

		default:
			*(r++) = c;
			break;
		}
	}
	*r = '\0';

	// ����������� SYSTEMTIME �ɕϊ�
	i = FormatDateConv(tmp, buf, &gTime);
	if (i == -1) {
		tstrcpy(ret, buf);
		return -1;
	}

	if (GetDateFormat(0, 0, &gTime, TEXT("yyyyMMdd"), fDay, BUF_SIZE - 1) == 0) {
		tstrcpy(ret, buf);
		return -1;
	}
	if (GetTimeFormat(0, 0, &gTime, TEXT("HHmm"), fTime, BUF_SIZE - 1) == 0) {
		tstrcpy(ret, buf);
		return -1;
	}

#ifdef UNICODE
	TStrJoin(wret, fDay, fTime, (TCHAR *)-1);
	TcharToChar(wret, ret, BUF_SIZE);
#else
	TStrJoin(ret, fDay, fTime, (TCHAR *)-1);
#endif
	return 0;
}

/*
 * GetTimeString - ���ԕ�����̎擾 (RFC 822, RFC 2822)
 */
void GetTimeString(TCHAR *buf)
{
	TIME_ZONE_INFORMATION tmzi;
	SYSTEMTIME st;
	DWORD ret;
	TCHAR c;
	int tmz;
	const TCHAR *Week[] = {
		TEXT("Sun"), TEXT("Mon"), TEXT("Tue"), TEXT("Wed"),
		TEXT("Thu"), TEXT("Fri"), TEXT("Sat"),
	};
	const TCHAR *Month[] = {
		TEXT("Jan"), TEXT("Feb"), TEXT("Mar"), TEXT("Apr"),
		TEXT("May"), TEXT("Jun"), TEXT("Jul"), TEXT("Aug"),
		TEXT("Sep"), TEXT("Oct"), TEXT("Nov"), TEXT("Dec"),
	};

	GetLocalTime(&st);

	tmz = (op.TimeZone != NULL && *op.TimeZone != TEXT('\0')) ? _ttoi(op.TimeZone) : DEF_TIME_ZONE;
	if (tmz < 0) {
		tmz *= -1;
		c = TEXT('-');
	} else {
		c = TEXT('+');
	}
	if (op.TimeZone == NULL || *op.TimeZone == TEXT('\0')) {
		ret = GetTimeZoneInformation(&tmzi);
		if (ret != 0xFFFFFFFF) {
			switch (ret) {
			case TIME_ZONE_ID_STANDARD:
				tmzi.Bias += tmzi.StandardBias;
				break;

			case TIME_ZONE_ID_DAYLIGHT:
				// �Ď��Ԃ̌v�Z
				tmzi.Bias += tmzi.DaylightBias;
				break;
			}

			if (tmzi.Bias < 0) {
				tmzi.Bias *= -1;
				c = TEXT('+');
			} else {
				c = TEXT('-');
			}
			tmz = (tmzi.Bias / 60) * 100 + tmzi.Bias % 60;
		}
	}
	wsprintf(buf, TEXT("%s, %d %s %d %02d:%02d:%02d %c%04d"), Week[st.wDayOfWeek],
		st.wDay, Month[st.wMonth - 1], st.wYear, st.wHour, st.wMinute, st.wSecond, c, tmz);
}

/*
 * EncodePassword_old - ��������Í��� (��)
 */
static void EncodePassword_old(TCHAR *buf, TCHAR *ret, int retsize)
{
	char *p, *r;
	int len;
	int i;

#ifdef UNICODE
	p = AllocTcharToChar(buf);
	if (p == NULL) {
		*ret = TEXT('\0');
		return;
	}
	r = (char *)mem_alloc(retsize);
	if (r == NULL) {
		mem_free(&p);
		*ret = TEXT('\0');
		return;
	}
#else
	p = buf;
	r = ret;
#endif

	Base64Decode(p, r);

	len = tstrlen(r);
	for (i = 0; i < len; i++) {
		*(r + i) ^= 0xFF;
	}
	*(r + i) = '\0';

#ifdef UNICODE
	CharToTchar(r, ret, retsize);
	mem_free(&p);
	mem_free(&r);
#endif
}

/*
 * EncodePassword - ��������Í���
 */
void EncodePassword(TCHAR *Key, TCHAR *Word, TCHAR *ret, int retsize, BOOL decode)
{
	MD5_CTX context;
	unsigned char digest[16];
	unsigned char *p, *r, *t;
	int len;
	int i, j;

	if (Word == NULL || *Word == TEXT('\0')) {
		*ret = TEXT('\0');
		return;
	}
	if (decode == TRUE && *Word != TEXT('=')) {
		EncodePassword_old(Word, ret, retsize);
		return;
	}

	// digest�l���擾
#ifdef UNICODE
	p = AllocTcharToChar((Key == NULL) ? TEXT('\0') : Key);
	if (p == NULL) {
		*ret = TEXT('\0');
		return;
	}
#else
	p = (Key == NULL) ? TEXT('\0') : Key;
#endif
	MD5Init(&context);
	MD5Update(&context, p, tstrlen(p));
	MD5Final(digest, &context);
#ifdef UNICODE
	mem_free(&p);
#endif

#ifdef UNICODE
	p = AllocTcharToChar(Word);
	if (p == NULL) {
		*ret = TEXT('\0');
		return;
	}
#else
	p = AllocCopy(Word);
	if (p == NULL) {
		*ret = TEXT('\0');
		return;
	}
#endif
	if (decode == TRUE) {
		// �f�R�[�h
		r = (char *)mem_alloc(tstrlen(p) + 1);
		if (r == NULL) {
			*ret = TEXT('\0');
			return;
		}
		t = Base64Decode(p + 1, r);
		len = t - r;
		mem_free(&p);
		p = r;

	} else {
		len = tstrlen(p);
	}

    // XOR
    for (i = 0, j = 0; i < len; i++, j++) {
		if (j >= 16) {
			j = 0;
		}
		*(p + i) = *(p + i) ^ digest[j];
    }
	*(p + i) = '\0';

	if (decode == TRUE) {
#ifdef UNICODE
		CharToTchar(p, ret, retsize);
#else
		lstrcpyn(ret, p, retsize);
#endif
	} else {
		// �G���R�[�h
		r = (char *)mem_alloc(len * 2 + 4 + 1);
		if (r == NULL) {
			*ret = TEXT('\0');
			return;
		}
		Base64Encode(p, r, len);
		mem_free(&p);
		p = r;

		*ret = TEXT('=');
#ifdef UNICODE
		CharToTchar(p, ret + 1, retsize - 1);
#else
		lstrcpyn(ret + 1, p, retsize - 1);
#endif
	}
	mem_free(&p);
}

/*
 * EncodeCtrlChar - ���䕶���� \�` �̌`���ɕϊ�����
 */
void EncodeCtrlChar(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	if (buf == NULL) {
		*ret = TEXT('\0');
		return;
	}
	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch (*p) {
		case TEXT('\t'):
			*(r++) = TEXT('\\');
			*(r++) = TEXT('t');
			break;

		case TEXT('\r'):
			break;

		case TEXT('\n'):
			*(r++) = TEXT('\\');
			*(r++) = TEXT('n');
			break;

		case TEXT('\\'):
			*(r++) = TEXT('\\');
			*(r++) = TEXT('\\');
			break;

		default:
			*(r++) = *p;
			break;
		}
	}
	*r = TEXT('\0');
}

/*
 * DecodeCtrlChar - \�` �̌`���̕�����𐧌䕶���ɕϊ�����
 */
void DecodeCtrlChar(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		if (*p != TEXT('\\')) {
			*(r++) = *p;
			continue;
		}
		p++;
		switch (*p) {
		case TEXT('t'):
			*(r++) = TEXT('\t');
			break;

		case TEXT('n'):
			*(r++) = TEXT('\r');
			*(r++) = TEXT('\n');
			break;

		case TEXT('\\'):
			*(r++) = TEXT('\\');
			break;

		default:
			break;
		}
	}
	*r = TEXT('\0');
}

/*
 * CreateMessageId - Message-Id �𐶐����� (RFC 822, RFC 2822)
 */
TCHAR *CreateMessageId(long id, TCHAR *MailAddress)
{
	SYSTEMTIME st;
	TCHAR *ret;
	int len;

	if (MailAddress == NULL) {
		return NULL;
	}
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (1 + 14 + 1 + 8 + 1 + lstrlen(MailAddress) + 1 + 1));
	if (ret == NULL) {
		return NULL;
	}

	GetLocalTime(&st);
	len = wsprintf(ret, TEXT("<%04d%02d%02d%02d%02d%02d.%08X"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
		id * ((long)ret + st.wSecond + st.wMilliseconds));
	TStrJoin(ret + len, TEXT("."), MailAddress, TEXT(">"), (TCHAR *)-1);
	return ret;
}

/*
 * CreateHeaderStringSize - �w�b�_��������쐬�����Ƃ��̃T�C�Y
 */
int CreateHeaderStringSize(TCHAR *buf, MAILITEM *tpMailItem)
{
	TCHAR *p;
	int ret = 0;

	if (buf == NULL) {
		return 0;
	}
	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
			ret += 2;
			continue;
		}
#endif
		if (*p != TEXT('%')) {
			ret++;
			continue;
		}
		p++;
		switch (*p) {
		case TEXT('F'): case TEXT('f'):
			ret += NULLCHECK_STRLEN(tpMailItem->From);
			break;

		case TEXT('I'): case TEXT('i'):
			ret += NULLCHECK_STRLEN(tpMailItem->MessageID);
			break;

		case TEXT('D'): case TEXT('d'):
			ret += NULLCHECK_STRLEN(tpMailItem->Date);
			break;

		case TEXT('S'): case TEXT('s'):
			ret += NULLCHECK_STRLEN(tpMailItem->Subject);
			break;

		case TEXT('T'): case TEXT('t'):
			ret += NULLCHECK_STRLEN(tpMailItem->To);
			break;

		case TEXT('C'): case TEXT('c'):
			ret += NULLCHECK_STRLEN(tpMailItem->Cc);
			break;

		case TEXT('B'): case TEXT('b'):
			ret += NULLCHECK_STRLEN(tpMailItem->Bcc);
			break;

		case TEXT('%'):
			ret++;
			break;

		default:
			break;
		}
	}
	return ret;
}

/*
 * CreateHeaderString - �w�b�_������̍쐬
 */
TCHAR *CreateHeaderString(TCHAR *buf, TCHAR *ret, MAILITEM *tpMailItem)
{
	TCHAR *p, *r, *t;

	if (buf == NULL) {
		return ret;
	}
	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		if (*p != TEXT('%')) {
			*(r++) = *p;
			continue;
		}
		p++;
		t = NULL;
		switch (*p) {
		// From:
		case TEXT('F'): case TEXT('f'):
			t = tpMailItem->From;
			break;

		// Message-ID:
		case TEXT('I'): case TEXT('i'):
			t = (tpMailItem->MessageID != NULL && *tpMailItem->MessageID == TEXT('<'))
				? tpMailItem->MessageID : NULL;
			break;

		// Date:
		case TEXT('D'): case TEXT('d'):
			t = tpMailItem->Date;
			break;

		// Subject:
		case TEXT('S'): case TEXT('s'):
			t = tpMailItem->Subject;
			break;

		// To:
		case TEXT('T'): case TEXT('t'):
			t = tpMailItem->To;
			break;

		// Cc:
		case TEXT('C'): case TEXT('c'):
			t = tpMailItem->Cc;
			break;

		// Bcc:
		case TEXT('B'): case TEXT('b'):
			t = tpMailItem->Bcc;
			break;

		// %
		case TEXT('%'):
			t = TEXT("%");
			break;
		}
		if (t != NULL) {
			r = TStrCpy(r, t);
		}
	}
	*r = TEXT('\0');
	return r;
}

/*
 * GetReplyBodySize - ���p�������t�������Ƃ��̕�����̃T�C�Y���擾
 */
int GetReplyBodySize(TCHAR *body, TCHAR *ReStr)
{
	TCHAR *p, *s;
	int len = 0;;

	if (ReStr == NULL || *ReStr == TEXT('\0')) {
		return lstrlen(body);
	}

	for (s = body + lstrlen(body) - 1; *s == '\r' || *s == '\n'; s--);

	len += lstrlen(ReStr);
	for (p = body; p <= s && *p != TEXT('\0'); p++) {
		if (*p == TEXT('\n')) {
			len++;
			if (*(p + 1) == TEXT('\0')) {
				break;
			}
			len += lstrlen(ReStr);
		} else {
			len++;
		}
	}
	return (len + 2);
}

/*
 * SetReplyBody - ������Ɉ��p�������ǉ�����
 */
TCHAR *SetReplyBody(TCHAR *body, TCHAR *ret, TCHAR *ReStr)
{
	TCHAR *p, *r, *s;

	if (ReStr == NULL || *ReStr == TEXT('\0')) {
		// ���p�����������ꍇ
		return TStrCpy(ret, body);
	}

	// �Ō���̋�s�͈��p���Ȃ�
	for (s = body + lstrlen(body) - 1; *s == '\r' || *s == '\n'; s--);

	// �擪�s�̈��p
	r = TStrCpy(ret, ReStr);
	for (p = body; p <= s && *p != TEXT('\0'); p++) {
		if (*p == TEXT('\n')) {
			// ���p
			*(r++) = *p;
			if (*(p + 1) == TEXT('\0')) {
				break;
			}
			r = TStrCpy(r, ReStr);
		} else {
			*(r++) = *p;
		}
	}
	return TStrCpy(r, TEXT("\r\n"));
}

/*
 * SetDotSize - �s���I�h����n�܂�s�̐擪�Ƀs���I�h��t�����邽�߂̃T�C�Y�̎擾
 *	(�s���� "From" �� ">From" �ɕϊ�)
 */

int SetDotSize(TCHAR *buf)
{
#define FROM_LEN		4		// lstrlen(TEXT("from"))
#define CRLF_FROM_LEN	6		// lstrlen(TEXT("\r\nfrom"))
	TCHAR *p;
	int len = 0;

	p = buf;
	if (TStrCmpNI(p, TEXT("from"), FROM_LEN) == 0) {
		len++;
	}
	for (; *p != TEXT('\0'); p++) {
		if (*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))) {
			len++;
		} else if (TStrCmpNI(p, TEXT("\r\nfrom"), CRLF_FROM_LEN) == 0) {
			len++;
		}
		len++;
	}
	return len;
}

/*
 * SetDot - �s���I�h����n�܂�s�̐擪�Ƀs���I�h��t������
 *	(�s���� "From" �� ">From" �ɕϊ�)
 */

void SetDot(TCHAR *buf, TCHAR *ret)
{
#define FROM_LEN		4		// lstrlen(TEXT("from"))
#define CRLF_FROM_LEN	6		// lstrlen(TEXT("\r\nfrom"))
	TCHAR *p, *r;

	p = buf;
	r = ret;
	if (TStrCmpNI(p, TEXT("from"), FROM_LEN) == 0) {
		*(r++) = TEXT('>');
	}
	for (; *p != TEXT('\0'); p++) {
		if (*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))) {
			*(r++) = TEXT('.');

		} else if (TStrCmpNI(p, TEXT("\r\nfrom"), CRLF_FROM_LEN) == 0) {
			*(r++) = *(p++);
			*(r++) = *(p++);
			*(r++) = TEXT('>');
		}
		*(r++) = *p;
	}
	*r = TEXT('\0');
}

/*
 * DelDot - �s���I�h����n�܂�s�̐擪�̃s���I�h���폜����
 */
void DelDot(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
		if (*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))) {
			continue;
		}
		*(r++) = *p;
	}
	*r = TEXT('\0');
}

/*
 * ReturnCheck - �S�p�����֑̋��`�F�b�N
 */
static void ReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag)
{
	TCHAR *t;

	if (op.Oida != NULL) {
		for (t = op.Oida; *t != TEXT('\0'); t += CSTEP) {
			if (TStrCmpNI(p, t, CSTEP) == 0) {
				*TopFlag = TRUE;
				return;
			}
		}
	}
	if (op.Bura != NULL) {
		for (t = op.Bura; *t != TEXT('\0'); t += CSTEP) {
			if (TStrCmpNI(p, t, CSTEP) == 0) {
				*EndFlag = TRUE;
				return;
			}
		}
	}
}

/*
 * sReturnCheck - ���p�����֑̋��`�F�b�N
 */
static void sReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag)
{
	TCHAR *t;

	if (op.sOida != NULL) {
		for (t = op.sOida; *t != TEXT('\0'); t++) {
			if (*p == *t) {
				*TopFlag = TRUE;
				return;
			}
		}
	}
	if (op.sBura != NULL) {
		for (t = op.sBura; *t != TEXT('\0'); t++) {
			if (*p == *t) {
				*EndFlag = TRUE;
				return;
			}
		}
	}
}

/*
 * WordBreakStringSize - ��������w��̒����Ő܂�Ԃ����Ƃ��̃T�C�Y
 */
int WordBreakStringSize(TCHAR *buf, TCHAR *str, int BreakCnt, BOOL BreakFlag)
{
	TCHAR *p, *s;
	int cnt = 0;
	int ret = 0;
	BOOL Flag = FALSE;
	BOOL TopFlag;
	BOOL EndFlag;

	if (BreakCnt <= 0) {
		return lstrlen(buf);
	}

	p = buf;

	if (str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0) {
		Flag = BreakFlag;
	}
	while (*p != TEXT('\0')) {
#ifdef UNICODE
		if (WideCharToMultiByte(CP_ACP, 0, p, 1, NULL, 0, NULL, NULL) != 1) {
#else
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
#endif
			TopFlag = FALSE;
			EndFlag = FALSE;
			if (Flag == FALSE && (cnt + 2) >= BreakCnt) {
				ReturnCheck(p, &TopFlag, &EndFlag);
			}

			if (Flag == FALSE && (((cnt + 2) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 2) == BreakCnt && TopFlag == TRUE))) {
				cnt = 0;
				ret += 2;
			}
			cnt += 2;
			p += CSTEP;
			ret += CSTEP;

		} else if (*p == TEXT('\r')) {
			cnt = 0;
			p += 2;
			ret += 2;
			if (str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0) {
				Flag = BreakFlag;
			} else {
				Flag = FALSE;
			}

		} else if (*p == TEXT('\t')) {
			cnt += (TABSTOPLEN - (cnt % TABSTOPLEN));
			if (Flag == FALSE && cnt > BreakCnt) {
				cnt = (TABSTOPLEN - (cnt % TABSTOPLEN));
				ret += 2;
			}
			p++;
			ret++;

		} else if (IsBreakAlNum(*p)) {
			for (s = p; IsBreakAlNum(*p); p++) {
				cnt++;
			}
			if (*p == TEXT(' ')) {
				cnt++;
				p++;
			}
			if (Flag == FALSE && cnt > BreakCnt) {
				if (cnt != p - s) {
					ret += 2;
				}
				cnt = p - s;
			}
			ret += p - s;

		} else {
			TopFlag = FALSE;
			EndFlag = FALSE;
			if (Flag == FALSE && (cnt + 1) >= BreakCnt) {
				sReturnCheck(p, &TopFlag, &EndFlag);
			}

			if (Flag == FALSE && (((cnt + 1) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 1) == BreakCnt && TopFlag == TRUE))) {
				cnt = 0;
				ret += 2;
			}
			cnt++;
			p++;
			ret++;
		}
	}
	return ret;
}

/*
 * WordBreakString - ��������w��̒����Ő܂�Ԃ�
 */
void WordBreakString(TCHAR *buf, TCHAR *ret, TCHAR *str, int BreakCnt, BOOL BreakFlag)
{
	TCHAR *p, *r, *s;
	int cnt = 0;
	BOOL Flag = FALSE;
	BOOL TopFlag;
	BOOL EndFlag;

	if (BreakCnt <= 0) {
		lstrcpy(ret, buf);
		return;
	}

	p = buf;
	r = ret;

	if (str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0) {
		Flag = BreakFlag;
	}
	while (*p != TEXT('\0')) {
#ifdef UNICODE
		if (WideCharToMultiByte(CP_ACP, 0, p, 1, NULL, 0, NULL, NULL) != 1) {
#else
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
#endif
			// 2�o�C�g�R�[�h
			TopFlag = FALSE;
			EndFlag = FALSE;
			if (Flag == FALSE && (cnt + 2) >= BreakCnt) {
				ReturnCheck(p, &TopFlag, &EndFlag);
			}

			if (Flag == FALSE && (((cnt + 2) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 2) == BreakCnt && TopFlag == TRUE))) {
				cnt = 0;
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
			}
			cnt += 2;
			*(r++) = *(p++);
#ifndef UNICODE
			*(r++) = *(p++);
#endif

		} else if (*p == TEXT('\r')) {
			// ���s
			cnt = 0;
			*(r++) = *(p++);
			*(r++) = *(p++);
			if (str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0) {
				Flag = BreakFlag;
			} else {
				Flag = FALSE;
			}

		} else if (*p == TEXT('\t')) {
			// �^�u
			cnt += (TABSTOPLEN - (cnt % TABSTOPLEN));
			if (Flag == FALSE && cnt > BreakCnt) {
				cnt = (TABSTOPLEN - (cnt % TABSTOPLEN));
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
			}
			*(r++) = *(p++);

		} else if (IsBreakAlNum(*p)) {
			// �p�����͓r�����s���Ȃ�
			for (s = p; IsBreakAlNum(*p); p++) {
				cnt++;
			}
			if (*p == TEXT(' ')) {
				cnt++;
				p++;
			}
			if (Flag == FALSE && cnt > BreakCnt) {
				if (cnt != p - s) {
					*(r++) = TEXT('\r');
					*(r++) = TEXT('\n');
				}
				cnt = p - s;
			}
			for (; s != p; s++) {
				*(r++) = *s;
			}

		} else {
			TopFlag = FALSE;
			EndFlag = FALSE;
			if (Flag == FALSE && (cnt + 1) >= BreakCnt) {
				sReturnCheck(p, &TopFlag, &EndFlag);
			}

			if (Flag == FALSE && (((cnt + 1) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 1) == BreakCnt && TopFlag == TRUE))) {
				cnt = 0;
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
			}
			cnt++;
			*(r++) = *(p++);
		}
	}
	*r = TEXT('\0');
}

/*
 * URLHeadToItem - URL���̃w�b�_���ڂ��A�C�e���ɐݒ�
 */
static BOOL URLHeadToItem(TCHAR *str, TCHAR *head, TCHAR **buf)
{
	TCHAR *p;

	if (TStrCmpNI(str, head, lstrlen(head)) != 0) {
		return FALSE;
	}
	p = str + lstrlen(head);
	for (; *p == TEXT(' '); p++);
	if (*p != TEXT('=')) {
		return FALSE;
	}
	p++;
	for (; *p == TEXT(' '); p++);
	if (*buf != NULL) {
		mem_free(&*buf);
	}
	*buf = AllocURLDecode(p);
	return TRUE;
}

/*
 * URLToMailItem - mailto URL scheme �����[���A�C�e���ɓW�J���� (RFC 2368)
 */
BOOL URLToMailItem(TCHAR *buf, MAILITEM *tpMailItem)
{
	TCHAR *tmp;
	TCHAR *p, *r, *s;

	for (p = buf; *p == TEXT(' '); p++);
	if (TStrCmpNI(p, URL_MAILTO, lstrlen(URL_MAILTO)) != 0) {
		// ���[���A�h���X�̂�
		tpMailItem->To = AllocCopy(p);
		return TRUE;
	}

	// ���[���A�h���X�̎擾
	p += lstrlen(URL_MAILTO);
	for (r = p; *r != TEXT('\0') && *r != TEXT('?'); r++);
	if (*r == TEXT('\0')) {
		tpMailItem->To = AllocURLDecode(p);
		return TRUE;
	}
	if ((r - p) > 0) {
		tmp = mem_alloc(sizeof(TCHAR) * (r - p + 1));
		if (tmp == NULL) {
			return FALSE;
		}
		TStrCpyN(tmp, p, r - p + 1);
		tpMailItem->To = AllocURLDecode(tmp);
		mem_free(&tmp);
	}

	tmp = mem_alloc(sizeof(TCHAR) * (lstrlen(r) + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	while (*r != TEXT('\0')) {
		// �w�b�_���ڂ̎擾
		for (r++; *r == TEXT(' '); r++);
		for (s = tmp; *r != TEXT('\0') && *r != TEXT('&'); r++, s++) {
			*s = *r;
		}
		*s = TEXT('\0');

		URLHeadToItem(tmp, TEXT("to"), &tpMailItem->To);
		URLHeadToItem(tmp, TEXT("cc"), &tpMailItem->Cc);
		URLHeadToItem(tmp, TEXT("bcc"), &tpMailItem->Bcc);
		URLHeadToItem(tmp, TEXT("replyto"), &tpMailItem->ReplyTo);
		URLHeadToItem(tmp, TEXT("subject"), &tpMailItem->Subject);
		URLHeadToItem(tmp, TEXT("body"), &tpMailItem->Body);
		URLHeadToItem(tmp, TEXT("mailbox"), &tpMailItem->MailBox);
		URLHeadToItem(tmp, TEXT("attach"), &tpMailItem->Attach);
	}
	mem_free(&tmp);
	return TRUE;
}

/*
 * GetNextQuote - ��������̎��̋�؂�ʒu�܂Ń|�C���^���ړ�����
 */
static TCHAR *GetNextQuote(TCHAR *buf, TCHAR qStr)
{
	TCHAR *p;

	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
			continue;
		}
#endif
		if (*p == TEXT('\\')) {
			p++;
			continue;
		}
		if (*p == qStr) {
			break;
		}
		if (*p == TEXT('\"')) {
			p = GetNextQuote(p + 1, TEXT('\"'));
			if (*p == TEXT('\0')) {
				break;
			}
		}
	}
	return p;
}

/*
 * GetMailAddress - �����񂩂烁�[���A�h���X�̒��o (RFC 822, RFC 2822 - addr-spec)
 *		MailAddress
 *		Comment <MailAddress>
 *		MailAddress (Comment)
 */

TCHAR *GetMailAddress(TCHAR *buf, TCHAR *ret, BOOL quote)
{
	BOOL kFlag = FALSE;
	BOOL qFlag = FALSE;
	TCHAR *p, *r, *t;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch (*p) {
		case TEXT(' '):
		case TEXT('\t'):
		case TEXT('\r'):
		case TEXT('\n'):
			break;

		case TEXT('\"'):
			if (kFlag == TRUE || quote == TRUE) {
				t = GetNextQuote(p + 1, TEXT('\"'));
				for (; p < t; p++) {
					*(r++) = *p;
				}
				if (*p != TEXT('\0')) {
					*(r++) = *p;
				}
				break;
			}
			p = GetNextQuote(p + 1, TEXT('\"'));
			qFlag = TRUE;
			break;

		case TEXT('('):
			p = GetNextQuote(p + 1, TEXT(')'));
			break;

		case TEXT('<'):
			r = ret;
			kFlag = TRUE;
			break;

		case TEXT('>'):
			if (kFlag == FALSE) {
				*(r++) = *p;
				break;
			}
			*r = TEXT('\0');
			return GetNextQuote(p + 1, TEXT(','));

		case TEXT(','):
			*r = TEXT('\0');

			if (quote == FALSE && kFlag == FALSE && qFlag == TRUE) {
				return GetMailAddress(buf, ret, TRUE);
			}
			return p;

		default:
			*(r++) = *p;
			break;
		}
		if (*p == TEXT('\0')) {
			break;
		}
	}
	*r = TEXT('\0');

	if (quote == FALSE && kFlag == FALSE && qFlag == TRUE) {
		return GetMailAddress(buf, ret, TRUE);
	}
	return p;
}

/*
 * GetMailString - �����񂩂烁�[���A�h���X(�R�����g��)�̒��o
 */
TCHAR *GetMailString(TCHAR *buf, TCHAR *ret)
{
	BOOL kFlag = FALSE;
	BOOL BreakFlag = FALSE;
	TCHAR *p, *r;

	for (p = buf; *p == TEXT(' '); p++);
	for (r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch (*p) {
		case TEXT('\t'):
		case TEXT('\r'):
		case TEXT('\n'):
			break;

		case TEXT('\\'):
			*(r++) = *(p++);
			if (*p == TEXT('\0')) {
				BreakFlag = TRUE;
				break;
			}
			*(r++) = *p;
			break;

		case TEXT('\"'):
			kFlag = !kFlag;
			*(r++) = *p;
			break;

		case TEXT(','):
			if (kFlag == TRUE) {
				*(r++) = *p;
				break;
			}
			BreakFlag = TRUE;
			break;

		default:
			*(r++) = *p;
			break;
		}
		if (BreakFlag == TRUE) {
			break;
		}
	}
	*r = TEXT('\0');
	for (; r != ret && *r == TEXT(' '); r--);
	*r = TEXT('\0');
	return p;
}

/*
 * SetUserName - From: �̃R�����g�������̕��������������
 */
void SetUserName(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch (*p) {
		case TEXT(','):
		case TEXT('<'):
		case TEXT('>'):
			*(r++) = TEXT('_');
			break;

		default:
			*(r++) = *p;
			break;
		}
	}
	*r = TEXT('\0');
}

/*
 * SetCcAddressSize - Cc, Bcc �̃��X�g�̃T�C�Y
 */
int SetCcAddressSize(TCHAR *To)
{
	TCHAR *ToMailAddress;
	int cnt = 0;

	if (To == NULL) {
		return 0;
	}
	ToMailAddress = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(To) + 1));
	if (ToMailAddress != NULL) {
		cnt += 5;
		while (*To != TEXT('\0')) {
			To = GetMailAddress(To, ToMailAddress, FALSE);
			cnt += lstrlen(ToMailAddress) + 2;
			To = (*To != TEXT('\0')) ? To + 1 : To;
		}
		cnt += 1;
		mem_free(&ToMailAddress);
	}
	return cnt;
}

/*
 * SetCcAddress - Cc, Bcc �̃��X�g�̍쐬
 */
TCHAR *SetCcAddress(TCHAR *Type, TCHAR *To, TCHAR *r)
{
	TCHAR *ToMailAddress;
	TCHAR *sep = TEXT(": ");

	if (To == NULL) {
		return r;
	}
	ToMailAddress = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(To) + 1));
	if (ToMailAddress != NULL) {
		r = TStrJoin(r, TEXT(" ("), Type, (TCHAR *)-1);
		while (*To != TEXT('\0')) {
			To = GetMailAddress(To, ToMailAddress, FALSE);
			r = TStrJoin(r, sep, ToMailAddress, (TCHAR *)-1);
			To = (*To != TEXT('\0')) ? To + 1 : To;
			sep = TEXT(", ");
		}
		r = TStrCpy(r, TEXT(")"));
		mem_free(&ToMailAddress);
	}
	return r;
}

/*
 * GetFileNameString - �t�@�C���p�X����t�@�C�����擾����
 */
TCHAR *GetFileNameString(TCHAR *p)
{
	TCHAR *fname;

	for (fname = p; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
			continue;
		}
#endif
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			fname = p + 1;
		}
	}
	return fname;
}

/*
 * SetAttachListSize - �Y�t�t�@�C���̃��X�g�̃T�C�Y
 */
int SetAttachListSize(TCHAR *buf)
{
	TCHAR *fname, *fpath;
	TCHAR *f;
	int len = 0;

	if (buf == NULL || *buf == TEXT('\0')) {
		return 0;
	}
	f = buf;
	fpath = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(f) + 1));
	if (fpath == NULL) {
		return 0;
	}
	len += 2;
	while (*f != TEXT('\0')) {
		if (f != buf) {
			len += 2;
		}
		f = TStrCpyF(fpath, f, ATTACH_SEP);
		fname = GetFileNameString(fpath);
		len += lstrlen(fname);
	}
	mem_free(&fpath);
	len++;
	return len;
}

/*
 * SetAttachList - �Y�t�t�@�C���̃��X�g�̍쐬
 */
TCHAR *SetAttachList(TCHAR *buf, TCHAR *ret)
{
	TCHAR *fname, *fpath;
	TCHAR *p, *f;

	if (buf == NULL || *buf == TEXT('\0')) {
		return ret;
	}
	f = buf;
	p = ret;
	fpath = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(f) + 1));
	if (fpath == NULL) {
		return ret;
	}
	p = TStrCpy(p, TEXT(" ("));
	while (*f != TEXT('\0')) {
		if (f != buf) {
			p = TStrCpy(p, TEXT(", "));
		}
		f = TStrCpyF(fpath, f, ATTACH_SEP);
		fname = GetFileNameString(fpath);
		p = TStrCpy(p, fname);
	}
	mem_free(&fpath);
	p = TStrCpy(p, TEXT(")"));
	return p;
}

/*
 * GetMIME2Extension - MIME�ƃt�@�C���^�C�v�̕ϊ� (RFC 2046)
 */
TCHAR *GetMIME2Extension(TCHAR *MIMEStr, TCHAR *Filename)
{
	const TCHAR *MIME_list[] = {
		TEXT("application/mac-binhex40"), TEXT(".hqx"),
		TEXT("application/postscript"), TEXT(".eps"),
		TEXT("application/postscript"), TEXT(".ps"),
		TEXT("application/postscript"), TEXT(".ai"),
		TEXT("application/rtf"), TEXT(".rtf"),
		TEXT("application/x-stuffit"), TEXT(".sit"),
		TEXT("application/x-uuencode"), TEXT(".uue"),
		TEXT("application/x-uuencode"), TEXT(".uu"),
		TEXT("application/x-zip-compressed"), TEXT(".zip"),
		TEXT("audio/basic"), TEXT(".au"),
		TEXT("audio/basic"), TEXT(".snd"),
		TEXT("audio/x-aiff"), TEXT(".aiff"),
		TEXT("audio/x-aiff"), TEXT(".aif"),
		TEXT("audio/x-pn-realaudio"), TEXT(".ra"),
		TEXT("audio/x-pn-realaudio"), TEXT(".ram"),
		TEXT("audio/x-wav"), TEXT(".wav"),
		TEXT("image/gif"), TEXT(".gif"),
		TEXT("image/jpeg"), TEXT(".jpg"),
		TEXT("image/jpeg"), TEXT(".jpeg"),
		TEXT("image/png"), TEXT(".png"),
		TEXT("image/tiff"), TEXT(".tiff"),
		TEXT("image/tiff"), TEXT(".tif"),
		TEXT("message/rfc822"), TEXT(".eml"),
		TEXT("text/html"), TEXT(".html"),
		TEXT("text/html"), TEXT(".htm"),
		TEXT("text/plain"), TEXT(".txt"),
		TEXT("text/x-vcard"), TEXT(".vcf"),
		TEXT("video/mpeg"), TEXT(".mpg"),
		TEXT("video/mpeg"), TEXT(".mpeg"),
		TEXT("video/quicktime"), TEXT(".qt"),
		TEXT("video/quicktime"), TEXT(".mov"),
		TEXT("video/x-msvideo"), TEXT(".avi"),
	};

	TCHAR *ret, *p, *r;
	int i;

	if (MIMEStr != NULL) {
		// Content type ����t�@�C���^�C�v���擾
		for (i = 0; i < (sizeof(MIME_list) / sizeof(TCHAR *)); i += 2) {
			if (TStrCmpI(MIMEStr, MIME_list[i]) == 0) {
				ret = AllocCopy(MIME_list[i + 1]);
				for (p = ret + 1; *p != TEXT('\0') && *p != TEXT('.'); p++);
				*p = TEXT('\0');
				return ret;
			}
		}
	} else if (Filename != NULL) {
		// �t�@�C�������� Content type ���擾
		for (r = p = Filename; *p != TEXT('\0'); p++) {
#ifndef UNICODE
			if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
				p++;
				continue;
			}
#endif
			if (*p == TEXT('.')) {
				r = p;
			}
		}
		if (TStrCmpI(r, TEXT(".txt")) != 0) {
			for (i = 1; i < (sizeof(MIME_list) / sizeof(TCHAR *)); i += 2) {
				if (TStrCmpI(r, MIME_list[i]) == 0) {
					return AllocCopy(MIME_list[i - 1]);
				}
			}
		}
		return AllocCopy(TEXT("application/octet-stream"));
	}
	return NULL;
}

/*
 * GetCommandLineSize - �R�}���h���C�����쐬�����Ƃ��̃T�C�Y
 */
static int GetCommandLineSize(TCHAR *buf, TCHAR *filename)
{
	TCHAR *p;
	int ret = 0;

	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			p++;
			ret += 2;
			continue;
		}
#endif
		if (*p != TEXT('%')) {
			ret++;
			continue;
		}
		p++;
		switch (*p) {
		case TEXT('1'):
			ret += NULLCHECK_STRLEN(filename);
			break;

		case TEXT('%'):
			ret++;
			break;

		default:
			break;
		}
	}
	return ret;
}

/*
 * CreateCommandLine - �R�}���h���C���̍쐬
 */
TCHAR *CreateCommandLine(TCHAR *buf, TCHAR *filename, BOOL spFlag)
{
	TCHAR *ret;
	TCHAR *p, *r, *t;
	int len;

	if (buf == NULL || *buf == TEXT('\0')) {
		return NULL;
	}

	len = GetCommandLineSize(buf, filename);
	len += ((spFlag == TRUE) ? 1 : 0);
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (ret == NULL) {
		return NULL;
	}

	if (spFlag == TRUE) {
		// �擪�ɋ󔒂�ǉ�
		*ret = TEXT(' ');
		r = ret + 1;
	} else {
		r = ret;
	}

	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		if (*p != TEXT('%')) {
			*(r++) = *p;
			continue;
		}
		p++;
		t = NULL;
		switch (*p) {
		// File
		case TEXT('1'):
			t = filename;
			break;

		// %
		case TEXT('%'):
			t = TEXT("%");
			break;
		}
		if (t != NULL) {
			while (*(r++) = *(t++));
			r--;
		}
	}
	*r = TEXT('\0');
	return ret;
}
/* End of source */
