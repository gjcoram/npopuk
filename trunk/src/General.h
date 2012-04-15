/*
 * nPOP
 *
 * General.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2012 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

#ifndef _INC_MAIL_GENERAL_H
#define _INC_MAIL_GENERAL_H

// hacks for old compilers (VC++6)
#if defined( _MSC_VER) && (_MSC_VER <= 1200)
#define strcpy_s(dest,size,src) strcpy(dest,src)
#define strcat_s(dest,size,append) strcat(dest,append)
#define sprintf_s(dest, size, fmtstr, arg1, arg2) sprintf(dest, fmtstr, arg1, arg2)
#ifndef _WIN32_WCE
extern int sprintf();
#endif
#elif defined(_WIN32_WCE)
#define sprintf_s(dest, size, fmtstr, arg1, arg2) sprintf(dest, fmtstr, arg1, arg2)
#else
extern int sprintf_s();
#endif

#if defined _WIN32_WCE && _WIN32_WCE >= 5
#define ENABLE_WIFI
#else
// RAS functions not available in WinCE5
#define ENABLE_RAS
#endif


/* Include Files */
#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE
#include <windowsx.h>
#include <commctrl.h>
#ifndef _WIN32_WCE_PPC
#include <commdlg.h>
#endif
#include <winsock.h>
#include <tchar.h>

#ifdef UNICODE
#include <stdlib.h>
#endif

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
#include "stdafx.h"
#elif defined(_WIN32_WCE)
#define MENU_HEIGHT 26
#endif
#include "resource.h"
#include "Strtbl.h"
#include "font.h"
#include "npop_ver.h"

/* Define */
#define APP_NAME				TEXT("nPOPuk Ver ") NPOPVERTXT
#define APP_NAME_A				"nPOPuk Ver " NPOPVERSTR
#define APP_VERSION_NUM			3000
// switch to wire-form and UTF8 storage
#define STATUS_REVISION_NPOPUK3 300000

////////////////////// MRP ////////////////////
#define HIGH_PRIORITY			TEXT("High")
#define NORMAL_PRIORITY			TEXT("Normal")
#define LOW_PRIORITY			TEXT("Low")
#define FLAG_PRIORITY			TEXT("Flag")

#define PRIORITY_NUMBER1		TEXT("1")
#define PRIORITY_NUMBER2		TEXT("2")
#define PRIORITY_NUMBER3		TEXT("3")
#define PRIORITY_NUMBER4		TEXT("4")
#define PRIORITY_NUMBER5		TEXT("5")

#define PRIORITY_URGENT			TEXT("urgent")
#define PRIORITY_NON_URGENT		TEXT("non-urgent")
////////////////////// --- ////////////////////

#define WINDOW_TITLE			TEXT("nPOPuk")
#define KEY_NAME				TEXT("nPOPuk")

#define STR_MUTEX				TEXT("_nPOP_Mutex_")
#define ATTACH_FILE				TEXT("_attach_")
#define ATTACH_FILE_A			"_attach_"
#define ATTACH_SEP				TEXT('|')

#ifdef _WIN32_WCE
#if _WIN32_WCE <= 200
#define _WCE_OLD				1
#endif
#if _WIN32_WCE >= 300
#define _WCE_NEW				1
#endif
#endif

//#if !defined(_WIN32_WCE) || (defined(_WIN32_WCE_PPC) && WIN32_PLATFORM_PSPC >= 310)
#if !defined(_WIN32_WCE)
#define	LOAD_USER_IMAGES		1
#endif

#define MAIN_WND_CLASS			TEXT("nPOPMainWndClass")
#define VIEW_WND_CLASS			TEXT("nPOPViewWndClass")
#define EDIT_WND_CLASS			TEXT("nPOPEditWndClass")

#define ADDRESS_FILE			TEXT("Address.lst")
#define SAVEBOX_FILE			TEXT("SaveBox.dat")
#define SENDBOX_FILE			TEXT("SendBox.dat")

#define VIEW_FILE				TEXT("$npop_view")
#define EDIT_FILE				TEXT("$npop_edit")

#define LOG_FILE				TEXT("nPOPuk.log")

#ifdef ENABLE_RAS
#define RAS_WAIT_EVENT			TEXT("RAS_WAIT_EVENT")
#define ID_RASWAIT_TIMER		10
#endif
#ifdef ENABLE_WIFI
#define WIFI_WAIT_EVENT			TEXT("WIFI_WAIT_EVENT")
#define ID_WIFIWAIT_TIMER		10
#endif

#define BUF_SIZE				256					// Buffer size
#define MSG_SIZE				300					// BUF_SIZE + some text
#define MULTI_BUF_SIZE			1024				// Note: wsprintf only handles up to 1024!
#define MAXSIZE					32768
#define EDITMAXSIZE				60000

#define MAX_MAILBOX_CNT			1000				// see ID_MENUITEM_COPY2MBOX
#define MAX_TRAY_CNT			10					// see ID_MENUITEM_TRAYCHECK

#define SICONSIZE				16					// Size of listview icons
#define TB_ICONSIZE				16					// Size of default toolbar buttons
#ifdef _WIN32_WCE
#define TB_MAINBUTTONS			13					// number of buttons on main toolbar
#else
#define TB_MAINBUTTONS			21					// number of buttons on main toolbar
#endif
#define TB_EDITBUTTONS			9					// number of buttons on edit toolbar
#define TB_VIEWBUTTONS			13					// number of buttons on view toolbar

#define TABSTOPLEN				8					// TAB Stop

#define LV_COL_CNT				4					//ListView of tool bar Fixed mailbox
#define AD_COL_CNT				3					// ListView Addressbook
#define MB_COL_CNT				6					// ListView Mailboxes

#define MAILBOX_SEND			0
#define MAILBOX_USER			1
#define MAILBOX_TYPE_ACCOUNT	0
#define MAILBOX_TYPE_SAVE		1
//#define MAILBOX_TYPE_IMAP		?
#define MAILBOX_IMPORT_SAVE		2
#define MAILBOX_ADD_SAVE		10

#define VSELBOX_FLAG			100

#define MAILITEMS_CHANGED		1					// for NeedsSave
#define MARKS_CHANGED			2
#define MBOX_FORMAT_CHANGED		4

#define MAIL2ITEM_TOP			0					// for item_mail_to_item
#define MAIL2ITEM_RETR			1
#define MAIL2ITEM_IMPORT		2
#define MAIL2ITEM_WIRE			3
#define MAIL2ITEM_ATTACH		4

#define IDC_MBMENU				400					//Control ID
#define IDC_LISTVIEW			401
#define IDC_STATUS				402

#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP			0x400
#endif

#define WM_SOCK_SELECT			(WM_APP + 1)		//Socket event
#define WM_SOCK_RECV			(WM_APP + 2)		//Socket reception event
#define WM_LV_EVENT				(WM_APP + 3)		//List view event
#define WM_STATUSTEXT			(WM_APP + 4)		//To status bar character string setting
#define WM_SHOWLASTWINDOW		(WM_APP + 5)		//Window indicatory
#define WM_SMTP_SENDMAIL		(WM_APP + 6)		//Mail transmission
#define WM_ENDCLOSE				(WM_APP + 7)		//End
#define WM_INITTRAYICON			(WM_APP + 8)		//of window Initialization

#define WM_SHOWDIALOG			(WM_APP + 9)		//of task tray idea contest Mail arrival message indicatory
#define WM_ENDDIALOG			(WM_APP + 10)		//Mail arrival message end

#define WM_CHANGE_MARK			(WM_APP + 11)
#define WM_MODFYMESSAGE			(WM_APP + 12)

#define ID_MENU					(WM_APP + 102)

#define EDIT_OPEN				0					//Type
#define EDIT_NEW				1
#define EDIT_REPLY				2
#define EDIT_REPLYALL			3
#define EDIT_FORWARD			4					// Added PHH 4-Oct-2003
#define EDIT_FILTERFORWARD		5					// Added GJC 5-Jul-2008
#define EDIT_REDIRECT			6					// Added GJC 15-Nov-2008

#define EDIT_NONEDIT			0					//of transmission mail compilation Return value
#define EDIT_INSIDEEDIT			1
#define EDIT_OUTSIDEEDIT		2
#define EDIT_SEND				3
#define EDIT_REOPEN				4

#define AUTOSEND_NO_QUIT		1
#define AUTOSEND_AND_QUIT		2

#define SELECT_MEM_ERROR		-2					//of transmission mail compilation Return value
#define SELECT_SOC_ERROR		-1
#define SELECT_SOC_CLOSE		0
#define SELECT_SOC_SUCCEED		1
#define SELECT_SOC_NODATA		2

#define SORT_NO					100					//of socket processing Sort flag
#define SORT_ICON				101
#define SORT_THREAD				102
#define SORT_SUBJ				0					// watch for swap of LvColumnOrder
#define SORT_FROM				1
#define SORT_DATE				2
#define SORT_SIZE				3

#define POP_ERR					-2					//pop3 command flag
#define POP_QUIT				-1
#define POP_START				0
#define POP_STARTTLS			1
#define POP_USER				2
#define POP_PASS				3
#define POP_LOGIN				4
#define POP_STAT				5
#define POP_LIST				6
#define POP_UIDL				7
#define POP_UIDL_ALL			8
#define POP_UIDL_CHECK			9
#define POP_TOP					10
#define POP_RETR				11
#define POP_DELE				12

#define SMTP_ERR				POP_ERR				//smtp command flag
#define SMTP_QUIT				POP_QUIT
#define SMTP_START				POP_START
#define SMTP_EHLO				1
#define SMTP_STARTTLS			2
#define SMTP_AUTH_CRAM_MD5		3
#define SMTP_AUTH_LOGIN			4
#define SMTP_AUTH_LOGIN_PASS	5
#define SMTP_HELO				6
#define SMTP_RSET				7
#define SMTP_MAILFROM			8
#define SMTP_RCPTTO				9
#define SMTP_DATA				100
#define SMTP_SENDBODY			101
#define SMTP_NEXTSEND			102
#define SMTP_SENDEND			103

#define ICON_NON				0					//Idea contest state
#define ICON_MAIL				1
#define ICON_READ				2
#define ICON_DOWN				3
#define ICON_DEL				4
#define ICON_SENTMAIL			5
#define ICON_SEND				6
#define ICON_ERROR				7
#define ICON_FLAG				8
// overlays -- INDEXTOOVERLAYMASK(i) is ((i) << 8)
#define ICON_NEW_MASK			0x04
#define ICON_REPL_MASK			0x01
#define ICON_FWD_MASK			0x02
#define ICON_REFWD_MASK			0x03
// for holding messages that are being forwarded
#define REFWD_FWDHOLD			0x08

// marks for forwarding/replying and the property dialog
#define MARK_REPLYING			1
#define MARK_REPL_SELTEXT		2
#define MARK_FORWARDING			3
#define MARK_FWD_SELTEXT		4

#define FILE_OPEN_SINGLE		0
#define FILE_OPEN_MULTI			1
#define FILE_SAVE_SINGLE		2
#define FILE_SAVE_MSG			3
#define FILE_CHOOSE_DIR			4
#define FILE_CHOOSE_BACKDIR		5

#define DECODE_ASK					0
#define DECODE_AUTO_OPEN			1
#define DECODE_SAVE_ALL				2
#define DECODE_OPEN_IF_MSG			3
#define DECODE_SAVE_EMBED			4
#define DECODE_SAVE_IMAGES			5
#define DECODE_VIEW_IMAGES			6

#define MULTIPART_NONE			0
#define MULTIPART_ATTACH		1
#define MULTIPART_CONTENT		2
#define MULTIPART_HTML			3

#define FILTER_REFILTER			0x02
#define FILTER_NOGLOBAL			0x04
#define FILTER_UNRECV			1					//Filter type
#define FILTER_RECV				2
#define FILTER_DOWNLOADMARK		4
#define FILTER_DELETEMARK		8
#define FILTER_READICON			16
#define FILTER_COPY				32
#define FILTER_MOVE				64
#define FILTER_PRIORITY			128
#define FILTER_FORWARD			256

#define MP_ERROR_FILE			-2
#define MP_ERROR_ALLOC			-1
#define MP_NO_ATTACH			0
#define MP_ATTACH				1

#define HEAD_SUBJECT			"Subject:"
#define HEAD_FROM				"From:"
#define HEAD_TO					"To:"
#define HEAD_CC					"Cc:"
#define HEAD_BCC				"Bcc:"
#define HEAD_REDIRECT			"X-Redirect-To:"
#define HEAD_REPLYTO			"Reply-To:"
#define HEAD_DATE				"Date:"
#define HEAD_SIZE				"Content-Length:"
#define HEAD_MESSAGEID			"Message-Id:"
#define HEAD_INREPLYTO			"In-Reply-To:"
#define HEAD_REFERENCES			"References:"
#define HEAD_MIMEVERSION		"MIME-Version:"
#define HEAD_CONTENTTYPE		"Content-Type:"
#define HEAD_CONTENTID			"Content-ID:"
#define HEAD_ENCODING			"Content-Transfer-Encoding:"
#define HEAD_DISPOSITION		"Content-Disposition:"
#define HEAD_X_MAILER			"X-Mailer:"
#define HEAD_X_UIDL				"X-UIDL:"
#define HEAD_X_NO				"X-No:"
#define HEAD_X_MARK				"X-Mark:"
#define HEAD_X_STATUS			"X-Status:"
#define HEAD_X_DOWNLOAD			"X-Download:"
#define HEAD_X_MAILBOX			"X-MailBox:"
#define HEAD_X_ATTACH			"X-File:"
#define HEAD_X_FWDATTACH		"X-FwdAttach:"
#define HEAD_X_HASHEADER		"X-HasHeader:"
////////////// MRP ///////////////////////
#define HEAD_X_PRIORITY			"X-Priority:"
#define HEAD_IMPORTANCE			"Importance:"
#define HEAD_PRIORITY			"Priority:"

#define HEAD_DELIVERY			"Return-Receipt-To:"
#define HEAD_READ1				"X-Confirm-Reading-To:"
#define HEAD_READ2				"Disposition-Notification-To:"
////////////// --- ///////////////////////

// note: don't fix "Encodeing" for backwards-compatibility; it's not transmitted anyway
#define HEAD_X_HEADCHARSET		"X-Header-Charset:"
#define HEAD_X_HEADENCODE		"X-Header-Encodeing:"
#define HEAD_X_BODYCHARSET		"X-Body-Charset:"
#define HEAD_X_BODYENCODE		"X-Body-Encodeing:"

#define HEAD_X_NO_OLD			"X-MailNo:"
#define HEAD_X_MARK_OLD			"X-MarkStatus:"
#define HEAD_X_STATUS_OLD		"X-MailStatus:"
#define HEAD_X_DOWNLOAD_OLD		"X-MailDownload:"
#define HEAD_X_ATTACH_OLD		"X-Attach:"

#define CHARSET_US_ASCII		"US-ASCII"
#define CHARSET_ISO_8859_1		"ISO-8859-1"
#define CHARSET_ISO_2022_JP		"ISO-2022-JP"
#define CHARSET_UTF_7			"UTF-7"
#define CHARSET_UTF_8			"UTF-8"

#define ENCODE_7BIT				"7bit"
#define ENCODE_8BIT				"8bit"
#define ENCODE_BASE64			"base64"
#define ENCODE_Q_PRINT			"quoted-printable"

#define URL_HTTP				TEXT("http://")
#define URL_HTTPS				TEXT("https://")
#define URL_FTP					TEXT("ftp://")
#define URL_MAILTO				TEXT("mailto:")

/* no value parameters */
#define PARM_S					TEXT("/s")
#define PARM_Q					TEXT("/q")

/* parameters with values */
#define PARM_Y					TEXT("/y:")
#define PARM_A					TEXT("/a:")
#define PARM_MAILBOX			TEXT("/mailbox:")
#define PARM_TO					TEXT("/to:")
#define PARM_CC					TEXT("/cc:")
#define PARM_BCC				TEXT("/bcc:")
#define PARM_REPLY_TO			TEXT("/reply-to:")
#define PARM_SUBJECT			TEXT("/subject:")
#define PARM_BODY				TEXT("/body:")
#define PARM_ATTACH				TEXT("/attach:")
#define PARM_MAILTO				TEXT("mailto:")

#define ABS(n)					((n < 0) ? (-n) : n)				// â‘Î’l

/* Struct */
typedef struct _OPTION {
	int StartPass;
	int ShowPass;
	int ConfigPass;
	int ScrambleMailboxes;
	TCHAR *Password;

	TCHAR DataFileDir[BUF_SIZE];
	TCHAR DecPt[5];
	TCHAR *BackupDir;

	FONT_INFO view_font;
	FONT_INFO lv_font;
	int StatusBarCharWidth;

#ifndef _WIN32_WCE
	RECT MainRect;
	RECT ViewRect;
	RECT EditRect;
#endif
	RECT AddrRect;
	RECT MblRect;
#ifdef LOAD_USER_IMAGES
	int MainBmpSize;
	int EditBmpSize;
	int ViewBmpSize;
#endif

	int Version;
	int osPlatformId, osMajorVer, osMinorVer;
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
	int LvSortFromAddressOnly;
	int LvThreadView;
	int LvStyle;
	int LvStyleEx;
	int ToolBarHeight; // not saved in INI, for resizing
	TCHAR *LvColumnOrder;
	TCHAR *PreviewHeader;
	int PreviewPaneHeight;
	int PreviewPaneMinHeight;
	int PreviewPaneWidth; // not saved in INI, for resizing
	int AutoPreview;
	int PreviewedIsReadTime;
	int MBMenuWidth;
	int MBMenuMinWidth;
	int MBMenuHeight; // not saved in INI, for resizing
	int ContextMenuOption;
	int SaveboxListCount;
	int ScanAllForUnread;
	int DeletedIsNotNew;
	int DelIsMarkDel;
	int RecvScroll;
	int SaveMsg;
	int AutoSave;
	int WriteMbox;
	int CheckQueuedOnExit;
	int PromptSaveOnExit;
	int LazyLoadMailboxes;
	int BlindAppend;
	int FixContentType;
	int LvColSize[LV_COL_CNT];
	int AddColSize[AD_COL_CNT];
	int MblColSize[MB_COL_CNT];
	int AddressSort;
	int AddressJumpKey;
	int AutoAddRecipients;
	TCHAR *AddressShowGroup;

	int ListGetLine;
	int ListDownload;
	int ShowHeader;
	int GetRecent;
	int ListSaveMode;
	int WordBreakFlag;
	int EditWordBreakFlag;
#ifdef _WIN32_WCE_PPC
	int OptionalScrollbar;
#endif
	int ViewShowDate;
	int MatchCase;
	int Wildcards;
	int AllMsgFind;
	int AllBoxFind;
	int SubjectFind;

	int ESMTP;
	TCHAR *SendHelo;
	int SendMessageId;
	int SendDate;
	int SelectSendBox;
	int ExpertMode;		// Added PHH 4-10-2003
	int PopBeforeSmtpIsLoginOnly;
	int PopBeforeSmtpWait;
	int NoEmptyMailbox;

	// SSL
	TCHAR *CAFile;

	TCHAR *Codepage;
	TCHAR *HeadCharset;
	int HeadEncoding;
	TCHAR *BodyCharset;
	int BodyEncoding;

	int AutoQuotation;
	int FwdQuotation;
	int SignForward;
	int SignReplyAbove;
	TCHAR *QuotationChar;
	int WordBreakSize;
	int QuotationBreak;
	TCHAR *ReSubject;
	TCHAR *FwdSubject;		// Added PHH 4-10-2003
	TCHAR *ReFwdPrefixes;
	TCHAR *ReHeader;
	TCHAR *FwdHeader;
	TCHAR *AltReplyTo;
	TCHAR *Bura;
	TCHAR *Oida;

	int IPCache;
	int EncodeType;
	TCHAR *TimeZone;
	TCHAR *DateFormat;
	TCHAR *TimeFormat;

	int ShowNewMailMessage;
	int ShowNoMailMessage;
	int ActiveNewMailMessage;
	int ClearNewOverlay;

	int HasResourceDir; // detected at start-up, not saved in INI
	int NewMailSound;
	int ExecEndSound;
	int ItemPlaySound;
	TCHAR *NewMailSoundFile;
	TCHAR *ExecEndSoundFile;
	TCHAR *ItemNewSoundFile;
	TCHAR *ItemPartialSoundFile;
	TCHAR *ItemFullSoundFile;
	TCHAR *ItemAttachSoundFile;
	TCHAR *ItemHtmlSoundFile;
	TCHAR *ItemNonSoundFile;
	TCHAR *ItemUnreadSoundFile;
	TCHAR *ItemReadSoundFile;
	TCHAR *ItemDownSoundFile;
	TCHAR *ItemDelSoundFile;
	TCHAR *ItemSendSoundFile;
	TCHAR *ItemSentSoundFile;
	TCHAR *ItemErrorSoundFile;
	TCHAR *ItemFlagSoundFile;

	int AutoCheck;
	int AutoCheckTime;
	int StartCheck;
	int CheckAfterUpdate;
	int SocIgnoreError;
	int SendIgnoreError;
	int NoIgnoreErrorTimeout;
	int SendAttachIndividually;
	int CheckEndExec;
	int CheckEndExecNoDelMsg;
	int DeletePerUpdateLimit;
	int TimeoutInterval;
	int ReorderSendbox;

	int ViewClose;
	int ViewCloseNoNext;
	int ViewNextAfterDel;
	int ViewSkipDeleted;
	TCHAR *ViewApp;
	TCHAR *ViewAppCmdLine;
	TCHAR *ViewFileSuffix;
	TCHAR *ViewFileHeader;
	int ViewAppClose;
	int DefViewApp;
	int ViewAppMsgSource;
	int AutoOpenAttachMsg;
	int ViewWindowCursor;
	int ViewShowAttach;
	TCHAR *EditApp;
	TCHAR *EditAppCmdLine;
	TCHAR *EditFileSuffix;
	int DefEditApp;
	TCHAR *AttachPath;
//	int AttachWarning;
	int AttachDelete;
	int StripHtmlTags;
	TCHAR *SavedOpenDir;
	TCHAR *SavedSaveDir;

	TCHAR *URLApp;
	TCHAR *URLAppCmdLine;

	int EnableLAN;

	int WifiCon;
	int WifiCheckEndDisCon;
	int WifiCheckEndDisConTimeout;
	int WifiExitDisCon;
	int WifiNoCheck;
	int WifiWaitSec;
	TCHAR *WifiDeviceName;

	int RasCon;
	int RasCheckEndDisCon;
	int RasCheckEndDisConTimeout;
	int RasEndDisCon;
	int RasNoCheck;
	int RasWaitSec;
	int RasInfoCnt;
	struct _RASINFO **RasInfo;
#ifdef _WIN32_WCE
///////////// MRP /////////////////////
	int	UsePOOMAddressBook;
///////////// --- /////////////////////
	int POOMNameIsComment;
	int LoadPOOMAtStart;
#endif
#ifdef _WIN32_WCE_PPC
	int UseWindowsSSL;
	int ShowNavButtons;
	int PromptIniEdit;
#endif

	int GlobalFilterEnable;
	int GlobalFilterCnt;
	struct _FILTER **tpFilter;

} OPTION;

typedef struct _SSL_INFO {
	int Type;
	int Verify;
	int Depth;
	TCHAR *Cert;
	TCHAR *Pkey;
	TCHAR *Pass;
} SSL_INFO;

typedef struct _MAILBOX {
	TCHAR *Name;
	TCHAR *Filename; // NULL means use MailBox%d.dat
	int Type;
	int NeedsSave;
	BOOL Loaded;
	BOOL WasMbox; // when file was read
	TCHAR *DefAccount; // for saveboxes, account for outgoing mail
	TCHAR *NewMailSoundFile;

	// POP
	TCHAR *Server;
	int Port;
	TCHAR *User;
	TCHAR *Pass;
	TCHAR *TmpPass;
	int APOP;
	int PopSSL;
	SSL_INFO PopSSLInfo;
	int NoRETR;
	int NoUIDL;
	int MessageSizeDelta;
	unsigned long PopIP;

	int MailCnt; // count on server
	unsigned int MailSize; // size on server
	long DiskSize;

	int StartInit;
	char *LastMessageId;
	int LastNo;

	// SMTP
	TCHAR *SmtpServer;
	int SmtpPort;
	TCHAR *UserName;
	TCHAR *MailAddress;
	TCHAR *Signature;
	TCHAR *ReplyTo;
	int UseReplyToForFrom;
	int SendWireForm;
	int MyAddr2Bcc;
	TCHAR *BccAddr;
	int PopBeforeSmtp;
	int SmtpAuth;
	int SmtpAuthType;
	int AuthUserPass;
	TCHAR *SmtpUser;
	TCHAR *SmtpPass;
	TCHAR *SmtpTmpPass;
	int SmtpSSL;
	SSL_INFO SmtpSSLInfo;
	unsigned long SmtpIP;

	// Check
	int NewMail;
	int UnreadCnt;
	BOOL HeldMail;
	BOOL ListInitMsg;
	int CyclicFlag;

	// Filter
	int FilterEnable;
	int FilterCnt;
	struct _FILTER **tpFilter;

	// Ras
	int RasMode;
	TCHAR *RasEntry;
	int RasReCon;

	// Recv
	int UseGlobalRecv;
	int ListGetLine;
	int ListDownload;
	int ShowHeader;
	int GetRecent;
	int ListSaveMode;

	// MailItem
	struct _MAILITEM **tpMailItem;
	int MailItemCnt;
	int AllocCnt;
} MAILBOX;

typedef struct _MAILITEM {
	char MailStatus;
	char Mark;
	char ReFwd;
	char Multipart;
	int No;
	int PrevNo;
	int NextNo;
	int Indent;
	int HasHeader;  // 0=no, 1=full headers, 2=stripped of duplicates
	BOOL New;
	BOOL Download;
	BOOL DefReplyTo;
	HWND hEditWnd;
	HANDLE hProcess;

	TCHAR *From;
	TCHAR *From_email;
	TCHAR *To;
	TCHAR *Cc;
	TCHAR *Bcc;
	TCHAR *RedirectTo;
	TCHAR *Date;
	TCHAR *FmtDate;
	TCHAR *Size;
	TCHAR *Subject;
	TCHAR *ReplyTo;
	TCHAR *ContentType;
	TCHAR *Encoding;
	TCHAR *MessageID;
	TCHAR *UIDL;
	TCHAR *InReplyTo;
	TCHAR *References;
	char *WireForm;
	char *Body;

	TCHAR *MailBox;
	TCHAR *Attach;
	TCHAR *FwdAttach;
	long AttachSize;
	TCHAR *HeadCharset;
	TCHAR *BodyCharset;
	int HeadEncoding;
	int BodyEncoding;	
	///////////////// MRP ////////////////////
	int Priority;
	int ReadReceipt;
	int DeliveryReceipt;
	///////////////// --- ////////////////////
} MAILITEM;

typedef struct _ADDRESSBOOK {
	struct _ADDRESSITEM **tpAddrItem;
	TCHAR *AddrList;
	BOOL GetAddrList, FromAddrInfo;
	int ItemCnt, EditNum;
} ADDRESSBOOK;

typedef struct _ADDRESSITEM {
	TCHAR *MailAddress; // may include "name" <address>
	TCHAR *AddressOnly;
	TCHAR *Comment;
	TCHAR *Group;
	int Num;
	BOOL Displayed;
} ADDRESSITEM;

typedef struct _FILTER {
	int Enable;
	int Action;
	int Boolean;
	int Priority;
	TCHAR *SaveboxName;
	TCHAR *FwdAddress;
	// TCHAR *TargetName; // for Savebox or Fwd?
	
	TCHAR *Header1;
	TCHAR *Content1;

	TCHAR *Header2;
	TCHAR *Content2;
} FILTER;

typedef enum {
	FILTER_UNRECV_INDEX = 0,
	FILTER_RECV_INDEX,
	FILTER_DOWNLOADMARK_INDEX,
	FILTER_DELETEMARK_INDEX,
	FILTER_READICON_INDEX,
	FILTER_COPY_INDEX,
	FILTER_MOVE_INDEX,
	FILTER_PRIORITY_INDEX,
	FILTER_FORWARD_INDEX
};
#define FILTER_BOOL_AND 0
#define FILTER_BOOL_OR 1
#define FILTER_BOOL_UNLESS 2

typedef struct _ATTACH_ITEM {
	TCHAR *fname;
	BOOL is_fwd;
	struct _ATTACH_ITEM *next;
} ATTACH_ITEM;

typedef struct _ATTACHINFO {
	TCHAR *from;
	TCHAR *fname;
	TCHAR *mime;
	int size;
} ATTACHINFO;

typedef struct _RASINFO {
	TCHAR *RasEntry;
	TCHAR *RasUser;
	TCHAR *RasPass;
} RASINFO;

/* Function Prototypes */
#ifdef ENABLE_WIFI
// Wifi
BOOL GetNetworkStatus(BOOL Print);
BOOL WifiConnect(HWND hWnd, int Dummy);
void WifiDisconnect(BOOL Force);
#endif
#ifdef ENABLE_RAS
// Ras
BOOL GetRasInfo(TCHAR *Entry);
BOOL SetRasInfo(TCHAR *Entry, TCHAR *User, TCHAR *Pass);
void FreeRasInfo(void);
void initRas(void);
void FreeRas(void);
BOOL GetRasEntries(HWND hCmboWnd);
BOOL GetRasStatus(void);
void RasDisconnect(void);
BOOL RasStatusProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL RasMailBoxStart(HWND hWnd, int BoxIndex);
#endif

// WinSock
unsigned long get_host_by_name(HWND hWnd, TCHAR *server, TCHAR *ErrStr);
SOCKET connect_server(HWND hWnd, unsigned long ip_addr, unsigned short port, const int ssl_tp, const SSL_INFO *si, TCHAR *ErrStr);
int recv_proc(HWND hWnd, SOCKET soc);
#ifndef WSAASYNC
int recv_select(HWND hWnd, SOCKET soc);
#endif	//WSAASYNC
int send_data(SOCKET soc, char *wbuf, int len);
int send_buf(SOCKET soc, char *buf);
#ifdef UNICODE
int send_buf_t(SOCKET soc, TCHAR *wbuf);
#else	//UNICODE
#define send_buf_t	send_buf
#endif	//UNICODE
void socket_close(HWND hWnd, SOCKET soc);
int init_ssl(const HWND hWnd, const SOCKET soc, TCHAR *ErrStr);
void free_ssl(void);
BOOL CALLBACK AboutBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Pop3
BOOL pop3_list_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
BOOL pop3_exec_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
BOOL pop3_salvage_buffer(HWND hWnd, MAILBOX *tpMailBox, BOOL ShowFlag);
void pop3_free(void);
#ifdef _WIN32_WCE_PPC
HWND findTodayPlugin(WCHAR *wTodayItem);
#endif

// Smtp
void HMAC_MD5(unsigned char *input, int len, unsigned char *key, int keylen, unsigned char *digest);
#ifdef WSAASYNC
BOOL smtp_send_proc(HWND hWnd, SOCKET soc, TCHAR *ErrStr);
#endif
BOOL smtp_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
SOCKET smtp_send_mail(HWND hWnd, MAILBOX *tpMailBox, MAILITEM *tpMailItem, int EndMailFlag, TCHAR *ErrStr);
void ClearFwdHold(MAILITEM *tpSentItem);
void smtp_set_error(HWND hWnd);
void smtp_free(void);

// File
TCHAR *log_read(void);
void log_flush(void);
BOOL log_save_a(char *buf);
#ifdef UNICODE
BOOL log_save(TCHAR *buf);
#else
#define log_save log_save_a
#endif
BOOL log_header(char *buf);
BOOL dir_check(const TCHAR *path);
BOOL dir_create(TCHAR *path);
BOOL dir_delete(TCHAR *Path, TCHAR *file);
void trunc_to_dirname(TCHAR *path);
BOOL trunc_to_parent_dir(TCHAR *path);
void filename_conv(TCHAR *buf);
BOOL filename_select(HWND hWnd, TCHAR *ret, TCHAR *DefExt, TCHAR *filter, int Action, TCHAR **opptr);
long file_get_size(TCHAR *FileName);
char *file_read(TCHAR *path, long FileSize);
BOOL file_read_select(HWND hWnd, TCHAR **buf);
BOOL file_savebox_convert(TCHAR *NewFileName);
BOOL file_copy_to_datadir(HWND hWnd, TCHAR *Source, TCHAR *FileName);
BOOL file_read_mailbox(TCHAR *FileName, MAILBOX *tpMailBox, BOOL Import, BOOL CheckDup);
int file_read_address_book(TCHAR *FileName, ADDRESSBOOK *tpAddrBook, BOOL GetContacts);
MAILITEM *file_scan_mailbox(TCHAR *FileName, char *m_id);
BOOL file_write(HANDLE hFile, char *buf, int len);
BOOL file_write_ascii(HANDLE hFile, TCHAR *buf, int len);
BOOL file_save_attach(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len, int do_what);
BOOL file_save_exec(HWND hWnd, TCHAR *FileName, char *buf, int len);
BOOL file_save_mailbox(TCHAR *FileName, TCHAR *SaveDir, int Index, BOOL IsBackup, BOOL IsAutosave, int SaveFlag);
BOOL file_append_savebox(TCHAR *FileName, MAILBOX *tpMailBox, MAILITEM *tpMailItem, int SaveFlag);
BOOL file_save_address_book(TCHAR *FileName, TCHAR *SaveDir, ADDRESSBOOK *tpAddrBook);
BOOL file_rename(HWND hWnd, TCHAR *Source, TCHAR *Destin);
BOOL file_delete(HWND hWnd, TCHAR *name);

// Ini
BOOL ini_start_auth_check(void);
void ini_read_general(HWND hWnd);
BOOL ini_read_setting(HWND hWnd);
void ini_write_general(void);
BOOL ini_save_setting(HWND hWnd, BOOL SaveMailFlag, BOOL SaveAll, TCHAR *SaveDir);
void ini_free(BOOL free_all);

// Item
int item_is_mailbox(MAILBOX *tpMailBox, MAILITEM *tpMailItem);
BOOL item_set_count(MAILBOX *tpMailBox, int i);
BOOL item_add(MAILBOX *tpMailBox, MAILITEM *tpNewMailItem);
void item_copy(MAILITEM *tpFromMailItem, MAILITEM *tpToMailItem, BOOL Override);
MAILITEM *item_to_mailbox(MAILBOX *tpMailBox, MAILITEM *tpNewMailItem, TCHAR *MailBoxName, BOOL SendClear);
BOOL item_resize_mailbox(MAILBOX *tpMailBox);
void item_free(MAILITEM **tpMailItem, int cnt);
void item_get_content_t(char *buf, char *header, TCHAR **ret);
char *item_get_message_id(char *buf);
int item_get_number_to_index(MAILBOX *tpMailBox, int No);
int item_get_next_download_mark(MAILBOX *tpMailBox, int Index, int *No);
int item_get_next_delete_mark(MAILBOX *tpMailBox, BOOL hold, int Index, int *No);
int item_get_next_new(MAILBOX *tpMailBox, int Index, int *No);
int item_get_next_send_mark(MAILBOX *tpMailBox, BOOL CheckErrors, BOOL ForSend);
int item_get_next_send_mark_mailbox(MAILBOX *tpMailBox, int Index, int MailBoxIndex);
void item_set_flags(MAILITEM *tpMailItem, MAILBOX *tpMailBox, int code);
void item_get_npop_headers(char *buf, MAILITEM *tpMailItem, MAILBOX *tpMailBox);
BOOL item_mail_to_item(MAILITEM *tpMailItem, char **buf, int Size, BOOL download, int status, MAILBOX *tpMailBox);
MAILITEM *item_header_to_item(MAILBOX *tpMailBox, char **buf, int Size, int status);
MAILITEM *item_string_to_item(MAILBOX *tpMailBox, char *buf, BOOL Import);
char *item_create_wireform(MAILITEM *tpMailItem, TCHAR *body);
int item_to_string_size(MAILITEM *tpMailItem, int WriteMbox, BOOL BodyFlag, BOOL SepFlag);
char *item_to_string(char *buf, MAILITEM *tpMailItem, int WriteMbox, BOOL BodyFlag, BOOL SepFlag);
int item_find_thread(MAILBOX *tpMailBox, TCHAR *p, int Index);
MAILITEM *item_find_thread_anywhere(TCHAR *p);
void item_create_thread(MAILBOX *tpMailBox);

// MailBox
BOOL mailbox_init(void);
int mailbox_create(HWND hWnd, int Add, int Index, BOOL ShowFlag, BOOL SelFlag);
int mailbox_delete(HWND hWnd, int DelIndex, BOOL CheckFilt, BOOL Select);
BOOL mailbox_read(void);
int mailbox_load_now(HWND hWnd, int num, BOOL ask, BOOL do_saveboxes);
BOOL mailbox_move_up(HWND hWnd, BOOL select);
BOOL mailbox_move_down(HWND hWnd, BOOL select);
BOOL mailbox_unread_check(int index, BOOL NewFlag);
int mailbox_next_unread(HWND hWnd, int index, int endindex);
void mailbox_select(HWND hWnd, int Sel);
BOOL mailbox_menu_rebuild(HWND hWnd, BOOL IsAttach);
int mailbox_name_to_index(TCHAR *Name, int Type);
void filter_sbox_check(HWND hWnd, TCHAR *ConvertName);
void filter_free(MAILBOX *tpMailBox);
ADDRESSBOOK *addressbook_copy(void);
BOOL addr_add(ADDRESSBOOK *tpAddrBook, ADDRESSITEM *tpNewAddrItem);
void addr_move(ADDRESSBOOK *tpAddrBook, int num, int step);
void addr_delete(ADDRESSBOOK *tpAddrBook, int num);
void addr_list_add(TCHAR *AddrList);
void addr_free(ADDRESSITEM **tpAddrItem, int cnt);
void addressbook_free(ADDRESSBOOK *tpAddrBook);
void mailbox_free(MAILBOX *tpMailBox);

// util
TCHAR *GetHeaderStringPointT(TCHAR *buf, TCHAR *str);
#ifdef UNICODE
char *GetHeaderStringPoint(char *buf, char *str);
#else
#define GetHeaderStringPoint GetHeaderStringPointT
#endif
int GetHeaderStringSizeT(TCHAR *buf, BOOL CrLfFlag);
#ifdef UNICODE
int GetHeaderStringSize(char *buf, BOOL CrLfFlag);
#else
#define GetHeaderStringSize GetHeaderStringSizeT
#endif
BOOL GetHeaderStringT(TCHAR *buf, TCHAR *ret, BOOL CrLfFlag);
#ifdef UNICODE
BOOL GetHeaderString(char *buf, char *ret, BOOL CrLfFlag);
#else
#define GetHeaderString GetHeaderStringT
#endif

TCHAR *GetBodyPointaT(TCHAR *buf);
#ifdef UNICODE
char *GetBodyPointa(char *buf);
#else
#define GetBodyPointa GetBodyPointaT
#endif

void TrimMessageId(char *buf);
int GetReferencesSize(char *p, BOOL Flag);
BOOL ConvReferences(char *p, char *r, BOOL Flag);

void DateAdd(SYSTEMTIME *sTime, char *tz);
int DateConv(char *buf, char *ret, BOOL for_sort);
int DateUnConv(char *buf, char *ret);
BOOL DateCompare(TCHAR *msg_date, int days, BOOL newer);
void FormatNumberString(long num, TCHAR *fmtstring, TCHAR *decpt, TCHAR *ret);
void GetTimeString(TCHAR *buf);
void EncodePassword(TCHAR *Key, TCHAR *Word, TCHAR *ret, int retsize, BOOL decode);
void EncodeCtrlChar(TCHAR *buf, TCHAR *ret);
void DecodeCtrlChar(TCHAR *buf, TCHAR *ret);
TCHAR *CreateMessageId(long id, TCHAR *MailAddress);
int CreateHeaderStringSize(TCHAR *buf, MAILITEM *tpMailItem, TCHAR *quotstr);
TCHAR *CreateHeaderString(TCHAR *buf, TCHAR *ret, MAILITEM *tpMailItem, TCHAR *quotstr);
int GetReplyBodySize(TCHAR *body, TCHAR *ReStr);
TCHAR *SetReplyBody(TCHAR *body, TCHAR *ret, TCHAR *ReStr);
int SetDotSize(TCHAR *buf);
void SetDot(TCHAR *buf, TCHAR *ret);
void DelDot(TCHAR *buf, TCHAR *ret);
int WordBreakStringSize(TCHAR *buf, TCHAR *str, int BreakCnt, BOOL BreakFlag);
void WordBreakString(TCHAR *buf, TCHAR *ret, TCHAR *str, int BreakCnt, BOOL BreakFlag);
BOOL GetQuoteString(TCHAR *str, TCHAR *prefix, int maxlen);
BOOL URLToMailItem(TCHAR *buf, MAILITEM *tpMailItem);
TCHAR *GetMailAddress(TCHAR *buf, TCHAR *ret, TCHAR *comment, BOOL quote);
#ifdef _WIN32_WCE
void GetNameFromComment(TCHAR *cmmt, TCHAR *fname, TCHAR *lname);
#endif
TCHAR *GetMailString(TCHAR *buf, TCHAR *ret);
void SetUserName(TCHAR *buf, TCHAR *ret);
int SetCcAddressSize(TCHAR *To);
TCHAR *SetCcAddress(TCHAR *Type, TCHAR *To, TCHAR *r);
TCHAR *GetFileNameString(TCHAR *p);
int SetAttachListSize(TCHAR *buf);
TCHAR *SetAttachList(TCHAR *buf, TCHAR *ret);
char *GetMIME2Extension(char *MIMEStr, char *Filename);
TCHAR *CreateCommandLine(TCHAR *buf, TCHAR *filename, BOOL spFlag);
TCHAR *strip_html_tags(TCHAR *buf, int insert_notice);
int remove_superfluous_headers(char *buf, BOOL dups);
int remove_npopuk_headers(char *buf);
BOOL item_in_list(TCHAR *item, TCHAR *list);
void rot13_cpy(char *dest, char *start, char *end);
void rot13(char *start, char *end);
#ifndef _WIN32_WCE
TCHAR *replace_env_var(TCHAR *buf);
#endif

// View
BOOL FindEditString(HWND hEdit, TCHAR *strFind, int CaseFlag, int Wildcards, BOOL Loop, DWORD start, DWORD end);
void SetWordBreakMenu(HWND hWnd, HMENU hEditMenu, int Flag);
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
int SetWordBreak(HWND hWnd, HMENU hMenu, int cmd);
#else
int SetWordBreak(HWND hWnd, int cmd);
#endif
MAILITEM *View_NextPrev(HWND hWnd, int dir, BOOL isView);
void View_FindMail(HWND hWnd, BOOL FindSet);
void View_Scroll(HWND hWnd, int dir, BOOL ViewWnd);
BOOL View_InitApplication(HINSTANCE hInstance);
BOOL View_InitInstance(HINSTANCE hInstance, LPVOID lpParam, BOOL NoAppFlag);
BOOL AttachDecode(HWND hWnd, int id, int DoWhat);
BOOL DeleteAttachFile(HWND hWnd, MAILITEM *tpMailItem);
BOOL SaveViewMail(TCHAR *fname, HWND hWnd, int MailBoxIndex, MAILITEM *tpMailItem, TCHAR *head, BOOL ViewSrc);
BOOL ShellOpen(TCHAR *FileName);

// Edit
#ifndef _WIN32_WCE
BOOL CALLBACK enum_windows_proc(const HWND hWnd, const LPARAM lParam);
#endif
BOOL EndEditWindow(HWND hWnd, BOOL sent);
BOOL Edit_InitApplication(HINSTANCE hInstance);
int Edit_MailToSet(HINSTANCE hInstance, HWND hWnd, TCHAR *mail_addr, int rebox);
void Edit_ConfigureWindow(HWND thisEditWnd, BOOL editable);
int Edit_InitInstance(HINSTANCE hInstance, HWND hWnd, int rebox,
					   MAILITEM *tpReMailItem, int OpenFlag, TCHAR *seltext, BOOL NoAppFlag);
LRESULT CALLBACK SubClassSentProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Option
void SetControlFont(HWND pWnd);
void SetSip(HWND hDlg, int edit_notify);
int AllocGetText(HWND hEdit, TCHAR **buf);
int SetMailBoxType(HWND hWnd, int Type);
BOOL ImportSavebox(HWND hWnd);
BOOL SetSaveBoxOption(HWND hWnd);
BOOL CALLBACK StartConfigProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL SetMailBoxOption(HWND hWnd, BOOL SelFlag);
BOOL CALLBACK MailBoxSummaryProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetEncodeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SelSaveBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AdvOptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SocLogViewProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL SetOption(HWND hWnd);
BOOL CALLBACK InputPassProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InitMailBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void attach_item_free();
BOOL CALLBACK SetAttachProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SaveAttachProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CheckDependence(HWND hEdit, TCHAR *buf, TCHAR *charset);
BOOL CALLBACK SetSendProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MailPropProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AddressListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetFindProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NewMailMessageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AttachNoticeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DigestReplyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ListView
void ListView_AddColumn(HWND hListView, int fmt, int cx, TCHAR *buf, int iSubItem);
HWND CreateListView(HWND hWnd, int Top, int Bottom, int Left, int Right);
void ListView_SetRedraw(HWND hListView, BOOL DrawFlag);
int ListView_InsertItemEx(HWND hListView, TCHAR *buf, int len, int Img, long lp, int iItem);
void ListView_MoveItem(HWND hListView, int SelectItem, int Move, int ColCnt);
TCHAR *ListView_GetSelStringList(HWND hListView);
long ListView_GetlParam(HWND hListView, int i);
int ListView_GetMemToItem(HWND hListView, MAILITEM *tpMemMailItem);
int ListView_GetNextDeleteItem(HWND hListView, int Index);
int ListView_GetNextMailItem(HWND hListView, int Index);
int ListView_GetPrevMailItem(HWND hListView, int Index);
int ListView_GetNextUnreadItem(HWND hListView, int Index, int endindex);
int ListView_GetNewItem(HWND hListView, MAILBOX *tpMailBox);
BOOL ListView_ShowItem(HWND hListView, MAILBOX *tpMailBox, BOOL AddLast);
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK AddrCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam);
int ListView_ComputeState(int Priority, int Multipart);

// main
void SwitchCursor(const BOOL Flag);
#ifdef _WIN32_WCE
#define _SetForegroundWindow		SetForegroundWindow
#else
BOOL _SetForegroundWindow(const HWND hWnd);
BOOL ConfirmPass(HWND hWnd, TCHAR *ps, BOOL Show);
#endif
void SetStatusTextT(HWND hWnd, TCHAR *buf, int Part);
void SetSocStatusTextT(HWND hWnd, TCHAR *buf);
#ifdef UNICODE
void SetSocStatusText(HWND hWnd, char *buf);
#else
#define SetSocStatusText(hWnd, buf)	SetSocStatusTextT(hWnd, buf)
#endif
void SetItemCntStatusText(MAILBOX *tpViewMailBox, BOOL bNotify);
void SetStatusRecvLen(HWND hWnd, int len, int size, TCHAR *msg);
void ErrorMessage(HWND hWnd, TCHAR *buf);
void SocketErrorMessage(HWND hWnd, TCHAR *buf, int BoxIndex);
void ErrorSocketEnd(HWND hWnd, int BoxIndex);
void ShowMenu(HWND hWnd, HMENU hMenu, int mpos, int PosFlag, BOOL timer);
void SetMailMenu(HWND hWnd);
void SetUnreadCntTitle(BOOL CheckMsgs);
BOOL MessageFunc(HWND hWnd, MSG *msg);
void OpenItem(HWND hWnd, BOOL MsgFlag, BOOL NoAppFlag, BOOL CheckSel);
BOOL ItemToSaveBox(HWND hWnd, MAILITEM *tpSingleItem, int TargetBox, TCHAR *fname, BOOL ask, BOOL del);
void SetReplyFwdMark(MAILITEM *tpReMailItem, char Mark, int rebox);
void ResetTimeoutTimer();
int ParanoidMessageBox(HWND hWnd, TCHAR *strMsg, TCHAR *strTitle, unsigned int nStyle);
#ifndef _WIN32_WCE
void ListViewSortMenuCheck(int sort_flag);
#endif
void DeleteMBMenu(int EntryNum);
void SelectMBMenu(int EntryNum);
int GetSelectedMBMenu(void);
int AddMBMenu(TCHAR *Name);
void InsertMBMenu(int EntryNum, TCHAR *Name);
BOOL GetStarMBMenu();
void SetStarMBMenu(int Flag);

#endif
/* End of source */
