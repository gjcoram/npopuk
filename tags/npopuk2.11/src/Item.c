/*
 * nPOP
 *
 * Item.c
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
#include "code.h"
#include "mime.h"
#include "multipart.h"

#include "global.h"
#include "md5.h"

/* Define */
#define MBOX_DELIMITER "From "
#define NPOPUK_MBOX_DELIMITER "From NPOPUK\r\n"
#define STATUS_REVISION_NUMBER 200000

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
static void item_set_body(MAILITEM *tpMailItem, char *buf, BOOL download);
static int item_save_header_size(TCHAR *header, TCHAR *buf);
static char *item_save_header(TCHAR *header, TCHAR *buf, char *ret);
static BOOL item_filter_check_content(char *buf, TCHAR *filter_header, TCHAR *filter_content);
static int item_check_filter(FILTER *tpFilter, char *buf, int *do_what_i, int flag_in);
static int item_filter_check(MAILBOX *tpMailBox, char *buf, int *do_what);
static int item_filter_domovecopy(MAILBOX *tpMailBox, MAILITEM *tpMailItem, BOOL refilter, int dw, int sbox);
static BOOL item_filter_execute(MAILBOX *tpMailBox, MAILITEM *tpMailItem, int fret, int *do_what, BOOL refilter);

/*
 * item_is_mailbox - ���[���{�b�N�X���̃��[�����X�g�Ɏw��̃��[�������݂��邩���ׂ�
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
 * item_set_count - �A�C�e�������̃��������m��
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
 * item_add - �A�C�e���̒ǉ�
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
	*(tpMailList + tpMailBox->MailItemCnt) = tpNewMailItem;
	tpMailBox->MailItemCnt++;
	if (*tpMailBox->tpMailItem != NULL) {
		(*tpMailBox->tpMailItem)->NextNo = 0;
	}
	return TRUE;
}

/*
 * item_copy - �A�C�e���̃R�s�[
 */
void item_copy(MAILITEM *tpFromMailItem, MAILITEM *tpToMailItem, BOOL Override)
{
	// copy is sufficient for all non-pointers
	CopyMemory(tpToMailItem, tpFromMailItem, sizeof(MAILITEM));

	if (Override) {
		// override a few values
		tpToMailItem->Mark = tpToMailItem->MailStatus;
		//tpToMailItem->New = FALSE;
		tpToMailItem->No = 0;
		tpToMailItem->UIDL = NULL;
	}

	// need to allocate new copies of the strings
	tpToMailItem->From = alloc_copy_t(tpFromMailItem->From);
	tpToMailItem->To = alloc_copy_t(tpFromMailItem->To);
	tpToMailItem->Cc = alloc_copy_t(tpFromMailItem->Cc);
	tpToMailItem->Bcc = alloc_copy_t(tpFromMailItem->Bcc);
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
	tpToMailItem->Body = alloc_copy(tpFromMailItem->Body);
	tpToMailItem->MailBox = alloc_copy_t(tpFromMailItem->MailBox);
	tpToMailItem->Attach = alloc_copy_t(tpFromMailItem->Attach);
	tpToMailItem->FwdAttach = alloc_copy_t(tpFromMailItem->FwdAttach);

	tpToMailItem->HeadCharset = alloc_copy_t(tpFromMailItem->HeadCharset);
	tpToMailItem->BodyCharset = alloc_copy_t(tpFromMailItem->BodyCharset);
}

/*
 * item_to_mailbox - �A�C�e�������[���{�b�N�X�ɒǉ�
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
 * item_resize_mailbox - �A�C�e�����̐���
 */
BOOL item_resize_mailbox(MAILBOX *tpMailBox)
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
 * item_free - ���[���A�C�e���̉��
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
		mem_free(&(*(tpMailItem + i))->To);
		mem_free(&(*(tpMailItem + i))->Cc);
		mem_free(&(*(tpMailItem + i))->Bcc);
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
 * item_get_content - �R���e���c�̎擾
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
	*ret = (char *)mem_alloc(len + 1);
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

	// �ʒu�̎擾
	p = GetHeaderStringPoint(buf, header);
	if (p == NULL) {
		*ret = NULL;
		return;
	}
	// �T�C�Y�̎擾
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
 * item_get_content_int - �R���e���c�̎擾
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
 * item_get_multi_content - ��������ꍇ�͈�ɂ܂Ƃ߂ăR���e���c�̎擾
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
			r = tmp = (char *)mem_alloc(ret_len + len + 2);
			if (tmp == NULL) {
				return ret_len;
			}
			r = str_cpy(r, *ret);
			r = str_cpy(r, ",");
			mem_free(&*ret);
			*ret = tmp;
			ret_len += (len + 1);
		} else {
			r = *ret = (char *)mem_alloc(len + 1);
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
 * item_get_mime_content - �w�b�_�̃R���e���c���擾����MIME�f�R�[�h���s��
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
 * item_get_message_id - ���b�Z�[�WID�̎擾
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
	Content = (char *)mem_alloc(16 * 2 + 1);
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
 * item_get_number_to_index - ���[���ԍ�����A�C�e���̃C���f�b�N�X���擾
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
 * item_get_next_send_mark - ���M�}�[�N�̕t�����A�C�e���̃C���f�b�N�X���擾
 */
int item_get_next_send_mark(MAILBOX *tpMailBox, BOOL CheckErrors)
{
	MAILITEM *tpMailItem;
	int i;

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem != NULL && (tpMailItem->Mark == ICON_SEND
				|| (CheckErrors == TRUE && tpMailItem->Mark == ICON_ERROR))) {
			return i;
		}
	}
	return -1;
}

/*
 * item_get_next_send_mark_mailbox - �w��̃��[���{�b�N�X�̑��M�}�[�N�̕t�����A�C�e���̃C���f�b�N�X���擾
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
		if (tpMailItem == NULL || tpMailItem->Mark != ICON_SEND) {
			continue;
		}
		BoxIndex = mailbox_name_to_index(tpMailItem->MailBox);
		if (MailBoxIndex == BoxIndex) {
			return i;
		} else if (MailBoxIndex == MAILBOX_SEND) {
			return BoxIndex;
		}
	}
	return -1;
}

/*
 * item_set_body - �A�C�e���ɖ{����ݒ�
 */
static void item_set_body(MAILITEM *tpMailItem, char *buf, BOOL download)
{
	char *p, *r;
	int len;
	int header_size;

	p = GetBodyPointa(buf);
	if (p != NULL && *p != '\0') {
		BOOL free_r = FALSE;
		// �f�R�[�h
		r = MIME_body_decode_transfer(tpMailItem, p);
		if (r == NULL) {
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return;
		} else if (r != p) {
			free_r = TRUE;
		}
		len = tstrlen(r);

		if (op.ShowHeader == 1 || KeyShowHeader == TRUE) {
			header_size = remove_duplicate_headers(buf);
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
			// if it's incoming mail, try to take control of buf (=mail_buf)
			tpMailItem->Body = claim_mail_buf(buf);
			if (tpMailItem->Body != NULL && header_size > 2) {
				tpMailItem->HasHeader = 2;
				// shift body backwards (after removing duplicate headers)
				tstrcpy(tpMailItem->Body + header_size, r);
				if (free_r == TRUE) {
					mem_free(&r);
				}
				return;
			}
		}
		if (tpMailItem->Body == NULL) {
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
		} else {
			if (header_size > 2 && (op.ShowHeader == 1 || KeyShowHeader == TRUE)) {
				// �w�b�_
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

	} else if (op.ShowHeader == 1 || KeyShowHeader == TRUE) {
		// �{�������݂��Ȃ��ꍇ�̓w�b�_�̂ݐݒ�
		mem_free(&tpMailItem->Body);
		tpMailItem->Body = alloc_copy(buf);

	} else if (download == TRUE) {
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
 * item_mail_to_item - �A�C�e���Ƀw�b�_�Ɩ{����ݒ�
 */
BOOL item_mail_to_item(MAILITEM *tpMailItem, char *buf, int Size, int download, MAILBOX *tpMailBox)
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

	if (download == TRUE) {
		// �����̏������
		mem_free(&tpMailItem->Subject);
		mem_free(&tpMailItem->From);
		mem_free(&tpMailItem->To);
		mem_free(&tpMailItem->Cc);
		mem_free(&tpMailItem->ReplyTo);
		mem_free(&tpMailItem->ContentType);
		mem_free(&tpMailItem->Encoding);
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
	} else if (download == 0) {
		// GJC - detect if fully downloaded
		if (Size > 0 && (int)tstrlen(buf) >= Size) { 
			// win32: tstrlen == Size+2
			// ppc:   tstrlen == Size
			tpMailItem->Download = TRUE;
		} else {
			tpMailItem->Download = FALSE;
		}
	} else {
		tpMailItem->Download = TRUE;
	}
	// Subject
	item_get_mime_content(buf, HEAD_SUBJECT, &tpMailItem->Subject, FALSE);
	// From
	item_get_mime_content(buf, HEAD_FROM, &tpMailItem->From, FALSE);
	// To
	item_get_mime_content(buf, HEAD_TO, &tpMailItem->To, TRUE);
	// Cc
	item_get_mime_content(buf, HEAD_CC, &tpMailItem->Cc, TRUE);
	// Reply-To
	item_get_mime_content(buf, HEAD_REPLYTO, &tpMailItem->ReplyTo, FALSE);
	// Content-Type
	item_get_mime_content(buf, HEAD_CONTENTTYPE, &tpMailItem->ContentType, FALSE);
	len = lstrlen(TEXT("multipart/alternative"));
	if (tpMailItem->ContentType != NULL &&
		str_cmp_ni_t(tpMailItem->ContentType, TEXT("multipart/alternative"), len) == 0) {
		BOOL verify = FALSE;
		TCHAR *p, *q;
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
			tpMailItem->Multipart = multipart_verify(ContentType, buf);
			mem_free(&ContentType);
#else
			tpMailItem->Multipart = multipart_verify(tpMailItem->ContentType, buf);
#endif
			if (tpMailItem->Multipart != MULTIPART_HTML) {
				//tpMailItem->OrigContentType = alloc_copy_t(tpMailItem->ContentType);
				p = q = tpMailItem->ContentType + lstrlen(TEXT("multipart/"));
				q += lstrlen(TEXT("alternative"));
				lstrcpy(p, TEXT("mixed"));
				p += lstrlen(TEXT("mixed"));
				while (*q != TEXT('\0')) {
					(*p++) = (*q++);
				}
			}
		}
	} else if (tpMailItem->ContentType != NULL &&
		str_cmp_ni_t(tpMailItem->ContentType, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0) {
		tpMailItem->Multipart = MULTIPART_CONTENT;
	} else {
		// Content-Transfer-Encoding
#ifdef UNICODE
		item_get_content(buf, HEAD_ENCODING, &Content);
		if (Content != NULL) {
			tpMailItem->Encoding = alloc_char_to_tchar(Content);
			mem_free(&Content);
		}
#else
		item_get_content(buf, HEAD_ENCODING, &tpMailItem->Encoding);
#endif
	}

	// Date
	item_get_content(buf, HEAD_DATE, &Content);
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
	Content = item_get_message_id(buf);
	if (Content != NULL) {
		tpMailItem->MessageID = alloc_char_to_tchar(Content);
		mem_free(&Content);
	}
#else
	tpMailItem->MessageID = item_get_message_id(buf);
#endif

	// In-Reply-To
#ifdef UNICODE
	item_get_content(buf, HEAD_INREPLYTO, &Content);
	TrimMessageId(Content);
	if (Content != NULL) {
		tpMailItem->InReplyTo = alloc_char_to_tchar(Content);
		mem_free(&Content);
	}
#else
	item_get_content(buf, HEAD_INREPLYTO, &tpMailItem->InReplyTo);
	TrimMessageId(tpMailItem->InReplyTo);
#endif

	// References
	item_get_content(buf, HEAD_REFERENCES, &Content);
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
	item_get_content(buf, HEAD_IMPORTANCE, &priority);
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
		tpMailItem->Priority = item_get_content_int(buf, HEAD_X_PRIORITY, -1);
		if (tpMailItem->Priority == 2) {
			tpMailItem->Priority = 1;
		} else if (tpMailItem->Priority == 4) {
			tpMailItem->Priority = 5;
		}
	}
	if (tpMailItem->Priority == -1) {
		tpMailItem->Priority = 3;
		item_get_content(buf, HEAD_PRIORITY, &priority);
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
	item_get_content(buf, HEAD_READ1, &Content);
	if(Content != NULL) {
		tpMailItem->ReadReceipt = 1;
		mem_free(&Content);
	}

	item_get_content(buf, HEAD_READ2, &Content);
	if(Content != NULL) {
		tpMailItem->ReadReceipt = 1;
		mem_free(&Content);
	}

	tpMailItem->DeliveryReceipt = 0;
	item_get_content(buf, HEAD_DELIVERY, &Content);
	if(Content != NULL) {
		tpMailItem->DeliveryReceipt = 1;
		mem_free(&Content);
	}
	////////////////// --- /////////////////////

	if (tpMailItem->InReplyTo == NULL || *tpMailItem->InReplyTo == TEXT('\0')) {
		mem_free(&tpMailItem->InReplyTo);
		tpMailItem->InReplyTo = alloc_copy_t(msgid1);
		t = msgid2;
	} else {
		t = (msgid1 != NULL && lstrcmp(tpMailItem->InReplyTo, msgid1) != 0) ? msgid1 : msgid2;
	}
	tpMailItem->References = alloc_copy_t(t);
	mem_free(&msgid1);
	mem_free(&msgid2);

	if (tpMailBox != NULL && (tpMailBox->FilterEnable & FILTER_REFILTER)) {
		//GJC refilter on full download
		do_what = (int *)mem_calloc(sizeof(int) * (op.GlobalFilterCnt + tpMailBox->FilterCnt));

		// �t�B���^���`�F�b�N
		fret = item_filter_check(tpMailBox, buf, do_what);
	}

	// Body
	item_set_body(tpMailItem, buf, download);

	if (tpMailBox != NULL && (tpMailBox->FilterEnable & 0x02)) {
		if (fret > FILTER_RECV) {
			retval = item_filter_execute(tpMailBox, tpMailItem, fret, do_what, TRUE);
		}
		mem_free(&do_what);
	}

	return retval;
}

/*
 * item_header_to_item - ���[���w�b�_����A�C�e�����쐬����
 */
MAILITEM *item_header_to_item(MAILBOX *tpMailBox, char *buf, int Size)
{
	MAILITEM *tpMailItem;
	int fret;
	int *do_what;
	do_what = (int *)mem_calloc(sizeof(int) * (op.GlobalFilterCnt + tpMailBox->FilterCnt));

	// �t�B���^���`�F�b�N
	fret = item_filter_check(tpMailBox, buf, do_what);
	if (fret == FILTER_UNRECV) {
		mem_free(&do_what);
		return (MAILITEM *)-1;
	}

	// ���[�����̊m��
	tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (tpMailItem == NULL) {
		mem_free(&do_what);
		return NULL;
	}
	// �w�b�_�Ɩ{����ݒ�
	item_mail_to_item(tpMailItem, buf, Size, FALSE, NULL);
	tpMailItem->New = TRUE;

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
 * item_string_to_item - �����񂩂�A�C�e�����쐬����
 */
MAILITEM *item_string_to_item(MAILBOX *tpMailBox, char *buf, BOOL Import)
{
	MAILITEM *tpMailItem;
	int i;
#ifdef UNICODE
	char *din, *dout;
#endif
	///////////// MRP /////////////////////
	TCHAR *Temp;
	///////////// --- /////////////////////

	tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (tpMailItem == NULL) {
		return NULL;
	}
	item_get_content_t(buf, HEAD_SUBJECT, &tpMailItem->Subject);
	item_get_content_t(buf, HEAD_FROM, &tpMailItem->From);
	item_get_content_t(buf, HEAD_TO, &tpMailItem->To);
	item_get_content_t(buf, HEAD_CC, &tpMailItem->Cc);
	item_get_content_t(buf, HEAD_BCC, &tpMailItem->Bcc);
	item_get_content_t(buf, HEAD_DATE, &tpMailItem->Date);
	if (tpMailItem->Date != NULL && *tpMailItem->Date != TEXT('\0')) {
#ifdef UNICODE
		din = alloc_tchar_to_char(tpMailItem->Date);
		dout = (char *)mem_alloc(BUF_SIZE);
		if (dout != NULL) {
			if(-1 == DateConv(din, dout, FALSE)) {
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
	item_get_content_t(buf, HEAD_SIZE, &tpMailItem->Size);
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
	item_get_content_t(buf, HEAD_X_UIDL, &tpMailItem->UIDL);
	
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

	item_get_content_t(buf, HEAD_X_MAILBOX, &tpMailItem->MailBox);
	item_get_content_t(buf, HEAD_X_ATTACH, &tpMailItem->Attach);
	if (tpMailItem->Attach == NULL) {
		item_get_content_t(buf, HEAD_X_ATTACH_OLD, &tpMailItem->Attach);
		if (tpMailItem->Attach != NULL) {
			TCHAR *p;
			for (p = tpMailItem->Attach; *p != TEXT('\0'); p++) {
#ifndef UNICODE
				// 2�o�C�g�R�[�h�̏ꍇ��2�o�C�g�i�߂�
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
	if (tpMailItem->FwdAttach != NULL
		&& (tpMailItem->References == NULL || *tpMailItem->FwdAttach == TEXT('\0'))) {
		mem_free(&tpMailItem->FwdAttach);
		tpMailItem->FwdAttach = NULL;
		if (tpMailBox) {
			tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
		}
	}
	item_get_content_t(buf, HEAD_X_HEADCHARSET, &tpMailItem->HeadCharset);
	tpMailItem->HeadEncoding = item_get_content_int(buf, HEAD_X_HEADENCODE, 0);
	item_get_content_t(buf, HEAD_X_BODYCHARSET, &tpMailItem->BodyCharset);
	tpMailItem->BodyEncoding = item_get_content_int(buf, HEAD_X_BODYENCODE, 0);

	// No
	tpMailItem->No = item_get_content_int(buf, HEAD_X_NO, -1);
	if (tpMailItem->No == -1) {
		tpMailItem->No = item_get_content_int(buf, HEAD_X_NO_OLD, 0);
	}
	tpMailItem->New = FALSE;
	// GJC: MailStatus, Mark, Download, and HasHeader combined into X-Status: header
	// order of codes must match item_to_string!
	item_get_content_t(buf, HEAD_X_STATUS, &Temp);
	if (Temp != NULL) {
		i = _ttoi(Temp);
	} else {
		i = -1;
	}
	// GJC:  100000 was the first STATUS_REVISION_NUMBER in the new system
	if (i >= 100000 && lstrlen(Temp) >= 6) {
		int rev, refwd, head, dwn, mrk, stat;

		// STATUS_REVISION_NUMBER
		rev = i / 100000;
		if (tpMailBox && rev != STATUS_REVISION_NUMBER/100000) {
			tpMailBox->NeedsSave |= MARKS_CHANGED;
		}
		i = i % 100000;

		// Replied/Forwarded
		refwd = i / 10000;
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
		i = i % 10000;

		// HasHeader
		head = i / 1000;
		tpMailItem->HasHeader = head;
		i = i % 1000;

		// Downloaded
		dwn = i / 100;
		tpMailItem->Download = (dwn == 0) ? FALSE : TRUE;
		i = i % 100;

		// Mark
		mrk = i / 10;
		tpMailItem->Mark = (mrk <= ICON_FLAG) ? mrk : ICON_NON;
		// Status
		stat = i % 10;
		tpMailItem->MailStatus = (stat <= ICON_FLAG) ? stat : ICON_NON;
		
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
	if (tpMailItem->Attach != NULL || tpMailItem->FwdAttach != NULL) {
		tpMailItem->Multipart = MULTIPART_ATTACH;
		tpMailItem->AttachSize = _ttoi(tpMailItem->Size); // - strlen(body) done later
	} else if (tpMailBox == MailBox + MAILBOX_SEND) {
		// presently, can't forward as attachment
		tpMailItem->Multipart = MULTIPART_NONE;
		if (tpMailItem->ContentType != NULL) {
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
	return tpMailItem;
}

/*
 * item_save_header_size - �ۑ�����w�b�_�̃T�C�Y
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
 * item_save_header - �w�b�_��ۑ����镶����̍쐬
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
	if (buf == NULL) {
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
 * item_to_string_size - ���[���̕ۑ�������̃T�C�Y�擾
 */
int item_to_string_size(MAILITEM *tpMailItem, int WriteMbox, BOOL BodyFlag, BOOL SepFlag)
{
	TCHAR X_No[10], X_Mstatus[10], X_HeadEncoding[10], X_BodyEncoding[10];
	int do_body, len = 0;

	do_body = (BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != '\0') ? 1 : 0;

	wsprintf(X_No, TEXT("%d"), tpMailItem->No);
	wsprintf(X_Mstatus, TEXT("%d"), STATUS_REVISION_NUMBER);
	wsprintf(X_HeadEncoding, TEXT("%d"), tpMailItem->HeadEncoding);
	wsprintf(X_BodyEncoding, TEXT("%d"), tpMailItem->BodyEncoding);

	if (SepFlag && WriteMbox == 1) {
		len = tstrlen(NPOPUK_MBOX_DELIMITER);
	}
	len += item_save_header_size(TEXT(HEAD_FROM), tpMailItem->From);
	len += item_save_header_size(TEXT(HEAD_TO), tpMailItem->To);
	len += item_save_header_size(TEXT(HEAD_CC), tpMailItem->Cc);
	len += item_save_header_size(TEXT(HEAD_BCC), tpMailItem->Bcc);
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
   
	if (tpMailItem->ReadReceipt == 1)
	{
		len += item_save_header_size(TEXT(HEAD_READ1), tpMailItem->From);
		len += item_save_header_size(TEXT(HEAD_READ2), tpMailItem->From);
	}

	if (tpMailItem->DeliveryReceipt == 1)
	{
		len += item_save_header_size(TEXT(HEAD_DELIVERY), tpMailItem->From);
	}

	//////////////////--- /////////////////////

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
		if (WriteMbox == 1) {
			char *r = tpMailItem->Body;
			int l = tstrlen(MBOX_DELIMITER);
			if (str_cmp_n(r, MBOX_DELIMITER, l) == 0) {
				len++;
			}
			for ( /**/ ; *r != '\0'; r++, len++) {
				if (*r == '\r' && *(r+1) == '\n' && str_cmp_n(r+2, MBOX_DELIMITER, l) == 0) {
					len++;
				}
			}
		} else {
			len += tstrlen(tpMailItem->Body);
		}
	}
	if (WriteMbox != 0) {
		len += 2; // \r\n
	} else {
		len += 5; // \r\n.\r\n
	}
	return len;
}

/*
 * item_to_string - ���[���̕ۑ�������̎擾
 */
char *item_to_string(char *buf, MAILITEM *tpMailItem, int WriteMbox, BOOL BodyFlag, BOOL SepFlag)
{
	char *p = buf;
	TCHAR X_No[10], X_Mstatus[10], X_HeadEncoding[10], X_BodyEncoding[10];
	int do_body, prio, composite_status;

	do_body = (BodyFlag == TRUE && tpMailItem->Body != NULL && *tpMailItem->Body != '\0') ? 1 : 0;

	// GJC: order of codes must match item_string_to_item!
	composite_status = STATUS_REVISION_NUMBER
		+ 10000 * (2 * (tpMailItem->ReFwd & ICON_REFWD_MASK) + ((tpMailItem->ReFwd & REFWD_FWDHOLD) != 0))
		+  1000 * tpMailItem->HasHeader * do_body
		+   100 * ((tpMailItem->Download == TRUE) ? do_body : 0)
		+    10 * tpMailItem->Mark
		+         tpMailItem->MailStatus;

	wsprintf(X_No, TEXT("%d"), tpMailItem->No);
	wsprintf(X_Mstatus, TEXT("%d"), composite_status);
	wsprintf(X_HeadEncoding, TEXT("%d"), tpMailItem->HeadEncoding);
	wsprintf(X_BodyEncoding, TEXT("%d"), tpMailItem->BodyEncoding);

	if (SepFlag && WriteMbox == 1) {
		p = str_cpy(p, NPOPUK_MBOX_DELIMITER);
	}
	p = item_save_header(TEXT(HEAD_FROM), tpMailItem->From, p);
	p = item_save_header(TEXT(HEAD_TO), tpMailItem->To, p);
	p = item_save_header(TEXT(HEAD_CC), tpMailItem->Cc, p);
	p = item_save_header(TEXT(HEAD_BCC), tpMailItem->Bcc, p);
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
		if (WriteMbox == 1) {
			char *r = tpMailItem->Body;
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
			p = str_cpy(p, tpMailItem->Body);
		}
	}
	if (WriteMbox != 0) {
		p = str_cpy(p, "\r\n");
	} else {
		p = str_cpy(p, "\r\n.\r\n");
	}
	return p;
}

/*
 * item_filter_check_content - ������̃`�F�b�N
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
		// �{���̈ʒu�擾
		p = GetBodyPointa(buf);
		if (p == NULL) {
			ret = str_match(cbuf, "");
		} else {
			// ��r
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
	// �R���e���c�̎擾
	len = item_get_mime_content(buf, cbuf, &Content, TRUE);
	mem_free(&cbuf);
#else
	// �R���e���c�̎擾
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
	if (match1 == FALSE &&  BoolOp != FILTER_BOOL_OR) {
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
		if (do_what_i != NULL) {
			*do_what_i = fret;
		}
		switch (fret) {
		case FILTER_UNRECV:
		case FILTER_RECV:
			RetFlag |= fret;
			break;

		case FILTER_DOWNLOADMARK:
		case FILTER_DELETEMARK:
			// �}�[�N�t���O
			if (!(RetFlag & (FILTER_DOWNLOADMARK | FILTER_DELETEMARK))) {
				RetFlag |= fret;
			}
			break;

		case FILTER_COPY:
		case FILTER_MOVE:
			if (!(RetFlag & FILTER_MOVE)) {
				RetFlag |= fret;
			}
			break;

		case FILTER_PRIORITY:
		case FILTER_FORWARD:
			if (!(RetFlag * FILTER_UNRECV)) {
				RetFlag |= fret;
			}
			break;

		default:  // FILTER_READICON
			RetFlag |= fret;
			break;
		}
	}
	return RetFlag;
}

/*
 * item_filter_check - �t�B���^������̃`�F�b�N
 */
static int item_filter_check(MAILBOX *tpMailBox, char *buf, int *do_what)
{
	int RetFlag = 0;
	int i;
	int *dwi = NULL;
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
				break;
			}
		}
	}
	if (tpMailBox->tpFilter != NULL) {
		for (i = 0; i < tpMailBox->FilterCnt; i++) {
			if (do_what != NULL) dwi = do_what + op.GlobalFilterCnt + i;
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
	} else {
		int j = item_find_thread(TargetBox, tpMailItem->MessageID, TargetBox->MailItemCnt);
		if (j == -1) {
			item_to_mailbox(TargetBox, tpMailItem, tpMailBox->Name, FALSE);
			TargetBox->NewMail++;
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
				sbox = mailbox_name_to_index((*(op.tpFilter + i))->SaveboxName);
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
				sbox = mailbox_name_to_index((*(tpMailBox->tpFilter + i))->SaveboxName);
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
	}

	return !(error);
}

/*
 * item_find_thread - ���b�Z�[�WID����������
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
 * item_create_thread - �X���b�h���\�z����
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
		// �����[���̌���
		parent = item_find_thread(tpMailBox, tpMailItem->InReplyTo, i);
		if (parent == -1) {
			parent = item_find_thread(tpMailBox, tpMailItem->References, i);
		}
		// �����[���Ȃ�
		if (parent == -1) {
			(*(tpMailBox->tpMailItem + no))->NextNo = i;
			tpMailItem->PrevNo = no;
			no = i;
			continue;
		}
		// �C���f���g��ݒ肷��
		tpMailItem->Indent = (*(tpMailBox->tpMailItem + parent))->Indent + 1;
		n = (*(tpMailBox->tpMailItem + parent))->NextNo;
		while (n != 0) {
			if ((tpNextMailItem = (*(tpMailBox->tpMailItem + n))) == NULL) {
				n = 0;
				break;
			}
			// �C���f���g���烁�[���̒ǉ��ʒu���擾����
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
	// �\�[�g���l��ݒ肷��
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