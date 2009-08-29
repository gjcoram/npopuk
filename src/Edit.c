/*
 * nPOP
 *
 * Edit.c
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
#include "Charset.h"
#include "mime.h"
#include "multipart.h"
#ifdef USE_NEDIT
#include "nEdit.h"
#endif

/* Define */
#define IDC_VCB				2000
#define IDC_VTB				2001
#define IDC_HEADER			2002
#define IDC_EDIT_BODY		2003

#define ID_APP_TIMER		1
#define ID_WAIT_TIMER		2

#define REPLY_SUBJECT		TEXT("Re:")
#define WNDPROC_KEY			TEXT("OldWndProc")

/* Global Variables */
HWND hEditWnd = NULL;
HWND hEditToolBar = NULL;
TCHAR *tmp_attach = NULL;
static int EditMaxLength;
static BOOL ProcessFlag;

#ifdef _WIN32_WCE_PPC
char EditMenuOpened = 0;
#endif

#ifdef _WIN32_WCE
static WNDPROC EditWindowProcedure;
#endif

#ifdef _WIN32_WCE_LAGENDA
static HMENU hViewMenu;
static int g_menu_height;
#endif

//External reference
extern OPTION op;
extern BOOL gSendAndQuit;

extern HINSTANCE hInst;  // Local copy of hInstance
extern TCHAR *DataDir;
extern HWND MainWnd;
extern HWND FocusWnd;
extern HFONT hListFont;
extern HWND hViewWnd;
extern HFONT hViewFont;
extern int font_charset;
extern int FindOrReplace;

extern MAILBOX *MailBox;
extern int SelBox;

extern SOCKET g_soc;
extern BOOL gSockFlag;

#ifdef _WIN32_WCE
extern HMENU hEditPop;
#endif

/* Local Function Prototypes */
static int GetCcListSize(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress);
static TCHAR *SetCcList(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress, TCHAR *ret);
static void SetAllReMessage(MAILITEM *tpMailItem, MAILITEM *tpReMailItem);
static void SetReplyMessage(MAILITEM *tpMailItem, MAILITEM *tpReMailItem, int rebox, int ReplyFlag);
static void SetReplyMessageBody(MAILITEM *tpMailItem, MAILITEM *tpReMailItem, int ReplyFlag, TCHAR *seltext);
static void SetWindowString(HWND hWnd, TCHAR *Subject, BOOL editable);
static void SetHeaderString(HWND hHeader, MAILITEM *tpMailItem);
static void SetBodyContents(HWND hWnd, MAILITEM *tpMailItem);
#ifndef _WIN32_WCE
static LRESULT TbNotifyProc(HWND hWnd,LPARAM lParam);
#endif
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam);
static BOOL InitWindow(HWND hWnd, MAILITEM *tpMailItem);
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
static void SetEditMenu(HWND hWnd);
static BOOL SetItemToSendBox(HWND hWnd, MAILITEM *tpMailItem, BOOL BodyFlag, int EndFlag, BOOL MarkFlag);
static BOOL CloseEditMail(HWND hWnd, BOOL SendFlag, BOOL ShowFlag);
static void UpdateEditWindow(HWND hWnd, MAILITEM *tpNewMailItem);
static void ShowSendInfo(HWND hWnd);
static BOOL AppEditMail(HWND hWnd, long id, char *buf, MAILITEM *tpMailItem);
static BOOL ReadEditMail(HWND hWnd, long id, MAILITEM *tpMailItem, BOOL ReadFlag);
static LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam,LPARAM lParam);

/*
 * enum_windows_proc
 */
#ifndef _WIN32_WCE
BOOL CALLBACK enum_windows_proc(const HWND hWnd, const LPARAM lParam)
{
	TCHAR class_name[BUF_SIZE];

	GetClassName(hWnd, class_name, BUF_SIZE - 1);
	if (lstrcmp(class_name, EDIT_WND_CLASS) == 0) {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
	}
	return TRUE;
}
#endif

/*
 * GetCcListSize - Length of the list of mail address is acquired
 */
static int GetCcListSize(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress)
{
	TCHAR *p;
	int cnt = 0;

	if (To == NULL) {
		return cnt;
	}

	p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(To) + 1));
	if (p == NULL) {
		return cnt;
	}
	while (*To != TEXT('\0')) {
		GetMailAddress(To, p, NULL, FALSE);
		if ((MyMailAddress != NULL && lstrcmpi(MyMailAddress, p) == 0) ||
			(ToMailAddress != NULL && lstrcmpi(ToMailAddress, p) == 0)) {
			//In case of your own address or the address which is set to To it does not count
			To = GetMailString(To, p);
		} else {
			To = GetMailString(To, p);
			cnt += 4;
			//Acquisition size of mail address
			cnt += lstrlen(p);
		}
		To = (*To != TEXT('\0')) ? To + 1 : To;
	}
	mem_free(&p);
	return cnt;
}

/*
 * SetCcList - Mail address is added to Cc list
 */
static TCHAR *SetCcList(TCHAR *To, TCHAR *MyMailAddress, TCHAR *ToMailAddress, TCHAR *ret)
{
	TCHAR *p, *r;

	*ret = TEXT('\0');

	if (To == NULL) {
		return ret;
	}

	r = ret;
	p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(To) + 1));
	if (p == NULL) {
		return ret;
	}
	while (*To != TEXT('\0')) {
		GetMailAddress(To, p, NULL, FALSE);
		if ((MyMailAddress != NULL && lstrcmpi(MyMailAddress, p) == 0) ||
			(ToMailAddress != NULL && lstrcmpi(ToMailAddress, p) == 0)) {
			//In case of your own address or the address which is set to To it does not add
			To = GetMailString(To, p);
		} else {
			To = GetMailString(To, p);
			if (ret != r) {
				//Addition
				r = str_cpy_t(r, TEXT(",\r\n "));
			}
			r = str_cpy_t(r, p);
		}
		To = (*To != TEXT('\0')) ? To + 1 : To;
	}
	mem_free(&p);
	*r = TEXT('\0');
	return r;
}

/*
 * SetAllReMessage - of window It sets reply in everyone
 */
static void SetAllReMessage(MAILITEM *tpMailItem, MAILITEM *tpReMailItem)
{
	TCHAR *MyMailAddress = NULL;
	TCHAR *ToMailAddress = NULL;
	TCHAR *r;
	int ToSize;
	int CcSize;
	int FromSize = 0;
	int i;

	//of pause Acquisition
	i = mailbox_name_to_index(tpMailItem->MailBox);
	if (i != -1) {
		MyMailAddress = (MailBox + i)->MailAddress;
	}
	//of your own mail address Acquisition
	if (tpMailItem->To != NULL) {
		ToMailAddress = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(tpMailItem->To) + 1));
		if (ToMailAddress != NULL) {
			GetMailAddress(tpMailItem->To, ToMailAddress, NULL, FALSE);
		}
	}

	//of mail address ahead transmitting When size acquisition
	ToSize = GetCcListSize(tpReMailItem->To, MyMailAddress, ToMailAddress);
	CcSize = GetCcListSize(tpReMailItem->Cc, MyMailAddress, ToMailAddress);
	if (tpReMailItem->ReplyTo != NULL) {
		// ReplyTo is set,
		FromSize = GetCcListSize(tpReMailItem->From, MyMailAddress, ToMailAddress);
	}
	if ((FromSize + ToSize + CcSize) <= 0) {
		mem_free(&ToMailAddress);
		return;
	}
	ToSize += ((ToSize > 0 && CcSize > 0) ? 4 : 0);
	ToSize += (((ToSize > 0 || CcSize > 0) && FromSize > 0) ? 4 : 0);

	tpMailItem->Cc = (TCHAR *)mem_alloc(sizeof(TCHAR) * (ToSize + CcSize + FromSize + 1));
	if (tpMailItem->Cc == NULL) {
		mem_free(&ToMailAddress);
		return;
	}
	//To which adds From to Cc to Cc list to Cc list is added to Cc list
	*tpMailItem->Cc = TEXT('\0');
	r = SetCcList(tpReMailItem->To, MyMailAddress, ToMailAddress, tpMailItem->Cc);
	if (CcSize > 0 && *tpMailItem->Cc != TEXT('\0')) {
		r = str_cpy_t(r, TEXT(",\r\n "));
	}
	// Cc which adds
	r = SetCcList(tpReMailItem->Cc, MyMailAddress, ToMailAddress, r);
	//From which adds
	if (FromSize > 0) {
		if (*tpMailItem->Cc != TEXT('\0')) {
			r = str_cpy_t(r, TEXT(",\r\n "));
		}
		SetCcList(tpReMailItem->From, MyMailAddress, ToMailAddress, r);
	}
	mem_free(&ToMailAddress);
}

/*
 * SetReplyMessage - It sets the reply mail and also forwards mail (Added PHH 4-Oct-2003)
 */
static void SetReplyMessage(MAILITEM *tpMailItem, MAILITEM *tpReMailItem, int rebox, int ReplyFlag)
{
	TCHAR *p;
	TCHAR *subject;
	TCHAR *strPrefix;
	char *q;
	int len = 0;
	BOOL is_fwd = ReplyFlag == EDIT_FORWARD || ReplyFlag == EDIT_FILTERFORWARD
		|| ReplyFlag == EDIT_REDIRECT;

	if (rebox != MAILBOX_SEND && (MailBox + rebox)->Type != MAILBOX_TYPE_SAVE) {
		tpMailItem->MailBox = alloc_copy_t((MailBox + rebox)->Name);
	} else if (tpReMailItem->MailBox != NULL) {
		tpMailItem->MailBox = alloc_copy_t(tpReMailItem->MailBox);
	} else if ((MailBox+rebox)->Type == MAILBOX_TYPE_SAVE 
		&& (MailBox+rebox)->DefAccount != NULL && *(MailBox+rebox)->DefAccount != TEXT('\0')) {
		tpMailItem->MailBox = alloc_copy_t((MailBox+rebox)->DefAccount);
	}

	// Set the appropriate To/From settings
	if (is_fwd) {	// It's actually forwarding
		//tpMailItem->To = NULL;				// don't set the To:
		strPrefix = op.FwdSubject;
		tpMailItem->AttachSize = 0;

		if (tpReMailItem->Attach != NULL) {
			// forwarding one's own message from MAILBOX_SEND (or savebox)
			tpMailItem->Attach = alloc_copy_t(tpReMailItem->Attach);
			tpMailItem->AttachSize = tpReMailItem->AttachSize;
		}
		if (ReplyFlag == EDIT_REDIRECT) {
			tpMailItem->Subject = alloc_copy_t(tpReMailItem->Subject);
			if (tpReMailItem->HasHeader == 0 || (q = GetBodyPointa(tpReMailItem->Body)) == NULL) {
				q = tpReMailItem->Body;
			}
			tpMailItem->Body = alloc_copy(q);
		} else if (op.FwdQuotation == 2 && tpMailItem->Mark != MARK_FWD_SELTEXT) {
			// forward as attachment
			tpMailItem->FwdAttach = alloc_copy_t(TEXT("|"));
			//if (tpMailItem->FwdAttach != NULL) {
			//	*tpMailItem->FwdAttach = ATTACH_SEP;
			//}
			tpMailItem->AttachSize = item_to_string_size(tpReMailItem, 2, TRUE, FALSE);
		} else if (tpReMailItem->Multipart != MULTIPART_NONE && tpReMailItem->Body != NULL) {
			// GJC copy attachments
			MULTIPART **tpMultiPart = NULL;
			char *fname;
			int i, j, cnt, len;
#ifdef UNICODE
			char *ContentType;

			ContentType = alloc_tchar_to_char(tpReMailItem->ContentType);
			cnt = multipart_parse(ContentType, tpReMailItem->Body, FALSE, &tpMultiPart, 0);
			mem_free(&ContentType);
#else
			cnt = multipart_parse(tpReMailItem->ContentType, tpReMailItem->Body, FALSE, &tpMultiPart, 0);
#endif
			len = 0;
			for (i = 0, j = 0; i < cnt; i++) {
				fname = (*(tpMultiPart + i))->Filename;
				if ((*(tpMultiPart + i))->Forwardable && fname != NULL && *fname != '\0') {
					len += tstrlen(fname) + 1;
					j++;
				}
			}
			if (len > 0) {
				p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
				tpMailItem->FwdAttach = p;
			} else {
				p = NULL;
			}
			if (p != NULL) {
				for (i = 0, j = 0; i < cnt; i++) {
					fname = (*(tpMultiPart + i))->Filename;
					if ((*(tpMultiPart + i))->Forwardable && fname != NULL && *fname != '\0') {
#ifdef UNICODE
						TCHAR *fn_t;
#endif
						int size;
						if (j > 0) {
							*(p++) = ATTACH_SEP;
						}
#ifdef UNICODE
						fn_t = alloc_char_to_tchar(fname);
						wsprintf(p, TEXT("%s"), fn_t);
						mem_free(&fn_t);
#else
						wsprintf(p, "%s", fname);
#endif
						p += tstrlen(fname);
						size = (*(tpMultiPart + i))->ePos - (*(tpMultiPart + i))->hPos;
						if (size > 0) tpMailItem->AttachSize += size;
						j++;
					}
				}
				*p = '\0';
			}
			multipart_free(&tpMultiPart, cnt);
		}

	} else {
		// Reply/ReplyAll Settings
		if (tpReMailItem->ReplyTo != NULL) {
			tpMailItem->To = alloc_copy_t(tpReMailItem->ReplyTo);
		} else if (tpReMailItem->From != NULL) {
			tpMailItem->To = alloc_copy_t(tpReMailItem->From);
		}

		//For ReplyAll, do Cc settings
		if(ReplyFlag == EDIT_REPLYALL) {
			SetAllReMessage(tpMailItem, tpReMailItem);
		}
		strPrefix = op.ReSubject;
	}

	///////////// MRP /////////////////////
	tpMailItem->Priority = tpReMailItem->Priority;
	///////////// --- /////////////////////

	if(tpReMailItem->MessageID != NULL && *tpReMailItem->MessageID == TEXT('<')) {
		//The setting
		if (is_fwd) {
			tpMailItem->References = alloc_copy_t(tpReMailItem->MessageID);
		} else {
			tpMailItem->InReplyTo = alloc_copy_t(tpReMailItem->MessageID);

			// of In-Reply-To of reply The setting
			if (tpReMailItem->InReplyTo != NULL && *tpReMailItem->InReplyTo == TEXT('<')) {
				tpMailItem->References = (TCHAR *)mem_alloc(
				sizeof(TCHAR) * (lstrlen(tpReMailItem->InReplyTo) + lstrlen(tpReMailItem->MessageID) + 2));
				if (tpMailItem->References != NULL) {
					str_join_t(tpMailItem->References, tpReMailItem->InReplyTo, TEXT(" "), tpReMailItem->MessageID, (TCHAR *)-1);
				}
			} else {
				tpMailItem->References = alloc_copy_t(tpReMailItem->MessageID);
			}
		}
	}

	//of References of reply Setting subject of reply
	if (ReplyFlag != EDIT_REDIRECT) {
		subject = (tpReMailItem->Subject != NULL) ? tpReMailItem->Subject : TEXT("");
		if (str_cmp_ni_t(subject, strPrefix, lstrlen(strPrefix)) == 0) {
			subject += lstrlen(strPrefix);
		} else if (str_cmp_ni_t(subject, REPLY_SUBJECT, lstrlen(REPLY_SUBJECT)) == 0) {
			subject += lstrlen(REPLY_SUBJECT);
		}
		for (; *subject == TEXT(' '); subject++);
		p = tpMailItem->Subject = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(subject) + lstrlen(strPrefix) + 1));
		if (tpMailItem->Subject != NULL) {
			str_join_t(p, strPrefix, subject, (TCHAR *)-1);
		}
	}
}

/*
 * SetReplyMessageBody - It sets the text of the reply mail
 */
static void SetReplyMessageBody(MAILITEM *tpMailItem, MAILITEM *tpReMailItem, int ReplyFlag, TCHAR *seltext)
{
	MULTIPART **tpMultiPart = NULL;
	TCHAR *p, *mBody, *body, *sig;
	TCHAR *quotchar = NULL;
	int len, cnt, i, TextIndex;
	int qlen = 0;
	BOOL is_fwd, fwd_as_att, do_sig, do_sig_above;
	BOOL has_sel = (seltext != NULL && (tpMailItem->Mark == MARK_REPL_SELTEXT || tpMailItem->Mark == MARK_FWD_SELTEXT));
	is_fwd = ReplyFlag == EDIT_FORWARD || ReplyFlag == EDIT_FILTERFORWARD;

	if (has_sel) {
		// overrides forward as attachment
		fwd_as_att = FALSE;
	} else if (op.FwdQuotation == 2 && is_fwd) {
		fwd_as_att = TRUE;
	} else {
		fwd_as_att = FALSE;
	}

	//Setting
	if ( fwd_as_att == FALSE && (has_sel || 
		(tpMailItem->Mark == 1 && (tpReMailItem != NULL && tpReMailItem->Body != NULL))) ) {
		if (has_sel) {
			mBody = alloc_copy_t(seltext);
		} else {
			mBody = MIME_body_decode(tpReMailItem, FALSE, TRUE, &tpMultiPart, &cnt, &TextIndex);

			if (op.StripHtmlTags == 1 &&
				((tpMailItem->ContentType != NULL && str_cmp_ni_t(tpMailItem->ContentType, TEXT("text/html"), lstrlen(TEXT("text/html")))==0)
				|| (TextIndex != -1 && (tpMultiPart[TextIndex])->ContentType != NULL &&
				str_cmp_ni((tpMultiPart[TextIndex])->ContentType, "text/html", tstrlen("text/html")) == 0))) {
				p = strip_html_tags(mBody, 0);
				if (p != NULL) {
					mem_free(&mBody);
					mBody = p;
				}
			}

			multipart_free(&tpMultiPart, cnt);
		}

		if (op.FwdQuotation != 0 || !is_fwd) {
			quotchar = op.QuotationChar;
			qlen = lstrlen(quotchar);
		}
		if (is_fwd) {
			len = CreateHeaderStringSize(op.FwdHeader, tpReMailItem, quotchar) + 2;
		} else {
			len = CreateHeaderStringSize(op.ReHeader, tpReMailItem, NULL) + 2;
		}
		if (op.QuotationBreak == FALSE) {
			TCHAR *qBody = (TCHAR *)mem_alloc(sizeof(TCHAR) * 
				(WordBreakStringSize(mBody, NULL, op.WordBreakSize - qlen, TRUE) + 1));
			if (qBody != NULL) {
				WordBreakString(mBody, qBody, NULL, op.WordBreakSize - qlen, TRUE);
				mem_free(&mBody);
				mBody = qBody;
			}
		}
		len += GetReplyBodySize(mBody, quotchar);

		i = mailbox_name_to_index(tpMailItem->MailBox);
		if (i!= -1) {
			sig = (MailBox + i)->Signature;
		} else {
			sig = NULL;
		}
		do_sig = (sig != NULL) && (*sig != TEXT('\0'))
			&& (!is_fwd	|| (op.SignForward == 1 || op.SignForward == 3));
		if (do_sig) {
			len += lstrlen(sig) + 2;
		}
		do_sig_above = do_sig &&
			((is_fwd && op.SignForward == 3) || (!is_fwd && op.SignReplyAbove == 1));

		body = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (body != NULL) {
			p = body;
			if (do_sig_above) {
				p = str_join_t(p, sig, TEXT("\r\n"), (TCHAR *)-1);
			}
			if (is_fwd) {
				p = CreateHeaderString(op.FwdHeader, p, tpReMailItem, quotchar);
			} else {
				p = CreateHeaderString(op.ReHeader, p, tpReMailItem, NULL);
			}
			p = str_cpy_t(p, TEXT("\r\n"));
			if (mBody != NULL) {
				p = SetReplyBody(mBody, p, quotchar);
			}
			if (do_sig && !do_sig_above) {
				str_join_t(p, TEXT("\r\n"), sig, (TCHAR *)-1);
			}
		}
		mem_free(&mBody);

		tpMailItem->BodyEncoding = op.BodyEncoding;
#ifndef _WCE_OLD
		if (tpMailItem->BodyCharset == NULL ||
			(tpMailItem->Body = MIME_charset_encode(charset_to_cp((BYTE)font_charset), body, tpMailItem->BodyCharset)) == NULL) {
#endif
#ifdef UNICODE
			tpMailItem->Body = alloc_tchar_to_char(body);
#else
			tpMailItem->Body = alloc_copy(body);
#endif
#ifndef _WCE_OLD
		}
#endif
		mem_free(&body);

	} else {
		if (is_fwd && (op.SignForward == 0 || op.SignForward == 2)) {
			return;
		}
		i = mailbox_name_to_index(tpMailItem->MailBox);
		if (i == -1 || (MailBox + i)->Signature == NULL || *(MailBox + i)->Signature == TEXT('\0')) {
			return;
		}
		len = lstrlen((MailBox + i)->Signature);
		body = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 3));
		if (body != NULL) {
			str_join_t(body, TEXT("\r\n"), (MailBox + i)->Signature, (TCHAR *)-1);
		}
		tpMailItem->BodyEncoding = op.BodyEncoding;
#ifndef _WCE_OLD
		if (tpMailItem->BodyCharset == NULL ||
			(tpMailItem->Body = MIME_charset_encode(charset_to_cp((BYTE)font_charset), body, tpMailItem->BodyCharset)) == NULL) {
#endif
#ifdef UNICODE
			tpMailItem->Body = alloc_tchar_to_char(body);
#else
			tpMailItem->Body = alloc_copy(body);
#endif
#ifndef _WCE_OLD
		}
#endif
		mem_free(&body);
	}
}

/*
 * SetWindowString - 
 */
static void SetWindowString(HWND hWnd, TCHAR *Subject, BOOL editable)
{
	TCHAR *buf;

	if (editable == TRUE) {
		if (Subject == NULL) {
			SetWindowText(hWnd, STR_TITLE_MAILEDIT);
			return;
		}
		buf = (TCHAR *)mem_alloc(sizeof(TCHAR) *
			(lstrlen(STR_TITLE_MAILEDIT) + lstrlen(Subject) + lstrlen(TEXT(" - []")) + 1));
		if (buf == NULL) {
			SetWindowText(hWnd, STR_TITLE_MAILEDIT);
			return;
		}
		str_join_t(buf, STR_TITLE_MAILEDIT TEXT(" - ["), Subject, TEXT("]"), (TCHAR *)-1);
		if (lstrlen(buf) > BUF_SIZE) {
			*(buf + BUF_SIZE) = TEXT('\0');
		}
	} else {
		if (Subject == NULL) {
			SetWindowText(hWnd, STR_TITLE_MAILSENT);
			return;
		}
		buf = (TCHAR *)mem_alloc(sizeof(TCHAR) *
			(lstrlen(STR_TITLE_MAILSENT) + lstrlen(Subject) + lstrlen(TEXT(" - []")) + 1));
		if (buf == NULL) {
			SetWindowText(hWnd, STR_TITLE_MAILSENT);
			return;
		}
		str_join_t(buf, STR_TITLE_MAILSENT TEXT(" - ["), Subject, TEXT("]"), (TCHAR *)-1);
		if (lstrlen(buf) > BUF_SIZE) {
			*(buf + BUF_SIZE) = TEXT('\0');
		}
	}
	SetWindowText(hWnd, buf);
	mem_free(&buf);
}

/*
 * SetHeaderString - of window title Mail header indication
 */
static void SetHeaderString(HWND hHeader, MAILITEM *tpMailItem)
{
	TCHAR *buf, *p;
	int len = 0;

	len += lstrlen(STR_EDIT_HEAD_MAILBOX);
	if (tpMailItem->MailBox != NULL) {
		len += lstrlen(tpMailItem->MailBox);
	}
	len += lstrlen(STR_EDIT_HEAD_TO);
	if (tpMailItem->RedirectTo != NULL) {
		len += lstrlen(tpMailItem->RedirectTo);
	} else if (tpMailItem->To != NULL) {
		len += lstrlen(tpMailItem->To);
	}
	len += SetCcAddressSize(tpMailItem->Cc);
	len += SetCcAddressSize(tpMailItem->Bcc);
	len += lstrlen(STR_EDIT_HEAD_SUBJECT);
	if (tpMailItem->Subject != NULL) {
		len += lstrlen(tpMailItem->Subject);
	}
	len += SetAttachListSize(tpMailItem->Attach);
	buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (buf == NULL) {
		return;
	}

	p = str_join_t(buf, STR_EDIT_HEAD_MAILBOX, tpMailItem->MailBox, STR_EDIT_HEAD_TO, 
		(tpMailItem->RedirectTo != NULL) ? tpMailItem->RedirectTo : tpMailItem->To, (TCHAR *)-1);
	p = SetCcAddress(TEXT("Cc"), tpMailItem->Cc, p);
	p = SetCcAddress(TEXT("Bcc"), tpMailItem->Bcc, p);
	p = str_join_t(p, STR_EDIT_HEAD_SUBJECT, tpMailItem->Subject, (TCHAR *)-1);
	SetAttachList(tpMailItem->Attach, p);

	SetWindowText(hHeader, buf);
	mem_free(&buf);
}

/*
 * SetBodyContents - put message body into text window
 */
static void SetBodyContents(HWND hWnd, MAILITEM *tpMailItem)
{
	TCHAR *buf, *tmp;

	if (tpMailItem->Body != NULL) {
		SwitchCursor(FALSE);
#ifndef _WCE_OLD
		if (tpMailItem->BodyCharset == NULL ||
			(tmp = MIME_charset_decode(charset_to_cp((BYTE)font_charset), tpMailItem->Body, tpMailItem->BodyCharset)) == NULL) {
#endif
#ifdef UNICODE
			tmp = alloc_char_to_tchar(tpMailItem->Body);
#else
			tmp = alloc_copy(tpMailItem->Body);
#endif
#ifndef _WCE_OLD
		}
#endif
		if (tmp != NULL) {
			buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(tmp) + 1));
			if (buf != NULL) {
				DelDot(tmp, buf);
				if (EditMaxLength != 0 && (int)lstrlen(buf) > EditMaxLength) {
					*(buf + EditMaxLength) = TEXT('\0');
				}
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETTEXT, 0, (LPARAM)buf);
				mem_free(&buf);
			}
		}
		mem_free(&tmp);
		SwitchCursor(TRUE);
	}
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETMODIFY, (WPARAM)FALSE, 0);
}

/*
 * SubClassSentProc - event handler for sent mail window
 */
static LRESULT CALLBACK SubClassSentProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WPARAM keycode;
	switch (msg) {
	case WM_CHAR:
		if ((TCHAR)wParam == TEXT(' ')) {
			if (GetKeyState(VK_SHIFT) < 0) {
				SendMessage(hWnd, WM_VSCROLL, SB_PAGEUP, 0);
			} else {
				SendMessage(hWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
			}
		} else if (wParam == 0x03) { // 0x03 is ascii for ctrl-C
			SendMessage(hWnd, WM_COPY, 0, 0);
		}
		// and fall through for the return
	case WM_DEADCHAR:
	case WM_CUT:
	case WM_CLEAR:
	case WM_PASTE:
	case EM_UNDO:
		return 0;
	case WM_KEYDOWN:
		keycode = LOWORD(wParam);
		if (keycode == VK_DELETE) {
			return 0;
		} else if (keycode == VK_RETURN || keycode == VK_ESCAPE) {
			SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
		}
		break;
	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam,lParam)) {
		case ID_MENUITEM_FIND:
			View_FindMail(hWnd, TRUE);
			return 0;

		case ID_MENUITEM_NEXTFIND:
			View_FindMail(hWnd, FALSE);
			return 0;

		case ID_MENUITEM_REPLACE:
		case ID_MENUITEM_PASTEQUOT:
		case ID_MENUITEM_REFLOW:
			return 0;
		}
	}
#ifdef _WIN32_WCE
	return CallWindowProc(EditWindowProcedure, hWnd, msg, wParam, lParam);
#else
	return CallWindowProc((WNDPROC)GetProp(hWnd, WNDPROC_KEY), hWnd, msg, wParam, lParam);
#endif
}

/*
 * SetSentSubClass - Subclass (callback) modification for sent mail
 */
static void SetSentSubClass(HWND hWnd)
{
#ifdef _WIN32_WCE
	EditWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)SubClassSentProc);
#else
	WNDPROC OldWndProc = NULL;

	OldWndProc = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)SubClassSentProc);
	SetProp(hWnd, WNDPROC_KEY, OldWndProc);
#endif
}

/*
 * DelSentSubClass - Reset window callback to standard
 */
static void DelSentSubClass(HWND hWnd)
{
#ifdef _WIN32_WCE
	SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)EditWindowProcedure);
	EditWindowProcedure = NULL;
#else
	WNDPROC OldWndProc = (WNDPROC)GetProp(hWnd, WNDPROC_KEY);
	if (OldWndProc) {
		SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)OldWndProc);
	}
	RemoveProp(hWnd, WNDPROC_KEY);
#endif
}

/*
 * SubClassEditProc - To subclass is converted the window procedure
 */
#ifdef _WIN32_WCE_PPC
static LRESULT CALLBACK SubClassEditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_LBUTTONDOWN:
		{
			SHRGINFO rg;

			rg.cbSize = sizeof(SHRGINFO);
			rg.hwndClient = hWnd;
			rg.ptDown.x = LOWORD(lParam);
			rg.ptDown.y = HIWORD(lParam);
			rg.dwFlags = SHRG_RETURNCMD;

			if (SHRecognizeGesture(&rg) == GN_CONTEXTMENU) {
				SendMessage(GetParent(hWnd), WM_COMMAND, ID_MENU, 0);
				return 0;
			}
		}
		break;
	}

	return CallWindowProc(EditWindowProcedure, hWnd, msg, wParam, lParam);
}
#endif

/*
 * SetEditSubClass - which Subclass conversion
 */
#ifdef _WIN32_WCE_PPC
static void SetEditSubClass(HWND hWnd)
{
	EditWindowProcedure = (WNDPROC)SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)SubClassEditProc);
}
#endif

/*
 * DelEditSubClass - of window Window class is reset to those of standard
 */
#ifdef _WIN32_WCE_PPC
static void DelEditSubClass(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)EditWindowProcedure);
	EditWindowProcedure = NULL;
}
#endif

/*
 * TbNotifyProc - Notification message of tool bar (Win32)
 */
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

/*
 * NotifyProc - Notification message
 */
static LRESULT NotifyProc(HWND hWnd, LPARAM lParam)
{
	NMHDR *CForm = (NMHDR *)lParam;

#ifndef _WIN32_WCE
	if (CForm->code == TTN_NEEDTEXT) {
		return TbNotifyProc(hWnd, lParam);
	}
#endif
	return FALSE;
}

/*
 * InitWindow - Initialization
 */
static BOOL InitWindow(HWND hWnd, MAILITEM *tpMailItem)
{
	HDC hdc;
	HFONT hFont;
	TEXTMETRIC lptm;
	RECT rcClient, StRect;
	int Height, FontHeight;

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
		ID_MENUITEM_CLOSE,		CSOBAR_COMMON_BUTTON,		CSO_BUTTON_DISP, (-1),					NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), CSO_ID_BACK,	CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		1,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, STR_MENU_FILE,	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		2,						CSOBAR_BUTTON_SUBMENU_DOWN,	CSO_BUTTON_DISP, (-1),					NULL, STR_MENU_EDIT,	NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, CLR_INVALID,				FALSE, FALSE,
		ID_MENUITEM_SEND,		CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_SEND,		NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_SBOXMARK,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_SENDBOX,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_SENDBOX,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_SENDBOX,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
		ID_MENUITEM_SENDINFO,	CSOBAR_BUTTON_NORM,			CSO_BUTTON_DISP, IDB_BITMAP_SENDINFO,	NULL, NULL,				NULL, CSOBAR_CODEPOS_CENTER, 1, (-1), (-1), (-1), (-1), 0,				CLR_INVALID, CLR_INVALID, RGB(0xC0, 0xC0, 0xC0),	FALSE, FALSE,
	};
	HWND hCSOBar;
	DWORD style;
#else	// _WIN32_WCE_LAGENDA
	TBBUTTON tbButton[] = {
#ifdef _WIN32_WCE
#ifndef _WIN32_WCE_PPC
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
#endif	// _WIN32_WCE_PPC
#endif	// _WIN32_WCE
		{0,	ID_MENUITEM_SEND,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{1,	ID_MENUITEM_SBOXMARK,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{2,	ID_MENUITEM_SENDBOX,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{3,	ID_MENUITEM_PREVMAIL,	TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{4,	ID_MENUITEM_NEXTMAIL,	TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{5,	ID_MENUITEM_SAVECOPY,	TBSTATE_HIDDEN,		TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{6,	ID_MENUITEM_SENDINFO,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{0,	0,						TBSTATE_ENABLED,	TBSTYLE_SEP,	0, 0, 0, -1},
		{7,	ID_MENUITEM_FIND,		TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1},
		{8,	ID_MENUITEM_NEXTFIND,	TBSTATE_ENABLED,	TBSTYLE_BUTTON,	0, 0, 0, -1}
	};
#ifdef _WIN32_WCE
	static TCHAR *szTips[] = {
#ifdef _WIN32_WCE_PPC
		NULL, // menu skipping
#endif	// _WIN32_WCE_PPC
		NULL, // menu skipping
		STR_CMDBAR_SEND,
		STR_CMDBAR_SBOXMARK,
		STR_CMDBAR_SENDBOX,
		STR_CMDBAR_PREVMAIL,
		STR_CMDBAR_NEXTMAIL,
		STR_CMDBAR_SAVECOPY,
		STR_CMDBAR_SENDINFO,
		STR_CMDBAR_FIND,
		STR_CMDBAR_NEXTFIND,
		NULL // extra for CE.net
	};
#ifdef _WIN32_WCE_PPC
	SHMENUBARINFO mbi;
#else	// _WIN32_WCE_PPC
	WORD idMenu;
#endif	// _WIN32_WCE_PPC
#else	// _WIN32_WCE
	RECT ToolbarRect;
#endif	// _WIN32_WCE
#endif	// _WIN32_WCE_LAGENDA

	if (tpMailItem == NULL) {
		return FALSE;
	}
	SetWindowString(hWnd, tpMailItem->Subject,
		(tpMailItem->MailStatus == ICON_SENTMAIL) ? FALSE : TRUE);

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	// PocketPC
	memset (&mbi, 0, sizeof (SHMENUBARINFO));
	mbi.cbSize     = sizeof (SHMENUBARINFO);
	mbi.hwndParent = hWnd;
	mbi.nToolBarId = IDM_MAIN_EDIT;
	mbi.hInstRes   = hInst;
	mbi.nBmpId     = 0;
	mbi.cBmpImages = 0;
	SHCreateMenuBar(&mbi);

	hEditToolBar = mbi.hwndMB;
    CommandBar_AddToolTips(hEditToolBar, 11, szTips);
	CommandBar_AddBitmap(hEditToolBar, hInst, IDB_TOOLBAR_EDIT, 9, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hEditToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	Height = 0;

#ifdef _WIN32_WCE_SP
	// code courtesy of Christian Ghisler
	if (op.osMajorVer >= 5) {
		// WM5 is 5.1, WM6 is 5.2
		SendMessage(hEditToolBar, SHCMBM_OVERRIDEKEY, VK_F1, 
			MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
		SendMessage(hEditToolBar, SHCMBM_OVERRIDEKEY, VK_F2, 
			MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY, SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
	}
#endif

#elif defined(_WIN32_WCE_LAGENDA)
	// BE-500
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
	// H/PC & PsPC
	hEditToolBar = CommandBar_Create(hInst, hWnd, IDC_VCB);
	// op.osMajorVer >= 4 is CE.net 4.2 and higher (MobilePro 900c)
	// else HPC2000 (Jornada 690, 720)
	CommandBar_AddToolTips(hEditToolBar, 11, ((op.osMajorVer >= 4) ? (szTips+1) : szTips));
	idMenu = (GetSystemMetrics(SM_CXSCREEN) >= 450) ? (WORD)IDR_MENU_EDIT_HPC : (WORD)IDR_MENU_EDIT;
	CommandBar_InsertMenubar(hEditToolBar, hInst, idMenu, 0);
	CommandBar_AddBitmap(hEditToolBar, hInst, IDB_TOOLBAR_EDIT, 9, TB_ICONSIZE, TB_ICONSIZE);
	CommandBar_AddButtons(hEditToolBar, sizeof(tbButton) / sizeof(TBBUTTON), tbButton);
	CommandBar_AddAdornments(hEditToolBar, 0, 0);

	Height = CommandBar_Height(hEditToolBar);
#endif
#else
	// Win32
	hEditToolBar = CreateToolbarEx(hWnd, WS_CHILD | TBSTYLE_TOOLTIPS, IDC_VTB, 9, hInst, IDB_TOOLBAR_EDIT,
		tbButton, sizeof(tbButton) / sizeof(TBBUTTON), 0, 0, TB_ICONSIZE, TB_ICONSIZE, sizeof(TBBUTTON));
	SetWindowLong(hEditToolBar, GWL_STYLE, GetWindowLong(hEditToolBar, GWL_STYLE) | TBSTYLE_FLAT);
	SendMessage(hEditToolBar, TB_SETINDENT, 5, 0);
	ShowWindow(hEditToolBar,SW_SHOW);

	GetWindowRect(hEditToolBar, &ToolbarRect);
	Height = ToolbarRect.bottom - ToolbarRect.top;
#endif
	GetClientRect(hWnd, &rcClient);

	//of text Indicates the header compilation
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

	//of the STATIC control which Setting
#ifdef _WIN32_WCE
	if (hListFont != NULL) {
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
	//of font Acquisition height of font
	GetTextMetrics(hdc, &lptm);
	if (hFont != NULL) {
		SelectObject(hdc, hFont);
	}
	ReleaseDC(GetDlgItem(hWnd, IDC_HEADER), hdc);
	FontHeight = (lptm.tmHeight + lptm.tmExternalLeading) * 3;

	//Setting temporarily, it re-calculates size
	MoveWindow(GetDlgItem(hWnd, IDC_HEADER), 0, Height, rcClient.right, FontHeight, TRUE);
	GetClientRect(GetDlgItem(hWnd, IDC_HEADER), &StRect);
	FontHeight = FontHeight + (FontHeight - StRect.bottom) + 1;
	MoveWindow(GetDlgItem(hWnd, IDC_HEADER), 0, Height, rcClient.right, FontHeight, TRUE);

	//Indicates the text compilation
	Height += FontHeight + 1;
#ifndef USE_NEDIT
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
		WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL |
		ES_NOHIDESEL | ((op.EditWordBreakFlag == 1) ? 0 : WS_HSCROLL),
		0, Height, rcClient.right, rcClient.bottom - Height,
		hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
#else
	CreateWindowEx(
		WS_EX_CLIENTEDGE,
		NEDIT_WND_CLASS, TEXT(""),
		WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | ((op.EditWordBreakFlag == 1) ? 0 : WS_HSCROLL),
		0, Height, rcClient.right, rcClient.bottom - Height,
		hWnd, (HMENU)IDC_EDIT_BODY, hInst, NULL);
#endif
	if (hViewFont != NULL) {
		SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
	}
	SetFocus(GetDlgItem(hWnd, IDC_EDIT_BODY));

#ifdef _WIN32_WCE_PPC
	SetWordBreakMenu(hWnd, SHGetSubMenu(hEditToolBar, ID_MENUITEM_EDIT), (op.EditWordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#elif defined(_WIN32_WCE_LAGENDA)
	SetWordBreakMenu(hWnd, hViewMenu, (op.EditWordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#else
	SetWordBreakMenu(hWnd, NULL, (op.EditWordBreakFlag == 1) ? MF_CHECKED : MF_UNCHECKED);
#endif

#ifdef _WIN32_WCE
	SendMessage(hWnd, WM_SETICON, (WPARAM)FALSE,
		(LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON_SENTMAIL), IMAGE_ICON, 16, 16, 0));
	EditMaxLength = EDITMAXSIZE;
#else
	EditMaxLength = (op.osPlatformId == VER_PLATFORM_WIN32_NT) ? 0 : EDITMAXSIZE;
#endif
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_LIMITTEXT, (WPARAM)EditMaxLength, 0);

	SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
	SetBodyContents(hWnd, tpMailItem);
	tpMailItem->BodyEncoding = op.BodyEncoding;

	tpMailItem->hEditWnd = hWnd;
	if (tpMailItem->MailStatus == ICON_SENTMAIL) {
		// GJC don't edit sent mail
		SetSentSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE_PPC
	} else {
		SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#endif
	}
	return TRUE;
}

/*
 * SetWindowSize - of window Size modification
 */
static BOOL SetWindowSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32_WCE
	RECT rcClient, HeaderRect;
	int Height = 0;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(GetDlgItem(hWnd, IDC_HEADER), &HeaderRect);

#ifndef _WIN32_WCE_PPC
	Height = CommandBar_Height(GetDlgItem(hWnd, IDC_VCB));
#endif
	MoveWindow(GetDlgItem(hWnd, IDC_HEADER), 0, Height, rcClient.right, HeaderRect.bottom - HeaderRect.top, TRUE);
	InvalidateRect(GetDlgItem(hWnd, IDC_HEADER), NULL, FALSE);
	UpdateWindow(GetDlgItem(hWnd, IDC_HEADER));

	Height += HeaderRect.bottom - HeaderRect.top;
	MoveWindow(GetDlgItem(hWnd, IDC_EDIT_BODY), 0, Height + 1,
		rcClient.right, rcClient.bottom - Height, TRUE);
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
	if (SipInfo.fdwFlags & SIPF_ON) {
		sip_height = ((SipInfo.rcSipRect).bottom - (SipInfo.rcSipRect).top);
		ret = TRUE;
	} else {
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

/*
 * EndEditWindow
 */
BOOL EndEditWindow(HWND hWnd, BOOL sent)
{
	MAILITEM *tpMailItem;

	tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if (tpMailItem != NULL) {
		sent = (tpMailItem->MailStatus == ICON_SENTMAIL);
		if (sent == FALSE && SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETMODIFY, 0, 0) == TRUE) {
#ifdef _WIN32_WCE
			FocusWnd = hEditWnd;
			ShowWindow(hEditWnd, SW_SHOW);
			_SetForegroundWindow(hEditWnd);
#endif
			if (MessageBox(hWnd, STR_Q_EDITCANCEL, STR_TITLE_MAILEDIT,
				MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDNO) {
				return FALSE;
			}
			FocusWnd = MainWnd;
		}
		//if (tpMailItem->InReplyTo != NULL || tpMailItem->References != NULL) {
			// GJC remove re/fwd overlay?
		//}
		tpMailItem->hEditWnd = NULL;
		if (tpMailItem->hProcess != NULL) {
			ReadEditMail(hWnd, (long)hWnd, tpMailItem, FALSE);
			tpMailItem->hProcess = NULL;
		}
		if (item_is_mailbox(MailBox + MAILBOX_SEND, tpMailItem) == -1) {
			item_free(&tpMailItem, 1);
		}
	}

	if (sent) {
		DelSentSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
	}
#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	if (!sent) {
		DelEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
	}
    DestroyWindow(hEditToolBar);
#elif defined _WIN32_WCE_LAGENDA
	DestroyMenu(hViewMenu);
#else
	CommandBar_Destroy(GetDlgItem(hWnd, IDC_VCB));
#endif
	DestroyIcon((HICON)SendMessage(hWnd, WM_GETICON, FALSE, 0));
#else
	if (IsWindowVisible(hWnd) != 0 && IsIconic(hWnd) == 0 && IsZoomed(hWnd) == 0) {
		GetWindowRect(hWnd, (LPRECT)&op.EditRect);
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

/*
 * SetEditMenu - window Activity of compilation menu/change
 */
static void SetEditMenu(HWND hWnd)
{
	HMENU hMenu;
	MAILITEM *tpMailItem;
	BOOL editable;
	int i, j;

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hEditToolBar, ID_MENUITEM_EDIT);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 1);
#else
	hMenu = CommandBar_GetMenu(GetDlgItem(hWnd, IDC_VCB), 0);
#endif
#else
	hMenu = GetMenu(hWnd);
#endif
	tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if (tpMailItem == NULL || tpMailItem->MailStatus != ICON_SENTMAIL) {
		// GJC don't edit sent mail
		editable = TRUE;
	} else {
		editable = FALSE;
		DeleteMenu(hMenu, ID_MENUITEM_REPLACE, MF_BYCOMMAND);
	}

	//of the EDIT control which Acquisition
	SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&i, (LPARAM)&j);
	EnableMenuItem(hMenu, ID_MENUITEM_CUT, (editable && (i < j)) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_MENUITEM_COPY, (i < j) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_MENUITEM_PASTEQUOT, (editable) ? MF_ENABLED : MF_GRAYED);
	EnableMenuItem(hMenu, ID_MENUITEM_REFLOW, (editable && (i < j)) ? MF_ENABLED : MF_GRAYED);
}

/*
 * SetItemToSendBox - Retention to transmission box
 */
static BOOL SetItemToSendBox(HWND hWnd, MAILITEM *tpMailItem, BOOL BodyFlag, int EndFlag, BOOL MarkFlag)
{
	TCHAR *buf, *tmp;
	TCHAR numbuf[10];
#ifdef _WIN32_WCE
	unsigned int len;
#else
	int len;
#endif
	int i;
	long size;
	BOOL mkdlg;
	HWND hListView;

	if (tpMailItem == NULL && hWnd != NULL) {
		tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
		if (tpMailItem == NULL) {
			return FALSE;
		}
	}

	if (BodyFlag == FALSE) {
		//of selected position of editing box Check
		if (EndFlag == 0 && CheckDependence(hWnd, IDC_EDIT_BODY, &tmp) == FALSE) {
			mem_free(&tmp);
			return FALSE;
		}
		mem_free(&tmp);

		//of type dependence letter When subject is not set, information of transmission is indicated
		if (EndFlag == 0 && gSendAndQuit == FALSE 
			&& (tpMailItem->Subject == NULL || *tpMailItem->Subject == TEXT('\0'))) {
#ifdef _WIN32_WCE
			int res = IDD_DIALOG_SETSEND;
			if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
				res = IDD_DIALOG_SETSEND_WIDE;
#ifdef _WIN32_WCE_PPC
			} else if (GetSystemMetrics(SM_CYSCREEN) <= 260) {
				res = IDD_DIALOG_SETSEND_SHORT;
#endif
			}
			mkdlg = DialogBoxParam(hInst, MAKEINTRESOURCE(res), hWnd, SetSendProc, (LPARAM)tpMailItem);
#else
			mkdlg = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
				(LPARAM)tpMailItem);
#endif
			if (mkdlg == FALSE) {
				return FALSE;
			}
		}

		//Setting text
		SwitchCursor(FALSE);
		mem_free(&tpMailItem->Body);
		len = SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXTLENGTH, 0, 0) + 1;
		buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (len > 0 && buf != NULL) {
			*buf = TEXT('\0');
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXT, len, (LPARAM)buf);

			//Automatic operation lapel
			tmp = (TCHAR *)mem_alloc(
				sizeof(TCHAR) * (WordBreakStringSize(buf, op.QuotationChar, op.WordBreakSize, op.QuotationBreak) + 1));
			if (tmp != NULL) {
				WordBreakString(buf, tmp, op.QuotationChar, op.WordBreakSize, op.QuotationBreak);
				mem_free(&buf);
				buf = tmp;
			}

			//Line head... conversion
			len = SetDotSize(buf);
			tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
			if (tmp != NULL) {
				SetDot(buf, tmp);
			}
			mem_free(&buf);
			tpMailItem->BodyEncoding = op.BodyEncoding;
#ifndef _WCE_OLD
			if (tpMailItem->BodyCharset == NULL ||
				(tpMailItem->Body = MIME_charset_encode(charset_to_cp((BYTE)font_charset), tmp, tpMailItem->BodyCharset)) == NULL) {
#endif
#ifdef UNICODE
				tpMailItem->Body = alloc_tchar_to_char(tmp);
#else
				tpMailItem->Body = alloc_copy(tmp);
#endif
#ifndef _WCE_OLD
			}
#endif
			mem_free(&tmp);
		} else if (len == 0) {
			*buf = TEXT('\0');
#ifdef UNICODE
			tpMailItem->Body = alloc_tchar_to_char(buf);
			mem_free(&buf);
#else
			tpMailItem->Body = buf;
#endif
		}
		SwitchCursor(TRUE);
	}

	//Setting size
	mem_free(&tpMailItem->Size);
	size = (tpMailItem->Body != NULL) ? (tstrlen(tpMailItem->Body)) : 0;
	if (size < 0) size = 0;
	size += tpMailItem->AttachSize;
	wsprintf(numbuf, TEXT("%d"), size);
	tpMailItem->Size = alloc_copy_t(numbuf);
	hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);

	if (MarkFlag == TRUE) {
		tpMailItem->Mark = ICON_SEND;
		SetStarMBMenu(TRUE);
	} else {
		if (tpMailItem->Mark != ICON_FLAG) {
			tpMailItem->Mark = ICON_NON;
		}
		if (GetStarMBMenu() && item_get_next_send_mark((MailBox + MAILBOX_SEND), TRUE) == -1) {
			SetStarMBMenu(FALSE);
		}

	}
	if (item_is_mailbox(MailBox + MAILBOX_SEND, tpMailItem) == -1) {

		if (item_add(MailBox + MAILBOX_SEND, tpMailItem) == FALSE) {
			return FALSE;
		}
		if (EndFlag == 0 && SelBox == MAILBOX_SEND) {
			ListView_InsertItemEx(hListView, (TCHAR *)LPSTR_TEXTCALLBACK, 0, 
				I_IMAGECALLBACK, (long)tpMailItem, ListView_GetItemCount(hListView));
		}
	}
	SetItemCntStatusText(NULL, FALSE);
	if (EndFlag == 0) {
		if (op.SelectSendBox == 1 && SelBox != MAILBOX_SEND) {
			mailbox_select(MainWnd, MAILBOX_SEND);
		}
	}
	if (EndFlag == 0 || EndFlag == EDIT_SEND) {
		if (SelBox == MAILBOX_SEND) {
			i = ListView_GetMemToItem(hListView, tpMailItem);
			if (i != -1) {
				int state = ListView_ComputeState(tpMailItem->Priority, tpMailItem->Multipart);
				ListView_SetItemState(hListView, i, INDEXTOSTATEIMAGEMASK(state), LVIS_STATEIMAGEMASK);
				ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
				ListView_SetItemState(hListView, i,
					LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);

				ListView_EnsureVisible(hListView, i, TRUE);
				ListView_RedrawItems(hListView, i, i);
				UpdateWindow(hListView);
#ifdef _WIN32_WCE
				if (EndFlag == 0) {
					FocusWnd = MainWnd;
					ShowWindow(MainWnd, SW_SHOW);
					_SetForegroundWindow(MainWnd);
				}
#endif
			}
		}
	}
	return TRUE;
}

/*
 * CloseEditMail - The compilation mail is closed
 */
static BOOL CloseEditMail(HWND hWnd, BOOL SendFlag, BOOL ShowFlag)
{
	MAILITEM *tpMailItem;
	BOOL sent;

	tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if (tpMailItem == NULL) {
		return FALSE;
	}

	tpMailItem->hEditWnd = NULL;
	tpMailItem->hProcess = NULL;
	// GJC don't edit sent mail
	sent = (tpMailItem->MailStatus == ICON_SENTMAIL) ? TRUE : FALSE;
	if (sent == FALSE) {
		(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
	}

	if (op.AutoSave != 0 && sent == FALSE) {
		//Transmission box retention to file
		file_save_mailbox(SENDBOX_FILE, DataDir, MAILBOX_SEND, FALSE, TRUE, 2);
	}

	SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)0);
	EndEditWindow(hWnd, sent);
#ifdef _WIN32_WCE
	FocusWnd = MainWnd;
#ifdef _WIN32_WCE_LAGENDA
	ShowWindow(MainWnd, SW_SHOW);
	if (ShowFlag == TRUE) {
		_SetForegroundWindow(MainWnd);
	}
#else
	if (ShowFlag == TRUE) {
		ShowWindow(MainWnd, SW_SHOW);
		_SetForegroundWindow(MainWnd);
	}
#endif
#endif

	if (SendFlag == TRUE) {
		SendMessage(MainWnd, WM_SMTP_SENDMAIL, 0, (LPARAM)tpMailItem);
	}
	return TRUE;
}

/*
 * UpdateEditWindow - change info for prev/next
 */
static void UpdateEditWindow(HWND hWnd, MAILITEM *tpNewMailItem) {
	MAILITEM *tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);

#ifndef _WIN32_WCE
	if (tpNewMailItem->hEditWnd != NULL) {
		CloseEditMail(hWnd, FALSE, FALSE);
		if (IsWindowVisible(tpNewMailItem->hEditWnd) == TRUE) {
			ShowWindow(tpNewMailItem->hEditWnd, SW_SHOW);
			if (IsIconic(tpNewMailItem->hEditWnd) != 0) {
				ShowWindow(tpNewMailItem->hEditWnd, SW_RESTORE);
			}
			_SetForegroundWindow(tpNewMailItem->hEditWnd);
		}
	} else
#endif
	if (tpNewMailItem->MailStatus != ICON_SENTMAIL) {
		CloseEditMail(hWnd, FALSE, FALSE);
		OpenItem(MainWnd, FALSE, FALSE);
	} else {
		if (tpMailItem != NULL) {
			tpMailItem->hEditWnd = NULL;
		}
		tpNewMailItem->hEditWnd = hWnd;
		SetWindowLong(hWnd, GWL_USERDATA, (long)tpNewMailItem);
		SetWindowString(hWnd, tpNewMailItem->Subject,
			(tpNewMailItem->MailStatus == ICON_SENTMAIL) ? FALSE : TRUE);
		SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpNewMailItem);
		SetBodyContents(hWnd, tpNewMailItem);
	}
}

/*
 * ShowSendInfo - Transmission information display
 */
static void ShowSendInfo(HWND hWnd)
{
	MAILITEM *tpMailItem;
	int res = IDD_DIALOG_SETSEND;

	tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
	if (tpMailItem == NULL) {
		return;
	}
#ifdef _WIN32_WCE
	if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
		res = IDD_DIALOG_SETSEND_WIDE;
#ifdef _WIN32_WCE_PPC
	} else if (GetSystemMetrics(SM_CYSCREEN) <= 260) {
		res = IDD_DIALOG_SETSEND_SHORT;
#endif
	}
#endif
	if (tpMailItem->RedirectTo != NULL) {
		res = IDD_DIALOG_REDIRECT;
	}
	if (DialogBoxParam(hInst, MAKEINTRESOURCE(res), hWnd, SetSendProc, (LPARAM)tpMailItem)) {
		(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
		SetWindowString(hWnd, tpMailItem->Subject,
			(tpMailItem->MailStatus == ICON_SENTMAIL) ? FALSE : TRUE);
		SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
	}
}

/*
 * AppEditMail - With external editor compilation
 */
static BOOL AppEditMail(HWND hWnd, long id, char *buf, MAILITEM *tpMailItem)
{
	HANDLE hFile;
	TCHAR path[BUF_SIZE];
	TCHAR *p;

#ifdef _WIN32_WCE
	str_join_t(path, DataDir, EDIT_FILE, TEXT("."), op.EditFileSuffix, (TCHAR *)-1);
#else
	wsprintf(path, TEXT("%s%ld.%s"), DataDir, id, op.EditFileSuffix);
#endif

	//The file which it retains is opened
	hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL || hFile == (HANDLE)-1) {
		return FALSE;
	}
	//Retention
	if (buf != NULL && file_write(hFile, buf, tstrlen(buf)) == FALSE) {
		CloseHandle(hFile);
		DeleteFile(path);
		return FALSE;
	}
	CloseHandle(hFile);

	{
#ifdef _WCE_OLD
		PROCESS_INFORMATION ProcInfo;
		TCHAR param[BUF_SIZE];

		str_join_t(param, TEXT(" "), path, (TCHAR *)-1);
		p = CreateCommandLine(op.EditAppCmdLine, path, TRUE);

		//of mail Starting
		if (CreateProcess(op.EditApp, ((p != NULL) ? p : param),
			NULL, NULL, FALSE, 0, NULL, NULL, NULL, &ProcInfo) == FALSE) {
			mem_free(&p);
			DeleteFile(path);
			return FALSE;
		}
		mem_free(&p);
		tpMailItem->hProcess = ProcInfo.hProcess;
#else
		SHELLEXECUTEINFO sei;

		ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
		sei.hwnd = NULL;
		sei.lpVerb = NULL;
		p = NULL;
		if (op.EditApp == NULL || *op.EditApp == TEXT('\0')) {
			sei.lpFile = path;
			sei.lpParameters = NULL;
		} else {
			sei.lpFile = op.EditApp;
			p = CreateCommandLine(op.EditAppCmdLine, path, FALSE);
			sei.lpParameters = (p != NULL) ? p : path;
		}
		sei.lpDirectory = NULL;
		sei.nShow = SW_SHOWNORMAL;
		sei.hInstApp = hInst;
		//Conversion to starting
		if (ShellExecuteEx(&sei) == FALSE) {
			mem_free(&p);
			DeleteFile(path);
			return FALSE;
		}
		mem_free(&p);
		tpMailItem->hProcess = sei.hProcess;
#endif
	}
	if (tpMailItem->hProcess == NULL) {
		DeleteFile(path);
		return FALSE;
	}
	return TRUE;
}

/*
 * ReadEditMail - It reads the file as the mail text
 */
static BOOL ReadEditMail(HWND hWnd, long id, MAILITEM *tpMailItem, BOOL ReadFlag)
{
	TCHAR path[BUF_SIZE];
	TCHAR *tmp, *p;
	char *fbuf;
	long len;

#ifdef _WIN32_WCE
	str_join_t(path, DataDir, EDIT_FILE, TEXT("."), op.EditFileSuffix, (TCHAR *)-1);
#else
	wsprintf(path, TEXT("%s%ld.%s"), DataDir, id, op.EditFileSuffix);
#endif

	if (ReadFlag == FALSE) {
		DeleteFile(path);
		return TRUE;
	}

	len = file_get_size(path);
	if (len < 0) {
		return FALSE;
	}
	fbuf = file_read(path, len);
#ifdef UNICODE
	//UNICODE
	tmp = alloc_char_to_tchar(fbuf);
	mem_free(&fbuf);
	p = tmp;
#else
	p = fbuf;
#endif
	if (p != NULL) {
		mem_free(&tpMailItem->Body);

		//Automatic operation label
		tmp = (TCHAR *)mem_alloc(
			sizeof(TCHAR) * (WordBreakStringSize(p, op.QuotationChar, op.WordBreakSize, op.QuotationBreak) + 1));
		if (tmp != NULL) {
			WordBreakString(p, tmp, op.QuotationChar, op.WordBreakSize, op.QuotationBreak);
			mem_free(&p);
			p = tmp;
		}

		//Line head... conversion
		len = SetDotSize(p);
#ifdef UNICODE
		tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (tmp != NULL) {
			SetDot(p, tmp);
		}
		tpMailItem->Body = alloc_tchar_to_char(tmp);
		mem_free(&tmp);
#else
		tpMailItem->Body = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (tpMailItem->Body != NULL) {
			SetDot(p, tpMailItem->Body);
		}
#endif
		mem_free(&p);
	}

	DeleteFile(path);
	return TRUE;
}

/*
 * EditProc - Mail indicatory procedure
 */
static LRESULT CALLBACK EditProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MAILITEM *tpMailItem;
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	static BOOL SipFlag = TRUE;
#endif
	BOOL dlg;

	switch (msg) {
	case WM_CREATE:
#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
		SipFlag = TRUE;
#endif
		if (InitWindow(hWnd, (MAILITEM *)(((CREATESTRUCT *)lParam)->lpCreateParams)) == FALSE) {
			DestroyWindow(hWnd);
			break;
		}
		SetWindowLong(hWnd, GWL_USERDATA, (LPARAM)((CREATESTRUCT *)lParam)->lpCreateParams);
#ifndef _WIN32_WCE
		SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES);
#endif
		break;

#ifdef _WIN32_WCE_PPC
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SHACTIVATEINFO sai;

			memset(&sai, 0, sizeof(SHACTIVATEINFO));
			SHHandleWMSettingChange(hWnd, -1, 0, &sai);
			SipFlag = sai.fSipUp;
			SetWindowSize(hWnd, 0, 0);
		}
		break;
#elif defined _WIN32_WCE_LAGENDA
	case WM_SETTINGCHANGE:
		if (SPI_SETSIPINFO == wParam && GetForegroundWindow() == hWnd) {
			SipFlag = SetWindowSize(hWnd, 0, 0);
		}
		break;
#endif

#ifndef _WIN32_WCE
	case WM_DROPFILES:
		tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
		if (tpMailItem != NULL) {
			TCHAR fpath[BUF_SIZE], *p;
			int i, len, size = 0;

			len = DragQueryFile((HANDLE)wParam, 0xFFFFFFFF, NULL, 0);
			// サイズ取得
			for (i = 0; i < len; i++) {
				DragQueryFile((HANDLE)wParam, i, fpath, BUF_SIZE - 1);
				if (dir_check(fpath) == FALSE) {
					size += lstrlen(fpath) + 1;
				}
			}
			if (size == 0) {
				DragFinish((HANDLE)wParam);
				break;
			}
			// メモリ確保
			p = tmp_attach = (TCHAR *)mem_alloc(sizeof(TCHAR) * (size + 1));
			if (tmp_attach == NULL) {
				DragFinish((HANDLE)wParam);
				break;
			}
			// ファイルリスト生成
			for (i = 0; i < len; i++) {
				if (p != tmp_attach) {
					*(p++) = ATTACH_SEP;
				}
				DragQueryFile((HANDLE)wParam, i, fpath, BUF_SIZE - 1);
				if (dir_check(fpath) == FALSE) {
					p = str_cpy_t(p, fpath);
				}
			}
			DragFinish((HANDLE)wParam);
			// 添付設定ダイアログ表示
			SendMessage(hWnd, WM_COMMAND, ID_MENUITEM_ATTACH, 0);
			mem_free(&tmp_attach);
			tmp_attach = NULL;
		}
		break;
#endif

	case WM_SIZE:
		SetWindowSize(hWnd, wParam, lParam);
		break;

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
			BOOL ShowFlag, BodyFlag = FALSE;

			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (tpMailItem != NULL && tpMailItem->hProcess != NULL) {
				ReadEditMail(hWnd, (long)hWnd, tpMailItem, TRUE);
				BodyFlag = TRUE;
				tpMailItem->hProcess = NULL;
#ifdef _WIN32_WCE
				ProcessFlag = FALSE;
#endif
			}
			if (wParam == 2) {
				ShowFlag = TRUE;
				wParam = 0;
			} else {
				ShowFlag = FALSE;
			}
			if ((tpMailItem != NULL && tpMailItem->MailStatus == ICON_SENTMAIL)
				|| SetItemToSendBox(hWnd, tpMailItem, BodyFlag, wParam, FALSE)) {
				CloseEditMail(hWnd, FALSE, ShowFlag);
#ifdef _WIN32_WCE_LAGENDA
				SipShowIM(SIPF_OFF);
#endif
			}
		}
		break;

#ifdef _WIN32_WCE_PPC
	case WM_HOTKEY:
		// code courtesy of Christian Ghisler
		if (op.osMajorVer >= 5 && LOWORD(lParam)==0) {
			HWND submenu;
			RECT r;
			POINT pt;
			int itemopen;
				switch(HIWORD(lParam)) {
			case VK_F1: // VK_TSOFT1
			case VK_F2: // VK_TSOFT2
				if (EditMenuOpened) {
					itemopen = EditMenuOpened - 1;
				} else {
					itemopen = (HIWORD(lParam)==VK_F1) ? 0 : 1;
				}
				SendMessage(hEditToolBar, TB_GETITEMRECT, itemopen, (LPARAM)&r);
				pt.x = (r.left + r.right) / 2;
				pt.y = (r.top + r.bottom) / 2;
				submenu = GetWindow(hEditToolBar, GW_CHILD);
				EditMenuOpened = 0;
				PostMessage(submenu, WM_LBUTTONDOWN, 1, MAKELONG(pt.x,pt.y));
				PostMessage(submenu, WM_LBUTTONUP, 1, MAKELONG(pt.x,pt.y));
				break;
			}
		}
		break;
	case WM_EXITMENULOOP:
		EditMenuOpened = 0;
		break;
	case WM_INITMENUPOPUP:
		SetEditMenu(hWnd);
		// try to enable an item on the menu to see which one is visible
		if (EnableMenuItem((HMENU)wParam, ID_MENUITEM_ALLSELECT, MF_BYCOMMAND | MF_ENABLED) != 0xFFFFFFFF)
			EditMenuOpened = 2;
		else if (EnableMenuItem((HMENU)wParam, ID_MENUITEM_SENDINFO, MF_BYCOMMAND | MF_ENABLED) != 0xFFFFFFFF)
			EditMenuOpened = 1;
		break;
#else 
	case WM_INITMENUPOPUP:
		if (LOWORD(lParam) == 1) {
			SetEditMenu(hWnd);
		}
		break;
#endif

	case WM_CLOSE:
		if (EndEditWindow(hWnd,FALSE) == FALSE) {
			break;
		}
#ifdef _WIN32_WCE
		FocusWnd = MainWnd;
		ShowWindow(MainWnd, SW_SHOW);
		_SetForegroundWindow(MainWnd);
#endif
		break;


	case WM_TIMER:
		switch (wParam) {

		case ID_APP_TIMER:
			KillTimer(hWnd, wParam);
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (tpMailItem == NULL) {
				break;
			}

#ifdef _WIN32_WCE
			if (hViewWnd != NULL) {
				SendMessage(hViewWnd, WM_CLOSE, 0, 0);
			}
			ProcessFlag = TRUE;
#endif
			//External editor starting
			if (AppEditMail(hWnd, (long)hWnd, tpMailItem->Body, tpMailItem) == FALSE) {
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
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (tpMailItem == NULL || tpMailItem->hProcess == NULL) {
				KillTimer(hWnd, wParam);
				break;
			}
			//End supervision
			if (WaitForSingleObject(tpMailItem->hProcess, 0) == WAIT_TIMEOUT) {
				break;
			}
			KillTimer(hWnd, wParam);
			tpMailItem->hProcess = NULL;

			if (tpMailItem->MailStatus == ICON_SENTMAIL) {
				// this actually just deletes the temporary file
				ReadEditMail(hWnd, (long)hWnd, tpMailItem, FALSE);
#ifdef _WIN32_WCE
				ProcessFlag = FALSE;
#endif
				CloseEditMail(hWnd, FALSE, TRUE);
				break;
			}

			//of external editor The file it does again to read,
			ReadEditMail(hWnd, (long)hWnd, tpMailItem, TRUE);

#ifdef _WIN32_WCE
			ProcessFlag = FALSE;
#endif
			//In transmission box retention
			if (SetItemToSendBox(hWnd, tpMailItem, TRUE, 0, (tpMailItem->Mark == ICON_SEND) ? TRUE : FALSE) == TRUE) {
				CloseEditMail(hWnd, FALSE, TRUE);
			} else {
				ShowWindow(hWnd, SW_SHOW);
				UpdateWindow(hWnd);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		return NotifyProc(hWnd, lParam);

	case WM_COMMAND:
		switch (GET_WM_COMMAND_ID(wParam,lParam)) {
#ifdef _WIN32_WCE
		case ID_MENU:
			SetEditMenu(hWnd);
			ShowMenu(hWnd, hEditPop, 0, 0);
			break;
#endif

#ifdef _WIN32_WCE_PPC
		case IDC_EDIT_BODY:
			switch (HIWORD(wParam)) {
			case EN_SETFOCUS:
				SHSipPreference(hWnd, (SipFlag) ? SIP_UP : SIP_DOWN);
				break;
			case EN_KILLFOCUS:
				SHSipPreference(hWnd, SIP_DOWN);
				break;
			}
			break;

		case IDOK:
			SendMessage(hWnd, WM_ENDCLOSE, (WPARAM)2, 0);
			break;
#elif defined _WIN32_WCE_LAGENDA
		case IDC_EDIT_BODY:
			switch (HIWORD(wParam)) {
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
			if (gSendAndQuit == FALSE
				&& ParanoidMessageBox(hWnd, STR_Q_SENDMAIL,
				STR_TITLE_SEND, MB_ICONQUESTION | MB_YESNO) == IDNO) {
				break;
			}
			if (g_soc != -1 || gSockFlag == TRUE) {
				ErrorMessage(NULL, STR_ERR_SENDLOCK);
				break;
			}
			if (SetItemToSendBox(hWnd, NULL, FALSE, 0, TRUE) == TRUE) {
				CloseEditMail(hWnd, TRUE, TRUE);
			}
			break;

		case ID_MENUITEM_SENDBOX:
			if (SetItemToSendBox(hWnd, NULL, FALSE, 0, FALSE) == TRUE) {
				CloseEditMail(hWnd, FALSE, TRUE);
			}
			break;

		case ID_MENUITEM_SBOXMARK:
			if (SetItemToSendBox(hWnd, NULL, FALSE, 0, TRUE) == TRUE) {
				CloseEditMail(hWnd, FALSE, TRUE);
			}
			break;

		case ID_MENUITEM_NEXTMAIL:
			tpMailItem = View_NextPrev(hWnd, +1, FALSE);
			if (tpMailItem != NULL) {
				UpdateEditWindow(hWnd, tpMailItem);
			}
			break;

		case ID_MENUITEM_PREVMAIL:
			tpMailItem = View_NextPrev(hWnd, -1, FALSE);
			if (tpMailItem != NULL) {
				UpdateEditWindow(hWnd, tpMailItem);
			}
			break;

		case ID_MENUITEM_SAVECOPY:
			// edit as new
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (tpMailItem != NULL) {
				MAILITEM *tpTmpMailItem = item_to_mailbox(MailBox + MAILBOX_SEND, tpMailItem, NULL, TRUE);
				if (tpTmpMailItem != NULL) {
					int j, state;
					HWND hListView = GetDlgItem(MainWnd, IDC_LISTVIEW);
					j = ListView_InsertItemEx(hListView,
						(TCHAR *)LPSTR_TEXTCALLBACK, 0, I_IMAGECALLBACK, (long)tpTmpMailItem,
						ListView_GetItemCount(hListView));

					state = ListView_ComputeState(tpTmpMailItem->Priority, tpTmpMailItem->Multipart);
					ListView_SetItemState(hListView, j, INDEXTOSTATEIMAGEMASK(state), LVIS_STATEIMAGEMASK)
					ListView_SetItemState(hListView, -1, 0, LVIS_SELECTED);
					ListView_SetItemState(hListView,
						j, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
					ListView_RedrawItems(hListView, j, j);
					ListView_EnsureVisible(hListView, j, TRUE);
					UpdateWindow(hListView);
					CloseEditMail(hWnd, FALSE, FALSE);
					OpenItem(MainWnd, FALSE, FALSE);
				}
			}
			break;

		case ID_MENUITEM_VIEW:
			// external editor
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (tpMailItem != NULL) {
				TCHAR *buf;
				int len;
				SwitchCursor(FALSE);
				mem_free(&tpMailItem->Body);
				tpMailItem->Body = NULL;
				len = SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXTLENGTH, 0, 0) + 1;
				if (len > 0) {
					buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
					if (buf != NULL) {
						*buf = TEXT('\0');
						SendDlgItemMessage(hWnd, IDC_EDIT_BODY, WM_GETTEXT, len, (LPARAM)buf);
						tpMailItem->BodyEncoding = op.BodyEncoding;
#ifndef _WCE_OLD
						if (tpMailItem->BodyCharset == NULL ||
							(tpMailItem->Body = MIME_charset_encode(charset_to_cp((BYTE)font_charset), buf, tpMailItem->BodyCharset)) == NULL) {
#endif
#ifdef UNICODE
							tpMailItem->Body = alloc_tchar_to_char(buf);
#else
							tpMailItem->Body = alloc_copy(buf);
#endif
#ifndef _WCE_OLD
						}
#endif
						mem_free(&buf);
					}
				}
				ShowWindow(hEditWnd, SW_HIDE);
				SetTimer(hEditWnd, ID_APP_TIMER, 1, NULL);
				SwitchCursor(TRUE);
			}
			break;

		case ID_MENUITEM_SENDINFO:
			ShowSendInfo(hWnd);
			break;

		case ID_MENUITEM_ATTACH:
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			dlg = FALSE;
			if (tpMailItem != NULL) {
				dlg = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ATTACH), hWnd, SetAttachProc, (LPARAM)tpMailItem);
				attach_item_free();
			}
			if (dlg == TRUE) {
				int st, i;
				(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
				SetHeaderString(GetDlgItem(hWnd, IDC_HEADER), tpMailItem);
				if ((tpMailItem->Attach != NULL && *tpMailItem->Attach != TEXT('\0')) ||
					(tpMailItem->FwdAttach != NULL && *tpMailItem->FwdAttach != TEXT('\0'))) {
					tpMailItem->Multipart = MULTIPART_ATTACH;
					st = 3;
				} else {
					tpMailItem->Multipart = MULTIPART_NONE;
					st = 0;
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
			}
			break;

		case ID_MENUITEM_FIND:
			View_FindMail(hWnd, TRUE);
			break;

		case ID_MENUITEM_NEXTFIND:
			View_FindMail(hWnd, FALSE);
			break;

		case ID_MENUITEM_REPLACE:
			FindOrReplace = 2;
#ifdef _WIN32_WCE_PPC
			{
				BOOL sipup = FALSE;
				if (SipFlag) {
					sipup = TRUE;
					SHSipPreference(hWnd, SIP_FORCEDOWN);
				}
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_REPLACE), hWnd, SetFindProc);
				if (sipup) {
					SHSipPreference(hWnd, SIP_UP);
				}
			}
#else
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_FIND), hWnd, SetFindProc);
#endif
			break;

		case ID_MENUITEM_ENCODE:
			// エンコード設定
			tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
			if (tpMailItem != NULL && DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_ENCODE), hWnd, SetEncodeProc, (LPARAM)tpMailItem) == TRUE) {
				(MailBox + MAILBOX_SEND)->NeedsSave |= MAILITEMS_CHANGED;
			}
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

		case ID_MENUITEM_PASTEQUOT:
		#ifdef UNICODE
			if (IsClipboardFormatAvailable(CF_UNICODETEXT) != 0) {
#else
			if (IsClipboardFormatAvailable(CF_TEXT) != 0) {
#endif
				if (OpenClipboard(hWnd) != 0) {
					HANDLE hclip;
#if (!defined(_WIN32_WCE) || defined(_WCE_NEW))
					TCHAR *p;
#endif
					TCHAR *clp, *buf, *qchar, dqch[3] = TEXT("> ");
					int len;
#ifdef UNICODE
					hclip = GetClipboardData(CF_UNICODETEXT); 
#else
					hclip = GetClipboardData(CF_TEXT); 
#endif
					clp = NULL;
#if (!defined(_WIN32_WCE) || defined(_WCE_NEW))
					if ((p = GlobalLock(hclip)) != NULL) {
						clp = alloc_copy_t(p);
						GlobalUnlock(hclip);
					}
#else
					// WCE 2.0 and 2.11 don't have GlobalLock?
					// (at least not with VisualStudio 6)
					clp = alloc_copy_t((TCHAR *)hclip);
#endif
					CloseClipboard();
					if (clp != NULL) {
						qchar = op.QuotationChar;
						if (qchar == NULL || *qchar == TEXT('\0')) {
							qchar = dqch;
						}
						len = GetReplyBodySize(clp, qchar) + 1;
						buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
						if (buf != NULL) {
							SetReplyBody(clp, buf, qchar);
							SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)buf);
							mem_free(&buf);
						}
						mem_free(&clp);
					}
				}
			}
			break;

		case ID_MENUITEM_REFLOW:
			{
#define MAX_QUOTE_LEN 40
				TCHAR *buf, *repl, *tmp, *end, *p, *r, *t;
				TCHAR qchar[MAX_QUOTE_LEN+1];
				int ss, se, ls, len;
				BOOL skip = FALSE, quoting = FALSE;
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_GETSEL, (WPARAM)&ss, (LPARAM)&se);
				if (se < ss) break;
				buf = NULL;
				AllocGetText(GetDlgItem(hWnd, IDC_EDIT_BODY), &buf);
				if (buf == NULL) {
					break;
				}
				end = (buf+se);

				// search backwards for \n (ls = line start)
				p = (buf+ss);
				ls = -1;
				while (p >= buf && *p != TEXT('\n')) {
					p--;
					ls++;
				}
				len = (se-ss+ls+1);
				if (op.WordBreakSize > 0) {
					len += (len/op.WordBreakSize) * MAX_QUOTE_LEN;
				}
				repl = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
				if (repl == NULL) {
					mem_free(&buf);
					break;
				}
				*repl = TEXT('\0');
				r = repl;
				tmp = (TCHAR *)mem_alloc(sizeof(TCHAR) * len);
				if (tmp == NULL) {
					mem_free(&buf);
					mem_free(&repl);
					break;
				}
				*tmp = TEXT('\0');
				t = tmp;

				if (len > MAX_QUOTE_LEN) len = MAX_QUOTE_LEN;
				quoting = GetQuoteString(buf+(ss-ls), qchar, len);
				len = lstrlen(qchar);

				// copy beginning of line (for linebreaking purposes)
				for (p = buf+(ss-ls); p < buf+ss; p++, t++) {
				  *t = *p;
				}
				*t = TEXT('\0');

				for (p = buf+ss; *p != TEXT('\0') && p < end; p++) {
#ifndef UNICODE
					if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p+1) != TEXT('\0')) {
						*(t++) = *(p++);
						*(t++) = *p;
						continue;
					}
#endif
					if (*p == TEXT('\r') && *(p+1) == TEXT('\n') && (p+2 < end)) {
						BOOL newq, wrapit = FALSE;
						TCHAR newqc[MAX_QUOTE_LEN+1];
						int newlen;
#ifndef UNICODE
						if (IsDBCSLeadByte((BYTE)*(p+2)) == TRUE) {
							newq = FALSE;
							*newqc = TEXT('\0');
							newlen = 0;
						} else 
#endif
						{
							newlen = (end - p) - 2;
							if (newlen > MAX_QUOTE_LEN) newlen = MAX_QUOTE_LEN;
							newq = GetQuoteString(p+2, newqc, newlen);
							newlen = lstrlen(newqc);
						}

						if (newq != quoting || newlen > len) {
							// done with this block
							wrapit = TRUE;
							skip = FALSE;
						} else if (newlen < len) {
							TCHAR *q;
							q = p + 2 + newlen;
							if (*q == TEXT('\r')) {
								skip = TRUE;
							}
							while (*q != TEXT('\r') && *q != TEXT('\0') && q < end) {
#ifndef UNICODE
								if (IsDBCSLeadByte((BYTE)*q) == TRUE && *(q+1) != TEXT('\0')) {
									q++;
								}
#endif
								q++;
							}
							if (*q == TEXT('\r') && *(q+1) == TEXT('\n') && (q+2 < end)
								&& str_cmp_ni_t(q+2, qchar, len) == 0) {
								// > > blah blah
								// > blah
								// > > blah blah
								if ( p > buf && *(p-1) != TEXT(' ') && (p+2+newlen < end)
									&& *(p+2+newlen) != TEXT(' ')) {
									*(t++) = TEXT(' ');
								}
								p += (2 + newlen);
							} else {
								wrapit = TRUE;
							}
						} else {
							if (skip == TRUE) {
								skip = FALSE;
							} else {
								TCHAR *w = p + 2 + newlen;
								while (*w == TEXT(' ') || *w == TEXT('\t')) w++;
								if ( *w == TEXT('\r')) {
									skip = TRUE;
								} else {
									if ( p > buf && *(p-1) != TEXT(' ') && (p+2+newlen < end)
										&& *(p+2+newlen) != TEXT(' ')) {
										*(t++) = TEXT(' ');
									}
									p += (2 + newlen);
								}
							}
						}
				
						if (wrapit) {
							TCHAR *wrap = NULL;
							*t = TEXT('\0');
							if (op.WordBreakSize > 0) {
								wrap = (TCHAR *)mem_alloc(sizeof(TCHAR)
									* (WordBreakStringSize(tmp, qchar, op.WordBreakSize, op.QuotationBreak) + 1));
							}
							if (wrap != NULL) {
								WordBreakString(tmp, wrap, qchar, op.WordBreakSize, op.QuotationBreak);
								r = str_join_t(r, wrap, (TCHAR*)-1);
								mem_free(&wrap);
							} else {
								r = str_join_t(r, tmp, (TCHAR*)-1);
							}
							lstrcpy(qchar, newqc);
							len = newlen;
							quoting = newq;
							t = tmp;
						}
					}
					if (p < end) {
						*(t++) = *p;
					}
				}
				*t = TEXT('\0');
				if (op.WordBreakSize > 0) {
					TCHAR *wrap = (TCHAR *)mem_alloc(sizeof(TCHAR)
						* (WordBreakStringSize(tmp, qchar, op.WordBreakSize, op.QuotationBreak) + 1));
					if (wrap != NULL) {
						WordBreakString(tmp, wrap, qchar, op.WordBreakSize, op.QuotationBreak);
						r = str_join_t(r, wrap, (TCHAR*)-1);
						mem_free(&wrap);
						mem_free(&tmp);
						tmp = NULL;
					}
				}
				if (tmp != NULL) {
					r = str_join_t(r, tmp, (TCHAR*)-1);
					mem_free(&tmp);
				}
				mem_free(&buf);
				r = repl + ls;
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)r);
				mem_free(&repl);
			}
			break;

		case ID_MENUITEM_ALLSELECT:
			SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_SETSEL, 0, -1);
			break;

		case ID_MENUITEM_FILEOPEN:
			{
				TCHAR *buf;

				if (file_read_select(hWnd, &buf) == FALSE) {
					ErrorMessage(hWnd, STR_ERR_OPEN);
					break;
				}
				if (buf == NULL) {
					break;
				}
				SwitchCursor(FALSE);
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)buf);
				SwitchCursor(TRUE);

				mem_free(&buf);
			}
			break;

		case ID_MENUITEM_WORDBREAK:
			{
				BOOL sent = FALSE;
				tpMailItem = (MAILITEM *)GetWindowLong(hWnd, GWL_USERDATA);
				if (tpMailItem != NULL) {
					sent = (tpMailItem->MailStatus == ICON_SENTMAIL);
				}
				if (sent) {
					DelSentSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
				}
#ifdef _WIN32_WCE_PPC
				if (!sent) {
					DelEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
				}
				op.EditWordBreakFlag = SetWordBreak(hWnd, SHGetSubMenu(hEditToolBar, ID_MENUITEM_EDIT));
#elif defined(_WIN32_WCE_LAGENDA)
				op.EditWordBreakFlag = SetWordBreak(hWnd, hViewMenu);
#else
				op.EditWordBreakFlag = SetWordBreak(hWnd);
#endif
				SendDlgItemMessage(hWnd, IDC_EDIT_BODY, EM_LIMITTEXT, (WPARAM)EditMaxLength, 0);
				if (sent) {
					SetSentSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#ifdef _WIN32_WCE_PPC
				} else {
					SetEditSubClass(GetDlgItem(hWnd, IDC_EDIT_BODY));
#endif
				}
			}
			break;

#ifndef _WIN32_WCE
		case ID_MENUITEM_FONT:
			// フォント
			if (font_select(hWnd, &op.view_font) == TRUE) {
				HDC hdc;
				if (hViewFont != NULL) {
					DeleteObject(hViewFont);
				}
				hdc = GetDC(hWnd);
				hViewFont = font_create_or_copy(hWnd, hdc, &op.view_font);
				ReleaseDC(hWnd, hdc);
				if (hViewWnd != NULL) {
					SendDlgItemMessage(hViewWnd, IDC_EDIT_BODY, WM_SETFONT, (WPARAM)hViewFont, MAKELPARAM(TRUE,0));
				}
				font_charset = op.view_font.charset;
				// 他のEditウィンドウのフォントを設定
				EnumWindows((WNDENUMPROC)enum_windows_proc, 0);
			}
			break;
#endif
		}
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

/*
 * Edit_InitApplication - Register
 */
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
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SENTMAIL));
	wc.hbrBackground = (HBRUSH)COLOR_BTNSHADOW;
	wc.lpszClassName = EDIT_WND_CLASS;

	return RegisterClass(&wc);
}

/*
 * Edit_MailToSet - of window class From mail address compilation
 */
int Edit_MailToSet(HINSTANCE hInstance, HWND hWnd, TCHAR *mail_addr, int rebox)
{
	static BOOL ExistFlag = FALSE;
	MAILITEM *tpMailItem;
	BOOL ret;

	if (ExistFlag == TRUE) {
		return EDIT_NONEDIT;
	}

	tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
	if (tpMailItem == NULL) {
		ErrorMessage(hWnd, STR_ERR_MEMALLOC);
		return FALSE;
	}

	//URL (mailto)
	if (URLToMailItem(mail_addr, tpMailItem) == FALSE) {
		item_free(&tpMailItem, 1);
		return FALSE;
	}
	ExistFlag = TRUE;
	ret = Edit_InitInstance(hInstance, hWnd, rebox, tpMailItem, EDIT_NEW, NULL, FALSE);
	if (ret == EDIT_INSIDEEDIT && tpMailItem->Mark == ICON_SEND) {
		ret = ICON_SEND;
	}
	item_free(&tpMailItem, 1);
	ExistFlag = FALSE;
	return ret;
}

/*
 * Edit_ConfigureWindow - disable edit window items when mail was already sent
 */
void Edit_ConfigureWindow(HWND thisEditWnd, BOOL editable) {
	HMENU hMenu;
	unsigned int menu_state;

	if (thisEditWnd == NULL) {
		return;
	}

#ifdef _WIN32_WCE
#ifdef _WIN32_WCE_PPC
	hMenu = SHGetSubMenu(hEditToolBar, ID_MENUITEM_EDIT);
	SHSipPreference(thisEditWnd, (editable) ? SIP_UP : SIP_DOWN);
#elif defined(_WIN32_WCE_LAGENDA)
	hMenu = GetSubMenu(hViewMenu, 1);
	SipShowIM((editable) ? SIPF_ON : SIPF_OFF);
#else
	hMenu = CommandBar_GetMenu(GetDlgItem(thisEditWnd, IDC_VCB), 0);
#endif
#else
	hMenu = GetMenu(thisEditWnd);
#endif

	menu_state = (editable == TRUE) ? MF_ENABLED : MF_GRAYED;
	if (hMenu != NULL) {
		EnableMenuItem(hMenu, ID_MENUITEM_UNDO, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_CUT, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_PASTE, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_PASTEQUOT, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_REFLOW, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_FILEOPEN, menu_state);
#ifdef _WIN32_WCE_PPC
	}
	hMenu = SHGetSubMenu(hEditToolBar, ID_MENUITEM_FILE);
	if (hMenu != NULL) {
#endif
		EnableMenuItem(hMenu, ID_MENUITEM_SEND, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_SBOXMARK, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_SENDBOX, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_ENCODE, menu_state);
		EnableMenuItem(hMenu, ID_MENUITEM_ATTACH, menu_state);
	}

	if (hEditToolBar != NULL) {
		LPARAM lp1, lp2;
		lp1 = (LPARAM)MAKELONG((editable)? 0 : 1, 0);
		lp2 = (LPARAM)MAKELONG((editable)? 1 : 0, 0);
		SendMessage(hEditToolBar, TB_HIDEBUTTON, ID_MENUITEM_SEND, lp1);
		SendMessage(hEditToolBar, TB_HIDEBUTTON, ID_MENUITEM_SBOXMARK, lp1);
		SendMessage(hEditToolBar, TB_HIDEBUTTON, ID_MENUITEM_SENDBOX, lp1);
		SendMessage(hEditToolBar, TB_HIDEBUTTON, ID_MENUITEM_PREVMAIL, lp2);
		SendMessage(hEditToolBar, TB_HIDEBUTTON, ID_MENUITEM_NEXTMAIL, lp2);
		SendMessage(hEditToolBar, TB_HIDEBUTTON, ID_MENUITEM_SAVECOPY, lp2);
		if (!editable) {
			SendMessage(hEditToolBar, TB_ENABLEBUTTON, ID_MENUITEM_PREVMAIL, lp1);
			SendMessage(hEditToolBar, TB_ENABLEBUTTON, ID_MENUITEM_NEXTMAIL, lp1);
			SendMessage(hEditToolBar, TB_ENABLEBUTTON, ID_MENUITEM_SAVECOPY, lp1);
		}
	}
}

/*
 * Edit_InitInstance - of transmission mail Compilation
 */
int Edit_InitInstance(HINSTANCE hInstance, HWND hWnd, int rebox, MAILITEM *tpReMailItem,
					  int OpenFlag, TCHAR *seltext, BOOL NoAppFlag)
{
	MAILITEM *tpMailItem;
	int key;
	BOOL mkdlg;
#ifdef _WIN32_WCE_PPC
	SIPINFO si;
	int x = CW_USEDEFAULT, y = CW_USEDEFAULT, cx, cy;
#endif

#ifdef _WIN32_WCE
	//CE plural try not to open the compilation picture
	if (hEditWnd != NULL && OpenFlag != EDIT_FILTERFORWARD) {
		if (ProcessFlag == TRUE) {
			return EDIT_NONEDIT;
		}
		ShowWindow(hEditWnd, SW_SHOW);
		_SetForegroundWindow(hEditWnd);
		SendMessage(hEditWnd, WM_CLOSE, 0, 0);
		if (hEditWnd != NULL) {
			return EDIT_INSIDEEDIT;
		}
	}
#endif
	key = GetKeyState(VK_SHIFT);

	//With the type which it compiles initialization
	switch(OpenFlag)
	{
	case EDIT_OPEN:
		//The existing transmission mail is opened
		if (tpReMailItem == NULL) {
			return EDIT_NONEDIT;
		}
#ifndef _WIN32_WCE
		if (tpReMailItem->hEditWnd != NULL) {
			if (IsWindowVisible(tpReMailItem->hEditWnd) == TRUE) {
				ShowWindow(tpReMailItem->hEditWnd, SW_SHOW);
				if (IsIconic(tpReMailItem->hEditWnd) != 0) {
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
		// A new compilation mail
		tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
		if (tpMailItem == NULL) {
			ErrorMessage(hWnd, STR_ERR_MEMALLOC);
			return EDIT_NONEDIT;
		}
		if (tpReMailItem != NULL) {
			// from mailto: URL
			item_copy(tpReMailItem, tpMailItem, TRUE);
		} else if ((MailBox+rebox)->Type == MAILBOX_TYPE_SAVE 
			&& (MailBox+rebox)->DefAccount != NULL
			&& *(MailBox+rebox)->DefAccount != TEXT('\0')) {
			tpMailItem->MailBox = alloc_copy_t((MailBox+rebox)->DefAccount);
		}
		tpMailItem->MailStatus = ICON_NON;
		tpMailItem->Download = TRUE;
		tpMailItem->DefReplyTo = TRUE;

		if (gSendAndQuit == TRUE) {
			if (tpMailItem->To == NULL || (tpMailItem->Subject == NULL && tpMailItem->Body == NULL)) {
				return FALSE;
			}
			if (tpMailItem->MailBox == NULL) {
				tpMailItem->MailBox = alloc_copy_t((MailBox + SelBox)->Name);
			}
		} else {
			int res = IDD_DIALOG_SETSEND;

			// Transmission information setting
			_SetForegroundWindow(hWnd);
	
#ifdef _WIN32_WCE
			if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
				res = IDD_DIALOG_SETSEND_WIDE;
#ifdef _WIN32_WCE_PPC
			} else if (GetSystemMetrics(SM_CYSCREEN) <= 260) {
				res = IDD_DIALOG_SETSEND_SHORT;
#endif
			}
#endif
			if (FALSE == DialogBoxParam(hInst, MAKEINTRESOURCE(res), hWnd, SetSendProc, (LPARAM)tpMailItem)) {
				item_free(&tpMailItem, 1);
				return EDIT_NONEDIT;
			}
		}
		if (tpMailItem->Body == NULL) {
			SetReplyMessageBody(tpMailItem, NULL, EDIT_NEW, NULL);
		}
		break;

	case EDIT_REPLY:
	case EDIT_REPLYALL:
		// Reply
		tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
		if (tpMailItem == NULL) {
			ErrorMessage(hWnd, STR_ERR_MEMALLOC);
			return EDIT_NONEDIT;
		}
		tpMailItem->MailStatus = ICON_NON;
		tpMailItem->Download = TRUE;
		tpMailItem->DefReplyTo = TRUE;

		if (seltext != NULL) {
			tpMailItem->Mark = MARK_REPL_SELTEXT;
		} else if (tpReMailItem != NULL && tpReMailItem->Body != NULL) {
			tpMailItem->Mark = MARK_REPLYING;
		}
		if (tpReMailItem != NULL) {
			// Reply setting
			SetReplyMessage(tpMailItem, tpReMailItem, rebox, OpenFlag);
		}

		//Transmission information setting
#ifdef _WIN32_WCE
		{
			int res = IDD_DIALOG_SETSEND;
			if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
				res = IDD_DIALOG_SETSEND_WIDE;
#ifdef _WIN32_WCE_PPC
			} else if (GetSystemMetrics(SM_CYSCREEN) <= 260) {
				res = IDD_DIALOG_SETSEND_SHORT;
#endif
			}
			mkdlg = DialogBoxParam(hInst, MAKEINTRESOURCE(res), hWnd, SetSendProc, (LPARAM)tpMailItem);
		}
#else
		mkdlg = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
			(LPARAM)tpMailItem);
#endif
		if (mkdlg == FALSE) {
			item_free(&tpMailItem, 1);
			return EDIT_NONEDIT;
		}
		SetReplyMessageBody(tpMailItem, tpReMailItem, EDIT_REPLY, seltext);
		tpMailItem->Mark = 0;
		if (rebox != MAILBOX_SEND && tpReMailItem != NULL) {
			SetReplyFwdMark(tpReMailItem, ICON_REPL_MASK, rebox);
		}
		break;

	case EDIT_FORWARD:
	case EDIT_FILTERFORWARD:
	case EDIT_REDIRECT:
		// Forward, forward by filter action, or redirect

		// Check if message is complete, give option to bail out here
		if (tpReMailItem != NULL && tpReMailItem->Download == FALSE && OpenFlag != EDIT_FILTERFORWARD 
			&& MessageBox(hWnd, STR_Q_FORWARDMAIL, WINDOW_TITLE, MB_ICONQUESTION | MB_YESNO) == IDNO) {
			return EDIT_NONEDIT;
		}

		tpMailItem = (MAILITEM *)mem_calloc(sizeof(MAILITEM));
		if (tpMailItem == NULL){
			if (hWnd != NULL) {
				ErrorMessage(hWnd, STR_ERR_MEMALLOC);
			}
			return EDIT_NONEDIT;
		}
		tpMailItem->MailStatus = ICON_NON;
		tpMailItem->Download = TRUE;
		tpMailItem->DefReplyTo = TRUE;

		if (OpenFlag == EDIT_FORWARD && seltext != NULL) {
			tpMailItem->Mark = MARK_FWD_SELTEXT;
		} else if (tpReMailItem != NULL && tpReMailItem->Body != NULL) {
			tpMailItem->Mark = MARK_FORWARDING;
		}
		if (tpReMailItem != NULL) {
			// Forward settings
			SetReplyMessage(tpMailItem, tpReMailItem, rebox, OpenFlag);
		}
		if (OpenFlag == EDIT_FILTERFORWARD) {
			tpMailItem->To = alloc_copy_t(seltext);
			tpMailItem->Mark = 1; // means quote entire message
			mkdlg = TRUE;
		} else if (OpenFlag == EDIT_REDIRECT) {
			tpMailItem->RedirectTo = alloc_copy_t(TEXT("")); // indicates redirection in SetSendProc
			tpMailItem->From = alloc_copy_t(tpReMailItem->From);
			tpMailItem->ContentType = alloc_copy_t(tpReMailItem->ContentType);
			tpMailItem->Encoding = alloc_copy_t(tpReMailItem->Encoding);
			tpMailItem->Mark = ICON_SEND;
			mkdlg = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_REDIRECT), hWnd, SetSendProc,
				(LPARAM)tpMailItem);
		} else {
			//Transmission information setting
#ifdef _WIN32_WCE
			int res = IDD_DIALOG_SETSEND;
			if (GetSystemMetrics(SM_CXSCREEN) >= 450) {
				res = IDD_DIALOG_SETSEND_WIDE;
#ifdef _WIN32_WCE_PPC
			} else if (GetSystemMetrics(SM_CYSCREEN) <= 260) {
				res = IDD_DIALOG_SETSEND_SHORT;
#endif
			}
			mkdlg = DialogBoxParam(hInst, MAKEINTRESOURCE(res), hWnd, SetSendProc, (LPARAM)tpMailItem);
#else
			mkdlg = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_DIALOG_SETSEND), hWnd, SetSendProc,
				(LPARAM)tpMailItem);
#endif
		}
		if (mkdlg == FALSE) {
			item_free(&tpMailItem, 1);
			return EDIT_NONEDIT;
		}
		if (OpenFlag == EDIT_REDIRECT) {
			if (rebox != MAILBOX_SEND && tpReMailItem != NULL) {
				SetReplyFwdMark(tpReMailItem, ICON_FWD_MASK, rebox);
			}
			SetItemToSendBox(NULL, tpMailItem, TRUE, EDIT_SEND, (tpMailItem->Mark == ICON_SEND));
			return EDIT_SEND;
		}
		SetReplyMessageBody(tpMailItem, tpReMailItem, OpenFlag, seltext);
		if (rebox != MAILBOX_SEND && tpReMailItem != NULL) {
			SetReplyFwdMark(tpReMailItem, ICON_FWD_MASK, rebox);
			if (tpMailItem->FwdAttach != NULL) {
				tpReMailItem->ReFwd |= REFWD_FWDHOLD;
				(MailBox + rebox)->HeldMail = TRUE;
				(MailBox + rebox)->NeedsSave |= MARKS_CHANGED;
			}
		}
		tpMailItem->Mark = 0;
		if (OpenFlag == EDIT_FILTERFORWARD) {
			SetItemToSendBox(NULL, tpMailItem, TRUE, EDIT_SEND, TRUE);
			return EDIT_SEND;
		}
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
		op.EditRect.left,
		op.EditRect.top,
		op.EditRect.right - op.EditRect.left,
		op.EditRect.bottom - op.EditRect.top,
		NULL, NULL, hInstance, (LPVOID)tpMailItem);
#endif
	if (hEditWnd == NULL) {
		if (OpenFlag != EDIT_OPEN) {
			item_free(&tpMailItem, 1);
		}
		ErrorMessage(hWnd, STR_ERR_INIT);
		return EDIT_NONEDIT;
	}
	if ( (NoAppFlag == FALSE) && (OpenFlag != EDIT_NEW || (gSendAndQuit == FALSE && tpMailItem->Mark != ICON_SEND))
		&& ((op.DefEditApp == 1 && key >= 0) || (op.DefEditApp == 0 && key < 0))) {
		ShowWindow(hEditWnd, SW_HIDE);
		SetTimer(hEditWnd, ID_APP_TIMER, 1, NULL);
		return EDIT_OUTSIDEEDIT;
	} else {
		ShowWindow(hEditWnd, SW_SHOW);
		UpdateWindow(hEditWnd);
	}
	return EDIT_INSIDEEDIT;
}
/* End of source */
