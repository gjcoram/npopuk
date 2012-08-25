/*
 * nPOP
 *
 * Pop3.c (RFC 1939)
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
#include "global.h"
#include "md5.h"

/* Define */
#define OK_LEN					3			// "+OK" ‚ÌƒoƒCƒg”

#ifdef _WCE_OLD
#define MAIL_BUF_SIZE			4096		//Early size
#else
#define MAIL_BUF_SIZE			32768		//of buffer for mail reception Early size
#endif
#define DOWNLOAD_SIZE			65535		// RETR–¢g—p‚Ì‘S•¶óMƒTƒCƒY

#define HEAD_LINE				30			// ƒwƒbƒ_[’·
#define LINE_LEN				80
#define REDRAWCNT				100			// ƒXƒe[ƒ^ƒXƒo[Äİ’è”
#define IDC_EDIT_BODY			2003

#define CMD_USER				"USER"
#define CMD_PASS				"PASS"
#define CMD_APOP				"APOP"
#define CMD_STLS				"STLS"
#define CMD_STAT				"STAT"
#define CMD_LIST				"LIST"
#define CMD_UIDL				"UIDL"
#define CMD_TOP					"TOP"
#define CMD_RETR				"RETR"
#define CMD_DELE				"DELE"
#define CMD_RSET				"RSET"
#define CMD_QUIT				"QUIT"
// make this an INI setting?
#define GMAIL_SYS_PROBLEM		"temporary system problem"
#define GMAIL_EMPTY_2			"empty mailbox.  Sorry."
#define GMAIL_EMPTY_3			"fake empty mailbox."

/* Global Variables */
static char *mail_buf = NULL;				// ƒ[ƒ‹óM—pƒoƒbƒtƒ@
static char *last_response = NULL;
static unsigned int mail_buf_size;			// ƒ[ƒ‹óM—pƒoƒbƒtƒ@‚ÌÀƒTƒCƒY
static unsigned int mail_buf_len;			// ƒ[ƒ‹óM—pƒoƒbƒtƒ@“à‚Ì•¶š—ñ’·
static int mail_size = -1;					// ƒ[ƒ‹ƒTƒCƒY
static int recvlen = 0;
static int recvcnt = 0;
static int list_get_no;						// óMƒ[ƒ‹ˆÊ’u
static int download_get_no;					// ƒ_ƒEƒ“ƒ[ƒhƒ[ƒ‹ˆÊ’u
static int delete_get_no;					// íœƒ[ƒ‹ˆÊ’u
static int deletes_remaining = 65535;
static int reverse_stop_point = 0;
static BOOL init_recv;						// V’…æ“¾ˆÊ’u‰Šú‰»ƒtƒ‰ƒO
static int mail_received;					// ‚PŒ–ÚóMƒtƒ‰ƒO
static BOOL receiving_uidl;					// UIDLƒŒƒXƒ|ƒ“ƒXóM’†
static BOOL receiving_data;					// ƒ[ƒ‹ƒf[ƒ^óM’†
static BOOL disable_uidl;					// UIDLƒTƒ|[ƒgƒtƒ‰ƒO
static BOOL disable_top;					// TOPƒTƒ|[ƒgƒtƒ‰ƒO (op.ListDownload ‚ª 1 or TOP‚ª–¢ƒTƒ|[ƒg)
static BOOL uidl_missing = FALSE;
static SYSTEMTIME recv_clock;

typedef struct _UIDL_INFO {
	int no;
	BOOL in_mailbox;
	TCHAR *uidl;
} UIDL_INFO;

static UIDL_INFO *ui;						// UIDLƒŠƒXƒg
static int ui_size;							// UIDLƒŠƒXƒg”
static int ui_pt;							// UIDL’Ç‰ÁˆÊ’u
static MAILITEM *uidl_item;					// UIDL‚ğƒZƒbƒg‚·‚éƒ[ƒ‹ƒAƒCƒeƒ€

// ŠO•”QÆ
extern OPTION op;

extern int command_status;
extern int NewMailCnt;
extern TCHAR *g_Pass;
extern HWND hViewWnd;		// view window
extern HWND mListView;		// mail list

extern BOOL ShowMsgFlag;
extern BOOL NewMail_Flag;
extern BOOL EndThreadSortFlag;
extern BOOL ServerDelete;
extern BOOL ViewReopen;

extern int PopBeforeSmtpFlag;
extern int ssl_type;
extern UINT nBroadcastMsg;

/* Local Function Prototypes */
static BOOL mail_buf_init(unsigned int size);
static BOOL mail_buf_set(char *buf, int len);
static BOOL uidl_init(int size);
static BOOL uidl_set(char *buf, int len);
static TCHAR *uidl_get(int get_no);
static int uidl_check(TCHAR *buf);
static int uidl_find_missing(HWND hWnd, int GotNo, int Reverse);
static void uidl_free(void);
static void init_mailbox(HWND hWnd, MAILBOX *tpMailBox, BOOL ShowFlag);
static char *skip_response(char *p);
static BOOL check_response(char *buf);
static BOOL check_message_id(char *buf, MAILITEM *tpMailItem, TCHAR *ErrStr, MAILBOX *tpMailBox);
static int check_last_mail(HWND hWnd, SOCKET soc, BOOL chek_flag, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static BOOL send_command(HWND hWnd, SOCKET soc, TCHAR *Command, int Cnt, TCHAR *ErrStr);
static int send_command_top(HWND hWnd, SOCKET soc, int Cnt, TCHAR *ErrStr, int len, int ret);
static TCHAR *create_apop(char *buf, TCHAR *ErrStr, MAILBOX *tpMailBox, TCHAR *sPass);
static int login_proc(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox);
static int list_proc_stat(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int list_proc_uidl_all(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int list_proc_uidl_check(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int list_proc_uidl_set(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int list_proc_list(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int list_proc_top(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int exec_send_check_command(HWND hWnd, SOCKET soc, int get_no, TCHAR *ErrStr, MAILBOX *tpMailBox);
static int exec_proc_init(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int exec_proc_stat(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int exec_proc_retr(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int exec_proc_uidl(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int exec_proc_top(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static int exec_proc_dele(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
static void pop_log_download_rate();

#ifdef _WIN32_WCE_PPC
//================================================
// findTodayPlugin: ...
//================================================
HWND findTodayPlugin(WCHAR *wTodayItem)
{
	BOOL bFound = FALSE;
	HWND hWnd1, hWnd2, hWnd3, hPlugin = NULL;

	if ((hWnd1 = GetDesktopWindow()) && (hWnd2 = GetWindow(hWnd1, GW_CHILD)) &&
		(hWnd3 = GetWindow(hWnd2, GW_CHILD)))
	{
		if ((hPlugin = GetWindow(hWnd3, GW_CHILD)) || ((hPlugin = GetWindow(hWnd3, GW_HWNDNEXT)) && (hPlugin = GetWindow(hPlugin, GW_CHILD))))
		{
			WCHAR className[MAX_PATH];
			do
			{
				GetClassName(hPlugin, className, MAX_PATH);
				if (wcscmp(className, wTodayItem) == 0)
				{
					bFound = TRUE;
					break;
				}
			} while (hPlugin = GetWindow(hPlugin, GW_HWNDNEXT));
		}
	}
	return (bFound?hPlugin:NULL);
}
#endif


/*
 * mail_buf_init - ƒ[ƒ‹ƒoƒbƒtƒ@‚ğ‰Šú‰»
 */
static BOOL mail_buf_init(unsigned int size)
{
	if (mail_buf == NULL || mail_buf_size < size) {
		mem_free(&mail_buf);
		mail_buf_size = size + 3;
		mail_buf = (char *)mem_alloc(mail_buf_size);
	}
	mail_buf_len = 0;
	return ((mail_buf == NULL) ? FALSE : TRUE);
}

/*
 * mail_buf_set - ƒ[ƒ‹ƒoƒbƒtƒ@‚É•¶š—ñ‚ğ’Ç‰Á
 */
static BOOL mail_buf_set(char *buf, int len)
{
	char *tmp;

	if (mail_buf_size < (mail_buf_len + len + 2 + 1)) {
		// ÄŠm•Û
		mail_buf_size += (MAIL_BUF_SIZE < len + 2 + 1) ? (len + 2 + 1) : MAIL_BUF_SIZE;
		tmp = (char *)mem_alloc(mail_buf_size);
		if (tmp == NULL) {
			mail_buf_size = 0;
			return FALSE;
		}
		CopyMemory(tmp, mail_buf, mail_buf_len);
		CopyMemory(tmp + mail_buf_len, buf, len);
		*(tmp + mail_buf_len + len + 0) = '\r';
		*(tmp + mail_buf_len + len + 1) = '\n';
		*(tmp + mail_buf_len + len + 2) = '\0';
		mail_buf_len += (len + 2);
		// ‹Œƒƒ‚ƒŠ‚Ì‰ğ•ú
		mem_free(&mail_buf);
		mail_buf = tmp;
	} else {
		// ’Ç‰Á
		CopyMemory(mail_buf + mail_buf_len, buf, len);
		*(mail_buf + mail_buf_len + len + 0) = '\r';
		*(mail_buf + mail_buf_len + len + 1) = '\n';
		*(mail_buf + mail_buf_len + len + 2) = '\0';
		mail_buf_len += (len + 2);
	}
	return TRUE;
}

/*
 * uidl_init - UIDLƒŠƒXƒg‚ğ‰Šú‰»
 */
static BOOL uidl_init(int size)
{
	uidl_free();
	ui_size = size;
	ui_pt = 0;
	ui = (UIDL_INFO *)mem_calloc(sizeof(UIDL_INFO) * ui_size);
	return ((ui == NULL) ? FALSE : TRUE);
}

/*
 * uidl_set - UIDLƒŠƒXƒg‚É•¶š—ñ‚ğ’Ç‰Á
 */
static BOOL uidl_set(char *buf, int len)
{
	char *p;

	if (ui_pt >= ui_size) {
		return FALSE;
	}

	ui[ui_pt].no = a2i(buf);
	ui[ui_pt].in_mailbox = FALSE;
	for (p = buf; *p != ' ' && *p != '\0'; p++);		// ”Ô†
	for (; *p == ' '; p++);								// ‹ó”’

	ui[ui_pt].uidl = alloc_char_to_tchar(p);
	if (ui[ui_pt].uidl == NULL) {
		return FALSE;
	}
	ui_pt++;
	return TRUE;
}

/*
 * uidl_get - UIDLæ“¾
 */
static TCHAR *uidl_get(int get_no)
{
	int i;

	for (i = 0; i < ui_size; i++) {
		if (ui[i].no == get_no) {
			return ui[i].uidl;
		}
	}
	return NULL;
}

/*
 * uidl_check - UIDL‚ªUIDL‚Ìˆê——‚É‘¶İ‚·‚é‚©ƒ`ƒFƒbƒN
 */
static int uidl_check(TCHAR *buf)
{
	int i;

	if (buf == NULL) {
		return 0;
	}
	for (i = 0; i < ui_size; i++) {
		if (lstrcmp(ui[i].uidl, buf) == 0) {
			ui[i].in_mailbox = TRUE;
			return ui[i].no;
		}
	}
	return 0;
}

/*
 * uidl_find_missing - find number of message whose UIDL is not in the mailbox
 */
static int uidl_find_missing(HWND hWnd, int GotNo, int Reverse)
{
	int i, j;

	for (i = 0; i < ui_size; i++) {
		if (Reverse > 0) {
			j = ui_size - 1 - i;
		} else {
			j = i;
		}
		if (GotNo > 0 && ui[j].no == GotNo) {
			ui[j].in_mailbox = TRUE;
		} else if (ui[j].in_mailbox == FALSE) {
			return ui[j].no;
		}
	}
	return -1;
}

/*
 * uidl_free - UIDLƒŠƒXƒg‚ğ‰ğ•ú
 */
static void uidl_free(void)
{
	int i;

	if (ui == NULL) return;

	for (i = 0; i < ui_size; i++) {
		mem_free(&ui[i].uidl);
	}
	mem_free(&ui);
	ui = NULL;
	ui_size = 0;
}

/*
 * init_mailbox - ƒ[ƒ‹‚ğÅ‰‚©‚çóM‚Éİ’è
 */
static void init_mailbox(HWND hWnd, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	if (tpMailBox->ListInitMsg == TRUE) {
		if (ShowFlag == TRUE) {
			ListView_DeleteAllItems(mListView);
			if (op.PreviewPaneHeight > 0) {
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, 
					(LPARAM)((op.ExpertMode) ? TEXT("") : STR_MSG_SELECT_PREVIEW));
			}
		}
		item_free(tpMailBox->tpMailItem, tpMailBox->MailItemCnt);
		mem_free((void **)&tpMailBox->tpMailItem);
		tpMailBox->tpMailItem = NULL;
		tpMailBox->AllocCnt = tpMailBox->MailItemCnt = 0;
		tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
	}
	tpMailBox->LastNo = 0;
	init_recv = FALSE;
}

/*
 * skip_response - ƒŒƒXƒ|ƒ“ƒX‚ÌƒXƒLƒbƒv
 */
static char *skip_response(char *p)
{
	for (; *p != ' ' && *p != '\0'; p++);	// +OK
	for (; *p == ' '; p++);					// ‹ó”’
	for (; *p != ' ' && *p != '\0'; p++);	// ”Ô†
	for (; *p == ' '; p++);					// ‹ó”’
	return p;
}

/*
 * check_response - ƒŒƒXƒ|ƒ“ƒX‚Ìƒ`ƒFƒbƒN
 */
static BOOL check_response(char *buf)
{
	return ((tstrlen(buf) < OK_LEN || *buf != '+' ||
		!(*(buf + 1) == 'O' || *(buf + 1) == 'o') ||
		!(*(buf + 2) == 'K' || *(buf + 2) == 'k')) ? FALSE : TRUE);
}

/*
 * check_message_id - ƒƒbƒZ[ƒWID‚Ìƒ`ƒFƒbƒN
 */
static BOOL check_message_id(char *buf, MAILITEM *tpMailItem, TCHAR *ErrStr, MAILBOX *tpMailBox)
{
	char *content;
#ifdef UNICODE
	char *p;
#endif

	// Message-Id
	content = item_get_message_id(buf);
	if (content == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOMESSAGEID);
		return FALSE;
	}

#ifdef UNICODE
	p = NULL;
	if (tpMailItem->MessageID != NULL) {
		p = alloc_tchar_to_char(tpMailItem->MessageID);
		if (p == NULL) {
			mem_free(&content);
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return FALSE;
		}
	}
	if (tpMailItem->MessageID == NULL || tstrcmp(p, content) != 0) {
		mem_free(&content);
		mem_free(&p);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		mem_free(&tpMailBox->LastMessageId);
		tpMailBox->LastMessageId = NULL;
		return FALSE;
	}
	mem_free(&p);
#else
	if (tpMailItem->MessageID == NULL || tstrcmp(tpMailItem->MessageID, content) != 0) {
		mem_free(&content);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		mem_free(&tpMailBox->LastMessageId);
		tpMailBox->LastMessageId = NULL;
		return FALSE;
	}
#endif
	mem_free(&content);
	return TRUE;
}

/*
 * check_last_mail - ÅŒã‚ÉóM‚µ‚½ƒ[ƒ‹‚Ìƒ`ƒFƒbƒN
 */
static int check_last_mail(HWND hWnd, SOCKET soc, BOOL check_flag, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	int reverse = (tpMailBox->UseGlobalRecv) ? op.GetReverse : tpMailBox->GetReverse;
	if (check_flag != FALSE) {
		// ‘O‰ñÅŒã‚Éæ“¾‚µ‚½ƒ[ƒ‹‚Å‚Í‚È‚¢
		if (disable_uidl == FALSE) {
			// UIDL‚Å‘Sƒ[ƒ‹‚Ì“¯Šú‚ğæ‚é
			receiving_uidl = FALSE;
			SetSocStatusTextT(hWnd, TEXT(CMD_UIDL));
			if (send_buf(soc, CMD_UIDL"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return FALSE;
			}
			return POP_UIDL_ALL;
		}
		// 1Œ–Ú‚©‚çæ“¾
		init_mailbox(hWnd, tpMailBox, ShowFlag);
		if (reverse > 0) {
			list_get_no = tpMailBox->MailCnt;
		} else {
			list_get_no = 1;
		}
	} else {
		// UIDL of last message matches
		if (reverse > 0) {
			if (tpMailBox->LastNo < tpMailBox->MailCnt) {
				// start getting from the most recent
				list_get_no = tpMailBox->MailCnt;
			} else {
				// already have the last
				list_get_no = tpMailBox->MailCnt + 1;
			}
		} else {
			// get the next message
			list_get_no++;
		}
		if (list_get_no > tpMailBox->MailCnt) {
			uidl_missing = FALSE;
			return POP_QUIT;
		}
	}
	if (send_command(hWnd, soc, TEXT(CMD_LIST), list_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_LIST;
}

/*
 * send_command - ƒRƒ}ƒ“ƒh‚Æˆø”(”’l)‚ğ‘—M‚·‚é
 */
static BOOL send_command(HWND hWnd, SOCKET soc, TCHAR *Command, int Cnt, TCHAR *ErrStr)
{
	TCHAR wBuf[BUF_SIZE];

	wsprintf(wBuf, TEXT("%s %d"), Command, Cnt);
	SetSocStatusTextT(hWnd, wBuf);
	lstrcat(wBuf, TEXT("\r\n"));

	if (send_buf_t(soc, wBuf) == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return FALSE;
	}
	return TRUE;
}

/*
 * send_command_top - TOPƒRƒ}ƒ“ƒh‚ğ‘—M‚·‚é
 */
static int send_command_top(HWND hWnd, SOCKET soc, int Cnt, TCHAR *ErrStr, int len, int ret)
{
	TCHAR wBuf[BUF_SIZE];

	wsprintf(wBuf, TEXT(CMD_TOP)TEXT(" %d %d"), Cnt, len);
	SetSocStatusTextT(hWnd, wBuf);
	lstrcat(wBuf, TEXT("\r\n"));

	if (send_buf_t(soc, wBuf) == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
		return POP_ERR;
	}
	return ret;
}

/*
 * create_apop - APOP‚Ì•¶š—ñ‚ğ¶¬‚·‚é
 */
static TCHAR *create_apop(char *buf, TCHAR *ErrStr, MAILBOX *tpMailBox, TCHAR *sPass)
{
	MD5_CTX context;
	TCHAR *wbuf;
	unsigned char digest[16];
	char *hidx = NULL;
	char *tidx = NULL;
	char *pass;
	char *p;
	int len;
	int i;

	//< Empty > to extraction
	for (hidx = NULL, p = buf; *p != '\0';p++) {
		if (*p == '<') {
			hidx = p;
		}
	}
	if (hidx != NULL) {
		for (tidx = NULL, p = hidx; *p != '\0';p++) {
			if (*p == '>') {
				tidx = p;
			}
		}
	}
	if (hidx == NULL || tidx == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOAPOP);
		return NULL;
	}

	pass = alloc_tchar_to_char(sPass);
	if (pass == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}

	p = (char *)mem_alloc((tidx - hidx + 2) + tstrlen(pass) + 1);
	if (p == NULL) {
		mem_free(&pass);
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}

	str_cpy_n(p, hidx, tidx - hidx + 2);
	tstrcat(p, pass);
	mem_free(&pass);

	//Digest value is taken the
	MD5Init(&context);
	MD5Update(&context, p, tstrlen(p));
	MD5Final(digest, &context);

	mem_free(&p);

	wbuf = (TCHAR *)mem_alloc(
		sizeof(TCHAR) * (lstrlen(TEXT(CMD_APOP)) + 1 + lstrlen(tpMailBox->User) + 1 + (16 * 2) + 2 + 1));
	if (wbuf == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}

	len = wsprintf(wbuf, TEXT(CMD_APOP)TEXT(" %s "), tpMailBox->User);
	for (i = 0; i < 16; i++, len += 2) {
		wsprintf(wbuf + len, TEXT("%02x"), digest[i]);
	}
	lstrcat(wbuf, TEXT("\r\n"));
	return wbuf;
}

/*
 * login_proc - ƒƒOƒCƒ“‚Ìˆ—‚ğs‚¤
 */
static int login_proc(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox)
{
	TCHAR *wbuf;
	static TCHAR *pass;
	int ret = POP_ERR;
	BOOL PopSTARTTLS = FALSE;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);

	switch (command_status) {
	case POP_STARTTLS:
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RESPONSE);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		ssl_type = 1;
		if (init_ssl(hWnd, soc, ErrStr) == -1) {
			return POP_ERR;
		}
		PopSTARTTLS = TRUE;

	case POP_START:
		// ƒtƒ‰ƒO‚ğ‰Šú‰»
		disable_top = FALSE;
		disable_uidl = (tpMailBox->NoUIDL == 0) ? FALSE : TRUE;
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_SOCK_RESPONSE);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		if (tpMailBox->User == NULL || *tpMailBox->User == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOUSERID);
			return POP_ERR;
		}

		// STARTTLS
		if (PopSTARTTLS == FALSE && tpMailBox->PopSSL == 1 && tpMailBox->PopSSLInfo.Type == 4) {
			SetSocStatusTextT(hWnd, TEXT(CMD_STLS));
			if (send_buf(soc, CMD_STLS"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return POP_ERR;
			}
			return POP_STARTTLS;
		}

		pass = tpMailBox->Pass;
		if (pass == NULL || *pass == TEXT('\0')) {
			pass = tpMailBox->TmpPass;
		}
		if (pass == NULL || *pass == TEXT('\0')) {
			pass = g_Pass;
		}
		if (pass == NULL || *pass == TEXT('\0')) {
			lstrcpy(ErrStr, STR_ERR_SOCK_NOPASSWORD);
			return POP_ERR;
		}
		// APOP‚É‚æ‚é”FØ
		if (tpMailBox->APOP == 1) {
			wbuf = create_apop(buf, ErrStr, tpMailBox, pass);
			if (wbuf == NULL) {
				return POP_ERR;
			}

			SetSocStatusTextT(hWnd, TEXT(CMD_APOP)TEXT(" ****"));

			if (send_buf_t(soc, wbuf) == -1) {
				mem_free(&wbuf);
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return POP_ERR;
			}
			mem_free(&wbuf);
			ret = POP_PASS;
			break;
		}
		// USER ‚Ì‘—M
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT(CMD_USER)) + 1 + lstrlen(tpMailBox->User) + 2 + 1));
		if (wbuf == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		str_join_t(wbuf, TEXT(CMD_USER), TEXT(" "), tpMailBox->User, (TCHAR *)-1);
		SetSocStatusTextT(hWnd, wbuf);
		lstrcat(wbuf, TEXT("\r\n"));
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return POP_ERR;
		}
		mem_free(&wbuf);

		ret = POP_USER;
		break;

	case POP_USER:
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_SOCK_ACCOUNT);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		// PASS ‚Ì‘—M
		wbuf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(TEXT(CMD_PASS)) + 1 + lstrlen(pass) + 2 + 1));
		if (wbuf == NULL) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		str_join_t(wbuf, TEXT(CMD_PASS), TEXT(" "), pass, TEXT("\r\n"), (TCHAR *)-1);
		SetSocStatusTextT(hWnd, TEXT(CMD_PASS)TEXT(" ****"));
		if (send_buf_t(soc, wbuf) == -1) {
			mem_free(&wbuf);
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return POP_ERR;
		}
		mem_free(&wbuf);

		ret = POP_PASS;
		break;

	case POP_PASS:
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_SOCK_BADPASSWORD);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		if (last_response != NULL) {
			mem_free(&last_response);
		}
		last_response = alloc_copy(buf);
		ret = POP_LOGIN;
		break;
	}
	return ret;
}

/*
 * list_proc_stat - STAT (the start of all "check" operations)
 */
static int list_proc_stat(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem = NULL;
	char *p, *r, *t;
	int get_no, reverse, ret;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);
	// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_STAT;
	}
	// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
	if (check_response(buf) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_SOCK_STAT);
		str_cat_n(ErrStr, buf, BUF_SIZE - 1);
		return POP_ERR;
	}

	// ƒ[ƒ‹”‚Ìæ“¾
	p = buf;
	t = NULL;
	for (; *p != ' ' && *p != '\0'; p++);
	for (; *p == ' '; p++);
	for (r = p; *r != '\0'; r++) {
		if (*r == ' ') {
			t = r + 1;
			*r = '\0';
			break;
		}
	}
	tpMailBox->MailCnt = a2i(p);
	if (t != NULL) {
		tpMailBox->MailSize = a2i(t);
	}
	if (tpMailBox->MailCnt == 0) {
		BOOL do_err = FALSE;
		if (op.NoEmptyMailbox) {
			do_err = TRUE;
		} else if (last_response != NULL) {
			for (p = last_response + 4; *p != '\0'; p++) {
				if (str_cmp_n(p, GMAIL_SYS_PROBLEM, strlen(GMAIL_SYS_PROBLEM)) == 0  ||
				    str_cmp_n(p, GMAIL_EMPTY_2, strlen(GMAIL_EMPTY_2)) == 0  ||
				    str_cmp_n(p, GMAIL_EMPTY_3, strlen(GMAIL_EMPTY_3)) == 0) {
					do_err = TRUE;
					break;
				}
			}
		}
		if (do_err) {
			if (last_response != NULL) {
#ifdef UNICODE
				int len = char_to_tchar_size(last_response+4);
				if (len < BUF_SIZE) {
					char_to_tchar(last_response+4, ErrStr, len);
				}
#else
				strcpy_s(ErrStr, BUF_SIZE, last_response + 4);
#endif
			}
			return (op.NoEmptyMailbox == 2) ? POP_QUIT : POP_ERR;
		}
		tpMailBox->ListInitMsg = TRUE;
		if (op.SocLog > 1) log_save_a("Clearing mailbox: server says 0 messages\r\n");
		init_mailbox(hWnd, tpMailBox, ShowFlag);
		SetItemCntStatusText(tpMailBox, FALSE, FALSE);
		return POP_QUIT;
	} else if (tpMailBox->LastNo == -1 && tpMailBox->ListInitMsg == TRUE) {
		init_mailbox(hWnd, tpMailBox, ShowFlag);
	}
	SetItemCntStatusText(tpMailBox, FALSE, FALSE);

	init_recv = FALSE;
	mail_received = 0;
	if (tpMailBox->UseGlobalRecv) {
		disable_top = (op.ListDownload != 0) ? TRUE : FALSE;
		reverse = op.GetReverse;
	} else {
		disable_top = (tpMailBox->ListDownload != 0) ? TRUE : FALSE;
		reverse = tpMailBox->GetReverse;
	}
	if (reverse > 0) {
		if (tpMailBox->ListInitMsg == TRUE || tpMailBox->LastMessageId == NULL) {
			// initializing or filling in from LastNo
			reverse_stop_point = tpMailBox->LastNo;
		} else {
			// regular check
			reverse_stop_point = tpMailBox->LastNo + 1;
		}
	}

	// UIDL‚Ì‰Šú‰»
	if (uidl_init(tpMailBox->MailCnt) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	if (tpMailBox->LastNo == 0 || tpMailBox->LastMessageId == NULL || *(tpMailBox->LastMessageId) == TEXT('\0')) {
		// w’è”Ô–Ú‚©‚çæ“¾
		if (disable_uidl == FALSE) {
			receiving_uidl = FALSE;
			SetSocStatusTextT(hWnd, TEXT(CMD_UIDL));
			if (send_buf(soc, CMD_UIDL"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return POP_ERR;
			}
			ret = POP_UIDL_ALL;
		} else {
			if (reverse > 0) {
				list_get_no = tpMailBox->MailCnt;
			} else {
				list_get_no = (tpMailBox->LastNo == 0) ? 1 : tpMailBox->LastNo;
			}
			init_recv = TRUE;
			if (send_command(hWnd, soc, TEXT(CMD_LIST), list_get_no, ErrStr) == FALSE) {
				return POP_ERR;
			}
			ret = POP_LIST;
		}
	} else if (tpMailBox->LastNo > tpMailBox->MailCnt) {
		// the server has fewer messages -> figure out which should be deleted
		if (disable_uidl == FALSE) {
			receiving_uidl = FALSE;
			SetSocStatusTextT(hWnd, TEXT(CMD_UIDL));
			if (send_buf(soc, CMD_UIDL"\r\n") == -1) {
				lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
				return POP_ERR;
			}
			ret = POP_UIDL_ALL;
		} else {
			// UIDL disallowed, have to initialize mailbox
			init_mailbox(hWnd, tpMailBox, ShowFlag);
			if (reverse > 0) {
				list_get_no = tpMailBox->MailCnt;
			} else {
				list_get_no = 1;
			}
			if (send_command(hWnd, soc, TEXT(CMD_LIST), list_get_no, ErrStr) == FALSE) {
				return POP_ERR;
			}
			ret = POP_LIST;
		}
	} else {
		// tpMailBox->LastNo <= tpMailBox->MailCnt
		// check UIDL of message with No=LastNo
		list_get_no = tpMailBox->LastNo;
		get_no = item_get_number_to_index(tpMailBox, list_get_no);
		if (get_no != -1) {
			tpMailItem = *(tpMailBox->tpMailItem + get_no);
		}
		if (disable_uidl == FALSE && tpMailItem != NULL && tpMailItem->UIDL != NULL) {
			if (send_command(hWnd, soc, TEXT(CMD_UIDL), list_get_no, ErrStr) == FALSE) {
				return POP_ERR;
			}
			return POP_UIDL_CHECK;
		} else {
			// UIDL disabled, or no message with No=LastNo, so start downloading
			int len;
			receiving_data = FALSE;
			if (mail_buf_init(MAIL_BUF_SIZE) == FALSE) {
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return POP_ERR;
			}
			if (reverse) {
				list_get_no = tpMailBox->MailCnt;
			}
			if (tpMailBox->UseGlobalRecv) {
				len = op.ListGetLine;
			} else {
				len = tpMailBox->ListGetLine;
			}
			ret = send_command_top(hWnd, soc, list_get_no, ErrStr, len, POP_TOP);
		}
	}
	return ret;
}

/*
 * list_proc_uidl_all - UIDL‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int list_proc_uidl_all(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem;
	MAILITEM *tpLastMailItem;
	int No, i, last_match, reverse;
	SetSocStatusTextT(hWnd, TEXT("uidl_all"));

	// UIDLƒŒƒXƒ|ƒ“ƒX‚Ì1s–Ú
	if (receiving_uidl == FALSE) {
		SetSocStatusText(hWnd, buf);
		// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
		if (*buf == '.' && *(buf + 1) == '\0') {
			return POP_UIDL_ALL;
		}
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == TRUE) {
			receiving_uidl = TRUE;
			return POP_UIDL_ALL;
		}
		// UIDL–¢ƒTƒ|[ƒg
		disable_uidl = TRUE;
		SetSocStatusTextT(hWnd, TEXT(CMD_STAT));
		if (send_buf(soc, CMD_STAT"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return POP_ERR;
		}
		return POP_STAT;
	}
	// UIDL‚ÌI‚í‚è‚Å‚Í‚È‚¢ê‡
	if (*buf != '.' || *(buf + 1) != '\0') {
		// óM•¶š—ñ‚ğ•Û‘¶‚·‚é
		if (uidl_set(buf, buflen) == FALSE) {
			uidl_free();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		return POP_UIDL_ALL;
	}

	reverse = (tpMailBox->UseGlobalRecv) ? op.GetReverse : tpMailBox->GetReverse;
	if (tpMailBox->LastNo == 0 || tpMailBox->LastMessageId == NULL || *(tpMailBox->LastMessageId) == TEXT('\0')) {
		init_recv = TRUE;
		if (tpMailBox->ListInitMsg == TRUE) {
			if (reverse > 0) {
				list_get_no = tpMailBox->MailCnt;
			} else {
				list_get_no = (tpMailBox->LastNo == 0) ? 1 : tpMailBox->LastNo;
			}
			if (send_command(hWnd, soc, TEXT(CMD_LIST), list_get_no, ErrStr) == FALSE) {
				return POP_ERR;
			}
			return POP_LIST;
		}
		// else "filling in", fall through to check UIDLs
	}

	if (ShowFlag == TRUE) {
		ListView_SetRedraw(mListView, FALSE);
	}
	SwitchCursor(FALSE);
	// Œ»İ•\¦‚³‚ê‚Ä‚¢‚éƒ[ƒ‹ˆê——‚ÆUIDL‚ğ”äŠr‚µ‚Ä”Ô†‚ğU‚è’¼‚·
	tpLastMailItem = NULL;
	last_match = 0;
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		switch ((No = uidl_check(tpMailItem->UIDL))) {
#ifdef UNICODE
		case -1:
			uidl_free();
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			if (ShowFlag == TRUE) {
				ListView_SetRedraw(mListView, TRUE);
			}
			SwitchCursor(TRUE);
			return POP_ERR;
#endif
		case 0:
			// UIDL‚ÌƒŠƒXƒg‚É‘¶İ‚µ‚È‚¢ƒ[ƒ‹‚Í‰ğ•ú‚·‚é
			if (ShowFlag == TRUE) {
				No = ListView_GetMemToItem(mListView, tpMailItem);
				if (tpMailItem != NULL && ((tpMailItem->ReFwd & REFWD_FWDHOLD) == 0)) {
					ListView_DeleteItem(mListView, No);
				}
			}
			item_free((tpMailBox->tpMailItem + i), 1);
			break;

		default:
			if (No > tpMailItem->No) {
				TCHAR msg[BUF_SIZE];
				uidl_missing = TRUE;
				wsprintf(msg, TEXT("X-No: was increased from %d to %d"), tpMailItem->No, No);
				SetSocStatusTextT(hWnd, msg);
			} else if (No == tpMailItem->No && uidl_missing == FALSE && No > last_match) {
				last_match = No;
			}
			// ƒ[ƒ‹”Ô†‚ğİ’è
			tpMailItem->No = No;
			if (tpLastMailItem == NULL || tpLastMailItem->No < tpMailItem->No) {
				tpLastMailItem = tpMailItem;
			}
			break;
		}
	}
	if (init_recv == TRUE) {
		// "filling in" from LastNo
		uidl_missing = TRUE;
		last_match = tpMailBox->LastNo;
	}
	// handles messages deleted from list
	if (last_match > 0) {
		for (i = 0; i < ui_size; i++) {
			if (ui[i].no < last_match) {
				ui[i].in_mailbox = TRUE;
			}
		}
	}
	// íœ‚³‚ê‚½ƒ[ƒ‹‚ğˆê——‚©‚çÁ‚·
	item_resize_mailbox(tpMailBox, FALSE);
	if (ShowFlag == TRUE) {
		ListView_SetRedraw(mListView, TRUE);
	}
	SwitchCursor(TRUE);
	SetItemCntStatusText(tpMailBox, FALSE, FALSE);

	mem_free(&tpMailBox->LastMessageId);
	tpMailBox->LastMessageId = NULL;
	tpMailBox->LastNo = 0;
	if (tpLastMailItem != NULL) {
		tpMailBox->LastMessageId = alloc_tchar_to_char(tpLastMailItem->MessageID);
		tpMailBox->LastNo = tpLastMailItem->No;
	}
	if (reverse > 0) {
		if (uidl_missing == TRUE) {
			// uidl_find_missing will set list_get_no to the one we need to get
			list_get_no = tpMailBox->MailCnt + 1;
		} else {
			// new mail has arrived on the server
			// get messages tpMailBox->MailCnt down to tpLastMailItem->No + 1
			list_get_no = tpMailBox->MailCnt;
			reverse_stop_point = tpMailBox->LastNo + 1;
		}
	} else {
		list_get_no = tpMailBox->LastNo + 1;
	}

	if (uidl_missing) {
		i = uidl_find_missing(hWnd, 0, reverse);
		if (i == -1) {
			uidl_missing = FALSE;
		} else {
			list_get_no = i;
		}
	}
	if (list_get_no > tpMailBox->MailCnt || (reverse > 0 && list_get_no < reverse_stop_point)) {
		return POP_QUIT;
	}
	if (send_command(hWnd, soc, TEXT(CMD_LIST), list_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_LIST;
}

/*
 * list_proc_uidl_check - UIDL‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int list_proc_uidl_check(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem = NULL;
	TCHAR *UIDL = NULL;
	int get_no;
	int ret;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);
	// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_UIDL_CHECK;
	}
	// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
	if (check_response(buf) == TRUE) {
		UIDL = alloc_char_to_tchar(skip_response(buf));
	} else {
		// UIDL–¢ƒTƒ|[ƒg
		disable_uidl = TRUE;
		receiving_data = FALSE;
		if (mail_buf_init(MAIL_BUF_SIZE) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		// TOP <msg_no> 0 to get headers?
		return send_command_top(hWnd, soc, list_get_no, ErrStr, 0, POP_TOP);
	}
	get_no = item_get_number_to_index(tpMailBox, list_get_no);
	if (get_no != -1) {
		tpMailItem = *(tpMailBox->tpMailItem + get_no);
	}
	// UIDL‚Ìƒ`ƒFƒbƒN
	ret = check_last_mail(hWnd, soc,
		(tpMailItem ==NULL || tpMailItem->UIDL == NULL || UIDL == NULL || lstrcmp(tpMailItem->UIDL, UIDL) != 0),
		ErrStr, tpMailBox, ShowFlag);
	mem_free(&UIDL);
	return ret;
}

/*
 * list_proc_uidl_set - UIDL‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int list_proc_uidl_set(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	int reverse;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);
	// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_UIDL;
	}
	// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
	if (check_response(buf) == TRUE) {
		uidl_item->UIDL = alloc_char_to_tchar(skip_response(buf));
	} else {
		disable_uidl = TRUE;
	}

	reverse = (tpMailBox->UseGlobalRecv) ? op.GetReverse : tpMailBox->GetReverse;
	if (reverse > 0) {
		list_get_no--;
		if (uidl_missing) {
			// filling in, keep looking for one we don't have
			while (list_get_no >= reverse_stop_point) {
				int get_no = item_get_number_to_index(tpMailBox, list_get_no);
				if (get_no == -1) {
					break;
				}
				list_get_no--;
			}
		} else if (item_get_number_to_index(tpMailBox, list_get_no) != -1) {
			// found one we already have
			list_get_no = 0;
		}
		if (list_get_no < reverse_stop_point) {
			uidl_missing = FALSE;
			return POP_QUIT;
		}
	} else {
		list_get_no++;
		if (uidl_missing) {
			while (list_get_no < tpMailBox->MailCnt) {
				int get_no = item_get_number_to_index(tpMailBox, list_get_no);
				if (get_no == -1) {
					break;
				}
				list_get_no++;
			}
		}
		if (list_get_no > tpMailBox->MailCnt) {
			uidl_missing = FALSE;
			return POP_QUIT;
		}
	}
	if (send_command(hWnd, soc, TEXT(CMD_LIST), list_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_LIST;
}

/*
 * list_proc_list - LIST‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int list_proc_list(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	char *p;
	int len = 0;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);
	// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_LIST;
	}
	// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
	mail_size = -1;
	if (check_response(buf) == TRUE) {
		p = skip_response(buf);
		mail_size = len = a2i(p);
		if (disable_top == FALSE) {
			int numchars;
			if (tpMailBox->UseGlobalRecv) {
				numchars = (op.ListGetLine + HEAD_LINE) * LINE_LEN;
			} else {
				numchars = (tpMailBox->ListGetLine + HEAD_LINE) * LINE_LEN;
			}
			len = (len > 0 && len < numchars) ? len : numchars;
		}
	}
	receiving_data = FALSE;

	if (mail_buf_init((len > 0) ? len : MAIL_BUF_SIZE) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	if (disable_top == TRUE) {
		// ˆê——æ“¾‚É–{•¶‚ğ‚·‚×‚Äƒ_ƒEƒ“ƒ[ƒh
		if (tpMailBox->NoRETR == 1) {
			return send_command_top(hWnd, soc, list_get_no, ErrStr, (len > 0) ? len : DOWNLOAD_SIZE, POP_RETR);
		}
		if (send_command(hWnd, soc, TEXT(CMD_RETR), list_get_no, ErrStr) == FALSE) {
			return POP_ERR;
		}
		return POP_RETR;
	}

	if (tpMailBox->UseGlobalRecv) {
		len = op.ListGetLine;
	} else {
		len = tpMailBox->ListGetLine;
	}

	return send_command_top(hWnd, soc, list_get_no, ErrStr, len, POP_TOP);
}

/*
 * list_proc_top - TOP‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int list_proc_top(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem;
	LV_ITEM lvi;
	TCHAR *p;
	char *new_message_id;
	int i, nOldMailCnt = NewMailCnt;
	int st = 0, reverse;

	// TOPƒŒƒXƒ|ƒ“ƒX‚Ì1s–Ú
	if (receiving_data == FALSE) {
		SetSocStatusText(hWnd, buf);
		// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
		if (*buf == '.' && *(buf + 1) == '\0') {
			return POP_TOP;
		}
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == TRUE) {
			char *p, *r = buf + OK_LEN;
			receiving_data = TRUE;
			recvlen = 0;
			recvcnt = REDRAWCNT;
			GetLocalTime(&recv_clock);
			// Yahoo! sometimes responds +OK nnn octets, and nnn != mail_size reported earlier
			for (; *r == ' '; r++); // skip spaces
			for (p = r; *p != '\0' && *p != '\r' && *p != '\n'; p++) {
				if (strcmp(p, "octets") == 0) {
					int new_size = a2i(r);
					if (new_size > mail_size - 10) {
						if (op.SocLog > 1) {
							char msg[100];
							sprintf_s(msg, 99, " message size updated from %d to %d", mail_size, new_size);
							log_save_a(msg);
						}
						mail_size = new_size;
					}
				}
			}
			return POP_TOP;
		}
		if (disable_top == FALSE && tpMailBox->NoRETR == 0) {
			// TOP‚ª–¢ƒTƒ|[ƒg‚Ì‰Â”\«‚ª‚ ‚é‚Ì‚ÅRETR‚ğ‘—M‚·‚é
			disable_top = TRUE;
			if (send_command(hWnd, soc, TEXT(CMD_RETR), list_get_no, ErrStr) == FALSE) {
				return POP_ERR;
			}
			return POP_RETR;
		}
		lstrcpy(ErrStr, (disable_top == FALSE) ? STR_ERR_SOCK_TOP : STR_ERR_SOCK_RETR);
		str_cat_n(ErrStr, buf, BUF_SIZE - 1);
		return POP_ERR;
	}
	// TOP‚ÌI‚í‚è‚Å‚Í‚È‚¢ê‡
	if (*buf != '.' || *(buf + 1) != '\0') {
		// óMƒf[ƒ^‚ğ•Û‘¶‚·‚é
		if (mail_buf_set(buf, buflen) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		recvlen += buflen;
		recvcnt++;
		if (recvcnt > REDRAWCNT) {
			recvcnt = 0;
			SetStatusRecvLen(hWnd, recvlen, mail_size, STR_STATUS_SOCKINFO_RECV);
		}
		return POP_TOP;
	}
	SetSocStatusTextT(hWnd, STR_STATUS_RECVDONE);

	reverse = (tpMailBox->UseGlobalRecv) ? op.GetReverse : tpMailBox->GetReverse;

	if (reverse > 0 && list_get_no == tpMailBox->MailCnt && init_recv == TRUE) {
		init_mailbox(hWnd, tpMailBox, ShowFlag);
	} else if (list_get_no == tpMailBox->LastNo) {
		// re-received message to check MessageId
		if (init_recv == TRUE) {
			// V’…æ“¾ˆÊ’u‚ª‰Šú‰»‚³‚ê‚½‚½‚ßƒ[ƒ‹ƒ{ƒbƒNƒX‚ğ‰Šú‰»
			init_mailbox(hWnd, tpMailBox, ShowFlag);
		} else {
			// ‘O‰ñÅŒã‚ÉóM‚µ‚½ƒ[ƒ‹‚ÌMessage-ID‚ğƒ`ƒFƒbƒN
			char *content = item_get_message_id(mail_buf);
			i = check_last_mail(hWnd, soc,
				(tpMailBox->LastMessageId == NULL || content == NULL || tstrcmp(tpMailBox->LastMessageId, content) != 0),
				ErrStr, tpMailBox, ShowFlag);
			mem_free(&content);
			return i;
		}
	}

	// óM‚ÌÅ‘åƒAƒCƒeƒ€”•ª‚Ìƒƒ‚ƒŠ‚ğŠm•Û
	if (mail_received == 0) {
		if (ShowFlag == TRUE) {
			ListView_SetItemCount(mListView, tpMailBox->MailCnt);
		}
		if (item_set_count(tpMailBox, tpMailBox->MailCnt) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
	}
	if (soc == -1) {
		// salvaging: check for minimal headers
		char *p;
		for (p = mail_buf; *p != '\0'; p++) {
			if (str_cmp_ni(p, "From:", 5) == 0 || str_cmp_ni(p, "Subject:", 8) == 0) {
				break;
			}
		}
		if (*p == '\0') {
			return POP_ERR;
		}
		st = ICON_ERROR;
	} else if (disable_top) {
		// using RETR, message is fully downloaded (even if server messes up reported size)
		st = ICON_DOWN;
	}
	if (op.SocLog > 1) {
		pop_log_download_rate();
	}

	new_message_id = item_get_message_id(mail_buf);
	// ƒwƒbƒ_‚©‚çƒAƒCƒeƒ€‚ğì¬
	tpMailItem = item_header_to_item(tpMailBox, &mail_buf, mail_size, list_get_no, st);
	if (mail_buf == NULL) {
		mail_buf_size = 0;
	}
	if (tpMailItem == NULL) {
		mem_free(&new_message_id);
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	if ((int)tpMailItem != -1 && tpMailItem->Body == NULL && disable_top == TRUE) {
		tpMailItem->Body = (char *)mem_alloc(sizeof(char));
		if (tpMailItem->Body != NULL) {
			*tpMailItem->Body = '\0';
			tpMailItem->Mark = tpMailItem->MailStatus = ICON_MAIL;
		}
	}

	if ((int)tpMailItem != -1) {
		BOOL first_new_msg = (tpMailItem->New == TRUE) && (mail_received != 1)
			&& (NewMail_Flag == FALSE) && (ShowMsgFlag == FALSE);

		// V’…ƒtƒ‰ƒO‚Ìœ‹
		if (first_new_msg && op.ClearNewOverlay == 1) {
			for (i = 0; i < tpMailBox->MailItemCnt; i++) {
				if (*(tpMailBox->tpMailItem + i) == NULL) {
					continue;
				}
				(*(tpMailBox->tpMailItem + i))->New = FALSE;
			}
		}

		if (ShowFlag == TRUE) {
			MAILITEM *listItem;

			st = ListView_ComputeState(tpMailItem->Priority, tpMailItem->Multipart);
			st = INDEXTOSTATEIMAGEMASK(st);
			if (tpMailItem->New) {
				st |= INDEXTOOVERLAYMASK(ICON_NEW_MASK);
			}

			if (first_new_msg) {
				int st2 = LVIS_FOCUSED;
				if (op.ClearNewOverlay == 1) {
					// clear new overlay from existing messages when new(er) mail received
					ListView_SetItemState(mListView, -1, 0, INDEXTOOVERLAYMASK(ICON_NEW_MASK));
					ListView_RedrawItems(mListView, 0, ListView_GetItemCount(mListView));
				}
				if (op.PreviewPaneHeight <= 0 || op.AutoPreview) {
					st2 |= LVIS_SELECTED;
				}
				// de-select all messages, then set up to select this new message
				ListView_SetItemState(mListView, -1, 0, st2);
				st |= st2;
			}
			st |= ((tpMailItem->Download == FALSE && tpMailItem->Mark != ICON_DOWN && tpMailItem->Mark != ICON_DEL)
				? LVIS_CUT : 0);
			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
			lvi.iItem = ListView_GetItemCount(mListView);
			if (lvi.iItem > 0 && op.LvAutoSort != 2 && op.LvThreadView == 0) {
				// insert in the middle (for fill-in or get reverse)
				// unless we're sorted by column or by thread
				for (i = lvi.iItem - 1; i >= 0; i--) {
					listItem = (MAILITEM *)ListView_GetlParam(mListView, i);
					if (listItem != NULL && listItem->No < tpMailItem->No) {
						i = i+1;
						break;
					}
				}
				if (i < 0) {
					lvi.iItem = 0;
				} else {
					lvi.iItem = i;
				}
			}
			lvi.iSubItem = 0;
			lvi.state = st;
			lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK | LVIS_CUT | LVIS_FOCUSED | LVIS_SELECTED;
			lvi.pszText = LPSTR_TEXTCALLBACK;
			lvi.cchTextMax = 0;
			lvi.iImage = I_IMAGECALLBACK;
			lvi.lParam = (long)tpMailItem;

			//of new arrival position The item is added to list view the
			i = ListView_InsertItem(mListView, &lvi);

			if (mail_received == 0) {
				ListView_EnsureVisible(mListView, i, TRUE);
			}
			// ˆês‰º‚ÖƒXƒNƒ[ƒ‹
			if (op.RecvScroll == 1) {
				SendMessage(mListView, WM_VSCROLL, SB_LINEDOWN, 0);
			}
			SetItemCntStatusText(tpMailBox, FALSE, FALSE);
			EndThreadSortFlag = TRUE;
		}
		if (first_new_msg) {
			mail_received = 1;
		} else if (mail_received == 0) {
			mail_received = -1;
		}

		if (tpMailItem->New) {
			// may be false, if message has been filtered to delete
			NewMailCnt++;
			tpMailBox->NewMail++;
		}
		tpMailBox->NeedsSave |= MAILITEMS_CHANGED;		
	}
	 
	if (nOldMailCnt != NewMailCnt && soc != -1)
	{
#ifdef _WIN32_WCE_PPC
		HWND hPlugin;
		if (hPlugin = findTodayPlugin(TEXT("phoneAlarmMaxCls")))
			PostMessage(hPlugin, nBroadcastMsg, (WPARAM)NewMailCnt, (LPARAM)0xFFFF);
#endif
		PostMessage(HWND_BROADCAST, nBroadcastMsg, (WPARAM)NewMailCnt, (LPARAM)0xFFFF);
	}

	// ÅŒã‚ÉóM‚µ‚½ƒ[ƒ‹‚Ì”Ô†‚ÆƒƒbƒZ[ƒWID‚ğ•Û‘¶‚·‚é
	if (list_get_no > tpMailBox->LastNo) {
		tpMailBox->LastNo = list_get_no;
		mem_free(&tpMailBox->LastMessageId);
		tpMailBox->LastMessageId = new_message_id;
	}
	if (new_message_id == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_NOMESSAGEID);
		return POP_ERR;
	} else if (tpMailBox->LastMessageId != new_message_id) {
		mem_free(&new_message_id);
	}
	if (soc == -1) {
		return POP_QUIT;
	}

	if ((int)tpMailItem != -1 && disable_uidl == FALSE) {
		p = uidl_get(list_get_no);
		if (p != NULL) {
			tpMailItem->UIDL = alloc_copy_t(p);
		} else {
			uidl_item = tpMailItem;
			if (send_command(hWnd, soc, TEXT(CMD_UIDL), list_get_no, ErrStr) == FALSE) {
				return POP_ERR;
			}
			return POP_UIDL;
		}
	}

	if (uidl_missing) {
		i = uidl_find_missing(hWnd, list_get_no, reverse);
		if (i == -1) {
			uidl_missing = FALSE;
			list_get_no = tpMailBox->MailCnt+1;
		} else {
			list_get_no = i;
		}
	} else if (reverse > 0) {
		list_get_no--;
	} else {
		list_get_no++;
	}
	if (list_get_no > tpMailBox->MailCnt || (reverse > 0 && list_get_no < reverse_stop_point)) {
		return POP_QUIT;
	}
	if (send_command(hWnd, soc, TEXT(CMD_LIST), list_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_LIST;
}

/*
 * exec_send_check_command - íœŠm”F—pƒRƒ}ƒ“ƒh‚ğ‘—M
 */
static int exec_send_check_command(HWND hWnd, SOCKET soc, int get_no, TCHAR *ErrStr, MAILBOX *tpMailBox)
{
	MAILITEM *tpMailItem = NULL;

	if (get_no != -1) {
		tpMailItem = *(tpMailBox->tpMailItem + get_no);
	}
	if (disable_uidl == TRUE || tpMailItem == NULL || tpMailItem->UIDL == NULL) {
		receiving_data = FALSE;
		if (mail_buf_init(MAIL_BUF_SIZE) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		if (disable_top == TRUE && tpMailBox->NoRETR == 0) {
			// RETR‚ğ‘—M
			if (send_command(hWnd, soc, TEXT(CMD_RETR), delete_get_no, ErrStr) == FALSE) {
				return POP_ERR;
			}
			return POP_TOP;
		}
		// TOP <msg_no> 0 to get headers?
		return send_command_top(hWnd, soc, delete_get_no, ErrStr, 0, POP_TOP);
	}
	// UIDL‚ğ‘—M
	if (send_command(hWnd, soc, TEXT(CMD_UIDL), delete_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_UIDL;
}

/*
 * exec_proc_init - Àsˆ—‚Ì‰Šú‰»
 */
static int exec_proc_init(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem;
	int size;
	int get_no;

	get_no = item_get_next_download_mark(tpMailBox, -1, &download_get_no);
	if (get_no == -1) {
		if (ServerDelete != FALSE) {
			get_no = item_get_next_delete_mark(tpMailBox, TRUE, -1, &delete_get_no);
		}
		if (get_no == -1) {
			return POP_QUIT;
		}
		// íœƒ[ƒ‹‚ÌŠm”FƒRƒ}ƒ“ƒh(UIDL)‚ğ‘—M
		return exec_send_check_command(hWnd, soc, get_no, ErrStr, tpMailBox);
	}

	// ƒ_ƒEƒ“ƒ[ƒh
	tpMailItem = *(tpMailBox->tpMailItem + get_no);
	if (tpMailItem == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	size = (tpMailItem->Size == NULL) ? 0 : _ttoi(tpMailItem->Size);
	receiving_data = FALSE;
	if (mail_buf_init((size > 0) ? size : MAIL_BUF_SIZE) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	// ‘S•¶óM‚·‚éƒRƒ}ƒ“ƒh‚ğ‘—M
	if (tpMailBox->NoRETR == 1) {
		return send_command_top(hWnd, soc, download_get_no, ErrStr, (size > 0) ? size : DOWNLOAD_SIZE, POP_RETR);
	}
	if (send_command(hWnd, soc, TEXT(CMD_RETR), download_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_RETR;
}

/*
 * exec_proc_stat - STAT‚ to get message count for ID_MENUITEM_DELETE_ALL_SERVER
 */
static int exec_proc_stat(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	char *p, *r, *t;
	int get_no;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_STAT;
	}
	if (check_response(buf) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_SOCK_STAT);
		str_cat_n(ErrStr, buf, BUF_SIZE - 1);
		return POP_ERR;
	}

	p = buf;
	t = NULL;
	for (; *p != ' ' && *p != '\0'; p++);
	for (; *p == ' '; p++);
	for (r = p; *r != '\0'; r++) {
		if (*r == ' ') {
			t = r + 1;
			*r = '\0';
			break;
		}
	}
	tpMailBox->MailCnt = a2i(p);
	if (t != NULL) {
		tpMailBox->MailSize = a2i(t);
	}
	get_no = -1;
	if (tpMailBox->MailCnt > 0) {
		if (mailbox_mark_all_delete(tpMailBox) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_INIT);
			return POP_ERR;
		}
		get_no = item_get_next_delete_mark(tpMailBox, TRUE, -1, &delete_get_no);
	}

	if (get_no == -1) {
		return POP_QUIT;
	}

	deletes_remaining--;
	if (send_command(hWnd, soc, TEXT(CMD_DELE), delete_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_DELE;
}

/*
 * exec_proc_retr - RETR‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int exec_proc_retr(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem;
	int i, size, get_no, st = 0;

	if (receiving_data == FALSE) {
		// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
		if (*buf == '.' && *(buf + 1) == '\0') {
			return POP_RETR;
		}
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == FALSE) {
			lstrcpy(ErrStr, (tpMailBox->NoRETR == 1) ? STR_ERR_SOCK_TOP : STR_ERR_SOCK_RETR);
			str_cat_n(ErrStr, buf, BUF_SIZE - 1);
			return POP_ERR;
		}
		SetSocStatusTextT(hWnd, STR_STATUS_RECV);
#ifdef WSAASYNC
		get_no = item_get_number_to_index(tpMailBox, download_get_no);
		if (get_no != -1) {
			tpMailItem = *(tpMailBox->tpMailItem + get_no);
			if (tpMailItem != NULL && tpMailItem->Size != NULL) {
#ifdef UNICODE
				char *tmp = alloc_tchar_to_char(tpMailItem->Size);
				if (tmp != NULL) {
					mail_size = a2i(tmp);
					mem_free(&tmp);
				}
#else
				mail_size = a2i(tpMailItem->Size);
#endif
			}
		}
#endif
		receiving_data = TRUE;
		recvlen = 0;
		recvcnt = REDRAWCNT;
		GetLocalTime(&recv_clock);
		return POP_RETR;
	}

	// RETR‚ÌI‚í‚è‚Å‚Í‚È‚¢ê‡
	if (*buf != '.' || *(buf + 1) != '\0') {
		// óMƒf[ƒ^‚ğ•Û‘¶‚·‚é
		if (mail_buf_set(buf, buflen) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		recvlen += buflen;
		recvcnt++;
		if (recvcnt > REDRAWCNT) {
			recvcnt = 0;
			SetStatusRecvLen(hWnd, recvlen, mail_size, STR_STATUS_SOCKINFO_RECV);
		}
		return POP_RETR;
	}
	SetSocStatusTextT(hWnd, STR_STATUS_RECVDONE);

	get_no = item_get_number_to_index(tpMailBox, download_get_no);
	if (get_no == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		mem_free(&tpMailBox->LastMessageId);
		tpMailBox->LastMessageId = NULL;
		return POP_ERR;
	}
	tpMailItem = *(tpMailBox->tpMailItem + get_no);
	if (tpMailItem == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	// ƒƒbƒZ[ƒWID‚Å—v‹‚µ‚½ƒ[ƒ‹‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN‚·‚é
	if (check_message_id(mail_buf, tpMailItem, ErrStr, tpMailBox) == FALSE) {
		return POP_ERR;
	}
	if (soc == -1) {
		// salvaging: check for minimal headers
		char *p;
		for (p = mail_buf; *p != '\0'; p++) {
			if (str_cmp_ni(p, "From:", 5) == 0 || str_cmp_ni(p, "Subject:", 8) == 0) {
				break;
			}
		}
		if (*p == '\0') {
			return POP_ERR;
		}
		// salvaging: did we get more than last time?
		if (tpMailItem->Body != NULL && strlen(tpMailItem->Body) > mail_buf_len) {
			return POP_ERR;
		}
		st = ICON_ERROR;
	}

	if (op.SocLog > 1) {
		pop_log_download_rate();
	}

	// convert mail_buf to message structure
	item_mail_to_item(tpMailItem, &mail_buf, -1, MAIL2ITEM_RETR, st, tpMailBox);
	if (mail_buf == NULL) {
		mail_buf_size = 0;
	} else {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	tpMailBox->NeedsSave |= MAILITEMS_CHANGED;

	if (ShowFlag == TRUE) {
		// ƒŠƒXƒgƒrƒ…[‚ÌXV
		i = ListView_GetMemToItem(mListView, tpMailItem);
		if (i != -1) {
			int state = ListView_ComputeState(tpMailItem->Priority, tpMailItem->Multipart);
			ListView_SetItemState(mListView, i, INDEXTOSTATEIMAGEMASK(state),
				LVIS_CUT | LVIS_STATEIMAGEMASK);
			ListView_RedrawItems(mListView, i, i);
			UpdateWindow(mListView);
			SetItemCntStatusText(tpMailBox, FALSE, FALSE);
		}
	}
	if (hViewWnd != NULL) {
#ifndef WSAASYNC
		if (ViewReopen == TRUE) {
			SendMessage(hViewWnd, WM_MODFYMESSAGE, 0, (LPARAM)tpMailItem);
			ViewReopen = FALSE;
		}
#endif
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
	if (soc == -1) {
		return POP_QUIT;
	}

	get_no = item_get_next_download_mark(tpMailBox, -1, &download_get_no);
	if (get_no == -1) {
		if (ServerDelete != FALSE) {
			get_no = item_get_next_delete_mark(tpMailBox, TRUE, -1, &delete_get_no);
		}
		if (get_no == -1) {
			return POP_QUIT;
		}
		// íœƒ[ƒ‹‚ÌŠm”FƒRƒ}ƒ“ƒh(UIDL)‚ğ‘—M
		return exec_send_check_command(hWnd, soc, get_no, ErrStr, tpMailBox);
	}
	tpMailItem = *(tpMailBox->tpMailItem + get_no);
	if (tpMailItem == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	size = (tpMailItem->Size == NULL) ? 0 : _ttoi(tpMailItem->Size);

	// Ÿ‚Ìƒwƒbƒ_‚Ìæ“¾
	receiving_data = FALSE;
	if (mail_buf_init((size > 0) ? size : MAIL_BUF_SIZE) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return POP_ERR;
	}
	// ‘S•¶óM‚·‚éƒRƒ}ƒ“ƒh‚ğ‘—M
	if (tpMailBox->NoRETR == 1) {
		return send_command_top(hWnd, soc, download_get_no, ErrStr, (size > 0) ? size : DOWNLOAD_SIZE, POP_RETR);
	}
	if (send_command(hWnd, soc, TEXT(CMD_RETR), download_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_RETR;
}

/*
 * exec_proc_uidl - íœƒ[ƒ‹Šm”F—pUIDL‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int exec_proc_uidl(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem = NULL;
	TCHAR *UIDL = NULL;
	int get_no;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);
	// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_UIDL;
	}
	// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
	if (check_response(buf) == TRUE) {
		UIDL = alloc_char_to_tchar(skip_response(buf));
	} else {
		// UIDL–¢ƒTƒ|[ƒg
		disable_uidl = TRUE;
		// íœƒ[ƒ‹‚ÌŠm”FƒRƒ}ƒ“ƒh(TOP)‚ğ‘—M
		return exec_send_check_command(hWnd, soc, -1, ErrStr, tpMailBox);
	}
	get_no = item_get_number_to_index(tpMailBox, delete_get_no);
	if (get_no != -1) {
		tpMailItem = *(tpMailBox->tpMailItem + get_no);
	}
	if (UIDL == NULL || tpMailItem == NULL || tpMailItem->UIDL == NULL) {
		mem_free(&UIDL);
		// íœƒ[ƒ‹‚ÌŠm”FƒRƒ}ƒ“ƒh(TOP)‚ğ‘—M
		return exec_send_check_command(hWnd, soc, -1, ErrStr, tpMailBox);
	}
	// ƒƒbƒZ[ƒWID‚Å—v‹‚µ‚½ƒ[ƒ‹‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN‚·‚é
	if (lstrcmp(tpMailItem->UIDL, UIDL) != 0) {
		mem_free(&UIDL);
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		mem_free(&tpMailBox->LastMessageId);
		tpMailBox->LastMessageId = NULL;
		return POP_ERR;
	}
	mem_free(&UIDL);
	// íœƒRƒ}ƒ“ƒh‚Ì‘—M
	deletes_remaining--;
	if (send_command(hWnd, soc, TEXT(CMD_DELE), delete_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_DELE;
}

/*
 * exec_proc_top - íœƒ[ƒ‹Šm”F—pTOP‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int exec_proc_top(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem;
	int get_no;

	if (receiving_data == FALSE) {
		SetSocStatusText(hWnd, buf);
		// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
		if (*buf == '.' && *(buf + 1) == '\0') {
			return POP_TOP;
		}
		// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
		if (check_response(buf) == FALSE) {
			// íœŠm”F‚ÌTOP‚ÆRETR‚ª—¼•û¸”s‚µ‚½ê‡‚ÍƒGƒ‰[
			if (disable_top == TRUE || tpMailBox->NoRETR == 1) {
				lstrcpy(ErrStr, (tpMailBox->NoRETR == 1) ? STR_ERR_SOCK_TOP : STR_ERR_SOCK_RETR);
				str_cat_n(ErrStr, buf, BUF_SIZE - 1);
				return POP_ERR;
			}
			// íœŠm”F‚ÌTOP‚Å¸”s‚µ‚½‚½‚ßRETR‚ÅíœŠm”F‚ğs‚¤
			disable_top = TRUE;
			// íœƒ[ƒ‹‚ÌŠm”FƒRƒ}ƒ“ƒh(RETR)‚ğ‘—M
			return exec_send_check_command(hWnd, soc, -1, ErrStr, tpMailBox);
		}
		receiving_data = TRUE;
		recvlen = 0;
		recvcnt = REDRAWCNT;
		GetLocalTime(&recv_clock);
		return POP_TOP;
	}

	// TOP‚ÌI‚í‚è‚Å‚Í‚È‚¢ê‡
	if (*buf != '.' || *(buf + 1) != '\0') {
		// óMƒwƒbƒ_‚ğ•Û‘¶‚·‚é
		if (mail_buf_set(buf, buflen) == FALSE) {
			lstrcpy(ErrStr, STR_ERR_MEMALLOC);
			return POP_ERR;
		}
		recvlen += buflen;
		recvcnt++;
		if (recvcnt > REDRAWCNT) {
			recvcnt = 0;
			SetStatusRecvLen(hWnd, recvlen, mail_size, STR_STATUS_SOCKINFO_RECV);
		}
		return POP_TOP;
	}
	SetSocStatusTextT(hWnd, STR_STATUS_RECVDONE);

	get_no = item_get_number_to_index(tpMailBox, delete_get_no);
	if (get_no == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		mem_free(&tpMailBox->LastMessageId);
		tpMailBox->LastMessageId = NULL;
		return POP_ERR;
	}
	tpMailItem = *(tpMailBox->tpMailItem + get_no);
	if (tpMailItem == NULL) {
		lstrcpy(ErrStr, STR_ERR_SOCK_GETITEMINFO);
		return POP_ERR;
	}
	// ƒƒbƒZ[ƒWID‚Å—v‹‚µ‚½ƒ[ƒ‹‚©‚Ç‚¤‚©ƒ`ƒFƒbƒN‚·‚é
	if (check_message_id(mail_buf, tpMailItem, ErrStr, tpMailBox) == FALSE) {
		return POP_ERR;
	}
	// íœƒRƒ}ƒ“ƒh‚Ì‘—M
	deletes_remaining--;
	if (send_command(hWnd, soc, TEXT(CMD_DELE), delete_get_no, ErrStr) == FALSE) {
		return POP_ERR;
	}
	return POP_DELE;
}

/*
 * exec_proc_dele - DELE‚ÌƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
 */
static int exec_proc_dele(HWND hWnd, SOCKET soc, char *buf, int buflen, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	MAILITEM *tpMailItem;
	int i, j;
	int get_no, del_stop;

	if (op.SocLog > 0) SetSocStatusText(hWnd, buf);
	// ‘OƒRƒ}ƒ“ƒhŒ‹‰Ê‚É'.'‚ª•t‚¢‚Ä‚¢‚éê‡‚ÍƒXƒLƒbƒv‚·‚é
	if (*buf == '.' && *(buf + 1) == '\0') {
		return POP_DELE;
	}
	// ƒŒƒXƒ|ƒ“ƒX‚Ì‰ğÍ
	if (check_response(buf) == FALSE) {
		lstrcpy(ErrStr, STR_ERR_SOCK_DELE);
		str_cat_n(ErrStr, buf, BUF_SIZE - 1);
		send_buf(soc, CMD_RSET"\r\n");
		return POP_ERR;
	}

	get_no = item_get_number_to_index(tpMailBox, delete_get_no);
	if (get_no == -1) {
		lstrcpy(ErrStr, STR_ERR_SOCK_MAILSYNC);
		mem_free(&tpMailBox->LastMessageId);
		tpMailBox->LastMessageId = NULL;
		return POP_ERR;
	}
	
	del_stop = -1;
	get_no = item_get_next_delete_mark(tpMailBox, TRUE, get_no, &delete_get_no);
	if (get_no != -1) {
		if (deletes_remaining > 0) {
#ifndef _WIN32_WCE
			if (ServerDelete == ID_MENUITEM_DELETE_ALL_SERVER) {
				deletes_remaining--;
				if (send_command(hWnd, soc, TEXT(CMD_DELE), delete_get_no, ErrStr) == FALSE) {
					return POP_ERR;
				}
				return POP_DELE;
			}
#endif
			return exec_send_check_command(hWnd, soc, get_no, ErrStr, tpMailBox);
		} else if (op.SocLog > 1) {
			log_save_a("DELE stopped by DeletePerUpdateLimit\r\n");
			del_stop = get_no;
		}
	}

	// remove deleted messages from ListView
	deletes_remaining = (op.DeletePerUpdateLimit > 0) ? op.DeletePerUpdateLimit : 65535;
	if (ShowFlag == TRUE) {
		ListView_SetRedraw(mListView, FALSE);
		while ((get_no = ListView_GetNextDeleteItem(mListView, -1)) != -1) {
			if (--deletes_remaining < 0) {
				break;
			}
			ListView_DeleteItem(mListView, get_no);
		}
		ListView_SetRedraw(mListView, TRUE);
	}
	// remove deleted messages from Mailbox
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (i == del_stop) {
			break;
		}
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL || tpMailItem->Mark != ICON_DEL) {
			continue;
		}
		if (tpMailItem->Size != NULL) {
#ifdef UNICODE
			tpMailBox->MailSize -= a2i_t(tpMailItem->Size);
#else
			tpMailBox->MailSize -= a2i(tpMailItem->Size);
#endif
		}
		item_free((tpMailBox->tpMailItem + i), 1);

		// íœ‚µ‚½ƒ[ƒ‹‚æ‚èŒã‚ë‚Ìƒ[ƒ‹‚Ì”Ô†‚ğŒ¸‚ç‚·
		for (j = i + 1; j < tpMailBox->MailItemCnt; j++) {
			tpMailItem = *(tpMailBox->tpMailItem + j);
			if (tpMailItem != NULL) {
				tpMailItem->No--;
			}
		}
		tpMailBox->MailCnt--;
		if (tpMailBox->MailCnt == 0) {
			tpMailBox->MailSize = 0;
		} else if (tpMailBox->MailSize < 0) {
			tpMailBox->MailSize = 0;
		}
		tpMailBox->LastNo--;
	}
	// ÅŒã‚ÌƒƒbƒZ[ƒW‚ªíœ‚³‚ê‚Ä‚¢‚éê‡‚Íˆê——æ“¾—pƒƒbƒZ[ƒWID‚ğ•ÏX‚·‚é
	if (*(tpMailBox->tpMailItem + tpMailBox->MailItemCnt - 1) == NULL) {
		tpMailItem = NULL;
		for (i = 0; i < tpMailBox->MailItemCnt; i++) {
			if (*(tpMailBox->tpMailItem + i) == NULL) {
				continue;
			}
			if (tpMailItem == NULL || tpMailItem->No < (*(tpMailBox->tpMailItem + i))->No) {
				tpMailItem = *(tpMailBox->tpMailItem + i);
			}
		}
		if (tpMailItem != NULL) {
			mem_free(&tpMailBox->LastMessageId);
			tpMailBox->LastMessageId = alloc_tchar_to_char(tpMailItem->MessageID);
			tpMailBox->LastNo = tpMailItem->No;
		}
	}
	item_resize_mailbox(tpMailBox, FALSE);
	SetItemCntStatusText(tpMailBox, FALSE, FALSE);
	return POP_QUIT;
}

/*
 * pop3_list_proc - ƒ[ƒ‹ˆê——æ“¾‚Ìˆ— (V’…ƒ`ƒFƒbƒN)
 */
BOOL pop3_list_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	switch (command_status) {
	case POP_START:
	case POP_STARTTLS:
	case POP_USER:
	case POP_PASS:
		command_status = login_proc(hWnd, soc, buf, len, ErrStr, tpMailBox);
		if (command_status == POP_LOGIN) {
			if (PopBeforeSmtpFlag == TRUE) {
				command_status = POP_QUIT;
			} else {
				SetSocStatusTextT(hWnd, TEXT(CMD_STAT));
				if (send_buf(soc, CMD_STAT"\r\n") == -1) {
					lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
					return FALSE;
				}
				command_status = POP_STAT;
			}
		}
		break;

	case POP_STAT:
		DateAdd(NULL, NULL);	// ƒ^ƒCƒ€ƒ][ƒ“‚Ì‰Šú‰»
		command_status = list_proc_stat(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_LIST:
		command_status = list_proc_list(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_UIDL_ALL:
		command_status = list_proc_uidl_all(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_UIDL_CHECK:
		command_status = list_proc_uidl_check(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_UIDL:
		command_status = list_proc_uidl_set(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_TOP:
	case POP_RETR:
		command_status = list_proc_top(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_QUIT:
		if (str_cmp_ni(buf, "+OK", 3) == 0 || str_cmp_ni(buf, "-ERR", 3) == 0) {
			SetSocStatusText(hWnd, buf);
		}
		return TRUE;
	}

	switch (command_status) {
	case POP_ERR:
		item_resize_mailbox(tpMailBox, TRUE); // and set LastNo
		send_buf(soc, CMD_QUIT"\r\n");
		return FALSE;

	case POP_QUIT:
		item_resize_mailbox(tpMailBox, TRUE); // and set LastNo
		SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
		if (send_buf(soc, CMD_QUIT"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		break;
	}
	return TRUE;
}

/*
 * pop3_exec_proc - RETR‚ÆDELE‚Ìˆ— (Às)
 */
BOOL pop3_exec_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	switch (command_status) {
	case POP_START:
	case POP_STARTTLS:
	case POP_USER:
	case POP_PASS:
		command_status = login_proc(hWnd, soc, buf, len, ErrStr, tpMailBox);
		if (command_status == POP_LOGIN) {
			deletes_remaining = (op.DeletePerUpdateLimit > 0) ? op.DeletePerUpdateLimit : 65535;
#ifndef _WIN32_WCE
			if (ServerDelete == ID_MENUITEM_DELETE_ALL_SERVER) {
				SetSocStatusTextT(hWnd, TEXT(CMD_STAT));
				if (send_buf(soc, CMD_STAT"\r\n") == -1) {
					lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
					return FALSE;
				}
				command_status = POP_STAT;
			} else
#endif
			command_status = exec_proc_init(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		}
		break;

	case POP_STAT:
		command_status = exec_proc_stat(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_RETR:
		command_status = exec_proc_retr(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_UIDL:
		command_status = exec_proc_uidl(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_TOP:
		command_status = exec_proc_top(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_DELE:
		command_status = exec_proc_dele(hWnd, soc, buf, len, ErrStr, tpMailBox, ShowFlag);
		break;

	case POP_QUIT:
		if (str_cmp_ni(buf, "+OK", 3) == 0 || str_cmp_ni(buf, "-ERR", 3) == 0) {
			SetSocStatusText(hWnd, buf);
		}
		return TRUE;
	}

	switch (command_status) {
	case POP_ERR:
		send_buf(soc, CMD_QUIT"\r\n");
		return FALSE;

	case POP_QUIT:
		SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
		if (send_buf(soc, CMD_QUIT"\r\n") == -1) {
			lstrcpy(ErrStr, STR_ERR_SOCK_SEND);
			return FALSE;
		}
		break;
	}
	return TRUE;
}

/*
 * pop3_salvage_buffer - when server disconnects, preserve what's been received
 */
BOOL pop3_salvage_buffer(HWND hWnd, MAILBOX *tpMailBox, BOOL ShowFlag)
{
	BOOL ret = FALSE;
	if (receiving_data == TRUE && mail_buf_len > 10) {
		TCHAR ErrStr[BUF_SIZE] = TEXT("");
		char end[2] = ".";
		int salvage = POP_ERR;
		if (op.SocLog > 1) log_save_a(STR_MSG_SALVAGING);
		if (command_status == POP_RETR) {
			salvage = exec_proc_retr(hWnd, -1, end, 1, ErrStr, tpMailBox, ShowFlag);
		} else if (command_status == POP_TOP) {
			salvage = list_proc_top(hWnd, -1, end, 1, ErrStr, tpMailBox, ShowFlag);
		}
		if (salvage == POP_QUIT) {
			ret = TRUE;
		}
		if (op.SocLog > 1 && *ErrStr != TEXT('\0')) {
			if (lstrlen(ErrStr) > BUF_SIZE - 3) {
				*(ErrStr + BUF_SIZE - 3) = TEXT('\0');
			}
			str_join_t(ErrStr, ErrStr, TEXT("\r\n"), (TCHAR *)-1);
			log_save(ErrStr);
		}
	}

	return ret;
}

/*
 * pop_log_download_rate - log bytes per second
 */
static void pop_log_download_rate() {
	TCHAR msg[MSG_SIZE];
	SYSTEMTIME st;
	TCHAR *units = TEXT("Bytes");
	int diff, diffms, rate = 0;
	GetLocalTime(&st);
	if (st.wDay != recv_clock.wDay) st.wHour += 24;
	diff = 60*(st.wHour - recv_clock.wHour
		+ 60*(st.wMinute - recv_clock.wMinute))
		+ st.wSecond - recv_clock.wSecond;
	diffms = diff * 1000 + (st.wMilliseconds - recv_clock.wMilliseconds);
	if (recvlen > 102400) {
		recvlen /= 1024;
		units = TEXT("KB");
	}
	if (diffms > 0) {
		rate = (recvlen * 1000) / diffms;
		wsprintf(msg, TEXT("%d %s received in %d seconds (%d %s/s)\r\n"),
			recvlen, units, diff, rate, units);
	} else {
		wsprintf(msg, TEXT("%d %s received in less than a second\r\n"),
			recvlen, units);
	}
	log_save(msg);
}

/*
 * pop3_free - POP3î•ñ‚Ì‰ğ•ú
 */
void pop3_free(void)
{
	if (mail_buf != NULL) {
		mem_free(&mail_buf);
		mail_buf = NULL;
	}
	if (last_response != NULL) {
		mem_free(&last_response);
		last_response = NULL;
	}
	uidl_free();
}
/* End of source */
