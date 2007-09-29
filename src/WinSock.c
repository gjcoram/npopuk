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
#define RECVBUFSIZE			4096	/* 受信バッファサイズ */
#else
#define RECVBUFSIZE			32768	/* 受信バッファサイズ */
#endif

/* Global Variables */
char *RecvBuf = NULL;				// 内部受信バッファ
char *RemainderBuf = NULL;			// 内部受信未処理バッファ
int RemainderBufLen = 0;

SSL_INFO SSLInfo;
int ssl_type = -1;
long ssl;
static HMODULE ssl_lib;
static FARPROC ssl_init;
static FARPROC ssl_send;
static FARPROC ssl_recv;
static FARPROC ssl_close;
static FARPROC ssl_free;

// 外部参照
extern OPTION op;
extern char *MailSize;				// メールサイズ

/* Local Function Prototypes */

/*
 * GetHostByName - ホスト名からIPアドレスを取得する
 */
unsigned long GetHostByName(HWND hWnd, TCHAR *Server, TCHAR *ErrStr)
{
	unsigned long ret;
	LPHOSTENT lpHostEnt;
#ifdef UNICODE
	char *HostName;
#endif

	SetSocStatusTextT(hWnd, STR_STATUS_GETHOSTBYNAME, 1);

	if (Server == NULL || *Server == TEXT('\0')) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOSERVER);
		return 0;
	}

#ifdef UNICODE
	HostName = AllocTcharToChar(Server);
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
	ret = inet_addr(Server);
	if (ret == -1) {
		lpHostEnt = gethostbyname(Server);
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
 * ConnectServer - サーバに接続する
 */
SOCKET ConnectServer(HWND hWnd, unsigned long IPaddr, unsigned short Port, const int ssl_tp, const SSL_INFO *si, TCHAR *ErrStr)
{
	SOCKET Soc;
	struct sockaddr_in serversockaddr;

	SetSocStatusTextT(hWnd, STR_STATUS_CONNECT, 1);

	SSLInfo = *si;
	ssl_type = ssl_tp;
	ssl = 0;

	// ソケットの作成
	Soc = socket(PF_INET, SOCK_STREAM, 0);
	if (Soc == INVALID_SOCKET) {
		lstrcpy(ErrStr, STR_ERR_SOCK_CREATESOCKET);
		return -1;
	}
	// 接続先の設定
	serversockaddr.sin_family = AF_INET;
	serversockaddr.sin_addr.s_addr = IPaddr;
	serversockaddr.sin_port = htons((unsigned short)Port);
	ZeroMemory(serversockaddr.sin_zero, sizeof(serversockaddr.sin_zero));

#ifdef WSAASYNC
	if (WSAAsyncSelect(Soc, hWnd, WM_SOCK_SELECT, FD_CONNECT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		lstrcpy(ErrStr, STR_ERR_SOCK_EVENT);
		return -1;
	}
#endif
	// 接続
	if (connect(Soc, (struct sockaddr *)&serversockaddr, sizeof(serversockaddr)) == SOCKET_ERROR) {
#ifdef WSAASYNC
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return Soc;
		}
#endif
		lstrcpy(ErrStr, STR_ERR_SOCK_CONNECT);
		return -1;
	}
#ifndef WSAASYNC
	// SSLの初期化
	if (init_ssl(hWnd, Soc, ErrStr) == -1) {
		return -1;
	}
#endif
	return Soc;
}

/*
 * RecvBufProc - 受信して改行単位で処理する
 */
int RecvBufProc(HWND hWnd, SOCKET soc)
{
	char *buf;
	char *rbuf = NULL;
	char *p, *r;
	int buf_len;
	int len;
	int i;

	// 受信用バッファの確保
	if (RecvBuf == NULL) {
		if ((RecvBuf = (char *)mem_alloc(RECVBUFSIZE)) == NULL) {
			return SELECT_MEM_ERROR;
		}
	}

	// 受信
	if (ssl_type == -1 || ssl_recv == NULL) {
		buf_len = recv(soc, RecvBuf, RECVBUFSIZE - 1, 0);
	} else {
		buf_len = ssl_recv(ssl, RecvBuf, RECVBUFSIZE - 1);
	}
	if (buf_len == SOCKET_ERROR || buf_len == 0) {
#ifdef WSAASYNC
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return SELECT_SOC_SUCCEED;
		}
#endif
		return SELECT_SOC_CLOSE;
	}
	*(RecvBuf + buf_len) = '\0';
	p = RecvBuf;

	if (RemainderBuf != NULL && *RemainderBuf != '\0') {
		// 前回の未処理分のバッファと今回のバッファを結合する
		p = rbuf = (char *)mem_alloc(RemainderBufLen + buf_len + 1);
		if (rbuf == NULL) {
			return SELECT_MEM_ERROR;
		}
		CopyMemory(rbuf, RemainderBuf, RemainderBufLen);
		CopyMemory(rbuf + RemainderBufLen, RecvBuf, buf_len);
		buf_len += RemainderBufLen;
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
	mem_free(&RemainderBuf);
	RemainderBuf = buf;
	RemainderBufLen = tstrlen(buf);
	mem_free(&rbuf);
	return SELECT_SOC_SUCCEED;
}

/*
 * RecvSelect - 受信可能なバッファがある場合に受信を行う
 */
#ifndef WSAASYNC
int RecvSelect(HWND hWnd, SOCKET soc)
{
#define TIMEOUT			0		/* タイムアウト秒数 */
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
	return RecvBufProc(hWnd, soc);
}
#endif

/*
 * Tsend - TCHAR型の文字列の送信
 */
int Tsend(SOCKET soc, TCHAR *wBuf)
{
#ifdef UNICODE
	char *p;
	int ret;

	p = AllocTcharToChar(wBuf);
	if (p == NULL) {
		return -1;
	}
	if (ssl_type == -1 || ssl_send == NULL) {
		ret = send(soc, p, tstrlen(p), 0);
	} else {
		ret = ssl_send(ssl, p, strlen(p));
	}
	mem_free(&p);
	return ret;
#else
	if (ssl_type == -1 || ssl_send == NULL) {
		return send(soc, wBuf, lstrlen(wBuf), 0);
	}
	return ssl_send(ssl, wBuf, strlen(wBuf));
#endif
}

/*
 * SendBuf - 文字列の送信
 */
int SendBuf(SOCKET soc, char *buf)
{
#define TIMEOUT			0
	struct timeval waittime;
	fd_set rdps;
	char send_buf[BUF_SIZE];
	char *r;
	int len;
	int selret;

	r = buf;

	waittime.tv_sec = TIMEOUT;
	waittime.tv_usec = 0;

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
 * TSendBuf - TCHAR型の文字列の送信
 */
#ifdef UNICODE
int TSendBuf(SOCKET soc, TCHAR *wBuf)
{
	char *p;
	int ret;

	p = AllocTcharToChar(wBuf);
	if (p == NULL) {
		return -1;
	}
	ret = SendBuf(soc, p);
	mem_free(&p);
	return ret;
}
#endif

/*
 * SocketClose - 通信終了
 */
void SocketClose(HWND hWnd, SOCKET soc)
{
#ifdef WSAASYNC
	WSAAsyncSelect(soc, hWnd, 0, 0);
#endif

	if (ssl_type != -1 && ssl_close != NULL) {
		ssl_close(ssl);
	}

	shutdown(soc, 2);
	closesocket(soc);

	if (ssl_type != -1 && ssl_free != NULL) {
		ssl_free(ssl);
	}
	ssl = 0;

	FreeUidlList();
	FreeMailBuf();

	mem_free(&RecvBuf);
	RecvBuf = NULL;
	mem_free(&RemainderBuf);
	RemainderBuf = NULL;
	RemainderBufLen = 0;
	mem_free(&MailSize);
	MailSize = NULL;
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

	SetSocStatusTextT(hWnd, STR_STATUS_SSL, 1);

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
		char *cert = AllocTcharToChar(SSLInfo.Cert);
		char *pkey = AllocTcharToChar(SSLInfo.Pkey);
		char *pass = AllocTcharToChar(SSLInfo.Pass);
		char err[BUF_SIZE];
		*err = '\0';
		ssl = ssl_init(soc, ssl_type, SSLInfo.Verify, SSLInfo.Depth, ca, NULL, cert, pkey, pass, err);
		CharToTchar(err, buf, BUF_SIZE - 1);
		mem_free(&ca);
		mem_free(&cert);
		mem_free(&pkey);
		mem_free(&pass);
	}
#else
	*buf = TEXT('\0');
	ssl = ssl_init(soc, ssl_type, SSLInfo.Verify, SSLInfo.Depth, op.CAFile, NULL, SSLInfo.Cert, SSLInfo.Pkey, SSLInfo.Pass, buf);
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
	SetSocStatusTextT(hWnd, tmp, 1);
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
