/*
 * nPOP
 *
 * General.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2008 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopsupport.org.uk
 */

#ifndef _INC_MAIL_GENERAL_H
#define _INC_MAIL_GENERAL_H

// hacks for old compilers (VC++6)
#if defined( _MSC_VER) && (_MSC_VER <= 1200)
#define strcpy_s(dest,size,src) strcpy(dest,src)
#define strcat_s(dest,size,append) strcat(dest,append)
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
#define APP_VERSION_NUM			2009
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

#define MAIN_WND_CLASS			TEXT("nPOPMainWndClass")
#define VIEW_WND_CLASS			TEXT("nPOPViewWndClass")
#define EDIT_WND_CLASS			TEXT("nPOPEditWndClass")

#define ADDRESS_FILE			TEXT("Address.lst")
#define SAVEBOX_FILE			TEXT("SaveBox.dat")
#define SENDBOX_FILE			TEXT("SendBox.dat")

#define VIEW_FILE				TEXT("$npop_view")
#ifdef _WIN32_WCE
#define EDIT_FILE				TEXT("$npop_edit")
#endif

#define LOG_FILE				TEXT("nPOPuk.log")

#define RAS_WAIT_EVENT			TEXT("RAS_WAIT_EVENT")
#define ID_RASWAIT_TIMER		10

#define BUF_SIZE				256					//of process Buffer size
#define MSG_SIZE				300					//BUF_SIZE + some text
#define MULTI_BUF_SIZE			1024
#define MAXSIZE					32768
#define EDITMAXSIZE				60000

#define MAX_MAILBOX_CNT			1000				// see ID_MENUITEM_COPY2MBOX
#define MAX_TRAY_CNT			10					// see ID_MENUITEM_TRAYCHECK

#define SICONSIZE				16					//Size
#define TB_ICONSIZE				16					//of small idea contest Column several

#define TABSTOPLEN				8					// TAB Stop

#define LV_COL_CNT				4					//ListView of tool bar Fixed mailbox
#define AD_COL_CNT				3					// ListView Addressbook
#define MB_COL_CNT				5					// ListView Mailboxes

#define MAILBOX_SEND			0
#define MAILBOX_USER			1
#define MAILBOX_TYPE_SAVE		1
#define MAILBOX_IMPORT_SAVE		2
#define MAILBOX_ADD_SAVE		10

#define MAILITEMS_CHANGED		1					// for NeedsSave
#define MARKS_CHANGED			2
#define MBOX_FORMAT_CHANGED		4

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

#define EDIT_OPEN				0					//Type
#define EDIT_NEW				1
#define EDIT_REPLY				2
#define EDIT_REPLYALL			3
#define EDIT_FORWARD			4					// Added PHH 4-Oct-2003

#define EDIT_NONEDIT			0					//of transmission mail compilation Return value
#define EDIT_INSIDEEDIT			1
#define EDIT_OUTSIDEEDIT		2
#define EDIT_SEND				3

#define SELECT_MEM_ERROR		-2					//of transmission mail compilation Return value
#define SELECT_SOC_ERROR		-1
#define SELECT_SOC_CLOSE		0
#define SELECT_SOC_SUCCEED		1
#define SELECT_SOC_NODATA		2

#define SORT_NO					100					//of socket processing Sort flag
#define SORT_ICON				101
#define SORT_THREAD				102

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
#define SMTP_DATA				10
#define SMTP_SENDBODY			11
#define SMTP_NEXTSEND			12
#define SMTP_SENDEND			13

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

// marks for forwarding/replying and the property dialog
#define MARK_REPLYING			1
#define MARK_REFWD_SELTEXT		2
#define MARK_FORWARDING			3

#define FILE_OPEN_SINGLE		0
#define FILE_OPEN_MULTI			1
#define FILE_SAVE_SINGLE		2
#define FILE_SAVE_MULTI			3
#define FILE_SAVE_MSG			4
#define FILE_CHOOSE_DIR			5

#define DECODE_ASK					0
#define DECODE_AUTO_OPEN			1
#define DECODE_SAVE_ALL				2
#define DECODE_OPEN_IF_MSG			3
#define DECODE_SAVE_EMBED			4

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

#define MP_ERROR_FILE			-2					//Is not the return value
#define MP_ERROR_ALLOC			-1
#define MP_NO_ATTACH			0
#define MP_ATTACH				1

#define HEAD_SUBJECT			"Subject:"
#define HEAD_FROM				"From:"
#define HEAD_TO					"To:"
#define HEAD_CC					"Cc:"
#define HEAD_BCC				"Bcc:"
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
	int LvThreadView;
	int LvStyle;
	int LvStyleEx;
	TCHAR *LvColumnOrder;
	int MBMenuWidth;
	int MBMenuHeight; // not saved in INI, for resizing
	int ScanAllForUnread;
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
	TCHAR *AddressShowGroup;

	int ListGetLine;
	int ListDownload;
	int ShowHeader;
	int ListSaveMode;
	int WordBreakFlag;
	int EditWordBreakFlag;
	int ViewShowDate;
	int MatchCase;
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

	// SSL
	TCHAR *CAFile;

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

	int NewMailSound;
	int ExecEndSound;
	int ItemPlaySound;
	TCHAR *NewMailSoundFile;
	TCHAR *ExecEndSoundFile;
	TCHAR *SoundDirSetting; // in ini file, may be relative to DataDir or AppDir
	TCHAR *SoundDirectory; // actual path
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
	int TimeoutInterval;

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
#endif
#ifdef _WIN32_WCE_PPC
	int UseBuiltinSSL;
	int ShowNavButtons;
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
	BOOL New;
	BOOL Download;
	int HasHeader;  // 0=no, 1=full headers, 2=stripped of duplicates
	BOOL DefReplyTo;
	HWND hEditWnd;
	HANDLE hProcess;

	TCHAR *From;
	TCHAR *To;
	TCHAR *Cc;
	TCHAR *Bcc;
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
	FILTER_PRIORITY_INDEX
};
#define FILTER_BOOL_AND 0
#define FILTER_BOOL_OR 1
#define FILTER_BOOL_UNLESS 2

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

// Pop3
BOOL pop3_list_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
BOOL pop3_exec_proc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr, MAILBOX *tpMailBox, BOOL ShowFlag);
BOOL pop3_salvage_buffer(HWND hWnd, MAILBOX *tpMailBox, BOOL ShowFlag);
char *claim_mail_buf(char *buf);
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
void smtp_set_error(HWND hWnd);
void smtp_free(void);

// File
void log_flush(void);
BOOL log_save(TCHAR *buf);
BOOL log_header(TCHAR *buf);
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
int file_read_address_book(TCHAR *FileName, ADDRESSBOOK *tpAddrBook);
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
BOOL ini_read_setting(HWND hWnd);
BOOL ini_save_setting(HWND hWnd, BOOL SaveMailFlag, BOOL SaveAll, TCHAR *SaveDir);
void ini_free(void);

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
int item_get_next_delete_mark(MAILBOX *tpMailBox, int Index, int *No);
int item_get_next_new(MAILBOX *tpMailBox, int Index, int *No);
int item_get_next_send_mark(MAILBOX *tpMailBox, BOOL CheckErrors);
int item_get_next_send_mark_mailbox(MAILBOX *tpMailBox, int Index, int MailBoxIndex);
BOOL item_mail_to_item(MAILITEM *tpMailItem, char *buf, int Size, BOOL download, MAILBOX *tpMailBox);
MAILITEM *item_header_to_item(MAILBOX *tpMailBox, char *buf, int Size);
MAILITEM *item_string_to_item(MAILBOX *tpMailBox, char *buf, BOOL Import);
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
void mailbox_move_up(HWND hWnd, BOOL select);
void mailbox_move_down(HWND hWnd, BOOL select);
BOOL mailbox_unread_check(int index, BOOL NewFlag);
int mailbox_next_unread(HWND hWnd, int index, int endindex);
void mailbox_select(HWND hWnd, int Sel);
BOOL mailbox_menu_rebuild(HWND hWnd, BOOL IsAttach);
int mailbox_name_to_index(TCHAR *Name);
void filter_sbox_check(HWND hWnd, TCHAR *ConvertName);
void filter_free(MAILBOX *tpMailBox);
ADDRESSBOOK *addressbook_copy(void);
BOOL addr_add(ADDRESSBOOK *tpAddrBook, ADDRESSITEM *tpNewAddrItem);
void addr_move(ADDRESSBOOK *tpAddrBook, int num, int step);
void addr_delete(ADDRESSBOOK *tpAddrBook, int num);
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
TCHAR *GetMailString(TCHAR *buf, TCHAR *ret);
void SetUserName(TCHAR *buf, TCHAR *ret);
int SetCcAddressSize(TCHAR *To);
TCHAR *SetCcAddress(TCHAR *Type, TCHAR *To, TCHAR *r);
TCHAR *GetFileNameString(TCHAR *p);
int SetAttachListSize(TCHAR *buf);
TCHAR *SetAttachList(TCHAR *buf, TCHAR *ret);
char *GetMIME2Extension(char *MIMEStr, char *Filename);
TCHAR *CreateCommandLine(TCHAR *buf, TCHAR *filename, BOOL spFlag);
TCHAR *strip_html_tags(TCHAR *buf, BOOL insert_notice);
int remove_duplicate_headers(char *buf);
BOOL item_in_list(TCHAR *item, TCHAR *list);
void rot13_cpy(char *dest, char *start, char *end);
void rot13(char *start, char *end);
#ifndef _WIN32_WCE
TCHAR *replace_env_var(TCHAR *buf);
#endif

// View
BOOL FindEditString(HWND hEdit, TCHAR *strFind, int CaseFlag, BOOL Loop);
void SetWordBreakMenu(HWND hWnd, HMENU hEditMenu, int Flag);
#ifdef _WIN32_WCE_PPC
int SetWordBreak(HWND hWnd, HWND hToolBar);
#elif defined(_WIN32_WCE_LAGENDA)
int SetWordBreak(HWND hWnd, HMENU hMenu);
#else
int SetWordBreak(HWND hWnd);
#endif
void View_FindMail(HWND hWnd, BOOL FindSet);
BOOL View_InitApplication(HINSTANCE hInstance);
BOOL View_InitInstance(HINSTANCE hInstance, LPVOID lpParam, BOOL NoAppFlag);
BOOL DeleteAttachFile(HWND hWnd, MAILITEM *tpMailItem);
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
					   MAILITEM *tpReMailItem, int OpenFlag, TCHAR *seltext);

// Option
int AllocGetText(HWND hEdit, TCHAR **buf);
int SetMailBoxType(HWND hWnd, int Type);
BOOL ImportSavebox(HWND hWnd);
BOOL SetSaveBoxName(HWND hWnd);
BOOL SetMailBoxOption(HWND hWnd);
BOOL CALLBACK MailBoxSummaryProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetEncodeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SelSaveBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL SetOption(HWND hWnd);
BOOL CALLBACK InputPassProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InitMailBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetAttachProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CheckDependence(HWND hWnd, int Ctl, TCHAR **buf);
BOOL CALLBACK SetSendProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MailPropProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AddressListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetFindProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NewMailMessageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AttachNoticeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DigestReplyProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// ListView
void ListView_AddColumn(HWND hListView, int fmt, int cx, TCHAR *buf, int iSubItem);
HWND CreateListView(HWND hWnd, int Top, int Bottom, int Left);
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
#endif
void SetStatusTextT(HWND hWnd, TCHAR *buf, int Part);
void SetSocStatusTextT(HWND hWnd, TCHAR *buf);
#ifdef UNICODE
void SetSocStatusText(HWND hWnd, char *buf);
#else
#define SetSocStatusText(hWnd, buf)	SetSocStatusTextT(hWnd, buf)
#endif
void SetItemCntStatusText(HWND hWnd, MAILBOX *tpViewMailBox, BOOL bNotify);
void SetStatusRecvLen(HWND hWnd, int len, int size, TCHAR *msg);
void ErrorMessage(HWND hWnd, TCHAR *buf);
void SocketErrorMessage(HWND hWnd, TCHAR *buf, int BoxIndex);
void ErrorSocketEnd(HWND hWnd, int BoxIndex);
int ShowMenu(HWND hWnd, HMENU hMenu, int mpos, int PosFlag, BOOL ReturnFlag);
int SetMailMenu(HWND hWnd);
void SetUnreadCntTitle(HWND hWnd, BOOL CheckMsgs);
BOOL MessageFunc(HWND hWnd, MSG *msg);
void OpenItem(HWND hWnd, BOOL MsgFlag, BOOL NoAppFlag);
BOOL ItemToSaveBox(HWND hWnd, MAILITEM *tpSingleItem, int TargetBox, TCHAR *fname, BOOL ask, BOOL delete);
void SetReplyFwdMark(MAILITEM *tpReMailItem, char Mark, int rebox);
void ResetTimeoutTimer();
int ParanoidMessageBox(HWND hWnd, TCHAR *strMsg, TCHAR *strTitle, unsigned int nStyle);
void DeleteMBMenu(int EntryNum);
void SelectMBMenu(int EntryNum);
int GetSelectedMBMenu(void);
int AddMBMenu(TCHAR *Name);
void InsertMBMenu(int EntryNum, TCHAR *Name);
BOOL GetStarMBMenu(int EntryNum, TCHAR *Name);
void SetStarMBMenu(int EntryNum, TCHAR *Name, BOOL UseFlag, BOOL SetCurSel);

#endif
/* End of source */
