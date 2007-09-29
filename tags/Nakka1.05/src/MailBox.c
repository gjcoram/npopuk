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
 * InitMailBox - ���[���{�b�N�X�̏�����
 */
BOOL InitMailBox(void)
{

	//���[���{�b�N�X�̃��X�g�̊m��
	MailBoxCnt = 2;
	MailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * MailBoxCnt);
	if(MailBox == NULL){
		return FALSE;
	}
	//�A�h���X���̊m��
	AddressBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX));
	if(AddressBox == NULL){
		return FALSE;
	}
	return TRUE;
}

/*
 * FreeFilterInfo - �t�B���^���̉��
 */
void FreeFilterInfo(MAILBOX *tpMailBox)
{
	int i;

	//�t�B���^���̉��
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
 * FreeMailBox - ���[���{�b�N�X�̉��
 */
void FreeMailBox(MAILBOX *tpMailBox)
{
	if(tpMailBox == NULL){
		return;
	}
	//�A�J�E���g���̉��
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

	//�t�B���^���̉��
	FreeFilterInfo(tpMailBox);

	//���[�����̉��
	if(tpMailBox->tpMailItem != NULL){
		FreeMailItem(tpMailBox->tpMailItem, tpMailBox->MailItemCnt);
		mem_free((void **)&tpMailBox->tpMailItem);
	}
	tpMailBox->tpMailItem = NULL;
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
}

/*
 * ReadMailBox - �Œ胁�[���{�b�N�X�̓ǂݍ���
 */
BOOL ReadMailBox(void)
{
	//�ۑ���
	(MailBox + MAILBOX_SAVE)->Name = AllocCopy(STR_SAVEBOX_NAME);
	if(ReadItemList(SAVEBOX_FILE, (MailBox + MAILBOX_SAVE)) == FALSE){
		return FALSE;
	}

	//���M��
	(MailBox + MAILBOX_SEND)->Name = AllocCopy(STR_SENDBOX_NAME);
	if(ReadItemList(SENDBOX_FILE, (MailBox + MAILBOX_SEND)) == FALSE){
		return FALSE;
	}

	//�A�h���X��
	if(ReadAddressBook(ADDRESS_FILE, AddressBox) == 0){
		if(ReadItemList(ADDRESS_FILE_OLD, AddressBox) == FALSE){
			return FALSE;
		}
	}
	return TRUE;
}

/*
 * CreateMailBox - ���[���{�b�N�X�̒ǉ�
 */
int CreateMailBox(HWND hWnd, BOOL ShowFlag)
{
	MAILBOX *TmpMailBox;
	int cnt, index;

	//���[���{�b�N�X�̃��X�g�ɒǉ�
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

		//�R���{�{�b�N�X�Ƀ��[���{�b�N�X��ǉ����đI������
		i = SendDlgItemMessage(hWnd, IDC_COMBO, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
		if(i == CB_ERR){
			return -1;
		}
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, i, 0);
		return i;
	}
	//�������̊m�ۂ̂�
	return index;
}

/*
 * DeleteMailBox - ���[���{�b�N�X�̍폜
 */
int DeleteMailBox(HWND hWnd, int DelIndex)
{
	MAILBOX *TmpMailBox;
	int cnt;
	int i, j;

	//���[���{�b�N�X�̃��X�g����폜
	cnt = MailBoxCnt - 1;
	TmpMailBox = (MAILBOX *)mem_calloc(sizeof(MAILBOX) * cnt);
	if(TmpMailBox == NULL){
		return -1;
	}
	j = 0;
	for(i = 0; i < MailBoxCnt; i++){
		if(i == DelIndex){
			FreeMailBox(MailBox + i);
			continue;
		}
		CopyMemory((TmpMailBox + j), (MailBox + i), sizeof(MAILBOX));
		j++;
	}
	mem_free(&MailBox);
	MailBox = TmpMailBox;
	MailBoxCnt = cnt;

	//�R���{�{�b�N�X����폜���āA��O�̃��[���{�b�N�X��I������
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, DelIndex, 0);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, DelIndex - 1, 0);
	return DelIndex - 1;
}

/*
 * MoveUpMailBox - ���[���{�b�N�X�̈ʒu����Ɉړ�����
 */
void MoveUpMailBox(HWND hWnd)
{
	MAILBOX *TmpMailBox;
	int i;

	if(SelBox <= MAILBOX_USER || MailBoxCnt <= MAILBOX_USER + 1){
		return;
	}

	//�������̈ʒu���ړ�
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

	//�R���{�{�b�N�X�ɕ\������Ă���ʒu���ړ�
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SelBox--;
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox,
		(LPARAM)(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
}

/*
 * MoveDownMailBox - ���[���{�b�N�X�̈ʒu�����Ɉړ�����
 */
void MoveDownMailBox(HWND hWnd)
{
	MAILBOX *TmpMailBox;
	int i;

	if(SelBox < MAILBOX_USER || SelBox >= MailBoxCnt - 1 || MailBoxCnt <= MAILBOX_USER + 1){
		return;
	}

	//�������̈ʒu���ړ�
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

	//�R���{�{�b�N�X�ɕ\������Ă���ʒu���ړ�
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SelBox++;
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox,
		(LPARAM)(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
}

/*
 * CheckNoReadMailBox - ���[���{�b�N�X�ɖ��J�����[�������݂��邩���ׂ�
 */
BOOL CheckNoReadMailBox(int index, BOOL NewFlag)
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
 * NextNoReadMailBox - ���J�����[�������݂��郁�[���{�b�N�X�̃C���f�b�N�X���擾
 */
int NextNoReadMailBox(int index, int endindex)
{
	int j;

	if(index < MAILBOX_USER){
		return -1;
	}
	for(j = index; j < endindex; j++){
		if(CheckNoReadMailBox(j, TRUE) == TRUE){
			return j;
		}
	}
	return -1;
}

/*
 * SetNoReadCntTitle - ���ǃ��[���{�b�N�X�̐����^�C�g���o�[�ɕ\��
 */
void SetNoReadCntTitle(HWND hWnd)
{
	TCHAR wbuf[BUF_SIZE];
	int i;
	int NoReadMailBox = 0;

	for(i = MAILBOX_USER; i < MailBoxCnt; i++){
		if((MailBox + i)->NoRead == TRUE){
			NoReadMailBox++;
		}
	}

	//���ǃA�J�E���g�����^�C�g���o�[�ɐݒ�
	if(NoReadMailBox == 0){
		SetWindowText(hWnd, WINDOW_TITLE);
	}else{
		wsprintf(wbuf, STR_TITLE_NOREADMAILBOX, WINDOW_TITLE, NoReadMailBox);
		SetWindowText(hWnd, wbuf);
	}
}

/*
 * SelectMailBox - ���[���{�b�N�X�̑I��
 */
void SelectMailBox(HWND hWnd, int Sel)
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
	//�V����������� "*" ���t���Ă���ꍇ�̓R���{�{�b�N�X�̃��X�g��ݒ肵����
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox, (LPARAM)p);
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);

	//���j���[�̎擾
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

	//�ԐM�A��M�p�Ƀ}�[�N �̍��ڂ��폜����
	DeleteMenu(hMenu, ID_MENUITEM_REMESSEGE, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITEM_DOWNMARK, MF_BYCOMMAND);
	DeleteMenu(hMenu, ID_MENUITE_SAVECOPY, MF_BYCOMMAND);

	if(SelBox == MAILBOX_SEND){
		//���M��
#ifdef _WIN32_WCE
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDINFO : STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SENDMARK : STR_LIST_MENU_SENDMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_CREATECOPY : STR_LIST_MENU_CREATECOPY);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_SENDINFO);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK, STR_LIST_MENU_SENDMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY, STR_LIST_MENU_CREATECOPY);
#endif
		lvc.pszText = STR_LIST_LVHEAD_TO;
	}else{
		//��M��
#ifdef _WIN32_WCE
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_REPLY : STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_RECVMARK : STR_LIST_MENU_RECVMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY,
			(PPCFlag == TRUE) ? STR_LIST_PPCMENU_SAVEBOXCOPY : STR_LIST_MENU_SAVEBOXCOPY);
#else
		InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING,
			ID_MENUITEM_REMESSEGE, STR_LIST_MENU_REPLY);
		InsertMenu(hMenu, ID_MENUITEM_DELMARK, MF_STRING,
			ID_MENUITEM_DOWNMARK, STR_LIST_MENU_RECVMARK);
		InsertMenu(hMenu, ID_MENUITEM_DELETE, MF_STRING,
			ID_MENUITE_SAVECOPY, STR_LIST_MENU_SAVEBOXCOPY);
#endif
		lvc.pszText = STR_LIST_LVHEAD_FROM;
	}

	//���X�g�r���[�w�b�_�̐ݒ�
	lvc.mask = LVCF_TEXT;
	lvc.cchTextMax = BUF_SIZE;
	ListView_SetColumn(GetDlgItem(hWnd, IDC_LISTVIEW), 1, &lvc);

	LvSortFlag = SORT_NO + 1;
	EndThreadSortFlag = FALSE;

	//���X�g�r���[�ɃA�C�e����ݒ肷��
	SwitchCursor(FALSE);
	ListView_ShowItem(GetDlgItem(hWnd, IDC_LISTVIEW), (MailBox + SelBox));
	SwitchCursor(TRUE);

	SetMailMenu(hWnd);
	SetItemCntStatusText(hWnd, NULL);
	SetNoReadCntTitle(hWnd);
}

/*
 * GetNameToMailBox - ���[���{�b�N�X�̖��O���烁�[���{�b�N�X�̃C���f�b�N�X���擾����
 */
int GetNameToMailBox(TCHAR *Name)
{
	int i;

	if(Name == NULL){
		return -1;
	}
	for(i = 0; i < MailBoxCnt; i++){
		if(TStrCmpI((MailBox + i)->Name, Name) == 0){
			return i;
		}
	}
	return -1;
}
/* End of source */
