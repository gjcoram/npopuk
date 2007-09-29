/**************************************************************************

	nPOP

	String.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"
#include "global.h"
#include "md5.h"


/**************************************************************************
	Define
**************************************************************************/

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


/**************************************************************************
	Global Variables
**************************************************************************/

extern TCHAR *Bura;
extern TCHAR *Oida;
extern TCHAR *sBura;
extern TCHAR *sOida;

extern TCHAR *TimeZone;
extern TCHAR *DateFormat;
extern TCHAR *TimeFormat;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

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


/******************************************************************************

	AllocTcharToChar

	メモリを確保して TCHAR を char に変換する

******************************************************************************/

#ifdef UNICODE
char *AllocTcharToChar(TCHAR *str)
{
	char *cchar;
	int len;

	len = TcharToCharSize(str);
	cchar = (char *)LocalAlloc(LMEM_FIXED, len + 1);
	if(cchar == NULL){
		return NULL;
	}
	TcharToChar(str, cchar, len);
	return cchar;
}
#endif


/******************************************************************************

	AllocCharToTchar

	メモリを確保して char を TCHAR に変換する

******************************************************************************/

#ifdef UNICODE
TCHAR *AllocCharToTchar(char *str)
{
	TCHAR *tchar;
	int len;

	len = CharToTcharSize(str);
	tchar = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(tchar == NULL){
		return NULL;
	}
	CharToTchar(str, tchar, len);
	return tchar;
}
#endif


/******************************************************************************

	a2i

	数字の文字列を数値(int)に変換する

******************************************************************************/

int a2i(const char *str)
{
	int num = 0;
	int m = 1;

	if(*str == '-'){
		m = -1;
		str++;
	}else if(*str == '+'){
		str++;
	}

	for(; *str >= '0' && *str <= '9'; str++){
		num = 10 * num + (*str - '0');
	}
	return num * m;
}


/******************************************************************************

	DelCtrlChar

	コントロール文字を削除する

******************************************************************************/

void DelCtrlChar(TCHAR *buf)
{
	TCHAR *p, *r;

	for(p = r = buf; *p != TEXT('\0'); p++){
		if(*p == TEXT('\t')){
			*(r++) = TEXT(' ');
		}else if(*p != TEXT('\r') && *p != TEXT('\n')){
			*(r++) = *p;
		}
	}
	*r = TEXT('\0');
}


/******************************************************************************

	AllocCopy

	バッファを確保して文字列をコピーする

******************************************************************************/

TCHAR *AllocCopy(const TCHAR *buf)
{
	TCHAR *ret;

	if(buf == NULL){
		return NULL;
	}
	ret = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(buf) + 1));
	if(ret != NULL){
		lstrcpy(ret, buf);
	}
	return ret;
}


/******************************************************************************

	TStrJoin

	文字列を連結して最後の文字のアドレスを返す

******************************************************************************/

TCHAR * __cdecl TStrJoin(TCHAR *ret, ... )
{
	va_list buf;
	TCHAR *str;

	va_start(buf, ret);

	str = va_arg(buf, TCHAR *);
	while(str != (TCHAR *)-1){
		if(str != NULL){
			while(*(ret++) = *(str++));
			ret--;
		}
		str = va_arg(buf, TCHAR *);
	}

	va_end(buf);
	return ret;
}


/******************************************************************************

	TStrCpy

	文字列をコピーして最後の文字のアドレスを返す

******************************************************************************/

TCHAR *TStrCpy(TCHAR *ret, TCHAR *buf)
{
	if(buf == NULL){
		*ret = TEXT('\0');
		return ret;
	}
	while(*(ret++) = *(buf++));
	ret--;
	return ret;
}


/******************************************************************************

	StrCpy

	文字列をコピーして最後の文字のアドレスを返す

******************************************************************************/

#ifdef UNICODE
char *StrCpy(char *ret, char *buf)
{
	if(buf == NULL){
		*ret = '\0';
		return ret;
	}
	while(*(ret++) = *(buf++));
	ret--;
	return ret;
}
#endif


/******************************************************************************

	StrCpyN

	指定された文字数なで文字列をコピーする

******************************************************************************/

#ifdef UNICODE
void StrCpyN(char *ret, char *buf, int len)
{
	while(--len && (*(ret++) = *(buf++)));
	*ret = '\0';
}
#endif


/******************************************************************************

	TStrCpyN

	指定された文字数なで文字列をコピーする

******************************************************************************/

void TStrCpyN(TCHAR *ret, TCHAR *buf, int len)
{
	while(--len && (*(ret++) = *(buf++)));
	*ret = TEXT('\0');
}


/******************************************************************************

	TStrCpyF

	指定の文字までの文字列をコピーする

******************************************************************************/

TCHAR *TStrCpyF(TCHAR *ret, TCHAR *buf, TCHAR c)
{
	TCHAR *p, *r;

	for(p = buf, r = ret; *p != c && *p != TEXT('\0'); p++, r++){
		*r = *p;
	}
	*r = TEXT('\0');
	return ((*p == c) ? p + 1 : p);
}


/******************************************************************************

	StrCatN

	指定された文字数まで文字列を追加する

******************************************************************************/

void StrCatN(TCHAR *ret, char *buf, int len)
{
	TCHAR *p, *r;
	int i;

#ifdef UNICODE
	TCHAR *tBuf;

	p = tBuf = AllocCharToTchar(buf);
	if(p == NULL){
		return;
	}
#else
	p = buf;
#endif

	i = lstrlen(ret);
	r = ret + i;
	while(*p != TEXT('\0') && i < len){
		*(r++) = *(p++);
		i++;
	}
	*r = TEXT('\0');

#ifdef UNICODE
	LocalFree(tBuf);
#endif
}


/******************************************************************************

	TStrCmp

	文字列の大文字小文字を区別する比較を行う

******************************************************************************/

int TStrCmp(const TCHAR *buf1, const TCHAR *buf2)
{
	int ret;

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		0, buf1, -1, buf2, -1);
	return ret - 2;
}


/******************************************************************************

	TStrCmpI

	文字列の大文字小文字を区別しない比較を行う (TCHAR)

******************************************************************************/

int TStrCmpI(const TCHAR *buf1, const TCHAR *buf2)
{
	int ret;

	ret = CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
		NORM_IGNORECASE, buf1, -1, buf2, -1);
	return ret - 2;
}


/******************************************************************************

	StrCmpI

	文字列の大文字小文字を区別しない比較を行う

******************************************************************************/

#ifdef UNICODE
int StrCmpI(const char *buf1, const char *buf2)
{
	TCHAR *str1, *str2;
	int ret;
	int llen;

	llen = CharToTcharSize(buf1);
	str1 = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (llen + 1));
	if(str1 == NULL){
		return -1;
	}
	CharToTchar(buf1, str1, llen);

	llen = CharToTcharSize(buf2);
	str2 = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (llen + 1));
	if(str2 == NULL){
		return -1;
	}
	CharToTchar(buf2, str2, llen);

	ret = TStrCmpI(str1, str2);

	LocalFree(str1);
	LocalFree(str2);
	return ret;
}
#endif


/******************************************************************************

	GetStrLen

	比較用の文字列の長さを取得する

******************************************************************************/

static int GetStrLen(const TCHAR *buf, int len)
{
	int i;

	for(i = 0; i < len; i++){
		if(*buf == TEXT('\0')){
			break;
		}
		buf++;
	}
	return i;
}


/******************************************************************************

	TStrCmpNI

	文字列の大文字小文字を区別しない比較を行う (TCHAR)

******************************************************************************/

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


/******************************************************************************

	StrCmpNI

	文字列の大文字小文字を区別しない比較を行う

******************************************************************************/

#ifdef UNICODE
int StrCmpNI(const char *buf1, const char *buf2, int len)
{
	TCHAR *str1, *str2;
	int ret;
	int llen;

	llen = ((unsigned int)len < tstrlen(buf1)) ? len : tstrlen(buf1);
	str1 = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (llen + 1));
	if(str1 == NULL){
		return -1;
	}
	CharToTchar(buf1, str1, llen);
	*(str1 + llen) = TEXT('\0');

	llen = ((unsigned int)len < tstrlen(buf2)) ? len : tstrlen(buf2);
	str2 = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (llen + 1));
	if(str2 == NULL){
		return -1;
	}
	CharToTchar(buf2, str2, llen);
	*(str2 + llen) = TEXT('\0');

	ret = TStrCmpNI(str1, str2, len);

	LocalFree(str1);
	LocalFree(str2);
	return ret;
}
#endif


/******************************************************************************

	StrMatch

	２つの文字列をワイルドカード(*)を使って比較を行う

******************************************************************************/

BOOL StrMatch(const TCHAR *Ptn, const TCHAR *Str)
{
#define ToLower(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)
	switch(*Ptn)
	{
	case TEXT('\0'):
		return (*Str == TEXT('\0'));
	case TEXT('*'):
		return StrMatch(Ptn + 1, Str) || (*Str != TEXT('\0')) && StrMatch(Ptn, Str + 1);
	case TEXT('?'):
		return (*Str != TEXT('\0')) && StrMatch(Ptn + 1, Str + 1);
	default:
		return (ToLower(*Ptn) == ToLower(*Str)) && StrMatch(Ptn + 1, Str + 1);
	}
}


/******************************************************************************

	StrFind

	文字列内に含まれる文字列を検索して位置を返す

******************************************************************************/

TCHAR *StrFind(TCHAR *Ptn, TCHAR *Str, int CaseFlag)
{
	TCHAR *p;
	int len1, len2;

	len1 = lstrlen(Ptn);
	for(p = Str; *p != '\0'; p++){
		len2 = GetStrLen(p, len1);
		if(CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
			(CaseFlag) ? 0 : NORM_IGNORECASE, p, len2, Ptn, len1) == 2){
			break;
		}
#ifndef UNICODE
		// 2バイトコードの場合は2バイト進める
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
		}
#endif
	}
	return p;
}


/******************************************************************************

	StrNextContentT

	ヘッダ内の次のコンテンツの先頭に移動する (TCHAR)

******************************************************************************/

static TCHAR *StrNextContentT(TCHAR *p)
{
	while(1){
		for(; !(*p == TEXT('\r') && *(p + 1) == TEXT('\n')) && *p != TEXT('\0'); p++);
		if(*p == TEXT('\0')){
			break;
		}
		p += 2;
		if(*p == TEXT(' ') || *p == TEXT('\t')){
			continue;
		}
		break;
	}
	return p;
}


/******************************************************************************

	GetHeaderStringPointT

	ヘッダから指定の項目のコンテンツの位置を取得 (TCHAR)

******************************************************************************/

TCHAR *GetHeaderStringPointT(TCHAR *buf, TCHAR *str)
{
	TCHAR *p;
	int len;

	len = lstrlen(str);
	p = buf;
	while(1){
		if(CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
			NORM_IGNORECASE, p, len, str, len) != 2){
			//次のコンテンツに移動する
			p = StrNextContentT(p);
			if(*p == TEXT('\0') || (*p == TEXT('\r') && *(p + 1) == TEXT('\n'))){
				break;
			}
			continue;
		}

		p += len;
		for(; *p == TEXT(' ') || *p == TEXT('\t'); p++);
		return p;
	}
	return NULL;
}


/******************************************************************************

	GetHeaderStringSizeT

	ヘッダから指定の項目のコンテンツのサイズを取得 (TCHAR)

******************************************************************************/

int GetHeaderStringSizeT(TCHAR *buf, BOOL CrLfFlag)
{
	TCHAR *p;
	int i;

	p = buf;
	i = 0;
	while(*p != TEXT('\0')){
		if(*p == TEXT('\r') && *(p + 1) == TEXT('\n')){
			p += 2;
			if(*p != TEXT(' ') && *p != TEXT('\t')){
				break;
			}
			if(CrLfFlag == TRUE){
				i += 2;
			}else{
				p++;
				continue;
			}
		}
		p++;
		i++;
	}
	return i;
}


/******************************************************************************

	GetHeaderStringT

	ヘッダから指定の項目のコンテンツを取得 (TCHAR)

******************************************************************************/

BOOL GetHeaderStringT(TCHAR *buf, TCHAR *ret, BOOL CrLfFlag)
{
	TCHAR *p, *r;

	p = buf;
	r = ret;
	while(*p != TEXT('\0')){
		if(*p == TEXT('\r') && *(p + 1) == TEXT('\n')){
			p += 2;
			if(*p != TEXT(' ') && *p != TEXT('\t')){
				break;
			}
			if(CrLfFlag == TRUE){
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
			}else{
				p++;
				continue;
			}
		}
		*(r++) = *(p++);
	}
	*r = TEXT('\0');
	return TRUE;
}


/******************************************************************************

	StrNextContent

	ヘッダ内の次のコンテンツの先頭に移動する (TCHAR)

******************************************************************************/

#ifdef UNICODE
static char *StrNextContent(char *p)
{
	while(1){
		for(; !(*p == '\r' && *(p + 1) == '\n') && *p != '\0'; p++);
		if(*p == '\0'){
			break;
		}
		p += 2;
		if(*p == ' ' || *p == '\t'){
			continue;
		}
		break;
	}
	return p;
}
#endif


/******************************************************************************

	GetHeaderStringPoint

	ヘッダから指定の項目のコンテンツの位置を取得

******************************************************************************/

#ifdef UNICODE
char *GetHeaderStringPoint(char *buf, char *str)
{
	char *p;
	int len;

	len = tstrlen(str);
	p = buf;
	while(1){
		if(StrCmpNI(p, str, len) != 0){
			//次のコンテンツに移動する
			p = StrNextContent(p);
			if(*p == '\0' || (*p == '\r' && *(p + 1) == '\n')){
				break;
			}
			continue;
		}

		p += len;
		for(; *p == ' ' || *p == '\t'; p++);
		return p;
	}
	return NULL;
}
#endif


/******************************************************************************

	GetHeaderStringSize

	ヘッダから指定の項目のコンテンツのサイズを取得

******************************************************************************/

#ifdef UNICODE
int GetHeaderStringSize(char *buf, BOOL CrLfFlag)
{
	char *p;
	int i;

	p = buf;
	i = 0;
	while(*p != '\0'){
		if(*p == '\r' && *(p + 1) == '\n'){
			p += 2;
			if(*p != ' ' && *p != '\t'){
				break;
			}
			if(CrLfFlag == TRUE){
				i += 2;
			}else{
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


/******************************************************************************

	GetHeaderString

	ヘッダから指定の項目のコンテンツを取得

******************************************************************************/

#ifdef UNICODE
BOOL GetHeaderString(char *buf, char *ret, BOOL CrLfFlag)
{
	char *p, *r;

	p = buf;
	r = ret;
	while(*p != '\0'){
		if(*p == '\r' && *(p + 1) == '\n'){
			p += 2;
			if(*p != ' ' && *p != '\t'){
				break;
			}
			if(CrLfFlag == TRUE){
				*(r++) = '\r';
				*(r++) = '\n';
			}else{
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


/******************************************************************************

	TrimMessageId

	文字列からMessage-IDのみを抽出する

******************************************************************************/

void TrimMessageId(char *buf)
{
	char *p, *r;

	if(buf == NULL){
		return;
	}

	for(p = buf; *p != '<' && *p != '\0'; p++);
	if(*p == '\0'){
		*buf = '\0';
		return;
	}

	for(r = p; *r != '>' && *r != '\0'; r++);
	if(*r == '\0'){
		*buf = '\0';
		return;
	}
	*(r + 1) = '\0';

	if(p != buf){
		tstrcpy(buf, p);
	}
}


/******************************************************************************

	GetReferencesSize

	Referencesを保存用に変換したときのサイズの取得

******************************************************************************/

int GetReferencesSize(char *p, BOOL Flag)
{
	char *t, *t2;

	t2 = t = p;
	while(*p != '\0'){
		if(*p == '>'){
			p++;
			for(; *p == ' ' || *p == '\t'; p++);
			if(*p == '\0'){
				break;
			}
			t2 = t;
			t = p;
		}else{
			p++;
		}
	}
	return tstrlen(((Flag == TRUE) ? t2 : t));
}


/******************************************************************************

	ConvReferences

	Referencesを保存用に変換する

******************************************************************************/

BOOL ConvReferences(char *p, char *r, BOOL Flag)
{
	char *t, *t2;

	t2 = t = p;
	while(*p != '\0'){
		if(*p == '>'){
			p++;
			for(; *p == ' ' || *p == '\t'; p++);
			if(*p == '\0'){
				break;
			}
			t2 = t;
			t = p;
		}else{
			p++;
		}
	}
	tstrcpy(r, ((Flag == TRUE) ? t2 : t));
	TrimMessageId(r);
	return TRUE;
}


/******************************************************************************

	GetBodyPointaT

	Body位置のを取得 (TCHAR)

******************************************************************************/

TCHAR *GetBodyPointaT(TCHAR *buf)
{
	TCHAR *p = buf;

	while(1){
		for(; *p != TEXT('\0') && !(*p == TEXT('\r') && *(p + 1) == TEXT('\n')); p++);
		if(*p == TEXT('\0')){
			break;
		}
		p += 2;
		if(*p == TEXT('\r') || *(p + 1) == TEXT('\n')){
			return (p + 2);
		}
	}
	return NULL;
}


/******************************************************************************

	GetBodyPointa

	Body位置のを取得

******************************************************************************/

#ifdef UNICODE
char *GetBodyPointa(char *buf)
{
	char *p = buf;

	while(1){
		for(; *p != '\0' && !(*p == '\r' && *(p + 1) == '\n'); p++);
		if(*p == '\0'){
			break;
		}
		p += 2;
		if(*p == '\r' || *(p + 1) == '\n'){
			return (p + 2);
		}
	}
	return NULL;
}
#endif


/******************************************************************************

	DateAdd

	日付の計算を行う

******************************************************************************/

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

	if(sTime == NULL || tmz == -1){
		//タイムゾーンの取得
		tmz = (TimeZone != NULL && *TimeZone != TEXT('\0')) ? _ttoi(TimeZone) : DEF_TIME_ZONE;
		tmz = tmz * 60 / 100;

		if(TimeZone == NULL || *TimeZone == TEXT('\0')){
			ret = GetTimeZoneInformation(&tmzi);
			if(ret != 0xFFFFFFFF){
				switch(ret)
				{
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
		//タイムゾーンの設定のみ
		if(sTime == NULL) return;
	}

	//変換元のタイムゾーンを'分'に変換
	if(*tz != '+' && *tz != '-'){
		if(StrCmpI(tz, "GMT") == 0 || StrCmpI(tz, "UT") == 0){
			//標準時間
			itz = 0;
		}else{
			return;
		}
	}else{
		itz = a2i(tz + 1);
		if(*tz == '-'){
			itz *= -1;
		}
	}
	itz = (itz / 100) * 60 + itz - (itz / 100) * 100;

	//差の計算
	diff = tmz - itz;
	if(diff == 0){
		return;
	}
	if(diff < 0){
		diff *= -1;
		f = -1;
	}

	//計算する値をFileTimeに変換
	tZeroMemory(&DiffFileTime, sizeof(FILETIME));
	if(FileTimeToSystemTime(&DiffFileTime, &DiffSystemTime) == FALSE){
		return;
	}
	DiffSystemTime.wHour += diff / 60;
	DiffSystemTime.wMinute += diff % 60;
	if(SystemTimeToFileTime(&DiffSystemTime, &DiffFileTime) == FALSE){
		return;
	}
	tCopyMemory(&DiffLInt, &DiffFileTime, sizeof(FILETIME));

	//FileTimeに変換して日付の計算を行う
	if(SystemTimeToFileTime(sTime, &fTime) == FALSE){
		return;
	}
	tCopyMemory(&LInt, &fTime, sizeof(FILETIME));
	LInt += DiffLInt * f;
	tCopyMemory(&fTime, &LInt, sizeof(FILETIME));
	FileTimeToSystemTime(&fTime, sTime);
}


/******************************************************************************

	FormatDateConv

	フォーマットにしたがって日付の展開を行う

******************************************************************************/

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

	tZeroMemory(gTime, sizeof(SYSTEMTIME));

	*tz = '\0';
	p = format;
	r = buf;
	while(*p != '\0'){
		switch(*p)
		{
		case 'w':	//曜日
			for(; IS_ALPHA(*r); r++);
			break;

		case 'd':	//日
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wDay = a2i(tmp);
			if(gTime->wDay == 0){
				return -1;
			}
			break;

		case 'M':	//月(数値)
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wMonth = a2i(tmp);
			if(gTime->wMonth == 0){
				return -1;
			}
			break;

		case 'm':	//月
			for(t = tmp; IS_ALPHA(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			for(i = 0; i < 12; i++){
				if(StrCmpNI(*(month + i), tmp, tstrlen(tmp) + 1) == 0){
					break;
				}
			}
			if(i >= 12){
				return -1;
			}
			gTime->wMonth = i + 1;
			break;

		case 'y':	//年
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wYear = a2i(tmp);
			if(gTime->wYear < 1900){
				if(gTime->wYear > 70){
					gTime->wYear += 1900;
				}else{
					gTime->wYear += 2000;
				}
			}
			break;

		case 'h':	//時
		case 'H':	//時
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wHour = a2i(tmp);
			break;

		case 'n':	//分
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wMinute = a2i(tmp);
			break;

		case 's':	//秒
			for(t = tmp; IS_NUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			gTime->wSecond = a2i(tmp);
			break;

		case 't':	//TZ
			for(t = tz; IS_ALNUM(*r); r++){
				*(t++) = *r;
			}
			*t = '\0';
			break;

		case '*':	//Not next char
			for(; *r != '\0' && *r != *(p + 1); r++);
			break;

		case ' ':	//Space
			for(; *r == ' '; r++);
			break;

		default:
			if(*p == *r){
				r++;
			}else{
				return -1;
			}
			break;
		}
		p++;
	}
	DateAdd(gTime, tz);
	return 0;
}


/******************************************************************************

	DateConv

	日付形式の変換を行う (RFC 822, RFC 2822)

******************************************************************************/

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

	if(tstrlen(buf) >= BUF_SIZE){
		StrCpyN(ret, buf, BUF_SIZE - 1);
		return -1;
	}

	//日時文字列を SYSTEMTIME に変換
	i = FormatDateConv("w, d m y h:n:s t", buf, &gTime);
	if(i == -1) i = FormatDateConv("w, d-m-y h:n:s t", buf, &gTime);
	if(i == -1) i = FormatDateConv("w, d m y h:n t", buf, &gTime);
	if(i == -1) i = FormatDateConv("w d m y h:n:s t", buf, &gTime);
	if(i == -1) i = FormatDateConv("w m d h:n:s y t", buf, &gTime);
	if(i == -1){
		tstrcpy(ret, buf);
		return -1;
	}

	fmt = (DateFormat != NULL && *DateFormat != TEXT('\0')) ? DateFormat : NULL;
	if(GetDateFormat(0, 0, &gTime, fmt, fDay, BUF_SIZE - 1) == 0){
		tstrcpy(ret, buf);
		return -1;
	}
	fmt = (TimeFormat != NULL && *TimeFormat != TEXT('\0')) ? TimeFormat : NULL;
	if(GetTimeFormat(0, 0, &gTime, fmt, fTime, BUF_SIZE - 1) == 0){
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


/******************************************************************************

	SortDateConv

	ソート用の日付形式変換を行う

******************************************************************************/

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

	if(tstrlen(buf) >= BUF_SIZE ||
		DateFormat == NULL || *DateFormat == TEXT('\0') ||
		TimeFormat == NULL || *TimeFormat == TEXT('\0')){
		StrCpyN(ret, buf, BUF_SIZE - 1);
		return -1;
	}

	//形式の生成
	r = tmp;
#ifdef UNICODE
	TcharToChar(DateFormat, fmt, BUF_SIZE);
	p = fmt;
#else
	p = DateFormat;
#endif
	while(*p != '\0'){
		c = *(p++);
		switch(c)
		{
		case 'y':
		case ' ':
			*(r++) = c;
			for(; *p == c; p++);
			break;

		case 'M':
			if(StrCmpNI(p, "MMM", 3) == 0){
				*(r++) = 'm';
			}else{
				*(r++) = 'M';
			}
			for(; *p == c; p++);
			break;

		case 'd':
			if(StrCmpNI(p, "dd", 2) == 0 || StrCmpNI(p, "ddd", 3) == 0){
				*(r++) = '*';
			}else{
				*(r++) = 'd';
			}
			for(; *p == c; p++);
			break;

		case 'g':
			*(r++) = '*';
			for(; *p == c; p++);
			break;

		default:
			*(r++) = c;
			break;
		}
	}
	*(r++) = ' ';

#ifdef UNICODE
	TcharToChar(TimeFormat, fmt, BUF_SIZE);
	p = fmt;
#else
	p = TimeFormat;
#endif
	while(*p != '\0'){
		c = *(p++);
		switch(c)
		{
		case 'H':
		case 'h':
		case 's':
		case ' ':
			*(r++) = c;
			for(; *p == c; p++);
			break;

		case 'm':
			*(r++) = 'n';
			for(; *p == c; p++);
			break;

		case 't':
			*(r++) = '*';
			for(; *p == c; p++);
			break;

		default:
			*(r++) = c;
			break;
		}
	}
	*r = '\0';

	//日時文字列を SYSTEMTIME に変換
	i = FormatDateConv(tmp, buf, &gTime);
	if(i == -1){
		tstrcpy(ret, buf);
		return -1;
	}

	if(GetDateFormat(0, 0, &gTime, TEXT("yyyyMMdd"), fDay, BUF_SIZE - 1) == 0){
		tstrcpy(ret, buf);
		return -1;
	}
	if(GetTimeFormat(0, 0, &gTime, TEXT("HHmm"), fTime, BUF_SIZE - 1) == 0){
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


/******************************************************************************

	GetTimeString

	時間文字列の取得 (RFC 822, RFC 2822)

******************************************************************************/

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

	tmz = (TimeZone != NULL && *TimeZone != TEXT('\0')) ? _ttoi(TimeZone) : DEF_TIME_ZONE;
	if(tmz < 0){
		tmz *= -1;
		c = TEXT('-');
	}else{
		c = TEXT('+');
	}
	if(TimeZone == NULL || *TimeZone == TEXT('\0')){
		ret = GetTimeZoneInformation(&tmzi);
		if(ret != 0xFFFFFFFF){
			switch(ret)
			{
			case TIME_ZONE_ID_STANDARD:
				tmzi.Bias += tmzi.StandardBias;
				break;

			case TIME_ZONE_ID_DAYLIGHT:
				//夏時間の計算
				tmzi.Bias += tmzi.DaylightBias;
				break;
			}

			if(tmzi.Bias < 0){
				tmzi.Bias *= -1;
				c = TEXT('+');
			}else{
				c = TEXT('-');
			}
			tmz = (tmzi.Bias / 60) * 100 + tmzi.Bias % 60;
		}
	}
	wsprintf(buf, TEXT("%s, %d %s %d %02d:%02d:%02d %c%04d"), Week[st.wDayOfWeek],
		st.wDay, Month[st.wMonth - 1], st.wYear, st.wHour, st.wMinute, st.wSecond, c, tmz);
}


/******************************************************************************

	EncodePassword_old

	文字列を暗号化 (旧)

******************************************************************************/

static void EncodePassword_old(TCHAR *buf, TCHAR *ret, int retsize)
{
	char *p, *r;
	int len;
	int i;

#ifdef UNICODE
	p = AllocTcharToChar(buf);
	if(p == NULL){
		*ret = TEXT('\0');
		return;
	}
	r = (char *)LocalAlloc(LMEM_FIXED, retsize);
	if(r == NULL){
		LocalFree(p);
		*ret = TEXT('\0');
		return;
	}
#else
	p = buf;
	r = ret;
#endif

	Base64Decode(p, r);

	len = tstrlen(r);
	for(i = 0; i < len; i++){
		*(r + i) ^= 0xFF;
	}
	*(r + i) = '\0';

#ifdef UNICODE
	CharToTchar(r, ret, retsize);
	LocalFree(p);
	LocalFree(r);
#endif
}


/******************************************************************************

	EncodePassword

	文字列を暗号化

******************************************************************************/

void EncodePassword(TCHAR *Key, TCHAR *Word, TCHAR *ret, int retsize, BOOL decode)
{
	MD5_CTX context;
	unsigned char digest[16];
	unsigned char *p, *r, *t;
	int len;
	int i, j;

	if(Word == NULL || *Word == TEXT('\0')){
		*ret = TEXT('\0');
		return;
	}
	if(decode == TRUE && *Word != TEXT('=')){
		EncodePassword_old(Word, ret, retsize);
		return;
	}

	// digest値を取得
#ifdef UNICODE
	p = AllocTcharToChar((Key == NULL) ? TEXT('\0') : Key);
	if(p == NULL){
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
	LocalFree(p);
#endif

#ifdef UNICODE
	p = AllocTcharToChar(Word);
	if(p == NULL){
		*ret = TEXT('\0');
		return;
	}
#else
	p = AllocCopy(Word);
	if(p == NULL){
		*ret = TEXT('\0');
		return;
	}
#endif
	if(decode == TRUE){
		//デコード
		r = (char *)LocalAlloc(LMEM_FIXED, tstrlen(p) + 1);
		if(r == NULL){
			*ret = TEXT('\0');
			return;
		}
		t = Base64Decode(p + 1, r);
		len = t - r;
		LocalFree(p);
		p = r;

	}else{
		len = tstrlen(p);
	}

    // XOR
    for(i = 0, j = 0; i < len; i++, j++) {
		if(j >= 16){
			j = 0;
		}
		*(p + i) = *(p + i) ^ digest[j];
    }
	*(p + i) = '\0';

	if(decode == TRUE){
#ifdef UNICODE
		CharToTchar(p, ret, retsize);
#else
		lstrcpyn(ret, p, retsize);
#endif
	}else{
		//エンコード
		r = (char *)LocalAlloc(LMEM_FIXED, len * 2 + 4 + 1);
		if(r == NULL){
			*ret = TEXT('\0');
			return;
		}
		Base64Encode(p, r, len);
		LocalFree(p);
		p = r;

		*ret = TEXT('=');
#ifdef UNICODE
		CharToTchar(p, ret + 1, retsize - 1);
#else
		lstrcpyn(ret + 1, p, retsize - 1);
#endif
	}
	LocalFree(p);
}


/******************************************************************************

	EncodeCtrlChar

	制御文字を \〜 の形式に変換する

******************************************************************************/

void EncodeCtrlChar(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	if(buf == NULL){
		*ret = TEXT('\0');
		return;
	}
	for(p = buf, r = ret; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch(*p)
		{
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


/******************************************************************************

	DecodeCtrlChar

	\〜 の形式の文字列を制御文字に変換する

******************************************************************************/

void DecodeCtrlChar(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for(p = buf, r = ret; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		if(*p != TEXT('\\')){
			*(r++) = *p;
			continue;
		}
		p++;
		switch(*p)
		{
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


/******************************************************************************

	CreateMessageId

	Message-Id を生成する (RFC 822, RFC 2822)

******************************************************************************/

TCHAR *CreateMessageId(long id, TCHAR *MailAddress)
{
	SYSTEMTIME st;
	TCHAR *ret;
	int len;

	if(MailAddress == NULL){
		return NULL;
	}
	ret = (TCHAR *)LocalAlloc(LMEM_FIXED,
		sizeof(TCHAR) * (1 + 14 + 1 + 8 + 1 + lstrlen(MailAddress) + 1 + 1));
	if(ret == NULL){
		return NULL;
	}

	GetLocalTime(&st);
	len = wsprintf(ret, TEXT("<%04d%02d%02d%02d%02d%02d.%08X"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
		id * ((long)ret + st.wSecond + st.wMilliseconds));
	TStrJoin(ret + len, TEXT("."), MailAddress, TEXT(">"), (TCHAR *)-1);
	return ret;
}


/******************************************************************************

	CreateHeaderStringSize

	ヘッダ文字列を作成したときのサイズ

******************************************************************************/

int CreateHeaderStringSize(TCHAR *buf, struct TPMAILITEM *tpMailItem)
{
	TCHAR *p;
	int ret = 0;

	if(buf == NULL){
		return 0;
	}
	for(p = buf; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
			ret += 2;
			continue;
		}
#endif
		if(*p != TEXT('%')){
			ret++;
			continue;
		}
		p++;
		switch(*p)
		{
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


/******************************************************************************

	CreateHeaderString

	ヘッダ文字列の作成

******************************************************************************/

TCHAR *CreateHeaderString(TCHAR *buf, TCHAR *ret, struct TPMAILITEM *tpMailItem)
{
	TCHAR *p, *r, *t;

	if(buf == NULL){
		return ret;
	}
	for(p = buf, r = ret; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		if(*p != TEXT('%')){
			*(r++) = *p;
			continue;
		}
		p++;
		t = NULL;
		switch(*p)
		{
		//From:
		case TEXT('F'): case TEXT('f'):
			t = tpMailItem->From;
			break;

		//Message-ID:
		case TEXT('I'): case TEXT('i'):
			t = (tpMailItem->MessageID != NULL && *tpMailItem->MessageID == TEXT('<'))
				? tpMailItem->MessageID : NULL;
			break;

		//Date:
		case TEXT('D'): case TEXT('d'):
			t = tpMailItem->Date;
			break;

		//Subject:
		case TEXT('S'): case TEXT('s'):
			t = tpMailItem->Subject;
			break;

		//To:
		case TEXT('T'): case TEXT('t'):
			t = tpMailItem->To;
			break;

		//Cc:
		case TEXT('C'): case TEXT('c'):
			t = tpMailItem->Cc;
			break;

		//Bcc:
		case TEXT('B'): case TEXT('b'):
			t = tpMailItem->Bcc;
			break;

		//%
		case TEXT('%'):
			t = TEXT("%");
			break;
		}
		if(t != NULL){
			r = TStrCpy(r, t);
		}
	}
	*r = TEXT('\0');
	return r;
}


/******************************************************************************

	GetReplyBodySize

	引用文字列を付加したときの文字列のサイズを取得

******************************************************************************/

int GetReplyBodySize(TCHAR *body, TCHAR *ReStr)
{
	TCHAR *p, *s;
	int len = 0;;

	if(ReStr == NULL || *ReStr == TEXT('\0')){
		return lstrlen(body);
	}

	for(s = body + lstrlen(body) - 1; *s == '\r' || *s == '\n'; s--);

	len += lstrlen(ReStr);
	for(p = body; p <= s && *p != TEXT('\0'); p++){
		if(*p == TEXT('\n')){
			len++;
			if(*(p + 1) == TEXT('\0')){
				break;
			}
			len += lstrlen(ReStr);
		}else{
			len++;
		}
	}
	return (len + 2);
}


/******************************************************************************

	SetReplyBody

	文字列に引用文字列を追加する

******************************************************************************/

TCHAR *SetReplyBody(TCHAR *body, TCHAR *ret, TCHAR *ReStr)
{
	TCHAR *p, *r, *s;

	if(ReStr == NULL || *ReStr == TEXT('\0')){
		//引用文字が無い場合
		return TStrCpy(ret, body);
	}

	//最後尾の空行は引用しない
	for(s = body + lstrlen(body) - 1; *s == '\r' || *s == '\n'; s--);

	//先頭行の引用
	r = TStrCpy(ret, ReStr);
	for(p = body; p <= s && *p != TEXT('\0'); p++){
		if(*p == TEXT('\n')){
			//引用
			*(r++) = *p;
			if(*(p + 1) == TEXT('\0')){
				break;
			}
			r = TStrCpy(r, ReStr);
		}else{
			*(r++) = *p;
		}
	}
	return TStrCpy(r, TEXT("\r\n"));
}


/******************************************************************************

	SetDotSize

	ピリオドから始まる行の先頭にピリオドを付加するためのサイズの取得
	(行頭の "From" は ">From" に変換)

******************************************************************************/

int SetDotSize(TCHAR *buf)
{
#define FROM_LEN		4		//lstrlen(TEXT("from"))
#define CRLF_FROM_LEN	6		//lstrlen(TEXT("\r\nfrom"))
	TCHAR *p;
	int len = 0;

	p = buf;
	if(TStrCmpNI(p, TEXT("from"), FROM_LEN) == 0){
		len++;
	}
	for(; *p != TEXT('\0'); p++){
		if(*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))){
			len++;
		}else if(TStrCmpNI(p, TEXT("\r\nfrom"), CRLF_FROM_LEN) == 0){
			len++;
		}
		len++;
	}
	return len;
}


/******************************************************************************

	SetDot

	ピリオドから始まる行の先頭にピリオドを付加する
	(行頭の "From" は ">From" に変換)

******************************************************************************/

void SetDot(TCHAR *buf, TCHAR *ret)
{
#define FROM_LEN		4		//lstrlen(TEXT("from"))
#define CRLF_FROM_LEN	6		//lstrlen(TEXT("\r\nfrom"))
	TCHAR *p, *r;

	p = buf;
	r = ret;
	if(TStrCmpNI(p, TEXT("from"), FROM_LEN) == 0){
		*(r++) = TEXT('>');
	}
	for(; *p != TEXT('\0'); p++){
		if(*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))){
			*(r++) = TEXT('.');

		}else if(TStrCmpNI(p, TEXT("\r\nfrom"), CRLF_FROM_LEN) == 0){
			*(r++) = *(p++);
			*(r++) = *(p++);
			*(r++) = TEXT('>');
		}
		*(r++) = *p;
	}
	*r = TEXT('\0');
}


/******************************************************************************

	DelDot

	ピリオドから始まる行の先頭のピリオドを削除する

******************************************************************************/

void DelDot(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for(p = buf, r = ret; *p != TEXT('\0'); p++){
		if(*p == TEXT('.') && (p == buf || *(p - 1) == TEXT('\n'))){
			continue;
		}
		*(r++) = *p;
	}
	*r = TEXT('\0');
}


/******************************************************************************

	ReturnCheck

	全角文字の禁則チェック

******************************************************************************/

static void ReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag)
{
	TCHAR *t;

	if(Oida != NULL){
		for(t = Oida; *t != TEXT('\0'); t += CSTEP){
			if(TStrCmpNI(p, t, CSTEP) == 0){
				*TopFlag = TRUE;
				return;
			}
		}
	}
	if(Bura != NULL){
		for(t = Bura; *t != TEXT('\0'); t += CSTEP){
			if(TStrCmpNI(p, t, CSTEP) == 0){
				*EndFlag = TRUE;
				return;
			}
		}
	}
}


/******************************************************************************

	sReturnCheck

	半角文字の禁則チェック

******************************************************************************/

static void sReturnCheck(TCHAR *p, BOOL *TopFlag, BOOL *EndFlag)
{
	TCHAR *t;

	if(sOida != NULL){
		for(t = sOida; *t != TEXT('\0'); t++){
			if(*p == *t){
				*TopFlag = TRUE;
				return;
			}
		}
	}
	if(sBura != NULL){
		for(t = sBura; *t != TEXT('\0'); t++){
			if(*p == *t){
				*EndFlag = TRUE;
				return;
			}
		}
	}
}


/******************************************************************************

	WordBreakStringSize

	文字列を指定の長さで折り返したときのサイズ

******************************************************************************/

int WordBreakStringSize(TCHAR *buf, TCHAR *str, int BreakCnt, BOOL BreakFlag)
{
	TCHAR *p, *s;
	int cnt = 0;
	int ret = 0;
	BOOL Flag = FALSE;
	BOOL TopFlag;
	BOOL EndFlag;

	if(BreakCnt <= 0){
		return lstrlen(buf);
	}

	p = buf;

	if(str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0){
		Flag = BreakFlag;
	}
	while(*p != TEXT('\0')){
#ifdef UNICODE
		if(WideCharToMultiByte(CP_ACP, 0, p, 1, NULL, 0, NULL, NULL) != 1){
#else
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
#endif
			TopFlag = FALSE;
			EndFlag = FALSE;
			if(Flag == FALSE && (cnt + 2) >= BreakCnt){
				ReturnCheck(p, &TopFlag, &EndFlag);
			}

			if(Flag == FALSE && (((cnt + 2) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 2) == BreakCnt && TopFlag == TRUE))){
				cnt = 0;
				ret += 2;
			}
			cnt += 2;
			p += CSTEP;
			ret += CSTEP;

		}else if(*p == TEXT('\r')){
			cnt = 0;
			p += 2;
			ret += 2;
			if(str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0){
				Flag = BreakFlag;
			}else{
				Flag = FALSE;
			}

		}else if(*p == TEXT('\t')){
			cnt += (TABSTOPLEN - (cnt % TABSTOPLEN));
			if(Flag == FALSE && cnt > BreakCnt){
				cnt = (TABSTOPLEN - (cnt % TABSTOPLEN));
				ret += 2;
			}
			p++;
			ret++;

		}else if(IsBreakAlNum(*p)){
			for(s = p; IsBreakAlNum(*p); p++){
				cnt++;
			}
			if(*p == TEXT(' ')){
				cnt++;
				p++;
			}
			if(Flag == FALSE && cnt > BreakCnt){
				if(cnt != p - s){
					ret += 2;
				}
				cnt = p - s;
			}
			ret += p - s;

		}else{
			TopFlag = FALSE;
			EndFlag = FALSE;
			if(Flag == FALSE && (cnt + 1) >= BreakCnt){
				sReturnCheck(p, &TopFlag, &EndFlag);
			}

			if(Flag == FALSE && (((cnt + 1) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 1) == BreakCnt && TopFlag == TRUE))){
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


/******************************************************************************

	WordBreakString

	文字列を指定の長さで折り返す

******************************************************************************/

void WordBreakString(TCHAR *buf, TCHAR *ret, TCHAR *str, int BreakCnt, BOOL BreakFlag)
{
	TCHAR *p, *r, *s;
	int cnt = 0;
	BOOL Flag = FALSE;
	BOOL TopFlag;
	BOOL EndFlag;

	if(BreakCnt <= 0){
		lstrcpy(ret, buf);
		return;
	}

	p = buf;
	r = ret;

	if(str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0){
		Flag = BreakFlag;
	}
	while(*p != TEXT('\0')){
#ifdef UNICODE
		if(WideCharToMultiByte(CP_ACP, 0, p, 1, NULL, 0, NULL, NULL) != 1){
#else
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
#endif
			//2バイトコード
			TopFlag = FALSE;
			EndFlag = FALSE;
			if(Flag == FALSE && (cnt + 2) >= BreakCnt){
				ReturnCheck(p, &TopFlag, &EndFlag);
			}

			if(Flag == FALSE && (((cnt + 2) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 2) == BreakCnt && TopFlag == TRUE))){
				cnt = 0;
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
			}
			cnt += 2;
			*(r++) = *(p++);
#ifndef UNICODE
			*(r++) = *(p++);
#endif

		}else if(*p == TEXT('\r')){
			//改行
			cnt = 0;
			*(r++) = *(p++);
			*(r++) = *(p++);
			if(str != NULL && TStrCmpNI(p, str, lstrlen(str)) == 0){
				Flag = BreakFlag;
			}else{
				Flag = FALSE;
			}

		}else if(*p == TEXT('\t')){
			//タブ
			cnt += (TABSTOPLEN - (cnt % TABSTOPLEN));
			if(Flag == FALSE && cnt > BreakCnt){
				cnt = (TABSTOPLEN - (cnt % TABSTOPLEN));
				*(r++) = TEXT('\r');
				*(r++) = TEXT('\n');
			}
			*(r++) = *(p++);

		}else if(IsBreakAlNum(*p)){
			//英数字は途中改行しない
			for(s = p; IsBreakAlNum(*p); p++){
				cnt++;
			}
			if(*p == TEXT(' ')){
				cnt++;
				p++;
			}
			if(Flag == FALSE && cnt > BreakCnt){
				if(cnt != p - s){
					*(r++) = TEXT('\r');
					*(r++) = TEXT('\n');
				}
				cnt = p - s;
			}
			for(; s != p; s++){
				*(r++) = *s;
			}

		}else{
			TopFlag = FALSE;
			EndFlag = FALSE;
			if(Flag == FALSE && (cnt + 1) >= BreakCnt){
				sReturnCheck(p, &TopFlag, &EndFlag);
			}

			if(Flag == FALSE && (((cnt + 1) > BreakCnt && EndFlag == FALSE) ||
				((cnt + 1) == BreakCnt && TopFlag == TRUE))){
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


/******************************************************************************

	URLHeadToItem

	URL中のヘッダ項目をアイテムに設定

******************************************************************************/

static BOOL URLHeadToItem(TCHAR *str, TCHAR *head, TCHAR **buf)
{
	TCHAR *p;

	if(TStrCmpNI(str, head, lstrlen(head)) != 0){
		return FALSE;
	}
	p = str + lstrlen(head);
	for(; *p == TEXT(' '); p++);
	if(*p != TEXT('=')){
		return FALSE;
	}
	p++;
	for(; *p == TEXT(' '); p++);
	if(*buf != NULL){
		LocalFree(*buf);
	}
	*buf = AllocURLDecode(p);
	return TRUE;
}


/******************************************************************************

	URLToMailItem

	mailto URL scheme をメールアイテムに展開する (RFC 2368)

******************************************************************************/

BOOL URLToMailItem(TCHAR *buf, struct TPMAILITEM *tpMailItem)
{
	TCHAR *tmp;
	TCHAR *p, *r, *s;

	for(p = buf; *p == TEXT(' '); p++);
	if(TStrCmpNI(p, URL_MAILTO, lstrlen(URL_MAILTO)) != 0){
		//メールアドレスのみ
		tpMailItem->To = AllocCopy(p);
		return TRUE;
	}

	//メールアドレスの取得
	p += lstrlen(URL_MAILTO);
	for(r = p; *r != TEXT('\0') && *r != TEXT('?'); r++);
	if(*r == TEXT('\0')){
		tpMailItem->To = AllocURLDecode(p);
		return TRUE;
	}
	if((r - p) > 0){
		tmp = LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (r - p + 1));
		if(tmp == NULL){
			return FALSE;
		}
		TStrCpyN(tmp, p, r - p + 1);
		tpMailItem->To = AllocURLDecode(tmp);
		LocalFree(tmp);
	}

	tmp = LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(r) + 1));
	if(tmp == NULL){
		return FALSE;
	}
	while(*r != TEXT('\0')){
		//ヘッダ項目の取得
		for(r++; *r == TEXT(' '); r++);
		for(s = tmp; *r != TEXT('\0') && *r != TEXT('&'); r++, s++){
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
	LocalFree(tmp);
	return TRUE;
}


/******************************************************************************

	GetNextQuote

	文字列内の次の区切り位置までポインタを移動する

******************************************************************************/

static TCHAR *GetNextQuote(TCHAR *buf, TCHAR qStr)
{
	TCHAR *p;

	for(p = buf; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
			continue;
		}
#endif
		if(*p == TEXT('\\')){
			p++;
			continue;
		}
		if(*p == qStr){
			break;
		}
		if(*p == TEXT('\"')){
			p = GetNextQuote(p + 1, TEXT('\"'));
			if(*p == TEXT('\0')){
				break;
			}
		}
	}
	return p;
}


/******************************************************************************

	GetMailAddress

	文字列からメールアドレスの抽出 (RFC 822, RFC 2822 - addr-spec)
		MailAddress
		Comment <MailAddress>
		MailAddress (Comment)

******************************************************************************/

TCHAR *GetMailAddress(TCHAR *buf, TCHAR *ret, BOOL quote)
{
	BOOL kFlag = FALSE;
	BOOL qFlag = FALSE;
	TCHAR *p, *r, *t;

	for(p = buf, r = ret; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch(*p)
		{
		case TEXT(' '):
		case TEXT('\t'):
		case TEXT('\r'):
		case TEXT('\n'):
			break;

		case TEXT('\"'):
			if(kFlag == TRUE || quote == TRUE){
				t = GetNextQuote(p + 1, TEXT('\"'));
				for(; p < t; p++){
					*(r++) = *p;
				}
				if(*p != TEXT('\0')){
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
			if(kFlag == FALSE){
				*(r++) = *p;
				break;
			}
			*r = TEXT('\0');
			return GetNextQuote(p + 1, TEXT(','));

		case TEXT(','):
			*r = TEXT('\0');

			if(quote == FALSE && kFlag == FALSE && qFlag == TRUE){
				return GetMailAddress(buf, ret, TRUE);
			}
			return p;

		default:
			*(r++) = *p;
			break;
		}
		if(*p == TEXT('\0')){
			break;
		}
	}
	*r = TEXT('\0');

	if(quote == FALSE && kFlag == FALSE && qFlag == TRUE){
		return GetMailAddress(buf, ret, TRUE);
	}
	return p;
}


/******************************************************************************

	GetMailString

	文字列からメールアドレス(コメント含)の抽出

******************************************************************************/

TCHAR *GetMailString(TCHAR *buf, TCHAR *ret)
{
	BOOL kFlag = FALSE;
	BOOL BreakFlag = FALSE;
	TCHAR *p, *r;

	for(p = buf; *p == TEXT(' '); p++);
	for(r = ret; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch(*p)
		{
		case TEXT('\t'):
		case TEXT('\r'):
		case TEXT('\n'):
			break;

		case TEXT('\\'):
			*(r++) = *(p++);
			if(*p == TEXT('\0')){
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
			if(kFlag == TRUE){
				*(r++) = *p;
				break;
			}
			BreakFlag = TRUE;
			break;

		default:
			*(r++) = *p;
			break;
		}
		if(BreakFlag == TRUE){
			break;
		}
	}
	*r = TEXT('\0');
	for(; r != ret && *r == TEXT(' '); r--);
	*r = TEXT('\0');
	return p;
}


/******************************************************************************

	SetUserName

	From: のコメントから幾つかの文字列を除去する

******************************************************************************/

void SetUserName(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;

	for(p = buf, r = ret; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		switch(*p)
		{
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


/******************************************************************************

	SetCcAddressSize

	Cc, Bcc のリストのサイズ

******************************************************************************/

int SetCcAddressSize(TCHAR *To)
{
	TCHAR *ToMailAddress;
	int cnt = 0;

	if(To == NULL){
		return 0;
	}
	ToMailAddress = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(To) + 1));
	if(ToMailAddress != NULL){
		cnt += 5;
		while(*To != TEXT('\0')){
			To = GetMailAddress(To, ToMailAddress, FALSE);
			cnt += lstrlen(ToMailAddress) + 2;
			To = (*To != TEXT('\0')) ? To + 1 : To;
		}
		cnt += 1;
		LocalFree(ToMailAddress);
	}
	return cnt;
}


/******************************************************************************

	SetCcAddress

	Cc, Bcc のリストの作成

******************************************************************************/

TCHAR *SetCcAddress(TCHAR *Type, TCHAR *To, TCHAR *r)
{
	TCHAR *ToMailAddress;
	TCHAR *sep = TEXT(": ");

	if(To == NULL){
		return r;
	}
	ToMailAddress = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(To) + 1));
	if(ToMailAddress != NULL){
		r = TStrJoin(r, TEXT(" ("), Type, (TCHAR *)-1);
		while(*To != TEXT('\0')){
			To = GetMailAddress(To, ToMailAddress, FALSE);
			r = TStrJoin(r, sep, ToMailAddress, (TCHAR *)-1);
			To = (*To != TEXT('\0')) ? To + 1 : To;
			sep = TEXT(", ");
		}
		r = TStrCpy(r, TEXT(")"));
		LocalFree(ToMailAddress);
	}
	return r;
}


/******************************************************************************

	GetFileNameString

	ファイルパスからファイルを取得する

******************************************************************************/

TCHAR *GetFileNameString(TCHAR *p)
{
	TCHAR *fname;

	for(fname = p; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
			continue;
		}
#endif
		if(*p == TEXT('\\') || *p == TEXT('/')){
			fname = p + 1;
		}
	}
	return fname;
}


/******************************************************************************

	SetAttachListSize

	添付ファイルのリストのサイズ

******************************************************************************/

int SetAttachListSize(TCHAR *buf)
{
	TCHAR *fname, *fpath;
	TCHAR *f;
	int len = 0;

	if(buf == NULL || *buf == TEXT('\0')){
		return 0;
	}
	f = buf;
	fpath = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(f) + 1));
	if(fpath == NULL){
		return 0;
	}
	len += 2;
	while(*f != TEXT('\0')){
		if(f != buf){
			len += 2;
		}
		f = TStrCpyF(fpath, f, TEXT(','));
		fname = GetFileNameString(fpath);
		len += lstrlen(fname);
	}
	LocalFree(fpath);
	len++;
	return len;
}


/******************************************************************************

	SetAttachList

	添付ファイルのリストの作成

******************************************************************************/

TCHAR *SetAttachList(TCHAR *buf, TCHAR *ret)
{
	TCHAR *fname, *fpath;
	TCHAR *p, *f;

	if(buf == NULL || *buf == TEXT('\0')){
		return ret;
	}
	f = buf;
	p = ret;
	fpath = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(f) + 1));
	if(fpath == NULL){
		return ret;
	}
	p = TStrCpy(p, TEXT(" ("));
	while(*f != TEXT('\0')){
		if(f != buf){
			p = TStrCpy(p, TEXT(", "));
		}
		f = TStrCpyF(fpath, f, TEXT(','));
		fname = GetFileNameString(fpath);
		p = TStrCpy(p, fname);
	}
	LocalFree(fpath);
	p = TStrCpy(p, TEXT(")"));
	return p;
}


/******************************************************************************

	GetMIME2Extension

	MIMEとファイルタイプの変換 (RFC 2046)

******************************************************************************/

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

	if(MIMEStr != NULL){
		//Content type からファイルタイプを取得
		for(i = 0; i < (sizeof(MIME_list) / sizeof(TCHAR *)); i += 2){
			if(TStrCmpI(MIMEStr, MIME_list[i]) == 0){
				ret = AllocCopy(MIME_list[i + 1]);
				for(p = ret + 1; *p != TEXT('\0') && *p != TEXT('.'); p++);
				*p = TEXT('\0');
				return ret;
			}
		}
	}else if(Filename != NULL){
		//ファイル名から Content type を取得
		for(r = p = Filename; *p != TEXT('\0'); p++){
#ifndef UNICODE
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				p++;
				continue;
			}
#endif
			if(*p == TEXT('.')){
				r = p;
			}
		}
		if(TStrCmpI(r, TEXT(".txt")) != 0){
			for(i = 1; i < (sizeof(MIME_list) / sizeof(TCHAR *)); i += 2){
				if(TStrCmpI(r, MIME_list[i]) == 0){
					return AllocCopy(MIME_list[i - 1]);
				}
			}
		}
		return AllocCopy(TEXT("application/octet-stream"));
	}
	return NULL;
}


/******************************************************************************

	GetCommandLineSize

	コマンドラインを作成したときのサイズ

******************************************************************************/

static int GetCommandLineSize(TCHAR *buf, TCHAR *filename)
{
	TCHAR *p;
	int ret = 0;

	for(p = buf; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			p++;
			ret += 2;
			continue;
		}
#endif
		if(*p != TEXT('%')){
			ret++;
			continue;
		}
		p++;
		switch(*p)
		{
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


/******************************************************************************

	CreateCommandLine

	コマンドラインの作成

******************************************************************************/

TCHAR *CreateCommandLine(TCHAR *buf, TCHAR *filename, BOOL spFlag)
{
	TCHAR *ret;
	TCHAR *p, *r, *t;
	int len;

	if(buf == NULL || *buf == TEXT('\0')){
		return NULL;
	}

	len = GetCommandLineSize(buf, filename);
	len += ((spFlag == TRUE) ? 1 : 0);
	ret = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(ret == NULL){
		return NULL;
	}

	if(spFlag == TRUE){
		//先頭に空白を追加
		*ret = TEXT(' ');
		r = ret + 1;
	}else{
		r = ret;
	}

	for(p = buf; *p != TEXT('\0'); p++){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			*(r++) = *(p++);
			*(r++) = *p;
			continue;
		}
#endif
		if(*p != TEXT('%')){
			*(r++) = *p;
			continue;
		}
		p++;
		t = NULL;
		switch(*p)
		{
		//File
		case TEXT('1'):
			t = filename;
			break;

		//%
		case TEXT('%'):
			t = TEXT("%");
			break;
		}
		if(t != NULL){
			while(*(r++) = *(t++));
			r--;
		}
	}
	*r = TEXT('\0');
	return ret;
}
/* End of source */
