/**************************************************************************

	nPOP

	Profile.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/


/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"
#include "Profile.h"


/**************************************************************************
	Define
**************************************************************************/

#define ALLOC_CNT		10

#define ToLower(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)


/**************************************************************************
	Global Variables
**************************************************************************/

static struct TPSECTION *tpSection;
static int SectionCnt;
static int SectionAllocCnt;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static BOOL AddSection(TCHAR *SectionName);
static int FindSection(TCHAR *SectionName);
static BOOL AddKey(struct TPSECTION *tpSec, TCHAR *KeyName, TCHAR *String, BOOL cFlag);
static int FindKey(struct TPSECTION *tpSec, TCHAR *KeyName);
static BOOL Profile_WriteData(TCHAR *Section, TCHAR *Key, TCHAR *str, TCHAR *File);


/******************************************************************************

	Trim

	文字列の前後の空白, Tabを除去する

******************************************************************************/

static BOOL Trim(TCHAR *buf)
{
	TCHAR *p, *r;

	//前後の空白を除いたポインタを取得
	for(p = buf; (*p == TEXT(' ') || *p == TEXT('\t')) && *p != TEXT('\0'); p++);
	for(r = buf + lstrlen(buf) - 1; r > p && (*r == TEXT(' ') || *r == TEXT('\t')); r--);
	*(r + 1) = TEXT('\0');

	//元の文字列にコピーを行う
	lstrcpy(buf, p);
	return TRUE;
}


/******************************************************************************

	str2hash

	文字列のハッシュ値を取得

******************************************************************************/

static int str2hash(TCHAR *str)
{
	int hash = 0;

	for(; *str != TEXT('\0'); str++){
		hash = ((hash << 1) + ToLower(*str));
	}
	return hash;
}


/******************************************************************************

	AddSection

	セクションの追加

******************************************************************************/

static BOOL AddSection(TCHAR *SectionName)
{
	struct TPSECTION *TmpSection;

	if(SectionName == NULL || *SectionName == TEXT('\0')){
		return FALSE;
	}

	if(SectionAllocCnt < SectionCnt + 1){
		SectionAllocCnt += ALLOC_CNT;
		TmpSection = (struct TPSECTION *)LocalAlloc(LPTR, sizeof(struct TPSECTION) * SectionAllocCnt);
		if(TmpSection == NULL){
			return FALSE;
		}
		if(tpSection != NULL){
			tCopyMemory(TmpSection, tpSection, sizeof(struct TPSECTION) * SectionCnt);
			LocalFree(tpSection);
		}
		tpSection = TmpSection;
	}
	lstrcpy((tpSection + SectionCnt)->SectionName, SectionName);
	Trim((tpSection + SectionCnt)->SectionName);
	(tpSection + SectionCnt)->hash = str2hash((tpSection + SectionCnt)->SectionName);

	SectionCnt++;
	return TRUE;
}


/******************************************************************************

	FindSection

	セクションの検索

******************************************************************************/

static int FindSection(TCHAR *SectionName)
{
	int hash;
	int i;

	if(tpSection == NULL || SectionName == NULL || *SectionName == TEXT('\0')){
		return -1;
	}

	hash = str2hash(SectionName);
	for(i = 0; i < SectionCnt; i++){
		if((tpSection + i)->hash != hash){
			continue;
		}
		if(lstrcmpi((tpSection + i)->SectionName, SectionName) == 0){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	AddKey

	キーの追加

******************************************************************************/

static BOOL AddKey(struct TPSECTION *tpSec, TCHAR *KeyName, TCHAR *String, BOOL cFlag)
{
	struct TPKEY *TmpKey;
	int index = -1;

	if(KeyName == NULL || *KeyName == TEXT('\0') || String == NULL){
		return FALSE;
	}

	if(tpSec->KeyAllocCnt < tpSec->KeyCnt + 1){
		tpSec->KeyAllocCnt += ALLOC_CNT;
		TmpKey = (struct TPKEY *)LocalAlloc(LPTR, sizeof(struct TPKEY) * tpSec->KeyAllocCnt);
		if(TmpKey == NULL){
			return FALSE;
		}
		if(tpSec->tpKey != NULL){
			tCopyMemory(TmpKey, tpSec->tpKey, sizeof(struct TPKEY) * tpSec->KeyCnt);
			LocalFree(tpSec->tpKey);
		}
		tpSec->tpKey = TmpKey;
	}
	(tpSec->tpKey + tpSec->KeyCnt)->String = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(String) + 1));
	if((tpSec->tpKey + tpSec->KeyCnt)->String == NULL){
		return FALSE;
	}
	lstrcpy((tpSec->tpKey + tpSec->KeyCnt)->KeyName, KeyName);
	Trim((tpSec->tpKey + tpSec->KeyCnt)->KeyName);
	if(cFlag == FALSE){
		(tpSec->tpKey + tpSec->KeyCnt)->hash = str2hash((tpSec->tpKey + tpSec->KeyCnt)->KeyName);
	}
	lstrcpy((tpSec->tpKey + tpSec->KeyCnt)->String, String);
	(tpSec->tpKey + tpSec->KeyCnt)->CommentFlag = cFlag;

	tpSec->KeyCnt++;
	return TRUE;
}


/******************************************************************************

	FindKey

	キーの検索

******************************************************************************/

static int FindKey(struct TPSECTION *tpSec, TCHAR *KeyName)
{
	int hash;
	int i;

	if(tpSec->tpKey == NULL || KeyName == NULL || *KeyName == TEXT('\0')){
		return -1;
	}

	hash = str2hash(KeyName);
	for(i = 0; i < tpSec->KeyCnt; i++){
		if((tpSec->tpKey + i)->CommentFlag == TRUE ||
			(tpSec->tpKey + i)->hash != hash){
			continue;
		}
		if(lstrcmpi((tpSec->tpKey + i)->KeyName, KeyName) == 0){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	Profile_Initialize

	初期化

******************************************************************************/

BOOL Profile_Initialize(TCHAR *path, BOOL ReadFlag)
{
	HANDLE hFile;
	TCHAR *buf, *p, *r, *s;
	TCHAR tmp[BUFSIZE];
	char *cBuf;
	DWORD fSizeLow, fSizeHigh;
	DWORD ret;
	long FileSize;
#ifdef UNICODE
	long Len;
#endif

	if(ReadFlag == FALSE){
		tpSection = NULL;
		return TRUE;
	}

	/* ファイルを開く */
	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	/*  確保するサイズの取得 */
	fSizeLow = GetFileSize(hFile, &fSizeHigh);
	if(fSizeLow == 0xFFFFFFFF){
		CloseHandle(hFile);
		return FALSE;
	}
	FileSize = (long)fSizeLow;

	/* 読み取る領域の確保 */
	cBuf = (char *)LocalAlloc(LMEM_FIXED, FileSize + 1);
	if(cBuf == NULL){
		CloseHandle(hFile);
		return FALSE;
	}
	/* ファイルを読み込む */
	if(ReadFile(hFile, cBuf, fSizeLow, &ret, NULL) == FALSE){
		LocalFree(cBuf);
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	*(cBuf + FileSize) = '\0';

#ifdef UNICODE
	Len = CharToTcharSize(cBuf);
	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (Len + 1));
	if(buf == NULL){
		LocalFree(cBuf);
		CloseHandle(hFile);
		return FALSE;
	}
	CharToTchar(cBuf, buf, Len);
	FileSize = Len;
	LocalFree(cBuf);
#else
	buf = cBuf;
#endif

	tpSection = (struct TPSECTION *)LocalAlloc(LPTR, sizeof(struct TPSECTION) * ALLOC_CNT);
	if(tpSection == NULL){
		LocalFree(buf);
		return FALSE;
	}
	SectionCnt = 1;
	SectionAllocCnt = ALLOC_CNT;

	p = buf;
	while((FileSize > (p - buf)) && *p != TEXT('\0')){
		for(r = p; (FileSize > (r - buf)) && (*r != TEXT('\r') && *r != TEXT('\n')); r++);

		switch(*p)
		{
		case TEXT('['):
			if(p == r || *(r - 1) != TEXT(']')) break;
			*(r - 1) = TEXT('\0');
			AddSection(p + 1);
			break;

		case TEXT('\r'):
		case TEXT('\n'):
			break;

		default:
			if(tpSection == NULL || p == r) break;
			if(*p == TEXT('#')){
				//コメント
				for(s = tmp; p < r; p++, s++){
					*s = *p;
				}
				*s = TEXT('\0');
				AddKey((tpSection + SectionCnt - 1), tmp, TEXT(""), TRUE);
			}else{
				//キー
				for(s = tmp; p < r; p++, s++){
					if(*p == TEXT('=')) break;
					*s = *p;
				}
				*s = TEXT('\0');
				if(*p == TEXT('=')) p++;
				*r = TEXT('\0');
				AddKey((tpSection + SectionCnt - 1), tmp, p, FALSE);
			}

			if(FileSize > (r - buf)) r++;
		}
		p = r;
		for(; (FileSize > (p - buf)) && (*p == TEXT('\r') || *p == TEXT('\n')); p++);
	}
	LocalFree(buf);
	return TRUE;
}


/******************************************************************************

	Profile_Flush

	バッファをファイルに書き込む

******************************************************************************/

BOOL Profile_Flush(TCHAR *path)
{
	HANDLE hFile;
	TCHAR *buf, *p;
	int len;
	int i, j;

	if(tpSection == NULL){
		return FALSE;
	}

	//保存サイズの計算
	len = 0;
	for(i = 0; i < SectionCnt; i++){
		if((tpSection + i)->tpKey == NULL){
			continue;
		}

		//セクション名
		if(i != 0){
			len += lstrlen((tpSection + i)->SectionName) + 2 + 2;
		}

		for(j = 0; j < (tpSection + i)->KeyCnt; j++){
			if(*((tpSection + i)->tpKey + j)->KeyName == TEXT('\0')){
				continue;
			}
			//キー名
			len += lstrlen(((tpSection + i)->tpKey + j)->KeyName);
			if(((tpSection + i)->tpKey + j)->CommentFlag == FALSE){
				len++;
				if(((tpSection + i)->tpKey + j)->String != NULL){
					//文字列
					len += lstrlen(((tpSection + i)->tpKey + j)->String);
				}
			}
			len += 2;
		}
		len += 2;
	}

	/* 保存するための領域の確保 */
	p = buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(buf == NULL){
		return FALSE;
	}

	//保存文字列の作成
	for(i = 0; i < SectionCnt; i++){
		if((tpSection + i)->tpKey == NULL){
			continue;
		}

		//セクション名
		if(i != 0){
			*(p++) = TEXT('[');
			p = TStrCpy(p, (tpSection + i)->SectionName);
			p = TStrCpy(p, TEXT("]\r\n"));
		}

		for(j = 0; j < (tpSection + i)->KeyCnt; j++){
			if(*((tpSection + i)->tpKey + j)->KeyName == TEXT('\0')){
				continue;
			}
			//キー名
			p = TStrCpy(p, ((tpSection + i)->tpKey + j)->KeyName);
			if(((tpSection + i)->tpKey + j)->CommentFlag == FALSE){
				*(p++) = TEXT('=');

				if(((tpSection + i)->tpKey + j)->String != NULL){
					//文字列
					p = TStrCpy(p, ((tpSection + i)->tpKey + j)->String);
				}
			}
			p = TStrCpy(p, TEXT("\r\n"));
		}
		p = TStrCpy(p, TEXT("\r\n"));
	}
	*p = TEXT('\0');

	/* ファイルを開く */
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		LocalFree(buf);
		return FALSE;
	}
	/* ファイルに書き込む */
	if(WriteAsciiFile(hFile, buf, len) == FALSE){
		LocalFree(buf);
		CloseHandle(hFile);
		return FALSE;
	}
	/* メモリの解放 */
	LocalFree(buf);
	/* ファイルを閉じる */
	CloseHandle(hFile);
	return TRUE;
}


/******************************************************************************

	Profile_Free

	設定情報の解放

******************************************************************************/

void Profile_Free(void)
{
	int i, j;

	if(tpSection == NULL){
		return;
	}
	for(i = 0; i < SectionCnt; i++){
		if((tpSection + i)->tpKey == NULL){
			continue;
		}
		for(j = 0; j < (tpSection + i)->KeyCnt; j++){
			if(((tpSection + i)->tpKey + j)->String != NULL){
				LocalFree(((tpSection + i)->tpKey + j)->String);
			}
		}
		LocalFree((tpSection + i)->tpKey);
	}
	LocalFree(tpSection);
	tpSection = NULL;
	SectionCnt = 0;
	SectionAllocCnt = 0;
}


/******************************************************************************

	Profile_GetString

	文字列の取得

******************************************************************************/

long Profile_GetString(TCHAR *Section, TCHAR *Key, TCHAR *Default, TCHAR *ret, long size, TCHAR *File)
{
	TCHAR *buf, *p;
	int SectionIndex;
	int KeyIndex;
	int len;

	SectionIndex = FindSection(Section);
	if(SectionIndex == -1){
		TStrCpyN(ret, Default, size);
		return lstrlen(ret);
	}

	KeyIndex = FindKey((tpSection + SectionIndex), Key);
	if(KeyIndex == -1 || ((tpSection + SectionIndex)->tpKey + KeyIndex)->String == NULL){
		TStrCpyN(ret, Default, size);
		return lstrlen(ret);
	}

	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(((tpSection + SectionIndex)->tpKey + KeyIndex)->String) + 1));
	if(buf != NULL){
		lstrcpy(buf, ((tpSection + SectionIndex)->tpKey + KeyIndex)->String);
		Trim(buf);
		p = (*buf == TEXT('\"')) ? buf + 1 : buf;
		len = lstrlen(p);
		if(len > 0){
			if(*(p + len - 1) == TEXT('\"')) *(p + len - 1) = TEXT('\0');
		}
		TStrCpyN(ret, p, size);
		LocalFree(buf);
	}else{
		TStrCpyN(ret, ((tpSection + SectionIndex)->tpKey + KeyIndex)->String, size);
	}
	return lstrlen(ret);
}


/******************************************************************************

	Profile_GetInt

	数値の取得

******************************************************************************/

int Profile_GetInt(TCHAR *Section, TCHAR *Key, int Default, TCHAR *File)
{
	TCHAR *buf, *p;
	int SectionIndex;
	int KeyIndex;
	int ret;
	int len;

	SectionIndex = FindSection(Section);
	if(SectionIndex == -1){
		return Default;
	}

	KeyIndex = FindKey((tpSection + SectionIndex), Key);
	if(KeyIndex == -1 || ((tpSection + SectionIndex)->tpKey + KeyIndex)->String == NULL){
		return Default;
	}

	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(((tpSection + SectionIndex)->tpKey + KeyIndex)->String) + 1));
	if(buf != NULL){
		lstrcpy(buf, ((tpSection + SectionIndex)->tpKey + KeyIndex)->String);
		Trim(buf);
		p = (*buf == TEXT('\"')) ? buf + 1 : buf;
		len = lstrlen(p);
		if(len > 0){
			if(*(p + len - 1) == TEXT('\"')) *(p + len - 1) = TEXT('\0');
		}
		ret = _ttoi(p);
		LocalFree(buf);
	}else{
		ret = _ttoi(((tpSection + SectionIndex)->tpKey + KeyIndex)->String);
	}
	return ret;
}


/******************************************************************************

	Profile_WriteData

	データの書き込み

******************************************************************************/

static BOOL Profile_WriteData(TCHAR *Section, TCHAR *Key, TCHAR *str, TCHAR *File)
{
	int SectionIndex;
	int KeyIndex;
	int j;

	if(Section == NULL){
		return FALSE;
	}

	if(tpSection == NULL){
		tpSection = (struct TPSECTION *)LocalAlloc(LPTR, sizeof(struct TPSECTION) * ALLOC_CNT);
		if(tpSection == NULL){
			return FALSE;
		}
		SectionCnt = 1;
		SectionAllocCnt = ALLOC_CNT;
	}

	SectionIndex = FindSection(Section);
	if(SectionIndex == -1){
		if(AddSection(Section) == FALSE){
			return FALSE;
		}
		SectionIndex = SectionCnt - 1;
	}

	if(Key == NULL){
		if((tpSection + SectionIndex)->tpKey != NULL){
			for(j = 0; j < (tpSection + SectionIndex)->KeyCnt; j++){
				if(((tpSection + SectionIndex)->tpKey + j)->String != NULL){
					LocalFree(((tpSection + SectionIndex)->tpKey + j)->String);
				}
			}
			LocalFree((tpSection + SectionIndex)->tpKey);
			(tpSection + SectionIndex)->tpKey = NULL;
			(tpSection + SectionIndex)->KeyCnt = 0;
			(tpSection + SectionIndex)->KeyAllocCnt = 0;
		}
		return TRUE;
	}

	KeyIndex = FindKey((tpSection + SectionIndex), Key);
	if(KeyIndex == -1){
		if(AddKey((tpSection + SectionIndex), Key, str, FALSE) == FALSE){
			return FALSE;
		}
	}else{
		if(((tpSection + SectionIndex)->tpKey + KeyIndex)->String != NULL){
			LocalFree(((tpSection + SectionIndex)->tpKey + KeyIndex)->String);
		}
		if(str == NULL){
			*((tpSection + SectionIndex)->tpKey + KeyIndex)->KeyName = TEXT('\0');
			((tpSection + SectionIndex)->tpKey + KeyIndex)->String = NULL;
			return TRUE;
		}
		((tpSection + SectionIndex)->tpKey + KeyIndex)->String = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(str) + 1));
		if(((tpSection + SectionIndex)->tpKey + KeyIndex)->String == NULL){
			return FALSE;
		}
		lstrcpy(((tpSection + SectionIndex)->tpKey + KeyIndex)->String, str);
	}
	return TRUE;
}


/******************************************************************************

	Profile_WriteString

	文字列の書き込み

******************************************************************************/

void Profile_WriteString(TCHAR *Section, TCHAR *Key, TCHAR *str, TCHAR *File)
{
	TCHAR *buf, *p;

	if(str == NULL || *str == TEXT('\0')){
		Profile_WriteData(Section, Key, TEXT(""), File);
		return;
	}

	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(str) + 3));
	if(buf == NULL){
		Profile_WriteData(Section, Key, str, File);
		return;
	}
	p = TStrCpy(buf, TEXT("\""));
	p = TStrCpy(p, str);
	p = TStrCpy(p, TEXT("\""));
	Profile_WriteData(Section, Key, buf, File);
	LocalFree(buf);
}


/******************************************************************************

	Profile_WriteInt

	数値の書き込み

******************************************************************************/

void Profile_WriteInt(TCHAR *Section, TCHAR *Key, int num, TCHAR *File)
{
	TCHAR ret[BUFSIZE];

	wsprintf(ret, TEXT("%d"), num);
	Profile_WriteData(Section, Key, ret, File);
}
/* End of source */
