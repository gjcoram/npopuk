/**************************************************************************

	nPOP

	File.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"


/**************************************************************************
	Global Variables
**************************************************************************/

TCHAR *DataDir;

extern HINSTANCE hInst;  // Local copy of hInstance
extern TCHAR *AppDir;
extern struct TPMAILBOX *MailBox;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int GetMailCnt(TCHAR *buf, long Size);
static BOOL SaveAddressString(HANDLE hFile, struct TPMAILITEM *tpMailItem);


/******************************************************************************

	SaveLog

	���O�̕ۑ�

******************************************************************************/

BOOL SaveLog(TCHAR *fpath, TCHAR *fname, TCHAR *buf)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;

	//�t�@�C���ɕۑ�
	wsprintf(path, TEXT("%s%s"), fpath, fname);

	//�ۑ�����t�@�C�����J��
	hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);

	if(WriteAsciiFile(hFile, buf, lstrlen(buf)) == FALSE){
		CloseHandle(hFile);
		return FALSE;
	}
	if(*(buf + lstrlen(buf) - 1) != TEXT('\n')){
		if(WriteFile(hFile, "\r\n", 2, &ret, NULL) == FALSE){
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}


/******************************************************************************

	SaveLogSep

	���O�̋�؂�̕ۑ�

******************************************************************************/

BOOL SaveLogSep(TCHAR *fpath, TCHAR *fname, TCHAR *buf)
{
#define LOG_SEP				TEXT("\r\n-------------------------------- ")
	TCHAR fDay[BUF_SIZE];
	TCHAR fTime[BUF_SIZE];
	TCHAR *p;
	BOOL ret;

	if(GetDateFormat(0, 0, NULL, NULL, fDay, BUF_SIZE - 1) == 0){
		return FALSE;
	}
	if(GetTimeFormat(0, 0, NULL, NULL, fTime, BUF_SIZE - 1) == 0){
		return FALSE;
	}
	p = LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(LOG_SEP) + lstrlen(fDay) + 1 + lstrlen(fTime) + 2 + lstrlen(buf) + 2));
	if(p == NULL){
		return FALSE;
	}
	TStrJoin(p, LOG_SEP, fDay, TEXT(" "), fTime, TEXT(" ("), buf, TEXT(")"), (TCHAR *)-1);
	ret = SaveLog(fpath, fname, p);
	LocalFree(p);
	return ret;
}


/******************************************************************************

	LogClear

	���O�̃N���A

******************************************************************************/

BOOL LogClear(TCHAR *fpath, TCHAR *fname)
{
	TCHAR path[BUF_SIZE];

	wsprintf(path, TEXT("%s%s"), fpath, fname);
	return DeleteFile(path);
}


/******************************************************************************

	GetFileName

	�t�@�C�����̎擾

******************************************************************************/

BOOL GetFileName(HWND hWnd, TCHAR *ret, TCHAR *DefExt, TCHAR *filter, BOOL OpenSave)
{
	OPENFILENAME of;
	TCHAR path[BUF_SIZE];
#ifdef _WIN32_WCE
	TCHAR *ph;
#endif

	//�t�@�C���ɕۑ�
	lstrcpy(path, ret);
	tZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hInstance = hInst;
	of.hwndOwner = hWnd;
	if(filter == NULL){
		of.lpstrFilter = STR_FILE_FILTER;
	}else{
		of.lpstrFilter = filter;
	}
	of.nFilterIndex = 1;
	if(OpenSave == TRUE){
		of.lpstrTitle = STR_TITLE_OPEN;
	}else{
		of.lpstrTitle = STR_TITLE_SAVE;
	}
	of.lpstrFile = path;
	of.nMaxFile = BUF_SIZE - 1;
	of.lpstrDefExt = DefExt;
	of.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	//�t�@�C���I���_�C�A���O��\������
	if(OpenSave == TRUE){
		if(GetOpenFileName((LPOPENFILENAME)&of) == FALSE){
			return FALSE;
		}
	}else{
		if(GetSaveFileName((LPOPENFILENAME)&of) == FALSE){
			return FALSE;
		}
	}
#ifdef _WIN32_WCE
	ph = path;
	if(*ph == TEXT('\\') && *(ph + 1) == TEXT('\\')){
		ph++;
	}
	if(*(ph + lstrlen(ph)) == TEXT('.')){
		*(ph + lstrlen(ph)) = TEXT('\0');
	}
	lstrcpy(ret, ph);
#else
	lstrcpy(ret, path);
#endif
	return TRUE;
}


/******************************************************************************

	ReadFileBuf

	�t�@�C����ǂݍ���

******************************************************************************/

char *ReadFileBuf(TCHAR *path, long FileSize)
{
	HANDLE hFile;
	DWORD ret;
	char *cBuf;

	//�t�@�C�����J��
	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return NULL;
	}
	cBuf = (char *)LocalAlloc(LPTR, FileSize + 1);
	if(cBuf == NULL){
		CloseHandle(hFile);
		return NULL;
	}

	if(ReadFile(hFile, cBuf, FileSize, &ret, NULL) == FALSE){
		LocalFree(cBuf);
		CloseHandle(hFile);
		return NULL;
	}
	CloseHandle(hFile);
	return cBuf;
}


/******************************************************************************

	OpenFileBuf

	�t�@�C�����J��

******************************************************************************/

BOOL OpenFileBuf(HWND hWnd, TCHAR **buf)
{
	TCHAR path[BUF_SIZE];
	char *cBuf;
	long FileSize;

	*buf = NULL;

	//�t�@�C�����擾
	lstrcpy(path, TEXT("*.txt"));
	if(GetFileName(hWnd, path, TEXT("txt"), STR_TEXT_FILTER, TRUE) == FALSE){
		return TRUE;
	}

	//�t�@�C���̃T�C�Y���擾
	FileSize = GetFileSerchSize(path);
	if(FileSize <= 0){
		return TRUE;
	}

	//�t�@�C����ǂݍ���
	SwitchCursor(FALSE);
	cBuf = ReadFileBuf(path, FileSize);
	if(cBuf == NULL){
		SwitchCursor(TRUE);
		return FALSE;
	}

#ifdef UNICODE
	//UNICODE�ɕϊ�
	*buf = AllocCharToTchar(cBuf);
	if(*buf == NULL){
		LocalFree(cBuf);
		SwitchCursor(TRUE);
		return FALSE;
	}
	LocalFree(cBuf);
#else
	*buf = cBuf;
#endif
	SwitchCursor(TRUE);
	return TRUE;
}


/******************************************************************************

	SaveFile

	�t�@�C���̕ۑ�

******************************************************************************/

BOOL SaveFile(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;

	//�t�@�C���ɕۑ�
	if(FileName == NULL){
		*path = TEXT('\0');
	}else{
		lstrcpy(path, FileName);
	}
	if(GetFileName(hWnd, path, Ext, NULL, FALSE) == FALSE){
		return TRUE;
	}

	//�ۑ�����t�@�C�����J��
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	SwitchCursor(FALSE);
	if(WriteFile(hFile, buf, len, &ret, NULL) == FALSE){
		CloseHandle(hFile);
		SwitchCursor(TRUE);
		return FALSE;
	}
	SwitchCursor(TRUE);
	CloseHandle(hFile);
	return TRUE;
}


/******************************************************************************

	GetSaveHeaderStringSize

	�ۑ�����w�b�_�̃T�C�Y

******************************************************************************/

int GetSaveHeaderStringSize(TCHAR *Head, TCHAR *buf)
{
	if(buf == NULL){
		return 0;
	}
	return lstrlen(Head) + 1 + lstrlen(buf) + 2;
}


/******************************************************************************

	SaveHeaderString

	�w�b�_��ۑ����镶����̍쐬

******************************************************************************/

TCHAR *SaveHeaderString(TCHAR *Head, TCHAR *buf, TCHAR *ret)
{
	if(buf == NULL){
		return ret;
	}
	return TStrJoin(ret, Head, TEXT(" "), buf, TEXT("\r\n"), (TCHAR *)-1);
}


/******************************************************************************

	WriteAsciiFile

	�}���`�o�C�g�ɕϊ����ĕۑ�

******************************************************************************/

BOOL WriteAsciiFile(HANDLE hFile, TCHAR *buf, int len)
{
	DWORD ret;
#ifdef UNICODE
	char *str;
	int clen;

	if(len == 0) return TRUE;

	clen = TcharToCharSize(buf);
	str = (char *)LocalAlloc(LMEM_FIXED, clen + 1);
	if(str == NULL){
		return FALSE;
	}
	TcharToChar(buf, str, clen);
	if(WriteFile(hFile, str, clen - 1, &ret, NULL) == FALSE){
		LocalFree(str);
		return FALSE;
	}
	LocalFree(str);
	return TRUE;

#else
	if(len == 0) return TRUE;
	return WriteFile(hFile, buf, len, &ret, NULL);
#endif
}


/******************************************************************************

	ConvFilename

	�t�@�C�����ɂł��Ȃ������� _ �ɕϊ�����

******************************************************************************/

void ConvFilename(TCHAR *buf)
{
	TCHAR *p = buf;
	while(*p != TEXT('\0')){
#ifndef UNICODE
		if(IsDBCSLeadByte((BYTE)*p) == TRUE){
			//2�o�C�g�R�[�h
			p += 2;
			continue;
		}
#endif
		//�t�@�C�����ɂł��Ȃ������͎w��̕����ɕϊ�
		switch(*p)
		{
		case TEXT('\\'): case TEXT('/'): case TEXT(':'):
		case TEXT(','): case TEXT(';'): case TEXT('*'):
		case TEXT('?'): case TEXT('\"'): case TEXT('<'):
		case TEXT('>'): case TEXT('|'):
			*p = TEXT('_');
			break;
		}
		p++;
	}
}


/******************************************************************************

	SaveMail

	���[���{�b�N�X���̃��[����ۑ�

******************************************************************************/

BOOL SaveMail(TCHAR *FileName, struct TPMAILBOX *tpMailBox, int SaveFlag)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR *tmp, *p;
	int len = 0;
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif	//_WIN32_WCE
	TStrJoin(path, DataDir, FileName, (TCHAR *)-1);

	if(SaveFlag == 0){
		//�ۑ����Ȃ��ꍇ�͍폜
		DeleteFile(path);
		return TRUE;
	}

#ifndef DIV_SAVE
	//���[��������쐬
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == NULL){
			continue;
		}
		len += Item_GetStringSize(*(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);
	}
	p = tmp = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(tmp == NULL){
		return FALSE;
	}
	*p = TEXT('\0');
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == NULL){
			continue;
		}
		p = Item_GetString(p, *(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);
	}
#endif	//DIV_SAVE

	//�ۑ�
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
#ifndef DIV_SAVE
		LocalFree(tmp);
#endif	//DIV_SAVE
		return FALSE;
	}
#ifndef DIV_SAVE
	if(WriteAsciiFile(hFile, tmp, len) == FALSE){
		LocalFree(tmp);
		return FALSE;
	}
	LocalFree(tmp);
#else	//DIV_SAVE
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == NULL){
			continue;
		}
		len = Item_GetStringSize(*(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);

		p = tmp = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
		if(tmp == NULL){
			CloseHandle(hFile);
			return FALSE;
		}
		Item_GetString(tmp, *(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);
		if(WriteAsciiFile(hFile, tmp, len) == FALSE){
			LocalFree(tmp);
			CloseHandle(hFile);
			return FALSE;
		}
		LocalFree(tmp);
	}
#endif	//DIV_SAVE
	CloseHandle(hFile);
	return TRUE;
}


/******************************************************************************

	GetFileSerchSize

	�t�@�C���̃T�C�Y���擾����

******************************************************************************/

long GetFileSerchSize(TCHAR *FileName)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if((hFindFile = FindFirstFile(FileName, &FindData)) == INVALID_HANDLE_VALUE){
		return -1;
	}
	FindClose(hFindFile);

	if((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0){
		//�f�B���N�g���ł͂Ȃ��ꍇ�̓T�C�Y��Ԃ�
		return (long)FindData.nFileSizeLow;
	}
	return -1;
}


/******************************************************************************

	GetMailCnt

	���[���ꗗ�̕����񂩂烁�[���̐����擾

******************************************************************************/

static int GetMailCnt(TCHAR *buf, long Size)
{
	TCHAR *p, *r, *t;
	int ret = 0;

	p = buf;
	while(Size > p - buf && *p != TEXT('\0')){
		for(t = r = p; Size > r - buf && *r != TEXT('\0'); r++){
			if(*r == TEXT('\r') && *(r + 1) == TEXT('\n')){
				if(*t == TEXT('.') && (r - t) == 1){
					break;
				}
				t = r + 2;
			}
		}
		p = r;
		if(Size > p - buf && *p != TEXT('\0')){
			p += 2;
		}
		ret++;
	}
	return ret;
}


/******************************************************************************

	ReadItemList

	�t�@�C�����烁�[���A�C�e���̍쐬

******************************************************************************/

BOOL ReadItemList(TCHAR *FileName, struct TPMAILBOX *tpMailBox)
{
	struct TPMAILITEM *tpMailItem;
#ifndef _NOFILEMAP
	HANDLE hFile;
	HANDLE hMapFile;
#endif
	TCHAR path[BUF_SIZE];
	TCHAR *MemFile, *AllocBuf = NULL;
	TCHAR *p, *r, *s, *t;
	char *FileBuf;
	long FileSize;
#ifdef UNICODE
	long Len;
#endif
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	TStrJoin(path, DataDir, FileName, (TCHAR *)-1);

	FileSize = GetFileSerchSize(path);
	if(FileSize <= 0){
		return TRUE;
	}
#ifdef _NOFILEMAP
	FileBuf = ReadFileBuf(path, FileSize);
	if(FileBuf == NULL){
		return FALSE;
	}
#else	//_NOFILEMAP
	//�t�@�C�����J��
#ifdef _WIN32_WCE
	hFile = CreateFileForMapping(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else	//_WIN32_WCE
	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif	//_WIN32_WCE
	if(hFile == NULL){
		return FALSE;
	}

	hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if(hMapFile == NULL){
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	//_WCE_OLD
		return FALSE;
	}
	FileBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
	if(FileBuf == NULL){
		CloseHandle(hMapFile);
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	//_WCE_OLD
		return FALSE;
	}
#endif	//_NOFILEMAP

#ifdef UNICODE
	//UNICODE�ɕϊ�
	if(FileSize > 2 && *(FileBuf + 1) != '\0'){
		Len = CharToTcharSize(FileBuf);
		MemFile = AllocBuf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (Len + 1));
		if(MemFile == NULL){
#ifdef _NOFILEMAP
			LocalFree(FileBuf);
#else	//_NOFILEMAP
			UnmapViewOfFile(FileBuf);
			CloseHandle(hMapFile);
#ifndef _WCE_OLD
			CloseHandle(hFile);
#endif	//_WCE_OLD
#endif	//_NOFILEMAP
			return FALSE;
		}
		CharToTchar(FileBuf, MemFile, Len);
		FileSize = Len;
	}else{
		MemFile = (TCHAR *)FileBuf;
		FileSize /= sizeof(TCHAR);
	}
#else	//UNICODE
	MemFile = (TCHAR *)FileBuf;
#endif	//UNICODE

	//���[�������̃��������m��
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = GetMailCnt(MemFile, FileSize);
	tpMailBox->tpMailItem = (struct TPMAILITEM **)LocalAlloc(LPTR, sizeof(struct TPMAILITEM *) * tpMailBox->MailItemCnt);
	if(tpMailBox->tpMailItem == NULL){
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
#ifdef _NOFILEMAP
		LocalFree(FileBuf);
#else	//_NOFILEMAP
		UnmapViewOfFile(FileBuf);
		CloseHandle(hMapFile);
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	//_WCE_OLD
#endif	//_NOFILEMAP
		NULLCHECK_FREE(AllocBuf);
		return FALSE;
	}

	i = 0;
	p = MemFile;
	while(FileSize > p - MemFile && *p != TEXT('\0')){
		//�w�b�_���烁�[���A�C�e�����쐬
		tpMailItem = *(tpMailBox->tpMailItem + i) = Item_StringToItem(tpMailBox, p);

		//Body�ʒu�̎擾
		p = GetBodyPointaT(p);
		if(p == NULL){
			break;
		}
		//���[���̏I���̈ʒu���擾
		for(t = r = p; *r != TEXT('\0'); r++){
			if(*r == TEXT('\r') && *(r + 1) == TEXT('\n')){
				if(*t == TEXT('.') && (r - t) == 1){
					t -= 2;
					for(; FileSize > r - MemFile && (*r == TEXT('\r') || *r == TEXT('\n')); r++);
					break;
				}
				t = r + 2;
			}
		}
		if(tpMailItem != NULL){
			//Body���R�s�[
			if((t - p) > 0){
				tpMailItem->Body = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (t - p + 1));
				if(tpMailItem->Body != NULL){
					for(s = tpMailItem->Body; p < t; p++, s++){
						*s = *p;
					}
					*s = TEXT('\0');
				}
			}
			if(tpMailItem->Body == NULL && tpMailItem->MailStatus < ICON_SENDMAIL){
				tpMailItem->MailStatus = ICON_NON;
				if(tpMailItem->Status != ICON_DOWN && tpMailItem->Status != ICON_DEL && tpMailItem->Status != ICON_SEND){
					tpMailItem->Status = ICON_NON;
				}
				tpMailItem->Download = FALSE;
			}
		}
		p = r;
		i++;
	}

#ifdef _NOFILEMAP
	LocalFree(FileBuf);
#else	//_NOFILEMAP
	UnmapViewOfFile(FileBuf);
	CloseHandle(hMapFile);
#ifndef _WCE_OLD
	CloseHandle(hFile);
#endif	//_WCE_OLD
#endif	//_NOFILEMAP
	NULLCHECK_FREE(AllocBuf);
	return TRUE;
}


/******************************************************************************

	SaveAddressString

	�A�h���X�����P���ۑ�

******************************************************************************/

static BOOL SaveAddressString(HANDLE hFile, struct TPMAILITEM *tpMailItem)
{
	TCHAR *tmp;
	int len = 0;

	if(tpMailItem->To != NULL){
		len += lstrlen(tpMailItem->To);
	}
	if(tpMailItem->Subject != NULL && *tpMailItem->Subject != TEXT('\0')){
		len += 1;	//TAB
		len += lstrlen(tpMailItem->Subject);
	}
	len += 2;		//CRLF

	tmp = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(tmp == NULL){
		return FALSE;
	}
	if(tpMailItem->Subject != NULL && *tpMailItem->Subject != TEXT('\0')){
		TStrJoin(tmp, tpMailItem->To, TEXT("\t"), tpMailItem->Subject, TEXT("\r\n"), (TCHAR *)-1);
	}else{
		TStrJoin(tmp, tpMailItem->To, TEXT("\r\n"), (TCHAR *)-1);
	}

	if(WriteAsciiFile(hFile, tmp, len) == FALSE){
		LocalFree(tmp);
		return FALSE;
	}
	LocalFree(tmp);
	return TRUE;
}


/******************************************************************************

	SaveAddressBook

	�A�h���X�����t�@�C���ɕۑ�

******************************************************************************/

BOOL SaveAddressBook(TCHAR *FileName, struct TPMAILBOX *tpMailBox)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	TStrJoin(path, DataDir, FileName, (TCHAR *)-1);

	//�ۑ�����t�@�C�����J��
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}

	//�ۑ����镶������R�s�[
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == NULL){
			continue;
		}
		if(SaveAddressString(hFile, *(tpMailBox->tpMailItem + i)) == FALSE){
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}


/******************************************************************************

	ReadAddressBook

	�t�@�C������A�h���X����ǂݍ���

******************************************************************************/

int ReadAddressBook(TCHAR *FileName, struct TPMAILBOX *tpMailBox)
{
	struct TPMAILITEM *tpMailItem;
	TCHAR path[BUF_SIZE];
	TCHAR *MemFile, *AllocBuf = NULL;
	TCHAR *p, *r, *s;
	char *FileBuf;
	long FileSize;
#ifdef UNICODE
	long Len;
#endif
	int LineCnt = 0;
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	TStrJoin(path, DataDir, FileName, (TCHAR *)-1);

	FileSize = GetFileSerchSize(path);
	if(FileSize < 0){
		return 0;
	}
	if(FileSize == 0){
		return 1;
	}
	FileBuf = ReadFileBuf(path, FileSize);
	if(FileBuf == NULL){
		return -1;
	}

#ifdef UNICODE
	//UNICODE�ɕϊ�
	Len = CharToTcharSize(FileBuf);
	MemFile = AllocBuf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (Len + 1));
	if(MemFile == NULL){
		LocalFree(FileBuf);
		return -1;
	}
	CharToTchar(FileBuf, MemFile, Len);
	FileSize = Len;
#else	//UNICODE
	MemFile = (TCHAR *)FileBuf;
#endif	//UNICODE

	//�s���̃J�E���g
	for(LineCnt = 0, p = MemFile; *p != TEXT('\0'); p++){
		if(*p == TEXT('\n')){
			LineCnt++;
		}
	}
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = LineCnt;
	tpMailBox->tpMailItem = (struct TPMAILITEM **)LocalAlloc(LPTR, sizeof(struct TPMAILITEM *) * tpMailBox->MailItemCnt);
	if(tpMailBox->tpMailItem == NULL){
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		LocalFree(FileBuf);
		NULLCHECK_FREE(AllocBuf);
		return -1;
	}
	i = 0;
	p = MemFile;
	while(FileSize > p - MemFile && *p != TEXT('\0')){
		tpMailItem = *(tpMailBox->tpMailItem + i) = (struct TPMAILITEM *)LocalAlloc(LPTR, sizeof(struct TPMAILITEM));

		//���[���A�h���X
		for(r = p; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if(tpMailItem != NULL){
			tpMailItem->To = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (r - p + 1));
			if(tpMailItem->To != NULL){
				for(s = tpMailItem->To; p < r; p++, s++){
					*s = *p;
				}
				*s = '\0';
			}
		}
		if(*r == TEXT('\t')) r++;

		//�R�����g
		for(p = r; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if(tpMailItem != NULL){
			tpMailItem->Subject = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (r - p + 1));
			if(tpMailItem->Subject != NULL){
				for(s = tpMailItem->Subject; p < r; p++, s++){
					*s = *p;
				}
				*s = '\0';
			}
		}
		for(; *p != TEXT('\0') && *p != TEXT('\r') && *p != TEXT('\n'); p++);
		for(; *p == TEXT('\r') || *p == TEXT('\n'); p++);
		i++;
	}
	LocalFree(FileBuf);
	NULLCHECK_FREE(AllocBuf);
	return 1;
}
/* End of source */
