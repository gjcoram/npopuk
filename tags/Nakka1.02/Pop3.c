/**************************************************************************

	nPOP

	Pop3.c (RFC 1939)

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"
#include "global.h"
#include "md5.h"


/**************************************************************************
	Define
**************************************************************************/

#define OK_LEN					3			//"+OK" �̃o�C�g��

#ifdef _WIN32_WCE
#define INIT_BUFSIZE			4096		//���[����M�p�o�b�t�@�̏����T�C�Y
#else
#define INIT_BUFSIZE			32768		//���[����M�p�o�b�t�@�̏����T�C�Y
#endif

#define HEAD_LINE				30			//�w�b�_�[��
#define LINE_LEN				80

#define REDRAWCNT				50			//�X�e�[�^�X�o�[�Đݒ萔


/**************************************************************************
	Global Variables
**************************************************************************/

char *MailSize = NULL;						//���[���T�C�Y
static char *MailBuf = NULL;				//���[����M�p�o�b�t�@
static int MailBufSize;						//���[����M�p�o�b�t�@�̎��T�C�Y
static int MailBuflen;						//���[����M�p�o�b�t�@���̕�����
static char **UIDL_List = NULL;				//UIDL�ꗗ
static int UIDL_ListSize;					//UIDL�ꗗ�̃��X�g��
static int UIDL_ListPoint;					//UIDL�ꗗ�̒ǉ��ʒu
static int GetCnt;							//��M���[���ʒu
static int GetNo;							//�_�E�����[�h���[���ʒu
static int DeleNo;							//�폜���[���ʒu
static BOOL TOP_Recv;						//TOP���X�|���X��M��
static BOOL UIDL_Recv;						//UIDL���X�|���X��M��
static BOOL LvSelect;						//�P���ڎ�M�t���O
static int UseRetr;							//�V�����[����M��RETR�g�p�t���O (ListDownload �� 1 or TOP���G���[��)
static BOOL DeleCheckRetr;					//�폜���[���m�F��RETR�g�p�t���O (TOP���G���[��)

//�O���Q��
extern int MailFlag;
extern int NewMailCnt;
extern TCHAR *g_Pass;
extern int ListGetLine;
extern int ListDownload;
extern int ShowHeader;
extern int RecvScroll;
extern int SocLog;
extern int CheckAfterUpdate;

extern BOOL ShowMsgFlag;
extern BOOL NewMail_Flag;
extern BOOL EndThreadSortFlag;

extern int PopBeforeSmtpFlag;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static BOOL CheckResponse(char *buf);
static BOOL InitMailBuf(int size);
static BOOL SetMailBuf(char *buf, int len);
static BOOL InitUidlList(int size);
static BOOL SetUidlList(char *buf, int len);
static int CheckUIDL(TCHAR *buf);
static BOOL CheckMessageId(char *buf, struct TPMAILITEM *tpMailItem,
						   TCHAR *ErrStr, struct TPMAILBOX *tpMailBox);
static int CheckLastMessageId(HWND hWnd, SOCKET soc, TCHAR *ErrStr,
							  struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int SendTopCommand(HWND hWnd, SOCKET soc, int Cnt, TCHAR *ErrStr, int len, int ret);
static BOOL SendCommand(HWND hWnd, SOCKET soc, TCHAR *Command, int Cnt, TCHAR *ErrStr);
static TCHAR *CreateApopString(char *buf, TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, TCHAR *sPass);
static int MailList_Stat(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int MailList_List(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int MailList_UIDL(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int MailList_Top(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int MailFunc_Init(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int MailDownload_Retr(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int MailDelete_Top(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int MailDelete_Dele(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
static int LoginProc(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr,
					 struct TPMAILBOX *tpMailBox);


/******************************************************************************

	CheckResponse

	POP�̃��X�|���X�`�F�b�N

******************************************************************************/

static BOOL CheckResponse(char *buf)
{
	return ((tstrlen(buf) < OK_LEN || *buf != '+' ||
		!(*(buf + 1) == 'O' || *(buf + 1) == 'o') ||
		!(*(buf + 2) == 'K' || *(buf + 2) == 'k')) ? FALSE : TRUE);
}


/******************************************************************************

	InitMailBuf

	���[���o�b�t�@��������

******************************************************************************/

static BOOL InitMailBuf(int size)
{
	NULLCHECK_FREE(MailBuf);
	MailBufSize = size + 3;
	MailBuflen = 0;
	MailBuf = (char *)LocalAlloc(LPTR, MailBufSize);
	return ((MailBuf == NULL) ? FALSE : TRUE);
}


/******************************************************************************

	SetMailBuf

	���[���o�b�t�@�ɕ������ǉ�

******************************************************************************/

static BOOL SetMailBuf(char *buf, int len)
{
	char *tmp;
	char *p;

	if(MailBufSize < (MailBuflen + len + 2 + 1)){
		MailBufSize += (len + 2);
		MailBuflen += (len + 2);
		tmp = (char *)LocalAlloc(LMEM_FIXED, MailBufSize);
		if(tmp == NULL){
			return FALSE;
		}

		p = StrCpy(tmp, MailBuf);
		p = StrCpy(p, buf);
		p = StrCpy(p, "\r\n");

		LocalFree(MailBuf);
		MailBuf = tmp;
	}else{
		p = StrCpy(MailBuf + MailBuflen, buf);
		p = StrCpy(p, "\r\n");
		MailBuflen += (len + 2);
	}
	return TRUE;
}


/******************************************************************************

	FreeMailBuf

	���[���o�b�t�@�����

******************************************************************************/

void FreeMailBuf(void)
{
	NULLCHECK_FREE(MailBuf);
	MailBuf = NULL;
}


/******************************************************************************

	InitUidlList

	UIDL���X�g��������

******************************************************************************/

static BOOL InitUidlList(int size)
{
	FreeUidlList();
	UIDL_ListSize = size;
	UIDL_ListPoint = 0;
	UIDL_List = (char **)LocalAlloc(LPTR, sizeof(char *) * UIDL_ListSize);
	return ((UIDL_List == NULL) ? FALSE : TRUE);
}


/******************************************************************************

	SetUidlList

	UIDL���X�g�ɕ������ǉ�

******************************************************************************/

static BOOL SetUidlList(char *buf, int len)
{
	char *p;

	if(UIDL_ListPoint >= UIDL_ListSize){
		return FALSE;
	}
	p = *(UIDL_List + UIDL_ListPoint) = (char *)LocalAlloc(LMEM_FIXED, len + 1);
	if(p == NULL){
		return FALSE;
	}
	StrCpy(p, buf);
	UIDL_ListPoint++;
	return TRUE;
}


/******************************************************************************

	FreeUidlList

	UIDL���X�g�����

******************************************************************************/

void FreeUidlList(void)
{
	int i;

	if(UIDL_List == NULL) return;

	for(i = 0; i < UIDL_ListSize; i++){
		NULLCHECK_FREE(*(UIDL_List + i));
	}
	NULLCHECK_FREE(UIDL_List);
	UIDL_List = NULL;
	UIDL_ListSize = 0;
}


/******************************************************************************

	CheckUIDL

	UIDL��UIDL�̈ꗗ�ɑ��݂��邩�`�F�b�N

******************************************************************************/

static int CheckUIDL(TCHAR *buf)
{
	char *uidl;
	char *p;
	int i;

	if(buf == NULL){
		return 0;
	}
#ifdef UNICODE
	p = AllocTcharToChar(buf);
	if(p == NULL){
		return -1;
	}
#else
	p = buf;
#endif
	for(i = 0; i < UIDL_ListSize; i++){
		uidl = *(UIDL_List + i);
		if(uidl == NULL) continue;
		for(; *uidl != ' ' && *uidl != '\0'; uidl++);
		if(*uidl == '\0') continue;
		uidl++;
		if(tstrcmp(uidl, p) == 0){
#ifdef UNICODE
			NULLCHECK_FREE(p);
#endif
			return a2i(*(UIDL_List + i));
		}
	}
#ifdef UNICODE
	NULLCHECK_FREE(p);
#endif
	return 0;
}


/******************************************************************************

	CheckMessageId

	�w�b�_���̃��b�Z�[�WID�̃`�F�b�N

******************************************************************************/

static BOOL CheckMessageId(char *buf, struct TPMAILITEM *tpMailItem,
						   TCHAR *ErrStr, struct TPMAILBOX *tpMailBox)
{
	char *Content;
#ifdef UNICODE
	char *p;
#endif

	//Message-Id
	Content = Item_GetMessageId(buf);
	if(Content == NULL){
		lstrcpy(ErrStr, STR_ERR_SOCK_NOMESSAGEID);
		return FALSE;
	}

#ifdef UNICODE
	p = NULL;
	if(tpMailItem->MessageID != NULL){
		p = AllocTcharToChar(tpMailItem->MessageID);
		if(p == NULL){
			LocalFree(Content);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
	}
	if(tpMailItem->MessageID == NULL || tstrcmp(p, Content) != 0){
		LocalFree(Content);
		NULLCHECK_FREE(p);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return FALSE;
	}
	NULLCHECK_FREE(p);
#else
	if(tpMailItem->MessageID == NULL || tstrcmp(tpMailItem->MessageID, Content) != 0){
		LocalFree(Content);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return FALSE;
	}
#endif
	LocalFree(Content);
	return TRUE;
}


/******************************************************************************

	CheckLastMessageId

	�O��Ō�Ɏ擾�������[�����`�F�b�N����

******************************************************************************/

static int CheckLastMessageId(HWND hWnd, SOCKET soc, TCHAR *ErrStr,
							  struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	char *Content;

	Content = Item_GetMessageId(MailBuf);
	if(Content == NULL){
		FreeMailBuf();
		lstrcpy(ErrStr, STR_ERR_SOCK_NOMESSAGEID);
		return POP_ERR;
	}

	if(tstrcmp(Content, tpMailBox->LastMessageId) != 0){
		LocalFree(Content);
		FreeMailBuf();

		UIDL_Recv = FALSE;
		SetSocStatusTextT(hWnd, TEXT("UIDL\r\n"), 1);
		if(SendBuf(soc, "UIDL\r\n") == -1){
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		return POP_UIDL;

	}else{
		LocalFree(Content);

		GetCnt++;
		if(GetCnt > tpMailBox->MailCnt){
			FreeMailBuf();
			return POP_QUIT;
		}
	}
	FreeMailBuf();

	if(SendCommand(hWnd, soc, TEXT("LIST"), GetCnt, ErrStr) == FALSE){
		return POP_ERR;
	}
	return POP_LIST;
}


/******************************************************************************

	SendTopCommand

	TOP�R�}���h�𑗐M����

******************************************************************************/

static int SendTopCommand(HWND hWnd, SOCKET soc, int Cnt, TCHAR *ErrStr, int len, int ret)
{
	TCHAR wBuf[BUF_SIZE];

	wsprintf(wBuf, TEXT("TOP %d %d\r\n"), Cnt, len);
	SetSocStatusTextT(hWnd, wBuf, 1);

	if(TSendBuf(soc, wBuf) == -1){
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return POP_ERR;
	}
	return ret;
}


/******************************************************************************

	SendCommand

	�R�}���h�ƈ���(���l)�𑗐M����

******************************************************************************/

static BOOL SendCommand(HWND hWnd, SOCKET soc, TCHAR *Command, int Cnt, TCHAR *ErrStr)
{
	TCHAR wBuf[BUF_SIZE];

	wsprintf(wBuf, TEXT("%s %d\r\n"), Command, Cnt);
	SetSocStatusTextT(hWnd, wBuf, 1);

	if(TSendBuf(soc, wBuf) == -1){
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	CreateApopString

	APOP�̕�����𐶐�����

******************************************************************************/

static TCHAR *CreateApopString(char *buf, TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, TCHAR *sPass)
{
	MD5_CTX context;
	TCHAR *wbuf;
	unsigned char digest[16];
	char *hidx = NULL;
	char *tidx = NULL;
	char *pass;
	char *p;
	int len;
	int i;

	// < ���� > �܂ł𒊏o
	for(hidx = NULL, p = buf; *p != '\0';p++){
		if(*p == '<'){
			hidx = p;
		}
	}
	if(hidx != NULL){
		for(tidx = NULL, p = hidx; *p != '\0';p++){
			if(*p == '>'){
				tidx = p;
			}
		}
	}
	if(hidx == NULL || tidx == NULL){
		lstrcpy(ErrStr, STR_ERR_SOCK_NOAPOP);
		return NULL;
	}

	pass = AllocTcharToChar(sPass);
	if(pass == NULL){
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}

	p = (char *)LocalAlloc(LMEM_FIXED, (tidx - hidx + 2) + tstrlen(pass) + 1);
	if(p == NULL){
		LocalFree(pass);
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}

	StrCpyN(p, hidx, tidx - hidx + 2);
	tstrcat(p, pass);
	LocalFree(pass);

	// digest �l�����
	MD5Init(&context);
	MD5Update(&context, p, tstrlen(p));
	MD5Final(digest, &context);

	LocalFree(p);

	wbuf = (TCHAR *)LocalAlloc(LMEM_FIXED,
		sizeof(TCHAR) * (lstrlen(TEXT("APOP  \r\n")) + lstrlen(tpMailBox->User) + (16 * 2) + 1));
	if(wbuf == NULL){
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}

	len = wsprintf(wbuf, TEXT("APOP %s "), tpMailBox->User);
	for(i = 0; i < 16; i++, len += 2){
		wsprintf(wbuf + len, TEXT("%02x"), digest[i]);
	}
	lstrcat(wbuf, TEXT("\r\n"));
	return wbuf;
}


/******************************************************************************

	MailList_Stat

	STAT�̃��X�|���X�̉��

******************************************************************************/

static int MailList_Stat(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	char *p, *r, *t;
	int ret;

	if(SocLog == 1) SetStatusText(hWnd, buf);

	if(CheckResponse(buf) == FALSE){
		lstrcpy(ErrStr, STR_ERR_SOCK_STAT);
		StrCatN(ErrStr, buf, BUF_SIZE - 1);
		return POP_ERR;
	}

	p = buf;
	t = NULL;
	for(; *p != ' ' && *p != '\0'; p++);
	for(; *p == ' '; p++);
	for(r = p; *r != '\0'; r++){
		if(*r == ' '){
			t = r + 1;
			*r = '\0';
			break;
		}
	}
	tpMailBox->MailCnt = a2i(p);
	if(t != NULL){
		tpMailBox->MailSize = a2i(t);
	}

	if(tpMailBox->MailCnt == 0 || tpMailBox->LastNo == -1){
		if(ShowFlag == TRUE){
			ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LISTVIEW));
		}
		FreeMailItem(tpMailBox->tpMailItem, tpMailBox->MailItemCnt);
		NULLCHECK_FREE(tpMailBox->tpMailItem);
		tpMailBox->tpMailItem = NULL;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		tpMailBox->LastNo = 0;

		if(tpMailBox->MailCnt == 0){
			SetItemCntStatusText(hWnd, tpMailBox);
			return POP_QUIT;
		}
	}
	SetItemCntStatusText(hWnd, tpMailBox);

	LvSelect = FALSE;
	UseRetr = ListDownload;

	if(tpMailBox->LastNo == 0 || tpMailBox->LastMessageId == NULL){
		//�w��Ԗڂ���擾
		GetCnt = (tpMailBox->LastNo == 0) ? 1 : tpMailBox->LastNo;

		if(SendCommand(hWnd, soc, TEXT("LIST"), GetCnt, ErrStr) == FALSE){
			return POP_ERR;
		}
		ret = POP_LIST;

	}else if(tpMailBox->LastNo > tpMailBox->MailCnt){
		//�O��Ō�Ɏ擾�������[������菭�Ȃ�
		UIDL_Recv = FALSE;
		SetSocStatusTextT(hWnd, TEXT("UIDL\r\n"), 1);
		if(SendBuf(soc, "UIDL\r\n") == -1){
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		ret = POP_UIDL;

	}else{
		//�O��Ō�Ɏ擾�������[���̈ʒu���ς���Ă��Ȃ����`�F�b�N
		GetCnt = tpMailBox->LastNo;

		TOP_Recv = FALSE;
		if(InitMailBuf(INIT_BUFSIZE) == FALSE){
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		ret = SendTopCommand(hWnd, soc, GetCnt, ErrStr, 0, POP_TOP);
	}
	return ret;
}


/******************************************************************************

	MailList_List

	LIST�̃��X�|���X�̉��

******************************************************************************/

static int MailList_List(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	char *p, *r, *s;
	int len = 0;

	if(SocLog == 1) SetStatusText(hWnd, buf);

	NULLCHECK_FREE(MailSize);
	MailSize = NULL;
	if(CheckResponse(buf) == TRUE){
		p = buf;
		for(; *p != ' ' && *p != '\0'; p++);	//+OK
		for(; *p == ' '; p++);					//��
		for(; *p != ' ' && *p != '\0'; p++);	//�ԍ�
		for(; *p == ' '; p++);					//��
		for(r = p; *r != ' ' && *r != '\0'; r++);
		MailSize = (char *)LocalAlloc(LMEM_FIXED, r - p + 1);
		if(MailSize != NULL){
			for(s = MailSize; p < r; p++, s++){
				*s = *p;
			}
			*s = '\0';

			len = a2i(MailSize);
			if(UseRetr == 0){
				len = (len > 0 && len < (ListGetLine + HEAD_LINE) * LINE_LEN)
					? len : ((ListGetLine + HEAD_LINE) * LINE_LEN);
			}
		}
	}

	TOP_Recv = FALSE;
	if(InitMailBuf((len > 0) ? len : INIT_BUFSIZE) == FALSE){
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}

	if(UseRetr == 1){
		//�ꗗ�擾���ɖ{�������ׂă_�E�����[�h
		if(SendCommand(hWnd, soc, TEXT("RETR"), GetCnt, ErrStr) == FALSE){
			return POP_ERR;
		}
		return POP_RETR;
	}
	return SendTopCommand(hWnd, soc, GetCnt, ErrStr, ListGetLine, POP_TOP);
}


/******************************************************************************

	MailList_UIDL

	UIDL�̃��X�|���X�̉��

******************************************************************************/

static int MailList_UIDL(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	struct TPMAILITEM *tpMailItem;
	struct TPMAILITEM *tpLastMailItem;
	HWND hListView;
	int No;
	int i;

	//UIDL���X�|���X��1�s��
	if(UIDL_Recv == FALSE){
		SetStatusText(hWnd, buf);

		//���X�|���X�̉��
		if(CheckResponse(buf) == TRUE){
			if(InitUidlList(tpMailBox->MailCnt) == FALSE){
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return POP_ERR;
			}
			UIDL_Recv = TRUE;
			return POP_UIDL;
		}

		//UIDL���T�|�[�g����Ă��Ȃ��ꍇ��1���ڂ���擾
		if(ShowFlag == TRUE){
			ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LISTVIEW));
		}
		FreeMailItem(tpMailBox->tpMailItem, tpMailBox->MailItemCnt);
		LocalFree(tpMailBox->tpMailItem);
		tpMailBox->tpMailItem = NULL;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		tpMailBox->LastNo = 0;
		GetCnt = 1;
		SetItemCntStatusText(hWnd, tpMailBox);

		if(SendCommand(hWnd, soc, TEXT("LIST"), GetCnt, ErrStr) == FALSE){
			return POP_ERR;
		}
		return POP_LIST;
	}

	//UIDL�̏I���ł͂Ȃ��ꍇ
	if(*buf != '.' || *(buf + 1) != '\0'){
		//��M�������ۑ����Ă���
		if(SetUidlList(buf, buflen) == FALSE){
			FreeUidlList();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return POP_UIDL;
	}

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if(ShowFlag == TRUE){
		ListView_SetRedraw(hListView, FALSE);
	}
	SwitchCursor(FALSE);

	//���ݕ\������Ă��郁�[���ꗗ��UIDL���r���Ĕԍ���U�蒼��
	tpLastMailItem = NULL;
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if(tpMailItem == NULL){
			continue;
		}
		switch((No = CheckUIDL(tpMailItem->UIDL)))
		{
#ifdef UNICODE
		case -1:
			FreeUidlList();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			if(ShowFlag == TRUE){
				ListView_SetRedraw(hListView, TRUE);
			}
			SwitchCursor(TRUE);
			return POP_ERR;
#endif

		case 0:
			//UIDL�̃��X�g�ɑ��݂��Ȃ����[���͉������
			if(ShowFlag == TRUE){
				No = ListView_GetMemToItem(hListView, tpMailItem);
				ListView_DeleteItem(hListView, No);
			}
			FreeMailItem((tpMailBox->tpMailItem + i), 1);
			break;

		default:
			//���[���ԍ���ݒ�
			tpMailItem->No = No;
			tpLastMailItem = tpMailItem;
			break;
		}
	}
	FreeUidlList();
	//�폜���ꂽ���[�����ꗗ�������
	Item_Resize(tpMailBox);
	if(ShowFlag == TRUE){
		ListView_SetRedraw(hListView, TRUE);
	}
	SwitchCursor(TRUE);
	SetItemCntStatusText(hWnd, tpMailBox);

	NULLCHECK_FREE(tpMailBox->LastMessageId);
	tpMailBox->LastMessageId = NULL;
	tpMailBox->LastNo = 0;

	if(tpLastMailItem != NULL){
		tpMailBox->LastMessageId = AllocTcharToChar(tpLastMailItem->MessageID);
		tpMailBox->LastNo = tpLastMailItem->No;
	}

	GetCnt = tpMailBox->LastNo + 1;
	if(GetCnt > tpMailBox->MailCnt){
		return POP_QUIT;
	}
	if(SendCommand(hWnd, soc, TEXT("LIST"), GetCnt, ErrStr) == FALSE){
		return POP_ERR;
	}
	return POP_LIST;
}


/******************************************************************************

	MailList_Top

	TOP�̃��X�|���X�̉��

******************************************************************************/

static int MailList_Top(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	struct TPMAILITEM *tpMailItem;
	LV_ITEM lvi;
	HWND hListView;
	int i;
	int st;

	//TOP���X�|���X��1�s��
	if(TOP_Recv == FALSE){
		SetStatusText(hWnd, buf);

		//LIST �� . ���t���Ă���ꍇ�̓X�L�b�v����
		if(*buf == '.' && *(buf + 1) == '\0'){
			return POP_TOP;
		}
		//���X�|���X�̉��
		if(CheckResponse(buf) == TRUE){
			TOP_Recv = TRUE;
			return POP_TOP;
		}
		FreeMailBuf();

		if(GetCnt == tpMailBox->LastNo){
			UIDL_Recv = FALSE;
			SetSocStatusTextT(hWnd, TEXT("UIDL\r\n"), 1);
			if(SendBuf(soc, "UIDL\r\n") == -1){
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return POP_ERR;
			}
			return POP_UIDL;
		}

		if(UseRetr == 0){
			//TOP���T�|�[�g���Ă��Ȃ��\��������̂�RETR�𑗐M����
			UseRetr = 1;
			if(SendCommand(hWnd, soc, TEXT("RETR"), GetCnt, ErrStr) == FALSE){
				return POP_ERR;
			}
			return POP_RETR;
		}
		lstrcpy(ErrStr, STR_ERR_SOCK_RETR);
		StrCatN(ErrStr, buf, BUF_SIZE - 1);
		return POP_ERR;
	}

	//TOP�̏I���ł͂Ȃ��ꍇ
	if(*buf != '.' || *(buf + 1) != '\0'){
		//��M�w�b�_��ۑ����Ă���
		if(SetMailBuf(buf, buflen) == FALSE){
			FreeMailBuf();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return POP_TOP;
	}

	//�O��Ō�Ɏ�M�������[����Message-ID���`�F�b�N
	if(GetCnt == tpMailBox->LastNo){
		if(tpMailBox->LastMessageId == NULL){
			if(ShowFlag == TRUE){
				ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LISTVIEW));
			}
			FreeMailItem(tpMailBox->tpMailItem, tpMailBox->MailItemCnt);
			LocalFree(tpMailBox->tpMailItem);
			tpMailBox->tpMailItem = NULL;
			tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		}else{
			//�O��`�F�b�N���̍Ō�̃��[��
			return CheckLastMessageId(hWnd, soc, ErrStr, tpMailBox, ShowFlag);
		}
	}

	//��M�̍ő�A�C�e�������̃��������m�ۂ��Ă���
	if(LvSelect == FALSE){
		if(ShowFlag == TRUE){
			ListView_SetItemCount(GetDlgItem(hWnd, IDC_LISTVIEW), tpMailBox->MailCnt);
		}
		if(Item_SetItemCnt(tpMailBox, tpMailBox->MailCnt) == FALSE){
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
	}

	//�w�b�_����A�C�e�����쐬
	tpMailItem = Item_HeadToItem(tpMailBox, MailBuf, MailSize);
	if(tpMailItem == NULL){
		NULLCHECK_FREE(MailSize);
		MailSize = NULL;
		FreeMailBuf();
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	NULLCHECK_FREE(MailSize);
	MailSize = NULL;

	if((int)tpMailItem != -1){
		//�V���t���O�̏���
		if(LvSelect == FALSE && NewMail_Flag == FALSE && ShowMsgFlag == FALSE){
			for(i = 0; i < tpMailBox->MailItemCnt; i++){
				if(*(tpMailBox->tpMailItem + i) == NULL){
					continue;
				}
				(*(tpMailBox->tpMailItem + i))->New = FALSE;
			}
		}
		tpMailItem->New = TRUE;
		tpMailItem->Download = (UseRetr == 1) ? TRUE : FALSE;
		tpMailItem->No = GetCnt;

		if(ShowFlag == TRUE){
			hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
			//�V���̃I�[�o���C�}�X�N
			st = INDEXTOOVERLAYMASK(1);
			st |= ((tpMailItem->Multipart == TRUE) ? INDEXTOSTATEIMAGEMASK(1) : 0);
			if(LvSelect == FALSE && NewMail_Flag == FALSE && ShowMsgFlag == FALSE){
				//�S�A�C�e���̐V���̃I�[�o�[���C�}�X�N������
				ListView_SetItemState(hListView, -1, 0, LVIS_OVERLAYMASK);
				ListView_RedrawItems(hListView, 0, ListView_GetItemCount(hListView));

				//�V���ʒu�̑I��
				ListView_SetItemState(hListView, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
				st |= (LVIS_FOCUSED | LVIS_SELECTED);
			}
			st |= ((tpMailItem->Download == FALSE && tpMailItem->Status != ICON_DOWN && tpMailItem->Status != ICON_DEL)
				? LVIS_CUT : 0);

			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
			lvi.iItem = ListView_GetItemCount(hListView);
			lvi.iSubItem = 0;
			lvi.state = st;
			lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK | LVIS_CUT | LVIS_FOCUSED | LVIS_SELECTED;
			lvi.pszText = LPSTR_TEXTCALLBACK;
			lvi.cchTextMax = 0;
			lvi.iImage = I_IMAGECALLBACK;
			lvi.lParam = (long)tpMailItem;

			// ���X�g�r���[�ɃA�C�e����ǉ�����
			i = ListView_InsertItem(hListView, &lvi);
			if(LvSelect == FALSE){
				ListView_EnsureVisible(hListView, i, TRUE);
			}
			//��s���փX�N���[��
			if(RecvScroll == 1){
				SendMessage(hListView, WM_VSCROLL, SB_LINEDOWN, 0);
			}
			SetItemCntStatusText(hWnd, tpMailBox);
			EndThreadSortFlag = TRUE;
		}
		LvSelect = TRUE;

		//�V���J�E���g
		NewMailCnt++;
		tpMailBox->NewMail = TRUE;
	}

	//�Ō�Ɏ�M�������[���̔ԍ��ƃ��b�Z�[�WID
	tpMailBox->LastNo = GetCnt;

	NULLCHECK_FREE(tpMailBox->LastMessageId);
	tpMailBox->LastMessageId = Item_GetMessageId(MailBuf);
	if(tpMailBox->LastMessageId == NULL){
		FreeMailBuf();
		lstrcpy(ErrStr, STR_ERR_SOCK_NOMESSAGEID);
		return POP_ERR;
	}

	FreeMailBuf();

	//���̃w�b�_�̎擾
	GetCnt++;
	if(GetCnt > tpMailBox->MailCnt){
		return POP_QUIT;
	}

	if(SendCommand(hWnd, soc, TEXT("LIST"), GetCnt, ErrStr) == FALSE){
		return POP_ERR;
	}
	return POP_LIST;
}


/******************************************************************************

	MailFunc_Init

	���[���̃_�E�����[�h�A�폜���̏����̏�����

******************************************************************************/

static int MailFunc_Init(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	struct TPMAILITEM *tpMailItem;
	int size;

	//�폜�m�F�� RETR ���g�p����t���O��������
	DeleCheckRetr = FALSE;

	GetCnt =  Item_GetNextDonloadItem(tpMailBox, -1, &GetNo);
	if(GetCnt == -1){
		GetCnt =  Item_GetNextDeleteItem(tpMailBox, -1, &DeleNo);
		if(GetCnt == -1){
			return POP_QUIT;
		}
		TOP_Recv = FALSE;
		if(InitMailBuf(INIT_BUFSIZE) == FALSE){
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		//�폜���[���̃w�b�_��v��
		return SendTopCommand(hWnd, soc, DeleNo, ErrStr, 0, POP_TOP);
	}

	tpMailItem = *(tpMailBox->tpMailItem + GetCnt);
	if(tpMailItem == NULL){
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	size = _ttoi(tpMailItem->Size);

	TOP_Recv = FALSE;
	if(InitMailBuf((size <= 0) ? INIT_BUFSIZE : size) == FALSE){
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}

	//�S����M����R�}���h�𑗐M
	if(SendCommand(hWnd, soc, TEXT("RETR"), GetNo, ErrStr) == FALSE){
		return POP_ERR;
	}
	return POP_RETR;
}


/******************************************************************************

	MailDownload_Retr

	RETR�̃��X�|���X�̉��

******************************************************************************/

static int MailDownload_Retr(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	struct TPMAILITEM *tpMailItem;
	HWND hListView;
	int i, len;
	static int recvlen;
	static int recvcnt;

	if(TOP_Recv == FALSE){
		if(CheckResponse(buf) == FALSE){
			FreeMailBuf();
			lstrcpy(ErrStr, STR_ERR_SOCK_RETR);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		recvlen = 0;
		recvcnt = REDRAWCNT;
		SetSocStatusTextT(hWnd, STR_STATUS_RECV, 1);
		TOP_Recv = TRUE;
		return POP_RETR;
	}

	//RETR�̏I���ł͂Ȃ��ꍇ
	if(tstrcmp(buf, ".") != 0){
		//��M�w�b�_��ۑ����Ă���
		if(SetMailBuf(buf, buflen) == FALSE){
			FreeMailBuf();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		recvlen += buflen;
		recvcnt++;
		if(recvcnt > REDRAWCNT){
			recvcnt = 0;
			SetStatusRecvLen(hWnd, recvlen, STR_STATUS_SOCKINFO_RECV);
		}
		return POP_RETR;
	}

	GetCnt = Item_GetMailNoToItemIndex(tpMailBox, GetNo);
	if(GetCnt == -1){
		FreeMailBuf();
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return POP_ERR;
	}

	tpMailItem = *(tpMailBox->tpMailItem + GetCnt);
	if(tpMailItem == NULL){
		FreeMailBuf();
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}

	//���b�Z�[�WID�ŗv���������[�����ǂ����`�F�b�N����
	if(CheckMessageId(MailBuf, tpMailItem, ErrStr, tpMailBox) == FALSE){
		FreeMailBuf();
		return POP_ERR;
	}

	//�{�����擾
	Item_SetMailItem(tpMailItem, MailBuf, NULL);
	tpMailItem->Download = TRUE;
	FreeMailBuf();

	if(ShowFlag == TRUE){
		hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		//���X�g�r���[�̍X�V
		i = ListView_GetMemToItem(hListView, tpMailItem);
		if(i != -1){
			ListView_SetItemState(hListView, i, 0, LVIS_CUT);
			ListView_RedrawItems(hListView, i, i);
			UpdateWindow(hListView);
			SetItemCntStatusText(hWnd, tpMailBox);
		}
	}

	GetCnt =  Item_GetNextDonloadItem(tpMailBox, -1, &GetNo);
	if(GetCnt == -1){
		GetCnt =  Item_GetNextDeleteItem(tpMailBox, -1, &DeleNo);
		if(GetCnt == -1){
			return POP_QUIT;
		}
		TOP_Recv = FALSE;
		if(InitMailBuf(INIT_BUFSIZE) == FALSE){
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return SendTopCommand(hWnd, soc, DeleNo, ErrStr, 0, POP_TOP);
	}

	tpMailItem = *(tpMailBox->tpMailItem + GetCnt);
	if(tpMailItem == NULL){
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	len = _ttoi(tpMailItem->Size);

	//���̃w�b�_�̎擾
	TOP_Recv = FALSE;
	if(InitMailBuf((len <= 0) ? INIT_BUFSIZE : len) == FALSE){
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}

	//�S����M����R�}���h�𑗐M
	if(SendCommand(hWnd, soc, TEXT("RETR"), GetNo, ErrStr) == FALSE){
		return POP_ERR;
	}
	return POP_RETR;
}


/******************************************************************************

	MailDelete_Top

	�폜���[���m�F�pTOP�̃��X�|���X�̉��

******************************************************************************/

static int MailDelete_Top(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	struct TPMAILITEM *tpMailItem;

	if(TOP_Recv == FALSE){
		SetStatusText(hWnd, buf);

		if(CheckResponse(buf) == FALSE){
			FreeMailBuf();

			//�폜�m�F�� TOP �� RETR ���g�p�������ʗ������s�����ꍇ
			if(DeleCheckRetr == TRUE){
				lstrcpy(ErrStr, STR_ERR_SOCK_RETR);
				StrCatN(ErrStr, buf, BUF_SIZE - 1);
				return POP_ERR;
			}

			//�폜�m�F��TOP�Ŏ��s�����̂�RETR�ō폜�m�F���s��
			DeleCheckRetr = TRUE;

			GetCnt =  Item_GetMailNoToItemIndex(tpMailBox, DeleNo);
			if(GetCnt == -1){
				lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
				return POP_ERR;
			}
			if(InitMailBuf(INIT_BUFSIZE) == FALSE){
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return POP_ERR;
			}
			if(SendCommand(hWnd, soc, TEXT("RETR"), DeleNo, ErrStr) == FALSE){
				FreeMailBuf();
				return POP_ERR;
			}
			TOP_Recv = FALSE;
			return POP_TOP;

		}
		TOP_Recv = TRUE;
		return POP_TOP;
	}

	//TOP�̏I���ł͂Ȃ��ꍇ
	if(tstrcmp(buf, ".") != 0){
		//��M�w�b�_��ۑ����Ă���
		if(SetMailBuf(buf, buflen) == FALSE){
			FreeMailBuf();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return POP_TOP;
	}

	GetCnt =  Item_GetMailNoToItemIndex(tpMailBox, DeleNo);
	if(GetCnt == -1){
		FreeMailBuf();
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return POP_ERR;
	}

	tpMailItem = *(tpMailBox->tpMailItem + GetCnt);
	if(tpMailItem == NULL){
		FreeMailBuf();
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	//���b�Z�[�WID�ŗv���������[�����ǂ����`�F�b�N����
	if(CheckMessageId(MailBuf, tpMailItem, ErrStr, tpMailBox) == FALSE){
		FreeMailBuf();
		return POP_ERR;
	}

	FreeMailBuf();

	//�폜�R�}���h�̑��M
	if(SendCommand(hWnd, soc, TEXT("DELE"), DeleNo, ErrStr) == FALSE){
		return POP_ERR;
	}
	return POP_DELE;
}


/******************************************************************************

	MailDelete_Dele

	DELE�̃��X�|���X�̉��

******************************************************************************/

static int MailDelete_Dele(HWND hWnd, SOCKET soc, char *buf, int buflen,
						 TCHAR *ErrStr, struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	struct TPMAILITEM *tpMailItem;
	HWND hListView;
	int i, j;

	if(SocLog == 1) SetStatusText(hWnd, buf);

	if(CheckResponse(buf) == FALSE){
		lstrcpy(ErrStr, STR_ERR_SOCK_DELE);
		StrCatN(ErrStr, buf, BUF_SIZE - 1);
		SendBuf(soc, RSET);
		return POP_ERR;
	}

	GetCnt = Item_GetMailNoToItemIndex(tpMailBox, DeleNo);
	if(GetCnt == -1){
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		return POP_ERR;
	}
	GetCnt = Item_GetNextDeleteItem(tpMailBox, GetCnt, &DeleNo);
	if(GetCnt != -1){
		//���̍폜���[���̃w�b�_��v��
		TOP_Recv = FALSE;
		if(InitMailBuf(INIT_BUFSIZE) == FALSE){
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		if(DeleCheckRetr == TRUE){
			if(SendCommand(hWnd, soc, TEXT("RETR"), DeleNo, ErrStr) == FALSE){
				FreeMailBuf();
				return POP_ERR;
			}
			return POP_TOP;
		}
		return SendTopCommand(hWnd, soc, DeleNo, ErrStr, 0, POP_TOP);
	}

	//�폜���������������̂ō폜���ꂽ���[�������X�g�r���[�ƃ���������폜����
	//���X�g�r���[����폜
	if(ShowFlag == TRUE){
		hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		ListView_SetRedraw(hListView, FALSE);
		while((GetCnt =  ListView_GetNextDeleteItem(hListView, -1)) != -1){
			ListView_DeleteItem(hListView, GetCnt);
		}
		ListView_SetRedraw(hListView, TRUE);
	}
	//��������NULL�ɐݒ�
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if(tpMailItem == NULL || tpMailItem->Status != ICON_DEL){
			continue;
		}
		FreeMailItem((tpMailBox->tpMailItem + i), 1);

		//�폜�������[�������̃��[���̔ԍ������炷
		for(j = i + 1; j < tpMailBox->MailItemCnt; j++){
			tpMailItem = *(tpMailBox->tpMailItem + j);
			if(tpMailItem == NULL){
				continue;
			}
			tpMailItem->No--;
		}
		tpMailBox->MailCnt--;
		tpMailBox->LastNo--;
	}
	//�Ō�̃��b�Z�[�W���폜����Ă���ꍇ�͈ꗗ�擾�p���b�Z�[�WID��ύX����
	if(*(tpMailBox->tpMailItem + tpMailBox->MailItemCnt - 1) == NULL){
		for(i = tpMailBox->MailItemCnt - 1; i >= 0; i--){
			tpMailItem = *(tpMailBox->tpMailItem + i);
			if(tpMailItem == NULL){
				continue;
			}
			if(tpMailItem->MessageID == NULL){
				break;
			}

			NULLCHECK_FREE(tpMailBox->LastMessageId);
			tpMailBox->LastMessageId = AllocTcharToChar(tpMailItem->MessageID);
			tpMailBox->LastNo = tpMailItem->No;
			break;
		}
	}
	Item_Resize(tpMailBox);
	SetItemCntStatusText(hWnd, tpMailBox);
	return POP_QUIT;
}


/******************************************************************************

	LoginProc

	���O�C���̏������s��

******************************************************************************/

static int LoginProc(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr,
					 struct TPMAILBOX *tpMailBox)
{
	TCHAR *wbuf;
	static TCHAR *pass;
	int ret = POP_ERR;

	if(SocLog == 1) SetStatusText(hWnd, buf);

	switch(MailFlag){
	case POP_START:
		if(CheckResponse(buf) == FALSE){
			lstrcpy(ErrStr, STR_ERR_SOCK_RESPONSE);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		if(tpMailBox->User == NULL || *tpMailBox->User == TEXT('\0')){
			lstrcpy(ErrStr, STR_ERR_SOCK_NOUSERID);
			return POP_ERR;
		}

		pass = tpMailBox->Pass;
		if(pass == NULL || *pass == TEXT('\0')){
			pass = tpMailBox->TmpPass;
		}
		if(pass == NULL || *pass == TEXT('\0')){
			pass = g_Pass;
		}
		if(pass == NULL || *pass == TEXT('\0')){
			lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
			return POP_ERR;
		}

		//APOP�ɂ��F��
		if(tpMailBox->APOP == 1){
			wbuf = CreateApopString(buf, ErrStr, tpMailBox, pass);
			if(wbuf == NULL){
				return POP_ERR;
			}

			SetSocStatusTextT(hWnd, TEXT("APOP ****"), 1);

			if(TSendBuf(soc, wbuf) == -1){
				LocalFree(wbuf);
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return POP_ERR;
			}
			LocalFree(wbuf);
			ret = POP_PASS;
			break;
		}

		//USER �̑��M
		wbuf = (TCHAR *)LocalAlloc(LMEM_FIXED,
			sizeof(TCHAR) * (lstrlen(TEXT("USER \r\n")) + lstrlen(tpMailBox->User) + 1));
		if(wbuf == NULL){
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		TStrJoin(wbuf, TEXT("USER "), tpMailBox->User, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, wbuf, 1);
		if(TSendBuf(soc, wbuf) == -1){
			LocalFree(wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return POP_ERR;
		}
		LocalFree(wbuf);

		ret = POP_USER;
		break;

	case POP_USER:
		if(CheckResponse(buf) == FALSE){
			lstrcpy(ErrStr, STR_ERR_SOCK_ACCOUNT);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}

		//PASS �̑��M
		wbuf = (TCHAR *)LocalAlloc(LMEM_FIXED,
			sizeof(TCHAR) * (lstrlen(TEXT("PASS \r\n")) + lstrlen(pass) + 1));
		if(wbuf == NULL){
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		TStrJoin(wbuf, TEXT("PASS "), pass, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, TEXT("PASS ****"), 1);
		if(TSendBuf(soc, wbuf) == -1){
			LocalFree(wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return POP_ERR;
		}
		LocalFree(wbuf);

		ret = POP_PASS;
		break;

	case POP_PASS:
		if(CheckResponse(buf) == FALSE){
			lstrcpy(ErrStr, STR_ERR_SOCK_BADPASSWORD);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		ret = POP_LOGIN;
		break;
	}
	return ret;
}


/******************************************************************************

	ListPopProc

	���[���ꗗ�擾�̏��� (�V���`�F�b�N)

******************************************************************************/

BOOL ListPopProc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr,
						  struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	switch(MailFlag)
	{
	case POP_START:
	case POP_USER:
	case POP_PASS:
		MailFlag = LoginProc(hWnd, soc, buf, len, ErrStr, tpMailBox);
		if(MailFlag == POP_LOGIN){
			if(PopBeforeSmtpFlag == TRUE){
				MailFlag = POP_QUIT;
			}else{
				SetSocStatusTextT(hWnd, TEXT("STAT\r\n"), 1);
				if(SendBuf(soc, "STAT\r\n") == -1){
					lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
					return FALSE;
				}
				MailFlag = POP_STAT;
			}
		}
		break;

	case POP_STAT:
		DateAdd(NULL, NULL);	//�^�C���]�[���̏�����
		MailFlag = MailList_Stat(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_LIST:
		MailFlag = MailList_List(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_UIDL:
		MailFlag = MailList_UIDL(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_TOP:
	case POP_RETR:
		MailFlag = MailList_Top(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_QUIT:
		if(StrCmpNI(buf, "+OK", 3) == 0 || StrCmpNI(buf, "-ERR", 3) == 0){
			SetStatusText(hWnd, buf);
		}
		return TRUE;
	}

	switch(MailFlag)
	{
	case POP_ERR:
		Item_Resize(tpMailBox);
		NULLCHECK_FREE(MailSize);
		MailSize = NULL;
		SendBuf(soc, QUIT);
		return FALSE;

	case POP_QUIT:
		Item_Resize(tpMailBox);
		NULLCHECK_FREE(MailSize);
		MailSize = NULL;
		SetSocStatusTextT(hWnd, TEXT(QUIT), 1);
		if(SendBuf(soc, QUIT) == -1){
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		break;
	}
	return TRUE;
}


/******************************************************************************

	DownLoadPopProc

	RETR��DELE�̏��� (���s)

******************************************************************************/

BOOL DownLoadPopProc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr,
							  struct TPMAILBOX *tpMailBox, BOOL ShowFlag)
{
	switch(MailFlag)
	{
	case POP_START:
	case POP_USER:
	case POP_PASS:
		MailFlag = LoginProc(hWnd, soc, buf, len, ErrStr, tpMailBox);
		if(MailFlag == POP_LOGIN){
			MailFlag = MailFunc_Init(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		}
		break;

	case POP_RETR:
		MailFlag = MailDownload_Retr(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_TOP:
		MailFlag = MailDelete_Top(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_DELE:
		MailFlag = MailDelete_Dele(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_QUIT:
		if(StrCmpNI(buf, "+OK", 3) == 0 || StrCmpNI(buf, "-ERR", 3) == 0){
			SetStatusText(hWnd, buf);
		}
		return TRUE;
	}

	switch(MailFlag)
	{
	case POP_ERR:
		SendBuf(soc, QUIT);
		return FALSE;

	case POP_QUIT:
		SetSocStatusTextT(hWnd, TEXT(QUIT), 1);
		if(SendBuf(soc, QUIT) == -1){
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		break;
	}
	return TRUE;
}
/* End of source */
