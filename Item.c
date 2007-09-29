/**************************************************************************

	nPOP

	Item.c

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

extern int ListGetLine;
extern int ShowHeader;
extern BOOL KeyShowHeader;

extern struct TPMAILBOX *MailBox;
extern struct TPMAILBOX *AddressBox;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static void Item_GetContentT(TCHAR *buf, TCHAR *str, TCHAR **ret);
static int Item_GetContentInt(TCHAR *buf, TCHAR *str, int DefaultRet);
static int Item_GetMultiContent(char *buf, char *str, char **ret);
static void GetMimeContent(char *buf, char *Head, TCHAR **ret, BOOL MultiFlag);
static void Item_SetMailBody(struct TPMAILITEM *tpMailItem, char *buf);
static BOOL FilterCheckItem(char *buf, TCHAR *FHead, TCHAR *Fcontent);
static BOOL FilterCheck(struct TPMAILBOX *tpMailBox, char *buf);


/******************************************************************************

	Item_SetItemCnt

	アイテム数分のメモリを確保

******************************************************************************/

BOOL Item_SetItemCnt(struct TPMAILBOX *tpMailBox, int i)
{
	struct TPMAILITEM **tpMailList;

	if(i <= tpMailBox->MailItemCnt){
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt;
		return TRUE;
	}
	tpMailList = (struct TPMAILITEM **)LocalAlloc(LPTR, sizeof(struct TPMAILITEM *) * i);
	if(tpMailList == NULL){
		return FALSE;
	}
	if(tpMailBox->tpMailItem != NULL){
		tCopyMemory(tpMailList, tpMailBox->tpMailItem,
			sizeof(struct TPMAILITEM *) * tpMailBox->MailItemCnt);
	}

	NULLCHECK_FREE(tpMailBox->tpMailItem);
	tpMailBox->tpMailItem = tpMailList;
	tpMailBox->AllocCnt = i;
	return TRUE;
}


/******************************************************************************

	Item_Add

	アイテムの追加

******************************************************************************/

BOOL Item_Add(struct TPMAILBOX *tpMailBox, struct TPMAILITEM *tpNewMailItem)
{
	struct TPMAILITEM **tpMailList = tpMailBox->tpMailItem;

	if(tpMailBox->AllocCnt <= tpMailBox->MailItemCnt){
		tpMailList = (struct TPMAILITEM **)LocalAlloc(LMEM_FIXED,
			sizeof(struct TPMAILITEM *) * (tpMailBox->MailItemCnt + 1));
		if(tpMailList == NULL){
			return FALSE;
		}
		if(tpMailBox->tpMailItem != NULL){
			tCopyMemory(tpMailList, tpMailBox->tpMailItem,
				sizeof(struct TPMAILITEM *) * tpMailBox->MailItemCnt);
			LocalFree(tpMailBox->tpMailItem);
		}
		tpMailBox->tpMailItem = tpMailList;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt + 1;
	}
	*(tpMailList + tpMailBox->MailItemCnt) = tpNewMailItem;
	tpMailBox->MailItemCnt++;
	if(*tpMailBox->tpMailItem != NULL){
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return TRUE;
}


/******************************************************************************

	CopyItem

	アイテムのコピー

******************************************************************************/

void CopyItem(struct TPMAILITEM *tpFromNewMailItem, struct TPMAILITEM *tpToMailItem)
{
	tCopyMemory(tpToMailItem, tpFromNewMailItem, sizeof(struct TPMAILITEM));
	tpToMailItem->Status = tpToMailItem->MailStatus;
	tpToMailItem->New = FALSE;
	tpToMailItem->No = 0;
	tpToMailItem->UIDL = NULL;

	tpToMailItem->From = AllocCopy(tpFromNewMailItem->From);
	tpToMailItem->To = AllocCopy(tpFromNewMailItem->To);
	tpToMailItem->Cc = AllocCopy(tpFromNewMailItem->Cc);
	tpToMailItem->Bcc = AllocCopy(tpFromNewMailItem->Bcc);
	tpToMailItem->Subject = AllocCopy(tpFromNewMailItem->Subject);
	tpToMailItem->Date = AllocCopy(tpFromNewMailItem->Date);
	tpToMailItem->Size = AllocCopy(tpFromNewMailItem->Size);
	tpToMailItem->ReplyTo = AllocCopy(tpFromNewMailItem->ReplyTo);
	tpToMailItem->ContentType = AllocCopy(tpFromNewMailItem->ContentType);
	tpToMailItem->Encoding = AllocCopy(tpFromNewMailItem->Encoding);
	tpToMailItem->MessageID = AllocCopy(tpFromNewMailItem->MessageID);
	tpToMailItem->InReplyTo = AllocCopy(tpFromNewMailItem->InReplyTo);
	tpToMailItem->References = AllocCopy(tpFromNewMailItem->References);
	tpToMailItem->Body = AllocCopy(tpFromNewMailItem->Body);
	tpToMailItem->MailBox = AllocCopy(tpFromNewMailItem->MailBox);
	tpToMailItem->Attach = AllocCopy(tpFromNewMailItem->Attach);
}


/******************************************************************************

	Item_CopyMailBox

	アイテムをメールボックスに追加

******************************************************************************/

struct TPMAILITEM *Item_CopyMailBox(struct TPMAILBOX *tpMailBox, struct TPMAILITEM *tpNewMailItem,
			   TCHAR *MailBoxName, BOOL SendClear)
{
	struct TPMAILITEM **tpMailList;
	int i = 0;

	tpMailList = (struct TPMAILITEM **)LocalAlloc(LPTR,
		sizeof(struct TPMAILITEM *) * (tpMailBox->MailItemCnt + 1));
	if(tpMailList == NULL){
		return NULL;
	}
	if(tpMailBox->tpMailItem != NULL){
		for(i = 0; i < tpMailBox->MailItemCnt; i++){
			*(tpMailList + i) = *(tpMailBox->tpMailItem + i);
		}
	}

	*(tpMailList + i) = (struct TPMAILITEM *)LocalAlloc(LPTR, sizeof(struct TPMAILITEM));
	if(*(tpMailList + i) == NULL){
		LocalFree(tpMailList);
		return NULL;
	}
	CopyItem(tpNewMailItem, *(tpMailList + i));
	if((*(tpMailList + i))->MailBox == NULL){
		(*(tpMailList + i))->MailBox = AllocCopy(MailBoxName);
	}
	if(SendClear == TRUE){
		(*(tpMailList + i))->MailStatus = (*(tpMailList + i))->Status = ICON_NON;
		NULLCHECK_FREE((*(tpMailList + i))->Date);
		(*(tpMailList + i))->Date = NULL;
		NULLCHECK_FREE((*(tpMailList + i))->MessageID);
		(*(tpMailList + i))->MessageID = NULL;
		(*(tpMailList + i))->hEditWnd = NULL;
	}

	NULLCHECK_FREE(tpMailBox->tpMailItem);
	tpMailBox->tpMailItem = tpMailList;
	tpMailBox->MailItemCnt++;
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt;
	if(*tpMailBox->tpMailItem != NULL){
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return *(tpMailList + i);
}


/******************************************************************************

	Item_Resize

	アイテム情報の整理

******************************************************************************/

BOOL Item_Resize(struct TPMAILBOX *tpMailBox)
{
	struct TPMAILITEM **tpMailList;
	int i, cnt = 0;

	if(tpMailBox->tpMailItem == NULL){
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		return FALSE;
	}

	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == NULL){
			continue;
		}
		cnt++;
	}

	tpMailList = (struct TPMAILITEM **)LocalAlloc(LPTR, sizeof(struct TPMAILITEM *) * cnt);
	if(tpMailList == NULL){
		NULLCHECK_FREE(tpMailBox->tpMailItem);
		tpMailBox->tpMailItem = NULL;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		return FALSE;
	}
	cnt = 0;
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == NULL){
			continue;
		}
		*(tpMailList + cnt) = *(tpMailBox->tpMailItem + i);
		cnt++;
	}

	NULLCHECK_FREE(tpMailBox->tpMailItem);
	tpMailBox->tpMailItem = tpMailList;
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = cnt;
	if(cnt != 0 && *tpMailBox->tpMailItem != NULL){
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return TRUE;
}


/******************************************************************************

	FreeMailItem

	メールアイテムの解放

******************************************************************************/

void FreeMailItem(struct TPMAILITEM **tpFreeMailItem, int Cnt)
{
	int i;

	for(i = 0; i < Cnt; i++){
		if(*(tpFreeMailItem + i) == NULL){
			continue;
		}
		if((*(tpFreeMailItem + i))->hEditWnd != NULL){
			(*(tpFreeMailItem + i)) = NULL;
			continue;
		}
		NULLCHECK_FREE((*(tpFreeMailItem + i))->From);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->To);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Cc);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Bcc);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Date);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Size);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Subject);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->ReplyTo);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->ContentType);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Encoding);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->MessageID);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->UIDL);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->InReplyTo);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->References);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Body);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->MailBox);
		NULLCHECK_FREE((*(tpFreeMailItem + i))->Attach);

		LocalFree(*(tpFreeMailItem + i));
		(*(tpFreeMailItem + i)) = NULL;
	}
}


/******************************************************************************

	Item_GetContent

	コンテンツの取得

******************************************************************************/

int Item_GetContent(char *buf, char *str, char **ret)
{
	char *p;
	int len;

	//位置の取得
	p = GetHeaderStringPoint(buf, str);
	if(p == NULL){
		*ret = NULL;
		return 0;
	}
	//サイズの取得
	len = GetHeaderStringSize(p, FALSE);
	*ret = (char *)LocalAlloc(LMEM_FIXED, len + 1);
	if(*ret == NULL){
		return 0;
	}
	GetHeaderString(p, *ret, FALSE);
	return len;
}


/******************************************************************************

	Item_GetMultiContent

	複数ある場合は一つにまとめてコンテンツの取得

******************************************************************************/

static int Item_GetMultiContent(char *buf, char *str, char **ret)
{
	char *tmp;
	char *p, *r;
	int len;
	int rLen = 0;

	*ret = NULL;
	p = buf;
	while(1){
		//位置の取得
		p = GetHeaderStringPoint(p, str);
		if(p == NULL){
			return rLen;
		}
		//サイズの取得
		len = GetHeaderStringSize(p, FALSE);
		if(*ret != NULL){
			r = tmp = (char *)LocalAlloc(LMEM_FIXED, rLen + len + 2);
			if(tmp == NULL){
				return rLen;
			}
			r = StrCpy(r, *ret);
			r = StrCpy(r, ",");
			LocalFree(*ret);
			*ret = tmp;
			rLen += (len + 1);
		}else{
			r = *ret = (char *)LocalAlloc(LMEM_FIXED, len + 1);
			if(*ret == NULL){
				return rLen;
			}
			rLen = len;
		}
		GetHeaderString(p, r, FALSE);
		p += len;
	}
}


/******************************************************************************

	GetMessageId

	メッセージIDの取得

******************************************************************************/

char *Item_GetMessageId(char *buf)
{
	char *Content;
	char *p, *p1, *p2;
	int len1, len2;

	Content = NULL;
	Item_GetContent(buf, HEAD_MESSAGEID, &Content);
	TrimMessageId(Content);
	if(Content != NULL && *Content != '\0'){
		return Content;
	}
	NULLCHECK_FREE(Content);
	//Message-Idを取得できなかったらUIDLを持ってくる
	Item_GetContent(buf, HEAD_X_UIDL, &Content);
	if(Content != NULL && *Content != '\0'){
		return Content;
	}

	NULLCHECK_FREE(Content);
	Content = NULL;

	len1 = Item_GetContent(buf, HEAD_DATE, &p1);
	len2 = Item_GetContent(buf, HEAD_FROM, &p2);
	Content = (char *)LocalAlloc(LMEM_FIXED, len1 + len2 + 2);
	if(Content != NULL){
		p = Content;
		if(p1 != NULL){
			p = StrCpy(p, p1);
		}
		p = StrCpy(p, ".");
		if(p2 != NULL){
			p = StrCpy(p, p2);
		}
	}
	NULLCHECK_FREE(p1);
	NULLCHECK_FREE(p2);
	return Content;
}


/******************************************************************************

	GetMimeContent

	ヘッダのコンテンツを取得してMIMEデコードを行う

******************************************************************************/

static void GetMimeContent(char *buf, char *Head, TCHAR **ret, BOOL MultiFlag)
{
	char *Content;
	int len;

	*ret = NULL;

	len = ((MultiFlag == TRUE) ? Item_GetMultiContent : Item_GetContent)(buf, Head, &Content);
	if(Content != NULL){
#ifdef UNICODE
		char *dcode;

		dcode = (char *)LocalAlloc(LMEM_FIXED, len + 1);
		if(dcode != NULL){
			MIMEdecode(Content, dcode);
			*ret = AllocCharToTchar(dcode);
			LocalFree(dcode);
		}
#else
		*ret = (char *)LocalAlloc(LMEM_FIXED, len + 1);
		if(*ret != NULL){
			MIMEdecode(Content, *ret);
		}
#endif
		LocalFree(Content);
	}
}


/******************************************************************************

	Item_SetMailBody

	アイテムに本文を設定

******************************************************************************/

static void Item_SetMailBody(struct TPMAILITEM *tpMailItem, char *buf)
{
	char *p, *r;
	int Len;
	int HdSize;

	p = GetBodyPointa(buf);
	if(p != NULL && *p != '\0'){
		//デコード
		r = DecodeBodyTransfer(tpMailItem, p);
		if(r == NULL){
			return;
		}
		Len = CharToTcharSize(r);

		//ヘッダを表示する設定の場合
		HdSize = (ShowHeader == 1 || KeyShowHeader == TRUE) ? (p - buf) : 0;

		NULLCHECK_FREE(tpMailItem->Body);
		tpMailItem->Body = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (Len + HdSize + 1));
		if(tpMailItem->Body != NULL){
			if(ShowHeader == 1 || KeyShowHeader == TRUE){
				//ヘッダ
				CharToTchar(buf, tpMailItem->Body, HdSize + 1);
			}
			//本文
			CharToTchar(r, tpMailItem->Body + HdSize, Len);
		}
		LocalFree(r);
		tpMailItem->Status = tpMailItem->MailStatus = ICON_MAIL;

	}else if(ShowHeader == 1 || KeyShowHeader == TRUE){
		//本文が存在しない場合はヘッダのみ設定
		NULLCHECK_FREE(tpMailItem->Body);
		tpMailItem->Body = AllocCharToTchar(buf);
		tpMailItem->Status = tpMailItem->MailStatus = ICON_MAIL;
	}
}


/******************************************************************************

	FilterCheckItem

	文字列のチェック

******************************************************************************/

static BOOL FilterCheckItem(char *buf, TCHAR *FHead, TCHAR *Fcontent)
{
	TCHAR *Content;
	BOOL ret;
#ifdef UNICODE
	char *head;
#endif

	if(Fcontent == NULL || *Fcontent == TEXT('\0')){
		return TRUE;
	}

#ifdef UNICODE
	head = AllocTcharToChar(FHead);
	if(head == NULL){
		return FALSE;
	}
	//コンテンツの取得
	GetMimeContent(buf, head, &Content, TRUE);
	LocalFree(head);
#else
	//コンテンツの取得
	GetMimeContent(buf, FHead, &Content, TRUE);
#endif
	if(Content == NULL){
		return StrMatch(Fcontent, TEXT(""));
	}

	//比較
	ret = StrMatch(Fcontent, Content);
	LocalFree(Content);
	return ret;
}


/******************************************************************************

	FilterCheck

	フィルタ文字列のチェック

	(*(tpMailBox->tpFilter + i))->Flag = 0 //and
	(*(tpMailBox->tpFilter + i))->Flag = 1 //not

******************************************************************************/

static int FilterCheck(struct TPMAILBOX *tpMailBox, char *buf)
{
	int RetFlag = 0;
	int fret;
	int i, j;

	if(tpMailBox->FilterEnable == 0 || tpMailBox->tpFilter == NULL ||
		buf == NULL || *buf == '\0'){
		return FILTER_RECV;
	}

	for(i = 0; i < tpMailBox->FilterCnt; i++){
		if(*(tpMailBox->tpFilter + i) == NULL ||
			(*(tpMailBox->tpFilter + i))->Enable == 0){
			continue;
		}

		//項目１のチェック
		if((*(tpMailBox->tpFilter + i))->Header1 == NULL ||
			*(*(tpMailBox->tpFilter + i))->Header1 == TEXT('\0')){
			continue;
		}
		if(FilterCheckItem(buf, (*(tpMailBox->tpFilter + i))->Header1,
			(*(tpMailBox->tpFilter + i))->Content1) == FALSE){
			continue;
		}

		if((*(tpMailBox->tpFilter + i))->Header2 == NULL ||
			*(*(tpMailBox->tpFilter + i))->Header2 == TEXT('\0') ||
			FilterCheckItem(buf, (*(tpMailBox->tpFilter + i))->Header2,
			(*(tpMailBox->tpFilter + i))->Content2) == TRUE){

			j = (*(tpMailBox->tpFilter + i))->Action;
			for(fret = 1; j > 0; j--){
				fret *= 2;
			}
			switch(fret)
			{
			case FILTER_UNRECV:
			case FILTER_RECV:
				//受信フラグ
				if(!(RetFlag & FILTER_RECV) && !(RetFlag & FILTER_UNRECV)){
					RetFlag |= fret;
				}
				break;

			case FILTER_DOWNLOADMARK:
			case FILTER_DELETEMARK:
				//マークフラグ
				if(!(RetFlag & FILTER_RECV) && !(RetFlag & FILTER_DOWNLOADMARK) && !(RetFlag & FILTER_DELETEMARK)){
					RetFlag |= fret;
				}
				break;

			default:
				if(!(RetFlag & FILTER_RECV)){
					RetFlag |= fret;
				}
				break;
			}
		}
	}
	return ((RetFlag == 0) ? FILTER_RECV : RetFlag);
}


/******************************************************************************

	Item_SetMailItem

	アイテムにヘッダと本文を設定

******************************************************************************/

BOOL Item_SetMailItem(struct TPMAILITEM *tpMailItem, char *buf, char *Size)
{
	TCHAR *msgid1 = NULL, *msgid2 = NULL, *t = NULL;
	char *Content;
#ifdef UNICODE
	char *dcode;
#endif

	//Subject
	GetMimeContent(buf, HEAD_SUBJECT, &tpMailItem->Subject, FALSE);
	//From
	GetMimeContent(buf, HEAD_FROM, &tpMailItem->From, FALSE);
	//To
	GetMimeContent(buf, HEAD_TO, &tpMailItem->To, TRUE);
	//Cc
	GetMimeContent(buf, HEAD_CC, &tpMailItem->Cc, TRUE);
	//Reply-To
	GetMimeContent(buf, HEAD_REPLYTO, &tpMailItem->ReplyTo, FALSE);

	//Content-Type
	GetMimeContent(buf, HEAD_CONTENTTYPE, &tpMailItem->ContentType, FALSE);
	if(tpMailItem->ContentType != NULL &&
		TStrCmpNI(tpMailItem->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0){
		tpMailItem->Multipart = TRUE;
	}else{
		//Content-Transfer-Encoding
#ifdef UNICODE
		Item_GetContent(buf, HEAD_ENCODING, &Content);
		if(Content != NULL){
			tpMailItem->Encoding = AllocCharToTchar(Content);
			LocalFree(Content);
		}
#else
		Item_GetContent(buf, HEAD_ENCODING, &tpMailItem->Encoding);
#endif
	}

	//Date
	Item_GetContent(buf, HEAD_DATE, &Content);
	if(Content != NULL){
#ifdef UNICODE
		dcode = (char *)LocalAlloc(LMEM_FIXED, BUF_SIZE);
		if(dcode != NULL){
			DateConv(Content, dcode);
			tpMailItem->Date = AllocCharToTchar(dcode);
			LocalFree(dcode);
		}
#else
		tpMailItem->Date = (char *)LocalAlloc(LMEM_FIXED, BUF_SIZE);
		if(tpMailItem->Date != NULL){
			DateConv(Content, tpMailItem->Date);
		}
#endif
		LocalFree(Content);
	}

	//Size
	if(Size != NULL){
		tpMailItem->Size = AllocCharToTchar(Size);
	}

	//Message-Id
#ifdef UNICODE
	Content = Item_GetMessageId(buf);
	if(Content != NULL){
		tpMailItem->MessageID = AllocCharToTchar(Content);
		LocalFree(Content);
	}
#else
	tpMailItem->MessageID = Item_GetMessageId(buf);
#endif

	//UIDL
#ifdef UNICODE
	Item_GetContent(buf, HEAD_X_UIDL, &Content);
	if(Content != NULL){
		tpMailItem->UIDL = AllocCharToTchar(Content);
		LocalFree(Content);
	}
#else
	Item_GetContent(buf, HEAD_X_UIDL, &tpMailItem->UIDL);
#endif

	//In-Reply-To
#ifdef UNICODE
	Item_GetContent(buf, HEAD_INREPLYTO, &Content);
	TrimMessageId(Content);
	if(Content != NULL){
		tpMailItem->InReplyTo = AllocCharToTchar(Content);
		LocalFree(Content);
	}
#else
	Item_GetContent(buf, HEAD_INREPLYTO, &tpMailItem->InReplyTo);
	TrimMessageId(tpMailItem->InReplyTo);
#endif

	//References
	Item_GetContent(buf, HEAD_REFERENCES, &Content);
	if(Content != NULL){
#ifdef UNICODE
		dcode = (char *)LocalAlloc(LMEM_FIXED, GetReferencesSize(Content, TRUE) + 1);
		if(dcode != NULL){
			ConvReferences(Content, dcode, FALSE);
			msgid1 = AllocCharToTchar(dcode);

			ConvReferences(Content, dcode, TRUE);
			msgid2 = AllocCharToTchar(dcode);
			LocalFree(dcode);
		}
#else
		msgid1 = (char *)LocalAlloc(LMEM_FIXED, GetReferencesSize(Content, FALSE) + 1);
		if(msgid1 != NULL){
			ConvReferences(Content, msgid1, FALSE);
		}

		msgid2 = (char *)LocalAlloc(LMEM_FIXED, GetReferencesSize(Content, TRUE) + 1);
		if(msgid2 != NULL){
			ConvReferences(Content, msgid2, TRUE);
		}
#endif
		if(msgid1 != NULL && msgid2 != NULL && TStrCmp(msgid1, msgid2) == 0){
			LocalFree(msgid2);
			msgid2 = NULL;
		}
		LocalFree(Content);
	}

	if(tpMailItem->InReplyTo == NULL || *tpMailItem->InReplyTo == TEXT('\0')){
		NULLCHECK_FREE(tpMailItem->InReplyTo);
		tpMailItem->InReplyTo = AllocCopy(msgid1);
		t = msgid2;
	}else{
		t = (msgid1 != NULL && TStrCmp(tpMailItem->InReplyTo, msgid1) != 0) ? msgid1 : msgid2;
	}
	tpMailItem->References = AllocCopy(t);
	NULLCHECK_FREE(msgid1);
	NULLCHECK_FREE(msgid2);

	//Body
	Item_SetMailBody(tpMailItem, buf);
	return TRUE;
}


/******************************************************************************

	Item_HeadToItem

	メールヘッダからアイテムを作成する

******************************************************************************/

struct TPMAILITEM *Item_HeadToItem(struct TPMAILBOX *tpMailBox, char *buf, char *Size)
{
	struct TPMAILITEM *tpMailItem;
	int fret;

	//フィルタをチェック
	fret = FilterCheck(tpMailBox, buf);
	if(fret == FILTER_UNRECV){
		return (struct TPMAILITEM *)-1;
	}

	//メール情報の確保
	tpMailItem = (struct TPMAILITEM *)LocalAlloc(LPTR, sizeof(struct TPMAILITEM));
	if(tpMailItem == NULL){
		return NULL;
	}
	//ヘッダと本文を設定
	Item_SetMailItem(tpMailItem, buf, Size);

	//メール情報のリストに追加
	if(!(fret & FILTER_UNRECV) && Item_Add(tpMailBox, tpMailItem) == -1){
		FreeMailItem(&tpMailItem, 1);
		return NULL;
	}

	//フィルタ動作設定
	//開封済み設定
	if(fret & FILTER_READICON && tpMailItem->MailStatus != ICON_NON){
		tpMailItem->Status = tpMailItem->MailStatus = ICON_READ;
	}
	//マーク設定
	if(fret & FILTER_DOWNLOADMARK){
		tpMailItem->Status = ICON_DOWN;
	}else if(fret & FILTER_DELETEMARK){
		tpMailItem->Status = ICON_DEL;
	}
	//保存箱へコピー
	if(fret & FILTER_SAVE &&
		tpMailItem->MailStatus != ICON_NON &&
		Item_FindThread(MailBox + MAILBOX_SAVE, tpMailItem->MessageID, (MailBox + MAILBOX_SAVE)->MailItemCnt) == -1){
		Item_CopyMailBox(MailBox + MAILBOX_SAVE, tpMailItem, tpMailBox->Name, FALSE);
	}
	if(fret & FILTER_UNRECV){
		//受信しないフラグが有効の場合は解放する
		FreeMailItem(&tpMailItem, 1);
		return (struct TPMAILITEM *)-1;
	}
	return tpMailItem;
}


/******************************************************************************

	Item_GetContentT

	コンテンツの取得

******************************************************************************/

static void Item_GetContentT(TCHAR *buf, TCHAR *str, TCHAR **ret)
{
	TCHAR *p;
	int len;

	//位置の取得
	p = GetHeaderStringPointT(buf, str);
	if(p == NULL){
		*ret = NULL;
		return;
	}
	//サイズの取得
	len = GetHeaderStringSizeT(p, TRUE);
	*ret = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(*ret == NULL){
		return;
	}
	GetHeaderStringT(p, *ret, TRUE);
}


/******************************************************************************

	Item_GetContentT

	コンテンツの取得

******************************************************************************/

static int Item_GetContentInt(TCHAR *buf, TCHAR *str, int DefaultRet)
{
	TCHAR *Content;
	int ret;

	Item_GetContentT(buf, str, &Content);
	if(Content == NULL){
		return DefaultRet;
	}
	ret = _ttoi(Content);
	LocalFree(Content);
	return ret;
}


/******************************************************************************

	Item_StringToItem

	文字列からアイテムを作成する

******************************************************************************/

struct TPMAILITEM *Item_StringToItem(struct TPMAILBOX *tpMailBox, TCHAR *buf)
{
	struct TPMAILITEM *tpMailItem;
	int i;

	tpMailItem = (struct TPMAILITEM *)LocalAlloc(LPTR, sizeof(struct TPMAILITEM));
	if(tpMailItem == NULL){
		return NULL;
	}

	Item_GetContentT(buf, TEXT(HEAD_SUBJECT), &tpMailItem->Subject);
	Item_GetContentT(buf, TEXT(HEAD_FROM), &tpMailItem->From);
	Item_GetContentT(buf, TEXT(HEAD_TO), &tpMailItem->To);
	Item_GetContentT(buf, TEXT(HEAD_CC), &tpMailItem->Cc);
	Item_GetContentT(buf, TEXT(HEAD_BCC), &tpMailItem->Bcc);
	Item_GetContentT(buf, TEXT(HEAD_DATE), &tpMailItem->Date);
	Item_GetContentT(buf, TEXT(HEAD_SIZE), &tpMailItem->Size);
	Item_GetContentT(buf, TEXT(HEAD_REPLYTO), &tpMailItem->ReplyTo);
	Item_GetContentT(buf, TEXT(HEAD_CONTENTTYPE), &tpMailItem->ContentType);
	Item_GetContentT(buf, TEXT(HEAD_ENCODING), &tpMailItem->Encoding);
	Item_GetContentT(buf, TEXT(HEAD_MESSAGEID), &tpMailItem->MessageID);
	Item_GetContentT(buf, TEXT(HEAD_INREPLYTO), &tpMailItem->InReplyTo);
	Item_GetContentT(buf, TEXT(HEAD_REFERENCES), &tpMailItem->References);
	Item_GetContentT(buf, TEXT(HEAD_X_UIDL), &tpMailItem->UIDL);
	Item_GetContentT(buf, TEXT(HEAD_X_MAILBOX), &tpMailItem->MailBox);
	Item_GetContentT(buf, TEXT(HEAD_X_ATTACH), &tpMailItem->Attach);

	//No
	tpMailItem->No = Item_GetContentInt(buf, TEXT(HEAD_X_NO), -1);
	if(tpMailItem->No == -1){
		tpMailItem->No = Item_GetContentInt(buf, TEXT(HEAD_X_NO_OLD), 0);
	}
	//MailStatus
	tpMailItem->MailStatus = Item_GetContentInt(buf, TEXT(HEAD_X_MSTATUS), -1);
	if(tpMailItem->MailStatus == -1){
		tpMailItem->MailStatus = Item_GetContentInt(buf, TEXT(HEAD_X_MSTATUS_OLD), 0);
	}
	//MarkStatus
	i = Item_GetContentInt(buf, TEXT(HEAD_X_STATUS), -1);
	tpMailItem->Status = (i != -1) ? i : tpMailItem->MailStatus;
	//Download
	tpMailItem->Download = Item_GetContentInt(buf, TEXT(HEAD_X_DOWNFLAG), -1);
	if(tpMailItem->Download == -1){
		tpMailItem->Download = Item_GetContentInt(buf, TEXT(HEAD_X_DOWNFLAG_OLD), 0);
	}

	//Multipart
	if(tpMailItem->Attach != NULL || (tpMailItem->ContentType != NULL &&
		TStrCmpNI(tpMailItem->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0)){
		tpMailItem->Multipart = TRUE;
	}
	return tpMailItem;
}


/******************************************************************************

	Item_GetStringSize

	メールの保存文字列のサイズ取得

******************************************************************************/

int Item_GetStringSize(struct TPMAILITEM *tpMailItem, BOOL BodyFlag)
{
	TCHAR X_No[10], X_Mstatus[10], X_Status[10], X_Downflag[10];
	int len = 0;

	wsprintf(X_No, TEXT("%d"), tpMailItem->No);
	wsprintf(X_Mstatus, TEXT("%d"), tpMailItem->MailStatus);
	wsprintf(X_Status, TEXT("%d"), tpMailItem->Status);
	wsprintf(X_Downflag, TEXT("%d"), tpMailItem->Download);

	len += GetSaveHeaderStringSize(TEXT(HEAD_FROM), tpMailItem->From);
	len += GetSaveHeaderStringSize(TEXT(HEAD_TO), tpMailItem->To);
	len += GetSaveHeaderStringSize(TEXT(HEAD_CC), tpMailItem->Cc);
	len += GetSaveHeaderStringSize(TEXT(HEAD_BCC), tpMailItem->Bcc);
	len += GetSaveHeaderStringSize(TEXT(HEAD_DATE), tpMailItem->Date);
	len += GetSaveHeaderStringSize(TEXT(HEAD_SUBJECT), tpMailItem->Subject);
	len += GetSaveHeaderStringSize(TEXT(HEAD_SIZE), tpMailItem->Size);
	len += GetSaveHeaderStringSize(TEXT(HEAD_REPLYTO), tpMailItem->ReplyTo);
	len += GetSaveHeaderStringSize(TEXT(HEAD_CONTENTTYPE), tpMailItem->ContentType);
	len += GetSaveHeaderStringSize(TEXT(HEAD_ENCODING), tpMailItem->Encoding);
	len += GetSaveHeaderStringSize(TEXT(HEAD_MESSAGEID), tpMailItem->MessageID);
	len += GetSaveHeaderStringSize(TEXT(HEAD_INREPLYTO), tpMailItem->InReplyTo);
	len += GetSaveHeaderStringSize(TEXT(HEAD_REFERENCES), tpMailItem->References);
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_UIDL), tpMailItem->UIDL);
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_MAILBOX), tpMailItem->MailBox);
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_ATTACH), tpMailItem->Attach);
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_NO), X_No);
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_MSTATUS), X_Mstatus);
	if(tpMailItem->MailStatus != tpMailItem->Status){
		len += GetSaveHeaderStringSize(TEXT(HEAD_X_STATUS), X_Status);
	}
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_DOWNFLAG), X_Downflag);
	len += 2;

	if(BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != TEXT('\0')){
		len += lstrlen(tpMailItem->Body);
	}
	len += 5;
	return len;
}


/******************************************************************************

	Item_GetString

	メールの保存文字列の取得

******************************************************************************/

TCHAR *Item_GetString(TCHAR *buf, struct TPMAILITEM *tpMailItem, BOOL BodyFlag)
{
	TCHAR *p = buf;
	TCHAR X_No[10], X_Mstatus[10], X_Status[10], X_Downflag[10];

	wsprintf(X_No, TEXT("%d"), tpMailItem->No);
	wsprintf(X_Mstatus, TEXT("%d"), tpMailItem->MailStatus);
	wsprintf(X_Status, TEXT("%d"), tpMailItem->Status);
	wsprintf(X_Downflag, TEXT("%d"), tpMailItem->Download);

	p = SaveHeaderString(TEXT(HEAD_FROM), tpMailItem->From, p);
	p = SaveHeaderString(TEXT(HEAD_TO), tpMailItem->To, p);
	p = SaveHeaderString(TEXT(HEAD_CC), tpMailItem->Cc, p);
	p = SaveHeaderString(TEXT(HEAD_BCC), tpMailItem->Bcc, p);
	p = SaveHeaderString(TEXT(HEAD_DATE), tpMailItem->Date, p);
	p = SaveHeaderString(TEXT(HEAD_SUBJECT), tpMailItem->Subject, p);
	p = SaveHeaderString(TEXT(HEAD_SIZE), tpMailItem->Size, p);
	p = SaveHeaderString(TEXT(HEAD_REPLYTO), tpMailItem->ReplyTo, p);
	p = SaveHeaderString(TEXT(HEAD_CONTENTTYPE), tpMailItem->ContentType, p);
	p = SaveHeaderString(TEXT(HEAD_ENCODING), tpMailItem->Encoding, p);
	p = SaveHeaderString(TEXT(HEAD_MESSAGEID), tpMailItem->MessageID, p);
	p = SaveHeaderString(TEXT(HEAD_INREPLYTO), tpMailItem->InReplyTo, p);
	p = SaveHeaderString(TEXT(HEAD_REFERENCES), tpMailItem->References, p);
	p = SaveHeaderString(TEXT(HEAD_X_UIDL), tpMailItem->UIDL, p);
	p = SaveHeaderString(TEXT(HEAD_X_MAILBOX), tpMailItem->MailBox, p);
	p = SaveHeaderString(TEXT(HEAD_X_ATTACH), tpMailItem->Attach, p);
	p = SaveHeaderString(TEXT(HEAD_X_NO), X_No, p);
	p = SaveHeaderString(TEXT(HEAD_X_MSTATUS), X_Mstatus, p);
	if(tpMailItem->MailStatus != tpMailItem->Status){
		p = SaveHeaderString(TEXT(HEAD_X_STATUS), X_Status, p);
	}
	p = SaveHeaderString(TEXT(HEAD_X_DOWNFLAG), X_Downflag, p);
	p = TStrCpy(p, TEXT("\r\n"));

	if(BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != TEXT('\0')){
		p = TStrCpy(p, tpMailItem->Body);
	}
	p = TStrCpy(p, TEXT("\r\n.\r\n"));
	return p;
}


/******************************************************************************

	Item_GetNextDonloadItem

	ダウンロードマークのアイテムのインデックスを取得

******************************************************************************/

int Item_GetNextDonloadItem(struct TPMAILBOX *tpMailBox, int Index, int *No)
{
	struct TPMAILITEM *tpMailItem;
	int i;

	for(i = Index + 1; i < tpMailBox->MailItemCnt; i++){
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if(tpMailItem == NULL){
			continue;
		}
		if(tpMailItem->Status == ICON_DOWN){
			if(No != NULL){
				*No = tpMailItem->No;
			}
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	Item_GetNextSendItem

	送信マークの付いたアイテムのインデックスを取得

******************************************************************************/

int Item_GetNextSendItem(struct TPMAILBOX *tpMailBox, int Index, int *MailBoxIndex)
{
	struct TPMAILITEM *tpMailItem;
	int BoxIndex;
	int i;
	int wkIndex = -1;
	int wkBoxIndex = -1;

	for(i = Index + 1; i < tpMailBox->MailItemCnt; i++){
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if(tpMailItem == NULL || tpMailItem->Status != ICON_SEND){
			continue;
		}
		if(MailBoxIndex == NULL){
			return i;
		}
		BoxIndex = GetNameToMailBox(tpMailItem->MailBox);
		if(*MailBoxIndex == -1 || *MailBoxIndex == BoxIndex){
			wkIndex = i;
			wkBoxIndex = BoxIndex;
			break;
		}
		if(wkIndex == -1){
			wkIndex = i;
			wkBoxIndex = BoxIndex;
		}
	}
	if(MailBoxIndex != NULL){
		*MailBoxIndex = wkBoxIndex;
	}
	return wkIndex;
}


/******************************************************************************

	Item_GetNextMailBoxSendItem

	指定のメールボックスの送信マークの付いたアイテムのインデックスを取得

******************************************************************************/

int Item_GetNextMailBoxSendItem(struct TPMAILBOX *tpMailBox, int Index, int MailBoxIndex)
{
	struct TPMAILITEM *tpMailItem;
	int BoxIndex;
	int i;

	if(MailBoxIndex == -1){
		return -1;
	}
	for(i = Index + 1; i < tpMailBox->MailItemCnt; i++){
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if(tpMailItem == NULL || tpMailItem->Status != ICON_SEND){
			continue;
		}
		BoxIndex = GetNameToMailBox(tpMailItem->MailBox);
		if(MailBoxIndex == BoxIndex){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	Item_GetNextDeleteItem

	削除マークのアイテムのインデックスを取得

******************************************************************************/

int Item_GetNextDeleteItem(struct TPMAILBOX *tpMailBox, int Index, int *No)
{
	struct TPMAILITEM *tpMailItem;
	int i;

	for(i = Index + 1; i < tpMailBox->MailItemCnt; i++){
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if(tpMailItem == NULL){
			continue;
		}
		if(tpMailItem->Status == ICON_DEL){
			if(No != NULL){
				*No = tpMailItem->No;
			}
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	Item_GetMailNoToItemIndex

	メール番号からアイテムのインデックスを取得

******************************************************************************/

int Item_GetMailNoToItemIndex(struct TPMAILBOX *tpMailBox, int No)
{
	int i;

	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == NULL){
			continue;
		}
		if((*(tpMailBox->tpMailItem + i))->No == No){
			return i;
		}
	}
	return -1;
}


/******************************************************************************

	Item_IsMailBox

	メールボックス内のメールリストに指定のメールが存在するか調べる

******************************************************************************/

BOOL Item_IsMailBox(struct TPMAILBOX *tpMailBox, struct TPMAILITEM *tpMailItem)
{
	int i;

	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + i) == tpMailItem){
			return TRUE;
		}
	}
	return FALSE;
}


/******************************************************************************

	Item_FindThread

	メッセージIDを検索する

******************************************************************************/

int Item_FindThread(struct TPMAILBOX *tpMailBox, TCHAR *p, int Index)
{
	struct TPMAILITEM *tpMailItem;
	int j;

	if(p == NULL){
		return -1;
	}
	for(j = Index - 1; j >= 0; j--){
		if((tpMailItem = (*(tpMailBox->tpMailItem + j))) == NULL ||
			tpMailItem->MessageID == NULL){
			continue;
		}
		if(TStrCmp(tpMailItem->MessageID, p) == 0){
			return j;
		}
	}
	return -1;
}


/******************************************************************************

	Item_SetThread

	スレッドを構築する

******************************************************************************/

void Item_SetThread(struct TPMAILBOX *tpMailBox)
{
	struct TPMAILITEM *tpMailItem;
	struct TPMAILITEM *tpNextMailItem;
	int i, no = 0, n;
	int parent;

	if(tpMailBox->MailItemCnt == 0 ||
		(*tpMailBox->tpMailItem != NULL && (*tpMailBox->tpMailItem)->NextNo != 0)){
		return;
	}
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if((tpMailItem = *(tpMailBox->tpMailItem + i)) == NULL){
			continue;
		}
		tpMailItem->NextNo = tpMailItem->PrevNo = tpMailItem->Indent = 0;
		//元メールの検索
		parent = Item_FindThread(tpMailBox, tpMailItem->InReplyTo, i);
		if(parent == -1){
			parent = Item_FindThread(tpMailBox, tpMailItem->References, i);
		}
		//元メールなし
		if(parent == -1){
			(*(tpMailBox->tpMailItem + no))->NextNo = i;
			tpMailItem->PrevNo = no;
			no = i;
			continue;
		}
		//インデントを設定する
		tpMailItem->Indent = (*(tpMailBox->tpMailItem + parent))->Indent + 1;
		n = (*(tpMailBox->tpMailItem + parent))->NextNo;
		while(n != 0){
			if((tpNextMailItem = (*(tpMailBox->tpMailItem + n))) == NULL){
				n = 0;
				break;
			}
			//インデントからメールの追加位置を取得する
			if(tpNextMailItem->Indent < tpMailItem->Indent){
				tpMailItem->PrevNo = tpNextMailItem->PrevNo;
				tpMailItem->NextNo = n;

				(*(tpMailBox->tpMailItem + tpNextMailItem->PrevNo))->NextNo = i;
				tpNextMailItem->PrevNo = i;
				break;
			}
			n = tpNextMailItem->NextNo;
		}
		if(n == 0){
			(*(tpMailBox->tpMailItem + no))->NextNo = i;
			tpMailItem->PrevNo = no;
			no = i;
		}
	}
	//ソート数値を設定する
	no = 0;
	for(i = 0; i < tpMailBox->MailItemCnt; i++){
		if(*(tpMailBox->tpMailItem + no) == NULL){
			break;
		}
		(*(tpMailBox->tpMailItem + no))->PrevNo = i;
		no = (*(tpMailBox->tpMailItem + no))->NextNo;
	}
}
/* End of source */
