/*
 * nPOP
 *
 * util.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2013 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#include "code.h"

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

#define NULLCHECK_STRLEN(m)	((m != NULL) ? lstrlen(m) : 0)
#define is_white(c)	(c == TEXT(' ') || c == TEXT('\r') || c == TEXT('\t') || c == TEXT('\0'))

/* Global Variables */
extern OPTION op;
extern TCHAR *AppDir;

/* Local Function Prototypes */
static TCHAR *AllocURLDecode(TCHAR *buf);
static void ReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag);
static void sReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag);
static BOOL URLHeadToItem(TCHAR *str, TCHAR *head, TCHAR **buf, TCHAR sep);
static TCHAR *GetNextQuote(TCHAR *buf, TCHAR qStr);

/*
 * AllocURLDecode - メモリを確保してURL encodingをデコード
 */
static TCHAR *AllocURLDecode(TCHAR *buf)
{
	TCHAR *ret;

	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(buf) + 1));
	if (ret == NULL) {
		return NULL;
	}
	URL_decode_t(buf, ret);
	return ret;
}

/*
 * StrNextContentT - ヘッダ内の次のコンテンツの先頭に移動する (TCHAR)
 */
TCHAR *StrNextContentT(TCHAR *p)
{
	while (1) {
		for (; *p != TEXT('\r') && *p != TEXT('\n') && *p != TEXT('\0'); p++);
		if (*p == TEXT('\0')) {
			break;
		}
		p++;
		if (*p == TEXT('\n') && *(p - 1) == TEXT('\r')) {
			p++;
		}
		if (*p == TEXT(' ') || *p == TEXT('\t')) {
			continue;
		}
		break;
	}
	return p;
}

/*
 * StrNextContent - ヘッダ内の次のコンテンツの先頭に移動する (TCHAR)
 */
#ifdef UNICODE
char *StrNextContent(char *p)
{
	while (1) {
		for (; *p != '\r' && *p != '\n' && *p != '\0'; p++);
		if (*p == '\0') {
			break;
		}
		p++;
		if (*p == '\n' && *(p - 1) == '\r') {
			p++;
		}
		if (*p == ' ' || *p == '\t') {
			continue;
		}
		break;
	}
	return p;
}
#endif

/*
 * GetHeaderStringPointT - ヘッダから指定の項目のコンテンツの位置を取得 (TCHAR)
 */
TCHAR *GetHeaderStringPointT(TCHAR *buf, TCHAR *str)
{
	TCHAR *p;
	int len;

	len = lstrlen(str);
	p = buf;
	while (1) {
		if (str_cmp_ni_t(p, str, len) != 0) {
			// 次のコンテンツに移動する
			p = StrNextContentT(p);
			if (*p == TEXT('\0') || *p == TEXT('\r') || *p == TEXT('\n')) {
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
 * GetHeaderStringPoint - ヘッダから指定の項目のコンテンツの位置を取得
 */
#ifdef UNICODE
char *GetHeaderStringPoint(char *buf, char *str)
{
	char *p;
	int len;

	len = tstrlen(str);
	p = buf;
	while (1) {
		if (str_cmp_ni(p, str, len) != 0) {
			// 次のコンテンツに移動する
			p = StrNextContent(p);
			if (*p == '\0' || *p == '\r' || *p == '\n') {
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
 * GetHeaderStringSizeT - ヘッダから指定の項目のコンテンツのサイズを取得 (TCHAR)
 */
int GetHeaderStringSizeT(TCHAR *buf, BOOL CrLfFlag)
{
	TCHAR *p = buf;
	int i = 0;

	while (*p != TEXT('\0')) {
		if (*p == TEXT('\r') || *p == TEXT('\n')) {
			p++;
			if (*p == TEXT('\n') && *(p - 1) == TEXT('\r')) {
				p++;
			}
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
 * GetHeaderStringSize - ヘッダから指定の項目のコンテンツのサイズを取得
 */
#ifdef UNICODE
int GetHeaderStringSize(char *buf, BOOL CrLfFlag)
{
	char *p = buf;
	int i = 0;

	while (*p != '\0') {
		if (*p == '\r' || *p == '\n') {
			p++;
			if (*p == '\n' && *(p - 1) == '\r') {
				p++;
			}
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
 * GetHeaderStringT - ヘッダから指定の項目のコンテンツを取得 (TCHAR)
 */
BOOL GetHeaderStringT(TCHAR *buf, TCHAR *ret, BOOL CrLfFlag)
{
	TCHAR *p, *r;

	p = buf;
	r = ret;
	while (*p != TEXT('\0')) {
		if (*p == TEXT('\r') || *p == TEXT('\n')) {
			p++;
			if (*p == TEXT('\n') && *(p - 1) == TEXT('\r')) {
				p++;
			}
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
 * GetHeaderString - ヘッダから指定の項目のコンテンツを取得
 */
#ifdef UNICODE
BOOL GetHeaderString(char *buf, char *ret, BOOL CrLfFlag)
{
	char *p, *r;

	p = buf;
	r = ret;
	while (*p != '\0') {
		if (*p == '\r' || *p == '\n') {
			p++;
			if (*p == '\n' && *(p - 1) == '\r') {
				p++;
			}
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
 * GetBodyPointaT - Body位置のを取得 (TCHAR)
 * \r\n\r\n -- or (from unix) \n\n or (from mac) \r\r
 */
TCHAR *GetBodyPointaT(TCHAR *buf)
{
	TCHAR *p = buf;

	while (1) {
		for (; *p != TEXT('\0') && *p != TEXT('\r') && *p != TEXT('\n'); p++);
		if (*p == TEXT('\0')) {
			break;
		} else if (*p == TEXT('\r')) {
			p++;
			if (*p == TEXT('\n')) {
				p++;
				if (*p == TEXT('\r') && *(p + 1) == TEXT('\n')) {
					return (p + 2);
				}
			} else if (*p == TEXT('\r')) {
				return (p + 1);
			}
		} else { // *p == TEXT('\n')
			p++;
			if (*p == TEXT('\n')) {
				return (p + 1);
			}
		}
	}
	return NULL;
}

/*
 * GetBodyPointa - Body位置のを取得
 */
#ifdef UNICODE
char *GetBodyPointa(char *buf)
{
	char *p = buf;

	while (1) {
		for (; *p != '\0' && *p != '\r' && *p != '\n'; p++);
		if (*p == '\0') {
			break;
		} else if (*p == '\r') {
			p++;
			if (*p == '\n') {
				p++;
				if (*p == '\r' && *(p + 1) == '\n') {
					return (p + 2);
				}
			} else if (*p == '\r') {
				return (p + 1);
			}
		} else { // *p == '\n'
			p++;
			if (*p == '\n') {
				return (p + 1);
			}
		}
	}
	return NULL;
}
#endif

/*
 * TrimMessageId - 文字列からMessage-IDのみを抽出する
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
 * GetReferencesSize - Referencesを保存用に変換したときのサイズの取得
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
 * ConvReferences - Referencesを保存用に変換する
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
 * DateAdd - 日付の計算を行う
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
		if (op.TimeZone != NULL && *op.TimeZone != TEXT('\0')) {
			tmz = _ttoi(op.TimeZone);
			tmz = (tmz / 100) * 60 + tmz - (tmz / 100) * 100;
		} else {
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
		// タイムゾーンの設定のみ
		if (sTime == NULL) return;
	}

	// 変換元のタイムゾーンを'分'に変換
	if (*tz != '+' && *tz != '-') {
		if (str_cmp_i(tz, "GMT") == 0 || str_cmp_i(tz, "UT") == 0) {
			// 標準時間
			itz = 0;
		} else if (str_cmp_i(tz, "EDT") == 0) {
			itz = -4;
		} else if (str_cmp_i(tz, "EST") == 0 || str_cmp_i(tz, "CDT") == 0) {
			itz = -5;
		} else if (str_cmp_i(tz, "CST") == 0 || str_cmp_i(tz, "MDT") == 0) {
			itz = -6;
		} else if (str_cmp_i(tz, "MST") == 0 || str_cmp_i(tz, "PDT") == 0) {
			itz = -7;
		} else if (str_cmp_i(tz, "PST") == 0) {
			itz = -8;
		} else {
			return;
		}
	} else {
		itz = a2i(tz + 1);
		if (*tz == '-') {
			itz *= -1;
		}
		itz = (itz / 100) * 60 + itz - (itz / 100) * 100;
	}

	// 差の計算
	diff = tmz - itz;
	if (diff == 0) {
		return;
	}
	if (diff < 0) {
		diff *= -1;
		f = -1;
	}

	// 計算する値をFileTimeに変換
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

	// FileTimeに変換して日付の計算を行う
	if (SystemTimeToFileTime(sTime, &fTime) == FALSE) {
		return;
	}
	CopyMemory(&LInt, &fTime, sizeof(FILETIME));
	LInt += DiffLInt * f;
	CopyMemory(&fTime, &LInt, sizeof(FILETIME));
	FileTimeToSystemTime(&fTime, sTime);
}

/*
 * FormatDateConv - フォーマットにしたがって日付の展開を行う
 */
static int FormatDateConv(char *format, char *buf, SYSTEMTIME *gTime)
{
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
		case 'w':	// 曜日
			for (; IS_ALPHA(*r); r++);
			break;

		case 'd':	// 日
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wDay = a2i(tmp);
			if (gTime->wDay == 0) {
				return -1;
			}
			break;

		case 'M':	// 月(数値)
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wMonth = a2i(tmp);
			if (gTime->wMonth == 0) {
				return -1;
			}
			break;

		case 'm':	// 月
			for (t = tmp; IS_ALPHA(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			for (i = 0; i < 12; i++) {
				if (str_cmp_ni(*(month + i), tmp, tstrlen(tmp) + 1) == 0) {
					break;
				}
			}
			if (i >= 12) {
				return -1;
			}
			gTime->wMonth = i + 1;
			break;

		case 'y':	// 年
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

		case 'h':	// 時
		case 'H':	// 時
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wHour = a2i(tmp);
			break;

		case 'n':	// 分
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wMinute = a2i(tmp);
			break;

		case 's':	// 秒
			for (t = tmp; IS_NUM(*r); r++) {
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wSecond = a2i(tmp);
			break;

		case 't':	// TZ
			for (t = tz; IS_ALNUM_PM(*r); r++) {
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
 * DateConv - 日付形式の変換を行う (RFC 822, RFC 2822)
 */
int DateConv(char *buf, char *ret, BOOL for_sort)
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
		str_cpy_n(ret, buf, BUF_SIZE - 1);
		return -1;
	}

	// 日時文字列を SYSTEMTIME に変換
	i = FormatDateConv("w, d m y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w, d-m-y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w, d m y h:n t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w d m y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w m d h:n:s y t", buf, &gTime);
	if (i == -1) i = FormatDateConv("d m y h:n t", buf, &gTime);
	if (i == -1) {
		tstrcpy(ret, buf);
		return -1;
	}

	if (for_sort) {
		if (GetDateFormat(0, 0, &gTime, TEXT("yyyyMMdd"), fDay, BUF_SIZE - 1) == 0) {
			tstrcpy(ret, buf);
			return -1;
		}
		if (GetTimeFormat(0, 0, &gTime, TEXT("HHmm"), fTime, BUF_SIZE - 1) == 0) {
			tstrcpy(ret, buf);
			return -1;
		}
	} else {
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
	}

#ifdef UNICODE
	str_join_t(wret, fDay, TEXT(" "), fTime, (TCHAR *)-1);
	tchar_to_char(wret, ret, BUF_SIZE);
#else
	str_join_t(ret, fDay, TEXT(" "), fTime, (TCHAR *)-1);
#endif
	return 0;
}

/*
 * DateUnConv - ソート用の日付形式変換を行う
 */
int DateUnConv(char *buf, char *ret)
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
		str_cpy_n(ret, buf, BUF_SIZE - 1);
		return -1;
	}

	// 形式の生成
	r = tmp;
#ifdef UNICODE
	tchar_to_char(op.DateFormat, fmt, BUF_SIZE);
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
			if (str_cmp_ni(p, "MMM", 3) == 0) {
				*(r++) = 'm';
			} else {
				*(r++) = 'M';
			}
			for (; *p == c; p++);
			break;

		case 'd':
			if (str_cmp_ni(p, "dd", 2) == 0 || str_cmp_ni(p, "ddd", 3) == 0) {
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
	tchar_to_char(op.TimeFormat, fmt, BUF_SIZE);
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

	// 日時文字列を SYSTEMTIME に変換
	i = FormatDateConv(tmp, buf, &gTime);
	if (i == -1) {
		tstrcpy(ret, buf);
		return -1;
	}
	if (gTime.wYear < 70) {
		gTime.wYear += 2000;
	}

	if (GetDateFormat(0, 0, &gTime, TEXT("ddd, dd MMM yyyy "), fDay, BUF_SIZE - 1) == 0) {
		tstrcpy(ret, buf);
		return -1;
	}
	if (GetTimeFormat(0, 0, &gTime, TEXT("HH:mm:ss"), fTime, BUF_SIZE - 1) == 0) {
		tstrcpy(ret, buf);
		return -1;
	}

#ifdef UNICODE
	str_join_t(wret, fDay, fTime, (TCHAR *)-1);
	tchar_to_char(wret, ret, BUF_SIZE);
#else
	str_join_t(ret, fDay, fTime, (TCHAR *)-1);
#endif
	return 0;
}

static int DayOfYear(SYSTEMTIME mytime)
{
	int retval = mytime.wDay;
	if (mytime.wMonth >  1) retval += 31;
	if (mytime.wMonth >  2) {
		retval += 28;
		if (mytime.wYear % 4 == 0 && (mytime.wYear % 100 != 0 || mytime.wYear % 400 == 0)) {
			// leap year: all years evenly divisible by four, except for centennial years
			// (those ending in -00), which receive the extra day only if they are evenly
			// divisible by 400
			retval++;
		}
	}
	if (mytime.wMonth >  3) retval += 31;
	if (mytime.wMonth >  4) retval += 30;
	if (mytime.wMonth >  5) retval += 31;
	if (mytime.wMonth >  6) retval += 30;
	if (mytime.wMonth >  7) retval += 31;
	if (mytime.wMonth >  8) retval += 31;
	if (mytime.wMonth >  9) retval += 30;
	if (mytime.wMonth > 10) retval += 31;
	if (mytime.wMonth > 11) retval += 30;
	return retval;
}

/*
 * DateCompare - compare msg_date to current date
 */
BOOL DateCompare(TCHAR *msg_date, int days, BOOL newer)
{
	SYSTEMTIME gTime, nTime;
	char *buf;
	int i;

#ifdef UNICODE
	buf = alloc_tchar_to_char(msg_date);
#else
	buf = msg_date;
#endif

	// 日時文字列を SYSTEMTIME に変換
	i = FormatDateConv("w, d m y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w, d-m-y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w, d m y h:n t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w d m y h:n:s t", buf, &gTime);
	if (i == -1) i = FormatDateConv("w m d h:n:s y t", buf, &gTime);
#ifdef UNICODE
		mem_free(&buf);
#endif
	if (i == -1) {
		return FALSE;
	}
	GetLocalTime(&nTime);
	i = 365*(nTime.wYear - gTime.wYear) + DayOfYear(nTime) - DayOfYear(gTime);
	if (newer == TRUE && i < days) {
		return TRUE;
	} else if (newer == FALSE && i > days) {
		return TRUE;
	}
	return FALSE;
}

/*
 * FormatNumberString - for size (KB,MB,GB)
 */
void FormatNumberString(long num, TCHAR *fmtstring, TCHAR *decpt, TCHAR *ret)
{
	TCHAR tmp[20];
	long div;

	div = num/1024;
	if (num < 10240) { // 1.23 MB
		wsprintf(tmp, TEXT("%d%s%2.2d"), div, decpt, (100*(num-div*1024))/1024);
		wsprintf(ret, fmtstring, tmp);
	} else if (num < 102400) { // 12.3 MB
		wsprintf(tmp, TEXT("%d%s%1.1d"), div, decpt, (10*(num-div*1024))/1024);
		wsprintf(ret, fmtstring, tmp);
	} else { // 123 MB
		wsprintf(tmp, TEXT("%d"), div);
		wsprintf(ret, fmtstring, tmp);
	}
}

/*
 * GetTimeString - 時間文字列の取得 (RFC 822, RFC 2822)
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

	if (op.TimeZone != NULL && *op.TimeZone != TEXT('\0')) {
		tmz = _ttoi(op.TimeZone);
		if (tmz < 0) {
			tmz *= -1;
			c = TEXT('-');
		} else {
			c = TEXT('+');
		}
	} else {
		ret = GetTimeZoneInformation(&tmzi);
		if (ret != 0xFFFFFFFF) {
			switch (ret) {
			case TIME_ZONE_ID_STANDARD:
				tmzi.Bias += tmzi.StandardBias;
				break;

			case TIME_ZONE_ID_DAYLIGHT:
				// 夏時間の計算
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
 * EncodePassword_old - 文字列を暗号化 (旧)
 */
static void EncodePassword_old(TCHAR *buf, TCHAR *ret, int retsize)
{
	char *p, *r;
	int len;
	int i;

#ifdef UNICODE
	p = alloc_tchar_to_char(buf);
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

	base64_decode(p, r, FALSE);

	len = tstrlen(r);
	for (i = 0; i < len; i++) {
		*(r + i) ^= 0xFF;
	}
	*(r + i) = '\0';

#ifdef UNICODE
	char_to_tchar(r, ret, retsize);
	mem_free(&p);
	mem_free(&r);
#endif
}

/*
 * EncodePassword - 文字列を暗号化
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

	// digest値を取得
#ifdef UNICODE
	p = alloc_tchar_to_char((Key == NULL) ? TEXT('\0') : Key);
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

	p = alloc_tchar_to_char(Word);
	if (p == NULL) {
		*ret = TEXT('\0');
		return;
	}
	if (decode == TRUE) {
		// デコード
		r = (char *)mem_alloc(tstrlen(p) + 1);
		if (r == NULL) {
			*ret = TEXT('\0');
			return;
		}
		t = base64_decode(p + 1, r, FALSE);
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
		char_to_tchar(p, ret, retsize);
#else
		lstrcpy(ret, p);
#endif
	} else {
		// エンコード
		r = (char *)mem_alloc(len * 2 + 4 + 1);
		if (r == NULL) {
			*ret = TEXT('\0');
			return;
		}
		base64_encode(p, r, len, 0);
		mem_free(&p);
		p = r;

		*ret = TEXT('=');
#ifdef UNICODE
		char_to_tchar(p, ret + 1, retsize - 1);
#else
		lstrcpy(ret + 1, p);
#endif
	}
	mem_free(&p);
}

/*
 * EncodeCtrlChar - 制御文字を \〜 の形式に変換する
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
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
 * DecodeCtrlChar - \〜 の形式の文字列を制御文字に変換する
 */
void DecodeCtrlChar(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
		if (p == TEXT('\0')) {
			break;
		}
		switch (*p) {
		case TEXT('t'):
			*(r++) = TEXT('\t');
			break;

		case TEXT('n'):
		case TEXT('r'):
			*(r++) = TEXT('\r');
			*(r++) = TEXT('\n');
			if (*p == TEXT('r') && *(p+1) == TEXT('\\') && *(p+2) == TEXT('n')) {
				p+=2;
			}
			break;

		default: // including '\\'
			*(r++) = *p;
			break;
		}
	}
	*r = TEXT('\0');
}

/*
 * CreateMessageId - Message-Id を生成する (RFC 822, RFC 2822)
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
	str_join_t(ret + len, TEXT("."), MailAddress, TEXT(">"), (TCHAR *)-1);
	return ret;
}

/*
 * CreateHeaderStringSize - This calculates an approximate header string size.
 * It doesn't compensate for {} causing some plain text removal.
 */
int CreateHeaderStringSize(TCHAR *buf, MAILITEM *tpMailItem, TCHAR *quotstr)
{
	TCHAR *p;
	int ret = 0;
	int quotlen = (quotstr == NULL) ? 0 : lstrlen(quotstr);

	if (buf == NULL) {
		return 0;
	}
	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
			ret += 2;
			continue;
		}
#endif
		if (quotlen > 0 && *p == TEXT('\r') && *(p+1) == TEXT('\n')) {
			p++;
			ret += 2 + quotlen;
		}
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
			if (tpMailItem->MessageID != NULL && *tpMailItem->MessageID == TEXT('<')) {
				ret += lstrlen(tpMailItem->MessageID);
			}
			break;

		case TEXT('D'):
			ret += NULLCHECK_STRLEN(tpMailItem->Date);
			break;
		
		case TEXT('d'):
			ret += NULLCHECK_STRLEN(tpMailItem->FmtDate);
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

		case TEXT('A'): case TEXT('a'):
			ret += NULLCHECK_STRLEN(tpMailItem->Attach) + NULLCHECK_STRLEN(tpMailItem->FwdAttach) + 1;
			break;

		case TEXT('%'): case TEXT('{'): case TEXT('}'):
			ret++;
			break;

		default:
			break;
		}
	}
	return ret;
}

/*
 * CreateHeaderString - substitute header values for placeholders
 */
TCHAR *CreateHeaderString(TCHAR *buf, TCHAR *ret, MAILITEM *tpMailItem, TCHAR *quotstr)
{
	TCHAR *p, *r, *s, *t, *t2 = NULL;
	int i, Optional = 0;
	int quotlen = (quotstr == NULL) ? 0 : lstrlen(quotstr);
	BOOL Found = FALSE;

	if (buf == NULL) {
		return ret;
	}
	s = ret;
	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		if (quotlen > 0 && *p == TEXT('\r') && *(p+1) == TEXT('\n')) {
			*(r++) = *(p++); 
			*(r++) = *p;
			for (i=0; i<quotlen; i++) {
				*(r++) = quotstr[i];
			}
			continue;
		}
		// Optional text: include only if header is non-null (GJC)
		// {CC: %C\n} will be included only if CC is non-null
		if (*p == TEXT('{')) {
			// {Optional text}
			if (Optional == 0) {
				// presently don't handle nested {}
				Found = FALSE;
				s = r;
			}
			Optional++;
			continue;
		} else if (*p == TEXT('}')) {
			// {%C}
			if (Optional > 0) {
				Optional--;
				if (Optional == 0 && Found == FALSE) {
					r = s; // reset to before {
				}
			} else {
				Optional = 0;
			}
			continue;
		}
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
		case TEXT('D'):
			t = tpMailItem->Date;
			break;
		case TEXT('d'):
			t = tpMailItem->FmtDate;
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

		// Attach:
		case TEXT('A'): case TEXT('a'):
			t = tpMailItem->Attach;
			t2 = tpMailItem->FwdAttach;
			if (t2 != NULL && (t == NULL || *t == TEXT('\0'))) {
				t = t2;
				t2 = NULL;
			}
			break;

		// %% becomes %, %{ becomes {, %{ becomes {
		case TEXT('%'):
		case TEXT('{'):
		case TEXT('}'):
			*(r++) = *p;
			continue;
		}
		if (t != NULL  &&  *t != TEXT('\0')) {
			if (t2 != NULL && *t2 != TEXT('\0')) {
				// "|" should be ATTACH_SEP, but str_join_t needs a string, not a single char
				r = str_join_t(r, t, TEXT("|"), t2, (TCHAR*)-1);
			} else {
				r = str_cpy_t(r, t);
			}
			Found = TRUE;
		}
	}
	*r = TEXT('\0');
	return r;
}

/*
 * GetReplyBodySize - 引用文字列を付加したときの文字列のサイズを取得
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
 * SetReplyBody - 文字列に引用文字列を追加する
 */
TCHAR *SetReplyBody(TCHAR *body, TCHAR *ret, TCHAR *ReStr)
{
	TCHAR *p, *r, *s;

	if (ReStr == NULL || *ReStr == TEXT('\0')) {
		// 引用文字が無い場合
		return str_cpy_t(ret, body);
	}

	// 最後尾の空行は引用しない
	for (s = body + lstrlen(body) - 1; *s == '\r' || *s == '\n'; s--);

	// 先頭行の引用
	r = str_cpy_t(ret, ReStr);
	for (p = body; p <= s && *p != TEXT('\0'); p++) {
		if (*p == TEXT('\n')) {
			// 引用
			*(r++) = *p;
			if (*(p + 1) == TEXT('\0')) {
				break;
			}
			r = str_cpy_t(r, ReStr);
		} else {
			*(r++) = *p;
		}
	}
	return str_cpy_t(r, TEXT("\r\n"));
}

/*
 * SetDotSize - ピリオドから始まる行の先頭にピリオドを付加するためのサイズの取得
 */

int SetDotSize(TCHAR *buf)
{
	TCHAR *p;
	int len = 0;

	p = buf;
	for (; *p != TEXT('\0'); p++) {
		if (*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))) {
			len++;
		}
		if (*p == TEXT('\n') && (p == buf || *(p - 1) != TEXT('\r'))) {
			len++;
		}
		if (*p == TEXT('\r') && *(p + 1) != TEXT('\n')) {
			len++;
		}
		len++;
	}
	return len;
}

/*
 * SetDot - ピリオドから始まる行の先頭にピリオドを付加する
 */

void SetDot(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	p = buf;
	r = ret;
	for (; *p != TEXT('\0'); p++) {
		if (*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))) {
			*(r++) = TEXT('.');
		}
		if (*p == TEXT('\n') && (p == buf || *(p - 1) != TEXT('\r'))) {
			*(r++) = TEXT('\r');
		}
		*(r++) = *p;
		if (*p == TEXT('\r') && *(p + 1) != TEXT('\n')) {
			*(r++) = TEXT('\n');
		}
	}
	*r = TEXT('\0');
}

/*
 * DelDot - ピリオドから始まる行の先頭のピリオドを削除する
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
 * FixCRLF - deal with bare \n or \r
 */
void FixCRLF(char **buf)
{
	int incr = 0, len = 0;
	char *p;
	for (p = *buf; *p != '\0'; p++, len++) {
		if (*p == '\r' && *(p+1) != '\n') {
			incr++;
		}
		if (*p == '\n' && (p == *buf || (p > *buf && *(p-1) != '\r'))) {
			incr++;
		}
	}
	if (incr > 0) {
		char *ret, *q;
		ret = q = (char *)mem_alloc(sizeof(char)*(len + incr + 1));
		// it's OK if ret is NULL in the code below, though it's very unlikely
		for (p = *buf; *p != '\0'; p++) {
			if (ret != NULL) {
				*q = *p;
			}
			if (*p == '\r' && *(p+1) != '\n') {
				if (ret == NULL) {
					if (*(p+1) == '\r') {
						*(++p) = '\n'; // \r\r -> \r\n
					} else {
						*p = ' '; // \r -> ' ', working in-place
					}
				} else {
					q++;
					*q = '\n';
				}
			}
			// YPOPs! fix (some messages come in with bare \n)
			if (*p == '\n' && (p == *buf || (p > *buf && *(p-1) != '\r'))) {
				if (ret == NULL) {
					if (*(p+1) == '\n') {
						*(p++) = '\r'; // \n\n -> \r\n
					} else {
						*p = ' '; // \n -> ' ', working in-place
					}
				} else {
					*(q++) = '\r';
					*q = '\n';
				}
			}
			if (ret != NULL) {
				q++;
			}
		}
		if (ret != NULL) {
			*q = '\0';
			mem_free(&*buf);
			*buf = ret;
		}
	}
}

#ifdef UNICODE
void FixCRLF_t(TCHAR **buf)
{
	int incr = 0, len = 0;
	TCHAR *p;
	for (p = *buf; *p != TEXT('\0'); p++, len++) {
		if (*p == TEXT('\r') && *(p+1) != TEXT('\n')) {
			incr++;
		}
		if (*p == TEXT('\n') && (p == *buf || (p > *buf && *(p-1) != TEXT('\r')))) {
			incr++;
		}
	}
	if (incr > 0) {
		TCHAR *ret, *q;
		ret = q = (TCHAR *)mem_alloc(sizeof(TCHAR)*(len + incr + 1));
		// it's OK if ret is NULL in the code below, though it's very unlikely
		for (p = *buf; *p != TEXT('\0'); p++) {
			if (ret != NULL) {
				*q = *p;
			}
			if (*p == TEXT('\r') && *(p+1) != TEXT('\n')) {
				if (ret == NULL) {
					if (*(p+1) == TEXT('\r')) {
						*(++p) = TEXT('\n'); // \r\r -> \r\n
					} else {
						*p = TEXT(' '); // \r -> ' ', working in-place
					}
				} else {
					q++;
					*q = TEXT('\n');
				}
			}
			// YPOPs! fix (some messages come in with bare \n)
			if (*p == TEXT('\n') && (p == *buf || (p > *buf && *(p-1) != TEXT('\r')))) {
				if (ret == NULL) {
					if (*(p+1) == TEXT('\n')) {
						*(p++) = TEXT('\r'); // \n\n -> \r\n
					} else {
						*p = TEXT(' '); // \n -> ' ', working in-place
					}
				} else {
					*(q++) = TEXT('\r');
					*q = TEXT('\n');
				}
			}
			if (ret != NULL) {
				q++;
			}
		}
		if (ret != NULL) {
			*q = TEXT('\0');
			mem_free(&*buf);
			*buf = ret;
		}
	}
}
#endif

/*
 * ReturnCheck - 全角文字の禁則チェック
 */
static void ReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag)
{
	TCHAR *t;

	if (op.Oida != NULL) {
		for (t = op.Oida; *t != TEXT('\0'); t += CSTEP) {
			if (str_cmp_ni_t(p, t, CSTEP) == 0) {
				*TopFlag = TRUE;
				return;
			}
		}
	}
	if (op.Bura != NULL) {
		for (t = op.Bura; *t != TEXT('\0'); t += CSTEP) {
			if (str_cmp_ni_t(p, t, CSTEP) == 0) {
				*EndFlag = TRUE;
				return;
			}
		}
	}
}


/*
 * WordBreakStringSize - 文字列を指定の長さで折り返したときのサイズ
 *                        (line break algorithm modified by GJC)
 */
int WordBreakStringSize(TCHAR *buf, TCHAR *str, int BreakCnt, BOOL BreakFlag)
{
	TCHAR *p, *s;
	int cnt = 0;
	int ret = 0;
	int quotlen = (str == NULL) ? 0 : lstrlen(str);
	BOOL Quoting = FALSE;
	BOOL Flag = FALSE;
	BOOL TopFlag;
	BOOL EndFlag;

	if (BreakCnt <= 0) {
		return lstrlen(buf);
	}

	p = buf;

	if (str != NULL && str_cmp_ni_t(p, str, quotlen) == 0) {
		Flag = BreakFlag;
		Quoting = TRUE;
	}
	while (*p != TEXT('\0')) {
#ifdef UNICODE
		if (WCtoMB(CP_int, 0, p, 1, NULL, 0, NULL, NULL) != 1) {
#else
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
				if (Quoting == TRUE) {
					ret += quotlen;
					cnt += quotlen;
				}
			}
			cnt += 2;
			p += CSTEP;
			ret += CSTEP;

		} else if (*p == TEXT('\r')) {
			// CRNL
			p += 2;
			ret += 2;
			if (str != NULL && str_cmp_ni_t(p, str, quotlen) == 0) {
				Flag = BreakFlag;
				Quoting = TRUE;
				p += quotlen;
				ret += quotlen;
				cnt = quotlen;
			} else {
				Flag = FALSE;
				Quoting = FALSE;
				cnt = 0;
			}
			if (*p == TEXT(' ')) {
				// don't remove spaces after user-specified CRNL
				p++;
				cnt++;
				ret++;
			}

		} else if (*p == TEXT('\t')) {
			cnt += (TABSTOPLEN - (cnt % TABSTOPLEN));
			if (Flag == FALSE && cnt > BreakCnt) {
				cnt = (TABSTOPLEN - (cnt % TABSTOPLEN));
				ret += 2;
				if (Quoting == TRUE) {
					ret += quotlen;
					cnt += quotlen;
				}
			}
			p++;
			ret++;

		} else if (*p == TEXT(' ')) {
			if (cnt == 0 || (Quoting == TRUE && cnt == quotlen)) {
				// remove spaces at start of line
				p++;
			} else {
				if (Flag == TRUE || cnt < BreakCnt) {
					cnt++;
					ret++;
				}
				p++;
			}
		} else {
			for (s = p; !(is_white(*p)); p++) {
				cnt++;
			}
			if (Flag == FALSE && cnt > BreakCnt) {
				ret += 2;
				cnt = p - s;
				if (Quoting == TRUE) {
					ret += quotlen;
					cnt += quotlen;
				}
			}
			ret += p - s;
		}
	}
	return ret;
}

/*
 * WordBreakString - 文字列を指定の長さで折り返す
 *                   (line break algorithm modified by GJC)
 */
void WordBreakString(TCHAR *buf, TCHAR *ret, TCHAR *str, int BreakCnt, BOOL BreakFlag)
{
	TCHAR *p, *r, *s;
	int i, cnt = 0;
	int quotlen = (str == NULL) ? 0 : lstrlen(str);
	BOOL Quoting = FALSE;
	BOOL Flag = FALSE;
	BOOL TopFlag;
	BOOL EndFlag;

	if (BreakCnt <= 0) {
		lstrcpy(ret, buf);
		return;
	}

	p = buf;
	r = ret;

	if (str != NULL && str_cmp_ni_t(p, str, quotlen) == 0) {
		Flag = BreakFlag;
		Quoting = TRUE;
	}
	while (*p != TEXT('\0')) {
#ifdef UNICODE
		if (WCtoMB(CP_int, 0, p, 1, NULL, 0, NULL, NULL) != 1) {
#else
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
#endif
			// 2バイトコード
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
				if (Quoting == TRUE) {
					cnt += quotlen;
					for (i=0; i<quotlen; i++) {
						*(r++) = str[i];
					}
				}
			}
			cnt += 2;
			*(r++) = *(p++);
#ifndef UNICODE
			*(r++) = *(p++);
#endif

		} else if (*p == TEXT('\r')) {
			// CRNL
			*(r++) = *(p++);
			*(r++) = *(p++);
			if (str != NULL && str_cmp_ni_t(p, str, quotlen) == 0) {
				Flag = BreakFlag;
				Quoting = TRUE;
				cnt = quotlen;
				for (i=0; i<quotlen; i++) {
					*(r++) = *(p++);
				}
			} else {
				Flag = FALSE;
				Quoting = FALSE;
				cnt = 0;
			}
			if (*p == TEXT(' ')) {
				// don't remove spaces after user-specified CRNL
				cnt++;
				*(r++) = *(p++);
			}

		} else if (*p == TEXT('\t')) {
			// タブ
			cnt += (TABSTOPLEN - (cnt % TABSTOPLEN));
			if (Flag == FALSE && cnt > BreakCnt) {
				cnt = (TABSTOPLEN - (cnt % TABSTOPLEN));
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
				if (Quoting == TRUE) {
					cnt += quotlen;
					for (i=0; i<quotlen; i++) {
						*(r++) = str[i];
					}
				}
			}
			*(r++) = *(p++);

		} else if (*p == TEXT(' ')) {
			if (cnt == 0 || (Quoting == TRUE && cnt == quotlen)) {
				// remove spaces at start of line
				p++;
			} else {
				if (Flag == TRUE || cnt < BreakCnt) {
					*(r++) = *(p++);
					cnt++;
				} else {
					p++;
				}
			}

		} else {
			for (s = p; !(is_white(*p)); p++) {
				cnt++;
			}
			if (Flag == FALSE && cnt > BreakCnt) {
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
				cnt = p - s;
				if (Quoting == TRUE) {
					cnt += quotlen;
					for (i=0; i<quotlen; i++) {
						*(r++) = str[i];
					}
				}
			}
			for (; s != p; s++) {
				*(r++) = *s;
			}
		}
	}
	*r = TEXT('\0');
}

/*
 * GetQuoteString - get quote prefix (GJC)
 */
BOOL GetQuoteString(TCHAR *str, TCHAR *prefix, int maxlen) {
	// assumes '>' and '|' are only quote characters ...
	if (*str == TEXT('>') || *str == TEXT('|')) {
		TCHAR *p, *q, *m;
		q = prefix;
		m = prefix + maxlen - 1;
		for (p = str; (*p == TEXT('>') || *p == TEXT('|') || *p == TEXT(' ')) && q < m; p++, q++) {
			*q = *p;
		}
		// remove extra spaces from end
		while (*(q-1) == TEXT(' ') && *(q-2) == TEXT(' ')) {
			q--;
		}
		*q = TEXT('\0');
		return TRUE;
	} else {
		*prefix = TEXT('\0');
		return FALSE;
	}
}

/*
 * URLHeadToItem - URL中のヘッダ項目をアイテムに設定
 */
static BOOL URLHeadToItem(TCHAR *str, TCHAR *head, TCHAR **buf, TCHAR sep)
{
	TCHAR *p;

	if (str_cmp_ni_t(str, head, lstrlen(head)) != 0) {
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
		if (sep == TEXT('\0')) {
			mem_free(&*buf);
		} else {
			TCHAR *tmp = AllocURLDecode(p);
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(tmp) + lstrlen(*buf) + 2));
			if (p != NULL) {
				wsprintf(p, TEXT("%s%c%s"), *buf, sep, tmp);
			} else {
				p = tmp;
			}
			mem_free(&*buf);
			*buf = p;
			mem_free(&tmp);
		}
	} else {
		*buf = AllocURLDecode(p);
	}
	return TRUE;
}

/*
 * URLToMailItem - mailto URL scheme をメールアイテムに展開する (RFC 2368)
 */
BOOL URLToMailItem(TCHAR *buf, MAILITEM *tpMailItem)
{
#ifdef UNICODE
	TCHAR *body = NULL;
#endif
	TCHAR *tmp;
	TCHAR *p, *r, *s;

	for (p = buf; *p == TEXT(' '); p++);
	r = p;
	if (*r == TEXT('"')) r++;
	if (str_cmp_ni_t(r, URL_MAILTO, lstrlen(URL_MAILTO)) != 0) {
		// メールアドレスのみ
		tpMailItem->To = alloc_copy_t(p);
		s = tpMailItem->To + lstrlen(tpMailItem->To) - 1;
		while (*s == TEXT('.') || *s == TEXT(',') || *s == TEXT(';') || *s == TEXT(':')
				|| *s == TEXT('!') || *s == TEXT('?')) {
			// remove trailing punctuation
			*s = TEXT('\0');
			s--;
		}
		return TRUE;
	}
	if (r > p) {
		if (*(p+lstrlen(p)-1) == TEXT('"')) {
			*(p+lstrlen(p)-1) = TEXT('\0');
		}
		p++;
	}

	// メールアドレスの取得
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
		str_cpy_n_t(tmp, p, r - p + 1);
		tpMailItem->To = AllocURLDecode(tmp);
		mem_free(&tmp);
	}

	tmp = mem_alloc(sizeof(TCHAR) * (lstrlen(r) + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	while (*r != TEXT('\0')) {
		// ヘッダ項目の取得
		for (r++; *r == TEXT(' '); r++);
		for (s = tmp; *r != TEXT('\0') && *r != TEXT('&'); r++, s++) {
			*s = *r;
		}
		*s = TEXT('\0');

		URLHeadToItem(tmp, TEXT("to"), &tpMailItem->To, TEXT(','));
		URLHeadToItem(tmp, TEXT("cc"), &tpMailItem->Cc, TEXT(','));
		URLHeadToItem(tmp, TEXT("bcc"), &tpMailItem->Bcc, TEXT(','));
		URLHeadToItem(tmp, TEXT("replyto"), &tpMailItem->ReplyTo, TEXT('\0'));
		URLHeadToItem(tmp, TEXT("reply-to"), &tpMailItem->ReplyTo, TEXT('\0'));
		URLHeadToItem(tmp, TEXT("subject"), &tpMailItem->Subject, TEXT('\0'));
#ifdef UNICODE
		if (URLHeadToItem(tmp, TEXT("body"), &body, TEXT('\0')) == TRUE) {
			tpMailItem->Body = alloc_tchar_to_char(body);
			mem_free(&body);
		}
#else
		URLHeadToItem(tmp, TEXT("body"), &tpMailItem->Body, TEXT('\0'));
#endif
		URLHeadToItem(tmp, TEXT("mailbox"), &tpMailItem->MailBox, TEXT('\0'));
		URLHeadToItem(tmp, TEXT("attach"), &tpMailItem->Attach, ATTACH_SEP);
		if (str_cmp_n_t(tmp, TEXT("s=1"), 3) == 0) {
			tpMailItem->Mark = ICON_SEND;
		}
	}
	mem_free(&tmp);
	return TRUE;
}

/*
 * GetNextQuote - 文字列内の次の区切り位置までポインタを移動する
 */
static TCHAR *GetNextQuote(TCHAR *buf, TCHAR qStr)
{
	TCHAR *p;

	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
 * GetMailAddress - 文字列からメールアドレスの抽出 (RFC 822, RFC 2822 - addr-spec)
 *		MailAddress
 *		Comment <MailAddress>
 *		MailAddress (Comment)
 */

TCHAR *GetMailAddress(TCHAR *buf, TCHAR *ret, TCHAR *comment, BOOL quote)
{
	BOOL kFlag = FALSE;
	BOOL qFlag = FALSE;
	BOOL doing_comment = FALSE;
	TCHAR *p, *r, *s, *t;

	for (p = buf, r = ret, s = comment; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			if (comment != NULL && doing_comment == TRUE) {
				*(s++) = *(p++);
				*(s++) = *p;
			} else {
				*(r++) = *(p++);
				*(r++) = *p;
			}
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
			t = GetNextQuote(p + 1, TEXT('\"'));
			if (kFlag == TRUE || quote == TRUE) {
				for (; p < t; p++) {
					*(r++) = *p;
				}
				if (*p != TEXT('\0')) {
					*(r++) = *p;
				}
				break;
			}
			if (comment != NULL) {
				p++;
				for (; p < t; p++) {
					*(s++) = *p;
				}
				*s = TEXT('\0');
				break;
			}
			p = t;
			qFlag = TRUE;
			break;

		case TEXT('('):
			t = GetNextQuote(p + 1, TEXT(')'));
			if (comment != NULL) {
				p++;
				for (; p < t; p++) {
					*(s++) = *p;
				}
				*s = TEXT('\0');
				break;
			}
			p = t;
			break;

		case TEXT('<'):
			if (comment != NULL) {
				s = comment;
				t = buf;
				while (is_white(*t) || *t == TEXT('\"')) t++;
				while (t < p) {
					*(s++) = *(t++);
				}
				while (s > comment && (is_white(*(s-1)) || *(s-1) == TEXT('\"'))) {
					s--;
				}
				*s = TEXT('\0');
			}
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
				return GetMailAddress(buf, ret, NULL, TRUE);
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
		return GetMailAddress(buf, ret, NULL, TRUE);
	}
	return p;
}

/*
 * GetNameFromComment - split comment into firstname lastname or lastname, firstname
 */
#ifdef _WIN32_WCE
void GetNameFromComment(TCHAR *cmmt, TCHAR *fname, TCHAR *lname)
{
	TCHAR *p;
	BOOL done = FALSE;
	if (fname == NULL || lname == NULL) {
		return;
	}
	for (p = cmmt; *p != TEXT('\0'); p++) {
		if (*p == TEXT(',')) {
			*p = TEXT('\0');
			str_cpy_t(lname, cmmt);
			*p = TEXT(',');
			p++;
			while (*p == TEXT(' ')) p++;
			str_cpy_t(fname, p);
			done = TRUE;
			break;
		}
	}
	if (!done) {
		for (p = cmmt; *p != TEXT('\0'); p++) {
			if (*p == TEXT(' ')) {
				*p = TEXT('\0');
				str_cpy_t(fname, cmmt);
				*p = TEXT(' ');
				p++;
				while (*p == TEXT(' ')) p++;
				str_cpy_t(lname, p);
				done = TRUE;
				break;
			}
		}
	}
	if (!done) {
		str_cpy_t(lname, cmmt);
		*fname = TEXT('\0');
	}
}
#endif

/*
 * GetMailString - 文字列からメールアドレス(コメント含)の抽出
 */
TCHAR *GetMailString(TCHAR *buf, TCHAR *ret)
{
	BOOL kFlag = FALSE;
	BOOL BreakFlag = FALSE;
	TCHAR *p, *r;

	for (p = buf; *p == TEXT(' '); p++);
	for (r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
 * SetUserName - From: のコメントから幾つかの文字列を除去する
 */
void SetUserName(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
 * SetCcAddressSize - Cc, Bcc のリストのサイズ
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
			To = GetMailAddress(To, ToMailAddress, NULL, FALSE);
			cnt += lstrlen(ToMailAddress) + 2;
			To = (*To != TEXT('\0')) ? To + 1 : To;
		}
		cnt += 1;
		mem_free(&ToMailAddress);
	}
	return cnt;
}

/*
 * SetCcAddress - Cc, Bcc のリストの作成
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
		r = str_join_t(r, TEXT(" ("), Type, (TCHAR *)-1);
		while (*To != TEXT('\0')) {
			To = GetMailAddress(To, ToMailAddress, NULL, FALSE);
			r = str_join_t(r, sep, ToMailAddress, (TCHAR *)-1);
			To = (*To != TEXT('\0')) ? To + 1 : To;
			sep = TEXT(", ");
		}
		r = str_cpy_t(r, TEXT(")"));
		mem_free(&ToMailAddress);
	}
	return r;
}

/*
 * GetFileNameString - ファイルパスからファイルを取得する
 */
TCHAR *GetFileNameString(TCHAR *p)
{
	TCHAR *fname;

	for (fname = p; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
 * SetAttachListSize - 添付ファイルのリストのサイズ
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
		f = str_cpy_f_t(fpath, f, ATTACH_SEP);
		fname = GetFileNameString(fpath);
		len += lstrlen(fname);
	}
	mem_free(&fpath);
	len++;
	return len;
}

/*
 * SetAttachList - 添付ファイルのリストの作成
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
	p = str_cpy_t(p, TEXT(" ("));
	while (*f != TEXT('\0')) {
		if (f != buf) {
			p = str_cpy_t(p, TEXT(", "));
		}
		f = str_cpy_f_t(fpath, f, ATTACH_SEP);
		fname = GetFileNameString(fpath);
		p = str_cpy_t(p, fname);
	}
	mem_free(&fpath);
	p = str_cpy_t(p, TEXT(")"));
	return p;
}

/*
 * GetMIME2Extension - MIMEとファイルタイプの変換 (RFC 2046)
 */
char *GetMIME2Extension(char *MIMEStr, char *Filename)
{
	const char *MIME_list[] = {
		"application/mac-binhex40",".hqx",
		"application/postscript", ".eps",
		"application/postscript", ".ps",
		"application/postscript", ".ai",
		"application/rtf", ".rtf",
		"application/x-stuffit", ".sit",
		"application/x-uuencode", ".uue",
		"application/x-uuencode", ".uu",
		"application/x-zip-compressed", ".zip",
		"audio/basic", ".au",
		"audio/basic", ".snd",
		"audio/x-aiff", ".aif",
		"audio/x-aiff", ".aiff",
		"audio/x-pn-realaudio", ".ra",
		"audio/x-pn-realaudio", ".ram",
		"audio/x-wav", ".wav",
		"image/gif", ".gif",
		"image/jpeg", ".jpg",
		"image/jpeg", ".jpeg",
		"image/png", ".png",
		"image/tiff", ".tif",
		"image/tiff", ".tiff",
		"message/rfc822", ".eml",
		"text/html", ".htm",
		"text/html", ".html",
		"text/plain", ".txt",
		"text/x-vcard", ".vcf",
		"video/mpeg", ".mpg",
		"video/mpeg", ".mpeg",
		"video/quicktime", ".qt",
		"video/quicktime", ".mov",
		"video/x-msvideo", ".avi",
	};

	char *ret, *p, *r;
	int i;

	if (MIMEStr != NULL) {
		// Content type からファイルタイプを取得
		for (i = 0; i < (sizeof(MIME_list) / sizeof(char *)); i += 2) {
			if (str_cmp_i(MIMEStr, MIME_list[i]) == 0) {
				ret = alloc_copy(MIME_list[i + 1]);
				for (p = ret + 1; *p != '\0' && *p != '.'; p++);
				*p = '\0';
				return ret;
			}
		}
	} else if (Filename != NULL) {
		// ファイル名から Content type を取得
		for (r = p = Filename; *p != '\0'; p++) {
			if (*p == '.') {
				r = p;
			}
		}
		if (str_cmp_i(r, ".txt") != 0) {
			for (i = 1; i < (sizeof(MIME_list) / sizeof(char *)); i += 2) {
				if (str_cmp_i(r, MIME_list[i]) == 0) {
					return alloc_copy(MIME_list[i - 1]);
				}
			}
		}
		return alloc_copy("application/octet-stream");
	}
	return NULL;
}

/*
 * GetCommandLineSize - コマンドラインを作成したときのサイズ
 */
static int GetCommandLineSize(TCHAR *buf, TCHAR *filename)
{
	TCHAR *p;
	int ret = 0;

	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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
 * CreateCommandLine - コマンドラインの作成
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
		// 先頭に空白を追加
		*ret = TEXT(' ');
		r = ret + 1;
	} else {
		r = ret;
	}

	for (p = buf; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
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

/*
 * strip_html_tags - convert text/html-only mail to something readable (GJC)
 */
#define LAST_NONWHITE 0
#define LAST_NL       1
#define LAST_WHITE    2
TCHAR *strip_html_tags(TCHAR *buf, int insert_notice)
{
	TCHAR *p, *q, *ret, *s;
	int len;
	int last_char = LAST_NONWHITE;
	BOOL zero_margin = FALSE;

	len = lstrlen(buf) + lstrlen(STR_HTML_CONV) + 1;
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
	if (ret == NULL) {
		return NULL;
	}
	q = ret;
	if (insert_notice == 1) {
		wsprintf(q, TEXT("%s"), STR_HTML_CONV);
		q += lstrlen(STR_HTML_CONV);
	} else if (insert_notice == 2) {
		wsprintf(q, TEXT("%s"), STR_HTML_CONV_PARTIAL);
		q += lstrlen(STR_HTML_CONV_PARTIAL);
	}

	for (p = buf; *p != TEXT('\0'); ) {
		if (*p == TEXT('\r') && *(p+1) == TEXT('\n')) {
			// compress consecutive newlines
			if (last_char == LAST_NL) {
				p += 2;
			} else if (last_char == LAST_WHITE) {
				p += 2;
				q = s; // rewind to start of whitespace
				last_char = LAST_NL;
			} else {
				*(q++) = *(p++);
				*(q++) = *(p++);
				last_char = LAST_NL;
			}
			continue;
		}
		
		if (*p == TEXT('<')) {
			// strip <tags>
			if (str_cmp_ni_t(p, TEXT("<a "), lstrlen(TEXT("<a "))) == 0
				|| str_cmp_ni_t(p, TEXT("<a\r\n"), lstrlen(TEXT("<a\r\n"))) == 0) {
				// <a style="link" href="http://www.nowhere.com"> -> <http://www.nowhere.com>
				for (; *p != TEXT('\0') && *p != TEXT('>'); p++) {
					if (str_cmp_ni_t(p, TEXT("href=\""), lstrlen(TEXT("href=\""))) == 0) {
						p += lstrlen(TEXT("href=\""));
						*(q++) = TEXT('<');
						while (*p != TEXT('\0') && *p != TEXT('\"')) {
							*(q++) = *(p++);
						}
						if (*p != TEXT('\0')) {
							*(q++) = TEXT('>');
							p++;
						}
						while (*p != TEXT('\0') && *p != TEXT('>')) {
							p++;
						}
						break;
					}
				}
				if (*p != TEXT('\0')) {
					p++;
				}
				last_char = LAST_NONWHITE;
			} else if (str_cmp_ni_t(p, TEXT("<br>"), lstrlen(TEXT("<br>"))) == 0) {
				// GJC could also be <br />
				p += lstrlen(TEXT("<br>"));
				if (last_char == LAST_NONWHITE) {
					*(q++) = TEXT('\r');
					*(q++) = TEXT('\n');
				}
				last_char = LAST_NL;
			} else if (str_cmp_ni_t(p, TEXT("<p>"), lstrlen(TEXT("<p>"))) == 0) {
				// GJC what about </p>?
				p += lstrlen(TEXT("<p>"));
				if (last_char != LAST_NL || zero_margin == FALSE) {
					*(q++) = TEXT('\r');
					*(q++) = TEXT('\n');
					last_char = LAST_NL;
				}
			} else if (str_cmp_ni_t(p, TEXT("<head>"), lstrlen(TEXT("<head>"))) == 0) {
				p += lstrlen(TEXT("<head>"));
				while (*p != TEXT('\0')) {
					if (str_cmp_ni_t(p, TEXT("</head>"), lstrlen(TEXT("</head>"))) == 0) {
						p += lstrlen(TEXT("</head>"));
						break;
					} else if (str_cmp_ni_t(p, TEXT("<body"), lstrlen(TEXT("<body"))) == 0) {
						p += lstrlen(TEXT("<body"));
						while (*p != TEXT('\0') && *p != TEXT('>')) {
							p++;
						}
						if (*p == TEXT('>')) {
							p++;
						}
						break;
					} else if (str_cmp_ni_t(p, TEXT("margin:"), lstrlen(TEXT("margin:"))) == 0) {
						p += lstrlen(TEXT("margin:"));
						if (*p == TEXT('0') || (*p == TEXT(' ') && *(p+1) == TEXT('0'))) {
							zero_margin = TRUE;
						}
					} else {
						p++;
					}
				}
			} else if (str_cmp_ni_t(p, TEXT("<style"), lstrlen(TEXT("<style"))) == 0) {
				p += lstrlen(TEXT("<style"));
				while (*p != TEXT('\0') && str_cmp_ni_t(p, TEXT("</style>"), lstrlen(TEXT("</style>"))) != 0) {
					if (str_cmp_ni_t(p, TEXT("margin:"), lstrlen(TEXT("margin:"))) == 0) {
						p += lstrlen(TEXT("margin:"));
						if (*p == TEXT('0') || (*p == TEXT(' ') && *(p+1) == TEXT('0'))) {
							zero_margin = TRUE;
						}
					}
					p++;
				}
				if (*p != TEXT('\0')) {
					p += lstrlen(TEXT("</style>"));
				}
			} else if (str_cmp_ni_t(p, TEXT("<script"), lstrlen(TEXT("<script"))) == 0) {
				p += lstrlen(TEXT("<script"));
				while (*p != TEXT('\0') && str_cmp_ni_t(p, TEXT("</script>"), lstrlen(TEXT("</script>"))) != 0) {
					p++;
				}
				if (*p != TEXT('\0')) {
					p += lstrlen(TEXT("</script>"));
				}
			} else {
				int opens = 1;
				p++;
				while (*p != TEXT('\0') && opens > 0) {
					if (*p == TEXT('>')) opens--;
					if (*p == TEXT('<')) opens++;
					p++;
				}
			}
		
		} else if (*p == TEXT('&')) {
			last_char = LAST_NONWHITE;
			// convert &-codes (see http://www.htmlgoodies.com/beyond/reference/article.php/3472611)
			if (str_cmp_ni_t(p, TEXT("&amp;"), lstrlen(TEXT("&amp;"))) == 0) {
				p += lstrlen(TEXT("&amp;"));
				*(q++) = TEXT('&');
			} else if (str_cmp_ni_t(p, TEXT("&lt;"), lstrlen(TEXT("&lt;"))) == 0) {
				p += lstrlen(TEXT("&lt;"));
				*(q++) = TEXT('<');
			} else if (str_cmp_ni_t(p, TEXT("&gt;"), lstrlen(TEXT("&gt;"))) == 0) {
				p += lstrlen(TEXT("&gt;"));
				*(q++) = TEXT('>');
			} else if (str_cmp_ni_t(p, TEXT("&nbsp;"), lstrlen(TEXT("&nbsp;"))) == 0) {
				p += lstrlen(TEXT("&nbsp;"));
				*(q++) = TEXT(' ');
			} else if (str_cmp_ni_t(p, TEXT("&quot;"), lstrlen(TEXT("&quot;"))) == 0) {
				p += lstrlen(TEXT("&quot;"));
				*(q++) = TEXT('\"');
			} else if (str_cmp_ni_t(p, TEXT("&rsquo;"), lstrlen(TEXT("&rsquo;"))) == 0) {
				p += lstrlen(TEXT("&rsquo;"));
				*(q++) = TEXT('\'');
			} else if (str_cmp_ni_t(p, TEXT("&copy;"), lstrlen(TEXT("&copy;"))) == 0) {
				p += lstrlen(TEXT("&copy;"));
				*(q++) = TEXT('(');
				*(q++) = TEXT('C');
				*(q++) = TEXT(')');
			} else if (str_cmp_ni_t(p, TEXT("&reg;"), lstrlen(TEXT("&reg;"))) == 0) {
				p += lstrlen(TEXT("&reg;"));
				*(q++) = TEXT('(');
				*(q++) = TEXT('R');
				*(q++) = TEXT(')');
			} else if (str_cmp_ni_t(p, TEXT("&euro;"), lstrlen(TEXT("&euro;"))) == 0) {
				p += lstrlen(TEXT("&euro;"));
				*(q++) = TEXT('E');
				*(q++) = TEXT('U');
				*(q++) = TEXT('R');
			} else {
				*(q++) = *(p++);
			}
		} else {
			if (*p == TEXT(' ') || *p == TEXT('\t')) {
				if (last_char == LAST_NL) {
					s = q;
					last_char = LAST_WHITE;
				}
			} else {
				last_char = LAST_NONWHITE;
			}
			*(q++) = *(p++);
		}
	}
	*q = TEXT('\0');
	return ret;
}

/*
 * remove_superfluous_headers - remove unnecessary header lines (GJC)
 *		if (dups == TRUE), remove those that nPOPuk stores separately
 *		if (dups == FALSE), remove those that nPOPuk doesn't care about
 */
int remove_superfluous_headers(char *buf, BOOL dups)
{
	BOOL skip, did_one = FALSE;
	char *p, *r;
	p = r = buf;
	while (*p != '\0') {
		skip = FALSE;
		if (str_cmp_n(p, HEAD_SUBJECT, strlen(HEAD_SUBJECT)) == 0
			|| str_cmp_ni(p, HEAD_FROM, strlen(HEAD_FROM)) == 0
			|| str_cmp_ni(p, HEAD_TO, strlen(HEAD_TO)) == 0
			|| str_cmp_ni(p, HEAD_CC, strlen(HEAD_CC)) == 0
			|| str_cmp_ni(p, HEAD_REPLYTO, strlen(HEAD_REPLYTO)) == 0
			|| str_cmp_ni(p, HEAD_CONTENTTYPE, strlen(HEAD_CONTENTTYPE)) == 0
			|| str_cmp_ni(p, HEAD_ENCODING, strlen(HEAD_ENCODING)) == 0
			|| str_cmp_ni(p, HEAD_DATE, strlen(HEAD_DATE)) == 0
			|| str_cmp_ni(p, HEAD_MESSAGEID, strlen(HEAD_MESSAGEID)) == 0
			|| str_cmp_ni(p, HEAD_X_UIDL, strlen(HEAD_X_UIDL)) == 0
			|| str_cmp_ni(p, HEAD_INREPLYTO, strlen(HEAD_INREPLYTO)) == 0
			|| str_cmp_ni(p, HEAD_REFERENCES, strlen(HEAD_REFERENCES)) == 0
			|| str_cmp_ni(p, HEAD_IMPORTANCE, strlen(HEAD_IMPORTANCE)) == 0
			|| str_cmp_ni(p, HEAD_X_PRIORITY, strlen(HEAD_X_PRIORITY)) == 0
			|| str_cmp_ni(p, HEAD_PRIORITY, strlen(HEAD_PRIORITY)) == 0
			|| str_cmp_ni(p, HEAD_READ1, strlen(HEAD_READ1)) == 0
			|| str_cmp_ni(p, HEAD_READ2, strlen(HEAD_READ2)) == 0
			|| str_cmp_ni(p, HEAD_DELIVERY, strlen(HEAD_DELIVERY)) == 0
			|| str_cmp_ni(p, HEAD_READ2, strlen(HEAD_READ2)) == 0) {

			skip = (dups) ? TRUE : FALSE;

		} else {
			skip = (dups) ? FALSE : TRUE;
		}
		if (skip) {
				did_one = TRUE;
				while (*p != '\0' && ((p > buf && *(p-1) != '\r') || *p != '\n' || *(p+1) == ' ' || *(p+1) == '\t')) {
					p++;
				}
				p++;
		} else {
				while (*p != '\0' && ((p > buf && *(p-1) != '\r') || *p != '\n' || *(p+1) == ' ' || *(p+1) == '\t')) {
					// if did_one == FALSE, r==p and the next line is pointless, but we still need to advance r and p
					*(r++) = *(p++);
				}
				*(r++) = *(p++);
		}

		if (*p == '\r' && *(p+1) == '\n') {
			// reached end of header lines
			*(r++) = *(p++);
			*(r++) = *(p++);
			if (dups == TRUE) {
				// only deal with header lines				
				if (did_one) {
					*r = '\0';
				}
			} else {
				if (did_one) {
					while (*p != '\0') {
						*(r++) = *(p++);
					}
					*r = '\0';
				} else {
					while (*r != '\0') {
						r++;
					}
				}
			}
			break;
		}
	}
	return (r - buf);
}

/* 
 * remove_npopuk_headers - remove nPOPuk-specific headers from old-format message (GJC)
 */
int remove_npopuk_headers(char *buf)
{
	BOOL did_one = FALSE;
	char *p, *r;
	p = r = buf;
	while (*p != '\0') {
			if (str_cmp_n(p, HEAD_X_UIDL, strlen(HEAD_X_UIDL)) == 0
			|| str_cmp_ni(p, HEAD_X_MAILBOX, strlen(HEAD_X_MAILBOX)) == 0
			|| str_cmp_ni(p, HEAD_X_NO, strlen(HEAD_X_NO)) == 0
			|| str_cmp_ni(p, HEAD_X_STATUS, strlen(HEAD_X_STATUS)) == 0) {
				// skip
				did_one = TRUE;
				while (*p != '\0' && ((p > buf && *(p-1) != '\r') || *p != '\n' || *(p+1) == ' ' || *(p+1) == '\t')) {
					p++;
				}
				p++;
		} else {
				while (*p != '\0' && ((p > buf && *(p-1) != '\r') || *p != '\n' || *(p+1) == ' ' || *(p+1) == '\t')) {
					*(r++) = *(p++);
				}
				*(r++) = *(p++);
		}

		if (*p == '\r' && *(p+1) == '\n') {
			*(r++) = *(p++);
			*(r++) = *(p++);
			if (did_one) {
				*r = '\0';
			}
			break;
		}
	}
	return (r - buf);
}

/*
 * item_in_list - check if item is in list (GJC)
 */
BOOL item_in_list(TCHAR *item, TCHAR *list)
{
	TCHAR *buf, *p;
	if (list == NULL || *list == TEXT('\0')) {
		return FALSE;
	}
	if (item == NULL || *item == TEXT('\0')) {
		return TRUE;
	}
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(list) + 1));
	if (buf == NULL) {
		return FALSE;
	}
	p = list;
	while (*p != TEXT('\0')) {
		while (*p == TEXT(' ')) p++;
		p = str_cpy_f_t(buf, p, TEXT(','));
		if (lstrcmp(buf, item) == 0) {
			mem_free(&buf);
			return TRUE;
		}
	}
	mem_free(&buf);
	return FALSE;
}

/*
 * rot13_cpy - convert and copy
 */
void rot13_cpy(char *dest, char *start, char *end) {
	char *p, *q;
	for (p = dest, q = start; q < end; p++, q++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*q) == TRUE && (q+1) < end) {
			*(p++) = *(q++);
			*p = *q;
			continue;
		}
#endif
		if (*q >= 'a' && *q <= 'm')
			*p = (*q) + 13;
		else if (*q >= 'n' && *q <= 'z')
			*p = (*q) - 13;
		else if (*q >= 'A' && *q <= 'M')
			*p = (*q) + 13;
		else if (*q >= 'N' && *q <= 'Z')
			*p = (*q) - 13;
		else
			*p = *q;
	}
}

/*
 * rot13 - in-place conversion
 */
void rot13(char *start, char *end)
{
	char *p;
	for (p = start; p < end; p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && (p+1) < end) {
			p+=2;
		}
#endif
		if (*p >= 'a' && *p <= 'm') 
			*p += 13;
		else if (*p >= 'n' && *p <= 'z')
			*p -= 13;
		else if (*p >= 'A' && *p <= 'M')
			*p += 13;
		else if (*p >= 'N' && *p <= 'Z')
			*p -= 13;
	}
}

#if defined( _MSC_VER) && (_MSC_VER <= 1200)  && !defined(_WIN32_WCE)
#ifdef UNICODE
void _wgetenv_s(size_t *rsz, TCHAR *ret, int len, TCHAR *name)
{
	TCHAR *s = _wgetenv(name);
	*ret = TEXT('\0');
	if (s) {
		*rsz = lstrlen(s);
		str_cpy_n_t(ret, s, len-1);
	} else {
		*rsz = 0;
	}
}
#else
void getenv_s(size_t *rsz, char *ret, int len, char *name)
{
	char *s = getenv(name);
	*ret = '\0';
	if (s) {
		*rsz = strlen(s);
		str_cpy_n(ret, s, len-1);
	} else {
		*rsz = 0;
	}
}
#endif
#endif

/*
 * make_absolute - make path absolute (or copy)
 */
TCHAR *make_absolute(TCHAR *path_in)
{
	TCHAR *p, *ret;
	BOOL is_abs = FALSE;
	if (path_in == NULL) {
		return NULL;
	}
	// if path_in starts with . (or ..), it's a relative path
	if (*path_in != '.') {
		// if path_in contains \ or :, it's an absolute path
		// else, it's just a filename, assumed in the Resource directory
		for (p = path_in; *p != TEXT('\0'); p++) {
			if (*p == TEXT('\\') || *p == TEXT(':')) {
				is_abs = TRUE;
				break;
			}
		}
	}
	if (is_abs) {
		ret = alloc_copy_t(path_in);
	} else {
		int len = lstrlen(AppDir) + lstrlen(TEXT("Resource\\")) + lstrlen(path_in) + 1;
		ret = (TCHAR*)mem_alloc(sizeof(TCHAR) * len);
		if (ret != NULL) {
			wsprintf(ret, TEXT("%sResource\\%s"), AppDir, path_in);
		}
	}
	return ret;
}

/*
 * replace_env_var - replace %ENVVAR% with value of environment variable ENVVAR
 */
#ifndef _WIN32_WCE
TCHAR *replace_env_var(TCHAR *buf) {
	TCHAR *p, *q, *ret;
	TCHAR ev[BUF_SIZE];
	BOOL found = FALSE;
	int len = lstrlen(buf);
	size_t req_size;

	for (p = buf; *p != TEXT('\0') && *p != TEXT('%'); p++) {
		;
	}
	if (*p == TEXT('\0')) {
		return buf;
	}
	p++;
	for (q = p; *q != TEXT('\0') && *q != TEXT('%'); q++) {
		;
	}
	if (*q == TEXT('\0')) {
		return buf;
	}
	*q = TEXT('\0');
#ifdef UNICODE
	_wgetenv_s(&req_size, ev, BUF_SIZE, p);
#else
	getenv_s(&req_size, ev, BUF_SIZE, p);
#endif
	*q = TEXT('%');
	if (ev == NULL && req_size > BUF_SIZE) {
		return buf;
	}
	len += req_size - (q-p);
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
	if (ret == NULL) {
		return buf;
	}
	len = (p - buf);
	str_cpy_n_t(ret, buf, len);
	p = ret + len - 1;
	p = str_cpy_t(p, ev);
	p = str_cpy_t(p, q+1);
	mem_free(&buf);
	return ret;
}
#endif

// Thanks to Christoph at stackoverflow.com
BOOL is_utf8(unsigned const char *bytes) {

	if (bytes == NULL)
		return FALSE;

	while (*bytes) {
		if(	(// ASCII
				bytes[0] == 0x09 ||
				bytes[0] == 0x0A ||
				bytes[0] == 0x0D ||
				(0x20 <= bytes[0] && bytes[0] <= 0x7E)
			) ) {
				bytes += 1;
				continue;
		}

		if(	(// non-overlong 2-byte
				(0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF)
			) ) {
				bytes += 2;
				continue;
		}

		if(	(// excluding overlongs
				bytes[0] == 0xE0 &&
				(0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ||
			(// straight 3-byte
				((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
						bytes[0] == 0xEE ||
						bytes[0] == 0xEF) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ||
			(// excluding surrogates
				bytes[0] == 0xED &&
				(0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF)
			) ) {
				bytes += 3;
				continue;
		}

		if(	(// planes 1-3
				bytes[0] == 0xF0 &&
				(0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
				(0x80 <= bytes[3] && bytes[3] <= 0xBF)
			) ||
			(// planes 4-15
				(0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
				(0x80 <= bytes[3] && bytes[3] <= 0xBF)
			) ||
			(// plane 16
				bytes[0] == 0xF4 &&
				(0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
				(0x80 <= bytes[3] && bytes[3] <= 0xBF)
			) ) {
				bytes += 4;
				continue;
		}

		return FALSE;
	}

	return TRUE;
}

/* End of source */