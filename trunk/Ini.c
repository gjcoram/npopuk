/**************************************************************************

	nPOP

	Ini.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"
#include "Profile.h"


/**************************************************************************
	Define
**************************************************************************/

#define GENERAL				TEXT("GENERAL")

#define INI_BUF_SIZE		1024


/**************************************************************************
	Global Variables
**************************************************************************/

int StertPass;
int ShowPass;
TCHAR *Password;

TCHAR DataFileDir[BUF_SIZE];

TCHAR *FontName;
int FontSize;
int FontCharset;

#ifndef _WIN32_WCE
RECT MainRect;
RECT ViewRect;
RECT EditRect;
#endif

int ShowTrayIcon;
int StartHide;
int MinsizeHide;
int CloseHide;
int TrayIconToggle;
int StartInit;
int SocLog;

int LvDefSelectPos;
int LvAutoSort;
int LvSortItem;
int LvThreadView;
int LvStyle;
int LvStyleEx;
TCHAR *LvFontName;
int LvFontSize;
int LvFontCharset;
int MoveAllMailBox;
int RecvScroll;
int SaveMsg;
int AutoSave;
int LvColSize[LV_COL_CNT];
int AddColSize[AD_COL_CNT];

int ListGetLine;
int ListDownload;
int ShowHeader;
int ListSaveMode;
int WordBreakFlag;
int EditWordBreakFlag;
int ViewShowDate;
int MstchCase;
int AllFind;
int SubjectFind;

int ESMTP;
TCHAR *SendHelo;
int SendMessageId;
int SendDate;
int SelectSendBox;
int PopBeforeSmtpIsLoginOnly;
int PopBeforeSmtpWait;

TCHAR *HeadCharset;
int HeadEncoding;
TCHAR *BodyCharset;
int BodyEncoding;

int AutoQuotation;
TCHAR *QuotationChar;
int WordBreakSize;
int QuotationBreak;
TCHAR *ReSubject;
TCHAR *ReHeader;
TCHAR *Bura;
TCHAR *Oida;
TCHAR *sBura;
TCHAR *sOida;

int IPCache;
int EncodeType;
TCHAR *TimeZone;
TCHAR *DateFormat;
TCHAR *TimeFormat;

int ShowNewMailMessgae;
int ShowNoMailMessage;
int ActiveNewMailMessgae;

int NewMailSound;
TCHAR *NewMailSoundFile;
int ExecEndSound;
TCHAR *ExecEndSoundFile;

int AutoCheck;
int AutoCheckTime;
int StartCheck;
int CheckAfterUpdate;
int SocIgnoreError;
int SendIgnoreError;
int CheckEndExec;
int CheckEndExecNoDelMsg;
int TimeoutInterval;

int ViewClose;
TCHAR *ViewApp;
TCHAR *ViewAppCmdLine;
TCHAR *ViewFileSuffix;
TCHAR *ViewFileHeader;
int ViewAppClose;
int DefViewApp;
TCHAR *EditApp;
TCHAR *EditAppCmdLine;
TCHAR *EditFileSuffix;
int DefEditApp;

TCHAR *URLApp;

int EnableLAN;

int RasCon;
int RasCheckEndDisCon;
int RasEndDisCon;
int RasNoCheck;
int RasWaitSec;
struct TPRASINFO **RasInfo;
int RasInfoCnt;

//外部参照
extern HINSTANCE hInst;
extern TCHAR *g_Pass;
extern int gPassSt;
extern TCHAR *AppDir;
extern TCHAR *DataDir;
extern struct TPMAILBOX *MailBox;
extern int MailBoxCnt;
extern BOOL first_start;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static TCHAR *GetAllocIniString(TCHAR *SubKey, TCHAR *Name, TCHAR *Def, TCHAR *AppPath);


/******************************************************************************

	FreeIniInfo

	設定情報を解放する

******************************************************************************/

void FreeIniInfo(void)
{
	NULLCHECK_FREE(FontName);
	NULLCHECK_FREE(LvFontName);
	NULLCHECK_FREE(SendHelo);
	NULLCHECK_FREE(QuotationChar);
	NULLCHECK_FREE(ReSubject);
	NULLCHECK_FREE(ReHeader);
	NULLCHECK_FREE(Bura);
	NULLCHECK_FREE(Oida);
	NULLCHECK_FREE(sBura);
	NULLCHECK_FREE(sOida);
	NULLCHECK_FREE(HeadCharset);
	NULLCHECK_FREE(BodyCharset);
	NULLCHECK_FREE(TimeZone);
	NULLCHECK_FREE(DateFormat);
	NULLCHECK_FREE(TimeFormat);
	NULLCHECK_FREE(NewMailSoundFile);
	NULLCHECK_FREE(ExecEndSoundFile);
	NULLCHECK_FREE(ViewApp);
	NULLCHECK_FREE(ViewAppCmdLine);
	NULLCHECK_FREE(ViewFileSuffix);
	NULLCHECK_FREE(ViewFileHeader);
	NULLCHECK_FREE(EditApp);
	NULLCHECK_FREE(EditAppCmdLine);
	NULLCHECK_FREE(EditFileSuffix);
	NULLCHECK_FREE(URLApp);
#ifndef _WIN32_WCE
	NULLCHECK_FREE(Password);
#endif
}


/******************************************************************************

	GetAllocIniString

	設定情報から文字列を取得してバッファのアドレスを返す

******************************************************************************/

static TCHAR *GetAllocIniString(TCHAR *SubKey, TCHAR *Name, TCHAR *Def, TCHAR *AppPath)
{
	TCHAR ret[BUF_SIZE];
	TCHAR *buf;
	long len;

	len = Profile_GetString(SubKey, Name, Def, ret, BUF_SIZE - 1, AppPath);
	buf = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(buf != NULL){
		lstrcpy(buf, ret);
	}
	return buf;
}


/******************************************************************************

	CheckStartPass

	起動時のパスワード

******************************************************************************/

#ifndef _WIN32_WCE
BOOL CheckStartPass(void)
{
	TCHAR app_path[BUF_SIZE];
	TCHAR ret[BUF_SIZE];
	TCHAR pass[BUF_SIZE];

	TStrJoin(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	Profile_Initialize(app_path, TRUE);

	StertPass = Profile_GetInt(GENERAL, TEXT("StertPass"), 0, app_path);
	if(StertPass == 1){
		Profile_GetString(GENERAL, TEXT("pw"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword(TEXT("_pw_"), ret, pass, BUF_SIZE - 1, TRUE);
		if(*pass == TEXT('\0')){
			Profile_Free();
			return TRUE;
		}
		while(1){
			//起動パスワード
			gPassSt = 0;
			if(DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_INPUTPASS), NULL, InputPassProc,
				(LPARAM)STR_TITLE_STARTPASSWORD) == FALSE){
				Profile_Free();
				return FALSE;
			}
			if(g_Pass == NULL || lstrcmp(pass, g_Pass) != 0){
				ErrorMessage(NULL, STR_ERR_SOCK_BADPASSWORD);
				continue;
			}
			break;
		}
	}
	Profile_Free();
	return TRUE;
}
#endif


/******************************************************************************

	GetINI

	INIファイルから設定情報を読みこむ

******************************************************************************/

BOOL GetINI(HWND hWnd)
{
	struct TPFILTER *tpFilter;
	HDC hdc;
	TCHAR app_path[BUF_SIZE];
	TCHAR buf[BUF_SIZE];
	TCHAR key_buf[BUF_SIZE];
	TCHAR conv_buf[INI_BUF_SIZE];
	TCHAR ret[BUF_SIZE];
	TCHAR tmp[BUF_SIZE];
	TCHAR *p, *r;
	UINT char_set;
	int i, j, t, cnt;
	int len;
	int fDef;

	hdc = GetDC(hWnd);
	char_set = GetTextCharset(hdc);
	ReleaseDC(hWnd, hdc);

	TStrJoin(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	Profile_Initialize(app_path, TRUE);

	len = Profile_GetString(GENERAL, TEXT("DataFileDir"), TEXT(""), DataFileDir, BUF_SIZE - 1, app_path);
	if(*DataFileDir == TEXT('\0')){
		DataDir = AppDir;
	}else{
		DataDir = DataFileDir;
		for(p = r = DataDir; *p != TEXT('\0'); p++){
#ifndef UNICODE
			if(IsDBCSLeadByte((BYTE)*p) == TRUE){
				p++;
				continue;
			}
#endif
			if(*p == TEXT('\\') || *p == TEXT('/')){
				r = p;
			}
		}
		if(r != (DataDir + lstrlen(DataDir) - 1) || lstrlen(DataDir) == 1){
			lstrcat(DataDir, TEXT("\\"));
		}
	}

	SocLog = Profile_GetInt(GENERAL, TEXT("SocLog"), 0, app_path);

	FontName = GetAllocIniString(GENERAL, TEXT("FontName"), STR_DEFAULT_FONT, app_path);
	FontSize = Profile_GetInt(GENERAL, TEXT("FontSize"), 9, app_path);
	FontCharset = Profile_GetInt(GENERAL, TEXT("FontCharset"), char_set, app_path);
	LvFontName = GetAllocIniString(GENERAL, TEXT("LvFontName"), TEXT(""), app_path);
	LvFontSize = Profile_GetInt(GENERAL, TEXT("LvFontSize"), 9, app_path);
	LvFontCharset = Profile_GetInt(GENERAL, TEXT("LvFontCharset"), char_set, app_path);
	HeadCharset = GetAllocIniString(GENERAL, TEXT("HeadCharset"), STR_DEFAULT_HEAD_CHARSET, app_path);
	HeadEncoding = Profile_GetInt(GENERAL, TEXT("HeadEncoding"), STR_DEFAULT_HEAD_ENCODE, app_path);
	BodyCharset = GetAllocIniString(GENERAL, TEXT("BodyCharset"), STR_DEFAULT_BODY_CHARSET, app_path);
	BodyEncoding = Profile_GetInt(GENERAL, TEXT("BodyEncoding"), STR_DEFAULT_BODY_ENCODE, app_path);
	TimeZone = GetAllocIniString(GENERAL, TEXT("TimeZone"), TEXT(""), app_path);
	DateFormat = GetAllocIniString(GENERAL, TEXT("DateFormat"), STR_DEFAULT_DATEFORMAT, app_path);
	TimeFormat = GetAllocIniString(GENERAL, TEXT("TimeFormat"), STR_DEFAULT_TIMEFORMAT, app_path);

#ifndef _WIN32_WCE
	MainRect.left = Profile_GetInt(GENERAL, TEXT("left"), 0, app_path);
	MainRect.top = Profile_GetInt(GENERAL, TEXT("top"), 0, app_path);
	MainRect.right = Profile_GetInt(GENERAL, TEXT("right"), 440, app_path);
	MainRect.bottom = Profile_GetInt(GENERAL, TEXT("bottom"), 320, app_path);
#endif

	ShowTrayIcon = Profile_GetInt(GENERAL, TEXT("ShowTrayIcon"), 1, app_path);
	StartHide = Profile_GetInt(GENERAL, TEXT("StartHide"), 0, app_path);
	MinsizeHide = Profile_GetInt(GENERAL, TEXT("MinsizeHide"), 0, app_path);
	CloseHide = Profile_GetInt(GENERAL, TEXT("CloseHide"), 0, app_path);
	TrayIconToggle = Profile_GetInt(GENERAL, TEXT("TrayIconToggle"), 0, app_path);
	StartInit = Profile_GetInt(GENERAL, TEXT("StartInit"), 0, app_path);

	LvDefSelectPos = Profile_GetInt(GENERAL, TEXT("LvDefSelectPos"), 1, app_path);
	LvAutoSort = Profile_GetInt(GENERAL, TEXT("LvAutoSort"), 1, app_path);
	LvSortItem = Profile_GetInt(GENERAL, TEXT("LvSortItem"), 3, app_path);
	LvThreadView = Profile_GetInt(GENERAL, TEXT("LvThreadView"), 0, app_path);
	LvStyle = Profile_GetInt(GENERAL, TEXT("LvStyle"), LVS_SHOWSELALWAYS | LVS_REPORT, app_path);
	LvStyleEx = Profile_GetInt(GENERAL, TEXT("LvStyleEx"), LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, app_path);
	MoveAllMailBox = Profile_GetInt(GENERAL, TEXT("MoveAllMailBox"), 1, app_path);
	RecvScroll = Profile_GetInt(GENERAL, TEXT("RecvScroll"), 1, app_path);
	SaveMsg = Profile_GetInt(GENERAL, TEXT("SaveMsg"), 1, app_path);
	AutoSave = Profile_GetInt(GENERAL, TEXT("AutoSave"), 1, app_path);

	StertPass = Profile_GetInt(GENERAL, TEXT("StertPass"), 0, app_path);
	ShowPass = Profile_GetInt(GENERAL, TEXT("ShowPass"), 0, app_path);
	Profile_GetString(GENERAL, TEXT("pw"), TEXT(""), ret, BUF_SIZE - 1, app_path);
	EncodePassword(TEXT("_pw_"), ret, tmp, BUF_SIZE - 1, TRUE);
	Password = AllocCopy(tmp);

	LvColSize[0] = Profile_GetInt(GENERAL, TEXT("LvColSize-0"), 150, app_path);
	LvColSize[1] = Profile_GetInt(GENERAL, TEXT("LvColSize-1"), 100, app_path);
	LvColSize[2] = Profile_GetInt(GENERAL, TEXT("LvColSize-2"), 110, app_path);
	LvColSize[3] = Profile_GetInt(GENERAL, TEXT("LvColSize-3"), 50, app_path);

#ifdef _WIN32_WCE
	AddColSize[0] = Profile_GetInt(GENERAL, TEXT("AddColSize-0"), 100, app_path);
	AddColSize[1] = Profile_GetInt(GENERAL, TEXT("AddColSize-1"), 100, app_path);
#else
	AddColSize[0] = Profile_GetInt(GENERAL, TEXT("AddColSize-0"), 250, app_path);
	AddColSize[1] = Profile_GetInt(GENERAL, TEXT("AddColSize-1"), 190, app_path);
#endif

#ifndef _WIN32_WCE
	ViewRect.left = Profile_GetInt(GENERAL, TEXT("viewleft"), 0, app_path);
	ViewRect.top = Profile_GetInt(GENERAL, TEXT("viewtop"), 0, app_path);
	ViewRect.right = Profile_GetInt(GENERAL, TEXT("viewright"), 450, app_path);
	ViewRect.bottom = Profile_GetInt(GENERAL, TEXT("viewbottom"), 400, app_path);

	EditRect.left = Profile_GetInt(GENERAL, TEXT("editleft"), 0, app_path);
	EditRect.top = Profile_GetInt(GENERAL, TEXT("edittop"), 0, app_path);
	EditRect.right = Profile_GetInt(GENERAL, TEXT("editright"), 450, app_path);
	EditRect.bottom = Profile_GetInt(GENERAL, TEXT("editbottom"), 400, app_path);
#endif

	ShowHeader = Profile_GetInt(GENERAL, TEXT("ShowHeader"), 0, app_path);
	ListGetLine = Profile_GetInt(GENERAL, TEXT("ListGetLine"), 100, app_path);
	ListDownload = Profile_GetInt(GENERAL, TEXT("ListDownload"), 0, app_path);
	ListSaveMode = Profile_GetInt(GENERAL, TEXT("ListSaveMode"), 2, app_path);
	WordBreakFlag = Profile_GetInt(GENERAL, TEXT("WordBreakFlag"), 1, app_path);
	EditWordBreakFlag = Profile_GetInt(GENERAL, TEXT("EditWordBreakFlag"), 1, app_path);
	ViewShowDate = Profile_GetInt(GENERAL, TEXT("ViewShowDate"), 0, app_path);
	MstchCase = Profile_GetInt(GENERAL, TEXT("MstchCase"), 0, app_path);
	AllFind = Profile_GetInt(GENERAL, TEXT("AllFind"), 1, app_path);
	SubjectFind = Profile_GetInt(GENERAL, TEXT("SubjectFind"), 0, app_path);

	ESMTP = Profile_GetInt(GENERAL, TEXT("ESMTP"), 0, app_path);
	SendHelo = GetAllocIniString(GENERAL, TEXT("SendHelo"), TEXT(""), app_path);
	SendMessageId = Profile_GetInt(GENERAL, TEXT("SendMessageId"), 1, app_path);
	SendDate = Profile_GetInt(GENERAL, TEXT("SendDate"), 1, app_path);
	SelectSendBox = Profile_GetInt(GENERAL, TEXT("SelectSendBox"), 1, app_path);
	PopBeforeSmtpIsLoginOnly = Profile_GetInt(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), 1, app_path);
	PopBeforeSmtpWait = Profile_GetInt(GENERAL, TEXT("PopBeforeSmtpWait"), 300, app_path);

	AutoQuotation = Profile_GetInt(GENERAL, TEXT("AutoQuotation"), 1, app_path);
	QuotationChar = GetAllocIniString(GENERAL, TEXT("QuotationChar"), TEXT(">"), app_path);
	WordBreakSize = Profile_GetInt(GENERAL, TEXT("WordBreakSize"), 70, app_path);
	QuotationBreak = Profile_GetInt(GENERAL, TEXT("QuotationBreak"), 1, app_path);
	ReSubject = GetAllocIniString(GENERAL, TEXT("ReSubject"), TEXT("Re: "), app_path);
	len = Profile_GetString(GENERAL, TEXT("ReHeader"), TEXT("\\n%f wrote:\\n(%d)\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
	ReHeader = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(ReHeader != NULL){
		DecodeCtrlChar(conv_buf, ReHeader);
	}

	Bura = GetAllocIniString(GENERAL, TEXT("Bura"), STR_DEFAULT_BURA, app_path);
	Oida = GetAllocIniString(GENERAL, TEXT("Oida"), STR_DEFAULT_OIDA, app_path);

	sBura = GetAllocIniString(GENERAL, TEXT("sBura"), TEXT(",.?!%:;)]}｣｡､ﾞﾟ"), app_path);
	sOida = GetAllocIniString(GENERAL, TEXT("sOida"), TEXT("\\$([{｢"), app_path);

	IPCache = Profile_GetInt(GENERAL, TEXT("IPCache"), 1, app_path);
	EncodeType = Profile_GetInt(GENERAL, TEXT("EncodeType"), 0, app_path);

	ShowNewMailMessgae = Profile_GetInt(GENERAL, TEXT("ShowNewMailMessgae"), 1, app_path);
	ShowNoMailMessage = Profile_GetInt(GENERAL, TEXT("ShowNoMailMessage"), 0, app_path);
#ifdef _WIN32_WCE
	ActiveNewMailMessgae = Profile_GetInt(GENERAL, TEXT("ActiveNewMailMessgae"), 1, app_path);
#else
	ActiveNewMailMessgae = Profile_GetInt(GENERAL, TEXT("ActiveNewMailMessgae"), 0, app_path);
#endif

	NewMailSound = Profile_GetInt(GENERAL, TEXT("NewMailSound"), 1, app_path);
	NewMailSoundFile = GetAllocIniString(GENERAL, TEXT("NewMailSoundFile"), TEXT(""), app_path);
	ExecEndSound = Profile_GetInt(GENERAL, TEXT("ExecEndSound"), 0, app_path);
	ExecEndSoundFile = GetAllocIniString(GENERAL, TEXT("ExecEndSoundFile"), TEXT(""), app_path);

	AutoCheck = Profile_GetInt(GENERAL, TEXT("AutoCheck"), 0, app_path);
	AutoCheckTime = Profile_GetInt(GENERAL, TEXT("AutoCheckTime"), 10, app_path);
	StartCheck = Profile_GetInt(GENERAL, TEXT("StartCheck"), 0, app_path);
	CheckAfterUpdate = Profile_GetInt(GENERAL, TEXT("CheckAfterUpdate"), 0, app_path);
	SocIgnoreError = Profile_GetInt(GENERAL, TEXT("SocIgnoreError"), 0, app_path);
	SendIgnoreError = Profile_GetInt(GENERAL, TEXT("SendIgnoreError"), 0, app_path);
	CheckEndExec = Profile_GetInt(GENERAL, TEXT("CheckEndExec"), 0, app_path);
	CheckEndExecNoDelMsg = Profile_GetInt(GENERAL, TEXT("CheckEndExecNoDelMsg"), 1, app_path);
	TimeoutInterval = Profile_GetInt(GENERAL, TEXT("TimeoutInterval"), 3, app_path);
	if(TimeoutInterval <= 0) TimeoutInterval = 1;

	ViewClose = Profile_GetInt(GENERAL, TEXT("ViewClose"), 1, app_path);
	ViewApp = GetAllocIniString(GENERAL, TEXT("ViewApp"), TEXT(""), app_path);
	ViewAppCmdLine = GetAllocIniString(GENERAL, TEXT("ViewAppCmdLine"), TEXT(""), app_path);
	ViewFileSuffix = GetAllocIniString(GENERAL, TEXT("ViewFileSuffix"), TEXT("txt"), app_path);
	len = Profile_GetString(GENERAL, TEXT("ViewFileHeader"),
		TEXT("From: %f\\nTo: %t\\nCc: %c\\nSubject: %s\\nDate: %d\\n\\n"), conv_buf, INI_BUF_SIZE - 1, app_path);
	ViewFileHeader = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(ViewFileHeader != NULL){
		DecodeCtrlChar(conv_buf, ViewFileHeader);
	}
	ViewAppClose = Profile_GetInt(GENERAL, TEXT("ViewAppClose"), 0, app_path);
	DefViewApp = Profile_GetInt(GENERAL, TEXT("DefViewApp"), 0, app_path);
	EditApp = GetAllocIniString(GENERAL, TEXT("EditApp"), TEXT(""), app_path);
	EditAppCmdLine = GetAllocIniString(GENERAL, TEXT("EditAppCmdLine"), TEXT(""), app_path);
	EditFileSuffix = GetAllocIniString(GENERAL, TEXT("EditFileSuffix"), TEXT("txt"), app_path);
	DefEditApp = Profile_GetInt(GENERAL, TEXT("DefEditApp"), 0, app_path);

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_LAGENDA
	URLApp = GetAllocIniString(GENERAL, TEXT("URLApp"), TEXT("internet.exe"), app_path);
#else	//_WIN32_WCE_LAGENDA
	URLApp = GetAllocIniString(GENERAL, TEXT("URLApp"), TEXT("iexplore.exe"), app_path);
#endif	//_WIN32_WCE_LAGENDA
#else	//_WIN32_WCE
	URLApp = GetAllocIniString(GENERAL, TEXT("URLApp"), TEXT(""), app_path);
#endif	//_WIN32_WCE

	EnableLAN = Profile_GetInt(GENERAL, TEXT("EnableLAN"), 0, app_path);

	RasCon = Profile_GetInt(GENERAL, TEXT("RasCon"), 1, app_path);
	RasCheckEndDisCon = Profile_GetInt(GENERAL, TEXT("RasCheckEndDisCon"), 1, app_path);
	RasEndDisCon = Profile_GetInt(GENERAL, TEXT("RasEndDisCon"), 1, app_path);
	RasNoCheck = Profile_GetInt(GENERAL, TEXT("RasNoCheck"), 1, app_path);
	RasWaitSec = Profile_GetInt(GENERAL, TEXT("RasWaitSec"), 5, app_path);

	RasInfoCnt = Profile_GetInt(GENERAL, TEXT("RasInfoCnt"), 0, app_path);
	RasInfo = (struct TPRASINFO **)LocalAlloc(LPTR, sizeof(struct TPRASINFO *) * RasInfoCnt);
	if(RasInfo == NULL){
		RasInfoCnt = 0;
	}
	for(j = 0; j < RasInfoCnt; j++){
		*(RasInfo + j) = (struct TPRASINFO *)LocalAlloc(LPTR, sizeof(struct TPRASINFO));
		if(*(RasInfo + j) == NULL){
			continue;
		}
		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasEntry"));
		(*(RasInfo + j))->RasEntry = GetAllocIniString(TEXT("RASINFO"), key_buf, TEXT(""), app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasUser"));
		(*(RasInfo + j))->RasUser = GetAllocIniString(TEXT("RASINFO"), key_buf, TEXT(""), app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), j, TEXT("RasPass"));
		len = Profile_GetString(TEXT("RASINFO"), key_buf, TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((*(RasInfo + j))->RasUser, ret, tmp, BUF_SIZE - 1, TRUE);
		(*(RasInfo + j))->RasPass = AllocCopy(tmp);
	}

	i = Profile_GetInt(GENERAL, TEXT("MailBoxCnt"), 0, app_path);
	if(i == 0){
		CreateMailBox(hWnd, FALSE);
		first_start = TRUE;
		return TRUE;
	}
	for(j = 0; j < i; j++){
		if((cnt = CreateMailBox(hWnd, FALSE)) == -1){
			continue;
		}
		wsprintf(buf, TEXT("MAILBOX-%d"), j);

		//Name
		(MailBox + cnt)->Name = GetAllocIniString(buf, TEXT("Name"), TEXT(""), app_path);
		//Server
		(MailBox + cnt)->Server = GetAllocIniString(buf, TEXT("Server"), TEXT(""), app_path);
		//Port
		(MailBox + cnt)->Port = Profile_GetInt(buf, TEXT("Port"), 110, app_path);
		//User
		(MailBox + cnt)->User = GetAllocIniString(buf, TEXT("User"), TEXT(""), app_path);
		//Pass
		Profile_GetString(buf, TEXT("Pass"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((MailBox + cnt)->User, ret, tmp, BUF_SIZE - 1, TRUE);
		(MailBox + cnt)->Pass = AllocCopy(tmp);
		//APOP
		(MailBox + cnt)->APOP = Profile_GetInt(buf, TEXT("APOP"), 0, app_path);

		//MailCnt
		(MailBox + cnt)->MailCnt = Profile_GetInt(buf, TEXT("MailCnt"), 0, app_path);
		//MailSize
		(MailBox + cnt)->MailSize = Profile_GetInt(buf, TEXT("MailSize"), 0, app_path);

		if(StartInit == 0){
			//LastMessageId
			Profile_GetString(buf, TEXT("LastMessageId"), TEXT(""), ret, BUF_SIZE - 1, app_path);
			(MailBox + cnt)->LastMessageId = AllocTcharToChar(ret);
			//LastNo
			(MailBox + cnt)->LastNo = Profile_GetInt(buf, TEXT("LastNo"), 0, app_path);
		}else{
			//起動時に新着位置の初期化
			(MailBox + cnt)->LastNo = -1;
		}

		//CyclicFlag
		(MailBox + cnt)->CyclicFlag = Profile_GetInt(buf, TEXT("CyclicFlag"), 0, app_path);

		//SmtpServer
		(MailBox + cnt)->SmtpServer = GetAllocIniString(buf, TEXT("SmtpServer"), TEXT(""), app_path);
		//SmtpPort
		(MailBox + cnt)->SmtpPort = Profile_GetInt(buf, TEXT("SmtpPort"), 25, app_path);
		//UserName
		(MailBox + cnt)->UserName = GetAllocIniString(buf, TEXT("UserName"), TEXT(""), app_path);
		//MailAddress
		(MailBox + cnt)->MailAddress = GetAllocIniString(buf, TEXT("MailAddress"), TEXT(""), app_path);
		//Signature
		p = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * MAXSIZE);
		if(p != NULL){
			len = Profile_GetString(buf, TEXT("Signature"), TEXT(""), p, MAXSIZE - 1, app_path);
			(MailBox + cnt)->Signature = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
			if((MailBox + cnt)->Signature != NULL){
				DecodeCtrlChar(p, (MailBox + cnt)->Signature);
			}
			LocalFree(p);
		}
		//ReplyTo
		(MailBox + cnt)->ReplyTo = GetAllocIniString(buf, TEXT("ReplyTo"), TEXT(""), app_path);
		//MyAddr2Bcc
		(MailBox + cnt)->MyAddr2Bcc = Profile_GetInt(buf, TEXT("MyAddr2Bcc"), 0, app_path);
		//BccAddr
		(MailBox + cnt)->BccAddr = GetAllocIniString(buf, TEXT("BccAddr"), TEXT(""), app_path);

		//POP before SMTP
		(MailBox + cnt)->PopBeforeSmtp = Profile_GetInt(buf, TEXT("PopBeforeSmtp"), 0, app_path);
		//SMTP Authentication
		(MailBox + cnt)->SmtpAuth = Profile_GetInt(buf, TEXT("SmtpAuth"), 0, app_path);
		//SMTP Authentication type
		(MailBox + cnt)->SmtpAuthType = Profile_GetInt(buf, TEXT("SmtpAuthType"), 0, app_path);
		//SMTP Authentication User & Pass mode
		(MailBox + cnt)->AuthUserPass = Profile_GetInt(buf, TEXT("AuthUserPass"), 0, app_path);
		//SMTP Authentication User
		(MailBox + cnt)->SmtpUser = GetAllocIniString(buf, TEXT("SmtpUser"), TEXT(""), app_path);
		//SMTP Authentication Pass
		Profile_GetString(buf, TEXT("SmtpPass"), TEXT(""), ret, BUF_SIZE - 1, app_path);
		EncodePassword((MailBox + cnt)->SmtpUser, ret, tmp, BUF_SIZE - 1, TRUE);
		(MailBox + cnt)->SmtpPass = AllocCopy(tmp);

		//Filter
		(MailBox + cnt)->FilterEnable = Profile_GetInt(buf, TEXT("FilterEnable"), 0, app_path);
		(MailBox + cnt)->FilterCnt = Profile_GetInt(buf, TEXT("FilterCnt"), 0, app_path);
		fDef = Profile_GetInt(buf, TEXT("FilterFlag"), -1, app_path);

		(MailBox + cnt)->tpFilter = (struct TPFILTER **)LocalAlloc(LPTR, sizeof(struct TPFILTER *) * (MailBox + cnt)->FilterCnt);
		if((MailBox + cnt)->tpFilter == NULL){
			(MailBox + cnt)->FilterCnt = 0;
		}
		for(t = 0; t < (MailBox + cnt)->FilterCnt; t++){
			tpFilter = *((MailBox + cnt)->tpFilter + t) = (struct TPFILTER *)LocalAlloc(LPTR, sizeof(struct TPFILTER));
			if(tpFilter == NULL){
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			tpFilter->Enable = Profile_GetInt(buf, key_buf, 0, app_path);

			if(fDef == -1){
				wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
				tpFilter->Action = Profile_GetInt(buf, key_buf, 0, app_path);
			}else{
				tpFilter->Action = fDef;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			tpFilter->Header1 = GetAllocIniString(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			tpFilter->Content1 = GetAllocIniString(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			tpFilter->Header2 = GetAllocIniString(buf, key_buf, TEXT(""), app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			tpFilter->Content2 = GetAllocIniString(buf, key_buf, TEXT(""), app_path);
		}

		//RAS
		(MailBox + cnt)->RasMode = Profile_GetInt(buf, TEXT("RasMode"), 0, app_path);
		(MailBox + cnt)->RasEntry = GetAllocIniString(buf, TEXT("RasEntry"), TEXT(""), app_path);
		(MailBox + cnt)->RasReCon = Profile_GetInt(buf, TEXT("RasReCon"), 0, app_path);

		//メールアイテム
		wsprintf(buf, TEXT("MailBox%d.dat"), j);
		if(ReadItemList(buf, (MailBox + cnt)) == FALSE){
			Profile_Free();
			return FALSE;
		}
	}
	Profile_Free();
	return TRUE;
}


/******************************************************************************

	PutINI

	INIファイルへ設定情報を書き出す

******************************************************************************/

BOOL PutINI(HWND hWnd, BOOL SaveMailFlag)
{
	struct TPFILTER *tpFilter;
	TCHAR app_path[BUF_SIZE];
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

	TStrJoin(app_path, AppDir, KEY_NAME TEXT(".ini"), (TCHAR *)-1);
	Profile_Initialize(app_path, TRUE);

	Profile_WriteString(GENERAL, TEXT("DataFileDir"), DataFileDir, app_path);
	Profile_WriteInt(GENERAL, TEXT("SocLog"), SocLog, app_path);

	Profile_WriteString(GENERAL, TEXT("FontName"), FontName, app_path);
	Profile_WriteInt(GENERAL, TEXT("FontSize"), FontSize, app_path);
	Profile_WriteInt(GENERAL, TEXT("FontCharset"), FontCharset, app_path);
	Profile_WriteString(GENERAL, TEXT("LvFontName"), LvFontName, app_path);
	Profile_WriteInt(GENERAL, TEXT("LvFontSize"), LvFontSize, app_path);
	Profile_WriteInt(GENERAL, TEXT("LvFontCharset"), LvFontCharset, app_path);
	Profile_WriteString(GENERAL, TEXT("HeadCharset"), HeadCharset, app_path);
	Profile_WriteInt(GENERAL, TEXT("HeadEncoding"), HeadEncoding, app_path);
	Profile_WriteString(GENERAL, TEXT("BodyCharset"), BodyCharset, app_path);
	Profile_WriteInt(GENERAL, TEXT("BodyEncoding"), BodyEncoding, app_path);
	Profile_WriteString(GENERAL, TEXT("TimeZone"), TimeZone, app_path);
	Profile_WriteString(GENERAL, TEXT("DateFormat"), DateFormat, app_path);
	Profile_WriteString(GENERAL, TEXT("TimeFormat"), TimeFormat, app_path);

#ifndef _WIN32_WCE
	Profile_WriteInt(GENERAL, TEXT("left"), MainRect.left, app_path);
	Profile_WriteInt(GENERAL, TEXT("top"), MainRect.top, app_path);
	Profile_WriteInt(GENERAL, TEXT("right"), MainRect.right, app_path);
	Profile_WriteInt(GENERAL, TEXT("bottom"), MainRect.bottom, app_path);
#endif

	Profile_WriteInt(GENERAL, TEXT("ShowTrayIcon"), ShowTrayIcon, app_path);
	Profile_WriteInt(GENERAL, TEXT("StartHide"), StartHide, app_path);
	Profile_WriteInt(GENERAL, TEXT("MinsizeHide"), MinsizeHide, app_path);
	Profile_WriteInt(GENERAL, TEXT("CloseHide"), CloseHide, app_path);
	Profile_WriteInt(GENERAL, TEXT("TrayIconToggle"), TrayIconToggle, app_path);
	Profile_WriteInt(GENERAL, TEXT("StartInit"), StartInit, app_path);

	Profile_WriteInt(GENERAL, TEXT("LvDefSelectPos"), LvDefSelectPos, app_path);
	Profile_WriteInt(GENERAL, TEXT("LvAutoSort"), LvAutoSort, app_path);
	Profile_WriteInt(GENERAL, TEXT("LvSortItem"), LvSortItem, app_path);
	Profile_WriteInt(GENERAL, TEXT("LvThreadView"), LvThreadView, app_path);
	Profile_WriteInt(GENERAL, TEXT("LvStyle"), LvStyle, app_path);
	Profile_WriteInt(GENERAL, TEXT("LvStyleEx"), LvStyleEx, app_path);
	Profile_WriteInt(GENERAL, TEXT("MoveAllMailBox"), MoveAllMailBox, app_path);
	Profile_WriteInt(GENERAL, TEXT("RecvScroll"), RecvScroll, app_path);
	Profile_WriteInt(GENERAL, TEXT("SaveMsg"), SaveMsg, app_path);
	Profile_WriteInt(GENERAL, TEXT("AutoSave"), AutoSave, app_path);
	Profile_WriteInt(GENERAL, TEXT("StertPass"), StertPass, app_path);
	Profile_WriteInt(GENERAL, TEXT("ShowPass"), ShowPass, app_path);
	EncodePassword(TEXT("_pw_"), Password, tmp, BUF_SIZE - 1, FALSE);
	Profile_WriteString(GENERAL, TEXT("pw"), tmp, app_path);

	Profile_WriteInt(GENERAL, TEXT("LvColSize-0"), LvColSize[0], app_path);
	Profile_WriteInt(GENERAL, TEXT("LvColSize-1"), LvColSize[1], app_path);
	Profile_WriteInt(GENERAL, TEXT("LvColSize-2"), LvColSize[2], app_path);
	Profile_WriteInt(GENERAL, TEXT("LvColSize-3"), LvColSize[3], app_path);

	Profile_WriteInt(GENERAL, TEXT("AddColSize-0"), AddColSize[0], app_path);
	Profile_WriteInt(GENERAL, TEXT("AddColSize-1"), AddColSize[1], app_path);

#ifndef _WIN32_WCE
	Profile_WriteInt(GENERAL, TEXT("viewleft"), ViewRect.left, app_path);
	Profile_WriteInt(GENERAL, TEXT("viewtop"), ViewRect.top, app_path);
	Profile_WriteInt(GENERAL, TEXT("viewright"), ViewRect.right, app_path);
	Profile_WriteInt(GENERAL, TEXT("viewbottom"), ViewRect.bottom, app_path);

	Profile_WriteInt(GENERAL, TEXT("editleft"), EditRect.left, app_path);
	Profile_WriteInt(GENERAL, TEXT("edittop"), EditRect.top, app_path);
	Profile_WriteInt(GENERAL, TEXT("editright"), EditRect.right, app_path);
	Profile_WriteInt(GENERAL, TEXT("editbottom"), EditRect.bottom, app_path);
#endif

	Profile_WriteInt(GENERAL, TEXT("ShowHeader"), ShowHeader, app_path);
	Profile_WriteInt(GENERAL, TEXT("ListGetLine"), ListGetLine, app_path);
	Profile_WriteInt(GENERAL, TEXT("ListDownload"), ListDownload, app_path);
	Profile_WriteInt(GENERAL, TEXT("ListSaveMode"), ListSaveMode, app_path);
	Profile_WriteInt(GENERAL, TEXT("WordBreakFlag"), WordBreakFlag, app_path);
	Profile_WriteInt(GENERAL, TEXT("EditWordBreakFlag"), EditWordBreakFlag, app_path);
	Profile_WriteInt(GENERAL, TEXT("ViewShowDate"), ViewShowDate, app_path);
	Profile_WriteInt(GENERAL, TEXT("MstchCase"), MstchCase, app_path);
	Profile_WriteInt(GENERAL, TEXT("AllFind"), AllFind, app_path);
	Profile_WriteInt(GENERAL, TEXT("SubjectFind"), SubjectFind, app_path);

	Profile_WriteInt(GENERAL, TEXT("ESMTP"), ESMTP, app_path);
	Profile_WriteString(GENERAL, TEXT("SendHelo"), SendHelo, app_path);
	Profile_WriteInt(GENERAL, TEXT("SendMessageId"), SendMessageId, app_path);
	Profile_WriteInt(GENERAL, TEXT("SendDate"), SendDate, app_path);
	Profile_WriteInt(GENERAL, TEXT("SelectSendBox"), SelectSendBox, app_path);
	Profile_WriteInt(GENERAL, TEXT("PopBeforeSmtpIsLoginOnly"), PopBeforeSmtpIsLoginOnly, app_path);
	Profile_WriteInt(GENERAL, TEXT("PopBeforeSmtpWait"), PopBeforeSmtpWait, app_path);

	Profile_WriteInt(GENERAL, TEXT("AutoQuotation"), AutoQuotation, app_path);
	Profile_WriteString(GENERAL, TEXT("QuotationChar"), QuotationChar, app_path);
	Profile_WriteInt(GENERAL, TEXT("WordBreakSize"), WordBreakSize, app_path);
	Profile_WriteInt(GENERAL, TEXT("QuotationBreak"), QuotationBreak, app_path);
	Profile_WriteString(GENERAL, TEXT("ReSubject"), ReSubject, app_path);
	EncodeCtrlChar(ReHeader, conv_buf);
	Profile_WriteString(GENERAL, TEXT("ReHeader"), conv_buf, app_path);

	Profile_WriteString(GENERAL, TEXT("Bura"), Bura, app_path);
	Profile_WriteString(GENERAL, TEXT("Oida"), Oida, app_path);

	Profile_WriteString(GENERAL, TEXT("sBura"), sBura, app_path);
	Profile_WriteString(GENERAL, TEXT("sOida"), sOida, app_path);

	Profile_WriteInt(GENERAL, TEXT("IPCache"), IPCache, app_path);
	Profile_WriteInt(GENERAL, TEXT("EncodeType"), EncodeType, app_path);

	Profile_WriteInt(GENERAL, TEXT("ShowNewMailMessgae"), ShowNewMailMessgae, app_path);
	Profile_WriteInt(GENERAL, TEXT("ShowNoMailMessage"), ShowNoMailMessage, app_path);
	Profile_WriteInt(GENERAL, TEXT("ActiveNewMailMessgae"), ActiveNewMailMessgae, app_path);

	Profile_WriteInt(GENERAL, TEXT("NewMailSound"), NewMailSound, app_path);
	Profile_WriteString(GENERAL, TEXT("NewMailSoundFile"), NewMailSoundFile, app_path);
	Profile_WriteInt(GENERAL, TEXT("ExecEndSound"), ExecEndSound, app_path);
	Profile_WriteString(GENERAL, TEXT("ExecEndSoundFile"), ExecEndSoundFile, app_path);

	Profile_WriteInt(GENERAL, TEXT("AutoCheck"), AutoCheck, app_path);
	Profile_WriteInt(GENERAL, TEXT("AutoCheckTime"), AutoCheckTime, app_path);
	Profile_WriteInt(GENERAL, TEXT("StartCheck"), StartCheck, app_path);
	Profile_WriteInt(GENERAL, TEXT("CheckAfterUpdate"), CheckAfterUpdate, app_path);
	Profile_WriteInt(GENERAL, TEXT("SocIgnoreError"), SocIgnoreError, app_path);
	Profile_WriteInt(GENERAL, TEXT("SendIgnoreError"), SendIgnoreError, app_path);
	Profile_WriteInt(GENERAL, TEXT("CheckEndExec"), CheckEndExec, app_path);
	Profile_WriteInt(GENERAL, TEXT("CheckEndExecNoDelMsg"), CheckEndExecNoDelMsg, app_path);
	Profile_WriteInt(GENERAL, TEXT("TimeoutInterval"), TimeoutInterval, app_path);

	Profile_WriteInt(GENERAL, TEXT("ViewClose"), ViewClose, app_path);
	Profile_WriteString(GENERAL, TEXT("ViewApp"), ViewApp, app_path);
	Profile_WriteString(GENERAL, TEXT("ViewAppCmdLine"), ViewAppCmdLine, app_path);
	Profile_WriteString(GENERAL, TEXT("ViewFileSuffix"), ViewFileSuffix, app_path);
	EncodeCtrlChar(ViewFileHeader, conv_buf);
	Profile_WriteString(GENERAL, TEXT("ViewFileHeader"), conv_buf, app_path);
	Profile_WriteInt(GENERAL, TEXT("ViewAppClose"), ViewAppClose, app_path);
	Profile_WriteInt(GENERAL, TEXT("DefViewApp"), DefViewApp, app_path);
	Profile_WriteString(GENERAL, TEXT("EditApp"), EditApp, app_path);
	Profile_WriteString(GENERAL, TEXT("EditAppCmdLine"), EditAppCmdLine, app_path);
	Profile_WriteString(GENERAL, TEXT("EditFileSuffix"), EditFileSuffix, app_path);
	Profile_WriteInt(GENERAL, TEXT("DefEditApp"), DefEditApp, app_path);

	Profile_WriteString(GENERAL, TEXT("URLApp"), URLApp, app_path);

	Profile_WriteInt(GENERAL, TEXT("EnableLAN"), EnableLAN, app_path);

	Profile_WriteInt(GENERAL, TEXT("RasCon"), RasCon, app_path);
	Profile_WriteInt(GENERAL, TEXT("RasCheckEndDisCon"), RasCheckEndDisCon, app_path);
	Profile_WriteInt(GENERAL, TEXT("RasEndDisCon"), RasEndDisCon, app_path);
	Profile_WriteInt(GENERAL, TEXT("RasNoCheck"), RasNoCheck, app_path);
	Profile_WriteInt(GENERAL, TEXT("RasWaitSec"), RasWaitSec, app_path);

	for(t = 0, j = 0; j < RasInfoCnt; j++){
		if(*(RasInfo + j) == NULL ||
			(*(RasInfo + j))->RasEntry == NULL || *(*(RasInfo + j))->RasEntry == TEXT('\0')){
			continue;
		}
		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasEntry"));
		Profile_WriteString(TEXT("RASINFO"), key_buf, (*(RasInfo + j))->RasEntry, app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasUser"));
		Profile_WriteString(TEXT("RASINFO"), key_buf, (*(RasInfo + j))->RasUser, app_path);

		wsprintf(key_buf, TEXT("RASINFO-%d_%s"), t, TEXT("RasPass"));
		EncodePassword((*(RasInfo + j))->RasUser, (*(RasInfo + j))->RasPass, tmp, BUF_SIZE - 1, FALSE);
		Profile_WriteString(TEXT("RASINFO"), key_buf, tmp, app_path);
		t++;
	}
	Profile_WriteInt(GENERAL, TEXT("RasInfoCnt"), t, app_path);

	//メールボックスの設定の保存
	Profile_WriteInt(GENERAL, TEXT("MailBoxCnt"), MailBoxCnt - MAILBOX_USER, app_path);

	for(j = MAILBOX_USER; j < MailBoxCnt; j++){
		if((MailBox + j) == NULL){
			continue;
		}
		wsprintf(buf, TEXT("MAILBOX-%d"), j - MAILBOX_USER);

		//Name
		Profile_WriteString(buf, TEXT("Name"), (MailBox + j)->Name, app_path);
		//Server
		Profile_WriteString(buf, TEXT("Server"), (MailBox + j)->Server, app_path);
		//Port
		Profile_WriteInt(buf, TEXT("Port"), (MailBox + j)->Port, app_path);
		//User
		Profile_WriteString(buf, TEXT("User"), (MailBox + j)->User, app_path);
		//Pass
		EncodePassword((MailBox + j)->User, (MailBox + j)->Pass, tmp, BUF_SIZE - 1, FALSE);
		Profile_WriteString(buf, TEXT("Pass"), tmp, app_path);
		//APOP
		Profile_WriteInt(buf, TEXT("APOP"), (MailBox + j)->APOP, app_path);

		//MailCnt
		Profile_WriteInt(buf, TEXT("MailCnt"), (MailBox + j)->MailCnt, app_path);
		//MailSize
		Profile_WriteInt(buf, TEXT("MailSize"), (MailBox + j)->MailSize, app_path);

		if(SaveMailFlag == TRUE){
			//LastMessageId
			if((MailBox + j)->LastMessageId != NULL){
#ifdef UNICODE
				CharToTchar((MailBox + j)->LastMessageId, ret, BUF_SIZE - 1);
				*(ret + BUF_SIZE - 1) = TEXT('\0');
				Profile_WriteString(buf, TEXT("LastMessageId"), ret, app_path);
#else
				Profile_WriteString(buf, TEXT("LastMessageId"), (MailBox + j)->LastMessageId, app_path);
#endif
			}
			//LastNo
			Profile_WriteInt(buf, TEXT("LastNo"), (MailBox + j)->LastNo, app_path);
		}

		//CyclicFlag
		Profile_WriteInt(buf, TEXT("CyclicFlag"), (MailBox + j)->CyclicFlag, app_path);

		//SmtpServer
		Profile_WriteString(buf, TEXT("SmtpServer"), (MailBox + j)->SmtpServer, app_path);
		//SmtpPort
		Profile_WriteInt(buf, TEXT("SmtpPort"), (MailBox + j)->SmtpPort, app_path);
		//UserName
		Profile_WriteString(buf, TEXT("UserName"), (MailBox + j)->UserName, app_path);
		//MailAddress
		Profile_WriteString(buf, TEXT("MailAddress"), (MailBox + j)->MailAddress, app_path);
		//Signature
		if((MailBox + j)->Signature != NULL){
			p = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen((MailBox + j)->Signature) * 2 + 1));
			if(p != NULL){
				EncodeCtrlChar((MailBox + j)->Signature, p);
				Profile_WriteString(buf, TEXT("Signature"), p, app_path);
				LocalFree(p);
			}
		}
		//ReplyTo
		Profile_WriteString(buf, TEXT("ReplyTo"), (MailBox + j)->ReplyTo, app_path);
		//MyAddr2Bcc
		Profile_WriteInt(buf, TEXT("MyAddr2Bcc"), (MailBox + j)->MyAddr2Bcc, app_path);
		//BccAddr
		Profile_WriteString(buf, TEXT("BccAddr"), (MailBox + j)->BccAddr, app_path);

		//POP before SMTP
		Profile_WriteInt(buf, TEXT("PopBeforeSmtp"), (MailBox + j)->PopBeforeSmtp, app_path);
		//SMTP Authentication
		Profile_WriteInt(buf, TEXT("SmtpAuth"), (MailBox + j)->SmtpAuth, app_path);
		//SMTP Authentication type
		Profile_WriteInt(buf, TEXT("SmtpAuthType"), (MailBox + j)->SmtpAuthType, app_path);
		//SMTP Authentication User & Pass mode
		Profile_WriteInt(buf, TEXT("AuthUserPass"), (MailBox + j)->AuthUserPass, app_path);
		//SMTP Authentication User
		Profile_WriteString(buf, TEXT("SmtpUser"), (MailBox + j)->SmtpUser, app_path);
		//SMTP Authentication Pass
		EncodePassword((MailBox + j)->SmtpUser, (MailBox + j)->SmtpPass, tmp, BUF_SIZE - 1, FALSE);
		Profile_WriteString(buf, TEXT("SmtpPass"), tmp, app_path);

		//Filter
		Profile_WriteInt(buf, TEXT("FilterEnable"), (MailBox + j)->FilterEnable, app_path);
		Profile_WriteInt(buf, TEXT("FilterCnt"), (MailBox + j)->FilterCnt, app_path);

		for(t = 0; (MailBox + j)->tpFilter != NULL && t < (MailBox + j)->FilterCnt; t++){
			tpFilter = *((MailBox + j)->tpFilter + t);
			if(tpFilter == NULL){
				continue;
			}

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Enable"));
			Profile_WriteInt(buf, key_buf, tpFilter->Enable, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Action"));
			Profile_WriteInt(buf, key_buf, tpFilter->Action, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header1"));
			Profile_WriteString(buf, key_buf, tpFilter->Header1, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content1"));
			Profile_WriteString(buf, key_buf, tpFilter->Content1, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Header2"));
			Profile_WriteString(buf, key_buf, tpFilter->Header2, app_path);

			wsprintf(key_buf, TEXT("FILTER-%d_%s"), t, TEXT("Content2"));
			Profile_WriteString(buf, key_buf, tpFilter->Content2, app_path);
		}

		//RAS
		Profile_WriteInt(buf, TEXT("RasMode"), (MailBox + j)->RasMode, app_path);
		Profile_WriteString(buf, TEXT("RasEntry"), (MailBox + j)->RasEntry, app_path);
		Profile_WriteInt(buf, TEXT("RasReCon"), (MailBox + j)->RasReCon, app_path);
	}
	Profile_Flush(app_path);
	Profile_Free();

	if(SaveMailFlag == FALSE){
		//設定保存のみ
		return rc;
	}

	//メールボックス内のメールの保存
	for(j = MAILBOX_USER; j < MailBoxCnt; j++){
		if((MailBox + j) == NULL){
			continue;
		}
		//メールアイテム
		wsprintf(buf, TEXT("MailBox%d.dat"), j - MAILBOX_USER);
		if(SaveMail(buf, MailBox + j, ListSaveMode) == FALSE){
			rc = FALSE;
		}
	}
	return rc;
}
/* End of source */
