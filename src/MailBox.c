/*
 * nPOP
 *
 * MailBox.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2008 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#ifdef _WIN32_WCE
#include "ppcpoom.h"
#endif

/* Define */
#define IDC_CB				2000

#define POP_PORT			110
#define SMTP_PORT			25

static HMENU hCOPYFLY = NULL, hMOVEFLY = NULL;
static HMENU vCOPYFLY = NULL, vMOVEFLY = NULL;
HMENU vMenuDone = NULL;

/* Global Variables */
extern OPTION op;
extern TCHAR *DataDir;
extern int g_soc;

#ifdef _WIN32_WCE_LAGENDA
extern HMENU hMainMenu;
extern HMENU hViewMenu;
#endif

extern int MailMenuPos;
extern HWND MainWnd, hViewWnd;
#ifdef _WIN32_WCE_PPC
extern HWND hMainToolBar, hViewToolBar;
#endif

extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern ADDRESSBOOK *AddressBook;

extern int SelBox, vSelBox, RecvBox;
extern int LvSortFlag;
extern BOOL EndThreadSortFlag;
extern BOOL PPCFlag;

/* Local Function Prototypes */
static BOOL mailbox_name_clash(TCHAR *name);
static void mailbox_swap_files(HWND hWnd, int i, int j);

/*
 * mailbox_init - メールボックスの初期化
 */
BOOL mailbox_init(void)
{
	MailBoxCnt = 1;
	MailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * MailBoxCnt);
	if(MailBox == NULL){
		return FALSE;
	}

	AddressBook = (ADDRESSBOOK *)mem_calloc(sizeof(ADDRESSBOOK));
	if(AddressBook == NULL){
		return FALSE;
	}

	return TRUE;
}

/*
 * mailbox_create - add mailbox(es) to the list
 */
int mailbox_create(HWND hWnd, int Add, int Index, BOOL ShowFlag, BOOL SelFlag)
{
	MAILBOX *TmpMailBox;
	int count, i;

	count = MailBoxCnt + Add;
	if (Index == -1) {
		Index = MailBoxCnt;
	}

	if (Index < 0 || Index > MailBoxCnt) {
		TCHAR buf[BUF_SIZE];
		wsprintf(buf, TEXT("Cannot add mailbox at postion %d"), Index);
		ErrorMessage(hWnd, buf);
		return -1;
	}

	TmpMailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * count);
	if(TmpMailBox == NULL){
		return -1;
	}

	CopyMemory(TmpMailBox, MailBox, sizeof(MAILBOX) * Index);
	if (Index < MailBoxCnt) {
		CopyMemory(TmpMailBox + Index + 1, MailBox + Index, sizeof(MAILBOX) * (MailBoxCnt - Index));
	}

	// initialize settings only for Index
	(TmpMailBox + Index)->Type = 0;
	(TmpMailBox + Index)->Loaded = 1;
	(TmpMailBox + Index)->Port = POP_PORT;
	(TmpMailBox + Index)->SmtpPort = SMTP_PORT;

	(TmpMailBox + Index)->PopSSLInfo.Verify = 1;
	(TmpMailBox + Index)->PopSSLInfo.Depth = -1;
	(TmpMailBox + Index)->SmtpSSLInfo.Verify = 1;
	(TmpMailBox + Index)->SmtpSSLInfo.Depth = -1;

	mem_free(&MailBox);
	MailBox = TmpMailBox;
	MailBoxCnt = count;

	if (g_soc != -1 && Index <= RecvBox) {
		RecvBox++;
	}

	// rename files above Index, watching out for collisions
	for (i = MailBoxCnt-1; Add == 1 && i >= Index; i--) {
		if ((MailBox + i)->Filename == NULL) {
			TCHAR defname[BUF_SIZE], oldname[BUF_SIZE];
			BOOL found = FALSE;
			int k;
			wsprintf(oldname, TEXT("MailBox%d.dat"), i - MAILBOX_USER - 1);
			wsprintf(defname, TEXT("MailBox%d.dat"), i - MAILBOX_USER);
			k = MAILBOX_USER;
			while (k < MailBoxCnt) {
				if ((MailBox+k) != NULL && (MailBox+k)->Filename != NULL
					&& lstrcmp(defname, (MailBox+k)->Filename) == 0) {
					found = TRUE;
					wsprintf(defname, TEXT("MailBox%d.dat"), k - MAILBOX_USER);
					k = MAILBOX_USER; // start over, looking for this Filename
				}
				k++;
			}
			if (found) {
				(MailBox+i)->Filename = alloc_copy_t(defname);
			}
			if (i > Index) {
				file_delete(hWnd, defname);
				file_rename(hWnd, oldname, defname);
			}
		}
	}

	if (ShowFlag == TRUE) {
		int i;

		//Adding the mailbox to the menu, to obtain the index
		if (Index < MailBoxCnt-1) {
			InsertMBMenu(Index, STR_MAILBOX_NONAME);
			i = Index;
		} else {
			i = AddMBMenu(STR_MAILBOX_NONAME);
		}
		if (i == CB_ERR) {
			return -1;
		}
		if (SelFlag == TRUE) {
			SelectMBMenu(i);
			Index = i;
			mailbox_menu_rebuild(hWnd, FALSE);
		} else {
			SelectMBMenu(SelBox);
		}
	}
	//Only guaranty of memory
	return Index;
}

/*
 * mailbox_delete - delete mailbox from list
 */
int mailbox_delete(HWND hWnd, int DelIndex, BOOL CheckFilt, BOOL Select)
{
	MAILBOX *TmpMailBox;
	TCHAR name1[BUF_SIZE], name2[BUF_SIZE];
	int cnt = MailBoxCnt - 1;
	int i, j;

	if (DelIndex <= 0 || DelIndex >= MailBoxCnt) {
		wsprintf(name1, TEXT("Cannot delete mailbox %d"), DelIndex);
		ErrorMessage(hWnd, name1);
		return -1;
	}

	if (DelIndex == vSelBox && hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CLOSE, 0, 0);
	}
	if (g_soc != -1 && DelIndex <= RecvBox) {
		RecvBox--;
	}

	// if alloc fails, will re-use current block of memory
	TmpMailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * cnt);

	if ((MailBox+DelIndex)->Filename == NULL)  {
		TCHAR path[2*BUF_SIZE];
		wsprintf(name1, TEXT("MailBox%d.dat"), DelIndex - MAILBOX_USER);
		str_join_t(path, DataDir, name1, (TCHAR *)-1);
		i = file_get_size(path);
		if (i == 0) {
			file_delete(hWnd, name1);
		} else if (i > 0 || i == -2) {
			wsprintf(name2, TEXT("MailBox%d.old"), DelIndex - MAILBOX_USER);
			file_delete(hWnd, name2);
			file_rename(hWnd, name1, name2);
		} // else i<0 => does not exist
#ifdef DELETE_FILE_ALWAYS
	} else {
		file_delete(hWnd, (MailBox+DelIndex)->Filename);
#endif
	}
	mailbox_free(MailBox + DelIndex);

	if (TmpMailBox == NULL) {
		// failed to allocate new, so just use existing block of memory
		if (DelIndex < cnt) {
			CopyMemory((MailBox+DelIndex), (MailBox+DelIndex+1), (cnt - DelIndex)*sizeof(MAILBOX));
		}
		//(MailBox + cnt) = NULL; or ZeroMemory((MailBox + cnt), sizeof(MAILBOX)); ??
	} else {
		if (DelIndex > 0) {
			CopyMemory(TmpMailBox, MailBox, DelIndex*sizeof(MAILBOX));
		}
		if (DelIndex < cnt) {
			CopyMemory((TmpMailBox+DelIndex), (MailBox+DelIndex+1), (cnt - DelIndex)*sizeof(MAILBOX));
		}
		mem_free(&MailBox);
		MailBox = TmpMailBox;
	}
	MailBoxCnt = cnt;

	// rename MailBox%d files above DelIndex, rather than loading&writing them
	for (i = DelIndex; i < cnt; i++) {
		if ((MailBox + i)->Filename == NULL) {
			BOOL clash;
			int k = i;
			wsprintf(name1, TEXT("MailBox%d.dat"), i + 1 - MAILBOX_USER);
			do {
				clash = FALSE;
				wsprintf(name2, TEXT("MailBox%d.dat"), k - MAILBOX_USER);
				for (j = 0; j < cnt; j++) {
					if ((MailBox + j)->Filename != NULL 
						&& lstrcmpi(name2, (MailBox + j)->Filename) == 0) {
						clash = TRUE;
						if (k == i) {
							k = cnt+1;
						} else {
							k++;
						}
						break;
					}
				}
			} while (clash == TRUE);
			if (k != i) {
				(MailBox + i)->Filename = alloc_copy_t(name2);
			}
			file_rename(hWnd, name1, name2);
		}
	}

	// check that no filter was pointing to this deleted mailbox
	if (CheckFilt == TRUE) {
		filter_sbox_check(hWnd, NULL);
	}

	//Deleting from the drop-down combo, it selects the mailbox of one ago the
	DeleteMBMenu(DelIndex);
	if (Select) {
		SelectMBMenu(DelIndex - 1);
		mailbox_menu_rebuild(hWnd, FALSE);
	}
	return DelIndex - 1;
}

/*
 * mailbox_read - 固定メールボックスの読み込み
 */
BOOL mailbox_read(void)
{
	//送信箱
	(MailBox + MAILBOX_SEND)->Name = alloc_copy_t(STR_SENDBOX_NAME);
	if (file_read_mailbox(SENDBOX_FILE, (MailBox + MAILBOX_SEND), FALSE, FALSE) == FALSE) {
		TCHAR tmp[BUF_SIZE];
		wsprintf(tmp, STR_ERR_OPENMAILBOX, SENDBOX_FILE);
		ErrorMessage(NULL, tmp);
		return FALSE;
	}

	//アドレス帳
	if(file_read_address_book(ADDRESS_FILE, AddressBook, op.LoadPOOMAtStart) < 0){
		return FALSE;
	}
	return TRUE;
}

/*
 * mailbox_load_now - dynamic loading of mailbox
 */
int mailbox_load_now(HWND hWnd, int num, BOOL ask, BOOL do_saveboxes)
{
	TCHAR msg[BUF_SIZE];
	MAILBOX *tpMailBox = MailBox + num;

	if (tpMailBox->Loaded == FALSE) {
		TCHAR Name[BUF_SIZE];
		if (ask && op.LazyLoadMailboxes == 3) {
			int retval;
			wsprintf(msg, STR_Q_LOADMAILBOX, 
				(tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0'))
				? STR_MAILBOX_NONAME : tpMailBox->Name);
			SwitchCursor(TRUE);
			retval = MessageBox(NULL, msg, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNOCANCEL);
			if (retval == IDNO) {
				return 0;
			} else if (retval == IDCANCEL) {
				return -1;
			}
			SwitchCursor(FALSE);
		}
		if (tpMailBox->Filename == NULL) {
			wsprintf(Name, TEXT("MailBox%d.dat"), num - MAILBOX_USER);
		} else {
			lstrcpy(Name, tpMailBox->Filename);
		}
		SwitchCursor(FALSE);
		if (file_read_mailbox(Name, tpMailBox, FALSE, (tpMailBox->Type == MAILBOX_TYPE_SAVE)) == FALSE) {
			if (hWnd != NULL) {
				wsprintf(msg, STR_ERR_LOADMAILBOX, Name);
				ErrorMessage(hWnd, msg);
			}
			return 0;
		}
	}

	if (do_saveboxes && tpMailBox->Type != MAILBOX_TYPE_SAVE) {
		FILTER *tpFilter;
		int t, j;
		for (t = 0; t < tpMailBox->FilterCnt; t++) {
			tpFilter = *(tpMailBox->tpFilter + t);
			if (tpFilter->Action == FILTER_COPY_INDEX || tpFilter->Action == FILTER_MOVE_INDEX) {
				for (j = 0; j < MailBoxCnt; j++) {
					if ((MailBox+j)->Loaded == FALSE && (MailBox+j)->Type == MAILBOX_TYPE_SAVE
						&& (MailBox+j)->Name != NULL && lstrcmp(tpFilter->SaveboxName, (MailBox+j)->Name) == 0) {
						if (mailbox_load_now(hWnd, j, FALSE, FALSE) != 1) {
							wsprintf(msg, STR_ERR_LOADMAILBOX, (MailBox+j)->Name);
							ErrorMessage(hWnd, msg);
							return 0;
						}
						break;
					}
				}
			}
		}
	}
	SwitchCursor(TRUE);
	return 1;
}

/*
 * mailbox_name_clash - check for Filename=name
 */
static BOOL mailbox_name_clash(TCHAR *name)
{
	int k;
	for (k = MAILBOX_USER; k < MailBoxCnt; k++) {
		if ((MailBox+k) != NULL && (MailBox+k)->Filename != NULL
			&& lstrcmpi(name, (MailBox+k)->Filename) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * mailbox_swap_files - exchange mailbox files (if Filename==NULL)
 */
void mailbox_swap_files(HWND hWnd, int i, int j)
{
	TCHAR name1[BUF_SIZE], name2[BUF_SIZE], path[2*BUF_SIZE];
	int len;

	if ((MailBox+i)->Filename != NULL && (MailBox+j)->Filename != NULL) {
		return;
	}
	if ((MailBox+i)->Filename == NULL) {
		wsprintf(name1, TEXT("MailBox%d.dat"), i - MAILBOX_USER);
		str_join_t(path, DataDir, name1, (TCHAR *)-1);
		len = file_get_size(path);
		if (len == -1) {
			// mailbox file "name1" doesn't exist
			if ((MailBox+j)->Filename == NULL) {
				wsprintf(name2, TEXT("MailBox%d.dat"), j - MAILBOX_USER);
				if (mailbox_name_clash(name1)) {
					// some other mailbox has Filename==name1 -- but it's empty??
					(MailBox+j)->Filename = alloc_copy_t(name2);
				} else {
					file_rename(hWnd, name2, name1);
				}
			}
			return;
		}
		if ((MailBox+j)->Filename == NULL) {
			TCHAR *tmp_name = TEXT("$npop_tmp_mailbox.dat");
			file_delete(hWnd, tmp_name);
			wsprintf(name2, TEXT("MailBox%d.dat"), j - MAILBOX_USER);
			if (file_rename(hWnd, name1, tmp_name) == FALSE) {
				// rename failed, so use the current names
				// [MAILBOX-i] in INI file is now Mailboxj.dat
				(MailBox+i)->Filename = alloc_copy_t(name1);
				(MailBox+j)->Filename = alloc_copy_t(name2);
			} else {
				// this could fail if name2 doesn't exist, but that's OK
				file_rename(hWnd, name2, name1);
				// this shouldn't fail ...
				file_rename(hWnd, tmp_name, name2);
			}
		} else {
			// MailBoxi becomes MailBoxj (unless MailBoxj.dat is in use)
			BOOL found = FALSE;
			wsprintf(name2, TEXT("MailBox%d.dat"), j - MAILBOX_USER);
			found = mailbox_name_clash(name2);
			if (found == FALSE) {
				file_delete(hWnd, name2);
			}
			if (found || file_rename(hWnd, name1, name2) == FALSE) {
				(MailBox+i)->Filename = alloc_copy_t(name1);
			}
		}
	} else {
		mailbox_swap_files(hWnd, j, i);
	}
}

/*
 * mailbox_move_up - メールボックスの位置を上に移動する
 */
void mailbox_move_up(HWND hWnd, BOOL select)
{
	MAILBOX *TmpMailBox;
	int i;

	if(SelBox <= MAILBOX_USER || MailBoxCnt <= MAILBOX_USER + 1){
		return;
	}

	//Position of memory portable
	TmpMailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * MailBoxCnt);
	if(TmpMailBox == NULL){
		return;
	}
	mailbox_swap_files(hWnd, SelBox, SelBox-1);
	for(i = 0; i < MailBoxCnt; i++){
		if(SelBox == i + 1){
			CopyMemory((TmpMailBox + i), (MailBox + i + 1), sizeof(MAILBOX));
			CopyMemory((TmpMailBox + i + 1), (MailBox + i), sizeof(MAILBOX));
			i++;
		}else{
			CopyMemory((TmpMailBox + i), (MailBox + i), sizeof(MAILBOX));
		}
	}
	mem_free(&MailBox);
	MailBox = TmpMailBox;

	//Change the position where it is indicated in the drop-down combo
	DeleteMBMenu(SelBox);
	SelBox--;
	InsertMBMenu(SelBox,
		(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
	if (select) {
		SelectMBMenu(SelBox);
		mailbox_menu_rebuild(hWnd, FALSE);
	}
}

/*
 * mailbox_move_down - メールボックスの位置を下に移動する
 */
void mailbox_move_down(HWND hWnd, BOOL select)
{
	MAILBOX *TmpMailBox;
	int i;

	if(SelBox < MAILBOX_USER || SelBox >= MailBoxCnt - 1 || MailBoxCnt <= MAILBOX_USER + 1){
		return;
	}

	//メモリの位置を移動
	TmpMailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * MailBoxCnt);
	if(TmpMailBox == NULL){
		return;
	}
	mailbox_swap_files(hWnd, SelBox, SelBox+1);
	for(i = 0; i < MailBoxCnt; i++){
		if(SelBox == i){
			CopyMemory((TmpMailBox + i), (MailBox + i + 1), sizeof(MAILBOX));
			CopyMemory((TmpMailBox + i + 1), (MailBox + i), sizeof(MAILBOX));
			i++;
		}else{
			CopyMemory((TmpMailBox + i), (MailBox + i), sizeof(MAILBOX));
		}
	}
	mem_free(&MailBox);
	MailBox = TmpMailBox;

	//Change the position where it is indicated in the drop-down combo
	DeleteMBMenu(SelBox);
	SelBox++;
	InsertMBMenu(SelBox,
		(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
	if (select) {
		SelectMBMenu(SelBox);
		mailbox_menu_rebuild(hWnd, FALSE);
	}
}

/*
 * mailbox_unread_check - メールボックスに未開封メールが存在するか調べる
 */
BOOL mailbox_unread_check(int index, BOOL NewFlag)
{
	int i;

	for(i = (MailBox + index)->MailItemCnt - 1; i >= 0 ; i--){
		if(*((MailBox + index)->tpMailItem + i) == NULL){
			continue;
		}
		if((NewFlag == TRUE || (*((MailBox + index)->tpMailItem + i))->New == TRUE) &&
			(*((MailBox + index)->tpMailItem + i))->MailStatus == ICON_MAIL){
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * mailbox_next_unread - 未開封メールが存在するメールボックスのインデックスを取得
 */
int mailbox_next_unread(HWND hWnd, int index, int endindex)
{
	int j;

	if(index < MAILBOX_USER){
		return -1;
	}
	for(j = index; j < endindex; j++){
		if ((MailBox+j)->Loaded == FALSE) {
			if (op.LazyLoadMailboxes == 2) {
				continue;
			} else {
				int load = mailbox_load_now(hWnd, j, TRUE, FALSE);
				if (load == 0) {
					continue;
				} else if (load == -1) {
					return -1;
				}
			}
		}
		if (mailbox_unread_check(j, TRUE) == TRUE) {
			return j;
		}
	}
	return -1;
}

/*
 * mailbox_menu_rebuild
 */
BOOL mailbox_menu_rebuild(HWND hWnd, BOOL IsAttach) {
	HMENU hMenu = NULL, vMenu = NULL;
	static BOOL hMenuDone = FALSE;
	static int first_cnt = -1;

	if (hWnd == MainWnd) {
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
		hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL);
#elif defined(_WIN32_WCE_LAGENDA)
		hMenu = GetSubMenu(hMainMenu, MailMenuPos);
#else
		hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(MainWnd, IDC_CB), 0), MailMenuPos);
#endif
#else
		hMenu = GetSubMenu(GetMenu(MainWnd), MailMenuPos);
#endif
	}

	if (hViewWnd != NULL) {
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
		vMenu = SHGetSubMenu(hViewToolBar, ID_MENUITEM_FILE);
#elif defined(_WIN32_WCE_LAGENDA)
		vMenu = GetSubMenu(hViewMenu, 0);
#else
#define IDC_VCB 2000
		vMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hViewWnd, IDC_VCB), 0), 2);
#endif
#else
		vMenu = GetSubMenu(GetMenu(hViewWnd), 2);
#endif
	}

	// on first entry, decide whether to make a flyout submenu or not
	if (first_cnt == -1) {
		int i;
		for (i = 0; i < MailBoxCnt; i++) {
			if ((MailBox+i) != NULL && (MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
				first_cnt++;
			}
		}
	}

	if (hMenu != NULL && hMenuDone == FALSE) {
		if (first_cnt < op.SaveboxListCount) {
			hCOPYFLY = CreatePopupMenu();
			hMOVEFLY = CreatePopupMenu();
			AppendMenu(hCOPYFLY, MF_STRING, ID_MENUITEM_COPY2NEW, STR_LIST_MENU_NEW);
			AppendMenu(hMOVEFLY, MF_STRING, ID_MENUITEM_MOVE2NEW, STR_LIST_MENU_NEW);
		} else {
			hCOPYFLY = NULL;
			hMOVEFLY = NULL;
		}
	}
	if (vMenu != NULL && vMenu != vMenuDone) {
		if (first_cnt < op.SaveboxListCount) {
			if (vCOPYFLY != NULL) {
				DestroyMenu(vCOPYFLY);
			}
			if (vMOVEFLY != NULL) {
				DestroyMenu(vMOVEFLY);
			}
			vCOPYFLY = CreatePopupMenu();
			vMOVEFLY = CreatePopupMenu();
			AppendMenu(vCOPYFLY, MF_STRING, ID_MENUITEM_COPY2NEW, STR_LIST_MENU_NEW);
			AppendMenu(vMOVEFLY, MF_STRING, ID_MENUITEM_MOVE2NEW, STR_LIST_MENU_NEW);
		} else {
			vCOPYFLY = NULL;
			vMOVEFLY = NULL;
		}
	}
	if (first_cnt < op.SaveboxListCount) {
		int i, this_id;
		int last_copy_id, last_move_id, last_copy_idv, last_move_idv;
		TCHAR *name;
		last_copy_id = last_copy_idv = ID_MENUITEM_COPY2NEW,
		last_move_id = last_move_idv = ID_MENUITEM_MOVE2NEW;

		if (hMenu != NULL && hMenuDone == FALSE) {
			DeleteMenu(hMenu, ID_MENUITEM_COPYSBOX, MF_BYCOMMAND);
			DeleteMenu(hMenu, ID_MENUITEM_MOVESBOX, MF_BYCOMMAND);
			InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_BYCOMMAND | MF_POPUP | MF_STRING,
				(UINT)hCOPYFLY, STR_LIST_MENU_COPYSBOX);
			InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_BYCOMMAND | MF_POPUP | MF_STRING,
				(UINT)hMOVEFLY, STR_LIST_MENU_MOVESBOX);
			hMenuDone = TRUE;
		}
		if (vMenu != NULL && vMenu != vMenuDone) {
			DeleteMenu(vMenu, ID_MENUITEM_COPYSBOX, MF_BYCOMMAND);
			DeleteMenu(vMenu, ID_MENUITEM_MOVESBOX, MF_BYCOMMAND);
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			InsertMenu(vMenu, 10, MF_BYPOSITION | MF_POPUP | MF_STRING,
				(UINT)vCOPYFLY, STR_LIST_MENU_COPYSBOX);
			InsertMenu(vMenu, 11, MF_BYPOSITION | MF_POPUP | MF_STRING,
				(UINT)vMOVEFLY, STR_LIST_MENU_MOVESBOX);
#else
			AppendMenu(vMenu, MF_POPUP | MF_STRING, (UINT)vCOPYFLY, STR_LIST_MENU_COPYSBOX);
			AppendMenu(vMenu, MF_POPUP | MF_STRING, (UINT)vMOVEFLY, STR_LIST_MENU_MOVESBOX);
#endif // _WIN32_WCE_PPC
#else
			InsertMenu(vMenu, 0, MF_BYPOSITION | MF_POPUP | MF_STRING,
				(UINT)vCOPYFLY, STR_LIST_MENU_COPYSBOX1);
			InsertMenu(vMenu, 1, MF_BYPOSITION | MF_POPUP | MF_STRING,
				(UINT)vMOVEFLY, STR_LIST_MENU_MOVESBOX1);
#endif // _WIN32_WCE
			vMenuDone = vMenu;
		}
		// delete all existing entries (in case saveboxes have been renamed or reordered)
		for (i = 0; i < MailBoxCnt; i++) {
			if (hCOPYFLY != NULL) {
				DeleteMenu(hCOPYFLY, ID_MENUITEM_COPY2MBOX+i, MF_BYCOMMAND);
			}
			if (hMOVEFLY != NULL) {
				DeleteMenu(hMOVEFLY, ID_MENUITEM_MOVE2MBOX+i, MF_BYCOMMAND);
			}
			if (vCOPYFLY != NULL) {
				DeleteMenu(vCOPYFLY, ID_MENUITEM_COPY2MBOX+i, MF_BYCOMMAND);
			}
			if (vMOVEFLY != NULL) {
				DeleteMenu(vMOVEFLY, ID_MENUITEM_MOVE2MBOX+i, MF_BYCOMMAND);
			}
		}
		// build in reverse order because InsertMenu puts item above
		for (i = MailBoxCnt; i >= 0; i--) {
			// insert those that are saveboxes
			if (i < MailBoxCnt && (MailBox+i) != NULL && (MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
				if ((MailBox+i)->Name == NULL || *(MailBox+i)->Name == TEXT('\0')) {
					name = STR_MAILBOX_NONAME;
				} else {
					name = (MailBox+i)->Name;
				}
				this_id = ID_MENUITEM_COPY2MBOX+i;
				if (hCOPYFLY != NULL && InsertMenu(hCOPYFLY, last_copy_id, MF_BYCOMMAND | MF_STRING, this_id, name)) {
					last_copy_id = this_id;
				}
				if (vCOPYFLY != NULL && InsertMenu(vCOPYFLY, last_copy_idv, MF_BYCOMMAND | MF_STRING, this_id, name)) {
					last_copy_idv = this_id;
				}
				this_id = ID_MENUITEM_MOVE2MBOX+i;
				if (hMOVEFLY != NULL && InsertMenu(hMOVEFLY, last_move_id, MF_BYCOMMAND | MF_STRING, this_id, name)) {
					last_move_id = this_id;
				}
				if (vMOVEFLY != NULL && InsertMenu(vMOVEFLY, last_move_idv, MF_BYCOMMAND | MF_STRING, this_id, name)) {
					last_move_idv = this_id;
				}
			}
		}
		if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
			if (hCOPYFLY != NULL) {
				EnableMenuItem(hCOPYFLY, ID_MENUITEM_COPY2MBOX + SelBox, MF_GRAYED);
			}
			if (hMOVEFLY != NULL) {
				EnableMenuItem(hMOVEFLY, ID_MENUITEM_MOVE2MBOX + SelBox, MF_GRAYED);
			}
		}
		if (vSelBox > 0 && (MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE && IsAttach == FALSE) {
			if (vCOPYFLY != NULL) {
				EnableMenuItem(vCOPYFLY, ID_MENUITEM_COPY2MBOX + vSelBox, MF_GRAYED);
			}
			if (vMOVEFLY != NULL) {
				EnableMenuItem(vMOVEFLY, ID_MENUITEM_MOVE2MBOX + vSelBox, MF_GRAYED);
			}
		} else if (IsAttach == TRUE && vMOVEFLY != NULL) {
			EnableMenuItem(vMOVEFLY, ID_MENUITEM_MOVE2NEW, MF_GRAYED);
		}
	} else {
		if (vMenu != NULL && IsAttach) {
			EnableMenuItem(vMenu, ID_MENUITEM_MOVESBOX, MF_GRAYED);
		}
	}
	return TRUE;
}

/*
 * mailbox_select - メールボックスの選択
 */
void mailbox_select(HWND hWnd, int Sel)
{
	HMENU hMenu;
	LV_COLUMN lvc;
	int colno;

	if (Sel == -1 || MailBox == NULL) {
		return;
	}

	if ((MailBox+Sel)->Loaded == FALSE) {
		if (mailbox_load_now(hWnd, Sel, FALSE, FALSE) == 0) {
			return;
		}
	}

	SelBox = Sel;
	SelectMBMenu(SelBox);

	//Acquisition
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hMainMenu, MailMenuPos);
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(MainWnd, IDC_CB), 0), MailMenuPos);
#endif
#else
	hMenu = GetSubMenu(GetMenu(MainWnd), MailMenuPos);
#endif

	// Delete entries and rebuild appropriate to SendBox or not
	DeleteMenu(hMenu, ID_MENUITEM_REMESSEGE, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_ALLREMESSEGE, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_REDIRECT, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_DOWNMARK, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_DELMARK, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_SAVECOPY, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_DELATTACH, MF_BYCOMMAND);

	mailbox_menu_rebuild(MainWnd, FALSE);
	if(SelBox == MAILBOX_SEND){
		//Transmission box
#ifdef _WIN32_WCE
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDINFO : STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_UNMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDMARK : STR_LIST_MENU_SENDMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITEM_SAVECOPY,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_CREATECOPY : STR_LIST_MENU_CREATECOPY);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_UNMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK, STR_LIST_MENU_SENDMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITEM_SAVECOPY, STR_LIST_MENU_CREATECOPY);
#endif
		lvc.pszText = STR_LIST_LVHEAD_TO;
	}else{
		//Reception box
#ifdef _WIN32_WCE
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_REPLY : STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_ALLREMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_REPLYALL : STR_LIST_MENU_REPLYALL);
		if ((MailBox+SelBox)->Type != MAILBOX_TYPE_SAVE) {
			InsertMenu(hMenu, ID_MENUITEM_UNMARK, MF_STRING,
				ID_MENUITEM_DOWNMARK,
				(PPCFlag == TRUE) ? STR_LIST_PPCMENU_RECVMARK : STR_LIST_MENU_RECVMARK);
			InsertMenu(hMenu, ID_MENUITEM_UNMARK, MF_STRING,
				ID_MENUITEM_DELMARK,
				(PPCFlag == TRUE) ? STR_LIST_PPCMENU_DELMARK : STR_LIST_MENU_DELMARK);
		}
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_ALLREMESSEGE, STR_LIST_MENU_REPLYALL);
		if ((MailBox+SelBox)->Type != MAILBOX_TYPE_SAVE) {
			InsertMenu(hMenu, ID_MENUITEM_UNMARK, MF_STRING,
				ID_MENUITEM_DOWNMARK, STR_LIST_MENU_RECVMARK);
			InsertMenu(hMenu, ID_MENUITEM_UNMARK, MF_STRING,
				ID_MENUITEM_DELMARK, STR_LIST_MENU_DELMARK);
		}
#endif
		InsertMenu(hMenu, 4, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REDIRECT, STR_LIST_MENU_REDIRECT);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITEM_DELATTACH, STR_LIST_MENU_DELATTACH);
		lvc.pszText = STR_LIST_LVHEAD_FROM;
	}

	//Setting
	lvc.mask = LVCF_TEXT;
	lvc.cchTextMax = BUF_SIZE;
	if (lstrcmpi(op.LvColumnOrder, TEXT("FSDZ")) == 0) {
		colno = 0;
	} else {
		colno = 1;
	}
	ListView_SetColumn(GetDlgItem(MainWnd, IDC_LISTVIEW), colno, &lvc);
	lvc.pszText = STR_LIST_LVHEAD_SUBJECT;
	ListView_SetColumn(GetDlgItem(MainWnd, IDC_LISTVIEW), (1-colno), &lvc);

	LvSortFlag = SORT_NO + 1;
	EndThreadSortFlag = FALSE;

	//of list view header The release
	SwitchCursor(FALSE);
	ListView_ShowItem(GetDlgItem(MainWnd, IDC_LISTVIEW), (MailBox + SelBox), FALSE);
	SwitchCursor(TRUE);

	SetMailMenu(MainWnd);
	SetItemCntStatusText(NULL, FALSE);
	SetUnreadCntTitle(FALSE);
}

/*
 * mailbox_name_to_index - メールボックスの名前からメールボックスのインデックスを取得する
 */
int mailbox_name_to_index(TCHAR *Name)
{
	int i;

	if(Name == NULL){
		return -1;
	}
	for(i = 0; i < MailBoxCnt; i++){
		if(lstrcmpi((MailBox + i)->Name, Name) == 0){
			return i;
		}
	}
	return -1;
}

/*
 * filter_sbox_check - make sure no filters try to save to non-existent savebox (GJC)
  */
void filter_sbox_check(HWND hWnd, TCHAR *ConvertName) {
	FILTER *tpFilter;
	int i, j, t;
	TCHAR buf[BUF_SIZE];
	TCHAR *p;
	BOOL filter_error = FALSE;

	for (t = 0; t < op.GlobalFilterCnt; t++) {
		tpFilter = *(op.tpFilter + t);
		if (tpFilter->Action == FILTER_COPY_INDEX || tpFilter->Action == FILTER_MOVE_INDEX) {
			BOOL found = FALSE;
			if (tpFilter->SaveboxName != NULL) {
				for (j = 0; j < MailBoxCnt; j++) {
					if ((MailBox+j)->Type == MAILBOX_TYPE_SAVE && (MailBox+j)->Name != NULL
						&& lstrcmp(tpFilter->SaveboxName, (MailBox+j)->Name) == 0) {
						found = TRUE;
						break;
					}
				}
			}
			if (found == FALSE) {
				filter_error = TRUE;
				tpFilter->Enable = 0;
			}
		}
	}
	if (filter_error == TRUE) {
		ErrorMessage(hWnd, STR_ERR_GBLFILTBOX);
	}
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->FilterCnt > 0) {
			filter_error = FALSE;
			for (t = 0; t < (MailBox+i)->FilterCnt; t++) {
				tpFilter = *((MailBox+i)->tpFilter + t);
				if (tpFilter->Action == FILTER_COPY_INDEX || tpFilter->Action == FILTER_MOVE_INDEX) {
					BOOL found = FALSE;
					if (tpFilter->SaveboxName == NULL || *(tpFilter->SaveboxName) == TEXT('\0')) {
						if (ConvertName != NULL) {
							tpFilter->SaveboxName = alloc_copy_t(ConvertName);
							found = TRUE;
						}
					} else {
						for (j = 0; j < MailBoxCnt; j++) {
							if ((MailBox+j)->Type == MAILBOX_TYPE_SAVE && (MailBox+j)->Name != NULL
								&& lstrcmp(tpFilter->SaveboxName, (MailBox+j)->Name) == 0) {
								found = TRUE;
								break;
							}
						}
					}
					if (found == FALSE) {
						filter_error = TRUE;
						tpFilter->Enable = 0;
					}
				}
			}
			if (filter_error == TRUE) {
				if ((MailBox+i)->Name == NULL || *(MailBox+i)->Name == TEXT('\0')) {
					p = STR_MAILBOX_NONAME;
				} else {
					p = (MailBox+i)->Name;
				}
				wsprintf(buf, STR_ERR_FILTBOX, p);
				ErrorMessage(hWnd, buf);
			}
		}
	}
}

/*
 * filter_free - フィルタ情報の解放
 */
void filter_free(MAILBOX *tpMailBox)
{
	int i, cnt;
	if (tpMailBox == NULL) {
		cnt = op.GlobalFilterCnt;
	} else {
		cnt = tpMailBox->FilterCnt;
	}

	for (i = 0; i < cnt; i++) {
		FILTER *tpFilter;
		if (tpMailBox == NULL) {
			tpFilter = *(op.tpFilter + i);
		} else {
			tpFilter = *(tpMailBox->tpFilter + i);
		}

		if (tpFilter == NULL){
			continue;
		}
		mem_free(&tpFilter->SaveboxName);
		mem_free(&tpFilter->FwdAddress);

		mem_free(&tpFilter->Header1);
		mem_free(&tpFilter->Content1);

		mem_free(&tpFilter->Header2);
		mem_free(&tpFilter->Content2);

		if (tpMailBox == NULL) {
			mem_free(&*(op.tpFilter + i));
		} else {
			mem_free(&*(tpMailBox->tpFilter + i));
		}
	}
	if (tpMailBox == NULL) {
		mem_free((void **)&op.tpFilter);
		op.tpFilter = NULL;
	} else {
		mem_free((void **)&tpMailBox->tpFilter);
		tpMailBox->tpFilter = NULL;
	}
}

/*
 * addressbook_copy - temporary copy for editing (GJC)
 */
ADDRESSBOOK *addressbook_copy(void)
{
	ADDRESSBOOK *tmpAddrBook;
	ADDRESSITEM *olditem, *newitem;

	int i;
	tmpAddrBook = (ADDRESSBOOK *)mem_calloc(sizeof(ADDRESSBOOK));
	if (tmpAddrBook != NULL) {
		tmpAddrBook->ItemCnt = AddressBook->ItemCnt;
		tmpAddrBook->tpAddrItem = (ADDRESSITEM **)mem_calloc(sizeof(ADDRESSITEM *) * tmpAddrBook->ItemCnt);
		if (tmpAddrBook->tpAddrItem == NULL) {
			mem_free(&tmpAddrBook);
			return NULL;
		}
		for (i = 0; i < tmpAddrBook->ItemCnt; i++) {
			olditem = *(AddressBook->tpAddrItem + i);
			newitem = *(tmpAddrBook->tpAddrItem + i) = (ADDRESSITEM *)mem_calloc(sizeof(ADDRESSITEM));
			newitem->MailAddress = alloc_copy_t(olditem->MailAddress);
			newitem->AddressOnly = alloc_copy_t(olditem->AddressOnly);
			newitem->Comment = alloc_copy_t(olditem->Comment);
			newitem->Group = alloc_copy_t(olditem->Group);
			newitem->Num = olditem->Num;
		}
	}
	return tmpAddrBook;
}
/*
 * addr_add - add item to temp addressbook (GJC)
*/
BOOL addr_add(ADDRESSBOOK *tpAddrBook, ADDRESSITEM *tpNewAddrItem)
{
	ADDRESSITEM **tpAddrList;

	tpAddrList = (ADDRESSITEM **)mem_alloc(sizeof(ADDRESSITEM *) * (tpAddrBook->ItemCnt + 1));
	if (tpAddrList == NULL) {
		return FALSE;
	}
	if (tpAddrBook->tpAddrItem != NULL) {
		CopyMemory(tpAddrList, tpAddrBook->tpAddrItem,
			sizeof(ADDRESSITEM *) * tpAddrBook->ItemCnt);
		mem_free((void **)&tpAddrBook->tpAddrItem);
	}
	tpAddrBook->tpAddrItem = tpAddrList;

	*(tpAddrList + tpAddrBook->ItemCnt) = tpNewAddrItem;
	tpAddrBook->ItemCnt++;
	return TRUE;
}

/*
 * addr_move - move item in temp addressbook (GJC)
 */
void addr_move(ADDRESSBOOK *tpAddrBook, int num, int step)
{
	ADDRESSITEM *HoldItem, *AddrItem; 
	int dir, cnt = 0, i = num;
	dir = (step > 0) ? 1 : -1;

	HoldItem = *(tpAddrBook->tpAddrItem + num);
	// move until we hit a visible item (or the end)
	while((i > 0 && dir == -1) || ( i < tpAddrBook->ItemCnt -1 && dir == +1)) {
		AddrItem = *(tpAddrBook->tpAddrItem + i) = *(tpAddrBook->tpAddrItem + i + dir);
		i += dir;
		AddrItem->Num -= dir;
		if (AddrItem->Displayed == TRUE) {
			cnt++;
			if (cnt >= abs(step)) {
				break;
			}
		}
	}
	HoldItem->Num += (dir * cnt);
	*(tpAddrBook->tpAddrItem + i) = HoldItem;
}

/*
 * addr_delete - delete item from temp addressbook (GJC)
 */
void addr_delete(ADDRESSBOOK *tpAddrBook, int num)
{
	int i;
	ADDRESSITEM *AddrItem = *(tpAddrBook->tpAddrItem + num);
	mem_free(&AddrItem->MailAddress);
	mem_free(&AddrItem->AddressOnly);
	mem_free(&AddrItem->Comment);
	mem_free(&AddrItem->Group);
	mem_free(&AddrItem);
	for (i = num; i < tpAddrBook->ItemCnt-1; i++) {
		AddrItem = *(tpAddrBook->tpAddrItem + i) = *(tpAddrBook->tpAddrItem + i + 1);
		AddrItem->Num--;
	}
	tpAddrBook->ItemCnt--;
	// tpAddrBook->tpAddrItem could be resized, but why bother?
}

/*
 * add_to_addressbook - add recipients to address book
 */
void addr_list_add(TCHAR *AddrList) {
	TCHAR *addr, *cmmt;
	int len;
	if (AddrList == NULL) {
		return;
	}
	len = lstrlen(AddrList) + 1;
	addr = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
	if (addr == NULL) {
		return;
	}
	cmmt = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
	if (cmmt == NULL) {
		return;
	}
	while (*AddrList != TEXT('\0')) {
		BOOL addit = TRUE;
		int i;

		*addr = TEXT('\0');
		*cmmt = TEXT('\0');
		GetMailAddress(AddrList, addr, cmmt, FALSE);
		for (i = 0; i < AddressBook->ItemCnt; i++) {
			if (lstrcmp(addr, (*(AddressBook->tpAddrItem + i))->AddressOnly) == 0) {
				addit = FALSE;
				break;
			}
		}
		if (addit == TRUE) {
			ADDRESSITEM *tpNewAddrItem = (ADDRESSITEM *)mem_calloc(sizeof(ADDRESSITEM));
			if (tpNewAddrItem == NULL) {
				mem_free(&addr);
				mem_free(&cmmt);
				return;
			}
			tpNewAddrItem->AddressOnly = alloc_copy_t(addr);
			tpNewAddrItem->Comment = alloc_copy_t(cmmt);
			AddrList = GetMailString(AddrList, addr);
			tpNewAddrItem->MailAddress = alloc_copy_t(addr);
			tpNewAddrItem->Group = alloc_copy_t(STR_AUTO_ADDED_ADDRESS);
			tpNewAddrItem->Num = AddressBook->ItemCnt;
			addr_add(AddressBook, tpNewAddrItem);
#ifdef _WIN32_WCE
			if (op.UsePOOMAddressBook != 0 && addr != NULL && cmmt != NULL) {
				TCHAR *fname, *lname;
				len = lstrlen(cmmt) + 1;
				if (len > 1) {
					fname = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
					lname = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
					*fname = *lname = TEXT('\0');
					GetNameFromComment(cmmt, fname, lname);
				} else {
					lname = fname = NULL;
				}
				AddPOOMContact(addr, fname, lname, tpNewAddrItem->Group);
				mem_free(&fname);
				mem_free(&lname);
			}
#endif

		} else {
			// move to next item
			AddrList = GetMailString(AddrList, addr);
		}
	}

	mem_free(&addr);
	mem_free(&cmmt);
	return;
}

/*
 * addr_free
 */
void addr_free(ADDRESSITEM **tpAddrItem, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++) {
		if (*(tpAddrItem + i) == NULL) {
			continue;
		}
		mem_free(&(*(tpAddrItem + i))->MailAddress);
		mem_free(&(*(tpAddrItem + i))->AddressOnly);
		mem_free(&(*(tpAddrItem + i))->Comment);
		mem_free(&(*(tpAddrItem + i))->Group);
		mem_free(&*(tpAddrItem + i));
		(*(tpAddrItem + i)) = NULL;
	}
	return;
}

/*
 * addressbook_free - free temp addressbook (GJC)
 */
void addressbook_free(ADDRESSBOOK *tpAddrBook)
{
	if (tpAddrBook == NULL) {
		return;
	}
	if(tpAddrBook->tpAddrItem != NULL){
		addr_free(tpAddrBook->tpAddrItem, tpAddrBook->ItemCnt);
		mem_free((void **)&tpAddrBook->tpAddrItem);
	}
	mem_free(&tpAddrBook->AddrList);
	tpAddrBook->tpAddrItem = NULL;
	tpAddrBook->ItemCnt = 0;
	mem_free(&tpAddrBook);
	tpAddrBook = NULL;
}

/*
 * mailbox_free - メールボックスの解放
 */
void mailbox_free(MAILBOX *tpMailBox)
{
	if(tpMailBox == NULL){
		return;
	}
	//アカウント情報の解放
	mem_free(&tpMailBox->Name);
	mem_free(&tpMailBox->Filename);
	mem_free(&tpMailBox->DefAccount);
	mem_free(&tpMailBox->Server);
	mem_free(&tpMailBox->User);
	mem_free(&tpMailBox->Pass);
	mem_free(&tpMailBox->TmpPass);
	mem_free(&tpMailBox->PopSSLInfo.Cert);
	mem_free(&tpMailBox->PopSSLInfo.Pkey);
	mem_free(&tpMailBox->PopSSLInfo.Pass);

	mem_free(&tpMailBox->LastMessageId);
	mem_free(&tpMailBox->SmtpServer);
	mem_free(&tpMailBox->UserName);
	mem_free(&tpMailBox->MailAddress);
	mem_free(&tpMailBox->Signature);
	mem_free(&tpMailBox->ReplyTo);
	mem_free(&tpMailBox->BccAddr);
	mem_free(&tpMailBox->SmtpUser);
	mem_free(&tpMailBox->SmtpPass);
	mem_free(&tpMailBox->SmtpTmpPass);
	mem_free(&tpMailBox->SmtpSSLInfo.Cert);
	mem_free(&tpMailBox->SmtpSSLInfo.Pkey);
	mem_free(&tpMailBox->SmtpSSLInfo.Pass);

	mem_free(&tpMailBox->RasEntry);

	//フィルタ情報の解放
	filter_free(tpMailBox);
	tpMailBox->FilterCnt = 0;

	//メール情報の解放
	if(tpMailBox->tpMailItem != NULL){
		item_free(tpMailBox->tpMailItem, tpMailBox->MailItemCnt);
		mem_free((void **)&tpMailBox->tpMailItem);
	}
	tpMailBox->tpMailItem = NULL;
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
}
/* End of source */
