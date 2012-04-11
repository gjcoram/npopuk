/*
 * nPOP
 *
 * Ini.c
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
#include "Profile.h"
#include "charset.h"

/* Define */
#define INI_BUF_SIZE		1024

/* Global Variables */
OPTION op;
BOOL ConvertFromNPOP = FALSE;
int CP_int = CP_ACP;

// 外部参照
extern HINSTANCE hInst;
extern TCHAR *g_Pass;
extern int gPassSt;
extern TCHAR *AppDir;
extern TCHAR *DefaultDataDir;
extern TCHAR *IniFile;
extern TCHAR *DataDir;
extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern BOOL first_start;
extern BOOL SaveBoxesLoaded;

/* Local Function Prototypes */
static void ini_get_encode_info(void);
static void get_sound_file(TCHAR *name, TCHAR **ret);
static void ini_check_window_pos(RECT *rect, RECT *scr_rect, int def_w, int def_l);

/*
 * ini_start_auth_check - Check Password
 */
BOOL ini_start_auth_check(void)
{
	TCHAR app_path[BUF_SIZE], app_path_old[BUF_SIZE];
	TCHAR tmp[MSG_SIZE];
	TCHAR pass[BUF_SIZE];
	BOOL retval = FALSE;

	ConvertFromNPOP = FALSE;
	if (IniFile != NULL) {
		long fsize;
		str_cpy_n_t(app_path, IniFile, BUF_SIZE);
		fsize = file_get_size(app_path);
		if (fsize == -2) {
			wsprintf(tmp, STR_ERR_FILE_TOO_LARGE, app_path);
			ErrorMessage(NULL, tmp);
			return FALSE;
		} else if (fsize == -1) {
			wsprintf(tmp, STR_ERR_INIFILE, app_path);
			ErrorMessage(NULL, tmp);
			return FALSE;
		}
	} else {
		long fsize;
		str_join_t(app_path, DefaultDataDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
		fsize = file_get_size(app_path);
		if (fsize == -2) {
			wsprintf(tmp, STR_ERR_FILE_TOO_LARGE, app_path);
			ErrorMessage(NULL, tmp);
			return FALSE;
		} else if (fsize == -1) {
			DWORD DirInfo;
			DirInfo = GetFileAttributes(DefaultDataDir);
			if (DirInfo & FILE_ATTRIBUTE_READONLY) {
				ErrorMessage(NULL, STR_ERR_READONLY);
			}
			// GJC - check if upgrading from original nPOP
			str_join_t(app_path_old, DefaultDataDir, TEXT("nPOP.ini"),  (TCHAR *)-1);
			if (file_get_size(app_path_old) != -1) {
				if (MessageBox(NULL, STR_Q_UPGRADE, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDYES) {
					ConvertFromNPOP = TRUE;
					CopyFile(app_path_old, app_path, FALSE);
				}
			}
		}
	}

	profile_initialize(app_path, TRUE);

	op.StartPass = profile_get_int(GENERAL, TEXT("StartPass"), 0);
	if (op.StartPass == 1) {
		profile_get_string(GENERAL, TEXT("pw"), TEXT(""), tmp, BUF_SIZE - 1);
		EncodePassword(TEXT("_pw_"), tmp, pass, BUF_SIZE - 1, TRUE);
		if (*pass == TEXT('\0')) {
			profile_free();
			return TRUE;
		}
		op.Password = alloc_copy_t(pass);
		//Starting password
		gPassSt = 0;
		retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), NULL, InputPassProc,
			(LPARAM)STR_TITLE_STARTPASSWORD);
		mem_free(&op.Password);
	} else {
		retval = TRUE;
	}
	profile_free();
	return retval;
}

/*
 * ini_get_encode_info - エンコード情報の取得
 */
static void ini_get_encode_info(void)
{
	mem_free(&op.HeadCharset);
	mem_free(&op.BodyCharset);

	switch (GetACP()) {
	case 932:	// ISO-2022-JP, euc-jp
		op.HeadCharset = alloc_copy_t(TEXT(CHARSET_ISO_2022_JP));
		op.HeadEncoding = 2;
		op.BodyCharset = alloc_copy_t(TEXT(CHARSET_ISO_2022_JP));
		op.BodyEncoding = 0;
		break;

	case 1250:	// ISO-8859-2
	case 1251:	// ISO-8859-5, koi8-r, koi8-ru
	case 1252:	// ISO-8859-1
	case 1257:	// ISO-8859-4
		op.HeadEncoding = 3;
		op.BodyEncoding = 3;
		break;

	case 874:	// tis-620, windows-874
	case 936:	// GB2312, hz-gb-2312
	case 949:	// ISO-2022-KR, euc-kr
	case 950:	// BIG5
	case 1253:	// ISO-8859-7
	case 1254:	// ISO-8859-3, ISO-8859-9
	case 1255:	// ISO-8859-8, ISO-8859-8-i, DOS-862
	case 1256:	// ISO-8859-6, ASMO-708, DOS-720, windows-1256
	case 1258:	// windows-1258
	default:
		op.HeadEncoding = 2;
		op.BodyEncoding = 2;
		break;
	}
#ifndef _WCE_OLD
	set_default_encode(GetACP(), &op.HeadCharset, &op.BodyCharset);
	if (op.HeadCharset == NULL) {
		op.HeadCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
	}
	if (op.BodyCharset == NULL) {
		op.BodyCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
	}
#else
	op.HeadCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
	op.BodyCharset = alloc_copy_t(TEXT(CHARSET_ISO_8859_1));
#endif
}

/*
 * get_sound_file
 */
static void get_sound_file(TCHAR *name, TCHAR **ret)
{
	TCHAR buf[BUF_SIZE];
	wsprintf(buf, TEXT("%sResource\\%s"), AppDir, name);
	if (file_get_size(buf) > 0) {
		*ret = alloc_copy_t(buf);
	}
}

/*
 * ini_read_general
 */
void ini_read_general(HWND hWnd)
{
	HDC hdc;
	RECT scr_rect;
	UINT char_set;
	TCHAR conv_buf[INI_BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
	int len, width, height, t, i;

	hdc = GetDC(hWnd);
#ifndef _WIN32_WCE
	char_set = GetTextCharset(hdc);
#else
	char_set = STR_DEFAULT_FONTCHARSET;
#endif
	ReleaseDC(hWnd, hdc);

#ifdef _DEBUG
	op.SocLog = profile_get_int(GENERAL, TEXT("SocLog"), 1);
#else
	op.SocLog = profile_get_int(GENERAL, TEXT("SocLog"), 0);
#endif

	memset (&scr_rect, 0, sizeof (RECT));
#if (WINVER >= 0x0500) && (!defined(_WIN32_WCE))
	if (op.osMajorVer > 4 || (op.osMajorVer == 4 && op.osMinorVer >= 10)) {
			// Win98 or later
		SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&scr_rect, 0);
	} else
#endif
	{
		scr_rect.left   = 0;
		scr_rect.top    = 0;
		scr_rect.right  = GetSystemMetrics(SM_CXSCREEN);
		scr_rect.bottom = GetSystemMetrics(SM_CYSCREEN);
#ifdef _WIN32_WCE
		scr_rect.top	 =  MENU_HEIGHT;
		scr_rect.right  -= 5; // so resize border is visible
		scr_rect.bottom -= MENU_HEIGHT; // ignoring sip status
#endif
	}

	op.view_font.name = profile_alloc_string(GENERAL, TEXT("FontName"), STR_DEFAULT_FONT);
	op.view_font.size = profile_get_int(GENERAL, TEXT("FontSize"), 9);
	if (op.view_font.size <= 0) {
		op.view_font.size = 9;
	}
	op.view_font.weight = profile_get_int(GENERAL, TEXT("FontWeight"), 0);
	op.view_font.italic = profile_get_int(GENERAL, TEXT("FontItalic"), 0);
	op.view_font.charset = profile_get_int(GENERAL, TEXT("FontCharset"), char_set);

	op.lv_font.name = profile_alloc_string(GENERAL, TEXT("LvFontName"), TEXT(""));
	if (op.Version < 2011 && (op.lv_font.name == NULL || *op.lv_font.name == TEXT('\0'))) {
		op.lv_font.size = 9;
	} else {
		op.lv_font.size = profile_get_int(GENERAL, TEXT("LvFontSize"), 9);
		if (op.lv_font.size <= 0) {
			op.lv_font.size = 9;
		}
	}
	op.lv_font.weight = profile_get_int(GENERAL, TEXT("LvFontWeight"), 0);
	op.lv_font.italic = profile_get_int(GENERAL, TEXT("LvFontItalic"), 0);
	op.lv_font.charset = profile_get_int(GENERAL, TEXT("LvFontCharset"), char_set);

	op.StatusBarCharWidth = profile_get_int(GENERAL, TEXT("StatusBarCharWidth"), 7);

	// only used in beta versions of nPOPuk 2.17
	op.Codepage = profile_alloc_string(GENERAL, TEXT("Codepage"), STR_DEFAULT_CODEPAGE);
	if (lstrcmpi(op.Codepage, TEXT("CP_ACP")) == 0) {
		CP_int = CP_ACP;
	} else if (lstrcmpi(op.Codepage, TEXT("CP_UTF8")) == 0) {
		CP_int = CP_UTF8;
	} else {
#ifdef UNICODE
		char *buf = alloc_tchar_to_char(op.Codepage);
		CP_int = atoi(buf);
		mem_free(&buf);
#else
		CP_int = atoi(op.Codepage);
#endif
	}
	op.HeadCharset = profile_alloc_string(GENERAL, TEXT("HeadCharset"), STR_DEFAULT_HEAD_CHARSET);
	op.HeadEncoding = profile_get_int(GENERAL, TEXT("HeadEncoding"), STR_DEFAULT_HEAD_ENCODE);
	op.BodyCharset = profile_alloc_string(GENERAL, TEXT("BodyCharset"), STR_DEFAULT_BODY_CHARSET);
	op.BodyEncoding = profile_get_int(GENERAL, TEXT("BodyEncoding"), STR_DEFAULT_BODY_ENCODE);
	if (op.HeadCharset == NULL || *op.HeadCharset == TEXT('\0') ||
		op.BodyCharset == NULL || *op.BodyCharset == TEXT('\0')) {
		ini_get_encode_info();
	}

	op.TimeZone = profile_alloc_string(GENERAL, TEXT("TimeZone"), STR_DEFAULT_TIMEZONE);
	op.DateFormat = profile_alloc_string(GENERAL, TEXT("DateFormat"), STR_DEFAULT_DATEFORMAT);
	op.TimeFormat = profile_alloc_string(GENERAL, TEXT("TimeFormat"), STR_DEFAULT_TIMEFORMAT);

	{
		int top, left;
#ifdef _WIN32_WCE
		top = 0; left = 0;
#else
		top = (scr_rect.bottom - scr_rect.top) / 4 + scr_rect.top;
		left = (scr_rect.right - scr_rect.left) / 4 + scr_rect.left;
		op.MainRect.left = profile_get_int(GENERAL, TEXT("left"), left);
		op.MainRect.top = profile_get_int(GENERAL, TEXT("top"), top);
		op.MainRect.right = profile_get_int(GENERAL, TEXT("right"), left+560);
		op.MainRect.bottom = profile_get_int(GENERAL, TEXT("bottom"), top+320);
		ini_check_window_pos(&op.MainRect, &scr_rect, 560, 320);

		top = op.MainRect.top; left = op.MainRect.left;
#endif
		op.AddrRect.left = profile_get_int(GENERAL, TEXT("AddressLeft"), left);
		op.AddrRect.top = profile_get_int(GENERAL, TEXT("AddressTop"), top);
		op.AddrRect.right = profile_get_int(GENERAL, TEXT("AddressRight"), left+400);
		op.AddrRect.bottom = profile_get_int(GENERAL, TEXT("AddressBottom"), top+300);
		ini_check_window_pos(&op.AddrRect, &scr_rect, 400, 300);

		op.MblRect.left = profile_get_int(GENERAL, TEXT("MblLeft"), left);
		op.MblRect.top = profile_get_int(GENERAL, TEXT("MblTop"), top);
		op.MblRect.right = profile_get_int(GENERAL, TEXT("MblRight"), left+400);
		op.MblRect.bottom = profile_get_int(GENERAL, TEXT("MblBottom"), top+300);
		ini_check_window_pos(&op.MblRect, &scr_rect, 400, 300);
	}

	op.ShowTrayIcon = profile_get_int(GENERAL, TEXT("ShowTrayIcon"), 1);
	op.StartHide = profile_get_int(GENERAL, TEXT("StartHide"), 0);
	op.MinsizeHide = profile_get_int(GENERAL, TEXT("MinsizeHide"), 0);
	op.CloseHide = profile_get_int(GENERAL, TEXT("CloseHide"), 0);
	op.TrayIconToggle = profile_get_int(GENERAL, TEXT("TrayIconToggle"), 0);
	op.StartInit = profile_get_int(GENERAL, TEXT("StartInit"), 0);

	op.LvDefSelectPos = profile_get_int(GENERAL, TEXT("LvDefSelectPos"), 1);
	op.LvAutoSort = profile_get_int(GENERAL, TEXT("LvAutoSort"), 1);
	op.LvSortItem = profile_get_int(GENERAL, TEXT("LvSortItem"), 3);
	op.LvSortFromAddressOnly = profile_get_int(GENERAL, TEXT("LvSortFromAddressOnly"), 0);
	op.LvThreadView = profile_get_int(GENERAL, TEXT("LvThreadView"), 0);
	op.LvStyle = profile_get_int(GENERAL, TEXT("LvStyle"), LVS_SHOWSELALWAYS | LVS_REPORT);
	op.LvStyleEx = profile_get_int(GENERAL, TEXT("LvStyleEx"), LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	op.LvColumnOrder = profile_alloc_string(GENERAL, TEXT("LvColumnOrder"), TEXT("SFDZ"));
	op.ContextMenuOption = profile_get_int(GENERAL, TEXT("ContextMenuOption"), 1);
	op.MBMenuWidth = profile_get_int(GENERAL, TEXT("MBMenuWidth"), 130);
	if (op.MBMenuWidth == 0) {
		op.MBMenuWidth = 130; // upgrade from previous default
	}
	op.MBMenuMinWidth = profile_get_int(GENERAL, TEXT("MBMenuMinWidth"), 35);
	if (op.MBMenuMinWidth < 0) {
		op.MBMenuMinWidth = -op.MBMenuMinWidth;
	}
	if (op.MBMenuWidth > 0 && op.MBMenuWidth < op.MBMenuMinWidth) {
		op.MBMenuWidth = op.MBMenuMinWidth;
	} else if (op.MBMenuWidth <= 0 && op.MBMenuWidth > -op.MBMenuMinWidth){
		op.MBMenuWidth = -op.MBMenuMinWidth;
	}
	width = GetSystemMetrics(SM_CXSCREEN);
	if (op.MBMenuWidth > width) {
		op.MBMenuWidth = width / 2;
	} else if (op.MBMenuWidth > width / 2) {
		op.MBMenuWidth = -op.MBMenuWidth; // hide it (too big)
	}
#ifdef _WIN32_WCE
	height = GetSystemMetrics(SM_CYSCREEN);
	height -= MENU_HEIGHT;
	op.PreviewPaneHeight = profile_get_int(GENERAL, TEXT("PreviewPaneHeight"), -height/2);
#else
	height = op.MainRect.bottom - op.MainRect.top;
	height -= 75; // allow for menubar, toolbar, status bar
	op.PreviewPaneHeight = profile_get_int(GENERAL, TEXT("PreviewPaneHeight"), -200);
#endif
	op.PreviewPaneMinHeight = profile_get_int(GENERAL, TEXT("PreviewPaneMinHeight"), 35);
	if (op.PreviewPaneMinHeight < 0) {
		op.PreviewPaneMinHeight = -op.PreviewPaneMinHeight;
	}
	if (op.PreviewPaneHeight > 0 && op.PreviewPaneHeight < op.PreviewPaneMinHeight) {
		op.PreviewPaneHeight = op.PreviewPaneMinHeight;
	} else if (op.PreviewPaneHeight <= 0 && op.PreviewPaneHeight > -op.PreviewPaneMinHeight){
		op.PreviewPaneHeight = -op.PreviewPaneMinHeight;
	}
	if (op.PreviewPaneHeight > height) {
		op.PreviewPaneHeight = height / 2;
	} else if (op.PreviewPaneHeight > (3*height)/4) {
		op.PreviewPaneHeight = -op.PreviewPaneHeight; // hide it (too big)
	}
	len = profile_get_string(GENERAL, TEXT("PreviewHeader"), TEXT("From: %F\\nTo: %T\\n{CC: %C\\n}Subject: %S\\nDate: %D\\n\\n"), conv_buf, INI_BUF_SIZE - 1);
	op.PreviewHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.PreviewHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.PreviewHeader);
	}
	op.AutoPreview = profile_get_int(GENERAL, TEXT("AutoPreview"), 0);
	op.PreviewedIsReadTime = profile_get_int(GENERAL, TEXT("PreviewedIsReadTime"), 5);

	op.SaveboxListCount = profile_get_int(GENERAL, TEXT("SaveboxListCount"), 10);

	t = profile_get_int(GENERAL, TEXT("MoveAllMailBox"), 1);
	op.ScanAllForUnread = profile_get_int(GENERAL, TEXT("ScanAllForUnread"), t);

	op.DelIsMarkDel = profile_get_int(GENERAL, TEXT("DelIsMarkDel"), 1);
	op.DeletedIsNotNew = profile_get_int(GENERAL, TEXT("DeletedIsNotNew"), 0);
	op.RecvScroll = profile_get_int(GENERAL, TEXT("RecvScroll"), 1);
	op.SaveMsg = profile_get_int(GENERAL, TEXT("SaveMsg"), 1);
	op.AutoSave = profile_get_int(GENERAL, TEXT("AutoSave"), 1);
	op.WriteMbox = profile_get_int(GENERAL, TEXT("WriteMbox"), 0);
	op.CheckQueuedOnExit = profile_get_int(GENERAL, TEXT("CheckQueuedOnExit"), 0);
	op.PromptSaveOnExit = profile_get_int(GENERAL, TEXT("PromptSaveOnExit"), 0);
#ifdef _WIN32_WCE
	op.LazyLoadMailboxes = profile_get_int(GENERAL, TEXT("LazyLoadMailboxes"), 1);
#else
	op.LazyLoadMailboxes = profile_get_int(GENERAL, TEXT("LazyLoadMailboxes"), 0);
#endif
	if (op.LazyLoadMailboxes == 0) {
		SaveBoxesLoaded = TRUE;
	}
	op.BlindAppend = profile_get_int(GENERAL, TEXT("BlindAppend"), 0);
	op.FixContentType = profile_get_int(GENERAL, TEXT("FixContentType"), 1);
	
	op.StartPass = profile_get_int(GENERAL, TEXT("StertPass"), 0);
	op.StartPass = profile_get_int(GENERAL, TEXT("StartPass"), op.StartPass);
	op.ShowPass = profile_get_int(GENERAL, TEXT("ShowPass"), 0);
	op.ConfigPass = profile_get_int(GENERAL, TEXT("ConfigPass"), 0);
	op.ScrambleMailboxes = profile_get_int(GENERAL, TEXT("ScrambleMailboxes"), 0);

#ifdef _WIN32_WCE
	op.LvColSize[0] = profile_get_int(GENERAL, TEXT("LvColSize-0"), 158);
	op.LvColSize[1] = profile_get_int(GENERAL, TEXT("LvColSize-1"), 146);
	op.LvColSize[2] = profile_get_int(GENERAL, TEXT("LvColSize-2"), 123);
	op.LvColSize[3] = profile_get_int(GENERAL, TEXT("LvColSize-3"), 77);
#else
	op.LvColSize[0] = profile_get_int(GENERAL, TEXT("LvColSize-0"), 150);
	op.LvColSize[1] = profile_get_int(GENERAL, TEXT("LvColSize-1"), 100);
	op.LvColSize[2] = profile_get_int(GENERAL, TEXT("LvColSize-2"), 110);
	op.LvColSize[3] = profile_get_int(GENERAL, TEXT("LvColSize-3"), 50);
#endif
	if (op.LvColSize[0] + op.LvColSize[1] + op.LvColSize[2] + op.LvColSize[3] < 10) {
		for (i = 0; i < LV_COL_CNT; i++) {
			op.LvColSize[i] = 20;
		}
	}

	op.AddColSize[0] = profile_get_int(GENERAL, TEXT("AddColSize-0"), 200);
	op.AddColSize[1] = profile_get_int(GENERAL, TEXT("AddColSize-1"), 150);
	op.AddColSize[2] = profile_get_int(GENERAL, TEXT("AddColSize-2"), -1);
	if (op.AddColSize[2] == -1) {
		op.AddColSize[2] = 100;
		if (op.AddColSize[0] > 200) {
			op.AddColSize[0] -= 50;
		}
		if (op.AddColSize[1] > 150) {
			op.AddColSize[1] -= 50;
		}
	}
	for (i = 0; i < AD_COL_CNT; i++) {
		if (op.AddColSize[i] > 1000) {
			op.AddColSize[i] = 1000;
		}
	}
	op.AddressSort = profile_get_int(GENERAL, TEXT("AddressSort"), 0);
	op.AddressJumpKey = profile_get_int(GENERAL, TEXT("AddressJumpKey"), 0);
	op.AutoAddRecipients = profile_get_int(GENERAL, TEXT("AutoAddRecipients"), 0);
	op.AddressShowGroup = profile_alloc_string(GENERAL, TEXT("AddressShowGroup"), TEXT(""));

	op.MblColSize[0] = profile_get_int(GENERAL, TEXT("MblColSize-0"), 110);
	op.MblColSize[1] = profile_get_int(GENERAL, TEXT("MblColSize-1"), 110);
	op.MblColSize[2] = profile_get_int(GENERAL, TEXT("MblColSize-2"), 70);
	op.MblColSize[3] = profile_get_int(GENERAL, TEXT("MblColSize-3"), 60);
	op.MblColSize[4] = profile_get_int(GENERAL, TEXT("MblColSize-4"), 50);
	op.MblColSize[5] = profile_get_int(GENERAL, TEXT("MblColSize-5"), 100);
	for (i = 0; i < MB_COL_CNT; i++) {
		if (op.MblColSize[i] > 1000) {
			op.MblColSize[i] = 1000;
		} else if (op.MblColSize[i] < 0) {
			op.MblColSize[i] = 10;	
		}
	}

#ifndef _WIN32_WCE
	op.ViewRect.left = profile_get_int(GENERAL, TEXT("viewleft"), 0);
	op.ViewRect.top = profile_get_int(GENERAL, TEXT("viewtop"), 0);
	op.ViewRect.right = profile_get_int(GENERAL, TEXT("viewright"), 450);
	op.ViewRect.bottom = profile_get_int(GENERAL, TEXT("viewbottom"), 400);
	ini_check_window_pos(&op.ViewRect, &scr_rect, 450, 400);

	op.EditRect.left = profile_get_int(GENERAL, TEXT("editleft"), 0);
	op.EditRect.top = profile_get_int(GENERAL, TEXT("edittop"), 0);
	op.EditRect.right = profile_get_int(GENERAL, TEXT("editright"), 450);
	op.EditRect.bottom = profile_get_int(GENERAL, TEXT("editbottom"), 400);
	ini_check_window_pos(&op.EditRect, &scr_rect, 450, 400);
#endif

	op.ShowHeader = profile_get_int(GENERAL, TEXT("ShowHeader"), 0);
	op.GetRecent = profile_get_int(GENERAL, TEXT("GetRecent"), -20);
#ifdef _WIN32_WCE_PPC
////////////////////// MRP ////////////////////
	op.ListGetLine = profile_get_int(GENERAL, TEXT("ListGetLine"), 50);
#else
	op.ListGetLine = profile_get_int(GENERAL, TEXT("ListGetLine"), 100);
////////////////////// --- ////////////////////
#endif

	op.ListDownload = profile_get_int(GENERAL, TEXT("ListDownload"), 0);
	op.ListSaveMode = profile_get_int(GENERAL, TEXT("ListSaveMode"), 2);
	op.WordBreakFlag = profile_get_int(GENERAL, TEXT("WordBreakFlag"), 1);
	op.EditWordBreakFlag = profile_get_int(GENERAL, TEXT("EditWordBreakFlag"), 1);

#ifdef _WIN32_WCE_PPC
	op.OptionalScrollbar = profile_get_int(GENERAL, TEXT("OptionalScrollbar"), 1);

////////////////////// MRP ////////////////////
	op.ViewShowDate = profile_get_int(GENERAL, TEXT("ViewShowDate"), 1);
#else
	op.ViewShowDate = profile_get_int(GENERAL, TEXT("ViewShowDate"), 0);
////////////////////// --- ////////////////////
#endif
	op.MatchCase = profile_get_int(GENERAL, TEXT("MstchCase"), 0);
	op.MatchCase = profile_get_int(GENERAL, TEXT("MatchCase"), op.MatchCase);
	op.Wildcards = profile_get_int(GENERAL, TEXT("Wildcards"), 0);
	op.AllMsgFind = profile_get_int(GENERAL, TEXT("AllFind"), 1);
	op.AllMsgFind = profile_get_int(GENERAL, TEXT("AllMsgFind"), op.AllMsgFind);
	op.AllBoxFind = profile_get_int(GENERAL, TEXT("AllBoxFind"), 1);
	op.SubjectFind = profile_get_int(GENERAL, TEXT("SubjectFind"), 0);

	op.ESMTP = profile_get_int(GENERAL, TEXT("ESMTP"), 0);
	op.SendHelo = profile_alloc_string(GENERAL, TEXT("SendHelo"), TEXT(""));
	op.SendMessageId = profile_get_int(GENERAL, TEXT("SendMessageId"), 1);
	op.SendDate = profile_get_int(GENERAL, TEXT("SendDate"), 1);
	op.SelectSendBox = profile_get_int(GENERAL, TEXT("SelectSendBox"), 1);
	op.ExpertMode = profile_get_int(GENERAL, TEXT("DisableWarning"), 0);		// Added PHH 4-Oct-2003
	op.PopBeforeSmtpIsLoginOnly = profile_get_int(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), 1);
	op.PopBeforeSmtpWait = profile_get_int(GENERAL, TEXT("PopBeforeSmtpWait"), 300);
	op.NoEmptyMailbox = profile_get_int(GENERAL, TEXT("NoEmptyMailbox"), 0);

	op.AutoQuotation = profile_get_int(GENERAL, TEXT("AutoQuotation"), 1);
	op.FwdQuotation = profile_get_int(GENERAL, TEXT("FwdQuotation"), 1);
	op.SignForward = profile_get_int(GENERAL, TEXT("SignForward"), 1);
	op.SignReplyAbove = profile_get_int(GENERAL, TEXT("SignReplyAbove"), 0);
	op.QuotationChar = profile_alloc_string(GENERAL, TEXT("QuotationChar"), TEXT("> "));
#ifdef _WIN32_WCE_PPC
////////////////////// MRP ////////////////////
	op.WordBreakSize = profile_get_int(GENERAL, TEXT("WordBreakSize"), 80);
#else
	op.WordBreakSize = profile_get_int(GENERAL, TEXT("WordBreakSize"), 70);
////////////////////// --- ////////////////////
#endif
	op.QuotationBreak = profile_get_int(GENERAL, TEXT("QuotationBreak"), 1);
	op.ReSubject = profile_alloc_string(GENERAL, TEXT("ReSubject"), TEXT("Re: "));
	op.FwdSubject = profile_alloc_string(GENERAL, TEXT("FwdSubject"), TEXT("Fwd: "));	// Added PHH 4-Oct-2003
	op.ReFwdPrefixes = profile_alloc_string(GENERAL, TEXT("ReFwdPrefixes"), TEXT("Re:,Fwd:"));
	len = profile_get_string(GENERAL, TEXT("ReHeader"), TEXT("\\n--------------------------------------------------\\n%f wrote:\\n(%d)\\n"), conv_buf, INI_BUF_SIZE - 1);
	op.ReHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.ReHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.ReHeader);
	}
	len = profile_get_string(GENERAL, TEXT("FwdHeader"), TEXT("\\n--------------------------------------------------\\nFrom: %F\\nTo: %T\\n{CC: %C\\n}Subject: %S\\nDate: %D\\n"), conv_buf, INI_BUF_SIZE - 1);
	op.FwdHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.FwdHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.FwdHeader);
	}
	op.AltReplyTo = profile_alloc_string(GENERAL, TEXT("ReplyTo"), TEXT(""));

	op.Bura = profile_alloc_string(GENERAL, TEXT("Bura"), STR_DEFAULT_BURA);
	op.Oida = profile_alloc_string(GENERAL, TEXT("Oida"), STR_DEFAULT_OIDA);

	op.CAFile = profile_alloc_string(GENERAL, TEXT("CAFile"), TEXT("ca.pem"));
	op.IPCache = profile_get_int(GENERAL, TEXT("IPCache"), 1);
	op.EncodeType = profile_get_int(GENERAL, TEXT("EncodeType"), 0);

	op.ShowNewMailMessage = profile_get_int(GENERAL, TEXT("ShowNewMailMessgae"), 1);
	op.ShowNewMailMessage = profile_get_int(GENERAL, TEXT("ShowNewMailMessage"), op.ShowNewMailMessage);
	op.ShowNoMailMessage = profile_get_int(GENERAL, TEXT("ShowNoMailMessage"), 0);
#ifdef _WIN32_WCE
	op.ActiveNewMailMessage = profile_get_int(GENERAL, TEXT("ActiveNewMailMessgae"), 1);
#else
	op.ActiveNewMailMessage = profile_get_int(GENERAL, TEXT("ActiveNewMailMessgae"), 0);
#endif
	op.ActiveNewMailMessage = profile_get_int(GENERAL, TEXT("ActiveNewMailMessage"), op.ActiveNewMailMessage);
	op.ClearNewOverlay = profile_get_int(GENERAL, TEXT("ClearNewOverlay"), 0);

#ifdef _WIN32_WCE
	///////////// MRP /////////////////////
	op.UsePOOMAddressBook = profile_get_int(GENERAL, TEXT("UsePOOMAddressBook"), 0);
	///////////// --- /////////////////////
	op.POOMNameIsComment = profile_get_int(GENERAL, TEXT("POOMNameIsComment"), 0);
	op.LoadPOOMAtStart = profile_get_int(GENERAL, TEXT("LoadPOOMAtStart"), 1);
#endif
#ifdef _WIN32_WCE_PPC
#ifdef DO_SSL
	i = 0;
#else
	i = profile_get_int(GENERAL, TEXT("UseBuiltinSSL"), 0);
#endif
	op.UseWindowsSSL = profile_get_int(GENERAL, TEXT("UseWindowsSSL"), i);
	op.ShowNavButtons = profile_get_int(GENERAL, TEXT("ShowNavButtons"), 1);
	op.PromptIniEdit = profile_get_int(GENERAL, TEXT("PromptIniEdit"), 0);
#endif

	wsprintf(tmp, TEXT("%sResource"), AppDir);
	if (dir_check(tmp)) {
		op.HasResourceDir = 1;
	} else {
		op.HasResourceDir = 0;
	}
	op.NewMailSound = profile_get_int(GENERAL, TEXT("NewMailSound"), 1);
	op.NewMailSoundFile = profile_alloc_string(GENERAL, TEXT("NewMailSoundFile"), TEXT(""));
	op.ExecEndSound = profile_get_int(GENERAL, TEXT("ExecEndSound"), 0);
	op.ExecEndSoundFile = profile_alloc_string(GENERAL, TEXT("ExecEndSoundFile"), TEXT(""));
	op.ItemPlaySound = profile_get_int(GENERAL, TEXT("ItemPlaySound"), 0);

	if (op.ItemPlaySound > 0 && op.HasResourceDir) {
		get_sound_file(TEXT("NEW.WAV"), &op.ItemNewSoundFile);
		get_sound_file(TEXT("PARTIAL.WAV"), &op.ItemPartialSoundFile);
		get_sound_file(TEXT("FULL.WAV"), &op.ItemFullSoundFile);
		get_sound_file(TEXT("ATTACH.WAV"), &op.ItemAttachSoundFile);
		get_sound_file(TEXT("ATTACH_HTML.WAV"), &op.ItemHtmlSoundFile);
		get_sound_file(TEXT("NO_ICON.WAV"), &op.ItemNonSoundFile);
		get_sound_file(TEXT("UNREAD.WAV"), &op.ItemUnreadSoundFile);
		get_sound_file(TEXT("READ.WAV"), &op.ItemReadSoundFile);
		get_sound_file(TEXT("DOWNLOAD.WAV"), &op.ItemDownSoundFile);
		get_sound_file(TEXT("DELETE.WAV"), &op.ItemDelSoundFile);
		get_sound_file(TEXT("SEND.WAV"), &op.ItemSendSoundFile);
		get_sound_file(TEXT("SENT.WAV"), &op.ItemSentSoundFile);
		get_sound_file(TEXT("ERROR.WAV"), &op.ItemErrorSoundFile);
		get_sound_file(TEXT("FLAG.WAV"), &op.ItemFlagSoundFile);
	}

	op.AutoCheck = profile_get_int(GENERAL, TEXT("AutoCheck"), 0);
	op.AutoCheckTime = profile_get_int(GENERAL, TEXT("AutoCheckTime"), 10);
	op.StartCheck = profile_get_int(GENERAL, TEXT("StartCheck"), 0);
	op.CheckAfterUpdate = profile_get_int(GENERAL, TEXT("CheckAfterUpdate"), 0);
	op.SocIgnoreError = profile_get_int(GENERAL, TEXT("SocIgnoreError"), 0);
	op.SendIgnoreError = profile_get_int(GENERAL, TEXT("SendIgnoreError"), 0);
	op.NoIgnoreErrorTimeout = profile_get_int(GENERAL, TEXT("NoIgnoreErrorTimeout"), 0);
	op.SendAttachIndividually = profile_get_int(GENERAL, TEXT("SendAttachIndividually"), 1);
	op.CheckEndExec = profile_get_int(GENERAL, TEXT("CheckEndExec"), 0);
	op.CheckEndExecNoDelMsg = profile_get_int(GENERAL, TEXT("CheckEndExecNoDelMsg"), 1);
	op.DeletePerUpdateLimit = profile_get_int(GENERAL, TEXT("DeletePerUpdateLimit"), 0);
	op.TimeoutInterval = profile_get_int(GENERAL, TEXT("TimeoutInterval"), -1);
	if (op.TimeoutInterval <= 0) {
		op.TimeoutInterval = 120;
	} else if (op.Version < 2007) {
		// value was in minutes before, now in seconds
		op.TimeoutInterval *= 60;
	}
	op.ReorderSendbox = profile_get_int(GENERAL, TEXT("ReorderSendbox"), 1);

	op.ViewClose = profile_get_int(GENERAL, TEXT("ViewClose"), 1);
	op.ViewCloseNoNext = profile_get_int(GENERAL, TEXT("ViewCloseNoNext"), 1);
	op.ViewNextAfterDel = profile_get_int(GENERAL, TEXT("ViewNextAfterDel"), 1);
	op.ViewSkipDeleted = profile_get_int(GENERAL, TEXT("ViewSkipDeleted"), 2);
	op.ViewApp = profile_alloc_string(GENERAL, TEXT("ViewApp"), TEXT(""));
	op.ViewAppCmdLine = profile_alloc_string(GENERAL, TEXT("ViewAppCmdLine"), TEXT(""));
#ifdef _WIN32_WCE_PPC
	///////////// MRP /////////////////////
	op.ViewFileSuffix = profile_alloc_string(GENERAL, TEXT("ViewFileSuffix"), TEXT("html"));
	len = profile_get_string(GENERAL, TEXT("ViewFileHeader"),
		TEXT("<B>From:</B> %f<BR>\\n<B>To:</B> %t<BR>\\n{<B>Cc:</B> %c<BR>\\n}<B>Subject:</B> %s<BR>\\n<B>Date:</B> %d<BR>\\n<HR>\\n<BR>\\n"), conv_buf, INI_BUF_SIZE - 1);
	///////////// --- /////////////////////
#else
	op.ViewFileSuffix = profile_alloc_string(GENERAL, TEXT("ViewFileSuffix"), TEXT("txt"));
	len = profile_get_string(GENERAL, TEXT("ViewFileHeader"),
		TEXT("From: %f\\nTo: %t\\n{Cc: %c\\n}Subject: %s\\nDate: %d\\n\\n"), conv_buf, INI_BUF_SIZE - 1);
#endif
	op.ViewFileHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.ViewFileHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.ViewFileHeader);
	}
	op.ViewAppClose = profile_get_int(GENERAL, TEXT("ViewAppClose"), 0);
	op.DefViewApp = profile_get_int(GENERAL, TEXT("DefViewApp"), 0);
	op.ViewAppMsgSource = profile_get_int(GENERAL, TEXT("ViewAppMsgSource"), 0);
	op.AutoOpenAttachMsg = profile_get_int(GENERAL, TEXT("AutoOpenAttachMsg"), 0);
	op.ViewWindowCursor = profile_get_int(GENERAL, TEXT("ViewWindowCursor"), 0);
	op.ViewShowAttach = profile_get_int(GENERAL, TEXT("ViewShowAttach"), 0);
	op.EditApp = profile_alloc_string(GENERAL, TEXT("EditApp"), TEXT(""));
	op.EditAppCmdLine = profile_alloc_string(GENERAL, TEXT("EditAppCmdLine"), TEXT(""));
	op.EditFileSuffix = profile_alloc_string(GENERAL, TEXT("EditFileSuffix"), TEXT("txt"));
	op.DefEditApp = profile_get_int(GENERAL, TEXT("DefEditApp"), 0);
	op.AttachPath = profile_alloc_string(GENERAL, TEXT("AttachPath"), TEXT("attach"));
	//op.AttachWarning = profile_get_int(GENERAL, TEXT("AttachWarning"), 1);
	op.AttachDelete = profile_get_int(GENERAL, TEXT("AttachDelete"), 1);
	op.StripHtmlTags = profile_get_int(GENERAL, TEXT("StripHtmlTags"), 1);
#ifdef _WIN32_WCE_PPC
	{
		TCHAR *OpenSaveDir = profile_alloc_string(GENERAL, TEXT("OpenSaveDir"), DataDir);
		op.SavedOpenDir = profile_alloc_string(GENERAL, TEXT("SavedOpenDir"), OpenSaveDir);
		op.SavedSaveDir = profile_alloc_string(GENERAL, TEXT("SavedSaveDir"), OpenSaveDir);
		mem_free(&OpenSaveDir);
	}
#else
	op.SavedOpenDir = profile_alloc_string(GENERAL, TEXT("SavedOpenDir"), DataDir);
	op.SavedSaveDir = profile_alloc_string(GENERAL, TEXT("SavedSaveDir"), DataDir);
#endif


#ifdef _WIN32_WCE_LAGENDA
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT("internet.exe"));
#else	// _WIN32_WCE_LAGENDA
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT(""));
#endif	// _WIN32_WCE_LAGENDA
	op.URLAppCmdLine = profile_alloc_string(GENERAL, TEXT("URLAppCmdLine"), TEXT(""));

	op.EnableLAN = profile_get_int(GENERAL, TEXT("EnableLAN"), 0);
}

/*
 * ini_read_setting - INIファイルから設定情報を読みこむ
 */
BOOL ini_read_setting(HWND hWnd)
{
	DWORD DirInfo;
	TCHAR app_path[BUF_SIZE];
	TCHAR buf[BUF_SIZE];
	TCHAR key_buf[BUF_SIZE];
	TCHAR ret[BUF_SIZE];
	TCHAR msg[MSG_SIZE];
	TCHAR *ConvertName = NULL;
	TCHAR *p, *r;
	int i, j, t, cnt, num, len;
	int fDef;

	if (IniFile != NULL) {
		long fsize;
		str_cpy_n_t(app_path, IniFile, BUF_SIZE);
		fsize = file_get_size(app_path);
		if (fsize == -2) {
			wsprintf(msg, STR_ERR_FILE_TOO_LARGE, app_path);
			ErrorMessage(hWnd, msg);
			return FALSE;
		} else if (fsize == -1) {
			wsprintf(msg, STR_ERR_INIFILE, app_path);
			ErrorMessage(NULL, msg);
			return FALSE;
		}
	} else {
		str_join_t(app_path, DefaultDataDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	}
	if (profile_initialize(app_path, FALSE) == FALSE) {
		return FALSE;
	}

	// cache at start-up
	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, (LPTSTR)op.DecPt, 4);

	len = profile_get_string(GENERAL, TEXT("DataFileDir"), TEXT(""), op.DataFileDir, BUF_SIZE - 1);

	if (*op.DataFileDir == TEXT('\0')) {
		DataDir = DefaultDataDir;
//GJC } else if (*op.DataFileDir == TEXT('.')) {
		// relative path
// look out: DataDir shouldn't be freed in the other two cases ...
	} else {
		DataDir = op.DataFileDir;
		for (p = r = DataDir; *p != TEXT('\0'); p++) {
#ifndef UNICODE
			if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
				p++;
				continue;
			}
#endif
			if (*p == TEXT('\\') || *p == TEXT('/')) {
				r = p;
			}
		}
		if (r != (DataDir + lstrlen(DataDir) - 1) || lstrlen(DataDir) == 1) {
			lstrcat(DataDir, TEXT("\\"));
		}
		DirInfo = GetFileAttributes(DataDir);
		if ((DirInfo == 0xFFFFFFFF) || !(DirInfo & FILE_ATTRIBUTE_DIRECTORY)) {
			MessageBox(NULL, STR_ERR_NODATADIR, KEY_NAME, MB_OK | MB_ICONERROR);
			return FALSE;
		} else if (DirInfo & FILE_ATTRIBUTE_READONLY) {
			MessageBox(NULL, STR_ERR_DATAREADONLY, KEY_NAME, MB_OK);
		}
	}

	op.BackupDir = profile_alloc_string(GENERAL, TEXT("BackupDir"), TEXT(""));

	op.Version = profile_get_int(GENERAL, TEXT("Version"), 0);
	if (op.Version > APP_VERSION_NUM) {
		wsprintf(msg, STR_MSG_NEWVERSION, app_path, KEY_NAME);
		if (MessageBox(hWnd, msg, WINDOW_TITLE, MB_YESNO) == IDNO) {
			return FALSE;
		}
	}
	// if op.Version < APP_VERSION_NUM, adjust at end (after checking other settings)

	{
		OSVERSIONINFO os_info;
		os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&os_info);
		op.osPlatformId = os_info.dwPlatformId;
		op.osMajorVer = os_info.dwMajorVersion;
		op.osMinorVer = os_info.dwMinorVersion;
	}

	ini_read_general(hWnd);

	profile_get_string(GENERAL, TEXT("pw"), TEXT(""), ret, BUF_SIZE - 1);
	EncodePassword(TEXT("_pw_"), ret, buf, BUF_SIZE - 1, TRUE);
	op.Password = alloc_copy_t(buf);

	op.WifiCon = profile_get_int(GENERAL, TEXT("WifiCon"), 1);
	op.WifiCheckEndDisCon = profile_get_int(GENERAL, TEXT("WifiCheckEndDisCon"), 1);
	op.WifiCheckEndDisConTimeout = profile_get_int(GENERAL, TEXT("WifiCheckEndDisConTimeout"), 0);
	op.WifiExitDisCon = profile_get_int(GENERAL, TEXT("WifiExitDisCon"), 1);
	op.WifiNoCheck = profile_get_int(GENERAL, TEXT("WifiNoCheck"), 1);
	op.WifiWaitSec = profile_get_int(GENERAL, TEXT("WifiWaitSec"), 5);
//	op.WifiDeviceName = profile_alloc_string(GENERAL, TEXT("WifiDeviceName"), TEXT(""));
	op.WifiDeviceName = profile_alloc_string(GENERAL, TEXT("WifiDeviceName"), TEXT("RT28701"));

	op.RasCon = profile_get_int(GENERAL, TEXT("RasCon"), 1);
	op.RasCheckEndDisCon = profile_get_int(GENERAL, TEXT("RasCheckEndDisCon"), 1);
	op.RasCheckEndDisConTimeout = profile_get_int(GENERAL, TEXT("RasCheckEndDisConTimeout"), 0);
	op.RasEndDisCon = profile_get_int(GENERAL, TEXT("RasEndDisCon"), 1);
	op.RasNoCheck = profile_get_int(GENERAL, TEXT("RasNoCheck"), 1);
	op.RasWaitSec = profile_get_int(GENERAL, TEXT("RasWaitSec"), 5);

	op.RasInfoCnt = profile_get_int(GENERAL, TEXT("RasInfoCnt"), 0);
	op.RasInfo = (RASINFO **)mem_calloc(sizeof(RASINFO *) * op.RasInfoCnt);
	if (op.RasInfo == NULL) {
		op.RasInfoCnt = 0;
	}
	for (j = 0; j < op.RasInfoCnt; j++) {
		*(op.RasInfo + j) = (RASINFO *)mem_calloc(sizeof(RASINFO));
		if (*(op.RasInfo + j) == NULL) {
			continue;
		}
		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasEntry"));
		(*(op.RasInfo + j))->RasEntry = profile_alloc_string(TEXT("RASINFO"), key_buf, TEXT(""));

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasUser"));
		(*(op.RasInfo + j))->RasUser = profile_alloc_string(TEXT("RASINFO"), key_buf, TEXT(""));

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasPass"));
		len = profile_get_string(TEXT("RASINFO"), key_buf, TEXT(""), ret, BUF_SIZE - 1);
		EncodePassword((*(op.RasInfo + j))->RasUser, ret, msg, BUF_SIZE - 1, TRUE);
		(*(op.RasInfo + j))->RasPass = alloc_copy_t(msg);
	}

	op.GlobalFilterEnable = profile_get_int(GENERAL, TEXT("GlobalFilterEnable"), 0);
	op.GlobalFilterCnt = profile_get_int(GENERAL, TEXT("GlobalFilterCnt"), 0);

	if (op.GlobalFilterCnt > 0) {
		op.tpFilter = (FILTER **)mem_calloc(sizeof(FILTER *) * op.GlobalFilterCnt);
		if (op.tpFilter == NULL) {
			op.GlobalFilterCnt = 0;
		}
	} else {
		op.tpFilter = NULL;
	}
	for (t = 0; t < op.GlobalFilterCnt; t++) {
		FILTER *tpFilter = *(op.tpFilter + t) = (FILTER *)mem_calloc(sizeof(FILTER));
		if (tpFilter == NULL) {
			continue;
		}

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
		tpFilter->Enable = profile_get_int(TEXT("FILTER"), key_buf, 0);
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
		tpFilter->Action = profile_get_int(TEXT("FILTER"), key_buf, 0);
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Priority"));
		tpFilter->Priority = profile_get_int(TEXT("FILTER"), key_buf, 0);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
		tpFilter->SaveboxName = profile_alloc_string(TEXT("FILTER"), key_buf, TEXT(""));
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("FwdAddress"));
		tpFilter->FwdAddress = profile_alloc_string(TEXT("FILTER"), key_buf, TEXT(""));

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
		tpFilter->Header1 = profile_alloc_string(TEXT("FILTER"), key_buf, TEXT(""));

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
		tpFilter->Content1 = profile_alloc_string(TEXT("FILTER"), key_buf, TEXT(""));

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
		tpFilter->Boolean = profile_get_int(TEXT("FILTER"), key_buf, 0);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
		tpFilter->Header2 = profile_alloc_string(TEXT("FILTER"), key_buf, TEXT(""));

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
		tpFilter->Content2 = profile_alloc_string(TEXT("FILTER"), key_buf, TEXT(""));
	}

	if (op.Version < APP_VERSION_NUM) {
		op.Version = APP_VERSION_NUM;
	}

	cnt = profile_get_int(GENERAL, TEXT("MailBoxCnt"), 0);
	if (cnt == 0) {
		mailbox_create(hWnd, 1, -1, FALSE, FALSE);
		first_start = TRUE;
		profile_free();
		return TRUE;
	}
	if (mailbox_create(hWnd, cnt, -1, FALSE, FALSE) == -1) {
		profile_free();
		return FALSE;
	}

	// load OutBox and Address book
	if (mailbox_read() == FALSE) {
		profile_free();
		return FALSE;
	}

	for (i = 0; i < cnt; i++) {
		num = i + MAILBOX_USER;
		wsprintf(buf, TEXT("MAILBOX-%d"), i);

		// Name
		(MailBox + num)->Name = profile_alloc_string(buf, TEXT("Name"), TEXT(""));
		// Filename
		(MailBox + num)->Filename = profile_alloc_string(buf, TEXT("Filename"), TEXT(""));
		if (*(MailBox + num)->Filename == TEXT('\0')) {
			mem_free(&(MailBox + num)->Filename);
			(MailBox + num)->Filename = NULL;
		}
		// NewMailSoundFile
		(MailBox + num)->NewMailSoundFile = profile_alloc_string(buf, TEXT("NewMailSoundFile"), TEXT(""));
		if (*(MailBox + num)->NewMailSoundFile == TEXT('\0')) {
			mem_free(&(MailBox + num)->NewMailSoundFile);
			(MailBox + num)->NewMailSoundFile = NULL;
		}
		// Type
		(MailBox + num)->Type = profile_get_int(buf, TEXT("Type"), 0);

		(MailBox + num)->WasMbox = -1; // unknown, updated when loaded
		if ((MailBox + num)->Type == MAILBOX_TYPE_SAVE) {
			// GJC - SaveBox type (not an account)

			// Default account
			(MailBox + num)->DefAccount = profile_alloc_string(buf, TEXT("DefAccount"), TEXT(""));

			// Exempt from check all
			(MailBox + num)->CyclicFlag = 1;
			if (op.LazyLoadMailboxes == 0) {
				if ((MailBox+num)->Filename == NULL) {
					wsprintf(buf, TEXT("MailBox%d.dat"), i);
				} else {
					lstrcpy(buf, (MailBox+num)->Filename);
				}
				if (file_read_mailbox(buf, (MailBox + num), FALSE, TRUE) == FALSE) {
					profile_free();
					wsprintf(msg, STR_ERR_OPENMAILBOX, buf);
					ErrorMessage(hWnd, msg);
					return FALSE;
				}
			} else {
				if ((MailBox+num)->Filename == NULL) {
					wsprintf(buf, TEXT("%sMailBox%d.dat"), DataDir, i);
				} else {
					wsprintf(buf, TEXT("%s%s"), DataDir, (MailBox+num)->Filename);
				}
				(MailBox + num)->DiskSize = file_get_size(buf);
				(MailBox + num)->Loaded = FALSE;
			}
			continue;
		}
		// Server
		(MailBox + num)->Server = profile_alloc_string(buf, TEXT("Server"), TEXT(""));
		// Port
		(MailBox + num)->Port = profile_get_int(buf, TEXT("Port"), 110);
		// User
		(MailBox + num)->User = profile_alloc_string(buf, TEXT("User"), TEXT(""));
		// Pass
		profile_get_string(buf, TEXT("Pass"), TEXT(""), ret, BUF_SIZE - 1);
		EncodePassword((MailBox + num)->User, ret, msg, BUF_SIZE - 1, TRUE);
		(MailBox + num)->Pass = alloc_copy_t(msg);
		// APOP
		(MailBox + num)->APOP = profile_get_int(buf, TEXT("APOP"), 0);
		// POP SSL
		(MailBox + num)->PopSSL = profile_get_int(buf, TEXT("PopSSL"), 0);
		// POP SSL Option
		(MailBox + num)->PopSSLInfo.Type = profile_get_int(buf, TEXT("PopSSLType"), 0);
		(MailBox + num)->PopSSLInfo.Verify = profile_get_int(buf, TEXT("PopSSLVerify"), 1);
		(MailBox + num)->PopSSLInfo.Depth = profile_get_int(buf, TEXT("PopSSLDepth"), -1);
		(MailBox + num)->PopSSLInfo.Cert = profile_alloc_string(buf, TEXT("PopSSLCert"), TEXT(""));
		(MailBox + num)->PopSSLInfo.Pkey = profile_alloc_string(buf, TEXT("PopSSLPkey"), TEXT(""));
		(MailBox + num)->PopSSLInfo.Pass = profile_alloc_string(buf, TEXT("PopSSLPass"), TEXT(""));
		// Disable RETR
		(MailBox + num)->NoRETR = profile_get_int(buf, TEXT("NoRETR"), 0);
		// Disable UIDL
		(MailBox + num)->NoUIDL = profile_get_int(buf, TEXT("NoUIDL"), 0);
		// Yahoo! sometimes misreports size
		(MailBox + num)->MessageSizeDelta = profile_get_int(buf, TEXT("MessageSizeDelta"), 0);

		// MailCnt
		(MailBox + num)->MailCnt = profile_get_int(buf, TEXT("MailCnt"), 0);
		// MailSize
		(MailBox + num)->MailSize = profile_get_int(buf, TEXT("MailSize"), 0);

		(MailBox + num)->StartInit = profile_get_int(buf, TEXT("StartInit"), 0);
		if (op.StartInit == 0 && (MailBox + num)->StartInit == 0) {
			// LastMessageId
			profile_get_string(buf, TEXT("LastMessageId"), TEXT(""), ret, BUF_SIZE - 1);
			(MailBox + num)->LastMessageId = alloc_tchar_to_char(ret);
			// LastNo
			(MailBox + num)->LastNo = profile_get_int(buf, TEXT("LastNo"), 0);
		} else {
			// 起動時に新着位置の初期化
			(MailBox + num)->LastNo = -1;
			if (op.StartInit == 2 || (MailBox + num)->StartInit == 2) {
				// fill in from start, don't delete existing messages
				(MailBox+num)->ListInitMsg = FALSE;
			} else {
				(MailBox+num)->ListInitMsg = TRUE;
			}
		}

		// CyclicFlag
		(MailBox + num)->CyclicFlag = profile_get_int(buf, TEXT("CyclicFlag"), 0);

		// SmtpServer
		(MailBox + num)->SmtpServer = profile_alloc_string(buf, TEXT("SmtpServer"), TEXT(""));
		// SmtpPort
		(MailBox + num)->SmtpPort = profile_get_int(buf, TEXT("SmtpPort"), 25);
		// UserName
		(MailBox + num)->UserName = profile_alloc_string(buf, TEXT("UserName"), TEXT(""));
		// MailAddress
		(MailBox + num)->MailAddress = profile_alloc_string(buf, TEXT("MailAddress"), TEXT(""));
		// Signature
		p = (TCHAR *)mem_alloc(sizeof(TCHAR) * MAXSIZE);
		if (p != NULL) {
			len = profile_get_string(buf, TEXT("Signature"), TEXT(""), p, MAXSIZE - 1);
			(MailBox + num)->Signature = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
			if ((MailBox + num)->Signature != NULL) {
				DecodeCtrlChar(p, (MailBox + num)->Signature);
			}
			mem_free(&p);
		}
		// ReplyTo
		(MailBox + num)->ReplyTo = profile_alloc_string(buf, TEXT("ReplyTo"), TEXT(""));
		// UseReplyToForFrom
		(MailBox + num)->UseReplyToForFrom = profile_get_int(buf, TEXT("UseReplyToForFrom"), 0);
		// SendWireForm
		(MailBox + num)->SendWireForm = profile_get_int(buf, TEXT("SendWireForm"), 1);
		// MyAddr2Bcc
		(MailBox + num)->MyAddr2Bcc = profile_get_int(buf, TEXT("MyAddr2Bcc"), 0);
		// BccAddr
		(MailBox + num)->BccAddr = profile_alloc_string(buf, TEXT("BccAddr"), TEXT(""));

		// POP before SMTP
		(MailBox + num)->PopBeforeSmtp = profile_get_int(buf, TEXT("PopBeforeSmtp"), 0);
		// SMTP Authentication
		(MailBox + num)->SmtpAuth = profile_get_int(buf, TEXT("SmtpAuth"), 0);
		// SMTP Authentication type
		(MailBox + num)->SmtpAuthType = profile_get_int(buf, TEXT("SmtpAuthType"), 0);
		// SMTP Authentication User & Pass mode
		(MailBox + num)->AuthUserPass = profile_get_int(buf, TEXT("AuthUserPass"), 0);
		// SMTP Authentication User
		(MailBox + num)->SmtpUser = profile_alloc_string(buf, TEXT("SmtpUser"), TEXT(""));
		// SMTP Authentication Pass
		profile_get_string(buf, TEXT("SmtpPass"), TEXT(""), ret, BUF_SIZE - 1);
		EncodePassword((MailBox + num)->SmtpUser, ret, msg, BUF_SIZE - 1, TRUE);
		(MailBox + num)->SmtpPass = alloc_copy_t(msg);
		// SMTP SSL
		(MailBox + num)->SmtpSSL = profile_get_int(buf, TEXT("SmtpSSL"), 0);
		// SMTP SSL Option
		(MailBox + num)->SmtpSSLInfo.Type = profile_get_int(buf, TEXT("SmtpSSLType"), 0);
		(MailBox + num)->SmtpSSLInfo.Verify = profile_get_int(buf, TEXT("SmtpSSLVerify"), 1);
		(MailBox + num)->SmtpSSLInfo.Depth = profile_get_int(buf, TEXT("SmtpSSLDepth"), -1);
		(MailBox + num)->SmtpSSLInfo.Cert = profile_alloc_string(buf, TEXT("SmtpSSLCert"), TEXT(""));
		(MailBox + num)->SmtpSSLInfo.Pkey = profile_alloc_string(buf, TEXT("SmtpSSLPkey"), TEXT(""));
		(MailBox + num)->SmtpSSLInfo.Pass = profile_alloc_string(buf, TEXT("SmtpSSLPass"), TEXT(""));

		// Filter
		(MailBox + num)->FilterEnable = profile_get_int(buf, TEXT("FilterEnable"), 0);
		(MailBox + num)->FilterCnt = profile_get_int(buf, TEXT("FilterCnt"), 0);
		fDef = profile_get_int(buf, TEXT("FilterFlag"), -1);

		(MailBox + num)->tpFilter = (FILTER **)mem_calloc(sizeof(FILTER *) * (MailBox + num)->FilterCnt);
		if ((MailBox + num)->tpFilter == NULL) {
			(MailBox + num)->FilterCnt = 0;
		}
		for (t = 0; t < (MailBox + num)->FilterCnt; t++) {
			FILTER *tpFilter = *((MailBox + num)->tpFilter + t) = (FILTER *)mem_calloc(sizeof(FILTER));
			if (tpFilter == NULL) {
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			tpFilter->Enable = profile_get_int(buf, key_buf, 0);

			if (fDef == -1) {
				wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
				tpFilter->Action = profile_get_int(buf, key_buf, 0);
			} else {
				tpFilter->Action = fDef;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Priority"));
			tpFilter->Priority = profile_get_int(buf, key_buf, 0);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
			tpFilter->SaveboxName = profile_alloc_string(buf, key_buf, TEXT(""));
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("FwdAddress"));
			tpFilter->FwdAddress = profile_alloc_string(buf, key_buf, TEXT(""));

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			tpFilter->Header1 = profile_alloc_string(buf, key_buf, TEXT(""));

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			tpFilter->Content1 = profile_alloc_string(buf, key_buf, TEXT(""));

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
			tpFilter->Boolean = profile_get_int(buf, key_buf, 0);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			tpFilter->Header2 = profile_alloc_string(buf, key_buf, TEXT(""));

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			tpFilter->Content2 = profile_alloc_string(buf, key_buf, TEXT(""));
		}

		// RAS
		(MailBox + num)->RasMode = profile_get_int(buf, TEXT("RasMode"), 0);
		(MailBox + num)->RasEntry = profile_alloc_string(buf, TEXT("RasEntry"), TEXT(""));
		(MailBox + num)->RasReCon = profile_get_int(buf, TEXT("RasReCon"), 0);

		// Recv
		(MailBox + num)->UseGlobalRecv = profile_get_int(buf, TEXT("UseGlobalRecv"), 1);
		if ((MailBox + num)->UseGlobalRecv == 0) {
			(MailBox + num)->ShowHeader = profile_get_int(buf, TEXT("ShowHeader"), 0);
			(MailBox + num)->GetRecent = profile_get_int(buf, TEXT("GetRecent"), -20);
			(MailBox + num)->ListGetLine = profile_get_int(buf, TEXT("ListGetLine"), 50);
			(MailBox + num)->ListDownload = profile_get_int(buf, TEXT("ListDownload"), 0);
			(MailBox + num)->ListSaveMode = profile_get_int(buf, TEXT("ListSaveMode"), 2);
		} else {
			(MailBox + num)->ListSaveMode = op.ListSaveMode;
		}

		if (op.LazyLoadMailboxes == 0) {
			if ((MailBox+num)->Filename == NULL) {
				wsprintf(buf, TEXT("MailBox%d.dat"), i);
			} else {
				lstrcpy(buf, (MailBox+num)->Filename);
			}
			if (file_read_mailbox(buf, (MailBox + num), FALSE, FALSE) == FALSE) {
				profile_free();
				wsprintf(msg, STR_ERR_OPENMAILBOX, buf);
				ErrorMessage(hWnd, msg);
				return FALSE;
			}
		} else {
			if ((MailBox+num)->Filename == NULL) {
				wsprintf(buf, TEXT("%sMailBox%d.dat"), DataDir, i);
			} else {
				wsprintf(buf, TEXT("%s%s"), DataDir, (MailBox+num)->Filename);
			}
			(MailBox + num)->DiskSize = file_get_size(buf);
			(MailBox + num)->Loaded = FALSE;
		}
	}

	// convert old-style [SaveBox] to a new savebox-type account (GJC)
	wsprintf(buf, TEXT("MailBox%d.dat"), MailBoxCnt);
	if (ConvertFromNPOP && file_savebox_convert(buf)) {
		MessageBox(NULL, TEXT("Converting [Savebox]"), WINDOW_TITLE, MB_OK);
		num = mailbox_create(hWnd, 1, -1, FALSE, FALSE);
		if (num == -1) {
			profile_free();
			return FALSE;
		}
		(MailBox + num)->Name = alloc_copy_t(STR_SAVEBOX_NAME);
		ConvertName = (MailBox + num)->Name;
		(MailBox + num)->Type = MAILBOX_TYPE_SAVE;
		(MailBox + num)->CyclicFlag = 1;
		if (file_read_mailbox(buf, (MailBox + num), FALSE, FALSE) == FALSE) {
			profile_free();
			wsprintf(msg, STR_ERR_OPENMAILBOX, buf);
			ErrorMessage(hWnd, msg);
			return FALSE;
		}
	}

	// check that Saveboxes for filters are valid
	filter_sbox_check(hWnd, ConvertName);

	if (op.LazyLoadMailboxes != 0 && (op.AutoCheck == 1 || op.StartCheck == 1)) {
		// need to load accounts + related saveboxes
		BOOL do_saveboxes = (op.BlindAppend == 0) ? TRUE : FALSE;
		BOOL err = FALSE;
		for (i = MAILBOX_USER; i < MailBoxCnt; i++) {
			if ((MailBox+i)->Type != MAILBOX_TYPE_SAVE && (MailBox+i)->CyclicFlag == 0) {
				if (mailbox_load_now(hWnd, i, FALSE, do_saveboxes) != 1) {
					err = TRUE;
				}
			}
		}
		if (err == FALSE) {
			SaveBoxesLoaded = do_saveboxes; // may become false if filter is added
		}
	}

	profile_free();
	return TRUE;
}

/*
 * ini_write_general - write GENERAL settings
 */
void ini_write_general(void)
{
	TCHAR conv_buf[INI_BUF_SIZE];

	profile_write_int(GENERAL, TEXT("SocLog"), op.SocLog);

	profile_write_string(GENERAL, TEXT("FontName"), op.view_font.name);
	profile_write_int(GENERAL, TEXT("FontSize"), op.view_font.size);
	profile_write_int(GENERAL, TEXT("FontWeight"), op.view_font.weight);
	profile_write_int(GENERAL, TEXT("FontItalic"), op.view_font.italic);
	profile_write_int(GENERAL, TEXT("FontCharset"), op.view_font.charset);
	
	profile_write_string(GENERAL, TEXT("LvFontName"), op.lv_font.name);
	profile_write_int(GENERAL, TEXT("LvFontSize"), op.lv_font.size);
	profile_write_int(GENERAL, TEXT("LvFontWeight"), op.lv_font.weight);
	profile_write_int(GENERAL, TEXT("LvFontItalic"), op.lv_font.italic);
	profile_write_int(GENERAL, TEXT("LvFontCharset"), op.lv_font.charset);

	profile_write_int(GENERAL, TEXT("StatusBarCharWidth"), op.StatusBarCharWidth);

	// only used in beta versions of nPOPuk 2.17
	// profile_write_string(GENERAL, TEXT("Codepage"), op.Codepage);
	profile_write_string(GENERAL, TEXT("HeadCharset"), op.HeadCharset);
	profile_write_int(GENERAL, TEXT("HeadEncoding"), op.HeadEncoding);
	profile_write_string(GENERAL, TEXT("BodyCharset"), op.BodyCharset);
	profile_write_int(GENERAL, TEXT("BodyEncoding"), op.BodyEncoding);
	profile_write_string(GENERAL, TEXT("TimeZone"), op.TimeZone);
	profile_write_string(GENERAL, TEXT("DateFormat"), op.DateFormat);
	profile_write_string(GENERAL, TEXT("TimeFormat"), op.TimeFormat);

#ifndef _WIN32_WCE
	profile_write_int(GENERAL, TEXT("left"), op.MainRect.left);
	profile_write_int(GENERAL, TEXT("top"), op.MainRect.top);
	profile_write_int(GENERAL, TEXT("right"), op.MainRect.right);
	profile_write_int(GENERAL, TEXT("bottom"), op.MainRect.bottom);
#endif

	profile_write_int(GENERAL, TEXT("AddressLeft"), op.AddrRect.left);
	profile_write_int(GENERAL, TEXT("AddressTop"), op.AddrRect.top);
	profile_write_int(GENERAL, TEXT("AddressRight"), op.AddrRect.right);
	profile_write_int(GENERAL, TEXT("AddressBottom"), op.AddrRect.bottom);

	profile_write_int(GENERAL, TEXT("MblLeft"), op.MblRect.left);
	profile_write_int(GENERAL, TEXT("MblTop"), op.MblRect.top);
	profile_write_int(GENERAL, TEXT("MblRight"), op.MblRect.right);
	profile_write_int(GENERAL, TEXT("MblBottom"), op.MblRect.bottom);

	profile_write_int(GENERAL, TEXT("ShowTrayIcon"), op.ShowTrayIcon);
	profile_write_int(GENERAL, TEXT("StartHide"), op.StartHide);
	profile_write_int(GENERAL, TEXT("MinsizeHide"), op.MinsizeHide);
	profile_write_int(GENERAL, TEXT("CloseHide"), op.CloseHide);
	profile_write_int(GENERAL, TEXT("TrayIconToggle"), op.TrayIconToggle);
	profile_write_int(GENERAL, TEXT("StartInit"), op.StartInit);

	profile_write_int(GENERAL, TEXT("LvDefSelectPos"), op.LvDefSelectPos);
	profile_write_int(GENERAL, TEXT("LvAutoSort"), op.LvAutoSort);
	profile_write_int(GENERAL, TEXT("LvSortItem"), op.LvSortItem);
	profile_write_int(GENERAL, TEXT("LvSortFromAddressOnly"), op.LvSortFromAddressOnly);
	profile_write_int(GENERAL, TEXT("LvThreadView"), op.LvThreadView);
	profile_write_int(GENERAL, TEXT("LvStyle"), op.LvStyle);
	profile_write_int(GENERAL, TEXT("LvStyleEx"), op.LvStyleEx);
	profile_write_string(GENERAL, TEXT("LvColumnOrder"), op.LvColumnOrder);
	profile_write_int(GENERAL, TEXT("ContextMenuOption"), op.ContextMenuOption);
	profile_write_int(GENERAL, TEXT("MBMenuWidth"), op.MBMenuWidth);
	profile_write_int(GENERAL, TEXT("MBMenuMinWidth"), op.MBMenuMinWidth);
	profile_write_int(GENERAL, TEXT("PreviewPaneHeight"), op.PreviewPaneHeight);
	profile_write_int(GENERAL, TEXT("PreviewPaneMinHeight"), op.PreviewPaneMinHeight);
	EncodeCtrlChar(op.PreviewHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("PreviewHeader"), conv_buf);
	profile_write_int(GENERAL, TEXT("AutoPreview"), op.AutoPreview);
	profile_write_int(GENERAL, TEXT("PreviewedIsReadTime"), op.PreviewedIsReadTime);
	profile_write_int(GENERAL, TEXT("SaveboxListCount"), op.SaveboxListCount);
	profile_write_int(GENERAL, TEXT("ScanAllForUnread"), op.ScanAllForUnread);
	profile_write_int(GENERAL, TEXT("DelIsMarkDel"), op.DelIsMarkDel);
	profile_write_int(GENERAL, TEXT("DeletedIsNotNew"), op.DeletedIsNotNew);
	profile_write_int(GENERAL, TEXT("RecvScroll"), op.RecvScroll);
	profile_write_int(GENERAL, TEXT("SaveMsg"), op.SaveMsg);
	profile_write_int(GENERAL, TEXT("AutoSave"), op.AutoSave);
	profile_write_int(GENERAL, TEXT("WriteMbox"), op.WriteMbox);
	profile_write_int(GENERAL, TEXT("CheckQueuedOnExit"), op.CheckQueuedOnExit);
	profile_write_int(GENERAL, TEXT("PromptSaveOnExit"), op.PromptSaveOnExit);
	profile_write_int(GENERAL, TEXT("LazyLoadMailboxes"), op.LazyLoadMailboxes);
	profile_write_int(GENERAL, TEXT("BlindAppend"), op.BlindAppend);
	profile_write_int(GENERAL, TEXT("FixContentType"), op.FixContentType);
	profile_write_int(GENERAL, TEXT("StartPass"), op.StartPass);
	profile_write_int(GENERAL, TEXT("ShowPass"), op.ShowPass);
	profile_write_int(GENERAL, TEXT("ConfigPass"), op.ConfigPass);
	profile_write_int(GENERAL, TEXT("ScrambleMailboxes"), op.ScrambleMailboxes);

	profile_write_int(GENERAL, TEXT("LvColSize-0"), op.LvColSize[0]);
	profile_write_int(GENERAL, TEXT("LvColSize-1"), op.LvColSize[1]);
	profile_write_int(GENERAL, TEXT("LvColSize-2"), op.LvColSize[2]);
	profile_write_int(GENERAL, TEXT("LvColSize-3"), op.LvColSize[3]);

	profile_write_int(GENERAL, TEXT("AddColSize-0"), op.AddColSize[0]);
	profile_write_int(GENERAL, TEXT("AddColSize-1"), op.AddColSize[1]);
	profile_write_int(GENERAL, TEXT("AddColSize-2"), op.AddColSize[2]);
	profile_write_int(GENERAL, TEXT("AddressSort"), op.AddressSort);
	profile_write_int(GENERAL, TEXT("AddressJumpKey"), op.AddressJumpKey);
	profile_write_int(GENERAL, TEXT("AutoAddRecipients"), op.AutoAddRecipients);
	profile_write_string(GENERAL, TEXT("AddressShowGroup"), op.AddressShowGroup);

	profile_write_int(GENERAL, TEXT("MblColSize-0"), op.MblColSize[0]);
	profile_write_int(GENERAL, TEXT("MblColSize-1"), op.MblColSize[1]);
	profile_write_int(GENERAL, TEXT("MblColSize-2"), op.MblColSize[2]);
	profile_write_int(GENERAL, TEXT("MblColSize-3"), op.MblColSize[3]);
	profile_write_int(GENERAL, TEXT("MblColSize-4"), op.MblColSize[4]);
	profile_write_int(GENERAL, TEXT("MblColSize-5"), op.MblColSize[5]);

#ifndef _WIN32_WCE
	profile_write_int(GENERAL, TEXT("viewleft"), op.ViewRect.left);
	profile_write_int(GENERAL, TEXT("viewtop"), op.ViewRect.top);
	profile_write_int(GENERAL, TEXT("viewright"), op.ViewRect.right);
	profile_write_int(GENERAL, TEXT("viewbottom"), op.ViewRect.bottom);

	profile_write_int(GENERAL, TEXT("editleft"), op.EditRect.left);
	profile_write_int(GENERAL, TEXT("edittop"), op.EditRect.top);
	profile_write_int(GENERAL, TEXT("editright"), op.EditRect.right);
	profile_write_int(GENERAL, TEXT("editbottom"), op.EditRect.bottom);
#endif

	profile_write_int(GENERAL, TEXT("ShowHeader"), op.ShowHeader);
	profile_write_int(GENERAL, TEXT("GetRecent"), op.GetRecent);
	profile_write_int(GENERAL, TEXT("ListGetLine"), op.ListGetLine);
	profile_write_int(GENERAL, TEXT("ListDownload"), op.ListDownload);
	profile_write_int(GENERAL, TEXT("ListSaveMode"), op.ListSaveMode);
	profile_write_int(GENERAL, TEXT("WordBreakFlag"), op.WordBreakFlag);
	profile_write_int(GENERAL, TEXT("EditWordBreakFlag"), op.EditWordBreakFlag);
#ifdef _WIN32_WCE_PPC
	profile_write_int(GENERAL, TEXT("OptionalScrollbar"), op.OptionalScrollbar);
#endif
	profile_write_int(GENERAL, TEXT("ViewShowDate"), op.ViewShowDate);
	profile_write_int(GENERAL, TEXT("MatchCase"), op.MatchCase);
	profile_write_int(GENERAL, TEXT("Wildcards"), op.Wildcards);
	profile_write_int(GENERAL, TEXT("AllMsgFind"), op.AllMsgFind);
	profile_write_int(GENERAL, TEXT("AllBoxFind"), op.AllBoxFind);
	profile_write_int(GENERAL, TEXT("SubjectFind"), op.SubjectFind);

	profile_write_int(GENERAL, TEXT("ESMTP"), op.ESMTP);
	profile_write_string(GENERAL, TEXT("SendHelo"), op.SendHelo);
	profile_write_int(GENERAL, TEXT("SendMessageId"), op.SendMessageId);
	profile_write_int(GENERAL, TEXT("SendDate"), op.SendDate);
	profile_write_int(GENERAL, TEXT("SelectSendBox"), op.SelectSendBox);
	profile_write_int(GENERAL, TEXT("DisableWarning"), op.ExpertMode);	// Added PHH 4-Oct-2003
	profile_write_int(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), op.PopBeforeSmtpIsLoginOnly);
	profile_write_int(GENERAL, TEXT("PopBeforeSmtpWait"), op.PopBeforeSmtpWait);
	profile_write_int(GENERAL, TEXT("NoEmptyMailbox"), op.NoEmptyMailbox);

	profile_write_int(GENERAL, TEXT("AutoQuotation"), op.AutoQuotation);
	profile_write_int(GENERAL, TEXT("FwdQuotation"), op.FwdQuotation);
	profile_write_int(GENERAL, TEXT("SignForward"), op.SignForward);
	profile_write_int(GENERAL, TEXT("SignReplyAbove"), op.SignReplyAbove);
	profile_write_string(GENERAL, TEXT("QuotationChar"), op.QuotationChar);
	profile_write_int(GENERAL, TEXT("WordBreakSize"), op.WordBreakSize);
	profile_write_int(GENERAL, TEXT("QuotationBreak"), op.QuotationBreak);
	profile_write_string(GENERAL, TEXT("ReSubject"), op.ReSubject);
	profile_write_string(GENERAL, TEXT("FwdSubject"), op.FwdSubject);		// Added PHH 4-10-2003
	profile_write_string(GENERAL, TEXT("ReFwdPrefixes"), op.ReFwdPrefixes);
	EncodeCtrlChar(op.ReHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("ReHeader"), conv_buf);
	EncodeCtrlChar(op.FwdHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("FwdHeader"), conv_buf);
	profile_write_string(GENERAL, TEXT("ReplyTo"), op.AltReplyTo);

	profile_write_string(GENERAL, TEXT("Bura"), op.Bura);
	profile_write_string(GENERAL, TEXT("Oida"), op.Oida);

	profile_write_string(GENERAL, TEXT("CAFile"), op.CAFile);

	profile_write_int(GENERAL, TEXT("IPCache"), op.IPCache);
	profile_write_int(GENERAL, TEXT("EncodeType"), op.EncodeType);

	profile_write_int(GENERAL, TEXT("ShowNewMailMessage"), op.ShowNewMailMessage);
	profile_write_int(GENERAL, TEXT("ShowNoMailMessage"), op.ShowNoMailMessage);
	profile_write_int(GENERAL, TEXT("ActiveNewMailMessage"), op.ActiveNewMailMessage);
	profile_write_int(GENERAL, TEXT("ClearNewOverlay"), op.ClearNewOverlay);

#ifdef _WIN32_WCE
	///////////// MRP /////////////////////
	profile_write_int(GENERAL, TEXT("UsePOOMAddressBook"), op.UsePOOMAddressBook);
	///////////// --- /////////////////////
	profile_write_int(GENERAL, TEXT("POOMNameIsComment"), op.POOMNameIsComment);
	profile_write_int(GENERAL, TEXT("LoadPOOMAtStart"), op.LoadPOOMAtStart);
#endif
#ifdef _WIN32_WCE_PPC
	profile_write_int(GENERAL, TEXT("UseWindowsSSL"), op.UseWindowsSSL);
	profile_write_int(GENERAL, TEXT("ShowNavButtons"), op.ShowNavButtons);
	profile_write_int(GENERAL, TEXT("PromptIniEdit"), op.PromptIniEdit);
#endif

	profile_write_int(GENERAL, TEXT("NewMailSound"), op.NewMailSound);
	profile_write_string(GENERAL, TEXT("NewMailSoundFile"), op.NewMailSoundFile);
	profile_write_int(GENERAL, TEXT("ExecEndSound"), op.ExecEndSound);
	profile_write_string(GENERAL, TEXT("ExecEndSoundFile"), op.ExecEndSoundFile);
	profile_write_int(GENERAL, TEXT("ItemPlaySound"), op.ItemPlaySound);

	profile_write_int(GENERAL, TEXT("AutoCheck"), op.AutoCheck);
	profile_write_int(GENERAL, TEXT("AutoCheckTime"), op.AutoCheckTime);
	profile_write_int(GENERAL, TEXT("StartCheck"), op.StartCheck);
	profile_write_int(GENERAL, TEXT("CheckAfterUpdate"), op.CheckAfterUpdate);
	profile_write_int(GENERAL, TEXT("SocIgnoreError"), op.SocIgnoreError);
	profile_write_int(GENERAL, TEXT("SendIgnoreError"), op.SendIgnoreError);
	profile_write_int(GENERAL, TEXT("NoIgnoreErrorTimeout"), op.NoIgnoreErrorTimeout);
	profile_write_int(GENERAL, TEXT("SendAttachIndividually"), op.SendAttachIndividually);
	profile_write_int(GENERAL, TEXT("CheckEndExec"), op.CheckEndExec);
	profile_write_int(GENERAL, TEXT("CheckEndExecNoDelMsg"), op.CheckEndExecNoDelMsg);
	profile_write_int(GENERAL, TEXT("DeletePerUpdateLimit"), op.DeletePerUpdateLimit);
	profile_write_int(GENERAL, TEXT("TimeoutInterval"), op.TimeoutInterval);
	profile_write_int(GENERAL, TEXT("ReorderSendbox"), op.ReorderSendbox);

	profile_write_int(GENERAL, TEXT("ViewClose"), op.ViewClose);
	profile_write_int(GENERAL, TEXT("ViewCloseNoNext"), op.ViewCloseNoNext);
	profile_write_int(GENERAL, TEXT("ViewNextAfterDel"), op.ViewNextAfterDel);
	profile_write_int(GENERAL, TEXT("ViewSkipDeleted"), op.ViewSkipDeleted);
	profile_write_string(GENERAL, TEXT("ViewApp"), op.ViewApp);
	profile_write_string(GENERAL, TEXT("ViewAppCmdLine"), op.ViewAppCmdLine);
	profile_write_string(GENERAL, TEXT("ViewFileSuffix"), op.ViewFileSuffix);
	EncodeCtrlChar(op.ViewFileHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("ViewFileHeader"), conv_buf);
	profile_write_int(GENERAL, TEXT("ViewAppClose"), op.ViewAppClose);
	profile_write_int(GENERAL, TEXT("DefViewApp"), op.DefViewApp);
	profile_write_int(GENERAL, TEXT("ViewAppMsgSource"), op.ViewAppMsgSource);
	profile_write_int(GENERAL, TEXT("AutoOpenAttachMsg"), op.AutoOpenAttachMsg);
	profile_write_int(GENERAL, TEXT("ViewWindowCursor"), op.ViewWindowCursor);
	profile_write_int(GENERAL, TEXT("ViewShowAttach"), op.ViewShowAttach);
	profile_write_string(GENERAL, TEXT("EditApp"), op.EditApp);
	profile_write_string(GENERAL, TEXT("EditAppCmdLine"), op.EditAppCmdLine);
	profile_write_string(GENERAL, TEXT("EditFileSuffix"), op.EditFileSuffix);
	profile_write_int(GENERAL, TEXT("DefEditApp"), op.DefEditApp);
	profile_write_string(GENERAL, TEXT("AttachPath"), op.AttachPath);
	//profile_write_int(GENERAL, TEXT("AttachWarning"), op.AttachWarning);
	profile_write_int(GENERAL, TEXT("AttachDelete"), op.AttachDelete);
	profile_write_int(GENERAL, TEXT("StripHtmlTags"), op.StripHtmlTags);
	profile_write_string(GENERAL, TEXT("SavedOpenDir"), op.SavedOpenDir);
	profile_write_string(GENERAL, TEXT("SavedSaveDir"), op.SavedSaveDir);

	profile_write_string(GENERAL, TEXT("URLApp"), op.URLApp);

	profile_write_int(GENERAL, TEXT("EnableLAN"), op.EnableLAN);
}

/*
 * ini_save_setting - INIファイルへ設定情報を書き出す
 */
BOOL ini_save_setting(HWND hWnd, BOOL SaveMailFlag, BOOL SaveAll, TCHAR *SaveDir)
{
	TCHAR app_path[BUF_SIZE];
	///////////// MRP /////////////////////
	TCHAR app_pathBackup[BUF_SIZE];
	///////////// --- /////////////////////
	TCHAR buf[BUF_SIZE];
	TCHAR key_buf[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
#ifdef UNICODE
	TCHAR ret[BUF_SIZE];
#endif
	int j, t;
	BOOL rc = TRUE;
	BOOL found;
	BOOL is_backup = FALSE;

	if (SaveDir == NULL) {
		if (IniFile == NULL) {
			str_join_t(app_path, DefaultDataDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
		} else {
			str_cpy_n_t(app_path, IniFile, BUF_SIZE);
		}
	} else {
		is_backup = TRUE;
		if (IniFile == NULL) {
			str_join_t(app_path, SaveDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
		} else {
			TCHAR *p, *q;
			for (p = q = IniFile; *p != TEXT('\0'); p++) {
#ifndef UNICODE
				if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p+1) != TEXT('\0')) {
					p++;
					continue;
				}
#endif
				if (*p == TEXT('\\') || *p == TEXT('/')) {
					q = p+1;
				}
			}
			wsprintf(app_path, TEXT("%s%s"), SaveDir, q);
		}
	}

	///////////// MRP /////////////////////
#ifdef UNICODE
	wcscpy(app_pathBackup, app_path);
	wcscat(app_pathBackup, TEXT(".bak"));
#else
	strcpy_s(app_pathBackup, BUF_SIZE-5, app_path);
	strcat_s(app_pathBackup, BUF_SIZE, TEXT(".bak"));
#endif
	CopyFile(app_path, app_pathBackup, FALSE); // Create the backup file.
	///////////// --- /////////////////////

	// if IniFile != NULL or SaveDir != NULL, this initializes from
	// the previous backup, not from the ini file in use!
	profile_initialize(app_path, FALSE);

	if (is_backup == TRUE) {
		profile_write_string(GENERAL, TEXT("DataFileDir"), TEXT(""));
	} else {
		profile_write_string(GENERAL, TEXT("DataFileDir"), op.DataFileDir);
	}
	profile_write_string(GENERAL, TEXT("BackupDir"), op.BackupDir);
	profile_write_int(GENERAL, TEXT("Version"), op.Version);

	ini_write_general();

	EncodePassword(TEXT("_pw_"), op.Password, tmp, BUF_SIZE - 1, FALSE);
	profile_write_string(GENERAL, TEXT("pw"), tmp);

	profile_write_int(GENERAL, TEXT("WifiCon"), op.WifiCon);
	profile_write_int(GENERAL, TEXT("WifiCheckEndDisCon"), op.WifiCheckEndDisCon);
	profile_write_int(GENERAL, TEXT("WifiCheckEndDisConTimeout"), op.WifiCheckEndDisConTimeout);
	profile_write_int(GENERAL, TEXT("WifiExitDisCon"), op.WifiExitDisCon);
	profile_write_int(GENERAL, TEXT("WifiNoCheck"), op.WifiNoCheck);
	profile_write_int(GENERAL, TEXT("WifiWaitSec"), op.WifiWaitSec);
	profile_write_string(GENERAL, TEXT("WifiDeviceName"), op.WifiDeviceName);

	profile_write_int(GENERAL, TEXT("RasCon"), op.RasCon);
	profile_write_int(GENERAL, TEXT("RasCheckEndDisCon"), op.RasCheckEndDisCon);
	profile_write_int(GENERAL, TEXT("RasCheckEndDisConTimeout"), op.RasCheckEndDisConTimeout);
	profile_write_int(GENERAL, TEXT("RasEndDisCon"), op.RasEndDisCon);
	profile_write_int(GENERAL, TEXT("RasNoCheck"), op.RasNoCheck);
	profile_write_int(GENERAL, TEXT("RasWaitSec"), op.RasWaitSec);

	// GJC delete obsolete entries
	profile_delete_key(GENERAL, TEXT("StertPass"));
	profile_delete_key(GENERAL, TEXT("MstchCase"));
	profile_delete_key(GENERAL, TEXT("AllFind"));
	profile_delete_key(GENERAL, TEXT("ShowNewMailMessgae"));
	profile_delete_key(GENERAL, TEXT("ActiveNewMailMessgae"));
	profile_delete_key(GENERAL, TEXT("sOida"));
	profile_delete_key(GENERAL, TEXT("sBura"));
	profile_delete_key(GENERAL, TEXT("AutoMarkSend"));
	profile_delete_key(GENERAL, TEXT("MoveAllMailBox"));
	profile_delete_key(GENERAL, TEXT("ViewAppFullHeaders"));
	profile_delete_key(GENERAL, TEXT("OpenSaveDir"));
	profile_delete_key(GENERAL, TEXT("RememberOpenSaveDir"));
	profile_delete_key(GENERAL, TEXT("DecodeInPlace"));
	profile_delete_key(GENERAL, TEXT("UseBuiltinSSL"));

	for (t = 0, j = 0; j < op.RasInfoCnt; j++) {
		if (*(op.RasInfo + j) == NULL ||
			(*(op.RasInfo + j))->RasEntry == NULL || *(*(op.RasInfo + j))->RasEntry == TEXT('\0')) {
			continue;
		}
		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasEntry"));
		profile_write_string(TEXT("RASINFO"), key_buf, (*(op.RasInfo + j))->RasEntry);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasUser"));
		profile_write_string(TEXT("RASINFO"), key_buf, (*(op.RasInfo + j))->RasUser);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasPass"));
		EncodePassword((*(op.RasInfo + j))->RasUser, (*(op.RasInfo + j))->RasPass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(TEXT("RASINFO"), key_buf, tmp);
		t++;
	}
	profile_write_int(GENERAL, TEXT("RasInfoCnt"), t);

	profile_write_int(GENERAL, TEXT("GlobalFilterEnable"), op.GlobalFilterEnable);
	profile_write_int(GENERAL, TEXT("GlobalFilterCnt"), op.GlobalFilterCnt);

	for (t = 0; op.tpFilter != NULL && t < op.GlobalFilterCnt; t++) {
		FILTER *tpFilter = *(op.tpFilter + t);
		if (tpFilter == NULL) {
			continue;
		}

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
		profile_write_int(TEXT("FILTER"), key_buf, tpFilter->Enable);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
		profile_write_int(TEXT("FILTER"), key_buf, tpFilter->Action);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Priority"));
		profile_write_int(TEXT("FILTER"), key_buf, tpFilter->Priority);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
		profile_write_string(TEXT("FILTER"), key_buf, tpFilter->SaveboxName);
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("FwdAddress"));
		profile_write_string(TEXT("FILTER"), key_buf, tpFilter->FwdAddress);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
		profile_write_string(TEXT("FILTER"), key_buf, tpFilter->Header1);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
		profile_write_string(TEXT("FILTER"), key_buf, tpFilter->Content1);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
		profile_write_int(TEXT("FILTER"), key_buf, tpFilter->Boolean);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
		profile_write_string(TEXT("FILTER"), key_buf, tpFilter->Header2);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
		profile_write_string(TEXT("FILTER"), key_buf, tpFilter->Content2);
	}

	// GJC clear any further filter keys
	found = TRUE;
	while (found) {
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
		found &= profile_delete_key(GENERAL, key_buf);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
		found &= profile_delete_key(GENERAL, key_buf);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
		found &= profile_delete_key(GENERAL, key_buf);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
		found &= profile_delete_key(GENERAL, key_buf);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
		found &= profile_delete_key(GENERAL, key_buf);

		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
		found &= profile_delete_key(GENERAL, key_buf);

		// may not exist in old versions, so don't &=
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
		profile_delete_key(GENERAL, key_buf);
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Priority"));
		profile_delete_key(GENERAL, key_buf);
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("FwdAddress"));
		profile_delete_key(GENERAL, key_buf);
		wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
		profile_delete_key(GENERAL, key_buf);

		t++;
	}

	// Retention
	profile_write_int(GENERAL, TEXT("MailBoxCnt"), MailBoxCnt - MAILBOX_USER);

	for (j = MAILBOX_USER; j < MailBoxCnt; j++) {
		if ((MailBox + j) == NULL) {
			continue;
		}
		wsprintf(buf, TEXT("MAILBOX-%d"), j - MAILBOX_USER);
		if ((MailBox + j)->Type == MAILBOX_TYPE_SAVE) {
			// GJC - SaveBox type (not an account)
			profile_clear_section(buf);
		}

		// Name
		profile_write_string(buf, TEXT("Name"), (MailBox + j)->Name);
		// Filename
		profile_write_string(buf, TEXT("Filename"), (MailBox + j)->Filename);
		// NewMailSoundFile
		profile_write_string(buf, TEXT("NewMailSoundFile"), (MailBox + j)->NewMailSoundFile);
		// Type
		profile_write_int(buf, TEXT("Type"), (MailBox + j)->Type);
		if ((MailBox + j)->Type == MAILBOX_TYPE_SAVE) {
			// Default account
			profile_write_string(buf, TEXT("DefAccount"), (MailBox + j)->DefAccount);
			// no other settings for SaveBox-type mailboxes
			continue;
		} else {
			profile_delete_key(buf, TEXT("DefAccount"));
		}

		// Server
		profile_write_string(buf, TEXT("Server"), (MailBox + j)->Server);
		// Port
		profile_write_int(buf, TEXT("Port"), (MailBox + j)->Port);
		// User
		profile_write_string(buf, TEXT("User"), (MailBox + j)->User);
		// Pass
		EncodePassword((MailBox + j)->User, (MailBox + j)->Pass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(buf, TEXT("Pass"), tmp);
		// APOP
		profile_write_int(buf, TEXT("APOP"), (MailBox + j)->APOP);
		// POP SSL
		profile_write_int(buf, TEXT("PopSSL"), (MailBox + j)->PopSSL);
		// POP SSL Option
		profile_write_int(buf, TEXT("PopSSLType"), (MailBox + j)->PopSSLInfo.Type);
		profile_write_int(buf, TEXT("PopSSLVerify"), (MailBox + j)->PopSSLInfo.Verify);
		profile_write_int(buf, TEXT("PopSSLDepth"), (MailBox + j)->PopSSLInfo.Depth);
		profile_write_string(buf, TEXT("PopSSLCert"), (MailBox + j)->PopSSLInfo.Cert);
		profile_write_string(buf, TEXT("PopSSLPkey"), (MailBox + j)->PopSSLInfo.Pkey);
		profile_write_string(buf, TEXT("PopSSLPass"), (MailBox + j)->PopSSLInfo.Pass);
		// Disable RETR
		profile_write_int(buf, TEXT("NoRETR"), (MailBox + j)->NoRETR);
		// Disable UIDL
		profile_write_int(buf, TEXT("NoUIDL"), (MailBox + j)->NoUIDL);

		profile_write_int(buf, TEXT("MessageSizeDelta"), (MailBox + j)->MessageSizeDelta);

		// MailCnt
		profile_write_int(buf, TEXT("MailCnt"), (MailBox + j)->MailCnt);
		// MailSize
		profile_write_int(buf, TEXT("MailSize"), (MailBox + j)->MailSize);

		profile_write_int(buf, TEXT("StartInit"), (MailBox + j)->StartInit);

		// LastMessageId
		if ((MailBox + j)->LastMessageId != NULL) {
#ifdef UNICODE
			char_to_tchar((MailBox + j)->LastMessageId, ret, BUF_SIZE - 1);
			*(ret + BUF_SIZE - 1) = TEXT('\0');
			profile_write_string(buf, TEXT("LastMessageId"), ret);
#else
			profile_write_string(buf, TEXT("LastMessageId"), (MailBox + j)->LastMessageId);
#endif
		} else {
			profile_delete_key(buf, TEXT("LastMessageId"));
		}
		// LastNo
		profile_write_int(buf, TEXT("LastNo"), (MailBox + j)->LastNo);

		// CyclicFlag
		profile_write_int(buf, TEXT("CyclicFlag"), (MailBox + j)->CyclicFlag);

		// SmtpServer
		profile_write_string(buf, TEXT("SmtpServer"), (MailBox + j)->SmtpServer);
		// SmtpPort
		profile_write_int(buf, TEXT("SmtpPort"), (MailBox + j)->SmtpPort);
		// UserName
		profile_write_string(buf, TEXT("UserName"), (MailBox + j)->UserName);
		// MailAddress
		profile_write_string(buf, TEXT("MailAddress"), (MailBox + j)->MailAddress);
		// Signature
		if ((MailBox + j)->Signature != NULL) {
			TCHAR *p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen((MailBox + j)->Signature) * 2 + 1));
			if (p != NULL) {
				EncodeCtrlChar((MailBox + j)->Signature, p);
				profile_write_string(buf, TEXT("Signature"), p);
				mem_free(&p);
			}
		}
		// ReplyTo
		profile_write_string(buf, TEXT("ReplyTo"), (MailBox + j)->ReplyTo);
		// UseReplyToForFrom
		if ((MailBox + j)->UseReplyToForFrom != 0) {
			profile_write_int(buf, TEXT("UseReplyToForFrom"), (MailBox + j)->UseReplyToForFrom);
		}
		// SendWireForm
		profile_write_int(buf, TEXT("SendWireForm"), (MailBox + j)->SendWireForm);
		// MyAddr2Bcc
		profile_write_int(buf, TEXT("MyAddr2Bcc"), (MailBox + j)->MyAddr2Bcc);
		// BccAddr
		profile_write_string(buf, TEXT("BccAddr"), (MailBox + j)->BccAddr);

		// POP before SMTP
		profile_write_int(buf, TEXT("PopBeforeSmtp"), (MailBox + j)->PopBeforeSmtp);
		// SMTP Authentication
		profile_write_int(buf, TEXT("SmtpAuth"), (MailBox + j)->SmtpAuth);
		// SMTP Authentication type
		profile_write_int(buf, TEXT("SmtpAuthType"), (MailBox + j)->SmtpAuthType);
		// SMTP Authentication User & Pass mode
		profile_write_int(buf, TEXT("AuthUserPass"), (MailBox + j)->AuthUserPass);
		// SMTP Authentication User
		profile_write_string(buf, TEXT("SmtpUser"), (MailBox + j)->SmtpUser);
		// SMTP Authentication Pass
		EncodePassword((MailBox + j)->SmtpUser, (MailBox + j)->SmtpPass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(buf, TEXT("SmtpPass"), tmp);
		// SMTP SSL
		profile_write_int(buf, TEXT("SmtpSSL"), (MailBox + j)->SmtpSSL);
		// SMTP SSL Option
		profile_write_int(buf, TEXT("SmtpSSLType"), (MailBox + j)->SmtpSSLInfo.Type);
		profile_write_int(buf, TEXT("SmtpSSLVerify"), (MailBox + j)->SmtpSSLInfo.Verify);
		profile_write_int(buf, TEXT("SmtpSSLDepth"), (MailBox + j)->SmtpSSLInfo.Depth);
		profile_write_string(buf, TEXT("SmtpSSLCert"), (MailBox + j)->SmtpSSLInfo.Cert);
		profile_write_string(buf, TEXT("SmtpSSLPkey"), (MailBox + j)->SmtpSSLInfo.Pkey);
		profile_write_string(buf, TEXT("SmtpSSLPass"), (MailBox + j)->SmtpSSLInfo.Pass);

		// Filter
		profile_write_int(buf, TEXT("FilterEnable"), (MailBox + j)->FilterEnable);
		profile_write_int(buf, TEXT("FilterCnt"), (MailBox + j)->FilterCnt);

		for (t = 0; (MailBox + j)->tpFilter != NULL && t < (MailBox + j)->FilterCnt; t++) {
			FILTER *tpFilter = *((MailBox + j)->tpFilter + t);
			if (tpFilter == NULL) {
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			profile_write_int(buf, key_buf, tpFilter->Enable);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
			profile_write_int(buf, key_buf, tpFilter->Action);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Priority"));
			profile_write_int(buf, key_buf, tpFilter->Priority);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
			profile_write_string(buf, key_buf, tpFilter->SaveboxName);
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("FwdAddress"));
			profile_write_string(buf, key_buf, tpFilter->FwdAddress);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			profile_write_string(buf, key_buf, tpFilter->Header1);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			profile_write_string(buf, key_buf, tpFilter->Content1);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
			profile_write_int(buf, key_buf, tpFilter->Boolean);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			profile_write_string(buf, key_buf, tpFilter->Header2);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			profile_write_string(buf, key_buf, tpFilter->Content2);
		}

		// GJC clear any further filter keys
		found = TRUE;
		while (found) {
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			found &= profile_delete_key(buf, key_buf);

			// may not exist in old versions, so don't &=
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
			profile_delete_key(buf, key_buf);
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Priority"));
			profile_delete_key(buf, key_buf);
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("FwdAddress"));
			profile_delete_key(buf, key_buf);
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
			profile_delete_key(buf, key_buf);

			t++;
		}

		// RAS
		profile_write_int(buf, TEXT("RasMode"), (MailBox + j)->RasMode);
		profile_write_string(buf, TEXT("RasEntry"), (MailBox + j)->RasEntry);
		profile_write_int(buf, TEXT("RasReCon"), (MailBox + j)->RasReCon);

		// Recv
		profile_write_int(buf, TEXT("UseGlobalRecv"), (MailBox + j)->UseGlobalRecv);
		if ((MailBox + j)->UseGlobalRecv) {
			profile_delete_key(buf, TEXT("ShowHeader"));
			profile_delete_key(buf, TEXT("GetRecent"));
			profile_delete_key(buf, TEXT("ListGetLine"));
			profile_delete_key(buf, TEXT("ListDownload"));
			profile_delete_key(buf, TEXT("ListSaveMode"));
		} else {
			profile_write_int(buf, TEXT("ShowHeader"), (MailBox + j)->ShowHeader);
			profile_write_int(buf, TEXT("GetRecent"), (MailBox + j)->GetRecent);
			profile_write_int(buf, TEXT("ListGetLine"), (MailBox + j)->ListGetLine);
			profile_write_int(buf, TEXT("ListDownload"), (MailBox + j)->ListDownload);
			profile_write_int(buf, TEXT("ListSaveMode"), (MailBox + j)->ListSaveMode);
		}

	}
	// GJC clear keys from deleted mailboxes
	found = TRUE;
	j = MailBoxCnt;
	while (found) {
		wsprintf(buf, TEXT("MAILBOX-%d"), j - MAILBOX_USER);
		found = profile_clear_section(buf);
		j++;
	}

	if (profile_flush(app_path, NULL) == FALSE) {
		rc = FALSE;
	}
	profile_free();	 

	///////////// MRP /////////////////////
	DeleteFile(app_pathBackup);
	///////////// --- /////////////////////

	if (SaveMailFlag == FALSE) {
		// 設定保存のみ
		return rc;
	}

	if (is_backup) {
		// GJC need to load all mailboxes
		for (j = 0; j < MailBoxCnt; j++) {
			(MailBox+j)->NeedsSave = MAILITEMS_CHANGED;
			if ((MailBox+j)->Loaded == FALSE) {
				mailbox_load_now(hWnd, j, FALSE, FALSE);
				// ignore return value; if it fails to load, it just won't be backed up
			}
		}
	} else {
		SaveDir = DataDir;
	}

	//Retention
	for (j = MAILBOX_USER; j < MailBoxCnt; j++) {
		MAILBOX *tpMailBox = MailBox + j;
		if (tpMailBox == NULL || tpMailBox->Loaded == FALSE) {
			continue;
		}
		if (op.WriteMbox != tpMailBox->WasMbox) {
			tpMailBox->NeedsSave |= MBOX_FORMAT_CHANGED;
		}
		if (SaveAll == FALSE &&	tpMailBox->NeedsSave == 0) {
			continue;
		}

		//of mail inside mailbox Mail item
		if (tpMailBox->Filename == NULL) {
			wsprintf(buf, TEXT("MailBox%d.dat"), j - MAILBOX_USER);
		} else {
			lstrcpy(buf, tpMailBox->Filename);
		}
		if (file_save_mailbox(buf, SaveDir, j, is_backup, FALSE,
			(tpMailBox->Type == MAILBOX_TYPE_SAVE) ? 2 : tpMailBox->ListSaveMode) == FALSE) {
			rc = FALSE;
			if (op.SocLog > 1) {
				TCHAR msg[MSG_SIZE];
				wsprintf(msg, TEXT("Error saving %s\r\n"), buf);
				log_save(msg);
			}
		}
	}
	return rc;
}

/*
 * ini_check_window_pos - check window isn't outside current screen (GJC)
 */
static void ini_check_window_pos(RECT *the_rect, RECT *scr_rect, int def_w, int def_l)
{
	int minwl = 10;

	if (the_rect->left < scr_rect->left) {
		the_rect->right += (scr_rect->left - the_rect->left);
		the_rect->left = scr_rect->left;
	} else if (the_rect->left > scr_rect->right) {
		the_rect->right -= the_rect->left;
		the_rect->left = 0;
	}
	if (the_rect->right < the_rect->left + minwl) {
		the_rect->right = the_rect->left + def_w;
	}
	if (the_rect->right > scr_rect->right) {
		the_rect->left += (scr_rect->right - the_rect->right);
		if (the_rect->left < scr_rect->left) {
			the_rect->left = scr_rect->left;
		}
		the_rect->right = scr_rect->right;
	}

	if (the_rect->top < scr_rect->top) {
		the_rect->bottom += (scr_rect->top - the_rect->top);
		the_rect->top = scr_rect->top;
	} else if (the_rect->top > scr_rect->bottom) {
		the_rect->bottom -= the_rect->top;
		the_rect->top = 0;
	}
	if (the_rect->bottom < the_rect->top + minwl) {
		the_rect->bottom = the_rect->top + def_l;
	}
	if (the_rect->bottom > scr_rect->bottom) {
		the_rect->top += (scr_rect->bottom - the_rect->bottom);
		if (the_rect->top < scr_rect->top) {
			the_rect->top = scr_rect->top;
		}
		the_rect->bottom = scr_rect->bottom;
	}
}

/*
 * ini_free - 設定情報を解放する
 */
void ini_free(BOOL free_all)
{
	if (free_all) {
		mem_free(&op.BackupDir);
		mem_free(&op.Password);
	}
	mem_free(&op.LvColumnOrder);
	mem_free(&op.PreviewHeader);
	mem_free(&op.AddressShowGroup);
	mem_free(&op.view_font.name);
	mem_free(&op.lv_font.name);
	mem_free(&op.SendHelo);
	mem_free(&op.CAFile);
	mem_free(&op.QuotationChar);
	mem_free(&op.ReSubject);
	mem_free(&op.FwdSubject);
	mem_free(&op.ReFwdPrefixes);
	mem_free(&op.ReHeader);
	mem_free(&op.FwdHeader);
	mem_free(&op.AltReplyTo);
	mem_free(&op.Bura);
	mem_free(&op.Oida);
	mem_free(&op.Codepage);
	mem_free(&op.HeadCharset);
	mem_free(&op.BodyCharset);
	mem_free(&op.TimeZone);
	mem_free(&op.DateFormat);
	mem_free(&op.TimeFormat);
	mem_free(&op.NewMailSoundFile);
	mem_free(&op.ExecEndSoundFile);
	mem_free(&op.ItemNewSoundFile);
	mem_free(&op.ItemPartialSoundFile);
	mem_free(&op.ItemFullSoundFile);
	mem_free(&op.ItemAttachSoundFile);
	mem_free(&op.ItemHtmlSoundFile);
	mem_free(&op.ItemNonSoundFile);
	mem_free(&op.ItemUnreadSoundFile);
	mem_free(&op.ItemReadSoundFile);
	mem_free(&op.ItemDownSoundFile);
	mem_free(&op.ItemDelSoundFile);
	mem_free(&op.ItemSendSoundFile);
	mem_free(&op.ItemSentSoundFile);
	mem_free(&op.ItemErrorSoundFile);
	mem_free(&op.ItemFlagSoundFile);
	mem_free(&op.ViewApp);
	mem_free(&op.ViewAppCmdLine);
	mem_free(&op.ViewFileSuffix);
	mem_free(&op.ViewFileHeader);
	mem_free(&op.EditApp);
	mem_free(&op.EditAppCmdLine);
	mem_free(&op.EditFileSuffix);
	mem_free(&op.SavedOpenDir);
	mem_free(&op.SavedSaveDir);
	mem_free(&op.URLApp);
	mem_free(&op.URLAppCmdLine);
	mem_free(&op.AttachPath);
}
/* End of source */
