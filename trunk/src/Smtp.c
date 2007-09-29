/*
 * nPOP
 *
 * Smtp.c (RFC 821, RFC 2821)
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
#define SEND_BUFSIZE			4096			// ���M���̕����T�C�Y
#define SMTP_ERRORSTATUS		400				// SMTP�G���[���X�|���X
#define MIME_VERSION			"1.0"			// MIME�o�[�W����

#define AUTH_CRAM_MD5			0				// SMTP-AUTH �̔F�؃^�C�v
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
static MAILBOX *send_mail_box;					// ���M���̃��[���{�b�N�X
static MAILITEM *send_mail_item;				// ���M���[��
static BOOL auth_flag;							// SMTP-AUTH
static BOOL starttls_flag;						// STARTTLS
static TCHAR *body, *send_pt;					// ���M�p�{��
static int send_len;							// ���M��
static int send_end_cmd;						// ���[�����M�������̃R�}���h

// �O���Q��
extern OPTION op;

extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern int SelBox;

extern BOOL GetHostFlag;
extern int ssl_type;

extern int MailFlag;
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
static BOOL send_header(SOCKET soc, TCHAR *header, TCHAR *content, TCHAR *ErrStr);
static BOOL send_mime_header(SOCKET soc, TCHAR *header, TCHAR *content, BOOL address, TCHAR *ErrStr);
static BOOL send_mail_data(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr);
static BOOL send_mail_proc(HWND hWnd, SOCKET soc, char *buf, TCHAR *ErrStr, MAILITEM *tpMailItem, BOOL ShowFlag);

/*
 * HMAC_MD5 - MD5�̃_�C�W�F�X�g�𐶐�����
 */
void HMAC_MD5(unsigned char *input, int len, unsigned char *key, int keylen, unsigned char *digest)
{
    MD5_CTX context;
    MD5_CTX tctx;
    unsigned char k_ipad[65];
    unsigned char k_opad[65];
    unsigned char tk[16];
    int i;

	// �L�[��64�o�C�g���傫���ꍇ�̓L�[�̃_�C�W�F�X�g���L�[�ɂ���
    if (keylen > 64) {
        MD5Init(&tctx);
        MD5Update(&tctx, key, keylen);
        MD5Final(tk, &tctx);

        key = tk;
        keylen = 16;
    }

	// MD5(key XOR opad, MD5(key XOR ipad, input)) �̌v�Z
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
 * check_response - SMTP���X�|���X�̉��
 */
static int check_response(char *buf)
{
	char stat[5];
	int ret;

	// �����s�̏ꍇ�� -1 ��Ԃ�
	if (tstrlen(buf) > 3 && *(buf + 3) == '-') {
		return -1;
	}

	// �X�e�[�^�X�R�[�h���擾
	StrCpyN(stat, buf, 4);
	ret = a2i(stat);
	if (ret <= 0) {
		ret = SMTP_ERRORSTATUS;
	}
	return ret;
}

/*
 * check_starttls - STARTTLS�����p�\���`�F�b�N
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
	if (StrCmpNI(p, CMD_STARTTLS, tstrlen(CMD_STARTTLS)) == 0) {
		return TRUE;
	}
	return FALSE;
}

/*
 * auth_get_type - ���p�\�� SMTP AUTH �̎�ނ��擾 (RFC 2554)
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
	if (StrCmpNI(p, CMD_AUTH, tstrlen(CMD_AUTH)) != 0) {
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
		if (StrCmpNI(r, CMD_AUTH_CRAM_MD5, tstrlen(CMD_AUTH_CRAM_MD5)) == 0) {
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
		if (StrCmpNI(r, CMD_AUTH_LOGIN, tstrlen(CMD_AUTH_LOGIN)) == 0) {
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
		if (StrCmpNI(r, CMD_AUTH_PLAIN, tstrlen(CMD_AUTH_PLAIN)) == 0) {
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
 * auth_create_cram_md5 - AUTH CRAM-MD5 �̃p�X���[�h�𐶐����� (RFC 2554)
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
	key = AllocTcharToChar(pass);
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
	Base64Decode(p, input);

	// �_�C�W�F�X�g�l�̎擾
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
 * auth_create_plain - AUTH PLAIN �̕�����𐶐����� (RFC 2554)
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
	c_user = AllocTcharToChar(user);
	if (c_user == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	c_pass = AllocTcharToChar(pass);
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
	Base64Encode(tmp, buf, len);
	mem_free(&tmp);
#ifdef UNICODE
	ret = AllocCharToTchar(buf);
	mem_free(&buf);
	return ret;
#else
	return buf;
#endif
}

/*
 * auth_get_password - �p�X���[�h�̎擾
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
 * send_address - ���[���A�h���X�𑗐M����
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
	TStrJoin(wbuf, command, TEXT(":<"), address, TEXT(">\r\n"), (TCHAR *)-1);

	SetSocStatusTextT(hWnd, wbuf);
	if (send_buf_t(soc, wbuf) == -1) {
		mem_free(&wbuf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&wbuf);
	return TRUE;
}

/*
 * send_rcpt_to - RCPT TO �𑗐M����
 */
static TCHAR *send_rcpt_to(HWND hWnd, SOCKET soc, TCHAR *address, TCHAR *ErrStr)
{
	TCHAR *p;

	// ���[���A�h���X�̒��o
	p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(address) + 1));
	if (p == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return (TCHAR *)-1;
	}
	address = GetMailAddress(address, p, FALSE);

	// ���[���A�h���X�̑��M
	if (send_address(hWnd, soc, TEXT(CMD_RCPT_TO), p, ErrStr) == FALSE) {
		mem_free(&p);
		return (TCHAR *)-1;
	}
	mem_free(&p);
	return ((*address != TEXT('\0')) ? address + 1 : address);
}

/*
 * send_header - �w�b�_���ڂ̑��M
 */
static BOOL send_header(SOCKET soc, TCHAR *header, TCHAR *content, TCHAR *ErrStr)
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
	TStrJoin(buf, header, TEXT(" "), content, TEXT("\r\n"), (TCHAR *)-1);
	if (send_buf_t(soc, buf) == -1) {
		mem_free(&buf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&buf);
	return TRUE;
}

/*
 * send_mime_header - �w�b�_���ڂ�MIME�G���R�[�h���đ��M
 */
static BOOL send_mime_header(SOCKET soc, TCHAR *header, TCHAR *content, BOOL address, TCHAR *ErrStr)
{
	TCHAR *p;
	BOOL ret;

	if (content == NULL || *content == TEXT('\0')) {
		return TRUE;
	}
	p = MIMEencode(content, address);
	if (p == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	ret = send_header(soc, header, p, ErrStr);
	mem_free(&p);
	return ret;
}

/*
 * send_mail_data - �w�b�_�Ɩ{���̑��M (RFC 822, RFC 2822, RFC 2045, RFC 2076)
 */
static BOOL send_mail_data(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr)
{
	TCHAR buf[BUF_SIZE];
	TCHAR enc_buf[BUF_SIZE];
	TCHAR *p, *r;
	TCHAR *mctypr;
	TCHAR *mbody;
	int len;

	// From
	mem_free(&tpMailItem->From);
	tpMailItem->From = NULL;
	if (send_mail_box->MailAddress != NULL && *send_mail_box->MailAddress != TEXT('\0')) {
		len = lstrlen(TEXT(" <>"));
		p = NULL;
		// ���[�U���̐ݒ�
		if (send_mail_box->UserName != NULL) {
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(send_mail_box->UserName) + 1));
			if (p != NULL) {
				SetUserName(send_mail_box->UserName, p);
				len += lstrlen(p);
			}
		}
		len += lstrlen(send_mail_box->MailAddress);

		// From�̍쐬�Ƒ��M
		tpMailItem->From = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (tpMailItem->From != NULL) {
			r = tpMailItem->From;
			if (p != NULL && *p != TEXT('\0')) {
				r = TStrJoin(r, p, TEXT(" "), (TCHAR *)-1);
			}

			TStrJoin(r, TEXT("<"), send_mail_box->MailAddress, TEXT(">"), (TCHAR *)-1);
			if (send_mime_header(soc, TEXT(HEAD_FROM), tpMailItem->From, TRUE, ErrStr) == FALSE) {
				return FALSE;
			}
		}
		mem_free(&p);
	}
	// To
	if (send_mime_header(soc, TEXT(HEAD_TO), tpMailItem->To, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Cc
	if (send_mime_header(soc, TEXT(HEAD_CC), tpMailItem->Cc, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Date
	GetTimeString(buf);
	if (op.SendDate == 1 && send_header(soc, TEXT(HEAD_DATE), buf, ErrStr) == FALSE) {
		return FALSE;
	}
	mem_free(&tpMailItem->Date);
#ifdef UNICODE
	{
		char *cbuf, *dbuf;

		cbuf = AllocTcharToChar(buf);
		if (cbuf != NULL) {
			dbuf = (char *)mem_alloc(BUF_SIZE);
			if (dbuf != NULL) {
				DateConv(cbuf, dbuf);
				tpMailItem->Date = AllocCharToTchar(dbuf);
				mem_free(&dbuf);
			}
			mem_free(&cbuf);
		}
	}
#else
	tpMailItem->Date = (char *)mem_alloc(BUF_SIZE);
	if (tpMailItem->Date != NULL) {
		DateConv(buf, tpMailItem->Date);
	}
#endif
	// Subject
	if (send_mime_header(soc, TEXT(HEAD_SUBJECT), tpMailItem->Subject, FALSE, ErrStr) == FALSE) {
		return FALSE;
	}
	// Reply-To
	if (tpMailItem->ReplyTo != NULL && *tpMailItem->ReplyTo != TEXT('\0')) {
		// ���[���ɐݒ肳��Ă��� Reply-To
		if (send_mime_header(soc, TEXT(HEAD_REPLYTO), tpMailItem->ReplyTo, TRUE, ErrStr) == FALSE) {
			return FALSE;
		}
	} else {
		// ���[���{�b�N�X�ɐݒ肳��Ă��� Reply-To
		if (send_mime_header(soc, TEXT(HEAD_REPLYTO), send_mail_box->ReplyTo, TRUE, ErrStr) == FALSE) {
			return FALSE;
		}
	}
	// In-Reply-To
	if (send_header(soc, TEXT(HEAD_INREPLYTO), tpMailItem->InReplyTo, ErrStr) == FALSE) {
		return FALSE;
	}
	// References
	if (send_header(soc, TEXT(HEAD_REFERENCES), tpMailItem->References, ErrStr) == FALSE) {
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
		if (send_header(soc, TEXT(HEAD_MESSAGEID), tpMailItem->MessageID, ErrStr) == FALSE) {
			return FALSE;
		}
	}
	// MIME-Version
	if (send_header(soc, TEXT(HEAD_MIMEVERSION), TEXT(MIME_VERSION), ErrStr) == FALSE) {
		return FALSE;
	}
	// �{���̃G���R�[�h
	body = BodyEncode(tpMailItem->Body, buf, enc_buf, ErrStr);
	if (body == NULL) {
		return FALSE;
	}
	// �Y�t�t�@�C��������ꍇ�̓}���`�p�[�g�ő��M����
	switch (CreateMultipart(tpMailItem->Attach, buf, enc_buf, &mctypr, body, &mbody)) {
	case MP_ERROR_FILE:
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_NOATTACH);
		return FALSE;

	case MP_ERROR_ALLOC:
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;

	case MP_NO_ATTACH:
		// Content-Type
		if (send_header(soc, TEXT(HEAD_CONTENTTYPE), buf, ErrStr) == FALSE) {
			mem_free(&body);
			body = NULL;
			return FALSE;
		}
		// Content-Transfer-Encoding
		if (send_header(soc, TEXT(HEAD_ENCODING), enc_buf, ErrStr) == FALSE) {
			mem_free(&body);
			body = NULL;
			return FALSE;
		}
		break;

	case MP_ATTACH:
		mem_free(&body);
		body = mbody;
		// Content-Type
		if (send_header(soc, TEXT(HEAD_CONTENTTYPE), mctypr, ErrStr) == FALSE) {
			mem_free(&body);
			body = NULL;
			mem_free(&mctypr);
			return FALSE;
		}
		mem_free(&mctypr);
		break;
	}
	// X-Mailer
	if (send_header(soc, TEXT(HEAD_X_MAILER), APP_NAME, ErrStr) == FALSE) {
		mem_free(&body);
		body = NULL;
		return FALSE;
	}
	// �w�b�_�Ɩ{���̋�؂�
	if (send_buf(soc, "\r\n") == -1) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	send_pt = body;
	send_len = 0;

#ifdef WSAASYNC
	// ���M�̃C�x���g���쐬
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
#else
	// �{�����M
	if (body != NULL && send_buf_t(soc, body) == -1) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&body);
	body = NULL;

	if (send_buf(soc, CMD_DATA_END) == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
#endif
	return TRUE;
}

/*
 * send_mail_proc - SMTP�̉����v���V�[�W�� (RFC 821, RFC 2821)
 */
static BOOL send_mail_proc(HWND hWnd, SOCKET soc, char *buf, TCHAR *ErrStr, MAILITEM *tpMailItem, BOOL ShowFlag)
{
	static TCHAR *To, *Cc, *Bcc, *MyBcc;
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
		if (auth_flag == TRUE && MailFlag == SMTP_EHLO) {
			auth_get_type(buf, &auth_type);
		}
		if (MailFlag == SMTP_EHLO) {
			if (check_starttls(buf) == TRUE) {
				starttls_flag = TRUE;
			}
		}
		if ((status = check_response(buf)) == -1) {
			return TRUE;
		}
	}
	switch (MailFlag) {
	case SMTP_START:
	case SMTP_STARTTLS:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RESPONSE);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// ���[���A�h���X�̃`�F�b�N
		if (send_mail_box->MailAddress == NULL || *send_mail_box->MailAddress == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_BADFROM);
			return FALSE;
		}
		// �h���C���̒��o
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
		if (MailFlag == SMTP_STARTTLS) {
			ssl_type = 1;
			if (init_ssl(hWnd, soc, ErrStr) == -1) {
				return FALSE;
			}
		}
		// HELO �h���C��\r\n
		r = TEXT(CMD_HELO)TEXT(" ");
		MailFlag = SMTP_HELO;
		if (send_mail_box->SmtpSSL == 1 && send_mail_box->SmtpSSLInfo.Type == 4) {
			r = TEXT(CMD_EHLO)TEXT(" ");
			MailFlag = SMTP_EHLO;
		}
		if (auth_flag == FALSE && send_mail_box->SmtpAuth == 1) {
			// SMTP�F��
			auth_flag = TRUE;
			auth_type = AUTH_NON;
			r = TEXT(CMD_EHLO)TEXT(" ");
			MailFlag = SMTP_EHLO;
		} else {
			auth_flag = FALSE;
			if (op.ESMTP != 0) {
				// ESMTP
				r = TEXT(CMD_EHLO)TEXT(" ");
				MailFlag = SMTP_EHLO;
			}
		}
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(r) + lstrlen(p) + lstrlen(TEXT("\r\n")) + 1));
		if (wbuf == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		TStrJoin(wbuf, r, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, wbuf);
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&wbuf);
		break;

	case SMTP_EHLO:
		if (status >= SMTP_ERRORSTATUS) {
			// EHLO �Ɏ��s�����ꍇ�� HELO �𑗐M������
			MailFlag = SMTP_START;
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
			MailFlag = SMTP_STARTTLS;
			break;
		}
		if (send_mail_box->SmtpAuth == 0) {
			// ������
			SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
			if (send_buf(soc, CMD_RSET"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			MailFlag = SMTP_RSET;
			break;
		}
		
		if (send_mail_box->SmtpAuthType != 0) {
			// �����I��SMTP�F�ؕ�����ݒ�
			auth_type = send_mail_box->SmtpAuthType - 1;
		}
		// SMTP�F��
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
			MailFlag = SMTP_AUTH_CRAM_MD5;
			break;

		case AUTH_LOGIN:
			// AUTH LOGIN
			SetSocStatusTextT(hWnd, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_LOGIN));
			if (send_buf(soc, CMD_AUTH" "CMD_AUTH_LOGIN"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			MailFlag = SMTP_AUTH_LOGIN;
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
			// AUTH PLAIN Base64(<NULL>User<NULL>Pass)[CRLF] �𑗐M����
			TStrJoin(wbuf, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_PLAIN)TEXT(" "), p, TEXT("\r\n"), (TCHAR *)-1);
			SetSocStatusTextT(hWnd, TEXT(CMD_AUTH)TEXT(" ")TEXT(CMD_AUTH_PLAIN)TEXT(" ****"));
			if (send_buf_t(soc, wbuf) == -1) {
				mem_free(&p);
				mem_free(&wbuf);
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			mem_free(&p);
			mem_free(&wbuf);
			MailFlag = SMTP_HELO;
			break;
		}
		break;

	case SMTP_AUTH_CRAM_MD5:
		if (status >= SMTP_ERRORSTATUS) {
			auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// AUTH CRAM-MD5�̃p�X���[�h���쐬
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

	case SMTP_AUTH_LOGIN:
		if (status >= SMTP_ERRORSTATUS) {
			auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		user = (send_mail_box->AuthUserPass == 1) ? send_mail_box->SmtpUser : send_mail_box->User;
		p = (TCHAR *)mem_calloc(sizeof(TCHAR) * (lstrlen(user) * 2 + 4));
		if (p == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		// Base64�G���R�[�h
		TBase64Encode(user, p, 0);
		// ���[�U�����M
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("\r\n")) + lstrlen(p) + 1));
		if (wbuf == NULL) {
			mem_free(&p);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		TStrJoin(wbuf, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, TEXT("Send user"));
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&p);
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&p);
		mem_free(&wbuf);
		MailFlag = SMTP_AUTH_LOGIN_PASS;
		break;

	case SMTP_AUTH_LOGIN_PASS:
		if (status >= SMTP_ERRORSTATUS) {
			auth_flag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		pass = auth_get_password(send_mail_box);
		if (pass == NULL || *pass == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
			return FALSE;
		}
		p = (TCHAR *)mem_calloc(sizeof(TCHAR) * (lstrlen(pass) * 2 + 4));
		if (p == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		// Base64�G���R�[�h
		TBase64Encode(pass, p, 0);
		// �p�X���[�h���M
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("\r\n")) + lstrlen(p) + 1));
		if (wbuf == NULL) {
			mem_free(&p);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		TStrJoin(wbuf, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, TEXT("Send password"));
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&p);
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&p);
		mem_free(&wbuf);
		MailFlag = SMTP_HELO;
		break;

	case SMTP_HELO:
		if (status >= SMTP_ERRORSTATUS) {
			if (auth_flag == FALSE) {
				lstrcpy(ErrStr, STR_ERR_SOCK_HELO);
			} else {
				auth_flag = FALSE;
				lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			}
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// ������
		SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
		if (send_buf(soc, CMD_RSET"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		MailFlag = SMTP_RSET;
		break;

	case SMTP_RSET:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RSET);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// �ԐM�惁�[���A�h���X�̑��M
		if (send_address(hWnd, soc, TEXT(CMD_MAIL_FROM), send_mail_box->MailAddress, ErrStr) == FALSE) {
			return FALSE;
		}
		MailFlag = SMTP_MAILFROM;
		break;

	case SMTP_MAILFROM:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILFROM);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		if (tpMailItem->To == NULL) {
			// ���斢�ݒ�
			lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
			return FALSE;
		}
		// ����ݒ�
		To = tpMailItem->To;
		Cc = tpMailItem->Cc;
		Bcc = tpMailItem->Bcc;
		// �������Ăɑ��M
		MyBcc = NULL;
		if (send_mail_box->MyAddr2Bcc == 1) {
			MyBcc = (send_mail_box->BccAddr != NULL && *send_mail_box->BccAddr != TEXT('\0')) ?
				send_mail_box->BccAddr : send_mail_box->MailAddress;
		}
		MailFlag = SMTP_RCPTTO;
		return send_mail_proc(hWnd, soc, NULL, ErrStr, tpMailItem, ShowFlag);

	case SMTP_RCPTTO:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RCPTTO);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}

		// To �Ɏw�肳�ꂽ���[���A�h���X�̑��M
		if (To != NULL && *To != TEXT('\0')) {
			if ((To = send_rcpt_to(hWnd, soc, To, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}
		// Cc �Ɏw�肳�ꂽ���[���A�h���X�̑��M
		if (Cc != NULL && *Cc != TEXT('\0')) {
			if ((Cc = send_rcpt_to(hWnd, soc, Cc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}
		// Bcc �Ɏw�肳�ꂽ���[���A�h���X�̑��M
		if (Bcc != NULL && *Bcc != TEXT('\0')) {
			if ((Bcc = send_rcpt_to(hWnd, soc, Bcc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}
		// ��������
		if (MyBcc != NULL && *MyBcc != TEXT('\0')) {
			if ((MyBcc = send_rcpt_to(hWnd, soc, MyBcc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}
		// DATA���M
		SetSocStatusTextT(hWnd, TEXT(CMD_DATA));
		if (send_buf(soc, CMD_DATA"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		MailFlag = SMTP_DATA;
		break;

	case SMTP_DATA:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_DATA);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		// ���[���f�[�^�̑��M
		SetSocStatusTextT(hWnd, STR_STATUS_SENDBODY);
		SwitchCursor(FALSE);
		if (send_mail_data(hWnd, soc, tpMailItem, ErrStr) == FALSE) {
			SwitchCursor(TRUE);
			return FALSE;
		}
		SwitchCursor(TRUE);
#ifdef WSAASYNC
		MailFlag = SMTP_SENDBODY;
		break;

	case SMTP_SENDBODY:
		break;
#else
		MailFlag = send_end_cmd;
		break;
#endif

	case SMTP_NEXTSEND:
		// ���̑��M���[��
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILSEND);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		tpMailItem->Status = tpMailItem->MailStatus = ICON_SENDMAIL;
		if (ShowFlag == TRUE) {
			hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
			i = ListView_GetMemToItem(hListView, tpMailItem);
			if (i != -1) {
				ListView_RedrawItems(hListView, i, i);
				UpdateWindow(hListView);
			}
		}
		if (op.AutoSave == 1) {
			// ���M�����t�@�C���ɕۑ�
			SaveMail(SENDBOX_FILE, MailBox + MAILBOX_SEND, 2);
		}

		// ���̑��M���[���̎擾
		i = Item_GetNextMailBoxSendItem((MailBox + MAILBOX_SEND), -1, GetNameToMailBox(send_mail_box->Name));
		if (i == -1) {
			// ���[�����M�I��
			SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
			if (send_buf(soc, CMD_QUIT"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			MailFlag = SMTP_QUIT;
			break;
		}
		if (send_mail_item == *((MailBox + MAILBOX_SEND)->tpMailItem + i)) {
			lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
			return FALSE;
		}
		send_mail_item = *((MailBox + MAILBOX_SEND)->tpMailItem + i);
		if ((send_mail_item->To == NULL || *send_mail_item->To == TEXT('\0')) &&
			(send_mail_item->Cc == NULL || *send_mail_item->Cc == TEXT('\0')) &&
			(send_mail_item->Bcc == NULL || *send_mail_item->Bcc == TEXT('\0'))) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
			return FALSE;
		}
		// ������
		SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
		if (send_buf(soc, CMD_RSET"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		MailFlag = SMTP_RSET;
		break;

	case SMTP_SENDEND:
		// ���M�I��
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILSEND);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
		if (send_buf(soc, CMD_QUIT"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		tpMailItem->Status = tpMailItem->MailStatus = ICON_SENDMAIL;
		if (ShowFlag == TRUE) {
			hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
			i = ListView_GetMemToItem(hListView, tpMailItem);
			if (i != -1) {
				ListView_RedrawItems(hListView, i, i);
				UpdateWindow(hListView);
			}
		}
		if (op.AutoSave == 1) {
			// ���M�����t�@�C���ɕۑ�
			SaveMail(SENDBOX_FILE, MailBox + MAILBOX_SEND, 2);
		}
		MailFlag = SMTP_QUIT;
		break;

	case SMTP_QUIT:
		break;
	}
	return TRUE;
}

/*
 * smtp_send_proc - �f�[�^���M�v���V�[�W��
 */
#ifdef WSAASYNC
BOOL smtp_send_proc(HWND hWnd, SOCKET soc, TCHAR *ErrStr, MAILBOX *tpMailBox)
{
	TCHAR buf[SEND_BUFSIZE];
	int len;

	if (body == NULL && send_pt != NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	// ���M�I��
	if (body == NULL || *send_pt == TEXT('\0')) {
		if (send_data(soc, TEXT(CMD_DATA_END)) == -1) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				return TRUE;
			}
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&body);
		body = NULL;
		MailFlag = send_end_cmd;
		if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
			mem_free(&body);
			body = NULL;
			lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
			return FALSE;
		}
		return TRUE;
	}

	// �������đ��M
	TStrCpyN(buf, send_pt, SEND_BUFSIZE - 1);
	if ((len = send_data(soc, buf)) == -1) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return TRUE;
		}
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	send_pt += len;
	send_len += len;
	SetStatusRecvLen(hWnd, send_len, STR_STATUS_SOCKINFO_SEND);
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
	return TRUE;
}
#endif

/*
 * smtp_proc - SMTP�v���V�[�W��
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
		smtp_set_error(hWnd);
	}
	return ret;
}

/*
 * smtp_send_mail - ���[�����M�̏�����
 */
SOCKET smtp_send_mail(HWND hWnd, MAILBOX *tpMailBox, MAILITEM *tpMailItem, int end_cmd, TCHAR *ErrStr)
{
	SOCKET soc;

	send_mail_box = tpMailBox;
	send_mail_item = tpMailItem;
	auth_flag = FALSE;
	starttls_flag = FALSE;
	send_end_cmd = end_cmd;

	if ((send_mail_item->To == NULL || *send_mail_item->To == TEXT('\0')) &&
		(send_mail_item->Cc == NULL || *send_mail_item->Cc == TEXT('\0')) &&
		(send_mail_item->Bcc == NULL || *send_mail_item->Bcc == TEXT('\0'))) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
		return -1;
	}

	// �z�X�g������IP�A�h���X���擾 (�擾����IP�͕ۑ�)
	if (send_mail_box->SmtpIP == 0 || op.IPCache == 0) {
		GetHostFlag = TRUE;
		send_mail_box->SmtpIP = get_host_by_name(hWnd, send_mail_box->SmtpServer, ErrStr);
		GetHostFlag = FALSE;
		if (send_mail_box->SmtpIP == 0) {
			return -1;
		}
	}
	// �ڑ��J�n
	soc = connect_server(hWnd,
		send_mail_box->SmtpIP, (unsigned short)send_mail_box->SmtpPort,
		(send_mail_box->SmtpSSL == 0 || send_mail_box->SmtpSSLInfo.Type == 4) ? -1 : send_mail_box->SmtpSSLInfo.Type,
		&send_mail_box->SmtpSSLInfo,
		ErrStr);
	if (soc == -1) {
		return -1;
	}
	return soc;
}

/*
 * smtp_set_error - ���M���[���ɃG���[�X�e�[�^�X��t������
 */
void smtp_set_error(HWND hWnd)
{
	HWND hListView;
	int i;

	mem_free(&body);
	body = NULL;
	auth_flag = FALSE;
	starttls_flag = FALSE;

	if (send_mail_item == NULL) {
		return;
	}
	send_mail_item->Status = send_mail_item->MailStatus = ICON_ERROR;

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
/* End of source */
