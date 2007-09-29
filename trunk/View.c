/**************************************************************************

	nPOP

	View.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"
#include <imm.h>


/**************************************************************************
	Define
**************************************************************************/

#define WM_MODFYMESSAGE				(WM_APP + 101)
#define ID_MENU						(WM_APP + 102)

#define IDC_VCB						2000
#define IDC_VTB						2001
#define IDC_HEADER					2002
#define IDC_EDIT_BODY				2003

#define ID_CLICK_TIMER				1
#define ID_HIDECARET_TIMER			2

#define ID_VIEW_SOURCE				400
#define ID_VIEW_PART				401
#define ID_VIEW_DELETE_ATTACH		402
#define ID_ATTACH					500

#define SAVE_HEADER					TEXT("From: %f\r\nTo: %t\r\nCc: %c\r\nSubject: %s\r\nDate: %d\r\nMessage-ID: %i\r\n\r\n")


/**************************************************************************
	Global Variables
**************************************************************************/

static WNDPROC EditWindowProcedure;
HWND hViewWnd = NULL;
#ifdef _WIN32_WCE_PPC
static HWND hToolBar;
#endif

#ifdef _WIN32_WCE_LAGENDA
static HMENU hViewMenu;
static int g_menu_height;
#endif

int vSelBox;

static struct TPMULTIPART **tpMultiPart;
static int MultiPartCnt;

TCHAR *FindStr = NULL;

static BOOL ESCFlag = FALSE;
static BOOL UnicodeEdit = 0;

//外部参照
extern HINSTANCE hInst;  // Local copy of hInstance
extern HWND MainWnd;
extern HWND FocusWnd;
extern HFONT hListFont;
extern struct TPMAILBOX *MailBox;
extern int SelBox;
extern int MailBoxCnt;
extern BOOL PPCFlag;

extern int WordBreakFlag;
extern int ViewShowDate;
#ifndef _WIN32_WCE
extern RECT ViewRect;
extern int ViewClose;
#endif
extern TCHAR *ViewApp;
extern TCHAR *ViewAppCmdLine;
extern TCHAR *ViewFileSuffix;
extern TCHAR *ViewFileHeader;
extern int ViewAppClose;
extern int DefViewApp;
extern TCHAR *URLApp;

extern HFONT hEditFont;

extern int MstchCase;
extern int AllFind;
extern int SubjectFind;

extern int MoveAllMailBox;

extern TCHAR *DataDir;
extern TCHAR *AppDir;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static void SetWindowString(HWND hWnd, TCHAR *MailBoxName, TCHAR *MailBoxName2, int No);
static void SetHeaderString(HWND hHeader, struct TPMAILITEM *tpMailItem);
static void SetHeaderSize(HWND hWnd);
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam);
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam);
#endif
static BOOL FindEditString(HWND hEdit, TCHAR *strFind, int CaseFlag);
static LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);
static void SetEditSubClass(HWND hWnd);
static void DelEditSubClass(HWND hWnd);
static BOOL InitWindow(HWND hWnd, struct TPMAILITEM *tpMailItem);
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void EndWindow(HWND hWnd);
static void SetEditMenu(HWND hWnd);
static void ModfyWindow(HWND hWnd, struct TPMAILITEM *tpMailItem, BOOL ViewSrc);
static struct TPMAILITEM *View_NextMail(HWND hWnd, BOOL St);
static void View_PrevMail(HWND hWnd);
static void View_NextNoReadMail(HWND hWnd);
static void View_NextScroll(HWND hEditWnd);
static BOOL ShellOpen(TCHAR *FileName);
static void OpenURL(HWND hWnd);
static void SetReMessage(HWND hWnd, int ReplyFag);
static BOOL Decode(HWND hWnd, int id);
static BOOL SaveViewMail(TCHAR *fname, HWND hWnd, int MailBoxIndex, struct TPMAILITEM *tpMailItem, TCHAR *head);
static BOOL AppViewMail(struct TPMAILITEM *tpMailItem);
static LRESULT CALLBACK ViewProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);


/******************************************************************************

	SetWindowString

	ウィンドウタイトルの設定

******************************************************************************/

static void SetWindowString(HWND hWnd, TCHAR *MailBoxName, TCHAR *MailBoxName2, int No)
{
	TCHAR *buf, *r;
	TCHAR *p, *p2;

	p = (MailBoxName == NULL || *MailBoxName == TEXT('\0')) ? STR_MAILBOX_NONAME : MailBoxName;
	p2 = (MailBoxName2 == NULL || *MailBoxName2 == TEXT('\0')) ? TEXT("") : MailBoxName2;

	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) *
		(lstrlen(STR_TITLE_MAILVIEW) + lstrlen(p) + lstrlen(p2) + lstrlen(TEXT(" -  ()") STR_TITLE_MAILVIEW_COUNT) + 10 + 1));
	if(buf == NULL){
		SetWindowText(hWnd, STR_TITLE_MAILVIEW);
		return;
	}

	r = TStrJoin(buf, STR_TITLE_MAILVIEW, TEXT(" - "), p, (TCHAR *)-1);
	if(*p2 != TEXT('\0')){
		TStrJoin(r, TEXT(" ("), p2, TEXT(")"), (TCHAR *)-1);
	}else if(No != 0){
		wsprintf(r, STR_TITLE_MAILVIEW_COUNT, No);
	}
	if(lstrlen(buf) > BUF_SIZE){
		*(buf + BUF_SIZE) = TEXT('\0');
	}
	SetWindowText(hWnd, buf);
	LocalFree(buf);
}


/******************************************************************************

	SetHeaderString

	メールヘッダ表示

******************************************************************************/

static void SetHeaderString(HWND hHeader, struct TPMAILITEM *tpMailItem)
{
	TCHAR *MyMailAddress = NULL;
	TCHAR *buf, *p;
	int len = 0;
	int i;
	BOOL ToFlag = FALSE;

	//自分のメールアドレスの取得
	if(SelBox == MAILBOX_SAVE){
		i = GetNameToMailBox(tpMailItem->MailBox);
		if(i >= MAILBOX_USER){
			MyMailAddress = (MailBox + i)->MailAddress;
		}
	}else{
		MyMailAddress = (MailBox + SelBox)->MailAddress;
	}

	//To を表示するべきかチェック
	if(tpMailItem->To != NULL){
		buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(tpMailItem->To) + 1));
		if(buf != NULL){
			p = GetMailAddress(tpMailItem->To, buf, FALSE);
			if(*buf != TEXT('\0') &&
				(*p != TEXT('\0') || MyMailAddress == NULL || TStrCmpI(MyMailAddress, buf) != 0)){
				ToFlag = TRUE;
			}
			LocalFree(buf);
		}
	}

	//作成する文字列のサイズを計算
	len += lstrlen(STR_VIEW_HEAD_FROM);
	if(tpMailItem->From != NULL){
		len += lstrlen(tpMailItem->From);
	}
	len += lstrlen(STR_VIEW_HEAD_SUBJECT);
	if(tpMailItem->Subject != NULL){
		len += lstrlen(tpMailItem->Subject);
	}
	if(ToFlag == TRUE){
		len += SetCcAddressSize(tpMailItem->To);
	}
	len += SetCcAddressSize(tpMailItem->Cc);
	len += SetCcAddressSize(tpMailItem->Bcc);
	if(ViewShowDate == 1){
		len += lstrlen(STR_VIEW_HEAD_DATE);
		if(tpMailItem->Date != NULL){
			len += lstrlen(tpMailItem->Date);
		}
	}

	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(buf == NULL){
		return;
	}
	*buf = TEXT('\0');

	//表示する文字列を作成する
	p = TStrJoin(buf, STR_VIEW_HEAD_FROM, tpMailItem->From, STR_VIEW_HEAD_SUBJECT, tpMailItem->Subject, (TCHAR *)-1);
	if(ToFlag == TRUE){
		p = SetCcAddress(TEXT("To"), tpMailItem->To, p);
	}
	p = SetCcAddress(TEXT("Cc"), tpMailItem->Cc, p);
	p = SetCcAddress(TEXT("Bcc"), tpMailItem->Bcc, p);
	if(ViewShowDate == 1){
		p = TStrJoin(p, STR_VIEW_HEAD_DATE, tpMailItem->Date, (TCHAR *)-1);
	}
	SetWindowText(hHeader, buf);
	LocalFree(buf);
}


/******************************************************************************

	SetHeaderSize

	メールヘッダのサイズ設定

******************************************************************************/

static void SetHeaderSize(HWND hWnd)
{
	HWND hHeader;
	HDC hdc;
	TEXTMETRIC lptm;
	RECT rcClient, StRect;
	int Height = 0;
	int FontHeight;
	int HLine;
	HMENU hMenu;
	HFONT hFont;
#ifndef _WIN32_WCE
	RECT ToolbarRect;
#endif

	hHeader = GetDlgItem(hWnd, IDC_HEADER);

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT);
#elif defined _WIN32_WCE_LAGENDA
	hMenu = GetSubMenu(hViewMenu, 1);
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0), 1);
#endif
#else
	hMenu = GetSubMenu(GetMenu(hWnd), 1);
#endif
	CheckMenuItem(hMenu, ID_MENUITEM_SHOW_DATE, (ViewShowDate == 1) ? MF_CHECKED : MF_UNCHECKED);

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	Height = g_menu_height;
#else
	Height = CommandBar_Height(GetDlgItem(hWnd, IDC_VCB));
#endif
#else
	GetWindowRect(GetDlgItem(hWnd, IDC_VTB), &ToolbarRect);
	Height = ToolbarRect.bottom - ToolbarRect.top;
#endif
	GetClientRect(hWnd, &rcClient);

	hdc = GetDC(hHeader);
#ifdef _WIN32_WCE
	hFont = (hListFont != NULL) ? SelectObject(hdc, hListFont) : NULL;
#else
	hFont = SelectObject(hdc, (hListFont != NULL) ? hListFont : GetStockObject(DEFAULT_GUI_FONT));
#endif
	//フォントの高さを取得
	GetTextMetrics(hdc, &lptm);
	if(hFont != NULL){
		SelectObject(hdc, hFont);
	}
	ReleaseDC(hHeader, hdc);
	HLine = (ViewShowDate == 1) ? 3 : 2;
	FontHeight = (lptm.tmHeight + lptm.tmExternalLeading) * HLine;

	//一時的に設定してサイズを再計算する
	MoveWindow(hHeader, 0, Height, rcClient.right, FontHeight, TRUE);
	GetClientRect(hHeader, &StRect);
	FontHeight = FontHeight + (FontHeight - StRect.bottom) + 1;
	MoveWindow(hHeader, 0, Height, rcClient.right, FontHeight, TRUE);

	InvalidateRect(hHeader, NULL, FALSE);
	UpdateWindow(hHeader);

	MoveWindow(GetDlgItem(hWnd, IDC_EDIT_BODY), 0, Height + FontHeight + 1,
#ifdef _WIN32_WCE_PPC
		rcClient.right, rcClient.bottom - Height - FontHeight, TRUE);
#else
		rcClient.right, rcClient.bottom - Height - FontHeight - 1, TRUE);
#endif
}


/******************************************************************************

	TbNotifyProc

	ツールバーの通知メッセージ (Win32)

******************************************************************************/

#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam)
{
	TOOLTIPTEXT *pTT;

	pTT = (TOOLTIPTEXT*)lParam;
	pTT->hinst = hInst;
	pTT->lpszText = MAKEINTRESOURCE(pTT->hdr.idFrom);
	return 0;
}
#endif


/******************************************************************************

	NotifyProc

	コントロールの通知メッセージ

******************************************************************************/

#ifndef _WIN32_WCE
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;

	if(CForm->code == TTN_NEEDTEXT){
		return TbNotifyProc(hWnd, lParam);
	}
	return FALSE;
}
#endif


/******************************************************************************

	FindEditString

	EDIT内の文字列を検索する

******************************************************************************/

static BOOL FindEditString(HWND hEdit, TCHAR *strFind, int CaseFlag)
{
	static DWORD dwPos;
	DWORD dwStart;
	DWORD dwEnd;
	TCHAR *buf = NULL;
	TCHAR *p;

	//検索位置の取得
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	//現在位置が前回検索位置と違う場合は現在位置を検索位置にする
	if((dwStart + 1U) != dwPos){
		dwPos = dwStart;
	}

	//エディットから文字列を取得する
	AllocGetText(hEdit, &buf);
	p = StrFind(strFind, buf + dwPos, CaseFlag);

	//検索文字列が見つからなかった場合
	if(*p == TEXT('\0')){
		LocalFree(buf);
		return FALSE;
	}

	//文字列が見つかった場合はその位置を選択状態にする
	dwPos = p - buf;
	SendMessage(hEdit, EM_SETSEL, dwPos, dwPos + lstrlen(strFind));
	SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
	LocalFree(buf);
	dwPos++;
	return TRUE;
}


/******************************************************************************

	SetWordBreakMenu

	折り返しのメニューのチェック切り替え

******************************************************************************/

void SetWordBreakMenu(HWND hWnd, HMENU hEditMenu, int Flag)
{
#ifdef _WIN32_WCE_PPC
	CheckMenuItem(hEditMenu, ID_MENUITEM_WORDBREAK, Flag);
#else	//_WIN32_WCE_PPC
	HMENU hMenu;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	hMenu = hEditMenu;
#else	//_WIN32_WCE_LAGENDA
	hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0);
#endif	//_WIN32_WCE_LAGENDA
#else	//_WIN32_WCE
	hMenu = GetMenu(hWnd);
#endif	//_WIN32_WCE
	CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_WORDBREAK, Flag);
#endif	//_WIN32_WCE_PPC
}


/******************************************************************************

	SetWordBreak

	折り返しの切り替え

******************************************************************************/

#ifdef _WIN32_WCE_LAGENDA
int SetWordBreak(HWND hWnd, HMENU hMenu)
#else
int SetWordBreak(HWND hWnd)
#endif
{
	HWND hEdit;
	RECT rcClient;
	RECT HeaderRect;
#ifndef _WIN32_WCE
	RECT ToolbarRect;
#endif
	TCHAR *buf;
	int hHeight = 0, tHeight = 0;
	int len, ret;
	int i;
	BOOL ModifyFlag;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_HEADER), &HeaderRect);
	hHeight = HeaderRect.bottom - HeaderRect.top;
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	tHeight = 0;
#else
	tHeight = CommandBar_Height(GetDlgItem(hWnd, IDC_VCB));
#endif
#else
	GetWindowRect(GetDlgItem(hWnd, IDC_VTB), &ToolbarRect);
	tHeight = ToolbarRect.bottom - ToolbarRect.top;
#endif

	hEdit = GetDlgItem(hWnd, IDC_EDIT_BODY);
	i = GetWindowLong(hEdit, GWL_STYLE);
	if(i & WS_HSCROLL){
		i ^= WS_HSCROLL;
#ifdef _WIN32_WCE_PPC
		SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), MF_CHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
		SetWordBreakMenu(hWnd, hMenu, MF_CHECKED);
#else
		SetWordBreakMenu(hWnd, NULL, MF_CHECKED);
#endif
		ret = 1;
	}else{
		i |= WS_HSCROLL;
#ifdef _WIN32_WCE_PPC
		SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
		SetWordBreakMenu(hWnd, hMenu, MF_UNCHECKED);
#else
		SetWordBreakMenu(hWnd, NULL, MF_UNCHECKED);
#endif
		ret = 0;
	}

	ModifyFlag = SendMessage(hEdit, EM_GETMODIFY, 0, 0);

	len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(buf == NULL){
		return !ret;
	}
	*buf = TEXT('\0');
	SendMessage(hEdit, WM_GETTEXT, len, (LPARAM)buf);

	DestroyWindow(hEdit);
	hEdit = CreateWindowEx(
#ifdef _WIN32_WCE_PPC
		0,
#else
		WS_EX_CLIENTEDGE,
#endif
		TEXT("EDIT"), TEXT(""), i,
		0, tHeight + hHeight + 1, rcClient.right,
#ifdef _WIN32_WCE_PPC
		rcClient.bottom - tHeight - hHeight,
#else
		rcClient.bottom - tHeight - hHeight - 1,
#endif
		hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
	SendMessage(hEdit, WM_SETFONT, (WPARAM)hEditFont, MAKELPARAM(TRUE,0));
	SetFocus(hEdit);

	SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)buf);
	SendMessage(hEdit, EM_SETMODIFY, (WPARAM)ModifyFlag, 0);
	LocalFree(buf);
	return ret;
}


/******************************************************************************

	SubClassEditProc

	サブクラス化したウィンドウプロシージャ

******************************************************************************/

static LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CHAR:
		if((TCHAR)wParam == TEXT(' ')){
			if(GetKeyState(VK_SHIFT) < 0){
				SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			}else if(GetKeyState(VK_CONTROL) < 0){
				View_NextNoReadMail(GetParent(hWnd));
			}else{
				View_NextScroll(hWnd);
			}
		}
	case WM_DEADCHAR:
	case WM_CUT:
	case WM_CLEAR:
	case WM_PASTE:
	case EM_UNDO:
		return 0;

	case WM_LBUTTONDBLCLK:
		SetTimer(GetParent(hWnd), ID_CLICK_TIMER, 100, NULL);
		break;

#ifdef _WIN32_WCE
	case WM_LBUTTONDOWN:
#ifdef _WIN32_WCE_PPC
		{
			SHRGINFO rg;

			rg.cbSize = sizeof(SHRGINFO);
			rg.hwndClient = hWnd;
			rg.ptDown.x = LOWORD(lParam);
			rg.ptDown.y = HIWORD(lParam);
			rg.dwFlags = SHRG_RETURNCMD;

			if(SHRecognizeGesture(&rg) == GN_CONTEXTMENU){
				SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENU, 0);
				return 0;
			}
		}
#else
		if(GetKeyState(VK_MENU) < 0){
			SetEditMenu(GetParent(hWnd));
			ShowMenu(GetParent(hWnd),
				CommandBar_GetMenu(GetDlgItem(GetParent(hWnd), IDC_VCB), 0), 1, 0, FALSE);
			return 0;
		}
		break;
#endif
#else
	case WM_CONTEXTMENU:
		SetEditMenu(GetParent(hWnd));
		ShowMenu(GetParent(hWnd), GetMenu(GetParent(hWnd)), 1, 0, FALSE);
		return 0;
#endif
	}
	return CallWindowProc(EditWindowProcedure, hWnd, msg, wParam, lParam);
}


/******************************************************************************

	SetEditSubClass

	ウィンドウのサブクラス化

******************************************************************************/

static void SetEditSubClass(HWND hWnd)
{
	SendMessage(hWnd, EM_LIMITTEXT, 1, 0);
	EditWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassEditProc);
}


/******************************************************************************

	DelEditSubClass

	ウィンドウクラスを標準のものに戻す

******************************************************************************/

static void DelEditSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)EditWindowProcedure);
	EditWindowProcedure = NULL;
}


/******************************************************************************

	InitWindow

	ウィンドウの初期化

******************************************************************************/

static BOOL InitWindow(HWND hWnd, struct TPMAILITEM *tpMailItem)
{
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
		ID_MENUITEM_CLOSE,		CSOBAR_COMMON_BUTTON,		CSO_BUTTON_DISP, (-1),					NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), CSO_ID_BACK,	CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		1,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, TEXT("ﾌｧｲﾙ"),	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		2,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, TEXT("編集"),	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		ID_MENUITEM_PREVMAIL,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_PREVMAIL,	NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_NEXTMAIL,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_NEXTMAIL,	NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_NEXTNOREAD,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_NEXTNOREAD,	NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_REMESSEGE,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_REMESSEGE,	NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
	};
	HWND hCSOBar;
	DWORD style;
#else	//_WIN32_WCE_LAGENDA
#ifndef _WIN32_WCE_PPC
	HWND hToolBar;
#endif	//_WIN32_WCE_PPC
	TBBUTTON tbButton[] = {
#ifdef _WIN32_WCE
#ifndef _WIN32_WCE_PPC
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
#endif	//_WIN32_WCE_PPC
#endif	//_WIN32_WCE
		{0,	ID_MENUITEM_PREVMAIL,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{1,	ID_MENUITEM_NEXTMAIL,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{2,	ID_MENUITEM_NEXTNOREAD,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{3,	ID_MENUITEM_REMESSEGE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{4,	ID_MENUITEM_ALLREMESSEGE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
	};
#ifdef _WIN32_WCE
	static TCHAR *szTips[] = {
#ifdef _WIN32_WCE_PPC
		NULL, //menu skipping
#endif	//_WIN32_WCE_PPC
		NULL, //menu skipping
		STR_CMDBAR_PREVMAIL,
		STR_CMDBAR_NEXTMAIL,
		STR_CMDBAR_NEXTNOREAD,
		STR_CMDBAR_REMESSEGE,
		STR_CMDBAR_ALLREMESSEGE,
	};
#ifdef _WIN32_WCE_PPC
	SHMENUBARINFO mbi;
#endif	//_WIN32_WCE_PPC
#endif	//_WIN32_WCE
#endif	//_WIN32_WCE_LAGENDA

	if(tpMailItem == NULL || tpMailItem->Body == NULL){
		return FALSE;
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	//PocketPC
	memset (&mbi, 0, sizeof (SHMENUBARINFO));
	mbi.cbSize     = sizeof (SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_VIEW;
	mbi.hInstRes   = hInst;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;
	SHCreateMenuBar(&mbi);
	hToolBar = mbi.hwndMB;

	CommandBar_AddToolTips(hToolBar, 7, szTips);
	CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR_VIEW, 5, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
#elif defined(_WIN32_WCE_LAGENDA)
	//BE-500
	hCSOBar = CSOBar_Create(hInst, hWnd, 1, BaseInfo);
	CSOBar_AddAdornments(hCSOBar, hInst, 1, CSOBAR_ADORNMENT_CLOSE, 0);

	hViewMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_VIEW));
	ButtonInfo[1].SubMenu = GetSubMenu(hViewMenu, 0);
	ButtonInfo[2].SubMenu = GetSubMenu(hViewMenu, 1);
	ButtonInfo[3].reshInst = hInst;
	ButtonInfo[4].reshInst = hInst;
	ButtonInfo[5].reshInst = hInst;
	ButtonInfo[6].reshInst = hInst;
	CSOBar_AddButtons(hCSOBar, hInst, sizeof(ButtonInfo) / sizeof(CSOBAR_BUTTONINFO), ButtonInfo);

	style = GetWindowLong(hCSOBar, GWL_STYLE);
	style &= ~WS_CLIPCHILDREN;
	SetWindowLong(hCSOBar, GWL_STYLE, style);

	g_menu_height = CSOBar_Height(hCSOBar);

#else
	//H/PC & PsPC
	hToolBar = CommandBar_Create(hInst, hWnd, IDC_VCB);
    CommandBar_AddToolTips(hToolBar, 6, szTips);
	if(GetSystemMetrics(SM_CXSCREEN) >= 450){
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_VIEW_HPC, 0);
	}else{
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_VIEW, 0);
	}
	CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR_VIEW, 5, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	CommandBar_AddAdornments(hToolBar, 0, 0);
#endif
#else
	//Win32
	hToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_VTB, 5, hInst, IDB_TOOLBAR_VIEW,
		tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, TB_ICONSIZE, TB_ICONSIZE, sizeof(TBBUTTON));
	SetWindowLong(hToolBar, GWL_STYLE, GetWindowLong(hToolBar, GWL_STYLE) | TBSTYLE_FLAT);
	SendMessage(hToolBar, TB_SETINDENT, 5, 0);
	ShowWindow(hToolBar, SW_SHOW);
#endif

	//ヘッダを表示するSTATICコントロールの作成
	CreateWindowEx(
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
		0,
#else
		WS_EX_CLIENTEDGE,
#endif
#else
		WS_EX_STATICEDGE,
#endif
		TEXT("STATIC"), TEXT(""),
		WS_VISIBLE | WS_CHILD | SS_LEFTNOWORDWRAP | SS_NOPREFIX,
		0, 0, 0, 0, hWnd, (HMENU)IDC_HEADER, hInst, NULL);
	//フォントの設定
#ifdef _WIN32_WCE
	if(hListFont != NULL){
		SendDlgItemMessage(hWnd, IDC_HEADER, WM_SETFONT, (WPARAM)hListFont, MAKELPARAM(TRUE,0));
	}
#else
	SendDlgItemMessage(hWnd, IDC_HEADER, WM_SETFONT,
		(WPARAM)((hListFont != NULL) ? hListFont : GetStockObject(DEFAULT_GUI_FONT)), MAKELPARAM(TRUE,0));
#endif

	CreateWindowEx(
#ifdef _WIN32_WCE_PPC
		0,
#else
		WS_EX_CLIENTEDGE,
#endif
		TEXT("EDIT"), TEXT(""),
#ifdef _WIN32_WCE_PPC
		WS_BORDER |
#endif
		WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ((WordBreakFlag == 1) ? 0 : WS_HSCROLL),
		0, 0, 0, 0, hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hEditFont, MAKELPARAM(TRUE,0));
	SetHeaderSize(hWnd);
#ifndef UNICODE
	if(UnicodeEdit == 0){
		int i, j;
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT("あ"));
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
		UnicodeEdit = (j != 2) ? 2 : 1;
	}
#endif

	SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE_PPC
	SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), (WordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
	SetWordBreakMenu(hWnd, hViewMenu, (WordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
	SetWordBreakMenu(hWnd, NULL, (WordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

#ifdef _WIN32_WCE
	SendMessage(hWnd, WM_SETICON, (WPARAM)FALSE,
		(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_READ), IMAGE_ICON, 16, 16, 0));
#endif

	//IMEをオフにする
	ImmAssociateContext(GetDlgItem(hWnd, IDC_EDIT_BODY), (HIMC)NULL);

	SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
	SetTimer(hWnd, ID_HIDECARET_TIMER, 10, NULL);
	return TRUE;
}


/******************************************************************************

	SetWindowSize

	ウィンドウのサイズ変更

******************************************************************************/

static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	RECT rcClient, HeaderRect;
	int hHeight = 0;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_HEADER), &HeaderRect);
	hHeight = HeaderRect.bottom - HeaderRect.top;

	MoveWindow(GetDlgItem(hWnd, IDC_EDIT_BODY), 0, hHeight + 1,
		rcClient.right, rcClient.bottom - hHeight, TRUE);
	return TRUE;
#elif defined _WIN32_WCE_LAGENDA
	COSIPINFO CoSipInfo;
	SIPINFO SipInfo;
	RECT rcClient, HeaderRect;
	int sip_height = 0;
	int hHeight = 0;
	int ret;

	memset(&SipInfo, 0, sizeof(SIPINFO));
	SipInfo.cbSize = sizeof(SIPINFO);
	memset(&CoSipInfo, 0, sizeof(COSIPINFO));
	CoSipInfo.dwInfoType = COSIP_GET_KBTYPE;
	SipInfo.pvImData = &CoSipInfo;
	SipInfo.dwImDataSize = sizeof(COSIPINFO);
	SipGetInfo(&SipInfo);
	if(SipInfo.fdwFlags & SIPF_ON){
		sip_height = ((SipInfo.rcSipRect).bottom - (SipInfo.rcSipRect).top);
		ret = TRUE;
	}else{
		ret = FALSE;
	}

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_HEADER), &HeaderRect);
	hHeight = HeaderRect.bottom - HeaderRect.top;
	hHeight += g_menu_height;

	MoveWindow(GetDlgItem(hWnd, IDC_EDIT_BODY), 0, hHeight + 1,
		rcClient.right, rcClient.bottom - hHeight - sip_height, TRUE);
	return ret;
#else
	return TRUE;
#endif
#else
	HWND hHeader, hBody;
	RECT rcClient, HeaderRect, ToolbarRect;
	int hHeight = 0, tHeight = 0;

	hHeader = GetDlgItem(hWnd, IDC_HEADER);
	hBody = GetDlgItem(hWnd, IDC_EDIT_BODY);

	SendDlgItemMessage(hWnd, IDC_VTB, WM_SIZE, wParam, lParam);

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hHeader, &HeaderRect);
	hHeight = HeaderRect.bottom - HeaderRect.top;
	GetWindowRect(GetDlgItem(hWnd, IDC_VTB), &ToolbarRect);
	tHeight = ToolbarRect.bottom - ToolbarRect.top;

	MoveWindow(hHeader, 0, tHeight, rcClient.right, hHeight, TRUE);
	InvalidateRect(hHeader, NULL, FALSE);
	UpdateWindow(hHeader);

	MoveWindow(hBody, 0, tHeight + hHeight + 1,
		rcClient.right, rcClient.bottom - tHeight - hHeight - 1, TRUE);
	return TRUE;
#endif
}


/******************************************************************************

	EndWindow

	ウィンドウの終了処理

******************************************************************************/

static void EndWindow(HWND hWnd)
{
	FreeMultipartInfo(&tpMultiPart, MultiPartCnt);
	tpMultiPart = NULL;
	MultiPartCnt = 0;

	DelEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
    DestroyWindow(hToolBar);
#elif defined _WIN32_WCE_LAGENDA
	DestroyMenu(hViewMenu);
#else
	CommandBar_Destroy(GetDlgItem(hWnd, IDC_VCB));
#endif
	DestroyIcon((HICON)SendMessage(hWnd, WM_GETICON, FALSE, 0));
#else
	DestroyWindow(GetDlgItem(hWnd, IDC_VTB));
#endif
	DestroyWindow(GetDlgItem(hWnd, IDC_HEADER));
	DestroyWindow(GetDlgItem(hWnd, IDC_EDIT_BODY));

	DestroyWindow(hWnd);
}


/******************************************************************************

	SetEditMenu

	編集メニューの活性／非活性の切り替え

******************************************************************************/

static void SetEditMenu(HWND hWnd)
{
	HMENU hMenu;
	int i, j;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 1);
#else
	hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0);
#endif
#else
	hMenu = GetMenu(hWnd);
#endif

	//エディットボックスの選択位置の取得
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
	EnableMenuItem(hMenu, ID_MENUITEM_COPY, (i < j) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_MENUITEM_FIND, (SelBox != MAILBOX_SEND) ? MF_ENABLED : MF_GRAYED);
}


/******************************************************************************

	SetAttachMenu

	表示するpartの選択と添付メニューの設定

******************************************************************************/

static int SetAttachMenu(HWND hWnd, struct TPMAILITEM *tpMailItem, BOOL ViewSrc)
{
	HMENU hMenu;
	TCHAR *str;
	TCHAR *p, *r;
	int i, ret = -1;
	int mFlag;
	BOOL AppendFlag = FALSE;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 1);
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0), 1);
#endif
#else
	hMenu = GetSubMenu(GetMenu(hWnd), 1);
#endif

	//メニューを初期化する
	while(DeleteMenu(hMenu, 8, MF_BYPOSITION) == TRUE);

	if(MultiPartCnt == 1 &&
		(*tpMultiPart)->sPos == tpMailItem->Body && (*tpMultiPart)->ePos == NULL &&
		(*tpMultiPart)->ContentType != NULL &&
		TStrCmpNI((*tpMultiPart)->ContentType, TEXT("text"), lstrlen(TEXT("text"))) == 0){
		return 0;
	}
	if(MultiPartCnt == 1 && ViewSrc == TRUE){
#ifdef _WIN32_WCE
		AppendMenu(hMenu, MF_STRING, ID_VIEW_PART,
			(PPCFlag == TRUE) ? STR_VIEW_PPCMENU_ATTACH : STR_VIEW_MENU_ATTACH);
#else
		AppendMenu(hMenu, MF_STRING, ID_VIEW_PART, STR_VIEW_MENU_ATTACH);
#endif
		return 0;
	}

	for(i = 0; i < MultiPartCnt; i++){
		if(ret == -1 && ((*(tpMultiPart + i))->ContentType == NULL ||
			TStrCmpNI((*(tpMultiPart + i))->ContentType, TEXT("text"), lstrlen(TEXT("text"))) == 0)){
			//一番目に出現したテキストデータは本文にする
			ret = i;
			if(MultiPartCnt == 1 && tpMailItem->Multipart == TRUE){
				AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
			}
		}else{
			if(AppendFlag == FALSE){
				if(MultiPartCnt > 1){
#ifdef _WIN32_WCE
					AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE,
						(PPCFlag == TRUE) ? STR_VIEW_PPCMENU_SOURCE : STR_VIEW_MENU_SOURCE);
					AppendMenu(hMenu, MF_STRING, ID_VIEW_DELETE_ATTACH,
						(PPCFlag == TRUE) ? STR_VIEW_PPCMENU_DELATTACH : STR_VIEW_MENU_DELATTACH);
#else
					AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
					AppendMenu(hMenu, MF_STRING, ID_VIEW_DELETE_ATTACH, STR_VIEW_MENU_DELATTACH);
#endif
				}
				//区切りの追加
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendFlag = TRUE;
			}
			//途中で切れているデータの場合はメニューを非活性にする
			mFlag = (i != 0 && (*(tpMultiPart + i))->ePos == NULL) ? MF_GRAYED : MF_ENABLED;

			//ファイル名をメニューに追加する
			str = AllocCopy((*(tpMultiPart + i))->ContentType);
			if(str != NULL){
				for(r = str; *r != TEXT('\0') && *r != TEXT(';'); r++);
				*r = TEXT('\0');
			}
			p = ((*(tpMultiPart + i))->Filename != NULL) ? (*(tpMultiPart + i))->Filename : str;
			AppendMenu(hMenu, MF_STRING | mFlag, ID_ATTACH + i, ((p != NULL && *p != TEXT('\0')) ? p : TEXT("Attach")));
			NULLCHECK_FREE(str);
		}
	}
	return ret;
}


/******************************************************************************

	ModfyWindow

	内容の変更

******************************************************************************/

static void ModfyWindow(HWND hWnd, struct TPMAILITEM *tpMailItem, BOOL ViewSrc)
{
	TCHAR *buf;
	int LvFocus;
#ifndef _WIN32_WCE
	OSVERSIONINFO os_info;
#endif

	if(tpMailItem == NULL || tpMailItem->Body == NULL){
		return;
	}

	SwitchCursor(FALSE);

	vSelBox = SelBox;
	LvFocus = ListView_GetNextItem(GetDlgItem(MainWnd, IDC_LISTVIEW), -1, LVNI_FOCUSED);

	//開封済みにする
	if(tpMailItem->MailStatus != ICON_NON && tpMailItem->MailStatus < ICON_SENDMAIL){
		tpMailItem->MailStatus = ICON_READ;
	}

	//一覧のアイコンの設定
	if(tpMailItem->Status != ICON_DOWN && tpMailItem->Status != ICON_DEL){
		tpMailItem->Status = tpMailItem->MailStatus;
		ListView_RedrawItems(GetDlgItem(MainWnd, IDC_LISTVIEW), LvFocus, LvFocus);
		UpdateWindow(GetDlgItem(MainWnd, IDC_LISTVIEW));
	}

	//ウィンドウタイトルの設定
	SetWindowString(hWnd, (MailBox + SelBox)->Name , tpMailItem->MailBox, tpMailItem->No);
	//ヘッダの設定
	SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
	UpdateWindow(GetDlgItem(hWnd, IDC_HEADER));

	SetItemCntStatusText(MainWnd, NULL);

	FreeMultipartInfo(&tpMultiPart, MultiPartCnt);
	tpMultiPart = NULL;
	MultiPartCnt = 0;
	//マルチパートの展開
	buf = BodyDecode(tpMailItem, ViewSrc, &tpMultiPart, &MultiPartCnt);

	//表示するpartの選択と添付メニューの設定
	SetAttachMenu(hWnd, tpMailItem, ViewSrc);

	//本文の表示
	if(buf != NULL){
#ifdef _WIN32_WCE
		if(lstrlen(buf) > EDITMAXSIZE){
			*(buf + EDITMAXSIZE) = TEXT('\0');
		}
#else
		//OS情報の取得
		os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&os_info);

		if(os_info.dwPlatformId != VER_PLATFORM_WIN32_NT && lstrlen(buf) > EDITMAXSIZE){
			*(buf + EDITMAXSIZE) = TEXT('\0');
		}
#endif
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)buf);
		LocalFree(buf);
	}else{
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)TEXT(""));
	}

	ShowWindow(hWnd, SW_SHOW);
#ifndef _WIN32_WCE
	if(IsIconic(hWnd) != 0){
		ShowWindow(hWnd, SW_RESTORE);
	}
#endif
	HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
	SwitchCursor(TRUE);
}


/******************************************************************************

	View_NextMail

	次のメールを表示

******************************************************************************/

static struct TPMAILITEM *View_NextMail(HWND hWnd, BOOL St)
{
	struct TPMAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int j;

	if(SelBox == MAILBOX_SEND){
		return NULL;
	}
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	if(St == FALSE){
		Index = ListView_GetMemToItem(hListView,
			(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));
	}else{
		Index = -1;
	}

	j = ListView_GetNextMailItem(hListView, Index);
	if(j == -1){
		return NULL;
	}
	ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
	ListView_SetItemState(hListView,
		j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_EnsureVisible(hListView, j, TRUE);

	tpMailItem = (struct TPMAILITEM *)ListView_GetlParam(hListView, j);
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModfyWindow(hWnd, tpMailItem, FALSE);
	return tpMailItem;
}


/******************************************************************************

	View_PrevMail

	前のメールを表示

******************************************************************************/

static void View_PrevMail(HWND hWnd)
{
	struct TPMAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int j;

	if(SelBox == MAILBOX_SEND){
		return;
	}
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	Index = ListView_GetMemToItem(hListView,
		(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));

	j = ListView_GetPrevMailItem(hListView, Index);
	if(j == -1){
		return;
	}
	ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
	ListView_SetItemState(hListView,
		j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_EnsureVisible(hListView, j, TRUE);

	tpMailItem = (struct TPMAILITEM *)ListView_GetlParam(hListView, j);
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModfyWindow(hWnd, tpMailItem, FALSE);
}


/******************************************************************************

	View_NextNoReadMail

	次の未開封メールを表示

******************************************************************************/

static void View_NextNoReadMail(HWND hWnd)
{
	struct TPMAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int i, j;

	if(SelBox == MAILBOX_SEND){
		return;
	}
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	Index = ListView_GetMemToItem(hListView,
		(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));

	//未開封メールのインデックスを取得
	j = ListView_GetNextNoReadItem(hListView, Index,
		ListView_GetItemCount(hListView));
	if(j == -1){
		j = ListView_GetNextNoReadItem(hListView, -1, Index);
	}
	if(j == -1){
		if(MoveAllMailBox == 0){
			return;
		}

		//次の未開封があるメールボックスのインデックスを取得
		SwitchCursor(FALSE);
		i = NextNoReadMailBox(SelBox + 1, MailBoxCnt);
		if(i == -1){
			i = NextNoReadMailBox(MAILBOX_USER, SelBox);
		}
		if(i == -1){
			SwitchCursor(TRUE);
			return;
		}
		//メールボックスの選択
		SelectMailBox(MainWnd, i);
		j = ListView_GetNextNoReadItem(hListView, -1,
			ListView_GetItemCount(hListView));
		SwitchCursor(TRUE);
	}
	ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
	ListView_SetItemState(hListView,
		j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_EnsureVisible(hListView, j, TRUE);

	tpMailItem = (struct TPMAILITEM *)ListView_GetlParam(hListView, j);
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModfyWindow(hWnd, tpMailItem, FALSE);
	SendMessage(MainWnd, WM_INITTRAYICON, 0, 0);
}


/******************************************************************************

	View_NextScroll

	表示メールをPageDownして一番下に来たら次ぎの未開封メールに移動する

******************************************************************************/

static void View_NextScroll(HWND hEditWnd)
{
	TEXTMETRIC lptm;
	SCROLLINFO ScrollInfo;
	RECT rRect;
	HDC hdc;
	HFONT hFont;
	BOOL Next = FALSE;

	//フォントの高さを取得
	hdc = GetDC(hEditWnd);
	hFont = SelectObject(hdc, hEditFont);

	GetTextMetrics(hdc, &lptm);

	SelectObject(hdc, hFont);
	ReleaseDC(hEditWnd, hdc);

	//スクロール情報の取得
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_POS | SIF_RANGE;
	GetScrollInfo(hEditWnd, SB_VERT, &ScrollInfo);

	GetClientRect(hEditWnd,(LPRECT)&rRect);
	ScrollInfo.nMax -= ((rRect.bottom - rRect.top) / (lptm.tmHeight + lptm.tmExternalLeading));

	if(ScrollInfo.nPos >= ScrollInfo.nMax){
		Next = TRUE;
	}

	//PageDownを行う
	SendMessage(hEditWnd, WM_VSCROLL, SB_PAGEDOWN, 0);

	if(Next == TRUE){
		//次の未開封メールへ移動する
		View_NextNoReadMail(GetParent(hEditWnd));
	}
}


/******************************************************************************

	View_FindMail

	メール内から文字列を検索

******************************************************************************/

void View_FindMail(HWND hWnd, BOOL FindSet)
{
	struct TPMAILITEM *tpMailItem;
	HWND hEdit;
	TCHAR *buf = NULL;
	TCHAR *p;
	DWORD dwStart;
	DWORD dwEnd;

	if(SelBox == MAILBOX_SEND){
		return;
	}

	hEdit = GetDlgItem(hWnd, IDC_EDIT_BODY);
	if(FindSet == TRUE || FindStr == NULL){
		//選択文字列を取得
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		if(dwStart != dwEnd){
			AllocGetText(hEdit, &buf);
			if(buf != NULL){
				NULLCHECK_FREE(FindStr);
				FindStr = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (dwEnd - dwStart + 1));
				if(FindStr != NULL){
					TStrCpyN(FindStr, buf + dwStart, dwEnd - dwStart + 1);
				}
				LocalFree(buf);
			}
		}

		//検索条件の設定
		if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_FIND), hWnd, SetFindProc) == FALSE){
			return;
		}
	}

	while(1){
		//本文から検索して見つかった位置を選択状態にする
		if(FindEditString(hEdit, FindStr, MstchCase) == TRUE){
			break;
		}

		//１メール内での検索の場合
		if(AllFind == 0){
			SendMessage(hEdit, EM_SETSEL, 0, 0);
			if(FindEditString(hEdit, FindStr, MstchCase) == TRUE){
				break;
			}
			buf = (TCHAR *)LocalAlloc(LMEM_FIXED,
				sizeof(TCHAR) * (lstrlen(FindStr) + lstrlen(STR_MSG_NOFIND) + 1));
			if(buf == NULL){
				break;
			}
			wsprintf(buf, STR_MSG_NOFIND, FindStr);
			MessageBox(hWnd, buf, STR_TITLE_FIND, MB_ICONINFORMATION);
			LocalFree(buf);
			break;
		}

		//次のメールに移動
		tpMailItem = View_NextMail(hWnd, FALSE);
		if(tpMailItem == NULL){
			buf = (TCHAR *)LocalAlloc(LMEM_FIXED,
				sizeof(TCHAR) * (lstrlen(FindStr) + lstrlen(STR_TITLE_ALLFIND) + 1));
			if(buf == NULL){
				break;
			}
			wsprintf(buf, STR_TITLE_ALLFIND, FindStr);
			if(MessageBox(hWnd, STR_Q_NEXTFIND, buf, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO){
				LocalFree(buf);
				break;
			}
			LocalFree(buf);
			tpMailItem = View_NextMail(hWnd, TRUE);
		}

		//ESC が押された場合は検索終了
		if(GetForegroundWindow() == hWnd && GetAsyncKeyState(VK_ESCAPE) < 0){
			ESCFlag = TRUE;
			break;
		}

		//Subjectから検索
		if(SubjectFind != 0 && tpMailItem->Subject != NULL){
			p = StrFind(FindStr, tpMailItem->Subject, MstchCase);
			if(*p != TEXT('\0')){
				break;
			}
		}
	}
}


/******************************************************************************

	ShellOpen

	ファイルを関連付けで実行

******************************************************************************/

static BOOL ShellOpen(TCHAR *FileName)
{
#ifndef _WIN32_WCE_LAGENDA
	SHELLEXECUTEINFO sei;

	tZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(sei);
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	if(URLApp == NULL || *URLApp == TEXT('\0')){
		sei.lpFile = FileName;
		sei.lpParameters = NULL;
	}else{
		sei.lpFile = URLApp;
		sei.lpParameters = FileName;
	}
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = hInst;
	return ShellExecuteEx(&sei);
#else
	TCHAR *file;
	TCHAR *param;

	if(URLApp == NULL || *URLApp == TEXT('\0')){
		file = FileName;
		param = NULL;
	}else{
		file = URLApp;
		param = FileName;
	}
	return CoshExecute(NULL, file, param);
#endif
}


/******************************************************************************

	OpenURL

	エディットボックスから選択されたURLを抽出して開く

******************************************************************************/

static void OpenURL(HWND hWnd)
{
	TCHAR *buf;
	TCHAR *str;
	TCHAR *p, *r, *s;
	int i, j;
	int len;
	int MailToFlag = 0;

	//エディットボックスの選択位置の取得
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
	if(i >= j){
		return;
	}

	//エディットボックスに設定された文字列の取得
	len = SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXTLENGTH, 0, 0) + 1;
	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(buf == NULL){
		return;
	}
	*buf = TEXT('\0');
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXT, len, (LPARAM)buf);

#ifdef UNICODE
	str = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (j - i + 2));
	if(str == NULL){
		LocalFree(buf);
		return;
	}
	//選択文字列の抽出
	for(p = buf + i, r = str; p < (buf + j); p++, r++){
		*r = *p;
	}
	*r = TEXT('\0');
	LocalFree(buf);
#else
	if(UnicodeEdit == 2){
		//WindowsXP
		WCHAR *wbuf, *wstr;
		WCHAR *wst, *wen, *wr;

		len = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
		wbuf = (WCHAR *)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * (len + 1));
		if(wbuf == NULL){
			LocalFree(buf);
			return;
		}
		MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, len);

		for(wst = wbuf + i; wst > wbuf && *wst != L'\r' && *wst != L'\n' && *wst != L'\t' && *wst != L' '; wst--);
		if(wst != wbuf) wst++;
		for(wen = wst; *wen != L'\0' && *wen != L'\r' && *wen != L'\n' && *wen != L'\t' && *wen != L' '; wen++);

		wstr = (WCHAR *)LocalAlloc(LPTR, sizeof(WCHAR) * (wen - wst + 1));
		if(wstr == NULL){
			LocalFree(wbuf);
			LocalFree(buf);
			return;
		}
		//選択文字列の抽出
		for(wr = wstr; wst < wen; wst++, wr++){
			*wr = *wst;
		}
		*wr = L'\0';
		LocalFree(wbuf);
		LocalFree(buf);

		len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
		str = (char *)LocalAlloc(LMEM_FIXED, len + 1);
		if(str == NULL){
			LocalFree(wstr);
			return;
		}
		WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
		LocalFree(wstr);
	}else{
		str = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (j - i + 2));
		if(str == NULL){
			LocalFree(buf);
			return;
		}
		//選択文字列の抽出
		for(p = buf + i, r = str; p < (buf + j); p++, r++){
			*r = *p;
		}
		*r = TEXT('\0');
		LocalFree(buf);
	}
#endif

	//開始位置の取得
	for(s = str; *s == TEXT('(') || *s == TEXT(')') || *s == TEXT('\"') ||
		*s == TEXT('<') || *s == TEXT('>') || *s == TEXT('\t') || *s == TEXT(' '); s++);
	//URLの開始位置を取得
	for(p = s; *p != TEXT('\0'); p++){
		if(TStrCmpNI(p, URL_HTTP, lstrlen(URL_HTTP)) == 0 ||
			TStrCmpNI(p, URL_HTTPS, lstrlen(URL_HTTPS)) == 0 ||
			TStrCmpNI(p, URL_FTP, lstrlen(URL_FTP)) == 0 ||
			TStrCmpNI(p, URL_MAILTO, lstrlen(URL_MAILTO)) == 0){
			s = p;
			break;
		}
	}
	//終了位置の取得
	for(p = s; *p != TEXT('\0'); p++){
		if(*p == TEXT('(') || *p == TEXT(')') || *p == TEXT('\"') ||
			*p == TEXT('<') || *p == TEXT('>') ||
			*p == TEXT('\r') || *p == TEXT('\n') || *p == TEXT('\t') ||
			*p == TEXT(' ')){ // || IsDBCSLeadByte((BYTE)*p) == TRUE){
			*p = TEXT('\0');
			break;
		}
		if(*p == TEXT('@')){
			if(MailToFlag != 0){
				MailToFlag = -1;
			}else{
				MailToFlag = 1;
			}
		}
	}
	if(*s == TEXT('\0')){
		LocalFree(str);
		return;
	}

	//URLのチェック
	if(TStrCmpNI(s, URL_HTTP, lstrlen(URL_HTTP)) == 0 ||
		TStrCmpNI(s, URL_HTTPS, lstrlen(URL_HTTPS)) == 0 ||
		TStrCmpNI(s, URL_FTP, lstrlen(URL_FTP)) == 0){
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)i, (LPARAM)i);
		ShellOpen(s);

	}else if(TStrCmpNI(s, URL_MAILTO, lstrlen(URL_MAILTO)) == 0 ||
		MailToFlag == 1){
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)i, (LPARAM)i);
		if(Edit_MailToSet(hInst, hWnd, s, vSelBox) == EDIT_INSIDEEDIT){
#ifdef _WIN32_WCE
			ShowWindow(hWnd, SW_HIDE);
#endif
		}
	}
	LocalFree(str);
}


/******************************************************************************

	SetReMessage

	返信の設定を行う

******************************************************************************/

static void SetReMessage(HWND hWnd, int ReplyFag)
{
	int ret;
	if(Item_IsMailBox(MailBox + vSelBox,
		(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
		ErrorMessage(hWnd, STR_ERR_NOMAIL);
		return;
	}

	ret = Edit_InitInstance(hInst, hWnd, vSelBox,
		(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA), EDIT_REPLY, ReplyFag);
#ifdef _WIN32_WCE
	if(ret == EDIT_INSIDEEDIT){
		ShowWindow(hWnd, SW_HIDE);
	}
#else
	if(ret != EDIT_NONEDIT){
		if(ViewClose == 1){
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
#endif
}


/******************************************************************************

	Decode

	添付ファイルをデコードして保存

******************************************************************************/

static BOOL Decode(HWND hWnd, int id)
{
	TCHAR *str, *fname, *ext = NULL;
	TCHAR *p, *r;
	char *b64str, *dstr, *endpoint;
	int len;
	int EncodeFlag = 0;

	if((*(tpMultiPart + id))->ePos != NULL){
		len = (*(tpMultiPart + id))->ePos - (*(tpMultiPart + id))->sPos;
	}else{
		len = lstrlen((*(tpMultiPart + id))->sPos) + 1;
	}
	str = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(str == NULL){
		SwitchCursor(TRUE);
		return FALSE;
	}
	if(len == 0){
		*str = TEXT('\0');
	}else{
		TStrCpyN(str, (*(tpMultiPart + id))->sPos, len - 1);
	}

#ifdef UNICODE
	//TCHAR から char に変換
	b64str = AllocTcharToChar(str);
	LocalFree(str);
	if(b64str == NULL){
		SwitchCursor(TRUE);
		return FALSE;
	}
#else
	b64str = str;
#endif

	if((*(tpMultiPart + id))->Encoding != NULL){
		if(TStrCmpI((*(tpMultiPart + id))->Encoding, TEXT("base64")) == 0){
			EncodeFlag = 1;
		}else if(TStrCmpI((*(tpMultiPart + id))->Encoding, TEXT("quoted-printable")) == 0){
			EncodeFlag = 2;
		}
	}
	switch(EncodeFlag)
	{
	case 1:
	case 2:
		//エンコードされている場合はデコードを行う
		dstr = (char *)LocalAlloc(LMEM_FIXED, tstrlen(b64str));
		if(dstr == NULL){
			LocalFree(b64str);
			SwitchCursor(TRUE);
			return FALSE;
		}
		endpoint = ((EncodeFlag == 1) ? Base64Decode : QuotedPrintableDecode)(b64str, dstr);
		LocalFree(b64str);
		break;

	default:
		dstr = b64str;
		endpoint = dstr + tstrlen(dstr);
		break;
	}

	str = AllocCopy((*(tpMultiPart + id))->ContentType);
	if(str != NULL){
		for(r = str; *r != TEXT('\0') && *r != TEXT(';'); r++);
		*r = TEXT('\0');
	}

	p = GetMIME2Extension(str, NULL);
	if(p != NULL){
		ext = AllocCopy(p + 1);
	}

	if((*(tpMultiPart + id))->Filename == NULL){
		//ファイル名が無い場合
		fname = (p != NULL) ? p : AllocCopy(str);
	}else{
		NULLCHECK_FREE(p);
		fname = AllocCopy((*(tpMultiPart + id))->Filename);
	}
	NULLCHECK_FREE(str);

	if(fname != NULL){
		//ファイル名が長すぎる場合は途中で切る
		if(lstrlen(fname) > BUF_SIZE - 50){
			*(fname + BUF_SIZE - 50) = TEXT('\0');
		}
		//ファイル名に使えない文字は _ に変換する
		ConvFilename(fname);
	}

	SwitchCursor(TRUE);
	//保存を行う
	if(SaveFile(hWnd, fname, ext, dstr, endpoint - dstr) == FALSE){
		NULLCHECK_FREE(ext);
		NULLCHECK_FREE(fname);
		LocalFree(dstr);
		return FALSE;
	}
	NULLCHECK_FREE(ext);
	NULLCHECK_FREE(fname);
	LocalFree(dstr);
	return TRUE;
}


/******************************************************************************

	DeleteAttachFile

	添付ファイルの削除

******************************************************************************/

static BOOL DeleteAttachFile(HWND hWnd, struct TPMAILITEM *tpMailItem)
{
	struct TPMULTIPART **tpPart = NULL;
	TCHAR *mBody;
	int cnt = 0;
	int TextIndex = -1;
	int i;

	//マルチパートの解析
	cnt = MultiPart_Parse(tpMailItem->ContentType, tpMailItem->Body, &tpPart, 0);
	if(cnt == 0){
		return FALSE;
	}

	//テキストのパートを検索
	for(i = 0; i < cnt; i++){
		if((*tpPart + i)->ContentType == NULL ||
			TStrCmpNI((*tpPart + i)->ContentType, TEXT("text"), lstrlen(TEXT("text"))) == 0){
			TextIndex = i;
			break;
		}
	}
	if(TextIndex == -1){
		FreeMultipartInfo(&tpPart, cnt);
		return FALSE;
	}
	if((*tpPart + TextIndex)->ePos == NULL){
		mBody = AllocCopy((*tpPart + TextIndex)->sPos);
		if(mBody == NULL){
			FreeMultipartInfo(&tpPart, cnt);
			return FALSE;
		}
	}else{
		i = (*tpPart + TextIndex)->ePos - (*tpPart + TextIndex)->sPos;
		mBody = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (i + 1));
		if(mBody == NULL){
			FreeMultipartInfo(&tpPart, cnt);
			return FALSE;
		}
		if(i == 0){
			*mBody = TEXT('\0');
		}else{
			TStrCpyN(mBody, (*tpPart + TextIndex)->sPos, i - 1);
		}
	}

	//内容の置き換え
	NULLCHECK_FREE(tpMailItem->Body);
	tpMailItem->Body = mBody;

	NULLCHECK_FREE(tpMailItem->Encoding);
	tpMailItem->Encoding = AllocCopy((*tpPart + TextIndex)->Encoding);
	NULLCHECK_FREE(tpMailItem->ContentType);
	tpMailItem->ContentType = AllocCopy((*tpPart + TextIndex)->ContentType);

	tpMailItem->Attach = AllocCopy(TEXT("_"));

	FreeMultipartInfo(&tpPart, cnt);
	return TRUE;
}


/******************************************************************************

	SaveViewMail

	メールをファイルに保存

******************************************************************************/

static BOOL SaveViewMail(TCHAR *fname, HWND hWnd, int MailBoxIndex, struct TPMAILITEM *tpMailItem, TCHAR *head)
{
	struct TPMULTIPART **tpPart = NULL;
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR *buf;
	TCHAR *tmp;
	int len = 0;
	int cnt = 0;

	if(fname == NULL){
		//ファイルの作成
		if(tpMailItem->Subject != NULL){
			TStrCpyN(path, tpMailItem->Subject, BUF_SIZE - 50);
			ConvFilename(path);
			TStrCpy(path + lstrlen(path), TEXT(".txt"));
		}else{
			lstrcpy(path, TEXT(".txt"));
		}
		//ファイル名の取得
		if(GetFileName(hWnd, path, TEXT("txt"), STR_TEXT_FILTER, FALSE) == FALSE){
			return TRUE;
		}
		if(Item_IsMailBox(MailBox + MailBoxIndex, tpMailItem) == FALSE){
			return FALSE;
		}
		fname = path;
	}

	//保存するファイルを開く
	hFile = CreateFile(fname, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	//ヘッダの保存
	len = CreateHeaderStringSize(head, tpMailItem);
	tmp = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(tmp == NULL){
		CloseHandle(hFile);
		return FALSE;
	}
	CreateHeaderString(head, tmp, tpMailItem);

	if(WriteAsciiFile(hFile, tmp, len) == FALSE){
		LocalFree(tmp);
		CloseHandle(hFile);
		return FALSE;
	}
	LocalFree(tmp);

	buf = BodyDecode(tpMailItem, FALSE, &tpPart, &cnt);
	FreeMultipartInfo(&tpPart, cnt);

	//本文の保存
	if(buf != NULL && WriteAsciiFile(hFile, buf, lstrlen(buf)) == FALSE){
		CloseHandle(hFile);
		NULLCHECK_FREE(buf);
		return FALSE;
	}
	CloseHandle(hFile);
	NULLCHECK_FREE(buf);
	return TRUE;
}


/******************************************************************************

	AppViewMail

	メールをファイルに保存して外部アプリケーションで表示

******************************************************************************/

static BOOL AppViewMail(struct TPMAILITEM *tpMailItem)
{
	struct TPMULTIPART **tpPart = NULL;
#ifndef _WIN32_WCE_LAGENDA
	SHELLEXECUTEINFO sei;
#else
	TCHAR *file;
	TCHAR *param;
#endif
	TCHAR path[BUF_SIZE];
	TCHAR *p;
	int len = 0;
	int cnt = 0;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	//メールをファイルに保存
	TStrJoin(path, DataDir, VIEW_FILE, TEXT("."), ViewFileSuffix, (TCHAR *)-1);
	if(SaveViewMail(path, NULL, 0, tpMailItem, ViewFileHeader) == FALSE){
		return FALSE;
	}

#ifndef _WIN32_WCE_LAGENDA
	tZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(sei);
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;

	p = NULL;
	if(ViewApp == NULL || *ViewApp == TEXT('\0')){
		sei.lpFile = path;
		sei.lpParameters = NULL;
	}else{
		sei.lpFile = ViewApp;
		p = CreateCommandLine(ViewAppCmdLine, path, FALSE);
		sei.lpParameters = (p != NULL) ? p : path;
	}
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = hInst;
	//起動
	if(ShellExecuteEx(&sei) == FALSE){
		NULLCHECK_FREE(p);
		return FALSE;
	}
	NULLCHECK_FREE(p);
	return TRUE;
#else
	p = NULL;
	if(ViewApp == NULL || *ViewApp == TEXT('\0')){
		file = path;
		param = NULL;
	}else{
		file = ViewApp;
		p = CreateCommandLine(ViewAppCmdLine, path, FALSE);
		param = (p != NULL) ? p : path;
	}
	return CoshExecute(NULL, file, param);
#endif
}


/******************************************************************************

	ViewProc

	メール表示プロシージャ

******************************************************************************/

static LRESULT CALLBACK ViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	static BOOL SipFlag = FALSE;
#endif

	switch(msg)
	{
	case WM_CREATE:
		if(InitWindow(hWnd, (struct TPMAILITEM *)(((CREATESTRUCT *)lParam)->lpCreateParams)) == FALSE){
			DestroyWindow(hWnd);
			break;
		}
		lParam = (LPARAM)(((CREATESTRUCT *)lParam)->lpCreateParams);
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		SipFlag = FALSE;
#endif

	case WM_MODFYMESSAGE:
		SetWindowLong(hWnd, GWL_USERDATA, lParam);
		ModfyWindow(hWnd, (struct TPMAILITEM *)lParam, FALSE);
		_SetForegroundWindow(hWnd);
		break;

#ifdef _WIN32_WCE_PPC
	case WM_SETTINGCHANGE:
		if(SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd){
			SHACTIVATEINFO sai;

			memset(&sai, 0, sizeof(SHACTIVATEINFO));
			SHHandleWMSettingChange(hWnd, -1, 0, &sai);
			SipFlag = sai.fSipUp;
			SetWindowSize(hWnd, 0, 0);
		}
		break;
#elif defined _WIN32_WCE_LAGENDA
	case WM_SETTINGCHANGE:
		if(SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd){
			SipFlag = SetWindowSize(hWnd, 0, 0);
		}
		break;
#endif

#ifndef _WIN32_WCE
	case WM_SIZE:
		SetWindowSize(hWnd, wParam, lParam);
		break;

	case WM_EXITSIZEMOVE:
		if(IsWindowVisible(hWnd) != 0 && IsIconic(hWnd) == 0 && IsZoomed(hWnd) == 0){
			GetWindowRect(hWnd, (LPRECT)&ViewRect);
			ViewRect.right -= ViewRect.left;
			ViewRect.bottom -= ViewRect.top;
		}
		break;
#endif

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));
		HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
		FocusWnd = hWnd;
		ESCFlag = FALSE;
		break;

#ifdef _WIN32_WCE
	case WM_HIBERNATE:
#endif
	case WM_ENDCLOSE:
#ifdef _WIN32_WCE
		FocusWnd = MainWnd;
#ifdef _WIN32_WCE_LAGENDA
		ShowWindow(MainWnd, SW_SHOW);
#endif
#endif
		EndWindow(hWnd);
		hViewWnd = NULL;
		break;

	case WM_CLOSE:
		if(ESCFlag == TRUE){
			ESCFlag = FALSE;
			break;
		}
#ifdef _WIN32_WCE
		FocusWnd = MainWnd;
		ShowWindow(MainWnd, SW_SHOW);
		_SetForegroundWindow(MainWnd);
#endif
		EndWindow(hWnd);
		hViewWnd = NULL;
		break;

	case WM_INITMENUPOPUP:
#ifdef _WIN32_WCE_PPC
		SetEditMenu(hWnd);
#else
		if(LOWORD(lParam) == 1){
			SetEditMenu(hWnd);
		}
#endif
		break;

	case WM_TIMER:
		switch(wParam)
		{
		case ID_CLICK_TIMER:
			KillTimer(hWnd, wParam);
			OpenURL(hWnd);
			break;

		case ID_HIDECARET_TIMER:
			KillTimer(hWnd, wParam);
			HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
			break;
		}
		break;

#ifndef _WIN32_WCE
	case WM_NOTIFY:
		return NotifyProc(hWnd, lParam);
#endif

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam, lParam))
		{
#ifdef _WIN32_WCE_PPC
		case ID_MENU:
			SetEditMenu(hWnd);
			ShowMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), 0, 0, FALSE);
			break;

		case IDC_EDIT_BODY:
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				SHSipPreference(hWnd, (SipFlag) ? SIP_UP : SIP_DOWN);
				break;
			case EN_KILLFOCUS:
				SHSipPreference(hWnd, SIP_DOWN);
				break;
			}
			break;
#elif defined _WIN32_WCE_LAGENDA
		case IDC_EDIT_BODY:
			switch(HIWORD(wParam))
			{
			case EN_SETFOCUS:
				SipShowIM((SipFlag) ? SIPF_ON : SIPF_OFF);
				break;
			case EN_KILLFOCUS:
				SipShowIM(SIPF_OFF);
				break;
			}
			break;
#endif

		case ID_KEY_UP:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_VSCROLL, SB_LINEUP, 0);
			break;

		case ID_KEY_DOWN:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_VSCROLL, SB_LINEDOWN, 0);
			break;

		case ID_KEY_LEFT:
			if(WordBreakFlag == 1){
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_VSCROLL, SB_PAGEUP, 0);
			}else{
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_HSCROLL, SB_LINELEFT, 0);
			}
			break;

		case ID_KEY_RIGHT:
			if(WordBreakFlag == 1){
				View_NextScroll(GetDlgItem(hWnd, IDC_EDIT_BODY));
			}else{
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_HSCROLL, SB_LINERIGHT, 0);
			}
			break;

		case ID_MENUITEM_NEXTMAIL:
			View_NextMail(hWnd, FALSE);
			break;

		case ID_MENUITEM_PREVMAIL:
			View_PrevMail(hWnd);
			break;

		case ID_MENUITEM_NEXTNOREAD:
			View_NextNoReadMail(hWnd);
			break;

		case ID_MENUITEM_REMESSEGE:
			SetReMessage(hWnd, 0);
			break;

		case ID_MENUITEM_ALLREMESSEGE:
			SetReMessage(hWnd, 1);
			break;

		case ID_MENUITEM_SAVE:
			if(Item_IsMailBox(MailBox + vSelBox,
				(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if(SaveViewMail(NULL, hWnd, vSelBox, (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA), SAVE_HEADER) == FALSE){
				ErrorMessage(hWnd, STR_ERR_SAVE);
			}
			break;

		case ID_MENUITEM_PROP:
			if(Item_IsMailBox(MailBox + vSelBox,
				(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MAILPROP), hWnd, MailPropProc, GetWindowLong(hWnd, GWL_USERDATA));
			break;

#ifdef _WIN32_WCE_PPC
		case IDOK:
#endif
		case ID_MENUITEM_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

#ifdef _WIN32_WCE_LAGENDA
		case CSOBAR_ADORNMENTID_CLOSE:
			SendMessage(MainWnd, WM_CLOSE, 0, 0);
			break;
#endif

		case ID_MENUITEM_COPY:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_COPY , 0, 0);
			break;

		case ID_MENUITEM_ALLSELECT:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, 0, -1);
			break;

		case ID_MENUITEM_FIND:
			View_FindMail(hWnd, TRUE);
			break;

		case ID_MENUITEM_NEXTFIND:
			View_FindMail(hWnd, FALSE);
			break;

		case ID_MENUITEM_WORDBREAK:
			DelEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE_LAGENDA
			WordBreakFlag = SetWordBreak(hWnd, hViewMenu);
#else
			WordBreakFlag = SetWordBreak(hWnd);
#endif
			SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
			HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
			break;

		case ID_MENUITEM_SHOW_DATE:
			ViewShowDate = (ViewShowDate == 1) ? 0 : 1;
			SetHeaderSize(hWnd);
			if(Item_IsMailBox(MailBox + vSelBox,
				(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == TRUE){
				SetHeaderString(GetDlgItem(hWnd, IDC_HEADER),
					(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));
			}
			break;

		case ID_VIEW_SOURCE:
		case ID_VIEW_PART:
			if(Item_IsMailBox(MailBox + vSelBox,
				(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			ModfyWindow(hWnd, (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA),
				((GET_WM_COMMAND_ID(wParam, lParam) == ID_VIEW_SOURCE) ? TRUE : FALSE));
			break;

		case ID_VIEW_DELETE_ATTACH:
			if(MessageBox(hWnd, STR_Q_DELATTACH, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO){
				break;
			}
			if(Item_IsMailBox(MailBox + vSelBox,
				(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			DeleteAttachFile(hWnd, (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));
			ModfyWindow(hWnd, (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA), FALSE);
			break;

		case ID_MENUITEM_VIEW:
			if(Item_IsMailBox(MailBox + vSelBox,
				(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if(AppViewMail((struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
				ErrorMessage(hWnd, STR_ERR_VIEW);
				break;
			}
			if(ViewAppClose == 1){
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

		default:
			{
				int id = GET_WM_COMMAND_ID(wParam, lParam);
				if(id >= ID_ATTACH && id <= ID_ATTACH + 100){
					if(Item_IsMailBox(MailBox + vSelBox,
						(struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE){
						ErrorMessage(hWnd, STR_ERR_NOMAIL);
						break;
					}
					if(Decode(hWnd, id - ID_ATTACH) == FALSE){
						ErrorMessage(hWnd, STR_ERR_SAVE);
					}
				}
			}
			break;
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}


/******************************************************************************

	View_InitApplication

	ウィンドウクラスの登録

******************************************************************************/

BOOL View_InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;

#ifdef _WIN32_WCE
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = NULL;
	wc.lpszMenuName = NULL;
#else
	wc.style = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_VIEW);
#endif
	wc.lpfnWndProc = (WNDPROC)ViewProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_READ));
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
	wc.lpszClassName = VIEW_WND_CLASS;

	return RegisterClass(&wc);
}


/******************************************************************************

	View_InitInstance

	ウィンドウの作成

******************************************************************************/

BOOL View_InitInstance(HINSTANCE hInstance, LPVOID lpParam, BOOL NoAppFlag)
{
#ifdef _WIN32_WCE_PPC
	SIPINFO si;
	int x = CW_USEDEFAULT, y = CW_USEDEFAULT, cx, cy;
#endif
	int key;

	key = GetKeyState(VK_SHIFT);
	if(NoAppFlag == FALSE && ((DefViewApp == 1 && key >= 0) || (DefViewApp == 0 && key < 0))){
		//ビューアで表示
		if(AppViewMail((struct TPMAILITEM *)lpParam) == FALSE){
			ErrorMessage(MainWnd, STR_ERR_VIEW);
			return FALSE;
		}

		//開封済みにする
		if(((struct TPMAILITEM *)lpParam)->MailStatus != ICON_NON && ((struct TPMAILITEM *)lpParam)->MailStatus < ICON_SENDMAIL){
			((struct TPMAILITEM *)lpParam)->MailStatus = ICON_READ;
		}

		//一覧のアイコンの設定
		if(((struct TPMAILITEM *)lpParam)->Status != ICON_DOWN && ((struct TPMAILITEM *)lpParam)->Status != ICON_DEL){
			int LvFocus;
			LvFocus = ListView_GetNextItem(GetDlgItem(MainWnd, IDC_LISTVIEW), -1, LVNI_FOCUSED);
			((struct TPMAILITEM *)lpParam)->Status = ((struct TPMAILITEM *)lpParam)->MailStatus;
			ListView_RedrawItems(GetDlgItem(MainWnd, IDC_LISTVIEW), LvFocus, LvFocus);
			UpdateWindow(GetDlgItem(MainWnd, IDC_LISTVIEW));
		}
		return FALSE;
	}

	if(hViewWnd != NULL){
		SendMessage(hViewWnd, WM_MODFYMESSAGE, 0, (LPARAM)lpParam);
		return TRUE;
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	SHSipInfo(SPI_GETSIPINFO, 0, &si, 0);
	cx = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
	cy = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top - ((si.fdwFlags & SIPF_ON) ? 0 : MENU_HEIGHT);

	hViewWnd = CreateWindowEx(WS_EX_CAPTIONOKBTN,
		VIEW_WND_CLASS,
		STR_TITLE_MAILVIEW,
		WS_VISIBLE,
		x, y, cx, cy,
		NULL, NULL, hInstance, lpParam);
#else
	hViewWnd = CreateWindow(VIEW_WND_CLASS,
		STR_TITLE_MAILVIEW,
		WS_VISIBLE,
		0, 0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL, NULL, hInstance, lpParam);
#endif
#else
	hViewWnd = CreateWindow(VIEW_WND_CLASS,
		STR_TITLE_MAILVIEW,
		WS_OVERLAPPEDWINDOW,
		ViewRect.left,
		ViewRect.top,
		ViewRect.right,
		ViewRect.bottom,
		NULL, NULL, hInstance, lpParam);
#endif

	if(!hViewWnd){
		return FALSE;
	}

	ShowWindow(hViewWnd, SW_SHOW);
	UpdateWindow(hViewWnd);
	return TRUE;
}
/* End of source */
