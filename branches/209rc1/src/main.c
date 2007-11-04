/*
 * nPOP
 *
 * main.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2007 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopsupport.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#include "Font.h"
#include "Charset.h"
#ifdef USE_NEDIT
#include "nEdit.h"
#endif

/* Define */
#define WM_TRAY_NOTIFY			(WM_APP + 100)		// タスクトレイ
#define WM_FINDMAILBOX			(WM_APP + 101)
#define WM_RAS_START			(WM_APP + 400)

#define ID_MENU					(WM_APP + 102)		//Control ID
#define IDC_CB					2000
#define IDC_TB					2001

#define ID_MAILITEM_OPEN		(WM_APP + 300)		//ID

#define ID_RECV_TIMER			1					//for mail Open Timer ID
#define ID_SMTP_TIMER			2
#define ID_SMTP_ONE_TIMER		3
#define ID_CHECK_TIMER			4
#define ID_EXEC_TIMER			5
#define ID_AUTOCHECK_TIMER		6
#define ID_TIMEOUT_TIMER		7
#define ID_NEWMAIL_TIMER		8

#define RECVTIME				1					//Timer interval
#define SMTPTIME				100
#define CHECKTIME				100
#define AUTOCHECKTIME			60000
#define TIMEOUTTIME				1000
#define TIMEOUT_QUIT_WPARAM		2007

#define TRAY_ID					100					// タスクトレイID

#define CMD_RSET				"RSET"
#define CMD_QUIT				"QUIT"
 
// Notification Message to external clients
UINT nBroadcastMsg = 0;
#define BROADCAST_STRING		TEXT("NPOP_MESSAGE")

/* Global Variables */
HINSTANCE hInst;							// Local copy of hInstance
TCHAR *AppDir = NULL;						// アプリケーションパス
TCHAR *DataDir = NULL;						// データ保存先のパス
TCHAR *IniFile = NULL;						// ini file specified by /y:
TCHAR *g_Pass = NULL;						// 一時パスワード
int gPassSt;								// 一時パスワード保存フラグ
int gAddressDialogResource = IDD_DIALOG_ADDRESS;
static TCHAR *CmdLine = NULL;				// コマンドライン
static TCHAR *InitialAccount = NULL;
BOOL gSendAndQuit = FALSE;
BOOL gCheckAndQuit = FALSE;
BOOL first_start;							// 初回起動フラグ
BOOL SaveBoxesLoaded = FALSE;
BOOL PPCFlag;								// PsPCフラグ
#ifndef _WIN32_WCE
static int confirm_flag;					// 認証フラグ
#endif

HWND MainWnd;								// メインウィンドウのハンドル
HWND FocusWnd;								// フォーカスを持つウィンドウのハンドル
HFONT hListFont;							// ListViewのフォント
HFONT hViewFont;							// 表示のフォント
int font_charset;
static HICON TrayIcon_Main;					// タスクトレイアイコン (待機)
static HICON TrayIcon_Check;				// タスクトレイアイコン (チェック中)
static HICON TrayIcon_Mail;					// タスクトレイアイコン (新着あり)
BOOL NewMail_Flag;							// タスクトレイアイコン用新着フラグ
static HMENU hPOPUP;						// タスクトレイアイコン用のポップアップメニュー
static HANDLE hAccel, hViewAccel, hEditAccel;	// アクセラレータのハンドル
#ifdef _WIN32_WCE_PPC
HWND hMainToolBar;							// ツールバー (PocketPC)
#endif
#ifdef _WIN32_WCE_LAGENDA
static HWND hCSOBar;						// CSOバー (l'agenda)
static RECT wnd_size;						// 初期ウィンドウサイズ (l'agenda)
static int g_menu_height;					// メニューの高さ (l'agenda)
HMENU hMainMenu;							// ウィンドウメニューのハンドル (l'agenda)
#endif
int MailMenuPos;							// メニュー位置

static WNDPROC ListViewWindowProcedure;		// サブクラス用プロシージャ(ListView)
int LvSortFlag;								// ListViewのソートフラグ
BOOL EndThreadSortFlag;						// 通信終了時の自動ソートフラグ(スレッド表示用)
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
static BOOL SelMode;						// 選択モード (PocketPC, l'agenda)
#endif

MAILBOX *MailBox = NULL;					//which Mailbox
ADDRESSBOOK *AddressBook = NULL;			//Address register
int MailBoxCnt = 2;							//Mailbox several

int SelBox;									// 選択中のメールボックス
int RecvBox;								// 送受信中のメールボックス
static int CheckBox;						// チェック中のメールボックス

SOCKET g_soc = -1;							// ソケット
BOOL gSockFlag;								// 通信中フラグ
BOOL GetHostFlag;							// ホスト名解決中フラグ
int NewMailCnt = -1;								// 新着メール数
BOOL ShowMsgFlag;							// 新着有りのメッセージ表示中
static BOOL ShowError = FALSE;				//During error message indicating
BOOL AutoCheckFlag = FALSE;					//Automatic check
BOOL PopBeforeSmtpFlag = FALSE;				//POP before SMTP
BOOL KeyShowHeader;							//Is by the key the transitory header indicatory flag
static BOOL AllCheck = FALSE;				//which While going around
BOOL ExecFlag = FALSE;				//While executing
BOOL ServerDelete = TRUE;
static BOOL ExecCheckFlag = FALSE;			//Check decision
static int AutoCheckCnt = 0;				//of check after the executing With fraction count
static int SmtpWait = 0;					//pop before SMTP to start of automatic operation check waiting after the certifying (milli-second)
static MAILITEM *wkSendMailItem;			//Mail item


typedef BOOL (*PPROC)(HWND, SOCKET, char*, int, TCHAR*, MAILBOX*, BOOL);
static PPROC command_proc;					// 送受信プロシージャ
int command_status;							// 送受信コマンドステータス (POP_, SMTP_)

HWND g_hwndTimedOwner;
BOOL g_bTimedOut;

// 外部参照
extern OPTION op;

extern TCHAR *FindStr;						//Searching character string
extern HWND hViewWnd;
extern HWND hEditWnd;
extern HWND MsgWnd;							//Mail arrival message

// RAS
extern UINT WM_RASEVENT;
extern BOOL RasLoop;
extern HANDLE hEvent;

/* Local Function Prototypes */
static BOOL GetAppPath(HINSTANCE hinst, TCHAR *lpCmdLine);
#ifndef _WIN32_WCE
static BOOL ConfirmPass(HWND hWnd, TCHAR *ps);
#endif
static BOOL TrayMessage(HWND hWnd, DWORD dwMessage, UINT uID, HICON hIcon, TCHAR *pszTip);
static void SetTrayIcon(HWND hWnd, HICON hIcon, TCHAR *buf);
static void FreeAllMailBox(void);
static void CloseViewWindow(int Flag);
static LRESULT CALLBACK SubClassListViewProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);
static void SetListViewSubClass(HWND hWnd);
static void DelListViewSubClass(HWND hWnd);
static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam);
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam);
#endif
static LRESULT NotifyProc(HWND hWnd, WPARAM wParam, LPARAM lParam);
static int CreateComboBox(HWND hWnd, int Top);
static BOOL InitWindow(HWND hWnd);
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
static BOOL SaveWindow(HWND hWnd, BOOL SelDir, BOOL PromptSave, BOOL UpdateStatus);
static BOOL EndWindow(HWND hWnd);
static BOOL SendMail(HWND hWnd, MAILITEM *tpMailItem, int end_cmd);
static BOOL RecvMailList(HWND hWnd, int BoxIndex, BOOL SmtpFlag);
static BOOL MailMarkCheck(HWND hWnd, BOOL IsAfterCheck);
static BOOL ExecItem(HWND hWnd, int BoxIndex);
static void ReMessageItem(HWND hWnd, int ReplyFlag);
static void ListDeleteItem(HWND hWnd, BOOL Ask);
static void ListDeleteAttach(HWND hWnd);
static void SetDownloadMark(HWND hWnd, BOOL Flag);
static void SetDeleteMark(HWND hWnd);
static void UnMark(HWND hWnd);
static void SetMailStats(HWND hWnd, int St);
static void EndSocketFunc(HWND hWnd, BOOL DoTimer);
static BOOL CheckEndAutoExec(HWND hWnd, int SocBox, int cnt, BOOL AllFlag);
static void Init_NewMailFlag(HWND hWnd);
static void NewMail_Message(HWND hWnd, int cnt);
static void AutoSave_Mailboxes(HWND hWnd);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL InitApplication(HINSTANCE hInstance);
static HWND InitInstance(HINSTANCE hInstance, int CmdShow);
void CALLBACK MessageBoxTimer(HWND hWnd, UINT uiMsg, UINT idEvent, DWORD dwTime);
static int TimedMessageBox(HWND hWnd, TCHAR *strMsg, TCHAR *strTitle, unsigned int nStyle, DWORD dwTimeout);
static void PlayMarkSound(int mark);

/*
 * GetAppPath - ユーザディレクトリの作成
 */
#ifdef _WIN32_WCE_LAGENDA
int GetUserDiskName(HINSTANCE hInstance, LPTSTR lpDiskName, int nMaxCount);
#endif

static BOOL GetAppPath(HINSTANCE hinst, TCHAR *lpCmdLine)
{
#ifdef _WIN32_WCE_LAGENDA
	TCHAR buf[BUF_SIZE];

	GetUserDiskName(hinst, buf, BUF_SIZE - 1);
	lstrcat(buf, TEXT("\\nPOP\\"));

	AppDir = alloc_copy_t(buf);
	if (AppDir == NULL) {
		return FALSE;
	}
	CreateDirectory(AppDir, NULL);
#else
	TCHAR *p, *r;
	TCHAR name[BUF_SIZE];

	if (lpCmdLine != NULL && *lpCmdLine != TEXT('\0')) {

		for (p = lpCmdLine; *p == TEXT(' '); p++); // remove spaces
		// command-line options should preceed any mailto: arguments
		// /y:inifile
		// /a:account
		// /s - send and quit
		// /q - quit after running check all
		while (*p == TEXT('/')) {
			BOOL slash_y = FALSE, slash_a = FALSE;
			p++;
			if (*p == TEXT('s') || *p == TEXT('S')) {
				gSendAndQuit = TRUE;
				p++;
				while (*p == TEXT(' ')) p++;
				continue;
			} else if (*p == TEXT('q') || *p == TEXT('Q')) {
				gCheckAndQuit = TRUE;
				p++;
				while (*p == TEXT(' ')) p++;
				continue;
			} else if (*(p+1) != TEXT(':')) {
				break;
			}
			
			if (*p == TEXT('y') || *p == TEXT('Y')) {
				slash_y = TRUE;
			} else if (*p == TEXT('a') || *p == TEXT('A')) {
				slash_a = TRUE;
			} else {
				// unknown switch
			}

			if (slash_y || slash_a) {
				p += 2;
				if (*p == TEXT('\"')) { // Collect everything between double quotes
					p++;
					for (r = p; *r != TEXT('\0') && *r != TEXT('\"'); r++);
				} else { // otherwise collect up to the end of the word
					for (r = p; *r != TEXT('\0') && *r != TEXT(' '); r++);
				}
			}
			if (slash_y) {
#ifdef _WIN32_WCE
				WIN32_FIND_DATA FindData;
				HANDLE hFindFile;
				TCHAR *q, *s;
#else
				DWORD ret;
#endif
				BOOL Found = TRUE;
				int len;

				len = ((r - p + 1) >= BUF_SIZE) ? BUF_SIZE : (r - p + 1);
#ifdef _WIN32_WCE
				if (*p != TEXT('\\') && *p != TEXT('/')) {
					GetModuleFileName(hinst, name, BUF_SIZE - 1);
					for (q = s = name; *q != TEXT('\0'); q++) {
						if (*q == TEXT('\\') || *q == TEXT('/')) {
							s = q + 1;
						}
					}
					if ((len + (s - name)) > BUF_SIZE) {
						len = BUF_SIZE + (s - name);
					}
				} else {
					s = name;
				}
				str_cpy_n_t(s, p, len);
				if ((hFindFile = FindFirstFile(name, &FindData)) == INVALID_HANDLE_VALUE) {
					Found = FALSE;
				}
				FindClose(hFindFile);
#else
				str_cpy_n_t(name, p, len);
				{
					TCHAR fullname[BUF_SIZE];
					if ((ret = GetFullPathName(name, BUF_SIZE, fullname, NULL)) == 0) {
						Found = FALSE;
					}
				}
#endif
				if (Found == FALSE) {
					TCHAR buf[BUF_SIZE];
					wsprintf(buf, STR_ERR_INIFILE, name);
					return FALSE;
				}
				IniFile = alloc_copy_t(name);
			} else if (slash_a) {
				int len = (r - p + 1);
				InitialAccount = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
				str_cpy_n_t(InitialAccount, p, len);
			}
			if (*r == TEXT('\"')) {
				r++;
			}
			for (p = r; *p == TEXT(' '); p++); // remove spaces
		}
		if (*p != TEXT('\0')) {
#ifdef UNICODE
			CmdLine = alloc_copy_t(p);
#else
			CmdLine = alloc_char_to_tchar(p);
#endif
		}
	}

	if (IniFile == NULL) {
		AppDir = (TCHAR *)mem_calloc(sizeof(TCHAR) * BUF_SIZE);
		if (AppDir == NULL) {
			return FALSE;
		}
		//Pass of application acquisition
		GetModuleFileName(hinst, AppDir, BUF_SIZE - 1);
	} else {
		AppDir = alloc_copy_t(IniFile);
		if (AppDir == NULL) {
			return FALSE;
		}
	}
	for (p = r = AppDir; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
			continue;
		}
#endif	// UNICODE
		if (*p == TEXT('\\') || *p == TEXT('/')) {
			r = p;
		}
	}
	*r = TEXT('\0');

	lstrcat(AppDir, TEXT("\\"));
#endif	// _WIN32_WCE_LAGENDA

	return TRUE;
}


/*
 * CommandLine - now just a stub; most options handled in GetAppPath
 */
static BOOL CommandLine(HWND hWnd, TCHAR *buf)
{
	TCHAR *p;

	if (buf == NULL || *buf == TEXT('\0')) {
		return FALSE;
	}
	for (p = buf; *p == TEXT(' ') || *p == TEXT('\t'); p++);
	return ((*p == TEXT('\0')) ? FALSE : TRUE);
}

/*
 * ConfirmPass - パスワードの確認
 */
#ifndef _WIN32_WCE
static BOOL ConfirmPass(HWND hWnd, TCHAR *ps)
{
	// ShowPass
	gPassSt = 0;
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), NULL, InputPassProc,
		(LPARAM)STR_TITLE_SHOWPASSWORD) == FALSE) {
		return FALSE;
	}
	confirm_flag = 2;
	return TRUE;
}
#endif

/*
 * TrayMessage - タスクトレイのアイコンの設定
 */
static BOOL TrayMessage(HWND hWnd, DWORD dwMessage, UINT uID, HICON hIcon, TCHAR *pszTip)
{
	NOTIFYICONDATA tnd;

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = hWnd;
	tnd.uID	= uID;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage = WM_TRAY_NOTIFY;
	tnd.hIcon = hIcon;
	lstrcpy(tnd.szTip, (pszTip == NULL) ? TEXT("") : pszTip);
	return Shell_NotifyIcon(dwMessage, &tnd);
}

/*
 * SetTrayIcon - タスクトレイにアイコンを設定する
 */
static void SetTrayIcon(HWND hWnd, HICON hIcon, TCHAR *buf)
{
	if (op.ShowTrayIcon != 1 || hIcon == NULL) {
		return;
	}
	if (TrayMessage(hWnd, NIM_MODIFY, TRAY_ID, hIcon, buf) == FALSE) {
		//When it cannot modify, it adds the
		TrayMessage(hWnd, NIM_ADD, TRAY_ID, hIcon, buf);
	}
}

/*
 * SwitchCursor - カーソルの状態の切り替え
 */
void SwitchCursor(const BOOL Flag)
{
	static HCURSOR hCursor = NULL;

	if (Flag == FALSE) {
#ifdef _WIN32_WCE
		if (hCursor == NULL) {
			SetCursor(hCursor);
		}
#endif
		hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	} else {
#ifdef _WIN32_WCE
		SetCursor(hCursor);
		hCursor = NULL;
#else
		SetCursor(LoadCursor(NULL, IDC_ARROW));
#endif
	}
}

/*
 * _SetForegroundWindow - ウィンドウをアクティブにする
 */
#ifndef _WIN32_WCE
BOOL _SetForegroundWindow(const HWND hWnd)
{
#ifndef SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT		  0x2000
#endif
#ifndef SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT		  0x2001
#endif
	int nTargetID, nForegroundID;
	UINT nTimeout;
	BOOL ret;

	nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	nTargetID = GetWindowThreadProcessId(hWnd, NULL);
	AttachThreadInput(nTargetID, nForegroundID, TRUE);

	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &nTimeout, 0);
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)0, 0);

	ret = SetForegroundWindow(hWnd);

	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)nTimeout, 0);

	AttachThreadInput(nTargetID, nForegroundID, FALSE);
	return ret;
}
#endif

/*
 * SetStatusTextT - ステータスの表示 (TCHAR)
 */
void SetStatusTextT(HWND hWnd, TCHAR *buf, int Part)
{
	if (Part == 0) {
		// GJC dynamically resize status bar
		int Width[2];
		int len = lstrlen(buf) * op.StatusBarCharWidth; // approx pixel count
		if (len > 220) len = 220;
		Width[0] = len;
		Width[1] = -1;
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETPARTS,
			(WPARAM)(sizeof(Width) / sizeof(int)), (LPARAM)((LPINT)Width));
	}
#ifdef _WIN32_WCE_PPC
	SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)Part | SBT_NOBORDERS, (LPARAM)buf);
#else
	SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)Part, (LPARAM)buf);
#endif
}

/*
 * SetSocStatusTextT - ステータスの表示 (TCHAR)
 */
void SetSocStatusTextT(HWND hWnd, TCHAR *buf)
{
	TCHAR *st_buf;

	if (PPCFlag == FALSE && RecvBox >= MAILBOX_USER && (MailBox + RecvBox)->Name != NULL) {
		st_buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen((MailBox + RecvBox)->Name) + lstrlen(buf) + 4));
		if (st_buf == NULL) {
			return;
		}
		str_join_t(st_buf, TEXT("["), (MailBox + RecvBox)->Name, TEXT("] "), buf, (TCHAR *)-1);
#ifdef _WIN32_WCE_PPC
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1 | SBT_NOBORDERS, (LPARAM)st_buf);
#else
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1, (LPARAM)st_buf);
#endif
		if (op.SocLog > 0) log_save(AppDir, LOG_FILE, st_buf);
		mem_free(&st_buf);
	} else {
#ifdef _WIN32_WCE_PPC
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1 | SBT_NOBORDERS, (LPARAM)buf);
#else
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1, (LPARAM)buf);
#endif
		if (op.SocLog > 0) log_save(AppDir, LOG_FILE, buf);
	}
}

/*
 * SetSocStatusText - ステータスの表示
 */
#ifdef UNICODE
void SetSocStatusText(HWND hWnd, char *buf)
{
	TCHAR *wbuf;

	wbuf = alloc_char_to_tchar(buf);
	if (wbuf == NULL) {
		return;
	}
	SetSocStatusTextT(hWnd, wbuf);
	mem_free(&wbuf);
}
#endif

void FormatNumberString(long num, TCHAR *fmtstring, TCHAR *decpt, TCHAR *ret)
{
	TCHAR tmp[20];
	long div;

	div = num/1024;
	if (num < 10240) { // 1.23 MB
		wsprintf(tmp, TEXT("%d%s%2.2d"), div, decpt, (100*(num-div*1024))/1024);
		wsprintf(ret, fmtstring, tmp);
	} else if (num < 102400) { // 12.3 MB
		wsprintf(tmp, TEXT("%d%s%1.1d"), div, decpt, (10*(num-div*1024))/1024);
		wsprintf(ret, fmtstring, tmp);
	} else { // 123 MB
		wsprintf(tmp, TEXT("%d"), div);
		wsprintf(ret, fmtstring, tmp);
	}
}

/*
 * SetItemCntStatusText - アイテム数の表示
 */
void SetItemCntStatusText(HWND hWnd, MAILBOX *tpViewMailBox, BOOL bNotify)
{
	MAILBOX *tpMailBox;
	MAILITEM *tpMailItem;
	TCHAR wbuf[BUF_SIZE], dtmp[20], decpt[5];
	long dsize;
	int ItemCnt;
	int NewCnt = 0, UnreadCnt = 0, UnsentCnt = 0;
	int i;

	tpMailBox = (MailBox + SelBox);
	if (tpMailBox == NULL || (tpViewMailBox != NULL && tpViewMailBox != tpMailBox)) {
		return;
	}

	ItemCnt = ListView_GetItemCount(GetDlgItem(MainWnd, IDC_LISTVIEW));
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, (LPTSTR)decpt, 4);
	dsize = tpMailBox->DiskSize;
	if (dsize < 0) {
		wsprintf(dtmp, STR_STATUS_MAILSIZE_KB, TEXT("?"));
	} else if (dsize < 1000) {
		wsprintf(dtmp, STR_STATUS_MAILSIZE_B, dsize);
	} else {
		if (dsize < 102400) {
			FormatNumberString(dsize, STR_STATUS_MAILSIZE_KB, decpt, dtmp);
		} else {
			dsize /= 1024;
			if (dsize < 102400) {
				FormatNumberString(dsize, STR_STATUS_MAILSIZE_MB, decpt, dtmp);
			} else {
				dsize /= 1024;
				FormatNumberString(dsize, STR_STATUS_MAILSIZE_GB, decpt, dtmp);
			}
		}
	}

	if (SelBox == MAILBOX_SEND || tpMailBox->Type == MAILBOX_TYPE_SAVE) {
		wsprintf(wbuf, STR_STATUS_VIEWINFO, ItemCnt, dtmp);
		SetStatusTextT(MainWnd, wbuf, 0);
		if (g_soc == -1) {
			SetStatusTextT(MainWnd, TEXT(""), 1);
		}
	} else {
#ifndef _WIN32_WCE
		TCHAR stmp[20];
		unsigned int ssize = tpMailBox->MailSize;
		if (ssize < 1000) {
			wsprintf(stmp, STR_STATUS_MAILSIZE_B, ssize);
		} else {
			if (ssize < 102400) {
				FormatNumberString(ssize, STR_STATUS_MAILSIZE_KB, decpt, stmp);
			} else {
				ssize /= 1024;
				if (ssize < 102400) {
					FormatNumberString(ssize, STR_STATUS_MAILSIZE_MB, decpt, stmp);
				} else {
					ssize /= 1024;
					FormatNumberString(ssize, STR_STATUS_MAILSIZE_GB, decpt, stmp);
				}
			}
		}
		wsprintf(wbuf, STR_STATUS_MAILBOXINFO, ItemCnt, dtmp, tpMailBox->MailCnt, stmp);
#else
		wsprintf(wbuf, STR_STATUS_MAILBOXINFO, ItemCnt, tpMailBox->MailCnt);
#endif
		SetStatusTextT(MainWnd, wbuf, 0);
	}

	if (g_soc != -1) {
		return;
	}
	tpMailBox = MailBox + MAILBOX_SEND;
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Mark == ICON_SEND || tpMailItem->Mark == ICON_ERROR) {
			UnsentCnt++;
		}
	}
	if (UnsentCnt > 0) {
		SetMenuStar(MAILBOX_SEND, STR_SENDBOX_NAME, TRUE, (SelBox == MAILBOX_SEND));
	} else {
#ifdef _WIN32_WCE
		unsigned int len;
#else
		int len;
#endif
		len = SendDlgItemMessage(MainWnd, IDC_COMBO, CB_GETLBTEXTLEN, MAILBOX_SEND, 0);
		if (len > lstrlen(STR_SENDBOX_NAME)) {
			SetMenuStar(MAILBOX_SEND, STR_SENDBOX_NAME, FALSE, (SelBox == MAILBOX_SEND));
		}
	}
	if (SelBox == MAILBOX_SEND) {
		wsprintf(wbuf, STR_STATUS_UNSENT, UnsentCnt);
		SetStatusTextT(MainWnd, wbuf, 1);
		return;
	}
	tpMailBox = (MailBox + SelBox);
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->New == TRUE) {
			NewCnt++;
		}
		if (tpMailItem->MailStatus == ICON_MAIL) {
			UnreadCnt++;
		}
	}
	if (UnsentCnt > 0) {
		wsprintf(wbuf, STR_STATUS_MAILINFO_U, NewCnt, UnreadCnt, UnsentCnt);
	} else {
		wsprintf(wbuf, STR_STATUS_MAILINFO, NewCnt, UnreadCnt);
	}
	SetStatusTextT(MainWnd, wbuf, 1);

	if (tpMailBox->NewMail == TRUE && NewCnt == 0) {
		// GJC - remove * from drop-down list
		TCHAR *p;

		p = (tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0'))
			? STR_MAILBOX_NONAME : tpMailBox->Name;

		SendDlgItemMessage(MainWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
		SendDlgItemMessage(MainWnd, IDC_COMBO, CB_INSERTSTRING, SelBox, (LPARAM)p);
		SendDlgItemMessage(MainWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
		tpMailBox->NewMail = FALSE;
		SetUnreadCntTitle(MainWnd, FALSE);
	}

	// Notify programs of new count
	if (bNotify)
	{
#ifdef _WIN32_WCE_PPC
		HWND hPlugin;
		if (hPlugin = findTodayPlugin(TEXT("phoneAlarmMaxCls")))
			PostMessage(hPlugin, nBroadcastMsg, (WPARAM)UnreadCnt, (LPARAM)NewCnt);
#endif
		PostMessage(HWND_BROADCAST, nBroadcastMsg, (WPARAM)UnreadCnt, (LPARAM)NewCnt);
	}
}

/*
 * SetStatusRecvLen - 送受信バイト数の表示
 */
void SetStatusRecvLen(HWND hWnd, int len, TCHAR *msg)
{
	TCHAR wbuf[BUF_SIZE];

	wsprintf(wbuf, STR_STATUS_SOCKINFO, len, msg);
	SetStatusTextT(hWnd, wbuf, 1);
}

/*
 * ErrorMessage - エラーメッセージ
 */
void ErrorMessage(HWND hWnd, TCHAR *buf)
{
	SwitchCursor(TRUE);
	ShowError = TRUE;
	MessageBox(hWnd, buf, STR_TITLE_ERROR, MB_OK | MB_ICONERROR);
	ShowError = FALSE;
}

/*
 * SocketErrorMessage - ソケットのエラーメッセージ
 */
void SocketErrorMessage(HWND hWnd, TCHAR *buf, int BoxIndex)
{
	MAILBOX *tpMailBox;
	TCHAR *Title;
	TCHAR *p;

	tpMailBox = MailBox + BoxIndex;
	RecvBox = -1;
	SetMailMenu(hWnd);
	SwitchCursor(TRUE);

	if (hWnd != NULL) {
		//In status bar information of error indicatory
		SetStatusTextT(hWnd, buf, 1);
	}
	if (op.SocLog > 0) log_save(AppDir, LOG_FILE, buf);
	if (op.SocIgnoreError == 1 && BoxIndex >= MAILBOX_USER) {
		// 受信エラーを無視する設定の場合
		return;
	}
	if (op.SendIgnoreError == 1 && BoxIndex == MAILBOX_SEND) {
		// 送信エラーを無視する設定の場合
		return;
	}
	if ((BoxIndex == MAILBOX_SEND || BoxIndex >= MAILBOX_USER) &&
		tpMailBox->Name != NULL && *tpMailBox->Name != TEXT('\0')) {
		// アカウント名付きのタイトルの作成
		p = Title = (TCHAR *)mem_alloc(
			sizeof(TCHAR) * (lstrlen(STR_TITLE_ERROR TEXT(" - ")) + lstrlen(tpMailBox->Name) + 1));
		if (Title == NULL) {
			p = tpMailBox->Name;
			Title = NULL;
		} else {
			str_join_t(Title, STR_TITLE_ERROR TEXT(" - "), tpMailBox->Name, (TCHAR *)-1);
		}
	} else {
		p = STR_TITLE_ERROR;
		Title = NULL;
	}
	ShowError = TRUE;
	// Message box with account name
	if (op.NoIgnoreErrorTimeout <= 0) {
		MessageBox(hWnd, buf, p, MB_OK | MB_ICONERROR);
	} else {
		TimedMessageBox(hWnd, buf, p, MB_OK | MB_ICONERROR, op.NoIgnoreErrorTimeout);
	}
	ShowError = FALSE;
	mem_free(&Title);
}

/*
 * ErrorSocketEnd - 通信の異常終了時の処理
 */
void ErrorSocketEnd(HWND hWnd, int BoxIndex)
{
	//of error message The socket is closed the
	if (g_soc != -1 && GetHostFlag == FALSE) {
		socket_close(hWnd, g_soc);
	}
	g_soc = -1;
	KillTimer(hWnd, ID_TIMEOUT_TIMER);
	if (BoxIndex == MAILBOX_SEND) {
		smtp_set_error(hWnd);
	}

#ifndef WSAASYNC
	KillTimer(hWnd, ID_RECV_TIMER);
#endif
	if (AllCheck == FALSE) {
		EndSocketFunc(hWnd, FALSE);
		return;
	}

	if (op.SocIgnoreError == 1 && BoxIndex >= MAILBOX_USER) {
		//In case of the setting which ignores reception error the
		return;
	}
	if (op.SendIgnoreError == 1 && BoxIndex == MAILBOX_SEND) {
		//In case of the setting which ignores transmission error the
		return;
	}
	//Stop
	KillTimer(hWnd, ID_SMTP_TIMER);
	KillTimer(hWnd, ID_SMTP_ONE_TIMER);
	KillTimer(hWnd, ID_CHECK_TIMER);
	KillTimer(hWnd, ID_EXEC_TIMER);
	KillTimer(hWnd, ID_RASWAIT_TIMER);
	gSockFlag = FALSE;
	AllCheck = FALSE;
	ExecFlag = FALSE;
	KeyShowHeader = FALSE;
	EndSocketFunc(hWnd, FALSE);
}

/*
 * ShowMenu - マウスの位置にメニューを表示する
 */
int ShowMenu(HWND hWnd, HMENU hMenu, int mpos, int PosFlag, BOOL ReturnFlag)
{
#ifndef _WIN32_WCE_PPC
	HMENU hShowMenu;
#endif
	HWND hListView;
	RECT WndRect;
	RECT ItemRect;
	int i;
	int x = 0, y = 0;
	DWORD ret = 0;
#ifndef _WIN32_WCE
	POINT apos;
#endif

#ifndef _WIN32_WCE_PPC
	_SetForegroundWindow(hWnd);
#endif
	switch (PosFlag) {
	case 0:
		//of round Acquisition
#ifdef _WIN32_WCE
		ret = GetMessagePos();
		x = LOWORD(ret);
		y = HIWORD(ret);
#else
		GetCursorPos((LPPOINT)&apos);
		x = apos.x;
		y = apos.y;
#endif
		break;

	case 1:
		//of mouse position Acquisition
		hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		i = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
		GetWindowRect(hListView, &WndRect);
		if (i == -1) {
			x = WndRect.left;
			y = WndRect.top;
		} else {
			ListView_EnsureVisible(hListView, i, TRUE);
			ListView_GetItemRect(hListView, i, &ItemRect, LVIR_ICON);
			if (ItemRect.left < 0) {
				ItemRect.left = 0;
			}
			if (ItemRect.top < 0) {
				ItemRect.top = 0;
			}
			if (ItemRect.top > (WndRect.bottom - WndRect.top)) {
				ItemRect.top = WndRect.bottom - WndRect.top;
			}
			x = WndRect.left + ItemRect.left + (SICONSIZE / 2);
			y = WndRect.top + ItemRect.top + (SICONSIZE / 2);
		}
		break;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	case 2:
		GetWindowRect(hWnd, &WndRect);
		TrackPopupMenu(GetSubMenu(hMenu, mpos), TPM_BOTTOMALIGN | TPM_RIGHTALIGN,
			0, wnd_size.bottom, 0, hWnd, NULL);
		PostMessage(hWnd, WM_NULL, 0, 0);
		return 0;
#else
	case 2:
		GetWindowRect(hWnd, &WndRect);
		TrackPopupMenu(GetSubMenu(hMenu, mpos), TPM_BOTTOMALIGN | TPM_RIGHTALIGN,
			WndRect.right, WndRect.bottom, 0, hWnd, NULL);
		PostMessage(hWnd, WM_NULL, 0, 0);
		return 0;
#endif
#endif
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	_SetForegroundWindow(hWnd);
	ret = TrackPopupMenu(hMenu,
		TPM_TOPALIGN | TPM_LEFTALIGN | ((ReturnFlag == TRUE) ? TPM_RETURNCMD : 0),
		x, y, 0, hWnd, NULL);
#else
	hShowMenu = GetSubMenu(hMenu, mpos);
	ret = TrackPopupMenu(hShowMenu,
		TPM_TOPALIGN | TPM_LEFTALIGN | ((ReturnFlag == TRUE) ? TPM_RETURNCMD : 0),
		x, y, 0, hWnd, NULL);
#endif
#else
	hShowMenu = GetSubMenu(hMenu, mpos);
	ret = TrackPopupMenu(hShowMenu,
		TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | ((ReturnFlag == TRUE) ? TPM_RETURNCMD : 0),
		x, y, 0, hWnd, NULL);
#endif
	PostMessage(hWnd, WM_NULL, 0, 0);
	return ret;
}

/*
 * SetMailMenu - メニューの活性／非活性の切り替え
 */
int SetMailMenu(HWND hWnd)
{
	HMENU hMenu;
	HWND hToolBar, hListView;
	int SelFlag, SocFlag;
	int RecvBoxFlag, SaveTypeFlag, SendBoxFlag;
	int MoveBoxFlag;
	int i, retval = -1;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE);
	hToolBar = hMainToolBar;
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hMainMenu, 0);
	hToolBar = NULL;
#else
	hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0);
	hToolBar = GetDlgItem(hWnd, IDC_CB);
#endif
#else
	hMenu = GetMenu(hWnd);
	hToolBar = GetDlgItem(hWnd, IDC_TB);
#endif
	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if (hMenu == NULL || hListView == NULL) {
		return -1;
	}

	i = ListView_GetSelectedCount(hListView);
	if (i == 1) {
		MAILITEM *tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, 
			ListView_GetNextItem(hListView, -1, LVNI_SELECTED));
		retval = tpMailItem->Mark;
		if (tpMailItem->New) retval |= 0x10;
		if (tpMailItem->Download) retval |= 0x20;
		if (tpMailItem->Multipart == MULTIPART_ATTACH || tpMailItem->Multipart == MULTIPART_CONTENT) 
			retval |= 0x40;
		if (tpMailItem->Multipart == MULTIPART_HTML) 
			retval |= 0x80;
	}
	SelFlag = (i <= 0) ? 0 : 1;
	SocFlag = (g_soc != -1 || gSockFlag == TRUE) ? 0 : 1;
	RecvBoxFlag = (SelBox == RecvBox) ? 0 : 1;
	SaveTypeFlag = ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) ? 0 : 1;
	SendBoxFlag = (SelBox == MAILBOX_SEND) ? 0 : 1;
	MoveBoxFlag = (MailBoxCnt <= 3) ? 0 : 1;

	EnableMenuItem(hMenu, ID_MENUITEM_SETMAILBOX, !(RecvBoxFlag & SendBoxFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_DELETEMAILBOX, !(RecvBoxFlag & SendBoxFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_LISTINIT, !(SaveTypeFlag & SendBoxFlag));

	EnableMenuItem(hMenu, ID_MENUITEM_MOVEUPMAILBOX, !(SocFlag & SendBoxFlag & MoveBoxFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_MOVEDOWNMAILBOX, !(SocFlag & SendBoxFlag & MoveBoxFlag));

	EnableMenuItem(hMenu, ID_MENUITEM_RAS_CONNECT,
		!(SocFlag & (MailBox + SelBox)->RasMode & !op.EnableLAN));
	EnableMenuItem(hMenu, ID_MENUITEM_RAS_DISCONNECT, op.EnableLAN);

#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hMainMenu, 1);
#endif
	EnableMenuItem(hMenu, ID_MENUITEM_OPEN, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_REMESSEGE, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_ALLREMESSEGE, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_FORWARD, !SelFlag);

	EnableMenuItem(hMenu, ID_MENUITEM_RECV, !(SocFlag & SaveTypeFlag & SendBoxFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_ALLCHECK, !SocFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_EXEC, !(SocFlag & SaveTypeFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_ALLEXEC, !SocFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_STOP, SocFlag | !SaveTypeFlag);
#ifdef _WIN32_WCE_LAGENDA
	CSOBar_SetButtonState(hCSOBar, TRUE, ID_MENUITEM_RECV, 1,
		(SocFlag & SaveTypeFlag & SendBoxFlag) ? CSO_BUTTON_DISP : CSO_BUTTON_GRAYED);
	CSOBar_SetButtonState(hCSOBar, TRUE, ID_MENUITEM_ALLCHECK, 1,
		(SocFlag) ? CSO_BUTTON_DISP : CSO_BUTTON_GRAYED);
	CSOBar_SetButtonState(hCSOBar, TRUE, ID_MENUITEM_EXEC, 1,
		(SocFlag & SaveTypeFlag) ? CSO_BUTTON_DISP : CSO_BUTTON_GRAYED);
	CSOBar_SetButtonState(hCSOBar, TRUE, ID_MENUITEM_ALLEXEC, 1,
		(SocFlag) ? CSO_BUTTON_DISP : CSO_BUTTON_GRAYED);
	CSOBar_SetButtonState(hCSOBar, TRUE, ID_MENUITEM_STOP, 1,
		(!SocFlag) ? CSO_BUTTON_DISP : CSO_BUTTON_GRAYED);
#else
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_RECV,
		(LPARAM)MAKELONG(SocFlag & SaveTypeFlag & SendBoxFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_ALLCHECK, (LPARAM)MAKELONG(SocFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_EXEC,
		(LPARAM)MAKELONG(SocFlag & SaveTypeFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_ALLEXEC, (LPARAM)MAKELONG(SocFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_STOP, (LPARAM)MAKELONG(!SocFlag, 0));
#endif

	EnableMenuItem(hMenu, ID_MENUITEM_DOWNMARK, !(SelFlag & SaveTypeFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	EnableMenuItem(hMenu, ID_MENUITEM_DELMARK, !(SelFlag & SaveTypeFlag & SendBoxFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	EnableMenuItem(hMenu, ID_MENUITEM_UNMARK, !(SelFlag & SaveTypeFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}

	EnableMenuItem(hMenu, ID_MENUITEM_READMAIL, !(SelFlag & SendBoxFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_UNREADMAIL, !(SelFlag & SendBoxFlag));

	EnableMenuItem(hMenu, (SelBox == MAILBOX_SEND) ? ID_MENUITEM_SAVECOPY : ID_MENUITEM_DELATTACH, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_DELETE, !SelFlag);

	for (i=0; i < MailBoxCnt; i++) {
		if (i != SelBox && (MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
			EnableMenuItem(hMenu, ID_MENUITEM_COPY2MBOX+i, !SelFlag);
			EnableMenuItem(hMenu, ID_MENUITEM_MOVE2MBOX+i, !SelFlag);
		}
	}
	EnableMenuItem(hMenu, ID_MENUITEM_COPY2NEW, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_MOVE2NEW, !SelFlag);

	return retval;
}

/*
 * FreeAllMailBox - ウィンドウの終了処理
 */
static void FreeAllMailBox(void)
{
	int i;

	//of item position Release
	addressbook_free(AddressBook);

	//of address register Release
	for (i = 0; i < MailBoxCnt; i++) {
		mailbox_free((MailBox + i));
	}
	mem_free(&MailBox);
}

/*
 * CloseViewWindow - メール表示ウィンドウとメール編集ウィンドウを閉じる
 */
static void CloseViewWindow(int Flag)
{
	HWND fWnd;
	//of all mailboxes The mail indicatory window is closed the
	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_ENDCLOSE, 0, 0);
	}
	//The mail compilation window is closed the
	while ((fWnd = FindWindow(EDIT_WND_CLASS, NULL)) != NULL) {
		SendMessage(fWnd, WM_ENDCLOSE, Flag, 0);
	}
}

/*
 * SubClassListViewProc - サブクラス化したウィンドウプロシージャ
 *	IMEが入った状態でもSpaceKeyを有効にするため
 */
static LRESULT CALLBACK SubClassListViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	LV_HITTESTINFO lvht;
	POINT apos;
#endif
	int i;

	switch (msg) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	case WM_LBUTTONDOWN:
		//Item under mouse acquisition
		apos.x = LOWORD(lParam);
		apos.y = HIWORD(lParam);

		lvht.pt = apos;
		lvht.flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
		lvht.iItem = 0;
		i = ListView_HitTest(hWnd, &lvht);

		if (SelMode == TRUE && GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_SHIFT) >= 0) {
			//Selective mode
			if (i != -1) {
				ListView_SetItemState(hWnd, i,
					LVIS_FOCUSED | (ListView_GetItemState(hWnd, i, LVIS_SELECTED) ^ LVIS_SELECTED),
					LVIS_FOCUSED | LVIS_SELECTED);
			}
			return 0;
		}
		break;
#endif

	case WM_FINDMAILBOX:
		SwitchCursor(FALSE);
		i = mailbox_next_unread(hWnd, SelBox + 1, MailBoxCnt);
		if (i == -1) {
			i = mailbox_next_unread(hWnd, MAILBOX_USER, SelBox);
		}
		if (i == -1) {
			SwitchCursor(TRUE);
			return 0;
		}
		//Space
		mailbox_select(GetParent(hWnd), i);
		SwitchCursor(TRUE);
		return 0;

	case WM_CHAR:
		if ((TCHAR)wParam == TEXT(' ') && GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_SHIFT) >= 0) {
			if (ListView_GetItemCount(hWnd) == 0 && op.ScanAllForUnread == 1) {
				SendMessage(hWnd, WM_FINDMAILBOX, 0, 0);
				return 0;
			} else if (ListView_GetItemState(hWnd, ListView_GetNextItem(hWnd, -1, LVNI_FOCUSED), LVIS_SELECTED) == LVIS_SELECTED) {
				SendMessage(GetParent(hWnd), WM_COMMAND, ID_MAILITEM_OPEN, 0);
				return 0;
			}
		}
		break;

#ifndef _WCE_OLD
	case WM_IME_CHAR:
		//2 byte of mailbox Setting
#ifdef UNICODE
		if (wParam == 0x3000 &&
#else
		if (wParam == 0x8140 &&
#endif
			GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_SHIFT) >= 0) {
			if (ListView_GetItemCount(hWnd) == 0 && op.ScanAllForUnread == 1) {
				SendMessage(hWnd, WM_FINDMAILBOX, 0, 0);
				return 0;
			} else if (ListView_GetItemState(hWnd, ListView_GetNextItem(hWnd, -1, LVNI_FOCUSED), LVIS_SELECTED) == LVIS_SELECTED) {
				SendMessage(GetParent(hWnd), WM_COMMAND, ID_MAILITEM_OPEN, 0);
				return 0;
			}
		}
		break;
#endif
	}
	return CallWindowProc(ListViewWindowProcedure, hWnd, msg, wParam, lParam);
}

/*
 * SetListViewSubClass - ウィンドウのサブクラス化
 */
static void SetListViewSubClass(HWND hWnd)
{
	ListViewWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassListViewProc);
}

/*
 * DelListViewSubClass - ウィンドウクラスを標準のものに戻す
 */
static void DelListViewSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)ListViewWindowProcedure);
	ListViewWindowProcedure = NULL;
}

/*
 * ListViewHeaderNotifyProc - リストビューヘッダメッセージ
 */
static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam)
{
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;
	HWND hListView;
	int col;

	switch (phd->hdr.code) {
	case HDN_ITEMCLICK:
		op.LvThreadView = 0;
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
		CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
		CheckMenuItem(GetSubMenu(hMainMenu, 1), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#else
		CheckMenuItem(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#endif
#else
		CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#endif
		// ソートの設定
		col = phd->iItem;
		if ((col == 0 || col == 1) && lstrcmp(op.LvColumnOrder, TEXT("FSDZ")) == 0) {
			col = (col == 0) ? 1 : 0;
		}
		LvSortFlag = (ABS(LvSortFlag) == (col + 1)) ? (LvSortFlag * -1) : (col + 1);
		//of sort Sort
		hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		SwitchCursor(FALSE);
		ListView_SortItems(hListView, CompareFunc, LvSortFlag);
		SwitchCursor(TRUE);

		ListView_EnsureVisible(hListView,
			ListView_GetNextItem(hListView, -1, LVNI_FOCUSED), TRUE);

		if (op.LvAutoSort == 2 || (MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
			op.LvSortItem = LvSortFlag;
		}
		break;
	}
	return FALSE;
}

/*
 * TbNotifyProc - ツールバーの通知メッセージ (Win32)
 */
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd, LPARAM lParam)
{
	TOOLTIPTEXT *pTT;

	pTT = (TOOLTIPTEXT*)lParam;
	pTT->hinst = hInst;
	pTT->lpszText = MAKEINTRESOURCE(pTT->hdr.idFrom);
	return 0;
}
#endif

/*
 * NotifyProc - コントロールの通知メッセージ
 */
static LRESULT NotifyProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;

	if (CForm->hwndFrom == GetDlgItem(hWnd, IDC_LISTVIEW)) {
#ifdef _WIN32_WCE_PPC
		if (CForm->code == GN_CONTEXTMENU) {
			//Pop rise menu indicatory
			SendMessage(hWnd, WM_COMMAND, ID_MENU, 0);
			return TRUE;
		}
#endif
		return ListView_NotifyProc(hWnd, lParam);
	}
	if (CForm->hwndFrom == GetWindow(GetDlgItem(hWnd, IDC_LISTVIEW), GW_CHILD)) {
		return ListViewHeaderNotifyProc(hWnd, lParam);
	}
#ifndef _WIN32_WCE
	if (CForm->code == TTN_NEEDTEXT) {
		return TbNotifyProc(hWnd, lParam);
	}
#endif
	return FALSE;
}

/*
 * CreateComboBox - コンボボックスの作成
 */
static int CreateComboBox(HWND hWnd, int Top)
{
	HWND hCombo;
	RECT rcClient, comboRect;
	int i;

	GetClientRect(hWnd, &rcClient);

	hCombo = CreateWindow(TEXT("COMBOBOX"), TEXT(""),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
		0, Top, rcClient.right, (rcClient.bottom < 200) ? 200 : rcClient.bottom,
		hWnd, (HMENU)IDC_COMBO, hInst, NULL);
	if (hCombo == NULL) {
		return -1;
	}

#ifndef _WIN32_WCE
	SendDlgItemMessage(hWnd, IDC_COMBO, WM_SETFONT,
		(WPARAM)((hListFont != NULL) ? hListFont : GetStockObject(DEFAULT_GUI_FONT)),
		MAKELPARAM(TRUE,0));
#endif
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETEXTENDEDUI, TRUE, 0);

	{
		int next = item_get_next_send_mark(MailBox + MAILBOX_SEND, TRUE);
		SendDlgItemMessage(MainWnd, IDC_COMBO, CB_ADDSTRING, 0, 
			((next==-1) ? (LPARAM)STR_SENDBOX_NAME : (LPARAM)STR_SENDBOX_NAME TEXT(" *")));
	}

	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_ADDSTRING, 0,
			(LPARAM)(((MailBox + i)->Name == NULL || *(MailBox + i)->Name == TEXT('\0'))
			? STR_MAILBOX_NONAME : (MailBox + i)->Name));
	}
	GetWindowRect(GetDlgItem(hWnd, IDC_COMBO), &comboRect);
	return (comboRect.bottom - comboRect.top);
}

/*
 * InitWindow - ウィンドウの初期化
 */
static BOOL InitWindow(HWND hWnd)
{
#ifndef _WIN32_WCE_PPC
#ifndef _WIN32_WCE_LAGENDA
	HWND hToolBar;
#endif	// _WIN32_WCE_LAGENDA
#endif	// _WIN32_WCE_PPC
	RECT StatusRect;
	HDC hdc;
	HFONT hFont;
	int Height = 0;
	int i, j;
	int Width[2];
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	CSOBAR_BASEINFO BaseInfo = {
		(-1),										// x
		(-1),										// y
		(-1),										// width
		(-1),										// height
		1,											// line
		CSOBAR_DEFAULT_BACKCOLOR,					// backColor
		CSOBAR_DEFAULT_CODECOLOR,					// titleColor
		NULL,										// titleText
		0,											// titleBmpResId
		NULL,										// titleBmpResIns
	};
	CSOBAR_BUTTONINFO ButtonInfo[] = {
		1,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, STR_MENU_FILE,	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID,			FALSE, FALSE,
		2,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, STR_MENU_MAIL,	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, CLR_INVALID,			FALSE, FALSE,
		ID_MENUITEM_RECV,		CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_RECV,		NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_ALLCHECK,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_ALLRECV,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_EXEC,		CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_EXEC,		NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_ALLEXEC,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_ALLEXEC,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_STOP,		CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_STOP,		NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0, CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
	};
	DWORD style;

#else	// _WIN32_WCE_LAGENDA
	TBBUTTON tbButton[] = {
#ifndef _WIN32_WCE_PPC
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
#endif	// _WIN32_WCE_PPC
		{0,	ID_MENUITEM_RECV,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{1,	ID_MENUITEM_ALLCHECK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{2,	ID_MENUITEM_EXEC,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{3,	ID_MENUITEM_ALLEXEC,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{4,	ID_MENUITEM_STOP,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{5,	ID_MENUITEM_NEWMAIL,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{6,	ID_MENUITEM_RAS_CONNECT,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{7,	ID_MENUITEM_RAS_DISCONNECT,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
	};
	static TCHAR *szTips[] = {
#ifdef _WIN32_WCE_PPC
		NULL, // menu skipping
		NULL, // menu skipping
#endif	// _WIN32_WCE_PPC
		STR_CMDBAR_RECV,
		STR_CMDBAR_ALLCHECK,
		STR_CMDBAR_EXEC,
		STR_CMDBAR_ALLEXEC,
		STR_CMDBAR_STOP,
		STR_CMDBAR_NEWMAIL,
		STR_CMDBAR_RAS_CONNECT,
		STR_CMDBAR_RAS_DISCONNECT
	};
#ifdef _WIN32_WCE_PPC
	SHMENUBARINFO mbi;
#endif	// _WIN32_WCE_PPC
#endif	// _WIN32_WCE_LAGENDA
#else	// _WIN32_WCE
	TBBUTTON tbButton[] = {
		{0,	ID_MENUITEM_RECV,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{1,	ID_MENUITEM_ALLCHECK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{2,	ID_MENUITEM_EXEC,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{3,	ID_MENUITEM_ALLEXEC,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{4,	ID_MENUITEM_STOP,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{5,	ID_MENUITEM_NEWMAIL,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
	};
	RECT ToolbarRect;
#endif	// _WIN32_WCE

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	// PocketPC
	memset (&mbi, 0, sizeof (SHMENUBARINFO));
	mbi.cbSize     = sizeof (SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_WINDOW;
	mbi.hInstRes   = hInst;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;
	SHCreateMenuBar(&mbi);

	hMainToolBar = mbi.hwndMB;
    CommandBar_AddToolTips(hMainToolBar, 11, szTips);
	CommandBar_AddBitmap(hMainToolBar, hInst, IDB_TOOLBAR, 8, 16, 16);
	CommandBar_AddButtons(hMainToolBar, sizeof(tbButton) / sizeof(TBBUTTON) - 3, tbButton);

	Height = 0;
	i = 0;
	CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
	PPCFlag = TRUE;
	MailMenuPos = 1;

#elif defined(_WIN32_WCE_LAGENDA)
	// BE-500
	hCSOBar = CSOBar_Create(hInst, hWnd, 1, BaseInfo);
	CSOBar_AddAdornments(hCSOBar, hInst, 1, CSOBAR_ADORNMENT_CLOSE, 0);

	hMainMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_WINDOW));
	ButtonInfo[0].SubMenu = GetSubMenu(hMainMenu, 0);
	ButtonInfo[1].SubMenu = GetSubMenu(hMainMenu, 1);
	ButtonInfo[2].reshInst = hInst;
	ButtonInfo[3].reshInst = hInst;
	ButtonInfo[4].reshInst = hInst;
	ButtonInfo[5].reshInst = hInst;
	ButtonInfo[6].reshInst = hInst;
	CSOBar_AddButtons(hCSOBar, hInst, sizeof(ButtonInfo) / sizeof(CSOBAR_BUTTONINFO), ButtonInfo);

	style = GetWindowLong(hCSOBar, GWL_STYLE);
	style &= ~WS_CLIPCHILDREN;
	SetWindowLong(hCSOBar, GWL_STYLE, style);

	Height = g_menu_height = CSOBar_Height(hCSOBar);
	i = 0;
	CheckMenuItem(GetSubMenu(hMainMenu, 0), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(GetSubMenu(hMainMenu, 1), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
	PPCFlag = TRUE;
	MailMenuPos = 1;

#else
	// H/PC & PsPC
	hToolBar = CommandBar_Create(hInst, hWnd, IDC_CB);
    CommandBar_AddToolTips(hToolBar, 9, szTips);
	CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR, 8, TB_ICONSIZE, TB_ICONSIZE);

	if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_WINDOW_HPC, 0);
		MailMenuPos = 3;
		CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON) -
			((GetSystemMetrics(SM_CXSCREEN) >= 640) ? 0 : 3), tbButton);
	} else {
		PPCFlag = TRUE;
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_WINDOW, 0);
		MailMenuPos = 1;
		CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON) - 5, tbButton);
	}
	CommandBar_AddAdornments(hToolBar, 0, 0);
	Height = CommandBar_Height(hToolBar);
	i = 0;
	CheckMenuItem(CommandBar_GetMenu(hToolBar, 0), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(CommandBar_GetMenu(hToolBar, 0), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
#else
	// Win32
	MailMenuPos = 3;
	hToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_TB, 6, hInst, IDB_TOOLBAR,
		tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, TB_ICONSIZE, TB_ICONSIZE, sizeof(TBBUTTON));
	SetWindowLong(hToolBar, GWL_STYLE,
		GetWindowLong(hToolBar, GWL_STYLE) | TBSTYLE_FLAT);
	SendMessage(hToolBar, TB_SETINDENT, 5, 0);
	ShowWindow(hToolBar,SW_SHOW);

	GetWindowRect(hToolBar, &ToolbarRect);
	Height = ToolbarRect.bottom - ToolbarRect.top;

	i = SBS_SIZEGRIP | SBT_NOBORDERS;

	CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

	// ListViewフォント
	if (op.lv_font.name != NULL && *op.lv_font.name != TEXT('\0')) {
		hListFont = font_create(hWnd, &op.lv_font);
	}
	//View of font and font
	if (op.view_font.name != NULL && *op.view_font.name != TEXT('\0')) {
		hViewFont = font_create(hWnd, &op.view_font);
	}
	if (hViewFont == NULL) {
#ifdef _WIN32_WCE
		hViewFont = font_copy(GetStockObject(SYSTEM_FONT));
#else
		hViewFont = font_copy(GetStockObject(DEFAULT_GUI_FONT));
#endif
	}
	hdc = GetDC(hWnd);
	if (hViewFont != NULL) {
		hFont = SelectObject(hdc, hViewFont);
	}
	font_charset = font_get_charset(hdc);
	if (hViewFont != NULL) {
		SelectObject(hdc, hFont);
	}

	// コンボボックス
	if ((j = CreateComboBox(hWnd, Height)) == -1) {
		return FALSE;
	}
	Height += j;

	//Status bar
	CreateWindowEx(0, STATUSCLASSNAME, TEXT(""),
		WS_VISIBLE | WS_CHILD | i,
		0, 0, 0, 0, hWnd, (HMENU)IDC_STATUS, hInst, NULL);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATUS), &StatusRect);
#ifdef _WIN32_WCE
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	Width[0] = 130;
#else
	Width[0] = 150;
#endif
#else
	Width[0] = 220;
#endif
	Width[1] = -1;
	SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETPARTS,
		(WPARAM)(sizeof(Width) / sizeof(int)), (LPARAM)((LPINT)Width));

	//List view
	if (CreateListView(hWnd, Height, StatusRect.bottom - StatusRect.top) == NULL) {
		return FALSE;
	}
	SetFocus(GetDlgItem(hWnd, IDC_LISTVIEW));
	if (hListFont != NULL) {
		//Font of list view setting
		SendMessage(GetDlgItem(hWnd, IDC_LISTVIEW), WM_SETFONT, (WPARAM)hListFont, MAKELPARAM(TRUE, 0));
	}
	//List view to subclass is converted the
	SetListViewSubClass(GetDlgItem(hWnd, IDC_LISTVIEW));

#ifdef _WIN32_WCE
	//Idea contest of window setting
	SendMessage(hWnd, WM_SETICON, (WPARAM)FALSE,
		(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN), IMAGE_ICON, SICONSIZE, SICONSIZE, 0));
#endif
	return TRUE;
}

/*
 * SetWindowSize - ウィンドウのサイズ変更
 */
#ifdef _WIN32_WCE
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient, StatusRect, comboRect;
	int Height = 0;

	SendDlgItemMessage(hWnd, IDC_STATUS, WM_SIZE, 0, 0);

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATUS), &StatusRect);
	GetWindowRect(GetDlgItem(hWnd, IDC_COMBO), &comboRect);

#ifndef _WIN32_WCE_PPC
	Height = CommandBar_Height(GetDlgItem(hWnd, IDC_CB));
#endif
	MoveWindow(GetDlgItem(hWnd, IDC_COMBO), 0, Height,
		rcClient.right, comboRect.bottom - comboRect.top, TRUE);

	Height += (comboRect.bottom - comboRect.top);
	MoveWindow(GetDlgItem(hWnd, IDC_LISTVIEW), 0, Height,
		rcClient.right, rcClient.bottom - Height - (StatusRect.bottom - StatusRect.top), TRUE);
	UpdateWindow(GetDlgItem(hWnd, IDC_LISTVIEW));
	return TRUE;
}
#elif defined _WIN32_WCE_LAGENDA
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	COSIPINFO CoSipInfo;
	SIPINFO SipInfo;
	RECT rcClient, StatusRect, comboRect;
	int sip_height = 0;
	int Height = 0;
	int ret;

	memset(&SipInfo, 0, sizeof(SIPINFO));
	SipInfo.cbSize = sizeof(SIPINFO);
	memset(&CoSipInfo, 0, sizeof(COSIPINFO));
	CoSipInfo.dwInfoType = COSIP_GET_KBTYPE;
	SipInfo.pvImData = &CoSipInfo;
	SipInfo.dwImDataSize = sizeof(COSIPINFO);
	SipGetInfo(&SipInfo);
	if (SipInfo.fdwFlags & SIPF_ON) {
		sip_height = ((SipInfo.rcSipRect).bottom - (SipInfo.rcSipRect).top);
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	if (wnd_size.right != 0 && wnd_size.bottom != 0) {
		MoveWindow(hWnd, 0, 0,
			wnd_size.right, wnd_size.bottom - sip_height, TRUE);
	}
	SendDlgItemMessage(hWnd, IDC_STATUS, WM_SIZE, 0, 0);

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATUS), &StatusRect);
	GetWindowRect(GetDlgItem(hWnd, IDC_COMBO), &comboRect);
	Height = (comboRect.bottom - comboRect.top) + g_menu_height;

	MoveWindow(GetDlgItem(hWnd, IDC_LISTVIEW), 0, Height,
		rcClient.right, rcClient.bottom - Height - (StatusRect.bottom - StatusRect.top), TRUE);
	return ret;
}
#else
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient, StatusRect, ToolbarRect, comboRect;
	int Height = 0;

	if (hWnd == NULL || IsIconic(hWnd) != 0) {
		return FALSE;
	}

	SendDlgItemMessage(hWnd, IDC_TB, WM_SIZE, wParam, lParam);
	SendDlgItemMessage(hWnd, IDC_STATUS, WM_SIZE, wParam, lParam);

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_TB), &ToolbarRect);
	GetWindowRect(GetDlgItem(hWnd, IDC_STATUS), &StatusRect);
	GetWindowRect(GetDlgItem(hWnd, IDC_COMBO), &comboRect);

	Height = ToolbarRect.bottom - ToolbarRect.top;
	MoveWindow(GetDlgItem(hWnd, IDC_COMBO), 0, Height,
		rcClient.right, comboRect.bottom - comboRect.top, TRUE);

	Height += comboRect.bottom - comboRect.top;
	MoveWindow(GetDlgItem(hWnd, IDC_LISTVIEW), 0, Height,
		rcClient.right, rcClient.bottom - Height - (StatusRect.bottom - StatusRect.top), TRUE);

	UpdateWindow(GetDlgItem(hWnd, IDC_LISTVIEW));
	return TRUE;
}
#endif

/*
 * SaveWindow - ウィンドウの保存処理
 */
static BOOL SaveWindow(HWND hWnd, BOOL SelDir, BOOL PromptSave, BOOL UpdateStatus)
{
	TCHAR SaveDir[BUF_SIZE];
	int i;
	BOOL SaveAll = (SelDir == TRUE) || (UpdateStatus == TRUE) || (op.AutoSave == 0);
	BOOL err = FALSE;
#ifdef _WIN32_WCE_PPC
	MSG msg;

	PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
#endif

	if (SelDir == FALSE) {
		lstrcpy(SaveDir, DataDir);
	} else {
		wsprintf(SaveDir, TEXT("%s%s"), op.BackupDir, STR_NPOPUK_FILES);
		if (filename_select(hWnd, SaveDir, NULL, NULL, FILE_CHOOSE_DIR, NULL) == FALSE) {
			return FALSE;
		} else if (lstrcmpi(SaveDir, AppDir) == 0) {
			ErrorMessage(hWnd, STR_ERROR_BACKUP_APPDIR);
			return FALSE;
		} else if (lstrcmpi(SaveDir, DataDir) == 0) {
			ErrorMessage(hWnd, STR_ERROR_BACKUP_DATADIR);
			return FALSE;
		}
		mem_free(&op.BackupDir);
		op.BackupDir = alloc_copy_t(SaveDir);
	}

	SwitchCursor(FALSE);

	//When it is in the midst of communicating, it cuts off the
	if (g_soc != -1 && GetHostFlag == FALSE) {
#ifndef WSAASYNC
		KillTimer(hWnd, ID_RECV_TIMER);
#endif
		KillTimer(hWnd, ID_SMTP_TIMER);
		KillTimer(hWnd, ID_SMTP_ONE_TIMER);
		KillTimer(hWnd, ID_CHECK_TIMER);
		KillTimer(hWnd, ID_EXEC_TIMER);
		KillTimer(hWnd, ID_TIMEOUT_TIMER);
		KillTimer(hWnd, ID_RASWAIT_TIMER);
		gSockFlag = FALSE;
		ExecFlag = FALSE;
		NewMailCnt = -1;
		command_status = POP_QUIT;
		send_buf(g_soc, CMD_RSET"\r\n");
		send_buf(g_soc, CMD_QUIT"\r\n");
		socket_close(hWnd, g_soc);
		g_soc = -1;
		if (op.RasCheckEndDisCon == 1) {
			RasDisconnect();
		}
	}
	//Cutting
	if (RasLoop == TRUE || op.RasEndDisCon == 1) {
		RasDisconnect();
	}

	//of dial rise The indicatory and compilation window is closed the
	CloseViewWindow(1);

	// Give user the option of not saving
	if (PromptSave == TRUE && op.PromptSaveOnExit != 0) {
		unsigned type = MB_YESNO;
		SwitchCursor(TRUE);
		if (op.PromptSaveOnExit == 2) {
			type |= MB_DEFBUTTON2;
		}
		if (MessageBox(hWnd, STR_Q_EXITSAVE, WINDOW_TITLE, type) == IDNO) {
			return TRUE;
		}
		SwitchCursor(FALSE);
	}

	if (SaveAll == TRUE) {
		//Address book retention
		err = !file_save_address_book(ADDRESS_FILE, SaveDir, AddressBook);
	}

	if (SaveAll == TRUE || (MailBox+MAILBOX_SEND)->NeedsSave != 0) {
		//Transmission box (SendBox) retention
		if (op.WriteMbox != (MailBox+MAILBOX_SEND)->WasMbox) {
			(MailBox+MAILBOX_SEND)->NeedsSave |= MBOX_FORMAT_CHANGED;
		}
		// BOOL IsBackup = SelDir;
		err |= !file_save_mailbox(SENDBOX_FILE, SaveDir, MailBox + MAILBOX_SEND, SelDir, 2);
		if (err != FALSE) {
			SwitchCursor(TRUE);
			if (MessageBox(hWnd, STR_ERR_SAVEEND,
				STR_TITLE_ERROR, MB_ICONERROR | MB_YESNO) == IDNO) {
				return FALSE;
			}
			SwitchCursor(FALSE);
		}
	}

	//INI and Mailbox Retention
	{
		HWND ListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		if (ListView != NULL) {
			for (i = 0; i < LV_COL_CNT; i++) {
				op.LvColSize[i] = ListView_GetColumnWidth(ListView, i);
			}
		}
	}
	if (ini_save_setting(hWnd, TRUE, SaveAll, (SelDir==TRUE) ? SaveDir : NULL) == FALSE) {
		SwitchCursor(TRUE);
		if (MessageBox(hWnd, STR_ERR_SAVEEND,
			STR_TITLE_ERROR, MB_ICONERROR | MB_YESNO) == IDNO) {
			return FALSE;
		}
	}
	if (UpdateStatus == TRUE) {
		SetItemCntStatusText(hWnd, NULL, FALSE);
	}
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * EndWindow - ウィンドウの終了処理
 */
static BOOL EndWindow(HWND hWnd)
{
	HIMAGELIST hImgList;
	TCHAR path[BUF_SIZE];

	SwitchCursor(FALSE);

	ListView_DeleteAllItems(GetDlgItem(hWnd, IDC_LISTVIEW));

	//of setting every of account The message window is closed the
	if (MsgWnd != NULL) {
		SendMessage(MsgWnd, WM_ENDDIALOG, 0, 0);
	}

	// 外部アプリ用ファイルの削除
	str_join_t(path, DataDir, VIEW_FILE, TEXT("."), op.ViewFileSuffix, (TCHAR *)-1);
	DeleteFile(path);
#ifdef _WIN32_WCE
	str_join_t(path, DataDir, EDIT_FILE, TEXT("."), op.EditFileSuffix, (TCHAR *)-1);
	DeleteFile(path);
#endif

	// 添付ファイルの削除
	if (op.AttachDelete != 0) {
		TCHAR file[BUF_SIZE];
		wsprintf(path, TEXT("%s%s\\"), DataDir, op.AttachPath);
		wsprintf(file, TEXT("%s*"), ATTACH_FILE);
		dir_delete(path, file);
		RemoveDirectory(path);
	}

	// 検索文字列の解放
	mem_free(&FindStr);
	FindStr = NULL;

	//in searching character string Release
	FreeAllMailBox();
	mem_free(&g_Pass);

	// 設定の解放
	ini_free();
	FreeRasInfo();

	// タスクトレイのアイコンの除去
	op.ShowTrayIcon = 0;
	TrayMessage(hWnd, NIM_DELETE, TRAY_ID, NULL, NULL);

	//of idea contest of task tray Cancellation
	DestroyIcon(TrayIcon_Main);
	DestroyIcon(TrayIcon_Check);
	DestroyIcon(TrayIcon_Mail);

	//of idea contest Cancellation
	DelListViewSubClass(GetDlgItem(hWnd, IDC_LISTVIEW));

	//of subclass conversion of list view Cancellation
	hImgList = ListView_SetImageList(GetDlgItem(hWnd, IDC_LISTVIEW), NULL, LVSIL_SMALL);
	ImageList_Destroy((void *)hImgList);
	hImgList = ListView_SetImageList(GetDlgItem(hWnd, IDC_LISTVIEW), NULL, LVSIL_STATE);
	ImageList_Destroy((void *)hImgList);

	//of image list Cancellation
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
    DestroyWindow(hMainToolBar);
#elif defined _WIN32_WCE_LAGENDA
	DestroyMenu(hMainMenu);
#else
	CommandBar_Destroy(GetDlgItem(hWnd, IDC_CB));
#endif
	DestroyIcon((HICON)SendMessage(hWnd, WM_GETICON, FALSE, 0));
#else
	DestroyWindow(GetDlgItem(hWnd, IDC_TB));
#endif
	DestroyWindow(GetDlgItem(hWnd, IDC_COMBO));
	DestroyWindow(GetDlgItem(hWnd, IDC_LISTVIEW));
	DestroyWindow(GetDlgItem(hWnd, IDC_STATUS));

	//of window Cancellation
	if (hListFont != NULL) {
		DeleteObject(hListFont);
	}
	if (hViewFont != NULL) {
		DeleteObject(hViewFont);
	}
	SwitchCursor(TRUE);
	DestroyWindow(hWnd);
	return TRUE;
}

/*
 * SendMail - メールの送信開始
 */
static BOOL SendMail(HWND hWnd, MAILITEM *tpMailItem, int end_cmd)
{
	MAILBOX *tpMailBox;
	TCHAR *pass;
	TCHAR ErrStr[BUF_SIZE];
	int BoxIndex;

	BoxIndex = mailbox_name_to_index(tpMailItem->MailBox);
	if (BoxIndex == -1) {
		ErrorSocketEnd(hWnd, MAILBOX_SEND);
		SocketErrorMessage(hWnd, STR_ERR_SELECTMAILBOX, MAILBOX_SEND);
		return FALSE;
	}
	tpMailBox = MailBox + BoxIndex;

	//of font Dial rise start
	if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, BoxIndex, 0) == FALSE) {
		ErrorSocketEnd(hWnd, MAILBOX_SEND);
		SetMailMenu(hWnd);
		return FALSE;
	}

	g_soc = 0;

	// SMTP Authentication
	if (tpMailBox->SmtpAuth == 1) {
		//When the password is not set, input of the password is urged the
		if (g_Pass != NULL) {
			mem_free(&g_Pass);
			g_Pass = NULL;
		}
		if (tpMailBox->AuthUserPass == 1) {
			pass = tpMailBox->SmtpPass;
			if (pass == NULL || *pass == TEXT('\0')) {
				pass = tpMailBox->SmtpTmpPass;
			}
		} else {
			pass = tpMailBox->Pass;
			if (pass == NULL || *pass == TEXT('\0')) {
				pass = tpMailBox->TmpPass;
			}
		}
		if (pass == NULL || *pass == TEXT('\0')) {
			gPassSt = 1;
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), hWnd, InputPassProc,
				(LPARAM)((tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0'))
				? STR_MAILBOX_NONAME : tpMailBox->Name)) == FALSE) {
				g_soc = -1;
				ErrorSocketEnd(hWnd, MAILBOX_SEND);
				SetMailMenu(hWnd);
				return FALSE;
			}
			if (gPassSt == 1) {
				//Temporarily the setting
				if (tpMailBox->AuthUserPass == 1) {
					tpMailBox->SmtpTmpPass = alloc_copy_t(g_Pass);
				} else {
					tpMailBox->TmpPass = alloc_copy_t(g_Pass);
				}
			}
		}
	}

	if (op.SocLog > 0) log_init(AppDir, LOG_FILE, TEXT("send"));

	SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);

	SwitchCursor(FALSE);
	command_proc = smtp_proc;
	command_status = SMTP_START;
	ExecFlag = TRUE;
	(MailBox + MAILBOX_SEND)->NeedsSave |= MARKS_CHANGED;

	*ErrStr = TEXT('\0');
	g_soc = smtp_send_mail(hWnd, tpMailBox, tpMailItem, end_cmd, ErrStr);
	if (g_soc == -1) {
		ErrorSocketEnd(hWnd, MAILBOX_SEND);
		SocketErrorMessage(hWnd, ErrStr, MAILBOX_SEND);
		return FALSE;
	}
	RecvBox = MAILBOX_SEND;

#ifndef WSAASYNC
	SetTimer(hWnd, ID_RECV_TIMER, RECVTIME, NULL);
#endif
	SetMailMenu(hWnd);
	SetTrayIcon(hWnd, TrayIcon_Check, WINDOW_TITLE);
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * RecvMailList - メールの一覧の受信
 */
static BOOL RecvMailList(HWND hWnd, int BoxIndex, BOOL SmtpFlag)
{
	MAILBOX *tpMailBox;
	TCHAR ErrStr[BUF_SIZE];

	if (g_soc != -1 || BoxIndex == MAILBOX_SEND || (MailBox+BoxIndex)->Type == MAILBOX_TYPE_SAVE) {
		return FALSE;
	}
	g_soc = 0;

	tpMailBox = MailBox + BoxIndex;

	//When the password is not set, input of the password is urged
	if (g_Pass != NULL) {
		mem_free(&g_Pass);
		g_Pass = NULL;
	}
	if ((tpMailBox->Pass == NULL || *tpMailBox->Pass == TEXT('\0')) &&
		(tpMailBox->TmpPass == NULL || *tpMailBox->TmpPass == TEXT('\0'))) {
		gPassSt = 1;
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), hWnd, InputPassProc,
			(LPARAM)((tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0'))
			? STR_MAILBOX_NONAME : tpMailBox->Name)) == FALSE) {
			g_soc = -1;
			return FALSE;
		}
		if (gPassSt == 1) {
			//Temporarily the setting
			tpMailBox->TmpPass = alloc_copy_t(g_Pass);
		}
	}

	if (op.SocLog > 0) log_init(AppDir, LOG_FILE, TEXT("recv"));

	RecvBox = BoxIndex;

	//of the password Acquisition (is acquired the IP which retains IP address from host name) the
	SwitchCursor(FALSE);
	if (tpMailBox->PopIP == 0 || op.IPCache == 0) {
		GetHostFlag = TRUE;
		tpMailBox->PopIP = get_host_by_name(hWnd, tpMailBox->Server, ErrStr);
		GetHostFlag = FALSE;
		if (tpMailBox->PopIP == 0) {
			ErrorSocketEnd(hWnd, BoxIndex);
			SocketErrorMessage(hWnd, ErrStr, BoxIndex);
			g_soc = -1;
			return FALSE;
		}
	}

	SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);

	//Connected start
	command_proc = pop3_list_proc;
	command_status = POP_START;
	PopBeforeSmtpFlag = SmtpFlag;
	g_soc = connect_server(hWnd,
		tpMailBox->PopIP, (unsigned short)tpMailBox->Port,
		(tpMailBox->PopSSL == 0 || tpMailBox->PopSSLInfo.Type == 4) ? -1 : tpMailBox->PopSSLInfo.Type,
		&tpMailBox->PopSSLInfo,
		ErrStr);
	if (g_soc == -1) {
		tpMailBox->PopIP = 0;
		ErrorSocketEnd(hWnd, BoxIndex);
		SocketErrorMessage(hWnd, ErrStr, BoxIndex);
		return FALSE;
	}
#ifndef WSAASYNC
	SetTimer(hWnd, ID_RECV_TIMER, RECVTIME, NULL);
#endif
	SetMailMenu(hWnd);
	SetTrayIcon(hWnd, TrayIcon_Check, WINDOW_TITLE);
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * MailMarkCheck - 削除メールがないかチェックする
 */
static BOOL MailMarkCheck(HWND hWnd, BOOL IsAfterCheck)
{
	HWND hListView;
	int i;
	BOOL ret = FALSE;
	ServerDelete = TRUE;
	if (IsAfterCheck == TRUE && op.CheckEndExecNoDelMsg == 2) {
		ServerDelete = FALSE;
	}

	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->CyclicFlag == 1) {
			continue;
		}
		if ((MailBox + i)->Loaded == FALSE && (MailBox + i)->Type != MAILBOX_TYPE_SAVE) {
			if (op.LazyLoadMailboxes == 2) {
				continue;
			} else {
				int load = mailbox_load_now(hWnd, i, TRUE, FALSE);
				if (load == 0) {
					continue;
				} else if (load == -1) {
					return FALSE;
				}
			}
		}
		if (item_get_next_download_mark((MailBox + i), -1, NULL) != -1) {
			ret = TRUE;
			if (ServerDelete == FALSE) {
				break;
			}
		}
		if (ServerDelete == TRUE && item_get_next_delete_mark((MailBox + i), -1, NULL) != -1) {
			if ((IsAfterCheck == FALSE && op.ExpertMode == 1) ||
				(IsAfterCheck == TRUE && op.CheckEndExecNoDelMsg == 0)) {
				ret = TRUE;
				break;
			} else {
				int ans = MessageBox(hWnd, STR_Q_DELSERVERMAIL,
					(MailBox + i)->Name, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
				if (ans == IDYES) {
					ret = TRUE;
					break;
				} else if (ans == IDCANCEL) {
					mailbox_select(hWnd, i);
					hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
					if ((i = ListView_GetNextDeleteItem(hListView, -1)) != -1) {
						ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
						ListView_SetItemState(hListView, i,
							LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
						ListView_EnsureVisible(hListView, i, TRUE);
					}
					return FALSE;
				} else { // IDNO
					ServerDelete = FALSE;
					if (ret == TRUE) {
						break;
					}
				}
			}
		}
	}
	if (IsAfterCheck == FALSE && op.CheckAfterUpdate == 1) {
		ret = TRUE;
	} else if (ret == FALSE && item_get_next_send_mark((MailBox + MAILBOX_SEND), FALSE) != -1) {
		ret = TRUE;
	} else if (IsAfterCheck == FALSE && ret == FALSE) {
		MessageBox(hWnd, STR_MSG_NOMARK, STR_TITLE_ALLEXEC, MB_ICONEXCLAMATION | MB_OK);
	}
	return ret;
}

/*
 * ExecItem - マークしたものを実行
 */
static BOOL ExecItem(HWND hWnd, int BoxIndex)
{
	MAILBOX *tpMailBox;
	TCHAR ErrStr[BUF_SIZE];

	if (g_soc != -1) {
		return FALSE;
	}

	tpMailBox = MailBox + BoxIndex;

	//When it is the transmission box, it transmits the
	if (BoxIndex == MAILBOX_SEND) {
		if (item_get_next_send_mark(tpMailBox, FALSE) == -1) {
			return FALSE;
		}
		AllCheck = TRUE;
		gSockFlag = TRUE;
		KeyShowHeader = FALSE;
		SmtpWait = 0;
		CheckBox = MAILBOX_USER - 1;
		SetTimer(hWnd, ID_SMTP_TIMER, SMTPTIME, NULL);
		return TRUE;
	}

	if (item_get_next_download_mark(tpMailBox, -1, NULL) == -1 &&
		item_get_next_delete_mark(tpMailBox, -1, NULL) == -1) {
		return FALSE;
	}
	g_soc = 0;

	mem_free(&g_Pass);
	g_Pass = NULL;
	if ((tpMailBox->Pass == NULL || *tpMailBox->Pass == TEXT('\0')) &&
		(tpMailBox->TmpPass == NULL || *tpMailBox->TmpPass == TEXT('\0'))) {
		gPassSt = 1;
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), hWnd, InputPassProc,
			(LPARAM)((tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0'))
			? STR_MAILBOX_NONAME : tpMailBox->Name)) == FALSE) {
			g_soc = -1;
			return FALSE;
		}
		if (gPassSt == 1) {
			//Temporarily the setting
			tpMailBox->TmpPass = alloc_copy_t(g_Pass);
		}
	}

	if (op.SocLog > 0) log_init(AppDir, LOG_FILE, TEXT("exec"));

	RecvBox = BoxIndex;

	//of the password Acquisition (is acquired the IP which retains IP address from host name) the
	SwitchCursor(FALSE);
	if (tpMailBox->PopIP == 0 || op.IPCache == 0) {
		GetHostFlag = TRUE;
		tpMailBox->PopIP = get_host_by_name(hWnd, tpMailBox->Server, ErrStr);
		GetHostFlag = FALSE;
		if (tpMailBox->PopIP == 0) {
			ErrorSocketEnd(hWnd, BoxIndex);
			SocketErrorMessage(hWnd, ErrStr, BoxIndex);
			g_soc = -1;
			return FALSE;
		}
	}

	SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);

	//Connected start
	command_proc = pop3_exec_proc;
	command_status = POP_START;
	g_soc = connect_server(hWnd,
		tpMailBox->PopIP, (unsigned short)tpMailBox->Port,
		(tpMailBox->PopSSL == 0 || tpMailBox->PopSSLInfo.Type == 4) ? -1 : tpMailBox->PopSSLInfo.Type,
		&tpMailBox->PopSSLInfo,
		ErrStr);
	if (g_soc == -1) {
		tpMailBox->PopIP = 0;
		ErrorSocketEnd(hWnd, BoxIndex);
		SocketErrorMessage(hWnd, ErrStr, BoxIndex);
		return FALSE;
	}
#ifndef WSAASYNC
	SetTimer(hWnd, ID_RECV_TIMER, RECVTIME, NULL);
#endif
	SetMailMenu(hWnd);
	SetTrayIcon(hWnd, TrayIcon_Check, WINDOW_TITLE);
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * OpenItem - アイテムを開く
 */
void OpenItem(HWND hWnd, BOOL MsgFlag, BOOL NoAppFlag)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int i;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if (ListView_GetSelectedCount(hListView) <= 0) {
		return;
	}
	i = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
	if (i == -1) {
		return;
	}
	ListView_EnsureVisible(hListView, i, TRUE);
	tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
	if (tpMailItem == NULL) {
		return;
	}
	if (SelBox == MAILBOX_SEND) {
		if (Edit_InitInstance(hInst, hWnd, -1, tpMailItem, EDIT_OPEN, NULL) == EDIT_INSIDEEDIT) {
			// GJC: don't edit sent mail
			Edit_ConfigureWindow(tpMailItem->hEditWnd, (tpMailItem->Mark == ICON_SENTMAIL) ? FALSE : TRUE);
#ifdef _WIN32_WCE
			ShowWindow(hWnd, SW_HIDE);
#endif
		}
		return;
	}
	if (tpMailItem->Body == NULL && SelBox != MAILBOX_SEND && (MailBox+SelBox)->Type != MAILBOX_TYPE_SAVE) {
		if (MsgFlag == TRUE) {
			if (MessageBox(hWnd, STR_MSG_NOBODY, STR_TITLE_OPEN, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
				return;
			}
		} else {
			(MailBox + SelBox)->NeedsSave = MARKS_CHANGED;
			if (tpMailItem->Mark == ICON_DOWN) {
				tpMailItem->Mark = ICON_NON;
				ListView_SetItemState(hListView, i, LVIS_CUT, LVIS_CUT);
			} else {
				tpMailItem->Mark = ICON_DOWN;
				ListView_SetItemState(hListView, i, 0, LVIS_CUT);
			}
			ListView_RedrawItems(hListView, i, i);
			UpdateWindow(hListView);
			return;
		}
	}
	if (View_InitInstance(hInst,
		(LPVOID)ListView_GetlParam(hListView, i), NoAppFlag) == TRUE) {
#ifdef _WIN32_WCE
		ShowWindow(hWnd, SW_HIDE);
#endif
	}
}

/*
 * ReMessageItem - 返信の作成
 */
static void ReMessageItem(HWND hWnd, int ReplyFlag)
{
	HWND hListView;
	int i;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	i = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
	if (i < 0)
		return;

	if (SelBox == MAILBOX_SEND && ReplyFlag == EDIT_REPLY) {
#ifdef _WIN32_WCE
		if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND_WIDE), hWnd, SetSendProc,
				(LPARAM)ListView_GetlParam(hListView, i));
		} else {
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
				(LPARAM)ListView_GetlParam(hListView, i));
		}
#else
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
			(LPARAM)ListView_GetlParam(hListView, i));
#endif
		// Refresh the screen with any changes
		ListView_RedrawItems(hListView, i, i);
		UpdateWindow(hListView);
	} else {
		if (Edit_InitInstance(hInst, hWnd, SelBox,
			(MAILITEM *)ListView_GetlParam(hListView, i), ReplyFlag, NULL) == EDIT_INSIDEEDIT) {
#ifdef _WIN32_WCE
			ShowWindow(hWnd, SW_HIDE);
#endif
		}
	}
}

/*
 * ItemToSaveBox - move/copy mailitem to a savebox
 */
BOOL ItemToSaveBox(HWND hWnd, MAILITEM *tpSingleItem, int TargetBox, BOOL ask, BOOL delete)
{
	MAILBOX *tpMailBox;
	MAILITEM *tpMailItem;
	MAILITEM *tpTmpMailItem;
	HWND hListView = NULL;
	TCHAR *buf, *title;
	TCHAR msgbuf[BUF_SIZE];
	int i, j, SelPoint = -1;
	BOOL retval = TRUE;

	if (tpSingleItem == NULL) {
		hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
		if ((i = ListView_GetSelectedCount(hListView)) <= 0) {
			return FALSE;
		}
	} else {
		i = 1;
	}
	if (delete) {
		title = STR_TITLE_MOVE;
	} else {
		title = STR_TITLE_COPY;
	}
	tpMailBox = MailBox + TargetBox;
	if (ask && TargetBox != MAILBOX_SEND && op.SaveMsg == 1) {
		if (delete) {
			wsprintf(msgbuf, STR_Q_MOVE, i, tpMailBox->Name);
		} else {
			wsprintf(msgbuf, STR_Q_COPY, i, tpMailBox->Name);
		}
		if (ParanoidMessageBox(hWnd, msgbuf, title, MB_ICONQUESTION | MB_YESNO) == IDNO) {
			return FALSE;
		}
	}

	SwitchCursor(FALSE);
	if (tpMailBox->Loaded == FALSE) {
		if (mailbox_load_now(hWnd, TargetBox, FALSE, FALSE) != 1) {
			return FALSE;
		}
	}
	i = -1;
	while (1) {
		if (tpSingleItem == NULL) {
			i = ListView_GetNextItem(hListView, i, LVNI_SELECTED);
			if (i == -1) {
				break;
			}
			tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
			if (tpMailItem == NULL) {
				continue;
			}
		} else {
			tpMailItem = tpSingleItem;
		}

		if (TargetBox == MAILBOX_SEND) {
			int state = 0;

			//The copy is drawn up in the transmission box the
			if ((tpTmpMailItem = item_to_mailbox(tpMailBox, tpMailItem, NULL, TRUE)) == NULL) {
				ErrorMessage(hWnd, STR_ERR_COPYFAIL);
				retval = FALSE;
				break;
			}
			j = ListView_InsertItemEx(hListView,
				(TCHAR *)LPSTR_TEXTCALLBACK, 0, I_IMAGECALLBACK, (long)tpTmpMailItem,
				ListView_GetItemCount(hListView));

			state = ListView_ComputeState(tpMailItem->Priority, tpMailItem->Multipart);
			ListView_SetItemState(hListView, j, INDEXTOSTATEIMAGEMASK(state), LVIS_STATEIMAGEMASK)
			ListView_RedrawItems(hListView, j, j);
			UpdateWindow(hListView);

			if (SelPoint == -1) {
				SelPoint = j;
			}
		} else {
			// See if message is already in this box
			j = item_find_thread(tpMailBox, tpMailItem->MessageID, tpMailBox->MailItemCnt);
			if (j != -1) {
				// If selected is fully downloaded, or saved is not either, OK to overwrite
				if (tpMailItem->Download == TRUE
					|| (*(tpMailBox->tpMailItem + j))->Download == FALSE) {
					item_free((tpMailBox->tpMailItem + j), 1);
				} else {
					// Selected message is incomplete but saved is complete, verify desire to overwrite
					buf = (TCHAR *)mem_alloc(
						sizeof(TCHAR) * (lstrlen(tpMailItem->Subject) + lstrlen(STR_Q_OVERWRITE) + 1));
					if (buf != NULL) {
						wsprintf(buf, STR_Q_OVERWRITE, tpMailItem->Subject);
						SwitchCursor(TRUE);
						if (MessageBox(hWnd, buf, title, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
							mem_free(&buf);
							retval = FALSE;
							continue;
						}
						mem_free(&buf);
					}
					item_free((tpMailBox->tpMailItem + j), 1);
				}
			}
			// Now copy the message
			if (item_to_mailbox(tpMailBox, tpMailItem, (MailBox + SelBox)->Name, FALSE) == NULL) {
				ErrorMessage(hWnd, STR_ERR_SAVECOPY);
				retval = FALSE;
				break;
			}
		}
		if (tpSingleItem != NULL) {
			break;
		}
	}
	item_resize_mailbox(tpMailBox);
	SetItemCntStatusText(hWnd, NULL, FALSE);
	if (SelPoint != -1) {
		//of mail item The item which is added is selected the
		ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
		ListView_SetItemState(hListView, SelPoint,
			LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(hListView, SelPoint, TRUE);
		SendDlgItemMessage(hWnd, IDC_LISTVIEW, WM_VSCROLL, SB_LINEDOWN, 0);
	}
	SwitchCursor(TRUE);
	return retval;
}

/*
 * ListDeleteItem - delete item from list view (not from server)
 */
static void ListDeleteItem(HWND hWnd, BOOL Ask)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int i;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if ((i = ListView_GetSelectedCount(hListView)) <= 0) {
		return;
	}
	if (Ask == TRUE) {
		TCHAR buf[BUF_SIZE];
		wsprintf(buf, STR_Q_DELLISTMAIL, i, (SelBox != MAILBOX_SEND && ((MailBox + SelBox)->Type != MAILBOX_TYPE_SAVE))
			? STR_Q_DELLISTMAIL_NOSERVER : TEXT(""));
		if (ParanoidMessageBox(hWnd, buf, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
			return;
		}
	}

	SwitchCursor(FALSE);
	ListView_SetRedraw(hListView, FALSE);
	while ((i = ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem != NULL) {
			tpMailItem->Mark = -1;
		}
		ListView_DeleteItem(hListView, i);
	}
	//As for memory in NULL setting
	for (i = 0; i < (MailBox + SelBox)->MailItemCnt; i++) {
		if (*((MailBox + SelBox)->tpMailItem + i) == NULL ||
			(*((MailBox + SelBox)->tpMailItem + i))->Mark != -1) {
			continue;
		}
		item_free(((MailBox + SelBox)->tpMailItem + i), 1);
	}
	item_resize_mailbox(MailBox + SelBox);

	ListView_SetRedraw(hListView, TRUE);
	SetItemCntStatusText(hWnd, NULL, FALSE);
	SwitchCursor(TRUE);
}
/*
 * ListDeleteAttach - delete attachments from selected messages
 */
static void ListDeleteAttach(HWND hWnd)
{
	HWND hListView;
	int i;
	BOOL did_one = FALSE;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if ((i = ListView_GetSelectedCount(hListView)) <= 0) {
		return;
	}
	if (ParanoidMessageBox(hWnd, STR_Q_DELATTACH, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
		return;
	}

	SwitchCursor(FALSE);
	ListView_SetRedraw(hListView, FALSE);
	i = -1;
	while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		MAILITEM *tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem != NULL && tpMailItem->Multipart > MULTIPART_ATTACH) {
			DeleteAttachFile(hWnd, tpMailItem);
			ListView_SetItemState(hListView, i, LVIS_CUT, LVIS_CUT);
			did_one = TRUE;
		}
	}
	if (did_one == TRUE) {
		(MailBox+SelBox)->NeedsSave |= MAILITEMS_CHANGED;
	}
	ListView_SetRedraw(hListView, TRUE);
	SwitchCursor(TRUE);
}

/*
 * SetDownloadMark - アイテムに受信マークを付加
 */
static void SetDownloadMark(HWND hWnd, BOOL Flag)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	BOOL MarkedOne = FALSE;
	int SendOrDownIcon = ((SelBox == MAILBOX_SEND) ? ICON_SEND : ICON_DOWN);
	int i;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if (ListView_GetSelectedCount(hListView) <= 0) {
		return;
	}
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	i = -1;
	while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Mark == SendOrDownIcon && Flag == TRUE) {
			tpMailItem->Mark = tpMailItem->MailStatus;
			if (SelBox != MAILBOX_SEND && tpMailItem->Download == FALSE) {
				ListView_SetItemState(hListView, i, LVIS_CUT, LVIS_CUT);
			}
		} else if (SelBox != MAILBOX_SEND || tpMailItem->Mark != ICON_SENTMAIL) {
			tpMailItem->Mark = SendOrDownIcon;
			MarkedOne = TRUE;
			ListView_SetItemState(hListView, i, 0, LVIS_CUT);
		}
		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);
	if (SelBox == MAILBOX_SEND && MarkedOne == TRUE) {
		SetMenuStar(MAILBOX_SEND, STR_SENDBOX_NAME, TRUE, TRUE);
	}

	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
}

/*
 * SetDeleteMark - アイテムに削除マークを付加
 */
static void SetDeleteMark(HWND hWnd)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int i;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if (ListView_GetSelectedCount(hListView) <= 0) {
		return;
	}
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	i = -1;
	while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		tpMailItem->Mark = ICON_DEL;
		ListView_SetItemState(hListView, i, 0, LVIS_CUT);
		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);

	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
}

/*
 * UnMark - アイテムのマークを解除
 */
static void UnMark(HWND hWnd)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int i;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if (ListView_GetSelectedCount(hListView) <= 0) {
		return;
	}
	i = -1;
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->MailStatus == ICON_ERROR) {
			tpMailItem->MailStatus = (SelBox == MAILBOX_SEND) ? ICON_NON : ICON_READ;
		}
		tpMailItem->Mark = tpMailItem->MailStatus;
		if (SelBox != MAILBOX_SEND && tpMailItem->Download == FALSE) {
			ListView_SetItemState(hListView, i, LVIS_CUT, LVIS_CUT);
		}
		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);

	if (SelBox == MAILBOX_SEND) {
#ifdef _WIN32_WCE
		unsigned int len;
#else
		int len;
#endif
		len = SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETLBTEXTLEN, MAILBOX_SEND, 0);
		if (len > lstrlen(STR_SENDBOX_NAME) && item_get_next_send_mark(MailBox + MAILBOX_SEND, TRUE) == -1) {
			SetMenuStar(MAILBOX_SEND, STR_SENDBOX_NAME, FALSE, TRUE);
		}
	}

	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
}

/*
 * SetReplyFwdMark - adds state, redraws window (GJC)
 */
void SetReplyFwdMark(MAILITEM *tpReMailItem, char Mark, int rebox)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int i;
	BOOL found = FALSE;

	if (!(tpReMailItem->ReFwd & Mark)) {
		tpReMailItem->ReFwd |= Mark;
		tpReMailItem->New = FALSE;
		(MailBox+rebox)->NeedsSave |= MARKS_CHANGED;
	}
	
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	if (hListView == NULL) {
		return;
	}

	i = -1;
	while (!found && (i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem != NULL && tpMailItem == tpReMailItem) {
			found = TRUE;
		}
	}
	if (!found) i = -1;
	while (!found && (i = ListView_GetNextItem(hListView, i, 0)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem != NULL && tpMailItem == tpReMailItem) {
			found = TRUE;
		}
	}
	if (found == TRUE) {
		ListView_SetItemState(hListView, i, INDEXTOOVERLAYMASK(tpMailItem->ReFwd), LVIS_OVERLAYMASK);
		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);

	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
}

/*
 * SetReadMail - 選択メールを開封済み、未開封にする
 */
static void SetMailStats(HWND hWnd, int St)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int i;

	hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
	if (ListView_GetSelectedCount(hListView) <= 0) {
		return;
	}
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	i = -1;
	while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL ||
			tpMailItem->MailStatus == ICON_NON || tpMailItem->MailStatus >= ICON_SENTMAIL) {
			continue;
		}
		tpMailItem->MailStatus = St;
		if (tpMailItem->Mark != ICON_DOWN && tpMailItem->Mark != ICON_DEL) {
			tpMailItem->Mark = St;
		}
		if (St == ICON_READ) {
			tpMailItem->New = FALSE;
			ListView_SetItemState(hListView, i, INDEXTOOVERLAYMASK(tpMailItem->ReFwd), LVIS_OVERLAYMASK);
		} else if (St == ICON_MAIL) {
			// also clear re/fwd overlay
			tpMailItem->ReFwd = ICON_NON;
			ListView_SetItemState(hListView, i, 0, LVIS_OVERLAYMASK);
		}
		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);
	SetItemCntStatusText(hWnd, NULL, FALSE);
}

/*
 * EndSocketFunc - 通信終了時の処理
 */
static void EndSocketFunc(HWND hWnd, BOOL DoTimer)
{
	HWND hListView;
	int i;

	if (op.RasCheckEndDisCon == 1) {
		if (DoTimer == FALSE ||
			op.RasCheckEndDisConTimeout==0 ||
			TimedMessageBox(hWnd, STR_Q_RASDISCON, WINDOW_TITLE, MB_YESNO, op.RasCheckEndDisConTimeout) != IDNO) {
			RasDisconnect();
		}
	}
	SetMailMenu(hWnd);

	if (NewMail_Flag == TRUE &&
		(IsWindowVisible(hWnd) == 0 ||
#ifndef _WIN32_WCE
		IsIconic(hWnd) != 0 ||
#endif
		GetForegroundWindow() != hWnd)) {
		SetTrayIcon(hWnd, TrayIcon_Mail, WINDOW_TITLE);
	} else {
		NewMail_Flag = FALSE;
		SetTrayIcon(hWnd, TrayIcon_Main, WINDOW_TITLE);
	}

	if (EndThreadSortFlag == TRUE) {
		//Sort
		SwitchCursor(FALSE);
		hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		if (op.LvThreadView == 1) {
			item_create_thread(MailBox + SelBox);
			ListView_SortItems(hListView, CompareFunc, SORT_THREAD + 1);
		} else if (op.LvAutoSort == 2) {
			ListView_SortItems(hListView, CompareFunc, op.LvSortItem);

			i = ListView_GetNextUnreadItem(hListView, -1, ListView_GetItemCount(hListView));
			if (i != -1) {
				//The not yet opening mail is selected the
				ListView_SetItemState(hListView, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_SetItemState(hListView, i,
					LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			}

		}
		ListView_EnsureVisible(hListView,
			ListView_GetNextItem(hListView, -1, LVNI_FOCUSED), TRUE);
		SwitchCursor(TRUE);

		EndThreadSortFlag = FALSE;
	}

	if (ExecFlag == TRUE && op.ExecEndSound == 1) {
		if (op.ExecEndSoundFile == NULL || *op.ExecEndSoundFile == TEXT('\0') ||
			sndPlaySound(op.ExecEndSoundFile, SND_ASYNC | SND_NODEFAULT) == FALSE) {
#ifdef _WIN32_WCE_LAGENDA
			MessageBuzzer(0xFFFFFFFF);
#else
			MessageBeep(MB_ICONASTERISK);
#endif
		}
	}
	ExecFlag = FALSE;
}

/*
 * CheckEndAutoExec - チェック後の自動実行
 */
static BOOL CheckEndAutoExec(HWND hWnd, int SocBox, int cnt, BOOL AllFlag)
{
	if (cnt == -1) {
		return FALSE;
	}
	if (g_soc != -1) {
		return FALSE;
	}

	if (AllFlag == TRUE) {
		//The loop of check and execution is avoided by the fact that it makes the round execution
		ShowError = TRUE;
		if (MailMarkCheck(hWnd, TRUE) == FALSE) {
			ShowError = FALSE;
			return FALSE;
		}
		ShowError = FALSE;

		AutoCheckFlag = FALSE;
		AllCheck = TRUE;
		ExecFlag = FALSE;		// FALSE にすることでチェックと実行のループを避ける
		KeyShowHeader = FALSE;
		CheckBox = MAILBOX_USER - 1;

		gSockFlag = TRUE;
		SetTimer(hWnd, ID_EXEC_TIMER, CHECKTIME, NULL);
	} else {
		ServerDelete = FALSE;
		if (item_get_next_delete_mark((MailBox + SocBox), -1, NULL) != -1) {
			if (op.CheckEndExecNoDelMsg == 0) {
				ServerDelete = TRUE;
			} else if (op.CheckEndExecNoDelMsg == 1) {
				BOOL ans;
				ShowError = TRUE;
				ans = MessageBox(hWnd, STR_Q_DELSERVERMAIL,
					STR_TITLE_EXEC, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
				ShowError = FALSE;
				if (ans == IDYES) {
					ServerDelete = TRUE;
				} else if (ans == IDCANCEL) {
					return FALSE;
				}
			}
		}
		if (ServerDelete == FALSE &&
			item_get_next_download_mark((MailBox + SocBox), -1, NULL) == -1 &&
			item_get_next_send_mark((MailBox + SocBox), FALSE) == -1) {
			return FALSE;
		}
		AutoCheckFlag = FALSE;
		AllCheck = FALSE;
		ExecFlag = FALSE;		// FALSE にすることでチェックと実行のループを避ける
		KeyShowHeader = FALSE;
		ExecItem(hWnd, SocBox);
	}
	return TRUE;
}

/*
 * Init_NewMailFlag - 新着メールフラグの初期化
 */
static void Init_NewMailFlag(HWND hWnd)
{
	int i;

	if (ShowMsgFlag == TRUE || op.ClearNewOverlay != 1) {
		return;
	}

	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->NewMail == TRUE) {
			// GJC - remove * from drop-down list
			TCHAR *p;
			p = ((MailBox + i)->Name == NULL || *(MailBox + i)->Name == TEXT('\0'))
				? STR_MAILBOX_NONAME : (MailBox + i)->Name;
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, i, 0);
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, i, (LPARAM)p);
			(MailBox + i)->NewMail = FALSE;
		}
	}

	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
}

/*
 * SetUnreadCntTitle - count of mailboxes that have new mail
 */
void SetUnreadCntTitle(HWND hWnd, BOOL CheckMsgs)
{
	TCHAR wbuf[BUF_SIZE];
	int i, j;
	int UnreadMailBox = 0;

	j = SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETCURSEL, 0, 0);
	for(i = MAILBOX_USER; i < MailBoxCnt; i++){
		if((MailBox + i)->NewMail == TRUE) {
			TCHAR *p;
			p = ((MailBox + i)->Name == NULL || *(MailBox + i)->Name == TEXT('\0'))
				? STR_MAILBOX_NONAME : (MailBox + i)->Name;
			// GJC - check if there still is new mail; if not, update drop-down list
			if (CheckMsgs == TRUE && item_get_next_new((MailBox + i), -1, NULL) == -1) {
				SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, i, 0);
				SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, i, (LPARAM)p);
				(MailBox + i)->NewMail = FALSE;
			} else {
#ifdef _WIN32_WCE
				unsigned int len;
#else
				int len;
#endif
				UnreadMailBox++;
				len = SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETLBTEXTLEN, i, 0);
				if (len == lstrlen(p)) {
					// * is missing, add it
					TCHAR *q = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 3));
					str_join_t(q, p, TEXT(" *"), (TCHAR *)-1);
					SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, i, 0);
					SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, i, (LPARAM)q);
					mem_free(&q);
				}
			}
		}
	}
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, j, 0);

	//未読アカウント数をタイトルバーに設定
	if(UnreadMailBox == 0){
		SetWindowText(hWnd, WINDOW_TITLE);
	}else{
		wsprintf(wbuf, STR_TITLE_NEWMAILBOX, WINDOW_TITLE, UnreadMailBox);
		SetWindowText(hWnd, wbuf);
	}
}

/*
 * NewMail_Message - 新着メールチェック結果のメッセージ
 */
static void NewMail_Message(HWND hWnd, int cnt)
{
	TCHAR *p;
	int i, j;

	if (cnt == -1) {
		return;
	}
	if (cnt == 0) {
		if (ShowError == FALSE && op.ShowNoMailMessage == 1 && ShowMsgFlag == FALSE &&
			AutoCheckFlag == FALSE) {
			// メッセージボックスの表示
			MessageBox(hWnd, STR_MSG_NONEWMAIL, WINDOW_TITLE,
				MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND);
		}
		SetUnreadCntTitle(hWnd, TRUE);
		if (gCheckAndQuit == TRUE) {
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_QUIT, 0);
		}
		return;
	}

	if (gCheckAndQuit == TRUE && op.NewMailSound == 0 && op.ShowNewMailMessage == 0) {
		SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_QUIT, 0);
	}

	if (IsWindowVisible(hWnd) == 0 ||
#ifndef _WIN32_WCE
		IsIconic(hWnd) != 0 ||
#endif
		GetForegroundWindow() != hWnd) {
		SetTrayIcon(hWnd, TrayIcon_Mail, WINDOW_TITLE);
		NewMail_Flag = TRUE;
	}

	// There is a new arrival in the message box; add the " *" in the drop-down combo
	j = SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETCURSEL, 0, 0);
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->NewMail == FALSE || (MailBox + i)->Loaded == FALSE ||
			((op.ListGetLine > 0 || op.ShowHeader == 1 || op.ListDownload == 1) &&
			mailbox_unread_check(i, FALSE) == FALSE)) {
			continue;
		}
		if (SelBox != i) {
			(MailBox + i)->NewMail = TRUE;
		}
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, i, 0);
		if ((MailBox + i)->Name == NULL || *(MailBox + i)->Name == TEXT('\0')) {
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, i,
				(LPARAM)STR_MAILBOX_NONAME\
				TEXT(" *"));
		} else {
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen((MailBox + i)->Name) + 3));
			if (p != NULL) {
				str_join_t(p, (MailBox + i)->Name, TEXT(" *"), (TCHAR *)-1);
				SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, i, (LPARAM)p);
				mem_free(&p);
			}
		}
	}
	SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, j, 0);

	SetUnreadCntTitle(hWnd, FALSE);

	//Index of mailbox of new arrival acquisition
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->NewMail == TRUE) {
			break;
		}
	}

	if (op.NewMailSound == 1) {
		if (op.NewMailSoundFile == NULL || *op.NewMailSoundFile == TEXT('\0') ||
			sndPlaySound(op.NewMailSoundFile, SND_ASYNC | SND_NODEFAULT) == FALSE) {
#ifdef _WIN32_WCE_LAGENDA
			MessageBuzzer(0xFFFFFFFF);
#else
			MessageBeep(MB_ICONASTERISK);
#endif
		}
	}

	//Indicatory
#ifdef _WCE_OLD
	//of message box The setting which does not indicate message in case of the mailbox which presently is indicated the message box is not produced the
	if (ShowError == TRUE || op.ShowNewMailMessage == 0 ||
		(AutoCheckFlag == FALSE && hWnd == GetForegroundWindow() && i == SelBox)) {
		return;
	}
	if (MsgWnd == NULL) {
		CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MSG), NULL, NewMailMessageProc, 0);
	} else {
		ShowWindow(MsgWnd, SW_SHOW);
		_SetForegroundWindow(MsgWnd);
	}
#else
	// メッセージを表示しない設定か現在表示されているメールボックスの場合はメッセージボックスを出さない
	if (ShowError == TRUE || op.ShowNewMailMessage == 0 || ShowMsgFlag == TRUE ||
		(AutoCheckFlag == FALSE && hWnd == GetForegroundWindow() && i == SelBox)) {
		return;
	}
	SendMessage(MsgWnd,	WM_SHOWDIALOG, 0, 0);
#endif
}

/*
 * AutoSave_Mailboxes - save MailBox.dat files (GJC)
 */
static void AutoSave_Mailboxes(HWND hWnd)
{
	TCHAR buf[BUF_SIZE];
	BOOL DidOne = FALSE;
	int i;

	if (op.AutoSave == 0) {
		return;
	}
	SwitchCursor(FALSE);
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		MAILBOX *tpMailBox = MailBox + i;
		if (tpMailBox == NULL || tpMailBox->Loaded == FALSE) {
			continue;
		}
		if ((tpMailBox->NeedsSave & MAILITEMS_CHANGED) && 
			((op.ListSaveMode != 0) || (tpMailBox->Type == MAILBOX_TYPE_SAVE))) {
			if (tpMailBox->Filename == NULL) {
				wsprintf(buf, TEXT("MailBox%d.dat"), i - MAILBOX_USER);
			} else {
				lstrcpy(buf, tpMailBox->Filename);
			}
			file_save_mailbox(buf, DataDir, tpMailBox, FALSE,
				(tpMailBox->Type == MAILBOX_TYPE_SAVE) ? 2 : op.ListSaveMode);
			DidOne = TRUE;
		}
	}
	if (DidOne) {
		ini_save_setting(hWnd, FALSE, FALSE, NULL);
	}
	SwitchCursor(TRUE);
}
/*
 * WndProc - メインウィンドウプロシージャ
 */
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	static BOOL SipFlag = FALSE;
#endif
	static BOOL save_flag = FALSE;
	int i, old_selbox;
	BOOL ret;

	switch (msg) {
	case WM_CREATE:
		MainWnd = hWnd;
		SwitchCursor(FALSE);

#ifdef _WIN32_WCE_LAGENDA
		GetClientRect(hWnd, &wnd_size);
#endif
		save_flag = TRUE;
		// メールボックスの初期化
		if (mailbox_init() == FALSE) {
			ErrorMessage(NULL, STR_ERR_INIT);
			DestroyWindow(hWnd);
			break;
		}

		// INIファイルの読み込み
		if (ini_read_setting(hWnd) == FALSE) {
			ErrorMessage(NULL, STR_ERR_INIT);
			DestroyWindow(hWnd);
			break;
		}

		if (mailbox_read() == FALSE) {
			ErrorMessage(NULL, STR_ERR_INIT);
			DestroyWindow(hWnd);
			break;
		}
#ifndef _WIN32_WCE
		SetWindowPos(hWnd, 0, op.MainRect.left, op.MainRect.top, 
			op.MainRect.right - op.MainRect.left, op.MainRect.bottom - op.MainRect.top,
			SWP_NOZORDER | SWP_HIDEWINDOW);
#endif

		//of initialization
		if (InitWindow(hWnd) == FALSE) {
			ErrorMessage(NULL, STR_ERR_INIT);
			DestroyWindow(hWnd);
			break;
		}
		save_flag = FALSE;
		mailbox_select(hWnd, MAILBOX_USER);

		if (op.SocLog > 0) log_clear(AppDir, LOG_FILE);
		SwitchCursor(TRUE);

		//of control inside window Setting
		TrayIcon_Main = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_NOCHECK),
			IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
		TrayIcon_Check = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_CHECK),
			IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
		TrayIcon_Mail = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN),
			IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
		if (op.ShowTrayIcon == 1 && TrayIcon_Main != NULL) {
			TrayMessage(hWnd, NIM_ADD, TRAY_ID, TrayIcon_Main, WINDOW_TITLE);
		}

		// 自動チェック用タイマーの起動
		if (op.AutoCheck == 1) {
			SetTimer(hWnd, ID_AUTOCHECK_TIMER, AUTOCHECKTIME, NULL);
		}

		//of timer for automatic operation check At the time of the first starting
		if (first_start == TRUE) {
			ShowWindow(hWnd, SW_SHOW);
			SetMailBoxOption(hWnd);
			ini_save_setting(hWnd, FALSE, FALSE, NULL);
			break;
		}
		
		// 起動時チェックの開始
		if (op.StartCheck == 1 && gSendAndQuit == FALSE) {
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLCHECK, 0);
		} else {
			gCheckAndQuit = FALSE;
		}

#ifdef _WIN32_WCE_PPC
		if (op.StartPass) {
			// need to bring window to top
			_SetForegroundWindow(hWnd);
			ShowWindow(hWnd, SW_SHOW);
		}
#endif

		//of check The compilation
		if (CmdLine != NULL || InitialAccount != NULL) {
			SetTimer(hWnd, ID_NEWMAIL_TIMER, 1, NULL);
		}
		if (CmdLine == NULL) {
			gSendAndQuit = FALSE;
		}
		break;

	case WM_COPYDATA:
#ifdef _WIN32_WCE
		CloseViewWindow(1);
		FocusWnd = hWnd;
		ShowWindow(hWnd, SW_SHOW);
		if (CommandLine(hWnd, ((PCOPYDATASTRUCT)lParam)->lpData) == TRUE) {
			_SetForegroundWindow(hWnd);
			if (Edit_MailToSet(hInst, hWnd, ((PCOPYDATASTRUCT)lParam)->lpData, -1) == EDIT_INSIDEEDIT) {
				ShowWindow(hWnd, SW_HIDE);
			}
		}
#else
		if (op.ShowPass == 1 &&
			(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
			op.Password != NULL && *op.Password != TEXT('\0') &&
			ConfirmPass(hWnd, op.Password) == FALSE) {
			break;
		}
		if (CommandLine(hWnd, ((PCOPYDATASTRUCT)lParam)->lpData) == TRUE) {
			Edit_MailToSet(hInst, hWnd, ((PCOPYDATASTRUCT)lParam)->lpData, -1);
		}
#endif
		break;

#ifdef _WIN32_WCE_LAGENDA
	case COSH_EXECUTEAPP:
		//2 of the mail it is heavy from mail address of the command line the starting processing
		{
			TCHAR buf[BUF_SIZE];

			ZeroMemory(buf, sizeof(TCHAR) * BUF_SIZE);
			if (CoshGetCommandLine(hWnd, lParam, buf, BUF_SIZE) == FALSE || *buf == TEXT('\0')) {
				SendMessage(hWnd, WM_SHOWLASTWINDOW, 0, 0);
				break;
			}
			CloseViewWindow(1);
			FocusWnd = hWnd;
			ShowWindow(hWnd, SW_SHOW);
			_SetForegroundWindow(hWnd);
			if (CommandLine(hWnd, buf) == TRUE && Edit_MailToSet(hInst, hWnd, buf, -1) == EDIT_INSIDEEDIT) {
				ShowWindow(hWnd, SW_HIDE);
			}
		}
		break;
#endif

#ifdef _WIN32_WCE_PPC
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SHACTIVATEINFO sai;

			memset(&sai, 0, sizeof(SHACTIVATEINFO));
			SHHandleWMSettingChange(hWnd, -1, 0, &sai);
			SipFlag = sai.fSipUp;
			SetWindowSize(hWnd, 0, 0);

			ListView_EnsureVisible(GetDlgItem(hWnd, IDC_LISTVIEW),
				ListView_GetNextItem(GetDlgItem(hWnd, IDC_LISTVIEW), -1, LVNI_FOCUSED), TRUE);
		}
		break;
#elif defined _WIN32_WCE_LAGENDA
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SipFlag = SetWindowSize(hWnd, 0, 0);

			ListView_EnsureVisible(GetDlgItem(hWnd, IDC_LISTVIEW),
				ListView_GetNextItem(GetDlgItem(hWnd, IDC_LISTVIEW), -1, LVNI_FOCUSED), TRUE);
		}
		break;
#endif

	case WM_SIZE:
#ifndef _WIN32_WCE
		if (wParam == SIZE_MINIMIZED) {
			confirm_flag = 1;
		} 
#endif
		if (wParam == SIZE_MINIMIZED && op.ShowTrayIcon == 1 && op.MinsizeHide == 1) {
			ShowWindow(hWnd, SW_HIDE);
			return 0;
		}
#ifndef _WIN32_WCE
		if (op.ShowPass == 1 &&
			(wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) &&
			op.Password != NULL && *op.Password != TEXT('\0') && confirm_flag == 1) {
			ShowWindow(hWnd, SW_MINIMIZE);
			return 0;
		}
#endif
		SetWindowSize(hWnd, wParam, lParam);
#ifndef _WIN32_WCE
		if (wParam != SIZE_MINIMIZED) {
			confirm_flag = 0;
		} 
#endif
		break;

#ifndef _WIN32_WCE
	case WM_EXITSIZEMOVE:
		if (IsWindowVisible(hWnd) != 0 &&
			IsIconic(hWnd) == 0 && IsZoomed(hWnd) == 0) {
			GetWindowRect(hWnd, (LPRECT)&op.MainRect);
		}
		break;

	case WM_QUERYENDSESSION:
		if (SaveWindow(hWnd, FALSE, TRUE, FALSE) == FALSE) {
			return FALSE;
		}
		save_flag = TRUE;
		return TRUE;

	case WM_ENDSESSION:
		EndWindow(hWnd);
		return 0;
#endif

#ifdef _WIN32_WCE
	case WM_HIBERNATE:
		CloseViewWindow(1);
		FocusWnd = hWnd;
		break;
#endif

	case WM_CLOSE:
		if (op.ShowTrayIcon == 1 && op.CloseHide == 1) {
			ShowWindow(hWnd, SW_HIDE);
#ifdef _WIN32_WCE
			CloseViewWindow(1);
#endif
			FocusWnd = hWnd;
#ifdef _WIN32_WCE_LAGENDA
			ShowWindow(hWnd, SW_SHOW);
#endif
			break;
		}
		if (op.CheckQueuedOnExit > 0 
			&& item_get_next_send_mark((MailBox + MAILBOX_SEND), (op.CheckQueuedOnExit == 2)) != -1) {
			if (MessageBox(hWnd, STR_Q_QUEUEDMAIL_EXIT, WINDOW_TITLE, MB_YESNO) == IDNO) {
				mailbox_select(hWnd, MAILBOX_SEND);
				break;
			}
		}
		if (SaveWindow(hWnd, FALSE, TRUE, FALSE) == FALSE) {
			break;
		}
		save_flag = TRUE;
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		SipFlag = FALSE;
#endif
		EndWindow(hWnd);
		break;

	case WM_DESTROY:
		if (save_flag == FALSE) {
			SaveWindow(hWnd, FALSE, FALSE, FALSE);
		}
		save_flag = TRUE;
		PostQuitMessage(0);
		break;

	case WM_NOTIFY:
		return NotifyProc(hWnd, wParam, lParam);

	case WM_SETFOCUS:
#ifdef _WIN32_WCE_PPC
		SHSipPreference(hWnd, (SipFlag) ? SIP_UP : SIP_DOWN);
#elif defined _WIN32_WCE_LAGENDA
		SipShowIM((SipFlag) ? SIPF_ON : SIPF_OFF);
#endif
		SetFocus(GetDlgItem(hWnd, IDC_LISTVIEW));
	case WM_INITTRAYICON:
		if (g_soc == -1) {
			SetTrayIcon(hWnd, TrayIcon_Main, WINDOW_TITLE);
		}
		NewMail_Flag = FALSE;
		break;

	case WM_TIMER:
		switch (wParam) {
#ifndef WSAASYNC
		// 受信処理
		case ID_RECV_TIMER:
			if (g_soc == -1) {
				KillTimer(hWnd, wParam);
				if (AllCheck == FALSE) {
					if (op.CheckEndExec == 1 &&
						CheckEndAutoExec(hWnd, RecvBox, NewMailCnt, FALSE) == TRUE) {
						//After the checking execution
						break;
					}
					if (ExecFlag == TRUE && op.CheckAfterUpdate == 1 && RecvBox != MAILBOX_SEND) {
						// 実行後チェック
						ExecFlag = FALSE;
						RecvMailList(hWnd, RecvBox, FALSE);
						break;
					}
					RecvBox = -1;
					EndSocketFunc(hWnd, TRUE);
					AutoSave_Mailboxes(hWnd);
					NewMail_Message(hWnd, NewMailCnt);
				} else {
					RecvBox = -1;
					SetMailMenu(hWnd);
				}
				break;
			}
			switch (recv_select(hWnd, g_soc)) {
			//Memory error
			case SELECT_MEM_ERROR:
				ErrorSocketEnd(hWnd, RecvBox);
				SocketErrorMessage(hWnd, STR_ERR_MEMALLOC, RecvBox);
				break;

			//select error
			case SELECT_SOC_ERROR:
				ErrorSocketEnd(hWnd, RecvBox);
				SocketErrorMessage(hWnd, STR_ERR_SOCK_SELECT, RecvBox);
				break;

			//Cutting
			case SELECT_SOC_CLOSE:
				if (command_status != POP_QUIT) {
					if (command_status == POP_RETR || command_status == POP_TOP) {
						pop3_salvage_buffer(hWnd, (MailBox + RecvBox), RecvBox == SelBox);
					}
					ErrorSocketEnd(hWnd, RecvBox);
					SocketErrorMessage(hWnd, STR_ERR_SOCK_DISCONNECT, RecvBox);
				} else {
					socket_close(hWnd, g_soc);
					g_soc = -1;
					KillTimer(hWnd, ID_TIMEOUT_TIMER);
					SetItemCntStatusText(hWnd, NULL, FALSE);
					SetUnreadCntTitle(hWnd, TRUE);
				}
				break;

			//There is a reception data, the
			case SELECT_SOC_SUCCEED:
				SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);
				break;
			}
			break;
#endif
		//1 case transmission
		case ID_SMTP_ONE_TIMER:
			if (g_soc != -1) {
				break;
			}
			if (SmtpWait > 0) {
				SmtpWait--;
				break;
			}
			KillTimer(hWnd, wParam);

			AutoCheckFlag = FALSE;
			AllCheck = FALSE;
			ExecFlag = TRUE;
			KeyShowHeader = FALSE;
			gSockFlag = FALSE;

			//Transmission
			SendMail(hWnd, (MAILITEM *)wkSendMailItem, SMTP_SENDEND);
			wkSendMailItem = NULL;
			break;

		//of mail Transmission main.
		case ID_SMTP_TIMER:
			if (g_soc != -1) {
				break;
			}
			if (SmtpWait > 0) {
				SmtpWait--;
				break;
			}
			if (CheckBox >= MAILBOX_USER &&
				item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox) != -1) {
				// メールの送信
				SendMail(hWnd, *((MailBox + MAILBOX_SEND)->tpMailItem +
					item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox)), SMTP_NEXTSEND);
				break;
			}

			CheckBox++;
			if (CheckBox >= MailBoxCnt) {
				KillTimer(hWnd, wParam);
				gSockFlag = FALSE;
				EndSocketFunc(hWnd, TRUE);
				NewMail_Message(hWnd, NewMailCnt);
				break;
			}
			if ((MailBox + CheckBox)->PopBeforeSmtp != 0 &&
				item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox) != -1) {
				// POP before SMTP
				if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
					ErrorSocketEnd(hWnd, CheckBox);
					SetMailMenu(hWnd);
					break;
				}
				if (op.PopBeforeSmtpIsLoginOnly == 0 && NewMailCnt == -1) NewMailCnt = 0;
				RecvMailList(hWnd, CheckBox, (op.PopBeforeSmtpIsLoginOnly == 1) ? TRUE : FALSE);
				SmtpWait = op.PopBeforeSmtpWait / SMTPTIME;
			}
			break;

		//of mail Round check
		case ID_CHECK_TIMER:
			if (g_soc != -1) {
				break;
			}

			do {
				CheckBox++;
			} while (CheckBox < MailBoxCnt && (MailBox + CheckBox)->Loaded == FALSE);
			if (CheckBox >= MailBoxCnt) {
				//Check completion
				KillTimer(hWnd, wParam);
				gSockFlag = FALSE;
				if (op.CheckEndExec == 1 &&
					CheckEndAutoExec(hWnd, 0, NewMailCnt, TRUE) == TRUE) {
					//of all mailboxes After the checking execution
					break;
				}
				EndSocketFunc(hWnd, TRUE);
				AutoSave_Mailboxes(hWnd);
				NewMail_Message(hWnd, NewMailCnt);
				AutoCheckFlag = FALSE;
				break;
			}
			//It does not go around the mailbox
			if ((MailBox + CheckBox)->CyclicFlag == 1) {
				break;
			}
			//of the setting which Dial rise start
			if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
				ErrorSocketEnd(hWnd, CheckBox);
				SetMailMenu(hWnd);
				break;
			}
			if (op.SocLog > 1) {
				TCHAR msg[BUF_SIZE];
				wsprintf(msg, TEXT("CheckTimer: box=%d"), SelBox);
				log_save(AppDir, LOG_FILE, msg);
			}
			//Mail reception start
			RecvMailList(hWnd, CheckBox, FALSE);
			break;

		//Round execution
		case ID_EXEC_TIMER:
			if (g_soc != -1) {
				break;
			}
			if (SmtpWait > 0) {
				SmtpWait--;
				break;
			}
			if (CheckBox >= MAILBOX_USER && (MailBox + CheckBox)->CyclicFlag == 0 &&
				item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox) != -1) {
				//Execution of transmission mail (POP before SMTP)
				SendMail(hWnd, *((MailBox + MAILBOX_SEND)->tpMailItem +
					item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox)), SMTP_NEXTSEND);
				break;
			}
			if (CheckBox >= MAILBOX_USER && (MailBox + CheckBox)->CyclicFlag == 0 &&
				op.CheckAfterUpdate == 1 && ExecCheckFlag == FALSE) {
				// 実行後チェック
				RecvMailList(hWnd, CheckBox, FALSE);
				ExecCheckFlag = TRUE;
				break;
			}
			ExecCheckFlag = FALSE;

			CheckBox++;
			if (CheckBox >= MailBoxCnt) {
				KillTimer(hWnd, wParam);
				if (item_get_next_send_mark((MailBox + MAILBOX_SEND), FALSE) != -1) {
					// 送信
					ExecItem(hWnd, MAILBOX_SEND);
					break;
				}
				//Round execution end
				gSockFlag = FALSE;
				EndSocketFunc(hWnd, TRUE);
				AutoSave_Mailboxes(hWnd);
				NewMail_Message(hWnd, NewMailCnt);
				break;
			}
			//It does not go around the mailbox
			if ((MailBox + CheckBox)->CyclicFlag == 1) {
				break;
			}
			if (item_get_next_download_mark((MailBox + CheckBox), -1, NULL) != -1 ||
				item_get_next_delete_mark((MailBox + CheckBox), -1, NULL) != -1 ||
				op.CheckAfterUpdate == 1) {
				// ダイヤルアップ開始
				if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
					ErrorSocketEnd(hWnd, CheckBox);
					SetMailMenu(hWnd);
					break;
				}
			}
			//Mark execution
			i = ExecItem(hWnd, CheckBox);
			if ((MailBox + CheckBox)->PopBeforeSmtp != 0 &&
				item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox) != -1) {
				if (i == FALSE) {
					// POP before SMTP
					if (op.PopBeforeSmtpIsLoginOnly == 0 && NewMailCnt == -1) NewMailCnt = 0;
					RecvMailList(hWnd, CheckBox, (op.PopBeforeSmtpIsLoginOnly == 1) ? TRUE : FALSE);
				}
				SmtpWait = op.PopBeforeSmtpWait / SMTPTIME;
			}
			break;

		//Automatic check
		case ID_AUTOCHECK_TIMER:
			if (op.AutoCheck == 0) {
				KillTimer(hWnd, wParam);
				AutoCheckCnt = 0;
				break;
			}
			AutoCheckCnt++;
			if (AutoCheckCnt < op.AutoCheckTime) {
				break;
			}
			if (g_soc != -1 || ShowError == TRUE) {
				break;
			}
			if (op.SocLog > 1) log_save(AppDir, LOG_FILE, TEXT("Auto check"));
			AutoCheckCnt = 0;
			AutoCheckFlag = TRUE;
			AllCheck = TRUE;
			KeyShowHeader = FALSE;
			NewMailCnt = 0;
			CheckBox = MAILBOX_USER - 1;
			Init_NewMailFlag(hWnd);

			gSockFlag = TRUE;
			ExecFlag = FALSE;
			SetTimer(hWnd, ID_CHECK_TIMER, CHECKTIME, NULL);
			break;

		//Timeout
		case ID_TIMEOUT_TIMER:
			if (g_soc == -1) {
				KillTimer(hWnd, wParam);
				break;
			}
			if (command_status == POP_RETR || command_status == POP_TOP) {
				pop3_salvage_buffer(hWnd, (MailBox + RecvBox), RecvBox == SelBox);
			}
			ErrorSocketEnd(hWnd, RecvBox);
			SocketErrorMessage(hWnd, STR_ERR_SOCK_TIMEOUT, RecvBox);
			break;

		//When starting waiting
		case ID_NEWMAIL_TIMER:
			KillTimer(hWnd, wParam);
			if (InitialAccount != NULL) {
				int i = mailbox_name_to_index(InitialAccount);
				mailbox_select(hWnd, i);
				mem_free(&InitialAccount);
				InitialAccount = NULL;
			}
			if (CmdLine != NULL) {
#ifdef _WIN32_WCE
				if (CommandLine(hWnd, CmdLine) == TRUE && Edit_MailToSet(hInst, hWnd, CmdLine, -1) == EDIT_INSIDEEDIT) {
					ShowWindow(hWnd, SW_HIDE);
					if (gSendAndQuit == TRUE) {
						SendMessage(hEditWnd, WM_COMMAND, ID_MENUITEM_SEND, 0);
					}
				}
#else
				if (CommandLine(hWnd, CmdLine) == TRUE) {
					if (Edit_MailToSet(hInst, hWnd, CmdLine, -1) == EDIT_INSIDEEDIT) {
						if (gSendAndQuit == TRUE && hEditWnd != NULL) {
							SendMessage(hEditWnd, WM_COMMAND, ID_MENUITEM_SEND, 0);
						}
					//} else if (gSendAndQuit == TRUE) {
					//	PostQuitMessage(1);
					}
				}
#endif
				mem_free(&CmdLine);
				CmdLine = NULL;
			}
			break;

		//ras Change
		case ID_RASWAIT_TIMER:
			KillTimer(hWnd, wParam);
			if (hEvent != NULL) {
				SetEvent(hEvent);
			}
			break;
		} // WM_TIMER switch
		break;

	case WM_COMMAND: {
		BOOL mark_del = FALSE;
		int mbox, command_id = GET_WM_COMMAND_ID(wParam, lParam);
		switch (command_id) {
		//of message compilation
		case IDC_COMBO:
			if (HIWORD(wParam) == CBN_CLOSEUP) {
				if (SelBox == SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETCURSEL, 0, 0)) {
					break;
				}
				SetFocus(GetDlgItem(hWnd, IDC_LISTVIEW));
				mailbox_select(hWnd, SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETCURSEL, 0, 0));
				SwitchCursor(TRUE);
			}
			break;

		//of indicatory account In account above portable
		case ID_KEY_ALTUP:
			i = SelBox - 1;
			if (i < 0) {
				i = MailBoxCnt-1;
			}
			mailbox_select(hWnd, i);
			break;

		//In account under portable
		case ID_KEY_ALTDOWN:
			i = SelBox + 1;
			if (i >= MailBoxCnt) {
				i = 0;
			}
			mailbox_select(hWnd, i);
			break;

		//Focusing the drop-down combo and the list view is changed
		case ID_KEY_TAB:
			if (GetFocus() == GetDlgItem(hWnd, IDC_LISTVIEW)) {
				SetFocus(GetDlgItem(hWnd, IDC_COMBO));
			} else {
				SetFocus(GetDlgItem(hWnd, IDC_LISTVIEW));
				if (SelBox != SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETCURSEL, 0, 0)) {
					mailbox_select(hWnd, SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETCURSEL, 0, 0));
				}
			}
			break;

		//In position of selective item pop rise menu indicatory
		case ID_KEY_SHOWMENU:
		case ID_KEY_ESC:
			if (IsWindowVisible(hWnd) == 0 ||
#ifndef _WIN32_WCE
				IsIconic(hWnd) != 0 ||
#endif
				GetForegroundWindow() != hWnd) {
				break;
			}
			if (GetFocus() == GetDlgItem(hWnd, IDC_COMBO)) {
				if (SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETDROPPEDSTATE, 0, 0) == FALSE) {
					SetFocus(GetDlgItem(hWnd, IDC_LISTVIEW));
				} else {
					SendDlgItemMessage(hWnd, IDC_COMBO, CB_SHOWDROPDOWN, FALSE, 0);
				}
				break;
			}
			SetFocus(GetDlgItem(hWnd, IDC_LISTVIEW));
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			ShowMenu(hWnd, SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), 0, 1, FALSE);
#elif defined(_WIN32_WCE_LAGENDA)
			ShowMenu(hWnd, hMainMenu, MailMenuPos, 1, FALSE);
#else
			ShowMenu(hWnd, CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), MailMenuPos, 1, FALSE);
#endif
#else
			ShowMenu(hWnd, GetMenu(hWnd), MailMenuPos, 1, FALSE);
#endif
			break;

		//In position of mouse pop rise menu indicatory
		case ID_MENU:
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			ShowMenu(hWnd, SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), 0, 0, FALSE);
#elif defined(_WIN32_WCE_LAGENDA)
			ShowMenu(hWnd, hMainMenu, MailMenuPos, 1, FALSE);
#else
			ShowMenu(hWnd, CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), MailMenuPos, 0, FALSE);
#endif
#else
			ShowMenu(hWnd, GetMenu(hWnd), MailMenuPos, 0, FALSE);
#endif
			break;

		//====== file =========
		//Compilation
		case ID_MENUITEM_NEWMAIL:
#ifndef _WIN32_WCE
			if (op.ShowPass == 1 &&
				(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
				op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password) == FALSE) {
				break;
			}
#endif
			if (Edit_InitInstance(hInst, hWnd, -1, NULL, EDIT_NEW, NULL) == EDIT_INSIDEEDIT) {
#ifdef _WIN32_WCE
				ShowWindow(hWnd, SW_HIDE);
#endif
			}
			break;

		// アドレス帳
		case ID_MENUITEM_ADDRESS:
			{
				// GJC make temporary addressbook for editing
				ADDRESSBOOK *tpTmpAddressBook = addressbook_copy();
				if (tpTmpAddressBook != NULL) {
					tpTmpAddressBook->GetAddrList = FALSE;
					DialogBoxParam(hInst, MAKEINTRESOURCE(gAddressDialogResource),
						hWnd, AddressListProc, (LPARAM)tpTmpAddressBook);
					addressbook_free(tpTmpAddressBook);
				}
			}
			break;

		//Option
		case ID_MENUITEM_OPTION:
			ret = SetOption(hWnd);

			SwitchCursor(FALSE);
			// 自動チェックタイマーの設定
			if (op.AutoCheck == 1) {
				SetTimer(hWnd, ID_AUTOCHECK_TIMER, AUTOCHECKTIME, NULL);
			} else {
				KillTimer(hWnd, ID_AUTOCHECK_TIMER);
				AutoCheckCnt = 0;
			}
			// タスクトレイのアイコンの設定
			if (op.ShowTrayIcon == 1) {
				SetTrayIcon(hWnd, TrayIcon_Main, WINDOW_TITLE);
			} else {
				TrayMessage(hWnd, NIM_DELETE, TRAY_ID, NULL, NULL);
			}
			if (ret == TRUE && op.AutoSave == 1) {
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
			}
			SwitchCursor(TRUE);
			break;

		//of idea contest of task tray Dial rise connected
		case ID_MENUITEM_RAS_CONNECT:
			if (g_soc != -1) {
				break;
			}
			SendMessage(hWnd, WM_RAS_START, SelBox, 0);
			break;

		//During dial rise cutting
		case ID_MENUITEM_RAS_DISCONNECT:
			RasDisconnect();
			break;

		//lan connecting
		case ID_MENUITEM_LAN:
			op.EnableLAN = (op.EnableLAN == 1) ? 0 : 1;
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
			CheckMenuItem(GetSubMenu(hMainMenu, 0), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
			CheckMenuItem(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
#else
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
			SetMailMenu(hWnd);
			break;

		//Version information
		case ID_MENUITEM_ABOUT:
	///////////// MRP /////////////////////
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ABOUT), NULL, AboutBoxProc, 0);
	///////////// --- /////////////////////
			break;

	///////////// MRP /////////////////////
		case ID_MENUITEM_SAVEALL:
			SaveWindow(hWnd, FALSE, FALSE, TRUE);
			break;
	///////////// --- /////////////////////

		case ID_MENUITEM_BACKUP:
			SaveWindow(hWnd, TRUE, FALSE, FALSE);
			break;

		//End
		case ID_MENUITEM_QUIT:
			if (op.CheckQueuedOnExit > 0
				&& item_get_next_send_mark((MailBox + MAILBOX_SEND), (op.CheckQueuedOnExit == 2)) != -1) {
				if (MessageBox(hWnd, STR_Q_QUEUEDMAIL_EXIT, WINDOW_TITLE, MB_YESNO) == IDNO) {
					mailbox_select(hWnd, MAILBOX_SEND);
					break;
				}
			}
			if (SaveWindow(hWnd, FALSE, TRUE, FALSE) == FALSE) {
				break;
			}
			save_flag = TRUE;
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
			SipFlag = FALSE;
#endif
			EndWindow(hWnd);
			break;

#ifdef _WIN32_WCE_LAGENDA
		// 閉じる
		case CSOBAR_ADORNMENTID_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
#endif
		// ====== アカウント =========
		// アカウントの追加
		case ID_MENUITEM_ADDMAILBOX:
			if (MailBoxCnt >= MAX_MAILBOX_CNT) {
				MessageBox(hWnd, TEXT("Too many accounts"), WINDOW_TITLE, MB_OK);
				break;
			}
			old_selbox = SelBox;
			mailbox_select(hWnd, mailbox_create(hWnd, 1, TRUE, TRUE));
			i = SetMailBoxType(hWnd, 0);
			(MailBox+SelBox)->NewMail = TRUE; // hack to force correct name into IDC_COMBO
			if (i == -1 || (i == 0 && SetMailBoxOption(hWnd) == FALSE)) {
				mailbox_delete(hWnd, SelBox, FALSE);
				mailbox_select(hWnd, old_selbox);
				break;
			} else if (i == MAILBOX_TYPE_SAVE) {
				mailbox_select(hWnd, SelBox);
			} else if (i == MAILBOX_IMPORT_SAVE) {
				if (ImportSavebox(hWnd) == FALSE) {
					mailbox_delete(hWnd, SelBox, FALSE);
					mailbox_select(hWnd, old_selbox);
					break;
				}
				(MailBox+SelBox)->Type = MAILBOX_TYPE_SAVE;
				(MailBox+SelBox)->NeedsSave = MAILITEMS_CHANGED;
				mailbox_select(hWnd, SelBox);
			}
			if (op.AutoSave == 1) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		// アカウントの設定
		case ID_MENUITEM_SETMAILBOX:
			if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
				if (SetSaveBoxName(hWnd) == FALSE) {
					break;
				}
			} else if (SetMailBoxOption(hWnd) == FALSE) {
				break;
			}
			if (op.AutoSave == 1) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		//of account Deletion
		case ID_MENUITEM_DELETEMAILBOX:
			if (SelBox == MAILBOX_SEND || SelBox == RecvBox) {
				break;
			}
			if (MessageBox(hWnd, STR_Q_DELMAILBOX, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
				break;
			}
			if (g_soc != -1 && SelBox < RecvBox) {
				RecvBox--;
			}
			mailbox_select(hWnd, mailbox_delete(hWnd, SelBox, TRUE));
			if (op.AutoSave == 1) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, TRUE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		//of account Account on portable
		case ID_MENUITEM_MOVEUPMAILBOX:
			mailbox_move_up(hWnd);
			if (op.AutoSave == 1) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		//Account under portable
		case ID_MENUITEM_MOVEDOWNMAILBOX:
			mailbox_move_down(hWnd);
			if (op.AutoSave == 1) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		//In idea contest order sort
		case ID_MENUITEM_ICONSORT:
			op.LvThreadView = 0;
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
			CheckMenuItem(GetSubMenu(hMainMenu, 1), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#else
			CheckMenuItem(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#endif
#else
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
#endif
			LvSortFlag = (ABS(LvSortFlag) == (SORT_ICON + 1)) ? (LvSortFlag * -1) : (SORT_ICON + 1);
			//Sort
			SwitchCursor(FALSE);
			ListView_SortItems(GetDlgItem(hWnd, IDC_LISTVIEW), CompareFunc, LvSortFlag);
			SwitchCursor(TRUE);

			ListView_EnsureVisible(GetDlgItem(hWnd, IDC_LISTVIEW),
				ListView_GetNextItem(GetDlgItem(hWnd, IDC_LISTVIEW), -1, LVNI_FOCUSED), TRUE);

			if (op.LvAutoSort == 2 || (MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
				op.LvSortItem = LvSortFlag;
			}
			break;

		//Thread indicatory
		case ID_MENUITEM_THREADVIEW:
			SwitchCursor(FALSE);
			if (op.LvThreadView == 1) {
				op.LvThreadView = 0;
				ListView_SortItems(GetDlgItem(hWnd, IDC_LISTVIEW), CompareFunc, LvSortFlag);
			} else {
				op.LvThreadView = 1;
				item_create_thread(MailBox + SelBox);
				ListView_SortItems(GetDlgItem(hWnd, IDC_LISTVIEW), CompareFunc, SORT_THREAD + 1);
			}
			SwitchCursor(TRUE);

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
			CheckMenuItem(GetSubMenu(hMainMenu, 1), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
			CheckMenuItem(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
#else
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

			ListView_EnsureVisible(GetDlgItem(hWnd, IDC_LISTVIEW),
				ListView_GetNextItem(GetDlgItem(hWnd, IDC_LISTVIEW), -1, LVNI_FOCUSED), TRUE);
			break;

		//Initialization
		case ID_MENUITEM_LISTINIT:
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INITMAILBOX),
				hWnd, InitMailBoxProc, (LPARAM)(MailBox + SelBox)) == FALSE) {
				break;
			}
			mem_free(&(MailBox + SelBox)->LastMessageId);
			(MailBox + SelBox)->LastMessageId = NULL;
			break;

		//of new arrival acquisition position ====== sending and receiving =========
		//New arrival check
		case ID_MENUITEM_RECV:
			if (g_soc != -1) {
				break;
			}
			if (SaveBoxesLoaded == FALSE) {
				if (mailbox_load_now(hWnd, SelBox, FALSE, TRUE) != 1) {
					break;
				}
			}
			i = SelBox;
			AutoCheckFlag = FALSE;
			// ダイヤルアップ開始
			if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, i, 0) == FALSE) {
				break;
			}
			if (op.SocLog > 1) {
				TCHAR msg[BUF_SIZE];
				wsprintf(msg, TEXT("Check: box=%d"), SelBox);
				log_save(AppDir, LOG_FILE, msg);
			}
			AllCheck = FALSE;
			ExecFlag = FALSE;
			KeyShowHeader = FALSE;
			NewMailCnt = 0;
			Init_NewMailFlag(hWnd);

			//Mail reception start
			RecvMailList(hWnd, i, FALSE);
			break;

		//Round check
		case ID_MENUITEM_ALLCHECK:
			if (g_soc != -1 || ShowError == TRUE) {
				break;
			}
			if (SaveBoxesLoaded == FALSE) {
				BOOL err = FALSE;
				for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
					if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->CyclicFlag == 0) {
						if (mailbox_load_now(hWnd, i, FALSE, TRUE) != 1) {
							err = TRUE;
							break;
						}
					}
				}
				if (err == TRUE) {
					break;
				}
				SaveBoxesLoaded = TRUE; // may become false if filter is added
			}
			if (op.SocLog > 1) log_save(AppDir, LOG_FILE, TEXT("Check all"));
			AutoCheckCnt = 0; // reset autocheck timer
			AutoCheckFlag = FALSE;
			AllCheck = TRUE;
			ExecFlag = FALSE;
			KeyShowHeader = FALSE;
			NewMailCnt = 0;
			CheckBox = MAILBOX_USER - 1;
			Init_NewMailFlag(hWnd);

			gSockFlag = TRUE;
			SetTimer(hWnd, ID_CHECK_TIMER, CHECKTIME, NULL);
			SendMessage(hWnd, WM_TIMER, ID_CHECK_TIMER, 0);
			break;

		//Those which it marks the execution
		case ID_MENUITEM_EXEC:
			if (g_soc != -1) {
				break;
			}
			KeyShowHeader = (GetKeyState(VK_SHIFT) < 0) ? TRUE : FALSE;
			ServerDelete = FALSE;

			if (item_get_next_delete_mark((MailBox + SelBox), -1, NULL) != -1) {
				int ans = ParanoidMessageBox(hWnd, STR_Q_DELSERVERMAIL,
					STR_TITLE_EXEC, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
				if (ans == IDYES) {
					ServerDelete = TRUE;
				} else if (ans == IDCANCEL) {
					ShowError = FALSE;
					break;
				}
			}
			if (ServerDelete == FALSE &&
				item_get_next_download_mark((MailBox + SelBox), -1, NULL) == -1 &&
				item_get_next_send_mark((MailBox + SelBox), FALSE) == -1) {
				MessageBox(hWnd,
					STR_MSG_NOMARK, STR_TITLE_EXEC, MB_ICONEXCLAMATION | MB_OK);
				break;
			}
			if (op.SocLog > 1) {
				TCHAR msg[BUF_SIZE];
				wsprintf(msg, TEXT("Update: box=%d, delete=%d"), SelBox, ServerDelete);
				log_save(AppDir, LOG_FILE, msg);
			}
			i = SelBox;
			AutoCheckFlag = FALSE;
			// ダイヤルアップ開始
			if (op.RasCon == 1 && i >= MAILBOX_USER && SendMessage(hWnd, WM_RAS_START, i, 0) == FALSE) {
				break;
			}
			AllCheck = FALSE;
			ExecFlag = TRUE;
			NewMailCnt = -1;
			if (op.CheckAfterUpdate == 1 && SelBox != MAILBOX_SEND) {
				NewMailCnt = 0;
				Init_NewMailFlag(hWnd);
				if (SaveBoxesLoaded == FALSE) {
					if (mailbox_load_now(hWnd, SelBox, FALSE, TRUE) != 1) {
						break;
					}
				}
			}
			//Mark execution
			ExecItem(hWnd, i);
			break;

		//Round execution
		case ID_MENUITEM_ALLEXEC:
			if (g_soc != -1) {
				break;
			}
			KeyShowHeader = (GetKeyState(VK_SHIFT) < 0) ? TRUE : FALSE;

			if (SaveBoxesLoaded == FALSE) {
				BOOL err = FALSE;
				for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
					if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->CyclicFlag == 0) {
						BOOL do_saveboxes = op.CheckAfterUpdate || (MailBox+i)->FilterEnable == 2;
						if (mailbox_load_now(hWnd, i, FALSE, do_saveboxes) != 1) {
							err = TRUE;
							break;
						}
					}
				}
				if (err == TRUE) {
					break;
				}
				SaveBoxesLoaded = op.CheckAfterUpdate; // may become false if filter is added
			}
			if (MailMarkCheck(hWnd, FALSE) == FALSE) {
				break;
			}
			if (op.SocLog > 1) {
				TCHAR msg[BUF_SIZE];
				wsprintf(msg, TEXT("Update all: delete=%d"), SelBox, ServerDelete);
				log_save(AppDir, LOG_FILE, msg);
			}

			AutoCheckFlag = FALSE;
			AllCheck = TRUE;
			ExecFlag = TRUE;
			CheckBox = MAILBOX_USER - 1;
			NewMailCnt = -1;
			if (op.CheckAfterUpdate == 1) {
				NewMailCnt = 0;
				Init_NewMailFlag(hWnd);
			}

			gSockFlag = TRUE;
			SetTimer(hWnd, ID_EXEC_TIMER, CHECKTIME, NULL);
			SendMessage(hWnd, WM_TIMER, ID_EXEC_TIMER, 0);
			break;

		//Discontinuance
		case ID_MENUITEM_STOP:
			KillTimer(hWnd, ID_SMTP_TIMER);
			KillTimer(hWnd, ID_SMTP_ONE_TIMER);
			KillTimer(hWnd, ID_CHECK_TIMER);
			KillTimer(hWnd, ID_EXEC_TIMER);
			KillTimer(hWnd, ID_TIMEOUT_TIMER);
			KillTimer(hWnd, ID_RASWAIT_TIMER);
			gSockFlag = FALSE;
			AllCheck = FALSE;
			AutoCheckFlag = FALSE;
			ExecFlag = FALSE;
			KeyShowHeader = FALSE;
			NewMailCnt = -1;
			if (RasLoop == TRUE || (g_soc == -1 && op.RasCheckEndDisCon == 1)) {
				RasDisconnect();
			}
			if (g_soc == -1 || GetHostFlag == TRUE) {
				g_soc = -1;
				RecvBox = -1;
				EndSocketFunc(hWnd, FALSE);
				break;
			}
			if (command_status == POP_RETR || command_status == POP_TOP) {
				pop3_salvage_buffer(hWnd, (MailBox + RecvBox), RecvBox == SelBox);
			}
			if (command_status == POP_QUIT || command_status == POP_START) {
				socket_close(hWnd, g_soc);
				g_soc = -1;
				RecvBox = -1;
				SetItemCntStatusText(hWnd, NULL, FALSE);
				SetUnreadCntTitle(hWnd, TRUE);
				EndSocketFunc(hWnd, FALSE);
				break;
			}
			command_status = POP_QUIT;
			SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
			send_buf(g_soc, CMD_RSET"\r\n");
			SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
			send_buf(g_soc, CMD_QUIT"\r\n");
			break;

		//====== mail =========
		//You open the
		case ID_KEY_ENTER:
			if (GetFocus() == GetDlgItem(hWnd, IDC_COMBO)) {
				SendDlgItemMessage(hWnd, IDC_COMBO, CB_SHOWDROPDOWN,
					!SendDlgItemMessage(hWnd, IDC_COMBO, CB_GETDROPPEDSTATE, 0, 0), 0);
				break;
			}
			// else fall through

		case ID_MAILITEM_OPEN:
			OpenItem(hWnd, FALSE, FALSE);
			break;

		case ID_MENUITEM_OPEN:
			OpenItem(hWnd, TRUE, FALSE);
			break;

		//Reply
		// (for SENDBOX, this menuitem ID is associated with "Property")
		case ID_MENUITEM_REMESSEGE:
			ReMessageItem(hWnd, EDIT_REPLY);
			break;

		//ReplyAll
		case ID_MENUITEM_ALLREMESSEGE:
			ReMessageItem(hWnd, EDIT_REPLYALL);
			break;

		//Forward
		case ID_MENUITEM_FORWARD:
			ReMessageItem(hWnd, EDIT_FORWARD);
			break;

		//Change
		case ID_KEY_CTRLENTER:
			if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || (SelBox == RecvBox && ExecFlag == TRUE)) {
				break;
			}
			SetDownloadMark(hWnd, TRUE);
			break;

		//of mark for reception In one for reception mark
		case ID_MENUITEM_DOWNMARK:
			if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || (SelBox == RecvBox && ExecFlag == TRUE)) {
				break;
			}
			SetDownloadMark(hWnd, FALSE);
			break;

		//In one for deletion mark
		case ID_MENUITEM_DELMARK:
			if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || SelBox == MAILBOX_SEND || (SelBox == RecvBox && ExecFlag == TRUE)) {
				break;
			}
			SetDeleteMark(hWnd);
			break;

		//Mark cancellation
		case ID_MENUITEM_UNMARK:
			if (SelBox == RecvBox && ExecFlag == TRUE) {
				break;
			}
			UnMark(hWnd);
			break;

		//It makes the opening being completed the
		case ID_MENUITEM_READMAIL:
			if (SelBox == MAILBOX_SEND) {
				break;
			}
			SetMailStats(hWnd, ICON_READ);
			break;

		//Not yet you open the
		case ID_MENUITEM_UNREADMAIL:
			if (SelBox == MAILBOX_SEND) {
				break;
			}
			SetMailStats(hWnd, ICON_MAIL);
			break;

		case ID_MENUITEM_MOVESAVE:
			mark_del = TRUE; // and fall through to do the move
		case ID_MENUITEM_SAVECOPY:
			{
				int i, cnt = 0, Target = -1;
				if (SelBox == MAILBOX_SEND && mark_del == FALSE) {
					// (in SendBox, Ctrl-C does "create copy"
					Target = MAILBOX_SEND;
				} else {
					for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
						if ((MailBox + i)->Type == MAILBOX_TYPE_SAVE) {
							Target = i;
							cnt++;
						}
					}
					if (cnt == 0) {
						MessageBox(hWnd, STR_ERR_NOSAVEBOXES, WINDOW_TITLE, MB_OK);
						Target = -1;
					} else if (cnt > 1) {
						Target = mark_del;
						if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SELSAVEBOX), hWnd, SelSaveBoxProc, (LPARAM)&Target) == FALSE) {
							Target = -1;
						}
					}
				}
				if (Target != -1) {
					if (ItemToSaveBox(hWnd, NULL, Target, (cnt<=1), mark_del) == TRUE) {
						if (mark_del == TRUE) {
							if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || SelBox == MAILBOX_SEND) {
								ListDeleteItem(hWnd, FALSE);
							} else {
								SetDeleteMark(hWnd);
							}
						}
						if (op.AutoSave == 1) {
							// save Target mailbox
							if (Target == MAILBOX_SEND) {
								file_save_mailbox(SENDBOX_FILE, DataDir, MailBox + MAILBOX_SEND, FALSE, 2);
							} else {
								TCHAR buf[BUF_SIZE];
								if ((MailBox + Target)->Filename == NULL) {
									wsprintf(buf, TEXT("MailBox%d.dat"), Target - MAILBOX_USER);
								} else {
									lstrcpy(buf, (MailBox + Target)->Filename);
								}
								file_save_mailbox(buf, DataDir, MailBox+Target, FALSE, 2);
							}
						}
					} else {
						ErrorMessage(hWnd, STR_ERR_SAVECOPY);
					}
				}
			}
			break;

		//of retention box From summary deletion
		case ID_MENUITEM_DELETE:
			ListDeleteItem(hWnd, TRUE);
			break;

		//delete attachments
		case ID_MENUITEM_DELATTACH:
			if (SelBox != MAILBOX_SEND) {
				ListDeleteAttach(hWnd);
			}
			break;

		//Entirely the selective
		case ID_MENUITEM_ALLSELECT:
			SetFocus(GetDlgItem(hWnd, IDC_LISTVIEW));
			ListView_SetItemState(GetDlgItem(hWnd, IDC_LISTVIEW), -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		//Change
		case ID_MENUITEM_CHANGEMAILBOX:
			SetFocus(GetDlgItem(hWnd, IDC_COMBO));
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_SHOWDROPDOWN, TRUE, 0);
			break;

#ifdef _WIN32_WCE_PPC
		//of account Selective mode
		case ID_MENUITEM_SELMODE:
			SelMode = !SelMode;
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), ID_MENUITEM_SELMODE, (SelMode == TRUE) ? MF_CHECKED : MF_UNCHECKED);
			break;
#elif defined(_WIN32_WCE_LAGENDA)
		//Selective mode
		case ID_MENUITEM_SELMODE:
			SelMode = !SelMode;
			CheckMenuItem(GetSubMenu(hMainMenu, 1), ID_MENUITEM_SELMODE, (SelMode == TRUE) ? MF_CHECKED : MF_UNCHECKED);
			break;
#endif

		//Indicatory
		case ID_MENUITEM_RESTORE:
#ifdef _WIN32_WCE
			CloseViewWindow(1);
			FocusWnd = hWnd;
			ShowWindow(hWnd, SW_SHOW);
#else
			if (op.ShowPass == 1 &&
				(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
				op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password) == FALSE) {
				break;
			}
			ShowWindow(hWnd, SW_SHOW);
			if (IsIconic(hWnd) != 0) {
				ShowWindow(hWnd, SW_RESTORE);
			}
#endif
			_SetForegroundWindow(hWnd);
			break;

		//of summary picture Indicatory
		case ID_MENUITEM_SHOWLASTWINDOW:
			SendMessage(hWnd, WM_SHOWLASTWINDOW, 0, 0);
			break;

		//of window Searching
		case ID_MENUITEM_FIND:
			View_FindMail(hWnd, TRUE);
			break;

		//The next searching
		case ID_MENUITEM_NEXTFIND:
			View_FindMail(hWnd, FALSE);
			break;

		default:
			if (command_id == ID_MENUITEM_COPY2NEW) {
				// GJC - copy to new SaveBox
				int old_selbox, newbox;
				old_selbox = SelBox;
				SelBox = newbox = mailbox_create(hWnd, 1, TRUE, FALSE);
				if (SetMailBoxType(hWnd, MAILBOX_ADD_SAVE) == -1) {
					mailbox_delete(hWnd, newbox, FALSE);
					command_id = 0;
				} else {
					command_id = newbox + ID_MENUITEM_COPY2MBOX;
					// and fall through to do the copy
				}
				SelBox = old_selbox;
			} else if (command_id == ID_MENUITEM_MOVE2NEW) {
				// GJC - move to new SaveBox
				int old_selbox, newbox;
				old_selbox = SelBox;
				SelBox = newbox = mailbox_create(hWnd, 1, TRUE, FALSE);
				if (SetMailBoxType(hWnd, MAILBOX_ADD_SAVE) == -1) {
					mailbox_delete(hWnd, newbox, FALSE);
					command_id = 0;
				} else {
					command_id = newbox + ID_MENUITEM_MOVE2MBOX;
					// and fall through to do the move
				}
				SelBox = old_selbox;
			}
			if (command_id >= ID_MENUITEM_COPY2MBOX) {
				// move or copy to SaveBox
				BOOL mark_del = FALSE;
				mbox = command_id - ID_MENUITEM_COPY2MBOX;
				if (command_id >= ID_MENUITEM_MOVE2MBOX) {
					mbox = command_id - ID_MENUITEM_MOVE2MBOX;
					mark_del = TRUE;
				}
				if (mbox >=0 && mbox < MailBoxCnt && (MailBox+mbox) != NULL) {
					if (ItemToSaveBox(hWnd, NULL, mbox, TRUE, mark_del) == TRUE) {
						if (mark_del == TRUE) {
							// delete from list or mark for deletion
							if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || SelBox == MAILBOX_SEND) {
								ListDeleteItem(hWnd, FALSE);
							} else {
								SetDeleteMark(hWnd);
							}
						}
						if (op.AutoSave == 1) {
							if (mbox == MAILBOX_SEND) {
								file_save_mailbox(SENDBOX_FILE, DataDir, MailBox + MAILBOX_SEND, FALSE, 2);
							} else {
								TCHAR buf[BUF_SIZE];
								if ((MailBox + mbox)->Filename == NULL) {
									wsprintf(buf, TEXT("MailBox%d.dat"), mbox - MAILBOX_USER);
								} else {
									lstrcpy(buf, (MailBox + mbox)->Filename);
								}
								file_save_mailbox(buf, DataDir, MailBox+mbox, FALSE, 2);
							}
						}
					} else {
						ErrorMessage(hWnd, STR_ERR_SAVECOPY);
					}
				}
			}
			break;
		} // end switch(command_id)
		}
		break;

	//Start
	case WM_RAS_START:
		return RasMailBoxStart(hWnd, wParam);

#ifndef _WIN32_WCE
	case WM_SYSCOMMAND:
		if (op.ShowPass == 1 &&
			(wParam == SC_RESTORE || wParam == SC_MAXIMIZE) &&
			(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
			op.Password != NULL && *op.Password != TEXT('\0') &&
			ConfirmPass(hWnd, op.Password) == FALSE) {
			return 0;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
#endif

	//of dial rise Task tray message
	case WM_TRAY_NOTIFY:
#ifdef _WIN32_WCE_LAGENDA
		EnableMenuItem(GetSubMenu(hPOPUP, 0), ID_MENUITEM_ALLCHECK, !(g_soc == -1));
		EnableMenuItem(GetSubMenu(hPOPUP, 0), ID_MENUITEM_STOP, (g_soc == -1));
		SendMessage(hWnd, WM_NULL, 0, 0);
		ShowMenu(hWnd, hPOPUP, 0, 2, FALSE);
#else
		switch (lParam) {
#ifdef _WIN32_WCE
		case WM_LBUTTONUP:
			EnableMenuItem(GetSubMenu(hPOPUP, 0), ID_MENUITEM_ALLCHECK, !(g_soc == -1));
			EnableMenuItem(GetSubMenu(hPOPUP, 0), ID_MENUITEM_STOP, (g_soc == -1));
			SendMessage(hWnd, WM_NULL, 0, 0);
			ShowMenu(hWnd, hPOPUP, 0, 2, FALSE);
			break;
#else
		case WM_LBUTTONDOWN:
			if (op.TrayIconToggle == 1 && IsIconic(hWnd) == 0 && IsWindowVisible(hWnd) != 0) {
				ShowWindow(hWnd, SW_MINIMIZE);
			} else {
				SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_RESTORE, 0);
			}
			break;

		case WM_RBUTTONUP:
			SetMenuDefaultItem(GetSubMenu(hPOPUP, 0), ID_MENUITEM_RESTORE, 0);
			EnableMenuItem(GetSubMenu(hPOPUP, 0), ID_MENUITEM_ALLCHECK, !(g_soc == -1));
			EnableMenuItem(GetSubMenu(hPOPUP, 0), ID_MENUITEM_STOP, (g_soc == -1));
			SendMessage(hWnd, WM_NULL, 0, 0);
			ShowMenu(hWnd, hPOPUP, 0, 0, FALSE);
			break;
#endif
		}
#endif
		break;

#ifdef WSAASYNC
	case WM_SOCK_SELECT:
		if (g_soc != -1 && WSAGETSELECTERROR(lParam) != 0) {
			ErrorSocketEnd(hWnd, RecvBox);
			switch (WSAGETSELECTEVENT(lParam)) {
			case FD_CONNECT:
				{
					TCHAR buf[BUF_SIZE];
					TCHAR *hname;
					if (RecvBox == MAILBOX_SEND) {
						(MailBox+RecvBox)->SmtpIP = 0; // clear cached IP
						hname = (MailBox+RecvBox)->SmtpServer;
					} else {
						(MailBox+RecvBox)->PopIP = 0; // clear cached IP
						hname = (MailBox+RecvBox)->Server;
					}
					wsprintf(buf, TEXT("%s (%s)"), STR_ERR_SOCK_CONNECT, hname); 
					SocketErrorMessage(hWnd, buf, RecvBox);
				}
				break;

			case FD_READ:
			case FD_WRITE:
				SocketErrorMessage(hWnd, STR_ERR_SOCK_SENDRECV, RecvBox);
				break;

			default:
				SocketErrorMessage(hWnd, STR_ERR_SOCK_DISCONNECT, RecvBox);
				break;
			}
			break;
		}
		// It should process the socket or decision
		if (g_soc != (int)wParam) {
			break;
		}
		// which It processes every socket event the
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_CONNECT:					/* サーバへの接続が完了した事を示すイベント */
			{
				TCHAR ErrStr[BUF_SIZE];
				*ErrStr = TEXT('\0');
				if (init_ssl(hWnd, g_soc, ErrStr) == -1) {
					ErrorSocketEnd(hWnd, RecvBox);
					SocketErrorMessage(hWnd, ErrStr, RecvBox);
				}
			}
			break;

		case FD_READ:						/* 受信バッファにデータがある事を示すイベント */
			/* データを受信して蓄積する */
			if (recv_proc(hWnd, g_soc) == SELECT_MEM_ERROR) {
				// メモリエラー
				ErrorSocketEnd(hWnd, RecvBox);
				SocketErrorMessage(hWnd, STR_ERR_MEMALLOC, RecvBox);
				break;
			}
			SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);
			break;

		case FD_WRITE:						/* 送信可能な事を示すイベント */
			{
				TCHAR ErrStr[BUF_SIZE];
				*ErrStr = TEXT('\0');
				if (smtp_send_proc(hWnd, g_soc, ErrStr) == FALSE) {
					ErrorSocketEnd(hWnd, RecvBox);
					SocketErrorMessage(hWnd, ErrStr, RecvBox);
					break;
				}
				SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);
			}
			break;

		case FD_CLOSE:						/* サーバへの接続が終了した事を示すイベント */
			/* 接続を終了する */
			if (command_status != POP_QUIT) {
				if (command_status == POP_RETR || command_status == POP_TOP) {
					pop3_salvage_buffer(hWnd, (MailBox + RecvBox), RecvBox == SelBox);
				}
				ErrorSocketEnd(hWnd, RecvBox);
				SocketErrorMessage(hWnd, STR_ERR_SOCK_DISCONNECT, RecvBox);
			} else {
				socket_close(hWnd, g_soc);
				g_soc = -1;
				KillTimer(hWnd, ID_TIMEOUT_TIMER);
				AutoSave_Mailboxes(hWnd);
				SetItemCntStatusText(hWnd, NULL, FALSE);
				SetUnreadCntTitle(hWnd, TRUE);
				if (AllCheck == FALSE) {
					if (op.CheckEndExec == 1 &&
						CheckEndAutoExec(hWnd, RecvBox, NewMailCnt, FALSE) == TRUE) {
						//After the checking execution
						break;
					}
					if (ExecFlag == TRUE && op.CheckAfterUpdate == 1 && RecvBox != MAILBOX_SEND) {
						// 実行後チェック
						ExecFlag = FALSE;
						RecvMailList(hWnd, RecvBox, FALSE);
						break;
					}
					RecvBox = -1;
					EndSocketFunc(hWnd, TRUE);
					NewMail_Message(hWnd, NewMailCnt);
				} else {
					RecvBox = -1;
					SetMailMenu(hWnd);
				}
			}
			break;
		}
		break;
#endif

	//Reception message
	case WM_SOCK_RECV:
		{
			TCHAR ErrStr[BUF_SIZE];

			*ErrStr = TEXT('\0');
			if (command_proc(hWnd, g_soc, (char *)lParam, wParam, ErrStr,
				(MailBox + RecvBox), RecvBox == SelBox) == FALSE) {
				ErrorSocketEnd(hWnd, RecvBox);
				if (*ErrStr != TEXT('\0')) {
					SocketErrorMessage(hWnd, ErrStr, RecvBox);
				} else {
					RecvBox = -1;
					SetMailMenu(hWnd);
				}
				return FALSE;
			}
		}
		return TRUE;

	//of socket Mail transmission
	case WM_SMTP_SENDMAIL:
		if (g_soc != -1 || lParam == 0) {
			break;
		}
		wkSendMailItem = (MAILITEM *)lParam;
		NewMailCnt = -1;
		SmtpWait = 0;
		CheckBox = mailbox_name_to_index(wkSendMailItem->MailBox);
		if (CheckBox != -1 && (MailBox + CheckBox)->PopBeforeSmtp != 0) {
			// POP before SMTP
			AutoCheckFlag = FALSE;
			if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
				ErrorSocketEnd(hWnd, CheckBox);
				SetMailMenu(hWnd);
				break;
			}
			AllCheck = TRUE;
			gSockFlag = TRUE;
			if (op.PopBeforeSmtpIsLoginOnly == 0) NewMailCnt = 0;
			RecvMailList(hWnd, CheckBox, (op.PopBeforeSmtpIsLoginOnly == 1) ? TRUE : FALSE);
			SmtpWait = op.PopBeforeSmtpWait / SMTPTIME;
		}
		SetTimer(hWnd, ID_SMTP_ONE_TIMER, SMTPTIME, NULL);
		break;

	//Event
	case WM_LV_EVENT:
		switch (wParam) {
		case LVN_ITEMCHANGED:
			// GJC sound if single selection, based on mailitem's Mark
			i = SetMailMenu(hWnd);
			if (i >= 0 && op.ItemPlaySound) {
				PlayMarkSound(i);
			}
			break;

#ifdef _WIN32_WCE_LAGENDA
		case LVN_BEGINLABELEDIT:
			if (SelMode == TRUE) {
				break;
			}
			SendMessage(hWnd, WM_COMMAND, ID_MAILITEM_OPEN, 0);
			return TRUE;
#endif	// _WIN32_WCE_LAGENDA

#ifdef _WIN32_WCE_PPC
		case LVN_BEGINDRAG:
			SetCapture(GetDlgItem(hWnd, IDC_LISTVIEW));
			break;

		case LVN_ITEMACTIVATE:
			SendMessage(hWnd, WM_COMMAND, ID_MAILITEM_OPEN, 0);
			break;

#else	// _WIN32_WCE_PPC
		case NM_CLICK:
#ifdef _WIN32_WCE
			if (GetKeyState(VK_MENU) < 0) {
				SendMessage(hWnd, WM_COMMAND, ID_MENU, 0);
				break;
			}
#endif	// _WIN32_WCE
			break;

		case NM_DBLCLK:
#ifdef _WIN32_WCE_LAGENDA
			if (SelMode == TRUE) {
				break;
			}
#endif	// _WIN32_WCE_LAGENDA
			SendMessage(hWnd, WM_COMMAND, ID_MAILITEM_OPEN, 0);
			break;

		case NM_RCLICK:
			SendMessage(hWnd, WM_COMMAND, ID_MENU, 0);
			break;
#endif	// _WIN32_WCE_PPC
		}
		break;

	//of list view In status bar character string indicatory
	case WM_STATUSTEXT:
		SetStatusTextT(hWnd, (TCHAR *)lParam, 1);
		break;

	//Indicatory
	case WM_SHOWLASTWINDOW:
#ifdef _WIN32_WCE
		if (IsWindow(FocusWnd) == 0) {
			FocusWnd = hWnd;
		}
		ShowWindow(FocusWnd, SW_SHOW);
		_SetForegroundWindow(FocusWnd);
#else
		SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_RESTORE, 0);
#endif
		if (op.StartCheck == 1 && gSendAndQuit == FALSE) {
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLCHECK, 0);
		}
		break;

	default:
		if (msg == WM_RASEVENT) {
			return RasStatusProc(hWnd, msg, wParam, lParam);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * InitApplication - ウィンドウクラスの登録
 */
static BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;

#ifdef _WIN32_WCE
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = NULL;
	wc.lpszMenuName = NULL;
#else
	wc.style = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_WINDOW);
#endif
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_MAIN));
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
	wc.lpszClassName = MAIN_WND_CLASS;

	return RegisterClass(&wc);
}

/*
 * InitInstance - ウィンドウの作成
 */
static HWND InitInstance(HINSTANCE hInstance, int CmdShow)
{
	HWND hwndMain =  NULL;
#ifdef _WIN32_WCE_PPC
	SIPINFO si;
	int x = CW_USEDEFAULT, y = CW_USEDEFAULT, cx, cy;
#endif

	hInst = hInstance;

#ifdef _WIN32_WCE
	if (GetSystemMetrics(SM_CXSCREEN) >= 330 && GetSystemMetrics(SM_CYSCREEN) >= 330) {
		gAddressDialogResource = IDD_DIALOG_ADDRESS_BIG;
	}
#ifdef _WIN32_WCE_PPC
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	SHSipInfo(SPI_GETSIPINFO, 0, &si, 0);
	cx = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
//	cy = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top - ((si.fdwFlags & SIPF_ON) ? 0 : MENU_HEIGHT);
	cy = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top;
	if (si.fdwFlags & SIPF_ON) {
		cy += si.rcSipRect.bottom - si.rcSipRect.top;
	} else {
		cy -= MENU_HEIGHT;
	}

	hwndMain = CreateWindow(MAIN_WND_CLASS,
		WINDOW_TITLE,
		WS_VISIBLE,
		x, y, cx, cy,
		NULL, NULL, hInstance, NULL);
#else
	hwndMain = CreateWindow(MAIN_WND_CLASS,
		WINDOW_TITLE,
		WS_VISIBLE,
		0, 0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);
#endif
#else
	hwndMain = CreateWindow(MAIN_WND_CLASS,
		WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW,
		0, 0,
		240,
		320,
		NULL, NULL, hInstance, NULL);
#endif

	if (!hwndMain) {
		return NULL;
	}

	if (op.ShowTrayIcon == 1 && op.StartHide == 1) {
		return hwndMain;
	}
#ifndef _WIN32_WCE
	if ((CmdShow == SW_MINIMIZE || CmdShow == SW_SHOWMINIMIZED) && op.ShowTrayIcon == 1 && op.MinsizeHide == 1) {
		return hwndMain;
	}
#endif

	ShowWindow(hwndMain, CmdShow);
	UpdateWindow(hwndMain);
	return hwndMain;
}

/*
 * MessageFunc - メッセージ処理
 */
BOOL MessageFunc(HWND hWnd, MSG *msg)
{
	HWND fWnd;

	fWnd = GetForegroundWindow();
	if (fWnd == NULL) {

	// window Accelerator
	} else if (fWnd == hWnd &&
		TranslateAccelerator(fWnd, hAccel, msg) == TRUE) {
		return TRUE;

	// view window Accelerator
	} else if (fWnd == hViewWnd &&
		TranslateAccelerator(fWnd, hViewAccel, msg) == TRUE) {
		return TRUE;

	// "you have new mail" window
	} else if (fWnd == MsgWnd &&
		IsDialogMessage(fWnd, msg) != 0) {
		return TRUE;

	// edit window Accelerator
	} else if (TranslateAccelerator(fWnd, hEditAccel, msg) == TRUE) {
		return TRUE;

	}
	TranslateMessage(msg);
	DispatchMessage(msg);
	return TRUE;
}


/*
 * WinMain - メイン
 */
#ifdef _WIN32_WCE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int CmdShow)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
#endif
{
	HWND hWnd;
	WSADATA WsaData;
	HANDLE hMutex = NULL;
	TCHAR *lptCmdLine = (TCHAR *)lpCmdLine;

	hInst = hInstance;

#ifndef _DEBUG
#ifdef _WCE_OLD
	//of mail compilation window Double starting starting prevention
	if ((hWnd = FindWindow(MAIN_WND_CLASS, NULL)) != NULL) {
		if (lpCmdLine != NULL && *lpCmdLine != TEXT('\0')) {
			COPYDATASTRUCT cpdata;

			cpdata.lpData = lpCmdLine;
			cpdata.cbData = sizeof(TCHAR) * (lstrlen(lpCmdLine) + 1);
			SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cpdata);
		} else {
			SendMessage(hWnd, WM_SHOWLASTWINDOW, 0, 0);
		}
		return 0;
	}
#else	// _WCE_OLD
	//Double starting starting prevention
	hMutex = CreateMutex(NULL, TRUE, STR_MUTEX);
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		hWnd = FindWindow(MAIN_WND_CLASS, NULL);
		if (hWnd != NULL) {
			if (lpCmdLine != NULL && *lpCmdLine != TEXT('\0')) {

				if (*lpCmdLine == TEXT('/') && *(lpCmdLine+1) == TEXT('q') ) {
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				} else {
					COPYDATASTRUCT cpdata;

#ifdef _WIN32_WCE
					CmdLine = alloc_copy_t(lpCmdLine);
#else
					CmdLine = alloc_char_to_tchar(lpCmdLine);
#endif
					cpdata.lpData = CmdLine;
#ifdef _WIN32_WCE
					cpdata.cbData = sizeof(TCHAR) * (lstrlen(lpCmdLine) + 1);
#else
					cpdata.cbData = sizeof(TCHAR) * (tstrlen(lpCmdLine) + 1);
#endif
					SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cpdata);
					mem_free(&CmdLine);
				}
			} else {
				SendMessage(hWnd, WM_SHOWLASTWINDOW, 0, 0);
			}
		}
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		return 0;
	}
#endif	// _WCE_OLD
#endif	// _DEBUG

#if defined(UNICODE) && !defined(_WIN32_WCE)
	// Win32 Unicode has char* lpCmdLine!
	lptCmdLine = alloc_char_to_tchar(lpCmdLine);
//	lptCmdLine = alloc_copy_t(GetCommandLineW()); // gets program name also
#endif
	// Sets AppDir and parses lpCmdLine to set IniFile and static CmdLine
	if (GetAppPath(hInstance, lptCmdLine) == FALSE) {
#if defined(UNICODE) && !defined(_WIN32_WCE)
		mem_free(&lptCmdLine);
#endif
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		ErrorMessage(NULL, STR_ERR_MEMALLOC);
		return 0;
	}
#if defined(UNICODE) && !defined(_WIN32_WCE)
		mem_free(&lptCmdLine);
#endif

	{
		int TmpCmdShow;
		//of job pass of application Check
		TmpCmdShow = CmdShow;
		if (ini_start_auth_check() == FALSE) {
			mem_free(&CmdLine);
			mem_free(&AppDir);
			mem_free(&IniFile);
			mem_free(&g_Pass);
			if (hMutex != NULL) {
				CloseHandle(hMutex);
			}
			return 0;
		}
		mem_free(&g_Pass);
		g_Pass = NULL;
		CmdShow = TmpCmdShow;
	}

	//of starting password Initialization
	if (WSAStartup(0x101, &WsaData) != 0) {
		mem_free(&CmdLine);
		mem_free(&AppDir);
		mem_free(&IniFile);
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		ErrorMessage(NULL, STR_ERR_INIT);
		return 0;
	}
#ifndef _WCE_OLD
	charset_init();
#endif
	InitCommonControls();
	initRas();

#ifdef _WIN32_WCE_LAGENDA
	SipShowIM(SIPF_OFF);
#endif
	
	//Window class register
	if (!InitApplication(hInstance) || !View_InitApplication(hInstance)
		|| !Edit_InitApplication(hInstance)) {
		mem_free(&CmdLine);
		mem_free(&AppDir);
		mem_free(&IniFile);
		WSACleanup();
		FreeRas();
#ifndef _WCE_OLD
		charset_uninit();
#endif
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		ErrorMessage(NULL, STR_ERR_INIT);
		return 0;
	}

#ifndef _WCE_OLD
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MSG), NULL, NewMailMessageProc, 0);
#endif

#ifdef USE_NEDIT
	// EDIT登録
	if (nedit_regist(hInstance) == FALSE) {
		return 0;
	}
#endif
	// メインウィンドウの作成
	if ((hWnd = InitInstance(hInstance, CmdShow)) == NULL) {
		FreeAllMailBox();
		mem_free(&CmdLine);
		mem_free(&AppDir);
		mem_free(&IniFile);
		WSACleanup();
		FreeRas();
#ifndef _WCE_OLD
		charset_uninit();
#endif
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		ErrorMessage(NULL, STR_ERR_INIT);
		return 0;
	}

	//of main window From resource pop rise menu load
	hPOPUP = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_POPUP));

	//From resource accelerator load
	hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	hViewAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_VIEW));
	hEditAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_EDIT));

	nBroadcastMsg = RegisterWindowMessage(BROADCAST_STRING);

	//Message loop
	while (1) {
		BOOL iResult;
		MSG msg;
		iResult = GetMessage(&msg, NULL, 0, 0);
		if (iResult == TRUE) {
			MessageFunc(hWnd, &msg);
		} else if (msg.hwnd != 0 || msg.wParam != TIMEOUT_QUIT_WPARAM) {
			break;
		}
	}

	mem_free(&CmdLine);
	mem_free(&AppDir);
	mem_free(&IniFile);
	DestroyMenu(hPOPUP);
	UnregisterClass(MAIN_WND_CLASS, hInstance);
	UnregisterClass(VIEW_WND_CLASS, hInstance);
	UnregisterClass(EDIT_WND_CLASS, hInstance);
	free_ssl();
	WSACleanup();
	FreeRas();
#ifndef _WCE_OLD
	charset_uninit();
#endif
	if (hMutex != NULL) {
		CloseHandle(hMutex);
	}
#ifdef _DEBUG
	mem_debug();
#endif
	return 0;
}

/*
 * ResetTimeoutTimer - when progress is made, in case of slow connection/large message
 */
void ResetTimeoutTimer()
{
	KillTimer(MainWnd, ID_TIMEOUT_TIMER);
	SetTimer(MainWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);
}

/********
 * Paranoid Messagebox
 * Added PHH 4-Oct-2003
 */

int ParanoidMessageBox(HWND hWnd, TCHAR *strMsg, TCHAR *strTitle, unsigned int nStyle)
{
   if (op.ExpertMode == 1)
	{
		return IDYES;
	}
	else
	{
      return MessageBox(hWnd, strMsg, strTitle, nStyle);
	}
}

/*
 * SetMenuStar - add * to IDC_COMBO drop-down
 */
void SetMenuStar(int EntryNum, TCHAR *Name, BOOL UseFlag, BOOL SetCurSel)
{
	TCHAR buf[BUF_SIZE], *p;

	if (UseFlag == FALSE) {
		p = Name;
	} else {
		wsprintf(buf, TEXT("%s *"), Name);
		p = buf;
	}
	SendDlgItemMessage(MainWnd, IDC_COMBO, CB_DELETESTRING, EntryNum, 0);
	SendDlgItemMessage(MainWnd, IDC_COMBO, CB_INSERTSTRING, EntryNum, (LPARAM)p);
	if (SetCurSel) {
		SendDlgItemMessage(MainWnd, IDC_COMBO, CB_SETCURSEL, EntryNum, 0);
	}
}

/*
 * MessageBoxTimer - callback for TimedMessageBox (GJC)
 */
void CALLBACK MessageBoxTimer(HWND hWnd, UINT uiMsg, UINT idEvent, DWORD dwTime)
{
	g_bTimedOut = TRUE;
	if (op.SocLog > 1) log_save(AppDir, LOG_FILE, TEXT("MessageBoxTimer timed out"));
	if (g_hwndTimedOwner)
		EnableWindow(g_hwndTimedOwner, TRUE);
	PostQuitMessage(TIMEOUT_QUIT_WPARAM);
}

/*
 * TimedMessageBox - message box that auto-answers after a delay (GJC)
 */
static int TimedMessageBox(HWND hWnd, TCHAR *strMsg, TCHAR *strTitle, unsigned int nStyle, DWORD dwTimeout)
{
	UINT idTimer;
	int iResult;

	g_hwndTimedOwner = NULL;
	g_bTimedOut = FALSE;

	if (hWnd && IsWindowEnabled(hWnd))
		g_hwndTimedOwner = hWnd;

	idTimer = SetTimer(NULL, 0, dwTimeout*1000, (TIMERPROC)MessageBoxTimer);

	iResult = MessageBox(hWnd, strMsg, strTitle, nStyle);

	KillTimer(NULL, idTimer);
	if (g_bTimedOut) {
		MSG msg;
		PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
	}

	return iResult;
}

/*
 * PlayMarkSound - sound based on Mark (icon status)
 */
static void PlayMarkSound(int mark)
{
	int lowbits = mark & 0x0F;
	if (mark & 0x20) {
		if (op.ItemFullSoundFile != NULL) {
			sndPlaySound(op.ItemFullSoundFile, SND_SYNC | SND_NODEFAULT);
		}
	} else if (op.ItemPartialSoundFile != NULL) {
		sndPlaySound(op.ItemPartialSoundFile, SND_SYNC | SND_NODEFAULT);
	}
	if ((mark & 0x40) && op.ItemAttachSoundFile != NULL) {
		sndPlaySound(op.ItemAttachSoundFile, SND_SYNC | SND_NODEFAULT);
	}
	if ((mark & 0x80) && op.ItemHtmlSoundFile != NULL) {
		sndPlaySound(op.ItemHtmlSoundFile, SND_SYNC | SND_NODEFAULT);
	}
	if (lowbits == ICON_ERROR) {
		if (op.ItemErrorSoundFile != NULL) {
			sndPlaySound(op.ItemErrorSoundFile, SND_ASYNC | SND_NODEFAULT);
		}
	} else if ((mark & 0x10) && op.ItemNewSoundFile != NULL) {
		sndPlaySound(op.ItemNewSoundFile, SND_ASYNC | SND_NODEFAULT);
	} else {
		switch (lowbits) {
			case ICON_MAIL:
				if (op.ItemUnreadSoundFile != NULL) {
					sndPlaySound(op.ItemUnreadSoundFile, SND_ASYNC | SND_NODEFAULT);
				}
				break;
			case ICON_READ:
				if (op.ItemReadSoundFile != NULL) {
					sndPlaySound(op.ItemReadSoundFile, SND_ASYNC | SND_NODEFAULT);
				}
				break;
			case ICON_DOWN:
				if (op.ItemDownSoundFile != NULL) {
					sndPlaySound(op.ItemDownSoundFile, SND_ASYNC | SND_NODEFAULT);
				}
				break;
			case ICON_DEL:
				if (op.ItemDelSoundFile != NULL) {
					sndPlaySound(op.ItemDelSoundFile, SND_ASYNC | SND_NODEFAULT);
				}
				break;
			case ICON_SENTMAIL:
				if (op.ItemSentSoundFile != NULL) {
					sndPlaySound(op.ItemSentSoundFile, SND_ASYNC | SND_NODEFAULT);
				}
				break;
			case ICON_SEND:
				if (op.ItemSendSoundFile != NULL) {
					sndPlaySound(op.ItemSendSoundFile, SND_ASYNC | SND_NODEFAULT);
				}
				break;
			default: // including ICON_NON
				if (op.ItemNonSoundFile != NULL) {
					sndPlaySound(op.ItemNonSoundFile, SND_ASYNC | SND_NODEFAULT);
				}
				break;
			}
	}
	return;
}

/***
 * Console startup
 */

#ifndef _WIN32_WCE
#ifndef UNICODE
#ifndef _DEBUG
void __cdecl WinMainCRTStartup(void)
{
    STARTUPINFO stinfo;
	char *cmdline;
	int ret;

	//Compilation
	cmdline = GetCommandLine();
    if (*cmdline == '"') {
		for (cmdline++; *cmdline != '\0' && *cmdline != '"'; cmdline++);
		if (*cmdline != '\0') cmdline++;
	} else {
		for (; *cmdline != '\0' && *cmdline != ' '; cmdline++);
	}
	for (; *cmdline == ' '; cmdline++);

	//of command line Call
	stinfo.cb = sizeof(STARTUPINFO);
	stinfo.dwFlags = STARTF_USESHOWWINDOW;
	GetStartupInfo(&stinfo);

	//WinMain of information when starting End
	ret = WinMain(GetModuleHandle(NULL), NULL, cmdline, stinfo.wShowWindow);
	//of acquisition
	ExitProcess(ret);
}
int main(void) {
	return 0;
}
#endif
#endif
#endif
/* End of source */
