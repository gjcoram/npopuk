/*
 * nPOP
 *
 * View.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2009 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include <windows.h>

#include "General.h"
#ifndef _WCE_OLD
#include <imm.h>
#endif

#include "Memory.h"
#include "String.h"
#include "code.h"
#include "mime.h"
#include "multipart.h"
#include "Font.h"
#ifdef USE_NEDIT
#include "nEdit.h"
#endif

#include "global.h"
#include "md5.h"

/* Define */
#define WM_MODFYMESSAGE				(WM_APP + 101)
#define ID_MENU						(WM_APP + 102)

#define IDC_VCB						2000
#define IDC_VTB						2001
#define IDC_HEADER					2002
#define IDC_EDIT_BODY				2003

#define ID_CLICK_TIMER				1
#define ID_HIDECARET_TIMER			2

#ifdef _WIN32_WCE
#define MENU_ATTACH_POS				8
#else
#define MENU_ATTACH_POS				9
#endif
#define ID_VIEW_SOURCE				400
#define ID_VIEW_PART				401
#define ID_VIEW_DELETE_ATTACH		402
#define ID_VIEW_SAVE_ATTACH			403
#define ID_ATTACH					500
#define ID_RETURN_TO_MASTER			404

#define SAVE_HEADER					TEXT("From: %f\r\nTo: %t\r\n{Cc: %c\r\n}Subject: %s\r\nDate: %D\r\nMessage-ID: %i\r\n\r\n")

/* Global Variables */
static WNDPROC EditWindowProcedure;
HWND hViewWnd = NULL;
#ifdef _WIN32_WCE_PPC
HWND hViewToolBar;
int LastXSize_V = 0;
#endif

#ifdef _WIN32_WCE_LAGENDA
static HMENU hViewMenu;
static int g_menu_height;
#endif

BOOL ViewWndViewSrc = FALSE;
int vSelBox = -1;
extern HMENU vMenuDone;

static MULTIPART **tpMultiPart;
static int MultiPartCnt, MultiPartTextIndex;

TCHAR *FindStr = NULL;
DWORD FindPos;
MAILITEM *FindMailItem = NULL;
static int FindBox = 0, FindStartBox = 0, FindStartItem = 0;
int FindNext = 0, FindOrReplace = 0;

static BOOL ESCFlag = FALSE;
static BOOL UnicodeEdit = 0;

int AttachProcess;
MAILITEM *AttachMailItem = NULL;
MAILITEM *DigestMaster = NULL;
int DigestMessageNum = -1;
int DigestMessageCnt = 0;

// 外部参照
extern OPTION op;

extern HINSTANCE hInst;  // Local copy of hInstance
extern HWND MainWnd;
#ifdef _WIN32_WCE
extern HWND hEditWnd;
#endif
extern HWND FocusWnd;
extern HFONT hListFont;
extern MAILBOX *MailBox;
extern int SelBox;
extern int RecvBox;
extern int MailBoxCnt;
extern BOOL ExecFlag;
extern BOOL PPCFlag;

extern HFONT hViewFont;
extern int font_charset;

extern TCHAR *DataDir;

/* Local Function Prototypes */
static void SetWindowString(HWND hWnd, TCHAR *MailBoxName, TCHAR *MailBoxName2, int No);
static void SetHeaderString(HWND hHeader, MAILITEM *tpMailItem);
static void SetHeaderSize(HWND hWnd);
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam);
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam);
#endif
static LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);
static void SetEditSubClass(HWND hWnd);
static void DelEditSubClass(HWND hWnd);
static BOOL InitWindow(HWND hWnd, MAILITEM *tpMailItem);
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void EndWindow(HWND hWnd);
static void SetEditMenu(HWND hWnd);
static void ModifyWindow(HWND hWnd, MAILITEM *tpMailItem, BOOL ViewSrc, BOOL BodyOnly);
static MAILITEM *View_NextMail(HWND hWnd);
static MAILITEM *View_PrevMail(HWND hWnd);
static MAILITEM *View_NextUnreadMail(HWND hWnd);
static void View_Scroll(HWND hViewWnd, int dir);
static void OpenURL(HWND hWnd);
static void SetReMessage(HWND hWnd, int ReplyFlag);
static BOOL Decode(HWND hWnd, int id, int DoWhat);
static BOOL SaveViewMail(TCHAR *fname, HWND hWnd, int MailBoxIndex, MAILITEM *tpMailItem, TCHAR *head, BOOL ViewSrc);
static BOOL AppViewMail(MAILITEM *tpMailItem, int MailBoxIndex);
static void SetMark(HWND hWnd, MAILITEM *tpMailItem, const int mark);
static void GetMarkStatus(HWND hWnd, MAILITEM *tpMailItem);
static LRESULT CALLBACK ViewProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);

/*
 * SetWindowString - ウィンドウタイトルの設定
 */
static void SetWindowString(HWND hWnd, TCHAR *MailBoxName, TCHAR *MailBoxName2, int No)
{
	TCHAR *buf, *r;
	TCHAR *p, *p2;

	p = (MailBoxName == NULL || *MailBoxName == TEXT('\0')) ? STR_MAILBOX_NONAME : MailBoxName;
	p2 = (MailBoxName2 == NULL || *MailBoxName2 == TEXT('\0')) ? TEXT("") : MailBoxName2;

	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) *
		(lstrlen(STR_TITLE_MAILVIEW) + lstrlen(p) + lstrlen(p2) + lstrlen(TEXT(" -  ()") STR_TITLE_MAILVIEW_COUNT) + 10 + 1));
	if (buf == NULL) {
		SetWindowText(hWnd, STR_TITLE_MAILVIEW);
		return;
	}

	r = str_join_t(buf, STR_TITLE_MAILVIEW, TEXT(" - "), p, (TCHAR *)-1);
	if (*p2 != TEXT('\0')) {
		str_join_t(r, TEXT(" ("), p2, TEXT(")"), (TCHAR *)-1);
	} else if (No != 0) {
		wsprintf(r, STR_TITLE_MAILVIEW_COUNT, No);
	}
	if (lstrlen(buf) > BUF_SIZE) {
		*(buf + BUF_SIZE) = TEXT('\0');
	}
	SetWindowText(hWnd, buf);
	mem_free(&buf);
}

/*
 * SetHeaderString - メールヘッダ表示
 */
static void SetHeaderString(HWND hHeader, MAILITEM *tpMailItem)
{
	TCHAR *MyMailAddress = NULL;
	TCHAR *buf, *p;
	int len = 0;
	int i;
	BOOL ToFlag = FALSE;

	// 自分のメールアドレスの取得
	if ((MailBox + SelBox)->Type == MAILBOX_TYPE_SAVE) {
		i = mailbox_name_to_index(tpMailItem->MailBox);
		if (i != -1 && (MailBox + i)->Type != MAILBOX_TYPE_SAVE) {
			MyMailAddress = (MailBox + i)->MailAddress;
		}
	} else {
		MyMailAddress = (MailBox + SelBox)->MailAddress;
	}

	// To を表示するべきかチェック
	if (tpMailItem->To != NULL) {
		buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(tpMailItem->To) + 1));
		if (buf != NULL) {
			p = GetMailAddress(tpMailItem->To, buf, NULL, FALSE);
			if (*buf != TEXT('\0') &&
				(*p != TEXT('\0') || MyMailAddress == NULL || lstrcmpi(MyMailAddress, buf) != 0)) {
				ToFlag = TRUE;
			}
			mem_free(&buf);
		}
	}

	// 作成する文字列のサイズを計算
	len += lstrlen(STR_VIEW_HEAD_FROM);
	if (tpMailItem->From != NULL) {
		len += lstrlen(tpMailItem->From);
	}
	len += lstrlen(STR_VIEW_HEAD_SUBJECT);
	if (tpMailItem->Subject != NULL) {
		len += lstrlen(tpMailItem->Subject);
	}
	if (ToFlag == TRUE) {
		len += SetCcAddressSize(tpMailItem->To);
	}
	len += SetCcAddressSize(tpMailItem->Cc);
	len += SetCcAddressSize(tpMailItem->Bcc);
	if (op.ViewShowDate == 1) {
		len += lstrlen(STR_VIEW_HEAD_DATE);
		if (tpMailItem->FmtDate != NULL) {
			len += lstrlen(tpMailItem->FmtDate);
		}
	}

	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (buf == NULL) {
		return;
	}
	*buf = TEXT('\0');

	// 表示する文字列を作成する
	p = str_join_t(buf, STR_VIEW_HEAD_FROM, tpMailItem->From, STR_VIEW_HEAD_SUBJECT, tpMailItem->Subject, (TCHAR *)-1);
	if (ToFlag == TRUE) {
		p = SetCcAddress(TEXT("To"), tpMailItem->To, p);
	}
	p = SetCcAddress(TEXT("Cc"), tpMailItem->Cc, p);
	p = SetCcAddress(TEXT("Bcc"), tpMailItem->Bcc, p);
	if (op.ViewShowDate == 1) {
		p = str_join_t(p, STR_VIEW_HEAD_DATE, tpMailItem->FmtDate, (TCHAR *)-1);
	}

	SetWindowText(hHeader, buf);
	mem_free(&buf);
}

/*
 * SetHeaderSize - メールヘッダのサイズ設定
 */
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
	hMenu = SHGetSubMenu(hViewToolBar, ID_MENUITEM_VIEW);
#elif defined _WIN32_WCE_LAGENDA
	hMenu = GetSubMenu(hViewMenu, 1);
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0), 1);
#endif
#else
	hMenu = GetSubMenu(GetMenu(hWnd), 1);
#endif
	CheckMenuItem(hMenu, ID_MENUITEM_SHOW_DATE, (op.ViewShowDate == 1) ? MF_CHECKED : MF_UNCHECKED);

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
	// フォントの高さを取得
	GetTextMetrics(hdc, &lptm);
	if (hFont != NULL) {
		SelectObject(hdc, hFont);
	}
	ReleaseDC(hHeader, hdc);
	HLine = (op.ViewShowDate == 1) ? 3 : 2;
	FontHeight = (lptm.tmHeight + lptm.tmExternalLeading) * HLine;

	// 一時的に設定してサイズを再計算する
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

/*
 * TbNotifyProc - ツールバーの通知メッセージ (Win32)
 */
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

/*
 * NotifyProc - コントロールの通知メッセージ
 */
#ifndef _WIN32_WCE
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;

	if (CForm->code == TTN_NEEDTEXT) {
		return TbNotifyProc(hWnd, lParam);
	}
	return FALSE;
}
#endif

/*
 * FindEditString - EDIT内の文字列を検索する
 */
BOOL FindEditString(HWND hEdit, TCHAR *strFind, int CaseFlag, BOOL Loop)
{
	DWORD dwStart;
	DWORD dwEnd;
	TCHAR *buf = NULL;
	TCHAR *p;

#ifdef UNICODE
	// 検索位置の取得
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	// 現在位置が前回検索位置と違う場合は現在位置を検索位置にする
	if ((dwStart + 1U) != FindPos) {
		FindPos = dwStart;
	}

	// エディットから文字列を取得する
	AllocGetText(hEdit, &buf);
	p = str_find(strFind, buf + FindPos, CaseFlag);
	if (Loop == TRUE && *p == TEXT('\0')) {
		p = str_find(strFind, buf, CaseFlag);
	}

	// 検索文字列が見つからなかった場合
	if (*p == TEXT('\0')) {
		mem_free(&buf);
		return FALSE;
	}

	// 文字列が見つかった場合はその位置を選択状態にする
	FindPos = p - buf;
	SendMessage(hEdit, EM_SETSEL, FindPos, FindPos + lstrlen(strFind));
	SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
	mem_free(&buf);
	FindPos++;
	return TRUE;
#else
	// エディットから文字列を取得する
	AllocGetText(hEdit, &buf);
	if (UnicodeEdit == 2) {
		int st, len;
		WCHAR *wbuf;

		len = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
		wbuf = (WCHAR *)mem_alloc(sizeof(WCHAR) * (len + 1));
		if (wbuf == NULL) {
			mem_free(&buf);
			return FALSE;
		}
		MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, len);
		// 検索位置の取得
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		st = WideCharToMultiByte(CP_ACP, 0, wbuf, dwStart, NULL, 0, NULL, NULL);
		mem_free(&wbuf);
		// 現在位置が前回検索位置と違う場合は現在位置を検索位置にする
		if ((st + 1U) != FindPos) {
			FindPos = st;
		}
		p = str_find(strFind, buf + FindPos, CaseFlag);
		if (Loop == TRUE && *p == TEXT('\0')) {
			p = str_find(strFind, buf, CaseFlag);
		}
		// 検索文字列が見つからなかった場合
		if (*p == TEXT('\0')) {
			mem_free(&buf);
			return FALSE;
		}
		st = MultiByteToWideChar(CP_ACP, 0, buf, p - buf, NULL, 0);
		len = MultiByteToWideChar(CP_ACP, 0, strFind, -1, NULL, 0) - 1;
		// 文字列が見つかった場合はその位置を選択状態にする
		SendMessage(hEdit, EM_SETSEL, st, st + len);
		FindPos = p - buf;
	} else {
		// 検索位置の取得
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		// 現在位置が前回検索位置と違う場合は現在位置を検索位置にする
		if ((dwStart + 1U) != FindPos) {
			FindPos = dwStart;
		}
		p = str_find(strFind, buf + FindPos, CaseFlag);
		if (Loop == TRUE && *p == TEXT('\0')) {
			p = str_find(strFind, buf, CaseFlag);
		}
		// 検索文字列が見つからなかった場合
		if (*p == TEXT('\0')) {
			mem_free(&buf);
			return FALSE;
		}
		// 文字列が見つかった場合はその位置を選択状態にする
		FindPos = p - buf;
		SendMessage(hEdit, EM_SETSEL, FindPos, FindPos + lstrlen(strFind));
	}
	SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
	mem_free(&buf);
	FindPos++;
	return TRUE;
#endif
}

/*
 * SetWordBreakMenu - 折り返しのメニューのチェック切り替え
 */
void SetWordBreakMenu(HWND hWnd, HMENU hEditMenu, int Flag)
{
#ifdef _WIN32_WCE_PPC
	CheckMenuItem(hEditMenu, ID_MENUITEM_WORDBREAK, Flag);
#else	// _WIN32_WCE_PPC
	HMENU hMenu;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	hMenu = hEditMenu;
#else	// _WIN32_WCE_LAGENDA
	hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0);
#endif	// _WIN32_WCE_LAGENDA
#else	// _WIN32_WCE
	hMenu = GetMenu(hWnd);
#endif	// _WIN32_WCE
	CheckMenuItem(GetSubMenu(hMenu, 1), ID_MENUITEM_WORDBREAK, Flag);
#endif	// _WIN32_WCE_PPC
}

/*
 * SetWordBreak - 折り返しの切り替え
 */
#ifdef _WIN32_WCE_PPC
int SetWordBreak(HWND hWnd, HWND hToolBar)
#elif defined(_WIN32_WCE_LAGENDA)
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
	if (i & WS_HSCROLL) {
		i ^= WS_HSCROLL;
#ifdef _WIN32_WCE_PPC
		SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_VIEW), MF_CHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
		SetWordBreakMenu(hWnd, hMenu, MF_CHECKED);
#else
		SetWordBreakMenu(hWnd, NULL, MF_CHECKED);
#endif
		ret = 1;
	} else {
		i |= WS_HSCROLL;
#ifdef _WIN32_WCE_PPC
		SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_VIEW), MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
		SetWordBreakMenu(hWnd, hMenu, MF_UNCHECKED);
#else
		SetWordBreakMenu(hWnd, NULL, MF_UNCHECKED);
#endif
		ret = 0;
	}

	ModifyFlag = SendMessage(hEdit, EM_GETMODIFY, 0, 0);

	len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (buf == NULL) {
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
	if (hViewFont != NULL) {
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
	}
	SetFocus(hEdit);

	SendMessage(hEdit, WM_SETTEXT, 0, (LPARAM)buf);
	SendMessage(hEdit, EM_SETMODIFY, (WPARAM)ModifyFlag, 0);
	mem_free(&buf);
	return ret;
}

/*
 * SubClassEditProc - サブクラス化したウィンドウプロシージャ
 */
static LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if ((TCHAR)wParam == TEXT(' ')) {
			if (GetKeyState(VK_SHIFT) < 0) {
				SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			} else if (GetKeyState(VK_CONTROL) < 0) {
				View_NextUnreadMail(GetParent(hWnd));
			} else {
				View_Scroll(hWnd, +1);
			}
		}
	case WM_DEADCHAR:
	case WM_CUT:
	case WM_CLEAR:
	case WM_PASTE:
	case EM_UNDO:
		return 0;

	case WM_KEYDOWN:
		if (op.ViewWindowCursor == 0) {
			short key_sh, key_ctl, key_alt;
			key_sh  = GetKeyState(VK_SHIFT);
			key_ctl = GetKeyState(VK_CONTROL);
			key_alt = GetKeyState(VK_MENU);
			switch(LOWORD(wParam)) {
			case VK_LEFT:
				if (key_sh >= 0) {
					SendMessage(GetParent(hWnd), WM_COMMAND, ID_KEY_LEFT, 0);
				}
				return 0;
				break;

			case VK_RIGHT:
				if (key_sh >= 0) {
					SendMessage(GetParent(hWnd), WM_COMMAND, ID_KEY_RIGHT, 0);
				}
				return 0;
				break;

			case VK_UP:
				if (key_ctl < 0 || key_alt < 0) {
					SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENUITEM_PREVMAIL, 0);
				} else if (key_sh >= 0) {
					SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, 0);
				}
				return 0;
				break;

			case VK_DOWN:
				if (key_ctl < 0 && key_alt < 0) {
					SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENUITEM_NEXTUNREAD, 0);
				} else if (key_ctl < 0 || key_alt < 0) {
					SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENUITEM_NEXTMAIL, 0);
				} else if (key_sh >= 0) {
					SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, 0);
				}
				return 0;
				break;
			}
		}
		break;

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

			if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
				SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENU, 0);
				return 0;
			}
		}
#else
		if (GetKeyState(VK_MENU) < 0) {
			SetEditMenu(GetParent(hWnd));
			ShowMenu(GetParent(hWnd),
				CommandBar_GetMenu(GetDlgItem(GetParent(hWnd), IDC_VCB), 0), 1, 0);
			return 0;
		}
#endif
		break;
#else
	case WM_CONTEXTMENU:
		SetEditMenu(GetParent(hWnd));
		ShowMenu(GetParent(hWnd), GetMenu(GetParent(hWnd)), 1, 0);
		return 0;
#endif
	}
	return CallWindowProc(EditWindowProcedure, hWnd, msg, wParam, lParam);
}

/*
 * SetEditSubClass - ウィンドウのサブクラス化
 */
static void SetEditSubClass(HWND hWnd)
{
	SendMessage(hWnd, EM_LIMITTEXT, 1, 0);
	EditWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassEditProc);
}

/*
 * DelEditSubClass - ウィンドウクラスを標準のものに戻す
 */
static void DelEditSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)EditWindowProcedure);
	EditWindowProcedure = NULL;
}

/*
 * InitWindow - ウィンドウの初期化
 */
static BOOL InitWindow(HWND hWnd, MAILITEM *tpMailItem)
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
		ID_MENUITEM_CLOSE,		CSOBAR_COMMON_BUTTON,		CSO_BUTTON_DISP, (-1),					NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), CSO_ID_BACK,	CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		1,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, STR_MENU_FILE,	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		2,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, STR_MENU_EDIT,	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		ID_MENUITEM_PREVMAIL,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_PREVMAIL,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_NEXTMAIL,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_NEXTMAIL,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_NEXTUNREAD,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_NEXTUNREAD,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_REMESSEGE,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_REMESSEGE,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
	};
	HWND hCSOBar;
	DWORD style;
#else	// _WIN32_WCE_LAGENDA
#ifndef _WIN32_WCE_PPC
	HWND hViewToolBar;
#endif	// _WIN32_WCE_PPC
	TBBUTTON tbButton[] = {
#ifdef _WIN32_WCE
#ifndef _WIN32_WCE_PPC
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
#endif	// _WIN32_WCE_PPC
#endif	// _WIN32_WCE
		{0,	ID_MENUITEM_PREVMAIL,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{1,	ID_MENUITEM_NEXTMAIL,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{2,	ID_MENUITEM_NEXTUNREAD,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{3,	ID_MENUITEM_REMESSEGE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{4,	ID_MENUITEM_ALLREMESSEGE,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{5,	ID_MENUITEM_FORWARD,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{6,	ID_MENUITEM_NEXTFIND,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{7,	ID_MENUITEM_DOWNMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{8,	ID_MENUITEM_DELMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{9,	ID_MENUITEM_DELETE,		TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{10,ID_MENUITEM_UNREADMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{11,ID_MENUITEM_FLAGMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
	};
#ifdef _WIN32_WCE
	static TCHAR *szTips[] = {
#ifdef _WIN32_WCE_PPC
		NULL, // menu skipping
#endif	// _WIN32_WCE_PPC
		NULL, // menu skipping
		STR_CMDBAR_PREVMAIL,
		STR_CMDBAR_NEXTMAIL,
		STR_CMDBAR_NEXTUNREAD,
		STR_CMDBAR_REMESSEGE,
		STR_CMDBAR_ALLREMESSEGE,
		STR_CMDBAR_FORWARD,
		STR_CMDBAR_NEXTFIND,
		STR_CMDBAR_DOWNMARK,
		STR_CMDBAR_DELMARK,
		STR_CMDBAR_DELETE,
		STR_CMDBAR_UNREADMARK,
		STR_CMDBAR_FLAGMARK
#ifndef _WIN32_WCE_PPC
		,NULL // extra for CE.net
#endif
	};
#ifdef _WIN32_WCE_PPC
	SHMENUBARINFO mbi;
#else	// _WIN32_WCE_PPC
	WORD idMenu;
#endif	// _WIN32_WCE_PPC
#endif	// _WIN32_WCE
#endif	// _WIN32_WCE_LAGENDA

	if (tpMailItem == NULL) { // || tpMailItem->Body == NULL) {
		return FALSE;
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	// PocketPC
	memset (&mbi, 0, sizeof (SHMENUBARINFO));
	mbi.cbSize     = sizeof (SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_VIEW;
	mbi.hInstRes   = hInst;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;
	SHCreateMenuBar(&mbi);
	hViewToolBar = mbi.hwndMB;

	CommandBar_AddToolTips(hViewToolBar, 14, szTips);
	CommandBar_AddBitmap(hViewToolBar, hInst, IDB_TOOLBAR_VIEW, 12, 16, 16);
	CommandBar_AddButtons(hViewToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);

#elif defined(_WIN32_WCE_LAGENDA)
	// BE-500
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
	// H/PC & PsPC
	hViewToolBar = CommandBar_Create(hInst, hWnd, IDC_VCB);
	// op.osMajorVer >= 4 is CE.net 4.2 and higher (MobilePro 900c)
	// else HPC2000 (Jornada 690, 720)
	CommandBar_AddToolTips(hViewToolBar, 13, ((op.osMajorVer >= 4) ? (szTips+1) : szTips));
	idMenu = (GetSystemMetrics(SM_CXSCREEN) >= 450) ? (WORD)IDR_MENU_VIEW_HPC : (WORD)IDR_MENU_VIEW;
	CommandBar_InsertMenubar(hViewToolBar, hInst, idMenu, 0);
	CommandBar_AddBitmap(hViewToolBar, hInst, IDB_TOOLBAR_VIEW, 12, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hViewToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	CommandBar_AddAdornments(hViewToolBar, 0, 0);
#endif
#else
	// Win32
	hViewToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_VTB, 12, hInst, IDB_TOOLBAR_VIEW,
		tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, TB_ICONSIZE, TB_ICONSIZE, sizeof(TBBUTTON));
	SetWindowLong(hViewToolBar, GWL_STYLE, GetWindowLong(hViewToolBar, GWL_STYLE) | TBSTYLE_FLAT);
	SendMessage(hViewToolBar, TB_SETINDENT, 5, 0);
	ShowWindow(hViewToolBar, SW_SHOW);
#endif

	// ヘッダを表示するSTATICコントロールの作成
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
	// フォントの設定
#ifdef _WIN32_WCE
	if (hListFont != NULL) {
		SendDlgItemMessage(hWnd, IDC_HEADER, WM_SETFONT, (WPARAM)hListFont, MAKELPARAM(TRUE,0));
	}
#else
	SendDlgItemMessage(hWnd, IDC_HEADER, WM_SETFONT,
		(WPARAM)((hListFont != NULL) ? hListFont : GetStockObject(DEFAULT_GUI_FONT)), MAKELPARAM(TRUE,0));
#endif

#ifndef USE_NEDIT
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
		WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ((op.WordBreakFlag == 1) ? 0 : WS_HSCROLL),
		0, 0, 0, 0, hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
#else
	CreateWindowEx(
		WS_EX_CLIENTEDGE,
		NEDIT_WND_CLASS, TEXT(""),
		WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ((op.WordBreakFlag == 1) ? 0 : WS_HSCROLL),
		0, 0, 0, 0, hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
#endif
	if (hViewFont != NULL) {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
	}
	SetHeaderSize(hWnd);
#ifndef UNICODE
	if (UnicodeEdit == 0) {
		int i, j;
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT("あ"));
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
		UnicodeEdit = (j != 2) ? 2 : 1;
	}
#endif

	SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE_PPC
	SetWordBreakMenu(hWnd, SHGetSubMenu(hViewToolBar, ID_MENUITEM_VIEW), (op.WordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
	SetWordBreakMenu(hWnd, hViewMenu, (op.WordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
	SetWordBreakMenu(hWnd, NULL, (op.WordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

#ifdef _WIN32_WCE
	SendMessage(hWnd, WM_SETICON, (WPARAM)FALSE,
		(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_READ), IMAGE_ICON, 16, 16, 0));
#endif

#ifndef _WCE_OLD
	// IMEをオフにする
	ImmAssociateContext(GetDlgItem(hWnd, IDC_EDIT_BODY), (HIMC)NULL);
#endif

	SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
	if (op.ViewWindowCursor == 0) {
		SetTimer(hWnd, ID_HIDECARET_TIMER, 10, NULL);
	}
	return TRUE;
}

/*
 * SetWindowSize - ウィンドウのサイズ変更
 */
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32_WCE
	RECT rcClient, HeaderRect;
	int Height = 0;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_HEADER), &HeaderRect);

#ifndef _WIN32_WCE_PPC
	Height = CommandBar_Height(GetDlgItem(hWnd, IDC_VCB));
#endif
	MoveWindow(GetDlgItem(hWnd, IDC_HEADER), 0, Height, rcClient.right, HeaderRect.bottom - HeaderRect.top, TRUE);
	InvalidateRect(GetDlgItem(hWnd, IDC_HEADER), NULL, FALSE);
	UpdateWindow(GetDlgItem(hWnd, IDC_HEADER));

	Height += HeaderRect.bottom - HeaderRect.top;
	MoveWindow(GetDlgItem(hWnd, IDC_EDIT_BODY), 0, Height + 1,
		rcClient.right, rcClient.bottom - Height, TRUE);
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
	if (SipInfo.fdwFlags & SIPF_ON) {
		sip_height = ((SipInfo.rcSipRect).bottom - (SipInfo.rcSipRect).top);
		ret = TRUE;
	} else {
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

/*
 * EndWindow - ウィンドウの終了処理
 */
static void EndWindow(HWND hWnd)
{
	multipart_free(&tpMultiPart, MultiPartCnt);
	tpMultiPart = NULL;
	MultiPartCnt = 0;
	MultiPartTextIndex = 0;

	item_free(&AttachMailItem, 1);
	DigestMaster = NULL;
	DigestMessageNum = -1;
	DigestMessageCnt = 0;

	DelEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	DestroyWindow(hViewToolBar);
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

	vSelBox = -1;
	vMenuDone = NULL;

	DestroyWindow(hWnd);
}

/*
 * SetEditMenu - 編集メニューの活性／非活性の切り替え
 */
static void SetEditMenu(HWND hWnd)
{
	HMENU hMenu;
	int i, j;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hViewToolBar, ID_MENUITEM_VIEW);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 1);
#else
	hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0);
#endif
#else
	hMenu = GetMenu(hWnd);
#endif
	// エディットボックスの選択位置の取得
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
	EnableMenuItem(hMenu, ID_MENUITEM_COPY, (i < j) ? MF_ENABLED : MF_GRAYED);
}

/*
 * SetAttachMenu - 表示するpartの選択と添付メニューの設定
 */
static int SetAttachMenu(HWND hWnd, MAILITEM *tpMailItem, BOOL ViewSrc, BOOL IsAttach, TCHAR **attlist)
{
	HMENU hMenu;
	TCHAR *str, *p, *r;
	int i, mFlag, ret = -1, cnt = 0;
	BOOL AppendFlag = FALSE, startbody = FALSE;
	*attlist = NULL;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hViewToolBar, ID_MENUITEM_VIEW);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 1);
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0), 1);
#endif
#else
	hMenu = GetSubMenu(GetMenu(hWnd), 1);
#endif
	// メニューを初期化する
	while (DeleteMenu(hMenu, MENU_ATTACH_POS, MF_BYPOSITION) == TRUE);

	if (DigestMaster != NULL) {
		AppendMenu(hMenu, MF_STRING, ID_RETURN_TO_MASTER, STR_VIEW_RETURN);
	}

	if (MultiPartCnt == 0 && ViewSrc == TRUE) {
		AppendMenu(hMenu, MF_STRING, ID_VIEW_PART, STR_VIEW_MENU_ATTACH);
		return 0;
	}
	if ((*tpMultiPart)->sPos == tpMailItem->Body) {
		startbody = TRUE;
	} else if (tpMailItem->HasHeader) {
		char *p = GetBodyPointa(tpMailItem->Body);
		if (p == (*tpMultiPart)->sPos) {
			startbody = TRUE;
		}
	}
	if (MultiPartCnt == 1 && startbody && (*tpMultiPart)->ePos == NULL &&
		(*tpMultiPart)->ContentType != NULL &&
		str_cmp_ni((*tpMultiPart)->ContentType, "text", tstrlen("text")) == 0) {

		BOOL htmlonly = (str_cmp_ni((*tpMultiPart)->ContentType, "text/html", tstrlen("text/html")) == 0) ? TRUE : FALSE;
		if (tpMailItem->HasHeader || (op.StripHtmlTags == 1 && htmlonly == TRUE)) {
			AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
		}
		if (htmlonly == TRUE) {
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			AppendMenu(hMenu, MF_STRING, ID_ATTACH, TEXT("text/html"));
		}
		return 0;
	}

	for (i = 0; i < MultiPartCnt; i++) {
		int part_is_text = 0;
		if ((*(tpMultiPart + i))->ContentType == NULL) {
			part_is_text = 1;
		} else if (str_cmp_ni((*(tpMultiPart + i))->ContentType, "text/html", tstrlen("text/html")) == 0) {
			part_is_text = 2;
		} else if (str_cmp_ni((*(tpMultiPart + i))->ContentType, "text", tstrlen("text")) == 0) {
			part_is_text = 1;
		}
		if (ret == -1 && part_is_text != 0) {
			// 一番目に出現したテキストデータは本文にする
			ret = i;
			if (MultiPartCnt == 1 && tpMailItem->Multipart > MULTIPART_ATTACH) {
				AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
			}
		}
		if (part_is_text != 1 || ret != i) {
			if (AppendFlag == FALSE) {
				if (MultiPartCnt > 1 || (MultiPartCnt == 1 && part_is_text == FALSE)) {
					AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
				}
				if (MultiPartCnt > 1) {
					AppendMenu(hMenu, MF_STRING, ID_VIEW_SAVE_ATTACH, STR_VIEW_MENU_SAVEATTACH);
					if (IsAttach == FALSE) {
						AppendMenu(hMenu, MF_STRING, ID_VIEW_DELETE_ATTACH, STR_VIEW_MENU_DELATTACH);
					}
				}
				// 区切りの追加
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendFlag = TRUE;
			}
			// 途中で切れているデータの場合はメニューを非活性にする
			mFlag = ((*(tpMultiPart + i))->ePos == NULL) ? MF_GRAYED : MF_ENABLED;
			if (MultiPartCnt == 1 && tpMailItem->Download == TRUE) {
				mFlag = MF_ENABLED;
			}

			if ((*(tpMultiPart + i))->IsDigestMsg == TRUE) {
				char *s, *t;
				int j, len = strlen(HEAD_SUBJECT);
				s = (*(tpMultiPart + i))->hPos;
				t = (*(tpMultiPart + i))->ePos;
				if (t == NULL) {
					t = s + strlen(s);
				}
				while ( s < t && str_cmp_ni(s, HEAD_SUBJECT, len)) {
					s++;
				}
				if (s < t) {
					item_get_content_t(s, HEAD_SUBJECT, &str);
					len = lstrlen(str);
					// limit subject to 50 characters, or at first CR/NL
					for (r = str, j = 0; 
						*r != TEXT('\0') && *r != TEXT('\r') && *r != TEXT('\n') && j < 50;
						r++, j++);
					if (*r != TEXT('\0')) {
						if (j + 3 > len) {
							r = str + len - 3;
						}
						*(r++) = TEXT('.');
						*(r++) = TEXT('.');
						*(r++) = TEXT('.');
						*r = TEXT('\0');
					}
				} else {
					str = alloc_copy_t(STR_LIST_NOSUBJECT);
				}
			} else {
				// ファイル名をメニューに追加する
				str = alloc_char_to_tchar((*(tpMultiPart + i))->ContentType);
				if (str != NULL) {
					for (r = str; *r != TEXT('\0') && *r != TEXT(';'); r++);
					*r = TEXT('\0');
				}
			}
#ifdef UNICODE
			if ((*(tpMultiPart + i))->Filename != NULL) {
				p = alloc_char_to_tchar((*(tpMultiPart + i))->Filename);
				mem_free(&str);
				str = p;
			} else {
				p = str;
			}
#else
			p = ((*(tpMultiPart + i))->Filename != NULL) ? (*(tpMultiPart + i))->Filename : str;
#endif
			if (op.ViewShowAttach && (*(tpMultiPart + i))->Filename != NULL) {
				TCHAR *tmp;
				int len = lstrlen(p) + lstrlen(STR_MSG_ATTACHMENT) + 4;
				if (*attlist != NULL) {
					len += lstrlen(*attlist);
				}
				tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
				if (tmp != NULL) {
					str_join_t(tmp, *attlist, TEXT("\r\n"), STR_MSG_ATTACHMENT, p, TEXT("]"), (TCHAR *)-1);
				}
				mem_free(&*attlist);
				*attlist = tmp;
			}
			AppendMenu(hMenu, MF_STRING | mFlag, ID_ATTACH + i, ((p != NULL && *p != TEXT('\0')) ? p : TEXT("Attach")));
			if (mFlag == MF_ENABLED) {
				cnt++;
			}
			mem_free(&str);
		}
	}
	return ret;
}

/*
 * ModifyWindow - 内容の変更
 */
static void ModifyWindow(HWND hWnd, MAILITEM *tpMailItem, BOOL ViewSrc, BOOL BodyOnly)
{
	TCHAR *buf, *p;
	int LvFocus, TextIndex = -1;
	BOOL redraw = FALSE;
	BOOL IsAttach = FALSE;
	HWND hListView;

	if (tpMailItem == NULL) { // || tpMailItem->Body == NULL) {
		return;
	}

	if (tpMailItem == AttachMailItem) {
		IsAttach = TRUE;
	} else {
		DigestMaster = NULL;
		DigestMessageNum = -1;
		DigestMessageCnt = 0;
	}

	SwitchCursor(FALSE);
	ViewWndViewSrc = ViewSrc;

	if (BodyOnly == FALSE) {
		if (IsAttach == FALSE) {
			hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
			vSelBox = SelBox;
			LvFocus = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);

			// 開封済みにする
			if (tpMailItem->MailStatus != ICON_NON && tpMailItem->MailStatus < ICON_SENTMAIL) {
				if (tpMailItem->MailStatus != ICON_READ) {
					(MailBox + vSelBox)->NeedsSave |= MARKS_CHANGED;
					tpMailItem->MailStatus = ICON_READ;
				}
				tpMailItem->New = FALSE;
				redraw = TRUE;
			}

			// 一覧のアイコンの設定
			if (tpMailItem->Mark != ICON_DOWN && tpMailItem->Mark != ICON_DEL
				&& tpMailItem->Mark != ICON_FLAG && tpMailItem->Mark != tpMailItem->MailStatus) {
				(MailBox + vSelBox)->NeedsSave |= MARKS_CHANGED;
				tpMailItem->Mark = tpMailItem->MailStatus;
				redraw = TRUE;
			}
			if (redraw == TRUE) {
				ListView_SetItemState(hListView, LvFocus, INDEXTOOVERLAYMASK(tpMailItem->ReFwd & ICON_REFWD_MASK), LVIS_OVERLAYMASK);
				ListView_RedrawItems(hListView, LvFocus, LvFocus);
				UpdateWindow(hListView);
			}

			// ウィンドウタイトルの設定
			SetWindowString(hWnd, (MailBox + vSelBox)->Name, tpMailItem->MailBox, tpMailItem->No);
		} else {
			SetWindowString(hWnd, TEXT("Attached message"), (MailBox + vSelBox)->Name, 0);
		}
		// ヘッダの設定
		SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
		UpdateWindow(GetDlgItem(hWnd, IDC_HEADER));
		mailbox_menu_rebuild(hWnd, IsAttach);

		if (IsAttach == FALSE) {
			SetItemCntStatusText(NULL, FALSE);
		}

		// マーク状態取得
		GetMarkStatus(hWnd, tpMailItem);
	}

	multipart_free(&tpMultiPart, MultiPartCnt);
	tpMultiPart = NULL;
	MultiPartCnt = 0;

	if (tpMailItem->Body == NULL) {
		buf = NULL;
	} else {
		TCHAR *attachlist;
		// マルチパートの展開
		buf = MIME_body_decode(tpMailItem, ViewSrc, FALSE, &tpMultiPart, &MultiPartCnt, &TextIndex);

		// GJC remove HTML tags
		if (ViewSrc == FALSE && op.StripHtmlTags == 1 &&
			((tpMailItem->ContentType != NULL && str_cmp_ni_t(tpMailItem->ContentType, TEXT("text/html"), lstrlen(TEXT("text/html")))==0)
			|| (TextIndex != -1 && (tpMultiPart[TextIndex])->ContentType != NULL &&
			str_cmp_ni((tpMultiPart[TextIndex])->ContentType, "text/html", tstrlen("text/html")) == 0))) {
			p = strip_html_tags(buf, TRUE);
			if (p != NULL) {
				mem_free(&buf);
				buf = p;
			}
		}

		// 表示するpartの選択と添付メニューの設定
		MultiPartTextIndex = SetAttachMenu(hWnd, tpMailItem, ViewSrc, IsAttach, &attachlist);
		if (attachlist != NULL) {
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(buf) + lstrlen(attachlist) + 1));
			if (p != NULL) {
				str_join_t(p, buf, attachlist, (TCHAR *)-1);
				mem_free(&buf);
				buf = p;
			}
			mem_free(&attachlist);
		}
	}

	// 検索位置の初期化
	FindPos = 0;

	// 本文の表示
	if (buf != NULL) {
#ifdef _WIN32_WCE
		if (lstrlen(buf) > EDITMAXSIZE) {
			str_cpy_t(buf + EDITMAXSIZE, TEXT("[nPOPuk: text too long]"));
		}
#else
		if (op.osPlatformId != VER_PLATFORM_WIN32_NT && lstrlen(buf) > EDITMAXSIZE) {
			str_cpy_t(buf + EDITMAXSIZE, TEXT("[nPOPuk: text too long]"));
		}
#endif
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)buf);
		mem_free(&buf);
	} else {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)TEXT(""));
	}

	ShowWindow(hWnd, SW_SHOW);
#ifndef _WIN32_WCE
	if (IsIconic(hWnd) != 0) {
		ShowWindow(hWnd, SW_RESTORE);
	}
#endif
	if (op.ViewWindowCursor == 0) {
		HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
	}
	SwitchCursor(TRUE);
}

/*
 * View_NextMail - 次のメールを表示
 */
static MAILITEM *View_NextMail(HWND hWnd)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int j;

	if (SelBox == MAILBOX_SEND) {
		return NULL;
	}
	if (DigestMessageNum > 0 && DigestMaster != NULL) {
		j = DigestMessageNum + 1;
		if (j < DigestMessageCnt) {
			TCHAR *tmp;
			multipart_free(&tpMultiPart, MultiPartCnt);
			tpMultiPart = NULL;
			MultiPartCnt = 0;
			tmp = MIME_body_decode(DigestMaster, FALSE, FALSE, &tpMultiPart, &MultiPartCnt, &Index);
			mem_free(&tmp);
			SetWindowLong(hWnd, GWL_USERDATA, (long)DigestMaster);
			if (Decode(hWnd, j, DECODE_OPEN_IF_MSG) == TRUE) {
				return AttachMailItem;
			}
		}
		// else return to master
		tpMailItem = DigestMaster;
	} else {
		hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
		tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);

		if (AttachMailItem == NULL || tpMailItem != AttachMailItem) {
			Index = ListView_GetMemToItem(hListView, tpMailItem);
		} else {
			Index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
		}

		j = ListView_GetNextMailItem(hListView, Index);
		if (j == -1) {
			return NULL;
		}
		ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
		ListView_SetItemState(hListView,
			j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(hListView, j, TRUE);

		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, j);
	}
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModifyWindow(hWnd, tpMailItem, FALSE, FALSE);
	return tpMailItem;
}

/*
 * View_PrevMail - 前のメールを表示
 */
static MAILITEM *View_PrevMail(HWND hWnd)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int j;

	if (SelBox == MAILBOX_SEND) {
		return NULL;
	}
	if (DigestMessageNum > 0 && DigestMaster != NULL) {
		j = DigestMessageNum - 1;
		if (j > 0) {
			TCHAR *tmp;
			multipart_free(&tpMultiPart, MultiPartCnt);
			tpMultiPart = NULL;
			MultiPartCnt = 0;
			tmp = MIME_body_decode(DigestMaster, FALSE, FALSE, &tpMultiPart, &MultiPartCnt, &Index);
			mem_free(&tmp);
			SetWindowLong(hWnd, GWL_USERDATA, (long)DigestMaster);
			if (Decode(hWnd, j, DECODE_OPEN_IF_MSG) == TRUE) {
				return AttachMailItem;
			}
		}
		// else return to master
		tpMailItem = DigestMaster;
	} else {
		hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
		tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
		if (AttachMailItem == NULL || tpMailItem != AttachMailItem) {
			Index = ListView_GetMemToItem(hListView, tpMailItem);
		} else {
			Index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
		}

		j = ListView_GetPrevMailItem(hListView, Index);
		if (j == -1) {
			return NULL;
		}
		ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
		ListView_SetItemState(hListView,
			j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(hListView, j, TRUE);

		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, j);
	}
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModifyWindow(hWnd, tpMailItem, FALSE, FALSE);
	return tpMailItem;
}

/*
 * View_NextUnreadMail - 次の未開封メールを表示
 */
static MAILITEM *View_NextUnreadMail(HWND hWnd)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int i, j;

	if (SelBox == MAILBOX_SEND) {
		return NULL;
	}
	if (DigestMessageNum > 0 && DigestMaster != NULL) {
		return View_NextMail(hWnd);
	}
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if (AttachMailItem == NULL || tpMailItem != AttachMailItem) {
		Index = ListView_GetMemToItem(hListView, tpMailItem);
	} else {
		Index = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
	}

	// 未開封メールのインデックスを取得
	j = ListView_GetNextUnreadItem(hListView, Index,
		ListView_GetItemCount(hListView));
	if (j == -1) {
		j = ListView_GetNextUnreadItem(hListView, -1, Index);
	}
	if (j == -1) {
		if (op.ScanAllForUnread == 0) {
			return NULL;
		}

		// 次の未開封があるメールボックスのインデックスを取得
		SwitchCursor(FALSE);
		i = mailbox_next_unread(hWnd, SelBox + 1, MailBoxCnt);
		if (i == -1) {
			i = mailbox_next_unread(hWnd, MAILBOX_USER, SelBox);
		}
		if (i == -1) {
			SwitchCursor(TRUE);
			return NULL;
		}
		// メールボックスの選択
		mailbox_select(MainWnd, i);
		j = ListView_GetNextUnreadItem(hListView, -1,
			ListView_GetItemCount(hListView));
		SwitchCursor(TRUE);
	}
	ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
	ListView_SetItemState(hListView,
		j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_EnsureVisible(hListView, j, TRUE);

	tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, j);
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModifyWindow(hWnd, tpMailItem, FALSE, FALSE);
	SendMessage(MainWnd, WM_INITTRAYICON, 0, 0);
	return tpMailItem;
}

/*
 * View_Scroll - page up or down, switch messages if at end
 */
static void View_Scroll(HWND hViewWnd, int dir)
{
	TEXTMETRIC lptm;
	SCROLLINFO ScrollInfo;
	RECT rRect;
	HDC hdc;
	HFONT hFont;
	BOOL Next = FALSE;

	// フォントの高さを取得
	hdc = GetDC(hViewWnd);
	if (hViewFont != NULL) {
		hFont = SelectObject(hdc, hViewFont);
	}
	GetTextMetrics(hdc, &lptm);
	if (hViewFont != NULL) {
		SelectObject(hdc, hFont);
	}
	ReleaseDC(hViewWnd, hdc);

	// スクロール情報の取得
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_POS | SIF_RANGE;
	GetScrollInfo(hViewWnd, SB_VERT, &ScrollInfo);

	GetClientRect(hViewWnd,(LPRECT)&rRect);
	if (dir > 0) {
		ScrollInfo.nMax -= ((rRect.bottom - rRect.top) / (lptm.tmHeight + lptm.tmExternalLeading));
		if (ScrollInfo.nPos >= ScrollInfo.nMax) {
			Next = TRUE;
		}
		// PageDown
		SendMessage(hViewWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
		if (Next == TRUE) {
			HWND hParent = GetParent(hViewWnd);
			// 次の未開封メールへ移動する
			if (View_NextUnreadMail(hParent) == NULL && op.ViewCloseNoNext == 1) {
				SendMessage(hParent, WM_CLOSE, 0, 0);
			}
		}
	} else {
		if (ScrollInfo.nPos <= 0) {
			Next = TRUE;
		}
		SendMessage(hViewWnd, WM_VSCROLL, SB_PAGEUP, 0);
		if (Next == TRUE) {
			HWND hParent = GetParent(hViewWnd);
			if (View_PrevMail(hParent) == NULL && op.ViewCloseNoNext == 1) {
				SendMessage(hParent, WM_CLOSE, 0, 0);
			}
		}

	}
}

/*
 * View_FindMail - メール内から文字列を検索
 */
void View_FindMail(HWND hWnd, BOOL FindSet)
{
	MAILBOX *tpMailBox;
	HWND hEdit = NULL;
	TCHAR *buf = NULL;
	DWORD dwStart, dwEnd;
	BOOL Init = FALSE, FirstLoop = TRUE, Done = FALSE;
	int i = 0;

	FindOrReplace = 1;
	if (hWnd != MainWnd) {
		hEdit = GetDlgItem(hWnd, IDC_EDIT_BODY); // used here & far below
		if (FindSet != FALSE || FindStr == NULL) {
#ifdef UNICODE
			// 選択文字列を取得
			SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
			if (dwStart != dwEnd) {
				AllocGetText(hEdit, &buf);
				if (buf != NULL) {
					mem_free(&FindStr);
					FindStr = (TCHAR *)mem_alloc(sizeof(TCHAR) * (dwEnd - dwStart + 1));
					if (FindStr != NULL) {
						str_cpy_n_t(FindStr, buf + dwStart, dwEnd - dwStart + 1);
						Init = TRUE;
					}
					mem_free(&buf);
				}
			}
#else
			if (UnicodeEdit == 2) {
				WCHAR *wbuf;
				int len;
				// 選択文字列を取得
				SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
				if (dwStart != dwEnd) {
					AllocGetText(hEdit, &buf);
					if (buf != NULL) {
						len = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
						wbuf = (WCHAR *)mem_alloc(sizeof(WCHAR) * (len + 1));
						if (wbuf == NULL) {
							mem_free(&buf);
							return;
						}
						MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, len);
					
						len = WideCharToMultiByte(CP_ACP, 0, wbuf + dwStart, dwEnd - dwStart, NULL, 0, NULL, NULL);
						mem_free(&FindStr);
						FindStr = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
						if (FindStr != NULL) {
							WideCharToMultiByte(CP_ACP, 0, wbuf + dwStart, dwEnd - dwStart, FindStr, len + 1, NULL, NULL);
							*(FindStr + len) = TEXT('\0');
							Init = TRUE;
						}
						mem_free(&buf);
						mem_free(&wbuf);
					}
				}
			} else {
				// 選択文字列を取得
				SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
				if (dwStart != dwEnd) {
					AllocGetText(hEdit, &buf);
					if (buf != NULL) {
						mem_free(&FindStr);
						FindStr = (TCHAR *)mem_alloc(sizeof(TCHAR) * (dwEnd - dwStart + 1));
						if (FindStr != NULL) {
							str_cpy_n_t(FindStr, buf + dwStart, dwEnd - dwStart + 1);
							Init = TRUE;
						}
						mem_free(&buf);
					}
				}
			}
#endif
		}
	}
	
	if (FindSet != FALSE || FindStr == NULL) {
		if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_FIND), hWnd, SetFindProc) == FALSE) {
			return;
		}
		Init = TRUE;
	} else {
		FindNext = 0;
	}
	SwitchCursor(FALSE);

	if (hWnd == MainWnd) {
		HWND hListView = GetDlgItem(hWnd, IDC_LISTVIEW);
		FindBox = SelBox;
		i = 0;
		if (ListView_GetSelectedCount(hListView) > 0) {
			i = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
			if (i < 0) {
				i = 0;
			}
		}
		FindMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
	} else if (hWnd == hViewWnd) {
		FindBox = vSelBox;
		FindMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	} else { // hWnd = EditWnd
		FindBox = MAILBOX_SEND;
		FindMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	}
	tpMailBox = MailBox + FindBox;
	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		if (*(tpMailBox->tpMailItem + i) == FindMailItem) {
			break;
		}
	}
	if (i == tpMailBox->MailItemCnt) {
		i = 0; // shouldn't happen ...
	}

	if (Init == TRUE || (op.AllBoxFind == FALSE && FindStartBox != SelBox)) {
		FindStartBox = FindBox;
		FindStartItem = i;
	}

	if (FindNext != 0) {
		FirstLoop = FALSE;
	}

	while (Done == FALSE) {
		if (FirstLoop == TRUE && hWnd != MainWnd) {
			// 本文から検索して見つかった位置を選択状態にする
			if (FindEditString(hEdit, FindStr, op.MatchCase, FALSE) == TRUE) {
				break;
			}

			// １メール内での検索の場合
			if (op.AllMsgFind == 0) {
				// clear selection and search again
				SendMessage(hEdit, EM_SETSEL, 0, 0);
				if (FindEditString(hEdit, FindStr, op.MatchCase, FALSE) == TRUE) {
					break;
				}
			}
		} else if (FindMailItem != NULL && FindNext == 0) {
			BOOL Found = FALSE;
			TCHAR *s, *sbody;

			if (FindBox == MAILBOX_SEND) {
#ifdef UNICODE
				sbody = alloc_char_to_tchar(FindMailItem->Body);
#else
				sbody = FindMailItem->Body;
#endif
			} else if (FindMailItem->Body != NULL) {
				MULTIPART **findMultiPart = NULL;
				int cnt, TextIndex;
				BOOL ViewSrc = FALSE; // GJC: TRUE if searching headers?

				sbody = MIME_body_decode(FindMailItem, ViewSrc, TRUE, &findMultiPart, &cnt, &TextIndex);
				multipart_free(&findMultiPart, cnt);
			} else {
				sbody = NULL;
			}

			if (sbody != NULL) {
				s = str_find(FindStr, sbody, op.MatchCase);
			}
			if (sbody != NULL && *s != TEXT('\0')) {
				Found = TRUE;
			} else if (op.SubjectFind != 0 && FindMailItem->Subject != NULL) {
				s = str_find(FindStr, FindMailItem->Subject, op.MatchCase);
				if (*s != TEXT('\0')) {
					Found = TRUE;
				}
			}
#ifdef UNICODE
			mem_free(&sbody);
#else
			if (FindBox != MAILBOX_SEND) {
				mem_free(&sbody);
			}
#endif
			if (Found == TRUE) {
				int idx;
				HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
				if (hWnd != NULL && hWnd != MainWnd && hWnd != hViewWnd) {
					EndEditWindow(hWnd, FALSE);
				} else if (hWnd == hViewWnd && FindBox == MAILBOX_SEND) {
#ifdef _WIN32_WCE
					FocusWnd = MainWnd;
					ShowWindow(MainWnd, SW_SHOW);
					_SetForegroundWindow(MainWnd);
#endif
					EndWindow(hViewWnd);
					hViewWnd = NULL;
				}
				if (FindBox != SelBox) {
					mailbox_select(hWnd, FindBox);
				}
				ListView_SetItemState(hListView, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
				idx = ListView_GetMemToItem(hListView, FindMailItem);
				ListView_EnsureVisible(hListView, idx, TRUE);
				ListView_SetItemState(hListView, idx, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				SwitchCursor(TRUE);
				OpenItem(MainWnd, TRUE, TRUE);
				if (FindBox == MAILBOX_SEND) {
#ifdef _WIN32_WCE
					ShowWindow(hEditWnd, SW_SHOW);
					_SetForegroundWindow(hEditWnd);
					hEdit = GetDlgItem(hEditWnd, IDC_EDIT_BODY);
#else
					hEdit = GetDlgItem(FindMailItem->hEditWnd, IDC_EDIT_BODY);
#endif
				} else if (hViewWnd != NULL) {
#ifdef _WIN32_WCE
					ShowWindow(hViewWnd, SW_SHOW);
					_SetForegroundWindow(hViewWnd);
#endif
					hEdit = GetDlgItem(hViewWnd, IDC_EDIT_BODY);
				}
				if (hEdit != NULL) {
					FindEditString(hEdit, FindStr, op.MatchCase, FALSE);
				}
				break;
			}
		}
		FirstLoop = FALSE;

		if (op.AllMsgFind == 0) {
			buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FindStr) + lstrlen(STR_MSG_NOFIND) + 1));
			if (buf == NULL) {
				break;
			}
			wsprintf(buf, STR_MSG_NOFIND, FindStr);
			SwitchCursor(TRUE);
			MessageBox(hWnd, buf, STR_TITLE_FIND, MB_ICONINFORMATION);
			mem_free(&buf);
			break;
		}

		// Move to next message
		FindPos = 0;
		if (FindNext == 2) { // next mailbox
			i = tpMailBox->MailItemCnt;
		}
		FindNext = 0;
		i++;
		if (i >= tpMailBox->MailItemCnt) {
			if (op.AllBoxFind != 0) {
				FindBox++;
				while (FindBox != FindStartBox) {
					if (FindBox >= MailBoxCnt) {
						FindBox = 0;
					}
					if ((MailBox+FindBox)->Loaded == TRUE) {
						if ((MailBox+FindBox)->MailItemCnt > 0) {
							break;
						} 
					} else if (op.LazyLoadMailboxes != 2) {
						int load = mailbox_load_now(hWnd, FindBox, TRUE, FALSE);
						if (load == -1) {
							Done = TRUE;
							break;
						} else if (load == 1 && (MailBox+FindBox)->MailItemCnt > 0) {
							break;
						}
					}
					FindBox++;
				}
				if (Done == TRUE) {
					break;
				}
			}
			i = 0;
		}
		if (FindBox == FindStartBox && i == FindStartItem) {
			buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FindStr) + lstrlen(STR_TITLE_ALLFIND) + 1));
			if (buf == NULL) {
				break;
			}
			wsprintf(buf, STR_TITLE_ALLFIND, FindStr);
			SwitchCursor(TRUE);
			if (MessageBox(hWnd, STR_Q_NEXTFIND, buf, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
				mem_free(&buf);
				break;
			}
			mem_free(&buf);
			SwitchCursor(FALSE);
		}
		tpMailBox = MailBox + FindBox;
		if (tpMailBox->tpMailItem != NULL) {
			FindMailItem = *(tpMailBox->tpMailItem + i);
		} else {
			FindMailItem = NULL;
		}

		// ESC が押された場合は検索終了
		if (GetForegroundWindow() == hWnd && GetAsyncKeyState(VK_ESCAPE) < 0) {
			ESCFlag = TRUE;
			break;
		}

	}
	SwitchCursor(TRUE);
}

/*
 * ShellOpen - ファイルを関連付けで実行
 */
BOOL ShellOpen(TCHAR *FileName)
{
	TCHAR *p;
	BOOL retval;
#ifndef _WIN32_WCE_LAGENDA
	SHELLEXECUTEINFO sei;

	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(sei);
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	p = NULL;
	if (op.URLApp == NULL || *op.URLApp == TEXT('\0')) {
		sei.lpFile = FileName;
		sei.lpParameters = NULL;
	} else {
		sei.lpFile = op.URLApp;
		p = CreateCommandLine(op.URLAppCmdLine, FileName, FALSE);
		sei.lpParameters = (p != NULL) ? p : FileName;
	}
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = hInst;
	retval = ShellExecuteEx(&sei);
	mem_free(&p);
	return retval;
#else
	TCHAR *file;
	TCHAR *param;

	p = NULL;
	if (op.URLApp == NULL || *op.URLApp == TEXT('\0')) {
		file = FileName;
		param = NULL;
	} else {
		file = op.URLApp;
		p = CreateCommandLine(op.URLAppCmdLine, FileName, FALSE);
		param = (p != NULL) ? p : FileName;
	}
	retval = CoshExecute(NULL, file, param);
	mem_free(&p);
	return retval;
#endif
}

/*
 * OpenURL - エディットボックスから選択されたURLを抽出して開く
 */
static void OpenURL(HWND hWnd)
{
	TCHAR *buf;
	TCHAR *str;
	TCHAR *p, *r, *s;
	int i, j, k;
	int len;
	int MailToFlag = 0;

	// エディットボックスの選択位置の取得
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
	if (i >= j) {
		return;
	}

	// エディットボックスに設定された文字列の取得
	len = SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXTLENGTH, 0, 0) + 1;
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (buf == NULL) {
		return;
	}
	*buf = TEXT('\0');
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXT, len, (LPARAM)buf);

	// look for "click to download" string
	len = lstrlen(STR_MSG_PARTIAL_T);
	if (i <= len) {
		k = 0;
	} else {
		k = i - len;
	}
	for (r = buf + k; *r != TEXT('\0') && r < buf + (j + len); r++) {
		if (str_cmp_n_t(r, STR_MSG_PARTIAL_T, len) == 0) {
			mem_free(&buf);
			SendMessage(hWnd, WM_COMMAND, ID_MESSAGE_DOWNLOAD, 0);
			return;
		}
	}
	len = lstrlen(STR_MSG_ATTACHMENT);
	if (i <= len) {
		k = 0;
	} else {
		k = i - len;
	}
	for (r = buf + k; *r != TEXT('\0') && r < buf + (j + len); r++) {
		if (str_cmp_n_t(r, STR_MSG_ATTACHMENT, len) == 0) {
			r += len;
			for (s = r; *s != TEXT('\0') && *s != TEXT(']'); s++);
			if (*s == TEXT(']')) {
				char *fname;
				*s = TEXT('\0');
				fname = alloc_tchar_to_char(r);
				for (k = 0; k < MultiPartCnt && fname; k++) {
					if ((*(tpMultiPart + k))->Filename != NULL &&
						strcmp(fname, (*(tpMultiPart + k))->Filename) == 0) {
						Decode(hWnd, k, DECODE_ASK);
						break;
					}
				}
				mem_free(&fname);
			}
			mem_free(&buf);
			return;
		}
	}

	for (; *(buf + j) != TEXT('\0') && *(buf + j) != TEXT('\r') && *(buf + j) != TEXT('\n') && *(buf + j) != TEXT(' '); j++);
#ifdef UNICODE
	str = (TCHAR *)mem_alloc(sizeof(TCHAR) * (j - i + 2));
	if (str == NULL) {
		mem_free(&buf);
		return;
	}
	// 選択文字列の抽出
	for (p = buf + i, r = str; p < (buf + j); p++, r++) {
		*r = *p;
	}
	*r = TEXT('\0');
	mem_free(&buf);
#else
	if (UnicodeEdit == 2) {
		// WindowsXP
		WCHAR *wbuf, *wstr;
		WCHAR *wst, *wen, *wr;

		len = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
		wbuf = (WCHAR *)mem_alloc(sizeof(WCHAR) * (len + 1));
		if (wbuf == NULL) {
			mem_free(&buf);
			return;
		}
		MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, len);

		for (wst = wbuf + i; wst > wbuf && *wst != L'\r' && *wst != L'\n' && *wst != L'\t' && *wst != L' '; wst--);
		if (wst != wbuf) wst++;
		for (wen = wst; *wen != L'\0' && *wen != L'\r' && *wen != L'\n' && *wen != L'\t' && *wen != L' '; wen++);

		wstr = (WCHAR *)mem_calloc(sizeof(WCHAR) * (wen - wst + 1));
		if (wstr == NULL) {
			mem_free(&wbuf);
			mem_free(&buf);
			return;
		}
		// 選択文字列の抽出
		for (wr = wstr; wst < wen; wst++, wr++) {
			*wr = *wst;
		}
		*wr = L'\0';
		mem_free(&wbuf);
		mem_free(&buf);

		len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
		str = (char *)mem_alloc(len + 1);
		if (str == NULL) {
			mem_free(&wstr);
			return;
		}
		WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
		mem_free(&wstr);
	} else {
		str = (TCHAR *)mem_alloc(sizeof(TCHAR) * (j - i + 2));
		if (str == NULL) {
			mem_free(&buf);
			return;
		}
		// 選択文字列の抽出
		for (p = buf + i, r = str; p < (buf + j); p++, r++) {
			*r = *p;
		}
		*r = TEXT('\0');
		mem_free(&buf);
	}
#endif

	// 開始位置の取得
	for (s = str; *s == TEXT('(') || *s == TEXT(')') || *s == TEXT('\"') ||
		*s == TEXT('<') || *s == TEXT('>') || *s == TEXT('\t') || *s == TEXT(' '); s++);
	// URLの開始位置を取得
	for (p = s; *p != TEXT('\0'); p++) {
		if (str_cmp_ni_t(p, URL_HTTP, lstrlen(URL_HTTP)) == 0 ||
			str_cmp_ni_t(p, URL_HTTPS, lstrlen(URL_HTTPS)) == 0 ||
			str_cmp_ni_t(p, URL_FTP, lstrlen(URL_FTP)) == 0 ||
			str_cmp_ni_t(p, URL_MAILTO, lstrlen(URL_MAILTO)) == 0) {
			s = p;
			break;
		}
	}
	// 終了位置の取得
	for (p = s; *p != TEXT('\0'); p++) {
		if (*p == TEXT('(') || *p == TEXT(')') || *p == TEXT('\"') ||
			*p == TEXT('<') || *p == TEXT('>') || *p == TEXT(']') ||
			*p == TEXT('\r') || *p == TEXT('\n') || *p == TEXT('\t') ||
			*p == TEXT(' ')) { // || IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			*p = TEXT('\0');
			break;
		}
		if (*p == TEXT('@')) {
			if (MailToFlag != 0) {
				MailToFlag = -1;
			} else {
				MailToFlag = 1;
			}
		}
	}
	if (*s == TEXT('\0')) {
		mem_free(&str);
		return;
	}

	// URLのチェック
	if (str_cmp_ni_t(s, URL_HTTP, lstrlen(URL_HTTP)) == 0 ||
		str_cmp_ni_t(s, URL_HTTPS, lstrlen(URL_HTTPS)) == 0 ||
		str_cmp_ni_t(s, URL_FTP, lstrlen(URL_FTP)) == 0) {

#ifndef _WIN32_WCE
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)i, (LPARAM)i);
#endif
		ShellOpen(s);

	} else if (str_cmp_ni_t(s, URL_MAILTO, lstrlen(URL_MAILTO)) == 0 ||
		MailToFlag == 1) {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)i, (LPARAM)i);
		if (Edit_MailToSet(hInst, hWnd, s, vSelBox) == EDIT_INSIDEEDIT) {
#ifdef _WIN32_WCE
			ShowWindow(hWnd, SW_HIDE);
#endif
		}
	}
	mem_free(&str);
}

/*
 * SetReMessage - 返信の設定を行う
 */
static void SetReMessage(HWND hWnd, int ReplyFlag)
{
	HWND hEdit;
	MAILITEM *tpMailItem;
	TCHAR *buf = NULL, *seltext = NULL, *old_from = NULL, *old_cc = NULL;
	BOOL free_from = FALSE, free_cc = FALSE;
	DWORD selStart, selEnd;
	int ret;

	tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if (vSelBox <= 0 || item_is_mailbox(MailBox + vSelBox, tpMailItem) == -1) {
		ErrorMessage(hWnd, STR_ERR_NOMAIL);
		return;
	}
	if (DigestMaster != NULL) {
		int len, dlg;
		TCHAR *tmp;
		if (tpMailItem != AttachMailItem) {
			ErrorMessage(hWnd, TEXT("GJC programming error"));
		}
		if (ReplyFlag == EDIT_REPLY) {
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_DIGEST_REPLY), hWnd, DigestReplyProc, (LPARAM)&dlg) == FALSE) {
				return;
			}
			if (dlg == 1) { // reply to list
				old_from = tpMailItem->From;
				tpMailItem->From = alloc_copy_t(DigestMaster->From);
				free_from = TRUE;
			} else if (dlg == EDIT_REPLYALL) {
				ReplyFlag = EDIT_REPLYALL;
			} // else reply to sender of this message
		} 
		if (ReplyFlag == EDIT_REPLYALL) {
			if (DigestMaster->From != NULL) {
				old_from = tpMailItem->From;
				tpMailItem->From = alloc_copy_t(DigestMaster->From);
				free_from = TRUE;
			}
			if (old_from != NULL) {
				old_cc = tpMailItem->Cc;
				if (tpMailItem->Cc != NULL) {
					len = lstrlen(old_from) + lstrlen(tpMailItem->Cc) + 3;
					tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
					if (tmp != NULL) {
						str_join_t(tmp, old_from, TEXT(", "), tpMailItem->Cc, (TCHAR *)-1);
						tpMailItem->Cc = tmp;
						free_cc = TRUE;
					}
				} else {
					tpMailItem->Cc = alloc_copy_t(old_from);
					free_cc = TRUE;
				}
			}
		}
	}

	if (ReplyFlag != EDIT_REDIRECT) {
		hEdit = GetDlgItem(hWnd, IDC_EDIT_BODY);
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
		if (selStart != selEnd) {
			AllocGetText(hEdit, &buf);
			if (buf != NULL) {
				seltext = (TCHAR *)mem_alloc(sizeof(TCHAR) * (selEnd - selStart + 1));
				if (seltext != NULL) {
					str_cpy_n_t(seltext, buf + selStart, selEnd - selStart + 1);
				}
				mem_free(&buf);
			}
		}
	}

	ret = Edit_InitInstance(hInst, hWnd, vSelBox, tpMailItem, ReplyFlag, seltext, FALSE);
	if (DigestMaster != NULL) {
		if (free_from) {
			mem_free(&tpMailItem->From);
			tpMailItem->From = old_from;
		}
		if (free_cc) {
			mem_free(&tpMailItem->Cc);
			tpMailItem->Cc = old_cc;
		}
	}
	mem_free(&seltext);
#ifdef _WIN32_WCE
	if (ret == EDIT_INSIDEEDIT) {
		ShowWindow(hWnd, SW_HIDE);
	}
#else
	if (ret != EDIT_NONEDIT) {
		if (op.ViewClose == 1) {
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
	}
#endif
}

/*
 * Decode - 添付ファイルをデコードして保存
 */
static BOOL Decode(HWND hWnd, int id, int DoWhat)
{
	ATTACHINFO ai;
	TCHAR *fname, *ext = NULL;
	TCHAR buf[BUF_SIZE];
	TCHAR *p, *str;
	char *b64str, *ctype, *dstr, *endpoint;
	int len;
	int EncodeFlag = 0;
	BOOL is_digest = FALSE, is_msg = FALSE, save_embed = FALSE;
	BOOL ret = TRUE;

	if (DoWhat == DECODE_SAVE_ALL && (*(tpMultiPart + id))->ePos == NULL) {
		return FALSE; // saved as much as we can
	}

	if ((*(tpMultiPart + id))->IsDigestMsg == TRUE) {
		is_digest = TRUE;
		DigestMaster = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
		DigestMessageNum = id;
		DigestMessageCnt = MultiPartCnt;
	} else if (DoWhat == DECODE_OPEN_IF_MSG) {
		return FALSE;
	}

	SwitchCursor(FALSE);

	if ((*(tpMultiPart + id))->ePos != NULL) {
		len = (*(tpMultiPart + id))->ePos - (*(tpMultiPart + id))->sPos;
	} else {
		len = tstrlen((*(tpMultiPart + id))->sPos) + 1;
	}
	b64str = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (b64str == NULL) {
		SwitchCursor(TRUE);
		return FALSE;
	}
	if (len == 0) {
		*b64str = '\0';
	} else {
		str_cpy_n(b64str, (*(tpMultiPart + id))->sPos, len - 1);
	}

	if ((*(tpMultiPart + id))->Encoding != NULL) {
		if (str_cmp_i((*(tpMultiPart + id))->Encoding, "base64") == 0) {
			EncodeFlag = 1;
		} else if (str_cmp_i((*(tpMultiPart + id))->Encoding, "quoted-printable") == 0) {
			EncodeFlag = 2;
		}
	}
	switch (EncodeFlag) {
	case 1:
	case 2:
		// エンコードされている場合はデコードを行う
		dstr = (char *)mem_alloc(tstrlen(b64str));
		if (dstr == NULL) {
			mem_free(&b64str);
			SwitchCursor(TRUE);
			return FALSE;
		}
		endpoint = ((EncodeFlag == 1) ? base64_decode : QuotedPrintable_decode)(b64str, dstr, FALSE);
		mem_free(&b64str);
		break;

	default:
		dstr = b64str;
		endpoint = dstr + tstrlen(dstr);
		break;
	}

	ctype = alloc_copy((*(tpMultiPart + id))->ContentType);
	if (ctype != NULL) {
		char *tmp;
		for (tmp = ctype; *tmp != '\0' && *tmp != ';'; tmp++)
			/**/;
		*tmp = TEXT('\0');
		str = alloc_char_to_tchar(ctype);

		if (str_cmp_ni(ctype, "message/rfc822", tstrlen("message/rfc822")) == 0) {
			p = alloc_copy_t(TEXT(".eml"));
			is_msg = TRUE;
		} else {
			tmp = GetMIME2Extension(ctype, NULL);
			p = alloc_char_to_tchar(tmp);
			mem_free(&tmp);
			if (str_cmp_ni(ctype, "text/html", tstrlen("text/html")) == 0) {
				int found = 0;
				for (tmp = dstr; tmp < endpoint && found != 2; tmp++) {
					if (str_cmp_ni(tmp, "<img ", 5) == 0) {
						for ( ; tmp < endpoint; tmp++) {
							if (*tmp == '>') break;
							if (str_cmp_ni(tmp, "src=\"cid:", 9) == 0) {
								found = 2;
								break;
							}
						}
					}
				}
				if (found == 2) {
					save_embed = TRUE;
				}
			}
		}
		mem_free(&ctype);
	} else if (is_digest) {
		str = alloc_copy_t(TEXT("digest message"));
		p = alloc_copy_t(TEXT(".eml"));
		is_msg = TRUE;
	} else {
		p = NULL;
	}
	if (p != NULL) {
		ext = alloc_copy_t(p + 1);
	}

	if ((*(tpMultiPart + id))->Filename == NULL || *(*(tpMultiPart + id))->Filename == TEXT('\0')) {
		// ファイル名が無い場合
		if (p != NULL) {
			fname = mem_calloc(sizeof(TCHAR) * (8 + lstrlen(p) + 1));
			if (fname != NULL) {
				MD5_CTX context;
				unsigned char digest[16];

				MD5Init(&context);
				MD5Update(&context, dstr, endpoint - dstr);
				MD5Final(digest, &context);
				wsprintf(fname, TEXT("%02X%02X%02X%02X%s"), digest[0], digest[4], digest[8], digest[12], p);
				mem_free(&p);
			} else {
				fname = p;
			}
		} else {
			fname = alloc_copy_t(str);
		}
	} else {
		char *tmp = (*(tpMultiPart + id))->Filename;
		int i;
		mem_free(&p);
		fname = alloc_char_to_tchar(tmp);
		for (i = id+1; i < MultiPartCnt; i++) {
			char *tmp2 = (*(tpMultiPart + i))->Filename;
			if (tmp2 != NULL && tstrcmp(tmp, tmp2) == 0) {
				if ((*(tpMultiPart + i))->ePos != NULL) {
					int len2 = (*(tpMultiPart + i))->ePos - (*(tpMultiPart + i))->sPos;
					if (len2 > len) {
						int ans = MessageBox(hWnd, STR_Q_ATT_SAME_NAME, STR_TITLE_ATTACHED, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
						if (ans != IDNO) {
							mem_free(&str);
							mem_free(&ext);
							mem_free(&fname);
							mem_free(&dstr);
						}
						if (ans == IDCANCEL) {
							return FALSE;
						} else if (ans == IDYES) {
							return TRUE;
						}
						break;
					}
				}
			}
		}
	}

	if (fname != NULL) {
		// ファイル名に使えない文字は _ に変換する
		filename_conv(fname);
		// ファイル名が長すぎる場合は途中で切る
		if (lstrlen(fname) > BUF_SIZE - 50) {
			*(fname + BUF_SIZE - 50) = TEXT('\0');
		}
	}
	SwitchCursor(TRUE);

	// 確認ダイアログ
	AttachProcess = 0;
	if (DoWhat != DECODE_SAVE_ALL && DoWhat != DECODE_SAVE_EMBED) {
		if (DoWhat == DECODE_AUTO_OPEN
			|| (is_msg == TRUE && (DoWhat == DECODE_OPEN_IF_MSG || op.AutoOpenAttachMsg == 1))) {
			AttachProcess = 1;
		} else { // DECODE_ASK (or DECODE_OPEN_IF_MSG but !is_msg ?)
			ai.from = ((MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA))->From;
			ai.fname = fname;
			ai.mime = str;
			ai.size = endpoint - dstr;
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ATTACH_NOTICE), hWnd, AttachNoticeProc, (LPARAM)&ai) == FALSE) {
				AttachProcess = -1;
			}
		}
	}
	if (save_embed && AttachProcess >= 0) {
		if (ParanoidMessageBox(hWnd, STR_Q_SAVE_EMBEDDED, STR_TITLE_ATTACHED, MB_ICONQUESTION | MB_YESNO) == IDYES) {
			char *tmp = convert_cid(dstr, endpoint, tpMultiPart, MultiPartCnt, (AttachProcess == 1));
			if (tmp == NULL) {
				ErrorMessage(hWnd, TEXT("Content-ID conversion error"));
				save_embed = 0;
			} else {
				mem_free(&dstr);
				dstr = tmp;
				endpoint = dstr + tstrlen(dstr);
			}
		}
	}
	if (AttachProcess == 0) {
		// save it
		ret = file_save_attach(hWnd, fname, ext, dstr, endpoint - dstr, DoWhat);
	} else if (AttachProcess == 1 && is_msg == TRUE) {
		item_free(&AttachMailItem, 1);
		AttachMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
		if (AttachMailItem == NULL) {
			ret = FALSE;
		} else {
			ret = item_mail_to_item(AttachMailItem, dstr, endpoint - dstr, 2, NULL);
			if (ret == TRUE) {
				if (AttachMailItem->Body == NULL) {
					ret = FALSE;
				} else {
					mem_free(&AttachMailItem->MailBox);
					if ((MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
						AttachMailItem->MailBox = alloc_copy_t(((MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA))->MailBox);
					} else {
						AttachMailItem->MailBox = alloc_copy_t((MailBox+vSelBox)->Name);
					}
					if (DigestMaster == NULL) {
						// if we're already in an attachment, don't reset the master
						DigestMaster = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
					}
					SetWindowLong(hWnd, GWL_USERDATA, (long)AttachMailItem);
					ModifyWindow(hWnd, AttachMailItem, FALSE, FALSE);
				}
			}
		}
	} else if (AttachProcess == 1) {
		if (save_embed) {
			int i;
			for (i = 0; i < MultiPartCnt; i++) {
				if ((*(tpMultiPart + i))->EmbeddedImage == TRUE) {
					Decode(hWnd, i, DECODE_SAVE_EMBED);
					//(*(tpMultiPart + i))->EmbeddedImage = FALSE;
				}
			}
		}
		// 保存して実行
//		if (op.AttachWarning == 0 ||
//			MessageBox(hWnd, STR_Q_ATTACH, STR_TITLE_OPEN, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
			if (op.AttachDelete != 0) {
				wsprintf(buf, TEXT("%s%s"), ATTACH_FILE, fname);
				p = buf;
			} else {
				p = fname;
			}
			ret = file_save_exec(hWnd, p, dstr, endpoint - dstr);
//		}
	}
	mem_free(&str);
	mem_free(&ext);
	mem_free(&fname);
	mem_free(&dstr);
	return ret;
}

/*
 * DeleteAttachFile - 添付ファイルの削除
 */
BOOL DeleteAttachFile(HWND hWnd, MAILITEM *tpMailItem)
{
	MULTIPART **tpPart = NULL;
	MAILBOX *tpMailBox;
	char *mBody;
#ifdef UNICODE
	char *ContentType;
#endif
	int cnt = 0;
	int TextIndex = -1;
	int i, len = 0;

	// マルチパートの解析
#ifdef UNICODE
	ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
	cnt = multipart_parse(ContentType, tpMailItem->Body, TRUE, &tpPart, 0);
	mem_free(&ContentType);
#else
	cnt = multipart_parse(tpMailItem->ContentType, tpMailItem->Body, TRUE, &tpPart, 0);
#endif
	if (cnt == 0) {
		return FALSE;
	}

	// テキストのパートを検索
	for (i = 0; i < cnt; i++) {
		if ((*(tpPart + i))->ContentType == NULL ||
			str_cmp_ni((*(tpPart + i))->ContentType, "text", tstrlen("text")) == 0) {
			TextIndex = i;
			break;
		}
	}
	if (TextIndex == -1) {
		if (MessageBox(hWnd, STR_Q_NOBODYDELATT, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
			multipart_free(&tpPart, cnt);
			return FALSE;
		} else {
			mBody = (char *)mem_alloc(sizeof(char));
			*mBody = '\0';
		}
	} else if ((*tpPart + TextIndex)->ePos == NULL) {
		mBody = alloc_copy((*tpPart + TextIndex)->sPos);
		if (mBody == NULL) {
			multipart_free(&tpPart, cnt);
			return FALSE;
		}
		len = strlen(mBody);
	} else {
		len = (*tpPart + TextIndex)->ePos - (*tpPart + TextIndex)->sPos;
		mBody = (char *)mem_alloc(sizeof(char) * (len + 1));
		if (mBody == NULL) {
			multipart_free(&tpPart, cnt);
			return FALSE;
		}
		if (len == 0) {
			*mBody = '\0';
		} else {
			str_cpy_n(mBody, (*tpPart + TextIndex)->sPos, len - 1);
		}
	}
	if (tpMailItem->HasHeader) {
		char *p, *newbody;
		p = GetBodyPointa(tpMailItem->Body);
		if (p != NULL) {
			int hlen = p - tpMailItem->Body;
			len = hlen + strlen(mBody) + 2;
			newbody = (char *)mem_alloc(sizeof(char) * len);
			if (newbody != NULL) {
				str_cpy_n(newbody, tpMailItem->Body, hlen + 1);
				str_cpy_n(newbody + hlen, mBody, strlen(mBody)+1);
				mem_free(&mBody);
				mBody = newbody;
			}
		}
	}

	// 内容の置き換え
	mem_free(&tpMailItem->Body);
	tpMailItem->Body = mBody;
	mem_free(&tpMailItem->Encoding);
	mem_free(&tpMailItem->ContentType);
	if (TextIndex != -1) {
		tpMailItem->Encoding = alloc_char_to_tchar((*tpPart + TextIndex)->Encoding);
		tpMailItem->ContentType = alloc_char_to_tchar((*tpPart + TextIndex)->ContentType);
	}
	mem_free(&tpMailItem->Attach);
	tpMailItem->Attach = alloc_copy_t(TEXT("_"));
	tpMailItem->Multipart = MULTIPART_ATTACH;
	tpMailItem->Download = FALSE;

	if (hWnd == MainWnd) {
		tpMailBox = (MailBox + SelBox);
	} else {
		tpMailBox = (MailBox + vSelBox);
	}
	tpMailBox->NeedsSave |= MAILITEMS_CHANGED;
	if (tpMailBox->Type == MAILBOX_TYPE_SAVE) {
		TCHAR num[20];
		TCHAR *s;
		wsprintf(num, TEXT("%d"), len);
		s = alloc_copy_t(num);
		if (s != NULL) {
			mem_free(&tpMailItem->Size);
			tpMailItem->Size = s;
		}
	}
	if (hWnd != MainWnd && SelBox == vSelBox) {
		HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
		i = ListView_GetMemToItem(hListView, tpMailItem);
		ListView_SetItemState(hListView, i, LVIS_CUT, LVIS_CUT);
		ListView_RedrawItems(hListView, i, i);
	}

	multipart_free(&tpPart, cnt);
	return TRUE;
}

/*
 * SaveViewMail - メールをファイルに保存
 */
static BOOL SaveViewMail(TCHAR *fname, HWND hWnd, int MailBoxIndex, MAILITEM *tpMailItem, TCHAR *head, BOOL ViewSrc)
{
	MULTIPART **tpPart = NULL;
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR *buf;
	TCHAR *tmp;
	int len = 0, cnt = 0, idx;
	
	if (fname == NULL) {
		// ファイルの作成
		if (tpMailItem->Subject != NULL) {
			str_cpy_n_t(path, tpMailItem->Subject, BUF_SIZE - 50);
			filename_conv(path);
			lstrcpy(path + lstrlen(path), TEXT(".txt"));
		} else {
			lstrcpy(path, TEXT(".txt"));
		}
		// ファイル名の取得
		if (filename_select(hWnd, path, TEXT("txt"), STR_TEXT_FILTER, FILE_SAVE_MSG, &op.SavedSaveDir) == FALSE) {
			return TRUE;
		}
		if (item_is_mailbox(MailBox + MailBoxIndex, tpMailItem) == -1) {
			return FALSE;
		}
		fname = path;
	}

	// 保存するファイルを開く
	hFile = CreateFile(fname, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}

	if (ViewSrc == TRUE) {
		char *cbuf;
		BOOL added = FALSE;
		if (tpMailItem->MailBox == NULL && (MailBox + MailBoxIndex)->Name != NULL) {
			tpMailItem->MailBox = alloc_copy_t((MailBox + MailBoxIndex)->Name);
			added = TRUE;
		}
		len = item_to_string_size(tpMailItem, op.WriteMbox, TRUE, FALSE);
		cbuf = (char *)mem_alloc(sizeof(char) * (len + 1));
		if (cbuf == NULL) {
			if (added) {
				mem_free(&tpMailItem->MailBox);
			}
			CloseHandle(hFile);
			return FALSE;
		}
		item_to_string(cbuf, tpMailItem, op.WriteMbox, TRUE, FALSE);
		if (added) {
			mem_free(&tpMailItem->MailBox);
		}
#ifdef UNICODE
		buf = alloc_char_to_tchar(cbuf);
		mem_free(&cbuf);
#else
		buf = cbuf;
#endif
	} else {
		// ヘッダの保存
		len = CreateHeaderStringSize(head, tpMailItem, NULL);
		tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (tmp == NULL) {
			CloseHandle(hFile);
			return FALSE;
		}
		CreateHeaderString(head, tmp, tpMailItem, NULL);
		len = lstrlen(tmp); // may be shorter by 2 characters

		if (file_write_ascii(hFile, tmp, len) == FALSE) {
			mem_free(&tmp);
			CloseHandle(hFile);
			return FALSE;
		}
		mem_free(&tmp);

		buf = MIME_body_decode(tpMailItem, ViewSrc, TRUE, &tpPart, &cnt, &idx);

		// GJC remove HTML tags
		if (ViewSrc == FALSE && op.StripHtmlTags == 1 && lstrcmp(op.ViewFileSuffix, TEXT("txt")) == 0 &&
			((tpMailItem->ContentType != NULL && str_cmp_ni_t(tpMailItem->ContentType, TEXT("text/html"), lstrlen(TEXT("text/html")))==0)
			|| (idx != -1 && (tpPart[idx])->ContentType != NULL &&
			str_cmp_ni((tpPart[idx])->ContentType, "text/html", tstrlen("text/html")) == 0))) {
			tmp = strip_html_tags(buf, FALSE);
			if (tmp != NULL) {
				mem_free(&buf);
				buf = tmp;
			}
		}
		multipart_free(&tpPart, cnt);
	}

	// 本文の保存
	if (buf != NULL && file_write_ascii(hFile, buf, lstrlen(buf)) == FALSE) {
		CloseHandle(hFile);
		mem_free(&buf);
		return FALSE;
	}
	CloseHandle(hFile);
	mem_free(&buf);
	return TRUE;
}

/*
 * AppViewMail - メールをファイルに保存して外部アプリケーションで表示
 */
static BOOL AppViewMail(MAILITEM *tpMailItem, int MailBoxIndex)
{
	MULTIPART **tpPart = NULL;
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
	BOOL ViewSrc, retval;

	// メールをファイルに保存
	str_join_t(path, DataDir, VIEW_FILE, TEXT("."), op.ViewFileSuffix, (TCHAR *)-1);
	ViewSrc = (ViewWndViewSrc == TRUE || op.ViewAppMsgSource == 1);
	if (SaveViewMail(path, NULL, MailBoxIndex, tpMailItem, op.ViewFileHeader, ViewSrc) == FALSE) {
		return FALSE;
	}

#ifndef _WIN32_WCE_LAGENDA
	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(sei);
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;

	p = NULL;
	if (op.ViewApp == NULL || *op.ViewApp == TEXT('\0')) {
		sei.lpFile = path;
		sei.lpParameters = NULL;
	} else {
		sei.lpFile = op.ViewApp;
		p = CreateCommandLine(op.ViewAppCmdLine, path, FALSE);
		sei.lpParameters = (p != NULL) ? p : path;
	}
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = hInst;
	retval = ShellExecuteEx(&sei);
	mem_free(&p);
	return retval;
#else
	p = NULL;
	if (op.ViewApp == NULL || *op.ViewApp == TEXT('\0')) {
		file = path;
		param = NULL;
	} else {
		file = op.ViewApp;
		p = CreateCommandLine(op.ViewAppCmdLine, path, FALSE);
		param = (p != NULL) ? p : path;
	}
	retval = CoshExecute(NULL, file, param);
	mem_free(&p);
	return retval;
#endif
}

/*
 * SetMark - アイテムにマークを付加
 */
static void SetMark(HWND hWnd, MAILITEM *tpMailItem, const int mark)
{
	HWND hListView;
	int i, cut = LVIS_CUT;

	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	i = ListView_GetMemToItem(hListView, tpMailItem);

	if (mark == ICON_MAIL && (tpMailItem->MailStatus == ICON_READ || tpMailItem->MailStatus == ICON_MAIL)) {
		tpMailItem->MailStatus = (tpMailItem->MailStatus == ICON_READ) ? ICON_MAIL : ICON_READ;
		tpMailItem->Mark = tpMailItem->MailStatus;
		if (tpMailItem->MailStatus == ICON_MAIL) {
			tpMailItem->ReFwd &= ~(ICON_REFWD_MASK);
		}
	} else if (tpMailItem->Mark == mark) {
		tpMailItem->Mark = tpMailItem->MailStatus;
	} else if (tpMailItem->MailStatus == ICON_NON) {
		tpMailItem->Mark = (tpMailItem->Mark == ICON_NON) ? mark : ICON_NON;
	} else {
		tpMailItem->Mark = mark;
		cut = 0;
	}
	if (vSelBox > 0) {
		(MailBox+vSelBox)->NeedsSave |= MARKS_CHANGED;
	}

	if (i != -1) {
		if (tpMailItem->Download == TRUE) {
			cut = 0;
		}
		ListView_SetItemState(hListView, i, cut, LVIS_CUT);
		ListView_SetItemState(hListView, i, INDEXTOOVERLAYMASK(tpMailItem->ReFwd & ICON_REFWD_MASK), LVIS_OVERLAYMASK);
		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);
}

/*
 * GetMarkStatus - アイテム状態取得
 */
static void GetMarkStatus(HWND hWnd, MAILITEM *tpMailItem)
{
	HMENU hMenu;
	HWND htv;
	int enable;
	BOOL IsAttach = (tpMailItem == AttachMailItem) ? TRUE : FALSE;
	BOOL IsSaveBox = ((MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) ? TRUE : FALSE;
#ifndef _WIN32_WCE_LAGENDA
	LPARAM lp;
#endif

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	int xsize = GetSystemMetrics(SM_CXSCREEN);
	int hidedel = 0;
	hMenu = SHGetSubMenu(hViewToolBar, ID_MENUITEM_FILE);
	htv = hViewToolBar;
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 0);
	htv = NULL;
#else
	hMenu = GetSubMenu(CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0), 2);
	htv = GetDlgItem(hWnd, IDC_VCB);
#endif
#else
	hMenu = GetMenu(hWnd);
	htv = GetDlgItem(hWnd, IDC_VTB);
#endif

	enable = !(vSelBox == RecvBox && ExecFlag == TRUE);
	if (IsSaveBox == TRUE || IsAttach == TRUE) {
		enable = 0;
		DeleteMenu(hMenu, ID_MENUITEM_DELETE, MF_BYCOMMAND); // may have been there already
		DeleteMenu(hMenu, ID_MENUITEM_DOWNMARK, MF_BYCOMMAND);
		DeleteMenu(hMenu, ID_MENUITEM_DELMARK, MF_BYCOMMAND);
		if (IsAttach == TRUE) {
			DeleteMenu(hMenu, ID_MENUITEM_FLAGMARK, MF_BYCOMMAND);
			EnableMenuItem(hMenu, ID_MENUITEM_READMAIL, MF_GRAYED);
			EnableMenuItem(hMenu, ID_MENUITEM_UNREADMAIL, MF_GRAYED);
		} else {
#ifndef _WIN32_WCE
			hMenu = GetSubMenu(hMenu, 0);
#endif
#ifdef _WIN32_WCE_PPC
			InsertMenu(hMenu, ID_MENUITEM_FLAGMARK, MF_BYCOMMAND | MF_STRING, ID_MENUITEM_DELETE,
				STR_LIST_PPCMENU_DELLIST);
#else
			InsertMenu(hMenu, 12, MF_BYPOSITION | MF_STRING, ID_MENUITEM_DELETE,
				STR_LIST_MENU_DELLIST);
#endif
			CheckMenuItem(hMenu, ID_MENUITEM_FLAGMARK, (tpMailItem->Mark == ICON_FLAG) ? MF_CHECKED : MF_UNCHECKED);
		}
	} else {
#ifndef _WIN32_WCE_PPC
		DeleteMenu(hMenu, ID_MENUITEM_DELETE, MF_BYCOMMAND);
#endif
		CheckMenuItem(hMenu, ID_MENUITEM_FLAGMARK, (tpMailItem->Mark == ICON_FLAG) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_MENUITEM_DOWNMARK, (tpMailItem->Mark == ICON_DOWN) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, ID_MENUITEM_DELMARK, (tpMailItem->Mark == ICON_DEL) ? MF_CHECKED : MF_UNCHECKED);
		EnableMenuItem(hMenu, ID_MENUITEM_DOWNMARK, !enable);
		EnableMenuItem(hMenu, ID_MENUITEM_DELMARK, !enable);
	}

#ifndef _WIN32_WCE_LAGENDA
	lp = (LPARAM)MAKELONG(enable, 0);
	SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_DOWNMARK, lp);
	SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_DELMARK, lp);

	if (IsAttach == TRUE) {
		lp = (LPARAM)MAKELONG(0, 0);
		SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_FLAGMARK, lp);
		SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_UNREADMARK, lp);
		SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_DELETE, lp);
	} else if (IsSaveBox) {
		SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_DELETE, (LPARAM)MAKELONG(1, 0));
	}

#ifdef _WIN32_WCE_PPC
	if (xsize >= 300 || op.ShowNavButtons == 0) {
#endif
	lp = (LPARAM) MAKELONG((tpMailItem->Mark == ICON_FLAG) ? 1 : 0, 0);
	SendMessage(htv, TB_CHECKBUTTON, ID_MENUITEM_FLAGMARK, lp);
	SendMessage(htv, TB_PRESSBUTTON, ID_MENUITEM_FLAGMARK, lp);

	if (IsSaveBox == FALSE && IsAttach == FALSE) {
		lp = (LPARAM) MAKELONG((tpMailItem->Mark == ICON_DEL) ? 1 : 0, 0);
		SendMessage(htv, TB_CHECKBUTTON, ID_MENUITEM_DELMARK, lp);
		SendMessage(htv, TB_PRESSBUTTON, ID_MENUITEM_DELMARK, lp);
	}

#ifdef _WIN32_WCE_PPC
	}
	if (xsize >= 300) {
#endif
	lp = (LPARAM) MAKELONG((tpMailItem->Mark == ICON_DOWN) ? 1 : 0, 0);
	SendMessage(htv, TB_CHECKBUTTON, ID_MENUITEM_DOWNMARK, lp);
	SendMessage(htv, TB_PRESSBUTTON, ID_MENUITEM_DOWNMARK, lp);

	lp = (LPARAM) MAKELONG((tpMailItem->Mark == ICON_READ) ? 1 : 0, 0);
	SendMessage(htv, TB_CHECKBUTTON, ID_MENUITEM_UNREADMARK, lp);
	SendMessage(htv, TB_PRESSBUTTON, ID_MENUITEM_UNREADMARK, lp);

#ifdef _WIN32_WCE_PPC
		lp = (LPARAM)MAKELONG(0, 0);
		SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_PREVMAIL,   lp);
		SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_NEXTMAIL,   lp);
		SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_NEXTUNREAD, lp);
		SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_NEXTFIND,   lp);
		SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_DOWNMARK,   lp);
		SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_FLAGMARK,   lp);
	} else {
		// xsize < 300: hide some buttons
		lp = (LPARAM)MAKELONG(1, 0);
		if (op.ShowNavButtons) {
			SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_NEXTFIND,   lp);
			SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_FLAGMARK,   lp);
			hidedel = 1;
		} else {
			SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_PREVMAIL,   lp);
			SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_NEXTMAIL,   lp);
			SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_NEXTUNREAD, lp);
		}
		SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_DOWNMARK,   lp);
	}
	SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_DELMARK, (LPARAM)MAKELONG(hidedel || IsSaveBox == TRUE, 0));
	SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_DELETE,  (LPARAM)MAKELONG(hidedel || IsSaveBox == FALSE, 0));
	SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_UNREADMARK, (LPARAM)MAKELONG((xsize < 350), 0));
#else
	SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_DELMARK, (LPARAM)MAKELONG(IsSaveBox == TRUE, 0));
	SendMessage(htv, TB_HIDEBUTTON, ID_MENUITEM_DELETE,  (LPARAM)MAKELONG(IsSaveBox == FALSE, 0));
#endif
#endif
}

/*
 * ViewDeleteItem - delete item from list (not from server)
 */
static MAILITEM *ViewDeleteItem(HWND hWnd, MAILITEM *delItem) {
	HWND hListView = NULL;
	MAILITEM *tpNextMail = NULL;
	int i;

	// delete item from listview
	if (SelBox == vSelBox) {
		hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
		i = -1;
		while ((i = ListView_GetNextItem(hListView, i, 0)) != -1) {
			MAILITEM *tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
			if (tpMailItem == delItem) {
				ListView_DeleteItem(hListView, i);
				ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
				ListView_SetItemState(hListView,
					i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(hListView, i, TRUE);
				tpNextMail = (MAILITEM *)ListView_GetlParam(hListView, i);
				break;
			}
		}
	}
	// free item memory in mailbox
	for (i = 0; i < (MailBox + vSelBox)->MailItemCnt; i++) {
		MAILITEM *tpMailItem = *((MailBox + vSelBox)->tpMailItem + i);
		if (tpMailItem == delItem) {
			if (vSelBox == MAILBOX_SEND) {
				ClearFwdHold(tpMailItem);
			}
			item_free(((MailBox + vSelBox)->tpMailItem + i), 1);
			break;
		}
	}
	item_resize_mailbox(MailBox + vSelBox);
	return tpNextMail;
}

/*
 * ViewProc - メール表示プロシージャ
 */
static LRESULT CALLBACK ViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MAILITEM *tpMailItem, *tpNextMail;
	int key, i, command_id;
	BOOL ret, del_it = FALSE, ask = TRUE;
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	static BOOL SipFlag = FALSE;
#endif

	key = GetKeyState(VK_SHIFT);
	if (key >= 0) {
		key = GetKeyState(VK_CONTROL);
	}

	switch (msg) {
	case WM_CREATE:
		if (InitWindow(hWnd, (MAILITEM *)(((CREATESTRUCT *)lParam)->lpCreateParams)) == FALSE) {
			DestroyWindow(hWnd);
			break;
		}
		lParam = (LPARAM)(((CREATESTRUCT *)lParam)->lpCreateParams);
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		SipFlag = FALSE;
#endif

	case WM_MODFYMESSAGE:
		SetWindowLong(hWnd, GWL_USERDATA, lParam);
		ModifyWindow(hWnd, (MAILITEM *)lParam, FALSE, FALSE);
		_SetForegroundWindow(hWnd);
		break;

	case WM_CHANGE_MARK:
		tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
		if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
			break;
		}
		GetMarkStatus(hWnd, tpMailItem);
		break;

#ifdef _WIN32_WCE_PPC
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SHACTIVATEINFO sai;

			memset(&sai, 0, sizeof(SHACTIVATEINFO));
			SHHandleWMSettingChange(hWnd, -1, 0, &sai);
			SipFlag = sai.fSipUp;
			SetWindowSize(hWnd, 0, 0);
		}
		if (LastXSize_V != GetSystemMetrics(SM_CXSCREEN)) {
			SendMessage(hWnd, WM_CHANGE_MARK, 0, 0);
			LastXSize_V = GetSystemMetrics(SM_CXSCREEN);
		}
		break;
#elif defined _WIN32_WCE_LAGENDA
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SipFlag = SetWindowSize(hWnd, 0, 0);
		}
		break;
#endif

	case WM_SIZE:
		SetWindowSize(hWnd, wParam, lParam);
		break;

#ifndef _WIN32_WCE
	case WM_EXITSIZEMOVE:
		if (IsWindowVisible(hWnd) != 0 && IsIconic(hWnd) == 0 && IsZoomed(hWnd) == 0) {
			GetWindowRect(hWnd, (LPRECT)&op.ViewRect);
		}
		break;
#endif

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));
		if (op.ViewWindowCursor == 0) {
			HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
		}
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
		if (ESCFlag == TRUE) {
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
		if (LOWORD(lParam) == 1) {
			SetEditMenu(hWnd);
		}
#endif
		break;

	case WM_TIMER:
		switch (wParam) {
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
		command_id = GET_WM_COMMAND_ID(wParam, lParam);
		switch (command_id) {
#ifdef _WIN32_WCE_PPC
		case ID_MENU:
			SetEditMenu(hWnd);
			ShowMenu(hWnd, SHGetSubMenu(hViewToolBar, ID_MENUITEM_VIEW), 0, 0);
			break;

		case IDC_EDIT_BODY:
			switch (HIWORD(wParam)) {
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
			switch (HIWORD(wParam)) {
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
			if (op.WordBreakFlag == 1) {
				View_Scroll(GetDlgItem(hWnd, IDC_EDIT_BODY), -1);
			} else {
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_HSCROLL, SB_LINELEFT, 0);
			}
			break;

		case ID_KEY_RIGHT:
			if (op.WordBreakFlag == 1) {
				View_Scroll(GetDlgItem(hWnd, IDC_EDIT_BODY), +1);
			} else {
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_HSCROLL, SB_LINERIGHT, 0);
			}
			break;

		case ID_MENUITEM_NEXTMAIL:
			if (View_NextMail(hWnd) == NULL && op.ViewCloseNoNext == 1) {
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

		case ID_MENUITEM_PREVMAIL:
			if (View_PrevMail(hWnd) == NULL && op.ViewCloseNoNext == 1) {
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

		case ID_MENUITEM_NEXTUNREAD:
			if (View_NextUnreadMail(hWnd) == NULL && op.ViewCloseNoNext == 1) {
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

		case ID_MENUITEM_REMESSEGE:
			SetReMessage(hWnd, EDIT_REPLY);
			break;

		case ID_MENUITEM_ALLREMESSEGE:
			SetReMessage(hWnd, EDIT_REPLYALL);
			break;

		case ID_MENUITEM_FORWARD:
			SetReMessage(hWnd, EDIT_FORWARD);
			break;

		case ID_MENUITEM_REDIRECT:
			SetReMessage(hWnd, EDIT_REDIRECT);
			break;

		case ID_MENUITEM_FLAGMARK:
			if (vSelBox <= MAILBOX_SEND) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			SetMark(hWnd, tpMailItem, ICON_FLAG);
			GetMarkStatus(hWnd, tpMailItem);
			break;

		// 受信用にマーク
		case ID_MENUITEM_DOWNMARK:
		case ID_MESSAGE_DOWNLOAD:
			if (vSelBox <= MAILBOX_SEND || (MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (command_id == ID_MESSAGE_DOWNLOAD) {
				if (tpMailItem->Download == TRUE) {
					break;
				} else {
					// in case message was already marked to download
					tpMailItem->Mark = tpMailItem->MailStatus;
				}
			}
			SetMark(hWnd, tpMailItem, ICON_DOWN);
			GetMarkStatus(hWnd, tpMailItem);
			if (command_id == ID_MESSAGE_DOWNLOAD) {
				PostMessage(MainWnd, WM_COMMAND, ID_MESSAGE_DOWNLOAD, (LPARAM)vSelBox);
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

		case ID_MENUITEM_UNREADMARK:
			if (vSelBox <= MAILBOX_SEND) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			SetMark(hWnd, tpMailItem, ICON_MAIL);
			GetMarkStatus(hWnd, tpMailItem);
			break;

		case ID_KEY_DELETE:
		case ID_MENUITEM_DELMARK:
		case ID_MENUITEM_DELETE:
			if (vSelBox <= MAILBOX_SEND) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (command_id == ID_MENUITEM_DELMARK) {
				del_it = FALSE;
			} else if (command_id == ID_MENUITEM_DELETE || (MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
				del_it = TRUE;
			} else {
				del_it = FALSE;
			}
			if (del_it == FALSE) {
				// mark for delete

				if ((MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
					break;
				}
				SetMark(hWnd, tpMailItem, ICON_DEL);
				GetMarkStatus(hWnd, tpMailItem);
				if (tpMailItem->Mark == ICON_DEL) {
					if (op.ViewNextAfterDel == 1) {
						tpMailItem = View_NextMail(hWnd);
					} else if (op.ViewNextAfterDel == 2) {
						tpMailItem = View_NextUnreadMail(hWnd);
					}
					if (tpMailItem == NULL && op.ViewCloseNoNext == 1) {
						SendMessage(hWnd, WM_CLOSE, 0, 0);
					}
				}
			} else {
				// delete from list
				if (tpMailItem->ReFwd & REFWD_FWDHOLD) {
					if (MessageBox(hWnd, STR_Q_DEL_FWDHOLD, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
						break;
					}
				} else if (op.ExpertMode != 1 || key >=0) {
					TCHAR buf[BUF_SIZE];
					wsprintf(buf, STR_Q_DELLISTMAIL, 1, TEXT(""));
					if (MessageBox(hWnd, buf, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
						break;
					}
				}
				tpNextMail = ViewDeleteItem(hWnd, tpMailItem);
				if (op.ViewNextAfterDel == 0) {
					tpNextMail = NULL;
				} else if (op.ViewNextAfterDel == 2 && (tpNextMail == NULL || tpMailItem->MailStatus != ICON_MAIL)) {
					tpNextMail = View_NextUnreadMail(hWnd);
				}
				if (tpNextMail == NULL && op.ViewCloseNoNext == 1) {
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				} else if (tpNextMail != NULL) {
					SetWindowLong(hWnd, GWL_USERDATA, (long)tpNextMail);
					ModifyWindow(hWnd, tpNextMail, FALSE, FALSE);
				}
			}
			break;

		case ID_MENUITEM_READMAIL:
			if (vSelBox <= MAILBOX_SEND) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (tpMailItem->MailStatus == ICON_READ) {
				break;
			}
			SetMark(hWnd, tpMailItem, ICON_MAIL);
			GetMarkStatus(hWnd, tpMailItem);
			break;

		case ID_MENUITEM_UNREADMAIL:
			if (vSelBox <= MAILBOX_SEND) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (tpMailItem->MailStatus == ICON_MAIL) {
				break;
			}
			SetMark(hWnd, tpMailItem, ICON_MAIL);
			GetMarkStatus(hWnd, tpMailItem);
			break;

		case ID_MENUITEM_SAVE:
			if (vSelBox <= MAILBOX_SEND) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == -1) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (SaveViewMail(NULL, hWnd, vSelBox, tpMailItem, SAVE_HEADER, FALSE) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_SAVE);
			}
			break;

		case ID_MENUITEM_PROP:
			if (item_is_mailbox(MailBox + vSelBox, 
				(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == -1) {
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
#ifdef _WIN32_WCE_PPC
			op.WordBreakFlag = SetWordBreak(hWnd, hViewToolBar);
#elif defined(_WIN32_WCE_LAGENDA)
			op.WordBreakFlag = SetWordBreak(hWnd, hViewMenu);
#else
			op.WordBreakFlag = SetWordBreak(hWnd);
#endif
			SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
			if (op.ViewWindowCursor == 0) {
				HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
			}
			break;

#ifndef _WIN32_WCE
		case ID_MENUITEM_FONT:
			// フォント
			if (font_select(hWnd, &op.view_font) == TRUE) {
				if (hViewFont != NULL) {
					DeleteObject(hViewFont);
				}
				hViewFont = font_create(hWnd, &op.view_font);
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
				font_charset = op.view_font.charset;
				// Editウィンドウのフォントを設定
				EnumWindows((WNDENUMPROC)enum_windows_proc, 0);
				tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
				if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == -1) {
					ModifyWindow(hWnd, tpMailItem, FALSE, FALSE);
				}
			}
			break;
#endif

		case ID_MENUITEM_SHOW_DATE:
			op.ViewShowDate = (op.ViewShowDate == 1) ? 0 : 1;
			SetHeaderSize(hWnd);
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) != -1) {
				SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
			}
			break;

		case ID_VIEW_SOURCE:
		case ID_VIEW_PART:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == -1) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			ModifyWindow(hWnd, tpMailItem,
				((GET_WM_COMMAND_ID(wParam, lParam) == ID_VIEW_SOURCE) ? TRUE : FALSE), FALSE);
			break;

		case ID_VIEW_SAVE_ATTACH:
			// GJC save all attachments
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == -1) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			ret = TRUE;
			for (i = 0; i < MultiPartCnt && ret == TRUE; i++) {
				if (i == MultiPartTextIndex) {
					continue;
				}
				ret = Decode(hWnd, i, DECODE_SAVE_ALL);
			}
			if (ret == FALSE) {
				if (tpMailItem->Download) {
					ErrorMessage(hWnd, STR_ERR_SAVE);
				} else {
					ErrorMessage(hWnd, STR_ERR_PARTIAL_ATTACH);
				}
			}
			break;

		case ID_VIEW_DELETE_ATTACH:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) < 0) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (tpMailItem->ReFwd & REFWD_FWDHOLD) {
				MessageBox(hWnd, STR_MSG_ATT_HELD, STR_TITLE_DELETE, MB_OK);
				break;
			}
			if (MessageBox(hWnd, STR_Q_DELATTACH, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
				break;
			}
			DeleteAttachFile(hWnd, tpMailItem);
			ModifyWindow(hWnd, tpMailItem, FALSE, TRUE);
			break;

		case ID_RETURN_TO_MASTER:
			if (DigestMaster != NULL) {
				SetWindowLong(hWnd, GWL_USERDATA, (long)DigestMaster);
				ModifyWindow(hWnd, DigestMaster, FALSE, FALSE);
			}
			break;

		case ID_MENUITEM_VIEW:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == -1) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (AppViewMail(tpMailItem, vSelBox) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_VIEW);
				break;
			}
			if (op.ViewAppClose == 1) {
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

		default:
			if (command_id >= ID_ATTACH && command_id <= ID_ATTACH + 100) {
				BOOL DoWhat = (key >=0) ? DECODE_ASK : DECODE_AUTO_OPEN;
				if (item_is_mailbox(MailBox + vSelBox,
					(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == -1) {
					ErrorMessage(hWnd, STR_ERR_NOMAIL);
					break;
				}
				if (Decode(hWnd, command_id - ID_ATTACH, DoWhat) == FALSE) {
					ErrorMessage(hWnd, STR_ERR_SAVE);
				}
				break;
			}

			if (command_id == ID_MENUITEM_MOVESBOX || command_id == ID_MENUITEM_COPYSBOX) {
				int cnt = 0, Target = -1;
				if (command_id == ID_MENUITEM_MOVESBOX) {
					del_it = TRUE;
				}
				for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
					if ((MailBox + i)->Type == MAILBOX_TYPE_SAVE) {
						Target = i;
						cnt++;
					}
				}
				if (cnt == 0) {
					MessageBox(hWnd, STR_ERR_NOSAVEBOXES, WINDOW_TITLE, MB_OK);
					Target = -1;
					break;
				} else if (cnt > 1) {
					Target = del_it + VSELBOX_FLAG;
					ask = FALSE;
					if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SELSAVEBOX), hWnd, SelSaveBoxProc, (LPARAM)&Target) == FALSE) {
						Target = -1;
						break;
					}
				}
				if (Target == 0) {
					command_id = (del_it == TRUE) ? ID_MENUITEM_MOVE2NEW : ID_MENUITEM_COPY2NEW;
				} else {
					command_id = (del_it == TRUE) ? (ID_MENUITEM_MOVE2MBOX + Target) :
						(ID_MENUITEM_COPY2MBOX + Target);
				}
			}

			if (command_id == ID_MENUITEM_COPY2NEW || command_id == ID_MENUITEM_MOVE2NEW) {
				// GJC - copy/move to new SaveBox
				int old_selbox, newbox;
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
				int mbox = command_id - ID_MENUITEM_COPY2MBOX;
				if (command_id >= ID_MENUITEM_MOVE2MBOX) {
					mbox = command_id - ID_MENUITEM_MOVE2MBOX;
					mark_del = TRUE;
				}
				if (mbox >= 0 && mbox < MailBoxCnt && (MailBox+mbox) != NULL) {
					TCHAR fname[BUF_SIZE];
					tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
					tpNextMail = NULL;
					if (tpMailItem == NULL || (mbox == vSelBox && tpMailItem != AttachMailItem)) {
						break;
					}
					if (mbox == MAILBOX_SEND) {
						lstrcpy(fname, SENDBOX_FILE);
					} else if ((MailBox + mbox)->Filename == NULL) {
						wsprintf(fname, TEXT("MailBox%d.dat"), mbox - MAILBOX_USER);
					} else {
						lstrcpy(fname, (MailBox + mbox)->Filename);
					}
					if (ItemToSaveBox(hWnd, tpMailItem, mbox, fname, ask, mark_del) == TRUE) {
						if (mark_del == TRUE) {
							// delete from list or mark for deletion
							if ((MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
								tpNextMail = ViewDeleteItem(hWnd, tpMailItem);
								if (op.ViewNextAfterDel == 0) {
									tpNextMail = NULL;
								} else if (op.ViewNextAfterDel == 2 && (tpNextMail == NULL || tpMailItem->MailStatus != ICON_MAIL)) {
									tpNextMail = View_NextUnreadMail(hWnd);
								}
								if (tpNextMail != NULL) {
									SetWindowLong(hWnd, GWL_USERDATA, (long)tpNextMail);
									ModifyWindow(hWnd, tpNextMail, FALSE, FALSE);
								}
							} else {
								tpMailItem->ReFwd &= ~(REFWD_FWDHOLD);
								SetMark(hWnd, tpMailItem, ICON_DEL);
								GetMarkStatus(hWnd, tpMailItem);
								if (op.ViewNextAfterDel == 1) {
									tpNextMail = View_NextMail(hWnd);
								} else if (op.ViewNextAfterDel == 2) {
									tpNextMail = View_NextUnreadMail(hWnd);
								}
							}
						}
						if (op.AutoSave == 1 && (MailBox+mbox)->Loaded == TRUE) {
							file_save_mailbox(fname, DataDir, mbox, FALSE, TRUE, 2);
						}
					} else {
						ErrorMessage(hWnd, STR_ERR_SAVECOPY);
					}
					if (SelBox == mbox) {
						SwitchCursor(FALSE);
						ListView_ShowItem(GetDlgItem(MainWnd, IDC_LISTVIEW), (MailBox + SelBox), FALSE);
						SwitchCursor(TRUE);
						SetItemCntStatusText(NULL, FALSE);
					}
					if (mark_del == TRUE && tpNextMail == NULL && op.ViewCloseNoNext == 1) {
						SendMessage(hWnd, WM_CLOSE, 0, 0);
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

/*
 * View_InitApplication - ウィンドウクラスの登録
 */
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

/*
 * View_InitInstance - ウィンドウの作成
 */
BOOL View_InitInstance(HINSTANCE hInstance, LPVOID lpParam, BOOL NoAppFlag)
{
#ifdef _WIN32_WCE_PPC
	SIPINFO si;
	int x = CW_USEDEFAULT, y = CW_USEDEFAULT, cx, cy;
#endif
	int key;

	key = GetKeyState(VK_SHIFT);
	if (NoAppFlag == FALSE && ((op.DefViewApp == 1 && key >= 0) || (op.DefViewApp == 0 && key < 0))) {
		// ビューアで表示
		if (AppViewMail((MAILITEM *)lpParam, SelBox) == FALSE) {
			ErrorMessage(MainWnd, STR_ERR_VIEW);
			return FALSE;
		}
		if (((MAILITEM *)lpParam)->MailStatus != ICON_READ) {
			(MailBox + SelBox)->NeedsSave |= MARKS_CHANGED;
		}

		// 開封済みにする
		if (((MAILITEM *)lpParam)->MailStatus != ICON_NON && ((MAILITEM *)lpParam)->MailStatus < ICON_SENTMAIL) {
			((MAILITEM *)lpParam)->MailStatus = ICON_READ;
			((MAILITEM *)lpParam)->New = FALSE;
		}

		// 一覧のアイコンの設定
		if (((MAILITEM *)lpParam)->Mark != ICON_DOWN && ((MAILITEM *)lpParam)->Mark != ICON_DEL && ((MAILITEM *)lpParam)->Mark != ICON_FLAG) {
			HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
			int LvFocus = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);
			((MAILITEM *)lpParam)->Mark = ((MAILITEM *)lpParam)->MailStatus;
			ListView_RedrawItems(hListView, LvFocus, LvFocus);
			UpdateWindow(hListView);
		}
		return FALSE;
	}

	if (hViewWnd != NULL) {
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
		op.ViewRect.left,
		op.ViewRect.top,
		op.ViewRect.right - op.ViewRect.left,
		op.ViewRect.bottom - op.ViewRect.top,
		NULL, NULL, hInstance, lpParam);
#endif
	if (hViewWnd == NULL) {
		return FALSE;
	}

	mailbox_menu_rebuild(hViewWnd, FALSE);
	ShowWindow(hViewWnd, SW_SHOW);
	UpdateWindow(hViewWnd);
	return TRUE;
}
/* End of source */