/*
 * nPOP
 *
 * Option.c
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
#include "jp.h"
#include "mime.h"
#include "multipart.h"
#include "charset.h"

/* Define */
#ifndef _WIN32_WCE
#define sizeof_PROPSHEETHEADER		40	//of end
#else
#define sizeof_PROPSHEETHEADER		sizeof(PROPSHEETHEADER)
#endif

#define ID_LV_EDIT					(WM_APP + 200)
#define ID_LV_DELETE				(WM_APP + 201)
#define ID_LV_ALLSELECT				(WM_APP + 202)

/* Global Variables */
HWND MsgWnd = NULL;

extern OPTION op;

static MAILBOX *tpOptionMailBox;
static BOOL PropRet;
static HWND hLvFilter;
static int ViewClose;

extern HINSTANCE hInst;  // Local copy of hInstance
extern HWND MainWnd;
extern HWND FocusWnd;
extern HFONT hListFont;
extern TCHAR *g_Pass;
extern int gPassSt;
extern BOOL ShowMsgFlag;
extern BOOL PPCFlag;
extern int AttachProcess;
extern TCHAR *tmp_attach;

extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern int SelBox;
extern int vSelBox;
extern ADDRESSBOOK *AddressBook;

extern TCHAR *FindStr;

/* Local Function Prototypes */
static void SetControlFont(HWND pWnd);
static LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView);
static int ListView_AddOptionItem(HWND hListView, TCHAR *buf);
static TCHAR *ListView_AllocGetText(HWND hListView, int Index, int Col);
static BOOL CALLBACK MboxTypeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK PopSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetSmtpAuthProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SmtpSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK MakeSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SetComboItem(HWND hCombo);
static void EnableFilterEditButton(HWND hDlg, BOOL EnableFlag);
static BOOL CALLBACK EditFilterProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void EnableFilterButton(HWND hDlg, BOOL EnableFlag);
static int GetSaveboxNum(TCHAR *name);
static int GetFilterActionInt(TCHAR *buf);
static TCHAR *GetFilterActionString(int i);
static void SetFilterList(HWND hListView);
static BOOL CALLBACK FilterSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void EnableRasOption(HWND hDlg, int Flag);
static BOOL CALLBACK RasSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK FilterSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetRecvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetSendOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetReplyOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetForwardOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetCheckOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetRasOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetSortOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetEtcOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK EnableSortColumns(HWND hDlg, BOOL EnableFlag);
static void SetCcList(HWND hDlg, TCHAR *strList, TCHAR *type);
static BOOL CALLBACK CcListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SetButtonText(HWND hButton, TCHAR *title, BOOL UseFlag);
static void SetAddressList(HWND hDlg, ADDRESSBOOK *tpAddressBook, TCHAR *Filter);
int ListSelToAddrNum(ADDRESSBOOK *tpAddressBook, int Sel);
static BOOL CALLBACK EditAddressProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
 * PropSheetCallback - プロパティシートのコールバック
 */
#ifdef _WIN32_WCE_PPC
static int CALLBACK PropSheetCallback(HWND hwndDlg, UINT message, LPARAM lParam)
{
	switch (message) {
	case PSCB_GETVERSION:
		return COMCTL32_VERSION;
	}
	return 0;
}
#endif

/*
 * InitDlg - ダイアログの初期化
 */
#ifdef _WIN32_WCE_PPC
static void InitDlg(HWND hDlg, TCHAR *str)
{
	SHINITDLGINFO shidi;

	shidi.dwMask = SHIDIM_FLAGS;
	shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
	shidi.hDlg = hDlg;
	SHInitDialog(&shidi);

	SetWindowText(hDlg, str);
}
#elif defined(_WIN32_WCE)
static void InitDlg(HWND hDlg)
{
	if (PPCFlag == FALSE) {
		RECT MainRect, DialogRect;
		int top;

		GetWindowRect(MainWnd, &MainRect);
		GetWindowRect(hDlg, &DialogRect);

		top = ((MainRect.bottom - MainRect.top) / 2) - ((DialogRect.bottom - DialogRect.top) / 2);
		if (top <= 0) top = DialogRect.top;
		SetWindowPos(hDlg, 0,
			((MainRect.right - MainRect.left) / 2) - ((DialogRect.right - DialogRect.left) / 2),
			top,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
}
#endif

/*
 * SetSip - SIPの設定
 */
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
static void SetSip(HWND hDlg, int edit_notify)
{
#ifdef _WIN32_WCE_PPC
	switch (edit_notify) {
	case EN_SETFOCUS:
	case CBN_SETFOCUS:
		SHSipPreference(hDlg, SIP_UP);
		break;
	case EN_KILLFOCUS:
	case CBN_KILLFOCUS:
		SHSipPreference(hDlg, SIP_DOWN);
		break;
	}
#elif defined(_WIN32_WCE_LAGENDA)
	switch (edit_notify) {
	case EN_SETFOCUS:
	case CBN_SETFOCUS:
		SipShowIM(SIPF_ON);
		break;
	case EN_KILLFOCUS:
	case CBN_KILLFOCUS:
		SipShowIM(SIPF_OFF);
		break;
	}
#endif
}
#endif

/*
 * SetControlFont - コントロールのフォント設定
 */
static void SetControlFont(HWND pWnd)
{
	HWND hWnd;
	TCHAR buf[BUF_SIZE];

	if (hListFont == NULL) return;

	hWnd = GetWindow(pWnd, GW_CHILD);
	do {
		GetClassName(hWnd, buf, BUF_SIZE);
		if (lstrcmpi(buf, TEXT("Edit")) == 0 ||
			lstrcmpi(buf, TEXT("ComboBox")) == 0 ||
			lstrcmpi(buf, TEXT("ListBox")) == 0 ||
			lstrcmpi(buf, WC_LISTVIEW) == 0) {
			SendMessage(hWnd, WM_SETFONT, (WPARAM)hListFont, MAKELPARAM(TRUE,0));
		}
	} while ((hWnd = GetWindow(hWnd, GW_HWNDNEXT)) != NULL);
}

/*
 * OptionNotifyProc - プロパティシートのイベントの通知
 */
static LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PSHNOTIFY *pshn = (PSHNOTIFY FAR *) lParam;
	NMHDR *lpnmhdr = (NMHDR FAR *)&pshn->hdr;

	switch (lpnmhdr->code) {
	case PSN_APPLY:				// OK
		PropRet = TRUE;
		SendMessage(hDlg, WM_COMMAND, IDOK, 0);
		break;

	case PSN_QUERYCANCEL:		//Cancellation
		break;
	}
	return PSNRET_NOERROR;
}

/*
 * DialogLvNotifyProc - オプション画面のリストビューメッセージ
 */
static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView)
{
	NMHDR *CForm = (NMHDR *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;

	if (CForm->hwndFrom != hListView) {
		return 0;
	}

	if (CForm->code == NM_DBLCLK) {
		SendMessage(hWnd, WM_COMMAND, ID_LV_EDIT, 0);
		return 1;
	}

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	if (LKey->hdr.code == LVN_BEGINDRAG) {
		return LVBD_DRAGSELECT;
	}
#endif

	if (LKey->hdr.code == LVN_KEYDOWN && LKey->wVKey == VK_DELETE) {
		SendMessage(hWnd, WM_COMMAND, ID_LV_DELETE, 0);
		return 1;
	}
	if (LKey->hdr.code == LVN_KEYDOWN && LKey->wVKey == TEXT('A') && GetKeyState(VK_CONTROL) < 0) {
		SendMessage(hWnd, WM_COMMAND, ID_LV_ALLSELECT, 0);
		return 1;
	}
	return 0;
}

/*
 * ListView_AddOptionItem - リストビューにテキストを追加する
 */
static int ListView_AddOptionItem(HWND hListView, TCHAR *buf)
{
	return ListView_InsertItemEx(hListView, buf, BUF_SIZE - 1, 0, 0, ListView_GetItemCount(hListView));
}

/*
 * ListView_AddOptionItem - リストビューにテキストを追加する
 */
static TCHAR *ListView_AllocGetText(HWND hListView, int Index, int Col)
{
	TCHAR buf[BUF_SIZE];

	*buf = TEXT('\0');
	ListView_GetItemText(hListView, Index, Col, buf, BUF_SIZE - 1);
	return alloc_copy_t(buf);
}

/*
 * AllocGetText - EDITに設定されているサイズ分のメモリを確保してEDITの内容を設定する
 */
void AllocGetText(HWND hEdit, TCHAR **buf)
{
	int len;

	mem_free(&*buf);
	len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
	*buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (*buf != NULL) {
		**buf = TEXT('\0');
		SendMessage(hEdit, WM_GETTEXT, len, (LPARAM)*buf);
	}
}

/*
 * DrawScrollControl - スクロールバーのボタンの描画
 */
#ifndef _WIN32_WCE
void DrawScrollControl(LPDRAWITEMSTRUCT lpDrawItem, UINT i)
{
	#define FOCUSRECT_SIZE		3

	if (lpDrawItem->itemState & ODS_DISABLED) {
		i |= DFCS_INACTIVE;
	}
	if (lpDrawItem->itemState & ODS_SELECTED) {
		i |= DFCS_PUSHED;
	}

	//Drawing
	DrawFrameControl(lpDrawItem->hDC, &(lpDrawItem->rcItem), DFC_SCROLL, i);

	//of frame control Focusing
	if (lpDrawItem->itemState & ODS_FOCUS) {
		lpDrawItem->rcItem.left += FOCUSRECT_SIZE;
		lpDrawItem->rcItem.top += FOCUSRECT_SIZE;
		lpDrawItem->rcItem.right -= FOCUSRECT_SIZE;
		lpDrawItem->rcItem.bottom -= FOCUSRECT_SIZE;
		DrawFocusRect(lpDrawItem->hDC, &(lpDrawItem->rcItem));
	}
}
#endif

/*
 * SetSSLProc - SSL 設定プロシージャ
 */
static BOOL CALLBACK SetSSLProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_SETSSL);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SSL_AUTO);
		SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SSL_TLS10);
		SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SSL_SSL30);
		SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SSL_SSL20);
		SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SSL_STARTTLS);

		if (lParam == 0) {
			SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_SETCURSEL, tpOptionMailBox->PopSSLInfo.Type, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_VERIFY, BM_SETCHECK, tpOptionMailBox->PopSSLInfo.Verify, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_SETCURSEL, tpOptionMailBox->SmtpSSLInfo.Type, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_VERIFY, BM_SETCHECK, tpOptionMailBox->SmtpSSLInfo.Verify, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (GetWindowLong(hDlg, GWL_USERDATA) == 0) {
				tpOptionMailBox->PopSSLInfo.Type = SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_GETCURSEL, 0, 0);
				tpOptionMailBox->PopSSLInfo.Verify = SendDlgItemMessage(hDlg, IDC_CHECK_VERIFY, BM_GETCHECK, 0, 0);
			} else {
				tpOptionMailBox->SmtpSSLInfo.Type = SendDlgItemMessage(hDlg, IDC_COMBO_SSLTYPE, CB_GETCURSEL, 0, 0);
				tpOptionMailBox->SmtpSSLInfo.Verify = SendDlgItemMessage(hDlg, IDC_CHECK_VERIFY, BM_GETCHECK, 0, 0);
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

/*
 * PopSetProc - POP3設定プロシージャ
 */
static BOOL CALLBACK PopSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* of control Initialization */
		SetControlFont(hDlg);

		SendDlgItemMessage(hDlg, IDC_EDIT_NAME, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->Name);
		SendDlgItemMessage(hDlg, IDC_EDIT_SERVER, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->Server);
		SetDlgItemInt(hDlg, IDC_EDIT_PORT, tpOptionMailBox->Port, FALSE);
		SendDlgItemMessage(hDlg, IDC_EDIT_USER, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->User);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->Pass);

		SendDlgItemMessage(hDlg, IDC_CHECK_APOP, BM_SETCHECK, tpOptionMailBox->APOP, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_SETCHECK, tpOptionMailBox->PopSSL, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_CYCLIC, BM_SETCHECK, tpOptionMailBox->CyclicFlag, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_NORETR, BM_SETCHECK, tpOptionMailBox->NoRETR, 0);

		SendDlgItemMessage(hDlg, IDC_EDIT_NAME, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_SERVER, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PORT, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_USER, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL),
			SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0));
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_NAME:
		case IDC_EDIT_SERVER:
		case IDC_EDIT_PORT:
		case IDC_EDIT_USER:
		case IDC_EDIT_PASS:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif
		case IDC_CHECK_SSL:
			if (SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0) != 0) {
				SetDlgItemInt(hDlg, IDC_EDIT_PORT, 995, FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL), 1);
			} else {
				SetDlgItemInt(hDlg, IDC_EDIT_PORT, 110, FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL), 0);
			}
			break;

		case IDC_BUTTON_SETSSL:
			// SSL 設定
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSSL), hDlg, SetSSLProc, (LPARAM)0);
			break;

		case IDOK:
			//Name
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_NAME), &tpOptionMailBox->Name);
			//Server
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_SERVER), &tpOptionMailBox->Server);
			tpOptionMailBox->PopIP = 0;
			//Port number
			tpOptionMailBox->Port = GetDlgItemInt(hDlg, IDC_EDIT_PORT, NULL, FALSE);
			// ユーザID
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_USER), &tpOptionMailBox->User);
			//Password
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &tpOptionMailBox->Pass);
			// APOP
			tpOptionMailBox->APOP = SendDlgItemMessage(hDlg, IDC_CHECK_APOP, BM_GETCHECK, 0, 0);
			// POP SSL
			tpOptionMailBox->PopSSL = SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0);
			// CyclicFlag
			tpOptionMailBox->CyclicFlag = SendDlgItemMessage(hDlg, IDC_CHECK_CYCLIC, BM_GETCHECK, 0, 0);
			// No RETR
			tpOptionMailBox->NoRETR = SendDlgItemMessage(hDlg, IDC_CHECK_NORETR, BM_GETCHECK, 0, 0);

			//Temporarily the release
			mem_free(&tpOptionMailBox->TmpPass);
			tpOptionMailBox->TmpPass = NULL;
			mem_free(&tpOptionMailBox->SmtpTmpPass);
			tpOptionMailBox->SmtpTmpPass = NULL;
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetSmtpAuthProc - SMTP認証 設定プロシージャ
 */
static BOOL CALLBACK SetSmtpAuthProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int j;
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_SMTPAUTH);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SendDlgItemMessage(hDlg, IDC_COMBO_SMTPAUTHTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SMTPAUTH_CRAM_MD5);
		SendDlgItemMessage(hDlg, IDC_COMBO_SMTPAUTHTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SMTPAUTH_LOGIN);
		SendDlgItemMessage(hDlg, IDC_COMBO_SMTPAUTHTYPE, CB_ADDSTRING, 0, (LPARAM)STR_SMTPAUTH_PLAIN);
		j = tpOptionMailBox->SmtpAuthType; // allowed values are 0, 2, 3
		if (j > 3 || j <= 0) {
			j = 0;
		} else {
			j--;
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_SMTPAUTHTYPE, CB_SETCURSEL, j, 0);

		SendDlgItemMessage(hDlg, IDC_CHECK_POP, BM_SETCHECK, !tpOptionMailBox->AuthUserPass, 0);

		SendDlgItemMessage(hDlg, IDC_EDIT_USER, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->SmtpUser);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->SmtpPass);
		SendDlgItemMessage(hDlg, IDC_EDIT_USER, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

		SendMessage(hDlg, WM_COMMAND, (WPARAM)IDC_CHECK_POP, 0);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_USER:
		case IDC_EDIT_PASS:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_CHECK_POP:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER),
				!SendDlgItemMessage(hDlg, IDC_CHECK_POP, BM_GETCHECK, 0, 0));
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS),
				!SendDlgItemMessage(hDlg, IDC_CHECK_POP, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			//auth type
			j = SendDlgItemMessage(hDlg, IDC_COMBO_SMTPAUTHTYPE, CB_GETCURSEL, 0, 0);
			if (j != 0) j++;
			tpOptionMailBox->SmtpAuthType = j;
			//of the password Certification mode
			tpOptionMailBox->AuthUserPass = !SendDlgItemMessage(hDlg, IDC_CHECK_POP, BM_GETCHECK, 0, 0);
			//User ID
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_USER), &tpOptionMailBox->SmtpUser);
			//Password
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &tpOptionMailBox->SmtpPass);
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

/*
 * SmtpSetProc - SMTP設定プロシージャ
 */
static BOOL CALLBACK SmtpSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* of control Initialization */
		SetControlFont(hDlg);

		SendDlgItemMessage(hDlg, IDC_EDIT_NAME, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->UserName);
		SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->MailAddress);
		SendDlgItemMessage(hDlg, IDC_EDIT_SERVER, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->SmtpServer);
		SetDlgItemInt(hDlg, IDC_EDIT_PORT, tpOptionMailBox->SmtpPort, FALSE);

		SendDlgItemMessage(hDlg, IDC_CHECK_SMTPAUTH, BM_SETCHECK, tpOptionMailBox->SmtpAuth, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_POPBEFORESMTP, BM_SETCHECK, tpOptionMailBox->PopBeforeSmtp, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_MYADDR2BCC, BM_SETCHECK, tpOptionMailBox->MyAddr2Bcc, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_SETCHECK, tpOptionMailBox->SmtpSSL, 0);

		SendDlgItemMessage(hDlg, IDC_EDIT_NAME, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_SERVER, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PORT, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

		SendMessage(hDlg, WM_COMMAND, (WPARAM)IDC_CHECK_SMTPAUTH, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL),
			SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0));
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_NAME:
		case IDC_EDIT_MAILADDRESS:
		case IDC_EDIT_SERVER:
		case IDC_EDIT_PORT:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif
		case IDC_CHECK_SMTPAUTH:
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETAUTH),
				SendDlgItemMessage(hDlg, IDC_CHECK_SMTPAUTH, BM_GETCHECK, 0, 0));
			break;

		case IDC_BUTTON_SETAUTH:
			//smtp certification setting
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSMTPAUTH), hDlg, SetSmtpAuthProc, (LPARAM)0);
			break;

		case IDC_CHECK_SSL:
			if (SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0) != 0) {
				SetDlgItemInt(hDlg, IDC_EDIT_PORT, 465, FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL), 1);
			} else {
				SetDlgItemInt(hDlg, IDC_EDIT_PORT, 25, FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL), 0);
			}
			break;

		case IDC_BUTTON_SETSSL:
			// SSL 設定
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSSL), hDlg, SetSSLProc, (LPARAM)1);
			break;

		case IDOK:
			//Autonym
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_NAME), &tpOptionMailBox->UserName);
			//Mail address
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), &tpOptionMailBox->MailAddress);
			//Server
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_SERVER), &tpOptionMailBox->SmtpServer);
			tpOptionMailBox->SmtpIP = 0;
			//Port number
			tpOptionMailBox->SmtpPort = GetDlgItemInt(hDlg, IDC_EDIT_PORT, NULL, FALSE);
			// SMTP Authentication
			tpOptionMailBox->SmtpAuth = SendDlgItemMessage(hDlg, IDC_CHECK_SMTPAUTH, BM_GETCHECK, 0, 0);
			// SMTP SSL
			tpOptionMailBox->SmtpSSL = SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0);
			// POP before SMTP
			tpOptionMailBox->PopBeforeSmtp = SendDlgItemMessage(hDlg, IDC_CHECK_POPBEFORESMTP, BM_GETCHECK, 0, 0);
			//Transmit the copy to your own address the flag
			tpOptionMailBox->MyAddr2Bcc = SendDlgItemMessage(hDlg, IDC_CHECK_MYADDR2BCC, BM_GETCHECK, 0, 0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * MakeSetProc - 作成設定プロシージャ
 */
static BOOL CALLBACK MakeSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SendDlgItemMessage(hDlg, IDC_EDIT_SIG, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->Signature);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_SIG:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			//which Signature
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_SIG), &tpOptionMailBox->Signature);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * EditFilterProc - フィルタ項目編集プロシージャ
 */
static void SetComboItem(HWND hCombo)
{
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("From:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("To:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Cc:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Subject:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Organization:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Reply-To:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Comments:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Content-Type:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Date:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Keywords:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("Sender:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("X-Mailer:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("X-Priority:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("X-Sender:"));
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)TEXT("X-ML-Name:"));
}

/*
 * EnableFilterEditButton - ボタンの活性／非活性の切り替え
 */
static void EnableFilterEditButton(HWND hDlg, BOOL EnableFlag)
{
	EnableWindow(GetDlgItem(hDlg, IDC_COMBO_ACTION), EnableFlag);
	// handle IDC_COMBO_FILT2BOX separately
	EnableWindow(GetDlgItem(hDlg, IDC_COMBO_HEAD1), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CONTENT1), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_COMBO_HEAD2), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_CONTENT2), EnableFlag);
}

/*
 * EditFilterProc - フィルタ項目編集プロシージャ
 */
static BOOL CALLBACK EditFilterProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[BUF_SIZE], Name[BUF_SIZE];
	TCHAR *p, *nptr;
	int i, j;
	BOOL enabled = TRUE;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_FILTER);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_UNRECV);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_RECV);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_DOWNLOADMARK);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_DELETEMARK);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_READICON);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_COPY);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_MOVE);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, 0, 0);

		SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_ADDSTRING, 0, (LPARAM)TEXT(""));
		for (i=0; i<MailBoxCnt; i++) {
			if ((MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
				if ((MailBox+i)->Name != NULL && *(MailBox+i)->Name != TEXT('\0')) {
					SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_ADDSTRING, 0, (LPARAM)(MailBox+i)->Name);
				} else {
					SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
				}
			}
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_SETCURSEL, 0, 0);

		SetComboItem(GetDlgItem(hDlg, IDC_COMBO_HEAD1));
		SetComboItem(GetDlgItem(hDlg, IDC_COMBO_HEAD2));

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		i = lParam;
		if (i == -1) {
			SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_SETCHECK, 1, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), FALSE);
			break;
		}

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 0, buf, BUF_SIZE - 1);
		if (lstrcmp(buf, STR_FILTER_USE) == 0) {
			SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_SETCHECK, 1, 0);
		} else {
			EnableFilterEditButton(hDlg, FALSE);
			enabled = FALSE;
		}

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 1, buf, BUF_SIZE - 1);
		if (str_cmp_ni_t(buf, STR_FILTER_COPY, lstrlen(STR_FILTER_COPY)) == 0) {
			nptr = buf + lstrlen(STR_FILTER_COPY);
			SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_SETCURSEL, GetSaveboxNum(nptr), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), enabled);
			*(buf + lstrlen(STR_FILTER_COPY)) = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);
		} else if (str_cmp_ni_t(buf, STR_FILTER_MOVE, lstrlen(STR_FILTER_MOVE)) == 0) {
			nptr = buf + lstrlen(STR_FILTER_MOVE);
			SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_SETCURSEL, GetSaveboxNum(nptr), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), enabled);
			*(buf + lstrlen(STR_FILTER_MOVE)) = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);
		} else {
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), FALSE);
			SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);
		}

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 2, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_COMBO_HEAD1, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 3, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_CONTENT1, WM_SETTEXT, 0, (LPARAM)buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_CONTENT1, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 4, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_COMBO_HEAD2, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 5, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_CONTENT2, WM_SETTEXT, 0, (LPARAM)buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_CONTENT2, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_CONTENT1:
		case IDC_EDIT_CONTENT2:
		case IDC_COMBO_HEAD1:
		case IDC_COMBO_HEAD2:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_CHECK_FLAG:
			enabled = SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_GETCHECK, 0, 0);
			EnableFilterEditButton(hDlg, enabled);
			if (enabled) {
				j = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
				if (j != FILTER_COPY_INDEX && j != FILTER_MOVE_INDEX) {
					enabled = FALSE;
				}
			}
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), enabled);
			break;

		case IDC_COMBO_ACTION:
			j = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
			if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX) {
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), TRUE);
			} else {
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), FALSE);
			}
			break;

		case IDOK:
			i = GetWindowLong(hDlg, GWL_USERDATA);

			if (SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_GETCHECK, 0, 0) == FALSE) {
				p = STR_FILTER_NOUSE;
			} else {
				p = STR_FILTER_USE;
			}

			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_HEAD1, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') && p == STR_FILTER_USE) {
				ErrorMessage(hDlg, STR_ERR_NOITEM1);
				break;
			}
			*Name = TEXT('\0');
			j = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
			if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX) {
				SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Name);
				if (*Name == TEXT('\0') && p == STR_FILTER_USE) {
					ErrorMessage(hDlg, STR_ERR_NOSAVEBOX);
					break;
				}
			}

			if (i == -1) {
				i = ListView_AddOptionItem(hLvFilter, p);
			} else {
				ListView_SetItemText(hLvFilter, i, 0, p);
			}

			if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX) {
				wsprintf(buf, TEXT("%s%s"), GetFilterActionString(j), Name);
			} else {
				wsprintf(buf, TEXT("%s"), GetFilterActionString(j));
			}
			ListView_SetItemText(hLvFilter, i, 1, buf);

			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_HEAD1, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			ListView_SetItemText(hLvFilter, i, 2, buf);

			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_EDIT_CONTENT1, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			ListView_SetItemText(hLvFilter, i, 3, buf);

			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_HEAD2, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			ListView_SetItemText(hLvFilter, i, 4, buf);
			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_EDIT_CONTENT2, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			ListView_SetItemText(hLvFilter, i, 5, buf);

			ListView_SetItemState(hLvFilter, -1, 0, LVIS_SELECTED);
			ListView_SetItemState(hLvFilter, i,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			ListView_EnsureVisible(hLvFilter, i, TRUE);

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

/*
 * EnableFilterButton - ボタンの活性／非活性の切り替え
 */
static void EnableFilterButton(HWND hDlg, BOOL EnableFlag)
{
	EnableWindow(GetDlgItem(hDlg, IDC_LIST_FILTER), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), EnableFlag);
}

/*
 * GetSaveboxNum - get combobox index to savebox with this name
 */
static int GetSaveboxNum(TCHAR *name) {
	int i, j = 0, ret=0;
	for (i=0; i<MailBoxCnt; i++) {
		if ((MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
			j++;
			if ((MailBox+i)->Name != NULL && *(MailBox+i)->Name != TEXT('\0')) {
				if (lstrcmp(name, (MailBox+i)->Name) == 0) {
					ret = j;
					break;
				}
			} else {
				if (lstrcmp(name, STR_MAILBOX_NONAME) == 0) {
					ret = j;
					break;
				}
			}
		}
	}
	return ret;
}

/*
 * GetFilterActionInt - 動作値の取得
 */
static int GetFilterActionInt(TCHAR *buf)
{
	if (lstrcmp(buf, STR_FILTER_RECV) == 0) {
		return FILTER_RECV_INDEX;

	} else if (lstrcmp(buf, STR_FILTER_DOWNLOADMARK) == 0) {
		return FILTER_DOWNLOADMARK_INDEX;

	} else if (lstrcmp(buf, STR_FILTER_DELETEMARK) == 0) {
		return FILTER_DELETEMARK_INDEX;

	} else if (lstrcmp(buf, STR_FILTER_READICON) == 0) {
		return FILTER_READICON_INDEX;

	} else if (lstrcmp(buf, STR_FILTER_COPY) == 0) {
		return FILTER_COPY_INDEX;

	} else if (lstrcmp(buf, STR_FILTER_MOVE) == 0) {
		return FILTER_MOVE_INDEX;

	}
	return FILTER_UNRECV_INDEX;
}

/*
 * GetFilterActionString - 動作文字列の取得
 */
static TCHAR *GetFilterActionString(int i)
{
	switch (i) {
	case FILTER_RECV_INDEX:
		return STR_FILTER_RECV;

	case FILTER_DOWNLOADMARK_INDEX:
		return STR_FILTER_DOWNLOADMARK;

	case FILTER_DELETEMARK_INDEX:
		return STR_FILTER_DELETEMARK;

	case FILTER_READICON_INDEX:
		return STR_FILTER_READICON;

	case FILTER_COPY_INDEX:
		return STR_FILTER_COPY;

	case FILTER_MOVE_INDEX:
		return STR_FILTER_MOVE;

	}
	return STR_FILTER_UNRECV;
}

/*
 * SetFilterList - リストビューにフィルタのリストを表示する
 */
static void SetFilterList(HWND hListView)
{
	TCHAR buf[BUF_SIZE];
	int ItemIndex;
	int i, j;

	for (i = 0; i < tpOptionMailBox->FilterCnt; i++) {
		if (*(tpOptionMailBox->tpFilter + i) == NULL) {
			continue;
		}

		ItemIndex = ListView_AddOptionItem(hListView,
			((*(tpOptionMailBox->tpFilter + i))->Enable == 0) ? STR_FILTER_NOUSE : STR_FILTER_USE);

		j = (*(tpOptionMailBox->tpFilter + i))->Action;
		if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX) {
			wsprintf(buf, TEXT("%s%s"), GetFilterActionString(j), (*(tpOptionMailBox->tpFilter + i))->SaveboxName);
		} else {
			wsprintf(buf, TEXT("%s"), GetFilterActionString(j));
		}
		ListView_SetItemText(hListView, ItemIndex, 1, buf);

		ListView_SetItemText(hListView, ItemIndex, 2,
			(*(tpOptionMailBox->tpFilter + i))->Header1);
		ListView_SetItemText(hListView, ItemIndex, 3,
			(*(tpOptionMailBox->tpFilter + i))->Content1);

		ListView_SetItemText(hListView, ItemIndex, 4,
			(*(tpOptionMailBox->tpFilter + i))->Header2);
		ListView_SetItemText(hListView, ItemIndex, 5,
			(*(tpOptionMailBox->tpFilter + i))->Content2);
	}
}

/*
 * FilterSetProc - フィルタ設定プロシージャ
 */
static BOOL CALLBACK FilterSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hListView;
	TCHAR buf[BUF_SIZE];
	int SelectItem;
	int i;

	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);

		hListView = hLvFilter = GetDlgItem(hDlg, IDC_LIST_FILTER);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 50, STR_FILTER_STATUS, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_ACTION, 1);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_ITEM1, 2);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_CONTENT1, 3);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_ITEM2, 4);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_CONTENT2, 5);
		ListView_SetExtendedListViewStyle(hListView,
			LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		if (tpOptionMailBox->FilterEnable == 0) {
			EnableFilterButton(hDlg, FALSE);
		} else {
			SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_SETCHECK, 1, 0);
		}
		SetFilterList(GetDlgItem(hDlg, IDC_LIST_FILTER));
		break;

#ifndef _WIN32_WCE
	case WM_DRAWITEM:
		switch ((UINT)wParam) {
		case IDC_BUTTON_UP:
			DrawScrollControl((LPDRAWITEMSTRUCT)lParam, DFCS_SCROLLUP);
			break;
		case IDC_BUTTON_DOWN:
			DrawScrollControl((LPDRAWITEMSTRUCT)lParam, DFCS_SCROLLDOWN);
			break;
		default:
			return FALSE;
		}
		break;
#endif

	case WM_NOTIFY:
		if (DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_FILTER)) == 1) {
			break;
		}
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_CHECK_FILTER:
			EnableFilterButton(hDlg, SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_GETCHECK, 0, 0));
			break;

		case IDC_BUTTON_UP:
			if ((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_FILTER), -1, LVNI_SELECTED)) <= 0) {
				break;
			}
			ListView_MoveItem(GetDlgItem(hDlg, IDC_LIST_FILTER), SelectItem, -1, 6);
			break;

		case IDC_BUTTON_DOWN:
			hListView = GetDlgItem(hDlg, IDC_LIST_FILTER);
			if ((SelectItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (SelectItem == ListView_GetItemCount(hListView) - 1) {
				break;
			}
			ListView_MoveItem(hListView, SelectItem, 1, 6);
			break;

		case IDC_BUTTON_ADD:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_FILTER_EDIT), hDlg, EditFilterProc, (LPARAM)-1);
			break;

		case IDC_BUTTON_EDIT:
		case ID_LV_EDIT:
			if ((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_FILTER), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_FILTER_EDIT), hDlg, EditFilterProc, (LPARAM)SelectItem);
			break;

		case IDC_BUTTON_DELETE:
		case ID_LV_DELETE:
			if ((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_FILTER), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (MessageBox(hDlg, STR_Q_DELETE, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_FILTER), SelectItem);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_FILTER), SelectItem,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			break;

		case IDOK:
			tpOptionMailBox->FilterEnable = SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_GETCHECK, 0, 0);
			if (tpOptionMailBox->tpFilter != NULL) {
				filter_free(tpOptionMailBox);
			}
			tpOptionMailBox->FilterCnt = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_FILTER));
			tpOptionMailBox->tpFilter = (FILTER **)mem_calloc(sizeof(FILTER *) * tpOptionMailBox->FilterCnt);
			if (tpOptionMailBox->tpFilter == NULL) {
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}
			for (i = 0; i < tpOptionMailBox->FilterCnt; i++) {
				*(tpOptionMailBox->tpFilter + i) = (FILTER *)mem_calloc(sizeof(FILTER));
				if (*(tpOptionMailBox->tpFilter + i) == NULL) {
					continue;
				}
				hListView = GetDlgItem(hDlg, IDC_LIST_FILTER);
				*buf = TEXT('\0');
				ListView_GetItemText(hListView, i, 0, buf, BUF_SIZE - 1);
				(*(tpOptionMailBox->tpFilter + i))->Enable = (lstrcmp(buf, STR_FILTER_USE) == 0) ? 1 : 0;

				*buf = TEXT('\0');
				ListView_GetItemText(hListView, i, 1, buf, BUF_SIZE - 1);
				if (str_cmp_ni_t(buf, STR_FILTER_COPY, lstrlen(STR_FILTER_COPY)) == 0) {
					(*(tpOptionMailBox->tpFilter + i))->SaveboxName = alloc_copy_t(buf + lstrlen(STR_FILTER_COPY));
				    *(buf + lstrlen(STR_FILTER_COPY)) = TEXT('\0');
				} else if (str_cmp_ni_t(buf, STR_FILTER_MOVE, lstrlen(STR_FILTER_MOVE)) == 0) {
					(*(tpOptionMailBox->tpFilter + i))->SaveboxName = alloc_copy_t(buf + lstrlen(STR_FILTER_MOVE));
					*(buf + lstrlen(STR_FILTER_MOVE)) = TEXT('\0');
				}
				(*(tpOptionMailBox->tpFilter + i))->Action = GetFilterActionInt(buf);

				(*(tpOptionMailBox->tpFilter + i))->Header1 = ListView_AllocGetText(hListView, i, 2);
				(*(tpOptionMailBox->tpFilter + i))->Content1 = ListView_AllocGetText(hListView, i, 3);
				(*(tpOptionMailBox->tpFilter + i))->Header2 = ListView_AllocGetText(hListView, i, 4);
				(*(tpOptionMailBox->tpFilter + i))->Content2 = ListView_AllocGetText(hListView, i, 5);
			}
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * EnableRasOption - RAS設定の活性／非活性の切り替え
 */
static void EnableRasOption(HWND hDlg, int Flag)
{
	TCHAR Entry[BUF_SIZE];

	*Entry = TEXT('\0');
	SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_GETLBTEXT,
		SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_GETCURSEL, 0, 0), (LPARAM)Entry);
	EnableWindow(GetDlgItem(hDlg, IDC_COMBO_ENTRY), Flag);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), (*Entry == TEXT('\0')) ? FALSE : Flag);
	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), (*Entry == TEXT('\0')) ? FALSE : Flag);
	EnableWindow(GetDlgItem(hDlg, IDC_CHECK_RASRECON), (*Entry == TEXT('\0')) ? FALSE : Flag);
}

/*
 * RasSetProc - RAS設定プロシージャ
 */
static BOOL CALLBACK RasSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR Entry[BUF_SIZE];
	TCHAR User[BUF_SIZE];
	TCHAR Pass[BUF_SIZE];
	int i;
	static int SelEntry;

	switch (uMsg) {
	case WM_INITDIALOG:
		SetControlFont(hDlg);
		/* コントロールの初期化 */
		if (GetRasEntries(GetDlgItem(hDlg, IDC_COMBO_ENTRY)) == FALSE) {
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_RASMODE_0), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_RASMODE_1), FALSE);
			EnableRasOption(hDlg, FALSE);
		}
		ComboBox_AddString(GetDlgItem(hDlg, IDC_COMBO_ENTRY), TEXT(""));

		// RAS設定
		SelEntry = SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_SELECTSTRING, -1, (LPARAM)tpOptionMailBox->RasEntry);
		if (SelEntry != -1 && (i = GetRasInfo(tpOptionMailBox->RasEntry)) != -1) {
			SendDlgItemMessage(hDlg, IDC_EDIT_USER, WM_SETTEXT, 0, (LPARAM)(*(op.RasInfo + i))->RasUser);
			SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0, (LPARAM)(*(op.RasInfo + i))->RasPass);
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_SETEXTENDEDUI, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_RASRECON, BM_SETCHECK, tpOptionMailBox->RasReCon, 0);

		switch (tpOptionMailBox->RasMode) {
		case 0:
			// LAN
			EnableRasOption(hDlg, FALSE);
			SendDlgItemMessage(hDlg, IDC_RADIO_RASMODE_0, BM_SETCHECK, 1, 0);
			break;

		case 1:
			// RAS
			EnableRasOption(hDlg, IsWindowEnabled(GetDlgItem(hDlg, IDC_RADIO_RASMODE_1)));
			SendDlgItemMessage(hDlg, IDC_RADIO_RASMODE_1, BM_SETCHECK, 1, 0);
			break;
		}

		SendDlgItemMessage(hDlg, IDC_EDIT_USER, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_USER:
		case IDC_EDIT_PASS:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif
		case IDC_COMBO_ENTRY:
			if (HIWORD(wParam) == CBN_CLOSEUP) {
				if (SelEntry != -1) {
					*Entry = TEXT('\0');
					SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_GETLBTEXT, SelEntry, (LPARAM)Entry);
					SendDlgItemMessage(hDlg, IDC_EDIT_USER, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)User);
					SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Pass);
					SetRasInfo(Entry, User, Pass);
				}

				SelEntry = SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_GETCURSEL, 0, 0);
				*Entry = TEXT('\0');
				SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_GETLBTEXT, SelEntry, (LPARAM)Entry);
				i = (*Entry == TEXT('\0')) ? -1 : GetRasInfo(Entry);
				SendDlgItemMessage(hDlg, IDC_EDIT_USER, WM_SETTEXT, 0,
					(LPARAM)((i == -1) ? TEXT("") : (*(op.RasInfo + i))->RasUser));
				SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0,
					(LPARAM)((i == -1) ? TEXT("") : (*(op.RasInfo + i))->RasPass));
				EnableRasOption(hDlg, TRUE);
			}
			break;

		case IDC_RADIO_RASMODE_0:
		case IDC_RADIO_RASMODE_1:
			i = SendDlgItemMessage(hDlg, IDC_RADIO_RASMODE_1, BM_GETCHECK, 0, 0);
			EnableRasOption(hDlg, i);
			break;

		case IDOK:
			tpOptionMailBox->RasMode = SendDlgItemMessage(hDlg, IDC_RADIO_RASMODE_1, BM_GETCHECK, 0, 0);
			AllocGetText(GetDlgItem(hDlg, IDC_COMBO_ENTRY), &tpOptionMailBox->RasEntry);

			*Entry = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_ENTRY, CB_GETLBTEXT, SelEntry, (LPARAM)Entry);
			SendDlgItemMessage(hDlg, IDC_EDIT_USER, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)User);
			SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Pass);
			SetRasInfo(Entry, User, Pass);

			tpOptionMailBox->RasReCon = SendDlgItemMessage(hDlg, IDC_CHECK_RASRECON, BM_GETCHECK, 0, 0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * MboxTypeProc - set type: Account or SaveBox (GJC)
*/
BOOL CALLBACK MboxTypeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAILBOX *mbox = (MailBox + SelBox);
	BOOL ret = TRUE;
	int Type;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_NEWMBOX);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		if (mbox == NULL) {
			ret = FALSE;
			break;
		}
		Type = (int)lParam;
		SendDlgItemMessage(hDlg, IDC_MBOX_NAME, WM_SETTEXT, 0, (LPARAM)mbox->Name);
		SendDlgItemMessage(hDlg, IDC_MBOX_NAME, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		if (Type == MAILBOX_TYPE_SAVE || Type == MAILBOX_ADD_SAVE) {
			SendMessage(hDlg, WM_SETTEXT, 0, (Type == MAILBOX_TYPE_SAVE) ?
				(LPARAM)STR_TITLE_RENAMESBOX : (LPARAM)STR_TITLE_ADDSBOX);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_MBOXIN), FALSE);
			SendDlgItemMessage(hDlg, IDC_RADIO_MBOXSAVE, BM_SETCHECK, BST_CHECKED, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_RADIO_MBOXIN, BM_SETCHECK, BST_CHECKED, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (mbox != NULL) {
				AllocGetText(GetDlgItem(hDlg, IDC_MBOX_NAME), &mbox->Name);
				mbox->Type = SendDlgItemMessage(hDlg, IDC_RADIO_MBOXSAVE, BM_GETCHECK, 0, 0);
				if (mbox->Type == 1 && (mbox->Name == NULL || *mbox->Name == TEXT('\0'))) {
					mem_free(&mbox->Name);
					mbox->Name = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(STR_SAVEBOX_NONAME)+1));
					if (mbox->Name != NULL) {
						wsprintf(mbox->Name, STR_SAVEBOX_NONAME, (SelBox%100));
					}
				}
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
	return ret;
}

/* 
 * SetMailBoxType - set type: Account or SaveBox (GJC)
 */
int SetMailBoxType(HWND hWnd, int Type)
{
	if (SelBox == MAILBOX_SEND) {
		return -1;
	}
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MBOXTYPE), hWnd, MboxTypeProc, (LPARAM)Type) == FALSE) {
		return -1;
	} else {
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox,
			(LPARAM)(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
			? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
		return (MailBox + SelBox)->Type;
	}
}

/*
 * SetSaveBoxName - change name of SaveBox (GJC)
 */
BOOL SetSaveBoxName(HWND hWnd)
{
	TCHAR old_name[BUF_SIZE], new_name[BUF_SIZE];
	int i, j;

	wsprintf(old_name, TEXT("%s"), (MailBox + SelBox)->Name);
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_MBOXTYPE), hWnd, MboxTypeProc, (LPARAM)MAILBOX_TYPE_SAVE) == TRUE) {
		if ((MailBox + SelBox)->Name != NULL) {
			wsprintf(new_name, TEXT("%s"), (MailBox + SelBox)->Name);
		} else {
			wsprintf(new_name, TEXT("%s"), STR_MAILBOX_NONAME);
		}
		if (lstrcmp(old_name, new_name) != 0) {
			// update MailBox list and move/copy filters
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox, (LPARAM)new_name);
			SendDlgItemMessage(hWnd, IDC_COMBO, CB_SETCURSEL, SelBox, 0);
			for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
				if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->FilterCnt > 0) {
					for (j = 0; j < (MailBox+i)->FilterCnt; j++) {
						FILTER *tpFilter;
						tpFilter = *((MailBox+i)->tpFilter + j);
						if (tpFilter->Action == FILTER_COPY_INDEX || tpFilter->Action == FILTER_MOVE_INDEX) {
							if (lstrcmp(tpFilter->SaveboxName, old_name) == 0) {
								mem_free(&tpFilter->SaveboxName);
								tpFilter->SaveboxName = alloc_copy_t(new_name);
							}
						}
					}
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * SetMailBoxOption - アカウントの設定を行う
 */
BOOL SetMailBoxOption(HWND hWnd)
{
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[5];
	TCHAR old_name[BUF_SIZE], new_name[BUF_SIZE];

	tpOptionMailBox = (MailBox + SelBox);
	if (tpOptionMailBox->Name != NULL) {
		wsprintf(old_name, TEXT("%s"), tpOptionMailBox->Name);
	} else {
		old_name[0] = TEXT('\0');
	}

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = hInst;

	// POP
#if defined(_WIN32_WCE) && !defined(_WIN32_WCE_PPC) && !defined(_WIN32_WCE_LAGENDA)
	if (PPCFlag == FALSE) {
		psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_POP_HPC);
	} else {
		psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_POP);
	}
#else
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_POP);
#endif
	psp.pfnDlgProc = PopSetProc;
	hpsp[0] = CreatePropertySheetPage(&psp);

	// SMTP
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_SMTP);
	psp.pfnDlgProc = SmtpSetProc;
	hpsp[1] = CreatePropertySheetPage(&psp);

	//Signature
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_SIG);
	psp.pfnDlgProc = MakeSetProc;
	hpsp[2] = CreatePropertySheetPage(&psp);

	//Filter
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_FILTER);
	psp.pfnDlgProc = FilterSetProc;
	hpsp[3] = CreatePropertySheetPage(&psp);

	// RAS
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_RAS);
	psp.pfnDlgProc = RasSetProc;
	hpsp[4] = CreatePropertySheetPage(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof_PROPSHEETHEADER;
#ifdef _WIN32_WCE_PPC
	psh.dwFlags = PSH_MAXIMIZE | PSH_NOAPPLYNOW | PSH_USECALLBACK;
#else
	psh.dwFlags = PSH_NOAPPLYNOW;
#endif
	psh.hInstance = hInst;
	psh.hwndParent = hWnd;
	psh.pszCaption = STR_TITLE_SETMAILBOX;
	psh.nPages = sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
	psh.phpage = hpsp;
	psh.nStartPage = 0;
#ifdef _WIN32_WCE_PPC
	psh.pfnCallback = PropSheetCallback;
#endif

	PropRet = FALSE;
	PropertySheet(&psh);
	if (PropRet == FALSE) {
		return FALSE;
	}
	if (tpOptionMailBox->Name != NULL) {
		wsprintf(new_name, TEXT("%s"), tpOptionMailBox->Name);
	} else {
		wsprintf(new_name, TEXT("%s"), STR_MAILBOX_NONAME);
	}
	if (lstrcmp(old_name, new_name) != 0) {
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_DELETESTRING, SelBox, 0);
		SendDlgItemMessage(hWnd, IDC_COMBO, CB_INSERTSTRING, SelBox, (LPARAM)new_name);
	}
	mailbox_select(hWnd, SelBox);
	return TRUE;
}

/*
 * SetRecvOptionProc - 受信設定プロシージャ
 */
static BOOL CALLBACK SetRecvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SetDlgItemInt(hDlg, IDC_EDIT_READLINE, op.ListGetLine, FALSE);

		SendDlgItemMessage(hDlg, IDC_CHECK_LISTDOWNLOAD, BM_SETCHECK, op.ListDownload, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SHOWHEAD, BM_SETCHECK, op.ShowHeader, 0);

		switch (op.ListSaveMode) {
		case 0:
			SendDlgItemMessage(hDlg, IDC_RADIO_NOSAVE, BM_SETCHECK, 1, 0);
			break;
		case 1:
			SendDlgItemMessage(hDlg, IDC_RADIO_HEADSAVE, BM_SETCHECK, 1, 0);
			break;
		case 2:
			SendDlgItemMessage(hDlg, IDC_RADIO_ALLSAVE, BM_SETCHECK, 1, 0);
			break;
		}

		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_READLINE), !op.ListDownload);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_READLINE:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_CHECK_LISTDOWNLOAD:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_READLINE),
				!SendDlgItemMessage(hDlg, IDC_CHECK_LISTDOWNLOAD, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			op.ListGetLine = GetDlgItemInt(hDlg, IDC_EDIT_READLINE, NULL, FALSE);

			op.ListDownload = SendDlgItemMessage(hDlg, IDC_CHECK_LISTDOWNLOAD, BM_GETCHECK, 0, 0);
			op.ShowHeader = SendDlgItemMessage(hDlg, IDC_CHECK_SHOWHEAD, BM_GETCHECK, 0, 0);

			if (SendDlgItemMessage(hDlg, IDC_RADIO_NOSAVE, BM_GETCHECK, 0, 0) == 1) {
				op.ListSaveMode = 0;
			} else if (SendDlgItemMessage(hDlg, IDC_RADIO_HEADSAVE, BM_GETCHECK, 0, 0) == 1) {
				op.ListSaveMode = 1;
			} else if (SendDlgItemMessage(hDlg, IDC_RADIO_ALLSAVE, BM_GETCHECK, 0, 0) == 1) {
				op.ListSaveMode = 2;
			}
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetEncodeProc - エンコード設定プロシージャ
 */
BOOL CALLBACK SetEncodeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAILITEM *tpMailItem;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ENCODE);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);

#ifndef _WCE_OLD
		charset_enum(GetDlgItem(hDlg, IDC_COMBO_CHARSET_H));
		if (SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET_H, CB_GETCOUNT, 0, 0) <= 0) {
			SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET_H, CB_ADDSTRING, 0, (LPARAM)TEXT(CHARSET_ISO_8859_1));
			SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET_H, CB_ADDSTRING, 0, (LPARAM)TEXT(CHARSET_ISO_2022_JP));
		}
#else
		SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET_H, CB_ADDSTRING, 0, (LPARAM)TEXT(CHARSET_ISO_8859_1));
		EnableWindow(GetDlgItem(hDlg, IDC_COMBO_CHARSET_H), FALSE);
#endif
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE_H, CB_ADDSTRING, 0, (LPARAM)TEXT(ENCODE_BASE64));
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE_H, CB_ADDSTRING, 0, (LPARAM)TEXT(ENCODE_Q_PRINT));

#ifndef _WCE_OLD
		charset_enum(GetDlgItem(hDlg, IDC_COMBO_CHARSET));
		if (SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET, CB_GETCOUNT, 0, 0) <= 0) {
			SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET, CB_ADDSTRING, 0, (LPARAM)TEXT(CHARSET_ISO_8859_1));
			SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET, CB_ADDSTRING, 0, (LPARAM)TEXT(CHARSET_ISO_2022_JP));
		}
#else
		SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET, CB_ADDSTRING, 0, (LPARAM)TEXT(CHARSET_ISO_8859_1));
		EnableWindow(GetDlgItem(hDlg, IDC_COMBO_CHARSET), FALSE);
#endif
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_ADDSTRING, 0, (LPARAM)TEXT(ENCODE_7BIT));
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_ADDSTRING, 0, (LPARAM)TEXT(ENCODE_8BIT));
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_ADDSTRING, 0, (LPARAM)TEXT(ENCODE_BASE64));
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_ADDSTRING, 0, (LPARAM)TEXT(ENCODE_Q_PRINT));

		if (op.HeadCharset != NULL) {
			SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET_H, WM_SETTEXT, 0, (LPARAM)op.HeadCharset);
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE_H, CB_SETCURSEL, (op.HeadEncoding >= 2) ? op.HeadEncoding - 2 : 0, 0);
		if (op.BodyCharset != NULL) {
			SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET, WM_SETTEXT, 0, (LPARAM)op.BodyCharset);
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_SETCURSEL, op.BodyEncoding, 0);

		tpMailItem = (MAILITEM *)lParam;
		if (tpMailItem != NULL) {
			if (tpMailItem->HeadCharset != NULL) {
				SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET_H, WM_SETTEXT, 0, (LPARAM)tpMailItem->HeadCharset);
				SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE_H, CB_SETCURSEL,
					(tpMailItem->HeadEncoding >= 2) ? tpMailItem->HeadEncoding - 2 : 0, 0);
			}
			if (tpMailItem->BodyCharset != NULL) {
				SendDlgItemMessage(hDlg, IDC_COMBO_CHARSET, WM_SETTEXT, 0, (LPARAM)tpMailItem->BodyCharset);
				SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_SETCURSEL, tpMailItem->BodyEncoding, 0);
			}
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_COMBO_CHARSET_H:
		case IDC_COMBO_CHARSET:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif
		case IDOK:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				AllocGetText(GetDlgItem(hDlg, IDC_COMBO_CHARSET_H), &op.HeadCharset);
				if (op.HeadCharset != NULL && *op.HeadCharset == TEXT('\0')) {
					mem_free(&op.HeadCharset);
					op.HeadCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
				}
				op.HeadEncoding = SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE_H, CB_GETCURSEL, 0, 0) + 2;
				AllocGetText(GetDlgItem(hDlg, IDC_COMBO_CHARSET), &op.BodyCharset);
				if (op.BodyCharset != NULL && *op.BodyCharset == TEXT('\0')) {
					mem_free(&op.BodyCharset);
					op.BodyCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
				}
				op.BodyEncoding = SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_GETCURSEL, 0, 0);
			} else {
				AllocGetText(GetDlgItem(hDlg, IDC_COMBO_CHARSET_H), &tpMailItem->HeadCharset);
				if (tpMailItem->HeadCharset != NULL &&
					(*tpMailItem->HeadCharset == TEXT('\0') || lstrcmpi(tpMailItem->HeadCharset, op.HeadCharset) == 0)) {
					mem_free(&tpMailItem->HeadCharset);
				}
				tpMailItem->HeadEncoding = SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE_H, CB_GETCURSEL, 0, 0) + 2;
				AllocGetText(GetDlgItem(hDlg, IDC_COMBO_CHARSET), &tpMailItem->BodyCharset);
				if (tpMailItem->BodyCharset != NULL &&
					(*tpMailItem->BodyCharset == TEXT('\0') || lstrcmpi(tpMailItem->BodyCharset, op.BodyCharset) == 0)) {
					mem_free(&tpMailItem->BodyCharset);
				}
				tpMailItem->BodyEncoding = SendDlgItemMessage(hDlg, IDC_COMBO_ENCODE, CB_GETCURSEL, 0, 0);
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

/*
 * SetSendOptionProc - 送信設定プロシージャ
 */
static BOOL CALLBACK SetSendOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SetDlgItemInt(hDlg, IDC_EDIT_BREAKCNT, op.WordBreakSize, FALSE);
		SendDlgItemMessage(hDlg, IDC_CHECK_QBREAK, BM_SETCHECK, op.QuotationBreak, 0);

		SendDlgItemMessage(hDlg, IDC_CHECK_SENDMESSAGEID, BM_SETCHECK, op.SendMessageId, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SENDDATE, BM_SETCHECK, op.SendDate, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_ENCODETYPE, BM_SETCHECK, op.EncodeType, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SELECTSENDBOX, BM_SETCHECK, op.SelectSendBox, 0);

		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_BREAKCNT:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif
		case IDC_BUTTON_ENCODE:
			// エンコード設定
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ENCODE), hDlg, SetEncodeProc, (LPARAM)0);
			break;

		case IDOK:
			op.WordBreakSize = GetDlgItemInt(hDlg, IDC_EDIT_BREAKCNT, NULL, FALSE);
			op.QuotationBreak = SendDlgItemMessage(hDlg, IDC_CHECK_QBREAK, BM_GETCHECK, 0, 0);

			op.SendMessageId = SendDlgItemMessage(hDlg, IDC_CHECK_SENDMESSAGEID, BM_GETCHECK, 0, 0);
			op.SendDate = SendDlgItemMessage(hDlg, IDC_CHECK_SENDDATE, BM_GETCHECK, 0, 0);
			op.EncodeType = SendDlgItemMessage(hDlg, IDC_CHECK_ENCODETYPE, BM_GETCHECK, 0, 0);
			op.SelectSendBox = SendDlgItemMessage(hDlg, IDC_CHECK_SELECTSENDBOX, BM_GETCHECK, 0, 0);

			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetReplyOptionProc - Global options Reply callback
 */
static BOOL CALLBACK SetReplyOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SendDlgItemMessage(hDlg, IDC_CHECK_AUTOQUOT, BM_SETCHECK, op.AutoQuotation, 0);
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_SETCHECK, ViewClose, 0);
#endif
		if (op.QuotationChar != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_QUOTCHAR, WM_SETTEXT, 0, (LPARAM)op.QuotationChar);
		}
		SendDlgItemMessage(hDlg, IDC_EDIT_QUOTCHAR, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_SIGNRE_ABOVE, BM_SETCHECK, op.SignReplyAbove, 0);
		if (op.ReHeader != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_REHEAD, WM_SETTEXT, 0, (LPARAM)op.ReHeader);
		}
		break;

	case WM_NOTIFY:
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_SETCHECK, ViewClose, 0);
#endif
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_QUOTCHAR:
		case IDC_EDIT_REHEAD:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

#ifndef _WIN32_WCE
		case IDC_CHECK_VIEWCLOSE:
			ViewClose = SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_GETCHECK, 0, 0);
			break;
#endif

		case IDC_CHECK_AUTOQUOT:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_QUOTCHAR),
				SendDlgItemMessage(hDlg, IDC_CHECK_AUTOQUOT, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			op.AutoQuotation = SendDlgItemMessage(hDlg, IDC_CHECK_AUTOQUOT, BM_GETCHECK, 0, 0);
#ifndef _WIN32_WCE
			op.ViewClose = SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_GETCHECK, 0, 0);
#endif
			// 引用記号
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_QUOTCHAR), &op.QuotationChar);
			op.SignReplyAbove = SendDlgItemMessage(hDlg, IDC_SIGNRE_ABOVE, BM_GETCHECK, 0, 0);
			// 返信用ヘッダ
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_REHEAD), &op.ReHeader);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetFwdOptionProc - Global options Forward callback
 */
static BOOL CALLBACK SetForwardOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		if (op.FwdQuotation == 1) {
			SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_0, BM_SETCHECK, 1, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_1, BM_SETCHECK, 1, 0);
		}
		SendDlgItemMessage(hDlg, IDC_SIGNFWD, BM_SETCHECK, (op.SignForward==0) ? 0 : 1, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_SIGNFWD_ABOVE), (op.SignForward==0) ? FALSE : TRUE);
		SendDlgItemMessage(hDlg, IDC_SIGNFWD_ABOVE, BM_SETCHECK, (op.SignForward>=2) ? 1 : 0, 0);
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_SETCHECK, ViewClose, 0);
#endif
		if (op.FwdHeader != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_FWDHEAD, WM_SETTEXT, 0, (LPARAM)op.FwdHeader);
		}
		break;

	case WM_NOTIFY:
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_SETCHECK, ViewClose, 0);
#endif
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_FWDHEAD:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

#ifndef _WIN32_WCE
		case IDC_CHECK_VIEWCLOSE:
			ViewClose = SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_GETCHECK, 0, 0);
			break;
#endif

		case IDC_SIGNFWD:
			EnableWindow(GetDlgItem(hDlg, IDC_SIGNFWD_ABOVE), 
				SendDlgItemMessage(hDlg, IDC_SIGNFWD, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			op.FwdQuotation = SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_0, BM_GETCHECK, 0, 0);

			op.SignForward = SendDlgItemMessage(hDlg, IDC_SIGNFWD, BM_GETCHECK, 0, 0)
				+ 2 * SendDlgItemMessage(hDlg, IDC_SIGNFWD_ABOVE, BM_GETCHECK, 0, 0);

#ifndef _WIN32_WCE
			op.ViewClose = SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_GETCHECK, 0, 0);
#endif
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_FWDHEAD), &op.FwdHeader);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetCheckOptionProc - チェック設定プロシージャ
 */
static BOOL CALLBACK SetCheckOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef _WIN32_WCE
	TCHAR buf[BUF_SIZE];
#endif

	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SendDlgItemMessage(hDlg, IDC_CHECK_SHIWNEWMESSAGE, BM_SETCHECK, op.ShowNewMailMessage, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SHIWNOMESSAGE, BM_SETCHECK, op.ShowNoMailMessage, 0);

		SendDlgItemMessage(hDlg, IDC_CHECK_SOUND, BM_SETCHECK, op.NewMailSound, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_EXECSOUND, BM_SETCHECK, op.ExecEndSound, 0);

#ifndef _WIN32_WCE
		if (op.NewMailSoundFile != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_SOUND, WM_SETTEXT, 0, (LPARAM)op.NewMailSoundFile);
		}
		SendDlgItemMessage(hDlg, IDC_EDIT_SOUND, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendMessage(hDlg, WM_COMMAND, IDC_CHECK_SOUND, 0);
#endif

		SendDlgItemMessage(hDlg, IDC_CHECK_AUTOCHECK, BM_SETCHECK, op.AutoCheck, 0);
		SetDlgItemInt(hDlg, IDC_EDIT_AUTOCHECKTIME, op.AutoCheckTime, FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_AUTOCHECKTIME), op.AutoCheck);

		SendDlgItemMessage(hDlg, IDC_CHECK_STARTCHECK, BM_SETCHECK, op.StartCheck, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_CHECKAFTERUPDATE, BM_SETCHECK, op.CheckAfterUpdate, 0);
		
		SendDlgItemMessage(hDlg, IDC_CHECK_IGNOREERROR, BM_SETCHECK, op.SocIgnoreError, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SENDIGNOREERROR, BM_SETCHECK, op.SendIgnoreError, 0);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_AUTOCHECKTIME:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

#ifndef _WIN32_WCE
		case IDC_CHECK_SOUND:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_SOUND),
				SendDlgItemMessage(hDlg, IDC_CHECK_SOUND, BM_GETCHECK, 0, 0));
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SOUND_BROWS),
				SendDlgItemMessage(hDlg, IDC_CHECK_SOUND, BM_GETCHECK, 0, 0));
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SOUND_PLAY),
				SendDlgItemMessage(hDlg, IDC_CHECK_SOUND, BM_GETCHECK, 0, 0));
			break;

		case IDC_BUTTON_SOUND_BROWS:
			lstrcpy(buf, TEXT("*.wav"));
			if (filename_select(hDlg, buf, TEXT("wav"), STR_WAVE_FILTER, FILE_OPEN_SINGLE) == FALSE) {
				break;
			}
			SendDlgItemMessage(hDlg, IDC_EDIT_SOUND, WM_SETTEXT, 0, (LPARAM)buf);
			break;

		case IDC_BUTTON_SOUND_PLAY:
			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_EDIT_SOUND, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0') || sndPlaySound(buf, SND_ASYNC | SND_NODEFAULT) == FALSE) {
				MessageBeep(MB_ICONASTERISK);
			}
			break;
#endif

		case IDC_CHECK_AUTOCHECK:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_AUTOCHECKTIME),
				SendDlgItemMessage(hDlg, IDC_CHECK_AUTOCHECK, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			op.ShowNewMailMessage = SendDlgItemMessage(hDlg, IDC_CHECK_SHIWNEWMESSAGE, BM_GETCHECK, 0, 0);
			op.ShowNoMailMessage = SendDlgItemMessage(hDlg, IDC_CHECK_SHIWNOMESSAGE, BM_GETCHECK, 0, 0);

			op.NewMailSound = SendDlgItemMessage(hDlg, IDC_CHECK_SOUND, BM_GETCHECK, 0, 0);
			op.ExecEndSound = SendDlgItemMessage(hDlg, IDC_CHECK_EXECSOUND, BM_GETCHECK, 0, 0);
#ifndef _WIN32_WCE
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_SOUND), &op.NewMailSoundFile);
#endif

			op.AutoCheck = SendDlgItemMessage(hDlg, IDC_CHECK_AUTOCHECK, BM_GETCHECK, 0, 0);
			op.AutoCheckTime = GetDlgItemInt(hDlg, IDC_EDIT_AUTOCHECKTIME, NULL, FALSE);

			op.StartCheck = SendDlgItemMessage(hDlg, IDC_CHECK_STARTCHECK, BM_GETCHECK, 0, 0);
			op.CheckAfterUpdate = SendDlgItemMessage(hDlg, IDC_CHECK_CHECKAFTERUPDATE, BM_GETCHECK, 0, 0);

			op.SocIgnoreError = SendDlgItemMessage(hDlg, IDC_CHECK_IGNOREERROR, BM_GETCHECK, 0, 0);
			op.SendIgnoreError = SendDlgItemMessage(hDlg, IDC_CHECK_SENDIGNOREERROR, BM_GETCHECK, 0, 0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetRasOptionProc - RAS設定プロシージャ
 */
static BOOL CALLBACK SetRasOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SendDlgItemMessage(hDlg, IDC_CHECK_RASCON, BM_SETCHECK, op.RasCon, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_CHENDDISCON, BM_SETCHECK, op.RasCheckEndDisCon, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_ENDDISCON, BM_SETCHECK, op.RasEndDisCon, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_NORASNOCHECK, BM_SETCHECK, op.RasNoCheck, 0);

		SetDlgItemInt(hDlg, IDC_EDIT_WAIT, op.RasWaitSec, FALSE);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_WAIT:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			op.RasCon = SendDlgItemMessage(hDlg, IDC_CHECK_RASCON, BM_GETCHECK, 0, 0);
			op.RasCheckEndDisCon = SendDlgItemMessage(hDlg, IDC_CHECK_CHENDDISCON, BM_GETCHECK, 0, 0);
			op.RasEndDisCon = SendDlgItemMessage(hDlg, IDC_CHECK_ENDDISCON, BM_GETCHECK, 0, 0);
			op.RasNoCheck = SendDlgItemMessage(hDlg, IDC_CHECK_NORASNOCHECK, BM_GETCHECK, 0, 0);

			op.RasWaitSec = GetDlgItemInt(hDlg, IDC_EDIT_WAIT, NULL, FALSE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetSortOptionProc - callback for GlobalOptions/Sort property sheet
 */
static BOOL CALLBACK SetSortOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL retval = TRUE;
	BOOL enable;
	static BOOL redraw = FALSE;
	switch (uMsg) {
	case WM_INITDIALOG:
		redraw = FALSE;

		if (op.LvAutoSort == 0)  // Added PHH 11-Nov-2003
		{
			SendDlgItemMessage(hDlg, IDC_AUTOSORT1, BM_SETCHECK, 1, 0);
			EnableSortColumns(hDlg, FALSE);
		}
		else if (op.LvAutoSort == 1) 
		{
			SendDlgItemMessage(hDlg, IDC_AUTOSORT2, BM_SETCHECK, 1, 0);
			EnableSortColumns(hDlg, TRUE);
		}
		else if (op.LvAutoSort == 2) 
		{
			SendDlgItemMessage(hDlg, IDC_AUTOSORT3, BM_SETCHECK, 1, 0);
			EnableSortColumns(hDlg, TRUE);
		}
		if (op.LvSortItem < 0) 
		{
			SendDlgItemMessage(hDlg, IDC_CHECK_SORTORDER, BM_SETCHECK, 1, 0);
		}
		if (abs(op.LvSortItem) == 1) 
		{
			SendDlgItemMessage(hDlg, IDC_SORTITEM1, BM_SETCHECK, 1, 0);
		}
		if (abs(op.LvSortItem) == 2) 
		{
			SendDlgItemMessage(hDlg, IDC_SORTITEM2, BM_SETCHECK, 1, 0);
		}
		if (abs(op.LvSortItem) == 3) 
		{
			SendDlgItemMessage(hDlg, IDC_SORTITEM3, BM_SETCHECK, 1, 0);
		}
		if (abs(op.LvSortItem) == 4) 
		{
			SendDlgItemMessage(hDlg, IDC_SORTITEM4, BM_SETCHECK, 1, 0);
		}
		if (op.LvDefSelectPos == 0) {
			SendDlgItemMessage(hDlg, IDC_SORT_SELECT_FIRST, BM_SETCHECK, 1, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_SORT_SELECT_LAST, BM_SETCHECK, 1, 0);
		}

	case WM_NOTIFY:
		retval = OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDOK:
			if (SendDlgItemMessage(hDlg, IDC_AUTOSORT1, BM_GETCHECK, 0, 0) == 1)  // Added PHH 11-Nov-2003
			{
				op.LvAutoSort = 0;	
			}
			else if (SendDlgItemMessage(hDlg, IDC_AUTOSORT2, BM_GETCHECK, 0, 0) == 1) 
			{
				op.LvAutoSort = 1;
			}
			else if (SendDlgItemMessage(hDlg, IDC_AUTOSORT3, BM_GETCHECK, 0, 0) == 1) 
			{
				op.LvAutoSort = 2;
			}
			else 
			{
				op.LvAutoSort = 1;
			}
			
			if (SendDlgItemMessage(hDlg, IDC_SORTITEM1, BM_GETCHECK, 0, 0) == 1) 
			{
				op.LvSortItem = 1;
			}
			else if (SendDlgItemMessage(hDlg, IDC_SORTITEM2, BM_GETCHECK, 0, 0) == 1) 
			{
				op.LvSortItem = 2;
			}
			else if (SendDlgItemMessage(hDlg, IDC_SORTITEM3, BM_GETCHECK, 0, 0) == 1) 
			{
				op.LvSortItem = 3;
			}
			else if (SendDlgItemMessage(hDlg, IDC_SORTITEM4, BM_GETCHECK, 0, 0) == 1) 
			{
				op.LvSortItem = 4;
			}
			else 
			{
				op.LvSortItem = 2;
			}
			
			if (SendDlgItemMessage(hDlg, IDC_CHECK_SORTORDER, BM_GETCHECK, 0, 0) == 1) 
			{
				op.LvSortItem = -op.LvSortItem;
			}

			if (SendDlgItemMessage(hDlg, IDC_SORT_SELECT_FIRST, BM_GETCHECK, 0, 0) == 1) {
				op.LvDefSelectPos = 0;
			} else {
				op.LvDefSelectPos = 1;
			}

			//GJC redraw window if sort order changed
			if (redraw) {
				mailbox_select(MainWnd, SendDlgItemMessage(MainWnd, IDC_COMBO, CB_GETCURSEL, 0, 0));
			}

			break;

		case IDC_AUTOSORT1:
		case IDC_AUTOSORT2:
		case IDC_AUTOSORT3:
			redraw = TRUE;
			if (SendDlgItemMessage(hDlg, IDC_AUTOSORT1, BM_GETCHECK, 0, 0) == 1) {
				enable = FALSE;
			} else {
				enable = TRUE;
			}
			retval = EnableSortColumns(hDlg, enable);
			break;
		case IDC_SORTITEM1:
		case IDC_SORTITEM2:
		case IDC_SORTITEM3:
		case IDC_SORTITEM4:
		case IDC_CHECK_SORTORDER:
			redraw = TRUE;
			break;
		}
		break;

	default:
		retval = FALSE;
	}
	return retval;
}

/*
 * SetEtcOptionProc - その他の設定プロシージャ
 */
static BOOL CALLBACK SetEtcOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL retval = TRUE;
	static BOOL redraw = FALSE;
	switch (uMsg) {
	case WM_INITDIALOG:
		redraw = FALSE;
		/* コントロールの初期化 */
		SendDlgItemMessage(hDlg, IDC_CHECK_SHOWTRAYICON, BM_SETCHECK, op.ShowTrayIcon, 0);
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_MINSIZEHIDE, BM_SETCHECK, op.MinsizeHide, 0);
#endif
		SendDlgItemMessage(hDlg, IDC_CHECK_CLOSEHIDE, BM_SETCHECK, op.CloseHide, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_MOVEALLMAILBOX, BM_SETCHECK, op.MoveAllMailBox, 0);

		SendDlgItemMessage(hDlg, IDC_CHECK_PARANOID, BM_SETCHECK, op.ExpertMode, 0);		// Added PHH 4-Oct-2003

#ifdef _WIN32_WCE_PPC
		SendDlgItemMessage(hDlg, IDC_CHECK_REMEMBERDIR, BM_SETCHECK, op.RememberOSD, 0);
		///////////// MRP /////////////////////
		SendDlgItemMessage(hDlg, IDC_CHECK_USEPOOM, BM_SETCHECK, op.UsePOOMAddressBook, 0);
#else
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_USEPOOM), FALSE);  // Hide the options
		ShowWindow(GetDlgItem(hDlg, IDC_CHECK_USEPOOM), FALSE);
		///////////// --- /////////////////////
#endif

#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_STARTPASS, BM_SETCHECK, op.StartPass, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SHOWPASS, BM_SETCHECK, op.ShowPass, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0, (LPARAM)op.Password);
#endif

#ifndef _WIN32_WCE
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MINSIZEHIDE), op.ShowTrayIcon);
#endif
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CLOSEHIDE), op.ShowTrayIcon);

		SendDlgItemMessage(hDlg, IDC_DATE_FORMAT, WM_SETTEXT, 0, (LPARAM)op.DateFormat);
		SendDlgItemMessage(hDlg, IDC_TIME_FORMAT, WM_SETTEXT, 0, (LPARAM)op.TimeFormat);

		break;

	case WM_NOTIFY:
		retval = OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_DATE_FORMAT:
		case IDC_TIME_FORMAT:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif
		case IDC_CHECK_SHOWTRAYICON:
#ifndef _WIN32_WCE
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MINSIZEHIDE),
				SendDlgItemMessage(hDlg, IDC_CHECK_SHOWTRAYICON, BM_GETCHECK, 0, 0));
#endif
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CLOSEHIDE),
				SendDlgItemMessage(hDlg, IDC_CHECK_SHOWTRAYICON, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			op.ShowTrayIcon = SendDlgItemMessage(hDlg, IDC_CHECK_SHOWTRAYICON, BM_GETCHECK, 0, 0);
#ifndef _WIN32_WCE
			op.MinsizeHide = SendDlgItemMessage(hDlg, IDC_CHECK_MINSIZEHIDE, BM_GETCHECK, 0, 0);
#endif
			op.CloseHide = SendDlgItemMessage(hDlg, IDC_CHECK_CLOSEHIDE, BM_GETCHECK, 0, 0);
			op.MoveAllMailBox = SendDlgItemMessage(hDlg, IDC_CHECK_MOVEALLMAILBOX, BM_GETCHECK, 0, 0);
			op.ExpertMode = SendDlgItemMessage(hDlg, IDC_CHECK_PARANOID, BM_GETCHECK, 0, 0);	// Added PHH 4-Oct-2003

#ifdef _WIN32_WCE_PPC
			op.RememberOSD = SendDlgItemMessage(hDlg, IDC_CHECK_REMEMBERDIR, BM_GETCHECK, 0, 0);
			///////////// MRP /////////////////////
			op.UsePOOMAddressBook = SendDlgItemMessage(hDlg, IDC_CHECK_USEPOOM, BM_GETCHECK, 0, 0);
			///////////// --- /////////////////////
#endif

#ifndef _WIN32_WCE
			op.StartPass = SendDlgItemMessage(hDlg, IDC_CHECK_STARTPASS, BM_GETCHECK, 0, 0);
			op.ShowPass = SendDlgItemMessage(hDlg, IDC_CHECK_SHOWPASS, BM_GETCHECK, 0, 0);
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &op.Password);
#endif
			AllocGetText(GetDlgItem(hDlg, IDC_DATE_FORMAT), &op.DateFormat);
			AllocGetText(GetDlgItem(hDlg, IDC_TIME_FORMAT), &op.TimeFormat);

			break;
		}
		break;

	default:
		retval = FALSE;
	}
	return retval;
}

/*
 * SetOption - 設定ダイアログの表示
 */
BOOL SetOption(HWND hWnd)
{
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[8];

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = hInst;

	ViewClose = op.ViewClose;

	// 受信
#if defined(_WIN32_WCE) && !defined(_WIN32_WCE_PPC) && !defined(_WIN32_WCE_LAGENDA)
	if (PPCFlag == FALSE) {
		psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_RECV_HPC);
	} else {
		psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_RECV);
	}
#else
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_RECV);
#endif
	psp.pfnDlgProc = SetRecvOptionProc;
	hpsp[0] = CreatePropertySheetPage(&psp);

	//Transmission
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_SEND);
	psp.pfnDlgProc = SetSendOptionProc;
	hpsp[1] = CreatePropertySheetPage(&psp);

	//Reply
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_REPLY);
	psp.pfnDlgProc = SetReplyOptionProc;
	hpsp[2] = CreatePropertySheetPage(&psp);

	//Forward
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_FWD);
	psp.pfnDlgProc = SetForwardOptionProc;
	hpsp[3] = CreatePropertySheetPage(&psp);

	//Check
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_CHECK);
	psp.pfnDlgProc = SetCheckOptionProc;
	hpsp[4] = CreatePropertySheetPage(&psp);

	//Dial up
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_RAS);
	psp.pfnDlgProc = SetRasOptionProc;
	hpsp[5] = CreatePropertySheetPage(&psp);

	//Sort
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_SORT);
	psp.pfnDlgProc = SetSortOptionProc;
	hpsp[6] = CreatePropertySheetPage(&psp);

	//In addition
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_ETC);
	psp.pfnDlgProc = SetEtcOptionProc;
	hpsp[7] = CreatePropertySheetPage(&psp);

	ZeroMemory(&psh, sizeof(PROPSHEETHEADER));
	psh.dwSize = sizeof_PROPSHEETHEADER;
#ifdef _WIN32_WCE_PPC
	psh.dwFlags = PSH_MAXIMIZE | PSH_NOAPPLYNOW | PSH_USECALLBACK;
#else
	psh.dwFlags = PSH_NOAPPLYNOW;
#endif
	psh.hInstance = hInst;
	psh.hwndParent = hWnd;
	psh.pszCaption = STR_TITLE_OPTION;
	psh.nPages = sizeof(hpsp) / sizeof(HPROPSHEETPAGE);
	psh.phpage = hpsp;
	psh.nStartPage = 0;
#ifdef _WIN32_WCE_PPC
	psh.pfnCallback = PropSheetCallback;
#endif

	return PropertySheet(&psh);
}

/*
 * InputPassProc - パスワード入力プロシージャ
 */
BOOL CALLBACK InputPassProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND PassWnd;
	TCHAR *buf;

	switch (uMsg) {
	case WM_INITDIALOG:
		SetControlFont(hDlg);
		if (PassWnd != NULL) {
			_SetForegroundWindow(PassWnd);
			EndDialog(hDlg, FALSE);
			break;
		}
		PassWnd = hDlg;

#ifndef _WIN32_WCE
		if (GetParent(hDlg) == NULL || IsIconic(GetParent(hDlg)) != 0) {
			RECT rect;
			GetWindowRect(hDlg, &rect);
			SetWindowPos(hDlg, HWND_TOPMOST,
				(GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2),
				(GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2),
				0, 0, SWP_NOSIZE);
			_SetForegroundWindow(hDlg);
		}
#endif
		mem_free(&g_Pass);
		g_Pass = NULL;

		if (gPassSt == 0) {
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_TMPPASS), SW_HIDE);
		} else {
			SendDlgItemMessage(hDlg, IDC_CHECK_TMPPASS, BM_SETCHECK, 1, 0);
		}

		buf = (TCHAR *)mem_alloc(
			sizeof(TCHAR) * (lstrlen(WINDOW_TITLE TEXT(" - [")) + lstrlen((TCHAR *)lParam) + 2));
		if (buf != NULL) {
			str_join_t(buf, WINDOW_TITLE TEXT(" - ["), (TCHAR *)lParam, TEXT("]"), (TCHAR *)-1);
			SetWindowText(hDlg, buf);
			mem_free(&buf);
		}
		break;

	case WM_CLOSE:
		PassWnd = NULL;
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_PASS:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			gPassSt = SendDlgItemMessage(hDlg, IDC_CHECK_TMPPASS, BM_GETCHECK, 0, 0);
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &g_Pass);
			PassWnd = NULL;
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			PassWnd = NULL;
			EndDialog(hDlg, FALSE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * InitMailBoxProc - 新着取得位置の設定
 */
BOOL CALLBACK InitMailBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAILBOX *SelMailBox;
	TCHAR buf[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
	TCHAR *p;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_INITMAILBOX);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		SendDlgItemMessage(hDlg, IDC_RADIO_FIRSTGET, BM_SETCHECK, 1, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GETNUM), 0);
		SetDlgItemInt(hDlg, IDC_EDIT_GETNUM, 1, FALSE);

		SelMailBox = (MAILBOX *)lParam;

		//Mail several
		wsprintf(buf, STR_STATUS_INIT_MAILCNT, SelMailBox->MailCnt);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MAILCNT), buf);

		//Mail size
		wsprintf(buf, TEXT("%d"), (SelMailBox->MailSize < 1024) ? SelMailBox->MailSize : SelMailBox->MailSize / 1024);
		if (GetNumberFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, buf, NULL, tmp, BUF_SIZE - 1) != 0) {
			for (p = tmp; *p != TEXT('\0') && *p != TEXT('.'); p++);
			*p = TEXT('\0');
			if (SelMailBox->MailSize < 1024) {
				wsprintf(buf, STR_STATUS_INIT_MAILSIZE_B, tmp);
			} else {
				wsprintf(buf, STR_STATUS_INIT_MAILSIZE_KB, tmp);
			}
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MAILSIZE), buf);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_GETNUM:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_RADIO_FIRSTGET:
		case IDC_RADIO_SETGET:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GETNUM),
				!SendDlgItemMessage(hDlg, IDC_RADIO_FIRSTGET, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			SelMailBox = (MAILBOX *)GetWindowLong(hDlg, GWL_USERDATA);
			if (SendDlgItemMessage(hDlg, IDC_RADIO_FIRSTGET, BM_GETCHECK, 0, 0) == 1) {
				SelMailBox->LastNo = -1;
			} else {
				SelMailBox->LastNo = GetDlgItemInt(hDlg, IDC_EDIT_GETNUM, NULL, FALSE);
				if (SelMailBox->LastNo < 1) {
					SelMailBox->LastNo = -1;
				}
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

/*
 * SetCcList - リストビューにCc , Bccのリストを表示する
 */
static void SetCcList(HWND hDlg, TCHAR *strList, TCHAR *type)
{
	TCHAR *buf;
	TCHAR *p ,*r, *s;
	int ItemIndex;

	if (strList == NULL || *strList == TEXT('\0')) {
		return;
	}

	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(strList) + 1));
	if (buf == NULL) {
		return;
	}
	*buf = TEXT('\0');

	p = strList;
	while (*p != TEXT('\0')) {
		//Mail address extraction
		r = GetMailString(p, buf);

		//Excessive blank removal
		for (; *p == TEXT(' ') || *p == TEXT('\t') || *p == TEXT('\r') || *p == TEXT('\n'); p++);

		//Is added to the list the extraction
		for (s = buf; p < r; p++) {
			if (*p == TEXT('\t')) {
				*(s++) = TEXT(' ');
			} else if (*p != TEXT('\r') && *p != TEXT('\n')) {
				*(s++) = *p;
			}
		}
		*s = TEXT('\0');
		if (*buf != TEXT('\0')) {
			//in the character string which Excessive blank removal
			if (*(s - 1) == TEXT(' ')) {
				for (s--; s > buf && *s == TEXT(' '); s--);
				*(s + 1) = TEXT('\0');
			}
			//It adds to the list the
			ItemIndex = ListView_AddOptionItem(GetDlgItem(hDlg, IDC_LIST_CC), type);
			ListView_SetItemText(GetDlgItem(hDlg, IDC_LIST_CC), ItemIndex, 1, buf);
		}
		p = (*r != TEXT('\0')) ? r + 1 : r;
	}
	mem_free(&buf);
}

/*
 * CcListProc - Cc , Bcc 設定プロシージャ
 */
static BOOL CALLBACK CcListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define LV_TITLE_CC		TEXT("Cc:")
#define LV_TITLE_BCC	TEXT("Bcc:")
	MAILITEM *tpMailItem;
	ADDRESSBOOK *tpTmpAddressBook;
	HWND hListView;
	TCHAR buf[BUF_SIZE], buf2[BUF_SIZE];
	TCHAR *p;
	int SelectItem;
	int CcLen, BccLen;
	int i, cnt;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_CCBCC);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		if (lParam == 0) {
			EndDialog(hDlg, FALSE);
			break;
		}
		tpMailItem = (MAILITEM *)lParam;
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		hListView = GetDlgItem(hDlg, IDC_LIST_CC);
#ifdef _WIN32_WCE
		ListView_AddColumn(hListView, LVCFMT_LEFT, 40, STR_CCLIST_TYPE, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 170, STR_CCLIST_MAILADDRESS, 1);
#else
		ListView_AddColumn(hListView, LVCFMT_LEFT, 50, STR_CCLIST_TYPE, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 350, STR_CCLIST_MAILADDRESS, 1);
#endif
		ListView_SetExtendedListViewStyle(hListView,
			LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
		SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_RADIO_CC, BM_SETCHECK, 1, 0);

		if (tpMailItem->Mark == ICON_SENTMAIL) {
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_CC), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_BCC), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TO), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 0);
		}

		// Cc
		SetCcList(hDlg, tpMailItem->Cc, LV_TITLE_CC);
		// Bcc
		SetCcList(hDlg, tpMailItem->Bcc, LV_TITLE_BCC);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_NOTIFY:
		DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_CC));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_MAILADDRESS:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_BUTTON_TO:
			i = -1;
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS));
			// GJC make temporary addressbook for possible editing
			tpTmpAddressBook = addressbook_copy();
			if (tpTmpAddressBook != NULL) {
				tpTmpAddressBook->GetAddrList = TRUE;
				if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS),
					hDlg, AddressListProc, (LPARAM)tpTmpAddressBook) == TRUE) {
					SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)tpTmpAddressBook->AddrList);
				}
				addressbook_free(tpTmpAddressBook);
			}
			break;

		case ID_LV_EDIT:
			if ((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_CC), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			*buf = TEXT('\0');
			ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_CC), SelectItem, 0, buf, BUF_SIZE - 1);
			if (lstrcmpi(buf, LV_TITLE_BCC) == 0) {
				SendDlgItemMessage(hDlg, IDC_RADIO_CC, BM_SETCHECK, 0, 0);
				SendDlgItemMessage(hDlg, IDC_RADIO_BCC, BM_SETCHECK, 1, 0);
			} else {
				SendDlgItemMessage(hDlg, IDC_RADIO_CC, BM_SETCHECK, 1, 0);
				SendDlgItemMessage(hDlg, IDC_RADIO_BCC, BM_SETCHECK, 0, 0);
			}
			*buf = TEXT('\0');
			ListView_GetItemText(GetDlgItem(hDlg, IDC_LIST_CC), SelectItem, 1, buf, BUF_SIZE - 1);
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)buf);

			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_CC), SelectItem);
			break;

		case IDC_BUTTON_DELETE:
		case ID_LV_DELETE:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			if (tpMailItem->Mark == ICON_SENTMAIL) {
				break;
			}
			if ((SelectItem = ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_CC), -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (MessageBox(hDlg, STR_Q_DELETE, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			ListView_DeleteItem(GetDlgItem(hDlg, IDC_LIST_CC), SelectItem);
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_CC), SelectItem,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			break;

		case IDC_BUTTON_ADD:
			if (CheckDependence(hDlg, IDC_EDIT_MAILADDRESS) == FALSE) {
				break;
			}

			p = NULL;
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), &p);
			if (p == NULL) {
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}
			if (*p == TEXT('\0')) {
				ErrorMessage(hDlg, STR_ERR_SETMAILADDR);
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS));
				break;
			}
			if (SendDlgItemMessage(hDlg, IDC_RADIO_BCC, BM_GETCHECK, 0, 0) == 1) {
				SetCcList(hDlg, p, LV_TITLE_BCC);
			} else {
				SetCcList(hDlg, p, LV_TITLE_CC);
			}
			mem_free(&p);

			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)TEXT(""));
//			SendDlgItemMessage(hDlg, IDC_RADIO_CC, BM_SETCHECK, 1, 0);
//			SendDlgItemMessage(hDlg, IDC_RADIO_BCC, BM_SETCHECK, 0, 0);
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS));
			break;

		case IDOK:
			CcLen = BccLen = 0;

			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			if (tpMailItem->Mark == ICON_SENTMAIL) {
				EndDialog(hDlg, TRUE);
				break;
			}

			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf != TEXT('\0')) {
				SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_ADD, 0);
			}

			mem_free(&tpMailItem->Cc);
			tpMailItem->Cc = NULL;

			mem_free(&tpMailItem->Bcc);
			tpMailItem->Bcc = NULL;

			hListView = GetDlgItem(hDlg, IDC_LIST_CC);
			cnt = ListView_GetItemCount(hListView);
			for (i = 0; i < cnt; i++) {
				*buf2 = TEXT('\0');
				ListView_GetItemText(hListView, i, 1, buf2, BUF_SIZE - 1);
				*buf = TEXT('\0');
				ListView_GetItemText(hListView, i, 0, buf, BUF_SIZE - 1);
				if (lstrcmpi(buf, LV_TITLE_BCC) == 0) {
					BccLen += lstrlen(buf2) + 4;
				} else {
					CcLen += lstrlen(buf2) + 4;
				}
			}

			if (CcLen > 0) {
				tpMailItem->Cc = (TCHAR *)mem_alloc(sizeof(TCHAR) * (CcLen + 1));
				if (tpMailItem->Cc != NULL) {
					*tpMailItem->Cc = TEXT('\0');
				}
			}
			if (BccLen > 0) {
				tpMailItem->Bcc = (TCHAR *)mem_alloc(sizeof(TCHAR) * (BccLen + 1));
				if (tpMailItem->Bcc != NULL) {
					*tpMailItem->Bcc = TEXT('\0');
				}
			}

			for (i = 0; i < cnt; i++) {
				*buf2 = TEXT('\0');
				ListView_GetItemText(hListView, i, 1, buf2, BUF_SIZE - 1);
				*buf = TEXT('\0');
				ListView_GetItemText(hListView, i, 0, buf, BUF_SIZE - 1);
				if (lstrcmpi(buf, LV_TITLE_BCC) == 0) {
					if (tpMailItem->Bcc == NULL) continue;
					if (*tpMailItem->Bcc != TEXT('\0')) {
						lstrcat(tpMailItem->Bcc, TEXT(",\r\n "));
					}
					lstrcat(tpMailItem->Bcc, buf2);
				} else {
					if (tpMailItem->Cc == NULL) continue;
					if (*tpMailItem->Cc != TEXT('\0')) {
						lstrcat(tpMailItem->Cc, TEXT(",\r\n "));
					}
					lstrcat(tpMailItem->Cc, buf2);
				}
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

/*
 * SetAttachProc - 添付ファイルの設定プロシージャ
 */
BOOL CALLBACK SetAttachProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define ID_FILE_ADD				(WM_APP + 100)
	MAILITEM *tpMailItem;
	MULTIPART **tpMultiPart = NULL;
	TCHAR fpath[MULTI_BUF_SIZE], buf[BUF_SIZE];
	TCHAR *f, *f1, *f2;
	int i, j, len, cnt, cnt1, cnt2, mpcnt;
	unsigned len1, len2;
	long FileSize;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ATTACH);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);
		if (lParam == 0) {
			EndDialog(hDlg, FALSE);
			break;
		}
		tpMailItem = (MAILITEM *)lParam;
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
#ifndef _WIN32_WCE
		SetWindowLong(hDlg, GWL_EXSTYLE, WS_EX_ACCEPTFILES);
#endif
		if (tpMailItem->Attach != NULL) {
			f = tpMailItem->Attach;
			while (*f != TEXT('\0')) {
				f = str_cpy_f_t(fpath, f, ATTACH_SEP);
				SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)fpath);
			}
		}
		// GJC original attachments of forwarded message
		if (tpMailItem->FwdAttach != NULL) {
			f = tpMailItem->FwdAttach;
			while (*f != TEXT('\0')) {
				f = str_cpy_f_t(buf, f, ATTACH_SEP);
				//fpath[0] = '\0';
				str_join_t(fpath, STR_FWDATT_PREFIX, buf, (TCHAR *)-1);
				SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)fpath);
			}
		}
		if (tmp_attach != NULL) {
			f = tmp_attach;
			while (*f != TEXT('\0')) {
				f = str_cpy_f_t(fpath, f, ATTACH_SEP);
				SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)fpath);
			}
		}
		SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_SETHORIZONTALEXTENT,
			MAX_PATH * 9 + GetSystemMetrics(SM_CXVSCROLL), 0);
		if (tpMailItem->Mark == ICON_SENTMAIL) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

#ifndef _WIN32_WCE
	case WM_DROPFILES:
		tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
		if (tpMailItem == NULL) {
			EndDialog(hDlg, FALSE);
			break;
		}
		if (tpMailItem->Mark == ICON_SENTMAIL) {
			break;
		}
		len = DragQueryFile((HANDLE)wParam, 0xFFFFFFFF, NULL, 0);
		for (i = 0; i < len; i++) {
			DragQueryFile((HANDLE)wParam, i, fpath, BUF_SIZE - 1);
			SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)fpath);
		}
		DragFinish((HANDLE)wParam);
		break;
#endif

	case WM_VKEYTOITEM:
		if (LOWORD(wParam) == VK_DELETE) {
			SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_DELETE, 0);
			return -2;
		}
		return -1;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_ADD:
			lstrcpy(fpath, TEXT("*.*"));
			if (filename_select(hDlg, fpath, NULL, NULL, FILE_OPEN_MULTI) == FALSE) {
				break;
			}
			// GJC multiselect - return value is either
			// \\path\\file\0 or
			// \\path\0file1\0file2\0...filen\0\0
			f1 = str_cpy_f_t(buf, fpath, TEXT('\0'));
#ifdef _WIN32_WCE_PPC
			if (*buf != TEXT('\0') && dir_check(buf) == FALSE) {
#else
			if (dir_check(buf) == FALSE) {
#endif
				SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)buf);
			} else {
				len1 = (unsigned)lstrlen(buf);
				*(buf + len1) = TEXT('\\');
				len1++;
				while (*f1 != TEXT('\0')) {
					f1 = str_cpy_f_t((buf+len1), f1, TEXT('\0'));
					SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)buf);
				}
			}
			break;

		case ID_FILE_ADD:
			if (dir_check((TCHAR *)lParam) == TRUE) {
				break;
			}
			for (i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0); i++) {
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETTEXT, i, (LPARAM)buf);
				if (lstrcmp((TCHAR *)lParam, buf) == 0) {
					SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_DELETESTRING, i, 0);
					break;
				}
			}
			SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_ADDSTRING, 0, lParam);
			break;

		case IDC_BUTTON_DELETE:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			if (tpMailItem->Mark == ICON_SENTMAIL) {
				break;
			}
			if (SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETSELCOUNT, 0, 0) <= 0) {
				break;
			}
			if (MessageBox(hDlg, STR_Q_UNLINKATTACH, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			for (i = SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0) - 1; i >= 0; i--) {
				if (SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETSEL, i, 0) == 0) {
					continue;
				}
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_DELETESTRING, i, 0);
			}
			break;

		case IDOK:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			if (tpMailItem->Mark == ICON_SENTMAIL) {
				EndDialog(hDlg, TRUE);
				break;
			}
			mem_free(&tpMailItem->Attach);
			tpMailItem->Attach = NULL;
			mem_free(&tpMailItem->FwdAttach);
			tpMailItem->FwdAttach = NULL;
			tpMailItem->AttachSize = 0;

			if (SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0) <= 0) {
				EndDialog(hDlg, TRUE);
				break;
			}

			len = lstrlen(STR_FWDATT_PREFIX);
			len1 = len2 = 0;
			cnt = SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0);
			cnt1 = cnt2 = 0;
			for (i = 0; i < cnt; i++) {
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETTEXT, i, (LPARAM)fpath);
				if (str_cmp_n_t(fpath, STR_FWDATT_PREFIX, len) != 0) {
					len1 += SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETTEXTLEN, i, 0);
					if (cnt1 != 0) {
						len1++;
					}
					cnt1++;
				} else {
					len2 += SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETTEXTLEN, i, 0);
					len2 -= len;
					if (cnt2 != 0) {
						len2++;
					}
					cnt2++;
				}
			}
			if (cnt1 > 0) {
				f1 = tpMailItem->Attach    = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len1 + 1));
				if (f1 == NULL) {
					EndDialog(hDlg, FALSE);
					break;
				}
			}
			if (cnt2 > 0) {
				MAILITEM *tpFwdMailItem = NULL;
				TCHAR *body;
				i = mailbox_name_to_index(tpMailItem->MailBox);
				if (i != -1) {
					j = item_find_thread(MailBox + i, tpMailItem->References, (MailBox+i)->MailItemCnt);
					if ( j!= -1) {
						tpFwdMailItem = (*((MailBox + i)->tpMailItem + j));
					}
				}
				if (tpFwdMailItem == NULL) {
					tpFwdMailItem = item_find_thread_anywhere(tpMailItem->References);
				}
				f2 = tpMailItem->FwdAttach = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len2 + 1));
				if (f2 == NULL || tpFwdMailItem == NULL) {
					if (f1 != NULL) mem_free(&f1);
					if (f2 != NULL) mem_free(&f2);
					tpMailItem->Attach = NULL;
					tpMailItem->FwdAttach = NULL;
					EndDialog(hDlg, FALSE);
					break;
				}
				body = MIME_body_decode(tpFwdMailItem, FALSE, &tpMultiPart, &mpcnt, &i);
				mem_free(&body);
			}
			cnt1 = cnt2 = 0;
			for (i = 0; i < cnt; i++) {
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETTEXT, i, (LPARAM)fpath);
				if (str_cmp_n_t(fpath, STR_FWDATT_PREFIX, len) != 0) {
					if (cnt1 != 0) {
						*(f1++) = ATTACH_SEP;
					}
					f1 = str_cpy_t(f1, fpath);
					FileSize = file_get_size(fpath);
					if (FileSize > 0) {
						tpMailItem->AttachSize += (FileSize*4)/3; // 4/3 for MIME encoding
					}
					cnt1++;
				} else {
					if (cnt2 != 0) {
						*(f2++) = ATTACH_SEP;
					}
					f2 = str_cpy_t(f2, fpath+len);
					FileSize = 0;
					for (j = 0; j < mpcnt; j++) {
						TCHAR *fname;
						if ((*(tpMultiPart + j))->Forwardable == FALSE) {
							continue;
						}
#ifdef UNICODE
						fname = alloc_char_to_tchar((*(tpMultiPart + j))->Filename);
#else
						fname = (*(tpMultiPart + j))->Filename;
#endif
						if (lstrcmp(fname, fpath+len) == 0) {
							FileSize = (*(tpMultiPart + j))->ePos - (*(tpMultiPart + j))->hPos;
							if ((*(tpMultiPart + j))->Encoding != NULL) {
								FileSize = (FileSize*4) / 3;
							}
#ifdef UNICODE
							mem_free(&fname);
#endif
							break;
						}
#ifdef UNICODE
						mem_free(&fname);
#endif
					}
					tpMailItem->AttachSize += FileSize;
					cnt2++;
				}
			}
			if (tpMultiPart != NULL) {
				multipart_free(&tpMultiPart, mpcnt);
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

/*
 * CheckDependence - EDITに機種依存文字が含まれていないかチェックする
 */
BOOL CheckDependence(HWND hWnd, int Ctl)
{
	TCHAR *buf = NULL;
	int i;
	BOOL ret = TRUE;

	AllocGetText(GetDlgItem(hWnd, Ctl), &buf);
	if (buf == NULL) {
		return TRUE;
	}
	if ((i = IsDependenceString(buf)) != -1 &&
		MessageBox(hWnd, STR_Q_DEPENDENCE,
		WINDOW_TITLE, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
		SetFocus(GetDlgItem(hWnd, Ctl));
#ifdef UNICODE
		SendDlgItemMessage(hWnd, Ctl, EM_SETSEL, i, i + 1);
#else
		SendDlgItemMessage(hWnd, Ctl, EM_SETSEL, i, i + 2);
#endif
		SendDlgItemMessage(hWnd, Ctl, EM_SCROLLCARET, 0, 0);
		ret = FALSE;
	}
	mem_free(&buf);
	return ret;
}

/*
 * SetButtonText - Buttonのテキストを設定する
 */
static void SetButtonText(HWND hButton, TCHAR *title, BOOL UseFlag)
{
	TCHAR buf[BUF_SIZE], *p;

	if (UseFlag == FALSE) {
		p = title;
	} else {
		wsprintf(buf, TEXT("%s *"), title);
		p = buf;
	}
	SetWindowText(hButton, p);
}

/*
 * SetSendProc - 送信設定プロシージャ
 */
BOOL CALLBACK SetSendProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAILITEM **tpSendMailIList;
	MAILITEM *tpMailItem;
	MAILITEM *tpTmpMailItem;
	ADDRESSBOOK *tpTmpAddressBook;
	TCHAR *p;
	TCHAR buf[BUF_SIZE];
	TCHAR *mb_replyto;
	int len;
	int i, j, st, mb, cnt, sel;
	BOOL BtnFlag, found;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_SENDINFO);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
#ifndef _WIN32_WCE
		if (GetParent(hDlg) == NULL || IsIconic(GetParent(hDlg)) != 0) {
			RECT rect;
			GetWindowRect(hDlg, &rect);
			SetWindowPos(hDlg, 0,
				(GetSystemMetrics(SM_CXSCREEN) / 2) - ((rect.right - rect.left) / 2),
				(GetSystemMetrics(SM_CYSCREEN) / 2) - ((rect.bottom - rect.top) / 2),
				0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
#endif
		SetControlFont(hDlg);
		_SetForegroundWindow(hDlg);

		if (lParam == 0) {
			EndDialog(hDlg, FALSE);
			break;
		}
		tpSendMailIList = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * 2);
		if (tpSendMailIList == NULL) {
			ErrorMessage(hDlg, STR_ERR_MEMALLOC);
			EndDialog(hDlg, FALSE);
			break;
		}
		SetWindowLong(hDlg, GWL_USERDATA, (long)tpSendMailIList);
		*tpSendMailIList = tpMailItem = (MAILITEM *)lParam;

		SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_SETEXTENDEDUI, TRUE, 0);
		cnt = 0;
		mb = -1;
		mb_replyto = NULL;
		/* of control Initialization */
		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			if ((MailBox + i)->Type == MAILBOX_TYPE_SAVE) {
				continue;
			}
			if ((mb == -1 && SelBox == i) || (tpMailItem->MailBox != NULL && mailbox_name_to_index(tpMailItem->MailBox) == i)) {
				mb = cnt;
				mb_replyto = (MailBox + i)->ReplyTo;
			}
			if ((MailBox + i)->Name == NULL || *(MailBox + i)->Name == TEXT('\0')) {
				SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
			} else {
				SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_ADDSTRING, 0, (LPARAM)(MailBox + i)->Name);
			}
			cnt++;
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_SETCURSEL, (mb>=0) ? mb : 0, 0);

		// GJC ReplyTo options: global replyto and replyto/address for each mailbox
		SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_SETEXTENDEDUI, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_SETHORIZONTALEXTENT, (WPARAM)100, 0);
		cnt = 0;
		sel = -1;
		SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_ADDSTRING, 0, (LPARAM)STR_OMIT_REPLYTO);
		if (tpMailItem->ReplyTo != NULL && *tpMailItem->ReplyTo != TEXT('\0')) {
			SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_ADDSTRING, 0, (LPARAM)tpMailItem->ReplyTo);
			sel = cnt = 1;
		}
		// global replyto
		if (op.AltReplyTo != NULL && *op.AltReplyTo != TEXT('\0')) {
			p = op.AltReplyTo;
			while (*p != TEXT('\0')) {
				p = str_cpy_f_t(buf, p, TEXT(','));
				// check that this address isn't a duplicate of one from an account
				found = FALSE;
				for (j = 1; j < MailBoxCnt; j++) {
					if ((MailBox + i)->Type == MAILBOX_TYPE_SAVE) {
						continue;
					}
					if ((MailBox + j)->ReplyTo != NULL && *(MailBox + j)->ReplyTo != TEXT('\0'))  {
						if (lstrcmp((MailBox + j)->ReplyTo, buf) == 0) {
							found = TRUE;
							break;
						}
					} else if ((MailBox + j)->MailAddress != NULL && *(MailBox + j)->MailAddress != TEXT('\0')) {
						if (lstrcmp((MailBox + j)->MailAddress, buf) == 0) {
							found = TRUE;
							break;
						}
					}
				}
				if (found == FALSE) {
					SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_ADDSTRING, 0, (LPARAM)buf);
					cnt++;
				}
			}
		}
		// replyto/address for user mailboxes
		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			TCHAR *addr_to_match = NULL;
			if ((MailBox + i)->Type == MAILBOX_TYPE_SAVE) {
				continue;
			}
			if ((MailBox + i)->ReplyTo != NULL && *(MailBox + i)->ReplyTo != TEXT('\0')) {
				addr_to_match = (MailBox + i)->ReplyTo;
			} else if ((MailBox + i)->MailAddress != NULL && *(MailBox + i)->MailAddress != TEXT('\0')) {
				addr_to_match = (MailBox + i)->MailAddress;
			}
			// check that this address isn't a duplicate of one from a later account
			if (addr_to_match != NULL) {
				found = FALSE;
				for (j = i+1; j < MailBoxCnt; j++) {
					if ((MailBox + i)->Type == MAILBOX_TYPE_SAVE) {
						continue;
					}
					if ((MailBox + j)->ReplyTo != NULL && *(MailBox + j)->ReplyTo != TEXT('\0'))  {
						if (lstrcmp((MailBox + j)->ReplyTo, addr_to_match) == 0) {
							found = TRUE;
							break;
						}
					} else if ((MailBox + j)->MailAddress != NULL && *(MailBox + j)->MailAddress != TEXT('\0')) {
						if (lstrcmp((MailBox + j)->MailAddress, addr_to_match) == 0) {
							found = TRUE;
							break;
						}
					}
				}
				if (found == FALSE) {
					SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_ADDSTRING, 0, (LPARAM)addr_to_match);
					cnt++;
					// check if this address is the ReplyTo for the selected account
					if (sel == -1 && mb_replyto != NULL && *mb_replyto != TEXT('\0')
						&& lstrcmp(mb_replyto, addr_to_match) == 0) {
						sel = cnt;
					}
				}
			}
		}
		sel = (sel < 0) ? 0 : sel;
		SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_SETCURSEL, sel, 0);

		/////////////////////// MRP //////////////////////
		SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_SETEXTENDEDUI, TRUE, 0);
		SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)HIGH_PRIORITY);
		SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)NORMAL_PRIORITY);
		SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_ADDSTRING, 0, (LPARAM)LOW_PRIORITY);

		SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_SETCURSEL, 1, 0);  // Normal or default

		if (tpMailItem->Priority == 1) {// High
			SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_SETCURSEL, 0, 0);
		} else if (tpMailItem->Priority == 5) {// Low
			SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_SETCURSEL, 2, 0);
		}

		SendDlgItemMessage(hDlg, IDC_DEL, BM_SETCHECK, tpMailItem->DeliveryReceipt, 0);
		SendDlgItemMessage(hDlg, IDC_READ, BM_SETCHECK, tpMailItem->ReadReceipt, 0);
		//////////////////// ______  //////////////////////////

		if (tpMailItem->To != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_TO, WM_SETTEXT, 0, (LPARAM)tpMailItem->To);
		}

		if (tpMailItem->Subject != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_SETTEXT, 0, (LPARAM)tpMailItem->Subject);
		}

		if ((tpMailItem->Cc != NULL && *tpMailItem->Cc != TEXT('\0')) ||
			(tpMailItem->Bcc != NULL && *tpMailItem->Bcc != TEXT('\0'))) {
			SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_CC), STR_SETSEND_BTN_CC, TRUE);
		}
		if ( (tpMailItem->Attach != NULL && *tpMailItem->Attach != TEXT('\0')) 
			|| (tpMailItem->FwdAttach != NULL && *tpMailItem->FwdAttach != TEXT('\0')) ) {
			SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ATTACH), STR_SETSEND_BTN_ATTACH, TRUE);
		}
		//of list of file name Quotation
		if (tpMailItem->Mark == 1 || tpMailItem->Mark == 3) {
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOT_3ST), SW_HIDE);
			SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_SETCHECK, (tpMailItem->Mark == 1) ? op.AutoQuotation : 1, 0);
		} else if (tpMailItem->Mark == 2) {
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOTATION), SW_HIDE);
			SendDlgItemMessage(hDlg, IDC_CHECK_QUOT_3ST, BM_SETCHECK, BST_INDETERMINATE, 0);
		} else {
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOT_3ST), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOTATION), SW_HIDE);
		}
		if (tpMailItem->Mark == ICON_SENTMAIL) {
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_SMTP), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_REPLYTO), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_PRIORITY), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_DEL), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_READ), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_TO), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_TITLE), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_QUOTATION), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_QUOT_3ST), 0);
		}

		*(tpSendMailIList + 1) = tpTmpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
		if (tpTmpMailItem == NULL) {
			mem_free((void **)&tpSendMailIList);
			ErrorMessage(hDlg, STR_ERR_MEMALLOC);
			EndDialog(hDlg, FALSE);
			break;
		}
		tpTmpMailItem->Cc = alloc_copy_t(tpMailItem->Cc);
		tpTmpMailItem->Bcc = alloc_copy_t(tpMailItem->Bcc);
		tpTmpMailItem->Attach = alloc_copy_t(tpMailItem->Attach);
		tpTmpMailItem->FwdAttach = alloc_copy_t(tpMailItem->FwdAttach);
		tpTmpMailItem->AttachSize = tpMailItem->AttachSize;
		tpTmpMailItem->ReplyTo = alloc_copy_t(tpMailItem->ReplyTo);
		tpTmpMailItem->DefReplyTo = tpMailItem->DefReplyTo;
		tpTmpMailItem->References = alloc_copy_t(tpMailItem->References);
		tpTmpMailItem->Mark = tpMailItem->Mark;
		break;

	case WM_CLOSE:
		tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
		if (*(tpSendMailIList + 1) != NULL) {
			item_free((tpSendMailIList + 1), 1);
		}
		if (tpSendMailIList != NULL) {
			mem_free((void **)&tpSendMailIList);
		}
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_TO:
		case IDC_EDIT_TITLE:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_BUTTON_TO:
			i = -1;
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_TITLE));
			// GJC make temporary addressbook for possible editing
			tpTmpAddressBook = addressbook_copy();
			if (tpTmpAddressBook != NULL) {
				tpTmpAddressBook->GetAddrList = TRUE;
				if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS),
					hDlg, AddressListProc, (LPARAM)tpTmpAddressBook) == TRUE) {
					SendDlgItemMessage(hDlg, IDC_EDIT_TO, WM_SETTEXT, 0, (LPARAM)tpTmpAddressBook->AddrList);
				} else {
					SetFocus(GetDlgItem(hDlg, IDC_EDIT_TO));
				}
				addressbook_free(tpTmpAddressBook);
			}
			break;

		case IDC_BUTTON_CC:
			tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpSendMailIList == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			tpTmpMailItem = *(tpSendMailIList + 1);
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_CC),
				hDlg, CcListProc, (LPARAM)tpTmpMailItem);

			if (tpTmpMailItem != NULL) {
				BtnFlag = ((tpTmpMailItem->Cc != NULL && *tpTmpMailItem->Cc != TEXT('\0')) ||
					(tpTmpMailItem->Bcc != NULL && *tpTmpMailItem->Bcc != TEXT('\0'))) ? TRUE : FALSE;
				SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_CC), STR_SETSEND_BTN_CC, BtnFlag);
			}
			break;

		case IDC_BUTTON_ATTACH:
			tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpSendMailIList == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			tpTmpMailItem = *(tpSendMailIList + 1);
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ATTACH),
				hDlg, SetAttachProc, (LPARAM)tpTmpMailItem);

			if (tpTmpMailItem != NULL) {
				BtnFlag = ( (tpTmpMailItem->Attach != NULL && *tpTmpMailItem->Attach != TEXT('\0'))
						|| (tpTmpMailItem->FwdAttach != NULL && *tpTmpMailItem->FwdAttach != TEXT('\0')) ) 
						? TRUE : FALSE;
				SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ATTACH), STR_SETSEND_BTN_ATTACH, BtnFlag);
			}
			break;

		case IDC_COMBO_SMTP:
			if (HIWORD(wParam) == CBN_CLOSEUP) {
				tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
				if (tpSendMailIList == NULL) {
					EndDialog(hDlg, FALSE);
					break;
				}
				tpTmpMailItem = *(tpSendMailIList + 1);
				if (tpTmpMailItem->DefReplyTo == TRUE) {
					sel = 0;
					// GJC update reply-to
					i = SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_GETCURSEL, 0, 0);
					if (i != CB_ERR) {
						int real=0, sbox=0;
						i += MAILBOX_USER;
						// smtp drop-down only has "real" accounts, increment i by the number of saveboxes below it
						for (j = MAILBOX_USER; j < MailBoxCnt && real < i; j++) {
							if ((MailBox + j)->Type == MAILBOX_TYPE_SAVE) {
								sbox++;
							} else {
								real++;
							}
						}
						i += sbox;
						p = (MailBox + i)->ReplyTo;
						if (p != NULL && *p != TEXT('\0')) {
							for (j=0; j < SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_GETCOUNT, 0, 0); j++) {
								SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_GETLBTEXT, j, (LPARAM)buf);
								if (lstrcmpi(p, buf) == 0) {
									sel = j;
									break;
								}
							}
						}
					}
					SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_SETCURSEL, sel, 0);
				}
			}
			break;

		case IDC_COMBO_REPLYTO: // GJC
			if (HIWORD(wParam) == CBN_EDITCHANGE || HIWORD(wParam) == CBN_CLOSEUP) {
				tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
				if (tpSendMailIList == NULL) {
					EndDialog(hDlg, FALSE);
					break;
				}
				tpMailItem = *tpSendMailIList;
				tpTmpMailItem = *(tpSendMailIList + 1);
				if (HIWORD(wParam) == CBN_EDITCHANGE) {
					tpTmpMailItem->DefReplyTo = FALSE;
				} else if (tpTmpMailItem->DefReplyTo == TRUE) {
					// check if ReplyTo is still the default for selected account
					SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
					i = SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_GETCURSEL, 0, 0);
					if (i != CB_ERR) {
						int real=0, sbox=0;
						i += MAILBOX_USER;
						// smtp drop-down only has "real" accounts, increment i by the number of saveboxes below it
						for (j = MAILBOX_USER; j < MailBoxCnt && real < i; j++) {
							if ((MailBox + j)->Type == MAILBOX_TYPE_SAVE) {
								sbox++;
							} else {
								real++;
							}
						}
						i += sbox;
						p = (MailBox + i)->ReplyTo;
						if (((p == NULL || *p == TEXT('\0')) && lstrcmp(buf, STR_OMIT_REPLYTO) != 0)
							|| (p != NULL && *p != TEXT('\0') && lstrcmp(buf, p) != 0)) {
							tpTmpMailItem->DefReplyTo = FALSE;
						}
					}

				}
			}
			break;

		case IDOK:
			//Check
			if (CheckDependence(hDlg, IDC_EDIT_TO) == FALSE ||
				CheckDependence(hDlg, IDC_EDIT_TITLE) == FALSE) {
				break;
			}

			tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpSendMailIList == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			tpMailItem = *tpSendMailIList;
			tpTmpMailItem = *(tpSendMailIList + 1);
			if (tpMailItem->Mark == ICON_SENTMAIL) {
				if (tpTmpMailItem != NULL) {
					item_free(&tpTmpMailItem, 1);
				}
				mem_free((void **)&tpSendMailIList);
				EndDialog(hDlg, TRUE);
				break;
			}

			//of type dependence letter ? ? ? ? ?
			mem_free(&tpMailItem->MailBox);
			tpMailItem->MailBox = NULL;
			i = SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_GETCURSEL, 0, 0);
			if (i != CB_ERR) {
				int real=0, sbox=0;
				// smtp drop-down only has "real" accounts, increment i by the number of saveboxes below it
				i += MAILBOX_USER;
				for (j = MAILBOX_USER; j < MailBoxCnt && real < i; j++) {
					if ((MailBox + j)->Type == MAILBOX_TYPE_SAVE) {
						sbox++;
					} else {
						real++;
					}
				}
				i += sbox;
			}
			if (i != CB_ERR && (MailBox + i)->Name != NULL) {
				tpMailItem->MailBox = alloc_copy_t((MailBox + i)->Name);

				mem_free(&tpMailItem->From);
				tpMailItem->From = NULL;
				if ((MailBox + i)->MailAddress != NULL && *(MailBox + i)->MailAddress != TEXT('\0')) {
					len = lstrlen(TEXT(" <>"));
					if ((MailBox + i)->UserName != NULL) {
						len += lstrlen((MailBox + i)->UserName);
					}
					len += lstrlen((MailBox + i)->MailAddress);
					tpMailItem->From = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
					if (tpMailItem->From != NULL) {
						p = tpMailItem->From;
						if ((MailBox + i)->UserName != NULL && *(MailBox + i)->UserName != TEXT('\0')) {
							p = str_join_t(p, (MailBox + i)->UserName, TEXT(" "), (TCHAR *)-1);
						}
						str_join_t(p, TEXT("<"), (MailBox + i)->MailAddress, TEXT(">"), (TCHAR *)-1);
					}
				}
			}

			SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			mem_free(&tpTmpMailItem->ReplyTo);
			if (buf[0] != TEXT('\0') && lstrcmp(buf, STR_OMIT_REPLYTO) != 0) {
				tpTmpMailItem->ReplyTo = alloc_copy_t(buf);
			} else {
				tpTmpMailItem->ReplyTo = NULL;
			}

			////////////////////////// MRP //////////////////////
			// Get my new settings here and set them into :
			// tpMailItem->Priority (int)
			// tpMailItem->ReadReceipt (int - 1 = yes, 0 = No)
			// tpMailItem->DeliveryReceipt (int - 1 = yes, 0 = No)
			i = SendDlgItemMessage(hDlg, IDC_PRIORITY, CB_GETCURSEL, 0, 0);
			if (i == 0)	{
				tpMailItem->Priority = 1;
			} else if (i == 1) {
				tpMailItem->Priority = 3;
			} else if (i == 2) {
				tpMailItem->Priority = 5;
			}

			tpMailItem->ReadReceipt = SendDlgItemMessage(hDlg, IDC_READ, BM_GETCHECK, 0, 0);
			tpMailItem->DeliveryReceipt = SendDlgItemMessage(hDlg, IDC_DEL, BM_GETCHECK, 0, 0);
			////////////////////////// --- ////////////////////////

			//Address
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_TO), &tpMailItem->To);
			if (tpMailItem->To != NULL) {
				delete_ctrl_char(tpMailItem->To);
			}
			// 件名
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_TITLE), &tpMailItem->Subject);
			if (tpMailItem->Subject != NULL) {
				delete_ctrl_char(tpMailItem->Subject);
			}
			if (tpTmpMailItem != NULL) {
				mem_free(&tpMailItem->Cc);
				mem_free(&tpMailItem->Bcc);
				mem_free(&tpMailItem->Attach);
				mem_free(&tpMailItem->FwdAttach);
				mem_free(&tpMailItem->ReplyTo);

				// Cc
				tpMailItem->Cc = alloc_copy_t(tpTmpMailItem->Cc);
				// Bcc
				tpMailItem->Bcc = alloc_copy_t(tpTmpMailItem->Bcc);
				// Attach
				tpMailItem->Attach = alloc_copy_t(tpTmpMailItem->Attach);
				tpMailItem->FwdAttach = alloc_copy_t(tpTmpMailItem->FwdAttach);
				tpMailItem->AttachSize = tpTmpMailItem->AttachSize;
				if ( (tpMailItem->Attach != NULL && *tpMailItem->Attach != TEXT('\0')) 
					|| (tpMailItem->FwdAttach != NULL && *tpMailItem->FwdAttach != TEXT('\0')) ) {
					tpMailItem->Multipart = MULTIPART_ATTACH;
				} else {
					tpMailItem->Multipart = MULTIPART_NONE;
				}

				switch (tpMailItem->Priority)
				{
					case 5:  // LOW
					case 4:
						st = 2 + 3*tpMailItem->Multipart;
						break;

					case 1:  // HIGH
					case 2:
						st = 1 + 3*tpMailItem->Multipart;
						break;

					case 3:  // NORMAL
					default:
						st = 0 + 3*tpMailItem->Multipart;
						break;
				}
				if (SelBox == MAILBOX_SEND) {
					HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
					i = ListView_GetMemToItem(hListView, tpMailItem);
					if (i != -1) {
						ListView_SetItemState(hListView, i, INDEXTOSTATEIMAGEMASK(st), LVIS_STATEIMAGEMASK)
						ListView_RedrawItems(hListView, i, i);
						UpdateWindow(hListView);
					}
				}
				// ReplyTo
				tpMailItem->ReplyTo = alloc_copy_t(tpTmpMailItem->ReplyTo);
				tpMailItem->DefReplyTo = tpTmpMailItem->DefReplyTo;
				
				item_free(&tpTmpMailItem, 1);
			}

			//Quotation
			if (tpMailItem->Mark == 1 || tpMailItem->Mark == 3) {
				tpMailItem->Mark = (char)SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_GETCHECK, 0, 0);
			} else if (tpMailItem->Mark == 2) {
				tpMailItem->Mark = (char)SendDlgItemMessage(hDlg, IDC_CHECK_QUOT_3ST, BM_GETCHECK, 0, 0);
			}
			mem_free((void **)&tpSendMailIList);
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * GetMailAddressList - リストビューからメールアドレスのリストを作成
 */
static TCHAR *GetMailAddressList(HWND hDlg, HWND hListView)
{
	TCHAR *buf;
	TCHAR get_buf[BUF_SIZE];
	int Len;
	int i;

	Len = 0;
	i = -1;
	while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		*get_buf = TEXT('\0');
		ListView_GetItemText(hListView, i, 1, get_buf, BUF_SIZE - 1);
		Len += lstrlen(get_buf) + 2;
	}
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (Len + 1));
	if (buf == NULL) {
		ErrorMessage(hDlg, STR_ERR_MEMALLOC);
		return NULL;
	}
	*buf = TEXT('\0');
	i = -1;
	while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
		*get_buf = TEXT('\0');
		ListView_GetItemText(hListView, i, 1, get_buf, BUF_SIZE - 1);
		if (*buf != TEXT('\0')) {
			lstrcat(buf, TEXT(", "));
		}
		lstrcat(buf, get_buf);
	}

	if (*buf == TEXT('\0')) {
		mem_free(&buf);
		return NULL;
	}
	return buf;
}

/*
 * MailPropProc - メールアドレス情報プロシージャ
 */
BOOL CALLBACK MailPropProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAILITEM *tpMailItem;
	ADDRESSITEM *tpAddrItem;
	HWND hListView;
	TCHAR *buf, *p;
	TCHAR *type;
	TCHAR msg[BUF_SIZE];
	int i, ItemIndex;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ADDRESSINFO);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		if (lParam == 0) {
			EndDialog(hDlg, FALSE);
			break;
		}
		tpMailItem = (MAILITEM *)lParam;
		SetWindowLong(hDlg, GWL_USERDATA, lParam);

		hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
#ifdef _WIN32_WCE_PPC
		///////////// MRP /////////////////////
		if (op.UsePOOMAddressBook == 1)
		{
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADDADDRESS), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_ADDADDRESS), FALSE);
		}
		///////////// --- /////////////////////

		ListView_AddColumn(hListView, LVCFMT_LEFT, 70, STR_MAILPROP_HEADER, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 140, STR_MAILPROP_MAILADDRESS, 1);
#else
		ListView_AddColumn(hListView, LVCFMT_LEFT, 70, STR_MAILPROP_HEADER, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 330, STR_MAILPROP_MAILADDRESS, 1);
#endif
		ListView_SetExtendedListViewStyle(hListView,
			LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		for (i = 0; i < 4; i++) {
			switch (i) {
			case 0:
				p = tpMailItem->From;
				type = TEXT("From:");
				break;
			case 1:
				p = tpMailItem->ReplyTo;
				type = TEXT("Reply-To:");
				break;
			case 2:
				p = tpMailItem->To;
				type = TEXT("To:");
				break;
			case 3:
				p = tpMailItem->Cc;
				type = TEXT("Cc:");
				break;
			default:
				p = NULL;
				type = NULL;
				break;
			}
			if (p == NULL || *p == TEXT('\0')) {
				continue;
			}
			buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(p) + 1));
			if (buf == NULL) {
				continue;
			}
			while (*p != TEXT('\0')) {
				p = GetMailString(p, buf);
				ItemIndex = ListView_AddOptionItem(hListView, type);
				ListView_SetItemText(hListView, ItemIndex, 1, buf);
				p = (*p != TEXT('\0')) ? p + 1 : p;
			}
			mem_free(&buf);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_NOTIFY:
		DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_ADDRESS));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_LV_ALLSELECT:
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_ADDRESS), -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		case IDC_BUTTON_NEW:
			if (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_ADDRESS)) <= 0) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}
		case ID_LV_EDIT:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			//From selective item of list view list compilation
			buf = GetMailAddressList(hDlg, hListView);
			if (buf == NULL) {
				break;
			}

			//Setting
			i = Edit_MailToSet(hInst, hDlg, buf, -1);
			if (i != EDIT_NONEDIT) {
				EndDialog(hDlg, TRUE);
#ifdef _WIN32_WCE
				if (i == EDIT_INSIDEEDIT) {
					ShowWindow(GetParent(hDlg), SW_HIDE);
				}
#else
				if (op.ViewClose == 1) {
					SendMessage(GetParent(hDlg), WM_CLOSE, 0, 0);
				}
#endif
			}
			mem_free(&buf);
			break;

		case IDC_BUTTON_REPLY:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}
			//of information of transmission From selective item of list view list compilation
			buf = GetMailAddressList(hDlg, hListView);
			if (buf == NULL) {
				break;
			}

			// 返信情報の作成
			tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
			if (tpMailItem == NULL) {
				mem_free(&buf);
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}
			item_copy((MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA), tpMailItem);
			mem_free(&tpMailItem->ReplyTo);
			tpMailItem->ReplyTo = buf;

			//of information of reply Setting
			i = Edit_InitInstance(hInst, hDlg, vSelBox, tpMailItem, EDIT_REPLY, NULL);
			item_free(&tpMailItem, 1);
			if (i != EDIT_NONEDIT) {
				EndDialog(hDlg, TRUE);
#ifdef _WIN32_WCE
				if (i == EDIT_INSIDEEDIT) {
					ShowWindow(GetParent(hDlg), SW_HIDE);
				}
#else
				if (op.ViewClose == 1) {
					SendMessage(GetParent(hDlg), WM_CLOSE, 0, 0);
				}
#endif
			}
			break;

		case IDC_BUTTON_ADDADDRESS:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}
			wsprintf(msg, STR_Q_ADDADDRESS,
				ListView_GetSelectedCount(hListView));
			if (MessageBox(hDlg, msg, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			i = -1;
			while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
				tpAddrItem = (ADDRESSITEM *)mem_calloc(sizeof(ADDRESSITEM));
				if (tpAddrItem == NULL) {
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
				tpAddrItem->MailAddress = ListView_AllocGetText(hListView, i, 1);
				if (tpAddrItem->MailAddress == NULL) {
					mem_free(&tpAddrItem);
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
				delete_ctrl_char(tpAddrItem->MailAddress);
				//of information of transmission In address register mail address additional
				if (addr_add(AddressBook, tpAddrItem) == FALSE) {
					mem_free(&tpAddrItem->MailAddress);
					mem_free(&tpAddrItem);
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
			}
			if (op.AutoSave == 1) {
				// アドレス帳を保存
				file_save_address_book(ADDRESS_FILE, AddressBook);
			}
			break;

		case IDOK:
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

/*
 * SetAddressList - リストビューにアドレス帳のリストを表示する
 */
static void SetAddressList(HWND hDlg, ADDRESSBOOK *tpAddressBook, TCHAR *Filter)
{
	HWND hAddrList;
	TCHAR *Group, *tmpGrp, *p, *q;
	int i, j, cnt, sel;
	int ItemIndex;

	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_ADDSTRING, 0, (LPARAM)STR_ADDRESSLIST_ALLGROUP);
	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_ADDSTRING, 0, (LPARAM)STR_ADDRESSLIST_NOGROUP);
	if (Filter != NULL && lstrcmp(Filter, STR_ADDRESSLIST_NOGROUP) == 0) {
		sel = 1;
	} else {
		sel = 0;
	}
	cnt = 1;

	hAddrList = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
	ListView_DeleteAllItems(hAddrList);

	for (i = 0; i < tpAddressBook->ItemCnt; i++) {
		BOOL AddIt = FALSE;
		ADDRESSITEM *item = *(tpAddressBook->tpAddrItem + i);
		if (item == NULL) {
			continue;
		}
		Group = item->Group;
		if (lstrcmp(Filter, STR_ADDRESSLIST_ALLGROUP) == 0) {
			AddIt = TRUE;
		} else if (Group == NULL || *Group == TEXT('\0')) {
			if (lstrcmp(Filter, STR_ADDRESSLIST_NOGROUP) == 0) {
				AddIt = TRUE;
			}
		} else {
			q = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Group) + 1));
			p = Group;
			while (q != NULL && *p != TEXT('\0')) {
				p = str_cpy_f_t(q, p, TEXT(','));
				tmpGrp = q;
				while (*tmpGrp == TEXT(' ')) tmpGrp++;
				if (lstrcmp(Filter, tmpGrp) == 0) {
					AddIt = TRUE;
					break;
				}
			}
			mem_free(&q);
		}
		if (AddIt == TRUE) {
			ItemIndex = ListView_AddOptionItem(hAddrList, item->MailAddress);
			ListView_SetItemText(hAddrList, ItemIndex, 1, item->Comment);
			ListView_SetItemText(hAddrList, ItemIndex, 2, Group);
			item->displayed = TRUE;
		} else {
			item->displayed = FALSE;
		}
		if (Group != NULL && *Group != TEXT('\0')) {
			p = Group;
			q = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Group) + 1));
			while (q != NULL && *p != TEXT('\0')) {
				BOOL Found = FALSE;
				p = str_cpy_f_t(q, p, TEXT(','));
				tmpGrp = q;
				while (*tmpGrp == TEXT(' ')) tmpGrp++;
				for (j = 0; j < SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_GETCOUNT, 0, 0); j++) {
					TCHAR buf[BUF_SIZE];
					SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_GETLBTEXT, j, (LPARAM)buf);
					if (lstrcmp(buf, tmpGrp) == 0) {
						Found = TRUE;
						break;
					}
				}

				if (Found == FALSE) {
					SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_ADDSTRING, 0, (LPARAM)tmpGrp);
					cnt++;
					if (Filter != NULL && lstrcmp(Filter, tmpGrp) == 0) {
						sel = cnt;
					}
				}
			}
			mem_free(&q);
		}
	}
	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_SETCURSEL, sel, 0);
	
	if ((i = tpAddressBook->EditNum) >= 0) {
		sel = 0;
		for (j = 0; j < i; j++) {
			ADDRESSITEM *item = *(tpAddressBook->tpAddrItem + j);
			if (item != NULL && item->displayed == TRUE) {
				sel++;
			}
		}
		ListView_SetItemState(hAddrList, -1, 0, LVIS_SELECTED);
		ListView_SetItemState(hAddrList, sel,
			LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		ListView_EnsureVisible(hAddrList, sel, TRUE);
	}
}

/*
 * ListSelToAddrNum - convert selection number to address item number (GJC)
 *                    (some items may not be displayed based on Group)
 */
int ListSelToAddrNum(ADDRESSBOOK *tpAddressBook, int Sel)
{
	int i, j;
	for (i = 0, j = 0; i < tpAddressBook->ItemCnt; i++) {
		ADDRESSITEM *AddrItem = *(tpAddressBook->tpAddrItem + i);
		if (AddrItem != NULL && AddrItem->displayed == TRUE) {
			if (j == Sel) {
				break;
			}
			j++;
		}
	}
	return i;
}

/*
 * EditAddressProc - アドレス編集プロシージャ
 *                   (significant modifications by GJC)
 */
static BOOL CALLBACK EditAddressProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hAddrList;
	ADDRESSBOOK *tpTmpAddressBook;
	ADDRESSITEM *AddrItem;
	TCHAR buf[BUF_SIZE];
	int i, j, jj, cnt, sel;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_EDITADDRESS);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		tpTmpAddressBook = (ADDRESSBOOK *)lParam;
		sel = i = tpTmpAddressBook->EditNum;
		if (sel >= 0) {
			i = ListSelToAddrNum(tpTmpAddressBook, sel);
			AddrItem = *(tpTmpAddressBook->tpAddrItem + i);

			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)AddrItem->MailAddress);
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

			SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)AddrItem->Comment);
			SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		} else if (sel == -2) {
			// adding Group to multiple addresses
			SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)STR_ADDRESSBOOK_ADDGROUP);
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)STR_MULTIPLE_ADDRESSES);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMENT), 0);
		} // else sel==-1, add new address item

		SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_ADDSTRING, 0, (LPARAM)TEXT(""));
		sel = cnt = 0;
		for (j = 0; j < tpTmpAddressBook->ItemCnt; j++) {
			TCHAR *Group;
			BOOL Found = FALSE;
			Group = (*(tpTmpAddressBook->tpAddrItem + j))->Group;
			if (Group == NULL || *Group == TEXT('\0')) continue;
			for (jj = 0; jj < SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_GETCOUNT, 0, 0); jj++) {
				SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_GETLBTEXT, jj, (LPARAM)buf);
				if (lstrcmp(buf, Group) == 0) {
					Found = TRUE;
					break;
				}
			}
			if (Found == FALSE) {
				SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_ADDSTRING, 0, (LPARAM)Group);
				cnt++;
				if (i >= 0 && lstrcmp(Group, AddrItem->Group) == 0) {
					sel = cnt;
				}
			}
		}
		SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_SETCURSEL, sel, 0);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_MAILADDRESS:
		case IDC_EDIT_COMMENT:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			sel = tpTmpAddressBook->EditNum;
			if (sel == -1) {
				AddrItem = (ADDRESSITEM *)mem_calloc(sizeof(ADDRESSITEM));
				if (AddrItem == NULL || addr_add(tpTmpAddressBook, AddrItem) == FALSE) {
					EndDialog(hDlg, FALSE);
					break;
				}
			} else if (sel >= 0) {
				i = ListSelToAddrNum(tpTmpAddressBook, sel);
				AddrItem = *(tpTmpAddressBook->tpAddrItem + i);
			}
			hAddrList = GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS);

			if (sel != -2) {
				//Mail address
				*buf = TEXT('\0');
				SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
				if (*buf == TEXT('\0')) {
					ErrorMessage(hDlg, STR_ERR_INPUTMAILADDR);
					break;
				}
				delete_ctrl_char(buf);
				mem_free(&AddrItem->MailAddress);
				AddrItem->MailAddress = alloc_copy_t(buf);
				ListView_SetItemText(hAddrList, sel, 0, buf);

				//Comment
				*buf = TEXT('\0');
				SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
				delete_ctrl_char(buf);
				mem_free(&AddrItem->Comment);
				if (*buf != TEXT('\0')) {
					AddrItem->Comment = alloc_copy_t(buf);
				} else {
					AddrItem->Comment = NULL;
				}
				ListView_SetItemText(hAddrList, sel, 1, buf);
			}

			//Group/Category
			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			delete_ctrl_char(buf);
			if (sel == -2) {
				if (*buf != TEXT('\0')) {
					TCHAR *tmp;
					i = -1;
					while ((i = ListView_GetNextItem(hAddrList, i, LVIS_SELECTED)) != -1) {
						j = ListSelToAddrNum(tpTmpAddressBook, i);
						AddrItem = *(tpTmpAddressBook->tpAddrItem + j);
						if (AddrItem->Group == NULL) {
							AddrItem->Group = alloc_copy_t(buf);
						} else {
							cnt = lstrlen(AddrItem->Group) + lstrlen(buf) + 3;
							tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * cnt);
							if (tmp == NULL) {
								EndDialog(hDlg, FALSE);
								break;
							}
							wsprintf(tmp, TEXT("%s, %s"), AddrItem->Group, buf);
							mem_free(&AddrItem->Group);
							AddrItem->Group = tmp;
						}
						ListView_SetItemText(hAddrList, i, 2, AddrItem->Group);
					}
				}
			} else {
				mem_free(&AddrItem->Group);
				if (*buf != TEXT('\0')) {
					TCHAR *q;
					for (q = buf; *q != TEXT('\0'); q++) {
						if (*q == TEXT(';')) *q = TEXT(',');
					}
					AddrItem->Group = alloc_copy_t(buf);
				} else {
					AddrItem->Group = NULL;
				}
				ListView_SetItemText(hAddrList, sel, 2, buf);
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

/*
 * AddressListProc - アドレス帳設定プロシージャ
 */
BOOL CALLBACK AddressListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hListView;
	TCHAR *StrAddr;
	TCHAR buf[BUF_SIZE];
	int SelectItem;
	int i;
	ADDRESSBOOK *tpTmpAddressBook;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ADDRESSLIST);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		tpTmpAddressBook = (ADDRESSBOOK *)lParam;
		if (tpTmpAddressBook->GetAddrList == TRUE) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MAIL), 0);
		}

		hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);

#ifdef _WIN32_WCE_PPC
		///////////// MRP /////////////////////
		if (op.UsePOOMAddressBook == 1) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), FALSE);
		}
		///////////// --- /////////////////////
#endif
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[0], STR_ADDRESSLIST_MAILADDRESS, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[1], STR_ADDRESSLIST_COMMENT, 1);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[2], STR_ADDRESSLIST_GROUP, 2);
		ListView_SetExtendedListViewStyle(hListView,
			LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		tpTmpAddressBook->EditNum = -1;
		SetAddressList(hDlg, tpTmpAddressBook, STR_ADDRESSLIST_ALLGROUP);
		break;

#ifndef _WIN32_WCE
	case WM_DRAWITEM:
		switch ((UINT)wParam) {
		case IDC_BUTTON_UP:
			DrawScrollControl((LPDRAWITEMSTRUCT)lParam, DFCS_SCROLLUP);
			break;
		case IDC_BUTTON_DOWN:
			DrawScrollControl((LPDRAWITEMSTRUCT)lParam, DFCS_SCROLLDOWN);
			break;
		default:
			return FALSE;
		}
		break;
#endif

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_NOTIFY:
		DialogLvNotifyProc(hDlg, lParam, GetDlgItem(hDlg, IDC_LIST_ADDRESS));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_LV_ALLSELECT:
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_ADDRESS), -1, LVIS_SELECTED, LVIS_SELECTED);
			break;

		case IDC_BUTTON_UP:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				break;
			}
			if (ListView_GetNextItem(hListView, -1, LVIS_SELECTED) <= 0) {
				break;
			}
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			SelectItem = -1;
			while ((SelectItem = ListView_GetNextItem(hListView, SelectItem, LVNI_SELECTED)) != -1) {
				ListView_SetItemState(hListView, SelectItem, 0, LVIS_SELECTED);
				ListView_MoveItem(hListView, SelectItem, -1, 3);
				ListView_SetItemState(hListView, SelectItem - 1, LVIS_SELECTED, LVIS_SELECTED);
				addr_move(tpTmpAddressBook, ListSelToAddrNum(tpTmpAddressBook, SelectItem), -1);
			}
			//SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			//SetAddressList(hDlg, tpTmpAddressBook, buf);
			break;

		case IDC_BUTTON_DOWN:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				break;
			}
			if (ListView_GetItemState(hListView, ListView_GetItemCount(hListView) - 1,
				LVNI_SELECTED) == LVNI_SELECTED) {
				break;
			}
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			for (i = ListView_GetItemCount(hListView) - 1; i >= 0; i--) {
				if (ListView_GetItemState(hListView, i, LVNI_SELECTED) == LVNI_SELECTED) {
					ListView_SetItemState(hListView, i, 0, LVIS_SELECTED);
					ListView_MoveItem(hListView, i, 1, 3);
					ListView_SetItemState(hListView, i + 1, LVIS_SELECTED, LVIS_SELECTED);
					addr_move(tpTmpAddressBook, ListSelToAddrNum(tpTmpAddressBook, i), +1);
				}
			}
			//SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			//SetAddressList(hDlg, tpTmpAddressBook, buf);
			break;

		case IDC_BUTTON_ADD:
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			tpTmpAddressBook->EditNum = -1;
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS_EDIT),
				hDlg, EditAddressProc, (LPARAM)tpTmpAddressBook);
			SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			tpTmpAddressBook->EditNum = tpTmpAddressBook->ItemCnt - 1;
			SetAddressList(hDlg, tpTmpAddressBook, buf);
			break;

		case ID_LV_EDIT:
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpTmpAddressBook->GetAddrList == TRUE) {
				//Address selective
				if (ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_ADDRESS)) > 0) {
					SendMessage(hDlg, WM_COMMAND, IDOK, 0);
				}
				break;
			}
			if (ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_ADDRESS), -1,
				LVNI_FOCUSED | LVIS_SELECTED) == -1) {
				break;
			}
		case IDC_BUTTON_EDIT:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			i = ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_ADDRESS));
			if (i > 1) {
				tpTmpAddressBook->EditNum = -2;
			} else if (i == 1) {
				tpTmpAddressBook->EditNum = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED | LVIS_SELECTED);
			} else {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS_EDIT),
				hDlg, EditAddressProc, (LPARAM)tpTmpAddressBook);
			//SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			//SetAddressList(hDlg, tpTmpAddressBook, buf);
			break;

		case IDC_BUTTON_DELETE:
		case ID_LV_DELETE:
			//Deletion
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}
			if (MessageBox(hDlg, STR_Q_DELETE, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			while ((SelectItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) != -1) {
				addr_delete(tpTmpAddressBook, ListSelToAddrNum(tpTmpAddressBook, SelectItem));
				ListView_DeleteItem(hListView, SelectItem);
			}
			//SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			//SetAddressList(hDlg, tpTmpAddressBook, buf);
			break;

		case IDC_BUTTON_MAIL:
			//The mail is written the
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if ((SelectItem = ListView_GetSelectedCount(hListView)) <= 0) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}

			StrAddr = ListView_GetSelStringList(hListView);
			if (StrAddr == NULL) {
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}

			SendMessage(hDlg, WM_COMMAND, IDOK, 0);
			if (Edit_MailToSet(hInst, MainWnd, StrAddr, -1) == EDIT_INSIDEEDIT) {
#ifdef _WIN32_WCE
				ShowWindow(MainWnd, SW_HIDE);
#endif
			}
			mem_free(&StrAddr);
			break;

		case IDC_ADDR_GRP_COMBOL:
			if (HIWORD(wParam) == CBN_CLOSEUP) {
				hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
				ListView_SetRedraw(hListView, FALSE);
				SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
				tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
				tpTmpAddressBook->EditNum = -1;
				SetAddressList(hDlg, tpTmpAddressBook, buf);
				ListView_SetRedraw(hListView, TRUE);
			}
			break;

		case IDOK:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpTmpAddressBook->GetAddrList == TRUE) {
				if ((SelectItem = ListView_GetSelectedCount(hListView)) <= 0) {
					ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
					break;
				}
				tpTmpAddressBook->AddrList = ListView_GetSelStringList(hListView);
				if (tpTmpAddressBook->AddrList == NULL) {
					ErrorMessage(hDlg, STR_ERR_MEMALLOC);
					break;
				}
			}

			if (AddressBook->tpAddrItem != NULL) {
				addr_free(AddressBook->tpAddrItem, AddressBook->ItemCnt);
				mem_free((void **)&AddressBook->tpAddrItem);
			}
			AddressBook->tpAddrItem = tpTmpAddressBook->tpAddrItem;
			AddressBook->ItemCnt = tpTmpAddressBook->ItemCnt;
			tpTmpAddressBook->tpAddrItem = NULL;
			tpTmpAddressBook->ItemCnt = 0;

			if (op.AutoSave == 1) {
				// アドレス帳を保存
				file_save_address_book(ADDRESS_FILE, AddressBook);
			}
			for (i = 0; i < AD_COL_CNT; i++) {
				op.AddColSize[i] = ListView_GetColumnWidth(hListView, i);
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

/*
 * SetFindProc - 検索設定プロシージャ
 */
BOOL CALLBACK SetFindProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_FIND);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);
		if (FindStr != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_FIND, WM_SETTEXT, 0, (LPARAM)FindStr);
		}
		SendDlgItemMessage(hDlg, IDC_CHECK_CASE, BM_SETCHECK, op.MatchCase, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_ALL, BM_SETCHECK, op.AllFind, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SUBJECT, BM_SETCHECK, op.SubjectFind, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SUBJECT), op.AllFind);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_FIND:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_CHECK_ALL:
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SUBJECT),
				SendDlgItemMessage(hDlg, IDC_CHECK_ALL, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_FIND), &FindStr);
			if (FindStr == NULL || *FindStr == TEXT('\0')) {
				ErrorMessage(hDlg, STR_ERR_INPUTFINDSTRING);
				break;
			}
			op.MatchCase = SendDlgItemMessage(hDlg, IDC_CHECK_CASE, BM_GETCHECK, 0, 0);
			op.AllFind = SendDlgItemMessage(hDlg, IDC_CHECK_ALL, BM_GETCHECK, 0, 0);
			op.SubjectFind = SendDlgItemMessage(hDlg, IDC_CHECK_SUBJECT, BM_GETCHECK, 0, 0);

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

/*
 * NewMailMessageProc - メール到着メッセージのプロシージャ
 */
BOOL CALLBACK NewMailMessageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT DialogRect;
	static int sBox = -1;
	int i, j;

	switch (uMsg) {
	case WM_INITDIALOG:
		MsgWnd = hDlg;
		SetWindowText(hDlg, WINDOW_TITLE);

#ifdef _WCE_OLD
		ShowMsgFlag = TRUE;

		GetWindowRect(hDlg, &DialogRect);
		SetWindowPos(hDlg, HWND_TOP,
			(GetSystemMetrics(SM_CXSCREEN) / 2) - ((DialogRect.right - DialogRect.left) / 2),
			(GetSystemMetrics(SM_CYSCREEN) / 2) - ((DialogRect.bottom - DialogRect.top) / 2),
			0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
		ShowWindow(hDlg, SW_SHOW);
		_SetForegroundWindow(hDlg);

		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			if ((MailBox + i)->NewMail == TRUE) {
				sBox = i;
				break;
			}
		}
#endif
		break;

#ifndef _WCE_OLD
	case WM_SHOWDIALOG:
		ShowMsgFlag = TRUE;

		GetWindowRect(hDlg, &DialogRect);

		SetWindowPos(hDlg, HWND_TOPMOST,
			(GetSystemMetrics(SM_CXSCREEN) / 2) - ((DialogRect.right - DialogRect.left) / 2),
			(GetSystemMetrics(SM_CYSCREEN) / 2) - ((DialogRect.bottom - DialogRect.top) / 2),
			0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
		ShowWindow(hDlg, SW_SHOWNOACTIVATE);
		if (op.ActiveNewMailMessage == 1) {
			_SetForegroundWindow(hDlg);
		}
		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			if ((MailBox + i)->NewMail == TRUE) {
				sBox = i;
				break;
			}
		}
		break;
#endif

	case WM_ENDDIALOG:
		ShowMsgFlag = FALSE;
		MsgWnd = NULL;
		DestroyWindow(hDlg);
		break;

	case WM_CLOSE:
		ShowMsgFlag = FALSE;
#ifdef _WCE_OLD
		MsgWnd = NULL;
		DestroyWindow(hDlg);
#else
		SetFocus(GetDlgItem(hDlg, IDOK));
		ShowWindow(hDlg, SW_HIDE);
#endif
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			PostMessage(MainWnd, WM_COMMAND, ID_MENUITEM_RESTORE, 0);
			//Selective
			if (sBox == -1) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			if (sBox != SelBox) {
				mailbox_select(MainWnd, sBox);
			}
			{
				HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
				i = ListView_GetItemCount(hListView);
				j = ListView_GetNewItem(hListView, (MailBox + sBox));
				if (i > 0 && j != -1) {
					ListView_EnsureVisible(hListView, i - 1, TRUE);
					ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
					ListView_SetItemState(hListView, j,
						LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
					ListView_EnsureVisible(hListView, (j <= 0) ? 0 : (j - 1), TRUE);
				}
			}

		case IDCANCEL:
			sBox = -1;
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * AttachNoticeProc - 添付確認プロシージャ
 */
BOOL CALLBACK AttachNoticeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ATTACHINFO *ai;
	TCHAR buf[BUF_SIZE];

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ATTACH);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SendDlgItemMessage(hDlg, IDC_RADIO_SAVE, BM_SETCHECK, BST_CHECKED, 0);

		ai = (ATTACHINFO *)lParam;
		if (ai == NULL) {
			break;
		}
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FROM), ai->from);
		wsprintf(buf, TEXT("%s, %d %s"), ai->mime, (ai->size < 1024) ? ai->size : (ai->size / 1024), (ai->size < 1024) ? TEXT("byte") : TEXT("KB"));
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TYPE), buf);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_FILENAME), ai->fname);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			AttachProcess = (SendDlgItemMessage(hDlg, IDC_RADIO_OPEN, BM_GETCHECK, 0, 0) == BST_CHECKED) ? 1 : 0;
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

/*
 * EnableSortColumns - enable/disable radio buttons for sorting
 */
BOOL CALLBACK EnableSortColumns(HWND hDlg, BOOL EnableFlag) {
	EnableWindow(GetDlgItem(hDlg, IDC_SORTITEM1), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_SORTITEM2), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_SORTITEM3), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_SORTITEM4), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SORTORDER), EnableFlag);

	return TRUE;
}


BOOL CALLBACK SelSaveBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR Name[BUF_SIZE];
	int i;
	int *sel;
	
	switch(uMsg)
	{
	case WM_INITDIALOG:
		sel = (int *)lParam;
		SetWindowText(GetDlgItem(hDlg, IDC_MOVECOPYTEXT), *sel ? STR_TITLE_MOVE2 : STR_TITLE_COPY2);
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			if ((MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
				SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, CB_ADDSTRING, 0, (LPARAM)(MailBox+i)->Name);
			}
		}
		SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, CB_SETCURSEL, 0, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Name);
			sel = (int*)GetWindowLong(hDlg, GWL_USERDATA);
			*sel = mailbox_name_to_index(Name);
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

///////////// MRP /////////////////////
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
		SetWindowText(GetDlgItem(hDlg, IDC_APPNAME), APP_NAME);
		SetWindowText(GetDlgItem(hDlg, IDC_ABOUT_TEXT), STR_ABOUT_TEXT);

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
///////////// --- /////////////////////

/* End of source */
