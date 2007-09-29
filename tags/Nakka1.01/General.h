/**************************************************************************

	nPOP

	General.h

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_MAIL_GENERAL_H
#define _INC_MAIL_GENERAL_H

/**************************************************************************
	Include Files
**************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <winsock.h>

#ifdef UNICODE
#include <tchar.h>
#include <stdlib.h>
#endif

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
#include "stdafx.h"
#endif
#include "resource.h"
#include "Strtbl.h"


/**************************************************************************
	Define
**************************************************************************/

#define APP_NAME				TEXT("nPOP Ver 1.0.1")
#define WINDOW_TITLE			TEXT("nPOP")
#define KEY_NAME				TEXT("nPOP")

#define STR_MUTEX				TEXT("_nPOP_Mutex_")

#ifdef _WIN32_WCE
#if _WIN32_WCE < 211
#define _WCE_OLD				1
#endif
#endif

#define MAIN_WND_CLASS			TEXT("nPOPMainWndClass")
#define VIEW_WND_CLASS			TEXT("nPOPViewWndClass")
#define EDIT_WND_CLASS			TEXT("nPOPEditWndClass")

#define ADDRESS_FILE_OLD		TEXT("Address.dat")
#define ADDRESS_FILE			TEXT("Address.lst")
#define SAVEBOX_FILE			TEXT("SaveBox.dat")
#define SENDBOX_FILE			TEXT("SendBox.dat")

#define VIEW_FILE				TEXT("$npop_view")
#ifdef _WIN32_WCE
#define EDIT_FILE				TEXT("$npop_edit")
#endif

#define LOG_FILE				TEXT("nPOP.log")

#define RAS_WAIT_EVENT			TEXT("RAS_WAIT_EVENT")
#define ID_RASWAIT_TIMER		10

#define BUF_SIZE				256					//�o�b�t�@�T�C�Y
#define MAXSIZE					32768
#define EDITMAXSIZE				30000

#define SICONSIZE				16					//�������A�C�R���̃T�C�Y
#define TB_ICONSIZE				16					//�c�[���o�[�̃{�^���T�C�Y

#define TABSTOPLEN				8					//TAB Stop

#define LV_COL_CNT				4					//ListView�̃J������

#define MAILBOX_SAVE			0					//�Œ胁�[���{�b�N�X
#define MAILBOX_SEND			1
#define MAILBOX_USER			2

#define IDC_COMBO				400					//�R���g���[��ID
#define IDC_LISTVIEW			401
#define IDC_STATUS				402

#ifndef LVS_EX_INFOTIP
#define LVS_EX_INFOTIP			0x400
#endif

#define WM_SOCK_SELECT			(WM_APP + 1)		//�\�P�b�g�C�x���g
#define WM_SOCK_RECV			(WM_APP + 2)		//�\�P�b�g��M�C�x���g
#define WM_LV_EVENT				(WM_APP + 3)		//���X�g�r���[�C�x���g
#define WM_STATUSTEXT			(WM_APP + 4)		//�X�e�[�^�X�o�[�֕�����ݒ�
#define WM_SHOWLASTWINDOW		(WM_APP + 5)		//�E�B���h�E�\��
#define WM_SMTP_SENDMAIL		(WM_APP + 6)		//���[�����M
#define WM_ENDCLOSE				(WM_APP + 7)		//�E�B���h�E�̏I��
#define WM_INITTRAYICON			(WM_APP + 8)		//�^�X�N�g���C�A�C�R���̏�����

#define WM_SHOWDIALOG			(WM_APP + 9)		//���[���������b�Z�[�W�\��
#define WM_ENDDIALOG			(WM_APP + 10)		//���[���������b�Z�[�W�I��

#define EDIT_OPEN				0					//���M���[���ҏW�̃^�C�v
#define EDIT_NEW				1
#define EDIT_REPLY				2

#define EDIT_NONEDIT			0					//���M���[���ҏW�̖߂�l
#define EDIT_INSIDEEDIT			1
#define EDIT_OUTSIDEEDIT		2

#define SELECT_MEM_ERROR		-2					//�\�P�b�g�����̖߂�l
#define SELECT_SOC_ERROR		-1
#define SELECT_SOC_CLOSE		0
#define SELECT_SOC_SUCCEED		1
#define SELECT_SOC_NODATA		2

#define SORT_NO					100					//�\�[�g�t���O
#define SORT_IOCN				101
#define SORT_THREAD				102

#define POP_ERR					-2					//POP3�R�}���h�t���O
#define POP_QUIT				-1
#define POP_START				0
#define POP_USER				1
#define POP_PASS				2
#define POP_LOGIN				3
#define POP_STAT				4
#define POP_LIST				5
#define POP_UIDL				6
#define POP_TOP					7
#define POP_RETR				8
#define POP_DELE				9

#define SMTP_ERR				POP_ERR				//SMTP�R�}���h�t���O
#define SMTP_QUIT				POP_QUIT
#define SMTP_START				POP_START
#define SMTP_EHLO				1
#define SMTP_AUTH				2
#define SMTP_AUTHLOGIN			3
#define SMTP_HELO				4
#define SMTP_RSET				5
#define SMTP_MAILFROM			6
#define SMTP_RCPTTO				7
#define SMTP_DATA				8
#define SMTP_SENDBODY			9
#define SMTP_NEXTSEND			10
#define SMTP_SENDEND			11

#define ICON_NON				0					//�A�C�R�����
#define ICON_MAIL				1
#define ICON_READ				2
#define ICON_DOWN				3
#define ICON_DEL				4
#define ICON_SENDMAIL			5
#define ICON_SEND				6
#define ICON_ERROR				7
#define ICON_NEW				8

#define FILTER_UNRECV			1					//�t�B���^�^�C�v
#define FILTER_RECV				2
#define FILTER_DOWNLOADMARK		4
#define FILTER_DELETEMARK		8
#define FILTER_READICON			16
#define FILTER_SAVE				32

#define MP_ERROR_FILE			-2					//�}���`�p�[�g�����̖߂�l
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
#define HEAD_ENCODING			"Content-Transfer-Encoding:"
#define HEAD_DISPOSITION		"Content-Disposition:"
#define HEAD_X_MAILER			"X-Mailer:"
#define HEAD_X_UIDL				"X-UIDL:"
#define HEAD_X_NO				"X-No:"
#define HEAD_X_STATUS			"X-Mark:"
#define HEAD_X_MSTATUS			"X-Status:"
#define HEAD_X_DOWNFLAG			"X-Download:"
#define HEAD_X_MAILBOX			"X-MailBox:"
#define HEAD_X_ATTACH			"X-Attach:"

#define HEAD_X_NO_OLD			"X-MailNo:"
#define HEAD_X_STATUS_OLD		"X-MarkStatus:"
#define HEAD_X_MSTATUS_OLD		"X-MailStatus:"
#define HEAD_X_DOWNFLAG_OLD		"X-MailDownload:"

#define RSET					"RSET\r\n"
#define QUIT					"QUIT\r\n"

#define URL_HTTP				TEXT("http://")
#define URL_HTTPS				TEXT("https://")
#define URL_FTP					TEXT("ftp://")
#define URL_MAILTO				TEXT("mailto:")


#define NULLCHECK_FREE(m)		((m != NULL) ? LocalFree(m) : 0)		//NULL�łȂ��������̉��
#define ABS(n)					((n < 0) ? (n * -1) : n)				//��Βl


#ifdef UNICODE
#define TcharToCharSize(wbuf) (WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, NULL))
#else
#define TcharToCharSize(wbuf) (lstrlen(wbuf) + 1)
#endif

#ifdef UNICODE
#define TcharToChar(wbuf, ret, len) (WideCharToMultiByte(CP_ACP, 0, wbuf, -1, ret, len, NULL, NULL))
#else
#define TcharToChar(wbuf, ret, len) (lstrcpyn(ret, wbuf, len))
#endif

#ifdef UNICODE
#define CharToTcharSize(buf) (MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0))
#else
#define CharToTcharSize(buf) (lstrlen(buf) + 1)
#endif

#ifdef UNICODE
#define CharToTchar(buf, wret, len) (MultiByteToWideChar(CP_ACP, 0, buf, -1, wret, len))
#else
#define CharToTchar(buf, wret, len) (lstrcpyn(wret, buf, len))
#endif

#ifdef UNICODE
#define tstrcpy					strcpy
#else
#define tstrcpy					lstrcpy
#endif

#ifdef UNICODE
#define tstrcat					strcat
#else
#define tstrcat					lstrcat
#endif

#ifdef UNICODE
#define tstrcmp					strcmp
#else
#define tstrcmp					lstrcmp
#endif

#ifdef UNICODE
#define tstrlen					strlen
#else
#define tstrlen					lstrlen
#endif

#ifndef UNICODE
#define _ttoi					a2i
#endif

#define tCopyMemory				memcpy
#define tZeroMemory(p, len)		(memset(p, 0, len))
#define tFillMemory(p, len, fill)	(memset(p, fill, len))


/**************************************************************************
	Struct
**************************************************************************/

struct TPFILTER{
	int Enable;
	int Action;

	TCHAR *Header1;
	TCHAR *Content1;

	TCHAR *Header2;
	TCHAR *Content2;
};

struct TPMAILBOX{
	TCHAR *Name;

	//POP
	TCHAR *Server;
	int Port;
	TCHAR *User;
	TCHAR *Pass;
	TCHAR *TmpPass;
	int APOP;
	unsigned long PopIP;

	int MailCnt;
	unsigned int MailSize;

	char *LastMessageId;
	int LastNo;

	//SMTP
	TCHAR *SmtpServer;
	int SmtpPort;
	TCHAR *UserName;
	TCHAR *MailAddress;
	TCHAR *Signature;
	TCHAR *ReplyTo;
	int MyAddr2Bcc;
	TCHAR *BccAddr;
	int PopBeforeSmtp;
	int SmtpAuth;
	int SmtpAuthType;
	int AuthUserPass;
	TCHAR *SmtpUser;
	TCHAR *SmtpPass;
	TCHAR *SmtpTmpPass;
	unsigned long SmtpIP;

	//Check
	BOOL NewMail;
	BOOL NoRead;
	int CyclicFlag;

	//Filter
	int FilterEnable;
	struct TPFILTER **tpFilter;
	int FilterCnt;

	//Ras
	int RasMode;
	TCHAR *RasEntry;
	int RasReCon;

	//MailItem
	struct TPMAILITEM **tpMailItem;
	int MailItemCnt;
	int AllocCnt;
};

struct TPMAILITEM{
	int No;
	int Status;
	int MailStatus;
	int PrevNo;
	int NextNo;
	int Indent;
	BOOL New;
	BOOL Download;
	BOOL Multipart;
	HWND hEditWnd;
	HANDLE hProcess;

	TCHAR *From;
	TCHAR *To;
	TCHAR *Cc;
	TCHAR *Bcc;
	TCHAR *Date;
	TCHAR *Size;
	TCHAR *Subject;
	TCHAR *ReplyTo;
	TCHAR *ContentType;
	TCHAR *Encoding;
	TCHAR *MessageID;
	TCHAR *UIDL;
	TCHAR *InReplyTo;
	TCHAR *References;
	TCHAR *Body;
	TCHAR *MailBox;

	TCHAR *Attach;
};

struct TPMULTIPART{
	TCHAR *ContentType;
	TCHAR *Filename;
	TCHAR *Encoding;
	TCHAR *sPos;
	TCHAR *ePos;
};

struct TPRASINFO{
	TCHAR *RasEntry;
	TCHAR *RasUser;
	TCHAR *RasPass;
};


/**************************************************************************
	Function Prototypes
**************************************************************************/
//--------------------------
//	String
//--------------------------

#ifdef UNICODE
char *AllocTcharToChar(TCHAR *str);
TCHAR *AllocCharToTchar(char *str);
#else
#define AllocTcharToChar AllocCopy
#define AllocCharToTchar AllocCopy
#endif

int a2i(const char *str);
void DelCtrlChar(TCHAR *buf);

TCHAR *AllocCopy(const TCHAR *buf);
TCHAR * __cdecl TStrJoin(TCHAR *ret, ... );

TCHAR *TStrCpy(TCHAR *ret, TCHAR *buf);
#ifdef UNICODE
char *StrCpy(char *ret, char *buf);
#else
#define StrCpy TStrCpy
#endif
#ifdef UNICODE
void StrCpyN(char *ret, char *buf, int len);
#else
#define StrCpyN TStrCpyN
#endif
void TStrCpyN(TCHAR *ret, TCHAR *buf, int len);
TCHAR *TStrCpyF(TCHAR *ret, TCHAR *buf, TCHAR c);
void StrCatN(TCHAR *ret, char *buf, int len);

int TStrCmp(const TCHAR *buf1, const TCHAR *buf2);
int TStrCmpI(const TCHAR *buf1, const TCHAR *buf2);
#ifdef UNICODE
int StrCmpI(const char *buf1, const char *buf2);
#else
#define StrCmpI TStrCmpI
#endif
int TStrCmpNI(const TCHAR *buf1, const TCHAR *buf2, int len);
#ifdef UNICODE
int StrCmpNI(const char *buf1, const char *buf2, int len);
#else
#define StrCmpNI TStrCmpNI
#endif

BOOL StrMatch(const TCHAR *Ptn, const TCHAR *Str);
TCHAR *StrFind(TCHAR *Ptn, TCHAR *Str, int CaseFlag);
TCHAR *GetHeaderStringPointT(TCHAR *buf, TCHAR *str);
int GetHeaderStringSizeT(TCHAR *buf, BOOL CrLfFlag);
BOOL GetHeaderStringT(TCHAR *buf, TCHAR *ret, BOOL CrLfFlag);


#ifdef UNICODE
char *GetHeaderStringPoint(char *buf, char *str);
#else
#define GetHeaderStringPoint GetHeaderStringPointT
#endif

#ifdef UNICODE
int GetHeaderStringSize(char *buf, BOOL CrLfFlag);
#else
#define GetHeaderStringSize GetHeaderStringSizeT
#endif

#ifdef UNICODE
BOOL GetHeaderString(char *buf, char *ret, BOOL CrLfFlag);
#else
#define GetHeaderString GetHeaderStringT
#endif

void TrimMessageId(char *buf);
int GetReferencesSize(char *p, BOOL Flag);
BOOL ConvReferences(char *p, char *r, BOOL Flag);
TCHAR *GetBodyPointaT(TCHAR *buf);

#ifdef UNICODE
char *GetBodyPointa(char *buf);
#else
#define GetBodyPointa GetBodyPointaT
#endif

void DateAdd(SYSTEMTIME *sTime, char *tz);
int DateConv(char *buf, char *ret);
int SortDateConv(char *buf, char *ret);
void GetTimeString(TCHAR *buf);
void EncodePassword(TCHAR *Key, TCHAR *Word, TCHAR *ret, int retsize, BOOL decode);
void EncodeCtrlChar(TCHAR *buf, TCHAR *ret);
void DecodeCtrlChar(TCHAR *buf, TCHAR *ret);
TCHAR *CreateMessageId(long id, TCHAR *MailAddress);
int CreateHeaderStringSize(TCHAR *buf, struct TPMAILITEM *tpMailItem);
TCHAR *CreateHeaderString(TCHAR *buf, TCHAR *ret, struct TPMAILITEM *tpMailItem);
int GetReplyBodySize(TCHAR *body, TCHAR *ReStr);
TCHAR *SetReplyBody(TCHAR *body, TCHAR *ret, TCHAR *ReStr);
int SetDotSize(TCHAR *buf);
void SetDot(TCHAR *buf, TCHAR *ret);
void DelDot(TCHAR *buf, TCHAR *ret);
int WordBreakStringSize(TCHAR *buf, TCHAR *str, int BreakCnt, BOOL BreakFlag);
void WordBreakString(TCHAR *buf, TCHAR *ret, TCHAR *str, int BreakCnt, BOOL BreakFlag);
BOOL URLToMailItem(TCHAR *buf, struct TPMAILITEM *tpMailItem);
TCHAR *GetMailAddress(TCHAR *buf, TCHAR *ret, BOOL quote);
TCHAR *GetMailString(TCHAR *buf, TCHAR *ret);
void SetUserName(TCHAR *buf, TCHAR *ret);
int SetCcAddressSize(TCHAR *To);
TCHAR *SetCcAddress(TCHAR *Type, TCHAR *To, TCHAR *r);
TCHAR *GetFileNameString(TCHAR *p);
int SetAttachListSize(TCHAR *buf);
TCHAR *SetAttachList(TCHAR *buf, TCHAR *ret);
TCHAR *GetMIME2Extension(TCHAR *MIMEStr, TCHAR *Filename);
TCHAR *CreateCommandLine(TCHAR *buf, TCHAR *filename, BOOL spFlag);

//--------------------------
//	Code
//--------------------------
int IsDependenceString(TCHAR *buf);
char *Base64Decode(char *buf, char *ret);
void Base64Encode(char *buf, char *ret, int size);
#ifdef UNICODE
void TBase64Encode(TCHAR *buf, TCHAR *ret, int size);
#else
#define TBase64Encode Base64Encode
#endif
char *QuotedPrintableDecode(char *buf, char *ret);
void QuotedPrintableEncode(unsigned char *buf, char *ret, int break_size);
char *URLDecode(char *buf, char *ret);
void URLEncode(unsigned char *buf, char *ret);
TCHAR *AllocURLDecode(TCHAR *buf);
void MIMEdecode(char *buf, char *ret);
TCHAR *MIMEencode(TCHAR *wbuf, BOOL Address);
TCHAR *ExtendedDecode(TCHAR *buf);
TCHAR *ExtendedEncode(TCHAR *wbuf);
char *DecodeBodyTransfer(struct TPMAILITEM *tpMailItem, char *body);
TCHAR *BodyDecode(struct TPMAILITEM *tpMailItem, BOOL ViewSrc, struct TPMULTIPART ***tpPart, int *cnt);
TCHAR *BodyEncode(TCHAR *body, TCHAR *content_type, TCHAR *encoding, TCHAR *ErrStr);

//--------------------------
//	MultiPart
//--------------------------
struct TPMULTIPART *AddMultiPartInfo(struct TPMULTIPART ***tpMultiPart, int cnt);
void FreeMultipartInfo(struct TPMULTIPART ***tpMultiPart, int cnt);
TCHAR *GetFilename(TCHAR *buf, TCHAR *Attribute);
int MultiPart_Parse(TCHAR *ContentType, TCHAR *buf, struct TPMULTIPART ***tpMultiPart, int cnt);
int CreateMultipart(TCHAR *Filename, TCHAR *ContentType, TCHAR *Encoding, TCHAR **RetContentType, TCHAR *body, TCHAR **RetBody);

//--------------------------
//	File
//--------------------------
BOOL SaveLog(TCHAR *fpath, TCHAR *fname, TCHAR *buf);
BOOL SaveLogSep(TCHAR *fpath, TCHAR *fname, TCHAR *buf);
BOOL LogClear(TCHAR *fpath, TCHAR *fname);
BOOL GetFileName(HWND hWnd, TCHAR *ret, TCHAR *DefExt, TCHAR *filter, BOOL OpenSave);
char *ReadFileBuf(TCHAR *path, long FileSize);
BOOL OpenFileBuf(HWND hWnd, TCHAR **buf);
BOOL SaveFile(HWND hWnd, TCHAR *FileName, TCHAR *Ext, char *buf, int len);
int GetSaveHeaderStringSize(TCHAR *Head, TCHAR *buf);
TCHAR *SaveHeaderString(TCHAR *Head, TCHAR *buf, TCHAR *ret);
BOOL WriteAsciiFile(HANDLE hFile, TCHAR *buf, int len);
void ConvFilename(TCHAR *buf);
BOOL SaveMail(TCHAR *FileName, struct TPMAILBOX *tpMailBox, int SaveFlag);
long GetFileSerchSize(TCHAR *FileName);
BOOL ReadItemList(TCHAR *FileName, struct TPMAILBOX *tpMailBox);
BOOL SaveAddressBook(TCHAR *FileName, struct TPMAILBOX *tpMailBox);
int ReadAddressBook(TCHAR *FileName, struct TPMAILBOX *tpMailBox);

//--------------------------
//	Ini
//--------------------------
void FreeIniInfo(void);
#ifndef _WIN32_WCE
BOOL CheckStartPass(void);
#endif
BOOL GetINI(HWND hWnd);
BOOL PutINI(HWND hWnd, BOOL SaveMailFlag);

//--------------------------
//	Font
//--------------------------
HFONT CreateEditFont(HWND hWnd, TCHAR *FontName, int FontSize, int Charset);

//--------------------------
//	Ras
//--------------------------
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

//--------------------------
//	WinSock
//--------------------------
unsigned long GetHostByName(HWND hWnd, TCHAR *Server, TCHAR *ErrStr);
SOCKET ConnectServer(HWND hWnd, unsigned long IPaddr, unsigned short Port, TCHAR *ErrStr);
int RecvBufProc(HWND hWnd, SOCKET soc);
#ifndef WSAASYNC
int RecvSelect(HWND hWnd, SOCKET soc);
#endif
int Tsend(SOCKET soc, TCHAR *wBuf);
int SendBuf(SOCKET soc, char *buf);
#ifdef UNICODE
int TSendBuf(SOCKET soc, TCHAR *wBuf);
#else
#define TSendBuf	SendBuf
#endif
void SocketClose(HWND hWnd, SOCKET soc);

//--------------------------
//	Pop3
//--------------------------
void FreeMailBuf(void);
void FreeUidlList(void);
BOOL ListPopProc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr,
						  struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
BOOL DownLoadPopProc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr,
							  struct TPMAILBOX *tpMailBox, BOOL ShowFlag);

//--------------------------
//	Smtp
//--------------------------
void HMAC_MD5(unsigned char *input, int len, unsigned char *key, int keylen, unsigned char *digest);
void SmtpError(HWND hWnd);
BOOL SmtpProc(HWND hWnd, SOCKET soc, char *buf, int len, TCHAR *ErrStr,
					   struct TPMAILBOX *tpMailBox, BOOL ShowFlag);
SOCKET SendMailItem(HWND hWnd, struct TPMAILBOX *tpMailBox, struct TPMAILITEM *tpMailItem, int EndMailFlag, TCHAR *ErrStr);
#ifdef WSAASYNC
BOOL SendProc(HWND hWnd, SOCKET soc, TCHAR *ErrStr, struct TPMAILBOX *tpMailBox);
#endif

//--------------------------
//	ListView
//--------------------------
void ListView_AddColumn(HWND hListView, int fmt, int cx, TCHAR *buf, int iSubItem);
HWND CreateListView(HWND hWnd, int Top, int bottom);
void ListView_SetRedraw(HWND hListView, BOOL DrawFlag);
int ListView_InsertItemEx(HWND hListView, TCHAR *buf, int len, int Img, long lp, int iItem);
void ListView_MoveItem(HWND hListView, int SelectItem, int Move, int ColCnt);
TCHAR *ListView_GetSelStringList(HWND hListView);
long ListView_GetlParam(HWND hListView, int i);
int ListView_GetMemToItem(HWND hListView, struct TPMAILITEM *tpMemMailItem);
int ListView_GetNextDeleteItem(HWND hListView, int Index);
int ListView_GetNextMailItem(HWND hListView, int Index);
int ListView_GetPrevMailItem(HWND hListView, int Index);
int ListView_GetNextNoReadItem(HWND hListView, int Index, int endindex);
int ListView_GetNewItem(HWND hListView, struct TPMAILBOX *tpMailBox);
BOOL ListView_ShowItem(HWND hListView, struct TPMAILBOX *tpMailBox);
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam);

//--------------------------
//	Item
//--------------------------
BOOL Item_SetItemCnt(struct TPMAILBOX *tpMailBox, int i);
BOOL Item_Add(struct TPMAILBOX *tpMailBox, struct TPMAILITEM *tpNewMailItem);
void CopyItem(struct TPMAILITEM *tpFromNewMailItem, struct TPMAILITEM *tpToMailItem);
struct TPMAILITEM *Item_CopyMailBox(struct TPMAILBOX *tpMailBox, struct TPMAILITEM *tpNewMailItem,
									TCHAR *MailBoxName, BOOL SendClear);
BOOL Item_Resize(struct TPMAILBOX *tpMailBox);
void FreeMailItem(struct TPMAILITEM **tpFreeMailItem, int Cnt);
int Item_GetContent(char *buf, char *str, char **ret);
char *Item_GetMessageId(char *buf);
BOOL Item_SetMailItem(struct TPMAILITEM *tpMailItem, char *buf, char *Size);
struct TPMAILITEM *Item_HeadToItem(struct TPMAILBOX *tpMailBox, char *buf, char *Size);
struct TPMAILITEM *Item_StringToItem(struct TPMAILBOX *tpMailBox, TCHAR *buf);
int Item_GetStringSize(struct TPMAILITEM *tpMailItem, BOOL BodyFlag);
TCHAR *Item_GetString(TCHAR *buf, struct TPMAILITEM *tpMailItem, BOOL BodyFlag);
int Item_GetNextDonloadItem(struct TPMAILBOX *tpMailBox, int Index, int *No);
int Item_GetNextSendItem(struct TPMAILBOX *tpMailBox, int Index, int *MailBoxIndex);
int Item_GetNextMailBoxSendItem(struct TPMAILBOX *tpMailBox, int Index, int MailBoxIndex);
int Item_GetNextDeleteItem(struct TPMAILBOX *tpMailBox, int Index, int *No);
int Item_GetMailNoToItemIndex(struct TPMAILBOX *tpMailBox, int No);
BOOL Item_IsMailBox(struct TPMAILBOX *tpMailBox, struct TPMAILITEM *tpMailItem);
int Item_FindThread(struct TPMAILBOX *tpMailBox, TCHAR *p, int Index);
void Item_SetThread(struct TPMAILBOX *tpMailBox);

//--------------------------
//	MailBox
//--------------------------
BOOL InitMailBox(void);
void FreeFilterInfo(struct TPMAILBOX *tpMailBox);
void FreeMailBox(struct TPMAILBOX *tpMailBox);
BOOL ReadMailBox(void);
int CreateMailBox(HWND hWnd, BOOL ShowFlag);
int DeleteMailBox(HWND hWnd, int DelIndex);
void MoveUpMailBox(HWND hWnd);
void MoveDownMailBox(HWND hWnd);
BOOL CheckNoReadMailBox(int index, BOOL NewFlag);
int NextNoReadMailBox(int index, int endindex);
void SetNoReadCntTitle(HWND hWnd);
void SelectMailBox(HWND hWnd, int Sel);
int GetNameToMailBox(TCHAR *Name);

//--------------------------
//	View
//--------------------------
void SetWordBreakMenu(HWND hWnd, HMENU hEditMenu, int Flag);
#ifdef _WIN32_WCE_LAGENDA
int SetWordBreak(HWND hWnd, HMENU hMenu);
#else
int SetWordBreak(HWND hWnd);
#endif
void View_FindMail(HWND hWnd, BOOL FindSet);
BOOL View_InitApplication(HINSTANCE hInstance);
BOOL View_InitInstance(HINSTANCE hInstance, LPVOID lpParam, BOOL NoAppFlag);

//--------------------------
//	Edit
//--------------------------
BOOL Edit_InitApplication(HINSTANCE hInstance);
int Edit_MailToSet(HINSTANCE hInstance, HWND hWnd, TCHAR *mail_addr, int rebox);
int Edit_InitInstance(HINSTANCE hInstance, HWND hWnd, int rebox,
					   struct TPMAILITEM *tpReMailItem, int OpenFlag, int ReplyFag);

//--------------------------
//	Option
//--------------------------
void AllocGetText(HWND hEdit, TCHAR **buf);
BOOL SetMailBoxOption(HWND hWnd);
void SetOption(HWND hWnd);
BOOL CALLBACK InputPassProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InitMailBoxProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CheckDependence(HWND hWnd, int Ctl);
BOOL CALLBACK SetSendProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MailPropProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AddressListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SetFindProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NewMailMessageProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//--------------------------
//	main
//--------------------------
void SwitchCursor(const BOOL Flag);
#ifdef _WIN32_WCE
#define _SetForegroundWindow		SetForegroundWindow
#else
BOOL _SetForegroundWindow(const HWND hWnd);
#endif
void SetStatusTextT(HWND hWnd, TCHAR *buf, int Part);
void SetSocStatusTextT(HWND hWnd, TCHAR *buf, int Part);
#ifdef UNICODE
void SetStatusText(HWND hWnd, char *buf);
#else
#define SetStatusText(hWnd, buf)	SetSocStatusTextT(hWnd, buf, 1)
#endif
void SetItemCntStatusText(HWND hWnd, struct TPMAILBOX *tpViewMailBox);
void SetStatusRecvLen(HWND hWnd, int len, TCHAR *msg);
void ErrorMessage(HWND hWnd, TCHAR *buf);
void SocketErrorMessage(HWND hWnd, TCHAR *buf, int BoxIndex);
void ErrorSocketEnd(HWND hWnd, int BoxIndex);
int ShowMenu(HWND hWnd, HMENU hMenu, int mpos, int PosFlag, BOOL ReturnFlag);
void SetMailMenu(HWND hWnd);
BOOL MessageFunc(HWND hWnd, MSG *msg);

#endif
/* End of source */