/*
 * nPOP
 *
 * Smtp.c (RFC 821, RFC 2821)
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2013 by Geoffrey Coram. All rights reserved.
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
#define SEND_SIZE				4096			// 送信時の分割サイズ
#define SMTP_ERRORSTATUS		400				// SMTPエラーレスポンス
#define MIME_VERSION			"1.0"			// MIMEバージョン

#define AUTH_CRAM_MD5			0				// SMTP-AUTH の認証タイプ
#define AUTH_LOGIN				1
#define AUTH_PLAIN				2
#define AUTH_NON				99

#define CMD_HELO				"HELO"
#define CMD_EHLO				"EHLO"
#define CMD_STARTTLS			"STARTTLS"
#define CMD_AUTH				"AUTH"
#define CMD_AUTH_CRAM_MD5		"CRAM-MD5"
#define CMD_AUTH_LOGIN			"LOGIN"
#define CMD_AUTH_PLAIN			"PLAIN"
#define CMD_RSET				"RSET"
#define CMD_MAIL_FROM			"MAIL FROM"
#define CMD_RCPT_TO				"RCPT TO"
#define CMD_DATA				"DATA"
#define CMD_DATA_END			"\r\n.\r\n"
#define CMD_QUIT				"QUIT"

/* Global Variables */
static MAILBOX *send_mail_box;					// 送信中のメールボックス
static MAILITEM *send_mail_item;				// 送信メール
static BOOL auth_flag;							// SMTP-AUTH
static BOOL starttls_flag;						// STARTTLS
static char *send_body, *send_pt;				// 送信用本文
static int send_len;							// 送信長
static int send_end_cmd;						// メール送信完了時のコマンド
MAILITEM *SmtpFwdMessage = NULL;

// 外部参照
extern HWND MainWnd;
extern OPTION op;
extern int gAutoSend;
extern TCHAR *DataDir;
extern ADDRESSBOOK *AddressBook;
extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern int SelBox;

extern BOOL GetHostFlag;
extern int ssl_type;

extern int command_status;
extern TCHAR *g_Pass;

/* Local Function Prototypes */
static int check_response(char *buf);
static BOOL check_starttls(char *buf);
static BOOL auth_get_type(char *buf, int *type);
static TCHAR *auth_create_cram_md5(char *buf, TCHAR *user, TCHAR *pass, TCHAR *ErrStr);
static TCHAR *auth_create_plain(TCHAR *user, TCHAR *pass, TCHAR *ErrStr);
static TCHAR *auth_get_password(MAILBOX *tpMailBox);
static BOOL send_address(HWND hWnd, SOCKET soc, TCHAR *command, TCHAR *address, TCHAR *ErrStr);
static TCHAR *send_rcpt_to(HWND hWnd, SOCKET soc, TCHAR *address, TCHAR *ErrStr);
static BOOL send_header_t(SOCKET soc, TCHAR *header, TCHAR *content, TCHAR *ErrStr);
#ifdef UNICODE
static BOOL send_header(SOCKET soc, char *header, char *content, TCHAR *ErrStr);
#else
#define send_header send_header_t
#endif
static BOOL send_mime_header(SOCKET soc, MAILITEM *tpMailItem, char *header, TCHAR *content, BOOL address, TCHAR *ErrStr);
static char *augment_wire_form(char *body, char *ctype, MAILITEM *tpMailItem, BOOL new_date);
static BOOL send_wire_form(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr);
static BOOL send_mail_data(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr);
static BOOL send_mail_proc(HWND hWnd, SOCKET soc, char *buf, TCHAR *ErrStr, MAILITEM *tpMailItem, BOOL ShowFlag);
static void reorder_sendbox();

/*
 * HMAC_MD5 - MD5のダイジェストを生成する
 */
void HMAC_MD5(unsigned char *input, int len, unsigned char *key, int keylen, unsigned char *digest)
{
    MD5_CTX context;
    MD5_CTX tctx;
    unsigned char k_ipad[65];
    unsigned char k_opad[65];
    unsigned char tk[16];
    int i;

	// キーが64バイトより大きい場合はキーのダイジェストをキーにする
    if (keylen > 64) {
        MD5Init(&tctx);
        MD5Update(&tctx, key, keylen);
        MD5Final(tk, &tctx);

        key = tk;
        keylen = 16;
    }

	// MD5(key XOR opad, MD5(key XOR ipad, input)) の計算
    FillMemory(k_ipad, sizeof(k_ipad), 0x36);
    FillMemory(k_opad, sizeof(k_opad), 0x5c);
    for (i = 0; i < keylen; i++) {
        *(k_ipad + i) ^= *(key + i);
        *(k_opad + i) ^= *(key + i);
    }

    MD5Init(&context);
    MD5Update(&context, k_ipad, 64);
    MD5Update(&context, input, len);
    MD5Final(digest, &context);

    MD5Init(&context);
    MD5Update(&context, k_opad, 64);
    MD5Update(&context, digest, 16);
    MD5Final(digest, &context);
}

/*
 * check_response - SMTPレスポンスの解析
 */
static int check_response(char *buf)
{
	char stat[5];
	int ret;

	// 複数行の場合は -1 を返す
	if (tstrlen(buf) > 3 && *(buf + 3) == '-') {
		return -1;
	}

	// ステータスコードを取得
	str_cpy_n(stat, buf, 4);
	ret = a2i(stat);
	if (ret <= 0) {
		ret = SMTP_ERRORSTATUS;
	}
	return ret;
}

/*
 * check_starttls - STARTTLSが利用可能かチェック
 */
static BOOL check_starttls(char *buf)
{
	char *p;

	for (p = buf; *p != '\0' && *p != ' ' && *p != '-'; p++);
	for (; *p == ' ' || *p == '-'; p++);
	if (*p == '\0') {
		return FALSE;
	}
	// STARTTLS
	if (str_cmp_ni(p, CMD_STARTTLS, tstrlen(CMD_STARTTLS)) == 0) {
		return TRUE;
	}
	return FALSE;
}

/*
 * auth_get_type - 利用可能な SMTP AUTH の種類を取得 (RFC 2554)
 */
static BOOL auth_get_type(char *buf, int *type)
{
	char *p, *r;

	for (p = buf; *p != '\0' && *p != ' ' && *p != '-'; p++);
	for (; *p == ' ' || *p == '-'; p++);
	if (*p == '\0') {
		return FALSE;
	}
	// AUTH
	if (str_cmp_ni(p, CMD_AUTH, tstrlen(CMD_AUTH)) != 0) {
		return FALSE;
	}
	for (; *p != '\0' && *p != ' ' && *p != '='; p++);
	for (; *p == ' ' || *p == '='; p++);
	if (*p == '\0') {
		return FALSE;
	}

	// CRAM-MD5
	r = p;
	while (*r != '\0') {
		if (str_cmp_ni(r, CMD_AUTH_CRAM_MD5, tstrlen(CMD_AUTH_CRAM_MD5)) == 0) {
			*type = AUTH_CRAM_MD5;
			break;
		}
		for (; *r != '\0' && *r != ' '; r++);
		for (; *r == ' '; r++);
	}
	if (*type == AUTH_CRAM_MD5) {
		return TRUE;
	}

	// LOGIN
	r = p;
	while (*r != '\0') {
		if (str_cmp_ni(r, CMD_AUTH_LOGIN, tstrlen(CMD_AUTH_LOGIN)) == 0) {
			*type = AUTH_LOGIN;
			break;
		}
		for (; *r != '\0' && *r != ' '; r++);
		for (; *r == ' '; r++);
	}
	if (*type == AUTH_LOGIN) {
		return TRUE;
	}

	// PLAIN
	r = p;
	while (*r != '\0') {
		if (str_cmp_ni(r, CMD_AUTH_PLAIN, tstrlen(CMD_AUTH_PLAIN)) == 0) {
			*type = AUTH_PLAIN;
			break;
		}
		for (; *r != '\0' && *r != ' '; r++);
		for (; *r == ' '; r++);
	}
	if (*type == AUTH_PLAIN) {
		return TRUE;
	}
	return FALSE;
}

/*
 * auth_create_cram_md5 - AUTH CRAM-MD5 のパスワードを生成する (RFC 2554)
 */
static TCHAR *auth_create_cram_md5(char *buf, TCHAR *user, TCHAR *pass, TCHAR *ErrStr)
{
	TCHAR *ret;
	char *input, *key;
	char *p;
	unsigned char digest[16];
	int len;
	int i;

#ifdef UNICODE
	key = alloc_tchar_to_char(pass);
	if (key == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
#else
	key = pass;
#endif

	for (p = buf; *p != '\0' && *p != ' '; p++);
	for (; *p == ' '; p++);

	input = (char *)mem_alloc(tstrlen(p) + 1);
	if (input == NULL) {
#ifdef UNICODE
		mem_free(&key);
#endif
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	base64_decode(p, input, FALSE);

	// ダイジェスト値の取得
	HMAC_MD5(input, tstrlen(input), key, tstrlen(key), digest);
#ifdef UNICODE
	mem_free(&key);
#endif
	mem_free(&input);

	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT(" \r\n")) + lstrlen(user) + (16 * 2) + 1));
	if (ret == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	len = wsprintf(ret, TEXT("%s "), user);
	for (i = 0; i < 16; i++, len += 2) {
		wsprintf(ret + len, TEXT("%02x"), digest[i]);
	}
	return ret;
}

/*
 * auth_create_plain - AUTH PLAIN の文字列を生成する (RFC 2554)
 */
static TCHAR *auth_create_plain(TCHAR *user, TCHAR *pass, TCHAR *ErrStr)
{
#ifdef UNICODE
	TCHAR *ret;
#endif
	char *c_user, *c_pass;
	char *buf, *tmp;
	int len;

#ifdef UNICODE
	c_user = alloc_tchar_to_char(user);
	if (c_user == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	c_pass = alloc_tchar_to_char(pass);
	if (c_pass == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
#else
	c_user = user;
	c_pass = pass;
#endif

	len = tstrlen(c_user) + tstrlen(c_pass) + 2;

	tmp = (char *)mem_calloc(len + 1);
	if (tmp == NULL) {
#ifdef UNICODE
		mem_free(&c_user);
		mem_free(&c_pass);
#endif
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	tstrcpy(tmp + 1, c_user);
	tstrcpy(tmp + 1 + tstrlen(c_user) + 1, c_pass);

#ifdef UNICODE
	mem_free(&c_user);
	mem_free(&c_pass);
#endif

	buf = (char *)mem_calloc(len * 2 + 4);
	if (buf == NULL) {
		mem_free(&tmp);
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	base64_encode(tmp, buf, len, 0);
	mem_free(&tmp);
#ifdef UNICODE
	ret = alloc_char_to_tchar(buf);
	mem_free(&buf);
	return ret;
#else
	return buf;
#endif
}

/*
 * auth_get_password - パスワードの取得
 */
static TCHAR *auth_get_password(MAILBOX *tpMailBox)
{
	TCHAR *r;

	if (tpMailBox->AuthUserPass == 1) {
		r = tpMailBox->SmtpPass;
		if (r == NULL || *r == TEXT('\0')) {
			r = tpMailBox->SmtpTmpPass;
		}
	} else {
		r = tpMailBox->Pass;
		if (r == NULL || *r == TEXT('\0')) {
			r = tpMailBox->TmpPass;
		}
	}
	if (r == NULL || *r == TEXT('\0')) {
		r = g_Pass;
	}
	if (r == NULL || *r == TEXT('\0')) {
		return NULL;
	}
	return r;
}

/*
 * send_address - メールアドレスを送信する
 */
static BOOL send_address(HWND hWnd, SOCKET soc, TCHAR *command, TCHAR *address, TCHAR *ErrStr)
{
	TCHAR *wbuf;

	wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT(":<>\r\n")) + lstrlen(command) + lstrlen(address) + 1));
	if (wbuf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	// command:<address>\r\n
	str_join_t(wbuf, command, TEXT(":<"), address, TEXT(">"), (TCHAR *)-1);

	SetSocStatusTextT(hWnd, wbuf);
	lstrcat(wbuf, TEXT("\r\n"));
	if (send_buf_t(soc, wbuf) == -1) {
		mem_free(&wbuf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&wbuf);
	return TRUE;
}

/*
 * send_rcpt_to - RCPT TO を送信する
 */
static TCHAR *send_rcpt_to(HWND hWnd, SOCKET soc, TCHAR *address, TCHAR *ErrStr)
{
	TCHAR *p;
	int len = lstrlen(address) + 1;

	// メールアドレスの抽出
	p = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
	if (p == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return (TCHAR *)-1;
	}
	address = GetMailAddress(address, p, NULL, FALSE);

	// メールアドレスの送信
	if (send_address(hWnd, soc, TEXT(CMD_RCPT_TO), p, ErrStr) == FALSE) {
#ifndef _WCE_OLD
		if (op.SocLog > 1) {
			char buf[BUF_SIZE];
			char *pa;
#ifdef UNICODE
			pa = alloc_tchar_to_char(p);
#else
			pa = p;
#endif
			if (len > 200) *(pa+200) = '\0'; // prevent buffer overflow
			sprintf_s(buf, BUF_SIZE, "%s failed for %a\r\n", CMD_RCPT_TO, pa);
#ifdef UNICODE
			mem_free(&pa);
#endif
			log_save_a(buf);
		}
#endif
		return (TCHAR *)-1;
	}
	mem_free(&p);
	return ((*address != TEXT('\0')) ? address + 1 : address);
}

/*
 * send_header_t - ヘッダ項目の送信
 */
static BOOL send_header_t(SOCKET soc, TCHAR *header, TCHAR *content, TCHAR *ErrStr)
{
	TCHAR *buf;

	if (content == NULL || *content == TEXT('\0')) {
		return TRUE;
	}
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(header) + 1 + lstrlen(content) + 2 + 1));
	if (buf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	str_join_t(buf, header, TEXT(" "), content, TEXT("\r\n"), (TCHAR *)-1);
	if (send_buf_t(soc, buf) == -1) {
		mem_free(&buf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&buf);
	return TRUE;
}

/*
 * send_header - ヘッダ項目の送信
 */
#ifdef UNICODE
static BOOL send_header(SOCKET soc, char *header, char *content, TCHAR *ErrStr)
{
	char *buf;

	if (content == NULL || *content == '\0') {
		return TRUE;
	}
	buf = (char *)mem_alloc(sizeof(char) * (tstrlen(header) + 1 + tstrlen(content) + 2 + 1));
	if (buf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	str_join(buf, header, " ", content, "\r\n", (char *)-1);
	if (send_buf(soc, buf) == -1) {
		mem_free(&buf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&buf);
	return TRUE;
}
#endif

/*
 * send_mime_header - ヘッダ項目をMIMEエンコードして送信
 */
static BOOL send_mime_header(SOCKET soc, MAILITEM *tpMailItem, char *header, TCHAR *content, BOOL address, TCHAR *ErrStr)
{
	char *p;
	BOOL ret;
	int len = tstrlen(header);

	if (content == NULL || *content == TEXT('\0')) {
		return TRUE;
	}
	if (tpMailItem->HeadCharset != NULL) {
		p = MIME_encode(content, address, tpMailItem->HeadCharset, tpMailItem->HeadEncoding, len);
	} else {
		p = MIME_encode(content, address, op.HeadCharset, op.HeadEncoding, len);
	}
	if (p == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	ret = send_header(soc, header, p, ErrStr);
	mem_free(&p);
	return ret;
}

/*
 * augment_wire_form -- add date, message-id, etc.
 */
static char *augment_wire_form(char *body, char *ctype, MAILITEM *tpMailItem, BOOL new_date)
{
	TCHAR buf[BUF_SIZE];
	char *ret = NULL, *new_wire, *p, *r, *w, *date=NULL, *m_id=NULL;
	int lenb = 0, lenw = 0, add = 0;

	if (body != NULL) {
		lenb = tstrlen(body) + 3; // \r\n\0
	}
	lenw = tstrlen(tpMailItem->WireForm) + 1;

	w = GetBodyPointa(tpMailItem->WireForm);
	if (w == NULL) {
		// it's all headers
		w = tpMailItem->WireForm + tstrlen(tpMailItem->WireForm);
	} else {
		// back up to before blank line separating headers from body
		if ( (w - tpMailItem->WireForm > 4)
				&& *(w-4) == '\r' && *(w-3) == '\n'
				&& *(w-2) == '\r' && *(w-1) == '\n' ) {
			w -=2;
		} else if ( (w - tpMailItem->WireForm > 2) 
					&& ((*(w-1) == '\r' && *(w-2) == '\r')
					|| (*(w-1) == '\n' && *(w-2) == '\n')) ) {
			w--;
		}
	}

	if (new_date == TRUE) {
		mem_free(&tpMailItem->Date);
		tpMailItem->Date = NULL;
	}
	if (tpMailItem->Date == NULL) {
		GetTimeString(buf);
		tpMailItem->Date = alloc_copy_t(buf);
	}

	if (op.SendDate) {
		date = alloc_tchar_to_char(tpMailItem->Date);
		add += tstrlen(HEAD_DATE) + tstrlen(date) + 3; // " \r\n"
	}

	if (op.SendMessageId == 1) {
		// Message-Id
		mem_free(&tpMailItem->MessageID);
		tpMailItem->MessageID = CreateMessageId((long)tpMailItem, send_mail_box->MailAddress);
		if (tpMailItem->MessageID == NULL) {
			return FALSE;
		}
		m_id = alloc_tchar_to_char(tpMailItem->MessageID);
		add += tstrlen(HEAD_MESSAGEID) + tstrlen(m_id) + 3; // " \r\n"
	}

	// MIME-Version
	add += tstrlen(HEAD_MIMEVERSION) + tstrlen(MIME_VERSION) + 3; // " \r\n"

	// X-Mailer
	add += tstrlen(HEAD_X_MAILER) + tstrlen(APP_NAME_A) + 3; // " \r\n"

	if (body != NULL) {
		int lenc, lene;
		lenb += add;

		lenb += (w - tpMailItem->WireForm); // previous headers

		if (ctype != NULL) {
			lenb += tstrlen(HEAD_CONTENTTYPE) + tstrlen(ctype) + 3; // " \r\n"
		}

		ret = (char *)mem_alloc(sizeof(char) * lenb);
		if (ret == NULL) {
			return FALSE;
		}

		lenc = tstrlen(HEAD_CONTENTTYPE);
		lene = tstrlen(HEAD_ENCODING);
		for (r = ret, p = tpMailItem->WireForm; p < w; p++) {
			if (ctype != NULL) {
				// replace Content-Type
				BOOL skip = FALSE;
				if (str_cmp_n(p, HEAD_CONTENTTYPE, lenc) == 0) {
					r = str_join(r, HEAD_CONTENTTYPE, " ", ctype, "\r\n", (char*)-1);
					p += lenc;
					skip = TRUE;
				} else if (str_cmp_n(p, HEAD_ENCODING, lene) == 0) {
					p += lene;
					skip = TRUE;
				}
				if (skip) {
					// skip the rest of the line
					while (*p != '\0') {
						if (*p == '\r' && *(p+1) == '\n') {
							p++;
							break;
						} else {
							p++;
						}
					}
					continue;
				}
			}
			*(r++) = *p;
		}

		if (date != NULL) {
			r = str_join(r, HEAD_DATE, " ", date, "\r\n", (char*)-1);
		}
		if (m_id != NULL) {
			r = str_join(r, HEAD_MESSAGEID, " ", m_id, "\r\n", (char*)-1);
		}
		r = str_join(r, HEAD_MIMEVERSION, " ", MIME_VERSION, "\r\n",
					HEAD_X_MAILER, " ", APP_NAME_A, "\r\n\r\n", 
					body, (char*)-1);
	}

	lenw += add;
	new_wire = (char *)mem_alloc(sizeof(char) * lenw);
	if (new_wire == NULL) {
		mem_free(&date);
		mem_free(&m_id);
		mem_free(&ret);
		return FALSE;
	}

	for (r = new_wire, p = tpMailItem->WireForm; p < w; p++) {
		*(r++) = *p;
	}
	
	if (date != NULL) {
		r = str_join(r, HEAD_DATE, " ", date, "\r\n", (char*)-1);
		mem_free(&date);
	}
	if (m_id != NULL) {
		r = str_join(r, HEAD_MESSAGEID, " ", m_id, "\r\n", (char*)-1);
		mem_free(&m_id);
	}
	r = str_join(r, HEAD_MIMEVERSION, " ", MIME_VERSION, "\r\n",
				HEAD_X_MAILER, " ", APP_NAME_A, "\r\n", 
				w, (char*)-1);

	mem_free(&tpMailItem->WireForm);
	tpMailItem->WireForm = new_wire;

	return ret;
}

/*
 * send_wire_form - send WireForm
 */
static BOOL send_wire_form(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr)
{
	MAILITEM *tpFwdMailItem = NULL;
#ifdef UNICODE
	char *din, *dout;
#else
	char buf[BUF_SIZE];
#endif
	char *orig_body, *orig_ctype, *orig_enc, *mctypr = NULL, *mbody;
	char **enc_att = NULL;
	int create, num_att;

	if (tpMailItem->FwdAttach != NULL  && *(tpMailItem->FwdAttach) != TEXT('\0')) {
		// GJC - find original message to forward attachments
		int i, j;
		i = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_ACCOUNT);
		if (i == -1) {
			i = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_SAVE);
		}
		if (i != -1) {
			if ((j = item_find_thread(MailBox + i, tpMailItem->References, (MailBox+i)->MailItemCnt)) != -1) {
				tpFwdMailItem = (*((MailBox + i)->tpMailItem + j));
			}
		}
		if (tpFwdMailItem == NULL) {
			tpFwdMailItem = item_find_thread_anywhere(tpMailItem->References);
		}
		if (tpFwdMailItem == NULL) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOATTACH);
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return FALSE;
		}
	}

	orig_body = GetBodyPointa(tpMailItem->WireForm);
	orig_ctype = alloc_tchar_to_char(tpMailItem->ContentType);
	orig_enc = alloc_tchar_to_char(tpMailItem->Encoding);
	if (orig_ctype == NULL || orig_enc == NULL) {
		mem_free(&orig_ctype);
		mem_free(&orig_enc);
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
		return FALSE;
	}

	create = multipart_create(tpMailItem->Attach, tpMailItem->FwdAttach, tpFwdMailItem, 
							orig_ctype, orig_enc, &mctypr, orig_body, &mbody,
							&num_att, &enc_att);
	mem_free(&orig_ctype);
	mem_free(&orig_enc);

	switch (create) {
		case MP_ERROR_FILE:
			lstrcpy(ErrStr, STR_ERR_SOCK_NOATTACH);
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return FALSE;

		case MP_ERROR_ALLOC:
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return FALSE;

		case MP_NO_ATTACH:
			break;

		case MP_ATTACH:
			orig_body = mbody;
			break;
	}

	// add date, x-mailer, message-id
	send_body = augment_wire_form(orig_body, mctypr, tpMailItem, TRUE);
	mem_free(&mbody);
	mem_free(&mctypr);
	if (send_body == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
#ifndef WSAASYNC
		encatt_free(&enc_att, num_att);
#endif
		return FALSE;
	}

#ifdef WSAASYNC
	send_pt = send_body;
	send_len = tstrlen(send_body);

	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
#else
	if (send_body != NULL && send_buf(soc, send_body) == -1) {
		send_body = send_pt = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	send_body = NULL;
	ResetTimeoutTimer();

	if (send_buf(soc, CMD_DATA_END) == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
#endif

	// format the date only if we get this far 
	mem_free(&tpMailItem->FmtDate);
#ifdef UNICODE
	din = alloc_tchar_to_char(tpMailItem->Date);
	dout = (char *)mem_alloc(BUF_SIZE);
	if (din != NULL && dout != NULL) {
		DateConv(din, dout, FALSE);
		tpMailItem->FmtDate = alloc_char_to_tchar(dout);
	}
	mem_free(&din);
	mem_free(&dout);
#else
	DateConv(tpMailItem->Date, buf, FALSE);
	tpMailItem->FmtDate = alloc_copy(buf);
#endif

	return TRUE;
}

/*
 * send_mail_data - ヘッダと本文の送信 (RFC 822, RFC 2822, RFC 2045, RFC 2076)
 */
static BOOL send_mail_data(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr)
{
	MAILITEM *tpFwdMailItem = NULL;
#ifdef UNICODE
	TCHAR *body;
	char *din, *dout;
#endif
	TCHAR *FromAddress = NULL;
	TCHAR *BodyCSet, *octype;
	TCHAR buf[BUF_SIZE];
	TCHAR *p, *q, *r;
	char ctype[BUF_SIZE], enc_type[BUF_SIZE];
	char *mctypr, *mbody;
	char **enc_att = NULL;
	int len, enc, num_att;

	if (send_mail_box->UseReplyToForFrom == 1
		&& tpMailItem->ReplyTo != NULL && *tpMailItem->ReplyTo != TEXT('\0')) {
		FromAddress = tpMailItem->ReplyTo;
	} else {
		FromAddress = send_mail_box->MailAddress;
	}
	if (FromAddress != NULL && *FromAddress != TEXT('\0')) {
		len = lstrlen(TEXT(" <>"));
		p = NULL;

		if (send_mail_box->UserName != NULL) {
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(send_mail_box->UserName) + 1));
			if (p != NULL) {
				SetUserName(send_mail_box->UserName, p);
				len += lstrlen(p);
			}
		}
		len += lstrlen(FromAddress);
		
		r = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (r != NULL) {
			q = r;
			if (p != NULL && *p != TEXT('\0')) {
				q = str_join_t(q, p, TEXT(" "), (TCHAR *)-1);
			}
			str_join_t(q, TEXT("<"), FromAddress, TEXT(">"), (TCHAR *)-1);
		}
		mem_free(&p);
		if (tpMailItem->RedirectTo != NULL) {
			len += lstrlen(tpMailItem->RedirectTo) + lstrlen(STR_MSG_BYWAYOF);
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
			if (p != NULL) {
				wsprintf(p, STR_MSG_BYWAYOF, tpMailItem->RedirectTo, r);
				mem_free(&r);
				r = p;
			}
		}
	}

	if (tpMailItem->RedirectTo != NULL) {
		mem_free(&tpMailItem->To);
		tpMailItem->To = r;
	} else {
		mem_free(&tpMailItem->From);
		tpMailItem->From = r;
	}

	// From
	if (tpMailItem->From != NULL) {
		if (send_mime_header(soc, tpMailItem, HEAD_FROM, tpMailItem->From, TRUE, ErrStr) == FALSE) {
			return FALSE;
		}
	}
	// To
	if (send_mime_header(soc, tpMailItem, HEAD_TO, tpMailItem->To, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Cc
	if (send_mime_header(soc, tpMailItem, HEAD_CC, tpMailItem->Cc, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Date
	GetTimeString(buf);
	if (op.SendDate == 1 && send_header_t(soc, TEXT(HEAD_DATE), buf, ErrStr) == FALSE) {
		return FALSE;
	}
	mem_free(&tpMailItem->Date);
	tpMailItem->Date = alloc_copy_t(buf);
	
	// Subject
	if (send_mime_header(soc, tpMailItem, HEAD_SUBJECT, tpMailItem->Subject, FALSE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Reply-To
	if (send_mail_box->UseReplyToForFrom == 0
		&& tpMailItem->ReplyTo != NULL && *tpMailItem->ReplyTo != TEXT('\0')) {
		// メールに設定されている Reply-To
		if (send_mime_header(soc, tpMailItem, HEAD_REPLYTO, tpMailItem->ReplyTo, TRUE, ErrStr) == FALSE) {
			return FALSE;
		}
	}
	// In-Reply-To
	if (send_header_t(soc, TEXT(HEAD_INREPLYTO), tpMailItem->InReplyTo, ErrStr) == FALSE) {
		return FALSE;
	}
	// References
	if (send_header_t(soc, TEXT(HEAD_REFERENCES), tpMailItem->References, ErrStr) == FALSE) {
		return FALSE;
	}
	if (op.SendMessageId == 1) {
		// Message-Id
		mem_free(&tpMailItem->MessageID);
		tpMailItem->MessageID = CreateMessageId((long)tpMailItem, send_mail_box->MailAddress);
		if (tpMailItem->MessageID == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		if (send_header_t(soc, TEXT(HEAD_MESSAGEID), tpMailItem->MessageID, ErrStr) == FALSE) {
			return FALSE;
		}
	}
	// MIME-Version
	if (send_header_t(soc, TEXT(HEAD_MIMEVERSION), TEXT(MIME_VERSION), ErrStr) == FALSE) {
		return FALSE;
	}
	if (tpMailItem->BodyCharset != NULL) {
		BodyCSet = tpMailItem->BodyCharset;
		enc = tpMailItem->BodyEncoding;
	} else {
		BodyCSet = op.BodyCharset;
		enc = op.BodyEncoding;
	}
	octype = NULL;
	if (tpMailItem->RedirectTo != NULL && tpMailItem->ContentType != NULL) {
		octype = alloc_copy_t(tpMailItem->ContentType);
		for (p = octype; *p != TEXT('\0'); p++) {
			while (*p == TEXT(' ') || *p == TEXT('\r') || *p == TEXT('\n') || *p == TEXT('\t')) {
				p++;
			}
			if (str_cmp_ni_t(p, TEXT("charset"), lstrlen(TEXT("charset"))) == 0) {
				q = r = p;
				while (*q != TEXT(';') && *q != TEXT('\0')) {
					q++;
				}
				if (*q == TEXT(';')) {
					q++;
				}
				while (*q == TEXT(' ') || *q == TEXT('\r') || *q == TEXT('\n') || *q == TEXT('\t')) {
					q++;
				}
				while (*q != TEXT('\0')) {
					*(r++) = *(q++);
				}
			}
			while (*p != TEXT(';') && *p != TEXT('\0')) {
				p++;
			}
		}
		*p = TEXT('\0');
	}
#ifdef UNICODE
	body = alloc_char_to_tchar(tpMailItem->Body);
	send_body = MIME_body_encode(body, BodyCSet, enc, octype, ctype, enc_type, ErrStr);
	mem_free(&body);
#else
	send_body = MIME_body_encode(tpMailItem->Body, BodyCSet, enc, octype, ctype, enc_type, ErrStr);
#endif
	mem_free(&octype);
	if (send_body == NULL) {
		tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
		return FALSE;
	}
	if (tpMailItem->FwdAttach != NULL  && *(tpMailItem->FwdAttach) != TEXT('\0')) {
		// GJC - find original message to forward attachments
		int i, j;
		i = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_ACCOUNT);
		if (i == -1) {
			i = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_SAVE);
		}
		if (i != -1) {
			if ((j = item_find_thread(MailBox + i, tpMailItem->References, (MailBox+i)->MailItemCnt)) != -1) {
				tpFwdMailItem = (*((MailBox + i)->tpMailItem + j));
			}
		}
		if (tpFwdMailItem == NULL) {
			tpFwdMailItem = item_find_thread_anywhere(tpMailItem->References);
		}
		if (tpFwdMailItem == NULL) {
			mem_free(&send_body);
			send_body = NULL;
			lstrcpy(ErrStr, STR_ERR_SOCK_NOATTACH);
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return FALSE;
		}
	}

	if (tpMailItem->RedirectTo != NULL) {
		// Content-Type - derived from original message
		if (send_header(soc, HEAD_CONTENTTYPE, ctype, ErrStr) == FALSE) {
			mem_free(&send_body);
			send_body = NULL;
			return FALSE;
		}
		// Content-Transfer-Encoding
		if (send_header(soc, HEAD_ENCODING, enc_type, ErrStr) == FALSE) {
			mem_free(&send_body);
			send_body = NULL;
			return FALSE;
		}
		num_att = 0;
		enc_att = NULL;
	} else {
		switch (multipart_create(tpMailItem->Attach, tpMailItem->FwdAttach, tpFwdMailItem, 
								ctype, enc_type, &mctypr, send_body, &mbody, &num_att, &enc_att)) {
		case MP_ERROR_FILE:
			mem_free(&send_body);
			send_body = NULL;
			lstrcpy(ErrStr, STR_ERR_SOCK_NOATTACH);
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return FALSE;

		case MP_ERROR_ALLOC:
			mem_free(&send_body);
			send_body = NULL;
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_ERROR;
			return FALSE;

		case MP_NO_ATTACH:
			// Content-Type
			if (send_header(soc, HEAD_CONTENTTYPE, ctype, ErrStr) == FALSE) {
				mem_free(&send_body);
				send_body = NULL;
				return FALSE;
			}
			// Content-Transfer-Encoding
			if (send_header(soc, HEAD_ENCODING, enc_type, ErrStr) == FALSE) {
				mem_free(&send_body);
				send_body = NULL;
				return FALSE;
			}
			break;

		case MP_ATTACH:
			mem_free(&send_body);
			send_body = mbody;
			// Content-Type
			if (send_header(soc, HEAD_CONTENTTYPE, mctypr, ErrStr) == FALSE) {
				mem_free(&send_body);
				send_body = NULL;
				mem_free(&mctypr);
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}
			mem_free(&mctypr);
			break;
		}

		////////////////////// MRP ////////////////////
		// this is where the Priority and receipts go.
		if (tpMailItem->Priority == 1) {
			if(send_header_t(soc, TEXT(HEAD_X_PRIORITY), PRIORITY_NUMBER1, ErrStr) == FALSE){
				mem_free(&send_body);
				send_body = NULL;
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}	

			if(send_header_t(soc, TEXT(HEAD_IMPORTANCE), HIGH_PRIORITY, ErrStr) == FALSE){
				mem_free(&send_body);
				send_body = NULL;
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}	
		} else if (tpMailItem->Priority == 5) {
			if(send_header_t(soc, TEXT(HEAD_X_PRIORITY), PRIORITY_NUMBER5, ErrStr) == FALSE){
				mem_free(&send_body);
				send_body = NULL;
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}	

			if(send_header_t(soc, TEXT(HEAD_IMPORTANCE), LOW_PRIORITY, ErrStr) == FALSE){
				mem_free(&send_body);
				send_body = NULL;
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}	
		} // else if (tpMailItem->Priority == 3) don't bother sending it

		// delivery receipt
		if (tpMailItem->DeliveryReceipt == 1) {
			if(send_header_t(soc, TEXT(HEAD_DELIVERY), tpMailItem->From, ErrStr) == FALSE){
				mem_free(&send_body);
				send_body = NULL;
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}	
		}

		// read receipt
		if (tpMailItem->ReadReceipt == 1) {
			if(send_header_t(soc, TEXT(HEAD_READ1), tpMailItem->From, ErrStr) == FALSE){
				mem_free(&send_body);
				send_body = NULL;
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}	
	
			if(send_header_t(soc, TEXT(HEAD_READ2), tpMailItem->From, ErrStr) == FALSE){
				mem_free(&send_body);
				send_body = NULL;
#ifndef WSAASYNC
				encatt_free(&enc_att, num_att);
#endif
				return FALSE;
			}	
		}
		////////////////////// ---- //////////////////

		// X-Mailer
		if (send_header_t(soc, TEXT(HEAD_X_MAILER), APP_NAME, ErrStr) == FALSE) {
			mem_free(&send_body);
			send_body = NULL;
#ifndef WSAASYNC
			encatt_free(&enc_att, num_att);
#endif
			return FALSE;
		}
	}

	// blank line - end of headers
	if (send_buf(soc, "\r\n") == -1) {
		mem_free(&send_body);
		send_body = NULL;
#ifndef WSAASYNC
		encatt_free(&enc_att, num_att);
#endif
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}

#ifdef WSAASYNC
	send_pt = send_body;
	send_len = tstrlen(send_body);
	// 送信のイベントを作成
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
#else
	// 本文送信
	if (send_body != NULL && send_buf(soc, send_body) == -1) {
		mem_free(&send_body);
		send_body = send_pt = NULL;
		encatt_free(&enc_att, num_att);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&send_body);
	send_body = NULL;
	ResetTimeoutTimer();
	if (num_att != 0) {
		int i;
		for (i = 0; i < num_att; i++) {
			if (i < num_att - 1) {
				wsprintf(buf, STR_STATUS_SEND_ATT, i+1);
			} else {
				wsprintf(buf, STR_STATUS_ATT_END);
			}
			SetSocStatusTextT(hWnd, buf);
			if (enc_att[i] != NULL && send_buf(soc, enc_att[i]) == -1) {
				mem_free(&send_body);
				send_body = NULL;
				encatt_free(&enc_att, num_att);
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			ResetTimeoutTimer();
		}
	}
	encatt_free(&enc_att, num_att);

	if (send_buf(soc, CMD_DATA_END) == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
#endif

	// format the date only if we get this far 
	mem_free(&tpMailItem->FmtDate);
#ifdef UNICODE
	din = alloc_tchar_to_char(tpMailItem->Date);
	dout = (char *)mem_alloc(BUF_SIZE);
	if (din != NULL && dout != NULL) {
		DateConv(din, dout, FALSE);
		tpMailItem->FmtDate = alloc_char_to_tchar(dout);
	}
	mem_free(&din);
	mem_free(&dout);
#else
	DateConv(tpMailItem->Date, buf, FALSE);
	tpMailItem->FmtDate = alloc_copy(buf);
#endif

	if (tpMailItem->WireForm != NULL) {
		augment_wire_form(NULL, NULL, tpMailItem, FALSE);
	}

	return TRUE;
}

/*
 * send_mail_proc - SMTPの応答プロシージャ (RFC 821, RFC 2821)
 */
static BOOL send_mail_proc(HWND hWnd, SOCKET soc, char *buf, TCHAR *ErrStr, MAILITEM *tpMailItem, BOOL ShowFlag)
{
	static TCHAR *To, *Cc, *Bcc;
	static int auth_type;
	HWND hListView;
	TCHAR *wbuf;
	TCHAR *user;
	TCHAR *pass;
	TCHAR *p, *r;
	int status = 0;
	int i;

	if (buf != NULL) {
		SetSocStatusText(hWnd, buf);
		if (auth_flag == TRUE && command_status == SMTP_EHLO) {
			auth_get_type(buf, &auth_type);
		}
		if (command_status == SMTP_EHLO) {
			if (check_starttls(buf) == TRUE) {
				starttls_flag = TRUE;
			}
		}
		if ((status = check_response(buf)) == -1) {
			return TRUE;
		}
	}
	switch (command_status) {
	case SMTP_START:
	case SMTP_STARTTLS:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RESPONSE);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// メールアドレスのチェック
		if (send_mail_box->MailAddress == NULL || *send_mail_box->MailAddress == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_BADFROM);
			return FALSE;
		}
		// ドメインの抽出
		if (op.SendHelo == NULL || *op.SendHelo == TEXT('\0')) {
			p = send_mail_box->MailAddress;

			for (; *p != TEXT('@') && *p != TEXT('\0'); p++);
			if (*p == TEXT('\0')) {
				lstrcpy(ErrStr, STR_ERR_SOCK_BADFROM);
				return FALSE;
			}
			p++;
		} else {
			p = op.SendHelo;
		}
		// STARTTLS
		if (command_status == SMTP_STARTTLS) {
			ssl_type = 1;
			if (init_ssl(hWnd, soc, ErrStr) == -1) {
				return FALSE;
			}
		}
		// HELO ドメイン\r\n
		r = TEXT(CMD_HELO)TEXT(" ");
		command_status = SMTP_HELO;
		if (send_mail_box->SmtpSSL == 1 && send_mail_box->SmtpSSLInfo.Type == 4) {
			r = TEXT(CMD_EHLO)TEXT(" ");
			command_status = SMTP_EHLO;
		}
		if (auth_flag == FALSE && send_mail_box->SmtpAuth == 1) {
			// SMTP認証
			auth_flag = TRUE;
			auth_type = AUTH_NON;
			r = TEXT(CMD_EHLO)TEXT(" ");
			command_status = SMTP_EHLO;
		} else {
			auth_flag = FALSE;
			if (op.ESMTP != 0) {
				// ESMTP
				r = TEXT(CMD_EHLO)TEXT(" ");
				command_status = SMTP_EHLO;
			}
		}
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(r) + lstrlen(p) + lstrlen(TEXT("\r\n")) + 1));
		if (wbuf == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		str_join_t(wbuf, r, p, (TCHAR *)-1);
		SetSocStatusTextT(hWnd, wbuf);
		lstrcat(wbuf, TEXT("\r\n"));
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&wbuf);
		break;

	case SMTP_EHLO:
		if (status >= SMTP_ERRORSTATUS) {
			// EHLO に失敗した場合は HELO を送信し直す
			command_status = SMTP_START;
			return send_mail_proc(hWnd, soc, NULL, ErrStr, tpMailItem, ShowFlag);
		}
		if (starttls_flag == TRUE && send_mail_box->SmtpSSL == 1 && send_mail_box->SmtpSSLInfo.Type == 4) {
			// STARTTLS
			SetSocStatusTextT(hWnd, TEXT(CMD_STARTTLS));
			if (send_buf(soc, CMD_STARTTLS"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			auth_flag = FALSE;
			starttls_flag = FALSE;
			command_status = SMTP_STARTTLS;
			break;
		}
		if (send_mail_box->SmtpAuth == 0) {
			// 初期化
			SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
			if (send_buf(soc, CMD_RSET"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			command_status = SMTP_RSET;
			break;
		}
		
		if (send_mail_box->SmtpAuthType != 0) {
			// 強制的にSMTP認証方式を設定
			auth_type = send_mail_box->SmtpAuthType - 1;
		}
		// SMTP認証
		switch (auth_type) {
		case AUTH_CRAM_MD5:
		case AUTH_NON:
		default:
			// AUTH CRAM-MD5
			SetSocStatusTextT(hWnd, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_CRAM_MD5));
			if (send_buf(soc, CMD_AUTH" "CMD_AUTH_CRAM_MD5"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			command_status = SMTP_AUTH_CRAM_MD5;
			break;

		case AUTH_LOGIN:
			// AUTH LOGIN
			SetSocStatusTextT(hWnd, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_LOGIN));
			if (send_buf(soc, CMD_AUTH" "CMD_AUTH_LOGIN"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			command_status = SMTP_AUTH_LOGIN;
			break;

		case AUTH_PLAIN:
			// AUTH PLAIN
			user = (send_mail_box->AuthUserPass == 1) ? send_mail_box->SmtpUser : send_mail_box->User;
			pass = auth_get_password(send_mail_box);
			if (pass == NULL || *pass == TEXT('\0')) {
				lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
				return FALSE;
			}
			p = auth_create_plain(user, pass, ErrStr);
			if (p == NULL) {
				return FALSE;
			}
			wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) *
				(lstrlen(TEXT(CMD_AUTH)) + 1 + lstrlen(TEXT(CMD_AUTH_PLAIN)) + 1 + lstrlen(p) + 2 + 1));
			if (wbuf == NULL) {
				mem_free(&p);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			// AUTH PLAIN Base64(<NULL>User<NULL>Pass)[CRLF] を送信する
			str_join_t(wbuf, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_PLAIN)TEXT(" "), p, TEXT("\r\n"), (TCHAR *)-1);
			SetSocStatusTextT(hWnd, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_PLAIN)TEXT(" ****"));
			if (send_buf_t(soc, wbuf) == -1) {
				mem_free(&p);
				mem_free(&wbuf);
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			mem_free(&p);
			mem_free(&wbuf);
			command_status = SMTP_HELO;
			break;
		}
		break;

	case SMTP_AUTH_CRAM_MD5:
		if (status >= SMTP_ERRORSTATUS) {
			auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// AUTH CRAM-MD5のパスワードを作成
		user = (send_mail_box->AuthUserPass == 1) ? send_mail_box->SmtpUser : send_mail_box->User;
		pass = auth_get_password(send_mail_box);
		if (pass == NULL || *pass == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
			return FALSE;
		}
		wbuf = auth_create_cram_md5(buf, user, pass, ErrStr);
		if (wbuf == NULL) {
			return FALSE;
		}
		p = (TCHAR *)mem_calloc(sizeof(TCHAR) * (lstrlen(wbuf) * 3 + 4));
		if (p == NULL) {
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		//Base64エンコード
		base64_encode_t(wbuf, p, 0, 0);
		mem_free(&wbuf);

		// パスワード送信
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("\r\n")) + lstrlen(p) + 1));
		if (wbuf == NULL) {
			mem_free(&p);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		str_join_t(wbuf, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, STR_STATUS_SEND_PASS);
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&p);
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&p);
		mem_free(&wbuf);
		command_status = SMTP_HELO;
		break;

	case SMTP_AUTH_LOGIN:
		if (status >= SMTP_ERRORSTATUS) {
			auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		user = (send_mail_box->AuthUserPass == 1) ? send_mail_box->SmtpUser : send_mail_box->User;
		p = (TCHAR *)mem_calloc(sizeof(TCHAR) * (lstrlen(user) * 3 + 4));
		if (p == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		// Base64エンコード
		base64_encode_t(user, p, 0, 0);
		// ユーザ名送信
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("\r\n")) + lstrlen(p) + 1));
		if (wbuf == NULL) {
			mem_free(&p);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		str_join_t(wbuf, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, STR_STATUS_SEND_USER);
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&p);
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&p);
		mem_free(&wbuf);
		command_status = SMTP_AUTH_LOGIN_PASS;
		break;

	case SMTP_AUTH_LOGIN_PASS:
		if (status >= SMTP_ERRORSTATUS) {
			auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		pass = auth_get_password(send_mail_box);
		if (pass == NULL || *pass == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
			return FALSE;
		}
		p = (TCHAR *)mem_calloc(sizeof(TCHAR) * (lstrlen(pass) * 3 + 4));
		if (p == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		// Base64エンコード
		base64_encode_t(pass, p, 0, 0);
		// パスワード送信
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("\r\n")) + lstrlen(p) + 1));
		if (wbuf == NULL) {
			mem_free(&p);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		str_join_t(wbuf, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, STR_STATUS_SEND_PASS);
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&p);
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&p);
		mem_free(&wbuf);
		command_status = SMTP_HELO;
		break;

	case SMTP_HELO:
		if (status >= SMTP_ERRORSTATUS) {
			if (auth_flag == FALSE) {
				lstrcpy(ErrStr, STR_ERR_SOCK_HELO);
			} else {
				auth_flag = FALSE;
				lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			}
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// 初期化
		SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
		if (send_buf(soc, CMD_RSET"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		command_status = SMTP_RSET;
		break;

	case SMTP_RSET:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RSET);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// FROM
		if (tpMailItem->RedirectTo != NULL) {
			if (tpMailItem->From == NULL) {
				lstrcpy(ErrStr, STR_ERR_SOCK_NOFROM);
				tpMailItem->MailStatus = tpMailItem->Mark = ICON_ERROR;
				return FALSE;
			}
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(tpMailItem->From) + 1));
			if (p != NULL) {
				GetMailAddress(tpMailItem->From, p, NULL, FALSE);
			}
		} else {
			p = alloc_copy_t(send_mail_box->MailAddress);
		}
		if (send_address(hWnd, soc, TEXT(CMD_MAIL_FROM), p, ErrStr) == FALSE) {
			mem_free(&p);
			return FALSE;
		}
		mem_free(&p);
		command_status = SMTP_MAILFROM;
		break;

	case SMTP_MAILFROM:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILFROM);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			tpMailItem->MailStatus = tpMailItem->Mark = ICON_ERROR;
			return FALSE;
		}
		if (tpMailItem->RedirectTo != NULL) {
			To = tpMailItem->RedirectTo;
			Cc = NULL;
			Bcc = NULL;
		} else {
			To = tpMailItem->To;
			Cc = tpMailItem->Cc;
			Bcc = tpMailItem->Bcc;
		}
		command_status = SMTP_RCPTTO;
		return send_mail_proc(hWnd, soc, NULL, ErrStr, tpMailItem, ShowFlag);

	case SMTP_RCPTTO:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RCPTTO);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			tpMailItem->MailStatus = tpMailItem->Mark = ICON_ERROR;
			return FALSE;
		}

		// To に指定されたメールアドレスの送信
		if (To != NULL && *To != TEXT('\0')) {
			if ((To = send_rcpt_to(hWnd, soc, To, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			command_status = SMTP_RCPTTO;
			break;
		}
		// Cc に指定されたメールアドレスの送信
		if (Cc != NULL && *Cc != TEXT('\0')) {
			if ((Cc = send_rcpt_to(hWnd, soc, Cc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			command_status = SMTP_RCPTTO;
			break;
		}
		// Bcc に指定されたメールアドレスの送信
		if (Bcc != NULL && *Bcc != TEXT('\0')) {
			if ((Bcc = send_rcpt_to(hWnd, soc, Bcc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			command_status = SMTP_RCPTTO;
			break;
		}
		// DATA送信
		SetSocStatusTextT(hWnd, TEXT(CMD_DATA));
		if (send_buf(soc, CMD_DATA"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		command_status = SMTP_DATA;
		break;

	case SMTP_DATA:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_DATA);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// メールデータの送信
		SetSocStatusTextT(hWnd, STR_STATUS_SENDBODY);
		SwitchCursor(FALSE);
		if (tpMailItem->WireForm != NULL) { // && send_mail_box->SendWireForm == TRUE
			if (send_wire_form(hWnd, soc, tpMailItem, ErrStr) == FALSE) {
				SwitchCursor(TRUE);
				return FALSE;
			}
		} else {
			if (send_mail_data(hWnd, soc, tpMailItem, ErrStr) == FALSE) {
				SwitchCursor(TRUE);
				return FALSE;
			}
		}
		SwitchCursor(TRUE);
#ifdef WSAASYNC
		command_status = SMTP_SENDBODY;
		break;

	case SMTP_SENDBODY:
		break;
#else
		command_status = send_end_cmd;
		break;
#endif

	case SMTP_NEXTSEND:
		// 次の送信メール
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILSEND);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		tpMailItem->Mark = tpMailItem->MailStatus = ICON_SENTMAIL;
		(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
		if (ShowFlag == TRUE) {
			hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
			i = ListView_GetMemToItem(hListView, tpMailItem);
			if (i != -1) {
				ListView_RedrawItems(hListView, i, i);
				UpdateWindow(hListView);
			}
		}
		
		if (tpMailItem->FwdAttach && tpMailItem->References) {
			ClearFwdHold(tpMailItem);
		}

		// check if there are other messages queued for sending
		i = item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, 
				mailbox_name_to_index(send_mail_box->Name, MAILBOX_TYPE_ACCOUNT));
		if (i == -1) {
			reorder_sendbox();
		}
		if (op.AutoSave == 1 || (i == -1 && op.AutoSave == 2)) {
			file_save_mailbox(SENDBOX_FILE, DataDir, MAILBOX_SEND, FALSE, TRUE, 2);
		}
		if (i == -1) {
			// メール送信終了
			SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
			if (send_buf(soc, CMD_QUIT"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			command_status = SMTP_QUIT;
			break;
		}
		if (send_mail_item == *((MailBox + MAILBOX_SEND)->tpMailItem + i)) {
			lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
			return FALSE;
		}
		send_mail_item = *((MailBox + MAILBOX_SEND)->tpMailItem + i);
		if ((send_mail_item->RedirectTo == NULL || *send_mail_item->RedirectTo == TEXT('\0')) &&
			(send_mail_item->To == NULL || *send_mail_item->To == TEXT('\0')) &&
			(send_mail_item->Cc == NULL || *send_mail_item->Cc == TEXT('\0')) &&
			(send_mail_item->Bcc == NULL || *send_mail_item->Bcc == TEXT('\0'))) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
			send_mail_item->Mark = send_mail_item->MailStatus = ICON_ERROR;
			return FALSE;
		}
		// 初期化
		SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
		if (send_buf(soc, CMD_RSET"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		command_status = SMTP_RSET;
		break;

	case SMTP_SENDEND:
		// 送信終了
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILSEND);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
		if (send_buf(soc, CMD_QUIT"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		tpMailItem->Mark = tpMailItem->MailStatus = ICON_SENTMAIL;
		if (ShowFlag == TRUE) {
			hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
			i = ListView_GetMemToItem(hListView, tpMailItem);
			if (i != -1) {
				ListView_RedrawItems(hListView, i, i);
				UpdateWindow(hListView);
			}
		}
		(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
		reorder_sendbox();
		if (op.AutoSave != 0) {
			file_save_mailbox(SENDBOX_FILE, DataDir, MAILBOX_SEND, FALSE, TRUE, 2);
		}
		if (tpMailItem->FwdAttach && tpMailItem->References) {
			ClearFwdHold(tpMailItem);
		}
		command_status = SMTP_QUIT;
		break;

	case SMTP_QUIT:
		if (gAutoSend == AUTOSEND_AND_QUIT) {
			PostMessage(hWnd, WM_COMMAND, ID_MENUITEM_QUIT, 0);
		}
		break;
	}

	return TRUE;
}

/*
 * smtp_send_proc - データ送信プロシージャ
 */
#ifdef WSAASYNC
BOOL smtp_send_proc(HWND hWnd, SOCKET soc, TCHAR *ErrStr)
{
	int len;

	if (send_body == NULL && send_pt != NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	// 送信終了
	if (send_body == NULL || *send_pt == '\0') {
		if (send_data(soc, CMD_DATA_END, tstrlen(CMD_DATA_END)) == -1) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				return TRUE;
			}
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&send_body);
		send_body = send_pt = NULL;
		command_status = send_end_cmd;
		if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
			lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
			return FALSE;
		}
		return TRUE;
	}
	// 分割して送信
	len = (send_len - (send_pt - send_body) < SEND_SIZE) ? send_len - (send_pt - send_body) : SEND_SIZE;
	if ((len = send_data(soc, send_pt, len)) == -1) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return TRUE;
		}
		mem_free(&send_body);
		send_body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	send_pt += len;
	SetStatusRecvLen(hWnd, send_pt - send_body, send_len, STR_STATUS_SOCKINFO_SEND);
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&send_body);
		send_body = send_pt = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
	return TRUE;
}
#endif

/*
 * smtp_proc - SMTPプロシージャ
 */
BOOL smtp_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	BOOL ret;

	if (send_mail_item == NULL) {
		return FALSE;
	}
	ret = send_mail_proc(hWnd, soc, buf, ErrStr, send_mail_item, ShowFlag);
	if (ret == FALSE) {
		send_buf(soc, CMD_QUIT"\r\n");
		if (hWnd != NULL) {
			//In status bar information of error indicatory
			SetStatusTextT(hWnd, STR_ERR_SOCK_SEND, 1);
		}
		if (op.SocLog > 0) log_save(STR_ERR_SOCK_SEND TEXT("\r\n"));
		if (send_mail_item != NULL && buf != NULL && *buf == '5') {
			log_save_a("buffer had a 5 -- some kind of error");
			send_mail_item->Mark = ICON_ERROR;
		}
		smtp_set_error(hWnd);
	}
	return ret;
}

/*
 * smtp_send_mail - メール送信の初期化
 */
SOCKET smtp_send_mail(HWND hWnd, MAILBOX *tpMailBox, MAILITEM *tpMailItem, int end_cmd, TCHAR *ErrStr)
{
	SOCKET soc;

	send_mail_box = tpMailBox;
	send_mail_item = tpMailItem;
	auth_flag = FALSE;
	starttls_flag = FALSE;
	send_end_cmd = end_cmd;

	if ((send_mail_item->RedirectTo == NULL || *send_mail_item->RedirectTo == TEXT('\0')) &&
		(send_mail_item->To == NULL || *send_mail_item->To == TEXT('\0')) &&
		(send_mail_item->Cc == NULL || *send_mail_item->Cc == TEXT('\0')) &&
		(send_mail_item->Bcc == NULL || *send_mail_item->Bcc == TEXT('\0'))) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
		send_mail_item->Mark = send_mail_item->MailStatus = ICON_ERROR;
		return -1;
	}

	// ホスト名からIPアドレスを取得 (取得したIPは保存)
	if (send_mail_box->SmtpIP == 0 || op.IPCache == 0) {
		GetHostFlag = TRUE;
		send_mail_box->SmtpIP = get_host_by_name(hWnd, send_mail_box->SmtpServer, ErrStr);
		GetHostFlag = FALSE;
		if (send_mail_box->SmtpIP == 0) {
			return -1;
		}
	}
	// 接続開始
	soc = connect_server(hWnd,
		send_mail_box->SmtpIP, (unsigned short)send_mail_box->SmtpPort,
		(send_mail_box->SmtpSSL == 0 || send_mail_box->SmtpSSLInfo.Type == 4) ? -1 : send_mail_box->SmtpSSLInfo.Type,
		&send_mail_box->SmtpSSLInfo,
		ErrStr);
	if (soc == -1) {
		send_mail_box->SmtpIP = 0;
		return -1;
	}
	return soc;
}

/*
 * reorder_sendbox - make sure drafts (unsent messages) stay at the end
 */
static void reorder_sendbox()
{
	if (op.ReorderSendbox) {
		MAILITEM **tpMailList = (MailBox + MAILBOX_SEND)->tpMailItem;
		int i, j, cnt = (MailBox + MAILBOX_SEND)->MailItemCnt;
		BOOL did = FALSE;

		SwitchCursor(FALSE);
		for (i=0; i < cnt-1; i++) {
			MAILITEM *tpItem1, *tpItem2;
			tpItem1 = *(tpMailList + i);
			tpItem2 = *(tpMailList + i + 1);
			if (tpItem2->MailStatus == ICON_SENTMAIL 
				&& (tpItem1->MailStatus == ICON_NON
					|| tpItem1->MailStatus == ICON_SEND
					|| tpItem1->MailStatus == ICON_ERROR)) {
				*(tpMailList + i) = tpItem2;
				*(tpMailList + i + 1) = tpItem1;
				did = TRUE;
				for (j = i - 1; j >= 0; j--) {
					// tpItem2 is the Sent message we're moving up
					tpItem1 = *(tpMailList + j);
					if (tpItem1->MailStatus == ICON_NON
							|| tpItem1->MailStatus == ICON_SEND
							|| tpItem1->MailStatus == ICON_ERROR) {
						*(tpMailList + j) = tpItem2;
						*(tpMailList + j + 1) = tpItem1;
					} else {
						break;
					}
				}
			}
		}
		if (did) {
			if (SelBox == MAILBOX_SEND) {
				ListView_ShowItem(GetDlgItem(MainWnd, IDC_LISTVIEW), MailBox + MAILBOX_SEND, FALSE, TRUE);
			}
			(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
		}
		SwitchCursor(TRUE);
	}
}

/*
 * ClearFwdHold - clear REFWD_FWDHOLD bit when OutBox message is sent or deleted
 */
void ClearFwdHold(MAILITEM *tpSentItem)
{
	MAILBOX *tpMailBox = MailBox + MAILBOX_SEND;
	MAILITEM *tpMailItem;
	TCHAR *ref = NULL;
	int i, j;

	if (tpSentItem != NULL) {
		ref = tpSentItem->References;
	}
	if (ref == NULL) {
		return;
	}
	if (tpMailBox != NULL && tpMailBox->Loaded) {
		for (j = 0; j < tpMailBox->MailItemCnt; j++) {
			tpMailItem = *(tpMailBox->tpMailItem + j);
			if (tpMailItem && tpMailItem != tpSentItem && tpMailItem->FwdAttach != NULL
				&& tpMailItem->References != NULL && tpMailItem->MailStatus != ICON_SENTMAIL
				&& lstrcmp(ref, tpMailItem->References) == 0) {
				// there's another unsent message that needs the hold
				return;
			}
		}
	}

	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		BOOL still_held = FALSE;
		tpMailBox = MailBox + i;
		if (tpMailBox != NULL && tpMailBox->Loaded && tpMailBox->HeldMail) {
			for (j = 0; j < tpMailBox->MailItemCnt; j++) {
				tpMailItem = *(tpMailBox->tpMailItem + j);
				if (tpMailItem && tpMailItem->MessageID && (tpMailItem->ReFwd & REFWD_FWDHOLD)) {
					if (lstrcmp(ref, tpMailItem->MessageID) == 0) {
						tpMailItem->ReFwd &= ~(REFWD_FWDHOLD);
					} else {
						still_held = TRUE;
					}
				}
			}
			tpMailBox->HeldMail = still_held;
		}
	}
}

/*
 * smtp_set_error - 送信メールにエラーステータスを付加する
 */
void smtp_set_error(HWND hWnd)
{
	HWND hListView;
	int i;

	smtp_free();

	if (send_mail_item == NULL || send_mail_item->Mark != ICON_ERROR) {
		return;
	}
	(MailBox + MAILBOX_SEND)->NeedsSave |= MARKS_CHANGED;

	if (SelBox == MAILBOX_SEND) {
		hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		i = ListView_GetMemToItem(hListView, send_mail_item);
		if (i != -1) {
			ListView_RedrawItems(hListView, i, i);
			UpdateWindow(hListView);
		}
	}
	send_mail_item = NULL;
}

/*
 * smtp_free - SMTP情報の解放
 */
void smtp_free(void)
{
	item_free(&SmtpFwdMessage, 1);
	mem_free(&send_body);
	send_body = NULL;
}
/* End of source */
