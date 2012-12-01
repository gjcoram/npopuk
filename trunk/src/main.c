/*
 * nPOP
 *
 * main.c
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
#include "Font.h"
#include "Charset.h"
#ifdef USE_NEDIT
#include "nEdit.h"
#endif
#include "code.h"
#include "mime.h"
#include "profile.h"
#ifdef _WIN32_WCE_PPC
#include "Projects.h"
#endif

/* Define */
#define WM_TRAY_NOTIFY			(WM_APP + 100)		// �^�X�N�g���C
#define WM_FINDMAILBOX			(WM_APP + 101)
#define ID_MAILITEM_OPEN		(WM_APP + 300)		//ID
#ifdef ENABLE_RAS
#define WM_RAS_START			(WM_APP + 400)
#endif
#ifdef ENABLE_WIFI
#define WM_WIFI_START			(WM_APP + 400)
#endif
#define IDC_CB					2000
#define IDC_TB					2001
#define IDC_EDIT_BODY			2003

#define ID_RECV_TIMER			1					//for mail Open Timer ID
#define ID_SMTP_TIMER			2
#define ID_SMTP_ONE_TIMER		3
#define ID_CHECK_TIMER			4
#define ID_EXEC_TIMER			5
#define ID_AUTOCHECK_TIMER		6
#define ID_TIMEOUT_TIMER		7
#define ID_NEWMAIL_TIMER		8
#define ID_RESTORESEL_TIMER		9
//#define ID_RASWAIT_TIMER		10 // in General.h
//#define ID_WIFIWAIT_TIMER		10 // in General.h
#define ID_ITEMCHANGE_TIMER		11
#define ID_PREVIEW_TIMER		12
//#define ID_WIFICHECK_TIMER	13 // in General.h

#define STATUS_DONE				0
#define STATUS_CHECK			1
#define STATUS_NEW				2
#define STATUS_ERROR			3

#define RECVTIME				1					//Timer interval
#define SMTPTIME				100
#define CHECKTIME				100
#define AUTOCHECKTIME			60000
#define TIMEOUTTIME				1000
#define TIMEOUT_QUIT_WPARAM		2007

#define TRAY_ID					100					// �^�X�N�g���CID

#define WNDPROC_KEY				TEXT("OldWndProc")

#define CMD_RSET				"RSET"
#define CMD_QUIT				"QUIT"
 
// Notification Message to external clients
UINT nBroadcastMsg = 0;
#define BROADCAST_STRING		TEXT("NPOP_MESSAGE")

/* Global Variables */
HINSTANCE hInst;							// Local copy of hInstance
TCHAR *AppDir = NULL;						// �A�v���P�[�V�����p�X
TCHAR *DefaultDataDir = NULL;
TCHAR *DataDir = NULL;						// �f�[�^�ۑ���̃p�X
TCHAR *IniFile = NULL;						// ini file specified by /y:
TCHAR *g_Pass = NULL;						// �ꎞ�p�X���[�h
int gPassSt;								// �ꎞ�p�X���[�h�ۑ��t���O
static TCHAR *CmdLine = NULL;				// �R�}���h���C��
static TCHAR *InitialAccount = NULL;
int gAutoSend = FALSE;
BOOL gCheckAndQuit = FALSE;
BOOL gDoingQuit = FALSE;
BOOL first_start = FALSE;					// ����N���t���O
BOOL SaveBoxesLoaded = FALSE;
BOOL PPCFlag;								// PsPC�t���O
#ifdef LOAD_USER_IMAGES
HBITMAP MainBmp = NULL, EditBmp = NULL, ViewBmp = NULL;
#endif
#ifndef _WIN32_WCE
static int confirm_flag;					// �F�؃t���O
#endif
#ifdef ENABLE_WIFI
BOOL WiFiStatus = FALSE;
#endif

HWND MainWnd;								// ���C���E�B���h�E�̃n���h��
HWND FocusWnd;								// �t�H�[�J�X�����E�B���h�E�̃n���h��
HWND mListView;								// mail list
HFONT hListFont = NULL;						// ListView�̃t�H���g
HFONT hViewFont = NULL;						// �\���̃t�H���g
int font_charset;
static HICON TrayIcon_Main = NULL;			// �^�X�N�g���C�A�C�R�� (�ҋ@)
static HICON TrayIcon_Check = NULL;			// �^�X�N�g���C�A�C�R�� (�`�F�b�N��)
static HICON TrayIcon_Mail = NULL;			// �^�X�N�g���C�A�C�R�� (�V������)
BOOL ResizingPreview = FALSE;
BOOL NewMail_Flag = FALSE;					// �^�X�N�g���C�A�C�R���p�V���t���O
static HMENU hMainPop, hPOPUP, hMBPOPUP;	// pop-up menus for main window, systray, mbpane
HMENU hADPOPUP, hViewPop=NULL;				// pop-up menus for Address list, View window
static HANDLE hAccel, hViewAccel, hEditAccel;	// �A�N�Z�����[�^�̃n���h��
#ifdef _WIN32_WCE
HMENU hEditPop=NULL;						// pop-up menu for Edit window
#endif
#ifdef _WIN32_WCE_PPC
HWND hMainToolBar;							// �c�[���o�[ (PocketPC)
int LastXSize = 0;
HMENU hEDITPOPUP;							// pop-up menu for Edit boxes (cut/copy/paste)
char MainMenuOpened=0;
#endif
#ifdef _WIN32_WCE_LAGENDA
static HWND hCSOBar;						// CSO�o�[ (l'agenda)
static RECT wnd_size;						// �����E�B���h�E�T�C�Y (l'agenda)
static int g_menu_height;					// ���j���[�̍��� (l'agenda)
HMENU hMainMenu;							// �E�B���h�E���j���[�̃n���h�� (l'agenda)
#endif
int MailMenuPos;							// ���j���[�ʒu

static WNDPROC ListViewWindowProcedure;		// �T�u�N���X�p�v���V�[�W��(ListView)
static WNDPROC MBPaneWndProc = NULL;
WNDPROC PreviewWindowProcedure = NULL;
int LvSortFlag = 0;							// ListView�̃\�[�g�t���O
BOOL EndThreadSortFlag;						// �ʐM�I�����̎����\�[�g�t���O(�X���b�h�\���p)
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
static BOOL SelMode;						// �I�����[�h (PocketPC, l'agenda)
#endif

MAILBOX *MailBox = NULL;					//which Mailbox
ADDRESSBOOK *AddressBook = NULL;			//Address register
int MailBoxCnt = 2;							//Mailbox several

int SelBox;									// �I�𒆂̃��[���{�b�N�X
int vSelBox;
int RecvBox;								// ����M���̃��[���{�b�N�X
static int CheckBox;						// �`�F�b�N���̃��[���{�b�N�X

SOCKET g_soc = -1;							// �\�P�b�g
BOOL gSockFlag;								// �ʐM���t���O
BOOL GetHostFlag;							// �z�X�g���������t���O
int NewMailCnt = -1;								// �V�����[����
BOOL ShowMsgFlag;							// �V���L��̃��b�Z�[�W�\����
static BOOL ShowError = FALSE;				//During error message indicating
BOOL AutoCheckFlag = FALSE;					//Automatic check
BOOL PopBeforeSmtpFlag = FALSE;				//POP before SMTP
BOOL KeyShowHeader;							//Is by the key the transitory header indicatory flag
static BOOL AllCheck = FALSE;				//which While going around
BOOL ExecFlag = FALSE;				//While executing
BOOL ServerDelete = TRUE;
BOOL CheckAfterThisUpdate = FALSE;
static BOOL ExecCheckFlag = FALSE;			//Check decision
static int AutoCheckCnt = 0;				//of check after the executing With fraction count
static int SmtpWait = 0;					//pop before SMTP to start of automatic operation check waiting after the certifying (milli-second)
static MAILITEM *wkSendMailItem;			//Mail item


typedef BOOL (*PPROC)(HWND, SOCKET, char*, int, TCHAR*, MAILBOX*, BOOL);
static PPROC command_proc;					// ����M�v���V�[�W��
int command_status;							// ����M�R�}���h�X�e�[�^�X (POP_, SMTP_)

HWND g_hwndTimedOwner;
BOOL g_bTimedOut;

// �O���Q��
extern OPTION op;

extern TCHAR *FindStr, *ReplaceStr;			//Search, replace character strings
extern HWND hViewWnd;
extern HWND hEditWnd;
extern HWND MsgWnd;							//Mail arrival message

#ifdef ENABLE_RAS
// RAS
extern UINT WM_RASEVENT;
extern BOOL RasLoop;
extern HANDLE hEvent;
#endif

#ifdef ENABLE_WIFI
// WiFi
extern BOOL WiFiLoop;
extern HANDLE hEvent;
#endif

/* Local Function Prototypes */
static BOOL GetAppPath(HINSTANCE hinst, TCHAR *lpCmdLine);
static BOOL TrayMessage(HWND hWnd, DWORD dwMessage, UINT uID, HICON hIcon);
static void SetTrayIcon(HWND hWnd, HICON hIcon);
static void FreeAllMailBox(void);
static BOOL CloseEditViewWindows(int Flag);
static LRESULT CALLBACK SubClassListViewProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);
static void SetListViewSubClass(HWND hWnd);
static void DelListViewSubClass(HWND hWnd);
static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam);
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam);
#endif
static LRESULT NotifyProc(HWND hWnd, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MBPaneProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MBWidthProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static int CreateMBMenu(HWND hWnd, int Top, int Bottom);
static void CreatePreviewPane(HWND hWnd, int Left, int Top, int width, int height);
static void DelPreviewSubClass(HWND hWnd);
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
static void SetDownloadMark(HWND hWnd);
static void SetFlagOrDeleteMark(HWND hWnd, int Mark, BOOL Clear);
static void UnMark(HWND hWnd);
static void SetMailStats(HWND hWnd, int St);
static void EndSocketFunc(HWND hWnd, BOOL DoTimer);
static BOOL CheckEndAutoExec(HWND hWnd, int SocBox, int cnt, BOOL AllFlag);
static void Init_NewMailFlag(HWND hWnd);
static void NewMail_Message(HWND hWnd, int cnt);
static void AutoSave_Mailboxes(HWND hWnd);
static BOOL AdvOptionEditor(HWND hWnd);
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static BOOL InitApplication(HINSTANCE hInstance);
static HWND InitInstance(HINSTANCE hInstance, int CmdShow);
void CALLBACK MessageBoxTimer(HWND hWnd, UINT uiMsg, UINT idEvent, DWORD dwTime);
static int TimedMessageBox(HWND hWnd, TCHAR *strMsg, TCHAR *strTitle, unsigned int nStyle, DWORD dwTimeout);
static void PlayMarkSound(int mark, int box);
static void PopulatePreviewPane(HWND hWnd, MAILITEM *tpMailItem);
static BOOL GetDroppedStateMBMenu(void);
static void DropMBMenu(BOOL drop);
static void ViewLogFile(HWND hWnd);

#ifdef _WIN32_WCE_LAGENDA
int GetUserDiskName(HINSTANCE hInstance, LPTSTR lpDiskName, int nMaxCount);
#else

/* OP is index into parms; duplicate syntax at end with duplicate op value.
 * OP_MAX_NO_VALUE discriminates between OP without value, and op with value.
 * value accumulates the parsed value of the OP
 * concat: if zero, only one instance allowed; non-zero is multi-value delim.
 *
 * compatibility issues:
 *   OPs 3-10 are redundant with mailto: URL -- no need for both, but if both
 *   exist, 3-10 get concatenated to end of URL
 *
 *   If email adress provided, it is converted to OP_TO.
 *
 *   At end, OPs 3-10 are converted to mailto: URL for use by the rest of the
 *   program.  Means reparsing, but this syntax is only for Windows command
 *   line; the URL parsing code has to exist anyway.
 *
 *   /a: parameter is redundant with /mailbox: for URL use, either accepted
 *   for standalone use.
 */
#define OP_S 0
#define OP_Q 1
#define OP_MAX_NO_VALUE 1
#define OP_Y 2
#define OP_MAILTO 3
#define OP_TO 4
#define OP_CC 5
#define OP_BCC 6
#define OP_REPLY_TO 7
#define OP_SUBJECT 8
#define OP_ATTACH 9
#define OP_BODY 10
#define OP_MAILBOX 11
struct parmdef {
	TCHAR *param;
	TCHAR *value;
	TCHAR concat;
	char op;
	char encode;
	char mailto;
} parms [] = {
	{ PARM_S,        NULL, 0,         OP_S,        1, 0 },
	{ PARM_Q,        NULL, 0,         OP_Q,        1, 0 },
	{ PARM_Y,        NULL, 0,         OP_Y,        1, 0 },
	{ PARM_MAILTO,   NULL, 0,         OP_MAILTO,   1, 1 },
	{ PARM_TO,       NULL, TEXT(','), OP_TO,       3, 1 },
	{ PARM_CC,       NULL, TEXT(','), OP_CC,       3, 1 },
	{ PARM_BCC,      NULL, TEXT(','), OP_BCC,      3, 1 },
	{ PARM_REPLY_TO, NULL, TEXT(','), OP_REPLY_TO, 3, 1 },
	{ PARM_SUBJECT,  NULL, TEXT(' '), OP_SUBJECT,  3, 1 },
	{ PARM_ATTACH,   NULL, TEXT('|'), OP_ATTACH,   3, 1 },
	{ PARM_BODY,     NULL, TEXT('+'), OP_BODY,     1, 1 },
	{ PARM_MAILBOX,  NULL, 0,         OP_MAILBOX,  3, 0 },

#if 1  ||  defined(_WIN32_WCE_PPC)
	// The next group are for handling parameters generated by PPC Contacts
	{ TEXT("-to"),   NULL, TEXT(','), OP_MAILTO,   1, 0 },
#endif

	// The next group are old aliases for newer options.
	{ PARM_A,        NULL, 0,         OP_MAILBOX,  3, 0 }
};
#define NUMPARMDEF (sizeof(parms)/sizeof(struct parmdef))

static void FreeParms(void)
{
	int parmix;
	for (parmix=0; parmix < NUMPARMDEF; parmix++) {
		if (parms[parmix].value) {
			mem_free(&parms[parmix].value);
		}
	}
	return;
}

// returns false for error, true for success
static BOOL MergePath(TCHAR path[BUF_SIZE], TCHAR *file)
{
	TCHAR *p;
	int plen;
	BOOL loop, found = FALSE;

	p = file;

	// Detect fully qualified file name
#ifdef _WIN32_WCE
	if (*p == TEXT('\\') || *p == TEXT('/')) {
		str_cpy_n_t(path, p, BUF_SIZE);
		return TRUE;
	}
#else
	// How to detect drive letter on PC?  Do that first
	// Don't permit drive letter without following \ or /
	if (*p  &&  *(p+1) == TEXT(':')) {
		if (*(p+2) == TEXT('\\') || *(p+2) == TEXT('/')) {
			str_cpy_n_t(path, p, BUF_SIZE);
			return TRUE;
		} else {
			// drive letter not followed by \ or /
			return FALSE;
		}
	}
	if (*p == TEXT('\\') || *p == TEXT('/')) {
		while(trunc_to_parent_dir(path));
	}
#endif

	loop = TRUE;
	while (loop  &&  *p == TEXT('.')) {
		loop = FALSE;
		if (*(p+1) == TEXT('.') && (*(p+2) == TEXT('\\') || *(p+2) == TEXT('/'))) {
			loop = TRUE;
			if (trunc_to_parent_dir(path)) {
				p += 3;
			} else {
				// too many .. for available path
				return FALSE;
			}
		} else if (*(p+1) == TEXT('\\') || *(p+1) == TEXT('/')) {
			loop = TRUE;
			p += 2;
		}
	}

	plen = lstrlen(path);
	str_cpy_n_t(&path[plen], p, BUF_SIZE - plen);
	return TRUE;
}
#endif // _WIN32_WCE_LAGENDA

/*
 * GetAppPath - parse parameters, and figure out where we are running
 */
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
	TCHAR *p, *r, *vp;
	TCHAR fname[MAX_PATH];
	int len, parmix, parmlen, vallen, oldlen;

	AppDir = (TCHAR *)mem_calloc(sizeof(TCHAR) * MAX_PATH);
	if (AppDir == NULL) {
		return FALSE;
	}
	//Pass of application acquisition
	GetModuleFileName(hinst, AppDir, MAX_PATH - 1);
	trunc_to_dirname(AppDir);

	for(p = lpCmdLine; p && *p == TEXT(' '); p++); // remove spaces

	// command-line options should preceed any mailto: arguments
	// valid options listed in parms structure array
	// parsing loop
	while (p && *p) {
		for (parmix = 0; parmix < NUMPARMDEF; parmix++) {
			parmlen = lstrlen(parms[parmix].param);
			if (str_cmp_ni_t(p, parms[parmix].param, parmlen) == 0) {
				if (parmix > OP_MAX_NO_VALUE || *(p+parmlen) == TEXT('\0') || *(p+parmlen) == TEXT(' '))
					break;
			}
		}
		if (parmix >= NUMPARMDEF ) {
			// no match to valid options
			// see if it's a filename
			*fname = TEXT('\0');
#ifdef _WIN32_WCE
			if (*p == TEXT('\\') || (*p == TEXT('"') && *(p+1) == TEXT('\\')) ) {
#else
			if ((*p != TEXT('\0') && *(p+1) == TEXT(':') && *(p+2) == TEXT('\\')) 
				|| (*p == TEXT('"') && *(p+1) != TEXT('\0') && *(p+2) == TEXT(':') && *(p+3) == TEXT('\\'))) {
#endif
				BOOL quoted = FALSE;
				r = p;
				if (*r == TEXT('"')) {
					r++;
					quoted = TRUE;
				}
				for (len = 0; len < MAX_PATH && *r != TEXT('\0'); len++, r++) {
					if ((quoted && *r == TEXT('"')) || (!quoted && *r == TEXT(' '))) {
						break;
					} else {
						fname[len] = *r;
					}
				}
				if (len >= MAX_PATH) {
					len = MAX_PATH-1;
				}
				fname[len] = TEXT('\0');
			}
			if (*fname != TEXT('\0') && file_get_size(fname) > 0) {
				parmix = OP_ATTACH;
				parmlen = 0;
			} else {
				// implicit /to:
				parmix = OP_TO;
				parmlen = 0;
			}
		}
		// found parameter
		if (parmix < OP_MAX_NO_VALUE) {
			// Followed by end of cmdline or space, so no parameter.
			// These are special cases, and are handled right here.
			// Don't matter if seen/done twice.
			if (parmix == OP_S) {
				gAutoSend = AUTOSEND_AND_QUIT;
			} else if (parmix == OP_Q) {
				gCheckAndQuit = TRUE;
			}
			p += parmlen;
			while (*p == TEXT(' ')) p++;
			continue;
		} else {
			TCHAR parse_temp;
			parmix = parms[parmix].op;
			if (parms[parmix].value && !parms[parmix].concat) {
				// Error, parameter specified twice
				TCHAR msg[MSG_SIZE];	
				wsprintf(msg,STR_ERR_DUPPARAM,parms[parmix].param);
				ErrorMessage(NULL, msg);
				mem_free(&AppDir);
				FreeParms();
				return FALSE;
			}
			// must parse value
			if (parmix == OP_MAILTO) {
				// last parameter, no quotes, includes spaces and param name
				p += parmlen; // skip param name
				while (*p == TEXT(' ')) p++; // skip leading spaces
				for (r = p; *r != TEXT('\0'); r++);
#if 1 || defined(_WIN32_WCE_PPC)
				// Or might be from WM5/6 -- still assumed to be at end of line
				// If so, "decode" the \" and eliminate outer quotation marks
				if ( *p == TEXT('"')) {
					while (*(r-1) == TEXT(' ')) r--; // no bounds problem, *p != ' '
					if (r-1 > p && *(r-1) == TEXT('"')) {
						// might be surrounded by " or might have multiple pairs on quotes, one in To, one in CC, one in Body, etc.
						// if we find \" or \\ assume it is WM5/6
						for (vp = p+1; vp < r && *vp != TEXT('\\') && *vp != TEXT('"'); vp++);
						if (vp == r-1) {
							p++;
							r--;
						} else if (*vp == TEXT('\\') && vp+1 < r && (*(vp+1) == TEXT('"') || *(vp+1) == TEXT('\\'))) {
							// must decode, drop outer "
							p++;
							r--;
							*r = TEXT('\0');
							for (r=vp; *vp != TEXT('\0'); vp++) {
								if (*vp != TEXT('\\')) {
									*r = *vp;
									r++;
								}
							}
							*r = TEXT('\0');
						}
					}
				}
#endif
			} else {
				p += parmlen; // skip param name
				while (*p == TEXT(' ')) p++; // skip any leading spaces
				if (*p == TEXT('"')) { // has quotes
					p++;
					for (r = p; *r != TEXT('\0') && *r != TEXT('"'); r++);
				} else { // no quotes
					for (r = p; *r != TEXT('\0') && *r != TEXT(' '); r++);
				}
			}
			// new value between p and r, not including r
			parse_temp = *r;
			*r = TEXT('\0'); // temporarily terminate value
			if (parms[parmix].value) {
				oldlen = lstrlen(parms[parmix].value);
				vallen = (r - p) * parms[parmix].encode;
				vp = (TCHAR *) mem_calloc((oldlen + vallen + 2) * sizeof(TCHAR));
				if (vp == NULL) {
					mem_free(&AppDir);
					FreeParms();
					return FALSE;
				}
				str_cpy_n_t(vp, parms[parmix].value, oldlen + 1);
				vp[oldlen] = parms[parmix].concat;
				if (parms[parmix].encode > 1) {
					URL_encode_t(p, vp + oldlen + 1);
				} else {
					str_cpy_n_t(vp + oldlen + 1, p, vallen + 1);
				}
				mem_free(&parms[parmix].value);
			} else {
				vallen = (r - p) * parms[parmix].encode;
				vp = (TCHAR *) mem_calloc((vallen + 1) * sizeof(TCHAR));
				if (vp == NULL) {
					mem_free(&AppDir);
					FreeParms();
					return FALSE;
				}
				if (parms[parmix].encode > 1) {
					URL_encode_t(p, vp);
				} else {
					str_cpy_n_t(vp, p, vallen + 1);
				}
			}
			parms[parmix].value = vp;
			*r = parse_temp;
			if (*r == TEXT('"'))
				r++;
			for (p = r; *p == TEXT(' '); p++); // skip spaces
		}
	}

	// processing steps

	// if any mailto: type new parameters, bundle them into
	// the "CmdLine" variable in the form of a mailto: URL
	oldlen = 0;
	len = 0;
	for(parmix=OP_MAILTO; parmix < NUMPARMDEF; parmix++) {
		if (parms[parmix].value != NULL) {
			len += parms[parmix].mailto;
			oldlen += lstrlen(parms[parmix].value) + lstrlen(parms[parmix].param);
		}
	}
	if (len) {
		oldlen++;
		if (parms[OP_MAILTO].value == NULL) {
			oldlen += lstrlen(PARM_MAILTO) + 1;
		}
		if (gAutoSend) {
			oldlen += 4; // lstrlen(TEXT("&s=1"));
		}
		
		p = mem_calloc(oldlen * sizeof(TCHAR));
		if (p == NULL) {
			mem_free(&AppDir);
			FreeParms();
			return FALSE;
		}

		if (parms[OP_MAILTO].value == NULL) {
			str_cpy_n_t(p, PARM_MAILTO, oldlen);
			oldlen -= lstrlen(PARM_MAILTO);
			*(p+lstrlen(p)) = TEXT('?');
			oldlen --;
		} else {
			str_cpy_n_t(p, PARM_MAILTO, oldlen);
			oldlen -= lstrlen(PARM_MAILTO);
			str_cpy_n_t(p+lstrlen(p), parms[OP_MAILTO].value, oldlen);
			oldlen -= lstrlen(parms[OP_MAILTO].value);
			mem_free(&parms[OP_MAILTO].value);
		}
		parms[OP_MAILTO].value = p;
		
		p += lstrlen(p);
		for(parmix=OP_MAILTO + 1; parmix < NUMPARMDEF; parmix++) {
			if (parms[parmix].value != NULL) {
				parmlen = lstrlen(parms[parmix].param);
				vallen = lstrlen(parms[parmix].value);
				str_cpy_n_t(p, parms[parmix].param, oldlen);
				*p = TEXT('&');
				p += parmlen;
				*(p - 1) = TEXT('=');
				oldlen -= parmlen;
				str_cpy_n_t(p, parms[parmix].value, oldlen);
				p += vallen;
				oldlen -= vallen;
			}
		}
		if (gAutoSend) {
			str_cpy_n_t(p, TEXT("&s=1"), oldlen);
			p += 4; // lstrlen(TEXT("&s=1"));
			oldlen -= 4; // lstrlen(TEXT("&s=1"));
		}
		*p = TEXT('\0');
		oldlen--;
	}

	if (parms[OP_Y].value) {
		TCHAR fullname[MAX_PATH];
		BOOL Found = TRUE;

		p = parms[OP_Y].value;
		str_cpy_n_t(fullname, AppDir, MAX_PATH);
		Found = MergePath(fullname, p);
		if (!Found) {
			// Error: can't make INI file name
			TCHAR msg[MSG_SIZE];
			wsprintf(msg, STR_ERR_INIFILE, p);
			ErrorMessage(NULL, msg);
			mem_free(&AppDir);
			FreeParms();
			return -1;
		}
		mem_free(&parms[OP_Y].value);
		parms[OP_Y].value = alloc_copy_t(fullname);
		if (parms[OP_Y].value == NULL) {
			mem_free(&AppDir);
			FreeParms();
			return FALSE;
		}
	}

	p = parms[OP_Y].value;
	DefaultDataDir = alloc_copy_t((p != NULL) ? p : AppDir);
	if (DefaultDataDir == NULL) {
		mem_free(&AppDir);
		FreeParms();
		return FALSE;
	}
	if (p == NULL) {
		str_join_t(fname, DefaultDataDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	} else {
		trunc_to_dirname(DefaultDataDir);
		str_cpy_n_t(fname, p, MAX_PATH);
	}

	// check for IniFile specification in ini file
	len = file_get_size(fname);
	if (len > 8) { // no point to read, unless room for "IniFile=" and more
		char *buf, *s, *t;
		// no point to read more than needed
		if (len > 10+MAX_PATH) len = 10+MAX_PATH;
		buf = file_read(fname, len);
		if (buf == NULL) {
			// Error: can't read file
			TCHAR msg[MSG_SIZE];
			wsprintf(msg, STR_ERR_READ_INI, fname);
			ErrorMessage(NULL, msg);
			mem_free(&AppDir);
			mem_free(&DefaultDataDir);
			FreeParms();
			return -1;
		}
		if (str_cmp_n(buf, "IniFile=", strlen("IniFile=")) == 0) {
			s = buf + strlen("IniFile=");
			if (*s == '\"') {
				s++;
			}
			for (t = s; *t != '\"' && *t != '\r' && *t != '\n' && *t != '\0'; t++) {
#ifndef UNICODE
				if (IsDBCSLeadByte((BYTE)*t) == TRUE && *(t + 1) != TEXT('\0')) {
					t++;
				}
#endif
			}
			*t = '\0';
			if (strlen(s) > 0) {
				TCHAR fullname[MAX_PATH];
				BOOL Found = FALSE;
#ifdef UNICODE
				p = alloc_char_to_tchar(s);
#else
				p = alloc_copy(s);
#endif
				mem_free(&buf);
				if (p == NULL) {
					mem_free(&AppDir);
					mem_free(&DefaultDataDir);
					FreeParms();
					return FALSE;
				}
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
				len = lstrlen(TEXT("%STORAGE_CARD%"));
				if (str_cmp_n_t(p, TEXT("%STORAGE_CARD%"), len) == 0) {
					HANDLE hFlashCard;
					WIN32_FIND_DATA lpwfdFlashCard;
					BOOL next = TRUE;

					hFlashCard = FindFirstFlashCard (&lpwfdFlashCard);
					if (hFlashCard != INVALID_HANDLE_VALUE) {
						while (next) {
							TCHAR *fcPath = lpwfdFlashCard.cFileName;
							wsprintf(fullname, TEXT("\\%s%s"), fcPath, (p+len));
							if (file_get_size(fullname) > 0) {
								Found = TRUE;
								break;
							}
							next = FindNextFlashCard(hFlashCard, &lpwfdFlashCard);
						}
						FindClose(hFlashCard);
					}
					if (Found == FALSE) {
						str_cpy_n_t(fullname, p, MAX_PATH);
						Found = TRUE;
					}
				}
#endif
#else
				p = replace_env_var(p);
#endif
				if (Found == FALSE) {
					str_cpy_n_t(fullname, DefaultDataDir, MAX_PATH);
					Found = MergePath(fullname, p);
				}
				if (!Found) {
					// Error: can't make INI file name
					TCHAR msg[MSG_SIZE];
					wsprintf(msg, STR_ERR_INIFILE, p);
					ErrorMessage(NULL, msg);
					mem_free(&p);
					mem_free(&AppDir);
					mem_free(&DefaultDataDir);
					FreeParms();
					return -1;
				}
				mem_free(&p);
				mem_free(&DefaultDataDir);
				mem_free(&parms[OP_Y].value);
				parms[OP_Y].value = alloc_copy_t(fullname);
				trunc_to_dirname(fullname);
				DefaultDataDir = alloc_copy_t(fullname);
				if (parms[OP_Y].value == NULL || DefaultDataDir == NULL) {
					mem_free(&AppDir);
					mem_free(&DefaultDataDir);
					FreeParms();
					return FALSE;
				}
			}
		}
		mem_free(&buf);
	}

	if (parms[OP_Y].value) {
		TCHAR msg[MSG_SIZE];
		IniFile = parms[OP_Y].value;
		parms[OP_Y].value = NULL;
		len = file_get_size(IniFile);
#ifdef _WIN32_WCE
		if (len == -1 && dir_check(DefaultDataDir) == FALSE) {
			wsprintf(msg, STR_ERR_DIR_NOT_READY, IniFile);
			MessageBox(NULL, msg, WINDOW_TITLE, MB_ICONERROR | MB_OK);
			mem_free(&IniFile);
			mem_free(&AppDir);
			mem_free(&DefaultDataDir);
			FreeParms();
			return -1;
		}			
#endif
		if (len == -1) {
			wsprintf(msg, STR_Q_CREATE_INIFILE, IniFile);
			if (MessageBox(NULL, msg, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDYES) {
				HANDLE hFile;
				dir_create(DefaultDataDir);
				hFile = CreateFile(IniFile, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile == NULL || hFile == (HANDLE)-1) {
					DWORD DirInfo;
					DirInfo = GetFileAttributes(DefaultDataDir);
					if (DirInfo & FILE_ATTRIBUTE_READONLY) {
						ErrorMessage(NULL, STR_ERR_READONLY);
					} else {
						wsprintf(msg, STR_ERR_WRITE_INI, IniFile);
						ErrorMessage(NULL, msg);
					}
					mem_free(&IniFile);
					mem_free(&AppDir);
					mem_free(&DefaultDataDir);
					FreeParms();
					return -1;
				}
				CloseHandle(hFile);
			} else {
				mem_free(&IniFile);
				mem_free(&AppDir);
				mem_free(&DefaultDataDir);
				FreeParms();
				return -1;
			}
		}
	}
	if (parms[OP_MAILTO].value) {
#ifdef UNICODE
		CmdLine = parms[OP_MAILTO].value;
		parms[OP_MAILTO].value = NULL;
#else
		CmdLine = alloc_char_to_tchar(parms[OP_MAILTO].value);
		if (CmdLine == NULL) {
			mem_free(&IniFile);
			mem_free(&AppDir);
			mem_free(&DefaultDataDir);
			FreeParms();
			return FALSE;
		}
#endif
	}
	if (parms[OP_MAILBOX].value) {
		InitialAccount = mem_alloc((lstrlen(parms[OP_MAILBOX].value)+1)*sizeof(TCHAR));
		if (InitialAccount == NULL) {
			mem_free(&CmdLine);
			mem_free(&IniFile);
			mem_free(&AppDir);
			mem_free(&DefaultDataDir);
			FreeParms();
			return FALSE;
		}
		URL_decode_t(parms[OP_MAILBOX].value, InitialAccount);
	}
	FreeParms();
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
 * ConfirmPass - �p�X���[�h�̊m�F
 */
#ifndef _WIN32_WCE
BOOL ConfirmPass(HWND hWnd, TCHAR *ps, BOOL Show)
{
	// ShowPass
	gPassSt = 0;
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), NULL, InputPassProc,
		((Show == TRUE) ? (LPARAM)STR_TITLE_SHOWPASSWORD : (LPARAM)STR_TITLE_CONFPASSWORD)) == FALSE) {
		return FALSE;
	}
	confirm_flag = 2;
	return TRUE;
}
#endif

/*
 * TrayMessage - �^�X�N�g���C�̃A�C�R���̐ݒ�
 */
static BOOL TrayMessage(HWND hWnd, DWORD dwMessage, UINT uID, HICON hIcon)
{
	NOTIFYICONDATA tnd;
	ZeroMemory(&tnd, sizeof(NOTIFYICONDATA));

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = hWnd;
	tnd.uID	= uID;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
#if (_WIN32_IE >= 0x0500)
	if (hIcon == TrayIcon_Mail && op.ShowNewMailMessage == 0) {
		tnd.uFlags |= NIF_INFO;
		tnd.uTimeout = 5000;
		lstrcpy(tnd.szInfoTitle, WINDOW_TITLE);
		lstrcpy(tnd.szInfo, STR_MSG_NEWMAIL);
	}
#endif
	tnd.uCallbackMessage = WM_TRAY_NOTIFY;
	tnd.hIcon = hIcon;
	lstrcpy(tnd.szTip, (dwMessage == NIM_DELETE) ? TEXT("") : WINDOW_TITLE);
	return Shell_NotifyIcon(dwMessage, &tnd);
}

/*
 * SetTrayIcon - �^�X�N�g���C�ɃA�C�R����ݒ肷��
 */
static void SetTrayIcon(HWND hWnd, HICON hIcon)
{
	if (op.ShowTrayIcon != 1 || hIcon == NULL) {
		return;
	}
	if (TrayMessage(hWnd, NIM_MODIFY, TRAY_ID, hIcon) == FALSE) {
		//When it cannot modify, it adds the
		TrayMessage(hWnd, NIM_ADD, TRAY_ID, hIcon);
	}
}

/*
 * SwitchCursor - �J�[�\���̏�Ԃ̐؂�ւ�
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
 * _SetForegroundWindow - �E�B���h�E���A�N�e�B�u�ɂ���
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
 * SetStatusTextT - �X�e�[�^�X�̕\�� (TCHAR)
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
 * SetSocStatusTextT - �X�e�[�^�X�̕\�� (TCHAR)
 */
void SetSocStatusTextT(HWND hWnd, TCHAR *buf)
{
	TCHAR *st_buf;

	if (PPCFlag == FALSE && RecvBox >= MAILBOX_USER && (MailBox + RecvBox)->Name != NULL) {
		st_buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen((MailBox + RecvBox)->Name) + lstrlen(buf) + 6));
		if (st_buf == NULL) {
			return;
		}
		str_join_t(st_buf, TEXT("["), (MailBox + RecvBox)->Name, TEXT("] "), buf, (TCHAR *)-1);
#ifdef _WIN32_WCE_PPC
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1 | SBT_NOBORDERS, (LPARAM)st_buf);
#else
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1, (LPARAM)st_buf);
#endif
		if (op.SocLog > 0) {
			str_join_t(st_buf, st_buf, TEXT("\r\n"), (TCHAR *)-1);
			log_save(st_buf);
		}
		mem_free(&st_buf);
	} else {
#ifdef _WIN32_WCE_PPC
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1 | SBT_NOBORDERS, (LPARAM)buf);
#else
		SendDlgItemMessage(hWnd, IDC_STATUS, SB_SETTEXT, (WPARAM)1, (LPARAM)buf);
#endif
		if (op.SocLog > 0) {
			TCHAR logbuf[BUF_SIZE];
			int len = lstrlen(buf) + 3;
			if (len < BUF_SIZE) {
				st_buf = logbuf;
			} else {
				st_buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
			}
			if (st_buf != NULL) {
				str_join_t(st_buf, buf, TEXT("\r\n"), (TCHAR *)-1);
				log_save(st_buf);
				if (st_buf != logbuf) {
					mem_free(&st_buf);
				}
			}
		}
	}
}

/*
 * SetSocStatusText - �X�e�[�^�X�̕\��
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

/*
 * SetItemCntStatusText - Sets View/Server, New/Unread counts in status bar
 */
void SetItemCntStatusText(MAILBOX *tpViewMailBox, BOOL bNotify, BOOL CountUnsent)
{
	MAILBOX *tpMailBox;
	MAILITEM *tpMailItem;
	TCHAR wbuf[BUF_SIZE], dtmp[20];
	long dsize;
	int ItemCnt;
	int NewCnt = 0, UnreadCnt = 0, UnsentCnt = 0, FlagCnt = 0;
	int i;
	BOOL changed = FALSE;

	tpMailBox = (MailBox + SelBox);
	if (tpMailBox == NULL || (tpViewMailBox != NULL && tpViewMailBox != tpMailBox)) {
		return;
	}

	ItemCnt = ListView_GetItemCount(mListView);
	dsize = tpMailBox->DiskSize;
	if (dsize < 0) {
		wsprintf(dtmp, STR_STATUS_MAILSIZE_KB, TEXT("?"));
	} else if (dsize < 1000) {
		wsprintf(dtmp, STR_STATUS_MAILSIZE_B, dsize);
	} else {
		if (dsize < 102400) {
			FormatNumberString(dsize, STR_STATUS_MAILSIZE_KB, op.DecPt, dtmp);
		} else {
			dsize /= 1024;
			if (dsize < 102400) {
				FormatNumberString(dsize, STR_STATUS_MAILSIZE_MB, op.DecPt, dtmp);
			} else {
				dsize /= 1024;
				FormatNumberString(dsize, STR_STATUS_MAILSIZE_GB, op.DecPt, dtmp);
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
				FormatNumberString(ssize, STR_STATUS_MAILSIZE_KB, op.DecPt, stmp);
			} else {
				ssize /= 1024;
				if (ssize < 102400) {
					FormatNumberString(ssize, STR_STATUS_MAILSIZE_MB, op.DecPt, stmp);
				} else {
					ssize /= 1024;
					FormatNumberString(ssize, STR_STATUS_MAILSIZE_GB, op.DecPt, stmp);
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
	if (CountUnsent || SelBox == MAILBOX_SEND) {
		int FlagOut = 0;
		BOOL err = FALSE;
		for (i = 0; i < tpMailBox->MailItemCnt; i++) {
			tpMailItem = *(tpMailBox->tpMailItem + i);
			if (tpMailItem == NULL) {
				continue;
			}
			if (tpMailItem->Mark == ICON_SEND || tpMailItem->Mark == ICON_ERROR) {
				UnsentCnt++;
			}
			if (tpMailItem->Mark == ICON_FLAG) {
				FlagOut++;
			}
			if (tpMailItem->Mark == ICON_ERROR) {
				err = TRUE;
			}
		}
		tpMailBox->NewMail = UnsentCnt;
		tpMailBox->FlagCount = FlagOut;
		if (err) {
			FlagOut = ICON_ERROR;
		} else if (UnsentCnt > 0) {
			FlagOut = TRUE;
		} else if (FlagOut > 0) {
			FlagOut = ICON_FLAG;
		} else {
			FlagOut = FALSE;
		}
		if (FlagOut != FALSE) {
			SetStarMBMenu(FlagOut);
		} else if (GetStarMBMenu()) {
			SetStarMBMenu(FALSE);
		}
	} else {
		UnsentCnt = tpMailBox->NewMail;
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
		if (tpMailItem->Mark == ICON_FLAG) {
			FlagCnt++;
		}
	}
	if (UnsentCnt > 0) {
		wsprintf(wbuf, STR_STATUS_MAILINFO_U, NewCnt, UnreadCnt, UnsentCnt);
	} else {
		wsprintf(wbuf, STR_STATUS_MAILINFO, NewCnt, UnreadCnt);
	}
	SetStatusTextT(MainWnd, wbuf, 1);

	if ((tpMailBox->NewMail > 0 && NewCnt == 0) ||
		(NewCnt == 0 && tpMailBox->FlagCount > 0 && FlagCnt == 0)) {
		changed = TRUE;
	}
	tpMailBox->NewMail = NewCnt;
	tpMailBox->UnreadCnt = UnreadCnt;
	tpMailBox->FlagCount = FlagCnt;
	if (changed) {
		SetMailboxMark(SelBox, STATUS_DONE, FALSE);
		SetUnreadCntTitle(FALSE);
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
 * SetStatusRecvLen - ����M�o�C�g���̕\��
 */
void SetStatusRecvLen(HWND hWnd, int len, int size, TCHAR *msg)
{
	TCHAR wbuf[BUF_SIZE];

	wsprintf(wbuf, STR_STATUS_SOCKINFO, len, msg);
#ifdef WSAASYNC
	// Progress bar - GJC
	// [....................]
	if (len > size) len = size;
	if (size > 0) {
		int i;
		for (i=0; i < (20 * len) / size; i++) {
			wbuf[i+1] = TEXT('|');
		}
	}
#endif
	SetStatusTextT(hWnd, wbuf, 1);
}

/*
 * ErrorMessage - �G���[���b�Z�[�W
 */
void ErrorMessage(HWND hWnd, TCHAR *buf)
{
	SwitchCursor(TRUE);
	ShowError = TRUE;
	MessageBox(hWnd, buf, STR_TITLE_ERROR, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
	ShowError = FALSE;
}

/*
 * SocketErrorMessage - �\�P�b�g�̃G���[���b�Z�[�W
 */
void SocketErrorMessage(HWND hWnd, TCHAR *buf, int BoxIndex)
{
	MAILBOX *tpMailBox;
	TCHAR *Title;
	TCHAR *p;

	tpMailBox = MailBox + BoxIndex;
	if (BoxIndex == RecvBox) {
		SetMailboxMark(RecvBox, STATUS_ERROR, FALSE);
	}
	RecvBox = -1;
	SetMailMenu(hWnd);
	SwitchCursor(TRUE);

	if (hWnd != NULL) {
		//In status bar information of error indicatory
		SetStatusTextT(hWnd, buf, 1);
	}
	if (op.SocLog > 0) {
		TCHAR logbuf[BUF_SIZE];
		str_join_t(logbuf, buf, TEXT("\r\n"), (TCHAR *)-1);
		log_save(logbuf);
		log_flush();
	}
	if (op.SocIgnoreError == 1 && BoxIndex >= MAILBOX_USER) {
		// ��M�G���[�𖳎�����ݒ�̏ꍇ
		return;
	}
	if (op.SendIgnoreError == 1 && BoxIndex == MAILBOX_SEND) {
		// ���M�G���[�𖳎�����ݒ�̏ꍇ
		return;
	}
	if ((BoxIndex == MAILBOX_SEND || BoxIndex >= MAILBOX_USER) &&
		tpMailBox->Name != NULL && *tpMailBox->Name != TEXT('\0')) {
		// �A�J�E���g���t���̃^�C�g���̍쐬
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
 * ErrorSocketEnd - �ʐM�ُ̈�I�����̏���
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
#ifdef ENABLE_RAS
	KillTimer(hWnd, ID_RASWAIT_TIMER);
#endif
#ifdef ENABLE_WIFI
	KillTimer(hWnd, ID_WIFIWAIT_TIMER);
#endif
	gSockFlag = FALSE;
	AllCheck = FALSE;
	ExecFlag = FALSE;
	KeyShowHeader = FALSE;
	EndSocketFunc(hWnd, FALSE);
}

/*
 * ShowMenu - post pop-up menu
 */
void ShowMenu(HWND hWnd, HMENU hMenu, int mpos, int PosFlag, BOOL timer)
{
	HWND hListView;
	RECT WndRect;
	RECT ItemRect;
#ifdef _WIN32_WCE
	DWORD ret;
#else
	POINT apos;
#endif
	int i;
	int x = 0, y = 0;

#ifndef _WIN32_WCE_PPC
	_SetForegroundWindow(hWnd);
#endif
	switch (PosFlag) {
		//of round Acquisition
	case 0: // pop-up at mouse position
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

	case 1: // pop-up at position of selection
	case 4:	// VK_APPS (menu key) to post hMBPOPUP
		if (PosFlag == 1) {
			hListView = mListView;
			i = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
		} else {
			hListView = GetDlgItem(hWnd, IDC_MBMENU);
			i = GetSelectedMBMenu();
		}
		GetWindowRect(hListView, &WndRect);
		if (i == -1) {
			x = WndRect.left;
			y = WndRect.top;
		} else {
			if (PosFlag == 1) {
				ListView_EnsureVisible(hListView, i, TRUE);
				ListView_GetItemRect(hListView, i, &ItemRect, LVIR_ICON);
			} else {
				SendMessage(hListView, LB_GETITEMRECT, i, (LPARAM)&ItemRect);
			}
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
	case 2:
		GetWindowRect(hWnd, &WndRect);
#ifdef _WIN32_WCE_LAGENDA
		x = 0;
		y = wnd_size.bottom; // ??
#else
		x = WndRect.right;
		y = WndRect.bottom;
#endif
		break;
#endif
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	_SetForegroundWindow(hWnd);
#endif
	TrackPopupMenu(GetSubMenu(hMenu, mpos), 
		(PosFlag == 2) ? (TPM_BOTTOMALIGN | TPM_RIGHTALIGN) : (TPM_TOPALIGN | TPM_LEFTALIGN),
		x, y, 0, hWnd, NULL);
#else
	TrackPopupMenu(GetSubMenu(hMenu, mpos), TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, 
		x, y, 0, hWnd, NULL);
#endif
	if (timer) {
		SetTimer(hWnd, ID_RESTORESEL_TIMER, 10, NULL);
	}
#ifndef _WIN32_WCE_PPC
	PostMessage(hWnd, WM_NULL, 0, 0);
#endif
}

/*
 * SetMailMenu - ���j���[�̊����^�񊈐��̐؂�ւ�
 */
void SetMailMenu(HWND hWnd)
{
	HMENU hMenu;
	HWND hToolBar;
	int SelFlag, SocFlag, Markable;
	int RecvBoxFlag, SaveTypeFlag, SendBoxFlag;
	int MoveBoxFlag;
	int i;
	LPARAM mklng;

#ifdef _WIN32_WCE_PPC
	LPARAM lpcon;
	int xflag, xsize;
#endif
#ifdef ENABLE_WIFI
	int WiFiFlag;
#endif
	
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	xsize = GetSystemMetrics(SM_CXSCREEN);
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE);
	hToolBar = hMainToolBar;
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hMainMenu, 0);
	hToolBar = NULL;
#else
	hToolBar = GetDlgItem(hWnd, IDC_CB);
	hMenu = CommandBar_GetMenu(hToolBar, 0);
#endif
#else
	hMenu = GetMenu(hWnd);
	hToolBar = GetDlgItem(hWnd, IDC_TB);
#endif
	if (hMenu == NULL || mListView == NULL) {
		return;
	}

	i = ListView_GetSelectedCount(mListView);
	if (i != 1 && op.PreviewPaneHeight > 0) {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, 
			(LPARAM) ((op.ExpertMode) ? TEXT("") :
				((i>1) ? STR_MSG_SINGLE_PREVIEW : STR_MSG_SELECT_PREVIEW)));
	}
	SelFlag = (i <= 0) ? 0 : 1;
	SocFlag = (g_soc != -1 || gSockFlag == TRUE) ? 0 : 1;
	RecvBoxFlag = (SelBox == RecvBox) ? 0 : 1;
	SaveTypeFlag = ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) ? 0 : 1;
	SendBoxFlag = (SelBox == MAILBOX_SEND) ? 0 : 1;
	MoveBoxFlag = (MailBoxCnt < 3) ? 0 : 1;

	Markable = SendBoxFlag;
	i = -1;
	if (SelBox == MAILBOX_SEND) {
		while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
			MAILITEM *tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
			if (tpMailItem != NULL && tpMailItem->MailStatus != ICON_SENTMAIL) {
				Markable = 1;
				break;
			}
		}
	}

	// EnableMenuItem enables an item if the last argument is 0

	EnableMenuItem(hMenu, ID_MENUITEM_BACKUP, !SocFlag);

	EnableMenuItem(hMenu, ID_MENUITEM_SETMAILBOX, !(RecvBoxFlag & SendBoxFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_DELETEMAILBOX, !(RecvBoxFlag & SendBoxFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_LISTINIT, !(SaveTypeFlag & SendBoxFlag));

	EnableMenuItem(hMenu, ID_MENUITEM_MOVEUPMAILBOX, !(SocFlag & SendBoxFlag & MoveBoxFlag && (SelBox > MAILBOX_USER)));
	EnableMenuItem(hMenu, ID_MENUITEM_MOVEDOWNMAILBOX, !(SocFlag & SendBoxFlag & MoveBoxFlag && (SelBox < MailBoxCnt-1)));

#ifdef ENABLE_RAS
	EnableMenuItem(hMenu, ID_MENUITEM_RAS_CONNECT,
		!(SocFlag & ((MailBox + SelBox)->RasMode | !SendBoxFlag) & !op.EnableLAN));
	EnableMenuItem(hMenu, ID_MENUITEM_RAS_DISCONNECT, op.EnableLAN);
#endif
#ifdef ENABLE_WIFI
	WiFiFlag = op.EnableLAN || op.WiFiDeviceName == NULL || *op.WiFiDeviceName == TEXT('\0');
	EnableMenuItem(hMenu, ID_MENUITEM_WIFI_CONNECT, !(SocFlag && !WiFiFlag));
	EnableMenuItem(hMenu, ID_MENUITEM_WIFI_DISCONNECT, WiFiFlag);
#endif

#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hMainMenu, 1);
#endif
	EnableMenuItem(hMenu, ID_MENUITEM_OPEN, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_REMESSEGE, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_ALLREMESSEGE, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_FORWARD, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_REDIRECT, !SelFlag);

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
#else	//_WIN32_WCE_LAGENDA
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_RECV,
		(LPARAM)MAKELONG(SocFlag & SaveTypeFlag & SendBoxFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_ALLCHECK, (LPARAM)MAKELONG(SocFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_EXEC,
		(LPARAM)MAKELONG(SocFlag & SaveTypeFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_ALLEXEC, (LPARAM)MAKELONG(SocFlag, 0));
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_STOP, (LPARAM)MAKELONG(!SocFlag, 0));

#ifndef _WIN32_WCE
	mklng = (LPARAM)MAKELONG(SelFlag, 0);
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_REMESSEGE, mklng);
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_ALLREMESSEGE, mklng);
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_FORWARD, mklng);
#endif

	if (SelBox == MAILBOX_SEND) {
		SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_SENDMARK,
			(LPARAM)MAKELONG((SelFlag & Markable & !(!RecvBoxFlag && ExecFlag == TRUE)), 0));
	} else {
		SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_DOWNMARK,
			(LPARAM)MAKELONG((SelFlag & SaveTypeFlag & !(!RecvBoxFlag && ExecFlag == TRUE)), 0));
	}

	mklng = (LPARAM)MAKELONG((SelFlag & !(!RecvBoxFlag && ExecFlag == TRUE)), 0);
	if (SendBoxFlag & SaveTypeFlag) {
		SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_DELMARK, mklng);
	} else {
		SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_DELETE, mklng);
	}
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_FLAGMARK, mklng);

#ifdef ENABLE_RAS
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_RAS_CONNECT,
		(LPARAM)MAKELONG((SocFlag & ((MailBox + SelBox)->RasMode | !SendBoxFlag) & !op.EnableLAN), 0));
#endif
#ifdef ENABLE_WIFI
	SendMessage(hToolBar, TB_ENABLEBUTTON, ID_MENUITEM_WIFI_CONNECT,
		(LPARAM)MAKELONG((SocFlag & !WiFiFlag), 0));
#endif

#ifdef _WIN32_WCE_PPC
	xflag = (xsize < 300) ? 1 : 0;
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_DOWNMARK,
		(LPARAM)MAKELONG(!SendBoxFlag || xflag, 0));
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_SENDMARK,
		(LPARAM)MAKELONG(SendBoxFlag || xflag, 0));
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_DELMARK,
		(LPARAM)MAKELONG(!(SendBoxFlag && SaveTypeFlag) || xflag, 0));
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_DELETE,
		(LPARAM)MAKELONG((SendBoxFlag && SaveTypeFlag) || xflag, 0));
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_FLAGMARK, xflag);
#ifdef ENABLE_RAS
	if (xsize < 350 || op.WiFiDeviceName == NULL || *op.WiFiDeviceName == TEXT('\0')) {
		lpcon = (LPARAM)MAKELONG(1,0);
	} else {
		lpcon = (LPARAM)MAKELONG(op.EnableLAN,0);
	}
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_RAS_CONNECT,    lpcon);
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_RAS_DISCONNECT, lpcon);
#endif
#ifdef ENABLE_WIFI
	if (xsize < 350 || op.WiFiDeviceName == NULL || *op.WiFiDeviceName == TEXT('\0')) {
		lpcon = (LPARAM)MAKELONG(1,0);
	} else {
		lpcon = (LPARAM)MAKELONG(WiFiFlag,0);
	}
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_WIFI_CONNECT,    lpcon);
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_WIFI_DISCONNECT, lpcon);
#endif
#else	// _WIN32_WCE_PPC
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_DOWNMARK, (LPARAM)MAKELONG(!SendBoxFlag, 0));
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_SENDMARK, (LPARAM)MAKELONG(SendBoxFlag, 0));
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_DELMARK, (LPARAM)MAKELONG(!(SendBoxFlag && SaveTypeFlag), 0));
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_DELETE, (LPARAM)MAKELONG(SendBoxFlag && SaveTypeFlag, 0));
#endif	// _WIN32_WCE_PPC

#if !defined(_WIN32_WCE)
#ifdef ENABLE_RAS
	mklng = (LPARAM)MAKELONG(op.EnableLAN, 0);
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_RAS_CONNECT,    mklng);
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_RAS_DISCONNECT, mklng);
#endif
#ifdef ENABLE_WIFI
	mklng = (LPARAM)MAKELONG(WiFiFlag, 0);
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_WIFI_CONNECT,    mklng);
	SendMessage(hToolBar, TB_HIDEBUTTON, ID_MENUITEM_WIFI_DISCONNECT, mklng);
#endif
#endif

#endif	//_WIN32_WCE_LAGENDA

	EnableMenuItem(hMenu, ID_MENUITEM_FLAGMARK, !(SelFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	EnableMenuItem(hMenu, ID_MENUITEM_DOWNMARK, !(SelFlag & SaveTypeFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	EnableMenuItem(hMenu, ID_MENUITEM_DELMARK, !(SelFlag & SaveTypeFlag & SendBoxFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	EnableMenuItem(hMenu, ID_MENUITEM_UNMARK, !(SelFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
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
	EnableMenuItem(hMenu, ID_MENUITEM_COPYSBOX, !SelFlag);
	EnableMenuItem(hMenu, ID_MENUITEM_MOVESBOX, !SelFlag);
	
	// pop-up context menu
	if (SendBoxFlag) { // actually means we're not in the sendbox
		hMenu = hMainPop; // GetSubMenu(hMainPop, 0);
		EnableMenuItem(hMenu, ID_MENUITEM_OPEN, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_REMESSEGE, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_ALLREMESSEGE, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_FORWARD, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_DOWNMARK, !(SelFlag & SaveTypeFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
		EnableMenuItem(hMenu, ID_MENUITEM_DELMARK, !(SelFlag & SaveTypeFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
		EnableMenuItem(hMenu, ID_MENUITEM_READMAIL, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_UNREADMAIL, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_COPYSBOX, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_MOVESBOX, !SelFlag);
#ifdef _WIN32_WCE_PPC
		EnableMenuItem(hMenu, ID_MENUITEM_DELATTACH, !SelFlag);
#endif
	} else {
		hMenu = GetSubMenu(hMainPop, 1);
		EnableMenuItem(hMenu, ID_MENUITEM_SAVECOPY, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_PROP, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_FORWARD, !SelFlag);
		EnableMenuItem(hMenu, ID_MENUITEM_SENDMARK, !(SelFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	}
	EnableMenuItem(hMenu, ID_MENUITEM_FLAGMARK, !(SelFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	EnableMenuItem(hMenu, ID_MENUITEM_UNMARK, !(SelFlag & !(!RecvBoxFlag && ExecFlag == TRUE)));
	EnableMenuItem(hMenu, ID_MENUITEM_DELETE, !SelFlag);

}

/*
 * FreeAllMailBox - �E�B���h�E�̏I������
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
 * CloseViewWindow - ���[���\���E�B���h�E�ƃ��[���ҏW�E�B���h�E�����
 */
static BOOL CloseEditViewWindows(int Flag)
{
#ifndef _WIN32_WCE
	HWND fWnd;
#endif
	BOOL retval = TRUE;

	// Close the View window
	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_ENDCLOSE, 0, 0);
	}

#ifdef _WIN32_WCE
	// Close the Edit or Sent Mail window (there's only one)
	if (hEditWnd != NULL) {
		if (Flag == 0) {
			retval = EndEditWindow(hEditWnd, FALSE);
		} else {
			SendMessage(hEditWnd, WM_ENDCLOSE, Flag, 0);
		}
	}
#else
	// Close all Edit or Sent Mail windows
	while ((fWnd = FindWindow(EDIT_WND_CLASS, NULL)) != NULL) {
		SendMessage(fWnd, WM_ENDCLOSE, Flag, 0); // Flag==1 for all calls on this platform
	}
#endif
	return retval;
}

/*
 * SubClassListViewProc - �T�u�N���X�������E�B���h�E�v���V�[�W��
 *	IME����������Ԃł�SpaceKey��L���ɂ��邽��
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
		if (SelMode == TRUE) {
			SHRGINFO rg;
			rg.cbSize = sizeof(SHRGINFO);
			rg.hwndClient = hWnd;
			rg.ptDown.x = LOWORD(lParam);
			rg.ptDown.y = HIWORD(lParam);
			rg.dwFlags = SHRG_RETURNCMD;
			if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
				SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENU, 0);
				return 0;
			}
		}
		//Item under mouse acquisition
		apos.x = LOWORD(lParam);
		apos.y = HIWORD(lParam);

		lvht.pt = apos;
		lvht.flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
		lvht.iItem = 0;
		i = ListView_HitTest(hWnd, &lvht);

		if (SelMode == TRUE) {
			if (GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_SHIFT) >= 0) {
				//Selective mode
				if (i != -1) {
					ListView_SetItemState(hWnd, i,
						LVIS_FOCUSED | (ListView_GetItemState(hWnd, i, LVIS_SELECTED) ^ LVIS_SELECTED),
						LVIS_FOCUSED | LVIS_SELECTED);
				}
				return 0;
			}
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
 * SetListViewSubClass - �E�B���h�E�̃T�u�N���X��
 */
static void SetListViewSubClass(HWND hWnd)
{
	ListViewWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassListViewProc);
}

/*
 * DelListViewSubClass - �E�B���h�E�N���X��W���̂��̂ɖ߂�
 */
static void DelListViewSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)ListViewWindowProcedure);
	ListViewWindowProcedure = NULL;
}

/*
 * ListViewSortCheck - set checkmark on main window menu item
 */
#ifndef _WIN32_WCE
void ListViewSortMenuCheck(int sort_flag)
{
	HMENU hMenu = GetMenu(MainWnd);
	CheckMenuItem(hMenu, ID_MENUITEM_FILESORT, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_ICONSORT, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_SUBJSORT, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_FROMSORT, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_DATESORT, MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_SIZESORT, MF_UNCHECKED);

	if (op.LvThreadView == 1) {
		CheckMenuItem(hMenu, ID_MENUITEM_THREADVIEW, MF_CHECKED);
	} else {
		int i = ABS(sort_flag) - 1;
		CheckMenuItem(hMenu, ID_MENUITEM_THREADVIEW, MF_UNCHECKED);
		switch(i) {
			case SORT_ICON:
				CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_ICONSORT, MF_CHECKED);
				break;
			case SORT_SUBJ:
				CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_SUBJSORT, MF_CHECKED);
				break;
			case SORT_FROM:
				CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_FROMSORT, MF_CHECKED);
				break;
			case SORT_DATE:
				CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_DATESORT, MF_CHECKED);
				break;
			case SORT_SIZE:
				CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_SIZESORT, MF_CHECKED);
				break;
			default:
				CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_FILESORT, MF_CHECKED);
				break;
		}
	}
}
#endif

/*
 * ListViewHeaderNotifyProc - ���X�g�r���[�w�b�_���b�Z�[�W
 */
static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam)
{
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;
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
		// �\�[�g�̐ݒ�
		col = phd->iItem;
		if ((col == 0 || col == 1) && str_cmp_ni_t(op.LvColumnOrder, TEXT("FSDZ"), 4) == 0) {
			col = (col == 0) ? 1 : 0;
		}
		LvSortFlag = (ABS(LvSortFlag) == (col + 1)) ? (LvSortFlag * -1) : (col + 1);
		//of sort Sort
		SwitchCursor(FALSE);
		ListView_SortItems(mListView, CompareFunc, LvSortFlag);
		SwitchCursor(TRUE);

		ListView_EnsureVisible(mListView,
			ListView_GetNextItem(mListView, -1, LVNI_FOCUSED), TRUE);

		if (op.LvAutoSort == 2 || (MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
			op.LvSortItem = LvSortFlag;
		}
#ifndef _WIN32_WCE
		ListViewSortMenuCheck(LvSortFlag);
#endif
		break;
	}
	return FALSE;
}

/*
 * TbNotifyProc - �c�[���o�[�̒ʒm���b�Z�[�W (Win32)
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
 * NotifyProc - �R���g���[���̒ʒm���b�Z�[�W
 */
static LRESULT NotifyProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;

	if (CForm->hwndFrom == mListView) {
#ifdef _WIN32_WCE_PPC
		if (CForm->code == GN_CONTEXTMENU) {
			//Pop rise menu indicatory
			SendMessage(hWnd, WM_COMMAND, ID_MENU, 0);
			return TRUE;
		}
#endif
		return ListView_NotifyProc(hWnd, lParam);
	} else if (CForm->hwndFrom == GetWindow(mListView, GW_CHILD)) {
		return ListViewHeaderNotifyProc(hWnd, lParam);
	}
#ifndef _WIN32_WCE
	if (CForm->code == TTN_NEEDTEXT) {
		return TbNotifyProc(hWnd, lParam);
	}
	if (CForm->code == EN_LINK) {
		ENLINK *openLink = (ENLINK *) lParam;
		if (openLink->msg == WM_LBUTTONUP) {
			OpenURL(hWnd, &openLink->chrg);
			return TRUE;
		}
	}
#endif
	return FALSE;
}

/*
 * MBPaneProc - mailbox pane message handler
 */
static LRESULT CALLBACK MBPaneProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int tmpselbox = -1;
	static BOOL pending = FALSE;
	DWORD sel;

	switch (msg) {
#ifdef OVERRIDE_WM_CHAR
		case WM_CHAR:
			{
				TCHAR p = (TCHAR)wParam, q = 0;
				if (p >= TEXT('a') && p <= TEXT('z')) {
					q = p + TEXT('A') - TEXT('a');
				} else if (p >= TEXT('A') && p <= TEXT('Z')) {
					q = p - TEXT('A') + TEXT('a');
				}
				if (q != 0) {
					TCHAR n[3], o[4];
					int i;
					str_cpy_n_t(n, STR_MAILBOX_NONAME, 2);
					str_cpy_n_t(o, STR_SENDBOX_NAME, 3);
					for (i = SelBox + 1; i < MailBoxCnt; i++) {
						if (((MailBox + i)->Name == NULL && (q == n[0] || p == n[0]))
							|| ((MailBox + i)->Name != NULL && (q == *(MailBox + i)->Name || p == *(MailBox + i)->Name))) {
							break;
						}
					}
					if (i >= MailBoxCnt) {
						if (SelBox != MAILBOX_SEND && (q == o[1] || p == o[1])) {
							i = MAILBOX_SEND;
						} else {
							for (i = MAILBOX_USER; i < SelBox; i++) {
								if (((MailBox + i)->Name == NULL && (q == n[0] || p == n[0]))
									|| ((MailBox + i)->Name != NULL && (q == *(MailBox + i)->Name || p == *(MailBox + i)->Name))) {
									break;
								}
							}
							if (i == SelBox) {
								i = MailBoxCnt;
							}
						}
					}
					if (i < MailBoxCnt) {
						mailbox_select(MainWnd, i);
					}
				}
				return 0;
			}
#endif

#ifndef _WIN32_WCE
		case WM_EXITSIZEMOVE:
#else
		case WM_SIZE: 
#endif
			{
				RECT paneRect;
				int height, width, dw;

				GetWindowRect(hWnd, &paneRect);
				width = paneRect.right - paneRect.left;
				if (width < op.MBMenuMinWidth) {
					width = op.MBMenuMinWidth;
				}
				dw = width - op.MBMenuWidth; // > 0 if it was made larger
				if (dw != 0) {
					op.MBMenuWidth = width;
					if (paneRect.left != 0) {
						MoveWindow(hWnd, 0, op.ToolBarHeight, width, op.MBMenuHeight, TRUE);
					}

#ifdef _WIN32_WCE_PPC
					width = GetSystemMetrics(SM_CXSCREEN) - op.MBMenuWidth;
#else
					GetWindowRect(mListView, &paneRect);
					width = paneRect.right - paneRect.left - dw;
#endif
					height = op.MBMenuHeight;
					if (op.PreviewPaneHeight > 0) {
						height -= op.PreviewPaneHeight;
						op.PreviewPaneWidth = width;
						MoveWindow(GetDlgItem(MainWnd, IDC_EDIT_BODY), op.MBMenuWidth, 
							op.ToolBarHeight+height, width, op.PreviewPaneHeight, TRUE);
					}
					MoveWindow(mListView, op.MBMenuWidth, op.ToolBarHeight, width, height, TRUE);
					MoveWindow(GetDlgItem(MainWnd, IDC_TB), 0, 0, op.MBMenuWidth+width,
						op.ToolBarHeight, TRUE);
				}
				return 0;
			}
			break;

#ifndef _WIN32_WCE
		case WM_GETMINMAXINFO:
			{
				LPMINMAXINFO minmax = (LPMINMAXINFO)lParam;
				minmax->ptMinTrackSize.y = op.MBMenuHeight - 5;
				minmax->ptMaxTrackSize.y = op.MBMenuHeight;
			}
			break;
#endif

#ifdef _WIN32_WCE_PPC
		case WM_LBUTTONDOWN:
			{
				SHRGINFO rg;

				rg.cbSize = sizeof(SHRGINFO);
				rg.hwndClient = hWnd;
				rg.ptDown.x = LOWORD(lParam);
				rg.ptDown.y = HIWORD(lParam);
				rg.dwFlags = SHRG_RETURNCMD;

				if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
					sel = SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
					if (HIWORD(sel) == 0) {
						tmpselbox = SelBox;
						SendMessage(hWnd, LB_SETCURSEL, (WPARAM)sel, 0);
						SelBox = sel;
					}
					SendMessage(hWnd, WM_COMMAND, ID_MENU, 0);
					return 0;
				}
			}
			break;

#elif defined (_WIN32_WCE)
		case WM_LBUTTONUP:
			if (GetKeyState(VK_MENU) < 0) {
				sel = SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
				if (HIWORD(sel) == 0) {
					tmpselbox = SelBox;
					SendMessage(hWnd, LB_SETCURSEL, (WPARAM)sel, 0);
					SelBox = sel;
				}
				SendMessage(hWnd, WM_COMMAND, ID_MENU, 0);
			}
			break;
#else
		case WM_RBUTTONDOWN:
			sel = SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
			if (HIWORD(sel) == 0) {
				tmpselbox = SelBox;
				SendMessage(hWnd, LB_SETCURSEL, (WPARAM)sel, 0);
				SelBox = sel;
			}
			return 0;

		case WM_RBUTTONUP:
			SendMessage(hWnd, WM_COMMAND, ID_MENU, 0);
			return 0;

		case WM_LBUTTONDBLCLK:
			sel = SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
			if (HIWORD(sel) == 0) {
				tmpselbox = SelBox;
				SendMessage(hWnd, LB_SETCURSEL, (WPARAM)sel, 0);
				SelBox = sel;
				SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_RECV, 0);
			}
			return 0;
#endif
		case WM_TIMER:
			if (wParam == ID_RESTORESEL_TIMER && pending == FALSE) {
				KillTimer(hWnd, wParam);
				if (tmpselbox >= 0) {
					SelBox = tmpselbox;
					SendMessage(hWnd, LB_SETCURSEL, (WPARAM)SelBox, 0);
					tmpselbox = -1;
				}
				return 0;
			}
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == ID_MENU) {
				BOOL SocFlag, RecvBoxFlag, SaveTypeFlag, SendBoxFlag, MoveBoxFlag;
				SocFlag = (g_soc != -1 || gSockFlag == TRUE) ? 0 : 1;
				RecvBoxFlag = (SelBox == RecvBox) ? 0 : 1;
				SaveTypeFlag = ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) ? 0 : 1;
				SendBoxFlag = (SelBox == MAILBOX_SEND) ? 0 : 1;
				MoveBoxFlag = (MailBoxCnt <= 3) ? 0 : 1;

				EnableMenuItem(hMBPOPUP, ID_MENUITEM_SETMAILBOX, !(RecvBoxFlag & SendBoxFlag));
				EnableMenuItem(hMBPOPUP, ID_MENUITEM_DELETEMAILBOX, !(RecvBoxFlag & SendBoxFlag));
				EnableMenuItem(hMBPOPUP, ID_MENUITEM_RECV, !(SaveTypeFlag & RecvBoxFlag & SendBoxFlag));
				EnableMenuItem(hMBPOPUP, ID_MENUITEM_EXEC, !(SaveTypeFlag & RecvBoxFlag & SendBoxFlag));
				EnableMenuItem(hMBPOPUP, ID_MENUITEM_LISTINIT, !(SaveTypeFlag & SendBoxFlag));

				EnableMenuItem(hMBPOPUP, ID_MENUITEM_MOVEUPMAILBOX, !(SocFlag & SendBoxFlag & MoveBoxFlag && (SelBox > MAILBOX_USER)));
				EnableMenuItem(hMBPOPUP, ID_MENUITEM_MOVEDOWNMAILBOX, !(SocFlag & SendBoxFlag & MoveBoxFlag && (SelBox < MailBoxCnt-1)));

				SendMessage(hWnd, WM_NULL, 0, 0);
				ShowMenu(hWnd, hMBPOPUP, 0, 0, TRUE);
#ifdef _WIN32_WCE
			} else if (LOWORD(wParam) == ID_MENUITEM_MBP_SETSIZE) {
				if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MBP_SIZE), hWnd, MBWidthProc, 0)) {
					SetWindowSize(GetParent(hWnd), 0, 0);
				}
#endif
			} else if (LOWORD(wParam) == ID_MENUITEM_ADDMAILBOX) {
				int i, below;
				if (MailBoxCnt >= MAX_MAILBOX_CNT) {
					ErrorMessage(hWnd, STR_ERR_TOOMANYMAILBOXES);
					return 0;
				}
#ifndef _WIN32_WCE
				if (op.ConfigPass == 1 && op.Password != NULL && *op.Password != TEXT('\0') &&
					ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
					return 0;
				}
#endif
				pending = TRUE;
				below = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
				if (tmpselbox == -1) {
					tmpselbox = SelBox;
				}
				SelBox = mailbox_create(hWnd, 1, below + 1, TRUE, -1);
				i = SetMailBoxType(hWnd, 0);
#ifndef _WIN32_WCE
				if (i == 0 && op.ConfigPass == 2 && op.Password != NULL && *op.Password != TEXT('\0') &&
					ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
					i = -1;
				}
#endif
				if (i == -1 || (i == 0 && SetMailBoxOption(hWnd, FALSE) == FALSE)) {
					mailbox_delete(hWnd, SelBox, FALSE, FALSE);
					pending = FALSE;
					return 0;
				} else if (i == MAILBOX_IMPORT_SAVE) {
					if (ImportSavebox(hWnd) == FALSE) {
						mailbox_delete(hWnd, SelBox, FALSE, FALSE);
						pending = FALSE;
						return 0;
					}
					(MailBox+SelBox)->Type = MAILBOX_TYPE_SAVE;
					(MailBox+SelBox)->NeedsSave = MAILITEMS_CHANGED;
					DeleteMBMenu(SelBox);
					InsertMBMenu(SelBox,
						(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
						? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
				}
				if (op.AutoSave != 0) {
					SwitchCursor(FALSE);
					ini_save_setting(hWnd, FALSE, FALSE, NULL);
					SwitchCursor(TRUE);
				}
				pending = FALSE;
			} else if (LOWORD(wParam) == ID_MENUITEM_DELETEMAILBOX) {
				TCHAR msg[MSG_SIZE];
				int DelBox = SendMessage(hWnd, LB_GETCURSEL, 0, 0);
				if (DelBox == MAILBOX_SEND || DelBox == RecvBox) {
					return 0;
				}
#ifndef _WIN32_WCE
				if ((op.ConfigPass == 1 ||
					 (op.ConfigPass == 2 && (MailBox+SelBox)->Type != MAILBOX_TYPE_SAVE))
					&& op.Password != NULL && *op.Password != TEXT('\0') &&
					ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
					break;
				}
#endif
				wsprintf(msg, STR_Q_DELMAILBOX, ((MailBox+DelBox)->Name) ? (MailBox+DelBox)->Name : STR_MAILBOX_NONAME);
				if (MessageBox(hWnd, msg, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
					return 0;
				}
				if (DelBox == SelBox) {
					if (op.LazyLoadMailboxes > 0) {
						// make sure SelBox-1 is loaded before deleting
						mailbox_select(hWnd, SelBox-1);
						mailbox_delete(hWnd, DelBox, TRUE, FALSE);
					} else {
						mailbox_select(hWnd, mailbox_delete(hWnd, DelBox, TRUE, FALSE));
					}
				} else {
					mailbox_delete(hWnd, DelBox, TRUE, FALSE);
				}
				if (op.AutoSave != 0) {
					SwitchCursor(FALSE);
					ini_save_setting(hWnd, TRUE, FALSE, NULL);
					SwitchCursor(TRUE);
				}
			} else {
				// pass mailbox commands to main window
				pending = TRUE;
				SendMessage(GetParent(hWnd), msg, wParam, lParam);
				pending = FALSE;
			}
			return 0;
	}

	return CallWindowProc(MBPaneWndProc, hWnd, msg, wParam, lParam);
}

/*
 * MBWidthProc - resize handler
 */
#ifdef _WIN32_WCE
static LRESULT CALLBACK MBWidthProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[BUF_SIZE];
	int tmp;
	switch (uMsg) {

	case WM_INITDIALOG:
		wsprintf(buf, TEXT("%d"), op.MBMenuWidth);
		SendDlgItemMessage(hDlg, IDC_EDIT_MBP_SIZE, WM_SETTEXT, 0, (LPARAM)buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_MBP_SIZE, EM_SETSEL, 0, -1);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_MBP_SIZE:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			SendDlgItemMessage(hDlg, IDC_EDIT_MBP_SIZE, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			tmp = _ttoi(buf);
			if (tmp >= 0 && tmp < GetSystemMetrics(SM_CXSCREEN)) {
				op.MBMenuWidth = tmp;
			}
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;

	default:
		return FALSE;
	}

	return TRUE;
}
#endif

/*
 * CreateMBMenu - create mailbox combobox or listview
 */
static int CreateMBMenu(HWND hWnd, int Top, int Bottom)
{
	HWND hCombo;
	RECT rcClient;
	int i, width, height, ret = 0;
	DWORD style = WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL;

	GetClientRect(hWnd, &rcClient);
	if (op.MBMenuWidth > 0) {
		style |= WS_HSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_SIZEBOX;
		height = op.MBMenuHeight = rcClient.bottom - Top - Bottom;
		width = op.MBMenuWidth;
	} else {
		// compatibility mode, drop down combo at top
		height = 200;
		if (rcClient.bottom > height) {
			height = rcClient.bottom;
		}
		op.MBMenuHeight = 0;
		width = rcClient.right;
		style |= CBS_DROPDOWNLIST;
	}

	hCombo = CreateWindow((op.MBMenuWidth>0) ? TEXT("LISTBOX") : TEXT("COMBOBOX"), TEXT(""), style,
		0, Top, width, height, hWnd, (HMENU)IDC_MBMENU, hInst, NULL);
	if (hCombo == NULL) {
		return -1;
	}

#ifndef _WIN32_WCE
	SendDlgItemMessage(hWnd, IDC_MBMENU, WM_SETFONT,
		(WPARAM)((hListFont != NULL) ? hListFont : GetStockObject(DEFAULT_GUI_FONT)),
		MAKELPARAM(TRUE,0));
#endif
	if (op.MBMenuWidth > 0) {
		// insert custom handler to deal with WM_SIZE
		MBPaneWndProc = (WNDPROC)SetWindowLong(hCombo, GWL_WNDPROC, (DWORD)MBPaneProc);
	} else {
		MBPaneWndProc = NULL;
		SendDlgItemMessage(hWnd, IDC_MBMENU, CB_SETEXTENDEDUI, TRUE, 0);
		GetWindowRect(GetDlgItem(hWnd, IDC_MBMENU), &rcClient);
		ret = op.MBMenuHeight = rcClient.bottom - rcClient.top;
	}

	SendMessage(hCombo, WM_SETREDRAW, (WPARAM)FALSE, 0);
	AddMBMenu(STR_SENDBOX_NAME); // updated later
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		SetMailboxMark(i, STATUS_DONE, TRUE);
	}
	SendMessage(hCombo, WM_SETREDRAW, (WPARAM)TRUE, 0);
	UpdateWindow(hCombo);

	return ret;
}

/*
 * CreatePreviewPane
 */
static void CreatePreviewPane(HWND hWnd, int Left, int Top, int width, int height) {
	HWND previewWnd = NULL;
#ifndef _WIN32_WCE
	WNDPROC OldWndProc = NULL;

#endif

	CreateWindowEx(
#ifdef _WIN32_WCE_PPC
		0,
#else
		WS_EX_CLIENTEDGE,
#endif
		op.WindowClass, TEXT(""),
#ifdef _WIN32_WCE_PPC
		WS_BORDER |
#endif
		WS_VISIBLE | WS_CHILD | ES_MULTILINE  | WS_SIZEBOX |
		WS_VSCROLL | ((op.WordBreakFlag == 1) ? 0 : WS_HSCROLL),
		Left, Top, width, height, hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
	if (hViewFont != NULL) {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
	}
	previewWnd = GetDlgItem(hWnd, IDC_EDIT_BODY);
#ifdef _WIN32_WCE
	PreviewWindowProcedure = (WNDPROC)SetWindowLong(previewWnd, GWL_WNDPROC, (DWORD)SubClassSentProc);
#else
	OldWndProc = (WNDPROC)SetWindowLong(previewWnd, GWL_WNDPROC, (DWORD)SubClassSentProc);
	SetProp(previewWnd, WNDPROC_KEY, OldWndProc);
	if (op.RichEdit) {
		// enable URL detection (color/underline) and messages
		SendMessage(previewWnd, EM_AUTOURLDETECT, 1, 0);
		SendMessage(previewWnd, EM_SETEVENTMASK, 0, ENM_LINK);
		// readonly for RichEdit; regular Edit gets a gray background
		SendMessage(previewWnd, EM_SETREADONLY, TRUE, 0);
	}
#endif
}

/*
 * DelPreviewSubClass - Reset window callback to standard
 */
static void DelPreviewSubClass(HWND hWnd)
{
#ifdef _WIN32_WCE
//	SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)PreviewWindowProcedure);
//	PreviewWindowProcedure = NULL;
#else
	WNDPROC OldWndProc = (WNDPROC)GetProp(hWnd, WNDPROC_KEY);
	if (OldWndProc) {
		SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)OldWndProc);
	}
	RemoveProp(hWnd, WNDPROC_KEY);
#endif
}

/*
 * InitWindow - �E�B���h�E�̏�����
 */
static BOOL InitWindow(HWND hWnd)
{
	RECT rcClient, StatusRect;
	HDC hdc;
	HFONT hFont;
	HMENU hMenu;
	int Top, Bottom, Left, Right;
	int i, j;
	int Height, Width[2];
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
#ifndef _WIN32_WCE_PPC
	HWND hToolBar;
#endif	// _WIN32_WCE_PPC
#ifndef _WIN32_WCE_SP
	TBBUTTON tbButton[] = {
#ifdef _WIN32_WCE
#ifndef _WIN32_WCE_PPC
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
#endif	// _WIN32_WCE_PPC
#endif
		{0,	ID_MENUITEM_RECV,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{1,	ID_MENUITEM_ALLCHECK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{2,	ID_MENUITEM_EXEC,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{3,	ID_MENUITEM_ALLEXEC,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{4,	ID_MENUITEM_STOP,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{5,	ID_MENUITEM_NEWMAIL,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
#ifdef _WIN32_WCE
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{6,	ID_MENUITEM_DOWNMARK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{7,	ID_MENUITEM_SENDMARK,		TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{8,	ID_MENUITEM_DELMARK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{9,	ID_MENUITEM_DELETE,			TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{10,ID_MENUITEM_FLAGMARK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
#ifdef ENABLE_RAS
		,{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1}
		,{11,ID_MENUITEM_RAS_CONNECT,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
		,{12,ID_MENUITEM_RAS_DISCONNECT, TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
#endif
#ifdef ENABLE_WIFI
		,{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1}
		,{11,ID_MENUITEM_WIFI_CONNECT,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
		,{12,ID_MENUITEM_WIFI_DISCONNECT, TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
#endif
#else
		{6,	ID_MENUITEM_REMESSEGE,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{7,	ID_MENUITEM_ALLREMESSEGE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{8,	ID_MENUITEM_FORWARD,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{9,	ID_MENUITEM_DOWNMARK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{10,ID_MENUITEM_SENDMARK,		TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{11,ID_MENUITEM_DELMARK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{12,ID_MENUITEM_DELETE,			TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{13,ID_MENUITEM_READMAIL,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{14,ID_MENUITEM_UNREADMAIL,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{15,ID_MENUITEM_FLAGMARK,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{16,ID_MENUITEM_FIND,			TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{17,ID_MENUITEM_NEXTFIND,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,							TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{18,ID_MENUITEM_ADDRESS,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
#ifdef ENABLE_RAS
		,{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1}
		,{19,ID_MENUITEM_RAS_CONNECT,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
		,{20,ID_MENUITEM_RAS_DISCONNECT, TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
#endif
#endif
	};
#endif // _WIN32_WCE_SP

#ifdef _WIN32_WCE
	static TCHAR *szTips[] = {
#ifdef _WIN32_WCE_PPC
		NULL, // menu skipping
#endif	// _WIN32_WCE_PPC
		NULL, // menu skipping
		STR_CMDBAR_RECV,
		STR_CMDBAR_ALLCHECK,
		STR_CMDBAR_EXEC,
		STR_CMDBAR_ALLEXEC,
		STR_CMDBAR_STOP,
		STR_CMDBAR_NEWMAIL,
		STR_CMDBAR_DOWNMARK,
		STR_CMDBAR_SENDMARK,
		STR_CMDBAR_DELMARK,
		STR_CMDBAR_DELETE,
		STR_CMDBAR_FLAGMARK
#ifdef ENABLE_RAS
		,STR_CMDBAR_RAS_CONNECT
		,STR_CMDBAR_RAS_DISCONNECT
#endif
#ifdef ENABLE_WIFI
		,STR_CMDBAR_WIFI_CONNECT
		,STR_CMDBAR_WIFI_DISCONNECT
#endif
#ifndef _WIN32_WCE_PPC
		,NULL // extra for CE.net
#endif
	};
#ifdef _WIN32_WCE_PPC
	SHMENUBARINFO mbi;
#endif	// _WIN32_WCE_PPC
#else	// _WIN32_WCE
	RECT ToolbarRect;
#endif	// _WIN32_WCE
#endif	// _WIN32_WCE_LAGENDA

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

	CommandBar_AddToolTips(hMainToolBar, 15, szTips);
#ifdef LOAD_USER_IMAGES
	if (MainBmp)
		CommandBar_AddBitmap(hMainToolBar, NULL, (int)MainBmp, TB_MAINBUTTONS, op.MainBmpSize, op.MainBmpSize);
	else
#endif
		CommandBar_AddBitmap(hMainToolBar, hInst, IDB_TOOLBAR, TB_MAINBUTTONS, TB_ICONSIZE, TB_ICONSIZE);

#ifndef _WIN32_WCE_SP
	CommandBar_AddButtons(hMainToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
#endif

	op.ToolBarHeight = 0;
	i = 0;
	hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE);
	PPCFlag = TRUE;
	MailMenuPos = 1;

#ifdef _WIN32_WCE_SP
	// code courtesy of Christian Ghisler
	if (op.osMajorVer >= 5) {
		// WM5 is 5.1, WM6 is 5.2
		SendMessage(hMainToolBar, SHCMBM_OVERRIDEKEY, VK_F1, 
			MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
		SendMessage(hMainToolBar, SHCMBM_OVERRIDEKEY, VK_F2, 
			MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
	}
#endif

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

	op.ToolBarHeight = g_menu_height = CSOBar_Height(hCSOBar);
	i = 0;
	CheckMenuItem(GetSubMenu(hMainMenu, 1), ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
	hMenu = GetSubMenu(hMainMenu, 0);
	PPCFlag = TRUE;
	MailMenuPos = 1;
#else
	// H/PC & PsPC
	hToolBar = CommandBar_Create(hInst, hWnd, IDC_CB);
	// op.osMajorVer >= 4 is CE.net 4.2 and higher (MobilePro 900c)
	// else HPC2000 (Jornada 690, 720)
	CommandBar_AddToolTips(hToolBar, 14, ((op.osMajorVer >= 4) ? (szTips+1) : szTips));
	CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR, TB_MAINBUTTONS, TB_ICONSIZE, TB_ICONSIZE);

	if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_WINDOW_HPC, 0);
		MailMenuPos = 3;
		CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON) -
			((GetSystemMetrics(SM_CXSCREEN) >= 640) ? 0 : 9), tbButton);
	} else {
		PPCFlag = TRUE;
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_WINDOW, 0);
		MailMenuPos = 1;
		CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON) - 14, tbButton);
	}
	CommandBar_AddAdornments(hToolBar, 0, 0);
	op.ToolBarHeight = CommandBar_Height(hToolBar);
	i = 0;
	hMenu = CommandBar_GetMenu(hToolBar, 0);
#endif
#else
	// Win32
	MailMenuPos = 3;
	if (MainBmp) {
		hToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_TB, TB_MAINBUTTONS, NULL, (UINT)MainBmp,
			tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, op.MainBmpSize, op.MainBmpSize, sizeof(TBBUTTON));
	} else {
		hToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_TB, TB_MAINBUTTONS, hInst, IDB_TOOLBAR,
			tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, TB_ICONSIZE, TB_ICONSIZE, sizeof(TBBUTTON));
	}
	SetWindowLong(hToolBar, GWL_STYLE,
		GetWindowLong(hToolBar, GWL_STYLE) | TBSTYLE_FLAT);
	SendMessage(hToolBar, TB_SETINDENT, 5, 0);
	ShowWindow(hToolBar,SW_SHOW);

	GetWindowRect(hToolBar, &ToolbarRect);
	op.ToolBarHeight = ToolbarRect.bottom - ToolbarRect.top;
	i = SBS_SIZEGRIP | SBT_NOBORDERS;
	hMenu = GetMenu(hWnd);
#endif
	Top = op.ToolBarHeight;

	CheckMenuItem(hMenu, ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_LAN, (op.EnableLAN == 1) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_PREVPANE, (op.PreviewPaneHeight>0) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_MBOXPANE, (op.MBMenuWidth>0) ? MF_CHECKED : MF_UNCHECKED);
#ifndef _WIN32_WCE_LAGENDA
	CheckMenuItem(hMenu, ID_MENUITEM_THREADVIEW, (op.LvThreadView == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

	// ListView�t�H���g
	hdc = GetDC(hWnd);
	if ((op.lv_font.name != NULL && *op.lv_font.name != TEXT('\0')) || op.lv_font.size != 9) {
		hListFont = font_create_or_copy(hWnd, hdc, &op.lv_font);
	}
	hViewFont = font_create_or_copy(hWnd, hdc, &op.view_font);
	if (hViewFont != NULL) {
		hFont = SelectObject(hdc, hViewFont);
	}
	font_charset = font_get_charset(hdc);
	if (hViewFont != NULL) {
		SelectObject(hdc, hFont);
	}
	ReleaseDC(hWnd, hdc);

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

	Bottom = StatusRect.bottom - StatusRect.top;
	// combobox or mailbox pane
	if ((j = CreateMBMenu(hWnd, Top, Bottom)) == -1) {
		return FALSE;
	}
	Top += j;
	Left = (op.MBMenuWidth>0) ? op.MBMenuWidth : 0;
	GetClientRect(hWnd, &rcClient);
	Right = rcClient.right - Left;
	Height = rcClient.bottom - rcClient.top - Top - Bottom;

	if (op.PreviewPaneHeight > 0) {
		Height -= op.PreviewPaneHeight;
		CreatePreviewPane(hWnd, Left, Top+Height, Right, op.PreviewPaneHeight);
	}
	op.PreviewPaneWidth = Right;

	//List view
	mListView = CreateListView(hWnd, Top, Height, Left, Right);
	if (mListView == NULL) {
		return FALSE;
	}

	SetFocus(mListView);
	if (hListFont != NULL) {
		//Font of list view setting
		SendMessage(mListView, WM_SETFONT, (WPARAM)hListFont, MAKELPARAM(TRUE, 0));
	}
	//List view to subclass is converted the
	SetListViewSubClass(mListView);

#ifdef _WIN32_WCE
	//Idea contest of window setting
	SendMessage(hWnd, WM_SETICON, (WPARAM)FALSE,
		(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN), IMAGE_ICON, SICONSIZE, SICONSIZE, 0));
#endif
	return TRUE;
}

/*
 * SetWindowSize - �E�B���h�E�̃T�C�Y�ύX
 */
#ifdef _WIN32_WCE_LAGENDA
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
	GetWindowRect(GetDlgItem(hWnd, IDC_MBMENU), &comboRect);
	Height = (comboRect.bottom - comboRect.top) + g_menu_height;

	if (op.MBMenuWidth > 0) MessageBox(hWnd, TEXT("SetWindowSize not ready!"), TEXT("ERROR"), MB_OK);
	MoveWindow(mListView, 0, Height,
		rcClient.right, rcClient.bottom - Height - (StatusRect.bottom - StatusRect.top), TRUE);
	return ret;
}
#else
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rcClient, subwinRect;
	int newTop, newHeight, Left, Right;
	static BOOL maximized = FALSE;

#ifdef _WIN32_WCE
	if (hWnd == NULL || mListView == NULL) {
#else
	if (hWnd == NULL || mListView == NULL || IsIconic(hWnd) != 0) {
#endif
		return FALSE;
	}

#ifndef _WIN32_WCE
	SendDlgItemMessage(hWnd, IDC_TB, WM_SIZE, wParam, lParam);
#endif
	SendDlgItemMessage(hWnd, IDC_STATUS, WM_SIZE, wParam, lParam);

	if (GetClientRect(hWnd, &rcClient) == 0 || rcClient.bottom == 0) {
		// this occurs somehow on Win7, and IsIconic is non-zero
		return FALSE;
	}
	newHeight = rcClient.bottom; // rcClient.top = 0 always

	// Toolbar
	newTop = op.ToolBarHeight;
	newHeight -= newTop;

	// Status bar
	GetWindowRect(GetDlgItem(hWnd, IDC_STATUS), &subwinRect);
	newHeight -= (subwinRect.bottom - subwinRect.top);

	Left = 0;
	Right = rcClient.right;

	if (op.MBMenuWidth > 0) {
		op.MBMenuHeight = newHeight;
		Left = op.MBMenuWidth;
		Right -= op.MBMenuWidth;
		MoveWindow(GetDlgItem(hWnd, IDC_MBMENU), 0, newTop, Left, newHeight, TRUE);

	} else {
		// combobar
		int comboHeight, comboDropped = 200;
		if (newHeight > comboDropped) {
			comboDropped  = newHeight;
		}
		GetWindowRect(GetDlgItem(hWnd, IDC_MBMENU), &subwinRect);
		comboHeight = op.MBMenuHeight = subwinRect.bottom - subwinRect.top;

		MoveWindow(GetDlgItem(hWnd, IDC_MBMENU), 0, newTop, Right, comboDropped, TRUE);
		newTop += comboHeight;
		newHeight -= comboHeight;
	}
	if (op.PreviewPaneHeight > 0) {
		if (newHeight < 2 * op.PreviewPaneMinHeight) {
			// not enough room -- hide preview window
			HWND previewWnd = GetDlgItem(hWnd, IDC_EDIT_BODY);
			HMENU hMenu;
#ifdef _WIN32_WCE_PPC
			hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE);
#elif defined(_WIN32_WCE)
			hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0);
#else
			hMenu = GetMenu(hWnd);
#endif
			DelPreviewSubClass(previewWnd);
			DestroyWindow(previewWnd);
			CheckMenuItem(hMenu, ID_MENUITEM_PREVPANE, MF_UNCHECKED);
			op.PreviewPaneHeight = -op.PreviewPaneHeight;
		} else {
			if (wParam != SIZE_MAXIMIZED && (wParam != SIZE_RESTORED || maximized == FALSE) ) {
				int oldHeight, dh;
				GetWindowRect(mListView, &subwinRect);
				oldHeight = subwinRect.bottom - subwinRect.top + op.PreviewPaneHeight;
				dh = oldHeight - newHeight;
				op.PreviewPaneHeight -= dh/2;
				if (op.PreviewPaneHeight < op.PreviewPaneMinHeight) {
					op.PreviewPaneHeight = op.PreviewPaneMinHeight;
				}
			}
			newHeight -= op.PreviewPaneHeight;
		}
	}
	if (wParam == SIZE_MAXIMIZED) {
		maximized = TRUE;
	} else {
		maximized = FALSE;
	}
	MoveWindow(mListView, Left, newTop, Right, newHeight, TRUE);
	if (op.PreviewPaneHeight > 0) {
		op.PreviewPaneWidth = Right;
		MoveWindow(GetDlgItem(hWnd, IDC_EDIT_BODY), Left, newTop+newHeight,
		   Right, op.PreviewPaneHeight, TRUE);
	}

	UpdateWindow(mListView);
	return TRUE;
}
#endif

/*
 * SaveWindow - �E�B���h�E�̕ۑ�����
 */
static BOOL SaveWindow(HWND hWnd, BOOL SelDir, BOOL PromptSave, BOOL UpdateStatus)
{
	TCHAR SaveDir[BUF_SIZE];
	int i;
	BOOL SaveAll = (SelDir == TRUE) || (UpdateStatus == TRUE) || (op.AutoSave == 0);
	BOOL err = FALSE;
#ifdef _WIN32_WCE_PPC
	// watch out for quit messages from task managers
	MSG msg;
	PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
#endif

	if (SelDir == FALSE) {
		lstrcpy(SaveDir, DataDir);
	} else {
		BOOL repeat;
		do {
			repeat = FALSE;
			lstrcpy(SaveDir, STR_NPOPUK_FILES);
			if (filename_select(hWnd, SaveDir, NULL, NULL, FILE_CHOOSE_BACKDIR, &op.BackupDir) == FALSE) {
				return FALSE;
			} else if (lstrcmpi(SaveDir, AppDir) == 0) {
				ErrorMessage(hWnd, STR_ERR_BACKUP_APPDIR);
				repeat = TRUE;
			} else if (lstrcmpi(SaveDir, DataDir) == 0) {
				ErrorMessage(hWnd, STR_ERR_BACKUP_DATADIR);
				repeat = TRUE;
			}
		} while (repeat == TRUE);
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
#ifdef ENABLE_RAS
		KillTimer(hWnd, ID_RASWAIT_TIMER);
#endif
#ifdef ENABLE_WIFI
		KillTimer(hWnd, ID_WIFIWAIT_TIMER);
#endif
		gSockFlag = FALSE;
		ExecFlag = FALSE;
		NewMailCnt = -1;
		command_status = POP_QUIT;
		send_buf(g_soc, CMD_RSET"\r\n");
		send_buf(g_soc, CMD_QUIT"\r\n");
		socket_close(hWnd, g_soc);
		g_soc = -1;
#ifdef ENABLE_RAS
		if (op.RasCheckEndDisCon == 1) {
			RasDisconnect();
		}
#endif
#ifdef ENABLE_WIFI
		if (op.WiFiCheckEndDisCon == 1) {
			WiFiDisconnect(FALSE);
		}
#endif
		if (op.SocLog > 0) log_flush();
	}
#ifdef ENABLE_RAS
	//Cutting of dial-up connection
	if (RasLoop == TRUE || op.RasEndDisCon == 1) {
		RasDisconnect();
	}
#endif
#ifdef ENABLE_WIFI
	//Cutting of wifi connection
	if (WiFiLoop == TRUE || op.WiFiExitDisCon > 0) {
		WiFiDisconnect(PromptSave && (op.WiFiExitDisCon==2));
	}
#endif

	//The indicatory and compilation window is closed the
	CloseEditViewWindows(1);

	// Give user the option of not saving
	if (PromptSave == TRUE && op.PromptSaveOnExit != 0) {
		unsigned type = MB_ICONQUESTION | MB_YESNO;
		SwitchCursor(TRUE);
		if (op.PromptSaveOnExit == 2) {
			type |= MB_DEFBUTTON2;
		}
		_SetForegroundWindow(hWnd);
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
		BOOL sendbox_ok;

		//Transmission box (SendBox) retention
		if (op.WriteMbox != (MailBox+MAILBOX_SEND)->WasMbox) {
			(MailBox+MAILBOX_SEND)->NeedsSave |= MBOX_FORMAT_CHANGED;
		}
		// BOOL IsBackup = SelDir;
		sendbox_ok = file_save_mailbox(SENDBOX_FILE, SaveDir, MAILBOX_SEND, SelDir, FALSE, 2);
		if (sendbox_ok == FALSE) {
			err = TRUE;
			if (op.SocLog > 1) {
				TCHAR msg[MSG_SIZE];
				wsprintf(msg, TEXT("Error saving %s\r\n"), SENDBOX_FILE);
				log_save(msg);
			}
		}
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
		if (mListView != NULL) {
			for (i = 0; i < LV_COL_CNT; i++) {
				op.LvColSize[i] = ListView_GetColumnWidth(mListView, i);
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
		SetItemCntStatusText(NULL, FALSE, TRUE);
	}
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * EndWindow - �E�B���h�E�̏I������
 */
static BOOL EndWindow(HWND hWnd)
{
	HIMAGELIST hImgList;
	TCHAR path[BUF_SIZE];

	SwitchCursor(FALSE);

	ListView_DeleteAllItems(mListView);

	//of setting every of account The message window is closed the
	if (MsgWnd != NULL) {
		SendMessage(MsgWnd, WM_ENDDIALOG, 0, 0);
	}

	// �O���A�v���p�t�@�C���̍폜
	str_join_t(path, DataDir, VIEW_FILE, TEXT("."), op.ViewFileSuffix, (TCHAR *)-1);
	DeleteFile(path);
#ifdef _WIN32_WCE
	str_join_t(path, DataDir, EDIT_FILE, TEXT("."), op.EditFileSuffix, (TCHAR *)-1);
	DeleteFile(path);
#endif

	// �Y�t�t�@�C���̍폜
	if (op.AttachDelete != 0) {
		TCHAR file[BUF_SIZE];
		wsprintf(path, TEXT("%s%s\\"), DataDir, op.AttachPath);
		wsprintf(file, TEXT("%s*"), ATTACH_FILE);
		dir_delete(path, file);
		RemoveDirectory(path);
	}

	// ����������̉��
	mem_free(&FindStr);
	FindStr = NULL;
	mem_free(&ReplaceStr);
	ReplaceStr = NULL;
	findparts_free();

	//in searching character string Release
	FreeAllMailBox();
	mem_free(&g_Pass);

	// �ݒ�̉��
	ini_free(TRUE);
	filter_free(NULL); // global filters
#ifdef ENABLE_RAS
	FreeRasInfo();
#endif
#ifdef ENABLE_WIFI
	FreeWiFiInfo();
	KillTimer(MainWnd, ID_WIFICHECK_TIMER);
#endif

	// �^�X�N�g���C�̃A�C�R���̏���
	op.ShowTrayIcon = 0;
	TrayMessage(hWnd, NIM_DELETE, TRAY_ID, NULL);

	//of idea contest of task tray Cancellation
	DestroyIcon(TrayIcon_Main);
	DestroyIcon(TrayIcon_Check);
	DestroyIcon(TrayIcon_Mail);

	//of idea contest Cancellation
	DelListViewSubClass(mListView);

	//of subclass conversion of list view Cancellation
	hImgList = ListView_SetImageList(mListView, NULL, LVSIL_SMALL);
	ImageList_Destroy((void *)hImgList);
	hImgList = ListView_SetImageList(mListView, NULL, LVSIL_STATE);
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
	DestroyWindow(GetDlgItem(hWnd, IDC_MBMENU));
	if (op.PreviewPaneHeight > 0) {
		HWND previewWnd = GetDlgItem(hWnd, IDC_EDIT_BODY);
		DelPreviewSubClass(previewWnd);
		DestroyWindow(previewWnd);
	}
	DestroyWindow(mListView);
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
 * SendMail - ���[���̑��M�J�n
 */
static BOOL SendMail(HWND hWnd, MAILITEM *tpMailItem, int end_cmd)
{
	MAILBOX *tpMailBox;
	TCHAR *pass;
	TCHAR ErrStr[BUF_SIZE];
	int BoxIndex;

	BoxIndex = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_ACCOUNT);
	if (BoxIndex == -1) {
		ErrorSocketEnd(hWnd, MAILBOX_SEND);
		SocketErrorMessage(hWnd, STR_ERR_SELECTMAILBOX, MAILBOX_SEND);
		return FALSE;
	}
	tpMailBox = MailBox + BoxIndex;

#ifdef ENABLE_RAS
	//of font Dial rise start
	if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, BoxIndex, 0) == FALSE) {
		ErrorSocketEnd(hWnd, MAILBOX_SEND);
		if (op.SocLog > 0) log_flush();
		SetMailMenu(hWnd);
		return FALSE;
	}
#endif
#ifdef ENABLE_WIFI
	// WiFi connection
	if (op.WiFiCon == 1 && SendMessage(hWnd, WM_WIFI_START, BoxIndex, 0) == FALSE) {
		ErrorSocketEnd(hWnd, MAILBOX_SEND);
		if (op.SocLog > 0) log_flush();
		SetMailMenu(hWnd);
		return FALSE;
	}
#endif

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
			if (gPassSt >= 10) {
				//Save in INI
				if (tpMailBox->AuthUserPass == 1) {
					mem_free(&tpMailBox->SmtpPass);
					tpMailBox->SmtpPass = alloc_copy_t(g_Pass);
				} else {
					mem_free(&tpMailBox->Pass);
					tpMailBox->Pass = alloc_copy_t(g_Pass);
				}
			} else if (gPassSt == 1) {
				//Temporarily the setting
				if (tpMailBox->AuthUserPass == 1) {
					tpMailBox->SmtpTmpPass = alloc_copy_t(g_Pass);
				} else {
					tpMailBox->TmpPass = alloc_copy_t(g_Pass);
				}
			}
		}
	}

	if (op.SocLog > 0) log_header("send");

	SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);

	SwitchCursor(FALSE);
	command_proc = smtp_proc;
	command_status = SMTP_START;
	ExecFlag = TRUE;
	(MailBox + MAILBOX_SEND)->NeedsSave |= MARKS_CHANGED;
	RecvBox = MAILBOX_SEND;

	*ErrStr = TEXT('\0');
	g_soc = smtp_send_mail(hWnd, tpMailBox, tpMailItem, end_cmd, ErrStr);
	if (g_soc == -1) {
		ErrorSocketEnd(hWnd, MAILBOX_SEND);
		SocketErrorMessage(hWnd, ErrStr, MAILBOX_SEND);
		return FALSE;
	}

#ifndef WSAASYNC
	SetTimer(hWnd, ID_RECV_TIMER, RECVTIME, NULL);
#endif
	SetMailMenu(hWnd);
	SetTrayIcon(hWnd, TrayIcon_Check);
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * RecvMailList - ���[���̈ꗗ�̎�M
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
		if (gPassSt >= 10) {
			//Save in INI
			mem_free(&tpMailBox->Pass);
			tpMailBox->Pass = alloc_copy_t(g_Pass);
		} else if (gPassSt == 1) {
			//Temporarily the setting
			tpMailBox->TmpPass = alloc_copy_t(g_Pass);
		}
	}

	if (op.SocLog > 0) log_header("recv");

	RecvBox = BoxIndex;
	SetMailboxMark(RecvBox, STATUS_CHECK, FALSE);

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
	SetTrayIcon(hWnd, TrayIcon_Check);
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * MailMarkCheck - �폜���[�����Ȃ����`�F�b�N����
 */
static BOOL MailMarkCheck(HWND hWnd, BOOL IsAfterCheck)
{
	int i, j;
	BOOL ret = FALSE;
	BOOL held = FALSE;
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
		if (ServerDelete == TRUE && (j = item_get_next_delete_mark((MailBox + i), FALSE, -1, NULL)) != -1) {
			TCHAR msg[MSG_SIZE];
			wsprintf(msg, STR_Q_DEL_FWDHOLD_ACCT, ((MailBox + i)->Name == NULL || *(MailBox + i)->Name == TEXT('\0'))
				? STR_MAILBOX_NONAME :(MailBox + i)->Name);
			if (MessageBox(hWnd, STR_Q_DEL_FWDHOLD, STR_TITLE_ALLEXEC, MB_ICONEXCLAMATION | MB_YESNOCANCEL) == IDNO) {
				held = TRUE; // "marked message is held" instead of "no marked mail"
			} else {
				ret = TRUE;
				while (j != -1) {
					(*((MailBox + i)->tpMailItem + j))->ReFwd &= ~(REFWD_FWDHOLD);
					j = item_get_next_delete_mark((MailBox + i), FALSE, j, NULL);
				}
			}
		}
		if (ServerDelete == TRUE && item_get_next_delete_mark((MailBox + i), TRUE, -1, NULL) != -1) {
			if ((IsAfterCheck == FALSE && op.ExpertMode == 1) ||
				(IsAfterCheck == TRUE && op.CheckEndExecNoDelMsg == 0)) {
				ret = TRUE;
				break;
			} else {
				int ans, st;
				ans = MessageBox(hWnd, (held == FALSE) ? STR_Q_DELSERVERMAIL : STR_Q_DELSERVERNOHOLD,
					(MailBox + i)->Name, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
				if (ans == IDYES) {
					ret = TRUE;
					break;
				} else if (ans == IDCANCEL) {
					mailbox_select(hWnd, i);
					if ((i = ListView_GetNextDeleteItem(mListView, -1)) != -1) {
						ListView_SetItemState(mListView, -1, 0, LVIS_SELECTED);
						st = LVIS_FOCUSED;
						if (op.PreviewPaneHeight <=0 || op.AutoPreview) {
							st |= LVIS_SELECTED;
						}
						ListView_SetItemState(mListView, i, st, st);
						ListView_EnsureVisible(mListView, i, TRUE);
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
	} else if (ret == FALSE && item_get_next_send_mark((MailBox + MAILBOX_SEND), FALSE, TRUE) != -1) {
		ret = TRUE;
	} else if (IsAfterCheck == FALSE && ret == FALSE) {
		MessageBox(hWnd, (held) ? STR_MSG_MARK_HELD : STR_MSG_NOMARK, STR_TITLE_ALLEXEC,
			MB_ICONEXCLAMATION | MB_OK);
	}
	return ret;
}

/*
 * ExecItem - �}�[�N�������̂����s
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
		if (item_get_next_send_mark(tpMailBox, FALSE, TRUE) == -1) {
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

#ifndef _WIN32_WCE
	if (ServerDelete != ID_MENUITEM_DELETE_ALL_SERVER)
#endif
	{
		if (item_get_next_download_mark(tpMailBox, -1, NULL) == -1 &&
			item_get_next_delete_mark(tpMailBox, TRUE, -1, NULL) == -1) {
			return FALSE;
		}
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
		if (gPassSt >= 10) {
			//Save in INI
			mem_free(&tpMailBox->Pass);
			tpMailBox->Pass = alloc_copy_t(g_Pass);
		} else if (gPassSt == 1) {
			//Temporarily the setting
			tpMailBox->TmpPass = alloc_copy_t(g_Pass);
		}
	}

	if (op.SocLog > 0) log_header("exec");

	RecvBox = BoxIndex;
	SetMailboxMark(RecvBox, STATUS_CHECK, FALSE);

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
	SetTrayIcon(hWnd, TrayIcon_Check);
	SwitchCursor(TRUE);
	return TRUE;
}

/*
 * OpenItem - �A�C�e�����J��
 */
void OpenItem(HWND hWnd, BOOL MsgFlag, BOOL NoAppFlag, BOOL CheckSel)
{
	MAILITEM *tpMailItem;
	int i;

	if (CheckSel && op.PreviewPaneHeight <= 0 && ListView_GetSelectedCount(mListView) <= 0) {
		return;
	}
	i = ListView_GetNextItem(mListView, -1, LVNI_FOCUSED);
	if (i == -1) {
		return;
	}
	ListView_EnsureVisible(mListView, i, TRUE);
	tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
	if (tpMailItem == NULL) {
		return;
	}
	if (SelBox == MAILBOX_SEND) {
		if (tpMailItem->RedirectTo != NULL && tpMailItem->MailStatus != ICON_SENTMAIL) {
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_REDIRECT), hWnd, SetSendProc,
				(LPARAM)tpMailItem) == TRUE) {
				(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
			}
		} else {
			int ret = Edit_InitInstance(hInst, hWnd, -1, tpMailItem, EDIT_OPEN, NULL, NoAppFlag);
			if (ret == EDIT_INSIDEEDIT) {
				// GJC: don't edit sent mail
				Edit_ConfigureWindow(tpMailItem->hEditWnd, (tpMailItem->MailStatus == ICON_SENTMAIL) ? FALSE : TRUE);
			}
#ifdef _WIN32_WCE
			if (ret == EDIT_INSIDEEDIT || ret == EDIT_REOPEN) {
				ShowWindow(hWnd, SW_HIDE);
			}
#endif
		}
		_SetForegroundWindow(tpMailItem->hEditWnd);
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
				ListView_SetItemState(mListView, i, LVIS_CUT, LVIS_CUT);
			} else {
				tpMailItem->Mark = ICON_DOWN;
				ListView_SetItemState(mListView, i, 0, LVIS_CUT);
			}
			ListView_RedrawItems(mListView, i, i);
			UpdateWindow(mListView);
			return;
		}
	}
	if (View_InitInstance(hInst,
		(LPVOID)ListView_GetlParam(mListView, i), NoAppFlag) == TRUE) {
#ifdef _WIN32_WCE
		ShowWindow(hWnd, SW_HIDE);
#endif
	}
}

/*
 * ReMessageItem - �ԐM�̍쐬
 */
static void ReMessageItem(HWND hWnd, int ReplyFlag)
{
	int i;

	// will choose the item with focus, even if it isn't selected (or if several are selected)
	i = ListView_GetNextItem(mListView, -1, LVNI_FOCUSED);
	if (i < 0)
		return;

	if (SelBox == MAILBOX_SEND && ReplyFlag == EDIT_REPLY) {
#ifdef _WIN32_WCE
		int res = IDD_DIALOG_SETSEND;
		if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
			res = IDD_DIALOG_SETSEND_WIDE;
#ifdef _WIN32_WCE_PPC
		} else if (GetSystemMetrics(SM_CYSCREEN) <= 260) {
			res = IDD_DIALOG_SETSEND_SHORT;
#endif
		}
		DialogBoxParam(hInst, MAKEINTRESOURCE(res), hWnd, SetSendProc,
				(LPARAM)ListView_GetlParam(mListView, i));
#else
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
			(LPARAM)ListView_GetlParam(mListView, i));
#endif
		// Refresh the screen with any changes
		ListView_RedrawItems(mListView, i, i);
		UpdateWindow(mListView);
	} else {
		if (Edit_InitInstance(hInst, hWnd, SelBox,
			(MAILITEM *)ListView_GetlParam(mListView, i), ReplyFlag, NULL, FALSE) == EDIT_INSIDEEDIT) {
#ifdef _WIN32_WCE
			ShowWindow(hWnd, SW_HIDE);
#endif
		}
	}
}

/*
 * ItemToSaveBox - move/copy mailitem to a savebox
 */
BOOL ItemToSaveBox(HWND hWnd, MAILITEM *tpSingleItem, int TargetBox, TCHAR *fname, BOOL ask, BOOL del)
{
	MAILBOX *tpMailBox;
	MAILITEM *tpMailItem;
	MAILITEM *tpTmpMailItem;
	TCHAR *title;
	int i, j, SelPoint = -1;
	int new_count = 0, flag_count = 0;
	BOOL retval = TRUE;

	if (tpSingleItem == NULL) {
		if ((i = ListView_GetSelectedCount(mListView)) <= 0) {
			ErrorMessage(hWnd, STR_ERR_NOSELECT);
			return FALSE;
		}
	} else {
		i = 1;
	}
	if (del) {
		title = STR_TITLE_MOVE;
	} else {
		title = STR_TITLE_COPY;
	}
	tpMailBox = MailBox + TargetBox;
	if (ask && TargetBox != MAILBOX_SEND && op.SaveMsg == 1) {
		TCHAR msg[MSG_SIZE];
		if (del) {
			wsprintf(msg, STR_Q_MOVE, i, tpMailBox->Name);
		} else {
			wsprintf(msg, STR_Q_COPY, i, tpMailBox->Name);
		}
		if (ParanoidMessageBox(hWnd, msg, title, MB_ICONQUESTION | MB_YESNO) == IDNO) {
			return FALSE;
		}
	}

	SwitchCursor(FALSE);
	if (tpMailBox->Loaded == FALSE) {
		if (op.BlindAppend == 0) { // || TargetBox == MAILBOX_SEND, assumed always loaded
			if (mailbox_load_now(hWnd, TargetBox, FALSE, FALSE) != 1) {
				ErrorMessage(hWnd, STR_ERR_SAVECOPY);
				return FALSE;
			}
		}
	}
	i = -1;
	while (1) {
		if (tpSingleItem == NULL) {
			i = ListView_GetNextItem(mListView, i, LVNI_SELECTED);
			if (i == -1) {
				break;
			}
			tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
			if (tpMailItem == NULL) {
				continue;
			}
		} else {
			tpMailItem = tpSingleItem;
		}
		if (tpMailItem->New) {
			new_count++;
		}
		if (tpMailItem->Mark == ICON_FLAG) {
			flag_count++;
		}

		if (TargetBox == MAILBOX_SEND) {
			int state = 0;

			//The copy is drawn up in the transmission box the
			if ((tpTmpMailItem = item_to_mailbox(tpMailBox, tpMailItem, NULL, TRUE)) == NULL) {
				ErrorMessage(hWnd, STR_ERR_SAVECOPY);
				retval = FALSE;
				break;
			}
			j = ListView_InsertItemEx(mListView,
				(TCHAR *)LPSTR_TEXTCALLBACK, 0, I_IMAGECALLBACK, (long)tpTmpMailItem,
				ListView_GetItemCount(mListView));

			state = ListView_ComputeState(tpMailItem->Priority, tpMailItem->Multipart);
			ListView_SetItemState(mListView, j, INDEXTOSTATEIMAGEMASK(state), LVIS_STATEIMAGEMASK)
			ListView_RedrawItems(mListView, j, j);
			UpdateWindow(mListView);
			if (tpMailItem->Attach != NULL) {
				TCHAR fpath[BUF_SIZE];
				TCHAR *f = tpMailItem->Attach;
				while (*f != TEXT('\0')) {
					f = str_cpy_f_t(fpath, f, ATTACH_SEP);
					if (file_get_size(fpath) == -1) {
						MessageBox(hWnd, STR_MSG_NOATT, STR_TITLE_COPY, MB_OK);
						break;
					}
				}
			}
			if (tpMailItem->FwdAttach != NULL) {
				MAILITEM *tpFwdMailItem = NULL;
				int k, kk;
				k = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_ACCOUNT);
				if (k == -1) {
					k = mailbox_name_to_index(tpMailItem->MailBox, MAILBOX_TYPE_SAVE);
				}
				if (k != -1) {
					if ((MailBox + k)->Loaded
							&& (kk = item_find_thread(MailBox + k, tpMailItem->References,
														(MailBox+k)->MailItemCnt)) != -1) {
						tpFwdMailItem = (*((MailBox + k)->tpMailItem + kk));
					}
				}
				if (tpFwdMailItem == NULL) {
					tpFwdMailItem = item_find_thread_anywhere(tpMailItem->References);
				}
				if (tpFwdMailItem == NULL) {
					MessageBox(hWnd, STR_MSG_NOFWD, STR_TITLE_COPY, MB_OK);
				}
			}

			if (SelPoint == -1) {
				SelPoint = j;
			}
		} else if (tpMailBox->Loaded) {
			// See if message is already in this box
			j = item_find_thread(tpMailBox, tpMailItem->MessageID, tpMailBox->MailItemCnt);
			if (j != -1) {
				// If selected is fully downloaded, or saved is not either, OK to overwrite
				if (tpMailItem->Download == TRUE
					|| (*(tpMailBox->tpMailItem + j))->Download == FALSE) {
					item_free((tpMailBox->tpMailItem + j), 1);
				} else {
					// Selected message is incomplete but saved is complete, verify desire to overwrite
					TCHAR msg[MSG_SIZE];
					wsprintf(msg, STR_Q_OVERWRITE, tpMailItem->Subject);
					SwitchCursor(TRUE);
					if (MessageBox(hWnd, msg, title, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
						retval = FALSE;
						continue;
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
		} else {
			// mailbox not loaded -> append to file
			file_append_savebox(fname, tpMailBox, tpMailItem, 2);
		}
		if (tpSingleItem != NULL) {
			break;
		}
	}
	if (tpMailBox->Loaded) {
		item_resize_mailbox(tpMailBox, FALSE);
	}
	tpMailBox->NewMail += new_count;
	tpMailBox->FlagCount += flag_count;
	SetItemCntStatusText(NULL, FALSE, TRUE);
	if (g_soc == -1 || TargetBox != SelBox) {
		SetMailboxMark(TargetBox, STATUS_DONE, FALSE);
	}
	if (SelPoint != -1) {
		int st = LVIS_FOCUSED;
		//of mail item The item which is added is selected the
		ListView_SetItemState(mListView, -1, 0, LVIS_SELECTED);
		if (op.PreviewPaneHeight <=0 || op.AutoPreview) {
			st |= LVIS_SELECTED;
		}
		ListView_SetItemState(mListView, SelPoint,	st, st);
		ListView_EnsureVisible(mListView, SelPoint, TRUE);
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
	BOOL had_mark = FALSE; // update mailbox flag (*~) if necessary
	int cnt, i;

	if ((cnt = ListView_GetSelectedCount(mListView)) <= 0) {
		return;
	}
	if (Ask == TRUE && op.ExpertMode != 1) {
		BOOL held = FALSE;
		i = -1;
		while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
			MAILITEM *tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
			if (tpMailItem != NULL && tpMailItem->ReFwd & REFWD_FWDHOLD) {
				held = TRUE;
				break;
			}
		}
		if (held) {
			if (MessageBox(hWnd, STR_Q_DEL_FWDHOLD, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
				return;
			}
		} else {
			TCHAR buf[BUF_SIZE];
			wsprintf(buf, STR_Q_DELLISTMAIL, cnt, (SelBox != MAILBOX_SEND && ((MailBox + SelBox)->Type != MAILBOX_TYPE_SAVE))
				? STR_Q_DELLISTMAIL_NOSERVER : TEXT(""));
			if (MessageBox(hWnd, buf, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
				return;
			}
		}
	}

	SwitchCursor(FALSE);
	ListView_SetRedraw(mListView, FALSE);
	while ((i = ListView_GetNextItem(mListView, -1, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem != NULL) {
			if (tpMailItem->New || tpMailItem->Mark == ICON_FLAG || tpMailItem->Mark == ICON_SEND) {
				had_mark = TRUE;
			}
			tpMailItem->Mark = -1;
		}
		ListView_DeleteItem(mListView, i);
	}
	//As for memory in NULL setting
	for (i = 0; i < (MailBox + SelBox)->MailItemCnt; i++) {
		MAILITEM *tpMailItem = *((MailBox + SelBox)->tpMailItem + i);
		if (tpMailItem == NULL || tpMailItem->Mark != -1) {
			continue;
		}
		if (SelBox == MAILBOX_SEND) {
			ClearFwdHold(tpMailItem);
		}
		item_free(((MailBox + SelBox)->tpMailItem + i), 1);
	}
	item_resize_mailbox(MailBox + SelBox, FALSE);

	ListView_SetRedraw(mListView, TRUE);
	SetItemCntStatusText(NULL, FALSE, (SelBox==MAILBOX_SEND) ? TRUE : FALSE);
	if (had_mark && (g_soc == -1 || RecvBox != SelBox)) {
		SetMailboxMark(SelBox, STATUS_DONE, FALSE);
	}
	SwitchCursor(TRUE);
}
/*
 * ListDeleteAttach - delete attachments from selected messages
 */
static void ListDeleteAttach(HWND hWnd)
{
	int i;
	BOOL did_one = FALSE;

	if ((i = ListView_GetSelectedCount(mListView)) <= 0) {
		return;
	}
	if (ParanoidMessageBox(hWnd, STR_Q_DELATTACH, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
		return;
	}

	SwitchCursor(FALSE);
	ListView_SetRedraw(mListView, FALSE);
	i = -1;
	while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		MAILITEM *tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem != NULL && tpMailItem->Multipart > MULTIPART_ATTACH) {
			if (tpMailItem->ReFwd & REFWD_FWDHOLD) {
				MessageBox(hWnd, STR_MSG_ATT_HELD, STR_TITLE_DELETE, MB_OK);
				continue;
			}
			if (DeleteAttachFile(hWnd, tpMailItem) == FALSE) {
				MessageBox(hWnd, STR_ERR_NO_DEL_ATTACH, STR_TITLE_DELETE, MB_OK);
				break;
			}
			ListView_SetItemState(mListView, i, LVIS_CUT, LVIS_CUT);
			did_one = TRUE;
		}
	}
	if (did_one == TRUE) {
		(MailBox+SelBox)->NeedsSave |= MAILITEMS_CHANGED;
	}
	ListView_SetRedraw(mListView, TRUE);
	SwitchCursor(TRUE);
}

/*
 * SetDownloadMark - �A�C�e���Ɏ�M�}�[�N��t��
 */
static void SetDownloadMark(HWND hWnd)
{
	MAILITEM *tpMailItem;
	BOOL marked_one = FALSE, set = FALSE, changed_flag = FALSE;
	int SendOrDownIcon = ((SelBox == MAILBOX_SEND) ? ICON_SEND : ICON_DOWN);
	int i;

	if (ListView_GetSelectedCount(mListView) <= 0) {
		return;
	}
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	// if one is unset, then set them all; else clear them all
	i = -1;
	while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Mark != SendOrDownIcon && (SelBox != MAILBOX_SEND || tpMailItem->MailStatus != ICON_SENTMAIL)) {
			set = TRUE;
			break;
		}
	}
	i = -1;
	while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (SelBox != MAILBOX_SEND || tpMailItem->MailStatus != ICON_SENTMAIL) {
			marked_one = TRUE;
			if (tpMailItem->Mark == ICON_FLAG) {
				changed_flag = TRUE;
				(MailBox+SelBox)->FlagCount--;
			}
			if (set == TRUE) {
				tpMailItem->Mark = SendOrDownIcon;
				ListView_SetItemState(mListView, i, 0, LVIS_CUT);
			} else {
				tpMailItem->Mark = tpMailItem->MailStatus;
				if (SelBox != MAILBOX_SEND && tpMailItem->Download == FALSE) {
					ListView_SetItemState(mListView, i, LVIS_CUT, LVIS_CUT);
				}
			}
			ListView_RedrawItems(mListView, i, i);
		}
	}
	UpdateWindow(mListView);
	if (SelBox == MAILBOX_SEND && marked_one == TRUE) {
		// actually marking to send
		SetItemCntStatusText(NULL, FALSE, TRUE);
	} else if (changed_flag) {
		SetMailboxMark(SelBox, STATUS_DONE, FALSE);
	}

	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
}

/*
 * SetFlagOrDeleteMark - �A�C�e���ɍ폜�}�[�N��t��
 */
static void SetFlagOrDeleteMark(HWND hWnd, int Mark, BOOL Clear)
{
	MAILITEM *tpMailItem;
	int i;
	BOOL set = Clear; // Clear == TRUE when called to move to savebox

	if (ListView_GetSelectedCount(mListView) <= 0) {
		return;
	}
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	// if one is unset, then set them all; else clear them all
	i = -1;
	while (set == FALSE && (i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Mark != Mark) {
			set = TRUE;
		}
	}

	i = -1;
	while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (set == TRUE) {
			tpMailItem->Mark = Mark;
			ListView_SetItemState(mListView, i, 0, LVIS_CUT);
			if (Clear == TRUE) { // && Mark == ICON_DEL
				tpMailItem->ReFwd &= ~(REFWD_FWDHOLD);
			}
		} else {
			tpMailItem->Mark = tpMailItem->MailStatus;
			if (SelBox != MAILBOX_SEND && tpMailItem->Download == FALSE) {
				ListView_SetItemState(mListView, i, LVIS_CUT, LVIS_CUT);
			}
		}
		ListView_RedrawItems(mListView, i, i);
	}
	UpdateWindow(mListView);
	if (Mark == ICON_FLAG) {
		SetItemCntStatusText(NULL, FALSE, FALSE);
		if (g_soc == -1 || RecvBox != SelBox) {
			SetMailboxMark(SelBox, STATUS_DONE, FALSE);
		}
	}

	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
}

/*
 * UnMark - �A�C�e���̃}�[�N������
 */
static void UnMark(HWND hWnd)
{
	MAILITEM *tpMailItem;
	int i;
	BOOL unmarked_one = FALSE, unflagged_one = FALSE;

	if (ListView_GetSelectedCount(mListView) <= 0) {
		return;
	}
	i = -1;
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->MailStatus == ICON_ERROR) {
			if (SelBox == MAILBOX_SEND) {
				unmarked_one = TRUE;
				tpMailItem->MailStatus = ICON_NON;
			} else {
				tpMailItem->MailStatus = ICON_READ;
			}
		}
		if (tpMailItem->Mark != tpMailItem->MailStatus) {
			unmarked_one = TRUE;
			if (tpMailItem->Mark == ICON_FLAG) {
				unflagged_one = TRUE;
				(MailBox+SelBox)->FlagCount--;
			}
			tpMailItem->Mark = tpMailItem->MailStatus;
		}
		if (SelBox != MAILBOX_SEND && tpMailItem->Download == FALSE) {
			ListView_SetItemState(mListView, i, LVIS_CUT, LVIS_CUT);
		}
		ListView_RedrawItems(mListView, i, i);
	}
	UpdateWindow(mListView);

	if (SelBox == MAILBOX_SEND && unmarked_one == TRUE) {
		SetItemCntStatusText(NULL, FALSE, TRUE);
	} else if (unflagged_one == TRUE) {
		SetMailboxMark(SelBox, STATUS_DONE, FALSE);
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
	int i;
	BOOL found = FALSE;

	if (!(tpReMailItem->ReFwd & Mark)) {
		tpReMailItem->ReFwd |= Mark;
		tpReMailItem->New = FALSE;
		if (((MailBox+rebox)->NewMail) > 1) {
			// leave the count at 1, so we'll know to remove
			// the * from the mailbox name in the drop-down menu
			SetItemCntStatusText(NULL, FALSE, FALSE);
		} else {
			(MailBox+rebox)->NewMail--;
		}
		(MailBox+rebox)->NeedsSave |= MARKS_CHANGED;
	}
	
	if (mListView == NULL) {
		return;
	}

	i = -1;
	while (!found && (i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem != NULL && tpMailItem == tpReMailItem) {
			found = TRUE;
		}
	}
	if (!found) i = -1;
	while (!found && (i = ListView_GetNextItem(mListView, i, 0)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem != NULL && tpMailItem == tpReMailItem) {
			found = TRUE;
		}
	}
	if (found == TRUE) {
		ListView_SetItemState(mListView, i, INDEXTOOVERLAYMASK(tpMailItem->ReFwd & ICON_REFWD_MASK), LVIS_OVERLAYMASK);
		ListView_RedrawItems(mListView, i, i);
	}
	UpdateWindow(mListView);

	if (hViewWnd != NULL) {
		SendMessage(hViewWnd, WM_CHANGE_MARK, 0, 0);
	}
}

/*
 * SetReadMail - �I�����[�����J���ς݁A���J���ɂ���
 */
static void SetMailStats(HWND hWnd, int St)
{
	MAILITEM *tpMailItem;
	int i;

	if (ListView_GetSelectedCount(mListView) <= 0) {
		return;
	}
	(MailBox+SelBox)->NeedsSave |= MARKS_CHANGED;

	i = -1;
	while ((i = ListView_GetNextItem(mListView, i, LVNI_SELECTED)) != -1) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
		if (tpMailItem == NULL ||
			tpMailItem->MailStatus == ICON_NON || tpMailItem->MailStatus >= ICON_SENTMAIL) {
			continue;
		}
		tpMailItem->MailStatus = St;
		if (tpMailItem->Mark != ICON_DOWN && tpMailItem->Mark != ICON_DEL && tpMailItem->Mark != ICON_FLAG) {
			tpMailItem->Mark = St;
		}
		if (St == ICON_READ) {
			tpMailItem->New = FALSE;
			ListView_SetItemState(mListView, i, INDEXTOOVERLAYMASK(tpMailItem->ReFwd & ICON_REFWD_MASK), LVIS_OVERLAYMASK);
		} else if (St == ICON_MAIL) {
			// also clear re/fwd overlay
			tpMailItem->ReFwd &= ~(ICON_REFWD_MASK);
			ListView_SetItemState(mListView, i, 0, LVIS_OVERLAYMASK);
		}
		ListView_RedrawItems(mListView, i, i);
	}
	UpdateWindow(mListView);
	SetItemCntStatusText(NULL, FALSE, TRUE);
}

/*
 * EndSocketFunc - �ʐM�I�����̏���
 */
static void EndSocketFunc(HWND hWnd, BOOL DoTimer)
{
	int i;

#ifdef ENABLE_RAS
	if (op.RasCheckEndDisCon == 1) {
		if (DoTimer == FALSE ||
			op.RasCheckEndDisConTimeout==0 ||
			TimedMessageBox(hWnd, STR_Q_RASDISCON, WINDOW_TITLE, MB_YESNO, op.RasCheckEndDisConTimeout) != IDNO) {
			RasDisconnect();
		}
	}
#endif
#ifdef ENABLE_WIFI
	if (op.WiFiCheckEndDisCon == 1) {
		if (DoTimer == FALSE || op.WiFiCheckEndDisConTimeout==0
			|| TimedMessageBox(hWnd, STR_Q_WIFIDISCON, WINDOW_TITLE, MB_YESNO, op.WiFiCheckEndDisConTimeout) != IDNO) {
			WiFiDisconnect((op.WiFiCheckEndDisConTimeout>0) ? TRUE : FALSE);
		}
	}
#endif
	SetMailMenu(hWnd);
	SetTrayIcon(hWnd, (NewMail_Flag == TRUE) ? TrayIcon_Mail : TrayIcon_Main);

	if (EndThreadSortFlag == TRUE) {
		//Sort
		SwitchCursor(FALSE);
		if (op.LvThreadView == 1) {
			item_create_thread(MailBox + SelBox);
			ListView_SortItems(mListView, CompareFunc, SORT_THREAD + 1);
		} else if (op.LvAutoSort == 2) {
			ListView_SortItems(mListView, CompareFunc, op.LvSortItem);

			i = ListView_GetNextUnreadItem(mListView, -1, ListView_GetItemCount(mListView));
			if (i != -1) {
				int st = LVIS_FOCUSED;
				if (op.PreviewPaneHeight <= 0 || op.AutoPreview) {
					st |= LVIS_SELECTED;
				}
				//The not yet opening mail is selected the
				ListView_SetItemState(mListView, -1, 0, LVIS_SELECTED);
				ListView_SetItemState(mListView, i, st, st);
			}

		}
		ListView_EnsureVisible(mListView,
			ListView_GetNextItem(mListView, -1, LVNI_FOCUSED), TRUE);
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
 * CheckEndAutoExec - �`�F�b�N��̎������s
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
		ExecFlag = FALSE;		// FALSE �ɂ��邱�ƂŃ`�F�b�N�Ǝ��s�̃��[�v�������
		KeyShowHeader = FALSE;
		CheckBox = MAILBOX_USER - 1;

		gSockFlag = TRUE;
		SetTimer(hWnd, ID_EXEC_TIMER, CHECKTIME, NULL);
	} else {
		ServerDelete = FALSE;
		if (item_get_next_delete_mark((MailBox + SocBox), TRUE, -1, NULL) != -1) {
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
			item_get_next_send_mark((MailBox + SocBox), FALSE, TRUE) == -1) {
			return FALSE;
		}
		AutoCheckFlag = FALSE;
		AllCheck = FALSE;
		ExecFlag = FALSE;		// FALSE �ɂ��邱�ƂŃ`�F�b�N�Ǝ��s�̃��[�v�������
		KeyShowHeader = FALSE;
		ExecItem(hWnd, SocBox);
	}
	return TRUE;
}

/*
 * Init_NewMailFlag - �V�����[���t���O�̏�����
 */
static void Init_NewMailFlag(HWND hWnd)
{
	int i;

	if (ShowMsgFlag == TRUE || op.ClearNewOverlay != 1) {
		return;
	}

	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->NewMail > 0) {
			(MailBox + i)->NewMail = 0;
			// GJC - remove * from drop-down list (replace with ~ perhaps)
			SetMailboxMark(i, STATUS_DONE, FALSE);
		}
	}
	SetWindowText(MainWnd, WINDOW_TITLE);

	SelectMBMenu(SelBox);
}

/*
 * SetUnreadCntTitle - count of mailboxes that have new mail
 */
void SetUnreadCntTitle(BOOL CheckMsgs)
{
	TCHAR wbuf[BUF_SIZE];
	int i, j;
	int UnreadMailBox = 0;

	j = GetSelectedMBMenu();
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->NewMail > 0) {
			// GJC - check if there still is new mail; if not, update drop-down list
			if (CheckMsgs == TRUE && (MailBox + i)->Loaded && item_get_next_new((MailBox + i), -1, NULL) == -1) {
				(MailBox + i)->NewMail = 0;
			} else {
				UnreadMailBox++;
			}
			SetMailboxMark(i, STATUS_DONE, FALSE);
		}
	}
	SelectMBMenu(j);

	//���ǃA�J�E���g�����^�C�g���o�[�ɐݒ�
	if (UnreadMailBox == 0) {
		SetWindowText(MainWnd, WINDOW_TITLE);
		if (NewMail_Flag == TRUE) {
			NewMail_Flag = FALSE; // no more new messages
			SendMessage(MainWnd, WM_INITTRAYICON, 0, 0);
		}
	} else {
		wsprintf(wbuf, STR_TITLE_NEWMAILBOX, WINDOW_TITLE, UnreadMailBox);
		SetWindowText(MainWnd, wbuf);
	}
}

/*
 * NewMail_Message - �V�����[���`�F�b�N���ʂ̃��b�Z�[�W
 */
static void NewMail_Message(HWND hWnd, int cnt)
{
	int i, j;

	if (cnt == -1) {
		return;
	}
	if (cnt == 0) {
		if (ShowError == FALSE && op.ShowNoMailMessage == 1 && ShowMsgFlag == FALSE &&
			AutoCheckFlag == FALSE) {
			// ���b�Z�[�W�{�b�N�X�̕\��
			MessageBox(hWnd, STR_MSG_NONEWMAIL, WINDOW_TITLE,
				MB_ICONINFORMATION | MB_OK | MB_SETFOREGROUND);
		}
		SetUnreadCntTitle(TRUE);
		if (gCheckAndQuit == TRUE) {
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_QUIT, 0);
		}
		return;
	}

	if (gCheckAndQuit == TRUE && op.NewMailSound == 0 && op.ShowNewMailMessage == 0) {
		SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_QUIT, 0);
	}

	NewMail_Flag = TRUE;
	SetTrayIcon(hWnd, TrayIcon_Mail);

	// There is a new arrival in the message box; add the "* " in the drop-down combo
	j = GetSelectedMBMenu();
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->NewMail == 0 || (MailBox + i)->Loaded == FALSE ||
			((op.ListGetLine > 0 || op.ShowHeader == 1 || op.ListDownload == 1) &&
			mailbox_unread_check(i, FALSE) == FALSE)) {
			continue;
		}
		SetMailboxMark(i, STATUS_DONE, FALSE);
		if ((MailBox + i)->NewMailSoundFile && *(MailBox + i)->NewMailSoundFile != TEXT('\0')) {
			sndPlaySound((MailBox + i)->NewMailSoundFile, SND_ASYNC | SND_NODEFAULT);
		}
	}
	SelectMBMenu(j);

	SetUnreadCntTitle(FALSE);

	//Index of mailbox of new arrival acquisition
	for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
		if ((MailBox + i)->NewMail > 0) {
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
	// ���b�Z�[�W��\�����Ȃ��ݒ肩���ݕ\������Ă��郁�[���{�b�N�X�̏ꍇ�̓��b�Z�[�W�{�b�N�X���o���Ȃ�
	if (ShowError == TRUE || op.ShowNewMailMessage == 0 || ShowMsgFlag == TRUE ||
		(AutoCheckFlag == FALSE && hWnd == GetForegroundWindow() && i == SelBox)) {
		return;
	}
	SendMessage(MsgWnd,	WM_SHOWDIALOG, 0, 0);
#endif
}

/*
 * SetMailboxMark - set mailbox indicator (GJC)
 */
void SetMailboxMark(int Box, int Status, BOOL Add)
{
	MAILBOX *tpMailBox = MailBox + Box;
	TCHAR *p;
	if (Box < 0) {
		return;
	}
	if (Add == FALSE) {
		DeleteMBMenu(Box);
	}
	p = (tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0'))
		? STR_MAILBOX_NONAME : tpMailBox->Name;
	if (Status == STATUS_DONE && tpMailBox->NewMail == 0 && tpMailBox->FlagCount == 0) {
		if (Add == TRUE) {
			AddMBMenu(p);
		} else {
			InsertMBMenu(Box, p);
		}
	} else {
		TCHAR *q = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(p) + 3));
		if (q != NULL) {
			TCHAR *r = TEXT("");
#ifdef _WIN32_WCE_PPC
			if (Status == STATUS_DONE) {
				if (tpMailBox->NewMail > 0) {
					r = TEXT("* ");
				} else if (tpMailBox->FlagCount > 0) {
					r = TEXT("~ ");
				}
			} else {
				r = (Status == STATUS_ERROR) ? TEXT("# ") : TEXT("> ");
			}
			str_join_t(q, r, p, (TCHAR *)-1);
#else
			if (Status == STATUS_DONE) {
				if (tpMailBox->NewMail > 0) {
					r = TEXT(" *");
				} else if (tpMailBox->FlagCount > 0) {
					r = TEXT(" ~");
				}
			} else {
				r = (Status == STATUS_ERROR) ? TEXT(" #") : TEXT(" <");
			}
			str_join_t(q, p, r, (TCHAR *)-1);
#endif
			if (Add == TRUE) {
				AddMBMenu(q);
			} else {
				InsertMBMenu(Box, q);
			}
			mem_free(&q);
		}
	}
	if (SelBox == Box) {
		SelectMBMenu(SelBox);
	}
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
			((tpMailBox->ListSaveMode != 0) || (tpMailBox->Type == MAILBOX_TYPE_SAVE))) {
			if (tpMailBox->Filename == NULL) {
				wsprintf(buf, TEXT("MailBox%d.dat"), i - MAILBOX_USER);
			} else {
				lstrcpy(buf, tpMailBox->Filename);
			}
			file_save_mailbox(buf, DataDir, i, FALSE, TRUE,
				(tpMailBox->Type == MAILBOX_TYPE_SAVE) ? 2 : tpMailBox->ListSaveMode);
			DidOne = TRUE;
		}
	}
	if (DidOne) {
		ini_save_setting(hWnd, FALSE, FALSE, NULL);
	}
	SwitchCursor(TRUE);
}

/*
 * AdvOptionEditor - text edit of global options section of INI file
 */
static BOOL AdvOptionEditor(HWND hWnd)
{
	BOOL ret = FALSE;
	TCHAR *buf;

	if (profile_create() == FALSE) {
		ErrorMessage(hWnd, STR_ERR_MEMALLOC);
		return FALSE;
	}
	ini_write_general();
	ret = profile_flush(NULL, &buf);
	profile_free();
	if (ret) {
		ret = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_OPTION_EDITOR), hWnd,
				AdvOptionProc, (LPARAM)&buf);
	}
	if (ret && profile_create() != FALSE) {
		HDC hdc;
		int oldmbw = op.MBMenuWidth, oldpph = op.PreviewPaneHeight;
		ini_free(FALSE);
		profile_parse(buf, lstrlen(buf), TRUE, NULL);
		ini_read_general(hWnd);
		if (hViewFont != NULL) {
			DeleteObject(hViewFont);
		}
		hdc = GetDC(hWnd);
		if ((op.lv_font.name != NULL && *op.lv_font.name != TEXT('\0')) || op.lv_font.size != 9) {
			if (hListFont != NULL) {
				DeleteObject(hListFont);
			}
			hListFont = font_create_or_copy(hWnd, hdc, &op.lv_font);
		}
		hViewFont = font_create_or_copy(hWnd, hdc, &op.view_font);
		ReleaseDC(hWnd, hdc);
		if (hViewWnd != NULL) {
			SendDlgItemMessage(hViewWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
		}
#ifndef _WIN32_WCE
		EnumWindows((WNDENUMPROC)enum_windows_proc, 0);
#endif
		font_charset = op.view_font.charset;
		if (oldpph != op.PreviewPaneHeight && (oldpph > 0 || op.PreviewPaneHeight > 0)) {
			int newpph = op.PreviewPaneHeight;
			if (oldpph > 0) {
				op.PreviewPaneHeight = oldpph;
				if (newpph > 0) {
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_PREVPANE, 0);
					op.PreviewPaneHeight = -newpph;
				}
			} else {
				op.PreviewPaneHeight = -op.PreviewPaneHeight;
			}
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_PREVPANE, 0);
			if (newpph < 0) {
				op.PreviewPaneHeight = newpph;
			}
		}
		if (oldmbw != op.MBMenuWidth && (oldmbw > 0 || op.MBMenuWidth > 0)) {
			int newmbw = op.MBMenuWidth;
			if (oldmbw > 0) {
				op.MBMenuWidth = oldmbw;
				if (newmbw > 0) {
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_MBOXPANE, 0);
					op.MBMenuWidth = -newmbw;
				}
			} else {
				op.MBMenuWidth = -op.MBMenuWidth;
			}
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_MBOXPANE, 0);
			if (newmbw < 0) {
				op.MBMenuWidth = newmbw;
			}
		}
		mailbox_select(hWnd, SelBox);
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
	}
	mem_free(&buf);
	profile_free();

	return ret;
}

/*
 * WndProc - Message handler for main window
 */
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	static BOOL SipFlag = FALSE;
#endif
	static BOOL save_flag = FALSE;
	int i, j, old_selbox, ans;
	BOOL ret;

	switch (msg) {
	case WM_CREATE:
		MainWnd = hWnd;
		SwitchCursor(FALSE);

#ifdef _WIN32_WCE_LAGENDA
		GetClientRect(hWnd, &wnd_size);
#endif
		save_flag = TRUE;
		// ���[���{�b�N�X�̏�����
		if (mailbox_init() == FALSE) {
			ErrorMessage(NULL, STR_ERR_INIT);
			DestroyWindow(hWnd);
			break;
		}

		// read INI settings (op.???) and load mailboxes
		if (ini_read_setting(hWnd) == FALSE) {
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

		SwitchCursor(TRUE);

		if (first_start == TRUE) {
			ShowWindow(hWnd, SW_SHOW);
			_SetForegroundWindow(hWnd);
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_STARTCONFIG), hWnd,
				StartConfigProc, 0) == FALSE) {
				first_start = -1;
				save_flag = TRUE;
				DestroyWindow(hWnd);
				break;
			}
			(MailBox+SelBox)->NewMail = 1; // hack to force correct name into IDC_MBMENU
			SetMailBoxOption(hWnd, TRUE);
			ini_save_setting(hWnd, FALSE, FALSE, NULL);
		}

		//of control inside window Setting
#ifdef LOAD_USER_IMAGES
		{
			TCHAR fpath[BUF_SIZE];
			wsprintf(fpath, TEXT("%sResource\\ico_nchk.ico"), AppDir);
			TrayIcon_Main = LoadImage(NULL, fpath, IMAGE_ICON, SICONSIZE, SICONSIZE, LR_LOADFROMFILE);
			wsprintf(fpath, TEXT("%sResource\\icon_chk.ico"), AppDir);
			TrayIcon_Check = LoadImage(NULL, fpath, IMAGE_ICON, SICONSIZE, SICONSIZE, LR_LOADFROMFILE);
			wsprintf(fpath, TEXT("%sResource\\ico_main.ico"), AppDir);
			TrayIcon_Mail = LoadImage(NULL, fpath, IMAGE_ICON, SICONSIZE, SICONSIZE, LR_LOADFROMFILE);
		}
#endif
		if (!TrayIcon_Main)
			TrayIcon_Main = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_NOCHECK),
				IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
		if (!TrayIcon_Check)
			TrayIcon_Check = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_CHECK),
				IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
		if (!TrayIcon_Mail)
			TrayIcon_Mail = LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_MAIN),
				IMAGE_ICON, SICONSIZE, SICONSIZE, 0);
		if (op.ShowTrayIcon == 1 && TrayIcon_Main != NULL) {
			TrayMessage(hWnd, NIM_ADD, TRAY_ID, TrayIcon_Main);
		}

#ifdef ENABLE_WIFI
		if (op.WiFiCon) {
			// check network connections at start-up
			WiFiStatus = GetNetworkStatus(TRUE);
		}
#endif

		if (first_start == TRUE) {
			break;
		}

		// timer for auto-check at start-up
		if (op.AutoCheck == 1) {
			SetTimer(hWnd, ID_AUTOCHECK_TIMER, AUTOCHECKTIME, NULL);
		}

		// �N�����`�F�b�N�̊J�n
		if (op.StartCheck == 1 && gAutoSend == FALSE) {
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
			gAutoSend = FALSE;
		}
		break;

	case WM_COPYDATA:
#ifdef _WIN32_WCE
		if (CloseEditViewWindows(0) == FALSE) {
			break;
		}
		FocusWnd = hWnd;
		ShowWindow(hWnd, SW_SHOW);
		if (CommandLine(hWnd, ((PCOPYDATASTRUCT)lParam)->lpData) == TRUE) {
			int ret;
			_SetForegroundWindow(hWnd);
			ret = Edit_MailToSet(hInst, hWnd, ((PCOPYDATASTRUCT)lParam)->lpData, -1);
			if (ret == EDIT_INSIDEEDIT) {
				ShowWindow(hWnd, SW_HIDE);
			} else if (ret == EDIT_SEND) {
				SendMessage(hEditWnd, WM_COMMAND, ID_MENUITEM_SEND, 0);
				gAutoSend = FALSE;
			}
		}
#else
		if (op.ShowPass == 1 &&
			(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
			op.Password != NULL && *op.Password != TEXT('\0') &&
			ConfirmPass(hWnd, op.Password, TRUE) == FALSE) {
			break;
		}
		if (CommandLine(hWnd, ((PCOPYDATASTRUCT)lParam)->lpData) == TRUE) {
			int ret = Edit_MailToSet(hInst, hWnd, ((PCOPYDATASTRUCT)lParam)->lpData, -1);
			if (ret == EDIT_SEND) {
				SendMessage(hEditWnd, WM_COMMAND, ID_MENUITEM_SEND, 0);
				gAutoSend = FALSE;
			}
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
			CloseEditViewWindows(0);
			FocusWnd = hWnd;
			ShowWindow(hWnd, SW_SHOW);
			_SetForegroundWindow(hWnd);
			if (CommandLine(hWnd, buf) == TRUE) {
				int ret = Edit_MailToSet(hInst, hWnd, buf, -1);
				if (ret == EDIT_INSIDEEDIT) {
					ShowWindow(hWnd, SW_HIDE);
				} else if (ret == EDIT_SEND) {
					SendMessage(hEditWnd, WM_COMMAND, ID_MENUITEM_SEND, 0);
				}
			}
		}
		break;
#endif

#ifdef _WIN32_WCE_PPC
	case WM_SETTINGCHANGE:
		if (gDoingQuit == TRUE && GetForegroundWindow() != hWnd) {
			ShowWindow(hWnd, SW_SHOW);
			_SetForegroundWindow(hWnd);
			break;
		}
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SHACTIVATEINFO sai;

			memset(&sai, 0, sizeof(SHACTIVATEINFO));
			SHHandleWMSettingChange(hWnd, -1, 0, &sai);
			SipFlag = sai.fSipUp;
			SetWindowSize(hWnd, 0, 0);

			ListView_EnsureVisible(mListView, ListView_GetNextItem(mListView, -1, LVNI_FOCUSED), TRUE);
		}
		if (LastXSize != GetSystemMetrics(SM_CXSCREEN)) {
			SetMailMenu(hWnd);
			LastXSize = GetSystemMetrics(SM_CXSCREEN);
		}
		break;
#elif defined _WIN32_WCE_LAGENDA
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SipFlag = SetWindowSize(hWnd, 0, 0);

			ListView_EnsureVisible(mListView, ListView_GetNextItem(mListView, -1, LVNI_FOCUSED), TRUE);
		}
		break;
#endif

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) {
#ifdef _WIN32_WCE_PPC
			if (op.AutoSave != 0) {
				// could also unload mailboxes here
				SaveWindow(hWnd, FALSE, FALSE, FALSE);
			}
#endif
#ifndef _WIN32_WCE
			confirm_flag = 1;
#endif
			if (op.ShowTrayIcon == 1 && op.MinsizeHide == 1) {
				ShowWindow(hWnd, SW_HIDE);
				return 0;
			}
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
		CloseEditViewWindows(1);
		FocusWnd = hWnd;
		break;
#endif

#ifdef _WIN32_WCE_PPC
	case WM_HOTKEY:
		// code courtesy of Christian Ghisler
		if (op.osMajorVer >= 5 && LOWORD(lParam)==0) {
			HWND submenu;
			RECT r;
			POINT pt;
			int itemopen;

			switch(HIWORD(lParam)) {
			case VK_F1: // VK_TSOFT1
			case VK_F2: // VK_TSOFT2
				if (MainMenuOpened) {
					itemopen = MainMenuOpened - 1;
				} else {
					itemopen = (HIWORD(lParam)==VK_F1) ? 0 : 1;
				}
				SendMessage(hMainToolBar, TB_GETITEMRECT, itemopen, (LPARAM)&r);
				pt.x = (r.left + r.right) / 2;
				pt.y = (r.top + r.bottom) / 2;
				submenu = GetWindow(hMainToolBar, GW_CHILD);
				MainMenuOpened = 0;
				PostMessage(submenu, WM_LBUTTONDOWN, 1, MAKELONG(pt.x,pt.y));
				PostMessage(submenu, WM_LBUTTONUP, 1, MAKELONG(pt.x,pt.y));
				break;
			}
		}
		break;
	case WM_EXITMENULOOP:
		MainMenuOpened = 0;
		break;
	case WM_INITMENUPOPUP:
		// try to enable an item on the menu to see which one is visible
		if (EnableMenuItem((HMENU)wParam, ID_MENUITEM_SELMODE, MF_BYCOMMAND | MF_ENABLED) != 0xFFFFFFFF)
			MainMenuOpened = 2;
		else if (EnableMenuItem((HMENU)wParam, ID_MENUITEM_NEWMAIL, MF_BYCOMMAND | MF_ENABLED) != 0xFFFFFFFF)
			MainMenuOpened = 1;
		break;
#endif

	case WM_CLOSE:
#ifdef _WIN32_WCE_PPC
		// some task managers send ctrl+q then wm_close in succession
		if (gDoingQuit == TRUE) {
			break;
		}
#endif
		if (op.ShowTrayIcon == 1 && op.CloseHide == 1) {
			ShowWindow(hWnd, SW_HIDE);
#ifdef _WIN32_WCE
			CloseEditViewWindows(1);
#endif
			FocusWnd = hWnd;
#ifdef _WIN32_WCE_LAGENDA
			ShowWindow(hWnd, SW_SHOW);
#endif
			break;
		}
		if (op.CheckQueuedOnExit > 0 && gAutoSend == FALSE
			&& item_get_next_send_mark((MailBox + MAILBOX_SEND), (op.CheckQueuedOnExit == 2), FALSE) != -1) {
#ifdef _WIN32_WCE_PPC
			// watch out for quit messages from task managers
			MSG msg;
			PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
#endif
			_SetForegroundWindow(hWnd);
			if (MessageBox(hWnd, STR_Q_QUEUEDMAIL_EXIT, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
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
		SetFocus(mListView);
		break;

	case WM_INITTRAYICON:
		if (g_soc == -1) {
			SetTrayIcon(hWnd, (NewMail_Flag == TRUE) ? TrayIcon_Mail : TrayIcon_Main);
		}
		break;

	case WM_TIMER:
		switch (wParam) {
#ifndef WSAASYNC
		// ��M����
		case ID_RECV_TIMER:
			if (g_soc == -1) {
				KillTimer(hWnd, wParam);
				if (AllCheck == FALSE) {
					if (op.CheckEndExec == 1 &&
						CheckEndAutoExec(hWnd, RecvBox, NewMailCnt, FALSE) == TRUE) {
						//After the checking execution
						break;
					}
					if (ExecFlag && CheckAfterThisUpdate && RecvBox != MAILBOX_SEND) {
						// ���s��`�F�b�N
						ExecFlag = FALSE;
						RecvMailList(hWnd, RecvBox, FALSE);
						break;
					}
					SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
					RecvBox = -1;
					EndSocketFunc(hWnd, TRUE);
					AutoSave_Mailboxes(hWnd);
					NewMail_Message(hWnd, NewMailCnt);
				} else {
					SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
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
					if (op.SocLog > 0) log_flush();
					KillTimer(hWnd, ID_TIMEOUT_TIMER);
					SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
					SetItemCntStatusText(NULL, FALSE, TRUE);
					SetUnreadCntTitle(TRUE);
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
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
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
			if (SendMail(hWnd, (MAILITEM *)wkSendMailItem, SMTP_SENDEND) == FALSE) {
				if (gAutoSend == AUTOSEND_AND_QUIT && op.SendIgnoreError == 1) {
					PostMessage(hWnd, WM_COMMAND, ID_MENUITEM_QUIT, 0);
				}
			}
			wkSendMailItem = NULL;
			break;

		//of mail Transmission main.
		case ID_SMTP_TIMER:
			if (g_soc != -1) {
				break;
			}
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
			if (SmtpWait > 0) {
				SmtpWait--;
				break;
			}
			if (CheckBox >= MAILBOX_USER) {
				int next = item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox);
				if (next != -1) {
					// ���[���̑��M
					ret = SendMail(hWnd, *((MailBox + MAILBOX_SEND)->tpMailItem + next), SMTP_NEXTSEND);
					if (ret == TRUE) {
						break;
					}
				}
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
#ifdef ENABLE_RAS
				if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
					ErrorSocketEnd(hWnd, CheckBox);
					if (op.SocLog > 0) log_flush();
					SetMailMenu(hWnd);
					break;
				}
#endif
#ifdef ENABLE_WIFI
				if (op.WiFiCon == 1 && SendMessage(hWnd, WM_WIFI_START, CheckBox, 0) == FALSE) {
					ErrorSocketEnd(hWnd, CheckBox);
					if (op.SocLog > 0) log_flush();
					SetMailMenu(hWnd);
					break;
				}
#endif
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
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
			if (op.AutoSave == 2) {
				AutoSave_Mailboxes(hWnd);
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
#ifdef ENABLE_RAS
			//of the setting which Dial rise start
			if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
				ErrorSocketEnd(hWnd, CheckBox);
				if (op.SocLog > 0) log_flush();
				SetMailMenu(hWnd);
				break;
			}
#endif
#ifdef ENABLE_WIFI
			// WiFi connection
			if (op.WiFiCon == 1 && SendMessage(hWnd, WM_WIFI_START, CheckBox, 0) == FALSE) {
				ErrorSocketEnd(hWnd, CheckBox);
				if (op.SocLog > 0) log_flush();
				SetMailMenu(hWnd);
				break;
			}
#endif
#ifndef _WCE_OLD
			if (op.SocLog > 1) {
				char msg[50];
				sprintf_s(msg, 50, "CheckTimer: box=%d%s", CheckBox, "\r\n");
				log_save_a(msg);
			}
#endif
			//Mail reception start
			RecvMailList(hWnd, CheckBox, FALSE);
			break;

		//Round execution
		case ID_EXEC_TIMER:
			if (g_soc != -1) {
				break;
			}
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
			if (SmtpWait > 0) {
				SmtpWait--;
				break;
			}
			if (CheckBox >= MAILBOX_USER && (MailBox + CheckBox)->CyclicFlag == 0) {
				int next = item_get_next_send_mark_mailbox((MailBox + MAILBOX_SEND), -1, CheckBox);
				if (next != -1) {
					//Execution of transmission mail (POP before SMTP)
					ret = SendMail(hWnd, *((MailBox + MAILBOX_SEND)->tpMailItem +	next), SMTP_NEXTSEND);
					if (ret == TRUE) {
						break;
					}
				}
			}
			if (CheckBox >= MAILBOX_USER && (MailBox + CheckBox)->CyclicFlag == 0 &&
				CheckAfterThisUpdate && ExecCheckFlag == FALSE) {
				// ���s��`�F�b�N
				RecvMailList(hWnd, CheckBox, FALSE);
				ExecCheckFlag = TRUE;
				break;
			}
			ExecCheckFlag = FALSE;
			if (op.AutoSave == 2) {
				AutoSave_Mailboxes(hWnd);
			}

			CheckBox++;
			if (CheckBox >= MailBoxCnt) {
				KillTimer(hWnd, wParam);
				if (item_get_next_send_mark((MailBox + MAILBOX_SEND), FALSE, TRUE) != -1) {
					// ���M
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
				item_get_next_delete_mark((MailBox + CheckBox), TRUE, -1, NULL) != -1 ||
				op.CheckAfterUpdate == 1) {
#ifdef ENABLE_RAS
				if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
					ErrorSocketEnd(hWnd, CheckBox);
					if (op.SocLog > 0) log_flush();
					SetMailMenu(hWnd);
					break;
				}
#endif
#ifdef ENABLE_WIFI
				if (op.WiFiCon == 1 && SendMessage(hWnd, WM_WIFI_START, CheckBox, 0) == FALSE) {
					ErrorSocketEnd(hWnd, CheckBox);
					if (op.SocLog > 0) log_flush();
					SetMailMenu(hWnd);
					break;
				}
#endif
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
#ifdef ENABLE_RAS
			if (op.EnableLAN == 0 && op.RasCon == 0 && op.RasNoCheck == 1 && !GetRasStatus()) {
				break;
			}
#endif
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
			if (op.EnableLAN == 0 && op.WiFiCon == 0 && op.WiFiNoCheck == 1 && !GetNetworkStatus(TRUE)) {
				break;
			}
#endif
			if (op.SocLog > 1) log_save_a("Auto check\r\n");
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

			//When selection changes SetMailMenu once
		case ID_ITEMCHANGE_TIMER:
			KillTimer(hWnd, wParam);
			SetMailMenu(hWnd);
			break;

		case ID_PREVIEW_TIMER:
			KillTimer(hWnd, wParam);
			if (SelBox != MAILBOX_SEND) {
				SetMailStats(hWnd, ICON_READ);
			}
			break;

		//When starting waiting
		case ID_NEWMAIL_TIMER:
			KillTimer(hWnd, wParam);
			if (InitialAccount != NULL) {
				int i = mailbox_name_to_index(InitialAccount, MAILBOX_TYPE_ACCOUNT);
				if (i == -1) {
					i = mailbox_name_to_index(InitialAccount, MAILBOX_TYPE_SAVE);
				}
				mailbox_select(hWnd, i);
				mem_free(&InitialAccount);
				InitialAccount = NULL;
			}
			if (CmdLine != NULL) {
#ifdef _WIN32_WCE
				if (CommandLine(hWnd, CmdLine) == TRUE) {
					int ret = Edit_MailToSet(hInst, hWnd, CmdLine, -1);
					if (ret == EDIT_INSIDEEDIT || ret == EDIT_SEND) {
						ShowWindow(hWnd, SW_HIDE);
						if (gAutoSend) {
							SendMessage(hEditWnd, WM_COMMAND, ID_MENUITEM_SEND, 0);
						}
					}
				}
#else
				if (CommandLine(hWnd, CmdLine) == TRUE) { 
					int ret = Edit_MailToSet(hInst, hWnd, CmdLine, -1);
					if (ret == EDIT_SEND && gAutoSend && hEditWnd != NULL) {
						SendMessage(hEditWnd, WM_COMMAND, ID_MENUITEM_SEND, 0);
					}
				}
#endif
				mem_free(&CmdLine);
				CmdLine = NULL;
			}
			break;

#ifdef ENABLE_RAS
		//ras Change
		case ID_RASWAIT_TIMER:
			KillTimer(hWnd, wParam);
			if (hEvent != NULL) {
				SetEvent(hEvent);
			}
			break;
#endif
#ifdef ENABLE_WIFI
		//wifi Change
		case ID_WIFIWAIT_TIMER:
			KillTimer(hWnd, wParam);
			if (hEvent != NULL) {
				SetEvent(hEvent);
			}
			break;
		case ID_WIFICHECK_TIMER:
			i = GetNetworkStatus(FALSE);
			if (op.SocLog > 1) {
				TCHAR msg[MSG_SIZE];
				wsprintf(msg, TEXT("WiFiCheck timer, status=%d, was %d\r\n"), i, WiFiStatus);
				log_save(msg);
			}
			if (i != WiFiStatus) {
				WiFiStatus = i;
				if (WiFiStatus == TRUE) {
					SetStatusTextT(MainWnd, STR_STATUS_NET_CONNECT, 1);
				} else {
					SetStatusTextT(MainWnd, STR_STATUS_NET_DROPPED, 1);
				}
			}
			break;
#endif
		} // WM_TIMER switch
		break;

	case WM_COMMAND: {
		BOOL mark_del = FALSE;
		HWND fWnd;
		int sort_val = 0;
		int menu_flag = 0;
		int mbox, command_id = GET_WM_COMMAND_ID(wParam, lParam);
		switch (command_id) {
		//of message compilation
		case IDC_MBMENU:
			if (op.MBMenuWidth <= 0 && HIWORD(wParam) == CBN_CLOSEUP) {
				if (SelBox != GetSelectedMBMenu()) {
					mailbox_select(hWnd, GetSelectedMBMenu());	
				}
				SetFocus(mListView);
				SwitchCursor(TRUE);
			} else if (op.MBMenuWidth > 0 && HIWORD(wParam) == LBN_SELCHANGE) {
				mailbox_select(hWnd, GetSelectedMBMenu());
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

		// Toggle focus between combo and the list view
		case ID_KEY_TAB:
			fWnd = GetFocus();
			if (fWnd == mListView) {
				if (op.PreviewPaneHeight > 0) {
					SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));
				} else {
					SetFocus(GetDlgItem(hWnd, IDC_MBMENU));
				}
			} else if (fWnd == GetDlgItem(hWnd, IDC_EDIT_BODY)) {
				SetFocus(GetDlgItem(hWnd, IDC_MBMENU));
			} else {
				SetFocus(mListView);
				if (SelBox != GetSelectedMBMenu()) {
					mailbox_select(hWnd, GetSelectedMBMenu());
				}
			}
			break;

		//In position of selective item pop rise menu indicatory
		case ID_KEY_SHOWMENU:
		case ID_KEY_ESC:
			menu_flag = 1;
			if (IsWindowVisible(hWnd) == 0 ||
#ifndef _WIN32_WCE
				IsIconic(hWnd) != 0 ||
#endif
				GetForegroundWindow() != hWnd) {
				break;
			}
			if (GetFocus() == GetDlgItem(hWnd, IDC_MBMENU)) {
				if (op.MBMenuWidth > 0) {
					ShowMenu(hWnd, hMBPOPUP, 0, 4, FALSE);
				} else {
					if (GetDroppedStateMBMenu() == FALSE) {
						SetFocus(mListView);
					} else {
						DropMBMenu(TRUE);
					}
				}
				break;
			}
			SetFocus(mListView);
			// and fall through to post the menu
			
		//In position of mouse pop rise menu indicatory
		case ID_MENU:
			if (op.ContextMenuOption == 0) {
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
				ShowMenu(hWnd, SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), 0, menu_flag, FALSE);
#elif defined(_WIN32_WCE_LAGENDA)
				ShowMenu(hWnd, hMainMenu, MailMenuPos, menu_flag, FALSE);
#else
				ShowMenu(hWnd, CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), MailMenuPos, menu_flag, FALSE);
#endif
#else
				ShowMenu(hWnd, GetMenu(hWnd), MailMenuPos, menu_flag, FALSE);
#endif
			} else {
				ShowMenu(hWnd, hMainPop, (SelBox==MAILBOX_SEND) ? 1 : 0, menu_flag, FALSE);
			}
			break;

		//====== file =========
		//Compilation
		case ID_MENUITEM_NEWMAIL:
#ifndef _WIN32_WCE
			if (op.ShowPass == 1 &&
				(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
				op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password, TRUE) == FALSE) {
				break;
			}
#endif
			if (Edit_InitInstance(hInst, hWnd, SelBox, NULL, EDIT_NEW, NULL, FALSE) == EDIT_INSIDEEDIT) {
#ifdef _WIN32_WCE
				ShowWindow(hWnd, SW_HIDE);
#endif
			}
			break;

		// �A�h���X��
		case ID_MENUITEM_ADDRESS:
			{
				// GJC make temporary addressbook for editing
				ADDRESSBOOK *tpTmpAddressBook = addressbook_copy();
				if (tpTmpAddressBook != NULL) {
					tpTmpAddressBook->GetAddrList = FALSE;
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS),
						hWnd, AddressListProc, (LPARAM)tpTmpAddressBook);
					addressbook_free(tpTmpAddressBook);
				}
			}
			break;

		//Option
		case ID_MENUITEM_OPTION:
			{
				int key = GetKeyState(VK_SHIFT);
#ifndef _WIN32_WCE
				if (op.ConfigPass != 0 && op.Password != NULL && *op.Password != TEXT('\0') &&
					ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
					break;
				}
#endif
				if (((key < 0) 
						&& ParanoidMessageBox(hWnd, STR_Q_ADV_OPT, WINDOW_TITLE, MB_YESNO) == IDYES)
#ifdef _WIN32_WCE_PPC
					|| (op.PromptIniEdit
						&& MessageBox(hWnd, STR_Q_ADV_OPT, WINDOW_TITLE, MB_YESNO | MB_DEFBUTTON2) == IDYES)
#endif
						) {
					ret = AdvOptionEditor(hWnd);
				} else {
					ret = SetOption(hWnd);
				}
			}

			SwitchCursor(FALSE);
			// �����`�F�b�N�^�C�}�[�̐ݒ�
			if (op.AutoCheck == 1) {
				SetTimer(hWnd, ID_AUTOCHECK_TIMER, AUTOCHECKTIME, NULL);
			} else {
				KillTimer(hWnd, ID_AUTOCHECK_TIMER);
				AutoCheckCnt = 0;
			}
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
			CheckMenuItem(CommandBar_GetMenu(GetDlgItem(MainWnd, IDC_CB), 0), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
#else
			CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
			// �^�X�N�g���C�̃A�C�R���̐ݒ�
			if (op.ShowTrayIcon == 1) {
				SetTrayIcon(hWnd, (NewMail_Flag == TRUE) ? TrayIcon_Mail : TrayIcon_Main);
			} else {
				TrayMessage(hWnd, NIM_DELETE, TRAY_ID, NULL);
			}
			if (ret == TRUE && op.AutoSave != 0) {
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
			}
			SwitchCursor(TRUE);
			break;

#ifdef ENABLE_RAS
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
#endif
#ifdef ENABLE_WIFI
		case ID_MENUITEM_WIFI_CONNECT:
			if (g_soc != -1) {
				break;
			}
			if (WiFiLoop == TRUE) {
				break;
			}
			SendMessage(hWnd, WM_WIFI_START, SelBox, 0);
			break;

		case ID_MENUITEM_WIFI_DISCONNECT:
			WiFiDisconnect(TRUE);
			break;
#endif

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

		case ID_MENUITEM_SOCLOG:
			if (op.SocLog > 0) {
				ViewLogFile(hWnd);
			} else {
				if (MessageBox(hWnd, STR_Q_ENABLE_SOCLOG, WINDOW_TITLE, MB_YESNO) == IDYES) {
					op.SocLog = 3;
				}
			}
			break;

		case ID_MENUITEM_AUTOCHECK:
			op.AutoCheck = (op.AutoCheck == 0) ? 1 : 0;
			if (op.AutoCheck == 1) {
				SetTimer(hWnd, ID_AUTOCHECK_TIMER, AUTOCHECKTIME, NULL);
			} else {
				KillTimer(hWnd, ID_AUTOCHECK_TIMER);
				AutoCheckCnt = 0;
			}
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
			CheckMenuItem(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
#else
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
			break;

		//Version information
		case ID_MENUITEM_ABOUT:
#ifndef _WIN32_WCE
		case ID_MENUITEM_ABOUT_SSL:
#endif
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ABOUT), NULL, AboutBoxProc, command_id);
			break;

#ifndef _WIN32_WCE_PPC
		case ID_MENUITEM_VISIT_WEB:
			ShellOpen(STR_WEB_DOC);
			break;
#endif

		///////////// MRP /////////////////////
		case ID_MENUITEM_SAVEALL:
			SaveWindow(hWnd, FALSE, FALSE, TRUE);
			break;
		///////////// --- /////////////////////

		case ID_MENUITEM_BACKUP:
			if (g_soc == -1) {
				SaveWindow(hWnd, TRUE, FALSE, FALSE);
			}
			break;

		//End
		case ID_MENUITEM_QUIT:
			gDoingQuit = TRUE;
			if (op.CheckQueuedOnExit > 0 && gAutoSend == FALSE
				&& item_get_next_send_mark((MailBox + MAILBOX_SEND), (op.CheckQueuedOnExit == 2), FALSE) != -1) {
				if (MessageBox(hWnd, STR_Q_QUEUEDMAIL_EXIT, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
					mailbox_select(hWnd, MAILBOX_SEND);
					gDoingQuit = FALSE;
					break;
				}
			}
			if (SaveWindow(hWnd, FALSE, (gAutoSend == FALSE), FALSE) == FALSE) {
				gDoingQuit = FALSE;
				break;
			}
			save_flag = TRUE;
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
			SipFlag = FALSE;
#endif
			EndWindow(hWnd);
			gDoingQuit = FALSE;
			break;

#ifdef _WIN32_WCE_LAGENDA
		// ����
		case CSOBAR_ADORNMENTID_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
#endif
		// ====== �A�J�E���g =========
		// �A�J�E���g�̒ǉ�
		case ID_MENUITEM_ADDMAILBOX:
			if (MailBoxCnt >= MAX_MAILBOX_CNT) {
				ErrorMessage(hWnd, STR_ERR_TOOMANYMAILBOXES);
				break;
			}
#ifndef _WIN32_WCE
			if (op.ConfigPass == 1 && op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
				break;
			}
#endif
			old_selbox = SelBox;
			mailbox_select(hWnd, mailbox_create(hWnd, 1, old_selbox + 1, TRUE, TRUE));
			i = SetMailBoxType(hWnd, 0);
			(MailBox+SelBox)->NewMail = 1; // hack to force correct name into IDC_MBMENU
#ifndef _WIN32_WCE
			if (i == 0 && op.ConfigPass == 2 && op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
				i = -1;
			}
#endif
			if (i == -1 || (i == 0 && SetMailBoxOption(hWnd, TRUE) == FALSE)) {
				mailbox_delete(hWnd, SelBox, FALSE, FALSE);
				mailbox_select(hWnd, old_selbox);
				break;
			} else if (i == MAILBOX_TYPE_SAVE) {
				mailbox_select(hWnd, SelBox);
			} else if (i == MAILBOX_IMPORT_SAVE) {
				if (ImportSavebox(hWnd) == FALSE) {
					mailbox_delete(hWnd, SelBox, FALSE, FALSE);
					mailbox_select(hWnd, old_selbox);
					break;
				}
				(MailBox+SelBox)->Type = MAILBOX_TYPE_SAVE;
				(MailBox+SelBox)->NeedsSave = MAILITEMS_CHANGED;
				mailbox_select(hWnd, SelBox);
			}
			if (op.AutoSave != 0) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		// �A�J�E���g�̐ݒ�
		case ID_MENUITEM_SETMAILBOX:
#ifndef _WIN32_WCE
			if ((op.ConfigPass == 1 ||
				 (op.ConfigPass == 2 && (MailBox+SelBox)->Type != MAILBOX_TYPE_SAVE))
				&& op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
				break;
			}
#endif
			if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
				if (SetSaveBoxOption(hWnd) == FALSE) {
					break;
				}
			} else {
				if (SetMailBoxOption(hWnd, FALSE) == FALSE) {
					break;
				}
			}
			if (op.AutoSave != 0) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		case ID_MENUITEM_MAILBOXES:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAILBOXES), hWnd, MailBoxSummaryProc, 0);
			break;

		case ID_MENUITEM_NEXTMAIL:
		case ID_MENUITEM_PREVMAIL:
			if (op.PreviewPaneHeight > 0) {
				i = ListView_GetNextItem(mListView, -1, LVNI_SELECTED);
				ListView_SetItemState(mListView, i, 0, LVIS_SELECTED);
				i += (command_id == ID_MENUITEM_PREVMAIL) ? -1 : +1;
				ListView_SetItemState(mListView, i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			}
			break;

		case ID_MENUITEM_PREVPANE:
			{
				RECT rcRect;
				HMENU hMenu;
				int top, left, height, width;

				op.PreviewPaneHeight = -op.PreviewPaneHeight;
				top = op.ToolBarHeight;
				if (op.MBMenuWidth <= 0) {
					top += op.MBMenuHeight;
				}
				left = (op.MBMenuWidth > 0) ? op.MBMenuWidth : 0;

				GetWindowRect(mListView, &rcRect);
				width = rcRect.right - rcRect.left;
				// when op.PreviewPaneHeight < 0, the next line grows IDC_LISTVIEW
				height = rcRect.bottom - rcRect.top - op.PreviewPaneHeight;
				MoveWindow(mListView, left, top, width, height, TRUE);

				if (op.PreviewPaneHeight > 0) {
					CreatePreviewPane(hWnd, left, top+height, width, op.PreviewPaneHeight);
					i = ListView_GetSelectedCount(mListView);
					if (i == 1) {
						MAILITEM *tpMailItem;
						i = ListView_GetNextItem(mListView, -1, LVNI_SELECTED);
						tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, i);
						PopulatePreviewPane(hWnd, tpMailItem);
						ListView_EnsureVisible(mListView, i, TRUE);
					}
				} else {
					HWND previewWnd = GetDlgItem(hWnd, IDC_EDIT_BODY);
					DelPreviewSubClass(previewWnd);
					DestroyWindow(previewWnd);
				}

#ifdef _WIN32_WCE_PPC
				hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE);
#elif defined(_WIN32_WCE)
				hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0);
#else
				hMenu = GetMenu(hWnd);
#endif
				CheckMenuItem(hMenu, ID_MENUITEM_PREVPANE, (op.PreviewPaneHeight>0) ? MF_CHECKED : MF_UNCHECKED);
			}
			break;

		case ID_MENUITEM_MBOXPANE:
			{
				RECT rcRect;
				HMENU hMenu;
				int dTop, dBottom, mbh, height, width, left;
				op.MBMenuWidth = -op.MBMenuWidth;
				if (MBPaneWndProc != NULL) {
					SetWindowLong(GetDlgItem(hWnd, IDC_MBMENU), GWL_WNDPROC, (DWORD)MBPaneWndProc);
				}
				DestroyWindow(GetDlgItem(hWnd, IDC_MBMENU));
				dTop = op.ToolBarHeight;

				GetWindowRect(GetDlgItem(hWnd, IDC_STATUS), &rcRect);
				dBottom = rcRect.bottom - rcRect.top;

				GetWindowRect(mListView, &rcRect);
				// when op.MBMenuWidth < 0, the next line grows IDC_LISTVIEW
				op.PreviewPaneWidth = width = rcRect.right - rcRect.left - op.MBMenuWidth;

				mbh = CreateMBMenu(hWnd, dTop, dBottom);
				if (mbh > 0) {
					dTop += mbh;
					height = rcRect.bottom - rcRect.top - mbh;
				} else {
					height = op.MBMenuHeight;
					if (op.PreviewPaneHeight > 0) {
						height -= op.PreviewPaneHeight;
					}
				}
				SelectMBMenu(SelBox);

				left = (op.MBMenuWidth>0) ? op.MBMenuWidth : 0;
				if (op.PreviewPaneHeight > 0) {
					ResizingPreview = TRUE;
					MoveWindow(GetDlgItem(hWnd, IDC_EDIT_BODY),
						left, dTop+height, width, op.PreviewPaneHeight, TRUE);
					ResizingPreview = FALSE;
				}

				MoveWindow(mListView, left, dTop, width, height, TRUE);
#ifdef _WIN32_WCE_PPC
				hMenu = SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE);
#elif defined(_WIN32_WCE)
				hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_CB), 0);
#else
				hMenu = GetMenu(hWnd);
#endif
				CheckMenuItem(hMenu, ID_MENUITEM_MBOXPANE, (op.MBMenuWidth>0) ? MF_CHECKED : MF_UNCHECKED);
			}
			break;

		// save mailbox
		case ID_MENUITEM_SAVEMAILBOX:
			if (SelBox != RecvBox) {
				TCHAR buf[BUF_SIZE];
				MAILBOX *tpMailBox = MailBox + SelBox;
				if (tpMailBox->Filename == NULL) {
					wsprintf(buf, TEXT("MailBox%d.dat"), SelBox - MAILBOX_USER);
				} else {
					lstrcpy(buf, tpMailBox->Filename);
				}
				file_save_mailbox(buf, DataDir, SelBox, FALSE, FALSE,
					(tpMailBox->Type == MAILBOX_TYPE_SAVE) ? 2 : tpMailBox->ListSaveMode);
			}
			break;

		//of account Deletion
		case ID_MENUITEM_DELETEMAILBOX:
			if (SelBox == MAILBOX_SEND || SelBox == RecvBox) {
				break;
			}
#ifndef _WIN32_WCE
			if ((op.ConfigPass == 1 ||
				 (op.ConfigPass == 2 && (MailBox+SelBox)->Type != MAILBOX_TYPE_SAVE))
				&& op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
				break;
			}
#endif
			{
				TCHAR msg[MSG_SIZE];
				wsprintf(msg, STR_Q_DELMAILBOX, ((MailBox+SelBox)->Name) ? (MailBox+SelBox)->Name : STR_MAILBOX_NONAME);
				if (MessageBox(hWnd, msg, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
					break;
				}
			}
			if (op.LazyLoadMailboxes > 0) {
				// make sure SelBox-1 is loaded before deleting
				int DelBox = SelBox;
				mailbox_select(hWnd, SelBox-1);
				mailbox_delete(hWnd, DelBox, TRUE, FALSE);
			} else {
				mailbox_select(hWnd, mailbox_delete(hWnd, SelBox, TRUE, FALSE));
			}
			if (op.AutoSave != 0) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, TRUE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		//of account Account on portable
		case ID_MENUITEM_MOVEUPMAILBOX:
			if (mailbox_move_up(hWnd, TRUE) == TRUE && op.AutoSave != 0) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		//Account under portable
		case ID_MENUITEM_MOVEDOWNMAILBOX:
			if (mailbox_move_down(hWnd, TRUE) == TRUE && op.AutoSave != 0) {
				SwitchCursor(FALSE);
				ini_save_setting(hWnd, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

			// ListView column sorting from menu item
			// (can also click on column headers, handled elsewhere)
		case ID_MENUITEM_ICONSORT:
			sort_val = (ABS(LvSortFlag) == (SORT_ICON + 1)) ? (LvSortFlag * -1) : (SORT_ICON + 1);
#ifndef _WIN32_WCE
		case ID_MENUITEM_FILESORT:
			if (sort_val == 0) {
				sort_val = (ABS(LvSortFlag) == (SORT_NO + 1)) ? (LvSortFlag * -1) : (SORT_NO + 1);
			}
		case ID_MENUITEM_SUBJSORT:
			if (sort_val == 0) {
				sort_val = (ABS(LvSortFlag) == (SORT_SUBJ + 1)) ? (LvSortFlag * -1) : (SORT_SUBJ + 1);
			}
		case ID_MENUITEM_FROMSORT:
			if (sort_val == 0) {
				sort_val = (ABS(LvSortFlag) == (SORT_FROM + 1)) ? (LvSortFlag * -1) : (SORT_FROM + 1);
			}
		case ID_MENUITEM_DATESORT:
			if (sort_val == 0) {
				sort_val = (ABS(LvSortFlag) == (SORT_DATE + 1)) ? (LvSortFlag * -1) : (SORT_DATE + 1);
			}
		case ID_MENUITEM_SIZESORT:
			if (sort_val == 0) {
				sort_val = (ABS(LvSortFlag) == (SORT_SIZE + 1)) ? (LvSortFlag * -1) : (SORT_SIZE + 1);
				
			}
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_FILESORT, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_ICONSORT, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_SUBJSORT, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_FROMSORT, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_DATESORT, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), ID_MENUITEM_SIZESORT, MF_UNCHECKED);
			CheckMenuItem(GetMenu(hWnd), command_id, MF_CHECKED);
#endif
			op.LvThreadView = 0;
			LvSortFlag = sort_val;
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
			//Sort
			SwitchCursor(FALSE);
			ListView_SortItems(mListView, CompareFunc, LvSortFlag);
			SwitchCursor(TRUE);

			ListView_EnsureVisible(mListView, ListView_GetNextItem(mListView, -1, LVNI_FOCUSED), TRUE);

			if (op.LvAutoSort == 2 || (MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
				op.LvSortItem = LvSortFlag;
			}
			break;

		//Thread indicatory
		case ID_MENUITEM_THREADVIEW:
			SwitchCursor(FALSE);
			if (op.LvThreadView == 1) {
				op.LvThreadView = 0;
				ListView_SortItems(mListView, CompareFunc, LvSortFlag);
			} else {
				op.LvThreadView = 1;
				item_create_thread(MailBox + SelBox);
				ListView_SortItems(mListView, CompareFunc, SORT_THREAD + 1);
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
			ListViewSortMenuCheck(LvSortFlag);
#endif

			ListView_EnsureVisible(mListView, ListView_GetNextItem(mListView, -1, LVNI_FOCUSED), TRUE);
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
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
			i = SelBox;
			if (i < MAILBOX_USER || i >= MailBoxCnt || (MailBox+i)->Type != MAILBOX_TYPE_ACCOUNT) {
				break;
			}
			if (SaveBoxesLoaded == FALSE && op.BlindAppend == 0) {
				if (mailbox_load_now(hWnd, i, FALSE, TRUE) != 1) {
					break;
				}
			}
			AutoCheckFlag = FALSE;
#ifdef ENABLE_RAS
			if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, i, 0) == FALSE) {
				break;
			}
#endif
#ifdef ENABLE_WIFI
			if (op.WiFiCon == 1 && SendMessage(hWnd, WM_WIFI_START, i, 0) == FALSE) {
				break;
			}
#endif
#ifndef _WCE_OLD
			if (op.SocLog > 1) {
				char msg[50];
				sprintf_s(msg, 50, "Check: box=%d%s", SelBox, "\r\n");
				log_save_a(msg);
			}
#endif
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
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
			if (SaveBoxesLoaded == FALSE) {
				BOOL do_saveboxes = (op.BlindAppend == 0) ? TRUE : FALSE;
				BOOL err = FALSE;
				for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
					if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->CyclicFlag == 0) {
						if (mailbox_load_now(hWnd, i, FALSE, do_saveboxes) != 1) {
							err = TRUE;
							break;
						}
					}
				}
				if (err == TRUE) {
					break;
				}
				SaveBoxesLoaded = do_saveboxes; // may become false if filter is added
			}
			if (op.SocLog > 1) log_save_a("Check all\r\n");
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
		case ID_MESSAGE_DOWNLOAD:
#ifndef _WIN32_WCE
		case ID_MENUITEM_DELETE_ALL_SERVER:
#endif
			if (g_soc != -1) {
				break;
			}
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
			KeyShowHeader = (GetKeyState(VK_SHIFT) < 0) ? TRUE : FALSE;
			ServerDelete = FALSE;
			ans = 0;

			if (command_id == ID_MESSAGE_DOWNLOAD) {
				CheckAfterThisUpdate = FALSE;
				i = (int)lParam;
				if (i < MAILBOX_USER || i >= MailBoxCnt) {
					break;
				}
				if (item_get_next_download_mark((MailBox + i), -1, NULL) == -1) {
					MessageBox(hWnd, STR_MSG_NOMARK, STR_TITLE_EXEC, MB_ICONEXCLAMATION | MB_OK);
					break;
				}
#ifndef _WIN32_WCE
			} else if (command_id == ID_MENUITEM_DELETE_ALL_SERVER) {
				CheckAfterThisUpdate = FALSE;
				ServerDelete = ID_MENUITEM_DELETE_ALL_SERVER;
				i = SelBox;
#endif
			} else {
				CheckAfterThisUpdate = (op.CheckAfterUpdate == 1) ? TRUE : FALSE;
				j  = item_get_next_delete_mark((MailBox + SelBox), FALSE, -1, NULL);
				if (j != -1) {
					TCHAR msg[MSG_SIZE];
					wsprintf(msg, STR_Q_DEL_FWDHOLD_ACCT, ((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
						? STR_MAILBOX_NONAME :(MailBox + SelBox)->Name);
					ans = MessageBox(hWnd, msg, STR_TITLE_EXEC, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
					if (ans == IDYES) {
						ServerDelete = TRUE;
						while (j != -1) {
							(*((MailBox + SelBox)->tpMailItem + j))->ReFwd &= ~(REFWD_FWDHOLD);
							j = item_get_next_delete_mark((MailBox + SelBox), FALSE, j, NULL);
						}
					} else if (ans == IDCANCEL) {
						ShowError = FALSE;
						break;
					} else {
						ans = -2;
					}
				}
				if (ServerDelete == FALSE
					&& item_get_next_delete_mark((MailBox + SelBox), TRUE, -1, NULL) != -1) {
					ans = ParanoidMessageBox(hWnd, (ans == 0) ? STR_Q_DELSERVERMAIL : STR_Q_DELSERVERNOHOLD,
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
					item_get_next_send_mark((MailBox + SelBox), FALSE, TRUE) == -1) {

					MessageBox(hWnd, (ans == -2) ? STR_MSG_MARK_HELD : STR_MSG_NOMARK,
						STR_TITLE_EXEC, MB_ICONEXCLAMATION | MB_OK);
					break;
				}
				i = SelBox;
			}
#ifndef _WCE_OLD
			if (op.SocLog > 1) {
				char msg[50];
				sprintf_s(msg, 50, "Update: box=%d, delete=%d\r\n", i, ServerDelete);
				log_save_a(msg);
			}
#endif
			AutoCheckFlag = FALSE;
#ifdef ENABLE_RAS
			if (op.RasCon == 1 && i >= MAILBOX_USER && SendMessage(hWnd, WM_RAS_START, i, 0) == FALSE) {
				break;
			}
#endif
#ifdef ENABLE_WIFI
			if (op.WiFiCon == 1 && i >= MAILBOX_USER && SendMessage(hWnd, WM_WIFI_START, i, 0) == FALSE) {
				break;
			}
#endif
			AllCheck = FALSE;
			ExecFlag = TRUE;
			NewMailCnt = -1;
			if (CheckAfterThisUpdate && SelBox != MAILBOX_SEND) {
				NewMailCnt = 0;
				Init_NewMailFlag(hWnd);
				if (SaveBoxesLoaded == FALSE && op.BlindAppend == 0) {
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
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
			KeyShowHeader = (GetKeyState(VK_SHIFT) < 0) ? TRUE : FALSE;

			if (SaveBoxesLoaded == FALSE) {
				BOOL err = FALSE;
				for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
					if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->CyclicFlag == 0) {
						BOOL do_saveboxes =  (op.BlindAppend == 0) && (op.CheckAfterUpdate || ((MailBox+i)->FilterEnable & FILTER_REFILTER));
						if (mailbox_load_now(hWnd, i, FALSE, do_saveboxes) != 1) {
							err = TRUE;
							break;
						}
					}
				}
				if (err == TRUE) {
					break;
				}
				SaveBoxesLoaded = (op.BlindAppend == 0) && op.CheckAfterUpdate; // may become false if filter is added
			}
			if (MailMarkCheck(hWnd, FALSE) == FALSE) {
				break;
			}
#ifndef _WCE_OLD
			if (op.SocLog > 1) {
				char msg[50];
				sprintf_s(msg, 50, "Update all: delete=%d%s", SelBox, "\r\n");
				log_save_a(msg);
			}
#endif

			AutoCheckFlag = FALSE;
			AllCheck = TRUE;
			ExecFlag = TRUE;
			CheckBox = MAILBOX_USER - 1;
			NewMailCnt = -1;
			CheckAfterThisUpdate = (op.CheckAfterUpdate == 1) ? TRUE : FALSE;
			if (CheckAfterThisUpdate) {
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
#ifdef ENABLE_RAS
			KillTimer(hWnd, ID_RASWAIT_TIMER);
#endif
#ifdef ENABLE_WIFI
			KillTimer(hWnd, ID_WIFIWAIT_TIMER);
#endif
			gSockFlag = FALSE;
			AllCheck = FALSE;
			AutoCheckFlag = FALSE;
			ExecFlag = FALSE;
			KeyShowHeader = FALSE;
			NewMailCnt = -1;
#ifdef ENABLE_RAS
			if (RasLoop == TRUE || (g_soc == -1 && op.RasCheckEndDisCon == 1)) {
				RasDisconnect();
			}
#endif
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE || (g_soc == -1 && op.WiFiCheckEndDisCon == 1)) {
				WiFiDisconnect(FALSE);
			}
#endif
			if (g_soc == -1 || GetHostFlag == TRUE) {
				g_soc = -1;
				SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
				RecvBox = -1;
				EndSocketFunc(hWnd, FALSE);
				if (op.SocLog > 0) log_flush();
				break;
			}
			if (command_status == POP_RETR || command_status == POP_TOP) {
				pop3_salvage_buffer(hWnd, (MailBox + RecvBox), RecvBox == SelBox);
			}
			if (command_status == POP_QUIT || command_status == POP_START) {
				socket_close(hWnd, g_soc);
				g_soc = -1;
				SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
				RecvBox = -1;
				SetItemCntStatusText(NULL, FALSE, TRUE);
				SetUnreadCntTitle(TRUE);
				EndSocketFunc(hWnd, FALSE);
				if (op.SocLog > 0) log_flush();
				break;
			}
			command_status = POP_QUIT;
			SetSocStatusTextT(hWnd, TEXT(CMD_RSET));
			send_buf(g_soc, CMD_RSET"\r\n");
			SetSocStatusTextT(hWnd, TEXT(CMD_QUIT));
			send_buf(g_soc, CMD_QUIT"\r\n");
			if (op.SocLog > 0) log_flush();
			break;

		//====== mail =========
		//Open the dropdown menu if focus on combobox
		case ID_KEY_ENTER:
			fWnd = GetFocus();
			if (fWnd == GetDlgItem(hWnd, IDC_MBMENU)) {
				if (op.MBMenuWidth > 0) {
					SetFocus(mListView);
				} else {
					DropMBMenu(!GetDroppedStateMBMenu());
				}
				break;
			} else if (op.PreviewPaneWidth > 0) {
				HWND previewWnd = GetDlgItem(hWnd, IDC_EDIT_BODY);
				if (fWnd == previewWnd) {
					SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_NEXTMAIL, 0);
					break;
				}
			}
			// else fall through

		case ID_MAILITEM_OPEN:
			OpenItem(hWnd, FALSE, FALSE, TRUE);
			break;

		case ID_MENUITEM_OPEN:
			OpenItem(hWnd, TRUE, FALSE, TRUE);
			break;

		//Reply
		// (for SENDBOX, this menuitem ID is associated with "Property")
		case ID_MENUITEM_REMESSEGE:
		case ID_MENUITEM_PROP:
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

		//Redirect (GJC)
		case ID_MENUITEM_REDIRECT:
			ReMessageItem(hWnd, EDIT_REDIRECT);
			break;

		// flag for follow-up
		case ID_MENUITEM_FLAGMARK:
			if (SelBox == RecvBox && ExecFlag == TRUE) {
				break;
			}
			SetFlagOrDeleteMark(hWnd, ICON_FLAG, FALSE);
			break;

		case ID_MENUITEM_DOWNMARK:	// set download mark
		case ID_KEY_CTRLENTER:		// toggle download mark
		case ID_MENUITEM_SENDMARK:  // set send mark
			if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || (SelBox == RecvBox && ExecFlag == TRUE)) {
				break;
			}
			SetDownloadMark(hWnd);
			break;

		case ID_KEY_DELETE:
		case ID_MENUITEM_DELETE:
			if ( command_id == ID_MENUITEM_DELETE 
				|| (MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || SelBox == MAILBOX_SEND
				|| (command_id == ID_KEY_DELETE && op.DelIsMarkDel == FALSE) ) {
				ListDeleteItem(hWnd, TRUE);
				break;
			} // else fall through: Del is mark for delete
		//In one for deletion mark
		case ID_MENUITEM_DELMARK:
			if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || SelBox == MAILBOX_SEND || (SelBox == RecvBox && ExecFlag == TRUE)) {
				break;
			}
			SetFlagOrDeleteMark(hWnd, ICON_DEL, FALSE);
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
			if (SelBox != MAILBOX_SEND) {
				SetMailStats(hWnd, ICON_READ);
			}
			break;

		//Not yet you open the
		case ID_MENUITEM_UNREADMAIL:
			if (SelBox == MAILBOX_SEND) {
				break;
			}
			SetMailStats(hWnd, ICON_MAIL);
			break;

		case ID_MENUITEM_MOVESBOX:
			mark_del = TRUE; // and fall through to do the move
		case ID_MENUITEM_COPYSBOX:
		case ID_MENUITEM_SAVECOPY:
			{
				int i, cnt = 0, Target = -1;
				if (ListView_GetSelectedCount(mListView) <= 0) {
					break;
				}
				fWnd = GetDlgItem(hWnd, IDC_EDIT_BODY);
				if (GetFocus() == fWnd) {
					SendMessage(fWnd, WM_COPY, 0, 0);
					break;
				}
				if (SelBox == MAILBOX_SEND && command_id == ID_MENUITEM_SAVECOPY) {
					// (in SendBox, Ctrl-C does "edit as new"
					Target = MAILBOX_SEND;
				} else {
					for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
						if ((MailBox + i)->Type == MAILBOX_TYPE_SAVE) {
							Target = i;
							cnt++;
						}
					}
					if (cnt == 0 || (cnt == 1 && Target == SelBox)) {
						MessageBox(hWnd, STR_ERR_NOSAVEBOXES, WINDOW_TITLE, MB_OK);
						Target = -1;
					} else if (cnt > 1) {
						Target = mark_del;
						if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SELSAVEBOX), hWnd, SelSaveBoxProc, (LPARAM)&Target) == FALSE) {
							Target = -1;
						}
						if (Target == 0) {
							WPARAM newcmd = (mark_del == TRUE) ? ID_MENUITEM_MOVE2NEW : ID_MENUITEM_COPY2NEW;
							SendMessage(hWnd, WM_COMMAND, newcmd, 0);
							break;
						}
					}
				}
				if (Target != -1) {
					TCHAR fname[BUF_SIZE];
					if (Target == MAILBOX_SEND) {
						lstrcpy(fname, SENDBOX_FILE);
					} else if ((MailBox + Target)->Filename == NULL) {
						wsprintf(fname, TEXT("MailBox%d.dat"), Target - MAILBOX_USER);
					} else {
						lstrcpy(fname, (MailBox + Target)->Filename);
					}
					if (ItemToSaveBox(hWnd, NULL, Target, fname, (cnt<=1), mark_del) == TRUE) {
						if (mark_del == TRUE) {
							if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || SelBox == MAILBOX_SEND) {
								ListDeleteItem(hWnd, FALSE);
							} else {
								SetFlagOrDeleteMark(hWnd, ICON_DEL, TRUE);
							}
						}
						if (op.AutoSave != 0 && (MailBox+Target)->Loaded == TRUE) {
							// save Target mailbox
							file_save_mailbox(fname, DataDir, Target, FALSE, TRUE, 2);
						}
					}
					if (SelBox == MAILBOX_SEND && Target == MAILBOX_SEND && mark_del == FALSE) {
						OpenItem(hWnd, TRUE, FALSE, TRUE);
					}
				}
			}
			break;

		//delete attachments
		case ID_MENUITEM_DELATTACH:
			if (SelBox != MAILBOX_SEND) {
				ListDeleteAttach(hWnd);
			}
			break;

		//Entirely the selective
		case ID_MENUITEM_ALLSELECT:
			if (op.PreviewPaneWidth > 0) {
				HWND previewWnd = GetDlgItem(hWnd, IDC_EDIT_BODY);
				if (GetFocus() == previewWnd) {
					SendMessage(previewWnd, EM_SETSEL, 0, -1);
					break;
				}
			}
			SetFocus(mListView);
			ListView_SetItemState(mListView, -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

#ifdef _WIN32_WCE_PPC
		//of account Selective mode
		case ID_MENUITEM_SELMODE:
			SelMode = !SelMode;
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_MAIL), ID_MENUITEM_SELMODE, (SelMode == TRUE) ? MF_CHECKED : MF_UNCHECKED);
			CheckMenuItem(hMainPop, ID_MENUITEM_SELMODE, (SelMode == TRUE) ? MF_CHECKED : MF_UNCHECKED);
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
			if (CloseEditViewWindows(0) == FALSE) {
				break;
			}
			FocusWnd = hWnd;
			ShowWindow(hWnd, SW_SHOW);
#else
			if (op.ShowPass == 1 &&
				(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
				op.Password != NULL && *op.Password != TEXT('\0') &&
				ConfirmPass(hWnd, op.Password, FALSE) == FALSE) {
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
#ifndef _WIN32_WCE
			if (command_id >= ID_MENUITEM_TRAYCHECK && command_id <= ID_MENUITEM_TRAYMAX) {
				int cnt = command_id - ID_MENUITEM_TRAYCHECK + 1;
				if (g_soc != -1) {
					break;
				}
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
				for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
					if ((MailBox + i)->Type != MAILBOX_TYPE_SAVE) {
						cnt--;
						if (cnt == 0) {
							if (SaveBoxesLoaded == FALSE && op.BlindAppend == 0) {
								if (mailbox_load_now(hWnd, SelBox, FALSE, TRUE) != 1) {
									break;
								}
							}
							AutoCheckFlag = FALSE;
#ifdef ENABLE_RAS
							if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, i, 0) == FALSE) {
								break;
							}
#endif
#ifdef ENABLE_WIFI
							if (op.WiFiCon == 1 && SendMessage(hWnd, WM_WIFI_START, i, 0) == FALSE) {
								break;
							}
#endif
#ifndef _WCE_OLD
							if (op.SocLog > 1) {
								char msg[50];
								sprintf_s(msg, 50, "Check: box=%d%s", SelBox, "\r\n");
								log_save_a(msg);
							}
#endif
							AllCheck = FALSE;
							ExecFlag = FALSE;
							KeyShowHeader = FALSE;
							NewMailCnt = 0;
							Init_NewMailFlag(hWnd);

							//Mail reception start
							RecvMailList(hWnd, i, FALSE);
							break;
						}
					}
				}
				break;
			}
#endif
			if (command_id == ID_MENUITEM_COPY2NEW || command_id == ID_MENUITEM_MOVE2NEW) {
				// GJC - copy/move to new SaveBox
				int old_selbox, newbox;
				if (ListView_GetSelectedCount(mListView) <= 0) {
					break;
				}
				old_selbox = SelBox;
				SelBox = newbox = mailbox_create(hWnd, 1, -1, TRUE, FALSE);
				if (SetMailBoxType(hWnd, MAILBOX_ADD_SAVE) == -1) {
					mailbox_delete(hWnd, newbox, FALSE, TRUE);
					SelBox = old_selbox;
					SelectMBMenu(SelBox);
					break;
				} else if (command_id == ID_MENUITEM_COPY2NEW) {
					command_id = newbox + ID_MENUITEM_COPY2MBOX;
					// and fall through to do the copy
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
				if (mbox >=0 && mbox != SelBox && mbox < MailBoxCnt && (MailBox+mbox) != NULL) {
					TCHAR fname[BUF_SIZE];
					if (mbox == MAILBOX_SEND) {
						lstrcpy(fname, SENDBOX_FILE);
					} else if ((MailBox + mbox)->Filename == NULL) {
						wsprintf(fname, TEXT("MailBox%d.dat"), mbox - MAILBOX_USER);
					} else {
						lstrcpy(fname, (MailBox + mbox)->Filename);
					}
					if (ItemToSaveBox(hWnd, NULL, mbox, fname, TRUE, mark_del) == TRUE) {
						if (mark_del == TRUE) {
							// delete from list or mark for deletion
							if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE || SelBox == MAILBOX_SEND) {
								ListDeleteItem(hWnd, FALSE);
							} else {
								SetFlagOrDeleteMark(hWnd, ICON_DEL, TRUE);
							}
						}
						if (op.AutoSave != 0 && (MailBox+mbox)->Loaded == TRUE) {
							file_save_mailbox(fname, DataDir, mbox, FALSE, TRUE, 2);
						}
					}
				}
			}
			break;
		} // end switch(command_id)
		}
		break;

#ifdef ENABLE_RAS
	//Start
	case WM_RAS_START:
		return RasMailBoxStart(hWnd, wParam);
#endif
#ifdef ENABLE_WIFI
	//Start
	case WM_WIFI_START:
		return WiFiConnect(hWnd, wParam);
#endif

#ifndef _WIN32_WCE
	case WM_SYSCOMMAND:
		if (op.ShowPass == 1 &&
			(wParam == SC_RESTORE || wParam == SC_MAXIMIZE) &&
			(IsWindowVisible(hWnd) == 0 || IsIconic(hWnd) != 0) &&
			op.Password != NULL && *op.Password != TEXT('\0') &&
			ConfirmPass(hWnd, op.Password, TRUE) == FALSE) {
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
			{
				HMENU hSubMenu = GetSubMenu(hPOPUP, 0);
				int i, cnt, flag = MF_STRING;
				SetMenuDefaultItem(hSubMenu, ID_MENUITEM_RESTORE, 0);
				EnableMenuItem(hSubMenu, ID_MENUITEM_ALLCHECK, !(g_soc == -1));
				EnableMenuItem(hSubMenu, ID_MENUITEM_STOP, (g_soc == -1));
				hSubMenu = GetSubMenu(hSubMenu, 3);
				for (i = 0; i <= MAX_TRAY_CNT; i++) {
					DeleteMenu(hSubMenu, ID_MENUITEM_TRAYCHECK + i, MF_BYCOMMAND);
				}
				flag |= ((g_soc == -1) ? MF_ENABLED : MF_GRAYED);
				for (i = MAILBOX_USER, cnt = 0; i < MailBoxCnt && cnt < MAX_TRAY_CNT; i++) {
					if ((MailBox + i)->Type != MAILBOX_TYPE_SAVE) {
						TCHAR *p = ((MailBox + i)->Name != NULL) ? (MailBox + i)->Name : STR_MAILBOX_NONAME;
						AppendMenu(hSubMenu, flag, ID_MENUITEM_TRAYCHECK + cnt, p);
						cnt++;
					}
				}
				SendMessage(hWnd, WM_NULL, 0, 0);
				ShowMenu(hWnd, hPOPUP, 0, 0, FALSE);
			}
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
		case FD_CONNECT:					/* �T�[�o�ւ̐ڑ��������������������C�x���g */
			{
				TCHAR ErrStr[BUF_SIZE];
				*ErrStr = TEXT('\0');
				if (init_ssl(hWnd, g_soc, ErrStr) == -1) {
					ErrorSocketEnd(hWnd, RecvBox);
					SocketErrorMessage(hWnd, ErrStr, RecvBox);
				}
			}
			break;

		case FD_READ:						/* ��M�o�b�t�@�Ƀf�[�^�����鎖�������C�x���g */
			/* �f�[�^����M���Ē~�ς��� */
			if (recv_proc(hWnd, g_soc) == SELECT_MEM_ERROR) {
				// �������G���[
				ErrorSocketEnd(hWnd, RecvBox);
				SocketErrorMessage(hWnd, STR_ERR_MEMALLOC, RecvBox);
				break;
			}
			SetTimer(hWnd, ID_TIMEOUT_TIMER, TIMEOUTTIME * op.TimeoutInterval, NULL);
			break;

		case FD_WRITE:						/* ���M�\�Ȏ��������C�x���g */
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

		case FD_CLOSE:						/* �T�[�o�ւ̐ڑ����I���������������C�x���g */
			/* �ڑ����I������ */
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
				if (op.SocLog > 0) log_flush();
				AutoSave_Mailboxes(hWnd);
				SetItemCntStatusText(NULL, FALSE, TRUE);
				SetUnreadCntTitle(TRUE);
				if (AllCheck == FALSE) {
					if (op.CheckEndExec == 1 &&
						CheckEndAutoExec(hWnd, RecvBox, NewMailCnt, FALSE) == TRUE) {
						//After the checking execution
						break;
					}
					if (ExecFlag && CheckAfterThisUpdate && RecvBox != MAILBOX_SEND) {
						// ���s��`�F�b�N
						ExecFlag = FALSE;
						RecvMailList(hWnd, RecvBox, FALSE);
						break;
					}
					SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
					RecvBox = -1;
					EndSocketFunc(hWnd, TRUE);
					NewMail_Message(hWnd, NewMailCnt);
				} else {
					SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
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
					SetMailboxMark(RecvBox, STATUS_DONE, FALSE);
					RecvBox = -1;
					if (op.SocLog > 0) log_flush();
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
#ifdef ENABLE_WIFI
			if (WiFiLoop == TRUE) {
				break;
			}
#endif
		wkSendMailItem = (MAILITEM *)lParam;
		NewMailCnt = -1;
		SmtpWait = 0;
		CheckBox = mailbox_name_to_index(wkSendMailItem->MailBox, MAILBOX_TYPE_ACCOUNT);
		if (CheckBox != -1 && (MailBox + CheckBox)->PopBeforeSmtp != 0) {
			// POP before SMTP
			AutoCheckFlag = FALSE;
#ifdef ENABLE_RAS
			if (op.RasCon == 1 && SendMessage(hWnd, WM_RAS_START, CheckBox, 0) == FALSE) {
				ErrorSocketEnd(hWnd, CheckBox);
				if (op.SocLog > 0) log_flush();
				SetMailMenu(hWnd);
				break;
			}
#endif
#ifdef ENABLE_WIFI
			if (op.WiFiCon == 1 && SendMessage(hWnd, WM_WIFI_START, CheckBox, 0) == FALSE) {
				ErrorSocketEnd(hWnd, CheckBox);
				if (op.SocLog > 0) log_flush();
				SetMailMenu(hWnd);
				break;
			}
#endif
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
			KillTimer(hWnd, ID_PREVIEW_TIMER);
			SetTimer(hWnd, ID_ITEMCHANGE_TIMER, 1, NULL);
			i = ListView_GetSelectedCount(mListView);
			if (i == 1) {
				MAILITEM *tpMailItem = (MAILITEM *)ListView_GetlParam(mListView, 
					ListView_GetNextItem(mListView, -1, LVNI_SELECTED));
				if (op.ItemPlaySound) {
					// GJC sound if single selection, based on mailitem's Mark
					j = tpMailItem->Mark;
					if (tpMailItem->New) j |= 0x10;
					if (tpMailItem->Download) j |= 0x20;
					if (tpMailItem->Multipart == MULTIPART_ATTACH || tpMailItem->Multipart == MULTIPART_CONTENT) 
						j |= 0x40;
					if (tpMailItem->Multipart == MULTIPART_HTML) 
						j |= 0x80;
					PlayMarkSound(j, SelBox);
				}
				if (op.PreviewPaneHeight > 0) {
					PopulatePreviewPane(hWnd, tpMailItem);
				}
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
			SetCapture(mListView);
			break;

		case LVN_ITEMACTIVATE:
			if (op.PreviewPaneHeight <= 0) {
				SendMessage(hWnd, WM_COMMAND, ID_MAILITEM_OPEN, 0);
			}
			break;

		case NM_DBLCLK:
			if (op.PreviewPaneHeight > 0) {
				SendMessage(hWnd, WM_COMMAND, ID_MAILITEM_OPEN, 0);
			}
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
		if (op.StartCheck == 1 && gAutoSend == FALSE) {
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ALLCHECK, 0);
		}
		break;

	default:
#ifdef ENABLE_RAS
		if (msg == WM_RASEVENT) {
			return RasStatusProc(hWnd, msg, wParam, lParam);
		}
#endif
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * InitApplication - �E�B���h�E�N���X�̓o�^
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
 * InitInstance - �E�B���h�E�̍쐬
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
 * MessageFunc - ���b�Z�[�W����
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
 * WinMain - ���C��
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
	TCHAR *lptCmdLine;
	BOOL ret;

	hInst = hInstance;

#if defined(UNICODE) && !defined(_WIN32_WCE)
	// Win32 Unicode has char* lpCmdLine!
	lptCmdLine = alloc_char_to_tchar(lpCmdLine);
//	lptCmdLine = alloc_copy_t(GetCommandLineW()); // gets program name also
#else
	lptCmdLine = lpCmdLine;
#endif
	// Sets AppDir and parses lpCmdLine to set IniFile and static CmdLine
	ret = GetAppPath(hInstance, lptCmdLine);
	if (ret != TRUE) {
#if defined(UNICODE) && !defined(_WIN32_WCE)
		mem_free(&lptCmdLine);
#endif
		if (ret == FALSE) {
			ErrorMessage(NULL, STR_ERR_MEMALLOC);
		}
		return 0;
	}
#if defined(UNICODE) && !defined(_WIN32_WCE)
	mem_free(&lptCmdLine);
#endif

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
			if (CmdLine != NULL) {

				if (gCheckAndQuit) {
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				} else {
					COPYDATASTRUCT cpdata;

					cpdata.lpData = CmdLine;
					cpdata.cbData = sizeof(TCHAR) * (lstrlen(CmdLine) + 1);

					SendMessage(hWnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cpdata);
				}
			} else {
				SendMessage(hWnd, WM_SHOWLASTWINDOW, 0, 0);
			}
		}
		mem_free(&CmdLine);
		mem_free(&AppDir);
		mem_free(&DefaultDataDir);
		mem_free(&IniFile);
		mem_free(&InitialAccount);
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		return 0;
	}
#endif	// _WCE_OLD
#endif	// _DEBUG

	{
		int TmpCmdShow;
		//of job pass of application Check
		TmpCmdShow = CmdShow;
		if (ini_start_auth_check() == FALSE) {
			mem_free(&g_Pass);
			mem_free(&CmdLine);
			mem_free(&AppDir);
			mem_free(&DefaultDataDir);
			mem_free(&IniFile);
			mem_free(&InitialAccount);
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
		ErrorMessage(NULL, STR_ERR_INIT);
		mem_free(&CmdLine);
		mem_free(&AppDir);
		mem_free(&DefaultDataDir);
		mem_free(&IniFile);
		mem_free(&InitialAccount);
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		return 0;
	}
#ifndef _WCE_OLD
	charset_init();
#endif
	InitCommonControls();
#ifdef ENABLE_RAS
	initRas();
#endif

#ifdef LOAD_USER_IMAGES
	// loading user-supplied bitmaps
	{
		BITMAP bitmap;
		TCHAR fpath[BUF_SIZE];
		wsprintf(fpath, TEXT("%sResource\\tbar_main.bmp"), AppDir);
#ifdef _WIN32_WCE_PPC
		MainBmp = SHLoadImageFile(fpath);
#else
		MainBmp = LoadImage(NULL, fpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
#endif
		if (MainBmp) {
			GetObject(MainBmp, sizeof(BITMAP), &bitmap);
			op.MainBmpSize = bitmap.bmHeight;
			if (bitmap.bmHeight < 8 || bitmap.bmWidth/bitmap.bmHeight != TB_MAINBUTTONS) {
				DeleteObject(MainBmp);
				MainBmp = NULL;
			}
		}
		wsprintf(fpath, TEXT("%sResource\\tbar_edit.bmp"), AppDir);
#ifdef _WIN32_WCE_PPC
		EditBmp = SHLoadImageFile(fpath);
#else
		EditBmp = LoadImage(NULL, fpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
#endif
		if (EditBmp) {
			GetObject(EditBmp, sizeof(BITMAP), &bitmap);
			op.EditBmpSize = bitmap.bmHeight;
			if (bitmap.bmHeight < 8 || bitmap.bmWidth/bitmap.bmHeight != TB_EDITBUTTONS) {
				DeleteObject(EditBmp);
				EditBmp = NULL;
			}
		}
		wsprintf(fpath, TEXT("%sResource\\tbar_view.bmp"), AppDir);
#ifdef _WIN32_WCE_PPC
		ViewBmp = SHLoadImageFile(fpath);
#else
		ViewBmp = LoadImage(NULL, fpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
#endif
		if (ViewBmp) {
			GetObject(ViewBmp, sizeof(BITMAP), &bitmap);
			op.ViewBmpSize = bitmap.bmHeight;
			if (bitmap.bmHeight < 8 || bitmap.bmWidth/bitmap.bmHeight != TB_VIEWBUTTONS) {
				DeleteObject(ViewBmp);
				ViewBmp = NULL;
			}
		}
	}
#endif

#ifdef _WIN32_WCE_LAGENDA
	SipShowIM(SIPF_OFF);
#endif

	//Window class register
	if (!InitApplication(hInstance) || !View_InitApplication(hInstance)
		|| !Edit_InitApplication(hInstance)) {
		mem_free(&CmdLine);
		mem_free(&AppDir);
		mem_free(&DefaultDataDir);
		mem_free(&IniFile);
		mem_free(&InitialAccount);
		WSACleanup();
#ifdef ENABLE_RAS
		FreeRas();
#endif
#ifndef _WCE_OLD
		charset_uninit();
#endif
#ifdef LOAD_USER_IMAGES
		DeleteObject(MainBmp);
		DeleteObject(EditBmp);
		DeleteObject(ViewBmp);
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
	// EDIT�o�^
	if (nedit_regist(hInstance) == FALSE) {
		return 0;
	}
#endif
	// ���C���E�B���h�E�̍쐬
	if ((hWnd = InitInstance(hInstance, CmdShow)) == NULL) {
		FreeAllMailBox();
		mem_free(&CmdLine);
		mem_free(&AppDir);
		mem_free(&DefaultDataDir);
		mem_free(&IniFile);
		mem_free(&InitialAccount);
		WSACleanup();
#ifdef ENABLE_RAS
		FreeRas();
#endif
#ifndef _WCE_OLD
		charset_uninit();
#endif
#ifdef LOAD_USER_IMAGES
		DeleteObject(MainBmp);
		DeleteObject(EditBmp);
		DeleteObject(ViewBmp);
#endif
		if (hMutex != NULL) {
			CloseHandle(hMutex);
		}
		if (first_start != -1) {
			ErrorMessage(NULL, STR_ERR_INIT);
		}
		return 0;
	}

	// tray icon pop-up
	hPOPUP = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_POPUP));
	// main window context menu
	hMainPop = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_MAINPOP));
	// view window context menu
	hViewPop = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_VIEWPOP));
	// mailbox pane context menu
	hMBPOPUP = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_MBPOPUP));
	// address book context menu
	hADPOPUP = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_ADDRESS_POPUP));
#ifdef _WIN32_WCE
	// edit window context menu (Win32 edit window class has its own context menu)
	hEditPop = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_EDITPOP));
#endif
#ifdef _WIN32_WCE_PPC
	// for single-line edittext widgets
	hEDITPOPUP = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_EDIT_POPUP));
#endif

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
	mem_free(&DefaultDataDir);
	mem_free(&IniFile);
	mem_free(&InitialAccount);
#ifdef LOAD_USER_IMAGES
	DeleteObject(MainBmp);
	DeleteObject(EditBmp);
	DeleteObject(ViewBmp);
#endif
	DestroyMenu(hPOPUP);
	DestroyMenu(hMainPop);
	DestroyMenu(hViewPop);
	DestroyMenu(hMBPOPUP);
	DestroyMenu(hADPOPUP);
#ifdef _WIN32_WCE
	DestroyMenu(hEditPop);
#endif
#ifdef _WIN32_WCE_PPC
	DestroyMenu(hEDITPOPUP);
#endif
	UnregisterClass(MAIN_WND_CLASS, hInstance);
	UnregisterClass(VIEW_WND_CLASS, hInstance);
	UnregisterClass(EDIT_WND_CLASS, hInstance);
	free_ssl();
	WSACleanup();
#ifdef ENABLE_RAS
	FreeRas();
#endif
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
 * MsgMBMenu
 */
#define MB_GETSEL		0
#define MB_SETSEL		1
#define MB_GETTXTLEN	2
#define MB_ADD			3
#define MB_INSERT		4
#define MB_DELETE		5
int MsgMBMenu(int msg)
{
	static CB_msgs[] = {
		CB_GETCURSEL, CB_SETCURSEL, CB_GETLBTEXTLEN, CB_ADDSTRING, CB_INSERTSTRING, CB_DELETESTRING };
	static LB_msgs[] = {
		LB_GETCURSEL, LB_SETCURSEL, LB_GETTEXTLEN, LB_ADDSTRING, LB_INSERTSTRING, LB_DELETESTRING };
	
	if (op.MBMenuWidth > 0) {
		return LB_msgs[ msg ];
	} else {
		return CB_msgs[ msg ];
	}
}

/*
 * GetSelectedMBMenu
 */
int GetSelectedMBMenu(void)
{
	int index;
	index = SendDlgItemMessage(MainWnd, IDC_MBMENU, MsgMBMenu(MB_GETSEL), 0, 0);
	return index;
}

/*
 * SelectMBMenu
 */
void SelectMBMenu(int index)
{
	index = SendDlgItemMessage(MainWnd, IDC_MBMENU, MsgMBMenu(MB_SETSEL), index, 0);
}

/*
 * AddMBMenu
 */
int AddMBMenu(TCHAR *Name)
{
	int index;
	index = SendDlgItemMessage(MainWnd, IDC_MBMENU, MsgMBMenu(MB_ADD), 0, (LPARAM) Name);
	return index;
}

/*
 * InsertMBMenu
 */
void InsertMBMenu(int index, TCHAR *Name)
{
	index = SendDlgItemMessage(MainWnd, IDC_MBMENU, MsgMBMenu(MB_INSERT), index, (LPARAM) Name);
}

/*
 * DeleteMBMenu
 */
void DeleteMBMenu(int index)
{
	index = SendDlgItemMessage(MainWnd, IDC_MBMENU, MsgMBMenu(MB_DELETE), index, 0);
}

/*
 * GetStarMBMenu - check if mailbox is prefaced with * # or ~
 */
BOOL GetStarMBMenu()
{
#ifdef _WIN32_WCE
	unsigned int len;
#else
	int len;
#endif
	len = SendDlgItemMessage(MainWnd, IDC_MBMENU, MsgMBMenu(MB_GETTXTLEN), MAILBOX_SEND, 0);
	return len > lstrlen(STR_SENDBOX_NAME);
}

/*
 * SetStarMBMenu - add * or # to IDC_MBMENU drop-down for SENDBOX
 */
void SetStarMBMenu(int Flag)
{
	DeleteMBMenu(MAILBOX_SEND);
	if (Flag == FALSE) {
		InsertMBMenu(MAILBOX_SEND, STR_SENDBOX_NAME);
#ifdef _WIN32_WCE_PPC
	} else if (Flag == TRUE) {
		InsertMBMenu(MAILBOX_SEND, TEXT("* ") STR_SENDBOX_NAME);
	} else if (Flag == ICON_FLAG) {
		InsertMBMenu(MAILBOX_SEND, TEXT("~ ") STR_SENDBOX_NAME);
	} else {
		InsertMBMenu(MAILBOX_SEND, TEXT("# ") STR_SENDBOX_NAME);
#else
	} else if (Flag == TRUE) {
		InsertMBMenu(MAILBOX_SEND, STR_SENDBOX_NAME TEXT(" *"));
	} else if (Flag == ICON_FLAG) {
		InsertMBMenu(MAILBOX_SEND, STR_SENDBOX_NAME TEXT(" ~"));
	} else {
		InsertMBMenu(MAILBOX_SEND, STR_SENDBOX_NAME TEXT(" #"));
#endif
	}
	if (SelBox == MAILBOX_SEND) {
		SelectMBMenu(MAILBOX_SEND);
	}
}

/*
 * GetDroppedStateMBMenu()
 */
BOOL GetDroppedStateMBMenu(void)
{
	if (op.MBMenuWidth <= 0) {
		return SendDlgItemMessage(MainWnd, IDC_MBMENU, CB_GETDROPPEDSTATE, 0, 0);
	} else {
		return FALSE;
	}
}

/*
 * DropMBMenu()
 */
void DropMBMenu(BOOL drop)
{
	if (op.MBMenuWidth <= 0) {
		SendDlgItemMessage(MainWnd, IDC_MBMENU, CB_SHOWDROPDOWN, drop, 0);
	}
}

/*
 * MessageBoxTimer - callback for TimedMessageBox (GJC)
 */
void CALLBACK MessageBoxTimer(HWND hWnd, UINT uiMsg, UINT idEvent, DWORD dwTime)
{
	g_bTimedOut = TRUE;
	if (op.SocLog > 1) log_save_a("MessageBoxTimer timed out\r\n");
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
 *   all but the last sound should be SND_SYNC to prevent playing overtop each other
 */
static void PlayMarkSound(int mark, int box)
{
	int lowbits = mark & 0x0F;
	BOOL do_more = FALSE;
	TCHAR *full_or_partial = NULL;
	DWORD async = SND_ASYNC;

	// report new first of all
	if ((mark & 0x10) && op.ItemNewSoundFile != NULL) {
		sndPlaySound(op.ItemNewSoundFile, SND_SYNC | SND_NODEFAULT);
	}

	// only report these for unread/read (not marked)
	if (mark & 0x20) {
		full_or_partial = op.ItemFullSoundFile;
	} else { 
		full_or_partial = op.ItemPartialSoundFile;
	}

	// report these at the end, but make unread/read sound be SND_SYNC
	if ((mark & 0x40) && op.ItemAttachSoundFile != NULL) {
		async = SND_SYNC;
	}
	if ((mark & 0x80) && op.ItemHtmlSoundFile != NULL) {
		async = SND_SYNC;
	}

	switch (lowbits) {
		case ICON_ERROR:
			if (op.ItemErrorSoundFile != NULL) {
				sndPlaySound(op.ItemErrorSoundFile, SND_ASYNC | SND_NODEFAULT);
			}
			break;
		case ICON_FLAG:
			if (op.ItemFlagSoundFile != NULL) {
				sndPlaySound(op.ItemFlagSoundFile, SND_ASYNC | SND_NODEFAULT);
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
				sndPlaySound(op.ItemSentSoundFile, async | SND_NODEFAULT);
				do_more = TRUE;
			}
			break;
		case ICON_SEND:
			if (op.ItemSendSoundFile != NULL) {
				sndPlaySound(op.ItemSendSoundFile, async | SND_NODEFAULT);
				do_more = TRUE;
			}
			break;
		case ICON_MAIL:
			if (full_or_partial != NULL) {
				sndPlaySound(full_or_partial, SND_SYNC | SND_NODEFAULT);
			}
			if (op.ItemUnreadSoundFile != NULL) {
				sndPlaySound(op.ItemUnreadSoundFile, async | SND_NODEFAULT);
				do_more = TRUE;
			}
			break;
		case ICON_READ:
			if (full_or_partial != NULL) {
				sndPlaySound(full_or_partial, SND_SYNC | SND_NODEFAULT);
			}
			if (op.ItemReadSoundFile != NULL) {
				sndPlaySound(op.ItemReadSoundFile, async | SND_NODEFAULT);
				do_more = TRUE;
			}
			break;
		default: // including ICON_NON
			if (op.ItemNonSoundFile != NULL) {
				sndPlaySound(op.ItemNonSoundFile, async | SND_NODEFAULT);
				do_more = TRUE;
			}
			break;
	}

	if (do_more) {
		if ((mark & 0x40) && op.ItemAttachSoundFile != NULL) {
			sndPlaySound(op.ItemAttachSoundFile, SND_ASYNC | SND_NODEFAULT);
		}
		if ((mark & 0x80) && op.ItemHtmlSoundFile != NULL) {
			sndPlaySound(op.ItemHtmlSoundFile, SND_ASYNC | SND_NODEFAULT);
		}
	}
	return;
}

/*
 * PopulatePreviewPane - put contents into preview pane
 */
static void PopulatePreviewPane(HWND hWnd, MAILITEM *tpMailItem)
{
	TCHAR *buf, *body, *p;
	int len, cnt, TextIndex;
	if (SelBox != MAILBOX_SEND) {
		MULTIPART **tpMultiPart = NULL;
		body = MIME_body_decode(tpMailItem, FALSE, TRUE, &tpMultiPart, &cnt, &TextIndex);

		if (op.StripHtmlTags == 1) {
			TCHAR *ctype = tpMailItem->ContentType;
			if (TextIndex != -1) {
				ctype = alloc_char_to_tchar((tpMultiPart[TextIndex])->ContentType);
			}
			if (ctype != NULL &&
				(str_cmp_ni_t(ctype, TEXT("text/html"), lstrlen(TEXT("text/html"))) == 0
				|| str_cmp_ni_t(ctype, TEXT("text/x-aol"), lstrlen(TEXT("text/x-aol"))) == 0)) {

				p = strip_html_tags(body, 2);
				if (p != NULL) {
					mem_free(&body);
					body = p;
				}
			}
			if (TextIndex != -1) {
				mem_free(&ctype);
			}

		}

		if (tpMailItem->MailStatus != ICON_READ) {
			if (op.PreviewedIsReadTime > 0) {
				SetTimer(hWnd, ID_PREVIEW_TIMER, 1000*op.PreviewedIsReadTime, NULL);
			} else if (op.PreviewedIsReadTime == 0) {
				SetMailStats(hWnd, ICON_READ);
			}
		}
		multipart_free(&tpMultiPart, cnt);
	} else {
		body = alloc_char_to_tchar(tpMailItem->Body);
	}
	if (body == NULL) {
		body = alloc_copy_t(TEXT(""));
	}
	len = CreateHeaderStringSize(op.PreviewHeader, tpMailItem, NULL);
	len += lstrlen(body) + 1;
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
	if (buf != NULL) {
		p = CreateHeaderString(op.PreviewHeader, buf, tpMailItem, NULL);
		str_join_t(p, body, (TCHAR *)-1);
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)buf);
		mem_free(&buf);
	}
	mem_free(&body);
}

/*
 * ViewLogFile
 */
static void ViewLogFile(HWND hWnd)
{
	TCHAR *buf = log_read();
	if (buf == NULL) {
		buf = alloc_copy_t(STR_MSG_LOG_FILE_EMPTY);
	}
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_OPTION_EDITOR), hWnd,
		SocLogViewProc, (LPARAM)buf);
	mem_free(&buf);
}

/***
 * Console startup
 */

#if 0  // eliminate this optimization, so that getenv_s will work
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
#endif
/* End of source */
