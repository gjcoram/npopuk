/*
 * nPOP
 *
 * Option.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2009 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#include "jp.h"
#include "mime.h"
#include "multipart.h"
#include "charset.h"
#ifdef _WIN32_WCE
#include "ppcpoom.h"
#endif

/* Define */
#ifndef _WIN32_WCE
#define sizeof_PROPSHEETHEADER		40	//of end
#else
#define sizeof_PROPSHEETHEADER		sizeof(PROPSHEETHEADER)
#endif

#define ID_LV_EDIT					(WM_APP + 200)
#define ID_LV_DELETE				(WM_APP + 201)
#define ID_LV_ALLSELECT				(WM_APP + 202)
#define IDC_EDIT_BODY				2003
#define ID_RESIZE_TIMER				1

typedef struct _ATTACH_ITEM {
	TCHAR *fname;
	BOOL is_fwd;
	struct _ATTACH_ITEM *next;
} ATTACH_ITEM;

/* Global Variables */
HWND MsgWnd = NULL;
BOOL ImportRead = TRUE;
BOOL ImportDown = TRUE;
int ReplaceCnt = 0;
static ATTACH_ITEM *top_attach_item = NULL;

extern OPTION op;
extern TCHAR *DataDir;

static MAILBOX *tpOptionMailBox;
static BOOL PropRet;
static HWND hLvFilter;
static int AddrSortFlag = 0;
static int ViewClose = 0; // to synchronize option on Recv and Fwd tabs

#ifdef _WIN32_WCE
static WNDPROC AddrListProcedure = NULL;
static WNDPROC EditToWndProc = NULL;
static WNDPROC CcAddrWndProc = NULL;
#ifdef _WIN32_WCE
static WNDPROC DefEditTextWndProc = NULL;
#endif
static BOOL CcWndShowing = FALSE;
#else
#define WNDPROC_KEY			TEXT("OldWndProc")
#endif
static TCHAR AutoCompleteStr[BUF_SIZE] = TEXT("");
static int MatchedAddrItem = 0;
static BOOL AutoCompleted = FALSE;

extern HINSTANCE hInst;  // Local copy of hInstance
extern HWND MainWnd;
#ifdef _WIN32_WCE_PPC
extern HWND hMainToolBar;
#endif
extern HWND FocusWnd;
extern HFONT hListFont;
extern HMENU hADPOPUP, hEDITPOPUP;
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
extern BOOL SaveBoxesLoaded;

extern MULTIPART **vMultiPart;
extern int MultiPartCnt, MultiPartTextIndex;

TCHAR *ReplaceStr = NULL;
extern TCHAR *FindStr;
extern int FindNext, FindOrReplace;
extern DWORD FindPos;

/* Local Function Prototypes */
static void SetControlFont(HWND pWnd);
static LRESULT OptionNotifyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, int ListId);
static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam);
static int ListView_AddOptionItem(HWND hListView, TCHAR *buf, long lp);
static TCHAR *ListView_AllocGetText(HWND hListView, int Index, int Col);
static BOOL CALLBACK MboxTypeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK ImportSboxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void MailboxSummaryAdd(int Num, HWND hListView, BOOL newSbox, int Pos);
static void MailboxFilenameUpdate(HWND hListView, int Start, int Stop);
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
static void EnableRasOption(HWND hDlg, int Flag);
static BOOL CALLBACK RasSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK FilterSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetRecvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetSendOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetReplyOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetUpdateOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetForwardOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetViewOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetCheckOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetRasOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetSortOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetEtcOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK SetAdvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK EnableSortColumns(HWND hDlg, BOOL EnableFlag);
static int SetCcList(HWND hDlg, TCHAR *strList, TCHAR *type);
static BOOL CALLBACK CcListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void SetButtonText(HWND hButton, TCHAR *title, BOOL UseFlag);
static void SetWindowSize(HWND hDlg, int ListID, int top, int bottom, int left, int right);
static void SetAddressList(HWND hDlg, ADDRESSBOOK *tpAddressBook, TCHAR *Filter);
static BOOL CALLBACK EditAddressProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef _WIN32_WCE
static LRESULT CALLBACK SubClassAddrListProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
static TCHAR *AddressGetWholeGroup(TCHAR *groupname);
static LRESULT CALLBACK AddrCompleteCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void SetEditToSubClass(HWND hWnd, BOOL CcWnd);
#ifdef _WIN32_WCE_PPC
static LRESULT CALLBACK EditTextCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

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
static void InitDlg(HWND hDlg, TCHAR *str, BOOL full)
{
	SHINITDLGINFO shidi;

	shidi.dwMask = SHIDIM_FLAGS;
	shidi.dwFlags = SHIDIF_SIPDOWN;
	if (full) {
		shidi.dwFlags |= SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
	}
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
void SetSip(HWND hDlg, int edit_notify)
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
static LRESULT DialogLvNotifyProc(HWND hWnd, LPARAM lParam, int ListId)
{
	NMHDR *CForm = (NMHDR *)lParam;
	LV_KEYDOWN *LKey = (LV_KEYDOWN *)lParam;
	HWND hListView = GetDlgItem(hWnd, ListId);

	if (CForm->hwndFrom == GetWindow(hListView, GW_CHILD)) {
		return ListViewHeaderNotifyProc(hWnd, lParam);
	} else if (CForm->hwndFrom != hListView) {
		return 0;
	}

	if (CForm->code == NM_DBLCLK) {
		SendMessage(hWnd, WM_COMMAND, ID_LV_EDIT, 0);
		return 1;
	} else if (CForm->code == NM_CLICK) {
		SendMessage(hWnd, WM_LV_EVENT, CForm->code, lParam);
		return 1;
	}


#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	if (LKey->hdr.code == LVN_BEGINDRAG) {
		return LVBD_DRAGSELECT;
	}
#endif

	if (LKey->hdr.code == LVN_KEYDOWN) {
		if (LKey->wVKey == VK_DELETE) {
			SendMessage(hWnd, WM_COMMAND, ID_LV_DELETE, 0);
			return 1;
		} else if (LKey->wVKey == TEXT('A') && GetKeyState(VK_CONTROL) < 0) {
			SendMessage(hWnd, WM_COMMAND, ID_LV_ALLSELECT, 0);
			return 1;
		}
	}
	return 0;
}

/*
 * ListViewHeaderNotifyProc - リストビューヘッダメッセージ
 */
static LRESULT ListViewHeaderNotifyProc(HWND hWnd, LPARAM lParam)
{
	HD_NOTIFY *phd = (HD_NOTIFY *)lParam;
	HWND hListView;
	int col;

	if (phd->hdr.code == HDN_ITEMCLICK) {
		col = phd->iItem;
		AddrSortFlag = (ABS(AddrSortFlag) == col) ? (AddrSortFlag * -1) : col;
		//of sort Sort
		hListView = GetDlgItem(hWnd, IDC_LIST_ADDRESS);
		SwitchCursor(FALSE);
		ListView_SortItems(hListView, AddrCompareFunc, AddrSortFlag);
		SwitchCursor(TRUE);

		ListView_EnsureVisible(hListView,
			ListView_GetNextItem(hListView, -1, LVNI_FOCUSED), TRUE);

		op.AddressSort = AddrSortFlag;
	}
	return FALSE;
}

/*
 * ListView_AddOptionItem - リストビューにテキストを追加する
 */
static int ListView_AddOptionItem(HWND hListView, TCHAR *buf, long lp)
{
	return ListView_InsertItemEx(hListView, buf, BUF_SIZE - 1, 0, lp, ListView_GetItemCount(hListView));
}

/*
 * ListView_AllocGetText
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
int AllocGetText(HWND hEdit, TCHAR **buf)
{
	int len;

	mem_free(&*buf);
	len = SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) + 1;
	*buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (*buf != NULL) {
		**buf = TEXT('\0');
		SendMessage(hEdit, WM_GETTEXT, len, (LPARAM)*buf);
	}
	return len;
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
		InitDlg(hDlg, STR_TITLE_SETSSL, TRUE);
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
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_NAME),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_SERVER),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_PORT),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_USER),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
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
			if (tpOptionMailBox->CyclicFlag == 0 && (MailBox+SelBox)->CyclicFlag == 1) {
				SaveBoxesLoaded = FALSE;
			}
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
		InitDlg(hDlg, STR_TITLE_SMTPAUTH, TRUE);
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
			j = !SendDlgItemMessage(hDlg, IDC_CHECK_POP, BM_GETCHECK, 0, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_USER), j);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_PASS), j);
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
	TCHAR *bccaddr;
	BOOL checked;
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
		checked = tpOptionMailBox->MyAddr2Bcc;
		SendDlgItemMessage(hDlg, IDC_CHECK_MYADDR2BCC, BM_SETCHECK, checked, 0);
		bccaddr = NULL;
		if (tpOptionMailBox->BccAddr != NULL && *tpOptionMailBox->BccAddr != TEXT('\0')) {
			bccaddr = tpOptionMailBox->BccAddr;
		} else if (checked) {
			bccaddr = tpOptionMailBox->MailAddress;
		}
		SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_SETCHECK, tpOptionMailBox->SmtpSSL, 0);
		SendDlgItemMessage(hDlg, IDC_BCCADDRESS, WM_SETTEXT, 0, (LPARAM)bccaddr);

		SendDlgItemMessage(hDlg, IDC_EDIT_NAME, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_SERVER, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PORT, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		SendDlgItemMessage(hDlg, IDC_BCCADDRESS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BCCADDRESS), checked);

		SendMessage(hDlg, WM_COMMAND, (WPARAM)IDC_CHECK_SMTPAUTH, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL),
			SendDlgItemMessage(hDlg, IDC_CHECK_SSL, BM_GETCHECK, 0, 0));
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_NAME),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_SERVER),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_PORT),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_BCCADDRESS),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
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
		case IDC_BCCADDRESS:
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
				checked = TRUE;
			} else {
				SetDlgItemInt(hDlg, IDC_EDIT_PORT, 25, FALSE);
				checked = FALSE;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SETSSL), checked);
			break;

		case IDC_BUTTON_SETSSL:
			// SSL 設定
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSSL), hDlg, SetSSLProc, (LPARAM)1);
			break;

		case IDC_CHECK_MYADDR2BCC:
			checked = SendDlgItemMessage(hDlg, IDC_CHECK_MYADDR2BCC, BM_GETCHECK, 0, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BCCADDRESS), checked);
			if (checked != 0) {
				if (SendDlgItemMessage(hDlg, IDC_BCCADDRESS, WM_GETTEXTLENGTH, 0, 0) == 0) {
					SendDlgItemMessage(hDlg, IDC_BCCADDRESS, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->MailAddress);
				}
			}
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
			AllocGetText(GetDlgItem(hDlg, IDC_BCCADDRESS), &tpOptionMailBox->BccAddr);
			if (lstrcmp(tpOptionMailBox->MailAddress, tpOptionMailBox->BccAddr) == 0) {
				mem_free(&tpOptionMailBox->BccAddr);
				tpOptionMailBox->BccAddr = NULL;
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
 * MakeSetProc - 作成設定プロシージャ
 */
static BOOL CALLBACK MakeSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		SendDlgItemMessage(hDlg, IDC_EDIT_SIG, WM_SETTEXT, 0, (LPARAM)tpOptionMailBox->Signature);
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_SIG),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
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

static void SetComboSaveboxes(HWND hCombo)
{
	int i;
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	for (i=0; i<MailBoxCnt; i++) {
		if ((MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
			if ((MailBox+i)->Name != NULL && *(MailBox+i)->Name != TEXT('\0')) {
				SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)(MailBox+i)->Name);
			} else {
				SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)STR_MAILBOX_NONAME);
			}
		}
	}
}

static void SetComboPriorities(HWND hCombo)
{
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)FLAG_PRIORITY);
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)HIGH_PRIORITY);
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)NORMAL_PRIORITY);
	SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)LOW_PRIORITY);
	SendMessage(hCombo, CB_SETCURSEL, 0, 0);
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
	EnableWindow(GetDlgItem(hDlg, IDC_FILTER_AND), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_FILTER_OR), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_FILTER_UNLESS), EnableFlag);
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
	int i, j, len, lenc, lenm;
	BOOL enabled = TRUE, is_copy = FALSE, is_move = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_FILTER, TRUE);
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
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_PRIORITY);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_ADDSTRING, 0, (LPARAM)STR_FILTER_FORWARD);
		SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, 0, 0);

		SetComboItem(GetDlgItem(hDlg, IDC_COMBO_HEAD1));
		SetComboItem(GetDlgItem(hDlg, IDC_COMBO_HEAD2));

#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_CONTENT1),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#ifdef _DEBUG
		{
			WNDPROC test = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_CONTENT2),
				GWL_WNDPROC, (DWORD)EditTextCallback);
			if (test != DefEditTextWndProc) {
				ErrorMessage(hDlg, TEXT("Programming error"));
			}
		}
#else
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_CONTENT2),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_FILTER_FWDADDR),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		i = lParam;
		if (i == -1) {
			SendDlgItemMessage(hDlg, IDC_CHECK_FLAG, BM_SETCHECK, 1, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), FALSE);
			ShowWindow(GetDlgItem(hDlg, IDC_FILTER_FWDADDR), SW_HIDE);
			SendDlgItemMessage(hDlg, IDC_FILTER_AND, BM_SETCHECK, 1, 0);
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
		lenc = lstrlen(STR_FILTER_COPY);
		lenm = lstrlen(STR_FILTER_MOVE);
		if (str_cmp_ni_t(buf, STR_FILTER_COPY, lenc) == 0) {
			is_copy = TRUE;
			len = lenc;
		} else if (str_cmp_ni_t(buf, STR_FILTER_MOVE, lenm) == 0) {
			is_move = TRUE;
			len = lenm;
		}
		ShowWindow(GetDlgItem(hDlg, IDC_FILTER_FWDADDR), SW_HIDE);
		SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_ADDSTRING, 0, (LPARAM)TEXT(""));
		if (is_copy || is_move) {
			SetWindowText(GetDlgItem(hDlg, IDC_SBOX_OR_PRIO), STR_FILTER_SAVEBOX);
			SetComboSaveboxes(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX));
			nptr = buf + len;
			SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_SETCURSEL, GetSaveboxNum(nptr), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), enabled);
			*(buf + len) = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);
		} else {
			len = lstrlen(STR_FILTER_PRIORITY);
			if (str_cmp_ni_t(buf, STR_FILTER_PRIORITY, len) == 0) {
				int sel;
				SetComboPriorities(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX));
				nptr = buf + len;
				if (lstrcmp(nptr, HIGH_PRIORITY) == 0) {
					sel = 1;
				} else if (lstrcmp(nptr, NORMAL_PRIORITY) == 0) {
					sel = 2;
				} else if (lstrcmp(nptr, LOW_PRIORITY) == 0) {
					sel = 3;
				} else {
					sel = 0;
				}
				SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, CB_SETCURSEL, sel, 0);
				*(buf + len) = TEXT('\0');
				SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);
			} else {
				len = lstrlen(STR_FILTER_FORWARD);
				if (str_cmp_ni_t(buf, STR_FILTER_FORWARD, len) == 0) {
					SetWindowText(GetDlgItem(hDlg, IDC_SBOX_OR_PRIO), STR_FILTER_FWDADDR);
					ShowWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), SW_HIDE);
					ShowWindow(GetDlgItem(hDlg, IDC_FILTER_FWDADDR), SW_SHOW);
					nptr = buf + len;
					SendDlgItemMessage(hDlg, IDC_FILTER_FWDADDR, WM_SETTEXT, 0, (LPARAM)nptr);
					*(buf + len) = TEXT('\0');
					SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);
				} else {
					EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), FALSE);
					SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_SETCURSEL, GetFilterActionInt(buf), 0);
				}
			}
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
		if (lstrcmp(buf, STR_FILTER_OR) == 0) {
			SendDlgItemMessage(hDlg, IDC_FILTER_OR, BM_SETCHECK, 1, 0);
		} else if (lstrcmp(buf, STR_FILTER_UNLESS) == 0) {
			SendDlgItemMessage(hDlg, IDC_FILTER_UNLESS, BM_SETCHECK, 1, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_FILTER_AND, BM_SETCHECK, 1, 0);
		}

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 5, buf, BUF_SIZE - 1);
		SendDlgItemMessage(hDlg, IDC_COMBO_HEAD2, WM_SETTEXT, 0, (LPARAM)buf);

		*buf = TEXT('\0');
		ListView_GetItemText(hLvFilter, i, 6, buf, BUF_SIZE - 1);
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
				if (j != FILTER_COPY_INDEX && j != FILTER_MOVE_INDEX  && j != FILTER_PRIORITY_INDEX) {
					enabled = FALSE;
				}
			}
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), enabled);
			break;

		case IDC_COMBO_ACTION:
			j = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
			ShowWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), (j == FILTER_FORWARD_INDEX) ? SW_HIDE : SW_SHOW);
			ShowWindow(GetDlgItem(hDlg, IDC_FILTER_FWDADDR), (j == FILTER_FORWARD_INDEX) ? SW_SHOW : SW_HIDE);
			if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX) {
				SetWindowText(GetDlgItem(hDlg, IDC_SBOX_OR_PRIO), STR_FILTER_SAVEBOX);
				SetComboSaveboxes(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX));
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), TRUE);
			} else if (j == FILTER_PRIORITY_INDEX) {
				SetWindowText(GetDlgItem(hDlg, IDC_SBOX_OR_PRIO), STR_FILTER_PRIOLVL);
				SetComboPriorities(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX));
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX), TRUE);
			} else if (j == FILTER_FORWARD_INDEX) {
				SetWindowText(GetDlgItem(hDlg, IDC_SBOX_OR_PRIO), STR_FILTER_FWDADDR);
			} else {
				SetWindowText(GetDlgItem(hDlg, IDC_SBOX_OR_PRIO), TEXT(""));
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
				SetFocus(GetDlgItem(hDlg, IDC_COMBO_HEAD1));
				break;
			}
			if (SendDlgItemMessage(hDlg, IDC_FILTER_UNLESS, BM_GETCHECK, 0, 0) == TRUE) {
				*Name = TEXT('\0');
				SendDlgItemMessage(hDlg, IDC_COMBO_HEAD2, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Name);
				if (*Name == TEXT('\0') && p == STR_FILTER_USE) {
					ErrorMessage(hDlg, STR_ERR_NOITEM2);
					SetFocus(GetDlgItem(hDlg, IDC_COMBO_HEAD2));
					break;
				}
			}

			*Name = TEXT('\0');
			j = SendDlgItemMessage(hDlg, IDC_COMBO_ACTION, CB_GETCURSEL, 0, 0);
			if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX || j == FILTER_PRIORITY_INDEX) {
				SendDlgItemMessage(hDlg, IDC_COMBO_FILT2BOX, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Name);
				if (*Name == TEXT('\0') && p == STR_FILTER_USE) {
					ErrorMessage(hDlg, (j == FILTER_PRIORITY_INDEX) ? STR_ERR_NOPRIORITY : STR_ERR_NOSAVEBOX);
					SetFocus(GetDlgItem(hDlg, IDC_COMBO_FILT2BOX));
					break;
				}
				if (j != FILTER_PRIORITY_INDEX && op.LazyLoadMailboxes != 0) {
					// need to re-check that all saveboxes are loaded
					SaveBoxesLoaded = FALSE;
				}
			} else if (j == FILTER_FORWARD_INDEX) {
				SendDlgItemMessage(hDlg, IDC_FILTER_FWDADDR, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Name);
				if (*Name == TEXT('\0')) {
					ErrorMessage(hDlg, STR_ERR_NOFWDADDRESS);
					SetFocus(GetDlgItem(hDlg, IDC_FILTER_FWDADDR));
					break;
				}
			}

			if (i == -1) {
				i = ListView_AddOptionItem(hLvFilter, p, 0);
			} else {
				ListView_SetItemText(hLvFilter, i, 0, p);
			}

			if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX || j == FILTER_PRIORITY_INDEX
				|| j == FILTER_FORWARD_INDEX) {
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

			if (SendDlgItemMessage(hDlg, IDC_FILTER_OR, BM_GETCHECK, 0, 0) == TRUE) {
				lstrcpy(buf, STR_FILTER_OR);
			} else if (SendDlgItemMessage(hDlg, IDC_FILTER_UNLESS, BM_GETCHECK, 0, 0) == TRUE) {
				lstrcpy(buf, STR_FILTER_UNLESS);
			} else {
				lstrcpy(buf, STR_FILTER_AND);
			}
			ListView_SetItemText(hLvFilter, i, 4, buf);

			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_COMBO_HEAD2, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			ListView_SetItemText(hLvFilter, i, 5, buf);
			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_EDIT_CONTENT2, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			ListView_SetItemText(hLvFilter, i, 6, buf);

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
	EnableWindow(GetDlgItem(hDlg, IDC_CHECK_REFILTER), EnableFlag);
	EnableWindow(GetDlgItem(hDlg, IDC_CHECK_GBLFILTER), EnableFlag && op.GlobalFilterEnable);
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
	int i, j = -1, ret=0;
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

	} else if (lstrcmp(buf, STR_FILTER_PRIORITY) == 0) {
		return FILTER_PRIORITY_INDEX;

	} else if (lstrcmp(buf, STR_FILTER_FORWARD) == 0) {
		return FILTER_FORWARD_INDEX;
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

	case FILTER_PRIORITY_INDEX:
		return STR_FILTER_PRIORITY;

	case FILTER_FORWARD_INDEX:
		return STR_FILTER_FORWARD;

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
	int i, j, cnt;
	if (tpOptionMailBox == NULL) {
		cnt = op.GlobalFilterCnt;
	} else {
		cnt = tpOptionMailBox->FilterCnt;
	}

	for (i = 0; i < cnt; i++) {
		FILTER *tpFilter;
		if (tpOptionMailBox == NULL) {
			tpFilter = *(op.tpFilter + i);
		} else {
			tpFilter = *(tpOptionMailBox->tpFilter + i);
		}
		if (tpFilter == NULL) {
			continue;
		}

		ItemIndex = ListView_AddOptionItem(hListView,
			(tpFilter->Enable == 0) ? STR_FILTER_NOUSE : STR_FILTER_USE, 0);

		j = tpFilter->Action;
		if (j == FILTER_COPY_INDEX || j == FILTER_MOVE_INDEX) {
			wsprintf(buf, TEXT("%s%s"), GetFilterActionString(j), tpFilter->SaveboxName);
		} else if (j == FILTER_FORWARD_INDEX) {
			wsprintf(buf, TEXT("%s%s"), GetFilterActionString(j), tpFilter->FwdAddress);
		} else if (j == FILTER_PRIORITY_INDEX) {
			TCHAR *p;
			switch (tpFilter->Priority) {
				case 1:
				case 2:
					p = HIGH_PRIORITY;
					break;
				case 3:
					p = NORMAL_PRIORITY;
					break;
				case 4:
				case 5:
					p = LOW_PRIORITY;
					break;
				default:
					p = FLAG_PRIORITY;
					break;

			}
			wsprintf(buf, TEXT("%s%s"), GetFilterActionString(j), p);
		} else {
			wsprintf(buf, TEXT("%s"), GetFilterActionString(j));
		}
		ListView_SetItemText(hListView, ItemIndex, 1, buf);

		ListView_SetItemText(hListView, ItemIndex, 2, tpFilter->Header1);
		ListView_SetItemText(hListView, ItemIndex, 3, tpFilter->Content1);

		j = tpFilter->Boolean;
		switch (j) {
			case FILTER_BOOL_OR:
				lstrcpy(buf, STR_FILTER_OR);
				break;
			case FILTER_BOOL_UNLESS:
				lstrcpy(buf, STR_FILTER_UNLESS);
				break;
			default:
				lstrcpy(buf, STR_FILTER_AND);
				break;
		}
		ListView_SetItemText(hListView, ItemIndex, 4, buf);

		ListView_SetItemText(hListView, ItemIndex, 5, tpFilter->Header2);
		ListView_SetItemText(hListView, ItemIndex, 6, tpFilter->Content2);
	}
}

/*
 * FilterSetProc - フィルタ設定プロシージャ
 */
static BOOL CALLBACK FilterSetProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hListView;
	TCHAR buf[BUF_SIZE];
	FILTER **tpFilter = NULL;
	int SelectItem;
	int i, cnt, oper;

	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);

		hListView = hLvFilter = GetDlgItem(hDlg, IDC_LIST_FILTER);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 50, STR_FILTER_STATUS, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_ACTION, 1);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_ITEM1, 2);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_CONTENT1, 3);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 30, STR_FILTER_BOOLEAN, 4);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_ITEM2, 5);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 100, STR_FILTER_CONTENT2, 6);
		ListView_SetExtendedListViewStyle(hListView,
			LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		if ((tpOptionMailBox == NULL && op.GlobalFilterEnable == 0)
			|| (tpOptionMailBox != NULL && tpOptionMailBox->FilterEnable == 0)) {
			EnableFilterButton(hDlg, FALSE);
		} else {
			SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_SETCHECK, 1, 0);
			if (tpOptionMailBox != NULL) {
				if (tpOptionMailBox->FilterEnable & FILTER_REFILTER) {
					SendDlgItemMessage(hDlg, IDC_CHECK_REFILTER, BM_SETCHECK, 1, 0);
				}
				if (op.GlobalFilterEnable && !(tpOptionMailBox->FilterEnable & FILTER_NOGLOBAL)) {
					SendDlgItemMessage(hDlg, IDC_CHECK_GBLFILTER, BM_SETCHECK, 1, 0);
				}
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_GBLFILTER), op.GlobalFilterEnable);
			}
		}
		if (tpOptionMailBox == NULL) {
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_REFILTER), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_GBLFILTER), SW_HIDE);
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
		if (DialogLvNotifyProc(hDlg, lParam, IDC_LIST_FILTER) == 1) {
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
			if (SendDlgItemMessage(hDlg, IDC_CHECK_FILTER, BM_GETCHECK, 0, 0) == 1) {
				if (tpOptionMailBox == NULL) {
					op.GlobalFilterEnable = 1;
				} else {
					tpOptionMailBox->FilterEnable = 1;
					if (SendDlgItemMessage(hDlg, IDC_CHECK_REFILTER, BM_GETCHECK, 0, 0) == 1) {
						tpOptionMailBox->FilterEnable |= FILTER_REFILTER;
					}
					if (op.GlobalFilterEnable) {
						if (SendDlgItemMessage(hDlg, IDC_CHECK_GBLFILTER, BM_GETCHECK, 0, 0) == 0) {
							tpOptionMailBox->FilterEnable |= FILTER_NOGLOBAL;
						}
					}
					if (op.LazyLoadMailboxes != 0) {
						// need to re-check that all saveboxes are loaded
						SaveBoxesLoaded = FALSE;
					}
				}
			} else {
				if (tpOptionMailBox == NULL) {
					op.GlobalFilterEnable = 0;
				} else {
					tpOptionMailBox->FilterEnable = 0;
				}
			}
			cnt = ListView_GetItemCount(GetDlgItem(hDlg, IDC_LIST_FILTER));
			if (tpOptionMailBox == NULL) {
				if (op.tpFilter != NULL) {
					filter_free(NULL);
				}
				op.GlobalFilterCnt = cnt;
				tpFilter = op.tpFilter = (FILTER **)mem_calloc(sizeof(FILTER *) * cnt);
			} else {
				if (tpOptionMailBox->tpFilter != NULL) {
					filter_free(tpOptionMailBox);
				}
				tpOptionMailBox->FilterCnt = cnt;
				tpFilter = tpOptionMailBox->tpFilter = (FILTER **)mem_calloc(sizeof(FILTER *) * cnt);
			}
			if (tpFilter == NULL) {
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}
			for (i = 0; i < cnt; i++) {
				*(tpFilter + i) = (FILTER *)mem_calloc(sizeof(FILTER));
				if (*(tpFilter + i) == NULL) {
					continue;
				}
				hListView = GetDlgItem(hDlg, IDC_LIST_FILTER);
				*buf = TEXT('\0');
				ListView_GetItemText(hListView, i, 0, buf, BUF_SIZE - 1);
				(*(tpFilter + i))->Enable = (lstrcmp(buf, STR_FILTER_USE) == 0) ? 1 : 0;

				*buf = TEXT('\0');
				ListView_GetItemText(hListView, i, 1, buf, BUF_SIZE - 1);
				if (str_cmp_ni_t(buf, STR_FILTER_COPY, lstrlen(STR_FILTER_COPY)) == 0) {
					(*(tpFilter + i))->SaveboxName = alloc_copy_t(buf + lstrlen(STR_FILTER_COPY));
					*(buf + lstrlen(STR_FILTER_COPY)) = TEXT('\0');
				} else if (str_cmp_ni_t(buf, STR_FILTER_MOVE, lstrlen(STR_FILTER_MOVE)) == 0) {
					(*(tpFilter + i))->SaveboxName = alloc_copy_t(buf + lstrlen(STR_FILTER_MOVE));
					*(buf + lstrlen(STR_FILTER_MOVE)) = TEXT('\0');
				} else if (str_cmp_ni_t(buf, STR_FILTER_FORWARD, lstrlen(STR_FILTER_FORWARD)) == 0) {
					(*(tpFilter + i))->FwdAddress = alloc_copy_t(buf + lstrlen(STR_FILTER_FORWARD));
					*(buf + lstrlen(STR_FILTER_FORWARD)) = TEXT('\0');
				} else {
					int len = lstrlen(STR_FILTER_PRIORITY);
					if (str_cmp_ni_t(buf, STR_FILTER_PRIORITY, len) == 0) {
						TCHAR *p = buf + len;
						int prio = 0;
						if (lstrcmp(p, HIGH_PRIORITY) == 0) {
							prio = 1;
						} else if (lstrcmp(p, NORMAL_PRIORITY) == 0) {
							prio = 3;
						} else if (lstrcmp(p, LOW_PRIORITY) == 0) {
							prio = 5;
						}
						(*(tpFilter + i))->Priority = prio;
						*(buf + len) = TEXT('\0');
					}
				}
				(*(tpFilter + i))->Action = GetFilterActionInt(buf);

				(*(tpFilter + i))->Header1 = ListView_AllocGetText(hListView, i, 2);
				(*(tpFilter + i))->Content1 = ListView_AllocGetText(hListView, i, 3);

				*buf = TEXT('\0');
				ListView_GetItemText(hListView, i, 4, buf, BUF_SIZE - 1);
				if (lstrcmp(buf, STR_FILTER_OR) == 0) {
					oper = FILTER_BOOL_OR;
				} else if (lstrcmp(buf, STR_FILTER_UNLESS) == 0) {
					oper = FILTER_BOOL_UNLESS;
				} else {
					oper = FILTER_BOOL_AND;
				}
				(*(tpFilter + i))->Boolean = oper;

				(*(tpFilter + i))->Header2 = ListView_AllocGetText(hListView, i, 5);
				(*(tpFilter + i))->Content2 = ListView_AllocGetText(hListView, i, 6);
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
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_USER),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
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
static BOOL CALLBACK MboxTypeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MAILBOX *mbox = (MailBox + SelBox);
	BOOL ret = TRUE;
	int Type;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_NEWMBOX, TRUE);
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
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_MBOX_NAME),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
		if (Type == MAILBOX_TYPE_SAVE || Type == MAILBOX_ADD_SAVE) {
			SendMessage(hDlg, WM_SETTEXT, 0, (Type == MAILBOX_TYPE_SAVE) ?
				(LPARAM)STR_TITLE_RENAMESBOX : (LPARAM)STR_TITLE_ADDSBOX);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_MBOXIN), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_IMPORTSBOX), FALSE);
			SendDlgItemMessage(hDlg, IDC_RADIO_MBOXSAVE, BM_SETCHECK, BST_CHECKED, 0);
			if (Type == MAILBOX_TYPE_SAVE) {
				SendMessage(GetDlgItem(hDlg, IDC_RADIO_MBOXSAVE), WM_SETTEXT, 0, (LPARAM)STR_RADIO_RENAMESBOX);
			}
		} else {
			SendDlgItemMessage(hDlg, IDC_RADIO_MBOXIN, BM_SETCHECK, BST_CHECKED, 0);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO_MBOXIN:
		case IDC_RADIO_MBOXSAVE:
		case IDC_RADIO_IMPORTSBOX:
			EnableWindow(GetDlgItem(hDlg, IDC_MBOX_NAME), 
				(SendDlgItemMessage(hDlg, IDC_RADIO_IMPORTSBOX, BM_GETCHECK, 0, 0) == 1) ? FALSE : TRUE);
			break;

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_MBOX_NAME:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			if (mbox != NULL) {
				AllocGetText(GetDlgItem(hDlg, IDC_MBOX_NAME), &mbox->Name);
				if (SendDlgItemMessage(hDlg, IDC_RADIO_IMPORTSBOX, BM_GETCHECK, 0, 0) == 1) {
					mbox->Type = MAILBOX_IMPORT_SAVE;
				} else {
					mbox->Type = SendDlgItemMessage(hDlg, IDC_RADIO_MBOXSAVE, BM_GETCHECK, 0, 0);
			
					if (mbox->Type == 1 && (mbox->Name == NULL || *mbox->Name == TEXT('\0'))) {
						mem_free(&mbox->Name);
						mbox->Name = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(STR_SAVEBOX_NONAME)+1));
						if (mbox->Name != NULL) {
							wsprintf(mbox->Name, STR_SAVEBOX_NONAME, (SelBox%100));
						}
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
		mailbox_menu_rebuild(hWnd, FALSE);
		DeleteMBMenu(SelBox);
		InsertMBMenu(SelBox,
			(((MailBox + SelBox)->Name == NULL || *(MailBox + SelBox)->Name == TEXT('\0'))
			? STR_MAILBOX_NONAME : (MailBox + SelBox)->Name));
		return (MailBox + SelBox)->Type;
	}
}

/*
 * ImportSboxProc - set type: Account or SaveBox (GJC)
*/
static BOOL CALLBACK ImportSboxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR *Filename;
	TCHAR *p, *r, *tmp;
	TCHAR msg[MSG_SIZE];
	MAILBOX *tpMailBox;
	BOOL breakout = FALSE;
	BOOL ret = TRUE;
	long fsize;
	int i;

	switch (uMsg) {
	case WM_INITDIALOG:
		Filename = (TCHAR *)lParam;
		SendDlgItemMessage(hDlg, IDC_IMPORT_FILE, WM_SETTEXT, 0, lParam);
		SendDlgItemMessage(hDlg, IDC_IMPORT_FILE, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

		// savebox filename
		for (p = r = Filename; *p != TEXT('\0'); p++) {
			if (*p == TEXT('\\') || *p == TEXT('/')) {
				r = p + 1;
			}
		}
		SendDlgItemMessage(hDlg, IDC_IMPORT_SBOXFILE, WM_SETTEXT, 0, (LPARAM)r);
		SendDlgItemMessage(hDlg, IDC_IMPORT_SBOXFILE, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);

		// savebox name
		tmp = alloc_copy_t(r);
		for (p = r = tmp; *p != TEXT('\0'); p++) {
			if (*p == TEXT('.')) {
				r = p;
			}
		}
		if (r > tmp) {
			*r = TEXT('\0');
		}
		SendDlgItemMessage(hDlg, IDC_IMPORT_SBOXNAME, WM_SETTEXT, 0, (LPARAM)tmp);
		SendDlgItemMessage(hDlg, IDC_IMPORT_SBOXNAME, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
		mem_free(&tmp);

#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_IMPORT_FILE),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_IMPORT_SBOXFILE),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_IMPORT_SBOXNAME),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif

		SendDlgItemMessage(hDlg, IDC_IMPORT_READ, BM_SETCHECK, BST_CHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_IMPORT_DOWN, BM_SETCHECK, BST_CHECKED, 0);

		SetFocus(GetDlgItem(hDlg, IDOK));
		ret = FALSE;
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_IMPORT_FILE:
		case IDC_IMPORT_SBOXFILE:
		case IDC_IMPORT_SBOXNAME:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDOK:
			ImportRead = SendDlgItemMessage(hDlg, IDC_IMPORT_READ, BM_GETCHECK, 0, 0);
			ImportDown = SendDlgItemMessage(hDlg, IDC_IMPORT_DOWN, BM_GETCHECK, 0, 0);

			Filename = NULL;
			AllocGetText(GetDlgItem(hDlg, IDC_IMPORT_FILE), &Filename);
			fsize = file_get_size(Filename);
			if (fsize == -2) {
				wsprintf(msg, STR_ERR_FILE_TOO_LARGE, Filename);
				mem_free(&Filename);
				ErrorMessage(hDlg, msg);
				break;
			} else if (fsize == -1) {
				wsprintf(msg, STR_ERR_FILEEXIST, Filename);
				mem_free(&Filename);
				ErrorMessage(hDlg, msg);
				break;
			}
			tmp = NULL;
			AllocGetText(GetDlgItem(hDlg, IDC_IMPORT_SBOXFILE), &tmp);
			if (tmp == NULL || *tmp == TEXT('\0')) {
				mem_free(&Filename);
				mem_free(&tmp);
				ErrorMessage(hDlg, STR_ERR_NOSBOXNAME);
				break;
			}
			for (p = tmp; *p != TEXT('\0') && breakout == FALSE; p++) {
				if (*p == TEXT('\\') || *p == TEXT('/') || *p == TEXT(':') 
					|| *p == TEXT('*') || *p == TEXT('?') || *p == TEXT('\"')
					|| *p == TEXT('<') || *p == TEXT('>') || *p == TEXT('|')) {
					breakout = TRUE;
					ErrorMessage(hDlg, STR_ERR_FILENAME);
				}
			}
			for (i = 0; i < MailBoxCnt-1 && breakout == FALSE; i++) {
				TCHAR Name[BUF_SIZE];
				if (i == SelBox) continue;
				tpMailBox = (MailBox + i);
				if (i == MAILBOX_SEND) {
					wsprintf(Name, SENDBOX_FILE);
				} else if (tpMailBox->Filename == NULL) {
					wsprintf(Name, TEXT("MailBox%d.dat"), i - MAILBOX_USER);
				} else {
					lstrcpy(Name, tpMailBox->Filename);
				}
				if (lstrcmp(tmp, Name) == 0) {
					breakout = TRUE;
					wsprintf(msg, STR_ERR_FILECONFLICT, tmp, 
						(tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0'))
						? STR_MAILBOX_NONAME : tpMailBox->Name);
					ErrorMessage(hDlg, msg);
					break;
				}
			}
			if (breakout) {
				mem_free(&Filename);
				mem_free(&tmp);
				break;
			}
			tpMailBox = MailBox + SelBox;
			mem_free(&tpMailBox->Filename);
			tpMailBox->Filename = tmp;

			AllocGetText(GetDlgItem(hDlg, IDC_IMPORT_SBOXNAME), &tpMailBox->Name);
			if (tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0')) {
				mem_free(&Filename);
				ErrorMessage(hDlg, STR_ERR_NOSBOXNAME);
				break;
			}
			if (file_copy_to_datadir(hDlg, Filename, tpMailBox->Filename) == FALSE) {
				mem_free(&Filename);
				ErrorMessage(hDlg, STR_ERR_COPYFAIL);
				break;
			}
			mem_free(&Filename);
			EndDialog(hDlg, TRUE);
			break;
	
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;

	default:
		ret = FALSE;
	}

	return ret;
}

/*
 * ImportSavebox (GJC)
 */
BOOL ImportSavebox(HWND hWnd) 
{
	TCHAR buf[BUF_SIZE];
	MAILBOX *tpMailBox = MailBox + SelBox;

#ifdef _WIN32_WCE
	*buf = TEXT('\0');
	if (filename_select(hWnd, buf, TEXT("*"), STR_FILE_FILTER, FILE_OPEN_SINGLE, &op.SavedOpenDir) == FALSE) {
#else
	*buf = TEXT('\0');
	if (filename_select(hWnd, buf, TEXT("mbx"), STR_MBOX_FILTER, FILE_OPEN_SINGLE, &op.SavedOpenDir) == FALSE) {
#endif
		return FALSE;
	}
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_IMPORTSBOX), hWnd, ImportSboxProc, (LPARAM)buf) == FALSE) {
		return FALSE;
	}
	return file_read_mailbox(tpMailBox->Filename, tpMailBox, TRUE, FALSE);
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
			DeleteMBMenu(SelBox);
			InsertMBMenu(SelBox, new_name);
			SelectMBMenu(SelBox);
			for (j = 0; j < op.GlobalFilterCnt; j++) {
				FILTER *tpFilter = *(op.tpFilter + j);
				if (tpFilter->Action == FILTER_COPY_INDEX || tpFilter->Action == FILTER_MOVE_INDEX) {
					if (lstrcmp(tpFilter->SaveboxName, old_name) == 0) {
						mem_free(&tpFilter->SaveboxName);
						tpFilter->SaveboxName = alloc_copy_t(new_name);
					}
				}
			}
			for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
				if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->FilterCnt > 0) {
					for (j = 0; j < (MailBox+i)->FilterCnt; j++) {
						FILTER *tpFilter = *((MailBox+i)->tpFilter + j);
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
 * MailboxSummaryAdd - add tpMailBox to summary list
 */
static void MailboxSummaryAdd(int Num, HWND hListView, BOOL newSbox, int Pos)
{
	MAILBOX *tpMailBox = MailBox + Num;
	TCHAR *p;
	TCHAR buf[BUF_SIZE];
	int ItemIndex, dsize;

	p = tpMailBox->Name;
	if (p == NULL) {
		p = STR_MAILBOX_NONAME;
	}
	if (Pos == 0) {
		ItemIndex = ListView_AddOptionItem(hListView, p, 0);
	} else {
		ItemIndex = ListView_InsertItemEx(hListView, p, BUF_SIZE - 1, 0, 0, Pos);
	}
	if (newSbox == TRUE) {
		wsprintf(buf, TEXT("0/0/0"));
	} else if (tpMailBox->Loaded) {
		wsprintf(buf, TEXT("%d/%d/%d"), tpMailBox->NewMail, tpMailBox->UnreadCnt, tpMailBox->MailItemCnt);
	} else {
		wsprintf(buf, TEXT("?"));
	}
	ListView_SetItemText(hListView, ItemIndex, 1, buf);

	dsize = tpMailBox->DiskSize;
	if (dsize < 0) {
		wsprintf(buf, STR_STATUS_MAILSIZE_KB, TEXT("?"));
	} else {
		if (dsize > 0 && dsize < 1000) {
			dsize = 1;
		} else {
			dsize /= 1024;
		}
		wsprintf(buf, STR_STATUS_MAILSIZE_KB_d, dsize);
	}
	ListView_SetItemText(hListView, ItemIndex, 2, buf);

	ListView_SetItemText(hListView, ItemIndex, 3, (tpMailBox->NeedsSave) ? TEXT("YES") : TEXT("no"));
	if (op.LazyLoadMailboxes > 0) {
		ListView_SetItemText(hListView, ItemIndex, 4, (tpMailBox->Loaded) ? TEXT("YES") : TEXT("no"));
	}
	MailboxFilenameUpdate(hListView, ItemIndex, ListView_GetItemCount(hListView)-1);
}

/*
 * MailboxFilenameUpdate - update Filename column after add/delete/move
 */
static void MailboxFilenameUpdate(HWND hListView, int Start, int Stop)
{
	MAILBOX *tpMailBox;
	TCHAR buf[BUF_SIZE];
	int i, max = ListView_GetItemCount(hListView);

	for (i = Start; i <= Stop && i< max; i++) {
		tpMailBox = MailBox + i + MAILBOX_USER;
		if (tpMailBox->Filename == NULL) {
#ifdef _DEBUG
			wsprintf(buf, TEXT("[MailBox%d.dat]"), i);
#else
			wsprintf(buf, TEXT("MailBox%d.dat"), i);
#endif
		} else {
			wsprintf(buf, tpMailBox->Filename);
		}
		ListView_SetItemText(hListView, i, ((op.LazyLoadMailboxes > 0) ? 5 : 4), buf);
	}
}

/*
 * MailBoxSummaryProc
 */
BOOL CALLBACK MailBoxSummaryProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hListView;
	MAILBOX *tpMailBox;
	TCHAR buf[BUF_SIZE];
	TCHAR *p;
	int i, sel, oldsel, first, cnt, move = 1;
	BOOL ret = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_MAILBOXLIST, TRUE);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.MblColSize[0], STR_MAILBOXES_NAME, 0);
		ListView_AddColumn(hListView, LVCFMT_RIGHT, op.MblColSize[1], 
			((op.MblColSize[1] > 105) ? STR_MAILBOXES_MAILCOUNT : STR_MAILBOXES_MAILCNT), 1);
		ListView_AddColumn(hListView, LVCFMT_RIGHT, op.MblColSize[2], STR_MAILBOXES_DISKSIZE, 2);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.MblColSize[3], STR_MAILBOXES_NEEDS_SAVE, 3);
		if (op.LazyLoadMailboxes > 0) {
			ListView_AddColumn(hListView, LVCFMT_LEFT, op.MblColSize[4], STR_MAILBOXES_LOADED, 4);
			i = 5;
		} else {
			i = 4;
		}
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.MblColSize[5], STR_MAILBOXES_FILENAME, i);
		ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			MailboxSummaryAdd(i, hListView, FALSE, 0);
		}
		if (SelBox >= MAILBOX_USER) {
			ListView_SetItemState(hListView, SelBox - MAILBOX_USER, LVIS_SELECTED, LVIS_SELECTED);
		} else {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 0);
			EnableWindow(GetDlgItem(hDlg, IDOK), 0);
		}
		SetTimer(hDlg, ID_RESIZE_TIMER, 1, NULL);
		break;

	case WM_TIMER:
		if (wParam == ID_RESIZE_TIMER) {
#ifdef _WIN32_WCE_PPC
			RECT rcDlg;
			int width, height, xsize, ysize;
			width = op.MblRect.right - op.MblRect.left;
			height = op.MblRect.bottom - op.MblRect.top;
			xsize = GetSystemMetrics(SM_CXSCREEN);
			ysize = GetSystemMetrics(SM_CYSCREEN);
			// in case of landscape/portrait swap
			if (width > xsize - 10) {
				width = xsize;
			}
			if (height > ysize) {
				height = ysize - 10 - MENU_HEIGHT;
			}
			GetWindowRect(hDlg, &rcDlg);
			if (rcDlg.left > xsize) {
				rcDlg.left = 5;
			}
			if (rcDlg.top > ysize) {
				rcDlg.top = 5;
			}
			MoveWindow(hDlg, rcDlg.left, rcDlg.top,
				width, height, TRUE);
#else
			MoveWindow(hDlg, op.MblRect.left, op.MblRect.top, 
				op.MblRect.right - op.MblRect.left,
				op.MblRect.bottom - op.MblRect.top, TRUE);
#endif
		}
		KillTimer(hDlg, wParam);
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

	case WM_SIZE:
		if (hDlg != NULL) {
			RECT rcWin;
			GetClientRect(hDlg, &rcWin);
			SetWindowSize(hDlg, IDC_LIST_MAILBOXES, rcWin.top, rcWin.bottom,
				rcWin.left, rcWin.right);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_NOTIFY:
		DialogLvNotifyProc(hDlg, lParam, IDC_LIST_MAILBOXES);
		{
			LV_DISPINFO *plv = (LV_DISPINFO *)lParam;
			if (plv->hdr.code == LVN_ITEMCHANGED) {
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 1);
				EnableWindow(GetDlgItem(hDlg, IDOK), 1);
			}
		}
		break;

	case WM_LV_EVENT:
		if (wParam == NM_CLICK) {
			BOOL en, en1 = FALSE;
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
			sel = ListView_GetSelectedCount(hListView);
			if (sel <= 0) {
				en = FALSE;
			} else {
				en = TRUE;
			}
			if (sel == 1) {
				en1 = TRUE;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), en);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), en);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), en);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), en);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), en1);
			EnableWindow(GetDlgItem(hDlg, IDOK), en1);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_LV_ALLSELECT:
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_MAILBOXES), -1, LVIS_SELECTED, LVIS_SELECTED);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), 1);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 1);
			EnableWindow(GetDlgItem(hDlg, IDOK), 0);
			break;

		case IDC_BUTTON_UP10:
			move = 10;
		case IDC_BUTTON_UP:
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
			if ((cnt = ListView_GetSelectedCount(hListView)) <= 0) {
				break;
			}
			sel = -1;
			oldsel = SelBox;
			first = ListView_GetNextItem(hListView, -1, LVIS_SELECTED) - 1;
			while ( (sel = ListView_GetNextItem(hListView, sel, LVIS_SELECTED)) >= 0) {
				SelBox = sel + 1;
				mailbox_move_up(hDlg, FALSE);
				ListView_MoveItem(hListView, sel, -1 * move, (op.LazyLoadMailboxes > 0) ? 5 : 4);
			}
			MailboxFilenameUpdate(hListView, first, first+move+cnt-1);
			SelBox = oldsel;
			break;

		case IDC_BUTTON_DOWN10:
			move = 10;
		case IDC_BUTTON_DOWN:
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
			if ((cnt = ListView_GetSelectedCount(hListView)) <= 0) {
				break;
			}
			sel = -1;
			oldsel = SelBox;
			first = ListView_GetNextItem(hListView, -1, LVIS_SELECTED);
			for (sel = ListView_GetItemCount(hListView) - 1; sel >= 0; sel--) {
				if (ListView_GetItemState(hListView, sel, LVNI_SELECTED) == LVNI_SELECTED) {
					SelBox = sel + 1;
					mailbox_move_down(hDlg, FALSE);
					ListView_MoveItem(hListView, sel, move, (op.LazyLoadMailboxes > 0) ? 5 : 4);
				}
			}
			MailboxFilenameUpdate(hListView, first, first+move+cnt-1);
			SelBox = oldsel;
			break;

		case IDC_BUTTON_ADD:
			if (MailBoxCnt >= MAX_MAILBOX_CNT) {
				ErrorMessage(hDlg, STR_ERR_TOOMANYMAILBOXES);
				break;
			}
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
			sel = -1;
			while( (i = ListView_GetNextItem(hListView, sel, LVIS_SELECTED)) >= 0) {
				sel = i;
			}
			oldsel = SelBox;
			SelBox = mailbox_create(hDlg, 1, sel + 1 + MAILBOX_USER, TRUE, FALSE);
			if (SelBox == -1) {
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}
			i = SetMailBoxType(hDlg, 0);
			(MailBox+SelBox)->NewMail = 1; // hack to force correct name into IDC_MBMENU
			ret = TRUE;
			if (i == -1 || (i == 0 && SetMailBoxOption(hDlg) == FALSE)) {
				mailbox_delete(hDlg, SelBox, FALSE, FALSE);
				ret = FALSE;
			} else if (i == MAILBOX_IMPORT_SAVE) {
				if (ImportSavebox(hDlg) == FALSE) {
					mailbox_delete(hDlg, SelBox, FALSE, FALSE);
					ret = FALSE;
				}
				(MailBox+SelBox)->Type = MAILBOX_TYPE_SAVE;
				(MailBox+SelBox)->NeedsSave = MAILITEMS_CHANGED;
			}
			if (ret == TRUE) {
				MailboxSummaryAdd(SelBox, hListView, (i != MAILBOX_IMPORT_SAVE), sel+1);
			}
			SelBox = oldsel;
			if (ret == TRUE && op.AutoSave == 1) {
				SwitchCursor(FALSE);
				ini_save_setting(hDlg, FALSE, FALSE, NULL);
				SwitchCursor(TRUE);
			}
			break;

		case IDC_BUTTON_SAVE:
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				break;
			}
			sel = -1;
			while( (sel = ListView_GetNextItem(hListView, sel, LVIS_SELECTED)) >= 0) {
				int mbox = sel + MAILBOX_USER;
				int dsize;
				MAILBOX *tpMailBox = MailBox + mbox;
				if (tpMailBox->Filename == NULL) {
					wsprintf(buf, TEXT("MailBox%d.dat"), sel);
				} else {
					lstrcpy(buf, tpMailBox->Filename);
				}
				file_save_mailbox(buf, DataDir, mbox, FALSE, FALSE,
					(tpMailBox->Type == MAILBOX_TYPE_SAVE) ? 2 : op.ListSaveMode);

				dsize = tpMailBox->DiskSize;
				if (dsize < 0) {
					wsprintf(buf, STR_STATUS_MAILSIZE_KB, TEXT("?"));
				} else {
					if (dsize > 0 && dsize < 1000) {
						dsize = 1;
					} else {
						dsize /= 1024;
					}
					wsprintf(buf, STR_STATUS_MAILSIZE_KB_d, dsize);
				}
				ListView_SetItemText(hListView, sel, 2, buf);
				ListView_SetItemText(hListView, sel, 3, (tpMailBox->NeedsSave) ? TEXT("YES") : TEXT("no"));
				if (op.LazyLoadMailboxes > 0) {
					ListView_SetItemText(hListView, sel, 4, (tpMailBox->Loaded) ? TEXT("YES") : TEXT("no"));
				}
			}
			break;

		case IDC_BUTTON_EDIT:
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
			if (ListView_GetSelectedCount(hListView) != 1) {
				break;
			}
			sel = ListView_GetNextItem(hListView, -1, LVIS_SELECTED);
			oldsel = SelBox;
			SelBox = sel + MAILBOX_USER;
			tpMailBox = MailBox + SelBox;
			if (tpMailBox->Type == MAILBOX_TYPE_SAVE) {
				ret = SetSaveBoxName(hDlg);
			} else {
				ret = SetMailBoxOption(hDlg);
			}
			if (ret == TRUE) {
				p = tpMailBox->Name;
				if (p == NULL) {
					p = STR_MAILBOX_NONAME;
				}
				ListView_SetItemText(hListView, sel, 0, p);
				SelBox = oldsel;
				SelectMBMenu(SelBox);
				if (op.AutoSave == 1) {
					SwitchCursor(FALSE);
					ini_save_setting(hDlg, FALSE, FALSE, NULL);
					SwitchCursor(TRUE);
				}
			}
			break;

		case IDC_BUTTON_DELETE:
		case ID_LV_DELETE:
			oldsel = SelBox;
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);
			sel = ListView_GetSelectedCount(hListView);
			if (sel <= 0) {
				break;
			} else {
				if (sel == 1) {
					sel = ListView_GetNextItem(hListView, -1, LVIS_SELECTED) + MAILBOX_USER;
					wsprintf(buf, STR_Q_DELMAILBOX, ((MailBox+sel)->Name) ? (MailBox+sel)->Name : STR_MAILBOX_NONAME);
				} else {
					wsprintf(buf, STR_Q_DELMAILBOXES, sel);
				}
				if (MessageBox(hDlg, buf, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
					break;
				}
			}
			first = ListView_GetNextItem(hListView, -1, LVIS_SELECTED);
			while( (sel = ListView_GetNextItem(hListView, -1, LVIS_SELECTED)) >= 0) {
				if (oldsel == sel + MAILBOX_USER) {
					oldsel = -1;
				}
				mailbox_delete(hDlg, sel + MAILBOX_USER, TRUE, FALSE);
				ListView_DeleteItem(hListView, sel);
			}
			MailboxFilenameUpdate(hListView, first, ListView_GetItemCount(hListView)-1);
			if (oldsel >= 0) {
				SelectMBMenu(oldsel);
			}
			// update button status
			SendMessage(hDlg, WM_LV_EVENT, NM_CLICK, 0);
			break;

		case ID_LV_EDIT:
			if (ListView_GetNextItem(GetDlgItem(hDlg, IDC_LIST_MAILBOXES), -1,
				LVNI_FOCUSED | LVIS_SELECTED) == -1) {
				break;
			} // fall through
		case IDOK: // Go to - select mailbox
			ret = TRUE;
			// fall through
		case IDCANCEL: // Done
			hListView = GetDlgItem(hDlg, IDC_LIST_MAILBOXES);

			for (i = 0; i < MB_COL_CNT-2; i++) {
				op.MblColSize[i] = ListView_GetColumnWidth(hListView, i);
			}
			if (op.LazyLoadMailboxes > 0) {
				op.MblColSize[MB_COL_CNT-2] = ListView_GetColumnWidth(hListView, MB_COL_CNT-2);
				op.MblColSize[MB_COL_CNT-1] = ListView_GetColumnWidth(hListView, MB_COL_CNT-1);
			} else {
				// op.MblColSize[MB_COL_CNT-2] unchanged
				op.MblColSize[MB_COL_CNT-1] = ListView_GetColumnWidth(hListView, MB_COL_CNT-2);
			}
			GetWindowRect(hDlg, &op.MblRect);

			if (ret == TRUE) {
				sel = ListView_GetNextItem(hListView, -1, LVIS_SELECTED);
				mailbox_select(hDlg, sel + MAILBOX_USER);
			}

			EndDialog(hDlg, TRUE);
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
		DeleteMBMenu(SelBox);
		InsertMBMenu(SelBox, new_name);
	}
	mailbox_select(hWnd, SelBox);
	return TRUE;
}

/*
 * SetRecvOptionProc - 受信設定プロシージャ
 */
static BOOL CALLBACK SetRecvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int newlsm = 0;
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
				newlsm = 0;
			} else if (SendDlgItemMessage(hDlg, IDC_RADIO_HEADSAVE, BM_GETCHECK, 0, 0) == 1) {
				newlsm = 1;
			} else if (SendDlgItemMessage(hDlg, IDC_RADIO_ALLSAVE, BM_GETCHECK, 0, 0) == 1) {
				newlsm = 2;
			}
			if (op.ListSaveMode != newlsm) {
				int i;
				for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
					if ((MailBox + i)->Type != MAILBOX_TYPE_SAVE) {
						(MailBox + i)->NeedsSave |= MAILITEMS_CHANGED;
					}
				}
				op.ListSaveMode = newlsm;
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
		InitDlg(hDlg, STR_TITLE_ENCODE, TRUE);
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
		if (tpMailItem == NULL) {
			SendDlgItemMessage(hDlg, IDC_CHECK_ENCODETYPE, BM_SETCHECK, op.EncodeType, 0);
		} else {
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_ENCODETYPE), SW_HIDE);
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
				op.EncodeType = SendDlgItemMessage(hDlg, IDC_CHECK_ENCODETYPE, BM_GETCHECK, 0, 0);
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
		SendDlgItemMessage(hDlg, IDC_CHECK_SELECTSENDBOX, BM_SETCHECK, op.SelectSendBox, 0);
#ifdef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_ATTACHSEP, BM_SETCHECK, op.SendAttachIndividually, 0);
#endif
		SendDlgItemMessage(hDlg, IDC_CHECK_QUEUE_EXIT, BM_SETCHECK, op.CheckQueuedOnExit, 0);
		if (op.CheckQueuedOnExit == 0) {
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SENDERR_EXIT), 0);
		} else if (op.CheckQueuedOnExit == 2) {
			SendDlgItemMessage(hDlg, IDC_CHECK_SENDERR_EXIT, BM_SETCHECK, 1, 0);
		}

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
		case IDC_CHECK_QUEUE_EXIT:
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SENDERR_EXIT), SendDlgItemMessage(hDlg, IDC_CHECK_QUEUE_EXIT, BM_GETCHECK, 0, 0));
			break;

		case IDC_BUTTON_ENCODE:
			// エンコード設定
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ENCODE), hDlg, SetEncodeProc, (LPARAM)0);
			break;

		case IDOK:
			op.WordBreakSize = GetDlgItemInt(hDlg, IDC_EDIT_BREAKCNT, NULL, FALSE);
			op.QuotationBreak = SendDlgItemMessage(hDlg, IDC_CHECK_QBREAK, BM_GETCHECK, 0, 0);

			op.SendMessageId = SendDlgItemMessage(hDlg, IDC_CHECK_SENDMESSAGEID, BM_GETCHECK, 0, 0);
			op.SendDate = SendDlgItemMessage(hDlg, IDC_CHECK_SENDDATE, BM_GETCHECK, 0, 0);
			op.SelectSendBox = SendDlgItemMessage(hDlg, IDC_CHECK_SELECTSENDBOX, BM_GETCHECK, 0, 0);
#ifdef _WIN32_WCE
			op.SendAttachIndividually = SendDlgItemMessage(hDlg, IDC_CHECK_ATTACHSEP, BM_GETCHECK, 0, 0);
#endif
			op.CheckQueuedOnExit = SendDlgItemMessage(hDlg, IDC_CHECK_QUEUE_EXIT, BM_GETCHECK, 0, 0);
			if (op.CheckQueuedOnExit) {
				op.CheckQueuedOnExit += SendDlgItemMessage(hDlg, IDC_CHECK_SENDERR_EXIT, BM_GETCHECK, 0, 0);
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
 * SetUpdateOptionProc - Update after Check options
 */
static BOOL CALLBACK SetUpdateOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		if (op.CheckEndExecNoDelMsg == 2) {
			SendDlgItemMessage(hDlg, IDC_RADIO_UAC_KEEP, BM_SETCHECK, 1, 0);
		} else if (op.CheckEndExecNoDelMsg == 1) {
			SendDlgItemMessage(hDlg, IDC_RADIO_UAC_ASK, BM_SETCHECK, 1, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_RADIO_UAC_DELETE, BM_SETCHECK, 1, 0);
		}
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			if (SendDlgItemMessage(hDlg, IDC_RADIO_UAC_KEEP, BM_GETCHECK, 0, 0)) {
				op.CheckEndExecNoDelMsg = 2;
			} else if (SendDlgItemMessage(hDlg, IDC_RADIO_UAC_ASK, BM_GETCHECK, 0, 0)) {
				op.CheckEndExecNoDelMsg = 1;
			} else {
				op.CheckEndExecNoDelMsg = 0;
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
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_REHEAD),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
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
	BOOL enable = TRUE;
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SetControlFont(hDlg);
		if (op.FwdQuotation == 1) {
			// quoted
			SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_0, BM_SETCHECK, 1, 0);
		} else if (op.FwdQuotation == 2) {
			// as attachment
			SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_2, BM_SETCHECK, 1, 0);
			enable = FALSE;
		} else {
			// inline
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
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FWDHEAD), enable);
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_FWDHEAD),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
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

		case IDC_QUOTSTYLE_0:
		case IDC_QUOTSTYLE_1:
		case IDC_QUOTSTYLE_2:
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_FWDHEAD), 
				!SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_2, BM_GETCHECK, 0, 0));
			break;

		case IDC_SIGNFWD:
			EnableWindow(GetDlgItem(hDlg, IDC_SIGNFWD_ABOVE), 
				SendDlgItemMessage(hDlg, IDC_SIGNFWD, BM_GETCHECK, 0, 0));
			break;

		case IDOK:
			op.FwdQuotation = SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_0, BM_GETCHECK, 0, 0)
				+ 2 * SendDlgItemMessage(hDlg, IDC_QUOTSTYLE_2, BM_GETCHECK, 0, 0);

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
		SendDlgItemMessage(hDlg, IDC_CHECK_UPDATEAFTERCHECK, BM_SETCHECK, op.CheckEndExec, 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPDATEOPTS), op.CheckEndExec);
		
		SendDlgItemMessage(hDlg, IDC_CHECK_IGNOREERROR, BM_SETCHECK, op.SocIgnoreError, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SENDIGNOREERROR, BM_SETCHECK, op.SendIgnoreError, 0);
		//if (op.SocIgnoreError == 0 && op.SendIgnoreError == 0) {
		//	EnableWindow(GetDlgItem(hDlg, IDC_EDIT_NOIGNORETIME), FALSE);
		//}
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
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FILE_BROWSE),
				SendDlgItemMessage(hDlg, IDC_CHECK_SOUND, BM_GETCHECK, 0, 0));
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SOUND_PLAY),
				SendDlgItemMessage(hDlg, IDC_CHECK_SOUND, BM_GETCHECK, 0, 0));
			break;

		case IDC_BUTTON_FILE_BROWSE:
			*buf = TEXT('\0');
			if (filename_select(hDlg, buf, TEXT("wav"), STR_WAVE_FILTER, FILE_OPEN_SINGLE, &op.SavedOpenDir) == FALSE) {
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

		case IDC_CHECK_UPDATEAFTERCHECK:
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UPDATEOPTS),
				SendDlgItemMessage(hDlg, IDC_CHECK_UPDATEAFTERCHECK, BM_GETCHECK, 0, 0));
			break;

		case IDC_BUTTON_UPDATEOPTS:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_UPDATEOPTS), hDlg, SetUpdateOptionProc, (LPARAM)0);
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
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
			CheckMenuItem(SHGetSubMenu(hMainToolBar, ID_MENUITEM_FILE), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
			CheckMenuItem(CommandBar_GetMenu(GetDlgItem(MainWnd, IDC_CB), 0), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif
#else
			CheckMenuItem(GetMenu(MainWnd), ID_MENUITEM_AUTOCHECK, (op.AutoCheck == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

			op.StartCheck = SendDlgItemMessage(hDlg, IDC_CHECK_STARTCHECK, BM_GETCHECK, 0, 0);
			op.CheckAfterUpdate = SendDlgItemMessage(hDlg, IDC_CHECK_CHECKAFTERUPDATE, BM_GETCHECK, 0, 0);
			op.CheckEndExec = SendDlgItemMessage(hDlg, IDC_CHECK_UPDATEAFTERCHECK, BM_GETCHECK, 0, 0);

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
 * SetViewOptionProc - various view window controls
 */
static BOOL CALLBACK SetViewOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL retval = TRUE;
	switch (uMsg) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_CHECK_SCANALLFORUNREAD, BM_SETCHECK, op.ScanAllForUnread, 0);
		if (op.ViewNextAfterDel != 0) {
			SendDlgItemMessage(hDlg, IDC_VIEWNEXTDEL_NEXT, BM_SETCHECK, 1, 0);
			if (op.ViewNextAfterDel == 2) {
				SendDlgItemMessage(hDlg, IDC_VIEWNEXTDEL_UNREAD, BM_SETCHECK, 1, 0);
			}
		} else {
			EnableWindow(GetDlgItem(hDlg, IDC_VIEWNEXTDEL_UNREAD), 0);
		}
		if (op.ViewSkipDeleted == 1) {
			SendDlgItemMessage(hDlg, IDC_VIEWSKIPDEL_ALWAYS, BM_SETCHECK, 1, 0);
		} else if (op.ViewSkipDeleted == 2) {
			SendDlgItemMessage(hDlg, IDC_VIEWSKIPDEL_UNREAD, BM_SETCHECK, 1, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_VIEWSKIPDEL_NEVER, BM_SETCHECK, 1, 0);
		}
		SendDlgItemMessage(hDlg, IDC_VIEWCLOSENONEXT, BM_SETCHECK, op.ViewCloseNoNext, 0);
		SendDlgItemMessage(hDlg, IDC_VIEWSHOWATTACH, BM_SETCHECK, op.ViewShowAttach, 0);
		SendDlgItemMessage(hDlg, IDC_VIEWAPP_MSGSRC, BM_SETCHECK, op.ViewAppMsgSource, 0);
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_VIEWWND_CURSOR, BM_SETCHECK, op.ViewWindowCursor, 0);
#endif
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_VIEWNEXTDEL_NEXT:
			EnableWindow(GetDlgItem(hDlg, IDC_VIEWNEXTDEL_UNREAD), 
				SendDlgItemMessage(hDlg, IDC_VIEWNEXTDEL_NEXT, BM_GETCHECK, 0, 0));
			
		case IDOK:
			op.ScanAllForUnread = SendDlgItemMessage(hDlg, IDC_CHECK_SCANALLFORUNREAD, BM_GETCHECK, 0, 0);
			if (SendDlgItemMessage(hDlg, IDC_VIEWNEXTDEL_NEXT, BM_GETCHECK, 0, 0) == 1) {
				op.ViewNextAfterDel = 1 + SendDlgItemMessage(hDlg, IDC_VIEWNEXTDEL_UNREAD, BM_GETCHECK, 0, 0);
			} else {
				op.ViewNextAfterDel = 0;
			}
			if (SendDlgItemMessage(hDlg, IDC_VIEWSKIPDEL_ALWAYS, BM_GETCHECK, 0, 0) == 1) {
				op.ViewSkipDeleted = 1;
			} else if (SendDlgItemMessage(hDlg, IDC_VIEWSKIPDEL_UNREAD, BM_GETCHECK, 0, 0) == 1) {
				op.ViewSkipDeleted = 2;
			} else {
				op.ViewSkipDeleted = 0;
			}
			op.ViewCloseNoNext = SendDlgItemMessage(hDlg, IDC_VIEWCLOSENONEXT, BM_GETCHECK, 0, 0);
			op.ViewShowAttach = SendDlgItemMessage(hDlg, IDC_VIEWSHOWATTACH, BM_GETCHECK, 0, 0);
			op.ViewAppMsgSource = SendDlgItemMessage(hDlg, IDC_VIEWAPP_MSGSRC, BM_GETCHECK, 0, 0);
#ifndef _WIN32_WCE
			op.ViewWindowCursor = SendDlgItemMessage(hDlg, IDC_VIEWWND_CURSOR, BM_GETCHECK, 0, 0);
#endif
			break;
		}
		break;

	default:
		retval = FALSE;
	}
	return retval;
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
		if (lstrcmp(op.LvColumnOrder, TEXT("FSDZ")) == 0) {
			SendDlgItemMessage(hDlg, IDC_COL_FSDZ, BM_SETCHECK, 1, 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_COL_SFDZ, BM_SETCHECK, 1, 0);
		}

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

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

			mem_free(&op.LvColumnOrder);
			if (SendDlgItemMessage(hDlg, IDC_COL_SFDZ, BM_GETCHECK, 0, 0) == 1) {
				op.LvColumnOrder = alloc_copy_t(TEXT("SFDZ"));
			} else {
				op.LvColumnOrder = alloc_copy_t(TEXT("FSDZ"));
			}

			//GJC redraw window if sort order changed
			if (redraw) {
				mailbox_select(MainWnd, GetSelectedMBMenu());
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
		case IDC_COL_FSDZ:
		case IDC_COL_SFDZ:
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
#ifdef _WIN32_WCE
	BOOL poomcheck;
#endif
	switch (uMsg) {
	case WM_INITDIALOG:
		/* コントロールの初期化 */
		SendDlgItemMessage(hDlg, IDC_CHECK_SHOWTRAYICON, BM_SETCHECK, op.ShowTrayIcon, 0);
#ifndef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_MINSIZEHIDE, BM_SETCHECK, op.MinsizeHide, 0);
#endif
		SendDlgItemMessage(hDlg, IDC_CHECK_CLOSEHIDE, BM_SETCHECK, op.CloseHide, 0);

		SendDlgItemMessage(hDlg, IDC_CHECK_PARANOID, BM_SETCHECK, op.ExpertMode, 0);		// Added PHH 4-Oct-2003

#ifdef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_USEPOOM, BM_SETCHECK, (op.UsePOOMAddressBook!=0), 0);
		SendDlgItemMessage(hDlg, IDC_POOMNAMECOMMENT, BM_SETCHECK, op.POOMNameIsComment, 0);
		if (op.UsePOOMAddressBook != 0) {
			SendDlgItemMessage(hDlg, IDC_RADIO_POOMFIRST, BM_SETCHECK, (op.UsePOOMAddressBook==1), 0);
			SendDlgItemMessage(hDlg, IDC_RADIO_POOMLAST, BM_SETCHECK, (op.UsePOOMAddressBook==2), 0);
		} else {
			SendDlgItemMessage(hDlg, IDC_RADIO_POOMFIRST, BM_SETCHECK, 1, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_POOMFIRST), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_POOMLAST), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_POOMNAMECOMMENT), 0);
		}
#endif // _WIN32_WCE
#ifndef _WIN32_WCE
		// on advanced tab for PPC
		SendDlgItemMessage(hDlg, IDC_CHECK_STARTPASS, BM_SETCHECK, op.StartPass, 0);
		SendDlgItemMessage(hDlg, IDC_CHECK_SHOWPASS, BM_SETCHECK, op.ShowPass, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0, (LPARAM)op.Password);

		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MINSIZEHIDE), op.ShowTrayIcon);
#endif // _WIN32_WCE
		EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CLOSEHIDE), op.ShowTrayIcon);

		SendDlgItemMessage(hDlg, IDC_DATE_FORMAT, WM_SETTEXT, 0, (LPARAM)op.DateFormat);
		SendDlgItemMessage(hDlg, IDC_TIME_FORMAT, WM_SETTEXT, 0, (LPARAM)op.TimeFormat);
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_DATE_FORMAT),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_TIME_FORMAT),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

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

#ifdef _WIN32_WCE
		case IDC_CHECK_USEPOOM:
			poomcheck = SendDlgItemMessage(hDlg, IDC_CHECK_USEPOOM, BM_GETCHECK, 0, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_POOMFIRST), poomcheck);
			EnableWindow(GetDlgItem(hDlg, IDC_RADIO_POOMLAST), poomcheck);
			EnableWindow(GetDlgItem(hDlg, IDC_POOMNAMECOMMENT), poomcheck);
			break;
#endif

		case IDOK:
			op.ShowTrayIcon = SendDlgItemMessage(hDlg, IDC_CHECK_SHOWTRAYICON, BM_GETCHECK, 0, 0);
#ifndef _WIN32_WCE
			op.MinsizeHide = SendDlgItemMessage(hDlg, IDC_CHECK_MINSIZEHIDE, BM_GETCHECK, 0, 0);
#endif
			op.CloseHide = SendDlgItemMessage(hDlg, IDC_CHECK_CLOSEHIDE, BM_GETCHECK, 0, 0);

			op.ExpertMode = SendDlgItemMessage(hDlg, IDC_CHECK_PARANOID, BM_GETCHECK, 0, 0);	// Added PHH 4-Oct-2003

#ifdef _WIN32_WCE
			if (SendDlgItemMessage(hDlg, IDC_CHECK_USEPOOM, BM_GETCHECK, 0, 0)) {
				int newsort, newcmmt;
				newsort = (SendDlgItemMessage(hDlg, IDC_RADIO_POOMFIRST, BM_GETCHECK, 0, 0) == 1) ? 1 : 2;
				newcmmt = SendDlgItemMessage(hDlg, IDC_POOMNAMECOMMENT, BM_GETCHECK, 0, 0);
				if (op.UsePOOMAddressBook == 0
					&& AddressBook->ItemCnt > 0
					&& MessageBox(hDlg, STR_Q_USEPOOM, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
						op.UsePOOMAddressBook = 0;
				} else if (op.UsePOOMAddressBook != newsort || op.POOMNameIsComment != newcmmt) {
					ADDRESSBOOK *tpTmpAddressBook = (ADDRESSBOOK *)mem_calloc(sizeof(ADDRESSBOOK));
					if (tpTmpAddressBook != NULL) {
						int read;
						op.UsePOOMAddressBook = newsort;
						op.POOMNameIsComment  = newcmmt;
						SwitchCursor(FALSE);
						read = file_read_address_book(ADDRESS_FILE, tpTmpAddressBook);
						SwitchCursor(TRUE);
						if (read < -50) {
							MessageBox(hDlg, STR_ERR_POOM, WINDOW_TITLE, MB_OK);
							read += 100;
						}
						if (read < 0) {
							addressbook_free(tpTmpAddressBook);
						} else {
							addressbook_free(AddressBook);
							AddressBook = tpTmpAddressBook;
						}
					}
				}
			} else {
				op.UsePOOMAddressBook = 0;
			}
#endif

#ifndef _WIN32_WCE
			// on advanced tab for PPC
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
 * SetAdvOptionProc - Advanced Options callback
 */
static BOOL CALLBACK SetAdvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL retval = TRUE;
	BOOL enable;
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE
		SendDlgItemMessage(hDlg, IDC_CHECK_STARTPASS, BM_SETCHECK, op.StartPass, 0);
		SendDlgItemMessage(hDlg, IDC_EDIT_PASS, WM_SETTEXT, 0, (LPARAM)op.Password);
#endif // _WIN32_WCE

		if (op.WriteMbox == 1) {
			SendDlgItemMessage(hDlg, IDC_RADIO_FORMAT_MBOX, BM_SETCHECK, 1, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SCRAMBLE), FALSE);
		} else {
			SendDlgItemMessage(hDlg, IDC_RADIO_FORMAT_NPOP, BM_SETCHECK, 1, 0);
			if (op.ScrambleMailboxes == 1) {
				SendDlgItemMessage(hDlg, IDC_CHECK_SCRAMBLE, BM_SETCHECK, 1, 0);
			}
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_LAZYLOAD, CB_ADDSTRING, 0, (LPARAM)STR_LAZYLOAD_STARTUP);
		SendDlgItemMessage(hDlg, IDC_COMBO_LAZYLOAD, CB_ADDSTRING, 0, (LPARAM)STR_LAZYLOAD_AUTO);
		SendDlgItemMessage(hDlg, IDC_COMBO_LAZYLOAD, CB_ADDSTRING, 0, (LPARAM)STR_LAZYLOAD_AUTO_NOSEARCH);
		SendDlgItemMessage(hDlg, IDC_COMBO_LAZYLOAD, CB_ADDSTRING, 0, (LPARAM)STR_LAZYLOAD_PROMPT);
		SendDlgItemMessage(hDlg, IDC_COMBO_LAZYLOAD, CB_SETCURSEL, op.LazyLoadMailboxes, 0);
		break;

	case WM_NOTIFY:
		return OptionNotifyProc(hDlg, uMsg, wParam, lParam);

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_PASS:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_RADIO_FORMAT_NPOP:
		case IDC_RADIO_FORMAT_MBOX:
			enable = SendDlgItemMessage(hDlg, IDC_RADIO_FORMAT_NPOP, BM_GETCHECK, 0, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SCRAMBLE), enable);
			if (enable && op.ScrambleMailboxes == 1) {
				SendDlgItemMessage(hDlg, IDC_CHECK_SCRAMBLE, BM_SETCHECK, 1, 0);
			}
			break;

		case IDOK:
#ifdef _WIN32_WCE
			op.StartPass = SendDlgItemMessage(hDlg, IDC_CHECK_STARTPASS, BM_GETCHECK, 0, 0);
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &op.Password);
#endif
			{
				int scramble = 0;
				op.WriteMbox = SendDlgItemMessage(hDlg, IDC_RADIO_FORMAT_MBOX, BM_GETCHECK, 0, 0);
				if (op.WriteMbox == 0) {
					scramble = SendDlgItemMessage(hDlg, IDC_CHECK_SCRAMBLE, BM_GETCHECK, 0, 0);
				}
				if (scramble != op.ScrambleMailboxes) {
					op.ScrambleMailboxes = scramble;
					if (MessageBox(hDlg, STR_Q_SCRAMBLE_SAVE, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDYES) {
						int mbox;
						for (mbox = 0; mbox < MailBoxCnt; mbox++) {
							MAILBOX *tpMailBox = MailBox + mbox;
							if (tpMailBox->Loaded == TRUE || mailbox_load_now(hDlg, mbox, FALSE, FALSE) == 1) {
								TCHAR fname[BUF_SIZE];
								if (mbox == MAILBOX_SEND) {
									lstrcpy(fname, SENDBOX_FILE);
								} else if (tpMailBox->Filename == NULL) {
									wsprintf(fname, TEXT("MailBox%d.dat"), mbox - MAILBOX_USER);
								} else {
									lstrcpy(fname, tpMailBox->Filename);
								}
								file_save_mailbox(fname, DataDir, mbox, FALSE, FALSE,
									(tpMailBox->Type == MAILBOX_TYPE_SAVE) ? 2 : op.ListSaveMode);
							}
						}
					}
				}
			}
			op.LazyLoadMailboxes = SendDlgItemMessage(hDlg, IDC_COMBO_LAZYLOAD, CB_GETCURSEL, 0, 0);
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
	HPROPSHEETPAGE hpsp[11];

	tpOptionMailBox = NULL;
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

	//Filter
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_FILTER);
	psp.pfnDlgProc = FilterSetProc;
	hpsp[4] = CreatePropertySheetPage(&psp);

	//Control
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_VIEW);
	psp.pfnDlgProc = SetViewOptionProc;
	hpsp[5] = CreatePropertySheetPage(&psp);

	//Check
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_CHECK);
	psp.pfnDlgProc = SetCheckOptionProc;
	hpsp[6] = CreatePropertySheetPage(&psp);

	//Dial up
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_RAS);
	psp.pfnDlgProc = SetRasOptionProc;
	hpsp[7] = CreatePropertySheetPage(&psp);

	//Sort
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_SORT);
	psp.pfnDlgProc = SetSortOptionProc;
	hpsp[8] = CreatePropertySheetPage(&psp);

	//In addition
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_ETC);
	psp.pfnDlgProc = SetEtcOptionProc;
	hpsp[9] = CreatePropertySheetPage(&psp);

	//Advanced
	psp.pszTemplate = MAKEINTRESOURCE(IDD_DIALOG_OPTION_ADV);
	psp.pfnDlgProc = SetAdvOptionProc;
	hpsp[10] = CreatePropertySheetPage(&psp);

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
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_PASS), &g_Pass);
			if (gPassSt == 0) {
				if (g_Pass == NULL || lstrcmp(op.Password, g_Pass) != 0) {
					ErrorMessage(hDlg, STR_ERR_SOCK_BADPASSWORD);
					break;
				}
			} else {
				gPassSt = SendDlgItemMessage(hDlg, IDC_CHECK_TMPPASS, BM_GETCHECK, 0, 0);
			}
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
	TCHAR decpt[4];
	TCHAR *p;
	BOOL St;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_INITMAILBOX, TRUE);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		SendDlgItemMessage(hDlg, IDC_RADIO_FIRSTGET, BM_SETCHECK, 1, 0);
		SetDlgItemInt(hDlg, IDC_EDIT_GETNUM, 1, FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GETNUM), 0);
		
		SelMailBox = (MAILBOX *)lParam;

		//Mail several
		wsprintf(buf, STR_STATUS_INIT_MAILCNT, SelMailBox->MailCnt);
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MAILCNT), buf);

		//Mail size
		wsprintf(buf, TEXT("%d"), (SelMailBox->MailSize < 1024) ? SelMailBox->MailSize : SelMailBox->MailSize / 1024);
		if (GetNumberFormat(LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, buf, NULL, tmp, BUF_SIZE - 1) != 0) {
			GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, (LPTSTR)decpt, 4);
			for (p = tmp; *p != TEXT('\0') && *p != decpt[0]; p++);
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
		case IDC_RADIO_FILLIN:
			St = !SendDlgItemMessage(hDlg, IDC_RADIO_FIRSTGET, BM_GETCHECK, 0, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_GETNUM), St);
			break;

		case IDOK:
			SelMailBox = (MAILBOX *)GetWindowLong(hDlg, GWL_USERDATA);
			if (SelMailBox->NoUIDL == TRUE
				&& SendDlgItemMessage(hDlg, IDC_RADIO_FILLIN, BM_GETCHECK, 0, 0) == 1) {
				SendDlgItemMessage(hDlg, IDC_RADIO_FILLIN, BM_SETCHECK, 0, 0);
				SendDlgItemMessage(hDlg, IDC_RADIO_SETGET, BM_SETCHECK, 1, 0);
				ErrorMessage(hDlg, STR_ERR_NOUIDLNOFILL);
				break;
			}
			if (SendDlgItemMessage(hDlg, IDC_RADIO_FIRSTGET, BM_GETCHECK, 0, 0) == 1) {
				SelMailBox->LastNo = -1;
				SelMailBox->ListInitMsg = FALSE;
			} else {
				SelMailBox->LastNo = GetDlgItemInt(hDlg, IDC_EDIT_GETNUM, NULL, FALSE);
				SelMailBox->ListInitMsg = SendDlgItemMessage(hDlg, IDC_RADIO_SETGET, BM_GETCHECK, 0, 0);
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
static int SetCcList(HWND hDlg, TCHAR *strList, TCHAR *type)
{
	TCHAR *buf;
	TCHAR *p ,*r, *s;
	int ItemIndex = 0;

	if (strList == NULL || *strList == TEXT('\0')) {
		return 0;
	}

	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(strList) + 1));
	if (buf == NULL) {
		return 0;
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
			ItemIndex = ListView_AddOptionItem(GetDlgItem(hDlg, IDC_LIST_CC), type, 0);
			ListView_SetItemText(GetDlgItem(hDlg, IDC_LIST_CC), ItemIndex, 1, buf);
		}
		p = (*r != TEXT('\0')) ? r + 1 : r;
	}
	mem_free(&buf);
	return ItemIndex;
}

/*
 * CcListProc - Cc , Bcc 設定プロシージャ
 */
static BOOL CALLBACK CcListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define LV_TITLE_TO		TEXT("To:")
#define LV_TITLE_CC		TEXT("Cc:")
#define LV_TITLE_BCC	TEXT("Bcc:")
	MAILITEM *tpMailItem;
	ADDRESSBOOK *tpTmpAddressBook;
	HWND hListView;
	TCHAR buf[BUF_SIZE], buf2[BUF_SIZE];
	TCHAR *p;
	int SelectItem;
	int ToLen, CcLen, BccLen;
	int i, cnt, cmd;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_CCBCC, TRUE);
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

		if (tpMailItem->MailStatus == ICON_SENTMAIL) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), 0);
		}
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TO), 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CC), 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BCC), 0);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 0);

		// To
		cnt = SetCcList(hDlg, tpMailItem->To, LV_TITLE_TO);
		// Cc
		SetCcList(hDlg, tpMailItem->Cc, LV_TITLE_CC);
		// Bcc
		SetCcList(hDlg, tpMailItem->Bcc, LV_TITLE_BCC);

		ListView_EnsureVisible(hListView, (cnt+1), TRUE);
		SetEditToSubClass(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), TRUE);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_NOTIFY:
		DialogLvNotifyProc(hDlg, lParam, IDC_LIST_CC);
		break;

	case WM_LV_EVENT:
		if (wParam == NM_CLICK) {
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			if (tpMailItem->MailStatus == ICON_SENTMAIL) {
				break;
			}
			hListView = GetDlgItem(hDlg, IDC_LIST_CC);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				i = 0;
			} else {
				i = 1;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), i);
			if (i == 0) {
				i = SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_GETTEXTLENGTH, 0, 0);
				if (i > 0) {
					i = 1;
				}
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TO), i);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CC), i);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BCC), i);
		}
		break;

	case WM_COMMAND:
		cmd = LOWORD(wParam);
		tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
		if (tpMailItem == NULL) {
			EndDialog(hDlg, FALSE);
			break;
		}
		if (cmd == IDCANCEL) {
#ifdef _WIN32_WCE_PPC
			SHSipPreference(hDlg, SIP_DOWN);
#elif defined(_WIN32_WCE_LAGENDA)
			SipShowIM(SIPF_OFF);
#endif
			EndDialog(hDlg, FALSE);
			break;
		}
		if (cmd != IDOK && tpMailItem->MailStatus == ICON_SENTMAIL) {
			break;
		}
		switch (cmd) {
		case IDC_BUTTON_ADD:
			i = -1;
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS));
			// GJC make temporary addressbook for possible editing
			tpTmpAddressBook = addressbook_copy();
			if (tpTmpAddressBook != NULL) {
				tpTmpAddressBook->GetAddrList = TRUE;
				if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS),
					hDlg, AddressListProc, (LPARAM)tpTmpAddressBook) == TRUE
					&& tpTmpAddressBook->AddrList != NULL) {
					SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)tpTmpAddressBook->AddrList);
				}
				addressbook_free(tpTmpAddressBook);
			}
			break;

		case IDC_EDIT_MAILADDRESS:
#ifdef _WIN32_WCE_PPC
			switch (HIWORD(wParam)) {
				case EN_SETFOCUS:
				case CBN_SETFOCUS:
					SHSipPreference(hDlg, SIP_UP);
				break;
			}
#elif defined(_WIN32_WCE_LAGENDA)
			switch (HIWORD(wParam)) {
				case EN_SETFOCUS:
				case CBN_SETFOCUS:
					SipShowIM(SIPF_ON);
				break;
			}
#endif
			if (HIWORD(wParam) != EN_CHANGE) {
				break;
			}
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_CC), -1, 0, LVIS_SELECTED);
			i = SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_GETTEXTLENGTH, 0, 0);
			if (i > 0) {
				i = 1;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TO), i);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CC), i);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BCC), i);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 0);
			break;

		case ID_LV_EDIT:
			hListView = GetDlgItem(hDlg, IDC_LIST_CC);
			if ((SelectItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) == -1) {
				break;
			}
			*buf = TEXT('\0');
			ListView_GetItemText(hListView, SelectItem, 1, buf, BUF_SIZE - 1);
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)buf);

			ListView_DeleteItem(hListView, SelectItem);
			break;

		case IDC_BUTTON_DELETE:
		case ID_LV_DELETE:
			hListView = GetDlgItem(hDlg, IDC_LIST_CC);
			if ((SelectItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED)) == -1) {
				break;
			}
			if (MessageBox(hDlg, STR_Q_DELETE, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			while (SelectItem != -1) {
				ListView_DeleteItem(hListView, SelectItem);
				SelectItem = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 0);
			break;

		case ID_LV_ALLSELECT:
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_CC), -1, LVIS_SELECTED, LVIS_SELECTED);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TO), 1);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CC), 1);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_BCC), 1);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 1);
			break;

		case IDC_BUTTON_TO:
		case IDC_BUTTON_CC:
		case IDC_BUTTON_BCC:
			if (CheckDependence(hDlg, IDC_EDIT_MAILADDRESS, &p) == FALSE) {
				mem_free(&p);
				break;
			}
			if (p == NULL) {
				ErrorMessage(hDlg, STR_ERR_MEMALLOC);
				break;
			}
			if (*p != TEXT('\0')) {
				if (cmd == IDC_BUTTON_BCC) {
					SetCcList(hDlg, p, LV_TITLE_BCC);
				} else if (cmd == IDC_BUTTON_TO) {
					SetCcList(hDlg, p, LV_TITLE_TO);
				} else {
					SetCcList(hDlg, p, LV_TITLE_CC);
				}
			}
			mem_free(&p);
			if (cmd == IDC_BUTTON_BCC) {
				p = alloc_copy_t(LV_TITLE_BCC);
			} else if (cmd == IDC_BUTTON_TO) {
				p = alloc_copy_t(LV_TITLE_TO);
			} else {
				p = alloc_copy_t(LV_TITLE_CC);
			}
			hListView = GetDlgItem(hDlg, IDC_LIST_CC);
			SelectItem = -1;
			while((SelectItem = ListView_GetNextItem(hListView, SelectItem, LVNI_SELECTED)) != -1) {
				ListView_SetItemText(hListView, SelectItem, 0, p);
			}
			mem_free(&p);

			SendMessage(hDlg, DM_SETDEFID, (WPARAM)cmd, 0);
			Button_SetStyle(GetDlgItem(hDlg, IDC_BUTTON_TO),
				((cmd==IDC_BUTTON_TO) ? BS_DEFPUSHBUTTON: BS_PUSHBUTTON), TRUE);
			Button_SetStyle(GetDlgItem(hDlg, IDC_BUTTON_CC),
				((cmd==IDC_BUTTON_CC) ? BS_DEFPUSHBUTTON: BS_PUSHBUTTON), TRUE);
			Button_SetStyle(GetDlgItem(hDlg, IDC_BUTTON_BCC),
				((cmd==IDC_BUTTON_BCC) ? BS_DEFPUSHBUTTON: BS_PUSHBUTTON), TRUE);

			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)TEXT(""));
			SetFocus(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS));
			break;

		case IDOK:
#ifdef _WIN32_WCE_PPC
			SHSipPreference(hDlg, SIP_DOWN);
#elif defined(_WIN32_WCE_LAGENDA)
			SipShowIM(SIPF_OFF);
#endif
			ToLen = CcLen = BccLen = 0;

			if (tpMailItem->MailStatus == ICON_SENTMAIL) {
				EndDialog(hDlg, TRUE);
				break;
			}

			i = SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_GETTEXTLENGTH, 0, 0);
			if (i > 0) {
				ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_CC), -1, 0, LVIS_SELECTED);
				SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_CC, 0);
				break;
			}

			mem_free(&tpMailItem->To);
			tpMailItem->To = NULL;

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
				} else if (lstrcmpi(buf, LV_TITLE_TO) == 0) {
					ToLen += lstrlen(buf2) + 2;
				} else {
					CcLen += lstrlen(buf2) + 4;
				}
			}

			if (ToLen > 0) {
				tpMailItem->To = (TCHAR *)mem_alloc(sizeof(TCHAR) * (ToLen + 1));
				if (tpMailItem->To != NULL) {
					*tpMailItem->To = TEXT('\0');
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
				} else if (lstrcmpi(buf, LV_TITLE_TO) == 0) {
					if (tpMailItem->To == NULL) continue;
					if (*tpMailItem->To != TEXT('\0')) {
						lstrcat(tpMailItem->To, TEXT(", "));
					}
					lstrcat(tpMailItem->To, buf2);
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
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void attach_item_free()
{
	ATTACH_ITEM *a_item = top_attach_item, *next;
	while (a_item) {
		mem_free(&a_item->fname);
		next = a_item->next;
		mem_free(&a_item);
		a_item = next;
	}
	top_attach_item = NULL;
}

/*
 * SetAttachProc - 添付ファイルの設定プロシージャ
 */
BOOL CALLBACK SetAttachProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#define ID_FILE_ADD				(WM_APP + 100)
#define ID_FWD_ADD				(WM_APP + 101)
	MAILITEM *tpMailItem;
	MULTIPART **tpMultiPart = NULL;
	ATTACH_ITEM *a_item = NULL, *i_item;
	TCHAR fpath[MULTI_BUF_SIZE], buf[BUF_SIZE];
	TCHAR *f, *f1, *f2;
	int i, j, cnt, cnt1, cnt2, mpcnt = 0;
	unsigned len1, len2;
	long FileSize;
	MEMORYSTATUS memInfo;
	int fwd_whole_msg = 0;
	BOOL is_fwd = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ATTACH, TRUE);
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
			if (*f == ATTACH_SEP) {
				SendMessage(hDlg, WM_COMMAND, ID_FWD_ADD, (LPARAM)STR_FWD_ORIG_MSG);
			} else {
				while (*f != TEXT('\0')) {
					f = str_cpy_f_t(fpath, f, ATTACH_SEP);
					SendMessage(hDlg, WM_COMMAND, ID_FWD_ADD, (LPARAM)fpath);
				}
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
		if (tpMailItem->MailStatus == ICON_SENTMAIL) {
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
		if (tpMailItem->MailStatus == ICON_SENTMAIL) {
			break;
		}
		cnt = DragQueryFile((HANDLE)wParam, 0xFFFFFFFF, NULL, 0);
		for (i = 0; i < cnt; i++) {
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
			*fpath = TEXT('\0');
			if (filename_select(hDlg, fpath, NULL, NULL, FILE_OPEN_MULTI, &op.SavedOpenDir) == FALSE) {
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
				// FILE_OPEN_MULTI may have a trailing '\' if the files are in the root dir (C:\)
				len1 = (unsigned)lstrlen(buf);
				if (*(buf + (len1 -1)) != TEXT('\\')) {
					*(buf + len1) = TEXT('\\');
					len1++;
				}
				while (*f1 != TEXT('\0')) {
					f1 = str_cpy_f_t((buf+len1), f1, TEXT('\0'));
					SendMessage(hDlg, WM_COMMAND, ID_FILE_ADD, (LPARAM)buf);
				}
			}
			break;

		case ID_FWD_ADD:
			is_fwd = TRUE;
		case ID_FILE_ADD:
			f = (TCHAR *)lParam;
			if (dir_check(f) == TRUE) {
				break;
			}
			cnt = SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0);
			if (top_attach_item != NULL && lstrcmp(f, top_attach_item->fname) == 0) {
				a_item = top_attach_item;
				top_attach_item = top_attach_item->next;
				a_item->next = NULL; // this one goes at the end
				a_item->is_fwd = is_fwd;
				// don't need to free and re-copy fname; it's the same!
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_DELETESTRING, 0, 0);
				cnt--;
			}
			i_item = top_attach_item;
			for (i = 0; i < cnt && i_item && i_item->next; i++) {
				if (lstrcmp(f, i_item->next->fname) == 0) {
					if (a_item == NULL) {
						a_item = i_item->next;
						i_item->next = i_item->next->next; // may be null
						a_item->next = NULL; // this one goes at the end
						a_item->is_fwd = is_fwd;
						// don't need to free and re-copy fname; it's the same!
					} else {
						// f was found twice??
						ATTACH_ITEM *d_item = i_item->next;
						i_item->next = i_item->next->next; // may be null
						mem_free(&d_item->fname);
						mem_free(&d_item);
					}
					SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_DELETESTRING, i+1, 0);
					cnt--;
				}
				if (i_item->next != NULL) {
					i_item = i_item->next;
				}
			}
			if (a_item == NULL) {
				a_item = (ATTACH_ITEM *)mem_calloc(sizeof(ATTACH_ITEM));
				a_item->fname = alloc_copy_t(f);
				a_item->is_fwd = is_fwd;
			}
 			if (top_attach_item == NULL) {
				top_attach_item = a_item;
			} else if (i_item) {
				i_item->next = a_item;
			} // else ??
			f = GetFileNameString(a_item->fname);
			if (a_item->is_fwd == TRUE) {
				wsprintf(buf, TEXT("%s%s"), STR_FWDATT_PREFIX, f);
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_ADDSTRING, 0, (LPARAM)buf);
			} else {
				SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_ADDSTRING, 0, (LPARAM)f);
			}
			break;

		case IDC_BUTTON_DELETE:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			if (tpMailItem->MailStatus == ICON_SENTMAIL) {
				break;
			}
			if (SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETSELCOUNT, 0, 0) <= 0) {
				break;
			}
			if (MessageBox(hDlg, STR_Q_UNLINKATTACH, STR_TITLE_DELETE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			cnt = SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0);
			i_item = top_attach_item;
			a_item = NULL;
			for (i = 0; i < cnt && i_item; i++) {
				ATTACH_ITEM *d_item;
				if (SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETSEL, i, 0) != 0) {
					d_item = i_item;
					if (i_item == top_attach_item) {
						top_attach_item = top_attach_item->next;
					} else if (a_item != NULL) {
						a_item->next = i_item->next;
					}
					i_item = i_item->next;
					mem_free(&d_item->fname);
					mem_free(&d_item);
				} else {
					a_item = i_item;
					i_item = i_item->next;
				}
			}
			for (i = cnt - 1; i >= 0; i--) {
				if (SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETSEL, i, 0) != 0) {
					SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_DELETESTRING, i, 0);
				}
			}
			break;

		case IDOK:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			if (tpMailItem->MailStatus == ICON_SENTMAIL) {
				EndDialog(hDlg, TRUE);
				break;
			}
			mem_free(&tpMailItem->Attach);
			tpMailItem->Attach = NULL;
			mem_free(&tpMailItem->FwdAttach);
			tpMailItem->FwdAttach = NULL;
			tpMailItem->AttachSize = 0;

			if ( (cnt = SendDlgItemMessage(hDlg, IDC_LIST_FILE, LB_GETCOUNT, 0, 0)) <= 0) {
				EndDialog(hDlg, TRUE);
				break;
			}

			len1 = len2 = 0;
			cnt1 = cnt2 = 0;
			a_item = top_attach_item;
			for (i = 0; i < cnt && a_item; i++) {
				f = a_item->fname;
				if (a_item->is_fwd == FALSE) {
					len1 += lstrlen(f);
					if (cnt1 != 0) {
						len1++;
					}
					cnt1++;
				} else if (lstrcmp(f, STR_FWD_ORIG_MSG) == 0) {
					fwd_whole_msg = 1;
					cnt2 = 1;
					len2 = 1;
				} else if (fwd_whole_msg == 0) {
					len2 += lstrlen(f);
					if (cnt2 != 0) {
						len2++;
					}
					cnt2++;
				}
				a_item = a_item->next;
			}
			if (cnt1 > 0) {
				f1 = tpMailItem->Attach = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len1 + 1));
				if (f1 == NULL) {
					EndDialog(hDlg, FALSE);
					break;
				}
			}
			if (cnt2 > 0) {
				MAILITEM *tpFwdMailItem = NULL;
#ifdef UNICODE
				char *ContentType;
#endif
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
				if (fwd_whole_msg > 0) {
					// slight under-estimation, because string is q-p encoded
					fwd_whole_msg = item_to_string_size(tpFwdMailItem, 2, TRUE, FALSE);
				} else {
#ifdef UNICODE
					ContentType = alloc_tchar_to_char(tpFwdMailItem->ContentType);
					mpcnt = multipart_parse(ContentType, tpFwdMailItem->Body, FALSE, &tpMultiPart, 0);
					mem_free(&ContentType);
#else
					mpcnt = multipart_parse(tpFwdMailItem->ContentType, tpFwdMailItem->Body, FALSE, &tpMultiPart, 0);
#endif
				}
			}
			cnt1 = cnt2 = 0;
			a_item = top_attach_item;
			for (i = 0; i < cnt && a_item; i++) {
				f = a_item->fname;
				if (a_item->is_fwd == FALSE) {
					if (cnt1 != 0) {
						*(f1++) = ATTACH_SEP;
					}
					f1 = str_cpy_t(f1, f);
					FileSize = file_get_size(f);
					if (FileSize > 0) {
						tpMailItem->AttachSize += (FileSize*4)/3; // 4/3 for MIME encoding
					}
					cnt1++;
				} else if (lstrcmp(f, STR_FWD_ORIG_MSG) == 0) {
					*(f2++) = ATTACH_SEP;
					*(f2++) = TEXT('\0');
					tpMailItem->AttachSize += fwd_whole_msg;
				} else if (fwd_whole_msg == 0) {
					if (cnt2 != 0) {
						*(f2++) = ATTACH_SEP;
					}
					f2 = str_cpy_t(f2, f);
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
						if (lstrcmp(fname, f) == 0) {
							FileSize = (*(tpMultiPart + j))->ePos - (*(tpMultiPart + j))->hPos;
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
				a_item = a_item->next;
			}
			memInfo.dwLength = sizeof(memInfo);
			GlobalMemoryStatus(&memInfo); 
			if (tpMailItem->AttachSize*2 > (long)memInfo.dwAvailPhys
				|| tpMailItem->AttachSize*2 > (long)memInfo.dwAvailVirtual) {
				MessageBox(hDlg, STR_WARN_ATTACH_MEM, WINDOW_TITLE, MB_OK);
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
 * SaveAttachProc - save all attachments
 */
BOOL CALLBACK SaveAttachProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hListView;
	MAILITEM *tpMailItem;
	TCHAR SaveDir[BUF_SIZE];
	int i, idx;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ATTACH, TRUE);
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
		hListView = GetDlgItem(hDlg, IDC_LIST_FILE);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 300, STR_ATTACH_NAME, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 50, STR_ATTACH_SIZE, 1);
		for (i = 0; i < MultiPartCnt; i++) {
			if (i != MultiPartTextIndex) {
				TCHAR *p, *r;
				if ((*(vMultiPart + i))->Filename != NULL) {
#ifdef UNICODE
					p = alloc_char_to_tchar((*(vMultiPart + i))->Filename);
#else
					p = alloc_copy((*(vMultiPart + i))->Filename);
#endif
				} else {
#ifdef UNICODE
					p = alloc_char_to_tchar((*(vMultiPart + i))->ContentType);
#else
					p = alloc_copy((*(vMultiPart + i))->ContentType);
#endif
					if (p != NULL) {
						for (r = p; *r != TEXT('\0') && *r != TEXT(';'); r++);
						*r = TEXT('\0');
					}
				}
				idx = ListView_AddOptionItem(hListView, p, 0);
				mem_free(&p);
				if ((*(vMultiPart + i))->ePos != NULL) {
					TCHAR sz[10];
					int len = (*(vMultiPart + i))->ePos - (*(vMultiPart + i))->sPos;
					wsprintf(sz, TEXT("%d"), len);
					ListView_SetItemText(hListView, idx, 1, sz);
				}
			}
		}
		ListView_SetItemState(hListView, -1, LVIS_SELECTED, LVIS_SELECTED);
		ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
		SendDlgItemMessage(hDlg, IDC_EDIT_SAVEDIR, WM_SETTEXT, 0, (LPARAM)op.SavedSaveDir);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_ALLSEL:
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_FILE), -1, LVIS_SELECTED, LVIS_SELECTED);
			SetFocus(GetDlgItem(hDlg, IDC_LIST_FILE));
			break;

		case IDC_BUTTON_NOSEL:
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_FILE), -1, 0, LVIS_SELECTED);
			break;

		case IDC_BUTTON_FILE_BROWSE:
			lstrcpy(SaveDir, STR_ATTACH_DIR);
			if (filename_select(hDlg, SaveDir, NULL, NULL, FILE_CHOOSE_DIR, &op.SavedSaveDir) == TRUE) {
				SendDlgItemMessage(hDlg, IDC_EDIT_SAVEDIR, WM_SETTEXT, 0, (LPARAM)op.SavedSaveDir);
			}
			break;

		case IDOK:
			tpMailItem = (MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpMailItem == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			hListView = GetDlgItem(hDlg, IDC_LIST_FILE);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				ErrorMessage(hDlg, STR_ERR_NOSELECT);
				break;
			}
			idx = 0;
			for (i = 0; i < MultiPartCnt; i++, idx++) {
				if (i == MultiPartTextIndex) {
					idx--;
				} else {
					if (ListView_GetItemState(hListView, idx, LVIS_SELECTED) == LVIS_SELECTED) {
						AttachDecode(hDlg, i, DECODE_SAVE_ALL);
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
	return TRUE;
}

/*
 * CheckDependence - EDITに機種依存文字が含まれていないかチェックする
 */
BOOL CheckDependence(HWND hWnd, int Ctl, TCHAR **buf)
{
	int i;
	BOOL ret = TRUE;
	*buf = NULL;

	AllocGetText(GetDlgItem(hWnd, Ctl), buf);
	if (*buf == NULL) {
		return TRUE;
	}
	if ((i = IsDependenceString(*buf)) != -1 &&
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
	MAILITEM *tpTmpMailItem = NULL;
	MAILBOX *tpMailBox = NULL;
	ADDRESSBOOK *tpTmpAddressBook;
	TCHAR *p;
	TCHAR buf[BUF_SIZE];
	TCHAR *mb_replyto = NULL, *mb_autobcc = NULL;
	int i, j, st, mb, cnt, sel, len;
	BOOL BtnFlag, found, ret, ReDoAttachButton = FALSE;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_SENDINFO, TRUE);
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
		cnt = j = 0;
		mb = -1;
		/* of control Initialization */
		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			tpMailBox = MailBox + i;
			if (tpMailBox == NULL || tpMailBox->Type == MAILBOX_TYPE_SAVE) {
				continue;
			}
			if ((mb == -1 && SelBox == i) || (tpMailItem->MailBox != NULL && mailbox_name_to_index(tpMailItem->MailBox) == i)) {
				j = i;
				mb = cnt;
				mb_replyto = tpMailBox->ReplyTo;
				if (tpMailBox->MyAddr2Bcc) {
					mb_autobcc = tpMailBox->BccAddr;
					if (mb_autobcc == NULL || *mb_autobcc == TEXT('\0')) {
						mb_autobcc = tpMailBox->MailAddress;
					}
				} else {
					mb_autobcc = NULL;
				}
			}
			SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_ADDSTRING, 0, 
				((tpMailBox->Name == NULL || *tpMailBox->Name == TEXT('\0')) ?
				(LPARAM)STR_MAILBOX_NONAME : (LPARAM)tpMailBox->Name));
			cnt++;
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_SETCURSEL, (mb>=0) ? mb : 0, 0);
		mb = j;

		if (tpMailItem->RedirectTo != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_TO, WM_SETTEXT, 0, (LPARAM)tpMailItem->RedirectTo);
			if (tpMailItem->Mark == ICON_SEND) {
				SendDlgItemMessage(hDlg, IDC_CHECK_MARKSEND, BM_SETCHECK, 1, 0);
			}
		} else {
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
					for (j = MAILBOX_USER; j < MailBoxCnt; j++) {
						tpMailBox = MailBox + j;
						if (tpMailBox == NULL || tpMailBox->Type == MAILBOX_TYPE_SAVE) {
							continue;
						}
						if (tpMailBox->ReplyTo != NULL && *tpMailBox->ReplyTo != TEXT('\0'))  {
							if (lstrcmp(tpMailBox->ReplyTo, buf) == 0) {
								found = TRUE;
								break;
							}
						} else if (tpMailBox->MailAddress != NULL && *tpMailBox->MailAddress != TEXT('\0')) {
							if (lstrcmp(tpMailBox->MailAddress, buf) == 0) {
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
				tpMailBox = MailBox + i;
				if (tpMailBox == NULL || tpMailBox->Type == MAILBOX_TYPE_SAVE) {
					continue;
				}
				if (tpMailBox->ReplyTo != NULL && *tpMailBox->ReplyTo != TEXT('\0')) {
					addr_to_match = tpMailBox->ReplyTo;
				} else if (tpMailBox->MailAddress != NULL && *tpMailBox->MailAddress != TEXT('\0')) {
					addr_to_match = tpMailBox->MailAddress;
				}
				// check that this address isn't a duplicate of one from a later account
				if (addr_to_match != NULL) {
					found = FALSE;
					for (j = i+1; j < MailBoxCnt; j++) {
						tpMailBox = MailBox + j;
						if (tpMailBox == NULL || tpMailBox->Type == MAILBOX_TYPE_SAVE) {
							continue;
						}
						if (tpMailBox->ReplyTo != NULL && *tpMailBox->ReplyTo != TEXT('\0'))  {
							if (lstrcmp(tpMailBox->ReplyTo, addr_to_match) == 0) {
								found = TRUE;
								break;
							}
						} else if (tpMailBox->MailAddress != NULL && *tpMailBox->MailAddress != TEXT('\0')) {
							if (lstrcmp(tpMailBox->MailAddress, addr_to_match) == 0) {
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

			SendDlgItemMessage(hDlg, IDC_DELIV, BM_SETCHECK, tpMailItem->DeliveryReceipt, 0);
			SendDlgItemMessage(hDlg, IDC_READ, BM_SETCHECK, tpMailItem->ReadReceipt, 0);
			//////////////////// ______  //////////////////////////

			if (tpMailItem->To != NULL) {
				SendDlgItemMessage(hDlg, IDC_EDIT_TO, WM_SETTEXT, 0, (LPARAM)tpMailItem->To);
			}

			if ((tpMailItem->Cc != NULL && *tpMailItem->Cc != TEXT('\0')) ||
				(tpMailItem->Bcc != NULL && *tpMailItem->Bcc != TEXT('\0')) ||
				(mb_autobcc != NULL && *mb_autobcc != TEXT('\0'))) {
				SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_CC), STR_SETSEND_BTN_CC, TRUE);
			}
			if ( (tpMailItem->Attach != NULL && *tpMailItem->Attach != TEXT('\0')) 
				|| (tpMailItem->FwdAttach != NULL && *tpMailItem->FwdAttach != TEXT('\0')) 
				|| (tpMailItem->Mark == MARK_FORWARDING && op.FwdQuotation == 2) ) {
				SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ATTACH), STR_SETSEND_BTN_ATTACH, TRUE);
			}
			//of list of file name Quotation
			if (tpMailItem->Mark == MARK_REPLYING || tpMailItem->Mark == MARK_FORWARDING) {
				ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOT_3ST), SW_HIDE);
				SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_SETCHECK, 
					(tpMailItem->Mark == MARK_REPLYING) ? op.AutoQuotation : ((op.FwdQuotation == 2) ? 0 : 1), 0);
			} else if (tpMailItem->Mark == MARK_REPL_SELTEXT || tpMailItem->Mark == MARK_FWD_SELTEXT) {
				// text is selected
				ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOTATION), SW_HIDE);
				SendDlgItemMessage(hDlg, IDC_CHECK_QUOT_3ST, BM_SETCHECK, BST_INDETERMINATE, 0);
			} else {
				ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOT_3ST), SW_HIDE);
				ShowWindow(GetDlgItem(hDlg, IDC_CHECK_QUOTATION), SW_HIDE);
			}
			if (tpMailItem->Mark == MARK_REPLYING || tpMailItem->Mark == MARK_REPL_SELTEXT) {
				SendDlgItemMessage(hDlg, IDC_CHECK_ADD_RECIP, BM_SETCHECK, (op.AutoAddRecipients == 1) ? 1 : 0, 0);
			} else {
				ShowWindow(GetDlgItem(hDlg, IDC_CHECK_ADD_RECIP), SW_HIDE);
			}
			if (tpMailItem->Mark == MARK_FORWARDING) {
				SendDlgItemMessage(hDlg, IDC_CHECK_ATT_MSG, BM_SETCHECK, (op.FwdQuotation == 2) ? 1 : 0, 0);
			} else {
				ShowWindow(GetDlgItem(hDlg, IDC_CHECK_ATT_MSG), SW_HIDE);
			}

			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_MARKSEND), SW_HIDE);
		}
		if (tpMailItem->Subject != NULL) {
			SendDlgItemMessage(hDlg, IDC_EDIT_TITLE, WM_SETTEXT, 0, (LPARAM)tpMailItem->Subject);
		}

		if (tpMailItem->MailStatus == ICON_SENTMAIL) {
			EnableWindow(GetDlgItem(hDlg, IDC_COMBO_SMTP), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TO), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_TO), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_TITLE), 0);
			if (tpMailItem->RedirectTo != NULL) {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_MARKSEND), 0);
			} else {
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO_REPLYTO), 0);
				EnableWindow(GetDlgItem(hDlg, IDC_PRIORITY), 0);
				EnableWindow(GetDlgItem(hDlg, IDC_DELIV), 0);
				EnableWindow(GetDlgItem(hDlg, IDC_READ), 0);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_QUOTATION), 0);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_QUOT_3ST), 0);
			}
		}

		*(tpSendMailIList + 1) = tpTmpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
		if (tpTmpMailItem == NULL) {
			mem_free((void **)&tpSendMailIList);
			ErrorMessage(hDlg, STR_ERR_MEMALLOC);
			EndDialog(hDlg, FALSE);
			break;
		}
		if (tpMailItem->RedirectTo != NULL) {
			tpTmpMailItem->RedirectTo = alloc_copy_t(tpMailItem->RedirectTo);
		} else {
			tpTmpMailItem->Cc = alloc_copy_t(tpMailItem->Cc);
			tpTmpMailItem->Bcc = alloc_copy_t(tpMailItem->Bcc);
			if (tpMailItem->MailStatus != ICON_SENTMAIL &&
				mb_autobcc != NULL && *mb_autobcc != TEXT('\0')) {
				if (tpTmpMailItem->Bcc == NULL) {
					tpTmpMailItem->Bcc = alloc_copy_t(mb_autobcc);
				} else {
					TCHAR *tmp;
					BOOL found = FALSE;
					p = tpTmpMailItem->Bcc;
					len = lstrlen(p) + 1;
					tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
					while (tmp != NULL && *p != TEXT('\0')) {
						p = str_cpy_f_t(tmp, p, TEXT(','));
						if (lstrcmp(mb_autobcc, tmp) == 0) {
							found = TRUE;
							break;
						}
					}
					mem_free(&tmp);
					if (found == FALSE) {
						len += lstrlen(mb_autobcc) + 2; // ", "
						tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
						if (tmp != NULL) {
							wsprintf(tmp, TEXT("%s, %s"), tpTmpMailItem->Bcc, mb_autobcc);
							mem_free(&tpTmpMailItem->Bcc);
							tpTmpMailItem->Bcc = tmp;
						}
					}
				}
			}
			tpTmpMailItem->Attach = alloc_copy_t(tpMailItem->Attach);
			if (tpMailItem->Mark == MARK_FORWARDING && op.FwdQuotation == 2) {
				// GJC forward entire message as attachment
				p = (TCHAR *)mem_alloc(sizeof(TCHAR) * 2);
				if (p != NULL) {
					tpTmpMailItem->FwdAttach = p;
					*(p++) = ATTACH_SEP;
					*(p++) = TEXT('\0');
				}
			} else {
				tpTmpMailItem->FwdAttach = alloc_copy_t(tpMailItem->FwdAttach);
			}
			tpTmpMailItem->AttachSize = tpMailItem->AttachSize;
			tpTmpMailItem->ReplyTo = alloc_copy_t(tpMailItem->ReplyTo);
			tpTmpMailItem->DefReplyTo = tpMailItem->DefReplyTo;
			tpTmpMailItem->References = alloc_copy_t(tpMailItem->References);
			tpTmpMailItem->MailStatus = tpMailItem->MailStatus;
			tpTmpMailItem->Mark = tpMailItem->Mark;
			tpTmpMailItem->No = mb; // hack
		}
		SetEditToSubClass(GetDlgItem(hDlg, IDC_EDIT_TO), FALSE);
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_TITLE),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
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
					hDlg, AddressListProc, (LPARAM)tpTmpAddressBook) == TRUE
					&& tpTmpAddressBook->AddrList != NULL) {
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
			if (tpTmpMailItem != NULL) {
				AllocGetText(GetDlgItem(hDlg, IDC_EDIT_TO), &tpTmpMailItem->To);
				if (tpTmpMailItem->To != NULL) {
					delete_ctrl_char(tpTmpMailItem->To);
				}
				DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_CC),
					hDlg, CcListProc, (LPARAM)tpTmpMailItem);
				SendDlgItemMessage(hDlg, IDC_EDIT_TO, WM_SETTEXT, 0, (LPARAM)tpTmpMailItem->To);
				BtnFlag = ((tpTmpMailItem->Cc != NULL && *tpTmpMailItem->Cc != TEXT('\0')) ||
					(tpTmpMailItem->Bcc != NULL && *tpTmpMailItem->Bcc != TEXT('\0'))) ? TRUE : FALSE;
				SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_CC), STR_SETSEND_BTN_CC, BtnFlag);
			}
			break;

		case IDC_BUTTON_ATTACH:
			if (top_attach_item != NULL) {
				// already modifying attachments for another message
				break;
			}
			tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpSendMailIList == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			tpTmpMailItem = *(tpSendMailIList + 1);
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ATTACH),
				hDlg, SetAttachProc, (LPARAM)tpTmpMailItem);
			attach_item_free();

			if (tpTmpMailItem != NULL) {
				ReDoAttachButton = TRUE;
			}
			break;

		case IDC_CHECK_QUOTATION:
			if (SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_GETCHECK, 0, 0)) {
				if (op.FwdQuotation == 2) {
					tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
					if (tpSendMailIList == NULL) {
						EndDialog(hDlg, FALSE);
						break;
					}
					tpMailItem = *tpSendMailIList;
					tpTmpMailItem = *(tpSendMailIList + 1);
					if (tpTmpMailItem != NULL && tpTmpMailItem->FwdAttach != NULL
						&& *tpTmpMailItem->FwdAttach == ATTACH_SEP) {
						mem_free(&tpTmpMailItem->FwdAttach);
						if (tpMailItem != NULL) {
							tpTmpMailItem->FwdAttach = alloc_copy_t(tpMailItem->FwdAttach);
						}
					}
					SendDlgItemMessage(hDlg, IDC_CHECK_ATT_MSG, BM_SETCHECK, 0, 0);
					ReDoAttachButton = TRUE;
				}
			}
			break;

		case IDC_CHECK_ATT_MSG:
			tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpSendMailIList == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			tpMailItem = *tpSendMailIList;
			tpTmpMailItem = *(tpSendMailIList + 1);
			if (tpMailItem != NULL && tpMailItem->Mark != MARK_FORWARDING) {
				break;
			}
			if (tpTmpMailItem != NULL) {
				if (SendDlgItemMessage(hDlg, IDC_CHECK_ATT_MSG, BM_GETCHECK, 0, 0)) {
					// attach whole message
					p = (TCHAR *)mem_alloc(sizeof(TCHAR) * 2);
					if (p != NULL) {
						mem_free(&tpTmpMailItem->FwdAttach);
						tpTmpMailItem->FwdAttach = p;
						*(p++) = ATTACH_SEP;
						*(p++) = TEXT('\0');
					}
					SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ATTACH), STR_SETSEND_BTN_ATTACH, TRUE);
					if (op.FwdQuotation != 2) {
						SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_SETCHECK, 0, 0);
					}
				} else if (tpMailItem != NULL) {
					// rebuild list of forwarded attachments
					mem_free(&tpTmpMailItem->FwdAttach);
					tpTmpMailItem->FwdAttach = alloc_copy_t(tpMailItem->FwdAttach);
					ReDoAttachButton = TRUE;
				}
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
				if (tpTmpMailItem == NULL) {
					EndDialog(hDlg, FALSE);
					break;
				}
				if (tpTmpMailItem->RedirectTo != NULL) {
					break;
				}
				i = SendDlgItemMessage(hDlg, IDC_COMBO_SMTP, CB_GETCURSEL, 0, 0);
				if (i != CB_ERR) {
					TCHAR *tmp, *q;
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
					mb = tpTmpMailItem->No;
					if ((MailBox + mb)->MyAddr2Bcc && tpTmpMailItem->Bcc != NULL) {
						// remove bcc of old account
						BOOL found = FALSE;
						mb_autobcc = (MailBox + mb)->BccAddr;
						if (mb_autobcc == NULL || *mb_autobcc == TEXT('\0')) {
							mb_autobcc = (MailBox + mb)->MailAddress;
						}
						if (lstrcmp(mb_autobcc, tpTmpMailItem->Bcc) == 0) {
							mem_free(&tpTmpMailItem->Bcc);
							tpTmpMailItem->Bcc = NULL;
						} else {
							p = tpTmpMailItem->Bcc;
							len = lstrlen(p) + 1;
							while (*p != TEXT('\0')) {
								if (*p == TEXT(',')) len +=3;
								p++;
							}
							tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
							if (tmp != NULL) {
								q = tmp;
								*q = TEXT('\0');
								p = tpTmpMailItem->Bcc;
								while (*p != TEXT('\0')) {
									p = str_cpy_f_t(buf, p, TEXT(','));
									if (lstrcmp(mb_autobcc, buf) == 0) {
										found = TRUE;
									} else {
										if (q == tmp) {
											q = str_join_t(q, buf, (TCHAR *)-1);
										} else {
											q = str_join_t(q, TEXT(",\r\n "), buf, (TCHAR *)-1);
										}
									}
									while (*p == TEXT(' ') || *p == TEXT('\r') || *p == TEXT('\n')) {
										p++;
									}
								}
							}
						}
						if (found == TRUE) {
							mem_free(&tpTmpMailItem->Bcc);
							tpTmpMailItem->Bcc = tmp;
						} else {
							mem_free(&tmp);
						}
					}
					if ((MailBox + i)->MyAddr2Bcc) {
						// add bcc of new account
						mb_autobcc = (MailBox + i)->BccAddr;
						if (mb_autobcc == NULL || *mb_autobcc == TEXT('\0')) {
							mb_autobcc = (MailBox + i)->MailAddress;
						}
						if (tpTmpMailItem->Bcc == NULL) {
							tpTmpMailItem->Bcc = alloc_copy_t(mb_autobcc);
						} else {
							BOOL found = FALSE;
							p = tpTmpMailItem->Bcc;
							while (*p != TEXT('\0')) {
								p = str_cpy_f_t(buf, p, TEXT(','));
								if (lstrcmp(mb_autobcc, buf) == 0) {
									found = TRUE;
									break;
								}
								while (*p == TEXT(' ') || *p == TEXT('\r') || *p == TEXT('\n')) {
									p++;
								}

							}
							if (found == FALSE) {
								len = lstrlen(tpTmpMailItem->Bcc) + lstrlen(mb_autobcc) + 5;
								tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
								if (tmp != NULL) {
									wsprintf(tmp, TEXT("%s,\r\n %s"), tpTmpMailItem->Bcc, mb_autobcc);
									mem_free(&tpTmpMailItem->Bcc);
									tpTmpMailItem->Bcc = tmp;
								}
							}
						}
					}
					tpTmpMailItem->No = i; // hack
					BtnFlag = ((tpTmpMailItem->Cc != NULL && *tpTmpMailItem->Cc != TEXT('\0')) ||
						(tpTmpMailItem->Bcc != NULL && *tpTmpMailItem->Bcc != TEXT('\0'))) ? TRUE : FALSE;
					SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_CC), STR_SETSEND_BTN_CC, BtnFlag);

					// GJC update reply-to
					if (tpTmpMailItem->DefReplyTo == TRUE) {
						sel = 0;
						p = (MailBox + i)->ReplyTo;
						if (p != NULL && *p != TEXT('\0')) {
							int cnt = SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_GETCOUNT, 0, 0);
							for (j=0; j < cnt; j++) {
								buf[0]=TEXT('\0');
								SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_GETLBTEXT, j, (LPARAM)buf);
								if (lstrcmpi(p, buf) == 0) {
									sel = j;
									break;
								}
							}
						}
						SendDlgItemMessage(hDlg, IDC_COMBO_REPLYTO, CB_SETCURSEL, sel, 0);
					}
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
			ret = CheckDependence(hDlg, IDC_EDIT_TO, &p);
			mem_free(&p);
			if (ret == FALSE) break;
			ret = CheckDependence(hDlg, IDC_EDIT_TITLE, &p);
			mem_free(&p);
			if (ret == FALSE) break;

			tpSendMailIList = (MAILITEM **)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpSendMailIList == NULL) {
				EndDialog(hDlg, FALSE);
				break;
			}
			tpMailItem = *tpSendMailIList;
			tpTmpMailItem = *(tpSendMailIList + 1);
			if (tpMailItem->MailStatus == ICON_SENTMAIL) {
				if (tpTmpMailItem != NULL) {
					item_free(&tpTmpMailItem, 1);
				}
				mem_free((void **)&tpSendMailIList);
				EndDialog(hDlg, TRUE);
				break;
			}

			//Account mailbox to send from
			mem_free(&tpMailItem->MailBox);
			tpMailItem->MailBox = NULL;
			if (tpMailItem->RedirectTo != NULL) {
				AllocGetText(GetDlgItem(hDlg, IDC_COMBO_SMTP), &tpMailItem->MailBox);
				AllocGetText(GetDlgItem(hDlg, IDC_EDIT_TO), &tpMailItem->RedirectTo);
				if (tpMailItem->RedirectTo == NULL) {
					tpMailItem->RedirectTo = alloc_copy_t(TEXT(""));
				} else {
					delete_ctrl_char(tpMailItem->RedirectTo);
				}
				tpMailItem->Mark = SendDlgItemMessage(hDlg, IDC_CHECK_MARKSEND, BM_GETCHECK, 0, 0) ? ICON_SEND : ICON_NON;
				item_free(&tpTmpMailItem, 1);
			} else {
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
					tpMailBox = MailBox + i;
					tpMailItem->MailBox = alloc_copy_t(tpMailBox->Name);

					mem_free(&tpMailItem->From);
					tpMailItem->From = NULL;
					if (tpMailBox->MailAddress != NULL && *tpMailBox->MailAddress != TEXT('\0')) {
						len = lstrlen(TEXT(" <>"));
						if (tpMailBox->UserName != NULL) {
							len += lstrlen(tpMailBox->UserName);
						}
						len += lstrlen(tpMailBox->MailAddress);
						tpMailItem->From = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
						if (tpMailItem->From != NULL) {
							p = tpMailItem->From;
							if (tpMailBox->UserName != NULL && *tpMailBox->UserName != TEXT('\0')) {
								p = str_join_t(p, tpMailBox->UserName, TEXT(" "), (TCHAR *)-1);
							}
							str_join_t(p, TEXT("<"), tpMailBox->MailAddress, TEXT(">"), (TCHAR *)-1);
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
				tpMailItem->DeliveryReceipt = SendDlgItemMessage(hDlg, IDC_DELIV, BM_GETCHECK, 0, 0);
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
					if (SelBox == MAILBOX_SEND) {
						HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
						i = ListView_GetMemToItem(hListView, tpMailItem);
						if (i != -1) {
							st = ListView_ComputeState(tpMailItem->Priority, tpMailItem->Multipart);
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
				if (tpMailItem->Mark == MARK_REPLYING
					&& SendDlgItemMessage(hDlg, IDC_CHECK_ADD_RECIP, BM_GETCHECK, 0, 0)) {
					addr_list_add(tpMailItem->To);
					addr_list_add(tpMailItem->Cc);
					addr_list_add(tpMailItem->Bcc);
				}

				//Quotation -- on exit from this dialog, ->Mark is 0, 1, or 2
				// 0: no quoting
				// 1: quote full message (reply or forward)
				// 2: quote selected text
				if (tpMailItem->Mark == MARK_REPLYING || tpMailItem->Mark == MARK_FORWARDING) {
					tpMailItem->Mark = (char)SendDlgItemMessage(hDlg, IDC_CHECK_QUOTATION, BM_GETCHECK, 0, 0);
				} else if (tpMailItem->Mark == MARK_REPL_SELTEXT || tpMailItem->Mark == MARK_FWD_SELTEXT) {
					tpMailItem->Mark = (char)SendDlgItemMessage(hDlg, IDC_CHECK_QUOT_3ST, BM_GETCHECK, 0, 0);
				}
			}
			mem_free((void **)&tpSendMailIList);
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			break;
		}
		if (ReDoAttachButton && tpTmpMailItem != NULL) {
			BtnFlag = ( (tpTmpMailItem->Attach != NULL && *tpTmpMailItem->Attach != TEXT('\0'))
					|| (tpTmpMailItem->FwdAttach != NULL && *tpTmpMailItem->FwdAttach != TEXT('\0')) ) 
					? TRUE : FALSE;
			SetButtonText(GetDlgItem(hDlg, IDC_BUTTON_ATTACH), STR_SETSEND_BTN_ATTACH, BtnFlag);
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/*
 * AddressGetWholeGroup - get string of all members of a group
 */
static TCHAR *AddressGetWholeGroup(TCHAR *groupname)
{
	TCHAR *ret, *p;
	int i, len = 1;
	for (i = 0; i < AddressBook->ItemCnt; i++) {
		ADDRESSITEM *item = *(AddressBook->tpAddrItem + i);
		if (item != NULL && item->Group != NULL && item_in_list(groupname, item->Group) == TRUE) {
			len += lstrlen(item->MailAddress) + 2;
		}
	}
	if (len < 2) {
		return NULL;
	}
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
	if (ret == NULL) {
		return NULL;
	}
	p = ret;
	for (i = 0; i < AddressBook->ItemCnt; i++) {
		ADDRESSITEM *item = *(AddressBook->tpAddrItem + i);
		if (item != NULL && item->Group != NULL && item_in_list(groupname, item->Group) == TRUE) {
			p = str_join_t(p, item->MailAddress, TEXT(", "), (TCHAR *)-1);
		}
	}
	p-=2;
	*p = TEXT('\0');
	return ret;
}

/*
 * AddrCompleteCallback - event handler for address auto-completion (GJC)
 */
static LRESULT CALLBACK AddrCompleteCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CHAR:
		if (AutoCompleted == TRUE && LOWORD(wParam) == VK_BACK) {
			DWORD dwStart, dwEnd;
			SendMessage(hWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
			if (dwEnd <= dwStart) {
				TCHAR *p, *addr = NULL;
				dwStart = 0;
				dwEnd = AllocGetText(hWnd, &addr);
				for (p = addr; *p != TEXT('\0'); p++) {
					if (*p == TEXT('\"')) {
						while (*p != TEXT('\"') && *p != TEXT('\0')) p++;
					} else if (*p == TEXT(',')) {
						p++;
						while (*p == TEXT(' ')) p++;
						dwStart = (p - addr);
					}
				}
				mem_free(&addr);
				SendMessage(hWnd, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
			}
			SendMessage(hWnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)AutoCompleteStr);
			AutoCompleted = FALSE;
			MatchedAddrItem = 0;
			*AutoCompleteStr = TEXT('\0');
			return 0;
		}
		if ((TCHAR)wParam == TEXT(' ')) {
			TCHAR part[BUF_SIZE];
			TCHAR *p, *q, *addr = NULL;
			DWORD dwStart, dwEnd;
			int i, start = 0;
#ifdef _WIN32_WCE
			unsigned int len;
#else
			int len;
#endif
			AllocGetText(hWnd, &addr);
			SendMessage(hWnd, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
			if (dwEnd > dwStart && lstrlen(AutoCompleteStr) > 0) {
				wsprintf(part, TEXT("%s"), AutoCompleteStr);
				start = (int)dwStart;
			} else {
				MatchedAddrItem = 0;
				*AutoCompleteStr = TEXT('\0');
				p = addr;
				q = part;
				i = 1;
				while (*p != TEXT('\0') && i < BUF_SIZE) {
					if (*p == TEXT(' ')) {
						p++;
					}
					if (*p == TEXT('\"')) {
						while (*p != TEXT('\"') && *p != TEXT('\0')) p++;
					} else if (*p == TEXT(',')) {
						q = part;
						i = 1;
						p++;
						while (*p == TEXT(' ')) p++;
						start = (p - addr);
					}
					if (*p != TEXT('\0')) {
						*(q++) = *(p++);
						i++;
					}
				}
				*q = TEXT('\0');
			}
			mem_free(&addr);
			len = lstrlen(part);
			if (len > 0) {
				TCHAR *match = NULL;
				BOOL more = FALSE, looped = FALSE, group = FALSE, free_match = FALSE;
				wsprintf(AutoCompleteStr, TEXT("%s"), part);
				i = MatchedAddrItem;
				if (i == AddressBook->ItemCnt) {
					// flag for group match
					match = AddressGetWholeGroup(part);
					free_match = TRUE;
					i = 0;
				}
				while (i < AddressBook->ItemCnt) {
					ADDRESSITEM *item = *(AddressBook->tpAddrItem + i);
					if (item != NULL) {
						if ((item->MailAddress != NULL && word_find_ni_t(part, item->MailAddress, len) == TRUE)
							|| (item->Comment != NULL && word_find_ni_t(part, item->Comment, len) == TRUE)) {
							if (match == NULL) {
								match = item->MailAddress;
							} else {
								more = TRUE;
								MatchedAddrItem = i;
								break;
							}
						}
						if (group == FALSE && item->Group != NULL && item_in_list(part, item->Group) == TRUE) {
							group = TRUE;
						}
					}
					i++;
					if (i == AddressBook->ItemCnt && looped == FALSE) {
						looped = TRUE;
						i = 0;
					}
					if (i == MatchedAddrItem) {
						if (group == TRUE) {
							if (match == NULL) {
								// the group is the only match
								match = AddressGetWholeGroup(part);
								free_match = TRUE;
								more = FALSE;
							} else {
								more = TRUE;
								MatchedAddrItem = AddressBook->ItemCnt;
							}
						}
						break;
					}
				}
				if (match != NULL && lstrlen(match) > len) {
					if (dwEnd <= dwStart) {
						SendMessage(hWnd, EM_SETSEL, (WPARAM)start, (LPARAM)(start+len));
					}
					SendMessage(hWnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)match);
					if (more == TRUE) {
						SendMessage(hWnd, EM_SETSEL, (WPARAM)start, (LPARAM)(start+lstrlen(match)));
					}
					if (free_match == TRUE) {
						mem_free(&match);
					}
					AutoCompleted = TRUE;
					return 0;
				}
			}
		}
	case WM_CUT:
	case WM_CLEAR:
	case WM_PASTE:
	case WM_KILLFOCUS:
		AutoCompleted = FALSE;
		*AutoCompleteStr = TEXT('\0');
		break;
	case WM_KEYDOWN:
		if (LOWORD(wParam) != VK_BACK && (TCHAR)wParam != TEXT(' ')) {
			AutoCompleted = FALSE;
			*AutoCompleteStr = TEXT('\0');
		}
		break;
#ifdef _WIN32_WCE_PPC
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case ID_MENUITEM_CUT:
			SendMessage(hWnd, WM_CUT, 0, 0);
			break;
		case ID_MENUITEM_COPY:
			SendMessage(hWnd, WM_COPY, 0, 0);
			break;
		case ID_MENUITEM_PASTE:
			SendMessage(hWnd, WM_PASTE, 0, 0);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			SHRGINFO rg;

			rg.cbSize = sizeof(SHRGINFO);
			rg.hwndClient = hWnd;
			rg.ptDown.x = LOWORD(lParam);
			rg.ptDown.y = HIWORD(lParam);
			rg.dwFlags = SHRG_RETURNCMD;

			if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
				ShowMenu(hWnd, hEDITPOPUP, 0, 0);
				return 0;
			}
		}
		break;
#endif
	}
#ifdef _WIN32_WCE
	return CallWindowProcW((CcWndShowing == TRUE) ? CcAddrWndProc : EditToWndProc, hWnd, msg, wParam, lParam);
#else
	return CallWindowProc((WNDPROC)GetProp(hWnd, WNDPROC_KEY), hWnd, msg, wParam, lParam);
#endif
}

/*
 * SetEditToSubClass - Subclass (callback) modification for IDC_EDIT_TO, IDC_EDIT_MAILADDRESS (GJC)
 *                     Note: this subclass is not removed; removal crashes PPC, and it's not really necessary
 */
static void SetEditToSubClass(HWND hWnd, BOOL CcWnd)
{
#ifdef _WIN32_WCE
	if (CcWnd) {
		CcAddrWndProc = (WNDPROC)SetWindowLongW(hWnd, GWL_WNDPROC, (DWORD)AddrCompleteCallback);
		CcWndShowing = TRUE;
	} else {
		EditToWndProc = (WNDPROC)SetWindowLongW(hWnd, GWL_WNDPROC, (DWORD)AddrCompleteCallback);
	}
#else
	WNDPROC OldWndProc = NULL;

	OldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)AddrCompleteCallback);
	SetProp(hWnd, WNDPROC_KEY, OldWndProc);
#endif
}

/*
 * EditTextCallback - tap&hold for all EDITTEXT boxes (GJC)
 *                    assumed that the DefEditTextWndProc is the same for all boxes!
 */
#ifdef _WIN32_WCE_PPC
static LRESULT CALLBACK EditTextCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case ID_MENUITEM_CUT:
			SendMessage(hWnd, WM_CUT, 0, 0);
			break;
		case ID_MENUITEM_COPY:
			SendMessage(hWnd, WM_COPY, 0, 0);
			break;
		case ID_MENUITEM_PASTE:
			SendMessage(hWnd, WM_PASTE, 0, 0);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			SHRGINFO rg;

			rg.cbSize = sizeof(SHRGINFO);
			rg.hwndClient = hWnd;
			rg.ptDown.x = LOWORD(lParam);
			rg.ptDown.y = HIWORD(lParam);
			rg.dwFlags = SHRG_RETURNCMD;

			if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
				ShowMenu(hWnd, hEDITPOPUP, 0, 0);
				return 0;
			}
		}
		break;
	}
	return CallWindowProcW(DefEditTextWndProc, hWnd, msg, wParam, lParam);
}
#endif


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
	int i, ItemIndex, ans;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ADDRESSINFO, TRUE);
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
		ListView_AddColumn(hListView, LVCFMT_LEFT, 70, STR_MAILPROP_HEADER, 0);
		// make this wide so the full address fits and a scrollbar appears
		ListView_AddColumn(hListView, LVCFMT_LEFT, 400, STR_MAILPROP_MAILADDRESS, 1);
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
				ItemIndex = ListView_AddOptionItem(hListView, type, 0);
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
		DialogLvNotifyProc(hDlg, lParam, IDC_LIST_ADDRESS);
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
			item_copy((MAILITEM *)GetWindowLong(hDlg, GWL_USERDATA), tpMailItem, TRUE);
			mem_free(&tpMailItem->ReplyTo);
			tpMailItem->ReplyTo = buf;

			//of information of reply Setting
			i = Edit_InitInstance(hInst, hDlg, vSelBox, tpMailItem, EDIT_REPLY, NULL, FALSE);
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
				SendMessage(hDlg, WM_COMMAND, ID_LV_ALLSELECT, 0);
			}
			i = -1;
			while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
				TCHAR *item, *addr;
				int j;
				item = ListView_AllocGetText(hListView, i, 1);
				if (item == NULL) {
					continue;
				}
				addr = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(item) + 1));
				if (addr == NULL) {
					continue;
				}
				*addr = TEXT('\0');
				GetMailAddress(item, addr, NULL, FALSE);
				for (j = 0; j < AddressBook->ItemCnt; j++) {
					if (lstrcmp(addr, (*(AddressBook->tpAddrItem + j))->AddressOnly) == 0) {
						ListView_SetItemState(hListView, i, 0, LVNI_SELECTED);
						break;
					}
				}
				j = i;
				while((j = ListView_GetNextItem(hListView, j, LVNI_SELECTED)) != -1) {
					TCHAR *item2, *addr2;
					item2 = ListView_AllocGetText(hListView, j, 1);
					if (item2 == NULL) {
						continue;
					}
					addr2 = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(item2) + 1));
					if (addr2 == NULL) {
						continue;
					}
					*addr2 = TEXT('\0');
					GetMailAddress(item2, addr2, NULL, FALSE);
					if (lstrcmp(addr, addr2) == 0) {
						ListView_SetItemState(hListView, j, 0, LVNI_SELECTED);
					}
					mem_free(&item2);
					mem_free(&addr2);
				}
				mem_free(&item);
				mem_free(&addr);
			}
			i = ListView_GetSelectedCount(hListView);
			if (i == 0) {
				MessageBox(hDlg, STR_MSG_ADDR_ALREADY, STR_TITLE_ADD_ADDR, MB_OK);
				break;
			}

#ifdef _WIN32_WCE
			wsprintf(msg, (op.UsePOOMAddressBook == 0) ? STR_Q_ADDADDRESS : STR_Q_EDITADDPOOM, i);
#else
			wsprintf(msg, STR_Q_ADDADDRESS, i);
#endif
			ans = MessageBox(hDlg, msg, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNOCANCEL);
			if (ans == IDCANCEL) {
				break;
			}
			i = -1;
			while ((i = ListView_GetNextItem(hListView, i, LVNI_SELECTED)) != -1) {
#ifdef _WIN32_WCE
				TCHAR *addr, *cmmt, *fname, *lname;
#endif
				tpAddrItem = (ADDRESSITEM *)mem_calloc(sizeof(ADDRESSITEM));
				if (tpAddrItem == NULL) {
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
				tpAddrItem->Num = AddressBook->ItemCnt;
				tpAddrItem->MailAddress = ListView_AllocGetText(hListView, i, 1);
				if (tpAddrItem->MailAddress == NULL) {
					mem_free(&tpAddrItem);
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
				delete_ctrl_char(tpAddrItem->MailAddress);
#ifdef _WIN32_WCE
				if (op.UsePOOMAddressBook != 0) {
					int len = lstrlen(tpAddrItem->MailAddress) + 1;
					addr = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
					cmmt = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
					*addr = *cmmt = TEXT('\0');
					GetMailAddress(tpAddrItem->MailAddress, addr, cmmt, FALSE);
					mem_free(&tpAddrItem->MailAddress);
					tpAddrItem->MailAddress = addr;
					tpAddrItem->Comment = cmmt;
				}
#endif
				//of information of transmission In address register mail address additional
				if (addr_add(AddressBook, tpAddrItem) == FALSE) {
					mem_free(&tpAddrItem->MailAddress);
					mem_free(&tpAddrItem->AddressOnly);
#ifdef _WIN32_WCE
					mem_free(&tpAddrItem->Comment);
#endif
					mem_free(&tpAddrItem);
					ErrorMessage(hDlg, STR_ERR_ADD);
					return FALSE;
				}
				if (ans == IDYES) {
					AddressBook->EditNum = AddressBook->ItemCnt - 1;
					AddressBook->FromAddrInfo = TRUE;
					DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ADDRESS_EDIT),
						hDlg, EditAddressProc, (LPARAM)AddressBook);
				}
#ifdef _WIN32_WCE
				// add to POOM Address Book
				addr = tpAddrItem->MailAddress;
				cmmt = tpAddrItem->Comment;
				if (op.UsePOOMAddressBook != 0 && addr != NULL && cmmt != NULL) {
					int len = lstrlen(cmmt) + 1;
					if (len > 1) {
						fname = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
						lname = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
						*fname = *lname = TEXT('\0');
						GetNameFromComment(cmmt, fname, lname);
					} else {
						lname = fname = NULL;
					}
					AddPOOMContact(addr, fname, lname, tpAddrItem->Group);
					mem_free(&fname);
					mem_free(&lname);
				}
#endif
			}
#ifdef _WIN32_WCE
			if (op.AutoSave == 1 && op.UsePOOMAddressBook == 0) {
#else
			if (op.AutoSave == 1) {
#endif
				// アドレス帳を保存
				file_save_address_book(ADDRESS_FILE, DataDir, AddressBook);
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
 * SetWindowSize - for Address List and Mailbox Summary
 */
#ifdef _WIN32_WCE
static void SetWindowSize(HWND hDlg, int ListID, int top, int bottom, int left, int right)
{
	HWND hItem;
	int width, height, midy, ypos, tmp;

	width = right - left;
	height = bottom - top;
	midy = (top + bottom - 30) / 2;

	ypos = (midy > 46) ? midy - 46 : 0;
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_UP),
		right-16, ypos, 15, 24, TRUE);

	ypos = (midy < bottom - 100) ? midy + 12 : bottom - 100;
	MoveWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN),
		right-16, ypos, 15, 24, TRUE);

	if (ListID == IDC_LIST_ADDRESS) {
		hItem = GetDlgItem(hDlg, IDC_BUTTON_NUM);
		ShowWindow(hItem, (midy - 46 > 30));
		MoveWindow(hItem, right-16, top, 15, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDC_BUTTON_ADD);
		ShowWindow(hItem, (width > 135));
		MoveWindow(hItem, left+1, bottom-30, 40, 21, TRUE);

		if (op.UsePOOMAddressBook == 0) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_EDIT);
			ShowWindow(hItem, (width > 175));
			MoveWindow(hItem, left+42, bottom-30, 40, 21, TRUE);

			hItem = GetDlgItem(hDlg, IDC_BUTTON_DELETE);
			ShowWindow(hItem, (width > 222));
			MoveWindow(hItem, left+83, bottom-30, 45, 21, TRUE);
		}

		hItem = GetDlgItem(hDlg, IDC_BUTTON_MAIL);
		ShowWindow(hItem, (width > 94));
		MoveWindow(hItem, right-((width>278)?139:93), bottom-30, 45, 21, TRUE);

#ifdef _WIN32_WCE_PPC
		hItem = GetDlgItem(hDlg, IDCANCEL);
		ShowWindow(hItem, (width > 70));
		MoveWindow(hItem, right-47, bottom-30, 45, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDOK);
		ShowWindow(hItem, (width > 278));
		MoveWindow(hItem, right-93, bottom-30, 45, 21, TRUE);
#else
		hItem = GetDlgItem(hDlg, IDCANCEL);
		ShowWindow(hItem, (width > 278));
		MoveWindow(hItem, right-47, bottom-30, 45, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDOK);
		ShowWindow(hItem, (width > 70));
		MoveWindow(hItem, right - ((width>278)?93:47), bottom-30, 45, 21, TRUE);
#endif
		hItem = GetDlgItem(hDlg, IDC_STATIC_TITLE);
		ShowWindow(hItem, (width > 142) && (height > 150));
		MoveWindow(hItem, left, bottom-60, 50, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDC_ADDR_GRP_COMBOL);
		ShowWindow(hItem, (width > 100) && (height > 150));
		tmp = (width>142) ? 55 : 10;
		MoveWindow(hItem, left + tmp, bottom-60, width-tmp-5, 150, TRUE);

		ypos = height - ((height > 150) ? 69 : 39);
	} else {
		hItem = GetDlgItem(hDlg, IDC_BUTTON_ADD);
		ShowWindow(hItem, (width > 90));
		MoveWindow(hItem, left+1, bottom-30, 42, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDC_BUTTON_SAVE);
		ShowWindow(hItem, (width > 134));
		MoveWindow(hItem,left+45, bottom-30, 42, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDC_BUTTON_EDIT);
		ShowWindow(hItem, (width > 178));
		MoveWindow(hItem, left+89, bottom-30, 42, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDC_BUTTON_DELETE);
		ShowWindow(hItem, (width > 222));
		MoveWindow(hItem, left+133, bottom-30, 42, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDCANCEL);
		ShowWindow(hItem, (width > 266));
		MoveWindow(hItem, right-89, bottom-30, 42, 21, TRUE);

		hItem = GetDlgItem(hDlg, IDOK);
		ShowWindow(hItem, (width > 70));
		MoveWindow(hItem, right-44, bottom-30, 42, 21, TRUE);

		ypos = height-39;
	}

	hItem = GetDlgItem(hDlg, ListID);
	MoveWindow(hItem, left, top, width-17, ypos, TRUE);
	UpdateWindow(hDlg);
}
#else
static void SetWindowSize(HWND hDlg, int ListID, int top, int bottom, int left, int right)
{
	HWND hItem;
	int width, height, midy, ypos;

	// hide all windows initially, to avoid redraw problems
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_UP10), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN10), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), SW_HIDE);
	if (ListID == IDC_LIST_ADDRESS) {
		ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_NUM), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_MAIL), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_STATIC_TITLE), SW_HIDE);
		ShowWindow(GetDlgItem(hDlg, IDC_ADDR_GRP_COMBOL), SW_HIDE);
	} else {
		ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_SAVE), SW_HIDE);
	}
	ShowWindow(GetDlgItem(hDlg, IDOK), SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDCANCEL), SW_HIDE);

	width = right - left;
	height = bottom - top;
	midy = (top + bottom - 30) / 2;

	hItem = GetDlgItem(hDlg, IDC_BUTTON_UP);
	ypos = (midy > 46) ? midy - 46 : 0;
	MoveWindow(hItem, right-36, ypos, 33, 24, TRUE);
	ShowWindow(hItem, SW_SHOW);

	if (height > 230) {
		hItem = GetDlgItem(hDlg, IDC_BUTTON_UP10);
		MoveWindow(hItem, right-36, ypos-30, 33, 24, TRUE);
		ShowWindow(hItem, SW_SHOW);
	}

	hItem = GetDlgItem(hDlg, IDC_BUTTON_DOWN);
	ypos = (midy < bottom - 100) ? midy + 12 : bottom - 100;
	MoveWindow(hItem, right-36, ypos, 33, 24, TRUE);
	ShowWindow(hItem, SW_SHOW);

	if (height > 230) {
		hItem = GetDlgItem(hDlg, IDC_BUTTON_DOWN10);
		MoveWindow(hItem, right-36, ypos+30, 33, 24, TRUE);
		ShowWindow(hItem, SW_SHOW);
	}

	if (ListID == IDC_LIST_ADDRESS) {
		if (midy - 46 > 30) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_NUM);
			MoveWindow(hItem, right-36, top, 33, 24, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 280) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_ADD);
			MoveWindow(hItem, right-273, bottom-60, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 210) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_EDIT);
			MoveWindow(hItem, right-204, bottom-60, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 140) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_DELETE);
			MoveWindow(hItem, right-135, bottom-60, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 70) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_MAIL);
			MoveWindow(hItem, right-66, bottom-60, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 350) {
			hItem = GetDlgItem(hDlg, IDC_STATIC_TITLE);
			MoveWindow(hItem, left+15, bottom-30, 66, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		
			hItem = GetDlgItem(hDlg, IDC_ADDR_GRP_COMBOL);
			MoveWindow(hItem, left+80, bottom-30, width-224, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		ypos = height-69;
	} else {
		if (width > 210) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_ADD);
			MoveWindow(hItem, left+6, bottom-30, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 280) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_SAVE);
			MoveWindow(hItem,left+75, bottom-30, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 350) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_EDIT);
			MoveWindow(hItem, left+144, bottom-30, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		if (width > 420) {
			hItem = GetDlgItem(hDlg, IDC_BUTTON_DELETE);
			MoveWindow(hItem, left+213, bottom-30, 60, 21, TRUE);
			ShowWindow(hItem, SW_SHOW);
		}

		ypos = height-39;
	}

	if (width > 70) {
		hItem = GetDlgItem(hDlg, IDCANCEL);
		MoveWindow(hItem, right-66, bottom-30, 60, 21, TRUE);
		ShowWindow(hItem, SW_SHOW);
	}

	if (width > 140) {
		hItem = GetDlgItem(hDlg, IDOK);
		MoveWindow(hItem, right-135, bottom-30, 60, 21, TRUE);
		ShowWindow(hItem, SW_SHOW);
	}

	hItem = GetDlgItem(hDlg, ListID);
	MoveWindow(hItem, 3, 3, width-42, ypos, TRUE);
	UpdateWindow(hDlg);
}
#endif

/*
 * SetAddressList - リストビューにアドレス帳のリストを表示する
 */
static void SetAddressList(HWND hDlg, ADDRESSBOOK *tpAddressBook, TCHAR *Filter)
{
	HWND hAddrList;
	TCHAR *Group, *tmpGrp, *p, *q;
	int i, j, total, sel;
	int ItemIndex;

	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_ADDSTRING, 0, (LPARAM)STR_ADDRESSLIST_ALLGROUP);
	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_ADDSTRING, 0, (LPARAM)STR_ADDRESSLIST_NOGROUP);
	if (Filter != NULL && lstrcmp(Filter, STR_ADDRESSLIST_NOGROUP) == 0) {
		sel = 1;
	} else {
		sel = 0;
	}
	total = 2;

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
			// jump key
			TCHAR key[12];
			int len = 0;
			if (op.AddressJumpKey == 1) {
				p = item->Comment;
			} else {
				p = item->MailAddress;
			}
			while (p != NULL && *p != TEXT('\0') && len < 9) {
#ifndef UNICODE
				if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
					key[len++] = *(p++);
					key[len++] = *p;
				} else
#endif
				if (!IS_PAREN_QUOTE_T(*p)) {
					key[len++] = *p;
				}
				p++;
			}
			key[len] = TEXT('\0');
			ItemIndex = ListView_AddOptionItem(hAddrList, key, (long)item);
			ListView_SetItemText(hAddrList, ItemIndex, 1, item->MailAddress);
			ListView_SetItemText(hAddrList, ItemIndex, 2, item->Comment);
			ListView_SetItemText(hAddrList, ItemIndex, 3, Group);
			item->Displayed = TRUE;
		} else {
			item->Displayed = FALSE;
		}
		if (Group != NULL && *Group != TEXT('\0')) {
			p = Group;
			q = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Group) + 1));
			while (q != NULL && *p != TEXT('\0')) {
				BOOL Found = FALSE;
				int sortpos = total;
				p = str_cpy_f_t(q, p, TEXT(','));
				tmpGrp = q;
				while (*tmpGrp == TEXT(' ')) tmpGrp++;
				for (j = 0; j < total; j++) {
					TCHAR buf[BUF_SIZE];
					SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_GETLBTEXT, j, (LPARAM)buf);
					if (lstrcmp(buf, tmpGrp) == 0) {
						Found = TRUE;
						break;
					} else if (sortpos == total && lstrcmpi(buf, tmpGrp) > 0) {
						sortpos = j;
					}
				}

				if (Found == FALSE) {
					SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_INSERTSTRING, (sortpos >= total) ? -1 : sortpos, (LPARAM)tmpGrp);
					total++;
					if (Filter != NULL && lstrcmp(Filter, tmpGrp) == 0) {
						sel = sortpos;
					} else if (sel >= sortpos) {
						sel++;
					}
				}
			}
			mem_free(&q);
		}
	}
	SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, CB_SETCURSEL, sel, 0);

	if (op.AddressSort != 0) {
		ListView_SortItems(hAddrList, AddrCompareFunc, op.AddressSort);
	}

	if (tpAddressBook->EditNum >= 0) {
		ListView_SetItemState(hAddrList, -1, 0, LVIS_SELECTED);
		for (j = 0; j < ListView_GetItemCount(hAddrList); j++) {
			i = ((ADDRESSITEM *)ListView_GetlParam(hAddrList, j))->Num;
			if (i == tpAddressBook->EditNum) {
				ListView_SetItemState(hAddrList, j,
					LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
				ListView_EnsureVisible(hAddrList, j, TRUE);
				break;
			}
		}
	}
}

/*
 * EditAddressProc - アドレス編集プロシージャ
 *                   (significant modifications by GJC)
 */
static BOOL CALLBACK EditAddressProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hAddrList = NULL;
	ADDRESSBOOK *tpTmpAddressBook;
	ADDRESSITEM *AddrItem;
	TCHAR buf[BUF_SIZE];
	int i, j, total, idx, sel;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_EDITADDRESS, TRUE);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		tpTmpAddressBook = (ADDRESSBOOK *)lParam;
		idx = tpTmpAddressBook->EditNum;
		if (idx >= 0) {
			AddrItem = *(tpTmpAddressBook->tpAddrItem + idx);
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)AddrItem->MailAddress);
			SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, WM_SETTEXT, 0, (LPARAM)AddrItem->Comment);
		} else if (idx == -1) {
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)STR_EXAMPLE_ADDRESS);
		}
		SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
#ifdef _WIN32_WCE
		if (tpTmpAddressBook->FromAddrInfo == TRUE && op.UsePOOMAddressBook != 0) {
			SendDlgItemMessage(hDlg, IDC_COMMENT_OR_NAME, WM_SETTEXT, 0, (LPARAM)STR_ADDREDIT_NAME);
		}
#endif
		SendDlgItemMessage(hDlg, IDC_EDIT_COMMENT, EM_LIMITTEXT, (WPARAM)BUF_SIZE - 2, 0);
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS),
			GWL_WNDPROC, (DWORD)EditTextCallback);
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_COMMENT),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
		// idx==-1, add new address item
		if (idx == -2) {
			// changing Group of multiple addresses
			SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)STR_ADDRESSBOOK_ADDGROUP);
			SendDlgItemMessage(hDlg, IDC_EDIT_MAILADDRESS, WM_SETTEXT, 0, (LPARAM)STR_MULTIPLE_ADDRESSES);
			SendDlgItemMessage(hDlg, IDC_RADIO_GROUP_ADD, BM_SETCHECK, 1, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_MAILADDRESS), 0);
			EnableWindow(GetDlgItem(hDlg, IDC_EDIT_COMMENT), 0);
		} else {
			ShowWindow(GetDlgItem(hDlg, IDC_RADIO_GROUP_ADD), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_RADIO_GROUP_DEL), SW_HIDE);
		}

		SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_ADDSTRING, 0, (LPARAM)TEXT(""));
		sel = 0;
		total = 1;
		for (i = 0; i < tpTmpAddressBook->ItemCnt; i++) {
			TCHAR *Group, *p, *newgrp;
			int sortpos1, sortpos2;
			BOOL Found = FALSE;
			sortpos1 = total;
			Group = (*(tpTmpAddressBook->tpAddrItem + i))->Group;
			if (Group == NULL || *Group == TEXT('\0')) continue;
			newgrp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Group) + 1));
			p = Group;
			while (*p != TEXT('\0')) {
				BOOL Found2 = FALSE;
				p = str_cpy_f_t(newgrp, p, TEXT(','));
				while (*p == TEXT(' ')) p++;
				sortpos2 = total;
				for (j = 0; j < total; j++) {
					SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_GETLBTEXT, j, (LPARAM)buf);
					if (lstrcmp(buf, Group) == 0) {
						Found = TRUE;
					} else if (sortpos2 == total && lstrcmpi(buf, Group) > 0) {
						sortpos2 = j;
					}
					if (lstrcmp(buf, newgrp) == 0) {
						Found2 = TRUE;
						if (Found == TRUE) {
							break;
						}
					} else if (sortpos1 == total && lstrcmpi(buf, newgrp) > 0) {
						sortpos1 = j;
					}
				}
				if (Found2 == FALSE) {
					SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_INSERTSTRING, (sortpos2 >= total) ? -1 : sortpos2, (LPARAM)newgrp);
					total++;
					if (lstrcmpi(Group, newgrp) > 0) {
						sortpos1++;
					}
					if (tpTmpAddressBook->EditNum >= 0) {
						if (lstrcmp(newgrp, AddrItem->Group) == 0) {
							sel = sortpos2;
						} else if (sel >= sortpos2) {
							sel++;
						}
					}
					if (lstrcmp(Group, newgrp) == 0) {
						Found = TRUE;
					}
				}
			}
			mem_free(&newgrp);
			if (Found == FALSE && tpTmpAddressBook->EditNum != -2) {
				SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, CB_INSERTSTRING, (sortpos1 >= total) ? -1 : sortpos1, (LPARAM)Group);
				total++;
				if (tpTmpAddressBook->EditNum >= 0) {
					if (lstrcmp(Group, AddrItem->Group) == 0) {
						sel = sortpos1;
					} else if (sel >= sortpos1) {
						sel++;
					}
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
			idx = tpTmpAddressBook->EditNum;
			if (idx == -1) {
				AddrItem = (ADDRESSITEM *)mem_calloc(sizeof(ADDRESSITEM));
				if (AddrItem == NULL || addr_add(tpTmpAddressBook, AddrItem) == FALSE) {
					EndDialog(hDlg, FALSE);
					break;
				}
				AddrItem->Num = tpTmpAddressBook->ItemCnt - 1;
			} else if (idx >= 0) {
				AddrItem = *(tpTmpAddressBook->tpAddrItem + idx);
			}
			if (tpTmpAddressBook->FromAddrInfo == FALSE) {
				hAddrList = GetDlgItem(GetParent(hDlg), IDC_LIST_ADDRESS);
				sel = ListView_GetNextItem(hAddrList, -1, LVIS_SELECTED);
#ifdef _DEBUG
				if (idx >= 0  && idx != ((ADDRESSITEM *)ListView_GetlParam(hAddrList, sel))->Num) {
					MessageBox(hDlg, TEXT("mismatch 2"), TEXT("gjc debug"), MB_OK);
				}
#endif
			}
			if (idx != -2) {
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
				if (sel >= 0 && hAddrList != NULL) {
					ListView_SetItemText(hAddrList, sel, 1, buf);
				}
				*buf = TEXT('\0');
				mem_free(&AddrItem->AddressOnly);
				GetMailAddress(AddrItem->MailAddress, buf, NULL, FALSE);
				AddrItem->AddressOnly = alloc_copy_t(buf);

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
				if (sel >= 0 && hAddrList != NULL) {
					ListView_SetItemText(hAddrList, sel, 2, buf);
				}

				{
					// sort key
					TCHAR *p, key[12];
					int len = 0;
					if (op.AddressJumpKey == 1) {
						p = AddrItem->Comment;
					} else {
						p = AddrItem->MailAddress;
					}
					while (p != NULL && *p != TEXT('\0') && len < 9) {
#ifndef UNICODE
						if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
							key[len++] = *(p++);
							key[len++] = *p;
						} else
#endif
						if (!IS_PAREN_QUOTE_T(*p)) {
							key[len++] = *p;
						}
						p++;
					}
					key[len] = TEXT('\0');
					if (sel >= 0 && hAddrList != NULL) {
						ListView_SetItemText(hAddrList, sel, 0, key);
					}
				}
			}

			//Group/Category
			*buf = TEXT('\0');
			SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBO, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			delete_ctrl_char(buf);
			if (idx == -2) { // changing Group of multiple addresses
				if (*buf != TEXT('\0')) {
					int add = SendDlgItemMessage(hDlg, IDC_RADIO_GROUP_ADD, BM_GETCHECK, 0, 0);
					while (sel >= 0 && hAddrList != NULL) {
						i = ((ADDRESSITEM *)ListView_GetlParam(hAddrList, sel))->Num;
						AddrItem = *(tpTmpAddressBook->tpAddrItem + i);
						if (add) {
							if (AddrItem->Group == NULL) {
								AddrItem->Group = alloc_copy_t(buf);
							} else if (lstrcmp(AddrItem->Group, buf) != 0) {
								TCHAR *old, *p;
								BOOL Found = FALSE;
								old = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(AddrItem->Group) + 1));
								if (old != NULL) {
									p = AddrItem->Group;
									while (*p != TEXT('\0')) {
										p = str_cpy_f_t(old, p, TEXT(','));
										while (*p == TEXT(' ')) p++;
										if (lstrcmp(old, buf) == 0) {
											Found = TRUE;
											break;
										}
									}
									mem_free(&old);
								}
								if (Found == FALSE) {
									TCHAR *tmp;
									int len = lstrlen(AddrItem->Group) + lstrlen(buf) + 3;
									tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
									if (tmp == NULL) {
										EndDialog(hDlg, FALSE);
										break;
									}
									wsprintf(tmp, TEXT("%s, %s"), AddrItem->Group, buf);
									mem_free(&AddrItem->Group);
									AddrItem->Group = tmp;
								}
							}
						} else if (AddrItem->Group != NULL) { // delete
							if (lstrcmp(AddrItem->Group, buf) == 0) {
								mem_free(&AddrItem->Group);
								AddrItem->Group = NULL;
							} else {
								TCHAR *old, *tmp, *p, *q;
								BOOL first = TRUE;
								int len = lstrlen(AddrItem->Group) + 1;
								old = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
								tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
								if (old == NULL || tmp == NULL) {
									mem_free(&old);
									mem_free(&tmp);
									EndDialog(hDlg, FALSE);
									break;
								}
								p = AddrItem->Group;
								q = tmp;
								while (*p != TEXT('\0')) {
									p = str_cpy_f_t(old, p, TEXT(','));
									while (*p == TEXT(' ')) p++;
									if (lstrcmp(old, buf) != 0) {
										if (first == FALSE) {
											*(q++) = TEXT(',');
										}
										q = str_cpy_t(q, old);
										first = FALSE;
									}
								}
								mem_free(&old);
								mem_free(&AddrItem->Group);
								AddrItem->Group = alloc_copy_t(tmp);
								mem_free(&tmp);
							}
						}
						ListView_SetItemText(hAddrList, sel, 3, AddrItem->Group);
						sel = ListView_GetNextItem(hAddrList, sel, LVIS_SELECTED);
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
				if (sel >= 0 && hAddrList != NULL) {
					ListView_SetItemText(hAddrList, sel, 3, buf);
				}
			}
#ifdef _WIN32_WCE
			if (idx == -1) {
				if (op.UsePOOMAddressBook != 0) {
					if (MessageBox(hDlg, STR_Q_ADDPOOM, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDYES) {
						TCHAR *fname = NULL, *lname = NULL;
						int len = lstrlen(AddrItem->MailAddress) + 1;
						if (len > 1) {
							TCHAR *addr, *cmmt;
							addr = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
							cmmt = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
							*addr = *cmmt = TEXT('\0');
							GetMailAddress(AddrItem->MailAddress, addr, cmmt, FALSE);
							mem_free(&addr);
							len = lstrlen(cmmt) + 1;
							if (len <= 1) {
								mem_free(&cmmt);
								cmmt = alloc_copy_t(AddrItem->Comment);
								len = lstrlen(cmmt) + 1;
							}
							if (len > 1) {
								fname = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
								lname = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
								*fname = *lname = TEXT('\0');
								GetNameFromComment(cmmt, fname, lname);
							}
							mem_free(&cmmt);
						}
						AddPOOMContact(AddrItem->AddressOnly, fname, lname, AddrItem->Group);
						mem_free(&fname);
						mem_free(&lname);
					}
				}
			}
#endif
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
 * SubClassAddrListProc
 */
#ifdef _WIN32_WCE
static LRESULT CALLBACK SubClassAddrListProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32_WCE_PPC
	if (msg == WM_LBUTTONDOWN) {
		SHRGINFO rg;
		LV_HITTESTINFO lvht;
		POINT apos;
		int sel;

		apos.x = LOWORD(lParam);
		apos.y = HIWORD(lParam);
		lvht.pt = apos;
		lvht.flags = LVHT_ONITEMICON | LVHT_ONITEMLABEL | LVHT_ONITEMSTATEICON;
		lvht.iItem = 0;
		sel = ListView_HitTest(hDlg, &lvht);
		if (sel >= 0 && GetKeyState(VK_CONTROL) >= 0 && GetKeyState(VK_SHIFT) >= 0) {
			ListView_SetItemState(hDlg, sel, 
				LVIS_FOCUSED | (ListView_GetItemState(hDlg, sel, LVIS_SELECTED) ^ LVIS_SELECTED),
				LVIS_FOCUSED | LVIS_SELECTED);
		}

		rg.cbSize = sizeof(SHRGINFO);
		rg.hwndClient = hDlg;
		rg.ptDown.x = LOWORD(lParam);
		rg.ptDown.y = HIWORD(lParam);
		rg.dwFlags = SHRG_RETURNCMD;
		if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
			if (op.UsePOOMAddressBook == 0 && ListView_GetSelectedCount(hDlg) > 0) {
				ShowMenu(GetParent(hDlg), hADPOPUP, 0, 0);
			}
			return 0;
		}
	}
#else
	if (msg == WM_LBUTTONUP && GetKeyState(VK_MENU) < 0 && op.UsePOOMAddressBook == 0) {
		ShowMenu(GetParent(hDlg), hADPOPUP, 0, 0);
		return 0;
	}
#endif
	return CallWindowProc(AddrListProcedure, hDlg, msg, wParam, lParam);
}
#endif

/*
 * AddressListProc - アドレス帳設定プロシージャ
 */
BOOL CALLBACK AddressListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ADDRESSBOOK *tpTmpAddressBook;
	HWND hListView;
	DWORD hiw;
	TCHAR *StrAddr;
	TCHAR buf[BUF_SIZE];
	int SelectItem, i, move = 1;
	BOOL enable;

	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		InitDlg(hDlg, STR_TITLE_ADDRESSLIST, TRUE);
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
		SetControlFont(hDlg);

		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		tpTmpAddressBook = (ADDRESSBOOK *)lParam;
		if (tpTmpAddressBook->GetAddrList == TRUE) {
			SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_MAIL), STR_ADDRESS_SELECT);
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
			SHSipPreference(hDlg, SIP_UP);
#endif
		}

		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MAIL), FALSE);

#ifdef _WIN32_WCE
		///////////// MRP /////////////////////
		if (op.UsePOOMAddressBook != 0) {
//			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), FALSE);
//			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_ADD), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), SW_HIDE);
		}
		///////////// --- /////////////////////
#endif
		
		hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
		ListView_AddColumn(hListView, LVCFMT_LEFT, 0, TEXT("jump key"), 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[0], STR_ADDRESSLIST_MAILADDRESS, 1);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[1], STR_ADDRESSLIST_COMMENT, 2);
		ListView_AddColumn(hListView, LVCFMT_LEFT, op.AddColSize[2], STR_ADDRESSLIST_GROUP, 3);
		ListView_SetExtendedListViewStyle(hListView,
			LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

		tpTmpAddressBook->EditNum = -1;
		AddrSortFlag = op.AddressSort;
		if (op.AddressSort != 0) {
			enable = FALSE;
		} else {
			enable = TRUE;
		}
		SetAddressList(hDlg, tpTmpAddressBook, 
			(op.AddressShowGroup == NULL || *op.AddressShowGroup == TEXT('\0'))
			? STR_ADDRESSLIST_ALLGROUP : op.AddressShowGroup);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP10), enable);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), enable);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), enable);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN10), enable);
		SetTimer(hDlg, ID_RESIZE_TIMER, 1, NULL);
#ifdef _WIN32_WCE
		AddrListProcedure = (WNDPROC)SetWindowLong(hListView, GWL_WNDPROC, (DWORD)SubClassAddrListProc);
#endif
		break;

	case WM_TIMER:
		if (wParam == ID_RESIZE_TIMER) {
			int width, height, xsize, ysize;
			width = op.AddrRect.right - op.AddrRect.left;
			height = op.AddrRect.bottom - op.AddrRect.top;
			xsize = GetSystemMetrics(SM_CXSCREEN);
			ysize = GetSystemMetrics(SM_CYSCREEN);
#ifdef _WIN32_WCE_PPC
			// in case of landscape/portrait swap
			if (width > xsize) {
				width = xsize - 10;
			}
			if (height > ysize) {
				height = ysize - 10 - MENU_HEIGHT;
			}
#endif
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpTmpAddressBook != NULL && tpTmpAddressBook->GetAddrList == TRUE) {
				RECT rcClient;
				GetWindowRect(hDlg, &rcClient);
				MoveWindow(hDlg, rcClient.left, rcClient.top, width, height, TRUE);
			} else {
				if (op.AddrRect.left > xsize) {
					op.AddrRect.left = xsize / 2;
				}
				if (op.AddrRect.top > ysize) {
					op.AddrRect.top = ysize / 2;
				}
				MoveWindow(hDlg, op.AddrRect.left, op.AddrRect.top, width, height, TRUE);
			}
		}
		KillTimer(hDlg, wParam);
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

	case WM_SIZE:
		if (hDlg != NULL) {
			RECT rcWin;
			GetClientRect(hDlg, &rcWin);
			SetWindowSize(hDlg, IDC_LIST_ADDRESS, rcWin.top, rcWin.bottom,
				rcWin.left, rcWin.right);
		}
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_NOTIFY:
		DialogLvNotifyProc(hDlg, lParam, IDC_LIST_ADDRESS);
		if (op.AddressSort != 0) {
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP10), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN10), FALSE);
		}
		{
			LV_DISPINFO *plv = (LV_DISPINFO *)lParam;
			if (plv->hdr.code == LVN_ITEMCHANGED) {
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 1);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MAIL), 1);
			}
		}
		break;

	case WM_LV_EVENT:
		if (wParam == NM_CLICK) {
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if (ListView_GetSelectedCount(hListView) <= 0) {
				enable = FALSE;
			} else {
				enable = TRUE;
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), enable);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MAIL), enable);
		}
		break;

#ifndef _WIN32_WCE
	case WM_CONTEXTMENU:
		ShowMenu(hDlg, hADPOPUP, 0, 0);
		break;
#endif

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_LV_ALLSELECT:
			ListView_SetItemState(GetDlgItem(hDlg, IDC_LIST_ADDRESS), -1, LVIS_SELECTED, LVIS_SELECTED);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_EDIT), 1);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DELETE), 1);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_MAIL), 1);
			break;

		case IDC_BUTTON_NUM:
			// sort by number
			if (op.AddressSort != 0) {
				op.AddressSort = 0;
				// AddrSortFlag = 0; ?
				hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
				SwitchCursor(FALSE);
				ListView_SortItems(hListView, AddrCompareFunc, op.AddressSort);
				SwitchCursor(TRUE);
				ListView_EnsureVisible(hListView,
					ListView_GetNextItem(hListView, -1, LVNI_FOCUSED), TRUE);
			}
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP10), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_UP), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_DOWN10), TRUE);
			break;

		case IDC_BUTTON_UP10:
			move = 10;
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
				addr_move(tpTmpAddressBook, ((ADDRESSITEM *)ListView_GetlParam(hListView, SelectItem))->Num, -1 * move);
				ListView_SetItemState(hListView, SelectItem, 0, LVIS_SELECTED);
				ListView_MoveItem(hListView, SelectItem, -1 * move, 4);
				//ListView_SetItemState(hListView, SelectItem - 1, LVIS_SELECTED, LVIS_SELECTED);
			}
			//SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			//SetAddressList(hDlg, tpTmpAddressBook, buf);
			break;

		case IDC_BUTTON_DOWN10:
			move = 10;
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
					addr_move(tpTmpAddressBook, ((ADDRESSITEM *)ListView_GetlParam(hListView, i))->Num, 1 * move);
					ListView_SetItemState(hListView, i, 0, LVIS_SELECTED);
					ListView_MoveItem(hListView, i, 1 * move, 4);
					//ListView_SetItemState(hListView, i + 1, LVIS_SELECTED, LVIS_SELECTED);
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
#ifdef _WIN32_WCE
			if (op.UsePOOMAddressBook != 0) {
				break;
			}
#endif
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			i = ListView_GetSelectedCount(GetDlgItem(hDlg, IDC_LIST_ADDRESS));
			if (i > 1) {
				// changing Group of multiple addresses
				tpTmpAddressBook->EditNum = -2;
			} else if (i == 1) {
				i = ListView_GetNextItem(hListView, -1, LVNI_FOCUSED | LVIS_SELECTED);
				if (i == -1) break;
				tpTmpAddressBook->EditNum = ((ADDRESSITEM *)ListView_GetlParam(hListView, i))->Num;
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
#ifdef _WIN32_WCE
			if (op.UsePOOMAddressBook != 0) {
				break;
			}
#endif
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
				addr_delete(tpTmpAddressBook, ((ADDRESSITEM *)ListView_GetlParam(hListView, SelectItem))->Num);
				ListView_DeleteItem(hListView, SelectItem);
			}
			//SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
			//SetAddressList(hDlg, tpTmpAddressBook, buf);
			SendMessage(hDlg, WM_LV_EVENT, NM_CLICK, 0);
			break;

		case IDC_BUTTON_MAIL:
			// Compose new message, or add addresses to Property dialog
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			if ((SelectItem = ListView_GetSelectedCount(hListView)) <= 0) {
				ErrorMessage(hDlg, STR_ERR_SELECTMAILADDR);
				break;
			}

			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpTmpAddressBook->GetAddrList == FALSE) {
				StrAddr = ListView_GetSelStringList(hListView);
				if (StrAddr == NULL) {
					ErrorMessage(hDlg, STR_ERR_MEMALLOC);
					break;
				}

				if (Edit_MailToSet(hInst, MainWnd, StrAddr, -1) == EDIT_INSIDEEDIT) {
#ifdef _WIN32_WCE
					ShowWindow(MainWnd, SW_HIDE);
#endif
				}
				mem_free(&StrAddr);
			}
			SendMessage(hDlg, WM_COMMAND, IDOK, 0);
			break;

		case IDC_ADDR_GRP_COMBOL:
			hiw = HIWORD(wParam);
			if (hiw == CBN_CLOSEUP || hiw == CBN_SELCHANGE) {
				hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
				ListView_SetRedraw(hListView, FALSE);
				SendDlgItemMessage(hDlg, IDC_ADDR_GRP_COMBOL, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)buf);
				tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
				tpTmpAddressBook->EditNum = -1;
				SetAddressList(hDlg, tpTmpAddressBook, buf);
				ListView_SetRedraw(hListView, TRUE);
				SendMessage(hDlg, WM_LV_EVENT, NM_CLICK, 0);
				if (hiw == CBN_CLOSEUP) {
					SetFocus(hListView);
				}
			}
			break;

		case IDOK:
			hListView = GetDlgItem(hDlg, IDC_LIST_ADDRESS);
			tpTmpAddressBook = (ADDRESSBOOK *)GetWindowLong(hDlg, GWL_USERDATA);
			if (tpTmpAddressBook->GetAddrList == TRUE) {
				if (ListView_GetSelectedCount(hListView) > 0) {
					tpTmpAddressBook->AddrList = ListView_GetSelStringList(hListView);
					if (tpTmpAddressBook->AddrList == NULL) {
						ErrorMessage(hDlg, STR_ERR_MEMALLOC);
						break;
					}
				}
			}

			if (AddressBook->tpAddrItem != NULL) {
				addr_free(AddressBook->tpAddrItem, AddressBook->ItemCnt);
				mem_free((void **)&AddressBook->tpAddrItem);
			}
			AddressBook->tpAddrItem = tpTmpAddressBook->tpAddrItem;
			AddressBook->ItemCnt = tpTmpAddressBook->ItemCnt;
#ifdef _DEBUG
			for (i = 0; i < AddressBook->ItemCnt; i++) {
				SelectItem = (*(AddressBook->tpAddrItem + i))->Num;
				if (SelectItem != i) {
					MessageBox(hDlg, TEXT("mismatch"), TEXT("gjc debug"), MB_OK);
				}
			}
#endif
			tpTmpAddressBook->tpAddrItem = NULL;
			tpTmpAddressBook->ItemCnt = 0;

			if (op.AutoSave == 1) {
				// アドレス帳を保存
				file_save_address_book(ADDRESS_FILE, DataDir, AddressBook);
			}
			for (i = 0; i < AD_COL_CNT; i++) {
				op.AddColSize[i] = ListView_GetColumnWidth(hListView, i+1);
			}
			GetWindowRect(hDlg, &op.AddrRect);
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
	BOOL enable = FALSE;
	int command;
	switch (uMsg) {
	case WM_INITDIALOG:
#ifdef _WIN32_WCE_PPC
		if (FindOrReplace < 2) {
			InitDlg(hDlg, STR_TITLE_FIND, TRUE);
		} else {
			InitDlg(hDlg, STR_TITLE_REPLACE, FALSE);
		}
#elif defined(_WIN32_WCE)
		InitDlg(hDlg);
#endif
#ifndef _WIN32_WCE_PPC
		if (FindOrReplace >= 2) {
			SetWindowText(hDlg, STR_TITLE_REPLACE);
		}
#endif
		SetControlFont(hDlg);
		if (FindStr != NULL) {
			TCHAR *buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FindStr) * 2 + 1));
			*buf = TEXT('\0');
			EncodeCtrlChar(FindStr, buf);
			SendDlgItemMessage(hDlg, IDC_EDIT_FIND, WM_SETTEXT, 0, (LPARAM)buf);
			mem_free(&buf);
		}
		SendDlgItemMessage(hDlg, IDC_CHECK_CASE, BM_SETCHECK, op.MatchCase, 0);
		if (FindOrReplace >= 2) { // Replace
#ifdef _WIN32_WCE_PPC
			DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_REPLACE),
				GWL_WNDPROC, (DWORD)EditTextCallback);
#else
			ShowWindow(GetDlgItem(hDlg, IDC_TEXT_SEARCH), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_FIND_THISMSG), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_FIND_ALLMSGS), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_FIND_ALLBOXES), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_CHECK_SUBJECT), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_FIND_NEXT_MESSAGE), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_FIND_NEXT_MAILBOX), SW_HIDE);
#endif
			SetWindowText(GetDlgItem(hDlg, IDCANCEL), STR_REPLACE_DONE);
			if (ReplaceStr != NULL) {
				TCHAR *buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FindStr) * 2 + 1));
				*buf = TEXT('\0');
				EncodeCtrlChar(ReplaceStr, buf);
				SendDlgItemMessage(hDlg, IDC_EDIT_REPLACE, WM_SETTEXT, 0, (LPARAM)buf);
				mem_free(&buf);
			}
		} else { // Find
			if (op.AllBoxFind) {
				SendDlgItemMessage(hDlg, IDC_FIND_ALLBOXES, BM_SETCHECK, 1, 0);
			} else if (op.AllMsgFind) {
				SendDlgItemMessage(hDlg, IDC_FIND_ALLMSGS, BM_SETCHECK, 1, 0);
			} else {
				SendDlgItemMessage(hDlg, IDC_FIND_THISMSG, BM_SETCHECK, 1, 0);
			}
			SendDlgItemMessage(hDlg, IDC_CHECK_SUBJECT, BM_SETCHECK, op.SubjectFind, 0);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SUBJECT), op.AllMsgFind || op.AllBoxFind);
			EnableWindow(GetDlgItem(hDlg, IDC_FIND_NEXT_MESSAGE), op.AllMsgFind || op.AllBoxFind);
			EnableWindow(GetDlgItem(hDlg, IDC_FIND_NEXT_MAILBOX), op.AllBoxFind);
#ifndef _WIN32_WCE_PPC
			ShowWindow(GetDlgItem(hDlg, IDC_REPLACE_LABEL), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_EDIT_REPLACE), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_REPLACE), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, IDC_REPLACE_ALL), SW_HIDE);
#endif
		}
#ifdef _WIN32_WCE_PPC
		DefEditTextWndProc = (WNDPROC)SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_FIND),
			GWL_WNDPROC, (DWORD)EditTextCallback);
#endif
		break;

	case WM_CLOSE:
#ifdef _WIN32_WCE_PPC
		 SetWindowLongW(GetDlgItem(hDlg, IDC_EDIT_FIND), GWL_WNDPROC, (DWORD)DefEditTextWndProc);
#endif
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		command = LOWORD(wParam);
		switch (command) {
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		case IDC_EDIT_FIND:
		case IDC_EDIT_REPLACE:
			SetSip(hDlg, HIWORD(wParam));
			break;
#endif

		case IDC_FIND_THISMSG:
		case IDC_FIND_ALLMSGS:
		case IDC_FIND_ALLBOXES:
			if (FindOrReplace == 1) {
				enable = SendDlgItemMessage(hDlg, IDC_FIND_THISMSG, BM_GETCHECK, 0, 0) ? 0 : 1;
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SUBJECT), enable);
				EnableWindow(GetDlgItem(hDlg, IDC_FIND_NEXT_MESSAGE), enable);
				enable = SendDlgItemMessage(hDlg, IDC_FIND_ALLBOXES, BM_GETCHECK, 0, 0);
				EnableWindow(GetDlgItem(hDlg, IDC_FIND_NEXT_MAILBOX), enable);
			}
			break;

		case IDC_REPLACE_ALL:
			if (FindOrReplace >= 2) {
				FindOrReplace = 3;
				ReplaceCnt = 0;
			}
			// fall through
		case IDC_REPLACE:
			if (FindOrReplace >= 2) {
				AllocGetText(GetDlgItem(hDlg, IDC_EDIT_FIND), &FindStr);
				DecodeCtrlChar(FindStr, FindStr);
				if (FindStr == NULL || *FindStr == TEXT('\0')) {
					ErrorMessage(hDlg, STR_ERR_INPUTFINDSTRING);
					SetFocus(GetDlgItem(hDlg, IDC_EDIT_FIND));
					break;
				}
				AllocGetText(GetDlgItem(hDlg, IDC_EDIT_REPLACE), &ReplaceStr);
				DecodeCtrlChar(ReplaceStr, ReplaceStr);
				if (lstrcmp(FindStr, ReplaceStr) == 0) {
					ErrorMessage(hDlg, STR_ERR_FINDISREPLACE);
					SetFocus(GetDlgItem(hDlg, IDC_EDIT_REPLACE));
					break;
				}
				{
					HWND hEdit = GetDlgItem(GetParent(hDlg), IDC_EDIT_BODY);
					if (hEdit != NULL) {
						int i, j;
						if (command == IDC_REPLACE) {
							FindOrReplace = 2;
							SendMessage(hEdit, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
							if (i < j) {
								TCHAR *buf = NULL;
								AllocGetText(hEdit, &buf);
								if (buf != NULL) {
									int len = lstrlen(FindStr);
									if ( ((j-i) == len) &&
										(op.MatchCase == FALSE && str_cmp_ni_t(FindStr, buf+i, len) == 0)
										|| (op.MatchCase == TRUE && str_cmp_n_t(FindStr, buf+i, len) == 0)) {
										FindOrReplace = 4;
									}
									mem_free(&buf);
								}
							}
						} else {
							SendMessage(hEdit, EM_SETSEL, 0, 0);
						}
						SendMessage(hDlg, WM_COMMAND, IDC_REPLACE_AGAIN, 0);
					}
				}
			}
			break;

		case IDC_FIND_NEXT_MESSAGE:
		case IDC_FIND_NEXT_MAILBOX:
			// above two commands shouldn't be accessible in Replace
{
	TCHAR buf[BUF_SIZE];
	wsprintf(buf, TEXT("Find next, cmd=%d; FindOrReplace=%d\n"), command, FindOrReplace);
	log_save(buf);
}
		case IDOK:
			if (FindOrReplace >= 2) {
				SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
				break;
			}
		case IDC_FIND:
			if (FindOrReplace > 2) {
				FindOrReplace = 2;
			}
		case IDC_REPLACE_AGAIN:
			AllocGetText(GetDlgItem(hDlg, IDC_EDIT_FIND), &FindStr);
			DecodeCtrlChar(FindStr, FindStr);
			if (FindStr == NULL || *FindStr == TEXT('\0')) {
				ErrorMessage(hDlg, STR_ERR_INPUTFINDSTRING);
				SetFocus(GetDlgItem(hDlg, IDC_EDIT_FIND));
				break;
			}
			FindNext = 0;
			op.MatchCase = SendDlgItemMessage(hDlg, IDC_CHECK_CASE, BM_GETCHECK, 0, 0);
			if (FindOrReplace >= 2) {
				HWND hEdit = GetDlgItem(GetParent(hDlg), IDC_EDIT_BODY);
				BOOL show_msg = TRUE, done = FALSE;
				if (FindOrReplace == 4) {
					SendMessage(hEdit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)ReplaceStr);
					FindOrReplace = 2;
					show_msg = FALSE;
				}
				while (done == FALSE) {
					BOOL found = FindEditString(hEdit, FindStr, op.MatchCase,
							(FindOrReplace == 3) ? FALSE : TRUE);
					if (found && FindOrReplace == 3) {
						SendMessage(hEdit, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)ReplaceStr);
						ReplaceCnt++;
					} else {
						done = TRUE;
						if (found) {
							show_msg = FALSE;
						}
					}
				}
				if (show_msg == TRUE) {
					TCHAR *msg, *title;
					if (FindOrReplace == 3 && ReplaceCnt > 0) {
						msg = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(STR_MSG_REPLACED_N) + 7));
					} else {
						msg = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FindStr) + lstrlen(STR_MSG_NOFIND) + 1));
					}
					if (msg == NULL) {
						EndDialog(hDlg, FALSE);
					}
					if (FindOrReplace == 3 && ReplaceCnt > 0) {
						if (ReplaceCnt > 999999) ReplaceCnt = 999999;
						wsprintf(msg, STR_MSG_REPLACED_N, ReplaceCnt);
						title = STR_TITLE_FIND;
					} else {
						wsprintf(msg, STR_MSG_NOFIND, FindStr);
						title = STR_TITLE_REPLACE;
					}
					MessageBox(hDlg, msg, title, MB_ICONINFORMATION);
					mem_free(&msg);
				}
			} else {
				op.AllBoxFind = SendDlgItemMessage(hDlg, IDC_FIND_ALLBOXES, BM_GETCHECK, 0, 0);
				if (op.AllBoxFind) {
					op.AllMsgFind = 1;
				} else {
					op.AllMsgFind = SendDlgItemMessage(hDlg, IDC_FIND_ALLMSGS, BM_GETCHECK, 0, 0);
				}
				op.SubjectFind = SendDlgItemMessage(hDlg, IDC_CHECK_SUBJECT, BM_GETCHECK, 0, 0);
				if (command == IDC_FIND_NEXT_MESSAGE) {
					FindNext = 1;
				} else if (command == IDC_FIND_NEXT_MAILBOX) {
					FindNext = 2;
				}
#ifdef _WIN32_WCE_PPC
				SHSipPreference(hDlg, SIP_DOWN);
#elif defined _WIN32_WCE_LAGENDA
				SipShowIM(SIPF_OFF);
#endif
				EndDialog(hDlg, TRUE);
			}
			break;

		case IDCANCEL:
#ifdef _WIN32_WCE_PPC
			SHSipPreference(hDlg, SIP_DOWN);
#elif defined _WIN32_WCE_LAGENDA
			SipShowIM(SIPF_OFF);
#endif
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
			if ((MailBox + i)->NewMail > 0) {
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
			if ((MailBox + i)->NewMail > 0) {
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
		InitDlg(hDlg, STR_TITLE_ATTACH, TRUE);
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
	int i, j;
	int *sel;
	BOOL skip_vselbox = FALSE;
	static int last_selected = 1;
	
	switch(uMsg)
	{
	case WM_INITDIALOG:
		sel = (int *)lParam;
		i = *sel;
		if (i >= VSELBOX_FLAG) {
			i -= VSELBOX_FLAG;
			skip_vselbox = TRUE;
		}
		SetWindowText(GetDlgItem(hDlg, IDC_MOVECOPYTEXT), i ? STR_TITLE_MOVE2 : STR_TITLE_COPY2);
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, CB_ADDSTRING, 0, (LPARAM)STR_LIST_MENU_NEW);
		for (i = MAILBOX_USER, j = 0; i < MailBoxCnt; i++) {
			if ((i == SelBox && skip_vselbox == FALSE) || (i == vSelBox && skip_vselbox == TRUE)) {
				continue;
			}
			if ((MailBox+i)->Type == MAILBOX_TYPE_SAVE) {
				SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, CB_ADDSTRING, 0, (LPARAM)(MailBox+i)->Name);
				j++;
			}
		}
		if (last_selected == 0) {
			last_selected = j; // the new one created last time
		}
		SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, CB_SETCURSEL, last_selected, 0);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			last_selected = SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, CB_GETCURSEL, 0, 0);
			sel = (int*)GetWindowLong(hDlg, GWL_USERDATA);
			if (last_selected == 0) {
				*sel = 0;
			} else {
				SendDlgItemMessage(hDlg, IDC_SAVEBOX_COMBO, WM_GETTEXT, BUF_SIZE - 1, (LPARAM)Name);
				*sel = mailbox_name_to_index(Name);
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
 * DigestReplyProc - set Reply for digest messages (GJC)
*/
BOOL CALLBACK DigestReplyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int *what;
	switch (uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hDlg, GWL_USERDATA, lParam);
		SendDlgItemMessage(hDlg, IDC_RADIO_DIGEST_LIST, BM_SETCHECK, BST_CHECKED, 0);
		break;

	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			what = (int*)GetWindowLong(hDlg, GWL_USERDATA);
			if (SendDlgItemMessage(hDlg, IDC_RADIO_DIGEST_ALL, BM_GETCHECK, 0, 0)) {
				*what = EDIT_REPLYALL;
			} else if (SendDlgItemMessage(hDlg, IDC_RADIO_DIGEST_SENDER, BM_GETCHECK, 0, 0)) {
				*what = EDIT_REPLY;
			} else {
				*what = 1; // reply to list
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
		SetWindowText(GetDlgItem(hDlg, IDC_VISIT_WEB), STR_WEB_ADDR);
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
		case IDC_VISIT_WEB:
			ShellOpen(STR_WEB_ADDR);
			break;

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
