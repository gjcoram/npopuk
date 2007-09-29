/*
 * nPOP
 *
 * View.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2007 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopsupport.org.uk
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

#define SAVE_HEADER					TEXT("From: %f\r\nTo: %t\r\nCc: %c\r\nSubject: %s\r\nDate: %D\r\nMessage-ID: %i\r\n\r\n")

/* Global Variables */
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

static MULTIPART **tpMultiPart;
static int MultiPartCnt, MultiPartTextIndex;

TCHAR *FindStr = NULL;
static DWORD FindPos;

static BOOL ESCFlag = FALSE;
static BOOL UnicodeEdit = 0;

int AttachProcess;

// �O���Q��
extern OPTION op;

extern HINSTANCE hInst;  // Local copy of hInstance
extern HWND MainWnd;
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
extern TCHAR *AppDir;

/* Local Function Prototypes */
static void SetWindowString(HWND hWnd, TCHAR *MailBoxName, TCHAR *MailBoxName2, int No);
static void SetHeaderString(HWND hHeader, MAILITEM *tpMailItem);
static void SetHeaderSize(HWND hWnd);
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam);
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam);
#endif
static BOOL FindEditString(HWND hEdit, TCHAR *strFind, int CaseFlag);
static LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);
static void SetEditSubClass(HWND hWnd);
static void DelEditSubClass(HWND hWnd);
static BOOL InitWindow(HWND hWnd, MAILITEM *tpMailItem);
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void EndWindow(HWND hWnd);
static void SetEditMenu(HWND hWnd);
static void ModifyWindow(HWND hWnd, MAILITEM *tpMailItem, BOOL ViewSrc);
static MAILITEM *View_NextMail(HWND hWnd, BOOL St);
static void View_PrevMail(HWND hWnd);
static void View_NextUnreadMail(HWND hWnd);
static void View_NextScroll(HWND hEditWnd);
static BOOL ShellOpen(TCHAR *FileName);
static void OpenURL(HWND hWnd);
static void SetReMessage(HWND hWnd, int ReplyFlag);
static BOOL Decode(HWND hWnd, int id, BOOL SaveAll);
static BOOL SaveViewMail(TCHAR *fname, HWND hWnd, int MailBoxIndex, MAILITEM *tpMailItem, TCHAR *head);
static BOOL AppViewMail(MAILITEM *tpMailItem);
static void SetMark(HWND hWnd, MAILITEM *tpMailItem, const int mark);
static void GetMarkStatus(HWND hWnd, MAILITEM *tpMailItem);
static LRESULT CALLBACK ViewProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);

/*
 * SetWindowString - �E�B���h�E�^�C�g���̐ݒ�
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
 * SetHeaderString - ���[���w�b�_�\��
 */
static void SetHeaderString(HWND hHeader, MAILITEM *tpMailItem)
{
	TCHAR *MyMailAddress = NULL;
	TCHAR *buf, *p;
	int len = 0;
	int i;
	BOOL ToFlag = FALSE;

	// �����̃��[���A�h���X�̎擾
	if ((MailBox + SelBox)->Type == MAILBOX_TYPE_SAVE) {
		i = mailbox_name_to_index(tpMailItem->MailBox);
		if (i != -1 && (MailBox + i)->Type != MAILBOX_TYPE_SAVE) {
			MyMailAddress = (MailBox + i)->MailAddress;
		}
	} else {
		MyMailAddress = (MailBox + SelBox)->MailAddress;
	}

	// To ��\������ׂ����`�F�b�N
	if (tpMailItem->To != NULL) {
		buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(tpMailItem->To) + 1));
		if (buf != NULL) {
			p = GetMailAddress(tpMailItem->To, buf, FALSE);
			if (*buf != TEXT('\0') &&
				(*p != TEXT('\0') || MyMailAddress == NULL || lstrcmpi(MyMailAddress, buf) != 0)) {
				ToFlag = TRUE;
			}
			mem_free(&buf);
		}
	}

	// �쐬���镶����̃T�C�Y���v�Z
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

	// �\�����镶������쐬����
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
 * SetHeaderSize - ���[���w�b�_�̃T�C�Y�ݒ�
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
	hMenu = SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT);
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
	// �t�H���g�̍������擾
	GetTextMetrics(hdc, &lptm);
	if (hFont != NULL) {
		SelectObject(hdc, hFont);
	}
	ReleaseDC(hHeader, hdc);
	HLine = (op.ViewShowDate == 1) ? 3 : 2;
	FontHeight = (lptm.tmHeight + lptm.tmExternalLeading) * HLine;

	// �ꎞ�I�ɐݒ肵�ăT�C�Y���Čv�Z����
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
 * TbNotifyProc - �c�[���o�[�̒ʒm���b�Z�[�W (Win32)
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
 * NotifyProc - �R���g���[���̒ʒm���b�Z�[�W
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
 * FindEditString - EDIT���̕��������������
 */
static BOOL FindEditString(HWND hEdit, TCHAR *strFind, int CaseFlag)
{
	DWORD dwStart;
	DWORD dwEnd;
	TCHAR *buf = NULL;
	TCHAR *p;

#ifdef UNICODE
	// �����ʒu�̎擾
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	// ���݈ʒu���O�񌟍��ʒu�ƈႤ�ꍇ�͌��݈ʒu�������ʒu�ɂ���
	if ((dwStart + 1U) != FindPos) {
		FindPos = dwStart;
	}

	// �G�f�B�b�g���當������擾����
	AllocGetText(hEdit, &buf);
	p = str_find(strFind, buf + FindPos, CaseFlag);

	// ���������񂪌�����Ȃ������ꍇ
	if (*p == TEXT('\0')) {
		mem_free(&buf);
		return FALSE;
	}

	// �����񂪌��������ꍇ�͂��̈ʒu��I����Ԃɂ���
	FindPos = p - buf;
	SendMessage(hEdit, EM_SETSEL, FindPos, FindPos + lstrlen(strFind));
	SendMessage(hEdit, EM_SCROLLCARET, 0, 0);
	mem_free(&buf);
	FindPos++;
	return TRUE;
#else
	// �G�f�B�b�g���當������擾����
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
		// �����ʒu�̎擾
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		st = WideCharToMultiByte(CP_ACP, 0, wbuf, dwStart, NULL, 0, NULL, NULL);
		mem_free(&wbuf);
		// ���݈ʒu���O�񌟍��ʒu�ƈႤ�ꍇ�͌��݈ʒu�������ʒu�ɂ���
		if ((st + 1U) != FindPos) {
			FindPos = st;
		}
		p = str_find(strFind, buf + FindPos, CaseFlag);
		// ���������񂪌�����Ȃ������ꍇ
		if (*p == TEXT('\0')) {
			mem_free(&buf);
			return FALSE;
		}
		st = MultiByteToWideChar(CP_ACP, 0, buf, p - buf, NULL, 0);
		len = MultiByteToWideChar(CP_ACP, 0, strFind, -1, NULL, 0) - 1;
		// �����񂪌��������ꍇ�͂��̈ʒu��I����Ԃɂ���
		SendMessage(hEdit, EM_SETSEL, st, st + len);
		FindPos = p - buf;
	} else {
		// �����ʒu�̎擾
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		// ���݈ʒu���O�񌟍��ʒu�ƈႤ�ꍇ�͌��݈ʒu�������ʒu�ɂ���
		if ((dwStart + 1U) != FindPos) {
			FindPos = dwStart;
		}
		p = str_find(strFind, buf + FindPos, CaseFlag);
		// ���������񂪌�����Ȃ������ꍇ
		if (*p == TEXT('\0')) {
			mem_free(&buf);
			return FALSE;
		}
		// �����񂪌��������ꍇ�͂��̈ʒu��I����Ԃɂ���
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
 * SetWordBreakMenu - �܂�Ԃ��̃��j���[�̃`�F�b�N�؂�ւ�
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
 * SetWordBreak - �܂�Ԃ��̐؂�ւ�
 */
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
	if (i & WS_HSCROLL) {
		i ^= WS_HSCROLL;
#ifdef _WIN32_WCE_PPC
		SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), MF_CHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
		SetWordBreakMenu(hWnd, hMenu, MF_CHECKED);
#else
		SetWordBreakMenu(hWnd, NULL, MF_CHECKED);
#endif
		ret = 1;
	} else {
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
 * SubClassEditProc - �T�u�N���X�������E�B���h�E�v���V�[�W��
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

			if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
				SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENU, 0);
				return 0;
			}
		}
#else
		if (GetKeyState(VK_MENU) < 0) {
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

/*
 * SetEditSubClass - �E�B���h�E�̃T�u�N���X��
 */
static void SetEditSubClass(HWND hWnd)
{
	SendMessage(hWnd, EM_LIMITTEXT, 1, 0);
	EditWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassEditProc);
}

/*
 * DelEditSubClass - �E�B���h�E�N���X��W���̂��̂ɖ߂�
 */
static void DelEditSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)EditWindowProcedure);
	EditWindowProcedure = NULL;
}

/*
 * InitWindow - �E�B���h�E�̏�����
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
	HWND hToolBar;
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
		{6,	ID_MENUITEM_DOWNMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{7,	ID_MENUITEM_DELMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{8,	ID_MENUITEM_UNREADMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
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
		STR_CMDBAR_DOWNMARK,
		STR_CMDBAR_DELMARK,
		STR_CMDBAR_UNREADMARK
	};
#ifdef _WIN32_WCE_PPC
	SHMENUBARINFO mbi;
#endif	// _WIN32_WCE_PPC
#endif	// _WIN32_WCE
#endif	// _WIN32_WCE_LAGENDA

	if (tpMailItem == NULL || tpMailItem->Body == NULL) {
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
	hToolBar = mbi.hwndMB;

	if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
		CommandBar_AddToolTips(hToolBar, 11, szTips);
		CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR_VIEW, 9, TB_ICONSIZE, TB_ICONSIZE);
		CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	} else {
		CommandBar_AddToolTips(hToolBar, 8, szTips);
		CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR_VIEW, 6, TB_ICONSIZE, TB_ICONSIZE);
		CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON) - 3, tbButton);
	}
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
	hToolBar = CommandBar_Create(hInst, hWnd, IDC_VCB);
    CommandBar_AddToolTips(hToolBar, 9, szTips);
	if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_VIEW_HPC, 0);
	} else {
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_VIEW, 0);
	}
	CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR_VIEW, 9, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	CommandBar_AddAdornments(hToolBar, 0, 0);
#endif
#else
	// Win32
	hToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_VTB, 9, hInst, IDB_TOOLBAR_VIEW,
		tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, TB_ICONSIZE, TB_ICONSIZE, sizeof(TBBUTTON));
	SetWindowLong(hToolBar, GWL_STYLE, GetWindowLong(hToolBar, GWL_STYLE) | TBSTYLE_FLAT);
	SendMessage(hToolBar, TB_SETINDENT, 5, 0);
	ShowWindow(hToolBar, SW_SHOW);
#endif

	// �w�b�_��\������STATIC�R���g���[���̍쐬
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
	// �t�H���g�̐ݒ�
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
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, (WPARAM)0, (LPARAM)TEXT("��"));
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
		UnicodeEdit = (j != 2) ? 2 : 1;
	}
#endif

	SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE_PPC
	SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), (op.WordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
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
	// IME���I�t�ɂ���
	ImmAssociateContext(GetDlgItem(hWnd, IDC_EDIT_BODY), (HIMC)NULL);
#endif

	SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
	SetTimer(hWnd, ID_HIDECARET_TIMER, 10, NULL);
	return TRUE;
}

/*
 * SetWindowSize - �E�B���h�E�̃T�C�Y�ύX
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
 * EndWindow - �E�B���h�E�̏I������
 */
static void EndWindow(HWND hWnd)
{
	multipart_free(&tpMultiPart, MultiPartCnt);
	tpMultiPart = NULL;
	MultiPartCnt = 0;
	MultiPartTextIndex = 0;

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

/*
 * SetEditMenu - �ҏW���j���[�̊����^�񊈐��̐؂�ւ�
 */
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
	// �G�f�B�b�g�{�b�N�X�̑I���ʒu�̎擾
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
	EnableMenuItem(hMenu, ID_MENUITEM_COPY, (i < j) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_MENUITEM_FIND, (SelBox != MAILBOX_SEND) ? MF_ENABLED : MF_GRAYED);
}

/*
 * SetAttachMenu - �\������part�̑I���ƓY�t���j���[�̐ݒ�
 */
static int SetAttachMenu(HWND hWnd, MAILITEM *tpMailItem, BOOL ViewSrc)
{
	HMENU hMenu;
	TCHAR *str;
	TCHAR *p, *r;
	int i, ret = -1, cnt = 0;
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
	// ���j���[������������
	while (DeleteMenu(hMenu, MENU_ATTACH_POS, MF_BYPOSITION) == TRUE);

	if (MultiPartCnt == 1 &&
		(*tpMultiPart)->sPos == tpMailItem->Body && (*tpMultiPart)->ePos == NULL &&
		(*tpMultiPart)->ContentType != NULL &&
		str_cmp_ni((*tpMultiPart)->ContentType, "text", tstrlen("text")) == 0) {

		if (tpMailItem->HasHeader == TRUE || (op.StripHtmlTags == 1 &&
			str_cmp_ni((*tpMultiPart)->ContentType, "text/html", tstrlen("text/html")) == 0)) {
			AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
		}
		return 0;
	}
	if (MultiPartCnt == 1 && ViewSrc == TRUE) {
		AppendMenu(hMenu, MF_STRING, ID_VIEW_PART, STR_VIEW_MENU_ATTACH);
		return 0;
	}

	for (i = 0; i < MultiPartCnt; i++) {
		if (ret == -1 && ((*(tpMultiPart + i))->ContentType == NULL ||
			str_cmp_ni((*(tpMultiPart + i))->ContentType, "text", tstrlen("text")) == 0)) {
			// ��Ԗڂɏo�������e�L�X�g�f�[�^�͖{���ɂ���
			ret = i;
			if (MultiPartCnt == 1 && tpMailItem->Multipart != MULTIPART_NONE) {
				AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
			}
		} else {
			if (AppendFlag == FALSE) {
				if (MultiPartCnt > 1) {
					AppendMenu(hMenu, MF_STRING, ID_VIEW_SOURCE, STR_VIEW_MENU_SOURCE);
					AppendMenu(hMenu, MF_STRING, ID_VIEW_DELETE_ATTACH, STR_VIEW_MENU_DELATTACH);
				}
				// ��؂�̒ǉ�
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendFlag = TRUE;
			}
			// �r���Ő؂�Ă���f�[�^�̏ꍇ�̓��j���[��񊈐��ɂ���
			mFlag = (i != 0 && (*(tpMultiPart + i))->ePos == NULL) ? MF_GRAYED : MF_ENABLED;

			// �t�@�C���������j���[�ɒǉ�����
			str = alloc_char_to_tchar((*(tpMultiPart + i))->ContentType);
			if (str != NULL) {
				for (r = str; *r != TEXT('\0') && *r != TEXT(';'); r++);
				*r = TEXT('\0');
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
			AppendMenu(hMenu, MF_STRING | mFlag, ID_ATTACH + i, ((p != NULL && *p != TEXT('\0')) ? p : TEXT("Attach")));
			if (mFlag == MF_ENABLED) {
				cnt++;
			}
			mem_free(&str);
		}
	}
	if (cnt > 0) {
		InsertMenu(hMenu, ID_VIEW_DELETE_ATTACH, MF_BYCOMMAND | MF_STRING, ID_VIEW_SAVE_ATTACH, STR_VIEW_MENU_SAVEATTACH);
	}
	return ret;
}

/*
 * ModifyWindow - ���e�̕ύX
 */
static void ModifyWindow(HWND hWnd, MAILITEM *tpMailItem, BOOL ViewSrc)
{
	TCHAR *buf, *mbuf, *p;
	int LvFocus, TextIndex = -1;
	BOOL redraw = FALSE;
#ifndef _WIN32_WCE
	OSVERSIONINFO os_info;
#endif
	HWND hListView;

	if (tpMailItem == NULL || tpMailItem->Body == NULL) {
		return;
	}

	SwitchCursor(FALSE);

	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	vSelBox = SelBox;
	LvFocus = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED);

	// �J���ς݂ɂ���
	if (tpMailItem->MailStatus != ICON_NON && tpMailItem->MailStatus < ICON_SENTMAIL) {
		tpMailItem->MailStatus = ICON_READ;
		tpMailItem->New = FALSE;
		redraw = TRUE;
	}

	// �ꗗ�̃A�C�R���̐ݒ�
	if (tpMailItem->Mark != ICON_DOWN && tpMailItem->Mark != ICON_DEL) {
		tpMailItem->Mark = tpMailItem->MailStatus;
		redraw = TRUE;
	}
	if (redraw == TRUE) {
		ListView_SetItemState(hListView, LvFocus, INDEXTOOVERLAYMASK(tpMailItem->ReFwd), LVIS_OVERLAYMASK);
		ListView_RedrawItems(hListView, LvFocus, LvFocus);
		UpdateWindow(hListView);
	}

	// �E�B���h�E�^�C�g���̐ݒ�
	SetWindowString(hWnd, (MailBox + SelBox)->Name , tpMailItem->MailBox, tpMailItem->No);
	// �w�b�_�̐ݒ�
	SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
	UpdateWindow(GetDlgItem(hWnd, IDC_HEADER));

	SetItemCntStatusText(MainWnd, NULL, FALSE);

	// �}�[�N��Ԏ擾
	GetMarkStatus(hWnd, tpMailItem);

	multipart_free(&tpMultiPart, MultiPartCnt);
	tpMultiPart = NULL;
	MultiPartCnt = 0;
	// �}���`�p�[�g�̓W�J
	buf = MIME_body_decode(tpMailItem, ViewSrc, &tpMultiPart, &MultiPartCnt, &TextIndex);
	mbuf = buf; // may advance buf to skip headers, need to keep mbuf for mem_free

	// GJC don't show headers unless ViewSrc
	if (ViewSrc == FALSE && tpMailItem->HasHeader == TRUE  && tpMailItem->Multipart == MULTIPART_NONE) {
		if ((p = GetBodyPointaT(buf)) != NULL) {
			buf = p;
		}
	}

	// GJC remove HTML tags
	if (ViewSrc == FALSE && op.StripHtmlTags == 1 &&
		((tpMailItem->ContentType != NULL && str_cmp_ni_t(tpMailItem->ContentType, TEXT("text/html"), lstrlen(TEXT("text/html")))==0)
		|| (TextIndex != -1 && (tpMultiPart[TextIndex])->ContentType != NULL &&
		str_cmp_ni((tpMultiPart[TextIndex])->ContentType, "text/html", tstrlen("text/html")) == 0))) {
		p = strip_html_tags(buf, TRUE);
		if (p != NULL) {
			mem_free(&mbuf);
			mbuf = buf = p;
		}
	}

	// �\������part�̑I���ƓY�t���j���[�̐ݒ�
	MultiPartTextIndex = SetAttachMenu(hWnd, tpMailItem, ViewSrc);

	// �����ʒu�̏�����
	FindPos = 0;

	// �{���̕\��
	if (buf != NULL) {
#ifdef _WIN32_WCE
		if (lstrlen(buf) > EDITMAXSIZE) {
			*(buf + EDITMAXSIZE) = TEXT('\0');
		}
#else
		// OS���̎擾
		os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&os_info);

		if (os_info.dwPlatformId != VER_PLATFORM_WIN32_NT && lstrlen(buf) > EDITMAXSIZE) {
			*(buf + EDITMAXSIZE) = TEXT('\0');
		}
#endif
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)buf);
		mem_free(&mbuf);
	} else {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)TEXT(""));
	}

	ShowWindow(hWnd, SW_SHOW);
#ifndef _WIN32_WCE
	if (IsIconic(hWnd) != 0) {
		ShowWindow(hWnd, SW_RESTORE);
	}
#endif
	HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
	SwitchCursor(TRUE);
}

/*
 * View_NextMail - ���̃��[����\��
 */
static MAILITEM *View_NextMail(HWND hWnd, BOOL St)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int j;

	if (SelBox == MAILBOX_SEND) {
		return NULL;
	}
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	if (St == FALSE) {
		Index = ListView_GetMemToItem(hListView,
			(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));
	} else {
		Index = -1;
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
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModifyWindow(hWnd, tpMailItem, FALSE);
	return tpMailItem;
}

/*
 * View_PrevMail - �O�̃��[����\��
 */
static void View_PrevMail(HWND hWnd)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int j;

	if (SelBox == MAILBOX_SEND) {
		return;
	}
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	Index = ListView_GetMemToItem(hListView,
		(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));

	j = ListView_GetPrevMailItem(hListView, Index);
	if (j == -1) {
		return;
	}
	ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
	ListView_SetItemState(hListView,
		j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_EnsureVisible(hListView, j, TRUE);

	tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, j);
	SetWindowLong(hWnd, GWL_USERDATA, (long)tpMailItem);
	ModifyWindow(hWnd, tpMailItem, FALSE);
}

/*
 * View_NextUnreadMail - ���̖��J�����[����\��
 */
static void View_NextUnreadMail(HWND hWnd)
{
	MAILITEM *tpMailItem;
	HWND hListView;
	int Index;
	int i, j;

	if (SelBox == MAILBOX_SEND) {
		return;
	}
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
	Index = ListView_GetMemToItem(hListView,
		(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA));

	// ���J�����[���̃C���f�b�N�X���擾
	j = ListView_GetNextUnreadItem(hListView, Index,
		ListView_GetItemCount(hListView));
	if (j == -1) {
		j = ListView_GetNextUnreadItem(hListView, -1, Index);
	}
	if (j == -1) {
		if (op.MoveAllMailBox == 0) {
			return;
		}

		// ���̖��J�������郁�[���{�b�N�X�̃C���f�b�N�X���擾
		SwitchCursor(FALSE);
		i = mailbox_next_unread(SelBox + 1, MailBoxCnt);
		if (i == -1) {
			i = mailbox_next_unread(MAILBOX_USER, SelBox);
		}
		if (i == -1) {
			SwitchCursor(TRUE);
			return;
		}
		// ���[���{�b�N�X�̑I��
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
	ModifyWindow(hWnd, tpMailItem, FALSE);
	SendMessage(MainWnd, WM_INITTRAYICON, 0, 0);
}

/*
 * View_NextScroll - �\�����[����PageDown���Ĉ�ԉ��ɗ����玟�̖��J�����[���Ɉړ�����
 */
static void View_NextScroll(HWND hEditWnd)
{
	TEXTMETRIC lptm;
	SCROLLINFO ScrollInfo;
	RECT rRect;
	HDC hdc;
	HFONT hFont;
	BOOL Next = FALSE;

	// �t�H���g�̍������擾
	hdc = GetDC(hEditWnd);
	if (hViewFont != NULL) {
		hFont = SelectObject(hdc, hViewFont);
	}
	GetTextMetrics(hdc, &lptm);
	if (hViewFont != NULL) {
		SelectObject(hdc, hFont);
	}
	ReleaseDC(hEditWnd, hdc);

	// �X�N���[�����̎擾
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_POS | SIF_RANGE;
	GetScrollInfo(hEditWnd, SB_VERT, &ScrollInfo);

	GetClientRect(hEditWnd,(LPRECT)&rRect);
	ScrollInfo.nMax -= ((rRect.bottom - rRect.top) / (lptm.tmHeight + lptm.tmExternalLeading));
	if (ScrollInfo.nPos >= ScrollInfo.nMax) {
		Next = TRUE;
	}
	// PageDown
	SendMessage(hEditWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
	if (Next == TRUE) {
		// ���̖��J�����[���ֈړ�����
		View_NextUnreadMail(GetParent(hEditWnd));
	}
}

/*
 * View_FindMail - ���[�������當���������
 */
void View_FindMail(HWND hWnd, BOOL FindSet)
{
	MAILITEM *tpMailItem;
	HWND hEdit;
	TCHAR *buf = NULL;
	TCHAR *p;
	DWORD dwStart;
	DWORD dwEnd;

	if (SelBox == MAILBOX_SEND) {
		return;
	}

	hEdit = GetDlgItem(hWnd, IDC_EDIT_BODY);
	if (FindSet == TRUE || FindStr == NULL) {
#ifdef UNICODE
		// �I�𕶎�����擾
		SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
		if (dwStart != dwEnd) {
			AllocGetText(hEdit, &buf);
			if (buf != NULL) {
				mem_free(&FindStr);
				FindStr = (TCHAR *)mem_alloc(sizeof(TCHAR) * (dwEnd - dwStart + 1));
				if (FindStr != NULL) {
					str_cpy_n_t(FindStr, buf + dwStart, dwEnd - dwStart + 1);
				}
				mem_free(&buf);
			}
		}
#else
		if (UnicodeEdit == 2) {
			WCHAR *wbuf;
			int len;
			// �I�𕶎�����擾
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
					}
					mem_free(&buf);
					mem_free(&wbuf);
				}
			}
		} else {
			// �I�𕶎�����擾
			SendMessage(hEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
			if (dwStart != dwEnd) {
				AllocGetText(hEdit, &buf);
				if (buf != NULL) {
					mem_free(&FindStr);
					FindStr = (TCHAR *)mem_alloc(sizeof(TCHAR) * (dwEnd - dwStart + 1));
					if (FindStr != NULL) {
						str_cpy_n_t(FindStr, buf + dwStart, dwEnd - dwStart + 1);
					}
					mem_free(&buf);
				}
			}
		}
#endif

		// ���������̐ݒ�
		if (DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_FIND), hWnd, SetFindProc) == FALSE) {
			return;
		}
	}

	while (1) {
		// �{�����猟�����Č��������ʒu��I����Ԃɂ���
		if (FindEditString(hEdit, FindStr, op.MatchCase) == TRUE) {
			break;
		}

		// �P���[�����ł̌����̏ꍇ
		if (op.AllFind == 0) {
			SendMessage(hEdit, EM_SETSEL, 0, 0);
			if (FindEditString(hEdit, FindStr, op.MatchCase) == TRUE) {
				break;
			}
			buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FindStr) + lstrlen(STR_MSG_NOFIND) + 1));
			if (buf == NULL) {
				break;
			}
			wsprintf(buf, STR_MSG_NOFIND, FindStr);
			MessageBox(hWnd, buf, STR_TITLE_FIND, MB_ICONINFORMATION);
			mem_free(&buf);
			break;
		}

		// ���̃��[���Ɉړ�
		tpMailItem = View_NextMail(hWnd, FALSE);
		if (tpMailItem == NULL) {
			buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FindStr) + lstrlen(STR_TITLE_ALLFIND) + 1));
			if (buf == NULL) {
				break;
			}
			wsprintf(buf, STR_TITLE_ALLFIND, FindStr);
			if (MessageBox(hWnd, STR_Q_NEXTFIND, buf, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
				mem_free(&buf);
				break;
			}
			mem_free(&buf);
			tpMailItem = View_NextMail(hWnd, TRUE);
		}

		// ESC �������ꂽ�ꍇ�͌����I��
		if (GetForegroundWindow() == hWnd && GetAsyncKeyState(VK_ESCAPE) < 0) {
			ESCFlag = TRUE;
			break;
		}

		// Subject���猟��
		if (op.SubjectFind != 0 && tpMailItem->Subject != NULL) {
			p = str_find(FindStr, tpMailItem->Subject, op.MatchCase);
			if (*p != TEXT('\0')) {
				break;
			}
		}
	}
}

/*
 * ShellOpen - �t�@�C�����֘A�t���Ŏ��s
 */
static BOOL ShellOpen(TCHAR *FileName)
{
#ifndef _WIN32_WCE_LAGENDA
	SHELLEXECUTEINFO sei;

	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(sei);
	sei.fMask = 0;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	if (op.URLApp == NULL || *op.URLApp == TEXT('\0')) {
		sei.lpFile = FileName;
		sei.lpParameters = NULL;
	} else {
		sei.lpFile = op.URLApp;
		sei.lpParameters = FileName;
	}
	sei.lpDirectory = NULL;
	sei.nShow = SW_SHOWNORMAL;
	sei.hInstApp = hInst;
	return ShellExecuteEx(&sei);
#else
	TCHAR *file;
	TCHAR *param;

	if (op.URLApp == NULL || *op.URLApp == TEXT('\0')) {
		file = FileName;
		param = NULL;
	} else {
		file = op.URLApp;
		param = FileName;
	}
	return CoshExecute(NULL, file, param);
#endif
}

/*
 * OpenURL - �G�f�B�b�g�{�b�N�X����I�����ꂽURL�𒊏o���ĊJ��
 */
static void OpenURL(HWND hWnd)
{
	TCHAR *buf;
	TCHAR *str;
	TCHAR *p, *r, *s;
	int i, j;
	int len;
	int MailToFlag = 0;

	// �G�f�B�b�g�{�b�N�X�̑I���ʒu�̎擾
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
	if (i >= j) {
		return;
	}

	// �G�f�B�b�g�{�b�N�X�ɐݒ肳�ꂽ������̎擾
	len = SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXTLENGTH, 0, 0) + 1;
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (buf == NULL) {
		return;
	}
	*buf = TEXT('\0');
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXT, len, (LPARAM)buf);

	for (; *(buf + j) != TEXT('\0') && *(buf + j) != TEXT('\r') && *(buf + j) != TEXT('\n') && *(buf + j) != TEXT(' '); j++);
#ifdef UNICODE
	str = (TCHAR *)mem_alloc(sizeof(TCHAR) * (j - i + 2));
	if (str == NULL) {
		mem_free(&buf);
		return;
	}
	// �I�𕶎���̒��o
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
		// �I�𕶎���̒��o
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
		// �I�𕶎���̒��o
		for (p = buf + i, r = str; p < (buf + j); p++, r++) {
			*r = *p;
		}
		*r = TEXT('\0');
		mem_free(&buf);
	}
#endif

	// �J�n�ʒu�̎擾
	for (s = str; *s == TEXT('(') || *s == TEXT(')') || *s == TEXT('\"') ||
		*s == TEXT('<') || *s == TEXT('>') || *s == TEXT('\t') || *s == TEXT(' '); s++);
	// URL�̊J�n�ʒu���擾
	for (p = s; *p != TEXT('\0'); p++) {
		if (str_cmp_ni_t(p, URL_HTTP, lstrlen(URL_HTTP)) == 0 ||
			str_cmp_ni_t(p, URL_HTTPS, lstrlen(URL_HTTPS)) == 0 ||
			str_cmp_ni_t(p, URL_FTP, lstrlen(URL_FTP)) == 0 ||
			str_cmp_ni_t(p, URL_MAILTO, lstrlen(URL_MAILTO)) == 0) {
			s = p;
			break;
		}
	}
	// �I���ʒu�̎擾
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

	// URL�̃`�F�b�N
	if (str_cmp_ni_t(s, URL_HTTP, lstrlen(URL_HTTP)) == 0 ||
		str_cmp_ni_t(s, URL_HTTPS, lstrlen(URL_HTTPS)) == 0 ||
		str_cmp_ni_t(s, URL_FTP, lstrlen(URL_FTP)) == 0) {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, (WPARAM)i, (LPARAM)i);
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
 * SetReMessage - �ԐM�̐ݒ���s��
 */
static void SetReMessage(HWND hWnd, int ReplyFlag)
{
	HWND hEdit;
	TCHAR *buf = NULL, *seltext = NULL;
	DWORD selStart, selEnd;
	int ret;

	if (item_is_mailbox(MailBox + vSelBox,
		(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE) {
		ErrorMessage(hWnd, STR_ERR_NOMAIL);
		return;
	}

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

	ret = Edit_InitInstance(hInst, hWnd, vSelBox,
		(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA), ReplyFlag, seltext);
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
 * Decode - �Y�t�t�@�C�����f�R�[�h���ĕۑ�
 */
static BOOL Decode(HWND hWnd, int id, BOOL SaveAll)
{
	ATTACHINFO ai;
	TCHAR *fname, *ext = NULL;
	TCHAR buf[BUF_SIZE];
	TCHAR *p, *r;
	TCHAR *str;
	char *b64str, *dstr, *endpoint;
	int len;
	int EncodeFlag = 0;
	BOOL ret = TRUE;

	SwitchCursor(FALSE);

	if (SaveAll == TRUE && (*(tpMultiPart + id))->ePos == NULL) {
		return FALSE; // saved as much as we can
	}

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
		// �G���R�[�h����Ă���ꍇ�̓f�R�[�h���s��
		dstr = (char *)mem_alloc(tstrlen(b64str));
		if (dstr == NULL) {
			mem_free(&b64str);
			SwitchCursor(TRUE);
			return FALSE;
		}
		endpoint = ((EncodeFlag == 1) ? base64_decode : QuotedPrintable_decode)(b64str, dstr);
		mem_free(&b64str);
		break;

	default:
		dstr = b64str;
		endpoint = dstr + tstrlen(dstr);
		break;
	}

	str = alloc_char_to_tchar((*(tpMultiPart + id))->ContentType);
	if (str != NULL) {
		for (r = str; *r != TEXT('\0') && *r != TEXT(';'); r++);
		*r = TEXT('\0');
	}

	p = GetMIME2Extension(str, NULL);
	if (p != NULL) {
		ext = alloc_copy_t(p + 1);
	}

	if ((*(tpMultiPart + id))->Filename == NULL || *(*(tpMultiPart + id))->Filename == TEXT('\0')) {
		// �t�@�C�����������ꍇ
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
		mem_free(&p);
		fname = alloc_char_to_tchar((*(tpMultiPart + id))->Filename);
	}

	if (fname != NULL) {
		// �t�@�C�����Ɏg���Ȃ������� _ �ɕϊ�����
		filename_conv(fname);
		// �t�@�C��������������ꍇ�͓r���Ő؂�
		if (lstrlen(fname) > BUF_SIZE - 50) {
			*(fname + BUF_SIZE - 50) = TEXT('\0');
		}
	}
	SwitchCursor(TRUE);

	// �m�F�_�C�A���O
	AttachProcess = 0;
	if (SaveAll == FALSE) {
		ai.from = ((MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA))->From;
		ai.fname = fname;
		ai.mime = str;
		ai.size = endpoint - dstr;
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ATTACH_NOTICE), hWnd, AttachNoticeProc, (LPARAM)&ai) == FALSE) {
			AttachProcess = -1;
		}
	}
	switch (AttachProcess) {
	  case 0:
		// �ۑ�
		ret = file_save(hWnd, fname, ext, dstr, endpoint - dstr, SaveAll);
		break;

	  case 1:
		// �ۑ����Ď��s
//		if (op.AttachWarning == 0 ||
//			MessageBox(hWnd, STR_Q_ATTACH, STR_TITLE_ATTACH_MSG, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
			if (op.AttachDelete != 0) {
				wsprintf(buf, TEXT("%s%s"), ATTACH_FILE, fname);
				p = buf;
			} else {
				p = fname;
			}
			ret = file_save_exec(hWnd, p, dstr, endpoint - dstr);
//		}
		break;
	}
	mem_free(&str);
	mem_free(&ext);
	mem_free(&fname);
	mem_free(&dstr);
	return ret;
}

/*
 * DeleteAttachFile - �Y�t�t�@�C���̍폜
 */
static BOOL DeleteAttachFile(HWND hWnd, MAILITEM *tpMailItem)
{
	MULTIPART **tpPart = NULL;
	char *mBody;
#ifdef UNICODE
	char *ContentType;
#endif
	int cnt = 0;
	int TextIndex = -1;
	int i;

	// �}���`�p�[�g�̉��
#ifdef UNICODE
	ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
	cnt = multipart_parse(ContentType, tpMailItem->Body, &tpPart, 0);
	mem_free(&ContentType);
#else
	cnt = multipart_parse(tpMailItem->ContentType, tpMailItem->Body, &tpPart, 0);
#endif
	if (cnt == 0) {
		return FALSE;
	}

	// �e�L�X�g�̃p�[�g������
	for (i = 0; i < cnt; i++) {
		if ((*tpPart + i)->ContentType == NULL ||
			str_cmp_ni((*tpPart + i)->ContentType, "text", tstrlen("text")) == 0) {
			TextIndex = i;
			break;
		}
	}
	if (TextIndex == -1) {
		multipart_free(&tpPart, cnt);
		return FALSE;
	}
	if ((*tpPart + TextIndex)->ePos == NULL) {
		mBody = alloc_copy((*tpPart + TextIndex)->sPos);
		if (mBody == NULL) {
			multipart_free(&tpPart, cnt);
			return FALSE;
		}
	} else {
		i = (*tpPart + TextIndex)->ePos - (*tpPart + TextIndex)->sPos;
		mBody = (char *)mem_alloc(sizeof(char) * (i + 1));
		if (mBody == NULL) {
			multipart_free(&tpPart, cnt);
			return FALSE;
		}
		if (i == 0) {
			*mBody = '\0';
		} else {
			str_cpy_n(mBody, (*tpPart + TextIndex)->sPos, i - 1);
		}
	}

	// ���e�̒u������
	mem_free(&tpMailItem->Body);
	tpMailItem->Body = mBody;
	mem_free(&tpMailItem->Encoding);
	tpMailItem->Encoding = alloc_char_to_tchar((*tpPart + TextIndex)->Encoding);
	mem_free(&tpMailItem->ContentType);
	tpMailItem->ContentType = alloc_char_to_tchar((*tpPart + TextIndex)->ContentType);
	mem_free(&tpMailItem->Attach);
	tpMailItem->Attach = alloc_copy_t(TEXT("_"));

	multipart_free(&tpPart, cnt);
	return TRUE;
}

/*
 * SaveViewMail - ���[�����t�@�C���ɕۑ�
 */
static BOOL SaveViewMail(TCHAR *fname, HWND hWnd, int MailBoxIndex, MAILITEM *tpMailItem, TCHAR *head)
{
	MULTIPART **tpPart = NULL;
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR *buf;
	TCHAR *tmp;
	int len = 0, cnt = 0, idx;


	if (fname == NULL) {
		// �t�@�C���̍쐬
		if (tpMailItem->Subject != NULL) {
			str_cpy_n_t(path, tpMailItem->Subject, BUF_SIZE - 50);
			filename_conv(path);
			lstrcpy(path + lstrlen(path), TEXT(".txt"));
		} else {
			lstrcpy(path, TEXT(".txt"));
		}
		// �t�@�C�����̎擾
		if (filename_select(hWnd, path, TEXT("txt"), STR_TEXT_FILTER, FILE_SAVE_SINGLE) == FALSE) {
			return TRUE;
		}
		if (item_is_mailbox(MailBox + MailBoxIndex, tpMailItem) == FALSE) {
			return FALSE;
		}
		fname = path;
	}

	// �ۑ�����t�@�C�����J��
	hFile = CreateFile(fname, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}
	// �w�b�_�̕ۑ�
	len = CreateHeaderStringSize(head, tpMailItem, NULL);
	tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (tmp == NULL) {
		CloseHandle(hFile);
		return FALSE;
	}
	CreateHeaderString(head, tmp, tpMailItem, NULL);

	if (file_write_ascii(hFile, tmp, len) == FALSE) {
		mem_free(&tmp);
		CloseHandle(hFile);
		return FALSE;
	}
	mem_free(&tmp);

	buf = MIME_body_decode(tpMailItem, FALSE, &tpPart, &cnt, &idx);

	// GJC remove HTML tags
	if (op.StripHtmlTags == 1 && lstrcmp(op.ViewFileSuffix, TEXT("txt")) == 0 &&
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

	// �{���̕ۑ�
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
 * AppViewMail - ���[�����t�@�C���ɕۑ����ĊO���A�v���P�[�V�����ŕ\��
 */
static BOOL AppViewMail(MAILITEM *tpMailItem)
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

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

	// ���[�����t�@�C���ɕۑ�
	str_join_t(path, DataDir, VIEW_FILE, TEXT("."), op.ViewFileSuffix, (TCHAR *)-1);
	if (SaveViewMail(path, NULL, 0, tpMailItem, op.ViewFileHeader) == FALSE) {
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
	// �N��
	if (ShellExecuteEx(&sei) == FALSE) {
		mem_free(&p);
		return FALSE;
	}
	mem_free(&p);
	return TRUE;
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
	return CoshExecute(NULL, file, param);
#endif
}

/*
 * SetMark - �A�C�e���Ƀ}�[�N��t��
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
	} else if (tpMailItem->Mark == mark) {
		tpMailItem->Mark = tpMailItem->MailStatus;
	} else {
		tpMailItem->Mark = mark;
		cut = 0;
	}
	if (i != -1) {
		if (tpMailItem->Download == TRUE) {
			cut = 0;
		}
		ListView_SetItemState(hListView, i, cut, LVIS_CUT);
		ListView_RedrawItems(hListView, i, i);
	}
	UpdateWindow(hListView);
}

/*
 * GetMarkStatus - �A�C�e����Ԏ擾
 */
static void GetMarkStatus(HWND hWnd, MAILITEM *tpMailItem)
{
	HMENU hMenu;
	HWND htv;
	int enable;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hToolBar, ID_MENUITEM_FILE);
	htv = NULL;
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 0);
	htv = NULL;
#else
	hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0);
	htv = GetDlgItem(hWnd, IDC_VCB);
#endif
#else
	hMenu = GetMenu(hWnd);
	htv = GetDlgItem(hWnd, IDC_VTB);
#endif

	enable = !(vSelBox == RecvBox && ExecFlag == TRUE);
	if ((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE) {
		enable = 0;
	}

	CheckMenuItem(hMenu, ID_MENUITEM_DOWNMARK, (tpMailItem->Mark == ICON_DOWN) ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(hMenu, ID_MENUITEM_DELMARK, (tpMailItem->Mark == ICON_DEL) ? MF_CHECKED : MF_UNCHECKED);
	EnableMenuItem(hMenu, ID_MENUITEM_DOWNMARK, !enable);
	EnableMenuItem(hMenu, ID_MENUITEM_DELMARK, !enable);

#if !defined(_WIN32_WCE) || (defined(_WIN32_WCE) && !defined(_WIN32_WCE_LAGENDA))
#if defined(_WIN32_WCE_PPC)
	if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
#endif
	SendMessage(htv, TB_CHECKBUTTON, ID_MENUITEM_DOWNMARK, (LPARAM) MAKELONG((tpMailItem->Mark == ICON_DOWN) ? 1 : 0, 0));
	SendMessage(htv, TB_PRESSBUTTON, ID_MENUITEM_DOWNMARK, (LPARAM) MAKELONG((tpMailItem->Mark == ICON_DOWN) ? 1 : 0, 0));

	SendMessage(htv, TB_CHECKBUTTON, ID_MENUITEM_DELMARK, (LPARAM) MAKELONG((tpMailItem->Mark == ICON_DEL) ? 1 : 0, 0));
	SendMessage(htv, TB_PRESSBUTTON, ID_MENUITEM_DELMARK, (LPARAM) MAKELONG((tpMailItem->Mark == ICON_DEL) ? 1 : 0, 0));

	SendMessage(htv, TB_CHECKBUTTON, ID_MENUITEM_UNREADMARK, (LPARAM) MAKELONG((tpMailItem->Mark == ICON_READ) ? 1 : 0, 0));
	SendMessage(htv, TB_PRESSBUTTON, ID_MENUITEM_UNREADMARK, (LPARAM) MAKELONG((tpMailItem->Mark == ICON_READ) ? 1 : 0, 0));

	SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_DOWNMARK, (LPARAM)MAKELONG(enable, 0));
	SendMessage(htv, TB_ENABLEBUTTON, ID_MENUITEM_DELMARK, (LPARAM)MAKELONG(enable, 0));
#if defined(_WIN32_WCE_PPC)
	}
#endif
#endif
}

/*
 * ViewProc - ���[���\���v���V�[�W��
 */
static LRESULT CALLBACK ViewProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MAILITEM *tpMailItem;
	int i;
	BOOL ret;
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	static BOOL SipFlag = FALSE;
#endif

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
		ModifyWindow(hWnd, (MAILITEM *)lParam, FALSE);
		_SetForegroundWindow(hWnd);
		break;

	case WM_CHANGE_MARK:
		tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
		if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
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
			op.ViewRect.right -= op.ViewRect.left;
			op.ViewRect.bottom -= op.ViewRect.top;
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
		switch (GET_WM_COMMAND_ID(wParam, lParam)) {
#ifdef _WIN32_WCE_PPC
		case ID_MENU:
			SetEditMenu(hWnd);
			ShowMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), 0, 0, FALSE);
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
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_VSCROLL, SB_PAGEUP, 0);
			} else {
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_HSCROLL, SB_LINELEFT, 0);
			}
			break;

		case ID_KEY_RIGHT:
			if (op.WordBreakFlag == 1) {
				View_NextScroll(GetDlgItem(hWnd, IDC_EDIT_BODY));
			} else {
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_HSCROLL, SB_LINERIGHT, 0);
			}
			break;

		case ID_MENUITEM_NEXTMAIL:
			View_NextMail(hWnd, FALSE);
			break;

		case ID_MENUITEM_PREVMAIL:
			View_PrevMail(hWnd);
			break;

		case ID_MENUITEM_NEXTUNREAD:
			View_NextUnreadMail(hWnd);
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

		// ��M�p�Ƀ}�[�N
		case ID_MENUITEM_DOWNMARK:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (vSelBox == MAILBOX_SEND || (MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
				break;
			}
			SetMark(hWnd, tpMailItem, ICON_DOWN);
			GetMarkStatus(hWnd, tpMailItem);
			break;

		// �폜�p�Ƀ}�[�N
		case ID_MENUITEM_DELMARK:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (vSelBox == MAILBOX_SEND || (MailBox+vSelBox)->Type == MAILBOX_TYPE_SAVE) {
				break;
			}
			SetMark(hWnd, tpMailItem, ICON_DEL);
			GetMarkStatus(hWnd, tpMailItem);
			break;

		case ID_MENUITEM_UNREADMARK:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (vSelBox == MAILBOX_SEND) {
				break;
			}
			SetMark(hWnd, tpMailItem, ICON_MAIL);
			tpMailItem->ReFwd = ICON_NON;
			GetMarkStatus(hWnd, tpMailItem);
			break;

		case ID_MENUITEM_SAVE:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (SaveViewMail(NULL, hWnd, vSelBox, tpMailItem, SAVE_HEADER) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_SAVE);
			}
			break;

		case ID_MENUITEM_PROP:
			if (item_is_mailbox(MailBox + vSelBox, 
				(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE) {
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
			op.WordBreakFlag = SetWordBreak(hWnd, hViewMenu);
#else
			op.WordBreakFlag = SetWordBreak(hWnd);
#endif
			SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
			HideCaret(GetDlgItem(hWnd, IDC_EDIT_BODY));
			break;

#ifndef _WIN32_WCE
		case ID_MENUITEM_FONT:
			// �t�H���g
			if (font_select(hWnd, &op.view_font) == TRUE) {
				if (hViewFont != NULL) {
					DeleteObject(hViewFont);
				}
				hViewFont = font_create(hWnd, &op.view_font);
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
				font_charset = op.view_font.charset;
				// Edit�E�B���h�E�̃t�H���g��ݒ�
				EnumWindows((WNDENUMPROC)enum_windows_proc, 0);
				tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
				if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == TRUE) {
					ModifyWindow(hWnd, tpMailItem, FALSE);
				}
			}
			break;
#endif

		case ID_MENUITEM_SHOW_DATE:
			op.ViewShowDate = (op.ViewShowDate == 1) ? 0 : 1;
			SetHeaderSize(hWnd);
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == TRUE) {
				SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
			}
			break;

		case ID_VIEW_SOURCE:
		case ID_VIEW_PART:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			ModifyWindow(hWnd, tpMailItem,
				((GET_WM_COMMAND_ID(wParam, lParam) == ID_VIEW_SOURCE) ? TRUE : FALSE));
			break;

		case ID_VIEW_SAVE_ATTACH:
			// GJC save all attachments
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			ret = TRUE;
			for (i = 0; i < MultiPartCnt && ret == TRUE; i++) {
				if (i == MultiPartTextIndex) {
					continue;
				}
				ret = Decode(hWnd, i, TRUE);
			}
			if (ret == FALSE) {
				ErrorMessage(hWnd, STR_ERR_SAVE);
			}
			break;

		case ID_VIEW_DELETE_ATTACH:
			if (MessageBox(hWnd, STR_Q_DELATTACH, STR_TITLE_DELETE, MB_ICONEXCLAMATION | MB_YESNO) == IDNO) {
				break;
			}
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			DeleteAttachFile(hWnd, tpMailItem);
			ModifyWindow(hWnd, tpMailItem, FALSE);
			break;

		case ID_MENUITEM_VIEW:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (item_is_mailbox(MailBox + vSelBox, tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_NOMAIL);
				break;
			}
			if (AppViewMail(tpMailItem) == FALSE) {
				ErrorMessage(hWnd, STR_ERR_VIEW);
				break;
			}
			if (op.ViewAppClose == 1) {
				SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			break;

		default:
			{
				int id = GET_WM_COMMAND_ID(wParam, lParam);
				if (id >= ID_ATTACH && id <= ID_ATTACH + 100) {
					if (item_is_mailbox(MailBox + vSelBox,
						(MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA)) == FALSE) {
						ErrorMessage(hWnd, STR_ERR_NOMAIL);
						break;
					}
					if (Decode(hWnd, id - ID_ATTACH, FALSE) == FALSE) {
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

/*
 * View_InitApplication - �E�B���h�E�N���X�̓o�^
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
 * View_InitInstance - �E�B���h�E�̍쐬
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
		// �r���[�A�ŕ\��
		if (AppViewMail((MAILITEM *)lpParam) == FALSE) {
			ErrorMessage(MainWnd, STR_ERR_VIEW);
			return FALSE;
		}

		// �J���ς݂ɂ���
		if (((MAILITEM *)lpParam)->MailStatus != ICON_NON && ((MAILITEM *)lpParam)->MailStatus < ICON_SENTMAIL) {
			((MAILITEM *)lpParam)->MailStatus = ICON_READ;
			((MAILITEM *)lpParam)->New = FALSE;
		}

		// �ꗗ�̃A�C�R���̐ݒ�
		if (((MAILITEM *)lpParam)->Mark != ICON_DOWN && ((MAILITEM *)lpParam)->Mark != ICON_DEL) {
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
		op.ViewRect.right,
		op.ViewRect.bottom,
		NULL, NULL, hInstance, lpParam);
#endif
	if (hViewWnd == NULL) {
		return FALSE;
	}

	ShowWindow(hViewWnd, SW_SHOW);
	UpdateWindow(hViewWnd);
	return TRUE;
}
/* End of source */
