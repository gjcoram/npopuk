/*
 * nPOP
 *
 * WinSock.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2009 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#if !defined(_WIN32_WCE) && defined( _MSC_VER) && (_MSC_VER <= 1200)
typedef void *HCERTSTORE;
extern int _stricmp(const char *, const char *);
#undef __WINCRYPT_H__
#include "wincrypt_stub.h"
#else
#include <wincrypt.h>
#endif
#include "openssl.h"

/* Define */
#define CRLF_LEN				2
#define MAX_DEPTH				9

#ifdef _WCE_OLD
#define RECV_SIZE				4096		// 受信バッファサイズ
#else
#define RECV_SIZE				32768		// 受信バッファサイズ
#endif
#define SEND_SIZE				4096		// 送信時の分割サイズ

typedef struct _OPENSSL_INFO {
	SSL_CTX *ctx;
	SSL *ssl;
	int depth;
} OPENSSL_INFO;

/* Global Variables */
extern TCHAR *AppDir;
static char *recv_buf;						// 受信バッファ
static char *old_buf;						// 未処理バッファ
static int old_buf_len;
static int old_buf_size;
static HINSTANCE crypt32_lib = NULL;
static FARPROC _CertOpenStore;
static FARPROC _CertFindCertificateInStore;
static FARPROC _CertCloseStore;

int ssl_type = -1;
static int ssl_idx = 0;
static OPENSSL_INFO *ssl = NULL;
static SSL_INFO ssl_info;

static OPENSSL_INFO *ssl_init(const int soc, const int ssl_type, int verify, int depth,
	char *ca_file, char *ca_path, char *cert_file, char *key_file, char *pass,
	char *err_str);

#ifdef _WIN32_WCE_PPC
// bj: Use built-in device SSL on PPC
#include <sslsock.h>

// Dummy SSL Certificate checker - we're not interested if Server cert. is valid!
int CALLBACK SSLValidateCertHook(DWORD dwType, LPVOID pvArg, DWORD dwChainLen, LPBLOB pCertChain, DWORD dwFlags)
{ 
    return SSL_ERR_OKAY; 
} 
#endif


// 外部参照
extern OPTION op;

/* Local Function Prototypes */


/*
 * get_host_by_name - ホスト名からIPアドレスを取得する
 */
unsigned long get_host_by_name(HWND hWnd, TCHAR *server, TCHAR *ErrStr)
{
	unsigned long ret;
	LPHOSTENT lpHostEnt;
	char *HostName;

	SetSocStatusTextT(hWnd, STR_STATUS_GETHOSTBYNAME);
	if (server == NULL || *server == TEXT('\0')) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOSERVER);
		return 0;
	}
#ifdef UNICODE
	HostName = alloc_tchar_to_char(server);
	if (HostName == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return 0;
	}
#else
	HostName = server;
#endif
	ret = inet_addr(HostName);
	while (ret == -1) {
		lpHostEnt = gethostbyname(HostName);
		if (lpHostEnt != NULL) {
			ret = ((struct in_addr *)lpHostEnt->h_addr_list[0])->s_addr;
		} else {
			int err = WSAGetLastError();
#ifdef _DEBUG
			if (op.SocLog > 1) {
				wsprintf(ErrStr, TEXT("%s, errno=%d\r\n"), STR_ERR_SOCK_GETIPADDR, err);
				log_save(ErrStr);
			}
#endif
			if (err != WSATRY_AGAIN) {
				lstrcpy(ErrStr, STR_ERR_SOCK_GETIPADDR);
				ret = 0;
			}
		}
	}
#ifdef UNICODE
	mem_free(&HostName);
#endif
	return ret;
}

/*
 * connect_server - サーバに接続する
 */
SOCKET connect_server(HWND hWnd, unsigned long ip_addr, unsigned short port, const int ssl_tp, const SSL_INFO *si, TCHAR *ErrStr)
{
	SOCKET	soc;
	struct	sockaddr_in serversockaddr;
	BOOL	bSSL_Local = FALSE;

	SetSocStatusTextT(hWnd, STR_STATUS_CONNECT);

	// SSL情報
	ssl_info = *si;
	ssl_type = ssl_tp;
	ssl = NULL;
	// ソケットの作成
	soc = socket(PF_INET, SOCK_STREAM, 0);
	if (soc == INVALID_SOCKET) {
		lstrcpy(ErrStr, STR_ERR_SOCK_CREATESOCKET);
		return -1;
	}

#ifdef _WIN32_WCE_PPC
	// Option to use Windows native SSL
	if (ssl_tp >= 0 && op.UseWindowsSSL == 1)
	{
		SSLVALIDATECERTHOOK hook;
		int		err;
		DWORD	dwFlags, optval = SO_SEC_SSL;

		bSSL_Local = TRUE;

		err = setsockopt(soc, SOL_SOCKET, SO_SECURE, (const char *)&optval, sizeof(optval)); 
		if (err==SOCKET_ERROR)
		{ 
			lstrcpy(ErrStr, STR_ERR_SOCK_CREATESOCKET);
			return -1;
		}

		hook.HookFunc = SSLValidateCertHook;
		hook.pvArg = (PVOID) soc;

		// Set the certificate validation callback. 
		if(WSAIoctl(soc, SO_SSL_SET_VALIDATE_CERT_HOOK, &hook, sizeof(SSLVALIDATECERTHOOK),
			NULL, 0, NULL, NULL, NULL))
		{
			lstrcpy(ErrStr, TEXT("SO_SSL_SET_VALIDATE_CERT_HOOK failed!"));
			return -1;
		}

		// Select deferred handshake mode. Security protocols will not be negotiated until 
		// the SO_SSL_PERFORM_HANDSHAKE ioctl is issued
		dwFlags = SSL_FLAG_DEFER_HANDSHAKE;
		if(WSAIoctl(soc, SO_SSL_SET_FLAGS, &dwFlags, sizeof(DWORD), NULL, 0, NULL, NULL, NULL))
		{
			lstrcpy(ErrStr, TEXT("SO_SSL_SET_FLAGS failed!"));
			return -1;
		}
	}
#endif

	// 接続先の設定
	serversockaddr.sin_family = AF_INET;
	serversockaddr.sin_addr.s_addr = ip_addr;
	serversockaddr.sin_port = htons((unsigned short)port);
	ZeroMemory(serversockaddr.sin_zero, sizeof(serversockaddr.sin_zero));
#ifdef WSAASYNC
	if (WSAAsyncSelect(soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return -1;
	}
#endif
	// 接続
	if (connect(soc, (struct sockaddr *)&serversockaddr, sizeof(serversockaddr)) == SOCKET_ERROR) {
#ifdef WSAASYNC
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return soc;
		}
#endif
		lstrcpy(ErrStr, STR_ERR_SOCK_CONNECT);
		return -1;
	}
#ifndef WSAASYNC
	// SSLの初期化
#ifdef _WIN32_WCE_PPC
	if (bSSL_Local)
	{
		// Negotiate security protocols 
        if(WSAIoctl(soc, SO_SSL_PERFORM_HANDSHAKE, NULL, 0, NULL, 0, NULL, NULL, NULL)) 
        { 
			lstrcpy(ErrStr, TEXT("WSAIoctl(SO_SSL_PERFORM_HANDSHAKE) failed!"));
			return -1;
        } 
	}
	else if (init_ssl(hWnd, soc, ErrStr) == -1) {
		return -1;
#else
	if (init_ssl(hWnd, soc, ErrStr) == -1) {
		return -1;
#endif
	}
#endif
	return soc;
}

/*
 * recv_proc - 受信して改行単位で処理する
 */
int recv_proc(HWND hWnd, SOCKET soc)
{
	char *buf;
	char *line;
	char *p;
	int buf_len;
	int len;

	// 受信用バッファの確保
	if (recv_buf == NULL && (recv_buf = (char *)mem_alloc(RECV_SIZE+1)) == NULL) {
		return SELECT_MEM_ERROR;
	}
	// 受信
	if (ssl_type == -1) {
		buf_len = recv(soc, recv_buf, RECV_SIZE, 0);
	} else if (ssl) {
		buf_len = SSL_read(ssl->ssl, recv_buf, RECV_SIZE);
	} else {
		buf_len = SOCKET_ERROR;
	}
	if (buf_len == SOCKET_ERROR || buf_len == 0) {
#ifdef WSAASYNC
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return SELECT_SOC_SUCCEED;
		}
#endif
		return SELECT_SOC_CLOSE;
	}
	buf = recv_buf;
	// 未処理文字列と受信文字列を結合
	if (old_buf_len > 0 && old_buf != NULL) {
		if (old_buf_size < old_buf_len + buf_len) {
			old_buf_size += buf_len;
			buf = (char *)mem_alloc(old_buf_size);
			if (buf == NULL) {
				return SELECT_MEM_ERROR;
			}
			CopyMemory(buf, old_buf, old_buf_len);
			CopyMemory(buf + old_buf_len, recv_buf, buf_len);
			mem_free(&old_buf);
			old_buf = buf;
		} else {
			buf = old_buf;
			CopyMemory(old_buf + old_buf_len, recv_buf, buf_len);
		}
		buf_len += old_buf_len;
	}
	// 行単位に処理
	p = buf;
	while (1) {
		// 一行抽出
		line = p;
		for (len = 0; (p - buf) < buf_len; p++, len++) {
			if (*p == '\r' && (p - buf) + 1 < buf_len && *(p + 1) == '\n') {
				break;
			}
		}
		if ((p - buf) >= buf_len) {
			break;
		}
		*p = '\0';
		p += CRLF_LEN;
		// ウィンドウに文字列を渡す
		if (SendMessage(hWnd, WM_SOCK_RECV, len, (LPARAM)line) == FALSE) {
			return SELECT_SOC_SUCCEED;
		}
	}
	// 未処理の文字列を保存
	old_buf_len = len;
	if (old_buf_len > 0) {
		if (old_buf == buf) {
			MoveMemory(old_buf, line, len);
		} else if (old_buf != NULL && old_buf_size >= len) {
			CopyMemory(old_buf, line, len);
		} else {
			old_buf_size += len;
			buf = (char *)mem_alloc(old_buf_size);
			if (buf == NULL) {
				return SELECT_MEM_ERROR;
			}
			CopyMemory(buf, line, len);
			mem_free(&old_buf);
			old_buf = buf;
		}
	}
	return SELECT_SOC_SUCCEED;
}

/*
 * recv_select - 受信可能なバッファがある場合に受信を行う
 */
#ifndef WSAASYNC
int recv_select(HWND hWnd, SOCKET soc)
{
#define TIMEOUT			0		// タイムアウト秒数
	struct timeval waittime;
	fd_set rdps;
	int selret;

	waittime.tv_sec = TIMEOUT;
	waittime.tv_usec = 0;

	ZeroMemory(&rdps, sizeof(fd_set));
	FD_ZERO(&rdps);
	FD_SET(soc, &rdps);
	selret = select(FD_SETSIZE, &rdps, (fd_set *)0, (fd_set *)0, &waittime);
	if (selret == SOCKET_ERROR) {
		return SELECT_SOC_ERROR;
	}
	if (selret == 0 || FD_ISSET(soc, &rdps) == FALSE) {
		return SELECT_SOC_NODATA;
	}
	return recv_proc(hWnd, soc);
}
#endif

/*
 * send_data - 文字列の送信
 */
int send_data(SOCKET soc, char *wbuf, int len)
{
	// データ送信
	if (ssl_type == -1) {
		return send(soc, wbuf, len, 0);
	} else if (ssl) {
		return SSL_write(ssl->ssl, wbuf, len);
	} else {
		return -1;
	}
}

/*
 * send_buf - 文字列の送信
 */
int send_buf(SOCKET soc, char *buf)
{
#define TIMEOUT			0
	struct timeval waittime;
	fd_set rdps;
	char *send_buf;
	int send_len;
	int selret;
	int len;

	// 送信バッファの設定
	send_buf = buf;
	send_len = tstrlen(send_buf);
	// タイムアウト設定
	waittime.tv_sec = TIMEOUT;
	waittime.tv_usec = 0;

	while (*send_buf != '\0') {
		// 送信バッファの空を確認
		ZeroMemory(&rdps, sizeof(fd_set));
		FD_ZERO(&rdps);
		FD_SET(soc, &rdps);
		selret = select(FD_SETSIZE, (fd_set *)0, &rdps, (fd_set *)0, &waittime);
		if (selret == SOCKET_ERROR) {
			return -1;
		}
		if (selret == 0 || FD_ISSET(soc, &rdps) == FALSE) {
			continue;
		}
		// 分割して送信
		len = (send_len - (send_buf - buf) < SEND_SIZE) ? send_len - (send_buf - buf) : SEND_SIZE;
		if (ssl_type == -1) {
			len = send(soc, send_buf, len, 0);
		} else if (ssl == NULL) {
			len = -1;
		} else {
			len = SSL_write(ssl->ssl, send_buf, len);
		}
		if (len == -1) {
			return -1;
		}
		send_buf += len;
	}
	return 0;
}

/*
 * send_buf_t - TCHAR型の文字列の送信
 */
#ifdef UNICODE
int send_buf_t(SOCKET soc, TCHAR *wbuf)
{
	char *p;
	int ret;

	p = alloc_tchar_to_char(wbuf);
	if (p == NULL) {
		return -1;
	}
	ret = send_buf(soc, p);
	mem_free(&p);
	return ret;
}
#endif

/*
 * socket_close - 通信終了
 */
void socket_close(HWND hWnd, SOCKET soc)
{
	if (ssl_type != -1 && ssl != NULL) {
		SSL_shutdown(ssl->ssl);
	}
	if (soc != -1) {
#ifdef WSAASYNC
		WSAAsyncSelect(soc, hWnd, 0, 0);
#endif
		// 切断
		shutdown(soc, 2);
		closesocket(soc);
	}
	if (ssl_type != -1 && ssl != NULL) {
		if (ssl->ssl != NULL) {
			SSL_free(ssl->ssl);
			ssl->ssl = NULL;
		}
		if (ssl->ctx != NULL) {
			SSL_CTX_free(ssl->ctx);
			ssl->ctx = NULL;
		}
		LocalFree(ssl);
	}
	ssl = NULL;

	// POP3情報の解放
	pop3_free();
	// SMTP情報の解放
	smtp_free();

	// 受信バッファの解放
	mem_free(&recv_buf);
	recv_buf = NULL;
	// 未処理バッファの解放
	mem_free(&old_buf);
	old_buf = NULL;
	old_buf_len = 0;
	old_buf_size = 0;
}

/*
 * init_ssl - SSLの初期化
 */
int init_ssl(const HWND hWnd, const SOCKET soc, TCHAR *ErrStr)
{
	TCHAR buf[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];

	if (ssl_type == -1 || ssl != NULL) {
		return 0;
	}

	SetSocStatusTextT(hWnd, STR_STATUS_SSL);

	if (crypt32_lib == NULL) {
		SSL_library_init();
		SSL_load_error_strings();
		ssl_idx = SSL_get_ex_new_index(0, "App Data Index", NULL, NULL, NULL);
		RAND_poll();
		while (RAND_status() == 0) {
			unsigned short rand_ret = rand() % 65536;
			RAND_seed(&rand_ret, sizeof(rand_ret));
		}

		if ((crypt32_lib = LoadLibrary(TEXT("crypt32.dll"))) != NULL) {
#ifdef _WIN32_WCE
			_CertOpenStore = GetProcAddress(crypt32_lib, TEXT("CertOpenStore"));
			_CertFindCertificateInStore = GetProcAddress(crypt32_lib, TEXT("CertFindCertificateInStore"));
			_CertCloseStore = GetProcAddress(crypt32_lib, TEXT("CertCloseStore"));
#else
			_CertOpenStore = GetProcAddress(crypt32_lib, "CertOpenStore");
			_CertFindCertificateInStore = GetProcAddress(crypt32_lib, "CertFindCertificateInStore");
			_CertCloseStore = GetProcAddress(crypt32_lib, "CertCloseStore");
#endif
		}

	}
	// SSLの初期化
#ifdef UNICODE
	{
		char *ca = alloc_tchar_to_char(op.CAFile);
		char *cert = alloc_tchar_to_char(ssl_info.Cert);
		char *pkey = alloc_tchar_to_char(ssl_info.Pkey);
		char *pass = alloc_tchar_to_char(ssl_info.Pass);
		char err[BUF_SIZE];
		*err = '\0';
		ssl = ssl_init(soc, ssl_type, ssl_info.Verify, ssl_info.Depth, ca, NULL, cert, pkey, pass, err);
		char_to_tchar(err, buf, BUF_SIZE - 1);
		mem_free(&ca);
		mem_free(&cert);
		mem_free(&pkey);
		mem_free(&pass);
	}
#else
	*buf = TEXT('\0');
	ssl = ssl_init(soc, ssl_type, ssl_info.Verify, ssl_info.Depth, op.CAFile, NULL, ssl_info.Cert, ssl_info.Pkey, ssl_info.Pass, buf);
#endif
	if (ssl == NULL || (long)ssl == -1) {
		if ((long)ssl == -1) {
			wsprintf(ErrStr, STR_ERR_SOCK_SSL_VERIFY, buf);
			ssl = NULL;
		} else {
			wsprintf(ErrStr, STR_ERR_SOCK_SSL_INIT, buf);
		}
		return -1;
	}
	wsprintf(tmp, TEXT("verify: %s"), buf);
	SetSocStatusTextT(hWnd, tmp);
	return 0;
}

/*
 * free_ssl - SSLの解放
 */
void free_ssl(void)
{
	ERR_free_strings();

	if (crypt32_lib != NULL) {
		FreeLibrary(crypt32_lib);
		crypt32_lib = NULL;
	}
}

/*
 * verify_callback - verify callback
 */
static int verify_callback(int ok, X509_STORE_CTX *ctx)
{
	OPENSSL_INFO *si;
    SSL *ssl;
    int depth;

	depth = X509_STORE_CTX_get_error_depth(ctx);
	ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	if (ssl == NULL) {
		return 0;
	}
    si = SSL_get_ex_data(ssl, ssl_idx);
	if (si == NULL) {
		return 0;
	}
 	if (depth > si->depth) {
		ok = 0;
		X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
	}
	return ok;
}

/*
 * password_callback - password callback
 */
static int password_callback(char *buf, int size, int rwflag, void *userdata)
{
	if (userdata == NULL) {
		return 0;
	}
	tstrcpy(buf, userdata);
	return strlen(buf);
}

/*
 * set_client_cert - クライアント証明書と秘密鍵の設定
 */
static int set_client_cert(SSL_CTX *ctx, char *ca_file, char *key_file, char *pass)
{

	if (ca_file != NULL && strlen(ca_file) > 4 &&
		_stricmp(ca_file + (strlen(ca_file) - 4), ".p12") == 0) {
		// PKCS#12
		FILE *fp;
		PKCS12 *p12;
		EVP_PKEY *key = NULL;
		X509 *cert = NULL;
#if !defined(_WIN32_WCE) && defined( _MSC_VER) && (_MSC_VER > 1200)
		errno_t err;
#endif

		PKCS12_PBE_add();

		// ファイルの読み込み
#if !defined(_WIN32_WCE) && defined( _MSC_VER) && (_MSC_VER > 1200)
		err = fopen_s(&fp, ca_file, "r");
		if (err || !fp) {
			return 0;
		}
#else
		fp = fopen(ca_file, "r");
		if (!fp) {
			return 0;
		}
#endif
		// 変換
		p12 = d2i_PKCS12_fp(fp, NULL);
		if (p12 == NULL) {
			fclose(fp);
			return 0;
		}
		fclose(fp);

		// 解析
		if (!PKCS12_parse(p12, pass, &key, &cert, NULL)) {
			return 0;
		}
		if (cert != NULL) {
			if (SSL_CTX_use_certificate(ctx, cert) == 0) {
				X509_free(cert);
				return 0;
			}
			X509_free(cert);
		}
		if (key != NULL) {
			if (SSL_CTX_use_PrivateKey(ctx, key) == 0) {
				EVP_PKEY_free(key);
				return 0;
			}
			EVP_PKEY_free(key);
		}
		PKCS12_free(p12);
	} else {
		// PEM
		if (ca_file != NULL && *ca_file != '\0') {
			if (SSL_CTX_use_certificate_file(ctx, ca_file, X509_FILETYPE_PEM) == 0) {
				return 0;
			}
		}
		if (key_file != NULL && *key_file != '\0') {
			if (SSL_CTX_use_PrivateKey_file(ctx, key_file, X509_FILETYPE_PEM) == 0) {
				return 0;
			}
			if (SSL_CTX_check_private_key(ctx) == 0) {
				return 0;
			}
		}
	}
	return 1;
}

/*
 * load_system_verify - システムに登録されているのCA証明書を取得
 */
static int load_system_verify(SSL_CTX *ssl_ctx)
{
    HCERTSTORE cs;
	X509_STORE *root_certs;
    X509 *cert;
    const CERT_CONTEXT *cert_context = NULL;

	if (_CertOpenStore == NULL ||
		_CertFindCertificateInStore == NULL ||
		_CertCloseStore == NULL) {
		return 0;
	}

	root_certs = X509_STORE_new();
	// 証明書ストアのハンドル取得
	cs = (HCERTSTORE)_CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, CERT_SYSTEM_STORE_CURRENT_USER, L"ROOT");

	// 証明書の検索
	while (cert_context = (CERT_CONTEXT *)_CertFindCertificateInStore(cs,
		X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, cert_context)) {
		// 変換
		cert = d2i_X509(NULL, (unsigned char **)&cert_context->pbCertEncoded, cert_context->cbCertEncoded);
		if (cert == NULL) {
			return 0;
		}
		X509_STORE_add_cert(root_certs, cert);
		X509_free(cert);
	}
	_CertCloseStore(cs, 0);

	// CTXに登録
	SSL_CTX_set_cert_store(ssl_ctx, root_certs);
	return 1;
}

/*
 * ssl_init - SSL初期化
 */
static OPENSSL_INFO *ssl_init(
	const int soc,		// ソケット
	const int ssl_type,	// 0-SSLv23, 1-TLSv1, 2-SSLv3, 3-SSLv2
	int verify,			// 1-VERIFY_PEER, 0-VERIFY_NONE
	int depth,			// 証明書チェーンの長さ
	char *ca_file,		// CA証明書ファイル
	char *ca_path,		// CA証明書パス
	char *cert_file,	// クライアント証明書
	char *key_file,		// 秘密鍵
	char *pass,			// パスワード
	char *err_str)		// (OUT)エラー
{
	OPENSSL_INFO *si;
	const SSL_METHOD *sslm;
	SYSTEMTIME st;
	long verify_result;
	int ret, e, loop_cnt, hflag, tstart, tnow;

	if (ssl_type == -1) {
		return 0;
	}
	//si = mem_calloc(sizeof(OPENSSL_INFO)); // would appear to be a memory leak, since si is freed with LocalFree
	si = LocalAlloc(LPTR, sizeof(OPENSSL_INFO));
	if (si == NULL) {
		return 0;
	}
	si->depth = (depth < 0 || depth > MAX_DEPTH) ? MAX_DEPTH : depth;
	// SSL
	switch (ssl_type) {
	case 0:
	default:
		sslm = SSLv23_client_method();
		break;
	case 1:
		sslm = TLSv1_client_method();
		break;
	case 2:
		sslm = SSLv3_client_method();
		break;
	case 3:
		sslm = SSLv2_client_method();
		break;
	}
	si->ctx = SSL_CTX_new(sslm);
	if (si->ctx == NULL){
		ERR_error_string(ERR_get_error(), err_str);
		LocalFree(si);
		return 0;
	}
	SSL_CTX_set_options(si->ctx, SSL_OP_ALL);
	SSL_CTX_set_mode(si->ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	SSL_CTX_set_default_passwd_cb(si->ctx, password_callback);
	SSL_CTX_set_default_passwd_cb_userdata(si->ctx, pass);
	SSL_CTX_set_verify(si->ctx, (verify != 0) ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, verify_callback);
    SSL_CTX_set_verify_depth(si->ctx, si->depth + 1);

	// クライアント証明書の設定
	if (set_client_cert(si->ctx, cert_file, key_file, pass) == 0) {
		ERR_error_string(ERR_get_error(), err_str);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return 0;
	}

	// CA証明書の設定
	if ((ca_file  == NULL && ca_path == NULL) ||
		SSL_CTX_load_verify_locations(si->ctx, ca_file, ca_path) == 0) {
		// システムのCA証明書を設定
		if (load_system_verify(si->ctx) == 0) {
			if (SSL_CTX_set_default_verify_paths(si->ctx) == 0) {
				ERR_error_string(ERR_get_error(), err_str);
				SSL_CTX_free(si->ctx);
				LocalFree(si);
				return 0;
			}
		}
	}

	si->ssl = SSL_new(si->ctx);
	if (si->ssl == NULL){
		ERR_error_string(ERR_get_error(), err_str);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return 0;
	}
	SSL_set_ex_data(si->ssl, ssl_idx, si);
	if (SSL_set_fd(si->ssl, soc) == 0){
		ERR_error_string(ERR_get_error(), err_str);
		SSL_free(si->ssl);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return 0;
	}
	// SSL開始
	GetLocalTime(&st);
	tstart = st.wMinute * 60 + st.wSecond;
	hflag = st.wHour + 1;
	while (hflag) {
		for (loop_cnt = 0; loop_cnt < 1000; loop_cnt++) {
			ret = SSL_connect(si->ssl);
			e = SSL_get_error(si->ssl, ret);
			if (e == SSL_ERROR_WANT_CONNECT ||
				e == SSL_ERROR_WANT_READ ||
				e == SSL_ERROR_WANT_WRITE) {
				continue;
			}
			hflag = 0;
			break;
		}
		if (hflag) {
			// check timeout
			GetLocalTime(&st);
			tnow = st.wMinute * 60 + st.wSecond;
			if (hflag != st.wHour + 1) {
				// wrapped an hour
				tnow += 60*60;
			}
			if (tnow - tstart > op.TimeoutInterval) {
				break;
			}
		}
	}
	if (hflag) {
		SSL_free(si->ssl);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return 0;
	}
	// 検証結果の取得
	verify_result = SSL_get_verify_result(si->ssl);
	if (ret == -1){
		if (verify_result == X509_V_OK) {
			ret = 0;
			ERR_error_string(ERR_get_error(), err_str);
		} else {
			sprintf_s(err_str, BUF_SIZE-1, "%s (%d)", X509_verify_cert_error_string(verify_result), verify_result);
		}
		SSL_free(si->ssl);
		SSL_CTX_free(si->ctx);
		LocalFree(si);
		return (OPENSSL_INFO *)ret;
	}
	sprintf_s(err_str, BUF_SIZE-1, "%s (%d)", X509_verify_cert_error_string(verify_result), verify_result);
	return si;
}

BOOL CALLBACK AboutBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND AboutWnd;
	static HFONT hFont;
	LOGFONT logfont;
	HDC hDC;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		SetControlFont(hDlg);
		if ((int)lParam == ID_MENUITEM_ABOUT) {
			SetWindowText(GetDlgItem(hDlg, IDC_APPNAME), APP_NAME STR_UNICODE TEXT(" (SSL)"));
			SetWindowText(GetDlgItem(hDlg, IDC_BUILDNAME), NPOPBUILDSSL);
			SetWindowText(GetDlgItem(hDlg, IDC_VISIT_WEB), STR_WEB_ADDR);
			ShowWindow(GetDlgItem(hDlg, IDC_VISIT_OPENSSL), SW_HIDE);
#ifdef _WIN32_WCE
			SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_TEXT), STR_ABOUT_TEXT TEXT("\r\n") STR_ABOUT_OPENSSL);
#else
			SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_TEXT), STR_ABOUT_TEXT);
#endif
		} else {
#ifdef UNICODE
			TCHAR *msg;
			msg = alloc_char_to_tchar(SSLeay_version(0));
			SetWindowText(GetDlgItem(hDlg, IDC_APPNAME), msg);
			mem_free(&msg);
#else
			// 0 = SSLEAY_VERSION
			SetWindowText(GetDlgItem(hDlg, IDC_APPNAME), SSLeay_version(0));
#endif
			SetWindowText(GetDlgItem(hDlg, IDC_VISIT_OPENSSL), STR_OPENSSL_WEB_ADDR);
			ShowWindow(GetDlgItem(hDlg, IDC_VISIT_WEB), SW_HIDE);
			SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_TEXT), STR_ABOUT_OPENSSL);
#ifdef _WIN32_WCE
			ShowWindow(GetDlgItem(hDlg, IDC_VISIT_WM_HELP), SW_HIDE);
#endif
		}

		memset ((char *)&logfont, 0, sizeof (logfont));

		hDC = GetDC(hDlg);
		logfont.lfHeight = - (int)((8 * GetDeviceCaps(hDC, LOGPIXELSY)) / 72);
		logfont.lfWidth = 0; 
		logfont.lfEscapement = 0; 
		logfont.lfOrientation = 0;
		logfont.lfWeight = FW_NORMAL; 
		logfont.lfItalic = FALSE; 
		logfont.lfUnderline = FALSE; 
		logfont.lfStrikeOut = FALSE; 
		logfont.lfCharSet = DEFAULT_CHARSET; 
		logfont.lfOutPrecision = OUT_DEFAULT_PRECIS; 
		logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS; 
		logfont.lfQuality = DEFAULT_QUALITY; 
		logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE; 
#ifdef _WIN32_WCE   // Windows CE - 8, "Tahoma"
#ifdef UNICODE
		wcscpy(logfont.lfFaceName, TEXT("Tahoma"));
#else
		strcpy_s(logfont.lfFaceName, 32, TEXT("Tahoma"));
#endif
#else // Win32 - 8, "MS Sans Serif"
#ifdef UNICODE
		wcscpy(logfont.lfFaceName, TEXT("MS Sans Serif"));
#else
		strcpy_s(logfont.lfFaceName, 32, TEXT("MS Sans Serif"));
#endif
#endif
		hFont = CreateFontIndirect( &logfont );

		SendMessage(GetDlgItem(hDlg, IDC_ABOUT_TEXT), WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE);  

		if(AboutWnd != NULL){
			_SetForegroundWindow(AboutWnd);
			EndDialog(hDlg, FALSE);
			break;
		}
		AboutWnd = hDlg;

		SetWindowText(hDlg, STR_TITLE_ABOUT);
		break;

	case WM_CLOSE:
		AboutWnd = NULL;
		DeleteObject(hFont);
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_VISIT_WEB:
			ShellOpen(STR_WEB_ADDR);
			break;
		case IDC_VISIT_OPENSSL:
			ShellOpen(STR_OPENSSL_WEB_ADDR);
			break;

#ifdef _WIN32_WCE
		case IDC_VISIT_WM_HELP:
#ifdef _WIN32_WCE_PPC
			ShellOpen(STR_WM_HELP_ADDR);
#else
			ShellOpen(STR_WEB_DOC);
#endif
			break;
#endif

		case IDOK:
			AboutWnd = NULL;
			DeleteObject(hFont);
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			AboutWnd = NULL;
			DeleteObject(hFont);
			EndDialog(hDlg, FALSE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/* End of source */