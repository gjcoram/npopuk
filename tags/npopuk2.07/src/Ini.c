/*
 * nPOP
 *
 * Ini.c
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
#include "Profile.h"
#include "charset.h"

/* Define */
#define GENERAL				TEXT("GENERAL")

#define INI_BUF_SIZE		1024

/* Global Variables */
OPTION op;
BOOL ConvertFromNPOP = FALSE;

// �O���Q��
extern HINSTANCE hInst;
extern TCHAR *g_Pass;
extern int gPassSt;
extern TCHAR *AppDir;
extern TCHAR *IniFile;
extern TCHAR *DataDir;
extern MAILBOX *MailBox;
extern int MailBoxCnt;
extern BOOL first_start;
extern BOOL SaveBoxesLoaded;

/* Local Function Prototypes */
static void ini_get_encode_info(void);

/*
 * ini_start_auth_check - Check Password
 */
BOOL ini_start_auth_check(void)
{
	TCHAR app_path[BUF_SIZE], app_path_old[BUF_SIZE];
	TCHAR ret[BUF_SIZE];
	TCHAR pass[BUF_SIZE];

	ConvertFromNPOP = FALSE;
	if (IniFile != NULL) {
		str_cpy_n_t(app_path, IniFile, BUF_SIZE);
		if (file_get_size(app_path) == -1) {
			wsprintf(ret, STR_ERR_INIFILE, IniFile);
			ErrorMessage(NULL, ret);
			return FALSE;
		}
	} else {
		str_join_t(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
		if (file_get_size(app_path) == -1) {
			DWORD DirInfo;
			DirInfo = GetFileAttributes(AppDir);
			if (DirInfo & FILE_ATTRIBUTE_READONLY) {
				ErrorMessage(NULL, STR_ERR_READONLY);
			}
			// GJC - check if upgrading from original nPOP
			str_join_t(app_path_old, AppDir, TEXT("nPOP.ini"),  (TCHAR *)-1);
			if (file_get_size(app_path_old) != -1) {
				if (MessageBox(NULL, STR_Q_UPGRADE, WINDOW_TITLE, MB_YESNO) == IDYES) {
					ConvertFromNPOP = TRUE;
					CopyFile(app_path_old, app_path, FALSE);
				}
			}
		}
	}
	profile_initialize(app_path, TRUE);

	op.StartPass = profile_get_int(GENERAL, TEXT("StartPass"), 0, app_path);
	if (op.StartPass == 1) {
		profile_get_string(GENERAL, TEXT("pw"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword(TEXT("_pw_"), ret, pass, BUF_SIZE - 1, TRUE);
		if (*pass == TEXT('\0')) {
			profile_free();
			return TRUE;
		}
		op.Password = alloc_copy_t(pass);
		//Starting password
		gPassSt = 0;
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), NULL, InputPassProc,
			(LPARAM)STR_TITLE_STARTPASSWORD) == FALSE) {
			profile_free();
			return FALSE;
		}
	}
	profile_free();
	return TRUE;
}

/*
 * ini_get_encode_info - �G���R�[�h���̎擾
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
 * ini_read_setting - INI�t�@�C������ݒ����ǂ݂���
 */
BOOL ini_read_setting(HWND hWnd)
{
	DWORD DirInfo;
	FILTER *tpFilter;
	HDC hdc;
	TCHAR app_path[BUF_SIZE];
	TCHAR buf[BUF_SIZE];
	TCHAR key_buf[BUF_SIZE];
	TCHAR conv_buf[INI_BUF_SIZE];
	TCHAR ret[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
	TCHAR *ConvertName = NULL;
	TCHAR *p, *r;
	UINT char_set;
	int i, j, t, cnt, num, len;
	int fDef;

	hdc = GetDC(hWnd);
#ifndef _WIN32_WCE
	char_set = GetTextCharset(hdc);
#else
	char_set = STR_DEFAULT_FONTCHARSET;
#endif
	ReleaseDC(hWnd, hdc);

	if (IniFile != NULL) {
		str_cpy_n_t(app_path, IniFile, BUF_SIZE);
		if (file_get_size(app_path) == -1) {
			ErrorMessage(NULL, STR_ERR_INIFILE);
			return FALSE;
		}
	} else {
		str_join_t(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	}
	if (profile_initialize(app_path, FALSE) == FALSE) {
		return FALSE;
	}

	len = profile_get_string(GENERAL, TEXT("DataFileDir"), TEXT(""), op.DataFileDir, BUF_SIZE - 1, app_path);

	if (*op.DataFileDir == TEXT('\0')) {
		DataDir = AppDir;
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

	op.BackupDir = profile_alloc_string(GENERAL, TEXT("BackupDir"), TEXT(""), app_path);

	op.Version = profile_get_int(GENERAL, TEXT("Version"), APP_VERSION_NUM, app_path);
	if (op.Version > APP_VERSION_NUM) {
		wsprintf(tmp, STR_MSG_NEWVERSION, app_path, KEY_NAME);
		if (MessageBox(hWnd, tmp, WINDOW_TITLE, MB_YESNO) == IDNO) {
			return FALSE;
		}
	}
	// if op.Version < APP_VERSION_NUM, adjust at end (after checking other settings)

#ifdef _DEBUG
	op.SocLog = profile_get_int(GENERAL, TEXT("SocLog"), 1, app_path);
#else
	op.SocLog = profile_get_int(GENERAL, TEXT("SocLog"), 0, app_path);
#endif

	op.view_font.name = profile_alloc_string(GENERAL, TEXT("FontName"), STR_DEFAULT_FONT, app_path);
	op.view_font.size = profile_get_int(GENERAL, TEXT("FontSize"), 9, app_path);
	op.view_font.weight = profile_get_int(GENERAL, TEXT("FontWeight"), 0, app_path);
	op.view_font.italic = profile_get_int(GENERAL, TEXT("FontItalic"), 0, app_path);
	op.view_font.charset = profile_get_int(GENERAL, TEXT("FontCharset"), char_set, app_path);

	op.lv_font.name = profile_alloc_string(GENERAL, TEXT("LvFontName"), TEXT(""), app_path);
	op.lv_font.size = profile_get_int(GENERAL, TEXT("LvFontSize"), 9, app_path);
	op.lv_font.weight = profile_get_int(GENERAL, TEXT("LvFontWeight"), 0, app_path);
	op.lv_font.italic = profile_get_int(GENERAL, TEXT("LvFontItalic"), 0, app_path);
	op.lv_font.charset = profile_get_int(GENERAL, TEXT("LvFontCharset"), char_set, app_path);

	op.StatusBarCharWidth = profile_get_int(GENERAL, TEXT("StatusBarCharWidth"), 7, app_path);

	op.HeadCharset = profile_alloc_string(GENERAL, TEXT("HeadCharset"), STR_DEFAULT_HEAD_CHARSET, app_path);
	op.HeadEncoding = profile_get_int(GENERAL, TEXT("HeadEncoding"), STR_DEFAULT_HEAD_ENCODE, app_path);
	op.BodyCharset = profile_alloc_string(GENERAL, TEXT("BodyCharset"), STR_DEFAULT_BODY_CHARSET, app_path);
	op.BodyEncoding = profile_get_int(GENERAL, TEXT("BodyEncoding"), STR_DEFAULT_BODY_ENCODE, app_path);
	if (op.HeadCharset == NULL || *op.HeadCharset == TEXT('\0') ||
		op.BodyCharset == NULL || *op.BodyCharset == TEXT('\0')) {
		ini_get_encode_info();
	}

	op.TimeZone = profile_alloc_string(GENERAL, TEXT("TimeZone"), STR_DEFAULT_TIMEZONE, app_path);
	op.DateFormat = profile_alloc_string(GENERAL, TEXT("DateFormat"), STR_DEFAULT_DATEFORMAT, app_path);
	op.TimeFormat = profile_alloc_string(GENERAL, TEXT("TimeFormat"), STR_DEFAULT_TIMEFORMAT, app_path);

#ifndef _WIN32_WCE
	op.MainRect.left = profile_get_int(GENERAL, TEXT("left"), 0, app_path);
	op.MainRect.top = profile_get_int(GENERAL, TEXT("top"), 0, app_path);
	op.MainRect.right = profile_get_int(GENERAL, TEXT("right"), 440, app_path);
	op.MainRect.bottom = profile_get_int(GENERAL, TEXT("bottom"), 320, app_path);
#endif

	op.ShowTrayIcon = profile_get_int(GENERAL, TEXT("ShowTrayIcon"), 1, app_path);
	op.StartHide = profile_get_int(GENERAL, TEXT("StartHide"), 0, app_path);
	op.MinsizeHide = profile_get_int(GENERAL, TEXT("MinsizeHide"), 0, app_path);
	op.CloseHide = profile_get_int(GENERAL, TEXT("CloseHide"), 0, app_path);
	op.TrayIconToggle = profile_get_int(GENERAL, TEXT("TrayIconToggle"), 0, app_path);
	op.StartInit = profile_get_int(GENERAL, TEXT("StartInit"), 0, app_path);

	op.LvDefSelectPos = profile_get_int(GENERAL, TEXT("LvDefSelectPos"), 1, app_path);
	op.LvAutoSort = profile_get_int(GENERAL, TEXT("LvAutoSort"), 1, app_path);
#ifdef _WIN32_WCE_PPC
////////////////////// MRP ////////////////////
	op.LvSortItem = profile_get_int(GENERAL, TEXT("LvSortItem"), 13, app_path);
#else
	op.LvSortItem = profile_get_int(GENERAL, TEXT("LvSortItem"), 3, app_path);
////////////////////// --- ////////////////////
#endif
	op.LvThreadView = profile_get_int(GENERAL, TEXT("LvThreadView"), 0, app_path);
	op.LvStyle = profile_get_int(GENERAL, TEXT("LvStyle"), LVS_SHOWSELALWAYS | LVS_REPORT, app_path);
	op.LvStyleEx = profile_get_int(GENERAL, TEXT("LvStyleEx"), LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, app_path);
	op.LvColumnOrder = profile_alloc_string(GENERAL, TEXT("LvColumnOrder"), TEXT("SFDZ"), app_path);
	t = profile_get_int(GENERAL, TEXT("MoveAllMailBox"), 1, app_path);
	op.ScanAllForUnread = profile_get_int(GENERAL, TEXT("ScanAllForUnread"), t, app_path);

	op.RecvScroll = profile_get_int(GENERAL, TEXT("RecvScroll"), 1, app_path);
	op.SaveMsg = profile_get_int(GENERAL, TEXT("SaveMsg"), 1, app_path);
	op.AutoSave = profile_get_int(GENERAL, TEXT("AutoSave"), 1, app_path);
	op.WriteMbox = profile_get_int(GENERAL, TEXT("WriteMbox"), 0, app_path);
	op.CheckQueuedOnExit = profile_get_int(GENERAL, TEXT("CheckQueuedOnExit"), 0, app_path);
	op.PromptSaveOnExit = profile_get_int(GENERAL, TEXT("PromptSaveOnExit"), 0, app_path);
#ifdef _WIN32_WCE
	op.LazyLoadMailboxes = profile_get_int(GENERAL, TEXT("LazyLoadMailboxes"), 1, app_path);
#else
	op.LazyLoadMailboxes = profile_get_int(GENERAL, TEXT("LazyLoadMailboxes"), 0, app_path);
#endif
	if (op.LazyLoadMailboxes == 0) {
		SaveBoxesLoaded = TRUE;
	}

	op.StartPass = profile_get_int(GENERAL, TEXT("StertPass"), 0, app_path);
	op.StartPass = profile_get_int(GENERAL, TEXT("StartPass"), op.StartPass, app_path);
	op.ShowPass = profile_get_int(GENERAL, TEXT("ShowPass"), 0, app_path);
//	op.ScrambleMailboxes = profile_get_int(GENERAL, TEXT("ScrambleMailboxes"), 0, app_path);
	profile_get_string(GENERAL, TEXT("pw"), TEXT(""), ret, BUF_SIZE - 1, app_path);
	EncodePassword(TEXT("_pw_"), ret, tmp, BUF_SIZE - 1, TRUE);
	op.Password = alloc_copy_t(tmp);

#ifdef _WIN32_WCE
	op.LvColSize[0] = profile_get_int(GENERAL, TEXT("LvColSize-0"), 158, app_path);
	op.LvColSize[1] = profile_get_int(GENERAL, TEXT("LvColSize-1"), 146, app_path);
	op.LvColSize[2] = profile_get_int(GENERAL, TEXT("LvColSize-2"), 123, app_path);
	op.LvColSize[3] = profile_get_int(GENERAL, TEXT("LvColSize-3"), 77, app_path);
#else
	op.LvColSize[0] = profile_get_int(GENERAL, TEXT("LvColSize-0"), 150, app_path);
	op.LvColSize[1] = profile_get_int(GENERAL, TEXT("LvColSize-1"), 100, app_path);
	op.LvColSize[2] = profile_get_int(GENERAL, TEXT("LvColSize-2"), 110, app_path);
	op.LvColSize[3] = profile_get_int(GENERAL, TEXT("LvColSize-3"), 50, app_path);
#endif
	if (op.LvColSize[0] + op.LvColSize[1] + op.LvColSize[2] + op.LvColSize[3] < 10) {
		for (i = 0; i < LV_COL_CNT; i++) {
			op.LvColSize[i] = 20;
		}
	}

	op.AddColSize[0] = profile_get_int(GENERAL, TEXT("AddColSize-0"), 200, app_path);
	op.AddColSize[1] = profile_get_int(GENERAL, TEXT("AddColSize-1"), 150, app_path);
	op.AddColSize[2] = profile_get_int(GENERAL, TEXT("AddColSize-2"), -1, app_path);
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

#ifndef _WIN32_WCE
	op.ViewRect.left = profile_get_int(GENERAL, TEXT("viewleft"), 0, app_path);
	op.ViewRect.top = profile_get_int(GENERAL, TEXT("viewtop"), 0, app_path);
	op.ViewRect.right = profile_get_int(GENERAL, TEXT("viewright"), 450, app_path);
	op.ViewRect.bottom = profile_get_int(GENERAL, TEXT("viewbottom"), 400, app_path);

	op.EditRect.left = profile_get_int(GENERAL, TEXT("editleft"), 0, app_path);
	op.EditRect.top = profile_get_int(GENERAL, TEXT("edittop"), 0, app_path);
	op.EditRect.right = profile_get_int(GENERAL, TEXT("editright"), 450, app_path);
	op.EditRect.bottom = profile_get_int(GENERAL, TEXT("editbottom"), 400, app_path);
#endif

	op.ShowHeader = profile_get_int(GENERAL, TEXT("ShowHeader"), 0, app_path);
#ifdef _WIN32_WCE_PPC
////////////////////// MRP ////////////////////
	op.ListGetLine = profile_get_int(GENERAL, TEXT("ListGetLine"), 50, app_path);
#else
	op.ListGetLine = profile_get_int(GENERAL, TEXT("ListGetLine"), 100, app_path);
////////////////////// --- ////////////////////
#endif

	op.ListDownload = profile_get_int(GENERAL, TEXT("ListDownload"), 0, app_path);
	op.ListSaveMode = profile_get_int(GENERAL, TEXT("ListSaveMode"), 2, app_path);
	op.WordBreakFlag = profile_get_int(GENERAL, TEXT("WordBreakFlag"), 1, app_path);
	op.EditWordBreakFlag = profile_get_int(GENERAL, TEXT("EditWordBreakFlag"), 1, app_path);

#ifdef _WIN32_WCE_PPC
////////////////////// MRP ////////////////////
	op.ViewShowDate = profile_get_int(GENERAL, TEXT("ViewShowDate"), 1, app_path);
#else
	op.ViewShowDate = profile_get_int(GENERAL, TEXT("ViewShowDate"), 0, app_path);
////////////////////// --- ////////////////////
#endif
	op.MatchCase = profile_get_int(GENERAL, TEXT("MstchCase"), 0, app_path);
	op.MatchCase = profile_get_int(GENERAL, TEXT("MatchCase"), op.MatchCase, app_path);
	op.AllMsgFind = profile_get_int(GENERAL, TEXT("AllFind"), 1, app_path);
	op.AllMsgFind = profile_get_int(GENERAL, TEXT("AllMsgFind"), op.AllMsgFind, app_path);
	op.AllBoxFind = profile_get_int(GENERAL, TEXT("AllBoxFind"), 1, app_path);
	op.SubjectFind = profile_get_int(GENERAL, TEXT("SubjectFind"), 0, app_path);

	op.ESMTP = profile_get_int(GENERAL, TEXT("ESMTP"), 0, app_path);
	op.SendHelo = profile_alloc_string(GENERAL, TEXT("SendHelo"), TEXT(""), app_path);
	op.SendMessageId = profile_get_int(GENERAL, TEXT("SendMessageId"), 1, app_path);
	op.SendDate = profile_get_int(GENERAL, TEXT("SendDate"), 1, app_path);
	op.SelectSendBox = profile_get_int(GENERAL, TEXT("SelectSendBox"), 1, app_path);
	op.ExpertMode = profile_get_int(GENERAL, TEXT("DisableWarning"), 0, app_path);		// Added PHH 4-Oct-2003
	op.GJCDebug = profile_get_int(GENERAL, TEXT("GJCDebug"), 1, app_path);
#ifdef _DEBUG
	op.GJCDebug = 1;
#endif
	op.PopBeforeSmtpIsLoginOnly = profile_get_int(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), 1, app_path);
	op.PopBeforeSmtpWait = profile_get_int(GENERAL, TEXT("PopBeforeSmtpWait"), 300, app_path);

	op.AutoQuotation = profile_get_int(GENERAL, TEXT("AutoQuotation"), 1, app_path);
	op.FwdQuotation = profile_get_int(GENERAL, TEXT("FwdQuotation"), 1, app_path);
	op.SignForward = profile_get_int(GENERAL, TEXT("SignForward"), 1, app_path);
	op.SignReplyAbove = profile_get_int(GENERAL, TEXT("SignReplyAbove"), 0, app_path);
	op.QuotationChar = profile_alloc_string(GENERAL, TEXT("QuotationChar"), TEXT("> "), app_path);
#ifdef _WIN32_WCE_PPC
////////////////////// MRP ////////////////////
	op.WordBreakSize = profile_get_int(GENERAL, TEXT("WordBreakSize"), 80, app_path);
#else
	op.WordBreakSize = profile_get_int(GENERAL, TEXT("WordBreakSize"), 70, app_path);
////////////////////// --- ////////////////////
#endif
	op.QuotationBreak = profile_get_int(GENERAL, TEXT("QuotationBreak"), 1, app_path);
	op.ReSubject = profile_alloc_string(GENERAL, TEXT("ReSubject"), TEXT("Re: "), app_path);
	op.FwdSubject = profile_alloc_string(GENERAL, TEXT("FwdSubject"), TEXT("Fwd: "), app_path);	// Added PHH 4-Oct-2003
	len = profile_get_string(GENERAL, TEXT("ReHeader"), TEXT("\\n--------------------------------------------------\\n%f wrote:\\n(%d)\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
	op.ReHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.ReHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.ReHeader);
	}
	len = profile_get_string(GENERAL, TEXT("FwdHeader"), TEXT("\\n--------------------------------------------------\\nTo: %T\\nCC: %C\\nSubject: %S\\nDate: %D\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
	op.FwdHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.FwdHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.FwdHeader);
	}
    op.AltReplyTo = profile_alloc_string(GENERAL, TEXT("ReplyTo"), TEXT(""), app_path);

	op.Bura = profile_alloc_string(GENERAL, TEXT("Bura"), STR_DEFAULT_BURA, app_path);
	op.Oida = profile_alloc_string(GENERAL, TEXT("Oida"), STR_DEFAULT_OIDA, app_path);

	op.CAFile = profile_alloc_string(GENERAL, TEXT("CAFile"), TEXT("ca.pem"), app_path);
	op.IPCache = profile_get_int(GENERAL, TEXT("IPCache"), 1, app_path);
	op.EncodeType = profile_get_int(GENERAL, TEXT("EncodeType"), 0, app_path);

	op.ShowNewMailMessage = profile_get_int(GENERAL, TEXT("ShowNewMailMessgae"), 1, app_path);
	op.ShowNewMailMessage = profile_get_int(GENERAL, TEXT("ShowNewMailMessage"), op.ShowNewMailMessage, app_path);
	op.ShowNoMailMessage = profile_get_int(GENERAL, TEXT("ShowNoMailMessage"), 0, app_path);
#ifdef _WIN32_WCE
	op.ActiveNewMailMessage = profile_get_int(GENERAL, TEXT("ActiveNewMailMessgae"), 1, app_path);
#else
	op.ActiveNewMailMessage = profile_get_int(GENERAL, TEXT("ActiveNewMailMessgae"), 0, app_path);
#endif
	op.ActiveNewMailMessage = profile_get_int(GENERAL, TEXT("ActiveNewMailMessage"), op.ActiveNewMailMessage, app_path);
	op.ClearNewOverlay = profile_get_int(GENERAL, TEXT("ClearNewOverlay"), 0, app_path);

#ifdef _WIN32_WCE_PPC
	///////////// MRP /////////////////////
	op.UsePOOMAddressBook = profile_get_int(GENERAL, TEXT("UsePOOMAddressBook"), 0, app_path);
	///////////// --- /////////////////////
	op.POOMNameIsComment = profile_get_int(GENERAL, TEXT("POOMNameIsComment"), 0, app_path);
	op.UseBuiltinSSL = profile_get_int(GENERAL, TEXT("UseBuiltinSSL"), 1, app_path);
#endif

	op.NewMailSound = profile_get_int(GENERAL, TEXT("NewMailSound"), 1, app_path);
	op.NewMailSoundFile = profile_alloc_string(GENERAL, TEXT("NewMailSoundFile"), TEXT(""), app_path);
	op.ExecEndSound = profile_get_int(GENERAL, TEXT("ExecEndSound"), 0, app_path);
	op.ExecEndSoundFile = profile_alloc_string(GENERAL, TEXT("ExecEndSoundFile"), TEXT(""), app_path);

	op.AutoCheck = profile_get_int(GENERAL, TEXT("AutoCheck"), 0, app_path);
	op.AutoCheckTime = profile_get_int(GENERAL, TEXT("AutoCheckTime"), 10, app_path);
	op.StartCheck = profile_get_int(GENERAL, TEXT("StartCheck"), 0, app_path);
	op.CheckAfterUpdate = profile_get_int(GENERAL, TEXT("CheckAfterUpdate"), 0, app_path);
	op.SocIgnoreError = profile_get_int(GENERAL, TEXT("SocIgnoreError"), 0, app_path);
	op.SendIgnoreError = profile_get_int(GENERAL, TEXT("SendIgnoreError"), 0, app_path);
	op.DecodeInPlace = profile_get_int(GENERAL, TEXT("DecodeInPlace"), 1, app_path);
	op.SendAttachIndividually = profile_get_int(GENERAL, TEXT("SendAttachIndividually"), 1, app_path);
	op.CheckEndExec = profile_get_int(GENERAL, TEXT("CheckEndExec"), 0, app_path);
	op.CheckEndExecNoDelMsg = profile_get_int(GENERAL, TEXT("CheckEndExecNoDelMsg"), 1, app_path);
	op.TimeoutInterval = profile_get_int(GENERAL, TEXT("TimeoutInterval"), 180, app_path);
	if (op.Version < 2007) {
		// value was in minutes before, now in seconds
		op.TimeoutInterval *= 60;
	}
	if (op.TimeoutInterval <= 0) op.TimeoutInterval = 60;

	op.ViewClose = profile_get_int(GENERAL, TEXT("ViewClose"), 1, app_path);
	op.ViewCloseNoNext = profile_get_int(GENERAL, TEXT("ViewCloseNoNext"), 1, app_path);
	op.ViewNextAfterDel = profile_get_int(GENERAL, TEXT("ViewNextAfterDel"), 1, app_path);
	op.ViewSkipDeleted = profile_get_int(GENERAL, TEXT("ViewSkipDeleted"), 2, app_path);
	op.ViewApp = profile_alloc_string(GENERAL, TEXT("ViewApp"), TEXT(""), app_path);
	op.ViewAppCmdLine = profile_alloc_string(GENERAL, TEXT("ViewAppCmdLine"), TEXT(""), app_path);
#ifdef _WIN32_WCE_PPC
	///////////// MRP /////////////////////
	op.ViewFileSuffix = profile_alloc_string(GENERAL, TEXT("ViewFileSuffix"), TEXT("html"), app_path);
	len = profile_get_string(GENERAL, TEXT("ViewFileHeader"),
	TEXT("<B>From:</B> %f<BR>\\n<B>To:</B> %t<BR>\\n<B>Cc:</B> %c<BR>\\n<B>Subject:</B> %s<BR>\\n<B>Date:</B> %d<BR>\\n<HR>\\n<BR>\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
	///////////// --- /////////////////////
#else
	op.ViewFileSuffix = profile_alloc_string(GENERAL, TEXT("ViewFileSuffix"), TEXT("txt"), app_path);
	len = profile_get_string(GENERAL, TEXT("ViewFileHeader"),
		TEXT("From: %f\\nTo: %t\\nCc: %c\\nSubject: %s\\nDate: %d\\n\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
#endif
	op.ViewFileHeader = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (op.ViewFileHeader != NULL) {
		DecodeCtrlChar(conv_buf, op.ViewFileHeader);
	}
	op.ViewAppClose = profile_get_int(GENERAL, TEXT("ViewAppClose"), 0, app_path);
	op.DefViewApp = profile_get_int(GENERAL, TEXT("DefViewApp"), 0, app_path);
	op.ViewAppMsgSource = profile_get_int(GENERAL, TEXT("ViewAppMsgSource"), 0, app_path);
	op.EditApp = profile_alloc_string(GENERAL, TEXT("EditApp"), TEXT(""), app_path);
	op.EditAppCmdLine = profile_alloc_string(GENERAL, TEXT("EditAppCmdLine"), TEXT(""), app_path);
	op.EditFileSuffix = profile_alloc_string(GENERAL, TEXT("EditFileSuffix"), TEXT("txt"), app_path);
	op.DefEditApp = profile_get_int(GENERAL, TEXT("DefEditApp"), 0, app_path);
	op.AttachPath = profile_alloc_string(GENERAL, TEXT("AttachPath"), TEXT("attach"), app_path);
	//op.AttachWarning = profile_get_int(GENERAL, TEXT("AttachWarning"), 1, app_path);
	op.AttachDelete = profile_get_int(GENERAL, TEXT("AttachDelete"), 1, app_path);
	op.StripHtmlTags = profile_get_int(GENERAL, TEXT("StripHtmlTags"), 1, app_path);
#ifdef _WIN32_WCE_PPC
	op.RememberOSD = profile_get_int(GENERAL, TEXT("RememberOpenSaveDir"), 0, app_path);
	if (op.RememberOSD == 1) {
		op.OpenSaveDir = profile_alloc_string(GENERAL, TEXT("OpenSaveDir"), TEXT(""), app_path);
	} else {
		op.OpenSaveDir = alloc_copy_t(TEXT(""));
	}
#endif


#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT("internet.exe"), app_path);
#else	// _WIN32_WCE_LAGENDA
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT("iexplore.exe"), app_path);
#endif	// _WIN32_WCE_LAGENDA
#else	// _WIN32_WCE
	op.URLApp = profile_alloc_string(GENERAL, TEXT("URLApp"), TEXT(""), app_path);
#endif	// _WIN32_WCE

	op.EnableLAN = profile_get_int(GENERAL, TEXT("EnableLAN"), 0, app_path);

	op.RasCon = profile_get_int(GENERAL, TEXT("RasCon"), 1, app_path);
	op.RasCheckEndDisCon = profile_get_int(GENERAL, TEXT("RasCheckEndDisCon"), 1, app_path);
	op.RasCheckEndDisConTimeout = profile_get_int(GENERAL, TEXT("RasCheckEndDisConTimeout"), 0, app_path);
	op.RasEndDisCon = profile_get_int(GENERAL, TEXT("RasEndDisCon"), 1, app_path);
	op.RasNoCheck = profile_get_int(GENERAL, TEXT("RasNoCheck"), 1, app_path);
	op.RasWaitSec = profile_get_int(GENERAL, TEXT("RasWaitSec"), 5, app_path);

	op.RasInfoCnt = profile_get_int(GENERAL, TEXT("RasInfoCnt"), 0, app_path);
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
		(*(op.RasInfo + j))->RasEntry = profile_alloc_string(TEXT("RASINFO"), key_buf, TEXT(""), app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasUser"));
		(*(op.RasInfo + j))->RasUser = profile_alloc_string(TEXT("RASINFO"), key_buf, TEXT(""), app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasPass"));
		len = profile_get_string(TEXT("RASINFO"), key_buf, TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((*(op.RasInfo + j))->RasUser, ret, tmp, BUF_SIZE - 1, TRUE);
		(*(op.RasInfo + j))->RasPass = alloc_copy_t(tmp);
	}

	cnt = profile_get_int(GENERAL, TEXT("MailBoxCnt"), 0, app_path);
	if (cnt == 0) {
		mailbox_create(hWnd, 1, FALSE, FALSE);
		first_start = TRUE;
		profile_free();
		return TRUE;
	}
	if (mailbox_create(hWnd, cnt, FALSE, FALSE) == -1) {
		profile_free();
		return FALSE;
	}
	for (i = 0; i < cnt; i++) {
		num = i + MAILBOX_USER;
		wsprintf(buf, TEXT("MAILBOX-%d"), i);

		// Name
		(MailBox + num)->Name = profile_alloc_string(buf, TEXT("Name"), TEXT(""), app_path);
		// Filename
		(MailBox + num)->Filename = profile_alloc_string(buf, TEXT("Filename"), TEXT(""), app_path);
		if (*(MailBox + num)->Filename == TEXT('\0')) {
			mem_free(&(MailBox + num)->Filename);
			(MailBox + num)->Filename = NULL;
		}
		// Type
		(MailBox + num)->Type = profile_get_int(buf, TEXT("Type"), 0, app_path);
		if ((MailBox + num)->Type == MAILBOX_TYPE_SAVE) {
			// GJC - SaveBox type (not an account)

			// Default account
			(MailBox + num)->DefAccount = profile_alloc_string(buf, TEXT("DefAccount"), TEXT(""), app_path);

			// Exempt from check all
			(MailBox + num)->CyclicFlag = 1;
			if (op.LazyLoadMailboxes == 0) {
				if ((MailBox+num)->Filename == NULL) {
					wsprintf(buf, TEXT("MailBox%d.dat"), i);
				} else {
					lstrcpy(buf, (MailBox+num)->Filename);
				}
				if (file_read_mailbox(buf, (MailBox + num), FALSE) == FALSE) {
					profile_free();
					return FALSE;
				}
			} else {
				(MailBox + num)->Loaded = FALSE;
			}
			continue;
		}
		// Server
		(MailBox + num)->Server = profile_alloc_string(buf, TEXT("Server"), TEXT(""), app_path);
		// Port
		(MailBox + num)->Port = profile_get_int(buf, TEXT("Port"), 110, app_path);
		// User
		(MailBox + num)->User = profile_alloc_string(buf, TEXT("User"), TEXT(""), app_path);
		// Pass
		profile_get_string(buf, TEXT("Pass"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((MailBox + num)->User, ret, tmp, BUF_SIZE - 1, TRUE);
		(MailBox + num)->Pass = alloc_copy_t(tmp);
		// APOP
		(MailBox + num)->APOP = profile_get_int(buf, TEXT("APOP"), 0, app_path);
		// POP SSL
		(MailBox + num)->PopSSL = profile_get_int(buf, TEXT("PopSSL"), 0, app_path);
		// POP SSL Option
		(MailBox + num)->PopSSLInfo.Type = profile_get_int(buf, TEXT("PopSSLType"), 0, app_path);
		(MailBox + num)->PopSSLInfo.Verify = profile_get_int(buf, TEXT("PopSSLVerify"), 1, app_path);
		(MailBox + num)->PopSSLInfo.Depth = profile_get_int(buf, TEXT("PopSSLDepth"), -1, app_path);
		(MailBox + num)->PopSSLInfo.Cert = profile_alloc_string(buf, TEXT("PopSSLCert"), TEXT(""), app_path);
		(MailBox + num)->PopSSLInfo.Pkey = profile_alloc_string(buf, TEXT("PopSSLPkey"), TEXT(""), app_path);
		(MailBox + num)->PopSSLInfo.Pass = profile_alloc_string(buf, TEXT("PopSSLPass"), TEXT(""), app_path);
		// Disable RETR
		(MailBox + num)->NoRETR = profile_get_int(buf, TEXT("NoRETR"), 0, app_path);
		// Disable UIDL
		(MailBox + num)->NoUIDL = profile_get_int(buf, TEXT("NoUIDL"), 0, app_path);

		// MailCnt
		(MailBox + num)->MailCnt = profile_get_int(buf, TEXT("MailCnt"), 0, app_path);
		// MailSize
		(MailBox + num)->MailSize = profile_get_int(buf, TEXT("MailSize"), 0, app_path);

		(MailBox + num)->StartInit = profile_get_int(buf, TEXT("StartInit"), 0, app_path);
		if (op.StartInit == 0 && (MailBox + num)->StartInit == 0) {
			// LastMessageId
			profile_get_string(buf, TEXT("LastMessageId"), TEXT(""), ret, BUF_SIZE - 1, app_path);
			(MailBox + num)->LastMessageId = alloc_tchar_to_char(ret);
			// LastNo
			(MailBox + num)->LastNo = profile_get_int(buf, TEXT("LastNo"), 0, app_path);
		} else {
			// �N�����ɐV���ʒu�̏�����
			(MailBox + num)->LastNo = -1;
			if (op.StartInit == 2 || (MailBox + num)->StartInit == 2) {
				// fill in from start, don't delete existing messages
				(MailBox+num)->ListInitMsg = FALSE;
			} else {
				(MailBox+num)->ListInitMsg = TRUE;
			}
		}

		// CyclicFlag
		(MailBox + num)->CyclicFlag = profile_get_int(buf, TEXT("CyclicFlag"), 0, app_path);

		// SmtpServer
		(MailBox + num)->SmtpServer = profile_alloc_string(buf, TEXT("SmtpServer"), TEXT(""), app_path);
		// SmtpPort
		(MailBox + num)->SmtpPort = profile_get_int(buf, TEXT("SmtpPort"), 25, app_path);
		// UserName
		(MailBox + num)->UserName = profile_alloc_string(buf, TEXT("UserName"), TEXT(""), app_path);
		// MailAddress
		(MailBox + num)->MailAddress = profile_alloc_string(buf, TEXT("MailAddress"), TEXT(""), app_path);
		// Signature
		p = (TCHAR *)mem_alloc(sizeof(TCHAR) * MAXSIZE);
		if (p != NULL) {
			len = profile_get_string(buf, TEXT("Signature"), TEXT(""), p, MAXSIZE - 1, app_path);
			(MailBox + num)->Signature = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
			if ((MailBox + num)->Signature != NULL) {
				DecodeCtrlChar(p, (MailBox + num)->Signature);
			}
			mem_free(&p);
		}
		// ReplyTo
		(MailBox + num)->ReplyTo = profile_alloc_string(buf, TEXT("ReplyTo"), TEXT(""), app_path);
		// MyAddr2Bcc
		(MailBox + num)->MyAddr2Bcc = profile_get_int(buf, TEXT("MyAddr2Bcc"), 0, app_path);
		// BccAddr
		(MailBox + num)->BccAddr = profile_alloc_string(buf, TEXT("BccAddr"), TEXT(""), app_path);

		// POP before SMTP
		(MailBox + num)->PopBeforeSmtp = profile_get_int(buf, TEXT("PopBeforeSmtp"), 0, app_path);
		// SMTP Authentication
		(MailBox + num)->SmtpAuth = profile_get_int(buf, TEXT("SmtpAuth"), 0, app_path);
		// SMTP Authentication type
		(MailBox + num)->SmtpAuthType = profile_get_int(buf, TEXT("SmtpAuthType"), 0, app_path);
		// SMTP Authentication User & Pass mode
		(MailBox + num)->AuthUserPass = profile_get_int(buf, TEXT("AuthUserPass"), 0, app_path);
		// SMTP Authentication User
		(MailBox + num)->SmtpUser = profile_alloc_string(buf, TEXT("SmtpUser"), TEXT(""), app_path);
		// SMTP Authentication Pass
		profile_get_string(buf, TEXT("SmtpPass"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((MailBox + num)->SmtpUser, ret, tmp, BUF_SIZE - 1, TRUE);
		(MailBox + num)->SmtpPass = alloc_copy_t(tmp);
		// SMTP SSL
		(MailBox + num)->SmtpSSL = profile_get_int(buf, TEXT("SmtpSSL"), 0, app_path);
		// SMTP SSL Option
		(MailBox + num)->SmtpSSLInfo.Type = profile_get_int(buf, TEXT("SmtpSSLType"), 0, app_path);
		(MailBox + num)->SmtpSSLInfo.Verify = profile_get_int(buf, TEXT("SmtpSSLVerify"), 1, app_path);
		(MailBox + num)->SmtpSSLInfo.Depth = profile_get_int(buf, TEXT("SmtpSSLDepth"), -1, app_path);
		(MailBox + num)->SmtpSSLInfo.Cert = profile_alloc_string(buf, TEXT("SmtpSSLCert"), TEXT(""), app_path);
		(MailBox + num)->SmtpSSLInfo.Pkey = profile_alloc_string(buf, TEXT("SmtpSSLPkey"), TEXT(""), app_path);
		(MailBox + num)->SmtpSSLInfo.Pass = profile_alloc_string(buf, TEXT("SmtpSSLPass"), TEXT(""), app_path);

		// Filter
		(MailBox + num)->FilterEnable = profile_get_int(buf, TEXT("FilterEnable"), 0, app_path);
		(MailBox + num)->FilterCnt = profile_get_int(buf, TEXT("FilterCnt"), 0, app_path);
		fDef = profile_get_int(buf, TEXT("FilterFlag"), -1, app_path);

		(MailBox + num)->tpFilter = (FILTER **)mem_calloc(sizeof(FILTER *) * (MailBox + num)->FilterCnt);
		if ((MailBox + num)->tpFilter == NULL) {
			(MailBox + num)->FilterCnt = 0;
		}
		for (t = 0; t < (MailBox + num)->FilterCnt; t++) {
			tpFilter = *((MailBox + num)->tpFilter + t) = (FILTER *)mem_calloc(sizeof(FILTER));
			if (tpFilter == NULL) {
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			tpFilter->Enable = profile_get_int(buf, key_buf, 0, app_path);

			if (fDef == -1) {
				wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
				tpFilter->Action = profile_get_int(buf, key_buf, 0, app_path);
			} else {
				tpFilter->Action = fDef;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
			tpFilter->SaveboxName = profile_alloc_string(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			tpFilter->Header1 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			tpFilter->Content1 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
			tpFilter->Boolean = profile_get_int(buf, key_buf, 0, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			tpFilter->Header2 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			tpFilter->Content2 = profile_alloc_string(buf, key_buf, TEXT(""), app_path);
		}

		// RAS
		(MailBox + num)->RasMode = profile_get_int(buf, TEXT("RasMode"), 0, app_path);
		(MailBox + num)->RasEntry = profile_alloc_string(buf, TEXT("RasEntry"), TEXT(""), app_path);
		(MailBox + num)->RasReCon = profile_get_int(buf, TEXT("RasReCon"), 0, app_path);

		if (op.LazyLoadMailboxes == 0) {
			if ((MailBox+num)->Filename == NULL) {
				wsprintf(buf, TEXT("MailBox%d.dat"), i);
			} else {
				lstrcpy(buf, (MailBox+num)->Filename);
			}
			if (file_read_mailbox(buf, (MailBox + num), FALSE) == FALSE) {
				profile_free();
				return FALSE;
			}
		} else {
			(MailBox + num)->Loaded = FALSE;
		}
	}

	// convert old-style [SaveBox] to a new savebox-type account (GJC)
	wsprintf(buf, TEXT("MailBox%d.dat"), MailBoxCnt);
	if (ConvertFromNPOP && file_savebox_convert(buf)) {
		MessageBox(NULL, TEXT("Converting [Savebox]"), WINDOW_TITLE, MB_OK);
		num = mailbox_create(hWnd, 1, FALSE, FALSE);
		if (num == -1) {
			profile_free();
			return FALSE;
		}
		(MailBox + num)->Name = alloc_copy_t(STR_SAVEBOX_NAME);
		ConvertName = (MailBox + num)->Name;
		(MailBox + num)->Type = MAILBOX_TYPE_SAVE;
		(MailBox + num)->CyclicFlag = 1;
		if (file_read_mailbox(buf, (MailBox + num), FALSE) == FALSE) {
			profile_free();
			return FALSE;
		}
	}

	if (op.Version < APP_VERSION_NUM) {
		op.Version = APP_VERSION_NUM;
	}

	// check that Saveboxes for filters are valid
	filter_sbox_check(hWnd, ConvertName);

	profile_free();
	return TRUE;
}

/*
 * ini_save_setting - INI�t�@�C���֐ݒ���������o��
 */
BOOL ini_save_setting(HWND hWnd, BOOL SaveMailFlag, BOOL SaveAll, TCHAR *SaveDir)
{
	FILTER *tpFilter;
	TCHAR app_path[BUF_SIZE];
	///////////// MRP /////////////////////
	TCHAR app_pathBackup[BUF_SIZE];
	///////////// --- /////////////////////
	TCHAR buf[BUF_SIZE];
	TCHAR key_buf[BUF_SIZE];
	TCHAR conv_buf[INI_BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
#ifdef UNICODE
	TCHAR ret[BUF_SIZE];
#endif
	TCHAR *p;
	int j, t;
	BOOL rc = TRUE;
	BOOL found;
	BOOL is_backup = FALSE;

	if (SaveDir == NULL) {
		if (IniFile == NULL) {
			str_join_t(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
		} else {
			str_cpy_n_t(app_path, IniFile, BUF_SIZE);
		}
	} else {
		is_backup = TRUE;
		str_join_t(app_path, SaveDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	}

	///////////// MRP /////////////////////
#ifdef UNICODE
   wcscpy(app_pathBackup, app_path);
   wcscat(app_pathBackup, TEXT(".bak"));
#else
   strcpy_s(app_pathBackup, BUF_SIZE-5, app_path);
   strcat_s(app_pathBackup, BUF_SIZE, TEXT(".bak"));
#endif
	DeleteFile(app_pathBackup);
	CopyFile(app_path, app_pathBackup, FALSE); // Create the backup file.
	///////////// --- /////////////////////

	// if IniFile != NULL or SaveDir != NULL, this initializes from
	// the previous backup, not from the ini file in use!
	profile_initialize(app_path, FALSE);

	if (is_backup == TRUE) {
		profile_write_string(GENERAL, TEXT("DataFileDir"), TEXT(""), app_path);
	} else {
		profile_write_string(GENERAL, TEXT("DataFileDir"), op.DataFileDir, app_path);
	}
	profile_write_string(GENERAL, TEXT("BackupDir"), op.BackupDir, app_path);
	profile_write_int(GENERAL, TEXT("Version"), op.Version, app_path);
	profile_write_int(GENERAL, TEXT("SocLog"), op.SocLog, app_path);

	profile_write_string(GENERAL, TEXT("FontName"), op.view_font.name, app_path);
	profile_write_int(GENERAL, TEXT("FontSize"), op.view_font.size, app_path);
	profile_write_int(GENERAL, TEXT("FontWeight"), op.view_font.weight, app_path);
	profile_write_int(GENERAL, TEXT("FontItalic"), op.view_font.italic, app_path);
	profile_write_int(GENERAL, TEXT("FontCharset"), op.view_font.charset, app_path);
	
	profile_write_string(GENERAL, TEXT("LvFontName"), op.lv_font.name, app_path);
	profile_write_int(GENERAL, TEXT("LvFontSize"), op.lv_font.size, app_path);
	profile_write_int(GENERAL, TEXT("LvFontWeight"), op.lv_font.weight, app_path);
	profile_write_int(GENERAL, TEXT("LvFontItalic"), op.lv_font.italic, app_path);
	profile_write_int(GENERAL, TEXT("LvFontCharset"), op.lv_font.charset, app_path);

	profile_write_int(GENERAL, TEXT("StatusBarCharWidth"), op.StatusBarCharWidth, app_path);

	profile_write_string(GENERAL, TEXT("HeadCharset"), op.HeadCharset, app_path);
	profile_write_int(GENERAL, TEXT("HeadEncoding"), op.HeadEncoding, app_path);
	profile_write_string(GENERAL, TEXT("BodyCharset"), op.BodyCharset, app_path);
	profile_write_int(GENERAL, TEXT("BodyEncoding"), op.BodyEncoding, app_path);
	profile_write_string(GENERAL, TEXT("TimeZone"), op.TimeZone, app_path);
	profile_write_string(GENERAL, TEXT("DateFormat"), op.DateFormat, app_path);
	profile_write_string(GENERAL, TEXT("TimeFormat"), op.TimeFormat, app_path);

#ifndef _WIN32_WCE
	profile_write_int(GENERAL, TEXT("left"), op.MainRect.left, app_path);
	profile_write_int(GENERAL, TEXT("top"), op.MainRect.top, app_path);
	profile_write_int(GENERAL, TEXT("right"), op.MainRect.right, app_path);
	profile_write_int(GENERAL, TEXT("bottom"), op.MainRect.bottom, app_path);
#endif

	profile_write_int(GENERAL, TEXT("ShowTrayIcon"), op.ShowTrayIcon, app_path);
	profile_write_int(GENERAL, TEXT("StartHide"), op.StartHide, app_path);
	profile_write_int(GENERAL, TEXT("MinsizeHide"), op.MinsizeHide, app_path);
	profile_write_int(GENERAL, TEXT("CloseHide"), op.CloseHide, app_path);
	profile_write_int(GENERAL, TEXT("TrayIconToggle"), op.TrayIconToggle, app_path);
	profile_write_int(GENERAL, TEXT("StartInit"), op.StartInit, app_path);

	profile_write_int(GENERAL, TEXT("LvDefSelectPos"), op.LvDefSelectPos, app_path);
	profile_write_int(GENERAL, TEXT("LvAutoSort"), op.LvAutoSort, app_path);
	profile_write_int(GENERAL, TEXT("LvSortItem"), op.LvSortItem, app_path);
	profile_write_int(GENERAL, TEXT("LvThreadView"), op.LvThreadView, app_path);
	profile_write_int(GENERAL, TEXT("LvStyle"), op.LvStyle, app_path);
	profile_write_int(GENERAL, TEXT("LvStyleEx"), op.LvStyleEx, app_path);
	profile_write_string(GENERAL, TEXT("LvColumnOrder"), op.LvColumnOrder, app_path);
	profile_write_int(GENERAL, TEXT("ScanAllForUnread"), op.ScanAllForUnread, app_path);
	profile_write_int(GENERAL, TEXT("RecvScroll"), op.RecvScroll, app_path);
	profile_write_int(GENERAL, TEXT("SaveMsg"), op.SaveMsg, app_path);
	profile_write_int(GENERAL, TEXT("AutoSave"), op.AutoSave, app_path);
	profile_write_int(GENERAL, TEXT("WriteMbox"), op.WriteMbox, app_path);
	profile_write_int(GENERAL, TEXT("CheckQueuedOnExit"), op.CheckQueuedOnExit, app_path);
	profile_write_int(GENERAL, TEXT("PromptSaveOnExit"), op.PromptSaveOnExit, app_path);
	profile_write_int(GENERAL, TEXT("LazyLoadMailboxes"), op.LazyLoadMailboxes, app_path);
	profile_write_int(GENERAL, TEXT("StartPass"), op.StartPass, app_path);
	profile_write_int(GENERAL, TEXT("ShowPass"), op.ShowPass, app_path);
//	profile_write_int(GENERAL, TEXT("ScrambleMailboxes"), op.ScrambleMailboxes, app_path);
	EncodePassword(TEXT("_pw_"), op.Password, tmp, BUF_SIZE - 1, FALSE);
	profile_write_string(GENERAL, TEXT("pw"), tmp, app_path);

	profile_write_int(GENERAL, TEXT("LvColSize-0"), op.LvColSize[0], app_path);
	profile_write_int(GENERAL, TEXT("LvColSize-1"), op.LvColSize[1], app_path);
	profile_write_int(GENERAL, TEXT("LvColSize-2"), op.LvColSize[2], app_path);
	profile_write_int(GENERAL, TEXT("LvColSize-3"), op.LvColSize[3], app_path);

	profile_write_int(GENERAL, TEXT("AddColSize-0"), op.AddColSize[0], app_path);
	profile_write_int(GENERAL, TEXT("AddColSize-1"), op.AddColSize[1], app_path);
	profile_write_int(GENERAL, TEXT("AddColSize-2"), op.AddColSize[2], app_path);

#ifndef _WIN32_WCE
	profile_write_int(GENERAL, TEXT("viewleft"), op.ViewRect.left, app_path);
	profile_write_int(GENERAL, TEXT("viewtop"), op.ViewRect.top, app_path);
	profile_write_int(GENERAL, TEXT("viewright"), op.ViewRect.right, app_path);
	profile_write_int(GENERAL, TEXT("viewbottom"), op.ViewRect.bottom, app_path);

	profile_write_int(GENERAL, TEXT("editleft"), op.EditRect.left, app_path);
	profile_write_int(GENERAL, TEXT("edittop"), op.EditRect.top, app_path);
	profile_write_int(GENERAL, TEXT("editright"), op.EditRect.right, app_path);
	profile_write_int(GENERAL, TEXT("editbottom"), op.EditRect.bottom, app_path);
#endif

	profile_write_int(GENERAL, TEXT("ShowHeader"), op.ShowHeader, app_path);
	profile_write_int(GENERAL, TEXT("ListGetLine"), op.ListGetLine, app_path);
	profile_write_int(GENERAL, TEXT("ListDownload"), op.ListDownload, app_path);
	profile_write_int(GENERAL, TEXT("ListSaveMode"), op.ListSaveMode, app_path);
	profile_write_int(GENERAL, TEXT("WordBreakFlag"), op.WordBreakFlag, app_path);
	profile_write_int(GENERAL, TEXT("EditWordBreakFlag"), op.EditWordBreakFlag, app_path);
	profile_write_int(GENERAL, TEXT("ViewShowDate"), op.ViewShowDate, app_path);
	profile_write_int(GENERAL, TEXT("MatchCase"), op.MatchCase, app_path);
	profile_write_int(GENERAL, TEXT("AllMsgFind"), op.AllMsgFind, app_path);
	profile_write_int(GENERAL, TEXT("AllBoxFind"), op.AllBoxFind, app_path);
	profile_write_int(GENERAL, TEXT("SubjectFind"), op.SubjectFind, app_path);

	profile_write_int(GENERAL, TEXT("ESMTP"), op.ESMTP, app_path);
	profile_write_string(GENERAL, TEXT("SendHelo"), op.SendHelo, app_path);
	profile_write_int(GENERAL, TEXT("SendMessageId"), op.SendMessageId, app_path);
	profile_write_int(GENERAL, TEXT("SendDate"), op.SendDate, app_path);
	profile_write_int(GENERAL, TEXT("SelectSendBox"), op.SelectSendBox, app_path);
	profile_write_int(GENERAL, TEXT("DisableWarning"), op.ExpertMode, app_path);	// Added PHH 4-Oct-2003
	profile_write_int(GENERAL, TEXT("GJCDebug"), op.GJCDebug, app_path);
	profile_write_int(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), op.PopBeforeSmtpIsLoginOnly, app_path);
	profile_write_int(GENERAL, TEXT("PopBeforeSmtpWait"), op.PopBeforeSmtpWait, app_path);

	profile_write_int(GENERAL, TEXT("AutoQuotation"), op.AutoQuotation, app_path);
	profile_write_int(GENERAL, TEXT("FwdQuotation"), op.FwdQuotation, app_path);
	profile_write_int(GENERAL, TEXT("SignForward"), op.SignForward, app_path);
	profile_write_int(GENERAL, TEXT("SignReplyAbove"), op.SignReplyAbove, app_path);
	profile_write_string(GENERAL, TEXT("QuotationChar"), op.QuotationChar, app_path);
	profile_write_int(GENERAL, TEXT("WordBreakSize"), op.WordBreakSize, app_path);
	profile_write_int(GENERAL, TEXT("QuotationBreak"), op.QuotationBreak, app_path);
	profile_write_string(GENERAL, TEXT("ReSubject"), op.ReSubject, app_path);
	profile_write_string(GENERAL, TEXT("FwdSubject"), op.FwdSubject, app_path);		// Added PHH 4-10-2003
	EncodeCtrlChar(op.ReHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("ReHeader"), conv_buf, app_path);
	EncodeCtrlChar(op.FwdHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("FwdHeader"), conv_buf, app_path);
	profile_write_string(GENERAL, TEXT("ReplyTo"), op.AltReplyTo, app_path);

	profile_write_string(GENERAL, TEXT("Bura"), op.Bura, app_path);
	profile_write_string(GENERAL, TEXT("Oida"), op.Oida, app_path);

	profile_write_string(GENERAL, TEXT("CAFile"), op.CAFile, app_path);

	profile_write_int(GENERAL, TEXT("IPCache"), op.IPCache, app_path);
	profile_write_int(GENERAL, TEXT("EncodeType"), op.EncodeType, app_path);

	profile_write_int(GENERAL, TEXT("ShowNewMailMessage"), op.ShowNewMailMessage, app_path);
	profile_write_int(GENERAL, TEXT("ShowNoMailMessage"), op.ShowNoMailMessage, app_path);
	profile_write_int(GENERAL, TEXT("ActiveNewMailMessage"), op.ActiveNewMailMessage, app_path);
	profile_write_int(GENERAL, TEXT("ClearNewOverlay"), op.ClearNewOverlay, app_path);

#ifdef _WIN32_WCE_PPC
	///////////// MRP /////////////////////
	profile_write_int(GENERAL, TEXT("UsePOOMAddressBook"), op.UsePOOMAddressBook, app_path);
	///////////// --- /////////////////////
	profile_write_int(GENERAL, TEXT("POOMNameIsComment"), op.POOMNameIsComment, app_path);
	profile_write_int(GENERAL, TEXT("UseBuiltinSSL"), op.UseBuiltinSSL, app_path);
#endif

	profile_write_int(GENERAL, TEXT("NewMailSound"), op.NewMailSound, app_path);
	profile_write_string(GENERAL, TEXT("NewMailSoundFile"), op.NewMailSoundFile, app_path);
	profile_write_int(GENERAL, TEXT("ExecEndSound"), op.ExecEndSound, app_path);
	profile_write_string(GENERAL, TEXT("ExecEndSoundFile"), op.ExecEndSoundFile, app_path);

	profile_write_int(GENERAL, TEXT("AutoCheck"), op.AutoCheck, app_path);
	profile_write_int(GENERAL, TEXT("AutoCheckTime"), op.AutoCheckTime, app_path);
	profile_write_int(GENERAL, TEXT("StartCheck"), op.StartCheck, app_path);
	profile_write_int(GENERAL, TEXT("CheckAfterUpdate"), op.CheckAfterUpdate, app_path);
	profile_write_int(GENERAL, TEXT("SocIgnoreError"), op.SocIgnoreError, app_path);
	profile_write_int(GENERAL, TEXT("SendIgnoreError"), op.SendIgnoreError, app_path);
	profile_write_int(GENERAL, TEXT("DecodeInPlace"), op.DecodeInPlace, app_path);
	profile_write_int(GENERAL, TEXT("SendAttachIndividually"), op.SendAttachIndividually, app_path);
	profile_write_int(GENERAL, TEXT("CheckEndExec"), op.CheckEndExec, app_path);
	profile_write_int(GENERAL, TEXT("CheckEndExecNoDelMsg"), op.CheckEndExecNoDelMsg, app_path);
	profile_write_int(GENERAL, TEXT("TimeoutInterval"), op.TimeoutInterval, app_path);

	profile_write_int(GENERAL, TEXT("ViewClose"), op.ViewClose, app_path);
	profile_write_int(GENERAL, TEXT("ViewCloseNoNext"), op.ViewCloseNoNext, app_path);
	profile_write_int(GENERAL, TEXT("ViewNextAfterDel"), op.ViewNextAfterDel, app_path);
	profile_write_int(GENERAL, TEXT("ViewSkipDeleted"), op.ViewSkipDeleted, app_path);
	profile_write_string(GENERAL, TEXT("ViewApp"), op.ViewApp, app_path);
	profile_write_string(GENERAL, TEXT("ViewAppCmdLine"), op.ViewAppCmdLine, app_path);
	profile_write_string(GENERAL, TEXT("ViewFileSuffix"), op.ViewFileSuffix, app_path);
	EncodeCtrlChar(op.ViewFileHeader, conv_buf);
	profile_write_string(GENERAL, TEXT("ViewFileHeader"), conv_buf, app_path);
	profile_write_int(GENERAL, TEXT("ViewAppClose"), op.ViewAppClose, app_path);
	profile_write_int(GENERAL, TEXT("DefViewApp"), op.DefViewApp, app_path);
	profile_write_int(GENERAL, TEXT("ViewAppMsgSource"), op.ViewAppMsgSource, app_path);
	profile_write_string(GENERAL, TEXT("EditApp"), op.EditApp, app_path);
	profile_write_string(GENERAL, TEXT("EditAppCmdLine"), op.EditAppCmdLine, app_path);
	profile_write_string(GENERAL, TEXT("EditFileSuffix"), op.EditFileSuffix, app_path);
	profile_write_int(GENERAL, TEXT("DefEditApp"), op.DefEditApp, app_path);
	profile_write_string(GENERAL, TEXT("AttachPath"), op.AttachPath, app_path);
	//profile_write_int(GENERAL, TEXT("AttachWarning"), op.AttachWarning, app_path);
	profile_write_int(GENERAL, TEXT("AttachDelete"), op.AttachDelete, app_path);
	profile_write_int(GENERAL, TEXT("StripHtmlTags"), op.StripHtmlTags, app_path);
#ifdef _WIN32_WCE_PPC
	profile_write_int(GENERAL, TEXT("RememberOpenSaveDir"), op.RememberOSD, app_path);
	if (op.RememberOSD == 1 && op.OpenSaveDir != NULL) {
		profile_write_string(GENERAL, TEXT("OpenSaveDir"), op.OpenSaveDir, app_path);
	} else {
		profile_write_string(GENERAL, TEXT("OpenSaveDir"), TEXT(""), app_path);
	}
#endif

	profile_write_string(GENERAL, TEXT("URLApp"), op.URLApp, app_path);

	profile_write_int(GENERAL, TEXT("EnableLAN"), op.EnableLAN, app_path);

	profile_write_int(GENERAL, TEXT("RasCon"), op.RasCon, app_path);
	profile_write_int(GENERAL, TEXT("RasCheckEndDisCon"), op.RasCheckEndDisCon, app_path);
	profile_write_int(GENERAL, TEXT("RasCheckEndDisConTimeout"), op.RasCheckEndDisConTimeout, app_path);
	profile_write_int(GENERAL, TEXT("RasEndDisCon"), op.RasEndDisCon, app_path);
	profile_write_int(GENERAL, TEXT("RasNoCheck"), op.RasNoCheck, app_path);
	profile_write_int(GENERAL, TEXT("RasWaitSec"), op.RasWaitSec, app_path);

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

	for (t = 0, j = 0; j < op.RasInfoCnt; j++) {
		if (*(op.RasInfo + j) == NULL ||
			(*(op.RasInfo + j))->RasEntry == NULL || *(*(op.RasInfo + j))->RasEntry == TEXT('\0')) {
			continue;
		}
		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasEntry"));
		profile_write_string(TEXT("RASINFO"), key_buf, (*(op.RasInfo + j))->RasEntry, app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasUser"));
		profile_write_string(TEXT("RASINFO"), key_buf, (*(op.RasInfo + j))->RasUser, app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasPass"));
		EncodePassword((*(op.RasInfo + j))->RasUser, (*(op.RasInfo + j))->RasPass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(TEXT("RASINFO"), key_buf, tmp, app_path);
		t++;
	}
	profile_write_int(GENERAL, TEXT("RasInfoCnt"), t, app_path);

	// Retention
	profile_write_int(GENERAL, TEXT("MailBoxCnt"), MailBoxCnt - MAILBOX_USER, app_path);

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
		profile_write_string(buf, TEXT("Name"), (MailBox + j)->Name, app_path);
		// Filename
		profile_write_string(buf, TEXT("Filename"), (MailBox + j)->Filename, app_path);
		// Type
		profile_write_int(buf, TEXT("Type"), (MailBox + j)->Type, app_path);

		if ((MailBox + j)->Type == MAILBOX_TYPE_SAVE) {
			// Default account
			profile_write_string(buf, TEXT("DefAccount"), (MailBox + j)->DefAccount, app_path);

			// no other settings for SaveBox-type mailboxes
			continue;
		}

		// Server
		profile_write_string(buf, TEXT("Server"), (MailBox + j)->Server, app_path);
		// Port
		profile_write_int(buf, TEXT("Port"), (MailBox + j)->Port, app_path);
		// User
		profile_write_string(buf, TEXT("User"), (MailBox + j)->User, app_path);
		// Pass
		EncodePassword((MailBox + j)->User, (MailBox + j)->Pass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(buf, TEXT("Pass"), tmp, app_path);
		// APOP
		profile_write_int(buf, TEXT("APOP"), (MailBox + j)->APOP, app_path);
		// POP SSL
		profile_write_int(buf, TEXT("PopSSL"), (MailBox + j)->PopSSL, app_path);
		// POP SSL Option
		profile_write_int(buf, TEXT("PopSSLType"), (MailBox + j)->PopSSLInfo.Type, app_path);
		profile_write_int(buf, TEXT("PopSSLVerify"), (MailBox + j)->PopSSLInfo.Verify, app_path);
		profile_write_int(buf, TEXT("PopSSLDepth"), (MailBox + j)->PopSSLInfo.Depth, app_path);
		profile_write_string(buf, TEXT("PopSSLCert"), (MailBox + j)->PopSSLInfo.Cert, app_path);
		profile_write_string(buf, TEXT("PopSSLPkey"), (MailBox + j)->PopSSLInfo.Pkey, app_path);
		profile_write_string(buf, TEXT("PopSSLPass"), (MailBox + j)->PopSSLInfo.Pass, app_path);
		// Disable RETR
		profile_write_int(buf, TEXT("NoRETR"), (MailBox + j)->NoRETR, app_path);
		// Disable UIDL
		profile_write_int(buf, TEXT("NoUIDL"), (MailBox + j)->NoUIDL, app_path);

		// MailCnt
		profile_write_int(buf, TEXT("MailCnt"), (MailBox + j)->MailCnt, app_path);
		// MailSize
		profile_write_int(buf, TEXT("MailSize"), (MailBox + j)->MailSize, app_path);

		profile_write_int(buf, TEXT("StartInit"), (MailBox + j)->StartInit, app_path);

		// LastMessageId
		if ((MailBox + j)->LastMessageId != NULL) {
#ifdef UNICODE
			char_to_tchar((MailBox + j)->LastMessageId, ret, BUF_SIZE - 1);
			*(ret + BUF_SIZE - 1) = TEXT('\0');
			profile_write_string(buf, TEXT("LastMessageId"), ret, app_path);
#else
			profile_write_string(buf, TEXT("LastMessageId"), (MailBox + j)->LastMessageId, app_path);
#endif
		} else {
			profile_delete_key(buf, TEXT("LastMessageId"));
		}
		// LastNo
		profile_write_int(buf, TEXT("LastNo"), (MailBox + j)->LastNo, app_path);

		// CyclicFlag
		profile_write_int(buf, TEXT("CyclicFlag"), (MailBox + j)->CyclicFlag, app_path);

		// SmtpServer
		profile_write_string(buf, TEXT("SmtpServer"), (MailBox + j)->SmtpServer, app_path);
		// SmtpPort
		profile_write_int(buf, TEXT("SmtpPort"), (MailBox + j)->SmtpPort, app_path);
		// UserName
		profile_write_string(buf, TEXT("UserName"), (MailBox + j)->UserName, app_path);
		// MailAddress
		profile_write_string(buf, TEXT("MailAddress"), (MailBox + j)->MailAddress, app_path);
		// Signature
		if ((MailBox + j)->Signature != NULL) {
			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen((MailBox + j)->Signature) * 2 + 1));
			if (p != NULL) {
				EncodeCtrlChar((MailBox + j)->Signature, p);
				profile_write_string(buf, TEXT("Signature"), p, app_path);
				mem_free(&p);
			}
		}
		// ReplyTo
		profile_write_string(buf, TEXT("ReplyTo"), (MailBox + j)->ReplyTo, app_path);
		// MyAddr2Bcc
		profile_write_int(buf, TEXT("MyAddr2Bcc"), (MailBox + j)->MyAddr2Bcc, app_path);
		// BccAddr
		profile_write_string(buf, TEXT("BccAddr"), (MailBox + j)->BccAddr, app_path);

		// POP before SMTP
		profile_write_int(buf, TEXT("PopBeforeSmtp"), (MailBox + j)->PopBeforeSmtp, app_path);
		// SMTP Authentication
		profile_write_int(buf, TEXT("SmtpAuth"), (MailBox + j)->SmtpAuth, app_path);
		// SMTP Authentication type
		profile_write_int(buf, TEXT("SmtpAuthType"), (MailBox + j)->SmtpAuthType, app_path);
		// SMTP Authentication User & Pass mode
		profile_write_int(buf, TEXT("AuthUserPass"), (MailBox + j)->AuthUserPass, app_path);
		// SMTP Authentication User
		profile_write_string(buf, TEXT("SmtpUser"), (MailBox + j)->SmtpUser, app_path);
		// SMTP Authentication Pass
		EncodePassword((MailBox + j)->SmtpUser, (MailBox + j)->SmtpPass, tmp, BUF_SIZE - 1, FALSE);
		profile_write_string(buf, TEXT("SmtpPass"), tmp, app_path);
		// SMTP SSL
		profile_write_int(buf, TEXT("SmtpSSL"), (MailBox + j)->SmtpSSL, app_path);
		// SMTP SSL Option
		profile_write_int(buf, TEXT("SmtpSSLType"), (MailBox + j)->SmtpSSLInfo.Type, app_path);
		profile_write_int(buf, TEXT("SmtpSSLVerify"), (MailBox + j)->SmtpSSLInfo.Verify, app_path);
		profile_write_int(buf, TEXT("SmtpSSLDepth"), (MailBox + j)->SmtpSSLInfo.Depth, app_path);
		profile_write_string(buf, TEXT("SmtpSSLCert"), (MailBox + j)->SmtpSSLInfo.Cert, app_path);
		profile_write_string(buf, TEXT("SmtpSSLPkey"), (MailBox + j)->SmtpSSLInfo.Pkey, app_path);
		profile_write_string(buf, TEXT("SmtpSSLPass"), (MailBox + j)->SmtpSSLInfo.Pass, app_path);

		// Filter
		profile_write_int(buf, TEXT("FilterEnable"), (MailBox + j)->FilterEnable, app_path);
		profile_write_int(buf, TEXT("FilterCnt"), (MailBox + j)->FilterCnt, app_path);

		for (t = 0; (MailBox + j)->tpFilter != NULL && t < (MailBox + j)->FilterCnt; t++) {
			tpFilter = *((MailBox + j)->tpFilter + t);
			if (tpFilter == NULL) {
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			profile_write_int(buf, key_buf, tpFilter->Enable, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
			profile_write_int(buf, key_buf, tpFilter->Action, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
			profile_write_string(buf, key_buf, tpFilter->SaveboxName, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			profile_write_string(buf, key_buf, tpFilter->Header1, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			profile_write_string(buf, key_buf, tpFilter->Content1, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
			profile_write_int(buf, key_buf, tpFilter->Boolean, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			profile_write_string(buf, key_buf, tpFilter->Header2, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			profile_write_string(buf, key_buf, tpFilter->Content2, app_path);
		}

		// GJC clear any further filter keys
		found = TRUE;
		while (found) {
			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("SaveboxName"));
			profile_delete_key(buf, key_buf); // may not exist in old versions

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Boolean"));
			profile_delete_key(buf, key_buf); // may not exist in old versions

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			found &= profile_delete_key(buf, key_buf);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			found &= profile_delete_key(buf, key_buf);

			t++;
		}

		// RAS
		profile_write_int(buf, TEXT("RasMode"), (MailBox + j)->RasMode, app_path);
		profile_write_string(buf, TEXT("RasEntry"), (MailBox + j)->RasEntry, app_path);
		profile_write_int(buf, TEXT("RasReCon"), (MailBox + j)->RasReCon, app_path);
	}
	// GJC clear keys from deleted mailboxes
	found = TRUE;
	j = MailBoxCnt;
	while (found) {
		wsprintf(buf, TEXT("MAILBOX-%d"), j - MAILBOX_USER);
		found = profile_clear_section(buf);
		j++;
	}

	if (profile_flush(app_path) == FALSE) {
		rc = FALSE;
	}
	profile_free();	 

	///////////// MRP /////////////////////
	DeleteFile(app_pathBackup);
	///////////// --- /////////////////////

	if (SaveMailFlag == FALSE) {
		// �ݒ�ۑ��̂�
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
		if (tpMailBox == NULL || tpMailBox->Loaded == FALSE
			|| (tpMailBox->NeedsSave == 0 && SaveAll == FALSE)) {
			continue;
		}
		if (op.WriteMbox != tpMailBox->WasMbox) {
			tpMailBox->NeedsSave |= MBOX_FORMAT_CHANGED;
		}
		//of mail inside mailbox Mail item
		if (tpMailBox->Filename == NULL) {
			wsprintf(buf, TEXT("MailBox%d.dat"), j - MAILBOX_USER);
		} else {
			lstrcpy(buf, tpMailBox->Filename);
		}
		if (file_save_mailbox(buf, SaveDir, MailBox + j, is_backup,
			(tpMailBox->Type == MAILBOX_TYPE_SAVE) ? 2 : op.ListSaveMode) == FALSE) {
			rc = FALSE;
		}
	}
	return rc;
}

/*
 * ini_free - �ݒ�����������
 */
void ini_free(void)
{
	mem_free(&op.BackupDir);
	mem_free(&op.LvColumnOrder);
	mem_free(&op.view_font.name);
	mem_free(&op.lv_font.name);
	mem_free(&op.SendHelo);
	mem_free(&op.CAFile);
	mem_free(&op.QuotationChar);
	mem_free(&op.ReSubject);
	mem_free(&op.FwdSubject);
	mem_free(&op.ReHeader);
	mem_free(&op.FwdHeader);
	mem_free(&op.AltReplyTo);
	mem_free(&op.Bura);
	mem_free(&op.Oida);
	mem_free(&op.HeadCharset);
	mem_free(&op.BodyCharset);
	mem_free(&op.TimeZone);
	mem_free(&op.DateFormat);
	mem_free(&op.TimeFormat);
	mem_free(&op.NewMailSoundFile);
	mem_free(&op.ExecEndSoundFile);
	mem_free(&op.ViewApp);
	mem_free(&op.ViewAppCmdLine);
	mem_free(&op.ViewFileSuffix);
	mem_free(&op.ViewFileHeader);
	mem_free(&op.EditApp);
	mem_free(&op.EditAppCmdLine);
	mem_free(&op.EditFileSuffix);
#ifdef _WIN32_WCE_PPC
	mem_free(&op.OpenSaveDir);
#endif
	mem_free(&op.URLApp);
	mem_free(&op.AttachPath);
	mem_free(&op.Password);
}
/* End of source */