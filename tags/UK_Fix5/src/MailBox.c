/*
 * nPOP
 *
 * MailBox.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
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
#endif

extern int MailMenuPos;
extern HWND hMainToolBar;

extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern MAILBOX *AddressBox;

extern int SelBox;
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
	MailBoxCnt = 2;
	MailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * MailBoxCnt);
	if(MailBox == NULL){
		return FALSE;
	}
	//アドレス帳の確保
	AddressBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX));
	if(AddressBox == NULL){
		return FALSE;
	}
	return TRUE;
}

/*
 * mailbox_create - メールボックスの追加
 */
int mailbox_create(HWND hWnd, BOOL ShowFlag)
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

	(TmpMailBox + index)->Port = POP_PORT;
	(TmpMailBox + index)->SmtpPort = SMTP_PORT;

	(TmpMailBox + index)->PopSSLInfo.Verify = 1;
	(TmpMailBox + index)->PopSSLInfo.Depth = -1;
	(TmpMailBox + index)->SmtpSSLInfo.Verify = 1;
	(TmpMailBox + index)->SmtpSSLInfo.Depth = -1;

	mem_free(&MailBox);
	MailBox = TmpMailBox;
	MailBoxCnt++;

	if(ShowFlag == TRUE){
		int i;

		//Adding the mailbox to the ?, it selects the
		i = SendDlgItemMessage(hWnd, IDC_COMBO, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
		if(i == CB_ERR){
			return -1;
		}
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, i, 0);
		return i;
	}
	//Only guaranty of memory
	return index;
}

/*
 * mailbox_delete - メールボックスの削除
 */
int mailbox_delete(HWND hWnd, int DelIndex)
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

	//Deleting from the ???????, it selects the mailbox of one ago the
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, DelIndex, 0);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, DelIndex - 1, 0);
	return DelIndex - 1;
}

/*
 * mailbox_read - 固定メールボックスの読み込み
 */
BOOL mailbox_read(void)
{
	//保存箱
	(MailBox + MAILBOX_SAVE)->Name = alloc_copy_t(STR_SAVEBOX_NAME);
	if(file_read_mailbox(SAVEBOX_FILE, (MailBox + MAILBOX_SAVE)) == FALSE){
		return FALSE;
	}

	//送信箱
	(MailBox + MAILBOX_SEND)->Name = alloc_copy_t(STR_SENDBOX_NAME);
	if(file_read_mailbox(SENDBOX_FILE, (MailBox + MAILBOX_SEND)) == FALSE){
		return FALSE;
	}

	//アドレス帳
	if(file_read_address_book(ADDRESS_FILE, AddressBox) == 0){
		if(file_read_mailbox(ADDRESS_FILE_OLD, AddressBox) == FALSE){
			return FALSE;
		}
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
 * mailbox_select - メールボックスの選択
 */
void mailbox_select(HWND hWnd, int Sel)
{
	HMENU hMenu;
	LV_COLUMN lvc;
	TCHAR *p;

	if(Sel == -1){
		return;
	}
	SelBox = Sel;

	p = ((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name;
	(MailBox + SelBox)->NoRead = FALSE;
	//There is a new arrival and shows when " * " it has been attached, it does again to set the list of the ??????? the
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox, (LPARAM)p);
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
	/*
	DeleteMenu(hMenu, ID_MENUITEM_FORWARD, MF_BYCOMMAND);
	*/
	DeleteMenu(hMenu, ID_MENUITEM_DOWNMARK, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_SAVECOPY, MF_BYCOMMAND);

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
		/*
		InsertMenu(hMenu, 3, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_FORWARD,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_FORWARD : STR_LIST_MENU_FORWARD);
			*/
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_RECVMARK : STR_LIST_MENU_RECVMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITEM_SAVECOPY,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SAVEBOXCOPY : STR_LIST_MENU_SAVEBOXCOPY);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, 2, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_ALLREMESSEGE, STR_LIST_MENU_REPLYALL);
		/*
		InsertMenu(hMenu, 3, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_FORWARD, STR_LIST_MENU_FORWARD);
			*/
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK, STR_LIST_MENU_RECVMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITEM_SAVECOPY, STR_LIST_MENU_SAVEBOXCOPY);
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
	ListView_ShowItem(GetDlgItem(hWnd, IDC_LISTVIEW), (MailBox + SelBox));
	SwitchCursor(TRUE);

	SetMailMenu(hWnd);
	SetItemCntStatusText(hWnd, NULL, FALSE);
	SetNoReadCntTitle(hWnd);
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
