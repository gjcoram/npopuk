/*
 * nPOP
 *
 * File.c
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
#include "mime.h"
#ifdef _WIN32_WCE_PPC
#include "SelectFile.h"
#endif
#ifdef _WIN32_WCE
///////////// MRP /////////////////////
#include "ppcpoom.h"
///////////// --- /////////////////////
#endif

/* Define */
#define MBOX_DELIMITER		"\r\nFrom "
#define ENCRYPT_PREAMBLE	"NPOPUK_ENCRYPT 0\r\n"

/* Global Variables */
HANDLE hLogFile;
BOOL gLogOpened = FALSE;

extern OPTION op;
extern HINSTANCE hInst;  // Local copy of hInstance
extern TCHAR *AppDir;
extern TCHAR *DataDir;
extern MAILBOX *MailBox;
extern BOOL SaveBoxesLoaded;
extern int SelBox, vSelBox, RecvBox;

/**************************************************************************
	Local Function Prototypes
**************************************************************************/
BOOL log_open(void);
void log_close(void);
static int file_get_mail_count(char *buf, long Size, int MboxFormat);
static BOOL file_save_address_item(HANDLE hFile, ADDRESSITEM *tpAddrItem);
static UINT CALLBACK OpenFileHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
 * log_open
 *
 * The first time through (gLogOpened==FALSE), we use CREATE_ALWAYS to start
 * with a clean, empty file.  After that, if some error causes the log file
 * to close, it is reopened without truncation.
 */
BOOL log_open(void)
{
	TCHAR path[BUF_SIZE];
	DWORD create = CREATE_ALWAYS;
	if (gLogOpened) create = OPEN_ALWAYS;

	// create the name
	if (DataDir != NULL) {
		wsprintf(path, TEXT("%s%s"), DataDir, LOG_FILE);
	} else {
		wsprintf(path, TEXT("%s%s"), AppDir, LOG_FILE);
	}

	// create or open the file
	hLogFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, 0, create, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
	if (hLogFile == NULL || hLogFile == (HANDLE)-1) {
		return FALSE;
	}
	SetFilePointer(hLogFile, 0, NULL, FILE_END);
	gLogOpened = TRUE;
	return TRUE;
}

void log_close(void)
{
	if (hLogFile != NULL && hLogFile != (HANDLE)-1) {
		CloseHandle(hLogFile);
		hLogFile = NULL;
	}
	return;
}

void log_flush(void)
{
#ifndef _WIN32_WCE
	BOOL ret;
	ret = FlushFileBuffers(hLogFile);
	if (ret == FALSE)
#endif
		log_close();
}

TCHAR *log_read(void)
{
	TCHAR *buf = NULL;
	TCHAR path[BUF_SIZE];
	long file_size;

	// create the name
	if (DataDir != NULL) {
		wsprintf(path, TEXT("%s%s"), DataDir, LOG_FILE);
	} else {
		wsprintf(path, TEXT("%s%s"), AppDir, LOG_FILE);
	}

	// close file -- will be re-opened for append, if needed
	log_close();

	file_size = file_get_size(path);
	if (file_size > 0) {
		char *cbuf;
		cbuf = file_read(path, file_size);
		if (cbuf != NULL) {
			buf = alloc_char_to_tchar(cbuf);
			mem_free(&cbuf);
		}
	}
	return buf;
}

#ifdef UNICODE
/*
 * log_save - write string to log file.  buf assumed to end with \r\n
 */
BOOL log_save(TCHAR *buf)
{
	char *ascii;
	BOOL ret;

	if (hLogFile == NULL || hLogFile == (HANDLE)-1) {
		if (log_open() == FALSE) {
			return FALSE;
		}
	}

	ascii = alloc_tchar_to_char(buf);
	ret = WriteFile(hLogFile, ascii, strlen(ascii), &ret, NULL);

#ifdef FLUSH_ON_EVERY_WRITE
	if (ret != FALSE)
		ret = FlushFileBuffers(hLogFile);
#endif

	if (ret == FALSE)
		log_close();
	mem_free(&ascii);
	return ret;
}
#endif

/*
 * log_save - write ascii string to log file.  buf assumed to end with \r\n
 */
BOOL log_save_a(char *ascii)
{
	BOOL ret;

	if (hLogFile == NULL || hLogFile == (HANDLE)-1) {
		if (log_open() == FALSE) {
			return FALSE;
		}
	}

	ret = WriteFile(hLogFile, ascii, strlen(ascii), &ret, NULL);
#ifdef FLUSH_ON_EVERY_WRITE
	if (ret != FALSE)
		ret = FlushFileBuffers(hLogFile);
#endif
	if (ret == FALSE)
		log_close();

	return ret;
}

/*
 * log_header - Write a header to the log.
 */
BOOL log_header(char *buf)
{
#define LOG_SEP				"\r\n-------------------------------- "
	char fDay[BUF_SIZE];
	char fTime[BUF_SIZE];
	char *p;
	BOOL ret;

#ifdef _WIN32_WCE
	TCHAR tDay[BUF_SIZE];
	TCHAR tTime[BUF_SIZE];
	if (GetDateFormat(0, 0, NULL, NULL, tDay, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
	if (GetTimeFormat(0, 0, NULL, NULL, tTime, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
	tchar_to_char(tDay,  fDay,  BUF_SIZE);
	tchar_to_char(tTime, fTime, BUF_SIZE);
#else
	if (GetDateFormatA(0, 0, NULL, NULL, fDay, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
	if (GetTimeFormatA(0, 0, NULL, NULL, fTime, BUF_SIZE - 1) == 0) {
		return FALSE;
	}
#endif
	p = mem_alloc(sizeof(char) * (strlen(LOG_SEP) + strlen(fDay) + 1 + strlen(fTime) + 2 + strlen(buf) + 2 + 2));
	if (p == NULL) {
		return FALSE;
	}
	str_join(p, LOG_SEP, fDay, " ", fTime, " (", buf, ")\r\n", (char *)-1);
	ret = log_save_a(p);
	mem_free(&p);
	return ret;
}

/*
 * dir_check - ディレクトリかどうかチェック
 */
BOOL dir_check(const TCHAR *path)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	int len;

	if (path == NULL) {
		return FALSE;
	} else {
		len = lstrlen(path);
	}

#ifndef _WIN32_WCE
	// FILE_OPEN_MULTI may return C:\ plus a set of filenames
	// FindFirstFile does not accept a trailing '\' -- nor "C:"
	if (len > 1 && path[1] == TEXT(':') &&
		(len == 2 || (len == 3 && path[2] == TEXT('\\')))) {
		TCHAR test[5];
		if (len == 2) {
			wsprintf(test, TEXT("%s\\*"), path);
		} else {
			wsprintf(test, TEXT("%s*"), path);
		}
		if ((hFindFile = FindFirstFile(test, &FindData)) == INVALID_HANDLE_VALUE) {
			return FALSE;
		}
		FindClose(hFindFile);
		return TRUE;
	}
#endif

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
 * dir_create
 */
BOOL dir_create(TCHAR *path)
{
	if (dir_check(path)) {
		return TRUE;
	}
	if (CreateDirectory(path, NULL)) {
		return TRUE;
	}
	return FALSE;
}

/*
 * dir_delete - ディレクトリ内のファイルを削除
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
 * trunc_to_dirname - truncate filename to directory part only
 */
void trunc_to_dirname(TCHAR *fname)
{
	TCHAR *p, *r;
	for (p = r = fname; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
			continue;
		}
#endif
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			r = p;
		}
	}
	*(r++) = TEXT('\\');
	*r = TEXT('\0');
}

/*
 * trunc_to_parent_dir - truncate at parent dir of fname
 * input fname ends with \ or / if it is a dir (see trunc_to_dirname above)
 */
BOOL trunc_to_parent_dir(TCHAR *fname)
{
	TCHAR *p, *r, *s;
	BOOL ret = TRUE;
	r = s = NULL;
	for (p = fname; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
			continue;
		}
#endif
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			s = r;
			r = p;
		}
	}
	if (s == NULL) {
		s = fname;
		if (*s && *(s+1) == TEXT(':'))
			s += 2;
		ret = FALSE;
	}
	*(s++) = TEXT('\\');
	*s = TEXT('\0');
	return ret;
}

/*
 * filename_conv - ファイル名にできない文字を _ に変換する
 */
void filename_conv(TCHAR *buf)
{
	TCHAR *p = buf, *r;

	while (*p != TEXT('\0')) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			// 2バイトコード
			p += 2;
			continue;
		}
#endif
		// 空白の除去
		if (*p == TEXT(' ') || *p == TEXT('\t')) {
			r = p + 1;
			for (; *r == TEXT(' ') || *r == TEXT('\t'); r++);
			if (p + 1 != r) {
				lstrcpy(p + 1, r);
			}
		}
		// ファイル名にできない文字は指定の文字に変換
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
BOOL filename_select(HWND hWnd, TCHAR *ret, TCHAR *DefExt, TCHAR *filter, int Action, TCHAR **opptr)
{
#ifdef _WIN32_WCE_PPC
	TCHAR path[BUF_SIZE];

	lstrcpy(path, ret);
	return SelectFile(hWnd, hInst, Action, path, ret, opptr);
#else
	OPENFILENAME of;
	TCHAR path[MULTI_BUF_SIZE];
#ifndef _WIN32_WCE
	TCHAR CurDir[BUF_SIZE];
#endif
	TCHAR *ph, *qh;
	BOOL bret = TRUE, is_open = (Action == FILE_OPEN_SINGLE || Action == FILE_OPEN_MULTI),
		is_dir = (Action == FILE_CHOOSE_DIR || Action == FILE_CHOOSE_BACKDIR);

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
	if (is_open) {
		of.lpstrTitle = STR_TITLE_OPEN;
		*path = TEXT('\0');
	} else {
		of.lpstrTitle = STR_TITLE_SAVE;
		lstrcpy(path, ret);
	}
	of.lpstrFile = path;
	of.nMaxFile = BUF_SIZE - 1;
	of.lpstrDefExt = DefExt;

	// check if directory exists
	if (opptr != NULL && *opptr != NULL && **opptr != TEXT('\0') && dir_check(*opptr)) {
		// yes, use it
		of.lpstrInitialDir = *opptr;
	} else if (is_open == FALSE && Action != FILE_CHOOSE_BACKDIR) {
		of.lpstrInitialDir = DataDir;
	} else if (Action == FILE_CHOOSE_BACKDIR) {
		ParanoidMessageBox(hWnd, STR_WARN_BACKUPDIR, WINDOW_TITLE, MB_ICONEXCLAMATION | MB_OK);
	} // else is_open or choose dir (backup): just let Windows determine the directory

	of.Flags = OFN_HIDEREADONLY;
	if (is_dir == FALSE) {
		of.Flags |= OFN_OVERWRITEPROMPT;
	}
#ifndef _WIN32_WCE
	// GJC allow multiselect
	if (Action == FILE_OPEN_MULTI) {
		of.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		of.lpstrFile = ret;
		of.nMaxFile = MULTI_BUF_SIZE - 1;
	}

	// save (then restore) current working directory, GetOpen/SaveFileName changes it
	GetCurrentDirectory(BUF_SIZE-1, CurDir);
#endif

	//File selective dialogue is indicated
	if (is_open) {
		of.Flags |= OFN_FILEMUSTEXIST;
		if (GetOpenFileName((LPOPENFILENAME)&of) == FALSE) {
			if (Action == FILE_OPEN_MULTI && lstrcmp(path, ret) != 0) {
				ErrorMessage(hWnd, STR_ERR_TOOMANYFILES); 
			}
			bret = FALSE;
		}
	} else {
		if (GetSaveFileName((LPOPENFILENAME)&of) == FALSE) {
			bret = FALSE;
		} else if (is_dir) {
			ph = qh = path;
			while (*ph != TEXT('\0')) {
				if (*ph == TEXT('\\')) {
					qh = ph + 1;
				}
				ph++;
			}
			*qh = TEXT('\0');
		}
	}

#ifndef _WIN32_WCE
	// restore working directory
	SetCurrentDirectory(CurDir);
#endif

	if (bret == TRUE) {
		if (opptr) {
			// wastes a bit of space (length of file name part) when only
			// one file returned, but simplifies the coding significantly.
			ph = alloc_copy_t(of.lpstrFile);
			ph[ of.nFileOffset - 1 ] = TEXT('\0');
			mem_free(opptr);
			*opptr = ph;
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
		if (Action != FILE_OPEN_MULTI) {
			lstrcpy(ret, path);
		}
#endif	// _WIN32_WCE
	}
	return bret;
#endif	// _WIN32_WCE_PPC
}

/*
 * file_get_size - ファイルのサイズを取得する
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
		if (FindData.nFileSizeHigh != 0 || (FindData.nFileSizeLow >> 31) != 0 ) {
			return -2;
		}
		// ディレクトリではない場合はサイズを返す
		return (long)FindData.nFileSizeLow;
	}
	return -1;
}

/*
 * file_get_mail_count - メール一覧の文字列からメールの数を取得
 */
static int file_get_mail_count(char *buf, long Size, int MboxFormat)
{
	char *p, *r, *t, *max;
	int ret = 0, len;

	if (MboxFormat) {
		ret = 1; /* won't match first "From " because match string includes prior "\r\n" */
		len = tstrlen(MBOX_DELIMITER);
		max = buf + Size - len;
		for (p = buf; p < max; p++) {
			if (str_cmp_n(p, MBOX_DELIMITER, len) == 0) {
				ret++;
				p += len;
			}
		}
	} else {
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
		hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}
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
 * file_read_select - The file is opened (for insertion in Edit window)
 */
BOOL file_read_select(HWND hWnd, TCHAR **buf)
{
	TCHAR path[BUF_SIZE];
	TCHAR msg[MSG_SIZE];
	unsigned char *cBuf;
	long FileSize;

	*buf = NULL;

	//Select file to import
#ifdef _WIN32_WCE
	lstrcpy(path, TEXT("*.txt"));
#else
	*path = TEXT('\0');
#endif
	if (filename_select(hWnd, path, TEXT("txt"), STR_TEXT_FILTER, FILE_OPEN_SINGLE, &op.SavedOpenDir) == FALSE) {
		return TRUE;
	}

	//Acquisition size of file
	FileSize = file_get_size(path);
	if (FileSize <= 0) {
		if (FileSize == -2) {
			wsprintf(msg, STR_ERR_FILE_TOO_LARGE, path);
			ErrorMessage(hWnd, msg);
		} else if (FileSize == -1) {
			wsprintf(msg, STR_ERR_FILEEXIST, path);
			ErrorMessage(hWnd, msg);
		} // else if (FileSize == 0), empty file, don't need to read it
		return TRUE; // so no error message will be shown by calling function
	}

	//Conversion to
	SwitchCursor(FALSE);
	cBuf = file_read(path, FileSize);
	if (cBuf == NULL) {
		SwitchCursor(TRUE);
		return FALSE;
	}
	// Fix bare \r or \n in file
	FixCRLF(&cBuf);

#ifdef UNICODE
	//UNICODE which reads the file
	if( *cBuf == 0xEF && *(cBuf+1) == 0xBB && *(cBuf+2) == 0xBF ) {
		// The file starts with the UTF-8 representation
		// of the BOM U+FEFF: 0xEF 0xBB 0xFB
		*buf = alloc_char_to_tchar(cBuf+3);

	} else if ( is_utf8(cBuf) ) {
		// it's valid UTF-8 encoding (could be plain ASCII)
		*buf = alloc_char_to_tchar(cBuf);

	} else {
		CP_int = CP_ACP; // assume default codepage
		*buf = alloc_char_to_tchar(cBuf);
		CP_int = CP_UTF8; // restore to UTF8
	}
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
 * file_savebox_convert (GJC)
 */
BOOL file_savebox_convert(TCHAR *NewFileName)
{
	TCHAR path[BUF_SIZE], newpath[BUF_SIZE];
	BOOL ret;

	str_join_t(path, DataDir, SAVEBOX_FILE, (TCHAR *)-1);

	if(file_get_size(path) < 0) {
		ret = FALSE;
	} else {
		str_join_t(newpath, DataDir, NewFileName, (TCHAR *)-1);
		CopyFile(path, newpath, FALSE);

		ret = TRUE;
	}
	return ret;
}

/*
 * file_copy_to_datadir - copy file to datadir
 */
BOOL file_copy_to_datadir(HWND hWnd, TCHAR *Source, TCHAR *FileName)
{
	TCHAR path[BUF_SIZE];
	TCHAR pathBackup[BUF_SIZE];
	TCHAR msg[MSG_SIZE];
	long fsize;

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

#ifdef UNICODE
   wcscpy(pathBackup, path);
   wcscat(pathBackup, TEXT(".bak"));
#else
   strcpy_s(pathBackup, BUF_SIZE-5, path);
   strcat_s(pathBackup, BUF_SIZE, TEXT(".bak"));
#endif

   // if backup exists, file_read_mailbox will replace the file with the backup!
   if (file_get_size(pathBackup) != -1) {
		wsprintf(msg, STR_Q_DELSBOXFILE, FileName, TEXT(".bak"));
		if (MessageBox(hWnd, msg, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
			return FALSE;
		} else {
			DeleteFile(pathBackup);
		}
	}

	if (lstrcmpi(Source, path) == 0) {
		// what if one uses 8.3 filenames?
		return TRUE;
	}
	fsize = file_get_size(path);
	if (fsize != -1 && fsize != 0) {
		wsprintf(msg, STR_Q_DELSBOXFILE, FileName, TEXT(""));
		if (MessageBox(hWnd, msg, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
			return FALSE;
		} else {
			DeleteFile(path);
		}
	}

	return CopyFile(Source, path, TRUE);
}

/*
 * file_read_mailbox - ファイルからメールアイテムの作成
 */
BOOL file_read_mailbox(TCHAR *FileName, MAILBOX *tpMailBox, BOOL Import, BOOL CheckDup)
{
	MAILITEM *tpMailItem;
#ifndef _NOFILEMAP
	HANDLE hFile;
	HANDLE hMapFile;
#endif
	TCHAR path[BUF_SIZE];
	char *p;
	///////////// MRP /////////////////////
	TCHAR pathBackup[BUF_SIZE];
	///////////// --- /////////////////////
	char *FileBuf, *MsgStart;
	long FileSize;
	int i, cnt, len;
	int MboxFormat = 0, encrypted = 0;

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	///////////// MRP /////////////////////
#ifdef UNICODE
	wcscpy(pathBackup, path);
	wcscat(pathBackup, TEXT(".bak"));
#else
	strcpy_s(pathBackup, BUF_SIZE-5, path);
	strcat_s(pathBackup, BUF_SIZE, TEXT(".bak"));
#endif

	FileSize = file_get_size(pathBackup);
	if (FileSize == 0) {
		DeleteFile(pathBackup); // GJC
	} else if (FileSize == -2) {
		TCHAR msg[MSG_SIZE];
		tpMailBox->Loaded = FALSE;
		wsprintf(msg, STR_ERR_FILE_TOO_LARGE, pathBackup);
		ErrorMessage(NULL, msg);
		return FALSE;
	} else if (FileSize != -1) { // Backup File exists
		DeleteFile(path);  // delete the current file
		MoveFile(pathBackup, path); // replace the the current file with the backup file.
	}
	///////////// --- /////////////////////

	tpMailBox->DiskSize = FileSize = file_get_size(path);
	if (FileSize == -2) {
		tpMailBox->Loaded = FALSE;
		if (op.SocLog > 9) {
			int pos = lstrlen(path);
			if (pos > 236) pos = 236; // 242 = BUF_SIZE - strlen(" too large!\r\n") - 1
			wsprintf(path+pos, TEXT(" too large!\r\n"));
			log_save(path);
		}
		return FALSE;
	} else if (FileSize <= 0) {
		tpMailBox->Loaded = TRUE;
		if (op.SocLog > 9) {
			int pos = lstrlen(path);
			if (pos > 236) pos = 236; // 236 = BUF_SIZE - strlen(" loaded but empty\r\n") - 1
			wsprintf(path+pos, TEXT(" loaded but empty\r\n"));
			log_save(path);
		}
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

	if (str_cmp_n(FileBuf, "From NPOPUK", 11) == 0) {
		MboxFormat = 2;
	} else if (str_cmp_n(FileBuf, "From ", 5) == 0) {
		MboxFormat = 1;
	}
	tpMailBox->WasMbox = (MboxFormat == 0) ? FALSE : TRUE;
	MsgStart = FileBuf;
	len = tstrlen(ENCRYPT_PREAMBLE);
	if (FileSize > len && str_cmp_n(FileBuf, ENCRYPT_PREAMBLE, len) == 0) {
		encrypted = 1;
		MsgStart += len;
		FileSize -= len;
	}
	cnt = file_get_mail_count(MsgStart, FileSize, MboxFormat);
	// check for confused file format: mbox-format mailbox was opened&saved by
	// previous version of npop/npopuk
	if (cnt == 1 && MboxFormat == 0) {
		int cnt2 = file_get_mail_count(MsgStart, FileSize, 1);
		if (cnt2 > 1) {
			char *s = GetHeaderStringPoint(MsgStart, HEAD_X_STATUS);
			if (s != NULL) {
				int rev = atoi(s)/100000;
				if (rev == 1) {
					TCHAR *title, msg[MSG_SIZE];
					title = tpMailBox->Name;
					if (title == NULL || *title == TEXT('\0')) {
						title = STR_MAILBOX_NONAME;
					}
					wsprintf(msg, STR_Q_LOADASMBOX, FileName, cnt2);
					if (MessageBox(NULL, msg, title, MB_ICONQUESTION | MB_YESNO) == IDYES) {
						cnt = cnt2;
						MboxFormat = 1;
					}
				}
			}
		}
	}

	//Guaranty memory of mail several parts
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = cnt;
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
	tpMailBox->FlagCount = 0;

	i = 0;
	len = 7; // = tstrlen(MBOX_DELIMITER);
	p = MsgStart;
	while (FileSize > p - MsgStart && *p != '\0') {
		unsigned char *q, *r, *s, *t, *max;
		int code;
		if (encrypted) {
			// decrypt the headers
			r = GetBodyPointa(p);
			if (r == NULL) break;
			q = (char *)mem_alloc(sizeof(char) * (r - p + 1));
			if (q == NULL) break;
			rot13_cpy(q, p, r);
			*(q+(r-p)) = '\0';
		} else {
			q = p;
		}

		code = 0;
		s = GetHeaderStringPoint(q, HEAD_X_STATUS);
		if (s != NULL) {
			code = atoi(s);
		}

		if (code >= STATUS_REVISION_NPOPUK3 || (code == 0 && Import)) {
			// saved in new "wire-form" format, or importing from non-nPOPuk MBOX format
			int m2i = (Import) ? MAIL2ITEM_IMPORT : MAIL2ITEM_WIRE;
			char *buf = NULL;

			tpMailItem = *(tpMailBox->tpMailItem + i) = (MAILITEM *)mem_calloc(sizeof(MAILITEM));

			if (s != NULL) {
				if (code > 0) {
					item_set_flags(tpMailItem, tpMailBox, code);
				}
				item_get_npop_headers(q, tpMailItem, tpMailBox);
				p += (s-q);
				while (*p != '\0' && *p != '\r' && *p != '\n') {
					p++;
				}
				while (*p == '\r' || *p == '\n') {
					p++;
				}
			}

			if (encrypted) {
				mem_free(&q);
			}

			// Find end of message
			if (MboxFormat) {
				max = MsgStart + FileSize - len;
				for (t = r = p; r < max; r++) {
					if (str_cmp_n(r, MBOX_DELIMITER, len) == 0) {
						t = r;
						r += 2;
						break;
					}
				}
				if ( r >= max ) {
					t = r = max + len;
				}
			} else {
				for (t = r = p; *r != '\0'; r++) {
					if (*r == '\r' && *(r + 1) == '\n') {
						if (*t == '.' && (r - t) == 1) {
							t -= 2;
							for (; FileSize > r - MsgStart && (*r == '\r' || *r == '\n'); r++);
							break;
						}
						t = r + 2;
					}
				}
			}
			if (t > p) {
				unsigned long tpdiff = t - p;
				if (tpMailItem->Multipart == MULTIPART_ATTACH && tpMailItem->AttachSize > tpdiff) {
					tpMailItem->AttachSize -= tpdiff; // - strlen(body)
				}
				buf = (char *)mem_alloc(sizeof(char) * (t - p + 1));
				if (buf != NULL) {
					for (s = buf; p < t; p++, s++) {
						*s = *p;
					}
					*s = '\0';
					if (encrypted) {
						rot13(buf, s);
					}
				}
			}
			CP_int = CP_UTF8;
			item_mail_to_item(tpMailItem, &buf, -1, m2i, 0, tpMailBox);
			p = r;

		} else {
			// old style mailitem
			int slashr = 0;
			if (lstrcmpi(op.Codepage, TEXT("CP_ACP")) == 0) {
				// nPOPuk 2.16 and earlier did all MultiByteToWideChar conversions using CP_ACP.
				CP_int = CP_ACP;
			}
			tpMailItem = *(tpMailBox->tpMailItem + i) = item_string_to_item(tpMailBox, q, Import);
			CP_int = CP_UTF8; // restore to UTF8

			if (encrypted) {
				mem_free(&q);
			}
			if (tpMailItem != NULL && tpMailItem->MailStatus == ICON_MAIL) {
				tpMailBox->UnreadCnt++;
			}

			// Body position Position of end of mail acquisition
			if (MboxFormat) {
				tpMailItem->HasHeader = 1;
				// r = GetHeaderStringPoint(p, HEAD_X_STATUS); // X-Status is always the last header written by nPOPuk
				r = s;
				if (r != NULL && r > p) {
					if (tpMailItem != NULL) tpMailItem->HasHeader = 2;
					while (*r != '\0' && (*(r-1) != '\r' || *r != '\n')) {
						r++;
					}
					if (*r == '\n') {
						r++;
						if (str_cmp_n(r, MBOX_DELIMITER, len) == 0) {
							if (tpMailItem != NULL) tpMailItem->HasHeader = 0;
						} else if (*r == '\r' && *(r+1) == '\n') {
							r += 2;
							if (tpMailItem != NULL) tpMailItem->HasHeader = 0;
						}
						p = r;
					} else if (*r == '\0') {
						break;
					}
				}
			} else {
				// discard all header lines
				if (encrypted) {
					p = r;
				} else {
					p = GetBodyPointa(p);
				}
				if (p == NULL) {
					break;
				}
			}

			// Find end of message
			if (MboxFormat) {
				max = MsgStart + FileSize - len;
				for (t = r = p; r < max; r++) {
					if (str_cmp_n(r, MBOX_DELIMITER, len) == 0) {
						t = r;
						r += 2;
						break;
					}
				}
				if ( r >= max ) {
					t = r = max + len;
				}
			} else {
				for (t = r = p; *r != '\0'; r++) {
					if (*r == '\r' || *r == '\n') {
						if (*t == '.' && (r - t) == 1) {
							t--;
							if (*(t - 1) == '\r') {
								t--;
							}
							for (; FileSize > r - MsgStart && (*r == '\r' || *r == '\n'); r++);
							break;
						}
						if (*r == '\r' && *(r + 1) == '\n') {
							r++;
						} else {
							++slashr;
						}
						t = r + 1;
					}
				}
			}
			if (tpMailItem != NULL) {
				//Body copy
				if (t > p) {
					unsigned long tpdiff = t - p;
					if (tpMailItem->Multipart == MULTIPART_ATTACH && tpMailItem->AttachSize > tpdiff) {
						tpMailItem->AttachSize -= tpdiff; // - strlen(body)
					}
					tpMailItem->Body = (char *)mem_alloc(sizeof(char) * (t - p + 1 + slashr));
					if (tpMailItem->Body != NULL) {
						BOOL all_ascii = TRUE;
						for (s = tpMailItem->Body; p < t && slashr >= 0; p++, s++) {
							if (*p == '\n' && (s == tpMailItem->Body || *(s - 1) != '\r')) {
								*(s++) = '\r';
								--slashr;
							}
							*s = *p;
							if (*s > 127) {
								all_ascii = FALSE;
							}
							if (*p == '\r' && *(p + 1) != '\n') {
								*(s++) = '\n';
								--slashr;
							}
						}
						*s = '\0';

						if (all_ascii == FALSE && lstrcmpi(op.Codepage, TEXT("CP_ACP")) == 0) {
							// convert and unconvert to fix codepage
							TCHAR *wtmp;
							char *tmp;
							CP_int = CP_ACP;
							wtmp = alloc_char_to_tchar(tpMailItem->Body);
							CP_int = CP_UTF8;
							if (wtmp != NULL) {
								tmp = alloc_tchar_to_char(wtmp);
								if (tmp != NULL) {
									mem_free(&tpMailItem->Body);
									tpMailItem->Body = tmp;
								}
								mem_free(&wtmp);
							}
						}

						if (encrypted) {
							rot13(tpMailItem->Body, s);
						}
						if (tpMailItem->HasHeader == 1 || (MboxFormat == 1 && tpMailItem->Encoding != NULL)) {
							// strip duplicate headers and/or convert from foreign MBOX format
							char *newbody;
							int len, header_size;
							BOOL free_t = FALSE;
							if (tpMailItem->HasHeader == 1) {
								header_size = remove_superfluous_headers(tpMailItem->Body, TRUE);
								s = tpMailItem->Body + header_size;
								tpMailItem->HasHeader = 2;
								tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
							} else {
								s = tpMailItem->Body;
								header_size = 0;
							}
							if (tpMailItem->Encoding != NULL) {
								t = MIME_body_decode_transfer(tpMailItem, s);
								if (t != s) {
									free_t = TRUE;
								}
							} else {
								t = s;
							}
							len = tstrlen(t);

							newbody = (char *)mem_alloc(sizeof(char) * (len + header_size + 1));
							if (newbody == NULL) {
								// failed to allocate a smaller buffer, just use Body, which is bigger than necessary
								if (header_size > 0) {
									// shift body backwards (after removing duplicate headers)
									tstrcpy(tpMailItem->Body + header_size, t);
								}
							} else {
								str_cpy_n(newbody, tpMailItem->Body, header_size + 1);
								tstrcpy(newbody + header_size, t);
								mem_free(&tpMailItem->Body);
								tpMailItem->Body = newbody;
							}
							if (free_t == TRUE) {
								mem_free(&t);
							}
						}
					}
				}
				if (tpMailItem->Body == NULL && (tpMailItem->MailStatus < ICON_SENTMAIL || tpMailBox == MailBox + MAILBOX_SEND)) {
					if (tpMailItem->Download == TRUE || tpMailBox == MailBox + MAILBOX_SEND) {
						tpMailItem->Body = (char *)mem_alloc(sizeof(char));
						if (tpMailItem->Body != NULL) {
							*tpMailItem->Body = '\0';
						}
					} else {
						tpMailItem->MailStatus = ICON_NON;
						if (tpMailItem->Mark != ICON_DOWN && tpMailItem->Mark != ICON_DEL && tpMailItem->Mark != ICON_SEND) {
							tpMailItem->Mark = ICON_NON;
						}
					}
				}
			}
			p = r;
		}
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
	tpMailBox->Loaded = TRUE;
	if (CheckDup) {
		// check for duplicate messages created by BlindAppend
		BOOL do_resize = FALSE;
		for (i = tpMailBox->MailItemCnt -1; i > 0; i--) {
			tpMailItem = *(tpMailBox->tpMailItem + i);
			if (tpMailItem != NULL) {
				TCHAR *mid = tpMailItem->MessageID;
				if (mid != NULL) {
					int j = item_find_thread(tpMailBox, mid, i);
					while (j != -1) {
						MAILITEM *dupItem = *(tpMailBox->tpMailItem + j);
						if (tpMailItem->Download == TRUE || dupItem->Body == NULL) {
							// tpMailItem is complete, so delete dupItem (may also be complete)
							item_free(&dupItem, 1);
							*(tpMailBox->tpMailItem + j) = NULL;
						} else if (dupItem->Download == TRUE || tpMailItem->Body == NULL) {
							// tpMailItem is incomplete
							item_free(&tpMailItem, 1);
							*(tpMailBox->tpMailItem + i) = NULL;
							j = -1;
						} else {
							// neither is complete ...
							if (tstrlen(tpMailItem->Body) >= tstrlen(dupItem->Body)) {
								item_free(&dupItem, 1);
								*(tpMailBox->tpMailItem + j) = NULL;
							} else {
								item_free(&tpMailItem, 1);
								*(tpMailBox->tpMailItem + i) = NULL;
								j = -1;
							}
						}
						do_resize = TRUE;
						j = item_find_thread(tpMailBox, mid, j);
					}
				}
			}
		}
		if (do_resize) {
			item_resize_mailbox(tpMailBox, FALSE);
		}
	}
	if (tpMailBox->FlagCount > 0) {
		int box = tpMailBox - MailBox;
		SetMailboxMark(box, 0, FALSE);
	}
	if (op.SocLog > 9) {
		int pos = lstrlen(path);
		if (pos > 242) pos = 242; // 242 = BUF_SIZE - strlen(" was loaded\r\n") - 1
		wsprintf(path+pos, TEXT(" was loaded\r\n"));
		log_save(path);
	}
	return TRUE;
}

/*
 * file_scan_mailbox - load only specified message-ID
 */
MAILITEM *file_scan_mailbox(TCHAR *FileName, char *m_id)
{
	HANDLE hFile;
	DWORD ret;
	MAILITEM *tpMailItem = NULL;
	TCHAR path[BUF_SIZE];
	char FileBuf[MAXSIZE+1];
	char *p, *r;
	long FileSize, FilePtr, ReadSize;
	int MsgStart, MsgEnd, MboxFormat = -1, encrypted = 0;
	int len, hlen = tstrlen(HEAD_MESSAGEID), slen = tstrlen(HEAD_X_STATUS);
	int code = 0, mlen = tstrlen(MBOX_DELIMITER), idlen = tstrlen(m_id);
	BOOL Found = FALSE, InHeader = TRUE;

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);
	FileSize = file_get_size(path);
	if (FileSize <= 0) {
		return NULL;
	}
	FilePtr = 0;

	hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return NULL;
	}

	// read MAXSIZE (32KB) chunks looking for m_id
	while (FilePtr < FileSize) {
		ReadSize = (FileSize - FilePtr < MAXSIZE) ? (FileSize - FilePtr) : MAXSIZE;
		if (ReadFile(hFile, FileBuf, ReadSize, &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return NULL;
		}
		FileBuf[ReadSize] = '\0';
		p = FileBuf;
		if (MboxFormat == -1) {
			if (str_cmp_n(FileBuf, "From ", 5) == 0) {
				MboxFormat = 1;
				while (*p != '\0' && *p != '\r' && *(p+1) != '\n') {
					p++;
				}
			} else {
				MboxFormat = 0;
			}
			len = tstrlen(ENCRYPT_PREAMBLE);
			if (FileSize > len && str_cmp_n(FileBuf, ENCRYPT_PREAMBLE, len) == 0) {
				encrypted = 1;
				while (*p != '\0' && *p != '\r' && *(p+1) != '\n') {
					p++;
				}
			}
			MsgStart = (p - FileBuf) + ((MboxFormat == TRUE) ? 2 : 0);
		}
		if (encrypted) {
			len = ReadSize + (p - FileBuf);
			rot13(p, p + len);
		}
		while (*p != '\0') {
			for (r = p + 1; *r != '\0'; r++) {
				// check for full line
				if (*r == '\r' && *(r+1) == '\n') {
					break;
				}
			}
			if (*r == '\0') {
				// back up to just before the last \r\n
				int rewind = ReadSize - (p - FileBuf);
				FilePtr -= rewind;
				SetFilePointer(hFile, -rewind, NULL, FILE_CURRENT);
				break;
			}
			if (InHeader) {
				if (str_cmp_ni(p+2, HEAD_MESSAGEID, hlen) == 0) {
					p += hlen + 3;
					if (str_cmp_n(p, m_id, idlen) == 0) {
						Found = TRUE;
					}
				} else if (str_cmp_ni(p+2, HEAD_X_STATUS, slen) == 0) {
					// check for new vs old format (code >= 300000)
					p += slen + 3;
					code = atoi(p);
				} else if (*(p+2) == '\r' && *(p+3) == '\n') {
					InHeader = FALSE;
				}
			} else {
				if (MboxFormat) {
					if (str_cmp_n(p, MBOX_DELIMITER, mlen) == 0) {
						MsgEnd = FilePtr + (p - FileBuf);
						if (Found) {
							break;
						}
						p += mlen;
						while (*p != '\0' && (*p != '\r' || *(p+1) != '\n')) {
							p++;
						}
						MsgStart = FilePtr + (p - FileBuf) + 2;
						InHeader = TRUE;
					}
				} else {
					if (str_cmp_n(p, "\r\n.\r\n", 5) == 0) {
						MsgEnd = FilePtr + (p - FileBuf);
						if (Found) {
							break;
						}
						p += 3;
						MsgStart = FilePtr + (p - FileBuf) + 2;
						InHeader = TRUE;
					}
				}
			}
			p += 2;
			// done with this line
			while (*p != '\0' && (*p != '\r' || *(p+1) != '\n')) {
				p++;
			}
		}
		FilePtr += ReadSize;
	}
	if (Found == TRUE) {
		char *tmp;
		len = MsgEnd - MsgStart;
		tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
		SetFilePointer(hFile, MsgStart, NULL, FILE_BEGIN);
		if (ReadFile(hFile, tmp, len, &ret, NULL) == FALSE) {
			CloseHandle(hFile);
			return NULL;
		}
		if (code >= STATUS_REVISION_NPOPUK3) {
			tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
			item_mail_to_item(tpMailItem, &tmp, -1, MAIL2ITEM_WIRE, 0, NULL);
		} else {
			CP_int = CP_ACP;
			tpMailItem = item_string_to_item(NULL, tmp, FALSE);
			CP_int = CP_UTF8; // restore to UTF8
			mem_free(&tmp);
		}
	}
	CloseHandle(hFile);

	return tpMailItem;
}

/*
 * file_write - マルチバイトに変換して保存
 */
BOOL file_write(HANDLE hFile, char *buf, int len)
{
	DWORD ret;

	if (len == 0) return TRUE;
	return WriteFile(hFile, buf, len, &ret, NULL);
}

/*
 * file_write_ascii - マルチバイトに変換して保存
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
 * file_save_attach - ファイルの保存
 */
BOOL file_save_attach(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len, int do_what)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;

	// ファイルに保存
	if (FileName == NULL) {
		*path = TEXT('\0');
	} else {
		lstrcpy(path, FileName);
	}

	if (do_what == DECODE_SAVE_EMBED || do_what == DECODE_SAVE_IMAGES) {
		wsprintf(path, TEXT("%s%s"), DataDir, op.AttachPath);
		dir_create(path);
		wsprintf(path, TEXT("%s%s\\%s%s"), DataDir, op.AttachPath, ATTACH_FILE, FileName);
	} else if (do_what == DECODE_SAVE_ALL) {
		wsprintf(path, TEXT("%s\\%s"), op.SavedSaveDir, FileName);
		if (file_get_size(path) > 0) {
			TCHAR msg[MSG_SIZE];
			wsprintf(msg, STR_Q_REPLACEFILE, path);
			if (MessageBox(hWnd, msg, STR_TITLE_SAVE,
				MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
				return FALSE;
			}
		}
	} else {
		if (filename_select(hWnd, path, Ext, NULL, FILE_SAVE_SINGLE, &op.SavedSaveDir) == FALSE) {
			return TRUE; // user cancelled, not an error
		}
	}

	// 保存するファイルを開く
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
 * file_save_exec - ファイルを保存して実行
 */
BOOL file_save_exec(HWND hWnd, TCHAR *FileName, char *buf, int len)
{
	SHELLEXECUTEINFO sei;
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR tmp_path[BUF_SIZE];
	DWORD ret;

	if (FileName == NULL) {
		return FALSE;
	}
	SwitchCursor(FALSE);

	// ディレクトリの検索
	wsprintf(path, TEXT("%s%s"), DataDir, op.AttachPath);
	dir_create(path);

	wsprintf(path, TEXT("%s%s\\%s"), DataDir, op.AttachPath, FileName);
	if (op.AttachDelete == 0) {
		// 一時ファイルに保存
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

		// ファイルの移動
		if (CopyFile(tmp_path, path, FALSE) == FALSE) {
			DeleteFile(tmp_path);
			SwitchCursor(TRUE);
			return FALSE;
		}
		DeleteFile(tmp_path);
	} else {
		// ファイルに保存
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

	// 実行
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
 * file_save_mailbox - メールボックス内のメールを保存
 */
BOOL file_save_mailbox(TCHAR *FileName, TCHAR *SaveDir, int Index, BOOL IsBackup, BOOL IsAutosave, int SaveFlag)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	MAILBOX *tpMailBox = MailBox + Index;
	char *tmp, *p, *s;
	///////////// MRP /////////////////////
	TCHAR pathBackup[BUF_SIZE];
	///////////// --- /////////////////////

	int len = 0;
	int i;
	if (op.ScrambleMailboxes && op.WriteMbox == 0) {
		len = tstrlen(ENCRYPT_PREAMBLE);
	}

	i = lstrlen(SaveDir) + lstrlen(FileName) + 5; // .bak\0
	if (i >= BUF_SIZE) {
		ErrorMessage(NULL, STR_ERR_FILENAME_TOO_LONG);
		return FALSE;
	}

	str_join_t(path, SaveDir, FileName, (TCHAR *)-1);
	tpMailBox->DiskSize = 0;

	if (tpMailBox->MailItemCnt == 0 || (SaveFlag == 0 && tpMailBox->Type != MAILBOX_TYPE_SAVE)) {
		//When it does not retain, deletion
		DeleteFile(path);
		if (IsBackup == FALSE) {
			tpMailBox->NeedsSave = 0;
		}
		return TRUE;
	}

#ifndef DIV_SAVE
	//Mail character string compilation
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		len += item_to_string_size(*(tpMailBox->tpMailItem + i), 
			op.WriteMbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);
	}
	p = s = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	*p = '\0';
	if (op.ScrambleMailboxes && op.WriteMbox == 0) {
		p = s = str_cpy(p, ENCRYPT_PREAMBLE);
	}
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		p = item_to_string(p, *(tpMailBox->tpMailItem + i), 
			op.WriteMbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);
	}
	if (op.ScrambleMailboxes && op.WriteMbox == 0) {
		rot13(s,p);
	}
	// adjust length for writing to what's actually used;
	// item_to_string_size can overestimate the length
	len = p - tmp;
#endif	// DIV_SAVE

	///////////// MRP /////////////////////
#ifdef UNICODE
	wcscpy(pathBackup, path);
	wcscat(pathBackup, TEXT(".bak"));
#else
	strcpy_s(pathBackup, BUF_SIZE-5, path);
	strcat_s(pathBackup, BUF_SIZE, TEXT(".bak"));
#endif
	// DeleteFile(pathBackup);
	// how to ensure the drive is ready?
	if (file_get_size(pathBackup) != -1) {
		if (DeleteFile(pathBackup) == 0 && op.SocLog > 9) {
			TCHAR msg[MSG_SIZE];
			DWORD err = GetLastError();
			wsprintf(msg, TEXT("Failed to delete old backup file %s (err=%X)\r\n"), pathBackup, err);
			log_save(msg);
			if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, msg, MSG_SIZE-1, NULL)) {
				log_save(msg);
			}
		}
	}
	// Create the backup file.
	if (MoveFile(path, pathBackup) == 0 && op.SocLog > 9 && file_get_size(path) != -1) {
		TCHAR msg[MSG_SIZE];
		DWORD err = GetLastError();
		wsprintf(msg, TEXT("Failed to create backup file %s (err=%X)\r\n"), pathBackup, err);
		log_save(msg);
		if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, msg, MSG_SIZE-1, NULL)) {
			log_save(msg);
		}
	}
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
	if (op.ScrambleMailboxes && op.WriteMbox == 0) {
		if (file_write(hFile, encrypt_header, len) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		len = item_to_string_size(*(tpMailBox->tpMailItem + i), 
			op.WriteMbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);

		p = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
		if (tmp == NULL) {
			CloseHandle(hFile);
			return FALSE;
		}
		item_to_string(tmp, *(tpMailBox->tpMailItem + i), 
			op.WriteMbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);
		if (op.ScrambleMailboxes && op.WriteMbox == 0) {
			rot13(tmp, tmp+len);
		}
		if (file_write(hFile, tmp, len) == FALSE) {
			mem_free(&tmp);
			CloseHandle(hFile);
			return FALSE;
		}
		mem_free(&tmp);
	}
#endif	// DIV_SAVE
	CloseHandle(hFile);
	tpMailBox->DiskSize = file_get_size(path);
	if (tpMailBox->DiskSize == -2) {
		TCHAR msg[MSG_SIZE];
		wsprintf(msg, STR_ERR_FILE_TOO_LARGE, path);
		ErrorMessage(NULL, msg);
	}
	tpMailBox->WasMbox = op.WriteMbox;

	///////////// MRP /////////////////////
	DeleteFile(pathBackup);
	///////////// --- /////////////////////
	if (IsBackup == FALSE) {
		tpMailBox->NeedsSave = 0;
	}
	if (op.LazyLoadMailboxes != 0 && Index != MAILBOX_SEND && Index != RecvBox
		&& Index != SelBox && Index != vSelBox && (IsAutosave == FALSE || tpMailBox->NewMail == 0)) {
		// unload
		tpMailBox->Loaded = FALSE;
		SaveBoxesLoaded = FALSE;
		if(tpMailBox->tpMailItem != NULL){
			item_free(tpMailBox->tpMailItem, tpMailBox->MailItemCnt);
			mem_free((void **)&tpMailBox->tpMailItem);
		}
		tpMailBox->tpMailItem = NULL;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		if (op.SocLog > 9) {
			int pos = lstrlen(path);
			if (pos > 242) pos = 240; // 240 = BUF_SIZE - strlen(" was unloaded\r\n") - 1
			wsprintf(path+pos, TEXT(" was unloaded\r\n"));
			log_save(path);
		}
	}
	return TRUE;
}

/*
 * file_append_savebox - append without loading
 */
BOOL file_append_savebox(TCHAR *FileName, MAILBOX *tpMailBox, MAILITEM *tpMailItem, int SaveFlag)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	char *tmp, *p, *s;
	int write_mbox = 0, encrypt = 0;
	int len = 0, hlen;
	BOOL add_sep = FALSE;
	long fsize;

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);
	fsize = file_get_size(path);
	hlen = tstrlen(ENCRYPT_PREAMBLE);

	// check existing file to see what format (npop/mbox) to write
	if (tpMailBox->WasMbox == -1) {
		if (fsize == -2) {
			// too large
			return FALSE;
		} else if (fsize == 0) {
			// no file -- use global option
			tpMailBox->WasMbox = (op.WriteMbox == 1) ? TRUE : FALSE;
			if (op.ScrambleMailboxes && op.WriteMbox == 0) {
				encrypt = 2;
			}
		} else {
			len = (fsize < hlen) ? fsize : hlen;
			if (len > 5) {
				tmp = file_read(path, len);
			} else {
				tmp = NULL;
			}
			if (tmp != NULL && str_cmp_n(tmp, "From ", 5) == 0) {
				tpMailBox->WasMbox = TRUE;
			} else {
				tpMailBox->WasMbox = FALSE;
			}
			if (fsize > hlen && str_cmp_n(tmp, ENCRYPT_PREAMBLE, hlen) == 0) {
				encrypt = 1;
			}
			mem_free(&tmp);
		}
	}
	write_mbox = (tpMailBox->WasMbox == TRUE) ? 1 : 0;
	if (encrypt == 2) {
		len = hlen;
	} else {
		len = 0;
	}
	if (fsize > 6) {
		//Check for proper termination
		hFile = CreateFile(path, GENERIC_READ, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != NULL && hFile != (HANDLE)-1) {
			char end[6] = {'\0','\0','\0','\0','\0','\0'};
			int numread;
			SetFilePointer(hFile, -5, NULL, FILE_END);
			ReadFile(hFile, end, 5, &numread, NULL);
			if (numread > 0) {
				if (write_mbox) {
					if (str_cmp_n(end+3, "\r\n", 2) != 0) {
						add_sep = TRUE;
						len += 2;
					}
				} else {
					if (str_cmp_n(end, "\r\n.\r\n", 5) != 0) {
						add_sep = TRUE;
						len += 5;
					}
				}
			}
			CloseHandle(hFile);
		}
	} else if (fsize > 0) {
		add_sep = TRUE;
		len += (write_mbox) ? 2 : 5;
	}

	len += item_to_string_size(tpMailItem, write_mbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);
	p = s = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	*p = '\0';
	if (encrypt == 2) {
		p = s = str_cpy(p, ENCRYPT_PREAMBLE);
	} else if (add_sep) {
		if (write_mbox == 0) {
			*(p++) = '\r';
			*(p++) = '\n';
			*(p++) = '.';
		}
		*(p++) = '\r';
		*(p++) = '\n';
		*p = '\0';
	}
	item_to_string(p, tpMailItem, write_mbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);
	if (encrypt) {
		rot13(s, p);
	}

	//Retention
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		mem_free(&tmp);
		return FALSE;
	}
	SetFilePointer(hFile, 0, NULL, FILE_END);
	if (file_write(hFile, tmp, len) == FALSE) {
		mem_free(&tmp);
		return FALSE;
	}
	mem_free(&tmp);
	CloseHandle(hFile);
	tpMailBox->DiskSize += len;

	return TRUE;
}

/*
 * file_save_address_item - アドレス帳を1件保存
 */
static BOOL file_save_address_item(HANDLE hFile, ADDRESSITEM *tpAddrItem)
{
	TCHAR *tmp;
	int len = 0;

	// Mail address
	if (tpAddrItem->MailAddress != NULL) {
		len += lstrlen(tpAddrItem->MailAddress);
	}
	// Group/Category (GJC)
	if (tpAddrItem->Group != NULL && *tpAddrItem->Group != TEXT('\0')) {
		len += 2;	// TAB TAB
		len += lstrlen(tpAddrItem->Group);
		if (tpAddrItem->Comment != NULL && *tpAddrItem->Comment != TEXT('\0')) {
			len += lstrlen(tpAddrItem->Comment);
		}
	// Comment
	} else if (tpAddrItem->Comment != NULL && *tpAddrItem->Comment != TEXT('\0')) {
		len += 1;	// TAB
		len += lstrlen(tpAddrItem->Comment);
	}
	len += 2;		// CRLF

	tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	if (tpAddrItem->Group != NULL && *tpAddrItem->Group != TEXT('\0')) {
		str_join_t(tmp, tpAddrItem->MailAddress, TEXT("\t"), tpAddrItem->Comment, TEXT("\t"), tpAddrItem->Group, TEXT("\r\n"), (TCHAR *)-1);
	} else if (tpAddrItem->Comment != NULL && *tpAddrItem->Comment != TEXT('\0')) {
		str_join_t(tmp, tpAddrItem->MailAddress, TEXT("\t"), tpAddrItem->Comment, TEXT("\r\n"), (TCHAR *)-1);
	} else {
		str_join_t(tmp, tpAddrItem->MailAddress, TEXT("\r\n"), (TCHAR *)-1);
	}

	if (file_write_ascii(hFile, tmp, len) == FALSE) {
		mem_free(&tmp);
		return FALSE;
	}
	mem_free(&tmp);
	return TRUE;
}

/*
 * file_save_address_book - アドレス帳をファイルに保存
 */
BOOL file_save_address_book(TCHAR *FileName, TCHAR *SaveDir, ADDRESSBOOK *tpAddrBook)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE], pathBackup[BUF_SIZE];
	int i;

	str_join_t(path, SaveDir, FileName, (TCHAR *)-1);

#ifdef UNICODE
	wcscpy(pathBackup, path);
	wcscat(pathBackup, TEXT(".bak"));
#else
	strcpy_s(pathBackup, BUF_SIZE-5, path);
	strcat_s(pathBackup, BUF_SIZE, TEXT(".bak"));
#endif
	DeleteFile(pathBackup);
	MoveFile(path, pathBackup); // Create the backup file.

	//The file which it retains is opened
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}

	//The character string which it retains conversion to copy
	for (i = 0; i < tpAddrBook->ItemCnt; i++) {
		if (*(tpAddrBook->tpAddrItem + i) == NULL) {
			continue;
		}
		if (file_save_address_item(hFile, *(tpAddrBook->tpAddrItem + i)) == FALSE) {
			CloseHandle(hFile);
			return FALSE;
		}
	}
	CloseHandle(hFile);
	DeleteFile(pathBackup);
	return TRUE;
}

/*
 * file_read_address_book - of pause of log The address register is read from the file
 */
int file_read_address_book(TCHAR *FileName, ADDRESSBOOK *tpAddrBook, BOOL GetContacts)
{
	ADDRESSITEM *tpAddrItem;
	TCHAR path[BUF_SIZE], pathBackup[BUF_SIZE];
	TCHAR *MemFile, *AllocBuf = NULL;
	TCHAR *p, *r, *s;
	unsigned char *FileBuf;
	long FileSize;
#ifdef UNICODE
	long Len;
#endif
	int LineCnt = 0;
	int i, offset = 0, retcode = 0;

	str_join_t(path, DataDir, FileName, (TCHAR *)-1);

	// GJC copies MRP code from file_read_mailbox
#ifdef UNICODE
	wcscpy(pathBackup, path);
	wcscat(pathBackup, TEXT(".bak"));
#else
	strcpy_s(pathBackup, BUF_SIZE-5, path);
	strcat_s(pathBackup, BUF_SIZE, TEXT(".bak"));
#endif
	FileSize = file_get_size(pathBackup);
	if (FileSize == 0) {
		DeleteFile(pathBackup); // GJC
	} else if (FileSize == -2) {
		TCHAR msg[MSG_SIZE];
		wsprintf(msg, STR_ERR_FILE_TOO_LARGE, pathBackup);
		ErrorMessage(NULL, msg);
		return -2;
	} else if (FileSize != -1) { // Backup File exists
		DeleteFile(path);  // delete the current file
		MoveFile(pathBackup, path); // replace the the current file with the backup file.
	}

#ifdef _WIN32_WCE
	///////////// MRP /////////////////////
	if (op.UsePOOMAddressBook != 0 && GetContacts != FALSE) {
		retcode = UpdateAddressBook(path, op.UsePOOMAddressBook, op.POOMNameIsComment);
		// retcode = number of addresses if > 0, some kind of failure if < 0
		retcode = (retcode < 0) ? -100 : 0;
	}
	///////////// --- /////////////////////
#endif

	FileSize = file_get_size(path);
	if (FileSize < 0) {
		return 0 + retcode;
	}
	if (FileSize == 0) {
		return 1 + retcode;
	}
	FileBuf = file_read(path, FileSize);
	if (FileBuf == NULL) {
		return -1;
	}

#ifdef UNICODE
	//UNICODE
	if( *FileBuf == 0xEF && *(FileBuf+1) == 0xBB && *(FileBuf+2) == 0xBF ) {
		CP_int = CP_UTF8;
		offset = 3;
	} else if (op.Version < 3000) {
		// nPOPuk used CP_ACP for Version < 3000
		CP_int = CP_ACP;
	}
	Len = char_to_tchar_size(FileBuf+offset);
	MemFile = AllocBuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (Len + 1));
	if (MemFile == NULL) {
		mem_free(&FileBuf);
		return -1;
	}
	char_to_tchar(FileBuf+offset, MemFile, Len);
	FileSize = Len;
	CP_int = CP_UTF8; // restore to UTF8
#else	// UNICODE
	MemFile = (TCHAR *)FileBuf;
#endif	// UNICODE

	//Count
	p = MemFile;
	while(*p == TEXT('\n') || *p == TEXT('\r')) p++;
	for (LineCnt = 0; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\n')) {
			LineCnt++;
			while(*p == TEXT('\n') || *p == TEXT('\r')) p++;
		}
	}
	tpAddrBook->ItemCnt = LineCnt;
	tpAddrBook->tpAddrItem = (ADDRESSITEM **)mem_calloc(sizeof(ADDRESSITEM *) * tpAddrBook->ItemCnt);
	if (tpAddrBook->tpAddrItem == NULL) {
		tpAddrBook->ItemCnt = 0;
		mem_free(&FileBuf);
		mem_free(&AllocBuf);
		return -1;
	}
	i = 0;
	p = MemFile;
	while(*p == TEXT('\n') || *p == TEXT('\r')) p++;
	while (FileSize > p - MemFile && *p != TEXT('\0') && i < LineCnt) {
		tpAddrItem = *(tpAddrBook->tpAddrItem + i) = (ADDRESSITEM *)mem_calloc(sizeof(ADDRESSITEM));

		//of the number of lines Mail address
		for (r = p; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if (tpAddrItem != NULL) {
			tpAddrItem->MailAddress = (TCHAR *)mem_alloc(sizeof(TCHAR) * (r - p + 1));
			if (tpAddrItem->MailAddress != NULL) {
				for (s = tpAddrItem->MailAddress; p < r; p++, s++) {
					*s = *p;
				}
				*s = '\0';
				if (s > tpAddrItem->MailAddress) {
					s = (TCHAR *)mem_alloc(sizeof(TCHAR) * (s - tpAddrItem->MailAddress + 1));
					if (s != NULL) {
						GetMailAddress(tpAddrItem->MailAddress, s, NULL, FALSE);
						tpAddrItem->AddressOnly = alloc_copy_t(s);
						mem_free(&s);
					}
				}
			}
		}
		if (*r == TEXT('\t')) r++;

		// Comment
		for (p = r; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if (tpAddrItem != NULL) {
			tpAddrItem->Comment = (TCHAR *)mem_alloc(sizeof(TCHAR) * (r - p + 1));
			if (tpAddrItem->Comment != NULL) {
				for (s = tpAddrItem->Comment; p < r; p++, s++) {
					*s = *p;
				}
				*s = '\0';
			}
		}
		if (*r == TEXT('\t')) r++;

		// Group/Category (GJC)
		for (p = r; *r != TEXT('\0') && *r != TEXT('\t') && *r != TEXT('\r') && *r != TEXT('\n'); r++);
		if (tpAddrItem != NULL && r > p) {
			tpAddrItem->Group = (TCHAR *)mem_alloc(sizeof(TCHAR) * (r - p + 1));
			if (tpAddrItem->Group != NULL) {
				for (s = tpAddrItem->Group; p < r; p++, s++) {
					if (*p == TEXT(';')) {
						*s = TEXT(',');
					} else {
						*s = *p;
					}
				}
				*s = '\0';
			}
		}
		while(*p != TEXT('\0') && *p != TEXT('\r') && *p != TEXT('\n')) p++;
		while(*p == TEXT('\n') || *p == TEXT('\r')) p++;
		tpAddrItem->Num = i;
		i++;
	}
	mem_free(&FileBuf);
	mem_free(&AllocBuf);
	return 1 + retcode;
}

/*
 * file_rename
 */
BOOL file_rename(HWND hWnd, TCHAR *Source, TCHAR *Destin)
{
	TCHAR source_path[2*BUF_SIZE], destin_path[2*BUF_SIZE];
	BOOL ret;

	str_join_t(source_path, DataDir, Source, (TCHAR *)-1);
	str_join_t(destin_path, DataDir, Destin, (TCHAR *)-1);

	ret = MoveFile(source_path, destin_path);

	if (ret == 0 && file_get_size(source_path) == -1) {
		ret = 1;
	}

	if (ret) {
		// need to delete backup file, lest it be found next time we file_read_mailbox
		TCHAR pathBackup[BUF_SIZE];
		wsprintf(pathBackup, TEXT("%s.bak"), Destin);
		file_delete(hWnd, pathBackup);
	}

	return ret;
}

BOOL file_delete(HWND hWnd, TCHAR *name)
{
	TCHAR path[2*BUF_SIZE];
	BOOL done = FALSE;
	str_join_t(path, DataDir, name, (TCHAR *)-1);
	if (file_get_size(path) != -1) {
#ifdef DO_RECYCLE_BIN
BOOL file_delete(HWND hWnd, TCHAR *name, BOOL recycle)
		if (recycle) {
#ifdef _WIN32_WCE
			TCHAR RecycPath[BUF_SIZE], pathR[2*BUF_SIZE];
			if (CeGetSpecialFolderPath(CSIDL_BITBUCKET, BUF_SIZE, RecycPath) > 0) {
				str_join_t(pathR, RecycPath, name, (TCHAR *)-1);
				done = MoveFile(path, pathR);
			}
#else
			// what's the call on Win32?
#endif
#endif
		if (done == FALSE && DeleteFile(path) == FALSE) {
			TCHAR msg[3*BUF_SIZE];
			wsprintf(msg, STR_ERR_CANTDELETE, path);
			ErrorMessage(hWnd, msg);
			done = FALSE;
		}
	} else {
		done = TRUE;
	}
	return done;
}

/* End of source */
