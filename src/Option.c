/*
 * nPOP
 *
 * Option.c
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
#ifndef _WIN32_WCE
#define sizeof_PROPSHEETHEADER		40	// 古いコモンコントロール対策
#else
#define sizeof_PROPSHEETHEADER		sizeof(PROPSHEETHEADER)
#endif

#define ID_LV_EDIT					(WM_APP + 200)
#define ID_LV_DELETE				(WM_APP + 201)
#define ID_LV_ALLSELECT				(WM_APP + 202)

typedef enum {
	FILTER_UNRECV_INDEX = 0,
	FILTER_RECV_INDEX,
	FILTER_DOWNLOADMARK_INDEX,
	FILTER_DELETEMARK_INDEX,
	FILTER_READICON_INDEX,
	FILTER_SAVE_INDEX,
};

/* Global Variables */
HWND MsgWnd = NULL;

extern OPTION op;

static MAILBOX *tpOptionMailBox;
static BOOL PropRet;
static HWND hLvFilter;

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
extern MAILBOX *AddressBox;

extern TCHAR *FindStr;

/* Local Function Prototypes */
static void SetControlFont(HWND pWnd);
static LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, HWND hListView);
static int ListView_AddOptionItem(HWND hListView, TCHAR *buf);
static TCHAR *ListView_AllocGetText(HWND hListView, int Index, int Col);
static BOOL CALLBACK PopSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetSmtpAuthProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SmtpSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK MakeSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SetComboItem(HWND hCombo);
static void EnableFilterEditButton(HWND hDlg, BOOL EnableFlag);
static BOOL CALLBACK EditFilterProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void EnableFilterButton(HWND hDlg, BOOL EnableFlag);
static int GetFilterActionInt(TCHAR *buf);
static TCHAR *GetFilterActionString(int i);
static void SetFilterList(HWND hListView);
static BOOL CALLBACK FilterSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void EnableRasOption(HWND hDlg, int Flag);
static BOOL CALLBACK RasSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void EnableFilterButton(HWND hDlg, BOOL EnableFlag);
static BOOL CALLBACK FilterSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetRecvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetSendOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetMakeOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetCheckOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetRasOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetEtcOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SetCcList(HWND hDlg, TCHAR *strList, TCHAR *type);
static BOOL CALLBACK CcListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK EtcHeaderProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SetButtonText(HWND hButton, TCHAR *title, BOOL UseFlag);
static void SetAddressList(HWND hDlg);
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

	case PSN_QUERYCANCEL:		// キャンセル
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
	return alloc_copy(buf);
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

	// フレームコントロールの描画
	DrawFrameControl(lpDrawItem->hDC, &(lpDrawItem->rcItem), DFC_SCROLL, i);

	// フォーカス
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
		/* コントロールの初期化 */
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
			// 名前
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_NAME), &tpOptionMailBox->Name);
			// サーバ
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_SERVER), &tpOptionMailBox->Server);
			tpOptionMailBox->PopIP = 0;
			// ポート番号
			tpOptionMailBox->Port = GetDlgItemInt(hDlg, IDC_EDIT_PORT, NULL, FALSE);
			// ユーザID
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_USER), &tpOptionMailBox->User);
			// パスワード
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &tpOptionMailBox->Pass);
			// APOP
			tpOptionMailBox->APOP = SendDlgItemMessage(hDlg, IDC_CHECK_APOP, BM_GETCHECK, 0, 0);
			// POP SSL
			tpOptionMailBox->PopSSL = SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0);
			// CyclicFlag
			tpOptionMailBox->CyclicFlag = SendDlgItemMessage(hDlg, IDC_CHECK_CYCLIC, BM_GETCHECK, 0, 0);
			// No RETR
			tpOptionMailBox->NoRETR = SendDlgItemMessage(hDlg, IDC_CHECK_NORETR, BM_GETCHECK, 0, 0);

			// 一時パスワードの解放
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
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_SMTPAUTH);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

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
			// 認証モード
			tpOptionMailBox->AuthUserPass = !SendDlgItemMessage(hDlg, IDC_CHECK_POP, BM_GETCHECK, 0, 0);
			// ユーザID
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_USER), &tpOptionMailBox->SmtpUser);
			// パスワード
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
		/* コントロールの初期化 */
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
			// SMTP認証 設定
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
			// 本名
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_NAME), &tpOptionMailBox->UserName);
			// メールアドレス
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), &tpOptionMailBox->MailAddress);
			// サーバ
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_SERVER), &tpOptionMailBox->SmtpServer);
			tpOptionMailBox->SmtpIP = 0;
			// ポート番号
			tpOptionMailBox->SmtpPort = GetDlgItemInt(hDlg, IDC_EDIT_PORT, NULL, FALSE);
			// SMTP Authentication
			tpOptionMailBox->SmtpAuth = SendDlgItemMessage(hDlg, IDC_CHECK_SMTPAUTH, BM_GETCHECK, 0, 0);
			// SMTP SSL
			tpOptionMailBox->SmtpSSL = SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0);
			// POP before SMTP
			tpOptionMailBox->PopBeforeSmtp = SendDlgItemMessage(hDlg, IDC_CHECK_POPBEFORESMTP, BM_GETCHECK, 0, 0);
			// 自分宛てにコピーを送信するフラグ
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
			// 署名
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
	TCHAR buf[BUF_SIZE];
	TCHAR *p;
	int i;

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
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_SAVE);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, 0, 0);

		SetComboItem(GetDlgItem(hDlg, IDC_COMBO_HEAD1));
		SetComboItem(GetDlgItem(hDlg, IDC_COMBO_HEAD2));

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		i = lParam;
		if (i == -1) {
			SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_SETCHECK, 1, 0);
			break;
		}

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 0, buf, BUF_SIZE - 1);
		if (lstrcmp(buf, STR_FILTER_USE) == 0) {
			SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_SETCHECK, 1, 0);
		} else {
			EnableFilterEditButton(hDlg, FALSE);
		}

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 1, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);

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
			EnableFilterEditButton(hDlg, SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			i = GetWindowLong(hDlg, GWL_USERDATA);

			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_HEAD1, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0')) {
				ErrorMessage(hDlg, STR_ERR_NOITEM1);
				break;
			}

			if (SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_GETCHECK, 0, 0) == FALSE) {
				p = STR_FILTER_NOUSE;
			} else {
				p = STR_FILTER_USE;
			}
			if (i == -1) {
				i = ListView_AddOptionItem(hLvFilter, p);
			} else {
				ListView_SetItemText(hLvFilter, i, 0, p);
			}
			ListView_SetItemText(hLvFilter, i, 1,
				GetFilterActionString(SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0)));

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

	} else if (lstrcmp(buf, STR_FILTER_SAVE) == 0) {
		return FILTER_SAVE_INDEX;
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

	case FILTER_SAVE_INDEX:
		return STR_FILTER_SAVE;
	}
	return STR_FILTER_UNRECV;
}

/*
 * SetFilterList - リストビューにフィルタのリストを表示する
 */
static void SetFilterList(HWND hListView)
{
	int ItemIndex;
	int i;

	for (i = 0; i < tpOptionMailBox->FilterCnt; i++) {
		if (*(tpOptionMailBox->tpFilter + i) == NULL) {
			continue;
		}

		ItemIndex = ListView_AddOptionItem(hListView,
			((*(tpOptionMailBox->tpFilter + i))->Enable == 0) ? STR_FILTER_NOUSE : STR_FILTER_USE);

		ListView_SetItemText(hListView, ItemIndex, 1,
			GetFilterActionString((*(tpOptionMailBox->tpFilter + i))->Action));

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
				FreeFilterInfo(tpOptionMailBox);
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
 * SetMailBoxOption - アカウントの設定を行う
 */
BOOL SetMailBoxOption(HWND hWnd)
{
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[5];

	if (SelBox == MAILBOX_SAVE || SelBox == MAILBOX_SEND) {
		return FALSE;
	}

	tpOptionMailBox = (MailBox + SelBox);

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

	// 作成
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_MAKE);
	psp.pfnDlgProc = MakeSetProc;
	hpsp[2] = CreatePropertySheetPage(&psp);

	// フィルタ
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

	SelectMailBox(hWnd, SelBox);
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
 * SetMakeOptionProc - 作成設定プロシージャ
 */
static BOOL CALLBACK SetMakeOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SendDlgItemMessage(hDlg, IDC_CHECK_AUTOQUOT, BM_SETCHECK, op.AutoQuotation, 0);
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_SETCHECK, op.ViewClose, 0);
#endif
		if (op.QuotationChar != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_QUOTCHAR, WM_SETTEXT, 0, (LPARAM)op.QuotationChar);
		}
		if (op.ReHeader != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_REHEAD, WM_SETTEXT, 0, (LPARAM)op.ReHeader);
		}
		SendDlgItemMessage(hDlg, IDC_EDIT_QUOTCHAR, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_QUOTCHAR:
		case IDC_EDIT_REHEAD:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			op.AutoQuotation = SendDlgItemMessage(hDlg, IDC_CHECK_AUTOQUOT, BM_GETCHECK, 0, 0);
#ifndef _WIN32_WCE
			op.ViewClose = SendDlgItemMessage(hDlg, IDC_CHECK_VIEWCLOSE, BM_GETCHECK, 0, 0);
#endif
			// 引用記号
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_QUOTCHAR), &op.QuotationChar);
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
		SendDlgItemMessage(hDlg, IDC_CHECK_SHIWNEWMESSAGE, BM_SETCHECK, op.ShowNewMailMessgae, 0);
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
			if (GetFileName(hDlg, buf, TEXT("wav"), STR_WAVE_FILTER, TRUE) == FALSE) {
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
			op.ShowNewMailMessgae = SendDlgItemMessage(hDlg, IDC_CHECK_SHIWNEWMESSAGE, BM_GETCHECK, 0, 0);
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
 * SetEtcOptionProc - その他の設定プロシージャ
 */
static BOOL CALLBACK SetEtcOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SendDlgItemMessage(hDlg, IDC_CHECK_SHOWTRAYICON, BM_SETCHECK, op.ShowTrayIcon, 0);
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_MINSIZEHIDE, BM_SETCHECK, op.MinsizeHide, 0);
#endif
		SendDlgItemMessage(hDlg, IDC_CHECK_CLOSEHIDE, BM_SETCHECK, op.CloseHide, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_MOVEALLMAILBOX, BM_SETCHECK, op.MoveAllMailBox, 0);

#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_STARTPASS, BM_SETCHECK, op.StertPass, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SHOWPASS, BM_SETCHECK, op.ShowPass, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0, (LPARAM)op.Password);
#endif

#ifndef _WIN32_WCE
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MINSIZEHIDE), op.ShowTrayIcon);
#endif
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CLOSEHIDE), op.ShowTrayIcon);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
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

#ifndef _WIN32_WCE
			op.StertPass = SendDlgItemMessage(hDlg, IDC_CHECK_STARTPASS, BM_GETCHECK, 0, 0);
			op.ShowPass = SendDlgItemMessage(hDlg, IDC_CHECK_SHOWPASS, BM_GETCHECK, 0, 0);
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &op.Password);
#endif
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * SetOption - 設定ダイアログの表示
 */
void SetOption(HWND hWnd)
{
	PROPSHEETPAGE psp;
	PROPSHEETHEADER psh;
	HPROPSHEETPAGE hpsp[6];

	psp.dwSize = sizeof(PROPSHEETPAGE);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = hInst;

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

	// 送信
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_SEND);
	psp.pfnDlgProc = SetSendOptionProc;
	hpsp[1] = CreatePropertySheetPage(&psp);

	// 作成
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_MAKE);
	psp.pfnDlgProc = SetMakeOptionProc;
	hpsp[2] = CreatePropertySheetPage(&psp);

	// チェック
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_CHECK);
	psp.pfnDlgProc = SetCheckOptionProc;
	hpsp[3] = CreatePropertySheetPage(&psp);

	// ダイアルアップ
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_RAS);
	psp.pfnDlgProc = SetRasOptionProc;
	hpsp[4] = CreatePropertySheetPage(&psp);

	// その他
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_ETC);
	psp.pfnDlgProc = SetEtcOptionProc;
	hpsp[5] = CreatePropertySheetPage(&psp);

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

	PropertySheet(&psh);
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
			str_join(buf, WINDOW_TITLE TEXT(" - ["), (TCHAR *)lParam, TEXT("]"), (TCHAR *)-1);
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

		// メール数
		wsprintf(buf, STR_STATUS_INIT_MAILCNT, SelMailBox->MailCnt);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MAILCNT), buf);

		// メールサイズ
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
		// メールアドレスを抽出
		r = GetMailString(p, buf);

		// 余分な空白を除去
		for (; *p == TEXT(' ') || *p == TEXT('\t') || *p == TEXT('\r') || *p == TEXT('\n'); p++);

		// リストに追加する文字列の抽出
		for (s = buf; p < r; p++) {
			if (*p == TEXT('\t')) {
				*(s++) = TEXT(' ');
			} else if (*p != TEXT('\r') && *p != TEXT('\n')) {
				*(s++) = *p;
			}
		}
		*s = TEXT('\0');
		if (*buf != TEXT('\0')) {
			// 余分な空白を除去
			if (*(s - 1) == TEXT(' ')) {
				for (s--; s > buf && *s == TEXT(' '); s--);
				*(s + 1) = TEXT('\0');
			}
			// リストに追加する
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
			p = NULL;
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS),
				hDlg, AddressListProc, (LPARAM)&p) == TRUE) {
				SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)p);
			}
			mem_free(&p);
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
			SendDlgItemMessage(hDlg, IDC_RADIO_CC, BM_SETCHECK, 1, 0);
			SendDlgItemMessage(hDlg, IDC_RADIO_BCC, BM_SETCHECK, 0, 0);
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS));
			break;

		case IDOK:
			CcLen = BccLen = 0;

			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
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
#define ID_FILE_ADD		(WM_APP + 100)
	MAILITEM *tpMailItem;
	TCHAR fpath[BUF_SIZE], buf[BUF_SIZE], *f;
	int i, len;

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
				f = str_cpy_f(fpath, f, ATTACH_SEP);
				SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)fpath);
			}
		}
		if (tmp_attach != NULL) {
			f = tmp_attach;
			while (*f != TEXT('\0')) {
				f = str_cpy_f(fpath, f, ATTACH_SEP);
				SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)fpath);
			}
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

#ifndef _WIN32_WCE
	case WM_DROPFILES:
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
			if (GetFileName(hDlg, fpath, NULL, NULL, TRUE) == FALSE) {
				break;
			}
			lParam = (LPARAM)fpath;
		case ID_FILE_ADD:
			if (CheckDir((TCHAR *)lParam) == TRUE) {
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
			mem_free(&tpMailItem->Attach);
			tpMailItem->Attach = NULL;

			if (SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0) <= 0) {
				EndDialog(hDlg, TRUE);
				break;
			}
			// サイズの取得
			len = 0;
			for (i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0); i++) {
				if (i != 0) {
					len++;
				}
				len += SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETTEXTLEN, i, 0);
			}
			f = tpMailItem->Attach = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
			if (tpMailItem->Attach == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			// ファイル名のリストの作成
			for (i = 0; i < SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0); i++) {
				if (i != 0) {
					*(f++) = ATTACH_SEP;
				}
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETTEXT, i, (LPARAM)fpath);
				f = str_cpy_t(f, fpath);
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
 * EtcHeaderProc - その他のヘッダの設定プロシージャ
 */
static BOOL CALLBACK EtcHeaderProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAILITEM *tpMailItem;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ETCHEADER);
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

		SendDlgItemMessage(hDlg, IDC_EDIT_REPLYTO, WM_SETTEXT, 0, (LPARAM)tpMailItem->ReplyTo);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_REPLYTO:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
			}
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_REPLYTO), &tpMailItem->ReplyTo);
			delete_ctrl_char(tpMailItem->ReplyTo);

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
	TCHAR *p;
	int len;
	int i, st;
	BOOL BtnFlag;

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
		/* コントロールの初期化 */
		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			if ((MailBox + i)->Name == NULL || *(MailBox + i)->Name == TEXT('\0')) {
				SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
				continue;
			}
			SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_ADDSTRING, 0, (LPARAM)(MailBox + i)->Name);
		}

		if (tpMailItem->MailBox != NULL) {
			i = GetNameToMailBox(tpMailItem->MailBox);
			SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_SETCURSEL, (i != -1) ? i - MAILBOX_USER : 0, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_SETCURSEL, (SelBox >= MAILBOX_USER) ? SelBox - MAILBOX_USER : 0, 0);
		}

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
		if (tpMailItem->Attach != NULL && *tpMailItem->Attach != TEXT('\0')) {
			SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ATTACH), STR_SETSEND_BTN_ATTACH, TRUE);
		}
		if (tpMailItem->ReplyTo != NULL && *tpMailItem->ReplyTo != TEXT('\0')) {
			SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ETC), STR_SETSEND_BTN_ETC, TRUE);
		}
		// 引用
		if (tpMailItem->Status == 1) {
			SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_SETCHECK, op.AutoQuotation, 0);
		} else {
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOTATION), SW_HIDE);
		}

		*(tpSendMailIList + 1) = tpTmpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
		if (tpTmpMailItem == NULL) {
			mem_free((void **)&tpSendMailIList);
			ErrorMessage(hDlg, STR_ERR_MEMALLOC);
			EndDialog(hDlg, FALSE);
			break;
		}
		tpTmpMailItem->Cc = alloc_copy(tpMailItem->Cc);
		tpTmpMailItem->Bcc = alloc_copy(tpMailItem->Bcc);
		tpTmpMailItem->Attach = alloc_copy(tpMailItem->Attach);
		tpTmpMailItem->ReplyTo = alloc_copy(tpMailItem->ReplyTo);
		break;

	case WM_CLOSE:
		tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
		if (*(tpSendMailIList + 1) != NULL) {
			FreeMailItem((tpSendMailIList + 1), 1);
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
			p = NULL;
			if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS),
				hDlg, AddressListProc, (LPARAM)&p) == TRUE) {
				SendDlgItemMessage(hDlg, IDC_EDIT_TO, WM_SETTEXT, 0, (LPARAM)p);
			} else {
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_TO));
			}
			mem_free(&p);
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
				BtnFlag = (tpTmpMailItem->Attach != NULL && *tpTmpMailItem->Attach != TEXT('\0')) ? TRUE : FALSE;
				SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ATTACH), STR_SETSEND_BTN_ATTACH, BtnFlag);
			}
			break;

		case IDC_BUTTON_ETC:
			tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpSendMailIList == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			tpTmpMailItem = *(tpSendMailIList + 1);
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ETCHEAD),
				hDlg, EtcHeaderProc, (LPARAM)tpTmpMailItem);

			if (tpTmpMailItem != NULL) {
				BtnFlag = (tpTmpMailItem->ReplyTo != NULL && *tpTmpMailItem->ReplyTo != TEXT('\0')) ? TRUE : FALSE;
				SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ETC), STR_SETSEND_BTN_ETC, BtnFlag);
			}
			break;

		case IDOK:
			// 機種依存文字のチェック
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

			// アカウント
			mem_free(&tpMailItem->MailBox);
			tpMailItem->MailBox = NULL;
			i = SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_GETCURSEL, 0, 0);
			if (i != CB_ERR && (MailBox + i)->Name != NULL) {
				i += MAILBOX_USER;
				tpMailItem->MailBox = alloc_copy((MailBox + i)->Name);

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
							p = str_join(p, (MailBox + i)->UserName, TEXT(" "), (TCHAR *)-1);
						}
						str_join(p, TEXT("<"), (MailBox + i)->MailAddress, TEXT(">"), (TCHAR *)-1);
					}
				}
			}
			// 宛先
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
				mem_free(&tpMailItem->ReplyTo);

				// Cc
				tpMailItem->Cc = alloc_copy(tpTmpMailItem->Cc);
				// Bcc
				tpMailItem->Bcc = alloc_copy(tpTmpMailItem->Bcc);
				// Attach
				tpMailItem->Attach = alloc_copy(tpTmpMailItem->Attach);
				if (tpMailItem->Attach != NULL && *tpMailItem->Attach != TEXT('\0')) {
					tpMailItem->Multipart = TRUE;
					st = INDEXTOSTATEIMAGEMASK(1);
				} else {
					tpMailItem->Multipart = FALSE;
					st = 0;
				}
				if (SelBox == MAILBOX_SEND) {
					i = ListView_GetMemToItem(GetDlgItem(MainWnd, IDC_LISTVIEW), tpMailItem);
					if (i != -1) {
						ListView_SetItemState(GetDlgItem(MainWnd, IDC_LISTVIEW), i, st, LVIS_STATEIMAGEMASK)
						ListView_RedrawItems(GetDlgItem(MainWnd, IDC_LISTVIEW), i, i);
						UpdateWindow(GetDlgItem(MainWnd, IDC_LISTVIEW));
					}
				}
				// ReplyTo
				tpMailItem->ReplyTo = alloc_copy(tpTmpMailItem->ReplyTo);
				FreeMailItem(&tpTmpMailItem, 1);
			}
			// 引用
			if (tpMailItem->Status == 1) {
				tpMailItem->Status = SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_GETCHECK, 0, 0);
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
 * GetMeilAddressList - リストビューからメールアドレスのリストを作成
 */
static TCHAR *GetMeilAddressList(HWND hDlg, HWND hListView)
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
#ifdef _WIN32_WCE
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
			// リストビューの選択項目からリストを作成
			buf = GetMeilAddressList(hDlg, hListView);
			if (buf == NULL) {
				break;
			}

			// 送信情報の設定
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
			// リストビューの選択項目からリストを作成
			buf = GetMeilAddressList(hDlg, hListView);
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
			CopyItem((MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA), tpMailItem);
			mem_free(&tpMailItem->ReplyTo);
			tpMailItem->ReplyTo = buf;

			// 送信情報の設定
			i = Edit_InitInstance(hInst, hDlg, vSelBox, tpMailItem, EDIT_REPLY, 0);
			FreeMailItem(&tpMailItem, 1);
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
				tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
				if (tpMailItem == NULL) {
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
				tpMailItem->To = ListView_AllocGetText(hListView, i, 1);
				if (tpMailItem->To == NULL) {
					mem_free(&tpMailItem);
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
				delete_ctrl_char(tpMailItem->To);
				// アドレス帳にメールアドレスを追加
				if (Item_Add(AddressBox, tpMailItem) == FALSE) {
					mem_free(&tpMailItem->To);
					mem_free(&tpMailItem);
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
			}
			if (op.AutoSave == 1) {
				// アドレス帳を保存
				SaveAddressBook(ADDRESS_FILE, AddressBox);
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
static void SetAddressList(HWND hDlg)
{
	int i;
	int ItemIndex;

	for (i = 0; i < AddressBox->MailItemCnt; i++) {
		if ((AddressBox->tpMailItem + i) == NULL) {
			continue;
		}

		ItemIndex = ListView_AddOptionItem(GetDlgItem(hDlg, IDC_LIST_ADDRESS),
			(*(AddressBox->tpMailItem + i))->To);
		ListView_SetItemText(GetDlgItem(hDlg, IDC_LIST_ADDRESS), ItemIndex, 1, (*(AddressBox->tpMailItem + i))->Subject);
	}
}

/*
 * EditAddressProc - アドレス編集プロシージャ
 */
static BOOL CALLBACK EditAddressProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buf[BUF_SIZE];
	int i;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_EDITADDRESS);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		i = lParam;
		if (i == -1) {
			break;
		}

		*buf = TEXT('\0');
		ListView_GetItemText(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), i, 0, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

		*buf = TEXT('\0');
		ListView_GetItemText(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), i, 1, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)buf);
		SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
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
			i = GetWindowLong(hDlg, GWL_USERDATA);

			// メールアドレス
			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			if (*buf == TEXT('\0')) {
				ErrorMessage(hDlg, STR_ERR_INPUTMAILADDR);
				break;
			}
			delete_ctrl_char(buf);
			if (i == -1) {
				i = ListView_AddOptionItem(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), buf);
			} else {
				ListView_SetItemText(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), i, 0, buf);
			}
			// コメント
			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			delete_ctrl_char(buf);
			ListView_SetItemText(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), i, 1, buf);

			// 設定したアイテムを選択状態にする
			ListView_SetItemState(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), -1, 0, LVIS_SELECTED);
			ListView_SetItemState(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), i,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			ListView_EnsureVisible(GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS), i, TRUE);

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
	TCHAR **ret;
	int SelectItem;
	int i;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ADDRESSLIST);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		if (lParam != 0) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MAIL), 0);
		}

		hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[0], STR_ADDRESSLIST_MAILADDRESS, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[1], STR_ADDRESSLIST_COMMENT, 1);
		ListView_SetExtendedListViewStyle(hListView,
			LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		SetAddressList(hDlg);
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
			// 上へ
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				break;
			}
			if (ListView_GetNextItem(hListView, -1, LVIS_SELECTED) <= 0) {
				break;
			}
			SelectItem = -1;
			while ((SelectItem = ListView_GetNextItem(hListView, SelectItem, LVNI_SELECTED)) != -1) {
				ListView_SetItemState(hListView, SelectItem, 0, LVIS_SELECTED);
				ListView_MoveItem(hListView, SelectItem, -1, 2);
				ListView_SetItemState(hListView, SelectItem - 1, LVIS_SELECTED, LVIS_SELECTED);
			}
			break;

		case IDC_BUTTON_DOWN:
			// 下へ
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				break;
			}
			if (ListView_GetItemState(hListView, ListView_GetItemCount(hListView) - 1,
				LVNI_SELECTED) == LVNI_SELECTED) {
				break;
			}
			for (i = ListView_GetItemCount(hListView) - 1; i >= 0; i--) {
				if (ListView_GetItemState(hListView, i, LVNI_SELECTED) == LVNI_SELECTED) {
					ListView_SetItemState(hListView, i, 0, LVIS_SELECTED);
					ListView_MoveItem(hListView, i, 1, 2);
					ListView_SetItemState(hListView, i + 1, LVIS_SELECTED, LVIS_SELECTED);
				}
			}
			break;

		case IDC_BUTTON_ADD:
			// 追加
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS_EDIT),
				hDlg, EditAddressProc, (LPARAM)-1);
			break;

		case ID_LV_EDIT:
			if (GetWindowLong(hDlg, GWL_USERDATA) != 0) {
				// アドレス選択
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
			// 編集
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if ((SelectItem = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED | LVIS_SELECTED)) == -1) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}
			ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
			ListView_SetItemState(hListView, SelectItem, LVIS_SELECTED, LVIS_SELECTED);
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS_EDIT),
				hDlg, EditAddressProc, (LPARAM)SelectItem);
			break;

		case IDC_BUTTON_DELETE:
		case ID_LV_DELETE:
			// 削除
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}
			if (MessageBox(hDlg, STR_Q_DELETE, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			while ((SelectItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) != -1) {
				ListView_DeleteItem(hListView, SelectItem);
			}
			break;

		case IDC_BUTTON_MAIL:
			// メールを書く
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

		case IDOK:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if ((ret = (TCHAR **)GetWindowLong(hDlg, GWL_USERDATA)) != 0) {
				if ((SelectItem = ListView_GetSelectedCount(hListView)) <= 0) {
					ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
					break;
				}
				*ret = ListView_GetSelStringList(hListView);
				if (*ret == NULL) {
					ErrorMessage(hDlg, STR_ERR_MEMALLOC);
					break;
				}
			}

			if (AddressBox->tpMailItem != NULL) {
				FreeMailItem(AddressBox->tpMailItem, AddressBox->MailItemCnt);
				mem_free((void **)&AddressBox->tpMailItem);
				AddressBox->tpMailItem = NULL;
			}

			AddressBox->MailItemCnt = ListView_GetItemCount(hListView);
			AddressBox->tpMailItem = (MAILITEM **)mem_calloc(sizeof(MAILITEM *) * AddressBox->MailItemCnt);
			if (AddressBox->tpMailItem == NULL) {
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}

			for (i = 0; i < AddressBox->MailItemCnt; i++) {
				*(AddressBox->tpMailItem + i) = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
				if (*(AddressBox->tpMailItem + i) == NULL) {
					continue;
				}

				(*(AddressBox->tpMailItem + i))->To = ListView_AllocGetText(hListView, i, 0);
				(*(AddressBox->tpMailItem + i))->Subject = ListView_AllocGetText(hListView, i, 1);
			}
			if (op.AutoSave == 1) {
				// アドレス帳を保存
				SaveAddressBook(ADDRESS_FILE, AddressBox);
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
		SendDlgItemMessage(hDlg, IDC_CHECK_CASE, BM_SETCHECK, op.MstchCase, 0);
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
			op.MstchCase = SendDlgItemMessage(hDlg, IDC_CHECK_CASE, BM_GETCHECK, 0, 0);
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
		SetWindowText(hDlg, APP_NAME);

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
		if (op.ActiveNewMailMessgae == 1) {
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
			// メールボックスの選択とリストビューの新着位置の選択
			if (sBox == -1) {
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				break;
			}
			if (sBox != SelBox) {
				SelectMailBox(MainWnd, sBox);
			}
			i = ListView_GetItemCount(GetDlgItem(MainWnd, IDC_LISTVIEW));
			j = ListView_GetNewItem(GetDlgItem(MainWnd, IDC_LISTVIEW), (MailBox + sBox));
			if (i > 0 && j != -1) {
				ListView_EnsureVisible(GetDlgItem(MainWnd, IDC_LISTVIEW), i - 1, TRUE);
				ListView_SetItemState(GetDlgItem(MainWnd, IDC_LISTVIEW), -1, 0, LVIS_SELECTED);
				ListView_SetItemState(GetDlgItem(MainWnd, IDC_LISTVIEW), j,
					LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(GetDlgItem(MainWnd, IDC_LISTVIEW), (j <= 0) ? 0 : (j - 1), TRUE);
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
/* End of source */
