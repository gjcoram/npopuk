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
#define SEND_BUFSIZE		4096				// 送信時の分割サイズ

#define SMTP_ERRORSTATUS	400					// SMTPエラーレスポンス

#define MIMEVER				"1.0"				// MIMEバージョン

#define AUTH_CRAM_MD5		0					// SMTP-AUTH の認証タイプ
#define AUTH_LOGIN			1
#define AUTH_PLAIN			2
#define AUTH_NON			99

/* Global Variables */
static MAILBOX *tpSendMailBox;					// 送信中のメールボックス
static MAILITEM *tpSendMailItem;				// 送信メール
static BOOL SmtpAuthFlag;						// SMTP-AUTH
static BOOL SmtpSTARTTLS;						// STARTTLS
static TCHAR *body, *SendPoint;					// 送信用本文
static int SendLen;								// 送信長
static int EndFlag;								// メール送信完了時のコマンドフラグ

// 外部参照
extern OPTION op;

extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern int SelBox;

extern SOCKET g_soc;
extern BOOL GetHostFlag;
extern int ssl_type;

extern int MailFlag;
extern TCHAR *g_Pass;

/* Local Function Prototypes */
static TCHAR *CreateAuthString(char *buf, TCHAR *User, TCHAR *Pass, TCHAR *ErrStr);
static TCHAR *CreatePlainString(TCHAR *User, TCHAR *Pass, TCHAR *ErrStr);
static BOOL GetAuthType(char *buf, int *type);
static TCHAR *GetPassword(MAILBOX *tpMailBox);
static int GetResponseStat(char *buf);
static BOOL SendHeader(SOCKET soc, TCHAR *Head, TCHAR *Sendbuf, TCHAR *ErrStr);
static BOOL SendConvHeader(SOCKET soc, TCHAR *Head, TCHAR *Sendbuf, BOOL Address, TCHAR *ErrStr);
static BOOL SendMailString(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr);
static BOOL SendMailAddr(HWND hWnd, SOCKET soc, TCHAR *str, TCHAR *MailAddress, TCHAR *ErrStr);
static TCHAR *SendRcptTo(HWND hWnd, SOCKET soc, TCHAR *To, TCHAR *ErrStr);
static BOOL SendMailProc(HWND hWnd, SOCKET soc, char *buf, TCHAR *ErrStr,
							  MAILITEM *tpMailItem, BOOL ShowFlag);

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

	// MD5(key XOR opad, MD5(key XOR ipad, input)) の計算を行う
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
 * CreateAuthString - AUTH CRAM-MD5 のパスワードを生成する (RFC 2554)
 */
static TCHAR *CreateAuthString(char *buf, TCHAR *User, TCHAR *Pass, TCHAR *ErrStr)
{
	TCHAR *ret;
	char *input, *key;
	char *p;
	unsigned char digest[16];
	int len;
	int i;

#ifdef UNICODE
	key = AllocTcharToChar(Pass);
	if (key == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
#else
	key = Pass;
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

	// ダイジェスト値の取得
	HMAC_MD5(input, tstrlen(input), key, tstrlen(key), digest);
#ifdef UNICODE
	mem_free(&key);
#endif
	mem_free(&input);

	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT(" \r\n")) + lstrlen(User) + (16 * 2) + 1));
	if (ret == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	len = wsprintf(ret, TEXT("%s "), User);
	for (i = 0; i < 16; i++, len += 2) {
		wsprintf(ret + len, TEXT("%02x"), digest[i]);
	}
	return ret;
}

/*
 * CreatePlainString - AUTH PLAIN の文字列を生成する (RFC 2554)
 */
static TCHAR *CreatePlainString(TCHAR *User, TCHAR *Pass, TCHAR *ErrStr)
{
#ifdef UNICODE
	TCHAR *ret;
#endif
	char *c_user, *c_pass;
	char *buf, *tmp;
	int len;

#ifdef UNICODE
	c_user = AllocTcharToChar(User);
	if (c_user == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	c_pass = AllocTcharToChar(Pass);
	if (c_pass == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
#else
	c_user = User;
	c_pass = Pass;
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
 * GetAuthType - 利用可能な SMTP AUTH の種類を取得 (RFC 2554)
 */
static BOOL GetAuthType(char *buf, int *type)
{
	char *p, *r;

	for (p = buf; *p != '\0' && *p != ' ' && *p != '-'; p++);
	for (; *p == ' ' || *p == '-'; p++);
	if (*p == '\0') {
		return FALSE;
	}
	// AUTH
	if (StrCmpNI(p, "AUTH", 4) != 0) {
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
		if (StrCmpNI(r, "CRAM-MD5", 8) == 0) {
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
		if (StrCmpNI(r, "LOGIN", 5) == 0) {
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
		if (StrCmpNI(r, "PLAIN", 5) == 0) {
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
 * GetStartTLS - STARTTLSが利用可能かチェック
 */
static BOOL GetStartTLS(char *buf)
{
	char *p;

	for (p = buf; *p != '\0' && *p != ' ' && *p != '-'; p++);
	for (; *p == ' ' || *p == '-'; p++);
	if (*p == '\0') {
		return FALSE;
	}
	// STARTTLS
	if (StrCmpNI(p, "STARTTLS", 8) == 0) {
		return TRUE;
	}
	return FALSE;
}

/*
 * GetPassword - パスワードの取得
 */
static TCHAR *GetPassword(MAILBOX *tpMailBox)
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
 * GetResponseStat - SMTPレスポンスの解析
 */
static int GetResponseStat(char *buf)
{
	char stat[5];
	int ret;

	// 複数行の場合は -1 を返す
	if (tstrlen(buf) > 3 && *(buf + 3) == '-') {
		return -1;
	}

	// ステータスコードを取得
	StrCpyN(stat, buf, 4);
	ret = a2i(stat);
	if (ret <= 0) {
		ret = SMTP_ERRORSTATUS;
	}
	return ret;
}

/*
 * SendHeader - ヘッダ項目の送信
 */
static BOOL SendHeader(SOCKET soc, TCHAR *Head, TCHAR *Sendbuf, TCHAR *ErrStr)
{
	TCHAR *buf;

	if (Sendbuf == NULL || *Sendbuf == TEXT('\0')) {
		return TRUE;
	}

	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Head) + 1 + lstrlen(Sendbuf) + 2 + 1));
	if (buf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}

	TStrJoin(buf, Head, TEXT(" "), Sendbuf, TEXT("\r\n"), (TCHAR *)-1);

	if (TSendBuf(soc, buf) == -1) {
		mem_free(&buf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&buf);
	return TRUE;
}

/*
 * SendConvHeader - ヘッダ項目をMIMEエンコードして送信
 */
static BOOL SendConvHeader(SOCKET soc, TCHAR *Head, TCHAR *Sendbuf, BOOL Address, TCHAR *ErrStr)
{
	TCHAR *p;
	BOOL ret;

	if (Sendbuf == NULL || *Sendbuf == TEXT('\0')) {
		return TRUE;
	}

	p = MIMEencode(Sendbuf, Address);
	if (p == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	ret = SendHeader(soc, Head, p, ErrStr);
	mem_free(&p);
	return ret;
}

/*
 * SendMailString - ヘッダと本文の送信 (RFC 822, RFC 2822, RFC 2045, RFC 2076)
 */
static BOOL SendMailString(HWND hWnd, SOCKET soc, MAILITEM *tpMailItem, TCHAR *ErrStr)
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
	if (tpSendMailBox->MailAddress != NULL && *tpSendMailBox->MailAddress != TEXT('\0')) {
		len = lstrlen(TEXT(" <>"));
		p = NULL;
		// ユーザ名の設定
		if (tpSendMailBox->UserName != NULL) {
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(tpSendMailBox->UserName) + 1));
			if (p != NULL) {
				SetUserName(tpSendMailBox->UserName, p);
				len += lstrlen(p);
			}
		}
		len += lstrlen(tpSendMailBox->MailAddress);

		// Fromの作成と送信
		tpMailItem->From = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (tpMailItem->From != NULL) {
			r = tpMailItem->From;
			if (p != NULL && *p != TEXT('\0')) {
				r = TStrJoin(r, p, TEXT(" "), (TCHAR *)-1);
			}

			TStrJoin(r, TEXT("<"), tpSendMailBox->MailAddress, TEXT(">"), (TCHAR *)-1);
			if (SendConvHeader(soc, TEXT(HEAD_FROM), tpMailItem->From, TRUE, ErrStr) == FALSE) {
				return FALSE;
			}
		}
		mem_free(&p);
	}

	// To
	if (SendConvHeader(soc, TEXT(HEAD_TO), tpMailItem->To, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}

	// Cc
	if (SendConvHeader(soc, TEXT(HEAD_CC), tpMailItem->Cc, TRUE, ErrStr) == FALSE) {
		return FALSE;
	}

	// Date
	GetTimeString(buf);
	if (op.SendDate == 1 && SendHeader(soc, TEXT(HEAD_DATE), buf, ErrStr) == FALSE) {
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
	if (SendConvHeader(soc, TEXT(HEAD_SUBJECT), tpMailItem->Subject, FALSE, ErrStr) == FALSE) {
		return FALSE;
	}

	// Reply-To
	if (tpMailItem->ReplyTo != NULL && *tpMailItem->ReplyTo != TEXT('\0')) {
		// メールに設定されている Reply-To
		if (SendConvHeader(soc, TEXT(HEAD_REPLYTO), tpMailItem->ReplyTo, TRUE, ErrStr) == FALSE) {
			return FALSE;
		}
	} else {
		// メールボックスに設定されている Reply-To
		if (SendConvHeader(soc, TEXT(HEAD_REPLYTO), tpSendMailBox->ReplyTo, TRUE, ErrStr) == FALSE) {
			return FALSE;
		}
	}

	// In-Reply-To
	if (SendHeader(soc, TEXT(HEAD_INREPLYTO), tpMailItem->InReplyTo, ErrStr) == FALSE) {
		return FALSE;
	}
	// References
	if (SendHeader(soc, TEXT(HEAD_REFERENCES), tpMailItem->References, ErrStr) == FALSE) {
		return FALSE;
	}

	if (op.SendMessageId == 1) {
		// Message-Id
		mem_free(&tpMailItem->MessageID);
		tpMailItem->MessageID = CreateMessageId((long)tpMailItem, tpSendMailBox->MailAddress);
		if (tpMailItem->MessageID == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		if (SendHeader(soc, TEXT(HEAD_MESSAGEID), tpMailItem->MessageID, ErrStr) == FALSE) {
			return FALSE;
		}
	}

	// MIME-Version
	if (SendHeader(soc, TEXT(HEAD_MIMEVERSION), TEXT(MIMEVER), ErrStr) == FALSE) {
		return FALSE;
	}

	// 本文のエンコード
	body = BodyEncode(tpMailItem->Body, buf, enc_buf, ErrStr);
	if (body == NULL) {
		return FALSE;
	}

	// 添付ファイルがある場合はマルチパートで送信する
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
		if (SendHeader(soc, TEXT(HEAD_CONTENTTYPE), buf, ErrStr) == FALSE) {
			mem_free(&body);
			body = NULL;
			return FALSE;
		}
		// Content-Transfer-Encoding
		if (SendHeader(soc, TEXT(HEAD_ENCODING), enc_buf, ErrStr) == FALSE) {
			mem_free(&body);
			body = NULL;
			return FALSE;
		}
		break;

	case MP_ATTACH:
		mem_free(&body);
		body = mbody;
		// Content-Type
		if (SendHeader(soc, TEXT(HEAD_CONTENTTYPE), mctypr, ErrStr) == FALSE) {
			mem_free(&body);
			body = NULL;
			mem_free(&mctypr);
			return FALSE;
		}
		mem_free(&mctypr);
		break;
	}

	// X-Mailer
	if (SendHeader(soc, TEXT(HEAD_X_MAILER), APP_NAME, ErrStr) == FALSE) {
		mem_free(&body);
		body = NULL;
		return FALSE;
	}

	if (TSendBuf(soc, TEXT("\r\n")) == -1) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}

	SendPoint = body;
	SendLen = 0;

#ifdef WSAASYNC
	// 送信のイベントを作成
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
#else
	// 本文送信
	if (body != NULL && TSendBuf(soc, body) == -1) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&body);
	body = NULL;

	if (TSendBuf(soc, TEXT("\r\n.\r\n")) == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
#endif
	return TRUE;
}

/*
 * SendMailAddr - メールアドレスを送信する
 */
static BOOL SendMailAddr(HWND hWnd, SOCKET soc, TCHAR *str, TCHAR *MailAddress, TCHAR *ErrStr)
{
	TCHAR *wbuf;

	wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT(":<>\r\n")) + lstrlen(str) + lstrlen(MailAddress) + 1));
	if (wbuf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return FALSE;
	}
	// RCPT TO:<送信先メールアドレス>\r\n
	TStrJoin(wbuf, str, TEXT(":<"), MailAddress, TEXT(">\r\n"), (TCHAR *)-1);

	SetSocStatusTextT(hWnd, wbuf, 1);
	if (TSendBuf(soc, wbuf) == -1) {
		mem_free(&wbuf);
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	mem_free(&wbuf);
	return TRUE;
}

/*
 * SendRcptTo - RCPT TO を送信する
 */
static TCHAR *SendRcptTo(HWND hWnd, SOCKET soc, TCHAR *To, TCHAR *ErrStr)
{
	TCHAR *p;

	// メールアドレスの抽出
	p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(To) + 1));
	if (p == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return (TCHAR *)-1;
	}
	To = GetMailAddress(To, p, FALSE);

	// メールアドレスの送信
	if (SendMailAddr(hWnd, soc, TEXT("RCPT TO"), p, ErrStr) == FALSE) {
		mem_free(&p);
		return (TCHAR *)-1;
	}
	mem_free(&p);
	return ((*To != TEXT('\0')) ? To + 1 : To);
}

/*
 * SendMailProc - SMTPの応答プロシージャ (RFC 821, RFC 2821)
 */
static BOOL SendMailProc(HWND hWnd, SOCKET soc, char *buf, TCHAR *ErrStr,
							  MAILITEM *tpMailItem, BOOL ShowFlag)
{
	HWND hListView;
	TCHAR *wbuf;
	TCHAR *user;
	TCHAR *pass;
	TCHAR *p, *r;
	static TCHAR *To, *Cc, *Bcc, *MyBcc;
	static int AuthType;
	int status = 0;
	int i;

	if (buf != NULL) {
		SetStatusText(hWnd, buf);
		if (SmtpAuthFlag == TRUE && MailFlag == SMTP_EHLO) {
			GetAuthType(buf, &AuthType);
		}
		if (MailFlag == SMTP_EHLO) {
			if (GetStartTLS(buf) == TRUE) {
				SmtpSTARTTLS = TRUE;
			}
		}
		if ((status = GetResponseStat(buf)) == -1) {
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

		// メールアドレスのチェック
		if (tpSendMailBox->MailAddress == NULL || *tpSendMailBox->MailAddress == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_BADFROM);
			return FALSE;
		}

		// ドメインの抽出
		if (op.SendHelo == NULL || *op.SendHelo == TEXT('\0')) {
			p = tpSendMailBox->MailAddress;

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
		r = TEXT("HELO ");
		MailFlag = SMTP_HELO;
		if (tpSendMailBox->SmtpSSL == 1 && tpSendMailBox->SmtpSSLInfo.Type == 4) {
			r = TEXT("EHLO ");
			MailFlag = SMTP_EHLO;
		}
		if (SmtpAuthFlag == FALSE && tpSendMailBox->SmtpAuth == 1) {
			SmtpAuthFlag = TRUE;
			AuthType = AUTH_NON;
			r = TEXT("EHLO ");
			MailFlag = SMTP_EHLO;
		} else {
			SmtpAuthFlag = FALSE;
			if (op.ESMTP != 0) {
				r = TEXT("EHLO ");
				MailFlag = SMTP_EHLO;
			}
		}
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(r) + lstrlen(p) + lstrlen(TEXT("\r\n")) + 1));
		if (wbuf == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}

		// HELO ドメイン\r\n
		TStrJoin(wbuf, r, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, wbuf, 1);
		if (TSendBuf(soc, wbuf) == -1) {
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&wbuf);
		break;

	case SMTP_EHLO:
		if (status >= SMTP_ERRORSTATUS) {
			// EHLO に失敗した場合は HELO を送信し直す
			MailFlag = SMTP_START;
			return SendMailProc(hWnd, soc, NULL, ErrStr, tpMailItem, ShowFlag);
		}
		if (SmtpSTARTTLS == TRUE && tpSendMailBox->SmtpSSL == 1 && tpSendMailBox->SmtpSSLInfo.Type == 4) {
			// STARTTLS
			SetSocStatusTextT(hWnd, TEXT("STARTTLS"), 1);
			if (TSendBuf(soc, TEXT("STARTTLS\r\n")) == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			SmtpAuthFlag = FALSE;
			SmtpSTARTTLS = FALSE;
			MailFlag = SMTP_STARTTLS;
			break;
		}
		if (tpSendMailBox->SmtpAuth == 0) {
			// 初期化
			SetSocStatusTextT(hWnd, TEXT("RSET\r\n"), 1);
			if (TSendBuf(soc, TEXT("RSET\r\n")) == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			MailFlag = SMTP_RSET;
			break;
		}
		
		// 強制的に認証方式を設定
		if (tpSendMailBox->SmtpAuthType != 0) {
			AuthType = tpSendMailBox->SmtpAuthType - 1;
		}
		switch (AuthType) {
		case AUTH_CRAM_MD5:
		case AUTH_NON:
		default:
			// AUTH CRAM-MD5
			SetSocStatusTextT(hWnd, TEXT("AUTH CRAM-MD5"), 1);
			if (TSendBuf(soc, TEXT("AUTH CRAM-MD5\r\n")) == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			MailFlag = SMTP_AUTH;
			break;

		case AUTH_LOGIN:
			// AUTH LOGIN
			SetSocStatusTextT(hWnd, TEXT("AUTH LOGIN"), 1);
			if (TSendBuf(soc, TEXT("AUTH LOGIN\r\n")) == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			MailFlag = SMTP_AUTHLOGIN;
			break;

		case AUTH_PLAIN:
			// AUTH PLAIN
			user = (tpSendMailBox->AuthUserPass == 1) ? tpSendMailBox->SmtpUser : tpSendMailBox->User;
			pass = GetPassword(tpSendMailBox);
			if (pass == NULL || *pass == TEXT('\0')) {
				lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
				return FALSE;
			}

			p = CreatePlainString(user, pass, ErrStr);
			if (p == NULL) {
				return FALSE;
			}

			wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("AUTH PLAIN \r\n")) + lstrlen(p) + 1));
			if (wbuf == NULL) {
				mem_free(&p);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}

			// AUTH PLAIN Base64(<NULL>User<NULL>Pass)[CRLF] を送信する
			TStrJoin(wbuf, TEXT("AUTH PLAIN "), p, TEXT("\r\n"), (TCHAR *)-1);
			SetSocStatusTextT(hWnd, TEXT("AUTH PLAIN ****"), 1);
			if (TSendBuf(soc, wbuf) == -1) {
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

	case SMTP_AUTH:
		if (status >= SMTP_ERRORSTATUS) {
			SmtpAuthFlag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}

		// AUTH CRAM-MD5のパスワードを作成
		user = (tpSendMailBox->AuthUserPass == 1) ? tpSendMailBox->SmtpUser : tpSendMailBox->User;
		pass = GetPassword(tpSendMailBox);
		if (pass == NULL || *pass == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
			return FALSE;
		}
		wbuf = CreateAuthString(buf, user, pass, ErrStr);
		if (wbuf == NULL) {
			return FALSE;
		}

	case SMTP_AUTHLOGIN:
		if (status >= SMTP_ERRORSTATUS) {
			SmtpAuthFlag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		user = (tpSendMailBox->AuthUserPass == 1) ? tpSendMailBox->SmtpUser : tpSendMailBox->User;
		p = (TCHAR *)mem_calloc(sizeof(TCHAR) * (lstrlen(user) * 2 + 4));
		if (p == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		// Base64エンコード
		TBase64Encode(user, p, 0);

		// ユーザ名送信
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("\r\n")) + lstrlen(p) + 1));
		if (wbuf == NULL) {
			mem_free(&p);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		TStrJoin(wbuf, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, TEXT("Send user"), 1);
		if (TSendBuf(soc, wbuf) == -1) {
			mem_free(&p);
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&p);
		mem_free(&wbuf);
		MailFlag = SMTP_AUTHLOGIN_PASS;
		break;

	case SMTP_AUTHLOGIN_PASS:
		if (status >= SMTP_ERRORSTATUS) {
			SmtpAuthFlag = FALSE;
			lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		pass = GetPassword(tpSendMailBox);
		if (pass == NULL || *pass == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
			return FALSE;
		}
		p = (TCHAR *)mem_calloc(sizeof(TCHAR) * (lstrlen(pass) * 2 + 4));
		if (p == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		// Base64エンコード
		TBase64Encode(pass, p, 0);

		// パスワード送信
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT("\r\n")) + lstrlen(p) + 1));
		if (wbuf == NULL) {
			mem_free(&p);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
		TStrJoin(wbuf, p, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, TEXT("Send password"), 1);
		if (TSendBuf(soc, wbuf) == -1) {
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
			if (SmtpAuthFlag == FALSE) {
				lstrcpy(ErrStr, STR_ERR_SOCK_HELO);
			} else {
				SmtpAuthFlag = FALSE;
				lstrcpy(ErrStr, STR_ERR_SOCK_SMTPAUTH);
			}
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}

		// 初期化
		SetSocStatusTextT(hWnd, TEXT("RSET\r\n"), 1);
		if (TSendBuf(soc, TEXT("RSET\r\n")) == -1) {
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

		// 返信先メールアドレスの送信
		if (SendMailAddr(hWnd, soc, TEXT("MAIL FROM"), tpSendMailBox->MailAddress, ErrStr) == FALSE) {
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
			lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
			return FALSE;
		}
		// 宛先設定
		To = tpMailItem->To;
		Cc = tpMailItem->Cc;
		Bcc = tpMailItem->Bcc;
		// 自分宛てに送信
		MyBcc = NULL;
		if (tpSendMailBox->MyAddr2Bcc == 1) {
			MyBcc = (tpSendMailBox->BccAddr != NULL && *tpSendMailBox->BccAddr != TEXT('\0')) ?
				tpSendMailBox->BccAddr : tpSendMailBox->MailAddress;
		}
		MailFlag = SMTP_RCPTTO;
		return SendMailProc(hWnd, soc, NULL, ErrStr, tpMailItem, ShowFlag);

	case SMTP_RCPTTO:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RCPTTO);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}

		// To に指定されたメールアドレスの送信
		if (To != NULL && *To != TEXT('\0')) {
			if ((To = SendRcptTo(hWnd, soc, To, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}

		// Cc に指定されたメールアドレスの送信
		if (Cc != NULL && *Cc != TEXT('\0')) {
			if ((Cc = SendRcptTo(hWnd, soc, Cc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}

		// Bcc に指定されたメールアドレスの送信
		if (Bcc != NULL && *Bcc != TEXT('\0')) {
			if ((Bcc = SendRcptTo(hWnd, soc, Bcc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}

		// 自分宛て
		if (MyBcc != NULL && *MyBcc != TEXT('\0')) {
			if ((MyBcc = SendRcptTo(hWnd, soc, MyBcc, ErrStr)) == (TCHAR *)-1) {
				return FALSE;
			}
			MailFlag = SMTP_RCPTTO;
			break;
		}

		// DATA送信
		SetSocStatusTextT(hWnd, TEXT("DATA\r\n"), 1);
		if (TSendBuf(soc, TEXT("DATA\r\n")) == -1) {
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

		SetSocStatusTextT(hWnd, STR_STATUS_SENDBODY, 1);
		SwitchCursor(FALSE);
		if (SendMailString(hWnd, soc, tpMailItem, ErrStr) == FALSE) {
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
		MailFlag = EndFlag;
		break;
#endif

	case SMTP_NEXTSEND:
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
			// 送信箱をファイルに保存
			SaveMail(SENDBOX_FILE, MailBox + MAILBOX_SEND, 2);
		}

		// 次の送信メールの取得
		i = Item_GetNextMailBoxSendItem((MailBox + MAILBOX_SEND), -1, GetNameToMailBox(tpSendMailBox->Name));
		if (i == -1) {
			// メール送信終了
			SetSocStatusTextT(hWnd, TEXT(QUIT), 1);
			if (TSendBuf(soc, TEXT(QUIT)) == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			MailFlag = SMTP_QUIT;
			break;
		}
		if (tpSendMailItem == *((MailBox + MAILBOX_SEND)->tpMailItem + i)) {
			lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
			return FALSE;
		}
		tpSendMailItem = *((MailBox + MAILBOX_SEND)->tpMailItem + i);
		if ((tpSendMailItem->To == NULL || *tpSendMailItem->To == TEXT('\0')) &&
			(tpSendMailItem->Cc == NULL || *tpSendMailItem->Cc == TEXT('\0')) &&
			(tpSendMailItem->Bcc == NULL || *tpSendMailItem->Bcc == TEXT('\0'))) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
			return FALSE;
		}
		// 初期化
		SetSocStatusTextT(hWnd, TEXT("RSET\r\n"), 1);
		if (TSendBuf(soc, TEXT("RSET\r\n")) == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		MailFlag = SMTP_RSET;
		break;

	case SMTP_SENDEND:
		if (status >= SMTP_ERRORSTATUS) {
			lstrcpy(ErrStr, STR_ERR_SOCK_MAILSEND);
			StrCatN(ErrStr, buf, BUF_SIZE - 1);
			return FALSE;
		}
		SetSocStatusTextT(hWnd, TEXT(QUIT), 1);
		if (TSendBuf(soc, TEXT(QUIT)) == -1) {
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
			// 送信箱をファイルに保存
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
 * SmtpError - 送信メールにエラーステータスを付加する
 */
void SmtpError(HWND hWnd)
{
	HWND hListView;
	int i;

	mem_free(&body);
	body = NULL;
	SmtpAuthFlag = FALSE;
	SmtpSTARTTLS = FALSE;

	if (tpSendMailItem == NULL) {
		return;
	}
	tpSendMailItem->Status = tpSendMailItem->MailStatus = ICON_ERROR;

	if (SelBox == MAILBOX_SEND) {
		hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		i = ListView_GetMemToItem(hListView, tpSendMailItem);
		if (i != -1) {
			ListView_RedrawItems(hListView, i, i);
			UpdateWindow(hListView);
		}
	}
	tpSendMailItem = NULL;
}

/*
 * SmtpProc - メール送信プロシージャ
 */
BOOL SmtpProc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr,
							  MAILBOX *tpMailBox, BOOL ShowFlag)
{
	BOOL ret;

	if (tpSendMailItem == NULL) {
		return FALSE;
	}
	ret = SendMailProc(hWnd, soc, buf, ErrStr, tpSendMailItem, ShowFlag);
	if (ret == FALSE) {
		SendBuf(soc, QUIT);
		SmtpError(hWnd);
	}
	return ret;
}

/*
 * SendMailItem - １件の送信メールの初期化
 */
SOCKET SendMailItem(HWND hWnd, MAILBOX *tpMailBox, MAILITEM *tpMailItem, int EndMailFlag, TCHAR *ErrStr)
{
	SOCKET soc;

	tpSendMailBox = tpMailBox;
	tpSendMailItem = tpMailItem;
	SmtpAuthFlag = FALSE;
	SmtpSTARTTLS = FALSE;
	EndFlag = EndMailFlag;

	if ((tpSendMailItem->To == NULL || *tpSendMailItem->To == TEXT('\0')) &&
		(tpSendMailItem->Cc == NULL || *tpSendMailItem->Cc == TEXT('\0')) &&
		(tpSendMailItem->Bcc == NULL || *tpSendMailItem->Bcc == TEXT('\0'))) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOTO);
		return -1;
	}

	// ホスト名からIPアドレスを取得 (取得したIPは保存しておく)
	if (tpSendMailBox->SmtpIP == 0 || op.IPCache == 0) {
		GetHostFlag = TRUE;
		tpSendMailBox->SmtpIP = GetHostByName(hWnd, tpSendMailBox->SmtpServer, ErrStr);
		GetHostFlag = FALSE;
		if (tpSendMailBox->SmtpIP == 0) {
			return -1;
		}
	}
	// 接続開始
	soc = ConnectServer(hWnd,
		tpSendMailBox->SmtpIP, (unsigned short)tpSendMailBox->SmtpPort,
		(tpSendMailBox->SmtpSSL == 0 || tpSendMailBox->SmtpSSLInfo.Type == 4) ? -1 : tpSendMailBox->SmtpSSLInfo.Type,
		&tpSendMailBox->SmtpSSLInfo,
		ErrStr);
	if (soc == -1) {
		return -1;
	}
	return soc;
}

/*
 * SendProc - 送信プロシージャ
 */
#ifdef WSAASYNC
BOOL SendProc(HWND hWnd, SOCKET soc, TCHAR *ErrStr, MAILBOX *tpMailBox)
{
	TCHAR buf[SEND_BUFSIZE];
	int len;

	if (body == NULL && SendPoint != NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}

	// 送信終了
	if (body == NULL || *SendPoint == TEXT('\0')) {
		if (Tsend(soc, TEXT("\r\n.\r\n")) == -1) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				return TRUE;
			}
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		mem_free(&body);
		body = NULL;
		MailFlag = EndFlag;
		if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
			mem_free(&body);
			body = NULL;
			lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
			return FALSE;
		}
		return TRUE;
	}

	// 分割して送信
	TStrCpyN(buf, SendPoint, SEND_BUFSIZE - 1);
	if ((len = Tsend(soc, buf)) == -1) {
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return TRUE;
		}
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}

	SendPoint += len;
	SendLen += len;
	SetStatusRecvLen(hWnd, SendLen, STR_STATUS_SOCKINFO_SEND);
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
		mem_free(&body);
		body = NULL;
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return FALSE;
	}
	return TRUE;
}
#endif
/* End of source */
