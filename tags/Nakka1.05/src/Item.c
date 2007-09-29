/*
 * nPOP
 *
 * Item.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "Memory.h"

#include "global.h"
#include "md5.h"

/* Define */

/* Global Variables */
extern OPTION op;
extern BOOL KeyShowHeader;

extern MAILBOX *MailBox;
extern MAILBOX *AddressBox;

/* Local Function Prototypes */
static void Item_GetContentT(TCHAR *buf, TCHAR *str, TCHAR **ret);
static int Item_GetContentInt(TCHAR *buf, TCHAR *str, int DefaultRet);
static int Item_GetMultiContent(char *buf, char *str, char **ret);
static int GetMimeContent(char *buf, char *Head, TCHAR **ret, BOOL MultiFlag);
static void Item_SetMailBody(MAILITEM *tpMailItem, char *buf, BOOL download);
static BOOL FilterCheckItem(char *buf, TCHAR *FHead, TCHAR *Fcontent);
static BOOL FilterCheck(MAILBOX *tpMailBox, char *buf);

/*
 * Item_SetItemCnt - アイテム数分のメモリを確保
 */
BOOL Item_SetItemCnt(MAILBOX *tpMailBox, int i)
{
	MAILITEM **tpMailList;

	if (i <= tpMailBox->MailItemCnt) {
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt;
		return TRUE;
	}
	tpMailList = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * i);
	if (tpMailList == NULL) {
		return FALSE;
	}
	if (tpMailBox->tpMailItem != NULL) {
		CopyMemory(tpMailList, tpMailBox->tpMailItem,
			sizeof(MAILITEM *) * tpMailBox->MailItemCnt);
	}

	mem_free((void **)&tpMailBox->tpMailItem);
	tpMailBox->tpMailItem = tpMailList;
	tpMailBox->AllocCnt = i;
	return TRUE;
}

/*
 * Item_Add - アイテムの追加
 */
BOOL Item_Add(MAILBOX *tpMailBox, MAILITEM *tpNewMailItem)
{
	MAILITEM **tpMailList = tpMailBox->tpMailItem;

	if (tpMailBox->AllocCnt <= tpMailBox->MailItemCnt) {
		tpMailList = (MAILITEM **)mem_alloc(sizeof(MAILITEM *) * (tpMailBox->MailItemCnt + 1));
		if (tpMailList == NULL) {
			return FALSE;
		}
		if (tpMailBox->tpMailItem != NULL) {
			CopyMemory(tpMailList, tpMailBox->tpMailItem,
				sizeof(MAILITEM *) * tpMailBox->MailItemCnt);
			mem_free((void **)&tpMailBox->tpMailItem);
		}
		tpMailBox->tpMailItem = tpMailList;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt + 1;
	}
	*(tpMailList + tpMailBox->MailItemCnt) = tpNewMailItem;
	tpMailBox->MailItemCnt++;
	if (*tpMailBox->tpMailItem != NULL) {
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return TRUE;
}

/*
 * CopyItem - アイテムのコピー
 */
void CopyItem(MAILITEM *tpFromNewMailItem, MAILITEM *tpToMailItem)
{
	CopyMemory(tpToMailItem, tpFromNewMailItem, sizeof(MAILITEM));
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

/*
 * Item_CopyMailBox - アイテムをメールボックスに追加
 */
MAILITEM *Item_CopyMailBox(MAILBOX *tpMailBox, MAILITEM *tpNewMailItem,
			   TCHAR *MailBoxName, BOOL SendClear)
{
	MAILITEM **tpMailList;
	int i = 0;

	tpMailList = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * (tpMailBox->MailItemCnt + 1));
	if (tpMailList == NULL) {
		return NULL;
	}
	if (tpMailBox->tpMailItem != NULL) {
		for (i = 0; i < tpMailBox->MailItemCnt; i++) {
			*(tpMailList + i) = *(tpMailBox->tpMailItem + i);
		}
	}

	*(tpMailList + i) = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (*(tpMailList + i) == NULL) {
		mem_free((void **)&tpMailList);
		return NULL;
	}
	CopyItem(tpNewMailItem, *(tpMailList + i));
	if ((*(tpMailList + i))->MailBox == NULL) {
		(*(tpMailList + i))->MailBox = AllocCopy(MailBoxName);
	}
	if (SendClear == TRUE) {
		(*(tpMailList + i))->MailStatus = (*(tpMailList + i))->Status = ICON_NON;
		mem_free(&(*(tpMailList + i))->Date);
		(*(tpMailList + i))->Date = NULL;
		mem_free(&(*(tpMailList + i))->MessageID);
		(*(tpMailList + i))->MessageID = NULL;
		(*(tpMailList + i))->hEditWnd = NULL;
	}

	mem_free((void **)&tpMailBox->tpMailItem);
	tpMailBox->tpMailItem = tpMailList;
	tpMailBox->MailItemCnt++;
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt;
	if (*tpMailBox->tpMailItem != NULL) {
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return *(tpMailList + i);
}

/*
 * Item_Resize - アイテム情報の整理
 */
BOOL Item_Resize(MAILBOX *tpMailBox)
{
	MAILITEM **tpMailList;
	int i, cnt = 0;

	if (tpMailBox->tpMailItem == NULL) {
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		return FALSE;
	}

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		cnt++;
	}

	tpMailList = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * cnt);
	if (tpMailList == NULL) {
		mem_free((void **)&tpMailBox->tpMailItem);
		tpMailBox->tpMailItem = NULL;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		return FALSE;
	}
	cnt = 0;
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		*(tpMailList + cnt) = *(tpMailBox->tpMailItem + i);
		cnt++;
	}

	mem_free((void **)&tpMailBox->tpMailItem);
	tpMailBox->tpMailItem = tpMailList;
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt = cnt;
	if (cnt != 0 && *tpMailBox->tpMailItem != NULL) {
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return TRUE;
}

/*
 * FreeMailItem - メールアイテムの解放
 */
void FreeMailItem(MAILITEM **tpMailItem, int Cnt)
{
	int i;

	for (i = 0; i < Cnt; i++) {
		if (*(tpMailItem + i) == NULL) {
			continue;
		}
		if ((*(tpMailItem + i))->hEditWnd != NULL) {
			(*(tpMailItem + i)) = NULL;
			continue;
		}
		mem_free(&(*(tpMailItem + i))->From);
		mem_free(&(*(tpMailItem + i))->To);
		mem_free(&(*(tpMailItem + i))->Cc);
		mem_free(&(*(tpMailItem + i))->Bcc);
		mem_free(&(*(tpMailItem + i))->Date);
		mem_free(&(*(tpMailItem + i))->Size);
		mem_free(&(*(tpMailItem + i))->Subject);
		mem_free(&(*(tpMailItem + i))->ReplyTo);
		mem_free(&(*(tpMailItem + i))->ContentType);
		mem_free(&(*(tpMailItem + i))->Encoding);
		mem_free(&(*(tpMailItem + i))->MessageID);
		mem_free(&(*(tpMailItem + i))->UIDL);
		mem_free(&(*(tpMailItem + i))->InReplyTo);
		mem_free(&(*(tpMailItem + i))->References);
		mem_free(&(*(tpMailItem + i))->Body);
		mem_free(&(*(tpMailItem + i))->MailBox);
		mem_free(&(*(tpMailItem + i))->Attach);

		mem_free(&*(tpMailItem + i));
		(*(tpMailItem + i)) = NULL;
	}
}

/*
 * Item_GetContent - コンテンツの取得
 */
int Item_GetContent(char *buf, char *str, char **ret)
{
	char *p;
	int len;

	// 位置の取得
	p = GetHeaderStringPoint(buf, str);
	if (p == NULL) {
		*ret = NULL;
		return 0;
	}
	// サイズの取得
	len = GetHeaderStringSize(p, FALSE);
	*ret = (char *)mem_alloc(len + 1);
	if (*ret == NULL) {
		return 0;
	}
	GetHeaderString(p, *ret, FALSE);
	return len;
}

/*
 * Item_GetMultiContent - 複数ある場合は一つにまとめてコンテンツの取得
 */
static int Item_GetMultiContent(char *buf, char *str, char **ret)
{
	char *tmp;
	char *p, *r;
	int len;
	int rLen = 0;

	*ret = NULL;
	p = buf;
	while (1) {
		// 位置の取得
		p = GetHeaderStringPoint(p, str);
		if (p == NULL) {
			return rLen;
		}
		// サイズの取得
		len = GetHeaderStringSize(p, FALSE);
		if (*ret != NULL) {
			r = tmp = (char *)mem_alloc(rLen + len + 2);
			if (tmp == NULL) {
				return rLen;
			}
			r = StrCpy(r, *ret);
			r = StrCpy(r, ",");
			mem_free(&*ret);
			*ret = tmp;
			rLen += (len + 1);
		} else {
			r = *ret = (char *)mem_alloc(len + 1);
			if (*ret == NULL) {
				return rLen;
			}
			rLen = len;
		}
		GetHeaderString(p, r, FALSE);
		p += len;
	}
}

/*
 * GetMessageId - メッセージIDの取得
 */
char *Item_GetMessageId(char *buf)
{
	char *Content, *p;
	MD5_CTX context;
	unsigned char digest[16];
	int len;

	// Message-Id取得
	Content = NULL;
	Item_GetContent(buf, HEAD_MESSAGEID, &Content);
	TrimMessageId(Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// UIDLを取得
	Content = NULL;
	Item_GetContent(buf, HEAD_X_UIDL, &Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// Dateを取得
	Content = NULL;
	Item_GetContent(buf, HEAD_DATE, &Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// ヘッダのハッシュ値を取得
	p = GetBodyPointa(buf);
	if (p != NULL) {
		len = p - buf;
	} else {
		len = tstrlen(buf);
	}
	MD5Init(&context);
	MD5Update(&context, buf, len);
	MD5Final(digest, &context);

	Content = (char *)mem_alloc(16 * 2 + 1);
	if (Content == NULL) {
		return NULL;
	}
	Base64Encode(digest, Content, 16);
	return Content;
}

/*
 * GetMimeContent - ヘッダのコンテンツを取得してMIMEデコードを行う
 */
static int GetMimeContent(char *buf, char *Head, TCHAR **ret, BOOL MultiFlag)
{
	char *Content;
	int len;

	*ret = NULL;

	len = ((MultiFlag == TRUE) ? Item_GetMultiContent : Item_GetContent)(buf, Head, &Content);
	if (Content != NULL) {
#ifdef UNICODE
		char *dcode;

		dcode = (char *)mem_alloc(len + 1);
		if (dcode != NULL) {
			MIMEdecode(Content, dcode);
			*ret = AllocCharToTchar(dcode);
			mem_free(&dcode);
		}
#else
		*ret = (char *)mem_alloc(len + 1);
		if (*ret != NULL) {
			MIMEdecode(Content, *ret);
		}
#endif
		mem_free(&Content);
	}
	return len;
}

/*
 * Item_SetMailBody - アイテムに本文を設定
 */
static void Item_SetMailBody(MAILITEM *tpMailItem, char *buf, BOOL download)
{
	char *p, *r;
	int Len;
	int HdSize;

	p = GetBodyPointa(buf);
	if (p != NULL && *p != '\0') {
		// デコード
		r = DecodeBodyTransfer(tpMailItem, p);
		if (r == NULL) {
			return;
		}
		Len = CharToTcharSize(r);

		// ヘッダを表示する設定の場合
		HdSize = (op.ShowHeader == 1 || KeyShowHeader == TRUE) ? (p - buf) : 0;

		mem_free(&tpMailItem->Body);
		tpMailItem->Body = (TCHAR *)mem_alloc(sizeof(TCHAR) * (Len + HdSize + 1));
		if (tpMailItem->Body != NULL) {
			if (op.ShowHeader == 1 || KeyShowHeader == TRUE) {
				// ヘッダ
				CharToTchar(buf, tpMailItem->Body, HdSize + 1);
			}
			// 本文
			CharToTchar(r, tpMailItem->Body + HdSize, Len);
		}
		mem_free(&r);

	} else if (op.ShowHeader == 1 || KeyShowHeader == TRUE) {
		// 本文が存在しない場合はヘッダのみ設定
		mem_free(&tpMailItem->Body);
		tpMailItem->Body = AllocCharToTchar(buf);

	} else if (download == TRUE) {
		mem_free(&tpMailItem->Body);
		tpMailItem->Body = (TCHAR *)mem_alloc(sizeof(TCHAR));
		if (tpMailItem->Body != NULL) {
			*tpMailItem->Body = TEXT('\0');
		}
	}
	if (tpMailItem->Body != NULL) {
		tpMailItem->Status = tpMailItem->MailStatus = ICON_MAIL;
	}
}

/*
 * FilterCheckItem - 文字列のチェック
 */
static BOOL FilterCheckItem(char *buf, TCHAR *FHead, TCHAR *Fcontent)
{
	TCHAR *Content;
	BOOL ret;
	int len;
#ifdef UNICODE
	char *head;
#endif

	if (Fcontent == NULL || *Fcontent == TEXT('\0')) {
		return TRUE;
	}
#ifdef UNICODE
	head = AllocTcharToChar(FHead);
	if (head == NULL) {
		return FALSE;
	}
	// コンテンツの取得
	len = GetMimeContent(buf, head, &Content, TRUE);
	mem_free(&head);
#else
	// コンテンツの取得
	len = GetMimeContent(buf, FHead, &Content, TRUE);
#endif
	if (Content == NULL) {
		return StrMatch(Fcontent, TEXT(""));
	}
	// 比較
	ret = StrMatch(Fcontent, Content);
	mem_free(&Content);
	return ret;
}

/*
 * FilterCheck - フィルタ文字列のチェック
 */
static int FilterCheck(MAILBOX *tpMailBox, char *buf)
{
	int RetFlag = 0;
	int fret;
	int i, j;

	if (tpMailBox->FilterEnable == 0 || tpMailBox->tpFilter == NULL ||
		buf == NULL || *buf == '\0') {
		return FILTER_RECV;
	}
	for (i = 0; i < tpMailBox->FilterCnt; i++) {
		if (*(tpMailBox->tpFilter + i) == NULL ||
			(*(tpMailBox->tpFilter + i))->Enable == 0) {
			continue;
		}
		// 項目のチェック
		if ((*(tpMailBox->tpFilter + i))->Header1 == NULL ||
			*(*(tpMailBox->tpFilter + i))->Header1 == TEXT('\0')) {
			continue;
		}
		if (FilterCheckItem(buf, (*(tpMailBox->tpFilter + i))->Header1,
			(*(tpMailBox->tpFilter + i))->Content1) == FALSE) {
			continue;
		}
		if ((*(tpMailBox->tpFilter + i))->Header2 == NULL ||
			*(*(tpMailBox->tpFilter + i))->Header2 == TEXT('\0') ||
			FilterCheckItem(buf, (*(tpMailBox->tpFilter + i))->Header2,
			(*(tpMailBox->tpFilter + i))->Content2) == TRUE) {

			j = (*(tpMailBox->tpFilter + i))->Action;
			for (fret = 1; j > 0; j--) {
				fret *= 2;
			}
			switch (fret) {
			case FILTER_UNRECV:
			case FILTER_RECV:
				// 受信フラグ
				if (!(RetFlag & FILTER_RECV) && !(RetFlag & FILTER_UNRECV)) {
					RetFlag |= fret;
				}
				break;

			case FILTER_DOWNLOADMARK:
			case FILTER_DELETEMARK:
				// マークフラグ
				if (!(RetFlag & FILTER_RECV) && !(RetFlag & FILTER_DOWNLOADMARK) && !(RetFlag & FILTER_DELETEMARK)) {
					RetFlag |= fret;
				}
				break;

			default:
				if (!(RetFlag & FILTER_RECV)) {
					RetFlag |= fret;
				}
				break;
			}
		}
	}
	return ((RetFlag == 0) ? FILTER_RECV : RetFlag);
}

/*
 * Item_SetMailItem - アイテムにヘッダと本文を設定
 */
BOOL Item_SetMailItem(MAILITEM *tpMailItem, char *buf, char *Size, BOOL download)
{
	TCHAR *msgid1 = NULL, *msgid2 = NULL, *t = NULL;
	char *Content;
#ifdef UNICODE
	char *dcode;
#endif

	if (download == TRUE) {
		// 既存の情報を解放
		mem_free(&tpMailItem->Subject);
		mem_free(&tpMailItem->From);
		mem_free(&tpMailItem->To);
		mem_free(&tpMailItem->Cc);
		mem_free(&tpMailItem->ReplyTo);
		mem_free(&tpMailItem->ContentType);
		mem_free(&tpMailItem->Encoding);
		mem_free(&tpMailItem->Date);
		if (Size != NULL) {
			mem_free(&tpMailItem->Size);
		}
		mem_free(&tpMailItem->MessageID);
		mem_free(&tpMailItem->InReplyTo);
		mem_free(&tpMailItem->References);
	}
	// Subject
	GetMimeContent(buf, HEAD_SUBJECT, &tpMailItem->Subject, FALSE);
	// From
	GetMimeContent(buf, HEAD_FROM, &tpMailItem->From, FALSE);
	// To
	GetMimeContent(buf, HEAD_TO, &tpMailItem->To, TRUE);
	// Cc
	GetMimeContent(buf, HEAD_CC, &tpMailItem->Cc, TRUE);
	// Reply-To
	GetMimeContent(buf, HEAD_REPLYTO, &tpMailItem->ReplyTo, FALSE);
	// Content-Type
	GetMimeContent(buf, HEAD_CONTENTTYPE, &tpMailItem->ContentType, FALSE);
	if (tpMailItem->ContentType != NULL &&
		TStrCmpNI(tpMailItem->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0) {
		tpMailItem->Multipart = TRUE;
	} else {
		// Content-Transfer-Encoding
#ifdef UNICODE
		Item_GetContent(buf, HEAD_ENCODING, &Content);
		if (Content != NULL) {
			tpMailItem->Encoding = AllocCharToTchar(Content);
			mem_free(&Content);
		}
#else
		Item_GetContent(buf, HEAD_ENCODING, &tpMailItem->Encoding);
#endif
	}

	// Date
	Item_GetContent(buf, HEAD_DATE, &Content);
	if (Content != NULL) {
#ifdef UNICODE
		dcode = (char *)mem_alloc(BUF_SIZE);
		if (dcode != NULL) {
			DateConv(Content, dcode);
			tpMailItem->Date = AllocCharToTchar(dcode);
			mem_free(&dcode);
		}
#else
		tpMailItem->Date = (char *)mem_alloc(BUF_SIZE);
		if (tpMailItem->Date != NULL) {
			DateConv(Content, tpMailItem->Date);
		}
#endif
		mem_free(&Content);
	}

	// Size
	if (Size != NULL) {
		tpMailItem->Size = AllocCharToTchar(Size);
	}

	// Message-Id
#ifdef UNICODE
	Content = Item_GetMessageId(buf);
	if (Content != NULL) {
		tpMailItem->MessageID = AllocCharToTchar(Content);
		mem_free(&Content);
	}
#else
	tpMailItem->MessageID = Item_GetMessageId(buf);
#endif

	// In-Reply-To
#ifdef UNICODE
	Item_GetContent(buf, HEAD_INREPLYTO, &Content);
	TrimMessageId(Content);
	if (Content != NULL) {
		tpMailItem->InReplyTo = AllocCharToTchar(Content);
		mem_free(&Content);
	}
#else
	Item_GetContent(buf, HEAD_INREPLYTO, &tpMailItem->InReplyTo);
	TrimMessageId(tpMailItem->InReplyTo);
#endif

	// References
	Item_GetContent(buf, HEAD_REFERENCES, &Content);
	if (Content != NULL) {
#ifdef UNICODE
		dcode = (char *)mem_alloc(GetReferencesSize(Content, TRUE) + 1);
		if (dcode != NULL) {
			ConvReferences(Content, dcode, FALSE);
			msgid1 = AllocCharToTchar(dcode);

			ConvReferences(Content, dcode, TRUE);
			msgid2 = AllocCharToTchar(dcode);
			mem_free(&dcode);
		}
#else
		msgid1 = (char *)mem_alloc(GetReferencesSize(Content, FALSE) + 1);
		if (msgid1 != NULL) {
			ConvReferences(Content, msgid1, FALSE);
		}

		msgid2 = (char *)mem_alloc(GetReferencesSize(Content, TRUE) + 1);
		if (msgid2 != NULL) {
			ConvReferences(Content, msgid2, TRUE);
		}
#endif
		if (msgid1 != NULL && msgid2 != NULL && TStrCmp(msgid1, msgid2) == 0) {
			mem_free(&msgid2);
			msgid2 = NULL;
		}
		mem_free(&Content);
	}

	if (tpMailItem->InReplyTo == NULL || *tpMailItem->InReplyTo == TEXT('\0')) {
		mem_free(&tpMailItem->InReplyTo);
		tpMailItem->InReplyTo = AllocCopy(msgid1);
		t = msgid2;
	} else {
		t = (msgid1 != NULL && TStrCmp(tpMailItem->InReplyTo, msgid1) != 0) ? msgid1 : msgid2;
	}
	tpMailItem->References = AllocCopy(t);
	mem_free(&msgid1);
	mem_free(&msgid2);

	// Body
	Item_SetMailBody(tpMailItem, buf, download);
	return TRUE;
}

/*
 * Item_HeadToItem - メールヘッダからアイテムを作成する
 */
MAILITEM *Item_HeadToItem(MAILBOX *tpMailBox, char *buf, char *Size)
{
	MAILITEM *tpMailItem;
	int fret;

	// フィルタをチェック
	fret = FilterCheck(tpMailBox, buf);
	if (fret == FILTER_UNRECV) {
		return (MAILITEM *)-1;
	}

	// メール情報の確保
	tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (tpMailItem == NULL) {
		return NULL;
	}
	// ヘッダと本文を設定
	Item_SetMailItem(tpMailItem, buf, Size, FALSE);

	// メール情報のリストに追加
	if (!(fret & FILTER_UNRECV) && Item_Add(tpMailBox, tpMailItem) == -1) {
		FreeMailItem(&tpMailItem, 1);
		return NULL;
	}

	// フィルタ動作設定
	// 開封済み設定
	if (fret & FILTER_READICON && tpMailItem->MailStatus != ICON_NON) {
		tpMailItem->Status = tpMailItem->MailStatus = ICON_READ;
	}
	// マーク設定
	if (fret & FILTER_DOWNLOADMARK) {
		tpMailItem->Status = ICON_DOWN;
	} else if (fret & FILTER_DELETEMARK) {
		tpMailItem->Status = ICON_DEL;
	}
	// 保存箱へコピー
	if (fret & FILTER_SAVE &&
		tpMailItem->MailStatus != ICON_NON &&
		Item_FindThread(MailBox + MAILBOX_SAVE, tpMailItem->MessageID, (MailBox + MAILBOX_SAVE)->MailItemCnt) == -1) {
		Item_CopyMailBox(MailBox + MAILBOX_SAVE, tpMailItem, tpMailBox->Name, FALSE);
	}
	if (fret & FILTER_UNRECV) {
		// 受信しないフラグが有効の場合は解放する
		FreeMailItem(&tpMailItem, 1);
		return (MAILITEM *)-1;
	}
	return tpMailItem;
}

/*
 * Item_GetContentT - コンテンツの取得
 */
static void Item_GetContentT(TCHAR *buf, TCHAR *str, TCHAR **ret)
{
	TCHAR *p;
	int len;

	// 位置の取得
	p = GetHeaderStringPointT(buf, str);
	if (p == NULL) {
		*ret = NULL;
		return;
	}
	// サイズの取得
	len = GetHeaderStringSizeT(p, TRUE);
	*ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (*ret == NULL) {
		return;
	}
	GetHeaderStringT(p, *ret, TRUE);
}

/*
 * Item_GetContentT - コンテンツの取得
 */
static int Item_GetContentInt(TCHAR *buf, TCHAR *str, int DefaultRet)
{
	TCHAR *Content;
	int ret;

	Item_GetContentT(buf, str, &Content);
	if (Content == NULL) {
		return DefaultRet;
	}
	ret = _ttoi(Content);
	mem_free(&Content);
	return ret;
}

/*
 * Item_StringToItem - 文字列からアイテムを作成する
 */
MAILITEM *Item_StringToItem(MAILBOX *tpMailBox, TCHAR *buf)
{
	MAILITEM *tpMailItem;
	int i;

	tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (tpMailItem == NULL) {
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
	if (tpMailItem->Attach == NULL) {
		Item_GetContentT(buf, TEXT(HEAD_X_ATTACH_OLD), &tpMailItem->Attach);
		if (tpMailItem->Attach != NULL) {
			TCHAR *p;
			for (p = tpMailItem->Attach; *p != TEXT('\0'); p++) {
				if (*p == TEXT(',')) {
					*p = ATTACH_SEP;
				}
			}
		}
	}

	// No
	tpMailItem->No = Item_GetContentInt(buf, TEXT(HEAD_X_NO), -1);
	if (tpMailItem->No == -1) {
		tpMailItem->No = Item_GetContentInt(buf, TEXT(HEAD_X_NO_OLD), 0);
	}
	// MailStatus
	tpMailItem->MailStatus = Item_GetContentInt(buf, TEXT(HEAD_X_STATUS), -1);
	if (tpMailItem->MailStatus == -1) {
		tpMailItem->MailStatus = Item_GetContentInt(buf, TEXT(HEAD_X_STATUS_OLD), 0);
	}
	// MarkStatus
	i = Item_GetContentInt(buf, TEXT(HEAD_X_MARK), -1);
	tpMailItem->Status = (i != -1) ? i : tpMailItem->MailStatus;
	// Download
	tpMailItem->Download = Item_GetContentInt(buf, TEXT(HEAD_X_DOWNLOAD), -1);
	if (tpMailItem->Download == -1) {
		tpMailItem->Download = Item_GetContentInt(buf, TEXT(HEAD_X_DOWNLOAD_OLD), 0);
	}
	// Multipart
	if (tpMailItem->Attach != NULL || (tpMailItem->ContentType != NULL &&
		TStrCmpNI(tpMailItem->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0)) {
		tpMailItem->Multipart = TRUE;
	}
	return tpMailItem;
}

/*
 * Item_GetStringSize - メールの保存文字列のサイズ取得
 */
int Item_GetStringSize(MAILITEM *tpMailItem, BOOL BodyFlag)
{
	TCHAR X_No[10], X_Mstatus[10], X_Status[10], X_Downflag[10];
	int len = 0;

#ifndef _itot
	wsprintf(X_No, TEXT("%d"), tpMailItem->No);
	wsprintf(X_Mstatus, TEXT("%d"), tpMailItem->MailStatus);
	wsprintf(X_Status, TEXT("%d"), tpMailItem->Status);
	wsprintf(X_Downflag, TEXT("%d"), tpMailItem->Download);
#else
	_itot(tpMailItem->No, X_No, 10);
	_itot(tpMailItem->MailStatus, X_Mstatus, 10);
	_itot(tpMailItem->Status, X_Status, 10);
	_itot(tpMailItem->Download, X_Downflag, 10);
#endif

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
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_STATUS), X_Mstatus);
	if (tpMailItem->MailStatus != tpMailItem->Status) {
		len += GetSaveHeaderStringSize(TEXT(HEAD_X_MARK), X_Status);
	}
	len += GetSaveHeaderStringSize(TEXT(HEAD_X_DOWNLOAD), X_Downflag);
	len += 2;

	if (BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != TEXT('\0')) {
		len += lstrlen(tpMailItem->Body);
	}
	len += 5;
	return len;
}

/*
 * Item_GetString - メールの保存文字列の取得
 */
TCHAR *Item_GetString(TCHAR *buf, MAILITEM *tpMailItem, BOOL BodyFlag)
{
	TCHAR *p = buf;
	TCHAR X_No[10], X_Mstatus[10], X_Status[10], X_Downflag[10];

#ifndef _itot
	wsprintf(X_No, TEXT("%d"), tpMailItem->No);
	wsprintf(X_Mstatus, TEXT("%d"), tpMailItem->MailStatus);
	wsprintf(X_Status, TEXT("%d"), tpMailItem->Status);
	wsprintf(X_Downflag, TEXT("%d"), tpMailItem->Download);
#else
	_itot(tpMailItem->No, X_No, 10);
	_itot(tpMailItem->MailStatus, X_Mstatus, 10);
	_itot(tpMailItem->Status, X_Status, 10);
	_itot(tpMailItem->Download, X_Downflag, 10);
#endif

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
	p = SaveHeaderString(TEXT(HEAD_X_STATUS), X_Mstatus, p);
	if (tpMailItem->MailStatus != tpMailItem->Status) {
		p = SaveHeaderString(TEXT(HEAD_X_MARK), X_Status, p);
	}
	p = SaveHeaderString(TEXT(HEAD_X_DOWNLOAD), X_Downflag, p);
	p = TStrCpy(p, TEXT("\r\n"));

	if (BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != TEXT('\0')) {
		p = TStrCpy(p, tpMailItem->Body);
	}
	p = TStrCpy(p, TEXT("\r\n.\r\n"));
	return p;
}

/*
 * Item_GetNextDonloadItem - ダウンロードマークのアイテムのインデックスを取得
 */
int Item_GetNextDonloadItem(MAILBOX *tpMailBox, int Index, int *No)
{
	MAILITEM *tpMailItem;
	int i;

	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Status == ICON_DOWN) {
			if (No != NULL) {
				*No = tpMailItem->No;
			}
			return i;
		}
	}
	return -1;
}

/*
 * Item_GetNextSendItem - 送信マークの付いたアイテムのインデックスを取得
 */
int Item_GetNextSendItem(MAILBOX *tpMailBox, int Index, int *MailBoxIndex)
{
	MAILITEM *tpMailItem;
	int BoxIndex;
	int i;
	int wkIndex = -1;
	int wkBoxIndex = -1;

	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL || tpMailItem->Status != ICON_SEND) {
			continue;
		}
		if (MailBoxIndex == NULL) {
			return i;
		}
		BoxIndex = GetNameToMailBox(tpMailItem->MailBox);
		if (*MailBoxIndex == -1 || *MailBoxIndex == BoxIndex) {
			wkIndex = i;
			wkBoxIndex = BoxIndex;
			break;
		}
		if (wkIndex == -1) {
			wkIndex = i;
			wkBoxIndex = BoxIndex;
		}
	}
	if (MailBoxIndex != NULL) {
		*MailBoxIndex = wkBoxIndex;
	}
	return wkIndex;
}

/*
 * Item_GetNextMailBoxSendItem - 指定のメールボックスの送信マークの付いたアイテムのインデックスを取得
 */
int Item_GetNextMailBoxSendItem(MAILBOX *tpMailBox, int Index, int MailBoxIndex)
{
	MAILITEM *tpMailItem;
	int BoxIndex;
	int i;

	if (MailBoxIndex == -1) {
		return -1;
	}
	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL || tpMailItem->Status != ICON_SEND) {
			continue;
		}
		BoxIndex = GetNameToMailBox(tpMailItem->MailBox);
		if (MailBoxIndex == BoxIndex) {
			return i;
		}
	}
	return -1;
}

/*
 * Item_GetNextDeleteItem - 削除マークのアイテムのインデックスを取得
 */
int Item_GetNextDeleteItem(MAILBOX *tpMailBox, int Index, int *No)
{
	MAILITEM *tpMailItem;
	int i;

	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Status == ICON_DEL) {
			if (No != NULL) {
				*No = tpMailItem->No;
			}
			return i;
		}
	}
	return -1;
}

/*
 * Item_GetMailNoToItemIndex - メール番号からアイテムのインデックスを取得
 */
int Item_GetMailNoToItemIndex(MAILBOX *tpMailBox, int No)
{
	int i;

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			continue;
		}
		if ((*(tpMailBox->tpMailItem + i))->No == No) {
			return i;
		}
	}
	return -1;
}

/*
 * Item_IsMailBox - メールボックス内のメールリストに指定のメールが存在するか調べる
 */
BOOL Item_IsMailBox(MAILBOX *tpMailBox, MAILITEM *tpMailItem)
{
	int i;

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == tpMailItem) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * Item_FindThread - メッセージIDを検索する
 */
int Item_FindThread(MAILBOX *tpMailBox, TCHAR *p, int Index)
{
	MAILITEM *tpMailItem;
	int j;

	if (p == NULL) {
		return -1;
	}
	for (j = Index - 1; j >= 0; j--) {
		if ((tpMailItem = (*(tpMailBox->tpMailItem + j))) == NULL ||
			tpMailItem->MessageID == NULL) {
			continue;
		}
		if (TStrCmp(tpMailItem->MessageID, p) == 0) {
			return j;
		}
	}
	return -1;
}

/*
 * Item_SetThread - スレッドを構築する
 */
void Item_SetThread(MAILBOX *tpMailBox)
{
	MAILITEM *tpMailItem;
	MAILITEM *tpNextMailItem;
	int i, no = 0, n;
	int parent;

	if (tpMailBox->MailItemCnt == 0 ||
		(*tpMailBox->tpMailItem != NULL && (*tpMailBox->tpMailItem)->NextNo != 0)) {
		return;
	}
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if ((tpMailItem = *(tpMailBox->tpMailItem + i)) == NULL) {
			continue;
		}
		tpMailItem->NextNo = tpMailItem->PrevNo = tpMailItem->Indent = 0;
		// 元メールの検索
		parent = Item_FindThread(tpMailBox, tpMailItem->InReplyTo, i);
		if (parent == -1) {
			parent = Item_FindThread(tpMailBox, tpMailItem->References, i);
		}
		// 元メールなし
		if (parent == -1) {
			(*(tpMailBox->tpMailItem + no))->NextNo = i;
			tpMailItem->PrevNo = no;
			no = i;
			continue;
		}
		// インデントを設定する
		tpMailItem->Indent = (*(tpMailBox->tpMailItem + parent))->Indent + 1;
		n = (*(tpMailBox->tpMailItem + parent))->NextNo;
		while (n != 0) {
			if ((tpNextMailItem = (*(tpMailBox->tpMailItem + n))) == NULL) {
				n = 0;
				break;
			}
			// インデントからメールの追加位置を取得する
			if (tpNextMailItem->Indent < tpMailItem->Indent) {
				tpMailItem->PrevNo = tpNextMailItem->PrevNo;
				tpMailItem->NextNo = n;

				(*(tpMailBox->tpMailItem + tpNextMailItem->PrevNo))->NextNo = i;
				tpNextMailItem->PrevNo = i;
				break;
			}
			n = tpNextMailItem->NextNo;
		}
		if (n == 0) {
			(*(tpMailBox->tpMailItem + no))->NextNo = i;
			tpMailItem->PrevNo = no;
			no = i;
		}
	}
	// ソート数値を設定する
	no = 0;
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + no) == NULL) {
			break;
		}
		(*(tpMailBox->tpMailItem + no))->PrevNo = i;
		no = (*(tpMailBox->tpMailItem + no))->NextNo;
	}
}
/* End of source */
