/*
 * nPOP
 *
 * Item.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2012 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#include "code.h"
#include "mime.h"
#include "charset.h"
#include "multipart.h"

#include "global.h"
#include "md5.h"

/* Define */
#define MBOX_DELIMITER "From "
#define NPOPUK_MBOX_DELIMITER "From NPOPUK\r\n"
#define STATUS_REVISION_NUMBER	300000
#define STATUS_REVISION_OLD 	200000
#define BLOCK_SIZE				1024

/* Global Variables */
extern HWND MainWnd;
extern HINSTANCE hInst;
extern OPTION op;
extern BOOL KeyShowHeader;
extern BOOL ImportRead;
extern BOOL ImportDown;

extern MAILBOX *MailBox;
extern MAILITEM *AttachMailItem;
extern MAILITEM *SmtpFwdMessage;
extern int SelBox;
extern int MailBoxCnt;
extern ADDRESSBOOK *AddressBook;

/* Local Function Prototypes */
static int item_get_content(char *buf, char *header, char **ret);
static int item_get_content_int(char *buf, char *header, int DefaultRet);
static int item_get_multi_content(char *buf, char *header, char **ret);
static int item_get_mime_content(char *buf, char *header, TCHAR **ret, BOOL multi_flag);
static void item_set_body(MAILITEM *tpMailItem, char *buf, int download);
static int item_save_header_size(TCHAR *header, TCHAR *buf);
static char *item_save_header(TCHAR *header, TCHAR *buf, char *ret);
static BOOL item_filter_check_content(char *buf, TCHAR *filter_header, TCHAR *filter_content);
static int item_check_filter(FILTER *tpFilter, char *buf, int *do_what_i, int flag_in);
static int item_filter_check(MAILBOX *tpMailBox, char *buf, int *do_what);
static int item_filter_domovecopy(MAILBOX *tpMailBox, MAILITEM *tpMailItem, BOOL refilter, int dw, int sbox);
static BOOL item_filter_execute(MAILBOX *tpMailBox, MAILITEM *tpMailItem, int fret, int *do_what, BOOL refilter);

/*
 * item_is_mailbox - メールボックス内のメールリストに指定のメールが存在するか調べる
 */
int item_is_mailbox(MAILBOX *tpMailBox, MAILITEM *tpMailItem)
{
	int i;

	if (tpMailItem == NULL) {
		return -1;
	}
	if (tpMailItem == AttachMailItem) {
		return -2;
	}

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == tpMailItem) {
			return i;
		}
	}
	return -1;
}

/*
 * item_set_count - アイテム数分のメモリを確保
 */
BOOL item_set_count(MAILBOX *tpMailBox, int i)
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
 * item_add - アイテムの追加
 */
BOOL item_add(MAILBOX *tpMailBox, MAILITEM *tpNewMailItem)
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
	if (tpNewMailItem->No > 0) {
		// put the new item in correct order
		// (in case of filling in or getting in reverse order)
		int i;
		for (i = tpMailBox->MailItemCnt-1; i >= 0; i--) {
			if ((*(tpMailList + i))->No <= tpNewMailItem->No) {
				*(tpMailList + i + 1) = tpNewMailItem;
				break;
			} else {
				*(tpMailList + i + 1) = *(tpMailList + i);
			}
		}
		if (i < 0) {
			*(tpMailList + 0) = tpNewMailItem;
		}
	} else {
		*(tpMailList + tpMailBox->MailItemCnt) = tpNewMailItem;
	}
	tpMailBox->MailItemCnt++;
	if (*tpMailBox->tpMailItem != NULL) {
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
	return TRUE;
}

/*
 * item_copy - アイテムのコピー
 */
void item_copy(MAILITEM *tpFromMailItem, MAILITEM *tpToMailItem, BOOL Override)
{
	// copy is sufficient for all non-pointers
	CopyMemory(tpToMailItem, tpFromMailItem, sizeof(MAILITEM));

	if (Override) {
		// override a few values
		if (tpToMailItem->Mark != ICON_FLAG) {
			tpToMailItem->Mark = tpToMailItem->MailStatus;
		}
		//tpToMailItem->New = FALSE;
		tpToMailItem->No = 0;
		tpToMailItem->UIDL = NULL;
	}

	// need to allocate new copies of the strings
	tpToMailItem->From = alloc_copy_t(tpFromMailItem->From);
	tpToMailItem->From_email = alloc_copy_t(tpFromMailItem->From_email);
	tpToMailItem->To = alloc_copy_t(tpFromMailItem->To);
	tpToMailItem->Cc = alloc_copy_t(tpFromMailItem->Cc);
	tpToMailItem->Bcc = alloc_copy_t(tpFromMailItem->Bcc);
	tpToMailItem->RedirectTo = alloc_copy_t(tpFromMailItem->RedirectTo);
	tpToMailItem->Subject = alloc_copy_t(tpFromMailItem->Subject);
	tpToMailItem->Date = alloc_copy_t(tpFromMailItem->Date);
	tpToMailItem->FmtDate = alloc_copy_t(tpFromMailItem->FmtDate);
	tpToMailItem->Size = alloc_copy_t(tpFromMailItem->Size);
	tpToMailItem->ReplyTo = alloc_copy_t(tpFromMailItem->ReplyTo);
	tpToMailItem->ContentType = alloc_copy_t(tpFromMailItem->ContentType);
	tpToMailItem->Encoding = alloc_copy_t(tpFromMailItem->Encoding);
	tpToMailItem->MessageID = alloc_copy_t(tpFromMailItem->MessageID);
	tpToMailItem->InReplyTo = alloc_copy_t(tpFromMailItem->InReplyTo);
	tpToMailItem->References = alloc_copy_t(tpFromMailItem->References);
	tpToMailItem->WireForm = alloc_copy(tpFromMailItem->WireForm);
	tpToMailItem->Body = alloc_copy(tpFromMailItem->Body);
	tpToMailItem->MailBox = alloc_copy_t(tpFromMailItem->MailBox);
	tpToMailItem->Attach = alloc_copy_t(tpFromMailItem->Attach);
	tpToMailItem->FwdAttach = alloc_copy_t(tpFromMailItem->FwdAttach);

	tpToMailItem->HeadCharset = alloc_copy_t(tpFromMailItem->HeadCharset);
	tpToMailItem->BodyCharset = alloc_copy_t(tpFromMailItem->BodyCharset);
}

/*
 * item_to_mailbox - アイテムをメールボックスに追加
 */
MAILITEM *item_to_mailbox(MAILBOX *tpMailBox, MAILITEM *tpNewMailItem, TCHAR *MailBoxName, BOOL SendClear)
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
	item_copy(tpNewMailItem, *(tpMailList + i), TRUE);
	if ((*(tpMailList + i))->MailBox == NULL) {
		(*(tpMailList + i))->MailBox = alloc_copy_t(MailBoxName);
	}
	if (SendClear == TRUE) {
		(*(tpMailList + i))->MailStatus = (*(tpMailList + i))->Mark = ICON_NON;
		mem_free(&(*(tpMailList + i))->Date);
		(*(tpMailList + i))->Date = NULL;
		mem_free(&(*(tpMailList + i))->FmtDate);
		(*(tpMailList + i))->FmtDate = NULL;
		mem_free(&(*(tpMailList + i))->MessageID);
		(*(tpMailList + i))->MessageID = NULL;
		(*(tpMailList + i))->hEditWnd = NULL;
#ifdef GJC_RECOMPUTE_SIZE
	} else if (tpMailBox->Type == MAILBOX_TYPE_SAVE && tpNewMailItem->Download == FALSE) {
		TCHAR num[20];
		TCHAR *s;
		int len = item_to_string_size(*(tpMailList+i), op.WriteMbox, TRUE, FALSE);
		wsprintf(num, TEXT("%d"), len);
		s = alloc_copy_t(num);
		if (s != NULL) {
			mem_free(&(*(tpMailList + i))->Size);
			(*(tpMailList + i))->Size = s;
		}
#endif
	}
	if (tpMailBox->Type == MAILBOX_TYPE_SAVE) {
		if ((*(tpMailList + i))->MailStatus == ICON_SEND) {
			(*(tpMailList + i))->MailStatus = ICON_NON;
		}
		if ((*(tpMailList + i))->Mark == ICON_SEND) {
			(*(tpMailList + i))->Mark = ICON_NON;
		}
	}

	mem_free((void **)&tpMailBox->tpMailItem);
	tpMailBox->tpMailItem = tpMailList;
	tpMailBox->MailItemCnt++;
	tpMailBox->AllocCnt = tpMailBox->MailItemCnt;
	tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
	if (*tpMailBox->tpMailItem != NULL) {
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return *(tpMailList + i);
}

/*
 * item_resize_mailbox - アイテム情報の整理
 */
BOOL item_resize_mailbox(MAILBOX *tpMailBox, int SetLastNo)
{
	MAILITEM **tpMailList;
	int i, cnt = 0;
	BOOL do_it = FALSE;

	if (tpMailBox->tpMailItem == NULL) {
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		return FALSE;
	}

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == NULL) {
			do_it = TRUE;
		} else {
			cnt++;
			if (SetLastNo && tpMailBox->LastNo < (*(tpMailBox->tpMailItem + i))->No) {
				tpMailBox->LastNo = (*(tpMailBox->tpMailItem + i))->No;
			}
		}
	}
	if (do_it == FALSE) {
		// nothing to be done
		return FALSE;
	}
	tpMailBox->NeedsSave |= MAILITEMS_CHANGED;

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
 * item_free - メールアイテムの解放
 */
void item_free(MAILITEM **tpMailItem, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++) {
		if (*(tpMailItem + i) == NULL) {
			continue;
		}
		if ((*(tpMailItem + i))->hEditWnd != NULL) {
			(*(tpMailItem + i)) = NULL;
			continue;
		}
		mem_free(&(*(tpMailItem + i))->From);
		mem_free(&(*(tpMailItem + i))->From_email);
		mem_free(&(*(tpMailItem + i))->To);
		mem_free(&(*(tpMailItem + i))->Cc);
		mem_free(&(*(tpMailItem + i))->Bcc);
		mem_free(&(*(tpMailItem + i))->RedirectTo);
		mem_free(&(*(tpMailItem + i))->Date);
		mem_free(&(*(tpMailItem + i))->FmtDate);
		mem_free(&(*(tpMailItem + i))->Size);
		mem_free(&(*(tpMailItem + i))->Subject);
		mem_free(&(*(tpMailItem + i))->ReplyTo);
		mem_free(&(*(tpMailItem + i))->ContentType);
		mem_free(&(*(tpMailItem + i))->Encoding);
		mem_free(&(*(tpMailItem + i))->MessageID);
		mem_free(&(*(tpMailItem + i))->UIDL);
		mem_free(&(*(tpMailItem + i))->InReplyTo);
		mem_free(&(*(tpMailItem + i))->References);
		mem_free(&(*(tpMailItem + i))->WireForm);
		mem_free(&(*(tpMailItem + i))->Body);
		mem_free(&(*(tpMailItem + i))->MailBox);
		mem_free(&(*(tpMailItem + i))->Attach);
		mem_free(&(*(tpMailItem + i))->FwdAttach);
		mem_free(&(*(tpMailItem + i))->HeadCharset);
		mem_free(&(*(tpMailItem + i))->BodyCharset);

		mem_free(&*(tpMailItem + i));
		(*(tpMailItem + i)) = NULL;
	}
}

/*
 * item_get_content - コンテンツの取得
 */
static int item_get_content(char *buf, char *header, char **ret)
{
	char *p;
	int len;

	//Acquisition
	p = GetHeaderStringPoint(buf, header);
	if (p == NULL) {
		*ret = NULL;
		return 0;
	}
	//of position Acquisition
	len = GetHeaderStringSize(p, FALSE);
	*ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (*ret == NULL) {
		return 0;
	}
	GetHeaderString(p, *ret, FALSE);
	return len;
}

/*
 * item_get_content_t
 */
void item_get_content_t(char *buf, char *header, TCHAR **ret)
{
#ifdef UNICODE
	char *cret;
#endif
	char *p;
	int len;

	// 位置の取得
	p = GetHeaderStringPoint(buf, header);
	if (p == NULL) {
		*ret = NULL;
		return;
	}
	// サイズの取得
	len = GetHeaderStringSize(p, TRUE);
#ifdef UNICODE
	cret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (cret == NULL) {
		return;
	}
	GetHeaderString(p, cret, TRUE);
	*ret = alloc_char_to_tchar(cret);
	mem_free(&cret);
#else
	*ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (*ret == NULL) {
		return;
	}
	GetHeaderString(p, *ret, TRUE);
#endif
}

/*
 * item_get_content_int - コンテンツの取得
 */
static int item_get_content_int(char *buf, char *header, int DefaultRet)
{
	TCHAR *Content;
	int ret;

	item_get_content_t(buf, header, &Content);
	if (Content == NULL) {
		return DefaultRet;
	}
	ret = _ttoi(Content);
	mem_free(&Content);
	return ret;
}

/*
 * item_get_multi_content - 複数ある場合は一つにまとめてコンテンツの取得
 */
static int item_get_multi_content(char *buf, char *header, char **ret)
{
	char *tmp;
	char *p, *r;
	int len;
	int ret_len = 0;

	*ret = NULL;
	p = buf;
	while (1) {
		//of size Acquisition
		p = GetHeaderStringPoint(p, header);
		if (p == NULL) {
			return ret_len;
		}
		//of position When acquisition
		len = GetHeaderStringSize(p, FALSE);
		if (*ret != NULL) {
			r = tmp = (char *)mem_alloc(sizeof(char) * (ret_len + len + 2));
			if (tmp == NULL) {
				return ret_len;
			}
			r = str_cpy(r, *ret);
			r = str_cpy(r, ",");
			mem_free(&*ret);
			*ret = tmp;
			ret_len += (len + 1);
		} else {
			r = *ret = (char *)mem_alloc(sizeof(char) * (len + 1));
			if (*ret == NULL) {
				return ret_len;
			}
			ret_len = len;
		}
		GetHeaderString(p, r, FALSE);
		p += len;
	}
}

/*
 * item_get_mime_content - ヘッダのコンテンツを取得してMIMEデコードを行う
 */
static int item_get_mime_content(char *buf, char *header, TCHAR **ret, BOOL multi_flag)
{
	char *Content;
	int len;

	*ret = NULL;

	len = ((multi_flag == TRUE) ? item_get_multi_content : item_get_content)(buf, header, &Content);
	if (Content != NULL) {
		len = MIME_decode(Content, NULL);
		*ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (*ret != NULL) {
			MIME_decode(Content, *ret);
		}
		mem_free(&Content);
	}
	return len;
}

/*
 * item_get_message_id - メッセージIDの取得
 */
char *item_get_message_id(char *buf)
{
	char *Content, *p;
	MD5_CTX context;
	unsigned char digest[16];
	int i, len;
#ifdef UNICODE
	TCHAR dtmp[3];
	char ctmp[3];
#endif

	// Look for Message-Id
	Content = NULL;
	item_get_content(buf, HEAD_MESSAGEID, &Content);
	TrimMessageId(Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// No Message-Id, look for UIDL
	Content = NULL;
	item_get_content(buf, HEAD_X_UIDL, &Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// Try to use Date
	Content = NULL;
	item_get_content(buf, HEAD_DATE, &Content);
	if (Content != NULL && *Content != '\0') {
		return Content;
	}
	mem_free(&Content);

	// Create MD5 hash of headers or message
	p = GetBodyPointa(buf);
	if (p != NULL) {
		len = p - buf;
	} else {
		len = tstrlen(buf);
	}
	MD5Init(&context);
	MD5Update(&context, buf, len);
	MD5Final(digest, &context);

	// Convert MD5 to nice characters
	Content = (char *)mem_alloc(sizeof(char) * (16 * 2 + 1));
	if (Content == NULL) {
		return NULL;
	}
	p = Content;
	for (i = 0; i < 16; i++) {
#ifdef UNICODE
		wsprintf(dtmp, TEXT("%02X"), digest[i]);
		tchar_to_char(dtmp, ctmp, 3);
		p = str_join(p, ctmp, (char *)-1);
#else
		wsprintf(p, "%02X", digest[i]);
		p += 2;
#endif
	}
	return Content;
}

/*
 * item_get_number_to_index - メール番号からアイテムのインデックスを取得
 */
int item_get_number_to_index(MAILBOX *tpMailBox, int No)
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
 * item_get_next_download_mark - find the next message marked for download
 */
int item_get_next_download_mark(MAILBOX *tpMailBox, int Index, int *No)
{
	MAILITEM *tpMailItem;
	int i;

	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Mark == ICON_DOWN) {
			if (No != NULL) {
				*No = tpMailItem->No;
			}
			return i;
		}
	}
	return -1;
}

/*
 * item_get_next_delete_mark - find the next message marked for delete
 */
int item_get_next_delete_mark(MAILBOX *tpMailBox, BOOL hold, int Index, int *No)
{
	MAILITEM *tpMailItem;
	int i;

	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if ((tpMailItem->Mark == ICON_DEL) && ((hold << 3) ^ (tpMailItem->ReFwd & REFWD_FWDHOLD))) {
			if (No != NULL) {
				*No = tpMailItem->No;
			}
			return i;
		}
	}
	return -1;
}

/*
 * item_get_next_new - find the next new message
 */
int item_get_next_new(MAILBOX *tpMailBox, int Index, int *No)
{
	MAILITEM *tpMailItem;
	int i;

	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->New == TRUE) {
			if (No != NULL) {
				*No = tpMailItem->No;
			}
			return i;
		}
	}
	return -1;
}

/*
 * item_get_next_send_mark - check Outbox for send and/or error mark
 */
int item_get_next_send_mark(MAILBOX *tpMailBox, BOOL CheckErrors, BOOL ForSend)
{
	MAILITEM *tpMailItem;
	int i, ret = -1;

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem != NULL) {
			if (CheckErrors != FALSE && tpMailItem->Mark == ICON_ERROR) {
				return i;
			} else if (tpMailItem->Mark == ICON_SEND && (ForSend == FALSE || tpMailItem->hEditWnd == NULL)) {
				if (CheckErrors != ICON_ERROR) {
					return i;
				} else {
					ret = i;
				}
			}
		}
	}
	if (CheckErrors == ICON_ERROR && ret != -1) {
		// CheckErrors == ICON_ERROR returns: # of error mark, if any; else -2 if send mark; else -1
		return -2;
	} else {
		return ret;
	}
}

/*
 * item_get_next_send_mark_mailbox - 指定のメールボックスの送信マークの付いたアイテムのインデックスを取得
 */
int item_get_next_send_mark_mailbox(MAILBOX *tpMailBox, int Index, int MailBoxIndex)
{
	MAILITEM *tpMailItem;
	int BoxIndex;
	int i;

	if (MailBoxIndex == -1) {
		return -1;
	}
	for (i = Index + 1; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL || tpMailItem->Mark != ICON_SEND || tpMailItem->hEditWnd != NULL) {
			continue;
		}
		BoxIndex = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_ACCOUNT);
		if (MailBoxIndex == BoxIndex) {
			return i;
		} else if (MailBoxIndex == MAILBOX_SEND) {
			return BoxIndex;
		}
	}
	return -1;
}

/*
 * item_set_body - NO LONGER USED
 */
static void item_set_body(MAILITEM *tpMailItem, char *buf, int download)
{
	char *p, *r;
	int len;
	int header_size;

	p = GetBodyPointa(buf);
	if (p != NULL && *p != '\0') {
		BOOL free_r = FALSE;
		// デコード
		r = MIME_body_decode_transfer(tpMailItem, p);
		if (r == NULL) {
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return;
		} else if (r != p) {
			if (download == MAIL2ITEM_WIRE || download == MAIL2ITEM_IMPORT || download == MAIL2ITEM_ATTACH) {
				tpMailItem->HasHeader = 0;
				tpMailItem->Body = r;
				return;
			}
			free_r = TRUE;
		}
		len = tstrlen(r);

		if (op.ShowHeader == 1 || KeyShowHeader == TRUE) {
			header_size = remove_superfluous_headers(buf, TRUE);
			if (header_size <= 2) header_size = 0;
		} else {
			header_size = 0;
		}

		// Allocate a new buffer, which may be smaller if:
		// a) we're not keeping headers
		// b) the body was encoded
		mem_free(&tpMailItem->Body);
		tpMailItem->Body = (char *)mem_alloc(sizeof(char) * (len + header_size + 1));
		if (tpMailItem->Body == NULL) {
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
		} else {
			if (header_size > 2 && (op.ShowHeader == 1 || KeyShowHeader == TRUE)) {
				// ヘッダ
				str_cpy_n(tpMailItem->Body, buf, header_size + 1);
				tpMailItem->HasHeader = 2;
			} else {
				tpMailItem->HasHeader = 0;
			}
			tstrcpy(tpMailItem->Body + header_size, r);
		}
		if (free_r == TRUE) {
			mem_free(&r);
		}

	} else if (download == MAIL2ITEM_WIRE || download == MAIL2ITEM_IMPORT || download == MAIL2ITEM_ATTACH) {
		// take the headers to be the body
		mem_free(&tpMailItem->Body);
		tpMailItem->Body = alloc_copy(buf);
		return;

	} else if (op.ShowHeader == 1 || KeyShowHeader == TRUE) {
		// take the headers to be the body
		mem_free(&tpMailItem->Body);
		tpMailItem->Body = alloc_copy(buf);

	} else if (download == MAIL2ITEM_RETR) {
		mem_free(&tpMailItem->Body);
		tpMailItem->Body = (char *)mem_alloc(sizeof(char));
		if (tpMailItem->Body != NULL) {
			*tpMailItem->Body = '\0';
		}
	}
	if (tpMailItem->Body != NULL) {
		tpMailItem->Mark = tpMailItem->MailStatus = ICON_MAIL;
	}
}

/*
 * item_get_npop_headers - read non-standard header info;
 *                         these headers are not expected in received mail
 */
void item_get_npop_headers(char *buf, MAILITEM *tpMailItem, MAILBOX *tpMailBox)
{
	// HEAD_X_STATUS handled in file_read_mailbox and item_set_flags
	item_get_content_t(buf, HEAD_BCC, &tpMailItem->Bcc);
	item_get_content_t(buf, HEAD_REDIRECT, &tpMailItem->RedirectTo);
	item_get_content_t(buf, HEAD_X_UIDL, &tpMailItem->UIDL);
	item_get_content_t(buf, HEAD_SIZE, &tpMailItem->Size); // could be in received message also

	item_get_content_t(buf, HEAD_X_MAILBOX, &tpMailItem->MailBox);
	item_get_content_t(buf, HEAD_X_ATTACH, &tpMailItem->Attach);
	if (tpMailItem->Attach == NULL) {
		item_get_content_t(buf, HEAD_X_ATTACH_OLD, &tpMailItem->Attach);
		if (tpMailItem->Attach != NULL) {
			TCHAR *p;
			for (p = tpMailItem->Attach; *p != TEXT('\0'); p++) {
#ifndef UNICODE
				// 2バイトコードの場合は2バイト進める
				if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
					p++;
					continue;
				}
#endif
				if (*p == TEXT(',')) {
					*p = ATTACH_SEP;
				}
			}
		}
	}
	item_get_content_t(buf, HEAD_X_FWDATTACH, &tpMailItem->FwdAttach);
	if (tpMailItem->FwdAttach != NULL && *tpMailItem->FwdAttach == TEXT('\0')) {
		mem_free(&tpMailItem->FwdAttach);
		tpMailItem->FwdAttach = NULL;
		if (tpMailBox) {
			tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
		}
	}
	if (tpMailItem->Attach != NULL || tpMailItem->FwdAttach != NULL) {
		tpMailItem->Multipart = MULTIPART_ATTACH;
		tpMailItem->AttachSize = _ttoi(tpMailItem->Size); // - strlen(body) done later
	}
	item_get_content_t(buf, HEAD_X_HEADCHARSET, &tpMailItem->HeadCharset);
	tpMailItem->HeadEncoding = item_get_content_int(buf, HEAD_X_HEADENCODE, 0);
	// BodyCharset may come from Content-Type: instead
	item_get_content_t(buf, HEAD_X_BODYCHARSET, &tpMailItem->BodyCharset);
	// BodyEncoding may come from Content-Transfer-Encoding: instead
	tpMailItem->BodyEncoding = item_get_content_int(buf, HEAD_X_BODYENCODE, 0);

	// No
	tpMailItem->No = item_get_content_int(buf, HEAD_X_NO, -1);
	if (tpMailItem->No == -1) {
		tpMailItem->No = item_get_content_int(buf, HEAD_X_NO_OLD, 0);
	}
}


/*
 * item_mail_to_item - アイテムにヘッダと本文を設定
 */
BOOL item_mail_to_item(MAILITEM *tpMailItem, char **buf, int Size, int download, int status, MAILBOX *tpMailBox)
{
	TCHAR *msgid1 = NULL, *msgid2 = NULL, *t = NULL;
	char *priority = NULL;
	char *Content;
#ifdef UNICODE
	char *dcode;
#endif
	int fret, len;
	int *do_what = NULL;
	BOOL retval = TRUE;

	mem_free(&tpMailItem->WireForm);
	tpMailItem->WireForm = *buf; // DANGER: make sure caller knows I've stolen buf!

	if (download == MAIL2ITEM_RETR) {
		// POP_RETR downloading complete message to replace headers/partial message
		mem_free(&tpMailItem->Subject);
		mem_free(&tpMailItem->From);
		mem_free(&tpMailItem->From_email);
		mem_free(&tpMailItem->To);
		mem_free(&tpMailItem->Cc);
		mem_free(&tpMailItem->ReplyTo);
		mem_free(&tpMailItem->ContentType);
		mem_free(&tpMailItem->Encoding);
		mem_free(&tpMailItem->BodyCharset);
		mem_free(&tpMailItem->Attach); // X-File: _ when attachments deleted
		mem_free(&tpMailItem->Date);
		tpMailItem->Date = NULL;
		mem_free(&tpMailItem->FmtDate);
		tpMailItem->FmtDate = NULL;
		if (Size >= 0) {
			mem_free(&tpMailItem->Size);
		}
		mem_free(&tpMailItem->MessageID);
		mem_free(&tpMailItem->InReplyTo);
		mem_free(&tpMailItem->References);
		tpMailItem->Download = TRUE;
	} else if (download == MAIL2ITEM_TOP) {
		// GJC - detect if fully downloaded
		if (Size > 0 && (int)tstrlen(*buf) >= Size - tpMailBox->MessageSizeDelta) { 
			// win32: tstrlen == Size+2
			// ppc:   tstrlen == Size
			// Yahoo! tstrlen < Size, by 3-5 bytes; use MessageSizeDelta
			tpMailItem->Download = TRUE;
		} else {
			tpMailItem->Download = FALSE;
		}
	} else if (download == MAIL2ITEM_IMPORT) {
		tpMailItem->MailStatus = tpMailItem->Mark = (ImportRead == TRUE) ? ICON_READ : ICON_MAIL;
		tpMailItem->ReFwd = ICON_NON;
		tpMailItem->Download = ImportDown;
	} else if (download == MAIL2ITEM_ATTACH) {
		// converting attached mail item
		tpMailItem->Download = TRUE;
	// } else {
		// new wire-form, flags set prior to call to item_mail_to_item
	}

	// Subject
	item_get_mime_content(*buf, HEAD_SUBJECT, &tpMailItem->Subject, FALSE);
	// From
	item_get_mime_content(*buf, HEAD_FROM, &tpMailItem->From, FALSE);
	if ((t = tpMailItem->From) != NULL) {
		TCHAR *p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(t) + 1));
		if (p != NULL) {
			GetMailAddress(t, p, NULL, FALSE);
			tpMailItem->From_email = p;
		}
	}
	// To
	item_get_mime_content(*buf, HEAD_TO, &tpMailItem->To, TRUE);
	// Cc
	item_get_mime_content(*buf, HEAD_CC, &tpMailItem->Cc, TRUE);
	// Reply-To
	item_get_mime_content(*buf, HEAD_REPLYTO, &tpMailItem->ReplyTo, FALSE);
	// Content-Type
	item_get_mime_content(*buf, HEAD_CONTENTTYPE, &tpMailItem->ContentType, FALSE);
	if (tpMailItem->ContentType != NULL) {
		// should this processing be done on char or TCHAR?
		// can charset contain non-ascii characters?
		TCHAR *p;
		len = lstrlen(TEXT("charset="));
		for (p = tpMailItem->ContentType; *p != TEXT('\0'); p++) {
			if (str_cmp_ni_t(p, TEXT("charset="), len) == 0) {
				p += len;
				if (*p == TEXT('\"')) p++;
				tpMailItem->BodyCharset = alloc_copy_t(p);
				for (p = tpMailItem->BodyCharset; *p != TEXT('\"') && *p != TEXT('\0'); p++) {
					;
				}
				*p = TEXT('\0');
				break;
			}
		}
	}

	len = lstrlen(TEXT("multipart/alternative"));
	if (tpMailItem->ContentType != NULL &&
		str_cmp_ni_t(tpMailItem->ContentType, TEXT("multipart/alternative"), len) == 0) {
		BOOL verify = FALSE;
		tpMailItem->Multipart = MULTIPART_HTML;
		// AppleMail sometimes does multipart/alternative where it should be multipart/mixed
		// op.FixContentType == 1: fix only Apple-Mail; == 2: all multipart/alternative messages
		if (op.FixContentType == 1) {
			TCHAR *p = tpMailItem->ContentType + len;
			len = lstrlen(TEXT("boundary="));
			while (*p != TEXT('\0')) {
				if (str_cmp_ni_t(p, TEXT("boundary="), len) == 0) {
					p += len;
					len = lstrlen(TEXT("Apple-Mail"));
					if (str_cmp_n_t(p, TEXT("Apple-Mail"), len) == 0) {
						verify = TRUE;
					}
					break;
				}
				p++;
			}
		} else if (op.FixContentType == 2) {
			verify = TRUE;
		}
		if (verify) {
#ifdef UNICODE
			char *ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
			tpMailItem->Multipart = multipart_verify(ContentType, *buf);
			mem_free(&ContentType);
#else
			tpMailItem->Multipart = multipart_verify(tpMailItem->ContentType, *buf);
#endif
			if (tpMailItem->Multipart != MULTIPART_HTML) {
				TCHAR *p, *q;
				//tpMailItem->OrigContentType = alloc_copy_t(tpMailItem->ContentType);
				p = q = tpMailItem->ContentType + lstrlen(TEXT("multipart/"));
				q += lstrlen(TEXT("alternative"));
				lstrcpy(p, TEXT("mixed"));
				p += lstrlen(TEXT("mixed"));
				while (*q != TEXT('\0')) {
					*(p++) = *(q++);
				}
				*p = TEXT('\0');
			}
		}
	} else if (tpMailItem->ContentType != NULL &&
		str_cmp_ni_t(tpMailItem->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0) {
		tpMailItem->Multipart = MULTIPART_CONTENT;
	}

	// Content-Transfer-Encoding
	item_get_content(*buf, HEAD_ENCODING, &Content);
	if (Content != NULL) {
		if (str_cmp_i(Content, ENCODE_7BIT) == 0) {
			tpMailItem->BodyEncoding = ENC_TYPE_7BIT;
		} else if (str_cmp_i(Content, ENCODE_8BIT) == 0) {
			tpMailItem->BodyEncoding = ENC_TYPE_8BIT;
		} else if (str_cmp_i(Content, ENCODE_BASE64) == 0) {
			tpMailItem->BodyEncoding = ENC_TYPE_BASE64;
		} else if (str_cmp_i(Content, ENCODE_Q_PRINT) == 0) {
			tpMailItem->BodyEncoding = ENC_TYPE_Q_PRINT;
		}
#ifdef UNICODE
		tpMailItem->Encoding = alloc_char_to_tchar(Content);
		mem_free(&Content);
#else
		tpMailItem->Encoding = Content;
		Content = NULL;
#endif
	}

	// Date
	item_get_content(*buf, HEAD_DATE, &Content);
	if (Content != NULL) {
		tpMailItem->Date = alloc_char_to_tchar(Content);
#ifdef UNICODE
		dcode = (char *)mem_alloc(BUF_SIZE);
		if (dcode != NULL) {
			DateConv(Content, dcode, FALSE);
			tpMailItem->FmtDate = alloc_char_to_tchar(dcode);
			mem_free(&dcode);
		}
#else
		tpMailItem->FmtDate = (char *)mem_alloc(BUF_SIZE);
		if (tpMailItem->FmtDate != NULL) {
			DateConv(Content, tpMailItem->FmtDate, FALSE);
		}
#endif
		mem_free(&Content);
	}

	// Size
	if (Size >= 0) {
		TCHAR num[20];
		wsprintf(num, TEXT("%d"), Size);
		tpMailItem->Size = alloc_copy_t(num);
	}

	// Message-Id
#ifdef UNICODE
	Content = item_get_message_id(*buf);
	if (Content != NULL) {
		tpMailItem->MessageID = alloc_char_to_tchar(Content);
		mem_free(&Content);
	}
#else
	tpMailItem->MessageID = item_get_message_id(*buf);
#endif

	// In-Reply-To
#ifdef UNICODE
	item_get_content(*buf, HEAD_INREPLYTO, &Content);
	TrimMessageId(Content);
	if (Content != NULL) {
		tpMailItem->InReplyTo = alloc_char_to_tchar(Content);
		mem_free(&Content);
	}
#else
	item_get_content(*buf, HEAD_INREPLYTO, &tpMailItem->InReplyTo);
	TrimMessageId(tpMailItem->InReplyTo);
#endif

	// References
	item_get_content(*buf, HEAD_REFERENCES, &Content);
	if (Content != NULL) {
#ifdef UNICODE
		dcode = (char *)mem_alloc(GetReferencesSize(Content, TRUE) + 1);
		if (dcode != NULL) {
			ConvReferences(Content, dcode, FALSE);
			msgid1 = alloc_char_to_tchar(dcode);

			ConvReferences(Content, dcode, TRUE);
			msgid2 = alloc_char_to_tchar(dcode);
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
		if (msgid1 != NULL && msgid2 != NULL && lstrcmp(msgid1, msgid2) == 0) {
			mem_free(&msgid2);
			msgid2 = NULL;
		}
		mem_free(&Content);
	}

	// GJC modifies MRP
	// X-Priority: is a number 1 (high) to 5 (low)
	//   officially non-standard (because of leading X-), but common
	// Priority: is a string: urgent, normal, non-urgent
	//   mentioned in RFC1327 (not for general use)
	// Importance: is string: high, normal, low
	//   mentioned in RFC2156 (SHOULD be used)

	tpMailItem->Priority = -1;
	item_get_content(*buf, HEAD_IMPORTANCE, &priority);
	if (priority != NULL && *priority != '\0') {
		if (str_cmp_ni(priority, "high", 4) == 0) {
			tpMailItem->Priority = 1;
		} else if (str_cmp_ni(priority, "low", 3) == 0) {
			tpMailItem->Priority = 5;
		} else if (str_cmp_ni(priority, "normal", 6) == 0) {
			tpMailItem->Priority = 3;
		}
	}
	mem_free(&priority);
	if (tpMailItem->Priority == -1) {
		tpMailItem->Priority = item_get_content_int(*buf, HEAD_X_PRIORITY, -1);
		if (tpMailItem->Priority == 2) {
			tpMailItem->Priority = 1;
		} else if (tpMailItem->Priority == 4) {
			tpMailItem->Priority = 5;
		}
	}
	if (tpMailItem->Priority == -1) {
		tpMailItem->Priority = 3;
		item_get_content(*buf, HEAD_PRIORITY, &priority);
		if (priority != NULL && *priority != '\0') {
			if (str_cmp_ni(priority, "urgent", 6) == 0) {
				tpMailItem->Priority = 1;
			} else if (str_cmp_ni(priority, "non-urgent", 10) == 0) {
				tpMailItem->Priority = 5;
			}
		}
		mem_free(&priority);
	}

	///////////// MRP /////////////////////
	tpMailItem->ReadReceipt = 0;
	item_get_content(*buf, HEAD_READ1, &Content);
	if(Content != NULL) {
		tpMailItem->ReadReceipt = 1;
		mem_free(&Content);
	}

	item_get_content(*buf, HEAD_READ2, &Content);
	if(Content != NULL) {
		tpMailItem->ReadReceipt = 1;
		mem_free(&Content);
	}

	tpMailItem->DeliveryReceipt = 0;
	item_get_content(*buf, HEAD_DELIVERY, &Content);
	if(Content != NULL) {
		tpMailItem->DeliveryReceipt = 1;
		mem_free(&Content);
	}
	////////////////// --- /////////////////////

	if (download == MAIL2ITEM_WIRE) {
		tpMailItem->References = alloc_copy_t(msgid1);
	} else {
		if (tpMailItem->InReplyTo == NULL || *tpMailItem->InReplyTo == TEXT('\0')) {
			// convert first References to InReplyTo
			mem_free(&tpMailItem->InReplyTo);
			tpMailItem->InReplyTo = alloc_copy_t(msgid1);
			t = msgid2;
		} else {
			t = (msgid1 != NULL && lstrcmp(tpMailItem->InReplyTo, msgid1) != 0) ? msgid1 : msgid2;
		}
		tpMailItem->References = alloc_copy_t(t);
	}
	mem_free(&msgid1);
	mem_free(&msgid2);

	if (download == MAIL2ITEM_RETR && tpMailBox != NULL && (tpMailBox->FilterEnable & FILTER_REFILTER)) {
		//GJC refilter on full download
		do_what = (int *)mem_calloc(sizeof(int) * (op.GlobalFilterCnt + tpMailBox->FilterCnt));

		// フィルタをチェック
		fret = item_filter_check(tpMailBox, *buf, do_what);
	}

	// Body
	// item_set_body(tpMailItem, *buf, download);
	if (download == MAIL2ITEM_TOP || download == MAIL2ITEM_RETR) {
		tpMailItem->Mark = tpMailItem->MailStatus = ICON_MAIL;
	}
	if (download == MAIL2ITEM_RETR) {
		mem_free(&tpMailItem->Body);
	}
	{
		char *p = GetBodyPointa(*buf);
		if (p != NULL && *p != '\0') {
			char *q = MIME_body_decode_transfer(tpMailItem, p);
			if (q == NULL) {
				tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			}
			tpMailItem->Body = q;
			tpMailItem->HasHeader = 0;
		} else {
			tpMailItem->Body = alloc_copy("\0");
		}
	}
	// Remove headers if "Keep all header lines" is not checked
	if ((download == MAIL2ITEM_RETR || download == MAIL2ITEM_TOP)
		&& op.ShowHeader == 0 && KeyShowHeader == FALSE) {
		int newlen;
		len = tstrlen(*buf);
		newlen = remove_superfluous_headers(*buf, FALSE);
		if (newlen < len) {
			tpMailItem->WireForm = alloc_copy(*buf);
			if (tpMailItem->WireForm == NULL) {
				// *buf is larger than necessary, but we failed to get a new buffer
				tpMailItem->WireForm = *buf;
			} else {
				mem_free(&*buf);
			}
		}
	}
	*buf = NULL;

	if (status != 0) {
		// set these marks before filters copy message to saveboxes
		if (status == ICON_ERROR) {
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
		}
		if (status == ICON_DOWN) {
			tpMailItem->Download = TRUE;
		}
	}

	if (do_what != NULL) {
		if (fret > FILTER_RECV) {
			retval = item_filter_execute(tpMailBox, tpMailItem, fret, do_what, TRUE);
		}
		mem_free(&do_what);
	}

	return retval;
}

/*
 * item_header_to_item - メールヘッダからアイテムを作成する
 */
MAILITEM *item_header_to_item(MAILBOX *tpMailBox, char **buf, int Size, int No, int status)
{
	MAILITEM *tpMailItem;
	int fret;
	int *do_what;
	do_what = (int *)mem_calloc(sizeof(int) * (op.GlobalFilterCnt + tpMailBox->FilterCnt));

	// フィルタをチェック
	fret = item_filter_check(tpMailBox, *buf, do_what);
	if (fret == FILTER_UNRECV) {
		mem_free(&do_what);
		return (MAILITEM *)-1;
	}

	// メール情報の確保
	tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (tpMailItem == NULL) {
		mem_free(&do_what);
		return NULL;
	}
	// ヘッダと本文を設定
	item_mail_to_item(tpMailItem, buf, Size, MAIL2ITEM_TOP, status, tpMailBox);
	tpMailItem->New = TRUE;
	tpMailItem->No = No;

	// Adding to list of mail information
	if (!(fret & FILTER_UNRECV) && item_add(tpMailBox, tpMailItem) == -1) {
		item_free(&tpMailItem, 1);
		mem_free(&do_what);
		return NULL;
	}

	// handle copy/move, forward, and mark
	if (item_filter_execute(tpMailBox, tpMailItem, fret, do_what, FALSE) == FALSE) {
		item_free(&tpMailItem, 1);
		mem_free(&do_what);
		return NULL;
	}
	mem_free(&do_what);

	if (fret & FILTER_UNRECV) {
		//When the flag which it does not receive is effective, it releases
		item_free(&tpMailItem, 1);
		return (MAILITEM *)-1;
	}
	return tpMailItem;
}

/* 
 * item_set_flags - read X-Status bits to set HasHeader, Downloaded, etc.
 */
void item_set_flags(MAILITEM *tpMailItem, MAILBOX *tpMailBox, int code)
{
	int rev, refwd, head, dwn, mrk, stat;

	// STATUS_REVISION_NUMBER
	rev = code / 100000;
	if (tpMailBox && rev != STATUS_REVISION_NUMBER/100000) {
		tpMailBox->NeedsSave |= MARKS_CHANGED;
	}
	code = code % 100000;

	// Replied/Forwarded
	refwd = code / 10000;
	tpMailItem->ReFwd = (refwd <= 9) ? (char)(refwd/2) : 0;
	if ((refwd - 2 * tpMailItem->ReFwd) && tpMailItem->MessageID) {
		// this message held for forwarding; check if outgoing message is still unsent
		if ((MailBox + MAILBOX_SEND)->Loaded) {
			int j;
			for (j = 0; j < (MailBox + MAILBOX_SEND)->MailItemCnt; j++) {
				MAILITEM *tpSendItem = *((MailBox + MAILBOX_SEND)->tpMailItem + j);
				if (tpSendItem != NULL && tpSendItem->MailStatus != ICON_SENTMAIL
					&& tpSendItem->FwdAttach != NULL && tpSendItem->References != NULL
					&& lstrcmp(tpMailItem->MessageID, tpSendItem->References) == 0) {
					tpMailItem->ReFwd |= REFWD_FWDHOLD;
					if (tpMailBox) {
						tpMailBox->HeldMail = TRUE;
					}
					break;
				}
			}
		} else {
			tpMailItem->ReFwd |= REFWD_FWDHOLD;
			if (tpMailBox) {
				tpMailBox->HeldMail = TRUE;
			}
		}
	}
	code = code % 10000;

	// HasHeader
	head = code / 1000;
	tpMailItem->HasHeader = head;
	code = code % 1000;

	// Downloaded
	dwn = code / 100;
	tpMailItem->Download = (dwn == 0) ? FALSE : TRUE;
	code = code % 100;

	// Mark
	mrk = code / 10;
	tpMailItem->Mark = (mrk <= ICON_FLAG) ? mrk : ICON_NON;
	if (tpMailBox) {
		if (tpMailItem->Mark == ICON_FLAG) {
			tpMailBox->FlagCount++;
		}
		if (tpMailItem->Mark == ICON_SEND) {
			// assume this can only happen in SendBox
			tpMailBox->NewMail++;
		}
	}

	// Status
	stat = code % 10;
	tpMailItem->MailStatus = (stat <= ICON_FLAG) ? stat : ICON_NON;
}

/*
 * item_string_to_item - 文字列からアイテムを作成する
 */
MAILITEM *item_string_to_item(MAILBOX *tpMailBox, char *buf, BOOL Import)
{
	MAILITEM *tpMailItem;
	int i;
#ifdef UNICODE
	char *din, *dout;
#endif
	TCHAR *Temp;

	tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (tpMailItem == NULL) {
		return NULL;
	}
	item_get_npop_headers(buf, tpMailItem, tpMailBox);

	item_get_content_t(buf, HEAD_SUBJECT, &tpMailItem->Subject);
	item_get_content_t(buf, HEAD_FROM, &tpMailItem->From);
	item_get_content_t(buf, HEAD_TO, &tpMailItem->To);
	item_get_content_t(buf, HEAD_CC, &tpMailItem->Cc);
	// moved to item_get_npop_headers
	//item_get_content_t(buf, HEAD_BCC, &tpMailItem->Bcc);
	//item_get_content_t(buf, HEAD_REDIRECT, &tpMailItem->RedirectTo);
	item_get_content_t(buf, HEAD_DATE, &tpMailItem->Date);
	if (tpMailItem->Date != NULL && *tpMailItem->Date != TEXT('\0')) {
#ifdef UNICODE
		din = alloc_tchar_to_char(tpMailItem->Date);
		dout = (char *)mem_alloc(BUF_SIZE);
		if (dout != NULL) {
			if(-1 == DateConv(din, dout, FALSE)) {
				// unconvert legacy date
				DateUnConv(din, dout);
				mem_free(&tpMailItem->Date);
				tpMailItem->Date = alloc_char_to_tchar(dout);
				mem_free(&din);
				din = alloc_tchar_to_char(tpMailItem->Date);
				DateConv(din, dout, FALSE);
			}
			tpMailItem->FmtDate = alloc_char_to_tchar(dout);
			mem_free(&dout);
		} else {
			tpMailItem->FmtDate = NULL;
		}
		mem_free(&din);
#else
		tpMailItem->FmtDate = (char *)mem_alloc(BUF_SIZE);
		if (tpMailItem->FmtDate != NULL) {
			if (-1 == DateConv(tpMailItem->Date, tpMailItem->FmtDate, FALSE)) {
				// unconvert legacy date
				Temp = tpMailItem->FmtDate;
				tpMailItem->FmtDate = tpMailItem->Date;
				tpMailItem->Date = Temp;
				DateUnConv(tpMailItem->FmtDate, tpMailItem->Date);
				mem_free(&tpMailItem->FmtDate);
				tpMailItem->FmtDate = (char *)mem_alloc(BUF_SIZE);
				if (tpMailItem->FmtDate != NULL) {
					DateConv(tpMailItem->Date, tpMailItem->FmtDate, FALSE);
				}
			}
		}
#endif
	}
	// moved to item_get_npop_headers
	//item_get_content_t(buf, HEAD_SIZE, &tpMailItem->Size);
	item_get_content_t(buf, HEAD_REPLYTO, &tpMailItem->ReplyTo);
	item_get_content_t(buf, HEAD_CONTENTTYPE, &tpMailItem->ContentType);
	item_get_content_t(buf, HEAD_ENCODING, &tpMailItem->Encoding);
	item_get_content_t(buf, HEAD_MESSAGEID, &tpMailItem->MessageID);
	if (tpMailBox && tpMailBox->WasMbox && tpMailBox != (MailBox + MAILBOX_SEND) && tpMailItem->MessageID == NULL) {
#ifdef UNICODE
		char *Content;
		Content = item_get_message_id(buf);
		if (Content != NULL) {
			tpMailItem->MessageID = alloc_char_to_tchar(Content);
			mem_free(&Content);
		}
#else
		tpMailItem->MessageID = item_get_message_id(buf);
#endif
	}
	item_get_content_t(buf, HEAD_INREPLYTO, &tpMailItem->InReplyTo);
	item_get_content_t(buf, HEAD_REFERENCES, &tpMailItem->References);
	//if (tpMailItem->FwdAttach != NULL && tpMailItem->References == NULL) {
	//	mem_free(&tpMailItem->FwdAttach);
	//	tpMailItem->FwdAttach = NULL;
	//	if (tpMailBox) {
	//		tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
	//	}
	//}
	
	///////////// MRP /////////////////////
	// MRP - HEAD_X_PRIORITY
	tpMailItem->Priority = item_get_content_int(buf, HEAD_X_PRIORITY, 3);
	// if (tpMailBox->WasMbox) may need to convert HEAD_IMPORTANCE, HEAD_PRIORITY
	
	tpMailItem->ReadReceipt = 0;
	item_get_content_t(buf, HEAD_READ1, &Temp);
	if (Temp != NULL) {
		tpMailItem->ReadReceipt = 1;
		mem_free(&Temp);
	}

	item_get_content_t(buf, HEAD_READ2, &Temp);
	if(Temp != NULL) {
		tpMailItem->ReadReceipt = 1;
		mem_free(&Temp);
	}

	tpMailItem->DeliveryReceipt = 0;
	item_get_content_t(buf, HEAD_DELIVERY, &Temp);
	if(Temp != NULL) {
		tpMailItem->DeliveryReceipt = 1;
		mem_free(&Temp);
	}
	//////////////////--- /////////////////////

	tpMailItem->New = FALSE;
	// GJC: MailStatus, Mark, Download, and HasHeader combined into X-Status: header
	// order of codes must match item_set_flags!
	item_get_content_t(buf, HEAD_X_STATUS, &Temp);
	if (Temp != NULL) {
		i = _ttoi(Temp);
	} else {
		i = -1;
	}
	// GJC:  100000 was the first STATUS_REVISION_NUMBER in the new system
	if (i >= 100000 && lstrlen(Temp) >= 6) {
		item_set_flags(tpMailItem, tpMailBox, i);
		
	} else if (Import == TRUE) {
		tpMailItem->MailStatus = tpMailItem->Mark = (ImportRead == TRUE) ? ICON_READ : ICON_MAIL;
		tpMailItem->ReFwd = ICON_NON;
		tpMailItem->Download = ImportDown;

	} else{ // legacy status
		// MailStatus
		tpMailItem->MailStatus = i;
		if (tpMailItem->MailStatus == -1) {
			tpMailItem->MailStatus = item_get_content_int(buf, HEAD_X_STATUS_OLD, 0);
		}
		// MarkStatus
		i = item_get_content_int(buf, HEAD_X_MARK, -1);
		tpMailItem->Mark = (i != -1) ? i : tpMailItem->MailStatus;
		// Download
		tpMailItem->Download = item_get_content_int(buf, HEAD_X_DOWNLOAD, -1);
		if (tpMailItem->Download == -1) {
			tpMailItem->Download = item_get_content_int(buf, HEAD_X_DOWNLOAD_OLD, 0);
		}
		// Full header info
		tpMailItem->HasHeader = item_get_content_int(buf, HEAD_X_HASHEADER, 0);
		// Replied or forwarded
		tpMailItem->ReFwd = ICON_NON;
	}
	mem_free(&Temp);

	// Multipart
	if (tpMailBox == MailBox + MAILBOX_SEND) {
		// tpMailItem->Multipart already set in item_get_npop_headers
		if (tpMailItem->RedirectTo == NULL && tpMailItem->ContentType != NULL) {
			// fix bug in previous versions: forwarded messages copied Content-Type of original
			mem_free(&tpMailItem->ContentType);
			if (tpMailBox) {
				tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
			}
		}
	} else if (tpMailItem->ContentType != NULL &&
		str_cmp_ni_t(tpMailItem->ContentType, TEXT("multipart/alternative"), lstrlen(TEXT("multipart/alternative"))) == 0) {
		tpMailItem->Multipart = MULTIPART_HTML;
	} else if (tpMailItem->ContentType != NULL &&
		str_cmp_ni_t(tpMailItem->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0) {
		tpMailItem->Multipart = MULTIPART_CONTENT;
	//} else {
	//	tpMailItem->Multipart = MULTIPART_NONE;
	}

	if (tpMailItem->MailStatus == ICON_SENTMAIL || tpMailItem->MailStatus == ICON_SEND) {
		if (Temp = tpMailItem->To) {
			TCHAR *p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Temp) + 1));
			if (p != NULL) {
				GetMailAddress(Temp, p, NULL, FALSE);
				tpMailItem->From_email = p;
			}
		}
	} else {
		if (Temp = tpMailItem->From) {
			TCHAR *p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Temp) + 1));
			if (p != NULL) {
				GetMailAddress(Temp, p, NULL, FALSE);
				tpMailItem->From_email = p;
			}
		}
	}
	return tpMailItem;
}

/*
 * item_save_header_size - 保存するヘッダのサイズ
 */
static int item_save_header_size(TCHAR *header, TCHAR *buf)
{
	if (buf == NULL) {
		return 0;
	}
#ifdef UNICODE
	return (tchar_to_char_size(header) + 1 + tchar_to_char_size(buf) + 2 - 2);
#else
	return (lstrlen(header) + 1 + lstrlen(buf) + 2);
#endif
}

/*
 * item_save_header - ヘッダを保存する文字列の作成
 */
static char *item_save_header(TCHAR *header, TCHAR *buf, char *ret)
{
#ifdef UNICODE
	TCHAR *wret;
	int len;

	if (buf == NULL) {
		return ret;
	}
	wret = mem_alloc(sizeof(TCHAR) * (lstrlen(header) + 1 + lstrlen(buf) + 2 + 1));
	if (wret == NULL) {
		return ret;
	}
	str_join_t(wret, header, TEXT(" "), buf, TEXT("\r\n"), (TCHAR *)-1);
	len = tchar_to_char_size(wret);
	tchar_to_char(wret, ret, len);
	mem_free(&wret);
	*(ret + len) = '\0';
	return (ret + len - 1);
#else
	if (buf == NULL) {
		return ret;
	}
	return str_join_t(ret, header, TEXT(" "), buf, TEXT("\r\n"), (TCHAR *)-1);
#endif
}

/*
 * item_join_header
 */
static char *item_join_header(char **ret, char *p, char *header, TCHAR *content, 
							  MAILITEM *tpMailItem, BOOL address, int *len, int *size)
{
	if (content != NULL && *ret != NULL) {
		char *buf = NULL;
		int hlen = tstrlen(header);

		buf = MIME_encode_opt(content, address, tpMailItem->HeadCharset, tpMailItem->HeadEncoding, hlen);		
		if (buf != NULL) {
			hlen += tstrlen(buf) + 4; // " \r\n\0"
			if (*len + hlen >= *size) {
				char *tmp;
				*size += (hlen > BLOCK_SIZE) ? hlen : BLOCK_SIZE;
				p = tmp = (char *)mem_alloc(sizeof(char) * (*size));
				if (tmp == NULL) {
					return NULL;
				}
				p = str_cpy(p, *ret);
				mem_free(&*ret);
				*ret = tmp;
			}
			p = str_join(p, header, " ", buf, "\r\n", (char *)-1);
			mem_free(&buf);
			*len += hlen;
		}
	}
	return p;
}

/*
 * item_create_wireform - create wire-form (for outgoing messages) (GJC)
 */
char *item_create_wireform(MAILITEM *tpMailItem, TCHAR *body)
{
	int len = 0, size, enc, prio, box;
	TCHAR ErrStr[BUF_SIZE];
	TCHAR *BodyCSet;
	char *buf, *p, *tmp, *send_body;
	char ctype[BUF_SIZE], enc_type[BUF_SIZE];

	// specify header encoding here, but don't put it in the wireform
	if (tpMailItem->HeadCharset == NULL) {
		tpMailItem->HeadCharset = alloc_copy_t(op.HeadCharset);
		tpMailItem->HeadEncoding = op.HeadEncoding;
	}

	// determine Content-Type, Content-Transfer-Encoding
	if (tpMailItem->BodyCharset != NULL) {
		BodyCSet = tpMailItem->BodyCharset;
		enc = tpMailItem->BodyEncoding;
	} else {
		BodyCSet = op.BodyCharset;
		enc = op.BodyEncoding;
	}
	send_body = MIME_body_encode(body, BodyCSet, enc, NULL, ctype, enc_type, ErrStr);
	mem_free(&tpMailItem->ContentType);
	tpMailItem->ContentType = alloc_char_to_tchar(ctype);
	mem_free(&tpMailItem->Encoding);
	tpMailItem->Encoding = alloc_char_to_tchar(enc_type);

	// initial buffer size; buffer is grown as needed
	size = BLOCK_SIZE;
	if (send_body != NULL) {
		size += tstrlen(send_body);
	}
	buf = p = (char *)mem_alloc(sizeof(char) * size);
	if (buf == NULL) {
		mem_free(&send_body);
		return NULL;
	}
	*p = '\0';

	// Handle Redirect and UseReplyToForFrom
	box = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_ACCOUNT);
	if (box != -1) {
		MAILBOX *send_mail_box = MailBox + box;
		TCHAR *FromAddress, *q, *r, *s;

		if (send_mail_box->UseReplyToForFrom == 1
			&& tpMailItem->ReplyTo != NULL && *tpMailItem->ReplyTo != TEXT('\0')) {
			FromAddress = tpMailItem->ReplyTo;
		} else {
			FromAddress = send_mail_box->MailAddress;
		}
		if (FromAddress != NULL && *FromAddress != TEXT('\0')) {
			len = lstrlen(TEXT(" <>"));
			s = NULL;

			if (send_mail_box->UserName != NULL) {
				s = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(send_mail_box->UserName) + 1));
				if (s != NULL) {
					SetUserName(send_mail_box->UserName, s);
					len += lstrlen(s);
				}
			}
			len += lstrlen(FromAddress);
			
			r = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
			if (r != NULL) {
				q = r;
				if (s != NULL && *s != TEXT('\0')) {
					q = str_join_t(q, s, TEXT(" "), (TCHAR *)-1);
				}
				str_join_t(q, TEXT("<"), FromAddress, TEXT(">"), (TCHAR *)-1);
			}
			mem_free(&s);
			if (tpMailItem->RedirectTo != NULL) {
				len += lstrlen(tpMailItem->RedirectTo) + lstrlen(STR_MSG_BYWAYOF);
				s = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
				if (s != NULL) {
					wsprintf(s, STR_MSG_BYWAYOF, tpMailItem->RedirectTo, r);
					mem_free(&r);
					r = s;
				}
			}
		} else {
			r = NULL;
		}
	
		if (tpMailItem->RedirectTo != NULL) {
			mem_free(&tpMailItem->To);
			tpMailItem->To = r;
		} else {
			mem_free(&tpMailItem->From);
			tpMailItem->From = r;
		}
	}

	p = item_join_header(&buf, p, HEAD_FROM, tpMailItem->From, tpMailItem, TRUE, &len, &size);
	p = item_join_header(&buf, p, HEAD_TO, tpMailItem->To, tpMailItem, TRUE, &len, &size);
	p = item_join_header(&buf, p, HEAD_CC, tpMailItem->Cc, tpMailItem, TRUE, &len, &size);
	// don't put tpMailItem->Bcc in the wireform
	p = item_join_header(&buf, p, HEAD_DATE, tpMailItem->Date, tpMailItem, FALSE, &len, &size);
	// don't save tpMailItem->FmtDate
	p = item_join_header(&buf, p, HEAD_SUBJECT, tpMailItem->Subject, tpMailItem, FALSE, &len, &size);
	p = item_join_header(&buf, p, HEAD_SIZE, tpMailItem->Size, tpMailItem, FALSE, &len, &size);
	p = item_join_header(&buf, p, HEAD_REPLYTO, tpMailItem->ReplyTo, tpMailItem, TRUE, &len, &size);
	p = item_join_header(&buf, p, HEAD_CONTENTTYPE, tpMailItem->ContentType, tpMailItem, FALSE, &len, &size);
	p = item_join_header(&buf, p, HEAD_ENCODING, tpMailItem->Encoding, tpMailItem, FALSE, &len, &size);
	p = item_join_header(&buf, p, HEAD_MESSAGEID, tpMailItem->MessageID, tpMailItem, FALSE, &len, &size);
	p = item_join_header(&buf, p, HEAD_INREPLYTO, tpMailItem->InReplyTo, tpMailItem, FALSE, &len, &size);
	p = item_join_header(&buf, p, HEAD_REFERENCES, tpMailItem->References, tpMailItem, FALSE, &len, &size);

	// GJC optimizes MRP
	prio = tpMailItem->Priority;
	if (prio == 2) prio = 1;
	if (prio == 4) prio = 5;
	p = item_join_header(&buf, p, HEAD_X_PRIORITY, 
			(prio==1) ? PRIORITY_NUMBER1 : ( (prio==5) ? PRIORITY_NUMBER5 : PRIORITY_NUMBER3),
			tpMailItem, FALSE, &len, &size);
	p = item_join_header(&buf, p, HEAD_IMPORTANCE,
			(prio==1) ? HIGH_PRIORITY : ( (prio==5) ? LOW_PRIORITY : NORMAL_PRIORITY),
			tpMailItem, FALSE, &len, &size);

	//////////////////MRP /////////////////////
	if (tpMailItem->ReadReceipt == 1) {
		p = item_join_header(&buf, p, HEAD_READ1, tpMailItem->From, tpMailItem, FALSE, &len, &size);
		p = item_join_header(&buf, p, HEAD_READ2, tpMailItem->From, tpMailItem, FALSE, &len, &size);
	}

	if (tpMailItem->DeliveryReceipt == 1) {
		p = item_join_header(&buf, p, HEAD_DELIVERY, tpMailItem->From, tpMailItem, FALSE, &len, &size);
	}
	//////////////////--- /////////////////////

	len += tstrlen(send_body) + 2 + 1;

	if (len > size) {
		tmp = (char *)mem_alloc(sizeof(char) * len);
		if (tmp != NULL) {
			p = str_cpy(tmp, buf);
			mem_free(&buf);
			buf = tmp;
		} else {
			mem_free(&buf);
			buf = NULL;
		}
	}

	if (buf != NULL) {
		p = str_cpy(p, "\r\n");
		p = str_cpy(p, send_body);
	}
	mem_free(&send_body);

	return buf;
}
	
/*
 * item_to_string_size - メールの保存文字列のサイズ取得
 */
int item_to_string_size(MAILITEM *tpMailItem, int WriteMbox, BOOL BodyFlag, BOOL SepFlag)
{
	TCHAR X_HeadEncoding[10], X_BodyEncoding[10];
	TCHAR X_No[10], X_Mstatus[10];
	int do_body, len = 0;

	do_body = (BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != '\0') ? 1 : 0;

	wsprintf(X_HeadEncoding, TEXT("%d"), tpMailItem->HeadEncoding);
	wsprintf(X_BodyEncoding, TEXT("%d"), tpMailItem->BodyEncoding);
	if (tpMailItem->WireForm != NULL) {
		wsprintf(X_Mstatus, TEXT("%d"), STATUS_REVISION_NUMBER);
	} else {
		wsprintf(X_Mstatus, TEXT("%d"), STATUS_REVISION_OLD);
	}
	wsprintf(X_No, TEXT("%d"), tpMailItem->No);

	if (SepFlag && WriteMbox == 1) {
		len = tstrlen(NPOPUK_MBOX_DELIMITER);
	}
	if (tpMailItem->WireForm != NULL) {
		char *bdy = GetBodyPointa(tpMailItem->WireForm);

		// These headers are modified by nPOPuk and don't belong
		// in the WireForm even for outgoing messages
		len += item_save_header_size(TEXT(HEAD_X_UIDL), tpMailItem->UIDL);
		len += item_save_header_size(TEXT(HEAD_X_MAILBOX), tpMailItem->MailBox);
		len += item_save_header_size(TEXT(HEAD_SIZE), tpMailItem->Size);
		if (tpMailItem->HeadCharset != NULL) {
			len += item_save_header_size(TEXT(HEAD_X_HEADCHARSET), tpMailItem->HeadCharset);
			len += item_save_header_size(TEXT(HEAD_X_HEADENCODE), X_HeadEncoding);
		}
		// BodyCharset saved in Content-Type:
		// BodyEncoding saved in Content-Transfer-Encoding:

		len += item_save_header_size(TEXT(HEAD_BCC), tpMailItem->Bcc);
		len += item_save_header_size(TEXT(HEAD_REDIRECT), tpMailItem->RedirectTo);
		len += item_save_header_size(TEXT(HEAD_X_ATTACH), tpMailItem->Attach);
		len += item_save_header_size(TEXT(HEAD_X_FWDATTACH), tpMailItem->FwdAttach);
		len += item_save_header_size(TEXT(HEAD_X_NO), X_No);
		// X-Status should always be the last nPOPuk-specific header
		len += item_save_header_size(TEXT(HEAD_X_STATUS), X_Mstatus);
		if (bdy != NULL) {
			if (do_body == 0) {
				len += (bdy - tpMailItem->WireForm);
			} else {
				len += tstrlen(tpMailItem->WireForm);
				if (WriteMbox == 1) {
					char *r = bdy;
					int l = tstrlen(MBOX_DELIMITER);
					if (str_cmp_n(r, MBOX_DELIMITER, l) == 0) {
						len++;
					}
					for ( /**/ ; *r != '\0'; r++, len++) {
						if (*r == '\r' && *(r+1) == '\n' && str_cmp_n(r+2, MBOX_DELIMITER, l) == 0) {
							len++;
						}
					}
				}
			}
		} else {
			len += tstrlen(tpMailItem->WireForm);
		}
		if (WriteMbox != 0) {
			len += 3; // \r\n\0
		} else {
			len += 6; // \r\n.\r\n\0
		}
	} else {
		// No WireForm -- old message read from file
		if (lstrcmpi(op.Codepage, TEXT("CP_ACP")) == 0) {
			// nPOPuk 2.16 and earlier did all MultiByteToWideChar conversions using CP_ACP.
			CP_int = CP_ACP;
		}

		len += item_save_header_size(TEXT(HEAD_FROM), tpMailItem->From);
		len += item_save_header_size(TEXT(HEAD_TO), tpMailItem->To);
		len += item_save_header_size(TEXT(HEAD_CC), tpMailItem->Cc);
		len += item_save_header_size(TEXT(HEAD_BCC), tpMailItem->Bcc);
		len += item_save_header_size(TEXT(HEAD_REDIRECT), tpMailItem->RedirectTo);
		len += item_save_header_size(TEXT(HEAD_DATE), tpMailItem->Date);
		// don't save tpMailItem->FmtDate
		len += item_save_header_size(TEXT(HEAD_SUBJECT), tpMailItem->Subject);
		len += item_save_header_size(TEXT(HEAD_SIZE), tpMailItem->Size);
		len += item_save_header_size(TEXT(HEAD_REPLYTO), tpMailItem->ReplyTo);
		len += item_save_header_size(TEXT(HEAD_CONTENTTYPE), tpMailItem->ContentType);
		len += item_save_header_size(TEXT(HEAD_ENCODING), tpMailItem->Encoding);
		len += item_save_header_size(TEXT(HEAD_MESSAGEID), tpMailItem->MessageID);
		len += item_save_header_size(TEXT(HEAD_INREPLYTO), tpMailItem->InReplyTo);
		len += item_save_header_size(TEXT(HEAD_REFERENCES), tpMailItem->References);

		// actual value doesn't matter, only the size
		len += item_save_header_size(TEXT(HEAD_X_PRIORITY), PRIORITY_NUMBER3);
	   
		if (tpMailItem->ReadReceipt == 1) {
			len += item_save_header_size(TEXT(HEAD_READ1), tpMailItem->From);
			len += item_save_header_size(TEXT(HEAD_READ2), tpMailItem->From);
		}

		if (tpMailItem->DeliveryReceipt == 1) {
			len += item_save_header_size(TEXT(HEAD_DELIVERY), tpMailItem->From);
		}

		len += item_save_header_size(TEXT(HEAD_X_UIDL), tpMailItem->UIDL);

		len += item_save_header_size(TEXT(HEAD_X_MAILBOX), tpMailItem->MailBox);
		len += item_save_header_size(TEXT(HEAD_X_ATTACH), tpMailItem->Attach);
		len += item_save_header_size(TEXT(HEAD_X_FWDATTACH), tpMailItem->FwdAttach);
		if (tpMailItem->HeadCharset != NULL) {
			len += item_save_header_size(TEXT(HEAD_X_HEADCHARSET), tpMailItem->HeadCharset);
			len += item_save_header_size(TEXT(HEAD_X_HEADENCODE), X_HeadEncoding);
		}
		if (tpMailItem->BodyCharset != NULL) {
			len += item_save_header_size(TEXT(HEAD_X_BODYCHARSET), tpMailItem->BodyCharset);
			len += item_save_header_size(TEXT(HEAD_X_BODYENCODE), X_BodyEncoding);
		}

		len += item_save_header_size(TEXT(HEAD_X_NO), X_No);
		len += item_save_header_size(TEXT(HEAD_X_STATUS), X_Mstatus);
		if (do_body && (WriteMbox == 0 || tpMailItem->HasHeader == 0)) {
			len += 2;
		}

		if (do_body) {
			// OK, this is goofy: the message was in CP_ACP, but we had to convert the body to UTF-8
			// when we read it in, so that everywhere in the program that we use the body, we don't have
			// to think about what format it's in.  So, now we have to take the body, convert from UTF-8
			// to wchar and then back to char using CP_ACP!
#ifdef UNICODE
			TCHAR *wbody;
			CP_int = CP_UTF8;
			wbody = alloc_char_to_tchar(tpMailItem->Body);
			CP_int = CP_ACP;
			if (wbody != NULL) {
				len += tchar_to_char_size(wbody);
				mem_free(&wbody);
			} else {
				// hope for the best?
				len += tstrlen(tpMailItem->Body);
			}
#else
			len += tstrlen(tpMailItem->Body);
#endif
			if (WriteMbox == 1) {
				// Need to add ">" if MBOX_DELIMITER ever occurs in the message;
				// MBOX_DELIMITER is ascii, so we don't care about the codepage.
				char *r = tpMailItem->Body;
				int l = tstrlen(MBOX_DELIMITER);
				if (str_cmp_n(r, MBOX_DELIMITER, l) == 0) {
					len++;
				}
				for ( /**/ ; *r != '\0'; r++) {
					if (*r == '\r' && *(r+1) == '\n' && str_cmp_n(r+2, MBOX_DELIMITER, l) == 0) {
						len++;
					}
				}
			}
		}
		CP_int = CP_UTF8; // restore to UTF8
	}
	if (WriteMbox != 0) {
		len += 3; // \r\n\0
	} else {
		len += 6; // \r\n.\r\n\0
	}
	return len;
}

/*
 * item_to_string - convert mailitem to string for saving to file
 */
char *item_to_string(char *buf, MAILITEM *tpMailItem, int WriteMbox, BOOL BodyFlag, BOOL SepFlag)
{
	char *p = buf;
	TCHAR X_HeadEncoding[10], X_BodyEncoding[10];
	TCHAR X_No[10], X_Mstatus[10];
	int do_body, prio, composite_status;

	do_body = (BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != '\0') ? 1 : 0;

	wsprintf(X_HeadEncoding, TEXT("%d"), tpMailItem->HeadEncoding);
	wsprintf(X_BodyEncoding, TEXT("%d"), tpMailItem->BodyEncoding);

	// GJC: order of codes must match item_set_flags!
	composite_status = 
		((tpMailItem->WireForm != NULL) ? STATUS_REVISION_NUMBER : STATUS_REVISION_OLD)
		+ 10000 * (2 * (tpMailItem->ReFwd & ICON_REFWD_MASK) + ((tpMailItem->ReFwd & REFWD_FWDHOLD) != 0))
		+  1000 * tpMailItem->HasHeader * do_body
		+   100 * ((tpMailItem->Download == TRUE) ? do_body : 0)
		+    10 * tpMailItem->Mark
		+         tpMailItem->MailStatus;
	wsprintf(X_Mstatus, TEXT("%d"), composite_status);
	wsprintf(X_No, TEXT("%d"), tpMailItem->No);

	if (SepFlag && WriteMbox == 1) {
		p = str_cpy(p, NPOPUK_MBOX_DELIMITER);
	}
	if (tpMailItem->WireForm != NULL) {
		char *r, *bdy = GetBodyPointa(tpMailItem->WireForm);

		// These headers are modified by nPOPuk and don't belong
		// in the WireForm even for outgoing messages
		p = item_save_header(TEXT(HEAD_X_UIDL), tpMailItem->UIDL, p);
		p = item_save_header(TEXT(HEAD_X_MAILBOX), tpMailItem->MailBox, p);
		p = item_save_header(TEXT(HEAD_SIZE), tpMailItem->Size, p);
		if (tpMailItem->HeadCharset != NULL) {
			p = item_save_header(TEXT(HEAD_X_HEADCHARSET), tpMailItem->HeadCharset, p);
			p = item_save_header(TEXT(HEAD_X_HEADENCODE), X_HeadEncoding, p);
		}
		// BodyCharset saved in Content-Type:
		// BodyEncoding saved in Content-Transfer-Encoding:
		p = item_save_header(TEXT(HEAD_BCC), tpMailItem->Bcc, p);
		p =	item_save_header(TEXT(HEAD_REDIRECT), tpMailItem->RedirectTo, p);
		p = item_save_header(TEXT(HEAD_X_ATTACH), tpMailItem->Attach, p);
		p = item_save_header(TEXT(HEAD_X_FWDATTACH), tpMailItem->FwdAttach, p);
		p = item_save_header(TEXT(HEAD_X_NO), X_No, p);
		// X-Status should always be the last nPOPuk-specific header
		p = item_save_header(TEXT(HEAD_X_STATUS), X_Mstatus, p);
		// append wire-form after HEAD_X_STATUS (includes original headers and body)
		if (bdy != NULL) {
			if (do_body == 0) {
				// bdy includes an extra \r\n (or maybe just \r or \n, see GetBodyPointa)
				if ( (bdy - tpMailItem->WireForm > 4)
						&& *(bdy-4) == '\r' && *(bdy-3) == '\n'
						&& *(bdy-2) == '\r' && *(bdy-1) == '\n' ) {
					bdy -=2;
				} else if ( (bdy - tpMailItem->WireForm > 2) 
							&& ((*(bdy-1) == '\r' && *(bdy-2) == '\r')
							|| (*(bdy-1) == '\n' && *(bdy-2) == '\n')) ) {
					bdy--;
				}
				for (r = tpMailItem->WireForm; r < bdy; r++) {
					*(p++) = *r;
				}
			} else {
				if (WriteMbox == 1) {
					char *r = tpMailItem->WireForm;
					int l = tstrlen(MBOX_DELIMITER);
					while (r < bdy) {
						// just copy headers
						*(p++) = *(r++);
					}
					if (str_cmp_n(r, MBOX_DELIMITER, l) == 0) {
						*(p++) = '>';
					}
					while (*r != '\0') {
						if (*r == '\r' && *(r+1) == '\n' && str_cmp_n(r+2, MBOX_DELIMITER, l) == 0) {
							*(p++) = *(r++);
							*(p++) = *(r++);
							*(p++) = '>';
						} else {
							*(p++) = *(r++);
						}
					}
				} else {
					p = str_cpy(p, tpMailItem->WireForm);
				}
			}
		} else {
			p = str_cpy(p, tpMailItem->WireForm);
		}
	} else {
		// No WireForm -- old message read from file
		if (lstrcmpi(op.Codepage, TEXT("CP_ACP")) == 0) {
			// nPOPuk 2.16 and earlier did all MultiByteToWideChar conversions using CP_ACP.
			CP_int = CP_ACP;
		}

		p = item_save_header(TEXT(HEAD_FROM), tpMailItem->From, p);
		p = item_save_header(TEXT(HEAD_TO), tpMailItem->To, p);
		p = item_save_header(TEXT(HEAD_CC), tpMailItem->Cc, p);
		p = item_save_header(TEXT(HEAD_BCC), tpMailItem->Bcc, p);
		p =	item_save_header(TEXT(HEAD_REDIRECT), tpMailItem->RedirectTo, p);
		p = item_save_header(TEXT(HEAD_DATE), tpMailItem->Date, p);
		// don't save tpMailItem->FmtDate
		p = item_save_header(TEXT(HEAD_SUBJECT), tpMailItem->Subject, p);
		p = item_save_header(TEXT(HEAD_SIZE), tpMailItem->Size, p);
		p = item_save_header(TEXT(HEAD_REPLYTO), tpMailItem->ReplyTo, p);
		p = item_save_header(TEXT(HEAD_CONTENTTYPE), tpMailItem->ContentType, p);
		p = item_save_header(TEXT(HEAD_ENCODING), tpMailItem->Encoding, p);
		p = item_save_header(TEXT(HEAD_MESSAGEID), tpMailItem->MessageID, p);
		p = item_save_header(TEXT(HEAD_INREPLYTO), tpMailItem->InReplyTo, p);
		p = item_save_header(TEXT(HEAD_REFERENCES), tpMailItem->References, p);

		// GJC optimizes MRP
		prio = tpMailItem->Priority;
		if (prio == 2) prio = 1;
		if (prio == 4) prio = 5;
		p = item_save_header(TEXT(HEAD_X_PRIORITY), 
			(prio==1) ? PRIORITY_NUMBER1 : ( (prio==5) ? PRIORITY_NUMBER5 : PRIORITY_NUMBER3), p);

		//////////////////MRP /////////////////////
		if (tpMailItem->ReadReceipt == 1) {
			p = item_save_header(TEXT(HEAD_READ1), tpMailItem->From, p);
			p = item_save_header(TEXT(HEAD_READ2), tpMailItem->From, p);
		}

		if (tpMailItem->DeliveryReceipt == 1) {
			p = item_save_header(TEXT(HEAD_DELIVERY), tpMailItem->From, p);
		}
		//////////////////--- /////////////////////

		p = item_save_header(TEXT(HEAD_X_UIDL), tpMailItem->UIDL, p);

		p = item_save_header(TEXT(HEAD_X_MAILBOX), tpMailItem->MailBox, p);
		p = item_save_header(TEXT(HEAD_X_ATTACH), tpMailItem->Attach, p);
		p = item_save_header(TEXT(HEAD_X_FWDATTACH), tpMailItem->FwdAttach, p);
		if (tpMailItem->HeadCharset != NULL) {
			p = item_save_header(TEXT(HEAD_X_HEADCHARSET), tpMailItem->HeadCharset, p);
			p = item_save_header(TEXT(HEAD_X_HEADENCODE), X_HeadEncoding, p);
		}
		if (tpMailItem->BodyCharset != NULL) {
			p = item_save_header(TEXT(HEAD_X_BODYCHARSET), tpMailItem->BodyCharset, p);
			p = item_save_header(TEXT(HEAD_X_BODYENCODE), X_BodyEncoding, p);
		}

		p = item_save_header(TEXT(HEAD_X_NO), X_No, p);
		// HEAD_X_STATUS should be the last header written by nPOPuk!
		p = item_save_header(TEXT(HEAD_X_STATUS), X_Mstatus, p);
		if (do_body && (WriteMbox == 0 || tpMailItem->HasHeader == 0)) {
			p = str_cpy(p, "\r\n");
		}

		if (do_body) {
			char *acp_body = tpMailItem->Body;
			// OK, this is goofy: the message was in CP_ACP, but we had to convert the body to UTF-8
			// when we read it in, so that everywhere in the program that we use the body, we don't have
			// to think about what format it's in.  So, now we have to take the body, convert from UTF-8
			// to wchar and then back to char using CP_ACP!
#ifdef UNICODE
			TCHAR *wbody;
			CP_int = CP_UTF8;
			wbody = alloc_char_to_tchar(tpMailItem->Body);
			CP_int = CP_ACP;
			if (wbody != NULL) {
				acp_body = alloc_tchar_to_char(wbody);
				mem_free(&wbody);
			// } else {
				// hope there aren't any non-ascii characters ...
				// acp_body = tpMailItem->Body;
			}
#endif
			if (WriteMbox == 1) {
				char *r = acp_body;
				int l = tstrlen(MBOX_DELIMITER);
				if (str_cmp_n(r, MBOX_DELIMITER, l) == 0) {
					*(p++) = '>';
				}
				while (*r != '\0') {
					if (*r == '\r' && *(r+1) == '\n' && str_cmp_n(r+2, MBOX_DELIMITER, l) == 0) {
						*(p++) = *(r++);
						*(p++) = *(r++);
						*(p++) = '>';
					} else {
						*(p++) = *(r++);
					}
				}
			} else {
				p = str_cpy(p, acp_body);
			}
#ifdef UNICODE
			if (acp_body != tpMailItem->Body) {
				mem_free(&acp_body);
			}
#endif
		}
		CP_int = CP_UTF8; // restore to UTF8
	}
	if (WriteMbox != 0) {
		p = str_cpy(p, "\r\n");
	} else {
		p = str_cpy(p, "\r\n.\r\n");
	}
	return p;
}

/*
 * item_filter_check_content - 文字列のチェック
 */
static BOOL item_filter_check_content(char *buf, TCHAR *filter_header, TCHAR *filter_content)
{
	TCHAR *Content;
	BOOL ret;
	int len, comptype=0;
	char *cbuf;

	if (filter_content == NULL || *filter_content == TEXT('\0')) {
		return TRUE;
	}
	if (lstrcmpi(filter_header, TEXT("body")) == 0) {
		char *p;
#ifdef UNICODE
		cbuf = alloc_tchar_to_char(filter_content);
#else
		cbuf = filter_content;
#endif
		// 本文の位置取得
		p = GetBodyPointa(buf);
		if (p == NULL) {
			ret = str_match(cbuf, "");
		} else {
			// 比較
			ret = str_match(cbuf, p);
		}
#ifdef UNICODE
		mem_free(&cbuf);
#endif
		return ret;
	}
#ifdef UNICODE
	cbuf = alloc_tchar_to_char(filter_header);
	if (cbuf == NULL) {
		return FALSE;
	}
	// コンテンツの取得
	len = item_get_mime_content(buf, cbuf, &Content, TRUE);
	mem_free(&cbuf);
#else
	// コンテンツの取得
	len = item_get_mime_content(buf, filter_header, &Content, TRUE);
#endif
	if (Content == NULL) {
		return str_match_t(filter_content, TEXT(""));
	}
#define FILTER_IN_ADDRESSBOOK TEXT("**INADDRESSBOOK")
#define FILTER_IN_GROUP TEXT("**INGROUP")
// Important that the next two have the same length!
#define FILTER_DATE_OLDER TEXT("**OLDERTHAN")
#define FILTER_DATE_NEWER TEXT("**NEWERTHAN")

	if (lstrcmpi(filter_header, TEXT("from:")) == 0) {
		len = lstrlen(FILTER_IN_ADDRESSBOOK);
		if (str_cmp_ni_t(filter_content, FILTER_IN_ADDRESSBOOK, len) == 0) {
			comptype = 10;
		}
		len = lstrlen(FILTER_IN_GROUP);
		if (str_cmp_ni_t(filter_content, FILTER_IN_GROUP, len) == 0) {
			comptype = 100;
		}
	} else if (lstrcmpi(filter_header, TEXT("date:")) == 0) {
		len = lstrlen(FILTER_DATE_OLDER);
		if (str_cmp_ni_t(filter_content, FILTER_DATE_OLDER, len) == 0) {
			comptype = -1;
		} else if (str_cmp_ni_t(filter_content, FILTER_DATE_NEWER, len) == 0) {
			comptype = 1;
		}
	}
	if (comptype == 10 || comptype == 100) {
		int i;
		TCHAR *fromaddr = NULL;
		TCHAR *group = NULL;
		if (comptype == 100) {
			group = filter_content + lstrlen(FILTER_IN_GROUP);
			while(*group == TEXT(' ')) group++;
			if (lstrlen(group) == 0) {
				return FALSE;
			}
		}
		fromaddr = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Content) + 1));
		GetMailAddress(Content, fromaddr, NULL, FALSE);
		ret = FALSE;
		for (i = 0; i < AddressBook->ItemCnt; i++) {
			ADDRESSITEM *item = *(AddressBook->tpAddrItem + i);
			if (item != NULL && item->AddressOnly != NULL
				&& (comptype == 10 || item->Group != NULL)
				&& lstrcmp(fromaddr, item->AddressOnly) == 0) {
				if (comptype == 10) {
					ret = TRUE;
					break;
				} else {
					len = lstrlen(item->Group);
					if (lstrcmp(group, item->Group) == 0) {
						ret = TRUE;
						break;
					} else if (len > (int)lstrlen(group)) {
						TCHAR *p, *q, *tmp;
						tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
						p = item->Group;
						q = tmp;
						while (q && *p != TEXT('\0')) {
							p = str_cpy_f_t(tmp, p, TEXT(','));
							if (lstrcmp(tmp, group) == 0) {
								ret = TRUE;
								break;
							}
							while (*p == TEXT(' ')) p++;
						}
						mem_free(&tmp);
					}
				}
			}
		}
		mem_free(&fromaddr);
	} else if (comptype != 0) {
		int days = _ttoi(filter_content + len);
		ret = DateCompare(Content, days, (comptype==1) ? TRUE : FALSE);
	} else {
		ret = str_match_t(filter_content, Content);
	}
	mem_free(&Content);
	return ret;
}

/*
 * item_check_filter
 */
static int item_check_filter(FILTER *tpFilter, char *buf, int *do_what_i, int flag_in)
{
	int RetFlag = flag_in;
	BOOL match1 = FALSE, match2 = FALSE, BoolOp, DoFilter = FALSE;
	if (tpFilter == NULL || tpFilter->Enable == 0) {
		return RetFlag;
	}
	BoolOp = tpFilter->Boolean;
	if (tpFilter->Header1 != NULL && *tpFilter->Header1 != TEXT('\0')) {
		match1 = item_filter_check_content(buf, tpFilter->Header1, tpFilter->Content1);
	}
	if (match1 == FALSE && BoolOp != FILTER_BOOL_OR) {
		return RetFlag;
	}
	if (tpFilter->Header2 != NULL && *tpFilter->Header2 != TEXT('\0')) {
		match2 = item_filter_check_content(buf, tpFilter->Header2, tpFilter->Content2);
	} else if (BoolOp == FILTER_BOOL_AND) {
		// header2 not required for AND
		match2 = TRUE;
	}
	switch (BoolOp) {
		case FILTER_BOOL_OR:
			DoFilter = match1 || match2;
			break;
		case FILTER_BOOL_UNLESS:
			DoFilter = match1 && (!match2);
			break;
		default:
			DoFilter = match1 && match2;
			break;
	}
	if (DoFilter) {
		int j, fret;
		j = tpFilter->Action;
		for (fret = 1; j > 0; j--) {
			fret *= 2;
		}
		switch (fret) {
		case FILTER_DOWNLOADMARK:
		case FILTER_DELETEMARK:
			if (RetFlag & (FILTER_DOWNLOADMARK | FILTER_DELETEMARK)) {
				fret = 0;
			}
			break;

		case FILTER_COPY:
		case FILTER_MOVE:
			if (RetFlag & FILTER_MOVE) {
				fret = 0;
			}
			break;

		case FILTER_PRIORITY:
		case FILTER_FORWARD:
			if (RetFlag & FILTER_UNRECV) {
				fret = 0;
			}
			break;

		//case FILTER_UNRECV:
		//case FILTER_RECV:
		//case FILTER_READICON
		default:
			break;
		}
		if (fret) {
			RetFlag |= fret;
			if (do_what_i != NULL) {
				*do_what_i = fret;
			}
		}

	}
	return RetFlag;
}

/*
 * item_filter_check - フィルタ文字列のチェック
 */
static int item_filter_check(MAILBOX *tpMailBox, char *buf, int *do_what)
{
	int RetFlag = 0;
	int i;
	int *dwi = NULL;
	BOOL done = FALSE;
	BOOL DoGlobal = op.GlobalFilterEnable && !(tpMailBox->FilterEnable & FILTER_NOGLOBAL)
					&& (op.tpFilter != NULL);

	if (tpMailBox->FilterEnable == 0 || buf == NULL || *buf == '\0' || 
		(tpMailBox->tpFilter == NULL && DoGlobal == FALSE)) {
		return FILTER_RECV;
	}
	if (DoGlobal) {
		for (i = 0; i < op.GlobalFilterCnt; i++) {
			if (do_what != NULL) dwi = do_what + i;
			RetFlag = item_check_filter(*(op.tpFilter+i), buf, dwi, RetFlag);
			if (RetFlag & (FILTER_RECV | FILTER_UNRECV)) {
				done = TRUE;
				break;
			}
		}
	}
	if (tpMailBox->tpFilter != NULL && done == FALSE) {
		for (i = 0; i < tpMailBox->FilterCnt; i++) {
			if (do_what != NULL) dwi = do_what + (op.GlobalFilterCnt + i);
			RetFlag = item_check_filter(*(tpMailBox->tpFilter+i), buf, dwi, RetFlag);
			if (RetFlag & (FILTER_RECV | FILTER_UNRECV)) {
				break;
			}
		}
	}
	return ((RetFlag == 0) ? FILTER_RECV : RetFlag);
}

/*
 * item_filter_domovecopy
 */
static int item_filter_domovecopy(MAILBOX *tpMailBox, MAILITEM *tpMailItem, BOOL refilter, int dw, int sbox)
{
	BOOL error = FALSE;
	MAILBOX *TargetBox = MailBox + sbox;
	if (TargetBox->Loaded == FALSE && op.BlindAppend == 0) {
		mailbox_load_now(NULL, sbox, FALSE, FALSE);
	}
	if (TargetBox->Loaded == FALSE) {
		TCHAR fname[BUF_SIZE];
		if (TargetBox->Filename == NULL) {
			wsprintf(fname, TEXT("MailBox%d.dat"), sbox - MAILBOX_USER);
		} else {
			lstrcpy(fname, TargetBox->Filename);
		}
		file_append_savebox(fname, TargetBox, tpMailItem, 2);
		TargetBox->NewMail++;
		if (tpMailItem->Mark == ICON_FLAG) {
			TargetBox->FlagCount++;
		}
		if (TargetBox->NewMail == 1 || (TargetBox->NewMail == 0 && TargetBox->FlagCount == 1)) {
			// first new mail or flag in this savebox
			SetMailboxMark(sbox, 0, FALSE);
		}
	} else {
		int j = item_find_thread(TargetBox, tpMailItem->MessageID, TargetBox->MailItemCnt);
		if (j == -1) {
			item_to_mailbox(TargetBox, tpMailItem, tpMailBox->Name, FALSE);
			TargetBox->NewMail++;
			if (tpMailItem->Mark == ICON_FLAG) {
				TargetBox->FlagCount++;
			}
			if (TargetBox->NewMail == 1 || (TargetBox->NewMail == 0 && TargetBox->FlagCount == 1)) {
				// first new mail or flag in this savebox
				SetMailboxMark(sbox, 0, FALSE);
			}
			if (sbox == SelBox) {
				ListView_ShowItem(GetDlgItem(MainWnd, IDC_LISTVIEW), TargetBox, TRUE);
			}
		} else if (refilter == TRUE) {
			MAILITEM *tpSboxItem = *(TargetBox->tpMailItem + j);
			char *newbody = alloc_copy(tpMailItem->Body);
			if (newbody != NULL) {
				mem_free(&tpSboxItem->Body);
				tpSboxItem->Body = newbody;
				tpSboxItem->Download = TRUE;
				TargetBox->NeedsSave |= MAILITEMS_CHANGED;
				if (sbox == SelBox) {
					// update icon
					HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
					int num = ListView_GetMemToItem(hListView, tpSboxItem);
					ListView_SetItemState(hListView, num, 0, LVIS_CUT);
					ListView_RedrawItems(hListView, num, num);
				}
			} else {
				error = TRUE;
			}
		}
		if (dw == FILTER_MOVE) {
			tpMailItem->Mark = ICON_DEL;
			tpMailItem->ReFwd &= ~(REFWD_FWDHOLD);
		}
	}
	return error;
}

/*
 * item_filter_execute - handle move/copy, forward, and mark
 */
static BOOL item_filter_execute(MAILBOX *tpMailBox, MAILITEM *tpMailItem, int fret, int *do_what, BOOL refilter)
{
	BOOL error = FALSE;
	// Move or Copy to SaveBox
	if ((fret & (FILTER_COPY | FILTER_MOVE | FILTER_PRIORITY | FILTER_FORWARD)) 
			&& tpMailItem->MailStatus != ICON_NON) {
		int i, dw, sbox;
		for (i = 0; i < op.GlobalFilterCnt; i++) {
			dw = do_what[i];
			if (dw == FILTER_COPY || dw == FILTER_MOVE) {
				sbox = mailbox_name_to_index((*(op.tpFilter + i))->SaveboxName, MAILBOX_TYPE_SAVE);
				if (sbox != -1) {
					error |= item_filter_domovecopy(tpMailBox, tpMailItem, refilter, dw, sbox);
				}
			} else if (dw == FILTER_FORWARD) {
				if (Edit_InitInstance(hInst, NULL, (tpMailBox - MailBox), tpMailItem,
						EDIT_FILTERFORWARD, (*(op.tpFilter + i))->FwdAddress, TRUE) != EDIT_SEND) {
						error = TRUE;
				}
			} else if (dw == FILTER_PRIORITY) {
				if ((*(op.tpFilter + i))->Priority == 0) {
					tpMailItem->Mark = ICON_FLAG;
				} else {
					tpMailItem->Priority = (*(op.tpFilter + i))->Priority;
				}
			} else if (dw == FILTER_READICON && tpMailItem->MailStatus != ICON_NON) {
				tpMailItem->MailStatus = ICON_READ;
				if (tpMailItem->Mark != ICON_FLAG) {
					tpMailItem->Mark = ICON_READ;
				}

			}
		}
		for (i = 0; i < tpMailBox->FilterCnt; i++) {
			dw = do_what[op.GlobalFilterCnt + i];
			if (dw == FILTER_COPY || dw == FILTER_MOVE) {
				sbox = mailbox_name_to_index((*(tpMailBox->tpFilter + i))->SaveboxName, MAILBOX_TYPE_SAVE);
				if (sbox != -1) {
					error |= item_filter_domovecopy(tpMailBox, tpMailItem, refilter, dw, sbox);
				}
			} else if (dw == FILTER_FORWARD) {
				if (Edit_InitInstance(hInst, NULL, (tpMailBox - MailBox), tpMailItem,
						EDIT_FILTERFORWARD, (*(tpMailBox->tpFilter + i))->FwdAddress, TRUE) != EDIT_SEND) {
						error = TRUE;
				}
			} else if (dw == FILTER_PRIORITY) {
				if ((*(tpMailBox->tpFilter + i))->Priority == 0) {
					tpMailItem->Mark = ICON_FLAG;
				} else {
					tpMailItem->Priority = (*(tpMailBox->tpFilter + i))->Priority;
				}
			} else if (dw == FILTER_READICON && tpMailItem->MailStatus != ICON_NON) {
				tpMailItem->MailStatus = ICON_READ;
				if (tpMailItem->Mark != ICON_FLAG) {
					tpMailItem->Mark = ICON_READ;
				}
			}
		}
	}

	//Filter operation setting
	//Opening being completed setting
	if (fret & FILTER_READICON && tpMailItem->MailStatus != ICON_NON) {
		tpMailItem->MailStatus = ICON_READ;
		if (tpMailItem->Mark != ICON_FLAG) {
			tpMailItem->Mark = ICON_READ;
		}
	}

	//Macro description
	if (fret & FILTER_DOWNLOADMARK) {
		if (tpMailItem->Download == FALSE) {
			tpMailItem->Mark = ICON_DOWN;
		}
	} else if (fret & FILTER_DELETEMARK) {
		tpMailItem->Mark = ICON_DEL;
		if (op.DeletedIsNotNew) {
			tpMailItem->New = FALSE;
		}
	}

	return !(error);
}

/*
 * item_find_thread - メッセージIDを検索する
 */
int item_find_thread(MAILBOX *tpMailBox, TCHAR *p, int Index)
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
		if (lstrcmp(tpMailItem->MessageID, p) == 0) {
			return j;
		}
	}
	return -1;
}

/*
 * item_find_thread_anywhere - find message with given MessageID
 */
MAILITEM *item_find_thread_anywhere(TCHAR *m_id)
{
	MAILITEM *tmp;
	BOOL unloaded = FALSE;
	int mbox, msg;
	if (m_id == NULL) {
		return NULL;
	}

	// look in loaded mailboxes first
	for (mbox = MAILBOX_USER; mbox < MailBoxCnt; mbox++) {
		if ((MailBox+mbox)->Loaded == FALSE) {
			unloaded = TRUE;
			continue;
		}
		for (msg = 0; msg < (MailBox+mbox)->MailItemCnt; msg++) {
			tmp = *((MailBox+mbox)->tpMailItem + msg);
			if (tmp != NULL && tmp->MessageID != NULL &&
				lstrcmp(tmp->MessageID, m_id) == 0) {
					return tmp;
			}
		}
	}

	if (unloaded) {
		char *char_id = alloc_tchar_to_char(m_id);

		for (mbox = MAILBOX_USER; mbox < MailBoxCnt; mbox++) {
			if ((MailBox+mbox)->Loaded == FALSE) {
				TCHAR fname[BUF_SIZE];
				if ((MailBox+mbox)->Filename == NULL) {
					wsprintf(fname, TEXT("MailBox%d.dat"), mbox - MAILBOX_USER);
				} else {
					lstrcpy(fname, (MailBox+mbox)->Filename);
				}
				tmp = file_scan_mailbox(fname, char_id);
				if (tmp != NULL) {
					if (SmtpFwdMessage != NULL) {
						item_free(&SmtpFwdMessage, 1);
					}
					SmtpFwdMessage = tmp;
					mem_free(&char_id);
					return tmp;
				}
			}
		}
		mem_free(&char_id);
	}
	return NULL;
}

/*
 * item_create_thread - スレッドを構築する
 */
void item_create_thread(MAILBOX *tpMailBox)
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
		parent = item_find_thread(tpMailBox, tpMailItem->InReplyTo, i);
		if (parent == -1) {
			parent = item_find_thread(tpMailBox, tpMailItem->References, i);
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
