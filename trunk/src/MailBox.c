/*
 * nPOP
 *
 * MailBox.c
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

/* Define */
#define IDC_CB				2000

#define POP_PORT			110
#define SMTP_PORT			25

/* Global Variables */
extern OPTION op;

#ifdef _WIN32_WCE_LAGENDA
extern HMENU hMainMenu;
extern HMENU hViewMenu;
#endif

extern int MailMenuPos;
extern HWND MainWnd;
extern HWND hMainToolBar;
extern HWND hViewWnd;
extern HWND hViewToolBar;

extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern ADDRESSBOOK *AddressBook;

extern int SelBox, vSelBox;
extern int LvSortFlag;
extern BOOL EndThreadSortFlag;
extern BOOL PPCFlag;

/* Local Function Prototypes */

/*
 * mailbox_init - メールボックスの初期化
 */
BOOL mailbox_init(void)
{

	//Guaranty
	MailBoxCnt = 1;
	MailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * MailBoxCnt);
	if(MailBox == NULL){
		return FALSE;
	}
	//アドレス帳の確保
	AddressBook = (ADDRESSBOOK *)mem_calloc(sizeof(ADDRESSBOOK));
	if(AddressBook == NULL){
		return FALSE;
	}
	return TRUE;
}

/*
 * mailbox_create - メールボックスの追加
 */
int mailbox_create(HWND hWnd, BOOL ShowFlag, BOOL SelFlag)
{
	MAILBOX *TmpMailBox;
	int cnt, index;

	//It adds to the list of the mailbox
	index = MailBoxCnt;
	cnt = MailBoxCnt + 1;

	TmpMailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * cnt);
	if(TmpMailBox == NULL){
		return -1;
	}

	CopyMemory(TmpMailBox, MailBox, sizeof(MAILBOX) * MailBoxCnt);

	(TmpMailBox + index)->Type = 0;
	(TmpMailBox + index)->Port = POP_PORT;
	(TmpMailBox + index)->SmtpPort = SMTP_PORT;

	(TmpMailBox + index)->PopSSLInfo.Verify = 1;
	(TmpMailBox + index)->PopSSLInfo.Depth = -1;
	(TmpMailBox + index)->SmtpSSLInfo.Verify = 1;
	(TmpMailBox + index)->SmtpSSLInfo.Depth = -1;

	mem_free(&MailBox);
	MailBox = TmpMailBox;
	MailBoxCnt++;

	if (ShowFlag == TRUE) {
		int i;

		//Adding the mailbox to the ?, it selects the
		i = SendDlgItemMessage(hWnd, IDC_COMBO, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
		if (i == CB_ERR) {
			return -1;
		}
		if (SelFlag == TRUE) {
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, i, 0);
			index = i;
			mailbox_menu_rebuild(hWnd);
		}
	}
	//Only guaranty of memory
	return index;
}

/*
 * mailbox_delete - メールボックスの削除
 */
int mailbox_delete(HWND hWnd, int DelIndex, BOOL CheckFilt)
{
	MAILBOX *TmpMailBox;
	int cnt;
	int i, j;

	//From list of mailbox deletion
	cnt = MailBoxCnt - 1;
	TmpMailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * cnt);
	if(TmpMailBox == NULL){
		return -1;
	}

	j = 0;
	for(i = 0; i < MailBoxCnt; i++){
		if(i == DelIndex){
			mailbox_free(MailBox + i);
			continue;
		}
		CopyMemory((TmpMailBox + j), (MailBox + i), sizeof(MAILBOX));
		j++;
	}
	mem_free(&MailBox);
	MailBox = TmpMailBox;
	MailBoxCnt = cnt;

	// check that no filter was pointing to this deleted mailbox
	if (CheckFilt == TRUE) {
		filter_sbox_check(hWnd, NULL);
	}

	//Deleting from the ???????, it selects the mailbox of one ago the
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, DelIndex, 0);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, DelIndex - 1, 0);
	mailbox_menu_rebuild(hWnd);
	return DelIndex - 1;
}

/*
 * mailbox_read - 固定メールボックスの読み込み
 */
BOOL mailbox_read(void)
{
	//送信箱
	(MailBox + MAILBOX_SEND)->Name = alloc_copy_t(STR_SENDBOX_NAME);
	if(file_read_mailbox(SENDBOX_FILE, (MailBox + MAILBOX_SEND)) == FALSE){
		return FALSE;
	}

	//アドレス帳
	if(file_read_address_book(ADDRESS_FILE, AddressBook) == -1){
		return FALSE;
	}
	return TRUE;
}

/*
 * mailbox_move_up - メールボックスの位置を上に移動する
 */
void mailbox_move_up(HWND hWnd)
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

	//The position where it is indicated in the ??????? the portable
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SelBox--;
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox,
		(LPARAM)(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
	mailbox_menu_rebuild(hWnd);
}

/*
 * mailbox_move_down - メールボックスの位置を下に移動する
 */
void mailbox_move_down(HWND hWnd)
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

	//The position where it is indicated in the ??????? the portable
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SelBox++;
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox,
		(LPARAM)(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
	mailbox_menu_rebuild(hWnd);
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
int mailbox_next_unread(int index, int endindex)
{
	int j;

	if(index < MAILBOX_USER){
		return -1;
	}
	for(j = index; j < endindex; j++){
		if(mailbox_unread_check(j, TRUE) == TRUE){
			return j;
		}
	}
	return -1;
}

/*
 * mailbox_menu_rebuild
 */
BOOL mailbox_menu_rebuild(HWND unused) {
	int i, this_id;
	int last_copy_id, last_move_id, last_copy_idv, last_move_idv;
	TCHAR *name;
	HMENU hMenu, vMenu = NULL;

	last_copy_id = last_copy_idv = ID_MENUITEM_COPY2NEW;
	last_move_id = last_move_idv = ID_MENUITEM_MOVE2NEW;

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

	if (hViewWnd != NULL) {
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
		vMenu = SHGetSubMenu(hViewToolBar, ID_MENUITEM_FILE);
#elif defined(_WIN32_WCE_LAGENDA)
		vMenu = GetSubMenu(hViewMenu, 0);
#else
#define IDC_VCB 2000
		vMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hViewWnd, IDC_VCB), 0), 0);
#endif
#else
		vMenu = GetSubMenu(GetMenu(hViewWnd), 2);
#endif
	}

	// build in reverse order because InsertMenu puts item above
	for (i = MailBoxCnt; i >= 0; i--) {
		// ModifyMenu not available for PPC2002, so delete all existing entries ...
		DeleteMenu(hMenu, ID_MENUITEM_COPY2MBOX+i, MF_BYCOMMAND);
		DeleteMenu(hMenu, ID_MENUITEM_MOVE2MBOX+i, MF_BYCOMMAND);
		if (vMenu != NULL) {
			DeleteMenu(vMenu, ID_MENUITEM_COPY2MBOX+i, MF_BYCOMMAND);
			DeleteMenu(vMenu, ID_MENUITEM_MOVE2MBOX+i, MF_BYCOMMAND);
		}
		// ... and repopulate those that are saveboxes
		if (i < MailBoxCnt && (MailBox+i) != NULL && (MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
			if ((MailBox+i)->Name == NULL || *(MailBox+i)->Name == TEXT('\0')) {
				name = STR_MAILBOX_NONAME;
			} else {
				name = (MailBox+i)->Name;
			}
			this_id = ID_MENUITEM_COPY2MBOX+i;
			if (InsertMenu(hMenu, last_copy_id, MF_BYCOMMAND | MF_STRING, this_id, name)) {
				last_copy_id = this_id;
			}
			if (vMenu != NULL && InsertMenu(vMenu, last_copy_idv, MF_BYCOMMAND | MF_STRING, this_id, name)) {
				last_copy_idv = this_id;
			}
			this_id = ID_MENUITEM_MOVE2MBOX+i;
			if (InsertMenu(hMenu, last_move_id, MF_BYCOMMAND | MF_STRING, this_id, name)) {
				last_move_id = this_id;
			}
			if (vMenu != NULL && InsertMenu(vMenu, last_move_idv, MF_BYCOMMAND | MF_STRING, this_id, name)) {
				last_move_idv = this_id;
			}
		}
	}
	if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
		EnableMenuItem(hMenu, ID_MENUITEM_COPY2MBOX + SelBox, 1);
		EnableMenuItem(hMenu, ID_MENUITEM_MOVE2MBOX + SelBox, 1);
	}
	if (vMenu != NULL && (MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
		EnableMenuItem(vMenu, ID_MENUITEM_COPY2MBOX + vSelBox, 1);
		EnableMenuItem(vMenu, ID_MENUITEM_MOVE2MBOX + vSelBox, 1);
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

	if(Sel == -1){
		return;
	}
	SelBox = Sel;
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);

	//Acquisition
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hMainMenu, MailMenuPos);
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), MailMenuPos);
#endif
#else
	hMenu = GetSubMenu(GetMenu(hWnd), MailMenuPos);
#endif

	//of menu Item of the mark is deleted to reply and one for reception the
	DeleteMenu(hMenu, ID_MENUITEM_REMESSEGE, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_ALLREMESSEGE, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_DOWNMARK, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_SAVECOPY, MF_BYCOMMAND);

	mailbox_menu_rebuild(hWnd);

	if(SelBox == MAILBOX_SEND){
		//Transmission box
#ifdef _WIN32_WCE
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDINFO : STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDMARK : STR_LIST_MENU_SENDMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITEM_SAVECOPY,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_CREATECOPY : STR_LIST_MENU_CREATECOPY);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
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
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_RECVMARK : STR_LIST_MENU_RECVMARK);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_ALLREMESSEGE, STR_LIST_MENU_REPLYALL);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK, STR_LIST_MENU_RECVMARK);
#endif
		lvc.pszText = STR_LIST_LVHEAD_FROM;
	}

	//Setting
	lvc.mask = LVCF_TEXT;
	lvc.cchTextMax = BUF_SIZE;
	ListView_SetColumn(GetDlgItem(hWnd, IDC_LISTVIEW), 1, &lvc);

	LvSortFlag = SORT_NO + 1;
	EndThreadSortFlag = FALSE;

	//of list view header The release
	SwitchCursor(FALSE);
	ListView_ShowItem(GetDlgItem(hWnd, IDC_LISTVIEW), (MailBox + SelBox), FALSE);
	SwitchCursor(TRUE);

	SetMailMenu(hWnd);
	SetItemCntStatusText(hWnd, NULL, FALSE);
	SetUnreadCntTitle(hWnd, FALSE);
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

	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->FilterCnt > 0) {
			BOOL filter_error = FALSE;
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
				MessageBox(hWnd, buf, STR_TITLE_ERROR, MB_OK);
			}
		}
	}
}

/*
 * filter_free - フィルタ情報の解放
 */
void filter_free(MAILBOX *tpMailBox)
{
	int i;

	//フィルタ情報の解放
	for(i = 0; i < tpMailBox->FilterCnt; i++){
		if(*(tpMailBox->tpFilter + i) == NULL){
			continue;
		}
		mem_free(&(*(tpMailBox->tpFilter + i))->SaveboxName);

		mem_free(&(*(tpMailBox->tpFilter + i))->Header1);
		mem_free(&(*(tpMailBox->tpFilter + i))->Content1);

		mem_free(&(*(tpMailBox->tpFilter + i))->Header2);
		mem_free(&(*(tpMailBox->tpFilter + i))->Content2);

		mem_free(&*(tpMailBox->tpFilter + i));
	}
	mem_free((void **)&tpMailBox->tpFilter);
	tpMailBox->tpFilter = NULL;
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
			newitem->Comment = alloc_copy_t(olditem->Comment);
			newitem->Group = alloc_copy_t(olditem->Group);
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
void addr_move(ADDRESSBOOK *tpAddrBook, int num, int dir)
{
	int i = num;
	ADDRESSITEM *HoldItem, *AddrItem; 
	HoldItem = *(tpAddrBook->tpAddrItem + num);
	// move until we hit a visible item (or the end)
	while((i > 0 && dir == -1) || ( i < tpAddrBook->ItemCnt -1 && dir == +1)) {
		AddrItem = *(tpAddrBook->tpAddrItem + i) = *(tpAddrBook->tpAddrItem + i + dir);
		i += dir;
		if (AddrItem->displayed == TRUE) {
			break;
		}
	}
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
	mem_free(&AddrItem->Comment);
	mem_free(&AddrItem->Group);
	mem_free(&AddrItem);
	for (i = num; i < tpAddrBook->ItemCnt-1; i++) {
		*(tpAddrBook->tpAddrItem + i) = *(tpAddrBook->tpAddrItem + i + 1);
	}
	tpAddrBook->ItemCnt--;
	// tpAddrBook->tpAddrItem could be resized, but why bother?
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
