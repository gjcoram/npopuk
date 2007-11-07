/*
 * nPOP
 *
 * File.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2007 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopsupport.org.uk
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
#define ENCRYPT_PREAMBLE	TEXT("NPOPUK_ENCRYPT")

/* Global Variables */
extern OPTION op;

extern HINSTANCE hInst;  // Local copy of hInstance
extern TCHAR *AppDir;
extern TCHAR *DataDir;
extern MAILBOX *MailBox;

/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int file_get_mail_count(char *buf, long Size, int MboxFormat);
static BOOL file_save_address_item(HANDLE hFile, ADDRESSITEM *tpAddrItem);
static UINT CALLBACK OpenFileHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
 * log_init - ログの区切りの保存
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
 * dir_check - ディレクトリかどうかチェック
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
 * OpenFileHook - change Cancel to Skip for saving multiple attachments (GJC)
 */
#ifndef _WIN32_WCE_PPC
static UINT CALLBACK OpenFileHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmh = (LPNMHDR) lParam;

	if (uMsg == WM_NOTIFY && pnmh != NULL && pnmh->code == CDN_INITDONE) {
		SetDlgItemText(GetParent(hDlg), IDCANCEL, TEXT("Skip"));
	}
	return 0;
}
#endif

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
	TCHAR path[MULTI_BUF_SIZE], buf[BUF_SIZE];
	TCHAR *ph, *qh;
	BOOL is_open = (Action == FILE_OPEN_SINGLE || Action == FILE_OPEN_MULTI);

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
	} else if (Action == FILE_SAVE_MSG) {
		of.lpstrInitialDir = DataDir;
	} else if (is_open == FALSE && Action != FILE_CHOOSE_DIR) {
		// saving an attachment
		wsprintf(buf, TEXT("%s%s"), DataDir, op.AttachPath);
		dir_create(buf);
		of.lpstrInitialDir = buf;
	} else if (Action == FILE_CHOOSE_DIR) {
		ParanoidMessageBox(hWnd, STR_WARN_BACKUPDIR, WINDOW_TITLE, MB_ICONEXCLAMATION | MB_OK);
	} // else is_open or choose dir (backup): just let Windows determine the directory

	of.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
#ifndef _WIN32_WCE
	// GJC allow multiselect
	if (Action == FILE_OPEN_MULTI) {
		of.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
		of.lpstrFile = ret;
		of.nMaxFile = MULTI_BUF_SIZE - 1;
	} else if (Action == FILE_SAVE_MULTI) {
		of.Flags |= OFN_ENABLEHOOK | OFN_EXPLORER | OFN_ENABLESIZING;
		of.lpfnHook = (LPOFNHOOKPROC)OpenFileHook;
	}
#endif

	//File selective dialogue is indicated
	if (is_open) {
		of.Flags |= OFN_FILEMUSTEXIST;
		if (GetOpenFileName((LPOPENFILENAME)&of) == FALSE) {
			if (Action == FILE_OPEN_MULTI && lstrcmp(path, ret) != 0) {
				ErrorMessage(hWnd, STR_ERR_TOOMANYFILES); 
			}
			return FALSE;
		}
	} else {
		if (GetSaveFileName((LPOPENFILENAME)&of) == FALSE) {
			return FALSE;
		}
		if (Action == FILE_CHOOSE_DIR) {
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
	return TRUE;
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
	char *p, *r, *t;
	int ret = 0, len;

	if (MboxFormat) {
		ret = 1;
		len = tstrlen(MBOX_DELIMITER);
		for (p = buf; Size > p - buf && *p != '\0'; p++) {
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
 * file_savebox_convert (GJC)
 */
BOOL file_savebox_convert(TCHAR *NewFileName)
{
	TCHAR path[BUF_SIZE], newpath[BUF_SIZE];
	BOOL ret;

	str_join_t(path, DataDir, SAVEBOX_FILE, (TCHAR *)-1);

	if(file_get_size(path) == -1) {
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
	TCHAR msg[BUF_SIZE];

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

	if (lstrcmp(Source, path) == 0) {
		return TRUE;
	}
	if (file_get_size(path) != -1) {
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
BOOL file_read_mailbox(TCHAR *FileName, MAILBOX *tpMailBox, BOOL Import)
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
	int i, cnt, len = 7; // len = tstrlen(MBOX_DELIMITER);
	int MboxFormat = 0;

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
	} else if (FileSize != -1) { // Backup File exists
		DeleteFile(path);  // delete the current file
		MoveFile(pathBackup, path); // replace the the current file with the backup file.
	}
	///////////// --- /////////////////////

	tpMailBox->DiskSize = FileSize = file_get_size(path);
	if (FileSize <= 0) {
		tpMailBox->Loaded = TRUE;
		if (op.SocLog > 1) {
			int pos = lstrlen(path);
			if (pos > 230) pos = 230;
			wsprintf(path+pos, TEXT(" loaded but empty"));
			log_save(AppDir, LOG_FILE, path);
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
	cnt = file_get_mail_count(FileBuf, FileSize, MboxFormat);
	// check for confused file format: mbox-format mailbox was opened&saved by
	// previous version of npop/npopuk
	if (cnt == 1 && MboxFormat == 0) {
		int cnt2 = file_get_mail_count(FileBuf, FileSize, 1);
		if (cnt2 > 1) {
			TCHAR *temp;
			item_get_content_t(FileBuf, HEAD_X_STATUS, &temp);
			if (temp != NULL) {
				int rev = _ttoi(temp)/100000;
				if (rev == 1) {
					TCHAR *title, msg[BUF_SIZE];
					title = tpMailBox->Name;
					if (title == NULL || *title == TEXT('\0')) {
						title = STR_MAILBOX_NONAME;
					}
					wsprintf(msg, STR_Q_LOADASMBOX, FileName, cnt2);
					if (MessageBox(NULL, msg, title, MB_YESNO) == IDYES) {
						cnt = cnt2;
						MboxFormat = 1;
					}
				}
				mem_free(&temp);
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

	i = 0;
	p = FileBuf;
	while (FileSize > p - FileBuf && *p != '\0') {
		// From header mail item acquisition
		tpMailItem = *(tpMailBox->tpMailItem + i) = item_string_to_item(tpMailBox, p, Import);

		// Body position Position of end of mail acquisition
		if (MboxFormat) {
			tpMailItem->HasHeader = 1;
			r = GetHeaderStringPoint(p, HEAD_X_STATUS); // X-Status is always the last header written by nPOPuk
			if (r != NULL && r > p) {
				tpMailItem->HasHeader = 2;
				while (*r != '\0' && (*(r-1) != '\r' || *r != '\n')) {
					r++;
				}
				if (*r == '\n') {
					r++;
					if (str_cmp_n(r, MBOX_DELIMITER, len) == 0) {
						tpMailItem->HasHeader = 0;
					} else if (*r == '\r' && *(r+1) == '\n') {
						r += 2;
						tpMailItem->HasHeader = 0;
					}
					p = r;
				} else if (*r == '\0') {
					break;
				}
			}
		} else {
			// discard all header lines
			p = GetBodyPointa(p);
			if (p == NULL) {
				break;
			}
		}

		// Find end of message
		if (MboxFormat) {
			for (t = r = p; *r != '\0'; r++) {
				if (str_cmp_n(r, MBOX_DELIMITER, len) == 0) {
					t = r;
					r += 2;
					break;
				}
			}
			if (*r == '\0') {
				t = r;
			}
		} else {
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
		}
		if (tpMailItem != NULL) {
			//Body copy
			if ((t - p) > 0) {
				if (tpMailItem->Multipart == MULTIPART_ATTACH) {
					tpMailItem->AttachSize -= (t - p);
				}
				tpMailItem->Body = (char *)mem_alloc(sizeof(char) * (t - p + 1));
				if (tpMailItem->Body != NULL) {
					for (s = tpMailItem->Body; p < t; p++, s++) {
						*s = *p;
					}
					*s = '\0';
// GJC: now, multipart/alternative <==> MULTIPART_HTML
#ifdef DO_MULTIPART_SCAN
					if (tpMailItem->Multipart == MULTIPART_CONTENT && tpMailItem->Download == TRUE
						&& tpMailItem->Attach == NULL && tpMailItem->FwdAttach == NULL) {
						// is it text/plain and text/html with no real attachments
#ifdef UNICODE
						char *ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
						tpMailItem->Multipart = multipart_scan(ContentType, tpMailItem->Body);
						mem_free(&ContentType);
#else
						tpMailItem->Multipart = multipart_scan(tpMailItem->ContentType, tpMailItem->Body);
#endif
					}
#endif
					if (tpMailItem->HasHeader == 1 || (MboxFormat == 1 && tpMailItem->Encoding != NULL)) {
						// strip duplicate headers and/or convert from foreign MBOX format
						char *newbody;
						int len, header_size;
						if (tpMailItem->HasHeader == 1) {
							s = GetBodyPointa(tpMailItem->Body);
							header_size = remove_duplicate_headers(tpMailItem->Body);
							tpMailItem->HasHeader = 2;
							tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
						} else {
							s = tpMailItem->Body;
							header_size = 0;
						}
						if (tpMailItem->Encoding != NULL) {
							t = MIME_body_decode_transfer(tpMailItem, s);
						} else {
							t = s;
						}
						len = tstrlen(t);

						newbody = (char *)mem_alloc(sizeof(char) * (len + header_size + 1));
						if (newbody == NULL) {
							if (header_size > 0) {
								// shift body backwards (after removing duplicate headers)
								tstrcpy(tpMailItem->Body + header_size, t);
							}
							// else the body is smaller than the allocated space; big deal
						} else {
							str_cpy_n(newbody, tpMailItem->Body, header_size + 1);
							tstrcpy(newbody + header_size, t);
							mem_free(&tpMailItem->Body);
							tpMailItem->Body = newbody;
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
	if (op.SocLog > 1) {
		int pos = lstrlen(path);
		if (pos > 240) pos = 240;
		wsprintf(path+pos, TEXT(" was loaded"));
		log_save(AppDir, LOG_FILE, path);
	}
	return TRUE;
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
 * file_save - ファイルの保存
 */
BOOL file_save_message(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len, BOOL Multi)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	DWORD ret;
	int SaveAction = (Multi == FALSE) ? FILE_SAVE_SINGLE : FILE_SAVE_MULTI;

	// ファイルに保存
	if (FileName == NULL) {
		*path = TEXT('\0');
	} else {
		lstrcpy(path, FileName);
	}

	if (filename_select(hWnd, path, Ext, NULL, SaveAction, &op.SavedSaveDir) == FALSE) {
		return TRUE;
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
BOOL file_save_mailbox(TCHAR *FileName, TCHAR *SaveDir, MAILBOX *tpMailBox, BOOL IsBackup, int SaveFlag)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	char *tmp, *p;
	///////////// MRP /////////////////////
	TCHAR pathBackup[BUF_SIZE];
	///////////// --- /////////////////////

//	TCHAR encrypt_header[80];
	int len = 0;
	int i;
//	if (op.ScrambleMailboxes) {
//		wsprintf(encrypt_header, TEXT("%s %d.\r\n"), ENCRYPT_PREAMBLE, tpMailBox->MailItemCnt);
//		len = lstrlen(encrypt_header);
//	}

	i = lstrlen(SaveDir) + lstrlen(FileName) + 5; // .bak\0
	if (i >= BUF_SIZE) {
		ErrorMessage(NULL, STR_ERR_FILENAME_TOO_LONG);
		return FALSE;
	}

	str_join_t(path, SaveDir, FileName, (TCHAR *)-1);
	tpMailBox->DiskSize = 0;

	if (SaveFlag == 0 && tpMailBox->Type != MAILBOX_TYPE_SAVE) {
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
		len += item_to_string_size(*(tpMailBox->tpMailItem + i), 
			op.WriteMbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);
	}
	p = tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (tmp == NULL) {
		return FALSE;
	}
	*p = '\0';
//	if (op.ScrambleMailboxes) {
//		p = str_cpy_t(p, encrypt_header);
//	}
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		p = item_to_string(p, *(tpMailBox->tpMailItem + i), 
			op.WriteMbox, (SaveFlag == 1) ? FALSE : TRUE, TRUE);
	}
#endif	// DIV_SAVE

	///////////// MRP /////////////////////
#ifdef UNICODE
	wcscpy(pathBackup, path);
	wcscat(pathBackup, TEXT(".bak"));
#else
	strcpy_s(pathBackup, BUF_SIZE-5, path);
	strcat_s(pathBackup, BUF_SIZE, TEXT(".bak"));
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
//	if (op.ScrambleMailboxes) {
//		if (file_write(hFile, encrypt_header, len) == FALSE) {
//			CloseHandle(hFile);
//			return FALSE;
//		}
//	}
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

	///////////// MRP /////////////////////
	DeleteFile(pathBackup);
	///////////// --- /////////////////////
	if (IsBackup == FALSE) {
		tpMailBox->NeedsSave = 0;
	}
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
int file_read_address_book(TCHAR *FileName, ADDRESSBOOK *tpAddrBook)
{
	ADDRESSITEM *tpAddrItem;
	TCHAR path[BUF_SIZE], pathBackup[BUF_SIZE];
	TCHAR *MemFile, *AllocBuf = NULL;
	TCHAR *p, *r, *s;
	char *FileBuf;
	long FileSize;
#ifdef UNICODE
	long Len;
#endif
	int LineCnt = 0;
	int i;
	int retcode = 0;

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
	} else if (FileSize != -1) { // Backup File exists
		DeleteFile(path);  // delete the current file
		MoveFile(pathBackup, path); // replace the the current file with the backup file.
	}

#ifdef _WIN32_WCE
	///////////// MRP /////////////////////
	if (op.UsePOOMAddressBook != 0) {
		retcode = UpdateAddressBook(path, op.UsePOOMAddressBook, op.POOMNameIsComment);
		if (retcode < 0) {
			// some kind of failure
			retcode = -100;
		}
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
	TCHAR source_path[BUF_SIZE], destin_path[BUF_SIZE];
	TCHAR pathBackup[BUF_SIZE];
	BOOL ret;

	str_join_t(source_path, DataDir, Source, (TCHAR *)-1);
	str_join_t(destin_path, DataDir, Destin, (TCHAR *)-1);

	ret = MoveFile(source_path, destin_path);

	if (ret) {
		// need to delete backup file, lest it be found next time we file_read_mailbox

#ifdef UNICODE
		wcscpy(pathBackup, destin_path);
		wcscat(pathBackup, TEXT(".bak"));
#else
		strcpy_s(pathBackup, BUF_SIZE-5, destin_path);
		strcat_s(pathBackup, BUF_SIZE, TEXT(".bak"));
#endif

		if (file_get_size(pathBackup) != -1) {
			if (DeleteFile(pathBackup) == FALSE) {
				wsprintf(source_path, STR_ERR_CANTDELETE, pathBackup);
				ErrorMessage(hWnd, source_path);
			}
		}
	}

	return ret;
}

/* End of source */