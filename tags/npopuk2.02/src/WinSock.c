/*
 * nPOP
 *
 * WinSock.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"

/* Define */
#define CRLF_LEN				2

#ifdef _WIN32_WCE
#define RECV_SIZE				4096		// 受信バッファサイズ
#else
#define RECV_SIZE				32768		// 受信バッファサイズ
#endif
#define SEND_SIZE				4096		// 送信時の分割サイズ

/* Global Variables */
static char *recv_buf;						// 受信バッファ
static char *old_buf;						// 未処理バッファ
static int old_buf_len;
static int old_buf_size;

int ssl_type = -1;
static HANDLE ssl;
static SSL_INFO ssl_info;

static HINSTANCE ssl_lib;
static FARPROC ssl_init;
static FARPROC ssl_send;
static FARPROC ssl_recv;
static FARPROC ssl_close;
static FARPROC ssl_free;

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
#ifdef UNICODE
	char *HostName;
#endif

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
	ret = inet_addr(HostName);
	if (ret == -1) {
		lpHostEnt = gethostbyname(HostName);
		if (lpHostEnt != NULL) {
			ret = ((struct in_addr *)lpHostEnt->h_addr_list[0])->s_addr;
		} else {
			lstrcpy(ErrStr, STR_ERR_SOCK_GETIPADDR);
			ret = 0;
		}
	}
	mem_free(&HostName);
#else
	ret = inet_addr(server);
	if (ret == -1) {
		lpHostEnt = gethostbyname(server);
		if (lpHostEnt != NULL) {
			ret = ((struct in_addr *)lpHostEnt->h_addr_list[0])->s_addr;
		} else {
			lstrcpy(ErrStr, STR_ERR_SOCK_GETIPADDR);
			ret = 0;
		}
	}
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
	// Use built-in SSL if no npopssl.dll
	if (ssl_tp >= 0 && INVALID_FILE_SIZE == GetFileAttributes(TEXT("\\Windows\\npopssl.dll")))
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
	if (recv_buf == NULL && (recv_buf = (char *)mem_alloc(RECV_SIZE)) == NULL) {
		return SELECT_MEM_ERROR;
	}
	// 受信
	if (ssl_type == -1 || ssl_recv == NULL) {
		buf_len = recv(soc, recv_buf, RECV_SIZE, 0);
	} else {
		buf_len = ssl_recv(ssl, recv_buf, RECV_SIZE);
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
	if (ssl_type == -1 || ssl_send == NULL) {
		return send(soc, wbuf, len, 0);
	}
	return ssl_send(ssl, wbuf, len);
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
		if (ssl_type == -1 || ssl_send == NULL) {
			len = send(soc, send_buf, len, 0);
		} else {
			len = ssl_send(ssl, send_buf, len);
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
	if (ssl_type != -1 && ssl_close != NULL) {
		// SSLを切断
		ssl_close(ssl);
	}
	if (soc != -1) {
#ifdef WSAASYNC
		WSAAsyncSelect(soc, hWnd, 0, 0);
#endif
		// 切断
		shutdown(soc, 2);
		closesocket(soc);
	}
	if (ssl_type != -1 && ssl_free != NULL) {
		// SSLの解放
		ssl_free(ssl);
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

	if (ssl_init == NULL) {
		if ((ssl_lib = LoadLibrary(TEXT("npopssl.dll"))) == NULL) {
#ifdef _WIN32_WCE
			lstrcpy(ErrStr, STR_ERR_SOCK_NOSSL);
#else
			*buf = TEXT('\0');
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buf, BUF_SIZE - 1, NULL);
			wsprintf(ErrStr, TEXT("%s\n%s"), STR_ERR_SOCK_NOSSL, buf);
#endif
			return -1;
		}
#ifdef _WIN32_WCE
		ssl_init = GetProcAddress(ssl_lib, TEXT("ssl_init"));
		ssl_send = GetProcAddress(ssl_lib, TEXT("ssl_send"));
		ssl_recv = GetProcAddress(ssl_lib, TEXT("ssl_recv"));
		ssl_close = GetProcAddress(ssl_lib, TEXT("ssl_close"));
		ssl_free = GetProcAddress(ssl_lib, TEXT("ssl_free"));
#else
		ssl_init = GetProcAddress(ssl_lib, "ssl_init");
		ssl_send = GetProcAddress(ssl_lib, "ssl_send");
		ssl_recv = GetProcAddress(ssl_lib, "ssl_recv");
		ssl_close = GetProcAddress(ssl_lib, "ssl_close");
		ssl_free = GetProcAddress(ssl_lib, "ssl_free");
#endif
	}
	if (ssl_init == NULL || ssl_send == NULL || ssl_recv == NULL || ssl_close == NULL || ssl_free == NULL) {
#ifdef _WIN32_WCE
		lstrcpy(ErrStr, STR_ERR_SOCK_NOSSL);
#else
		*buf = TEXT('\0');
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buf, BUF_SIZE - 1, NULL);
		wsprintf(ErrStr, TEXT("%s\n%s"), STR_ERR_SOCK_NOSSL, buf);
#endif
		return -1;
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
		ssl = (HANDLE)ssl_init(soc, ssl_type, ssl_info.Verify, ssl_info.Depth, ca, NULL, cert, pkey, pass, err);
		char_to_tchar(err, buf, BUF_SIZE - 1);
		mem_free(&ca);
		mem_free(&cert);
		mem_free(&pkey);
		mem_free(&pass);
	}
#else
	*buf = TEXT('\0');
	ssl = (HANDLE)ssl_init(soc, ssl_type, ssl_info.Verify, ssl_info.Depth, op.CAFile, NULL, ssl_info.Cert, ssl_info.Pkey, ssl_info.Pass, buf);
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
	if (ssl_lib != NULL) {
		FreeLibrary(ssl_lib);
		ssl_lib = NULL;
	}
}
/* End of source */
