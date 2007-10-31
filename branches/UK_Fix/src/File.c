/*
 * nPOP
 *
 * File.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#ifdef _WIN32_WCE_PPC
#include "SelectFile.h"
///////////// MRP /////////////////////
#include "ppcpoom.h"
///////////// --- /////////////////////
#endif

/* Define */

/* Global Variables */
extern OPTION op;

extern HINSTANCE hInst;  // Local copy of hInstance
extern TCHAR *AppDir;
extern TCHAR *DataDir;
extern MAILBOX *MailBox;

/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int file_get_mail_count(char *buf, long Size);
static BOOL file_save_address_item(HANDLE hFile, MAILITEM *tpMailItem);

/*
 * log_init - ���O�̋�؂�̕ۑ�
 */
BOOL log_init(TCHAR *fpath, TCHAR *fname, TCHAR *buf)
{
#define LOG_SEP				TEXT("\r\n-------------------------------- ")
	TCHAR fDay[BUF_SIZE];
	TCHAR fTime[BUF_SIZE];
	TCHAR *p;
	BOOL ret;

	if (GetDateFormat(0, 0, NULL, NULL, fDay, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
	if (GetTimeFormat(0, 0, NULL, NULL, fTime, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
	p = mem_alloc(sizeof(TCHAR) * (lstrlen(LOG_SEP) + lstrlen(fDay) + 1 + lstrlen(fTime) + 2 + lstrlen(buf) + 2));
	if (p == NULL) {
		return FALSE;
	}
	str_join_t(p, LOG_SEP, fDay, TEXT(" "), fTime, TEXT(" ("), buf, TEXT(")"), (TCHAR *)-1);
	ret = log_save(fpath, fname, p);
	mem_free(&p);
	return ret;
}

/*
 * log_save
 */
BOOL log_save(TCHAR *fpath, TCHAR *fname, TCHAR *buf)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;

	//Retention to file
	wsprintf(path, TEXT("%s%s"), fpath, fname);

	//The file which it retains is opened
	hFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);

	if (file_write_ascii(hFile, buf, lstrlen(buf)) == FALSE) {
		CloseHandle(hFile);
		return FALSE;
	}
	if (*(buf + lstrlen(buf) - 1) != TEXT('\n')) {
		if (WriteFile(hFile, "\r\n", 2, &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}

/*
 * log_clear - of non activity Clearing
 */
BOOL log_clear(TCHAR *fpath, TCHAR *fname)
{
	TCHAR path[BUF_SIZE];

	wsprintf(path, TEXT("%s%s"), fpath, fname);
	return DeleteFile(path);
}

/*
 * dir_check - �f�B���N�g�����ǂ����`�F�b�N
 */
BOOL dir_check(TCHAR *path)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if ((hFindFile = FindFirstFile(path, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);

	if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return TRUE;
	}
	return FALSE;
}

/*
 * dir_delete - �f�B���N�g�����̃t�@�C�����폜
 */
BOOL dir_delete(TCHAR *Path, TCHAR *file)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	TCHAR sPath[BUF_SIZE];
	TCHAR buf[BUF_SIZE];

	if (Path == NULL || *Path == TEXT('\0')) {
		return FALSE;
	}
	wsprintf(sPath, TEXT("%s\\%s"), Path, file);

	if ((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	do{
		if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			wsprintf(buf, TEXT("%s\\%s"), Path, FindData.cFileName);
			DeleteFile(buf);
		}
	} while (FindNextFile(hFindFile, &FindData) == TRUE);
	FindClose(hFindFile);
	return TRUE;
}

/*
 * filename_conv - �t�@�C�����ɂł��Ȃ������� _ �ɕϊ�����
 */
void filename_conv(TCHAR *buf)
{
	TCHAR *p = buf, *r;

	while (*p != TEXT('\0')) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
			// 2�o�C�g�R�[�h
			p += 2;
			continue;
		}
#endif
		// �󔒂̏���
		if (*p == TEXT(' ') || *p == TEXT('\t')) {
			r = p + 1;
			for (; *r == TEXT(' ') || *r == TEXT('\t'); r++);
			if (p + 1 != r) {
				lstrcpy(p + 1, r);
			}
		}
		// �t�@�C�����ɂł��Ȃ������͎w��̕����ɕϊ�
		switch (*p) {
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

/*
 * filename_select - the log Acquisition
 */
BOOL filename_select(HWND hWnd, TCHAR *ret, TCHAR *DefExt, TCHAR *filter, BOOL OpenSave)
{
#ifdef _WIN32_WCE_PPC
	TCHAR path[BUF_SIZE];

	lstrcpy(path, ret);
	return SelectFile(hWnd, hInst, OpenSave, path, ret);
#else	// _WIN32_WCE_PPC
	OPENFILENAME of;
	TCHAR path[BUF_SIZE];
#ifdef _WIN32_WCE
	TCHAR *ph;
#endif	// _WIN32_WCE

	//Retention to file
	lstrcpy(path, ret);
	ZeroMemory(&of, sizeof(OPENFILENAME));
	of.lStructSize = sizeof(OPENFILENAME);
	of.hInstance = hInst;
	of.hwndOwner = hWnd;
	if (filter == NULL) {
		of.lpstrFilter = STR_FILE_FILTER;
	} else {
		of.lpstrFilter = filter;
	}
	of.nFilterIndex = 1;
	if (OpenSave == TRUE) {
		of.lpstrTitle = STR_TITLE_OPEN;
	} else {
		of.lpstrTitle = STR_TITLE_SAVE;
	}
	of.lpstrFile = path;
	of.nMaxFile = BUF_SIZE - 1;
	of.lpstrDefExt = DefExt;
	of.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	//File selective dialogue is indicated
	if (OpenSave == TRUE) {
		of.Flags |= OFN_FILEMUSTEXIST;
		if (GetOpenFileName((LPOPENFILENAME)&of) == FALSE) {
			return FALSE;
		}
	} else {
		if (GetSaveFileName((LPOPENFILENAME)&of) == FALSE) {
			return FALSE;
		}
	}
#ifdef _WIN32_WCE
	ph = path;
	if (*ph == TEXT('\\') && *(ph + 1) == TEXT('\\')) {
		ph++;
	}
	if (*(ph + lstrlen(ph)) == TEXT('.')) {
		*(ph + lstrlen(ph)) = TEXT('\0');
	}
	lstrcpy(ret, ph);
#else	// _WIN32_WCE
	lstrcpy(ret, path);
#endif	// _WIN32_WCE
	return TRUE;
#endif	// _WIN32_WCE_PPC
}

/*
 * file_get_size - �t�@�C���̃T�C�Y���擾����
 */
long file_get_size(TCHAR *FileName)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;

	if ((hFindFile = FindFirstFile(FileName, &FindData)) == INVALID_HANDLE_VALUE) {
		return -1;
	}
	FindClose(hFindFile);

	if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		// �f�B���N�g���ł͂Ȃ��ꍇ�̓T�C�Y��Ԃ�
		return (long)FindData.nFileSizeLow;
	}
	return -1;
}

/*
 * file_get_mail_count - ���[���ꗗ�̕����񂩂烁�[���̐����擾
 */
static int file_get_mail_count(char *buf, long Size)
{
	char *p, *r, *t;
	int ret = 0;

	p = buf;
	while (Size > p - buf && *p != '\0') {
		for (t = r = p; Size > r - buf && *r != '\0'; r++) {
			if (*r == '\r' && *(r + 1) == '\n') {
				if (*t == '.' && (r - t) == 1) {
					break;
				}
				t = r + 2;
			}
		}
		p = r;
		if (Size > p - buf && *p != '\0') {
			p += 2;
		}
		ret++;
	}
	return ret;
}

/*
 * file_read - of file name The file is read
 */
char *file_read(TCHAR *path, long FileSize)
{
	HANDLE hFile;
	DWORD ret;
	char *cBuf;

	//The file is opened
	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return NULL;
	}
	cBuf = (char *)mem_calloc(FileSize + 1);
	if (cBuf == NULL) {
		CloseHandle(hFile);
		return NULL;
	}

	if (ReadFile(hFile, cBuf, FileSize, &ret, NULL) == FALSE) {
		mem_free(&cBuf);
		CloseHandle(hFile);
		return NULL;
	}
	CloseHandle(hFile);
	return cBuf;
}

/*
 * file_read_select - The file is opened
 */
BOOL file_read_select(HWND hWnd, TCHAR **buf)
{
	TCHAR path[BUF_SIZE];
	char *cBuf;
	long FileSize;

	*buf = NULL;

	//File Natori profit
	lstrcpy(path, TEXT("*.txt"));
	if (filename_select(hWnd, path, TEXT("txt"), STR_TEXT_FILTER, TRUE) == FALSE) {
		return TRUE;
	}

	//Acquisition size of file
	FileSize = file_get_size(path);
	if (FileSize <= 0) {
		return TRUE;
	}

	//Conversion to
	SwitchCursor(FALSE);
	cBuf = file_read(path, FileSize);
	if (cBuf == NULL) {
		SwitchCursor(TRUE);
		return FALSE;
	}

#ifdef UNICODE
	//UNICODE which reads the file
	*buf = alloc_char_to_tchar(cBuf);
	if (*buf == NULL) {
		mem_free(&cBuf);
		SwitchCursor(TRUE);
		return FALSE;
	}
	mem_free(&cBuf);
#else
	*buf = cBuf;
#endif
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * file_read_mailbox - �t�@�C�����烁�[���A�C�e���̍쐬
 */
BOOL file_read_mailbox(TCHAR *FileName, MAILBOX *tpMailBox)
{
	MAILITEM *tpMailItem;
#ifndef _NOFILEMAP
	HANDLE hFile;
	HANDLE hMapFile;
#endif
	TCHAR path[BUF_SIZE];
	char *p, *r, *s, *t;
	///////////// MRP /////////////////////
	TCHAR pathBackup[BUF_SIZE];
	///////////// --- /////////////////////
	char *FileBuf;
	long FileSize;
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	///////////// MRP /////////////////////
#ifdef UNICODE
   wcscpy(pathBackup, path);
   wcscat(pathBackup, TEXT(".bak"));
#else
   strcpy(pathBackup, path);
   strcat(pathBackup, TEXT(".bak"));
#endif

	if(file_get_size(pathBackup) != -1) // Backup File exists
	{
		DeleteFile(path);  // delete the current file
		MoveFile(pathBackup, path); // replace the the current file with the backup file.
	}
	///////////// --- /////////////////////

	FileSize = file_get_size(path);
	if (FileSize <= 0) {
		return TRUE;
	}
#ifdef _NOFILEMAP
	FileBuf = file_read(path, FileSize);
	if (FileBuf == NULL) {
		return FALSE;
	}
#else	// _NOFILEMAP
	//Conversion to
#ifdef _WIN32_WCE
	hFile = CreateFileForMapping(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#else	// _WIN32_WCE
	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
#endif	// _WIN32_WCE
	if (hFile == NULL) {
		return FALSE;
	}

	hMapFile = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMapFile == NULL) {
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	// _WCE_OLD
		return FALSE;
	}
	FileBuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
	if (FileBuf == NULL) {
		CloseHandle(hMapFile);
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	// _WCE_OLD
		return FALSE;
	}
#endif	// _NOFILEMAP


	//Guaranty memory of mail several parts
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = file_get_mail_count(FileBuf, FileSize);
	tpMailBox->tpMailItem = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * tpMailBox->MailItemCnt);
	if (tpMailBox->tpMailItem == NULL) {
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
#ifdef _NOFILEMAP
		mem_free(&FileBuf);
#else	// _NOFILEMAP
		UnmapViewOfFile(FileBuf);
		CloseHandle(hMapFile);
#ifndef _WCE_OLD
		CloseHandle(hFile);
#endif	// _WCE_OLD
#endif	// _NOFILEMAP
		return FALSE;
	}

	i = 0;
	p = FileBuf;
	while (FileSize > p - FileBuf && *p != '\0') {
		// From header mail item acquisition
		tpMailItem = *(tpMailBox->tpMailItem + i) = item_string_to_item(tpMailBox, p);

		// Body position Position of end of mail acquisition
		p = GetBodyPointa(p);
		if (p == NULL) {
			break;
		}
		// ���[���̏I���̈ʒu���擾
		for (t = r = p; *r != '\0'; r++) {
			if (*r == '\r' && *(r + 1) == '\n') {
				if (*t == '.' && (r - t) == 1) {
					t -= 2;
					for (; FileSize > r - FileBuf && (*r == '\r' || *r == '\n'); r++);
					break;
				}
				t = r + 2;
			}
		}
		if (tpMailItem != NULL) {
			//Body copy
			if ((t - p) > 0) {
				tpMailItem->Body = (char *)mem_alloc(sizeof(char) * (t - p + 1));
				if (tpMailItem->Body != NULL) {
					for (s = tpMailItem->Body; p < t; p++, s++) {
						*s = *p;
					}
					*s = '\0';
				}
			}
			if (tpMailItem->Body == NULL && tpMailItem->MailStatus < ICON_SENTMAIL) {
				if (tpMailItem->Download == TRUE) {
					tpMailItem->Body = (char *)mem_alloc(sizeof(char));
					if (tpMailItem->Body != NULL) {
						*tpMailItem->Body = '\0';
					}
				} else {
					tpMailItem->MailStatus = ICON_NON;
					if (tpMailItem->Mark != ICON_DOWN && tpMailItem->Mark != ICON_DEL && tpMailItem->Mark != ICON_SEND) {
						tpMailItem->Mark = ICON_NON;
					}
					tpMailItem->Download = FALSE;
				}
			}
		}
		p = r;
		i++;
	}

#ifdef _NOFILEMAP
	mem_free(&FileBuf);
#else	// _NOFILEMAP
	UnmapViewOfFile(FileBuf);
	CloseHandle(hMapFile);
#ifndef _WCE_OLD
	CloseHandle(hFile);
#endif	// _WCE_OLD
#endif	// _NOFILEMAP
	return TRUE;
}


/*
 * file_write - �}���`�o�C�g�ɕϊ����ĕۑ�
 */
BOOL file_write(HANDLE hFile, char *buf, int len)
{
	DWORD ret;

	if (len == 0) return TRUE;
	return WriteFile(hFile, buf, len, &ret, NULL);
}

/*
 * file_write_ascii - �}���`�o�C�g�ɕϊ����ĕۑ�
 */
BOOL file_write_ascii(HANDLE hFile, TCHAR *buf, int len)
{
	DWORD ret;
#ifdef UNICODE
	char *str;
	int clen;

	if (len == 0) return TRUE;

	clen = tchar_to_char_size(buf);
	str = (char *)mem_alloc(clen + 1);
	if (str == NULL) {
		return FALSE;
	}
	tchar_to_char(buf, str, clen);
	if (WriteFile(hFile, str, clen - 1, &ret, NULL) == FALSE) {
		mem_free(&str);
		return FALSE;
	}
	mem_free(&str);
	return TRUE;

#else
	if (len == 0) return TRUE;
	return WriteFile(hFile, buf, len, &ret, NULL);
#endif
}

/*
 * file_save - �t�@�C���̕ۑ�
 */
BOOL file_save(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;

	// �t�@�C���ɕۑ�
	if (FileName == NULL) {
		*path = TEXT('\0');
	} else {
		lstrcpy(path, FileName);
	}
	if (filename_select(hWnd, path, Ext, NULL, FALSE) == FALSE) {
		return TRUE;
	}

	// �ۑ�����t�@�C�����J��
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}
	SwitchCursor(FALSE);
	if (WriteFile(hFile, buf, len, &ret, NULL) == FALSE) {
		CloseHandle(hFile);
		SwitchCursor(TRUE);
		return FALSE;
	}
	SwitchCursor(TRUE);
	CloseHandle(hFile);
	return TRUE;
}

/*
 * file_save_exec - �t�@�C����ۑ����Ď��s
 */
BOOL file_save_exec(HWND hWnd, TCHAR *FileName, char *buf, int len)
{
	WIN32_FIND_DATA FindData;
	SHELLEXECUTEINFO sei;
	HANDLE hFindFile;
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR tmp_path[BUF_SIZE];
	DWORD ret;

	if (FileName == NULL) {
		return FALSE;
	}
	SwitchCursor(FALSE);

	// �f�B���N�g���̌���
	wsprintf(path, TEXT("%s%s"), DataDir, op.AttachPath);
	hFindFile = FindFirstFile(path, &FindData);
	FindClose(hFindFile);
	if (hFindFile == INVALID_HANDLE_VALUE || !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		CreateDirectory(path, NULL);
	}

	wsprintf(path, TEXT("%s%s\\%s"), DataDir, op.AttachPath, FileName);
	if (op.AttachDelete == 0) {
		// �ꎞ�t�@�C���ɕۑ�
		wsprintf(tmp_path, TEXT("%s_%lX.tmp"), DataDir, (long)buf);
		hFile = CreateFile(tmp_path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == NULL || hFile == (HANDLE)-1) {
			SwitchCursor(TRUE);
			return FALSE;
		}
		if (WriteFile(hFile, buf, len, &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			DeleteFile(tmp_path);
			SwitchCursor(TRUE);
			return FALSE;
		}
		FlushFileBuffers(hFile);
		CloseHandle(hFile);

		// �t�@�C���̈ړ�
		if (CopyFile(tmp_path, path, FALSE) == FALSE) {
			DeleteFile(tmp_path);
			SwitchCursor(TRUE);
			return FALSE;
		}
		DeleteFile(tmp_path);
	} else {
		// �t�@�C���ɕۑ�
		hFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == NULL || hFile == (HANDLE)-1) {
			SwitchCursor(TRUE);
			return FALSE;
		}
		if (WriteFile(hFile, buf, len, &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			SwitchCursor(TRUE);
			return FALSE;
		}
		FlushFileBuffers(hFile);
		CloseHandle(hFile);
	}
	SwitchCursor(TRUE);

	// ���s
	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(sei);
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = path;
	sei.lpParameters = NULL;
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = hInst;
	ShellExecuteEx(&sei);
	return TRUE;
}

/*
 * file_save_mailbox - ���[���{�b�N�X���̃��[����ۑ�
 */
BOOL file_save_mailbox(TCHAR *FileName, MAILBOX *tpMailBox, int SaveFlag)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	char *tmp, *p;
	///////////// MRP /////////////////////
	TCHAR pathBackup[BUF_SIZE];
	///////////// --- /////////////////////

	int len = 0;
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif	// _WIN32_WCE
	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	if (SaveFlag == 0) {
		//When it does not retain, deletion
		DeleteFile(path);
		return TRUE;
	}

#ifndef DIV_SAVE
	//Mail character string compilation
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		len += item_to_string_size(*(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);
	}
	p = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	*p = '\0';
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		p = item_to_string(p, *(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);
	}
#endif	// DIV_SAVE

	///////////// MRP /////////////////////
#ifdef UNICODE
   wcscpy(pathBackup, path);
   wcscat(pathBackup, TEXT(".bak"));
#else
   strcpy(pathBackup, path);
   strcat(pathBackup, TEXT(".bak"));
#endif
	DeleteFile(pathBackup);
	MoveFile(path, pathBackup); // Create the backup file.
	///////////// --- /////////////////////

	//Retention
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
#ifndef DIV_SAVE
		mem_free(&tmp);
#endif	// DIV_SAVE
		return FALSE;
	}
#ifndef DIV_SAVE
	if (file_write(hFile, tmp, len) == FALSE) {
		mem_free(&tmp);
		return FALSE;
	}
	mem_free(&tmp);
#else	// DIV_SAVE
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		len = item_to_string_size(*(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);

		p = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
		if (tmp == NULL) {
			CloseHandle(hFile);
			return FALSE;
		}
		item_to_string(tmp, *(tpMailBox->tpMailItem + i), (SaveFlag == 1) ? FALSE : TRUE);
		if (file_write(hFile, tmp, len) == FALSE) {
			mem_free(&tmp);
			CloseHandle(hFile);
			return FALSE;
		}
		mem_free(&tmp);
	}
#endif	// DIV_SAVE
	CloseHandle(hFile);

	///////////// MRP /////////////////////
	DeleteFile(pathBackup);
	///////////// --- /////////////////////
	return TRUE;
}

/*
 * file_save_address_item - �A�h���X����1���ۑ�
 */
static BOOL file_save_address_item(HANDLE hFile, MAILITEM *tpMailItem)
{
	TCHAR *tmp;
	int len = 0;

	if (tpMailItem->To != NULL) {
		len += lstrlen(tpMailItem->To);
	}
	if (tpMailItem->Subject != NULL && *tpMailItem->Subject != TEXT('\0')) {
		len += 1;	// TAB
		len += lstrlen(tpMailItem->Subject);
	}
	len += 2;		// CRLF

	tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	if (tpMailItem->Subject != NULL && *tpMailItem->Subject != TEXT('\0')) {
		str_join_t(tmp, tpMailItem->To, TEXT("\t"), tpMailItem->Subject, TEXT("\r\n"), (TCHAR *)-1);
	} else {
		str_join_t(tmp, tpMailItem->To, TEXT("\r\n"), (TCHAR *)-1);
	}

	if (file_write_ascii(hFile, tmp, len) == FALSE) {
		mem_free(&tmp);
		return FALSE;
	}
	mem_free(&tmp);
	return TRUE;
}

/*
 * file_save_address_book - �A�h���X�����t�@�C���ɕۑ�
 */
BOOL file_save_address_book(TCHAR *FileName, MAILBOX *tpMailBox)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	int i;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	//The file which it retains is opened
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}

	//The character string which it retains conversion to copy
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		if (file_save_address_item(hFile, *(tpMailBox->tpMailItem + i)) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	return TRUE;
}

/*
 * file_read_address_book - of pause of log The address register is read from the file
 */
int file_read_address_book(TCHAR *FileName, MAILBOX *tpMailBox)
{
	MAILITEM *tpMailItem;
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

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

#ifdef _WIN32_WCE_PPC
	///////////// MRP /////////////////////
	if (op.UsePOOMAddressBook == 1)
	{
		UpdateAddressBook(path);
	}
	///////////// --- /////////////////////
#endif

	FileSize = file_get_size(path);
	if (FileSize < 0) {
		return 0;
	}
	if (FileSize == 0) {
		return 1;
	}
	FileBuf = file_read(path, FileSize);
	if (FileBuf == NULL) {
		return -1;
	}

#ifdef UNICODE
	//UNICODE
	Len = char_to_tchar_size(FileBuf);
	MemFile = AllocBuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (Len + 1));
	if (MemFile == NULL) {
		mem_free(&FileBuf);
		return -1;
	}
	char_to_tchar(FileBuf, MemFile, Len);
	FileSize = Len;
#else	// UNICODE
	MemFile = (TCHAR *)FileBuf;
#endif	// UNICODE

	//Count
	for (LineCnt = 0, p = MemFile; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\n')) {
			LineCnt++;
		}
	}
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = LineCnt;
	tpMailBox->tpMailItem = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * tpMailBox->MailItemCnt);
	if (tpMailBox->tpMailItem == NULL) {
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		mem_free(&FileBuf);
		mem_free(&AllocBuf);
		return -1;
	}
	i = 0;
	p = MemFile;
	while (FileSize > p - MemFile && *p != TEXT('\0')) {
		tpMailItem = *(tpMailBox->tpMailItem + i) = (MAILITEM *)mem_calloc(sizeof(MAILITEM));

		//of the number of lines Mail address
		for (r = p; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if (tpMailItem != NULL) {
			tpMailItem->To = (TCHAR *)mem_alloc(sizeof(TCHAR) * (r - p + 1));
			if (tpMailItem->To != NULL) {
				for (s = tpMailItem->To; p < r; p++, s++) {
					*s = *p;
				}
				*s = '\0';
			}
		}
		if (*r == TEXT('\t')) r++;

		//Comment
		for (p = r; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if (tpMailItem != NULL) {
			tpMailItem->Subject = (TCHAR *)mem_alloc(sizeof(TCHAR) * (r - p + 1));
			if (tpMailItem->Subject != NULL) {
				for (s = tpMailItem->Subject; p < r; p++, s++) {
					*s = *p;
				}
				*s = '\0';
			}
		}
		for (; *p != TEXT('\0') && *p != TEXT('\r') && *p != TEXT('\n'); p++);
		for (; *p == TEXT('\r') || *p == TEXT('\n'); p++);
		i++;
	}
	mem_free(&FileBuf);
	mem_free(&AllocBuf);
	return 1;
}

/* End of source */