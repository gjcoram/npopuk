/**************************************************************************

	nPOP

	Edit.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"


/**************************************************************************
	Define
**************************************************************************/

#define ID_MENU				(WM_APP + 102)

#define IDC_VCB				2000
#define IDC_VTB				2001
#define IDC_HEADER			2002
#define IDC_EDIT_BODY		2003

#define ID_APP_TIMER		1
#define ID_WAIT_TIMER		2

#define REPLY_SUBJECT		TEXT("Re:")


/**************************************************************************
	Global Variables
**************************************************************************/

HWND hEditWnd = NULL;
static int EditMaxLength;
static BOOL ProcessFlag;

#ifdef _WIN32_WCE_PPC
static WNDPROC EditWindowProcedure;
static HWND hToolBar;
#endif

#ifdef _WIN32_WCE_LAGENDA
static HMENU hViewMenu;
static int g_menu_height;
#endif

//外部参照
extern HINSTANCE hInst;  // Local copy of hInstance
extern TCHAR *AppDir;
extern TCHAR *DataDir;
extern HWND MainWnd;
extern HWND FocusWnd;
extern HWND hViewWnd;
extern HFONT hListFont;
extern struct TPMAILBOX *MailBox;
extern int SelBox;

extern SOCKET g_soc;
extern BOOL gSockFlag;

extern TCHAR *EditApp;
extern TCHAR *EditAppCmdLine;
extern TCHAR *EditFileSuffix;
extern int DefEditApp;
extern int AutoSave;
extern int EditWordBreakFlag;
#ifndef _WIN32_WCE
extern RECT EditRect;
#endif
extern HFONT hEditFont;

extern int SelectSendBox;
extern int AutoQuotation;
extern TCHAR *QuotationChar;
extern int WordBreakSize;
extern int QuotationBreak;
extern TCHAR *ReSubject;
extern TCHAR *ReHeader;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static int GetCcListSize(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress);
static TCHAR *SetCcList(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress, TCHAR *ret);
static void SetAllReMessage(struct TPMAILITEM *tpMailItem, struct TPMAILITEM *tpReMailItem);
static void SetReplyMessage(struct TPMAILITEM *tpMailItem, struct TPMAILITEM *tpReMailItem, int rebox, int ReplyFag);
static void SetReplyMessageBody(struct TPMAILITEM *tpMailItem, struct TPMAILITEM *tpReMailItem);
static void SetWindowString(HWND hWnd, TCHAR *Subject);
static void SetHeaderString(HWND hHeader, struct TPMAILITEM *tpMailItem);
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam);
#endif
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam);
static BOOL InitWindow(HWND hWnd, struct TPMAILITEM *tpMailItem);
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
static BOOL EndWindow(HWND hWnd);
static void SetEditMenu(HWND hWnd);
static BOOL SetItemToSendBox(HWND hWnd, BOOL BodyFlag, int EndFlag);
static BOOL CloseEditMail(HWND hWnd, BOOL SendFlag, BOOL ShowFlag);
static void ShowSendInfo(HWND hWnd);
static BOOL AppEditMail(HWND hWnd, long id, TCHAR *buf, struct TPMAILITEM *tpMailItem);
static BOOL ReadEditMail(HWND hWnd, long id, struct TPMAILITEM *tpMailItem, BOOL ReadFlag);
static LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);


/******************************************************************************

	GetCcListSize

	メールアドレスのリストの長さを取得する

******************************************************************************/

static int GetCcListSize(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress)
{
	TCHAR *p;
	int cnt = 0;

	if(To == NULL){
		return cnt;
	}

	p = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(To) + 1));
	if(p == NULL){
		return cnt;
	}
	while(*To != TEXT('\0')){
		GetMailAddress(To, p, FALSE);
		if((MyMailAddress != NULL && TStrCmpI(MyMailAddress, p) == 0) ||
			(ToMailAddress != NULL && TStrCmpI(ToMailAddress, p) == 0)){
			//自分のアドレスか To に設定されたアドレスの場合はカウントしない
			To = GetMailString(To, p);
		}else{
			To = GetMailString(To, p);
			cnt += 4;
			//メールアドレスのサイズを取得
			cnt += lstrlen(p);
		}
		To = (*To != TEXT('\0')) ? To + 1 : To;
	}
	LocalFree(p);
	return cnt;
}


/******************************************************************************

	SetCcList

	メールアドレスをCcのリストに追加する

******************************************************************************/

static TCHAR *SetCcList(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress, TCHAR *ret)
{
	TCHAR *p, *r;

	*ret = TEXT('\0');

	if(To == NULL){
		return ret;
	}

	r = ret;
	p = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(To) + 1));
	if(p == NULL){
		return ret;
	}
	while(*To != TEXT('\0')){
		GetMailAddress(To, p, FALSE);
		if((MyMailAddress != NULL && TStrCmpI(MyMailAddress, p) == 0) ||
			(ToMailAddress != NULL && TStrCmpI(ToMailAddress, p) == 0)){
			//自分のアドレスか To に設定されたアドレスの場合は追加しない
			To = GetMailString(To, p);
		}else{
			To = GetMailString(To, p);
			if(ret != r){
				//区切りの追加
				r = TStrCpy(r, TEXT(",\r\n "));
			}
			r = TStrCpy(r, p);
		}
		To = (*To != TEXT('\0')) ? To + 1 : To;
	}
	LocalFree(p);
	*r = TEXT('\0');
	return r;
}


/******************************************************************************

	SetAllReMessage

	全員に返信の設定を行う

******************************************************************************/

static void SetAllReMessage(struct TPMAILITEM *tpMailItem, struct TPMAILITEM *tpReMailItem)
{
	TCHAR *MyMailAddress = NULL;
	TCHAR *ToMailAddress = NULL;
	TCHAR *r;
	int ToSize;
	int CcSize;
	int FromSize = 0;
	int i;

	//自分のメールアドレスの取得
	i = GetNameToMailBox(tpMailItem->MailBox);
	if(i != -1){
		MyMailAddress = (MailBox + i)->MailAddress;
	}
	//送信先のメールアドレスの取得
	if(tpMailItem->To != NULL){
		ToMailAddress = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(tpMailItem->To) + 1));
		if(ToMailAddress != NULL){
			GetMailAddress(tpMailItem->To, ToMailAddress, FALSE);
		}
	}

	//サイズを取得
	ToSize = GetCcListSize(tpReMailItem->To, MyMailAddress, ToMailAddress);
	CcSize = GetCcListSize(tpReMailItem->Cc, MyMailAddress, ToMailAddress);
	if(tpReMailItem->ReplyTo != NULL){
		//ReplyTo が設定されている場合は From を Cc に追加する
		FromSize = GetCcListSize(tpReMailItem->From, MyMailAddress, ToMailAddress);
	}
	if((FromSize + ToSize + CcSize) <= 0){
		NULLCHECK_FREE(ToMailAddress);
		return;
	}
	ToSize += ((ToSize > 0 && CcSize > 0) ? 4 : 0);
	ToSize += (((ToSize > 0 || CcSize > 0) && FromSize > 0) ? 4 : 0);

	tpMailItem->Cc = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (ToSize + CcSize + FromSize + 1));
	if(tpMailItem->Cc == NULL){
		NULLCHECK_FREE(ToMailAddress);
		return;
	}
	//To を Cc のリストに追加する
	*tpMailItem->Cc = TEXT('\0');
	r = SetCcList(tpReMailItem->To, MyMailAddress, ToMailAddress, tpMailItem->Cc);
	if(CcSize > 0 && *tpMailItem->Cc != TEXT('\0')){
		r = TStrCpy(r, TEXT(",\r\n "));
	}
	//Cc を Cc のリストに追加する
	r = SetCcList(tpReMailItem->Cc, MyMailAddress, ToMailAddress, r);
	//From を Cc のリストに追加する
	if(FromSize > 0){
		if(*tpMailItem->Cc != TEXT('\0')){
			r = TStrCpy(r, TEXT(",\r\n "));
		}
		SetCcList(tpReMailItem->From, MyMailAddress, ToMailAddress, r);
	}
	NULLCHECK_FREE(ToMailAddress);
}


/******************************************************************************

	SetReplyMessage

	返信メールの設定を行う

******************************************************************************/

static void SetReplyMessage(struct TPMAILITEM *tpMailItem, struct TPMAILITEM *tpReMailItem, int rebox, int ReplyFag)
{
	TCHAR *p;
	TCHAR *subject;
	int len = 0;

	//返信のMailBoxの設定
	if(rebox >= MAILBOX_USER){
		tpMailItem->MailBox = AllocCopy((MailBox + rebox)->Name);
	}else if(tpReMailItem->MailBox != NULL){
		tpMailItem->MailBox = AllocCopy(tpReMailItem->MailBox);
	}

	//返信の宛先の設定
	if(tpReMailItem->ReplyTo != NULL){
		tpMailItem->To = AllocCopy(tpReMailItem->ReplyTo);
	}else if(tpReMailItem->From != NULL){
		tpMailItem->To = AllocCopy(tpReMailItem->From);
	}

	//全員に返信の場合は Cc を設定
	if(ReplyFag == 1){
		SetAllReMessage(tpMailItem, tpReMailItem);
	}

	if(tpReMailItem->MessageID != NULL && *tpReMailItem->MessageID == TEXT('<')){
		//返信のIn-Reply-Toの設定
		tpMailItem->InReplyTo = AllocCopy(tpReMailItem->MessageID);

		//返信のReferencesの設定
		if(tpReMailItem->InReplyTo != NULL && *tpReMailItem->InReplyTo == TEXT('<')){
			tpMailItem->References = (TCHAR *)LocalAlloc(LMEM_FIXED,
				sizeof(TCHAR) * (lstrlen(tpReMailItem->InReplyTo) + lstrlen(tpReMailItem->MessageID) + 2));
			if(tpMailItem->References != NULL){
				TStrJoin(tpMailItem->References, tpReMailItem->InReplyTo, TEXT(" "), tpReMailItem->MessageID, (TCHAR *)-1);
			}
		}else{
			tpMailItem->References = AllocCopy(tpReMailItem->MessageID);
		}
	}

	//返信の件名を設定
	subject = (tpReMailItem->Subject != NULL) ? tpReMailItem->Subject : TEXT("");
	if(TStrCmpNI(subject, ReSubject, lstrlen(ReSubject)) == 0){
		subject += lstrlen(ReSubject);
	}else if(TStrCmpNI(subject, REPLY_SUBJECT, lstrlen(REPLY_SUBJECT)) == 0){
		subject += lstrlen(REPLY_SUBJECT);
	}
	for(; *subject == TEXT(' '); subject++);
	p = tpMailItem->Subject = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(subject) + lstrlen(ReSubject) + 1));
	if(tpMailItem->Subject != NULL){
		TStrJoin(p, ReSubject, subject, (TCHAR *)-1);
	}
}

/******************************************************************************

	SetReplyMessageBody

	返信メールの本文の設定を行う

******************************************************************************/

static void SetReplyMessageBody(struct TPMAILITEM *tpMailItem, struct TPMAILITEM *tpReMailItem)
{
	struct TPMULTIPART **tpMultiPart = NULL;
	TCHAR *p, *mBody;
	int len;
	int cnt, i;

	//本文の設定
	if(tpMailItem->Status == 1 && tpReMailItem != NULL && tpReMailItem->Body != NULL){
		mBody = BodyDecode(tpReMailItem, FALSE, &tpMultiPart, &cnt);
		FreeMultipartInfo(&tpMultiPart, cnt);

		len = CreateHeaderStringSize(ReHeader, tpReMailItem) + 2;
		len += GetReplyBodySize(mBody, QuotationChar);

		i = GetNameToMailBox(tpMailItem->MailBox);
		if(i != -1 && (MailBox + i)->Signature != NULL && *(MailBox + i)->Signature != TEXT('\0')){
			len += lstrlen((MailBox + i)->Signature) + 2;
		}

		tpMailItem->Body = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
		if(tpMailItem->Body != NULL){
			p = CreateHeaderString(ReHeader, tpMailItem->Body, tpReMailItem);
			p = TStrCpy(p, TEXT("\r\n"));
			if(mBody != NULL){
				p = SetReplyBody(mBody, p, QuotationChar);
			}
			if(i != -1 && (MailBox + i)->Signature != NULL && *(MailBox + i)->Signature != TEXT('\0')){
				TStrJoin(p, TEXT("\r\n"), (MailBox + i)->Signature, (TCHAR *)-1);
			}
		}
		NULLCHECK_FREE(mBody);

	}else{
		i = GetNameToMailBox(tpMailItem->MailBox);
		if(i == -1 || (MailBox + i)->Signature == NULL || *(MailBox + i)->Signature == TEXT('\0')){
			return;
		}
		len = lstrlen((MailBox + i)->Signature);
		tpMailItem->Body = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 3));
		if(tpMailItem->Body != NULL){
			TStrJoin(tpMailItem->Body, TEXT("\r\n"), (MailBox + i)->Signature, (TCHAR *)-1);
		}
	}
}

/******************************************************************************

	SetWindowString

	ウィンドウタイトルの設定

******************************************************************************/

static void SetWindowString(HWND hWnd, TCHAR *Subject)
{
	TCHAR *buf;

	if(Subject == NULL){
		SetWindowText(hWnd, STR_TITLE_MAILEDIT);
		return;
	}
	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) *
		(lstrlen(STR_TITLE_MAILEDIT) + lstrlen(Subject) + lstrlen(TEXT(" - []")) + 1));
	if(buf == NULL){
		SetWindowText(hWnd, STR_TITLE_MAILEDIT);
		return;
	}
	TStrJoin(buf, STR_TITLE_MAILEDIT TEXT(" - ["), Subject, TEXT("]"), (TCHAR *)-1);
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
	TCHAR *buf, *p;
	int len = 0;

	len += lstrlen(STR_EDIT_HEAD_MAILBOX);
	if(tpMailItem->MailBox != NULL){
		len += lstrlen(tpMailItem->MailBox);
	}
	len += lstrlen(STR_EDIT_HEAD_TO);
	if(tpMailItem->To != NULL){
		len += lstrlen(tpMailItem->To);
	}
	len += SetCcAddressSize(tpMailItem->Cc);
	len += SetCcAddressSize(tpMailItem->Bcc);
	len += lstrlen(STR_EDIT_HEAD_SUBJECT);
	if(tpMailItem->Subject != NULL){
		len += lstrlen(tpMailItem->Subject);
	}
	len += SetAttachListSize(tpMailItem->Attach);

	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(buf == NULL){
		return;
	}

	p = TStrJoin(buf, STR_EDIT_HEAD_MAILBOX, tpMailItem->MailBox, STR_EDIT_HEAD_TO, tpMailItem->To, (TCHAR *)-1);
	p = SetCcAddress(TEXT("Cc"), tpMailItem->Cc, p);
	p = SetCcAddress(TEXT("Bcc"), tpMailItem->Bcc, p);
	p = TStrJoin(p, STR_EDIT_HEAD_SUBJECT, tpMailItem->Subject, (TCHAR *)-1);
	SetAttachList(tpMailItem->Attach, p);

	SetWindowText(hHeader, buf);
	LocalFree(buf);
}


/******************************************************************************

	SubClassEditProc

	サブクラス化したウィンドウプロシージャ

******************************************************************************/

#ifdef _WIN32_WCE_PPC
static LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_LBUTTONDOWN:
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
		break;
	}

	return CallWindowProc(EditWindowProcedure, hWnd, msg, wParam, lParam);
}
#endif


/******************************************************************************

	SetEditSubClass

	ウィンドウのサブクラス化

******************************************************************************/

#ifdef _WIN32_WCE_PPC
static void SetEditSubClass(HWND hWnd)
{
	EditWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (long)SubClassEditProc);
}
#endif


/******************************************************************************

	DelEditSubClass

	ウィンドウクラスを標準のものに戻す

******************************************************************************/

#ifdef _WIN32_WCE_PPC
static void DelEditSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (long)EditWindowProcedure);
	EditWindowProcedure = NULL;
}
#endif


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

static LRESULT NotifyProc(HWND hWnd, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;

#ifndef _WIN32_WCE
	if(CForm->code == TTN_NEEDTEXT){
		return TbNotifyProc(hWnd, lParam);
	}
#endif
	return FALSE;
}


/******************************************************************************

	InitWindow

	ウィンドウの初期化

******************************************************************************/

static BOOL InitWindow(HWND hWnd, struct TPMAILITEM *tpMailItem)
{
	HDC hdc;
	HFONT hFont;
#ifndef _WIN32_WCE_PPC
#ifndef _WIN32_WCE_LAGENDA
	HWND hToolBar;
#endif	//_WIN32_WCE_LAGENDA
#endif	//_WIN32_WCE_PPC
	TEXTMETRIC lptm;
	RECT rcClient, StRect;
	TCHAR *buf;
	int Height;
	int FontHeight;

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
		ID_MENUITEM_SEND,		CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_SEND,		NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_SENDBOX,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_SENDBOX,	NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_SENDINFO,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_SENDINFO,	NULL, NULL,			NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
	};
	HWND hCSOBar;
	DWORD style;
#else	//_WIN32_WCE_LAGENDA
	TBBUTTON tbButton[] = {
#ifdef _WIN32_WCE
#ifndef _WIN32_WCE_PPC
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
#endif	//_WIN32_WCE_PPC
#endif	//_WIN32_WCE
		{0,	ID_MENUITEM_SEND,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{1,	ID_MENUITEM_SENDBOX,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{2,	ID_MENUITEM_SENDINFO,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
	};
#ifdef _WIN32_WCE
	static TCHAR *szTips[] = {
#ifdef _WIN32_WCE_PPC
		NULL, //menu skipping
#endif	//_WIN32_WCE_PPC
		NULL, //menu skipping
		STR_CMDBAR_SEND,
		STR_CMDBAR_SENDBOX,
		STR_CMDBAR_SENDINFO
	};
#ifdef _WIN32_WCE_PPC
	SHMENUBARINFO mbi;
#endif	//_WIN32_WCE_PPC
#else	//_WIN32_WCE
	RECT ToolbarRect;
#endif	//_WIN32_WCE
#endif	//_WIN32_WCE_LAGENDA

	if(tpMailItem == NULL){
		return FALSE;
	}
	SetWindowString(hWnd, tpMailItem->Subject);

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	//PocketPC
	memset (&mbi, 0, sizeof (SHMENUBARINFO));
	mbi.cbSize     = sizeof (SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_EDIT;
	mbi.hInstRes   = hInst;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;
	SHCreateMenuBar(&mbi);

	hToolBar = mbi.hwndMB;
    CommandBar_AddToolTips(hToolBar, 5, szTips);
	CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR_EDIT, 3, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	Height = 0;
#elif defined(_WIN32_WCE_LAGENDA)
	//BE-500
	hCSOBar = CSOBar_Create(hInst, hWnd, 1, BaseInfo);
	CSOBar_AddAdornments(hCSOBar, hInst, 1, CSOBAR_ADORNMENT_CLOSE, 0);

	hViewMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU_EDIT));
	ButtonInfo[1].SubMenu = GetSubMenu(hViewMenu, 0);
	ButtonInfo[2].SubMenu = GetSubMenu(hViewMenu, 1);
	ButtonInfo[3].reshInst = hInst;
	ButtonInfo[4].reshInst = hInst;
	ButtonInfo[5].reshInst = hInst;
	CSOBar_AddButtons(hCSOBar, hInst, sizeof(ButtonInfo) / sizeof(CSOBAR_BUTTONINFO), ButtonInfo);

	style = GetWindowLong(hCSOBar, GWL_STYLE);
	style &= ~WS_CLIPCHILDREN;
	SetWindowLong(hCSOBar, GWL_STYLE, style);

	Height = g_menu_height = CSOBar_Height(hCSOBar);
#else
	//H/PC & PsPC
	hToolBar = CommandBar_Create(hInst, hWnd, IDC_VCB);
    CommandBar_AddToolTips(hToolBar, 4, szTips);
	if(GetSystemMetrics(SM_CXSCREEN) >= 450){
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_EDIT_HPC, 0);
	}else{
		CommandBar_InsertMenubar(hToolBar, hInst, IDR_MENU_EDIT, 0);
	}
	CommandBar_AddBitmap(hToolBar, hInst, IDB_TOOLBAR_EDIT, 3, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	CommandBar_AddAdornments(hToolBar, 0, 0);

	Height = CommandBar_Height(hToolBar);
#endif
#else
	//Win32
	hToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_VTB, 3, hInst, IDB_TOOLBAR_EDIT,
		tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, TB_ICONSIZE, TB_ICONSIZE, sizeof(TBBUTTON));
	SetWindowLong(hToolBar, GWL_STYLE, GetWindowLong(hToolBar, GWL_STYLE) | TBSTYLE_FLAT);
	SendMessage(hToolBar, TB_SETINDENT, 5, 0);
	ShowWindow(hToolBar,SW_SHOW);

	GetWindowRect(hToolBar, &ToolbarRect);
	Height = ToolbarRect.bottom - ToolbarRect.top;
#endif
	GetClientRect(hWnd, &rcClient);

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
		0, Height, rcClient.right, 0,
		hWnd, (HMENU)IDC_HEADER, hInst, NULL);

	//フォントの設定
#ifdef _WIN32_WCE
	if(hListFont != NULL){
		SendDlgItemMessage(hWnd, IDC_HEADER, WM_SETFONT, (WPARAM)hListFont, MAKELPARAM(TRUE,0));
	}
#else
	SendDlgItemMessage(hWnd, IDC_HEADER, WM_SETFONT,
		(WPARAM)((hListFont != NULL) ? hListFont : GetStockObject(DEFAULT_GUI_FONT)), MAKELPARAM(TRUE,0));
#endif

	hdc = GetDC(GetDlgItem(hWnd, IDC_HEADER));
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
	ReleaseDC(GetDlgItem(hWnd, IDC_HEADER), hdc);
	FontHeight = (lptm.tmHeight + lptm.tmExternalLeading) * 3;

	//一時的に設定してサイズを再計算する
	MoveWindow(GetDlgItem(hWnd, IDC_HEADER), 0, Height, rcClient.right, FontHeight, TRUE);
	GetClientRect(GetDlgItem(hWnd, IDC_HEADER), &StRect);
	FontHeight = FontHeight + (FontHeight - StRect.bottom) + 1;
	MoveWindow(GetDlgItem(hWnd, IDC_HEADER), 0, Height, rcClient.right, FontHeight, TRUE);

	//本文を表示するEDITコントロールの作成
	Height += FontHeight + 1;
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
		WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ((EditWordBreakFlag == 1) ? 0 : WS_HSCROLL),
		0, Height, rcClient.right, rcClient.bottom - Height,
		hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hEditFont, MAKELPARAM(TRUE,0));

	SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));

#ifdef _WIN32_WCE_PPC
	SetWordBreakMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), (EditWordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
	SetWordBreakMenu(hWnd, hViewMenu, (EditWordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
	SetWordBreakMenu(hWnd, NULL, (EditWordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

#ifdef _WIN32_WCE
	SendMessage(hWnd, WM_SETICON, (WPARAM)FALSE,
		(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_SENDMAIL), IMAGE_ICON, 16, 16, 0));
	EditMaxLength = EDITMAXSIZE;
#else
	{
		OSVERSIONINFO os_info;

		os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&os_info);

		EditMaxLength = (os_info.dwPlatformId == VER_PLATFORM_WIN32_NT) ? 0 : EDITMAXSIZE;
	}
#endif
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_LIMITTEXT, (WPARAM)EditMaxLength, 0);

	SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
	if(tpMailItem->Body != NULL){
		SwitchCursor(FALSE);
		buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(tpMailItem->Body) + 1));
		if(buf != NULL){
			DelDot(tpMailItem->Body, buf);
			if(EditMaxLength != 0 && (int)lstrlen(buf) > EditMaxLength){
				*(buf + EditMaxLength) = TEXT('\0');
			}
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)buf);
			LocalFree(buf);
		}
		SwitchCursor(TRUE);
	}
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETMODIFY, (WPARAM)FALSE, 0);

	tpMailItem->hEditWnd = hWnd;
#ifdef _WIN32_WCE_PPC
	SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#endif
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
	int hHeight;

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
	int hHeight, tHeight;

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

static BOOL EndWindow(HWND hWnd)
{
	struct TPMAILITEM *tpMailItem;

	tpMailItem = (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if(tpMailItem != NULL){
		if(SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETMODIFY, 0, 0) == TRUE &&
			MessageBox(hWnd, STR_Q_EDITCANSEL,
				STR_TITLE_MAILEDIT, MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDNO){
			return FALSE;
		}
		tpMailItem->hEditWnd = NULL;
		if(tpMailItem->hProcess != NULL){
			ReadEditMail(hWnd, (long)hWnd, tpMailItem, FALSE);
			tpMailItem->hProcess = NULL;
		}
		if(Item_IsMailBox(MailBox + MAILBOX_SEND, tpMailItem) == FALSE){
			FreeMailItem(&tpMailItem, 1);
		}
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	DelEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
    DestroyWindow(hToolBar);
#elif defined _WIN32_WCE_LAGENDA
	DestroyMenu(hViewMenu);
#else
	CommandBar_Destroy(GetDlgItem(hWnd, IDC_VCB));
#endif
	DestroyIcon((HICON)SendMessage(hWnd, WM_GETICON, FALSE, 0));
#else
	if(IsWindowVisible(hWnd) != 0 && IsIconic(hWnd) == 0 && IsZoomed(hWnd) == 0){
		GetWindowRect(hWnd, (LPRECT)&EditRect);
		EditRect.right -= EditRect.left;
		EditRect.bottom -= EditRect.top;
	}
	DestroyWindow(GetDlgItem(hWnd, IDC_VTB));
#endif
	DestroyWindow(GetDlgItem(hWnd, IDC_HEADER));
	DestroyWindow(GetDlgItem(hWnd, IDC_EDIT_BODY));

	DestroyWindow(hWnd);
	hEditWnd = NULL;
#ifdef _WIN32_WCE
	ProcessFlag = FALSE;
#endif
	return TRUE;
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
	EnableMenuItem(hMenu, ID_MENUITEM_CUT, (i < j) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_MENUITEM_COPY, (i < j) ? MF_ENABLED : MF_GRAYED);
}


/******************************************************************************

	SetItemToSendBox

	送信箱に保存

******************************************************************************/

static BOOL SetItemToSendBox(HWND hWnd, BOOL BodyFlag, int EndFlag)
{
	struct TPMAILITEM *tpMailItem;
	TCHAR *buf, *tmp;
	TCHAR numbuf[10];
	int len;
	int i;

	tpMailItem = (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if(tpMailItem == NULL){
		return FALSE;
	}

	if(BodyFlag == FALSE){
		//機種依存文字のチェック
		if(EndFlag == 0 && CheckDependence(hWnd, IDC_EDIT_BODY) == FALSE){
			return FALSE;
		}

		//件名が設定されていない場合は送信情報を表示する
		if(EndFlag == 0 && (tpMailItem->Subject == NULL || *tpMailItem->Subject == TEXT('\0'))){
			if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
				(LPARAM)tpMailItem) == FALSE){
				return FALSE;
			}
		}

		//本文を設定
		SwitchCursor(FALSE);
		NULLCHECK_FREE(tpMailItem->Body);
		len = SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXTLENGTH, 0, 0) + 1;
		buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
		if(buf != NULL){
			*buf = TEXT('\0');
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXT, len, (LPARAM)buf);

			//自動折り返し
			tmp = (TCHAR *)LocalAlloc(LMEM_FIXED,
				sizeof(TCHAR) * (WordBreakStringSize(buf, QuotationChar, WordBreakSize, QuotationBreak) + 1));
			if(tmp != NULL){
				WordBreakString(buf, tmp, QuotationChar, WordBreakSize, QuotationBreak);
				LocalFree(buf);
				buf = tmp;
			}

			//行頭の . を .. に変換
			len = SetDotSize(buf);
			tpMailItem->Body = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
			if(tpMailItem->Body != NULL){
				SetDot(buf, tpMailItem->Body);
			}
			LocalFree(buf);
		}
		SwitchCursor(TRUE);
	}

	//サイズを設定
	NULLCHECK_FREE(tpMailItem->Size);
	wsprintf(numbuf, TEXT("%d"), (tpMailItem->Body != NULL)
		? TcharToCharSize(tpMailItem->Body) - 1 : 0);
	tpMailItem->Size = AllocCopy(numbuf);

	if(Item_IsMailBox(MailBox + MAILBOX_SEND, tpMailItem) == FALSE){
		if(Item_Add(MailBox + MAILBOX_SEND, tpMailItem) == FALSE){
			return FALSE;
		}
		if(EndFlag == 0 && SelBox == MAILBOX_SEND){
			ListView_InsertItemEx(GetDlgItem(MainWnd, IDC_LISTVIEW),
				(TCHAR *)LPSTR_TEXTCALLBACK, 0, I_IMAGECALLBACK, (long)tpMailItem,
				ListView_GetItemCount(GetDlgItem(MainWnd, IDC_LISTVIEW)));
			SetItemCntStatusText(MainWnd, NULL);
		}
	}
	if(EndFlag == 0){
		if(SelectSendBox == 1 && SelBox != MAILBOX_SEND){
			SelectMailBox(MainWnd, MAILBOX_SEND);
		}
		if(SelBox == MAILBOX_SEND){
			i = ListView_GetMemToItem(GetDlgItem(MainWnd, IDC_LISTVIEW), tpMailItem);
			if(i != -1){

				ListView_SetItemState(GetDlgItem(MainWnd, IDC_LISTVIEW), i,
					((tpMailItem->Attach != NULL && *tpMailItem->Attach != TEXT('\0')) ? INDEXTOSTATEIMAGEMASK(1) : 0),
					LVIS_STATEIMAGEMASK)

				ListView_SetItemState(GetDlgItem(MainWnd, IDC_LISTVIEW), -1, 0, LVIS_SELECTED);
				ListView_SetItemState(GetDlgItem(MainWnd, IDC_LISTVIEW),
					i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

				ListView_EnsureVisible(GetDlgItem(MainWnd, IDC_LISTVIEW), i, TRUE);
				ListView_RedrawItems(GetDlgItem(MainWnd, IDC_LISTVIEW), i, i);
				UpdateWindow(GetDlgItem(MainWnd, IDC_LISTVIEW));
#ifdef _WIN32_WCE
				FocusWnd = MainWnd;
				ShowWindow(MainWnd, SW_SHOW);
				_SetForegroundWindow(MainWnd);
#endif
			}
		}
	}
	return TRUE;
}


/******************************************************************************

	CloseEditMail

	編集メールを閉じる

******************************************************************************/

static BOOL CloseEditMail(HWND hWnd, BOOL SendFlag, BOOL ShowFlag)
{
	struct TPMAILITEM *tpMailItem;

	tpMailItem = (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if(tpMailItem == NULL){
		return FALSE;
	}

	tpMailItem->hEditWnd = NULL;
	tpMailItem->hProcess = NULL;

	if(AutoSave == 1){
		//送信箱をファイルに保存
		SaveMail(SENDBOX_FILE, MailBox + MAILBOX_SEND, 2);
	}

	SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)0);
	EndWindow(hWnd);
#ifdef _WIN32_WCE
	FocusWnd = MainWnd;
#ifdef _WIN32_WCE_LAGENDA
	ShowWindow(MainWnd, SW_SHOW);
	if(ShowFlag == TRUE){
		_SetForegroundWindow(MainWnd);
	}
#else
	if(ShowFlag == TRUE){
		ShowWindow(MainWnd, SW_SHOW);
		_SetForegroundWindow(MainWnd);
	}
#endif
#endif

	if(SendFlag == TRUE){
		SendMessage(MainWnd, WM_SMTP_SENDMAIL, 0, (LPARAM)tpMailItem);
	}
	return TRUE;
}


/******************************************************************************

	ShowSendInfo

	送信情報表示

******************************************************************************/

static void ShowSendInfo(HWND hWnd)
{
	struct TPMAILITEM *tpMailItem;

	tpMailItem = (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if(tpMailItem == NULL){
		return;
	}
	if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
		(LPARAM)tpMailItem) == FALSE){
		return;
	}
	SetWindowString(hWnd, tpMailItem->Subject);
	SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
}


/******************************************************************************

	AppEditMail

	外部エディタで編集

******************************************************************************/

static BOOL AppEditMail(HWND hWnd, long id, TCHAR *buf, struct TPMAILITEM *tpMailItem)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR *p;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

#ifdef _WIN32_WCE
	TStrJoin(path, DataDir, EDIT_FILE, TEXT("."), EditFileSuffix, (TCHAR *)-1);
#else
	wsprintf(path, TEXT("%s%ld.%s"), DataDir, id, EditFileSuffix);
#endif

	//保存するファイルを開く
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == NULL || hFile == (HANDLE)-1){
		return FALSE;
	}
	//メールの保存
	if(buf != NULL && WriteAsciiFile(hFile, buf, lstrlen(buf)) == FALSE){
		CloseHandle(hFile);
		DeleteFile(path);
		return FALSE;
	}
	CloseHandle(hFile);

	{
#ifdef _WCE_OLD
		PROCESS_INFORMATION ProcInfo;
		TCHAR param[BUF_SIZE];

		TStrJoin(param, TEXT(" "), path, (TCHAR *)-1);
		p = CreateCommandLine(EditAppCmdLine, path, TRUE);

		//起動
		if(CreateProcess(EditApp, ((p != NULL) ? p : param),
			NULL, NULL, FALSE, 0, NULL, NULL, NULL, &ProcInfo) == FALSE){
			NULLCHECK_FREE(p);
			DeleteFile(path);
			return FALSE;
		}
		NULLCHECK_FREE(p);
		tpMailItem->hProcess = ProcInfo.hProcess;
#else
		SHELLEXECUTEINFO sei;

		tZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
		sei.hwnd = NULL;
		sei.lpVerb = NULL;
		p = NULL;
		if(EditApp == NULL || *EditApp == TEXT('\0')){
			sei.lpFile = path;
			sei.lpParameters = NULL;
		}else{
			sei.lpFile = EditApp;
			p = CreateCommandLine(EditAppCmdLine, path, FALSE);
			sei.lpParameters = (p != NULL) ? p : path;
		}
		sei.lpDirectory = NULL;
		sei.nShow = SW_SHOWNORMAL;
		sei.hInstApp = hInst;
		//起動
		if(ShellExecuteEx(&sei) == FALSE){
			NULLCHECK_FREE(p);
			DeleteFile(path);
			return FALSE;
		}
		NULLCHECK_FREE(p);
		tpMailItem->hProcess = sei.hProcess;
#endif
	}
	if(tpMailItem->hProcess == NULL){
		DeleteFile(path);
		return FALSE;
	}
	return TRUE;
}


/******************************************************************************

	ReadEditMail

	ファイルをメール本文として読み込む

******************************************************************************/

static BOOL ReadEditMail(HWND hWnd, long id, struct TPMAILITEM *tpMailItem, BOOL ReadFlag)
{
	TCHAR path[BUF_SIZE];
	TCHAR *tmp, *p;
	char *fbuf;
	int len;

#ifndef _WIN32_WCE
	SetCurrentDirectory(AppDir);
#endif

#ifdef _WIN32_WCE
	TStrJoin(path, DataDir, EDIT_FILE, TEXT("."), EditFileSuffix, (TCHAR *)-1);
#else
	wsprintf(path, TEXT("%s%ld.%s"), DataDir, id, EditFileSuffix);
#endif

	if(ReadFlag == FALSE){
		DeleteFile(path);
		return TRUE;
	}

	len = GetFileSerchSize(path);
	fbuf = ReadFileBuf(path, len);
#ifdef UNICODE
	//UNICODEに変換
	tmp = AllocCharToTchar(fbuf);
	LocalFree(fbuf);
	p = tmp;
#else
	p = fbuf;
#endif
	if(p != NULL){
		NULLCHECK_FREE(tpMailItem->Body);

		//自動折り返し
		tmp = (TCHAR *)LocalAlloc(LMEM_FIXED,
			sizeof(TCHAR) * (WordBreakStringSize(p, QuotationChar, WordBreakSize, QuotationBreak) + 1));
		if(tmp != NULL){
			WordBreakString(p, tmp, QuotationChar, WordBreakSize, QuotationBreak);
			LocalFree(p);
			p = tmp;
		}

		//行頭の . を .. に変換
		len = SetDotSize(p);
		tpMailItem->Body = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
		if(tpMailItem->Body != NULL){
			SetDot(p, tpMailItem->Body);
		}
		LocalFree(p);
	}

	DeleteFile(path);
	return TRUE;
}


/******************************************************************************

	EditProc

	メール表示プロシージャ

******************************************************************************/

static LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam)
{
	struct TPMAILITEM *tpMailItem;
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	static BOOL SipFlag = TRUE;
#endif

	switch(msg)
	{
	case WM_CREATE:
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		SipFlag = TRUE;
#endif
		if(InitWindow(hWnd, (struct TPMAILITEM *)(((CREATESTRUCT *)lParam)->lpCreateParams)) == FALSE){
			DestroyWindow(hWnd);
			break;
		}
		SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)((CREATESTRUCT *)lParam)->lpCreateParams);
		break;

#ifdef _WIN32_WCE_PPC
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd){
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
#endif

	case WM_SETFOCUS:
		SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));
		FocusWnd = hWnd;
		break;

#ifdef _WIN32_WCE
	case WM_HIBERNATE:
		wParam = 1;
#endif
	case WM_ENDCLOSE:
		{
			BOOL BodyFlag = FALSE;

			tpMailItem = (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if(tpMailItem != NULL && tpMailItem->hProcess != NULL){
				ReadEditMail(hWnd, (long)hWnd, tpMailItem, TRUE);
				BodyFlag = TRUE;
				tpMailItem->hProcess = NULL;
#ifdef _WIN32_WCE
				ProcessFlag = FALSE;
#endif
			}
			if(SetItemToSendBox(hWnd, BodyFlag, wParam) == TRUE){
				CloseEditMail(hWnd, FALSE, FALSE);
#ifdef _WIN32_WCE_LAGENDA
				SipShowIM(SIPF_OFF);
#endif
			}
		}
		break;

	case WM_CLOSE:
		if(EndWindow(hWnd) == FALSE){
			break;
		}
#ifdef _WIN32_WCE
		FocusWnd = MainWnd;
		ShowWindow(MainWnd, SW_SHOW);
		_SetForegroundWindow(MainWnd);
#endif
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
		case ID_APP_TIMER:
			KillTimer(hWnd, wParam);
			tpMailItem = (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if(tpMailItem == NULL){
				break;
			}

#ifdef _WIN32_WCE
			if(hViewWnd != NULL){
				SendMessage(hViewWnd, WM_CLOSE, 0, 0);
			}
			ProcessFlag = TRUE;
#endif
			//外部エディタ起動
			if(AppEditMail(hWnd, (long)hWnd, tpMailItem->Body, tpMailItem) == FALSE){
#ifdef _WIN32_WCE
				ProcessFlag = FALSE;
#endif
				ShowWindow(hWnd, SW_SHOW);
				UpdateWindow(hWnd);
				break;
			}
			SetTimer(hWnd, ID_WAIT_TIMER, 1, NULL);
			break;

		case ID_WAIT_TIMER:
			tpMailItem = (struct TPMAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if(tpMailItem == NULL || tpMailItem->hProcess == NULL){
				KillTimer(hWnd, wParam);
				break;
			}
			//外部エディタの終了監視
			if(WaitForSingleObject(tpMailItem->hProcess, 0) == WAIT_TIMEOUT){
				break;
			}
			KillTimer(hWnd, wParam);
			tpMailItem->hProcess = NULL;

			//ファイルの読み直し
			ReadEditMail(hWnd, (long)hWnd, tpMailItem, TRUE);

#ifdef _WIN32_WCE
			ProcessFlag = FALSE;
#endif
			//送信箱に保存
			if(SetItemToSendBox(hWnd, TRUE, 0) == TRUE){
				CloseEditMail(hWnd, FALSE, TRUE);
			}else{
				ShowWindow(hWnd, SW_SHOW);
				UpdateWindow(hWnd);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		return NotifyProc(hWnd, lParam);

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam,lParam))
		{
#ifdef _WIN32_WCE_PPC
		case ID_MENU:
			SetEditMenu(hWnd);
			ShowMenu(hWnd, SHGetSubMenu(hToolBar, ID_MENUITEM_EDIT), 0, 0, FALSE);
			break;

		case IDC_EDIT_BODY:
			switch(HIWORD(wParam)){
			case EN_SETFOCUS:
				SHSipPreference(hWnd, (SipFlag) ? SIP_UP : SIP_DOWN);
				break;
			case EN_KILLFOCUS:
				SHSipPreference(hWnd, SIP_DOWN);
				break;
			}
			break;

		case IDOK:
			SendMessage(hWnd, WM_ENDCLOSE, 0, 0);
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

		case ID_MENUITEM_SEND:
			if(MessageBox(hWnd, STR_Q_SENDMAIL,
				STR_TITLE_SEND, MB_ICONQUESTION | MB_YESNO) == IDNO){
				break;
			}
			if(g_soc != -1 || gSockFlag == TRUE){
				ErrorMessage(NULL, STR_ERR_SENDLOCK);
				break;
			}
			if(SetItemToSendBox(hWnd, FALSE, 0) == TRUE){
				CloseEditMail(hWnd, TRUE, TRUE);
			}
			break;

		case ID_MENUITEM_SENDBOX:
			if(SetItemToSendBox(hWnd, FALSE, 0) == TRUE){
				CloseEditMail(hWnd, FALSE, TRUE);
			}
			break;

		case ID_MENUITEM_SENDINFO:
			ShowSendInfo(hWnd);
			break;

		case ID_MENUITEM_CLOSE:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

#ifdef _WIN32_WCE_LAGENDA
		case CSOBAR_ADORNMENTID_CLOSE:
			SendMessage(MainWnd, WM_CLOSE, 0, 0);
			break;
#endif

		case ID_MENUITEM_UNDO:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_UNDO, 0, 0);
			break;

		case ID_MENUITEM_CUT:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_CUT , 0, 0);
			break;

		case ID_MENUITEM_COPY:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_COPY , 0, 0);
			break;

		case ID_MENUITEM_PASTE:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_PASTE , 0, 0);
			break;

		case ID_MENUITEM_ALLSELECT:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, 0, -1);
			break;

		case ID_MENUITEM_FILEOPEN:
			{
				TCHAR *buf;

				if(OpenFileBuf(hWnd, &buf) == FALSE){
					ErrorMessage(hWnd, STR_ERR_OPEN);
					break;
				}
				if(buf == NULL){
					break;
				}
				SwitchCursor(FALSE);
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)buf);
				SwitchCursor(TRUE);

				LocalFree(buf);
			}
			break;

		case ID_MENUITEM_WORDBREAK:
#ifdef _WIN32_WCE_LAGENDA
			EditWordBreakFlag = SetWordBreak(hWnd, hViewMenu);
#else
			EditWordBreakFlag = SetWordBreak(hWnd);
#endif
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_LIMITTEXT, (WPARAM)EditMaxLength, 0);
			break;
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}


/******************************************************************************

	Edit_InitApplication

	ウィンドウクラスの登録

******************************************************************************/

BOOL Edit_InitApplication(HINSTANCE hInstance)
{
	WNDCLASS wc;

#ifdef _WIN32_WCE
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = NULL;
	wc.lpszMenuName = NULL;
#else
	wc.style = 0;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_EDIT);
#endif
	wc.lpfnWndProc = (WNDPROC)EditProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SENDMAIL));
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
	wc.lpszClassName = EDIT_WND_CLASS;

	return RegisterClass(&wc);
}


/******************************************************************************

	Edit_MailToSet

	メールアドレスから送信メールの作成

******************************************************************************/

int Edit_MailToSet(HINSTANCE hInstance, HWND hWnd, TCHAR *mail_addr, int rebox)
{
	static BOOL ExistFlag = FALSE;
	struct TPMAILITEM *tpMailItem;
	BOOL ret;

	if(ExistFlag == TRUE){
		return EDIT_NONEDIT;
	}

	tpMailItem = (struct TPMAILITEM *)LocalAlloc(LPTR, sizeof(struct TPMAILITEM));
	if(tpMailItem == NULL){
		ErrorMessage(hWnd, STR_ERR_MEMALLOC);
		return FALSE;
	}

	//URL(mailto:)をメールアイテムに設定
	if(URLToMailItem(mail_addr, tpMailItem) == FALSE){
		FreeMailItem(&tpMailItem, 1);
		return FALSE;
	}
	ExistFlag = TRUE;
	ret = Edit_InitInstance(hInstance, hWnd, rebox, tpMailItem, EDIT_NEW, 0);
	FreeMailItem(&tpMailItem, 1);
	ExistFlag = FALSE;
	return ret;
}


/******************************************************************************

	Edit_InitInstance

	ウィンドウの作成

******************************************************************************/

int Edit_InitInstance(HINSTANCE hInstance, HWND hWnd, int rebox, struct TPMAILITEM *tpReMailItem, int OpenFlag, int ReplyFag)
{
	struct TPMAILITEM *tpMailItem;
	int key;
#ifdef _WIN32_WCE_PPC
	SIPINFO si;
	int x = CW_USEDEFAULT, y = CW_USEDEFAULT, cx, cy;
#endif

#ifdef _WIN32_WCE
	//CEの場合は編集画面を複数開かないようにする
	if(hEditWnd != NULL){
		if(ProcessFlag == TRUE){
			return EDIT_NONEDIT;
		}
		ShowWindow(hEditWnd, SW_SHOW);
		_SetForegroundWindow(hEditWnd);
		SendMessage(hEditWnd, WM_CLOSE, 0, 0);
		if(hEditWnd != NULL){
			return EDIT_INSIDEEDIT;
		}
	}
#endif
	key = GetKeyState(VK_SHIFT);

	//編集する種類によって初期化
	switch(OpenFlag)
	{
	case EDIT_OPEN:
		//既存の送信メールを開く
		if(tpReMailItem == NULL){
			return EDIT_NONEDIT;
		}
#ifndef _WIN32_WCE
		if(tpReMailItem->hEditWnd != NULL){
			if(IsWindowVisible(tpReMailItem->hEditWnd) == TRUE){
				ShowWindow(tpReMailItem->hEditWnd, SW_SHOW);
				if(IsIconic(tpReMailItem->hEditWnd) != 0){
					ShowWindow(tpReMailItem->hEditWnd, SW_RESTORE);
				}
				_SetForegroundWindow(tpReMailItem->hEditWnd);
			}
			return EDIT_INSIDEEDIT;
		}
#endif
		tpMailItem = tpReMailItem;
		break;

	case EDIT_NEW:
		//新規にメールを作成
		tpMailItem = (struct TPMAILITEM *)LocalAlloc(LPTR, sizeof(struct TPMAILITEM));
		if(tpMailItem == NULL){
			ErrorMessage(hWnd, STR_ERR_MEMALLOC);
			return EDIT_NONEDIT;
		}
		if(tpReMailItem != NULL){
			CopyItem(tpReMailItem, tpMailItem);
		}
		tpMailItem->Download = TRUE;
		//送信情報設定
		_SetForegroundWindow(hWnd);
		if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc, (LPARAM)tpMailItem) == FALSE){
			return EDIT_NONEDIT;
		}
		if(tpMailItem->Body == NULL){
			SetReplyMessageBody(tpMailItem, NULL);
		}
		break;

	case EDIT_REPLY:
		//返信
		tpMailItem = (struct TPMAILITEM *)LocalAlloc(LPTR, sizeof(struct TPMAILITEM));
		if(tpMailItem == NULL){
			ErrorMessage(hWnd, STR_ERR_MEMALLOC);
			return EDIT_NONEDIT;
		}
		tpMailItem->Download = TRUE;

		if(tpReMailItem != NULL){
			//返信設定
			SetReplyMessage(tpMailItem, tpReMailItem, rebox, ReplyFag);
			if(tpReMailItem->Body != NULL){
				tpMailItem->Status = 1;
			}
		}
		//送信情報設定
		if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc, (LPARAM)tpMailItem) == FALSE){
			FreeMailItem(&tpMailItem, 1);
			return EDIT_NONEDIT;
		}
		SetReplyMessageBody(tpMailItem, tpReMailItem);
		tpMailItem->Status = 0;
		break;
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	memset(&si, 0, sizeof(si));
	si.cbSize = sizeof(si);
	SHSipInfo(SPI_GETSIPINFO, 0, &si, 0);
	cx = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
	cy = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top - ((si.fdwFlags & SIPF_ON) ? 0 : MENU_HEIGHT);

	hEditWnd = CreateWindowEx(WS_EX_CAPTIONOKBTN,
		EDIT_WND_CLASS,
		STR_TITLE_MAILEDIT,
		WS_VISIBLE,
		x, y, cx, cy,
		NULL, NULL, hInstance, (LPVOID)tpMailItem);

	SHFullScreen(hEditWnd, SHFS_SHOWSIPBUTTON);
	SHSipPreference(hEditWnd, SIP_UP);
#else
	hEditWnd = CreateWindow(EDIT_WND_CLASS,
		STR_TITLE_MAILEDIT,
		WS_VISIBLE,
		0, 0,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL, NULL, hInstance, (LPVOID)tpMailItem);
#endif
#else
	hEditWnd = CreateWindow(EDIT_WND_CLASS,
		STR_TITLE_MAILEDIT,
		WS_OVERLAPPEDWINDOW,
		EditRect.left,
		EditRect.top,
		EditRect.right,
		EditRect.bottom,
		NULL, NULL, hInstance, (LPVOID)tpMailItem);
#endif

	if(!hEditWnd){
		if(OpenFlag != EDIT_OPEN){
			FreeMailItem(&tpMailItem, 1);
		}
		ErrorMessage(hWnd, STR_ERR_INIT);
		return EDIT_NONEDIT;
	}

	if((DefEditApp == 1 && key >= 0) || (DefEditApp == 0 && key < 0)){
		ShowWindow(hEditWnd, SW_HIDE);
		SetTimer(hEditWnd, ID_APP_TIMER, 1, NULL);
		return EDIT_OUTSIDEEDIT;
	}else{
		ShowWindow(hEditWnd, SW_SHOW);
		UpdateWindow(hEditWnd);
	}
	return EDIT_INSIDEEDIT;
}
/* End of source */
