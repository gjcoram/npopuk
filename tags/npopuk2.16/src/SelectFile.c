/*
 * nPOP
 *
 * SelectFile.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2008 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopsupport.org.uk
 */

/* Include Files */
#include <windows.h>
#include <Commctrl.h>
#include <aygshell.h>

#include "General.h"
#include "Memory.h"
#include "Strtbl.h"
#include "resource.h"
#include "String.h"

/* Define */
#define WM_SHOWFILELIST			(WM_USER + 1)
#define WM_LVCLICK				(WM_USER + 2)
#define WM_LVDBLCLICK			(WM_USER + 3)
#define WM_LVITEMCHANGED		(WM_USER + 4)
#define WM_LVKEYDOWN			(WM_USER + 5)

#define BUF_SIZE				256

/* Global Variables */
extern HINSTANCE hInst;  // Local copy of hInstance
extern OPTION op;
extern TCHAR *DataDir;
static TCHAR path[BUF_SIZE];
static TCHAR filename[BUF_SIZE];
static BOOL g_mode_open, g_action;

typedef struct _FILELIST {
	int att;
	int Icon;
	TCHAR Name[BUF_SIZE];
	TCHAR Type[BUF_SIZE];
	int Size;
	FILETIME ftLastWriteTime;
} FILELIST;

static FILELIST *FileList;
static int FileListCnt;

/* Local Function Prototypes */

/*
 * CreateList - ファイル一覧の作成
 */
static BOOL CreateList(HWND hDlg, HWND hListView)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	LV_ITEM lvi;
	TCHAR sPath[BUF_SIZE];
	TCHAR fname[BUF_SIZE];
	TCHAR *p;
	int cnt;
	int i;

	ListView_DeleteAllItems(hListView);
	if (FileList != NULL) {
		mem_free(&FileList);
	}
	FileList = NULL;
	FileListCnt = 0;

	*fname = TEXT('\0');
	SendDlgItemMessage(hDlg, IDC_EDIT_NAME, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)fname);
	for (p = fname; *p != TEXT('\0'); p++) {
		if (*p == TEXT('*')) {
			break;
		}
	}
	if (*p == TEXT('\0')) {
		lstrcpy(fname, TEXT("*"));
	}

	cnt = 0;
	// count directories
	wsprintf(sPath, TEXT("%s\\%s"), path, TEXT("*"));
	if ((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	do{
		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			cnt++;
		}
	} while (FindNextFile(hFindFile, &FindData) == TRUE);
	FindClose(hFindFile);
	
	wsprintf(sPath, TEXT("%s\\%s"), path, fname);
	// count matching files
	if ((hFindFile = FindFirstFile(sPath, &FindData)) != INVALID_HANDLE_VALUE) {
		do{
			if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				cnt++;
			}
		} while (FindNextFile(hFindFile, &FindData) == TRUE);
		FindClose(hFindFile);
	} else if (cnt == 0) {
		return FALSE;
	}

	// 確保
	FileList = (FILELIST *)mem_calloc(sizeof(FILELIST) * cnt);
	if (FileList == NULL) {
		return FALSE;
	}
	FileListCnt = cnt;

	i = 0;
	// get directories
	wsprintf(sPath, TEXT("%s\\%s"), path, TEXT("*"));
	if ((hFindFile = FindFirstFile(sPath, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	do{
		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			(FileList + i)->Icon = -1;
			lstrcpy((FileList + i)->Name, FindData.cFileName);
			(FileList + i)->Size = FindData.nFileSizeLow;
			memcpy(&((FileList + i)->ftLastWriteTime), &FindData.ftLastWriteTime, sizeof(FILETIME));
			i++;
		}
	} while (FindNextFile(hFindFile, &FindData) == TRUE);
	FindClose(hFindFile);

	wsprintf(sPath, TEXT("%s\\%s"), path, fname);
	// get matching files
	if ((hFindFile = FindFirstFile(sPath, &FindData)) != INVALID_HANDLE_VALUE) {
		do{
			if (!(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				(FileList + i)->att = 1;
				(FileList + i)->Icon = -1;
				lstrcpy((FileList + i)->Name, FindData.cFileName);
				(FileList + i)->Size = FindData.nFileSizeLow;
				memcpy(&((FileList + i)->ftLastWriteTime), &FindData.ftLastWriteTime, sizeof(FILETIME));
				i++;
			}
		} while (FindNextFile(hFindFile, &FindData) == TRUE);
		FindClose(hFindFile);
	} else if (i == 0) {
		// no directories either
		return FALSE;
	}

	ListView_SetItemCount(hListView, FileListCnt);

	// リストビューにアイテムを追加
	for (i = 0; i < FileListCnt; i++) {
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = LPSTR_TEXTCALLBACK;
		lvi.cchTextMax = 0;
		lvi.iImage = I_IMAGECALLBACK;
		lvi.lParam = (LPARAM)(FileList + i);
		ListView_InsertItem(hListView, &lvi);
	}
	return TRUE;
}

/*
 * ListView_GetDispItem - リストビューに表示するアイテム情報の設定
 */
static void ListView_GetDispItem(LV_ITEM *hLVItem)
{
	FILELIST *FileListItem = (FILELIST *)hLVItem->lParam;
	SHFILEINFO shfi;
	SYSTEMTIME st;
	TCHAR fdate[BUF_SIZE];
	TCHAR ftime[BUF_SIZE];
	TCHAR buf[BUF_SIZE];
	TCHAR *p;
	int f = 0;

	if (FileListItem == NULL) {
		return;
	}

	if (hLVItem->mask & LVIF_IMAGE && FileListItem->Icon == -1) {
		f |= SHGFI_SYSICONINDEX;
	}
	if (hLVItem->mask & LVIF_TEXT && hLVItem->iSubItem == 1 && *FileListItem->Type == TEXT('\0')) {
		f |= SHGFI_TYPENAME;
	}
	if (f != 0) {
		p = lstrcpy(buf, path) + lstrlen(path);
		*(p++) = TEXT('\\');
		lstrcpy(p, FileListItem->Name);
		SHGetFileInfo(buf, 0, &shfi, sizeof(SHFILEINFO), f);
	}

	// アイコン
	if (hLVItem->mask & LVIF_IMAGE) {
		if (FileListItem->Icon == -1) {
			FileListItem->Icon = shfi.iIcon;
		}
		hLVItem->iImage = FileListItem->Icon;
	}

	if (hLVItem->mask & LVIF_TEXT) {
		switch (hLVItem->iSubItem) {
		case 0:
			lstrcpy(hLVItem->pszText, FileListItem->Name);
			break;

		case 1:
			if (FileListItem->att != 0) {
				wsprintf(hLVItem->pszText, TEXT("%ld KB"),
					((FileListItem->Size > 1024) ? FileListItem->Size / 1024
					: (FileListItem->Size != 0) ? 1 : 0));
			}
			break;

		case 2:
			if (*FileListItem->Type == TEXT('\0')) {
				lstrcpy(FileListItem->Type, shfi.szTypeName);
			}
			lstrcpy(hLVItem->pszText, FileListItem->Type);
			break;

		case 3:
			FileTimeToSystemTime(&FileListItem->ftLastWriteTime, &st);
			GetDateFormat(0, 0, &st, NULL, fdate, BUF_SIZE - 1);
			GetTimeFormat(0, 0, &st, NULL, ftime, BUF_SIZE - 1);
			wsprintf(hLVItem->pszText, TEXT("%s %s"), fdate, ftime);
			break;
		}
	}
}

/*
 * CompareFunc - ソート用の比較
 */
static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	FILELIST *FileList1 = (FILELIST *)lParam1;
	FILELIST *FileList2 = (FILELIST *)lParam2;
	int ret, sfg, ghed;

	sfg = (lParamSort < 0) ? 1 : 0;
	ghed = ((lParamSort < 0) ? lParamSort * -1 : lParamSort) - 1;

	if (FileList1->att != FileList2->att) {
		ret = FileList1->att - FileList2->att;
		return (((ret < 0 && sfg == 1) || (ret > 0 && sfg == 0)) ? 1 : -1);
	}

	switch (ghed) {
	case 0:
		ret = lstrcmpi(FileList1->Name, FileList2->Name);
		break;

	case 1:
		ret = FileList1->Size - FileList2->Size;
		break;

	case 2:
		ret = lstrcmpi(FileList1->Type, FileList2->Type);
		break;

	case 3:
		ret = CompareFileTime(&FileList1->ftLastWriteTime, &FileList2->ftLastWriteTime);
		break;
	}
	return (((ret < 0 && sfg == 1) || (ret > 0 && sfg == 0)) ? 1 : -1);
}

/*
 * ListView_NotifyProc - リストビューイベント
 */
static LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;
	LV_DISPINFO *plv = (LV_DISPINFO *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;
	TCHAR buf[BUF_SIZE];
	static int LvSortFlag;
	static int LvSortCol = -1;
	int cnt, i;

	switch (plv->hdr.code) {
	case LVN_GETDISPINFO:
		ListView_GetDispItem(&(plv->item));
		return TRUE;

	case LVN_ITEMCHANGED:
		SendMessage(hWnd, WM_LVITEMCHANGED, 0, 0);
		return FALSE;
	}

	switch (LKey->hdr.code) {
	case LVN_KEYDOWN:
		return SendMessage(hWnd, WM_LVKEYDOWN, 0, lParam);
	}

	if (GetParent(CForm->hwndFrom) == GetDlgItem(hWnd, IDC_LIST_FILE)) {
		switch (phd->hdr.code) {
		case HDN_ITEMCLICK:
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			if (phd->iItem == 1) {
				cnt = ListView_GetItemCount(GetDlgItem(hWnd, IDC_LIST_FILE));
				for (i = 0; i < cnt; i++) {
					ListView_GetItemText(GetDlgItem(hWnd, IDC_LIST_FILE), i, 1, buf, BUF_SIZE - 1);
				}
			}
			// ソートの設定
			if (LvSortCol != phd->iItem) {
				LvSortCol = phd->iItem;
				LvSortFlag = 1;
			} else {
				LvSortFlag = (LvSortFlag < 0) ? 1 : -1;
			}
			// ソート
			ListView_SortItems(GetDlgItem(hWnd, IDC_LIST_FILE), CompareFunc, (LvSortCol + 1) * LvSortFlag);
			SetCursor(NULL);
			break;
		}
		return FALSE;
	}

	if (CForm->hwndFrom == GetDlgItem(hWnd, IDC_LIST_FILE)) {
		switch (CForm->code) {
		case NM_CLICK:
			// クリック
			return SendMessage(hWnd, WM_LVCLICK, 0, 0);

		case NM_DBLCLK:
			// ダブルクリック
			return SendMessage(hWnd, WM_LVDBLCLICK, 0, 0);
		}
	}
	return FALSE;
}

/*
 * CheckDir - ディレクトリかどうかチェック
 */
static BOOL CheckDir(TCHAR *fname)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	TCHAR buf[BUF_SIZE];

	wsprintf(buf, TEXT("%s\\%s"), path, fname);
	if ((hFindFile = FindFirstFile(buf, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);

	if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return TRUE;
	}
	return FALSE;
}

/*
 * CheckFile - ファイルが存在するかチェック
 */
static BOOL CheckFile(TCHAR *fname)
{
	WIN32_FIND_DATA FindData;
	HANDLE hFindFile;
	TCHAR buf[BUF_SIZE];

	wsprintf(buf, TEXT("%s\\%s"), path, fname);
	if ((hFindFile = FindFirstFile(buf, &FindData)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);

	if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		return TRUE;
	}
	return FALSE;
}


/*
 * ChangeSelPath - パスの変更
 */
static BOOL ChangeSelPath(TCHAR *fname)
{
	TCHAR buf[BUF_SIZE];

	if (CheckDir(fname) == TRUE) {
		wsprintf(buf, TEXT("%s\\%s"), path, fname);
		lstrcpy(path, buf);
		return TRUE;
	}
	return FALSE;
}

/*
 * CmboBox_AddPath - コンボボックスにパスを設定する
 */
static void CmboBox_AddPath(HWND hCombo, TCHAR *fpath)
{
	TCHAR buf[BUF_SIZE];
	TCHAR *p, *r;
	int i;

	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("\\"));
	if (*fpath == TEXT('\0')) {
		SendMessage(hCombo, CB_SETCURSEL, 0, 0);
		return;
	}
	
	for (r = buf, p = fpath + 1; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\\')) {
			*r = TEXT('\0');
			SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
			r = buf;
		} else {
			*(r++) = *p;
		}
	}
	*r = TEXT('\0');
	i = SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)buf);
	SendMessage(hCombo, CB_SETCURSEL, i, 0);
}

/*
 * CmboBox_GetPath - コンボボックスの指定位置のパスを取得
 */
static BOOL CmboBox_GetPath(HWND hCombo, int sel, TCHAR *fpath)
{
	TCHAR buf[BUF_SIZE];
	TCHAR *p;
	int len, i;

	p = buf;
	for (i = 1; i <= sel; i++) {
		*(p++) = TEXT('\\');
		len = SendMessage(hCombo, CB_GETLBTEXT, i, (LPARAM)p);
		if (len == -1) {
			return FALSE;
		}
		p += len;
	}
	*p = TEXT('\0');

	lstrcpy(fpath, buf);
	return TRUE;
}

static BOOL CALLBACK NewDirProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[BUF_SIZE];
	TCHAR *newdirname = NULL;

	switch (uMsg) {
	case WM_INITDIALOG:
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			AllocGetText(GetDlgItem(hDlg, IDC_NEWDIRNAME), &newdirname);
			if (newdirname != NULL && *newdirname != TEXT('\0')) {
				wsprintf(buf, TEXT("%s\\%s"), path, newdirname);
				if (CreateDirectory(buf, NULL) != 0) {
					lstrcpy(path, buf);
					EndDialog(hDlg, TRUE);
				}
			}
			mem_free(&newdirname);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;

}

/*
 * SelectFileDlgProc - ファイル選択プロシージャ
 */
static BOOL CALLBACK SelectFileDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SHINITDLGINFO shidi;
	SHFILEINFO shfi;
	LVCOLUMN lvc;
	HIMAGELIST IconList;
	TCHAR buf[BUF_SIZE];
	TCHAR *p;
	int i;

	switch (uMsg) {
	case WM_INITDIALOG:
		// PocketPC用初期化
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		SHInitDialog(&shidi);

		SetWindowText(hDlg, STR_SF_TITLE);

		if ((TCHAR *)lParam != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_NAME, WM_SETTEXT, 0, lParam);
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_PATH, CB_SETEXTENDEDUI, TRUE, 0);
		if (g_action == FILE_OPEN_MULTI || g_action == FILE_OPEN_SINGLE) {
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_NEW), SW_HIDE);
		}

		lvc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;

		lvc.cx = 150;
		lvc.pszText = STR_SF_LV_NAME;
		lvc.cchTextMax = lstrlen(STR_SF_LV_NAME);
		lvc.iSubItem = 0;
		lvc.fmt = LVCFMT_LEFT;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_FILE), 0, &lvc);

		lvc.cx = 70;
		lvc.pszText = STR_SF_LV_SIZE;
		lvc.cchTextMax = lstrlen(STR_SF_LV_SIZE);
		lvc.iSubItem = 1;
		lvc.fmt = LVCFMT_RIGHT;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_FILE), 1, &lvc);

		lvc.cx = 100;
		lvc.pszText = STR_SF_LV_TYPE;
		lvc.cchTextMax = lstrlen(STR_SF_LV_TYPE);
		lvc.iSubItem = 2;
		lvc.fmt = LVCFMT_LEFT;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_FILE), 2, &lvc);

		lvc.cx = 150;
		lvc.pszText = STR_SF_LV_DATE;
		lvc.cchTextMax = lstrlen(STR_SF_LV_DATE);
		lvc.iSubItem = 3;
		lvc.fmt = LVCFMT_LEFT;
		ListView_InsertColumn(GetDlgItem(hDlg, IDC_LIST_FILE), 3, &lvc);

		IconList = (HIMAGELIST)SHGetFileInfo(TEXT(""), 0,
			&shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
		ListView_SetImageList(GetDlgItem(hDlg, IDC_LIST_FILE), IconList, LVSIL_SMALL);

		ListView_SetExtendedListViewStyle(GetDlgItem(hDlg, IDC_LIST_FILE),
			LVS_EX_ONECLICKACTIVATE | LVS_EX_FULLROWSELECT);

	case WM_SHOWFILELIST:
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		SendMessage(GetDlgItem(hDlg, IDC_LIST_FILE), WM_SETREDRAW, (WPARAM)FALSE, 0);

		CreateList(hDlg, GetDlgItem(hDlg, IDC_LIST_FILE));
		ListView_SortItems(GetDlgItem(hDlg, IDC_LIST_FILE), CompareFunc, 1);

		CmboBox_AddPath(GetDlgItem(hDlg, IDC_COMBO_PATH), path);

		SetCursor(NULL);
		SendMessage(GetDlgItem(hDlg, IDC_LIST_FILE), WM_SETREDRAW, (WPARAM)TRUE, 0);
		break;

	case WM_NOTIFY:
		return ListView_NotifyProc(hDlg, lParam);

	case WM_LVCLICK:
		if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_FILE), -1, LVIS_SELECTED)) != -1) {
			*buf = TEXT('\0');
			ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_FILE), i, 0, buf, BUF_SIZE - 1);
			if (ChangeSelPath(buf) == TRUE) {
				SendMessage(hDlg, WM_SHOWFILELIST, 0, 0);
				break;
			}
		}
		break;

	case WM_LVDBLCLICK:
		if (g_action == FILE_CHOOSE_DIR || g_action == FILE_CHOOSE_BACKDIR) {
			break;
		}
		if(ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_FILE), -1, LVIS_SELECTED) != -1){
			SendMessage(hDlg, WM_COMMAND, IDOK, 0);
		}
		break;

	case WM_LVITEMCHANGED:
		if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_FILE), -1, LVIS_SELECTED)) == -1) {
			break;
		}
		*buf = TEXT('\0');
		ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_FILE), i, 0, buf, BUF_SIZE - 1);
		if (g_action == FILE_CHOOSE_DIR || g_action == FILE_CHOOSE_BACKDIR || CheckDir(buf) == TRUE) {
			break;
		}
		// GJC - allow multiple selection with CTRL
		if (g_action == FILE_OPEN_MULTI && GetKeyState(VK_CONTROL) < 0) {
			TCHAR list[BUF_SIZE], add[BUF_SIZE];
			lstrcpy(add, buf);
			SendDlgItemMessage(hDlg, IDC_EDIT_NAME, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)list);
			if (lstrcmp(list, TEXT("*.*")) == 0) {
				;
			} else if (list[0] == TEXT('\"')) {
				TCHAR *p;
				BOOL found = FALSE;
				p = list+1;
				while (*p != TEXT('\0')) {
					p = str_cpy_f_t(buf, p, TEXT('\"'));
					if (lstrcmp(buf, add) == 0) {
						found = TRUE;
						break;
					}
					if (*p == TEXT(' ') || *(p+1) == TEXT('\"')) {
						p += 2;
					}
				}
				if (found == FALSE) {
					wsprintf(buf, TEXT("\"%s\" %s"), add, list);
				} else {
					lstrcpy(buf, list);
				}
			} else if (lstrcmp(add, list) != 0) {
				wsprintf(buf, TEXT("\"%s\" \"%s\""), add, list);
			}
		}
		SendDlgItemMessage(hDlg, IDC_EDIT_NAME, WM_SETTEXT, 0, (LPARAM)buf);
		break;

	case WM_LVKEYDOWN:
		if (((LV_KEYDOWN *)lParam)->wVKey == VK_BACK) {
			SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_UP, 0);
			break;
		} else if (((LV_KEYDOWN *)lParam)->wVKey == VK_RETURN) {
			SendMessage(hDlg, WM_COMMAND, IDOK, 0);
			break;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDIT_NAME:
			switch (HIWORD(wParam)) {
			case EN_SETFOCUS:
				SHSipPreference(hDlg, SIP_UP);
				break;
			case EN_KILLFOCUS:
				SHSipPreference(hDlg, SIP_DOWN);
				break;
			}
			break;

		case IDC_COMBO_PATH:
			if (HIWORD(wParam) == CBN_CLOSEUP) {
				i = SendDlgItemMessage(hDlg, IDC_COMBO_PATH, CB_GETCURSEL, 0, 0);
				if (i < SendDlgItemMessage(hDlg, IDC_COMBO_PATH, CB_GETCOUNT, 0, 0) - 1) {
					CmboBox_GetPath(GetDlgItem(hDlg, IDC_COMBO_PATH),
						SendDlgItemMessage(hDlg, IDC_COMBO_PATH, CB_GETCURSEL, 0, 0), path);
					SendMessage(hDlg, WM_SHOWFILELIST, 0, 0);
				}
			}
			break;

		case IDC_BUTTON_UP:
			for (p = path + lstrlen(path) - 1; p > path && *p != TEXT('\\'); p--);
			if (p <= path) {
				*path = TEXT('\0');
			} else {
				*p = TEXT('\0');
			}
			SendMessage(hDlg, WM_SHOWFILELIST, 0, 0);
			break;

		case IDC_BUTTON_NEW:
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_NEWDIR), NULL, 
				NewDirProc, (LPARAM)0) == TRUE) {
					SendMessage(hDlg, WM_SHOWFILELIST, 0, 0);
			}
			break;

		case IDCANCEL:
			ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST_FILE));
			DestroyWindow(GetDlgItem(hDlg, IDC_LIST_FILE));
			EndDialog(hDlg, FALSE);
			break;

		case IDOK:
			if ((i = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_FILE), -1, LVIS_SELECTED)) != -1) {
				*buf = TEXT('\0');
				ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_FILE), i, 0, buf, BUF_SIZE - 1);
				if (ChangeSelPath(buf) == TRUE) {
					SendMessage(hDlg, WM_SHOWFILELIST, 0, 0);
					break;
				}
			}
			*filename = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_EDIT_NAME, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)filename);
			if (*filename == TEXT('\0')) {
				break;
			}
			for (p = filename; *p != TEXT('\0'); p++) {
				if (*p == TEXT('*')) {
					break;
				}
			}
			if (*p != TEXT('\0')) {
				SendMessage(hDlg, WM_SHOWFILELIST, 0, 0);
				break;
			}
			if (ChangeSelPath(filename) == TRUE) {
				SendMessage(hDlg, WM_SHOWFILELIST, 0, 0);
				break;
			}
			if (g_action == FILE_OPEN_MULTI && filename[0] == TEXT('\"')) {
				; // don't check filename
			} else if (g_mode_open == TRUE && CheckFile(filename) == FALSE) {
				break;
			}
			if (g_mode_open == FALSE && CheckFile(filename) == TRUE &&
				MessageBox(hDlg, STR_SF_Q_OVERWRITE, STR_SF_TITLE, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
				break;
			}
			ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST_FILE));
			EndDialog(hDlg, TRUE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SelectFile - ファイル選択
 */
BOOL SelectFile(HWND hDlg, HINSTANCE hInst, int Action, TCHAR *fname, TCHAR *ret, TCHAR **opptr)
{
	BOOL rc;
	TCHAR *filepart = fname;

	g_mode_open = (Action == FILE_OPEN_SINGLE || Action == FILE_OPEN_MULTI) ? TRUE : FALSE;
	g_action = Action;
	if (opptr != NULL && *opptr != NULL && lstrcmp(*opptr, TEXT("\\")) == 0) {
		*path = TEXT('\0');
	} else if (opptr != NULL && *opptr != NULL && **opptr != TEXT('\0') && dir_check(*opptr)) {
		// directory exists, use it
		lstrcpy(path, *opptr);
	} else if (g_mode_open == FALSE && Action != FILE_CHOOSE_BACKDIR) {
		lstrcpy(path, DataDir);
	} else {
		*path = TEXT('\0');
		if (Action == FILE_CHOOSE_BACKDIR) {
			ParanoidMessageBox(hDlg, STR_WARN_BACKUPDIR, WINDOW_TITLE, MB_ICONEXCLAMATION | MB_OK);
		}
	}
	rc = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SELECTFILE), hDlg, SelectFileDlgProc, (LPARAM)filepart);
	if (rc == TRUE) {
		if (Action == FILE_OPEN_MULTI && filename[0] == TEXT('\"')) {
			// GJC - format multiple selected files as done by OFN_MULTISELECT for Win32
			TCHAR *p, *q;
			unsigned i, maxlen;
			p = str_cpy_t(ret, path);
			maxlen = MULTI_BUF_SIZE - lstrlen(path) - 1;
			p++;
			for (i = 1; i < lstrlen(filename) && i < maxlen; p++, i++) {
				if (filename[i] == TEXT('\"')) {
					*p = TEXT('\0');
					q = p;
					i+=2;
				} else {
					*p = filename[i];
				}
			}
			if (i >= maxlen) {
				p = q + 1;
				MessageBox(hDlg, STR_ERR_TOOMANYFILES, STR_TITLE_ERROR, MB_OK | MB_ICONERROR);
			}
			*p = TEXT('\0');
		} else if (Action == FILE_CHOOSE_DIR || g_action == FILE_CHOOSE_BACKDIR) {
			wsprintf(ret, TEXT("%s\\"), path);
		} else {
			wsprintf(ret, TEXT("%s\\%s"), path, filename);
		}
		if (opptr != NULL) {
			if (*path == TEXT('\0')) {
				mem_free(opptr);
				*opptr = alloc_copy_t(TEXT("\\"));
			} else if (lstrcmp(path, *opptr) != 0) {
				mem_free(opptr);
				*opptr = alloc_copy_t(path);
			}
		}
	}
	if (FileList != NULL) {
		mem_free(&FileList);
	}
	FileList = NULL;
	FileListCnt = 0;
	return rc;
}
/* End of source */
