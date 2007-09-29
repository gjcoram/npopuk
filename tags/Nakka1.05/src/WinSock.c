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

/* Define */
#define CRLF_LEN			2
#define LF_LEN				1

#ifdef _WIN32_WCE
#define RECVBUFSIZE			4096		// 受信バッファサイズ */
#else
#define RECVBUFSIZE			32768		// 受信バッファサイズ */
#endif

/* Global Variables */
static char *recv_buf = NULL;			// 内部受信バッファ
static char *remainder_buf = NULL;		// 内部受信未処理バッファ
static int remainder_buf_len = 0;

int ssl_type = -1;
static long ssl;
static SSL_INFO ssl_info;

static HMODULE ssl_lib;
static FARPROC ssl_init;
static FARPROC ssl_send;
static FARPROC ssl_recv;
static FARPROC ssl_close;
static FARPROC ssl_free;

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
	HostName = AllocTcharToChar(server);
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
	SOCKET soc;
	struct sockaddr_in serversockaddr;

	SetSocStatusTextT(hWnd, STR_STATUS_CONNECT);

	// SSL情報
	ssl_info = *si;
	ssl_type = ssl_tp;
	ssl = 0;
	// ソケットの作成
	soc = socket(PF_INET, SOCK_STREAM, 0);
	if (soc == INVALID_SOCKET) {
		lstrcpy(ErrStr, STR_ERR_SOCK_CREATESOCKET);
		return -1;
	}
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
	if (init_ssl(hWnd, soc, ErrStr) == -1) {
		return -1;
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
	char *rbuf = NULL;
	char *p, *r;
	int buf_len;
	int len;
	int i;

	// 受信用バッファの確保
	if (recv_buf == NULL) {
		if ((recv_buf = (char *)mem_alloc(RECVBUFSIZE)) == NULL) {
			return SELECT_MEM_ERROR;
		}
	}

	// 受信
	if (ssl_type == -1 || ssl_recv == NULL) {
		buf_len = recv(soc, recv_buf, RECVBUFSIZE - 1, 0);
	} else {
		buf_len = ssl_recv(ssl, recv_buf, RECVBUFSIZE - 1);
	}
	if (buf_len == SOCKET_ERROR || buf_len == 0) {
#ifdef WSAASYNC
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return SELECT_SOC_SUCCEED;
		}
#endif
		return SELECT_SOC_CLOSE;
	}
	*(recv_buf + buf_len) = '\0';
	p = recv_buf;

	if (remainder_buf != NULL && *remainder_buf != '\0') {
		// 前回の未処理分のバッファと今回のバッファを結合する
		p = rbuf = (char *)mem_alloc(remainder_buf_len + buf_len + 1);
		if (rbuf == NULL) {
			return SELECT_MEM_ERROR;
		}
		CopyMemory(rbuf, remainder_buf, remainder_buf_len);
		CopyMemory(rbuf + remainder_buf_len, recv_buf, buf_len);
		buf_len += remainder_buf_len;
	}
	buf = (char *)mem_alloc(buf_len + 1);
	if (buf == NULL) {
		mem_free(&rbuf);
		return SELECT_MEM_ERROR;
	}

	i = 0;
	while (1) {
		// CR LF までの文字列を抽出
		for (r = buf, len = 0; i < buf_len; p++, r++, len++, i++) {
			if (*p == '\0') {
				len--;
				continue;
			}
			if (*p == '\r' && *(p + 1) == '\n') {
				break;
			}
			*r = *p;
		}
		*r = '\0';
		if (i >= buf_len) {
			break;
		}
		p += CRLF_LEN;
		i += CRLF_LEN;
		// ウィンドウに処理すべき文字列を渡す
		if (SendMessage(hWnd, WM_SOCK_RECV, len, (LPARAM)buf) == FALSE) {
			mem_free(&rbuf);
			mem_free(&buf);
			return SELECT_SOC_SUCCEED;
		}
	}
	// 未処理の文字列を待避
	mem_free(&remainder_buf);
	remainder_buf = buf;
	remainder_buf_len = tstrlen(buf);
	mem_free(&rbuf);
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
int send_data(SOCKET soc, TCHAR *wbuf)
{
#ifdef UNICODE
	char *p;
	int ret;

	p = AllocTcharToChar(wbuf);
	if (p == NULL) {
		return -1;
	}
	// データ送信
	if (ssl_type == -1 || ssl_send == NULL) {
		ret = send(soc, p, tstrlen(p), 0);
	} else {
		ret = ssl_send(ssl, p, strlen(p));
	}
	mem_free(&p);
	return ret;
#else
	// データ送信
	if (ssl_type == -1 || ssl_send == NULL) {
		return send(soc, wbuf, lstrlen(wbuf), 0);
	}
	return ssl_send(ssl, wbuf, strlen(wbuf));
#endif
}

/*
 * send_buf - 文字列の送信
 */
int send_buf(SOCKET soc, char *buf)
{
#define TIMEOUT			0
	struct timeval waittime;
	fd_set rdps;
	char send_buf[BUF_SIZE];
	char *r;
	int len;
	int selret;

	waittime.tv_sec = TIMEOUT;
	waittime.tv_usec = 0;

	r = buf;
	while (*r != '\0') {
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

		// 送信
		StrCpyN(send_buf, r, BUF_SIZE - 1);
		if (ssl_type == -1 || ssl_send == NULL) {
			if ((len = send(soc, send_buf, tstrlen(send_buf), 0)) == -1) {
				return -1;
			}
		} else {
			if ((len = ssl_send(ssl, send_buf, tstrlen(send_buf))) == -1) {
				return -1;
			}
		}
		r += len;
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

	p = AllocTcharToChar(wbuf);
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
#ifdef WSAASYNC
	WSAAsyncSelect(soc, hWnd, 0, 0);
#endif
	if (ssl_type != -1 && ssl_close != NULL) {
		// SSLを切断
		ssl_close(ssl);
	}
	// 切断
	shutdown(soc, 2);
	closesocket(soc);
	if (ssl_type != -1 && ssl_free != NULL) {
		// SSLの解放
		ssl_free(ssl);
	}
	ssl = 0;

	// POP3情報の解放
	pop3_free();

	// 受信バッファを解放
	mem_free(&recv_buf);
	recv_buf = NULL;
	mem_free(&remainder_buf);
	remainder_buf = NULL;
	remainder_buf_len = 0;
}

/*
 * socket_free - ソケット情報の解放
 */
void socket_free(void)
{
	mem_free(&recv_buf);
	mem_free(&remainder_buf);
}

/*
 * init_ssl - SSLの初期化
 */
int init_ssl(const HWND hWnd, const SOCKET soc, TCHAR *ErrStr)
{
	TCHAR buf[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];

	if (ssl_type == -1 || ssl != 0) {
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
		char *ca = AllocTcharToChar(op.CAFile);
		char *cert = AllocTcharToChar(ssl_info.Cert);
		char *pkey = AllocTcharToChar(ssl_info.Pkey);
		char *pass = AllocTcharToChar(ssl_info.Pass);
		char err[BUF_SIZE];
		*err = '\0';
		ssl = ssl_init(soc, ssl_type, ssl_info.Verify, ssl_info.Depth, ca, NULL, cert, pkey, pass, err);
		CharToTchar(err, buf, BUF_SIZE - 1);
		mem_free(&ca);
		mem_free(&cert);
		mem_free(&pkey);
		mem_free(&pass);
	}
#else
	*buf = TEXT('\0');
	ssl = ssl_init(soc, ssl_type, ssl_info.Verify, ssl_info.Depth, op.CAFile, NULL, ssl_info.Cert, ssl_info.Pkey, ssl_info.Pass, buf);
#endif
	if (ssl == 0 || ssl == -1) {
		if (ssl == -1) {
			wsprintf(ErrStr, STR_ERR_SOCK_SSL_VERIFY, buf);
			ssl = 0;
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
