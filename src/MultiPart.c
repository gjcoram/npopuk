/*
 * nPOP
 *
 * Multipart.c
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
#include "code.h"
#include "mime.h"
#include "multipart.h"

/* Define */
#define is_white(c)	(c == TEXT(' ') || c == TEXT('\r') || c == TEXT('\t'))

/* Global Variables */
extern OPTION op;

/* Local Function Prototypes */
static char *get_next_part(char *buf, char *Boundary);
static void get_content(char *buf, char *str, char **ret);
static BOOL get_content_value(char *Content, char *Attribute, char *ret);
static char *multipart_get_filename_rfc2231(char *buf);

/*
 * multipart_add - Part情報の追加
 */
MULTIPART *multipart_add(MULTIPART ***tpMultiPart, int cnt)
{
	MULTIPART **TmpMultiPart;

	TmpMultiPart = (MULTIPART **)mem_calloc(sizeof(MULTIPART *) * (cnt + 1));
	if (TmpMultiPart == NULL) {
		return NULL;
	}
	if (*tpMultiPart != NULL) {
		CopyMemory(TmpMultiPart, *tpMultiPart,
			sizeof(MULTIPART *) * cnt);
		mem_free((void **)&*tpMultiPart);
	}
	*(TmpMultiPart + cnt) = (MULTIPART *)mem_calloc(sizeof(MULTIPART));
	*tpMultiPart = TmpMultiPart;
	return *(TmpMultiPart + cnt);
}

/*
 * multipart_free - Part情報の解放
 */
void multipart_free(MULTIPART ***tpMultiPart, int cnt)
{
	int i;

	// Part情報の解放
	for (i = 0; i < cnt; i++) {
		if (*(*tpMultiPart + i) == NULL) {
			continue;
		}
		mem_free(&(*(*tpMultiPart + i))->ContentType);
		mem_free(&(*(*tpMultiPart + i))->Filename);
		mem_free(&(*(*tpMultiPart + i))->Encoding);
		mem_free(&(*(*tpMultiPart + i))->ContentID);

		mem_free(&*(*tpMultiPart + i));
	}
	mem_free((void **)&*tpMultiPart);
}

/*
 * encatt_free - free encoded attachments
 */
#ifndef WSAASYNC
void encatt_free(char ***EncAtt, int cnt)
{
	int i;
	for (i = 0; i < cnt; i++) {
		mem_free(&(*(*EncAtt+i)));
	}
	mem_free((void **)&*EncAtt);
}
#endif

/*
 * get_next_part - 次のPartの位置を取得
 */
static char *get_next_part(char *buf, char *Boundary)
{
	char *p = buf;
	int len = tstrlen(Boundary);

	if (*p == '-' && *(p + 1) == '-' && str_cmp_n(p + 2, Boundary, len) == 0) {
		return p;
	}
	while (1) {
		for (; !(*p == '\r' && *(p + 1) == '\n') && *p != '\0'; p++);
		if (*p == '\0') {
			break;
		}
		p += 2;
		if (!(*p == '-' && *(p + 1) == '-')) {
			continue;
		}
		if (str_cmp_n(p + 2, Boundary, len) != 0) {
			continue;
		}
		break;
	}
	return p;
}

/*
 * get_content - コンテンツの取得
 */
static void get_content(char *buf, char *str, char **ret)
{
	char *p;
	int len;

	// 位置の取得
	p = GetHeaderStringPoint(buf, str);
	if (p == NULL) {
		*ret = NULL;
		return;
	}
	// サイズの取得
	len = GetHeaderStringSize(p, FALSE);
	*ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (*ret == NULL) {
		return;
	}
	GetHeaderString(p, *ret, FALSE);
}

/*
 * get_content_value - コンテンツ内の指定した値を取得
 */
static BOOL get_content_value(char *Content, char *Attribute, char *ret)
{
	char *p = Content;
	char *r = ret;

	for (; *p == ' ' || *p == '\t'; p++);

	while (*p != '\0') {
		// attributeをチェック
		if (str_cmp_ni(p, Attribute, tstrlen(Attribute)) != 0) {
			// 次のparameterに移動
			for (; *p != '\0' && *p != ';'; p++);
			if (*p == ';') {
				p++;
				for (; *p == ' '; p++);
				if (*p == '\r' && *(p+1) == '\n' && (*(p+2) == ' ' || *(p+2) == '\t')) {
					p+=3;
				}
			}
			for (; *p == ' ' || *p == '\t'; p++);
			continue;
		}
		p += tstrlen(Attribute);
		for (; *p == ' ' || *p == '\t'; p++);
		if (*p == '=') {
			p++;
		}
		for (; *p == ' ' || *p == '\t'; p++);
		if (*p == '\"') {
			p++;
		}
		// valueの取得
		if (r != NULL) {
			for (; *p != '\0' && *p != '\"' && *p != ';'; p++, r++) {
				*r = *p;
			}
			*r = '\0';
		}
		return TRUE;
	}
	return FALSE;
}

/*
 * multipart_get_filename_rfc2231 - ファイル名の取得 (RFC 2231)
 */
static char *multipart_get_filename_rfc2231(char *buf)
{
	char *p = buf, *r;
	char *tmp;
	char *ret;
	char **Names = NULL, **NamesTmp;
	int No;
	int AllocNo = 0;
	int len;
	BOOL EncFlag = FALSE;

	if (buf == NULL) {
		return NULL;
	}

	while (*p != '\0') {
		// attributeをチェック
		if (str_cmp_ni(p, "filename*", tstrlen("filename*")) != 0) {
			// 次のparameterに移動
			for (; *p != '\0' && *p != ';'; p++);
			if (*p == ';') p++;
			for (; *p == ' ' || *p == '\t'; p++);
			continue;
		}
		p += tstrlen("filename");
		if (*(p + 1) == '=') {
			// 単一の場合
			No = 0;
		} else {
			// 連番が付いている場合
			p++;
			No = a2i(p);
		}
		if (AllocNo <= No) {
			// ファイル名のリストの確保
			NamesTmp = (char **)mem_calloc(sizeof(char *) * (No + 1));
			if (NamesTmp == NULL) {
				break;
			}
			if (Names != NULL) {
				CopyMemory(NamesTmp, Names, sizeof(char *) * AllocNo);
				mem_free((void **)&Names);
			}
			Names = NamesTmp;
			AllocNo = No + 1;
		}
		for (; *p != '\0' && *p != '*' && *p != '='; p++);
		if (No == 0) {
			EncFlag = (*p == '*') ? TRUE : FALSE;
		}
		for (; *p == '*' || *p == '=' || *p == '\"'; p++);

		// valueの取得
		for (r = p; *p != '\0' && *p != '\"' && *p != ';'; p++);
		tmp = (char *)mem_alloc(sizeof(char) * (p - r + 1));
		if (tmp == NULL) {
			break;
		}
		str_cpy_n(tmp, r, p - r + 1);

		*(Names + No) = (char *)mem_alloc(sizeof(char) * (tstrlen(tmp) + 1));
		if (*(Names + No) == NULL) {
			mem_free(&tmp);
			break;
		}
		tstrcpy(*(Names + No), tmp);
		mem_free(&tmp);
	}
	if (Names == NULL) {
		return NULL;
	}

	// ファイル名の長さを計算
	for (No = 0, len = 0; No < AllocNo; No++) {
		if (*(Names + No) != NULL) {
			len += tstrlen(*(Names + No));
		}
	}
	ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (ret != NULL) {
		// ファイル名を連結
		for (No = 0, r = ret; No < AllocNo; No++) {
			if (*(Names + No) != NULL) {
				r = str_cpy(r, *(Names + No));
			}
		}
		// デコード
		if (EncFlag == TRUE) {
			tmp = MIME_rfc2231_decode(ret);
			if (tmp != NULL) {
				mem_free(&ret);
				ret = tmp;
			}
		}
	}
	// ファイル名のリストの解放
	for (No = 0; No < AllocNo; No++) {
		if (*(Names + No) != NULL) {
			mem_free(&*(Names + No));
		}
	}
	mem_free((void **)&Names);
	return ret;
}

/*
 * multipart_get_filename - ファイル名の取得
 */
char *multipart_get_filename(char *buf, char *Attribute)
{
	char *p = buf;
	char *fname, *dname;
#ifdef UNICODE
	TCHAR *wdname;
#endif
	int len;

	if (buf == NULL) {
		return NULL;
	}

	fname = (char *)mem_alloc(sizeof(char) * (tstrlen(buf) + 1));
	if (fname == NULL) {
		return NULL;
	}
	if (get_content_value(buf, Attribute, fname) == FALSE) {
		mem_free(&fname);
		return NULL;
	}
	len = MIME_decode(fname, NULL);
#ifdef UNICODE
	wdname = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
	if (wdname == NULL) {
		mem_free(&fname);
		return NULL;
	}
	MIME_decode(fname, wdname);
	mem_free(&fname);
	dname = alloc_tchar_to_char(wdname);
	mem_free(&wdname);
#else
	dname = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (dname == NULL) {
		mem_free(&fname);
		return NULL;
	}
	MIME_decode(fname, dname);
	mem_free(&fname);
#endif
	return dname;
}

/*
 * multipart_verify - quick check if mail has real attachments (GJC)
 *                    now used to fix AppleMail bug
 */
int multipart_verify(char *ContentType, char *buf) {
	char *Boundary, *Content = NULL, *p;

	// Content-Type: multipart/alternative was found before calling this function,
	// so default return value is MULTIPART_HTML
	int retval = MULTIPART_HTML;

	if (ContentType == NULL || buf == NULL) {
		return retval;
	}
	Boundary = (char *)mem_alloc(sizeof(char) * (tstrlen(ContentType) + 1));
	if (Boundary == NULL) {
		return retval;
	}
	if (get_content_value(ContentType, "boundary", Boundary) == TRUE) {
		int cnt = 0;
		p = get_next_part(buf, Boundary);
		while (p != '\0' && tstrlen(p) > 2 + tstrlen(Boundary)) {
			p += (2 + tstrlen(Boundary));
			if (*p == '-' && *(p + 1) == '-') {
				// end of message, we're done
				break;
			}
			cnt++;
			if (cnt > 2) {
				// more than 2 parts
				retval = MULTIPART_CONTENT;
				break;
			}
			get_content(p, HEAD_CONTENTTYPE, &Content);
			if (Content == NULL) {
				// imcomplete message: assume mailer set the correct content-type
				// retval = MULTIPART_HTML;
				break;
			}
			if (get_content_value(Content, "name", NULL) == TRUE) {
				retval = MULTIPART_CONTENT;
				break;
			} else if (str_cmp_ni(Content, "text", tstrlen("text")) != 0) {
				retval = MULTIPART_CONTENT;
				break;
			} else {
				char *Dispo;
				get_content(p, HEAD_DISPOSITION, &Dispo);
				if (Dispo != NULL && get_content_value(Dispo, "filename", NULL) == TRUE) {
					retval = MULTIPART_CONTENT;
					break;
				}
				mem_free(&Dispo);
				// MULTIPART_HTML is the default
				//if (str_cmp_ni(Content, "text/html", tstrlen("text/html")) == 0) {
				//	retval = MULTIPART_HTML;
				//}
			}
			mem_free(&Content);
			p = GetBodyPointa(p);
			if (p == NULL || *p == '\0') {
				retval = MULTIPART_CONTENT;
				break;
			}
			p = get_next_part(p, Boundary);
		}
		mem_free(&Content);
	}
	mem_free(&Boundary);
	return retval;
}

/*
 * multipart_parse - Partを解析する (RFC 2046)
 */
int multipart_parse(char *ContentType, char *buf, BOOL StopAtTextPart, MULTIPART ***tpMultiPart, int cnt)
{
	MULTIPART *tpMultiPartItem;
	char *Boundary;
	char *p, *tmpname = NULL;
	char *Content, *sPos;
	BOOL is_digest = FALSE;

	if (ContentType == NULL || buf == NULL) {
		return 0;
	}

	// boundaryの取得
	Boundary = (char *)mem_alloc(sizeof(char) * (tstrlen(ContentType) + 1));
	if (Boundary == NULL) {
		return 0;
	}
	if (get_content_value(ContentType, "boundary", Boundary) == FALSE) {
		mem_free(&Boundary);
		return 0;
	}
	if (str_cmp_ni(ContentType, "multipart/digest", tstrlen("multipart/digest")) == 0) {
		is_digest = TRUE;
	}

	// partの位置の取得
	p = get_next_part(buf, Boundary);

	while (1) {
		if (*p == '\0') {
			break;
		}
		p += (2 + tstrlen(Boundary));
		// パーツの終わりかチェック
		if (*p == '-' && *(p + 1) == '-') {
			break;
		}
		
		get_content(p, HEAD_CONTENTTYPE, &Content);
		if (Content != NULL && str_cmp_ni(Content, "multipart", tstrlen("multipart")) == 0) {
			// 階層になっている場合は再帰する
			sPos = GetBodyPointa(p);
			if (sPos == NULL) {
				mem_free(&Content);
				break;
			}
			cnt = multipart_parse(Content, sPos, StopAtTextPart, tpMultiPart, cnt);
			mem_free(&Content);
			p = get_next_part(sPos, Boundary);
			continue;
		}

		// マルチパート情報の追加
		if ((tpMultiPartItem = multipart_add(tpMultiPart, cnt)) == NULL) {
			break;
		}
		cnt++;

		// ヘッダを取得
		tpMultiPartItem->ContentType = Content;
		tpMultiPartItem->IsDigestMsg = is_digest;
		get_content(p, HEAD_ENCODING, &tpMultiPartItem->Encoding);
		get_content(p, HEAD_CONTENTID, &tpMultiPartItem->ContentID);
		get_content(p, HEAD_DISPOSITION, &Content);

		// ファイル名の取得
		if ((tmpname = multipart_get_filename_rfc2231(Content)) == NULL &&
			(tmpname = multipart_get_filename(Content, "filename")) == NULL &&
			(tmpname = multipart_get_filename(tpMultiPartItem->ContentType, "name")) == NULL &&
			tpMultiPartItem->ContentType != NULL && tpMultiPartItem->ContentID != NULL) {
			char *c, *ctype, *ext;
			ctype = alloc_copy(tpMultiPartItem->ContentType);
			for (c = ctype; *c != '\0' && *c != ';'; c++)
				/**/;
			*c = '\0';
			ext = GetMIME2Extension(ctype, NULL);
			c = (char *)mem_alloc(sizeof(char) * (tstrlen(ctype) + tstrlen(tpMultiPartItem->ContentID) + 1));
			if (c != NULL) {
				tmpname = c;
				str_join(c, tpMultiPartItem->ContentID, ext, (char *)-1);
				for ( ; *c != '\0'; c++) {
					if (*c == '\\' || *c == '/' || *c == ':' || *c == '*' || *c == '?' ||
						*c == '\"' || *c == '>' || *c == '<' || *c == '|' ) {
						*c = '_';
					}
				}
			}
			mem_free(&ext);
			mem_free(&ctype);
		}
		mem_free(&Content);

		// handle case of repeated filename
		if (cnt > 1 && tmpname) {
			char *ext, *newname = tmpname;
			int i=0, j=0, offset = strlen(tmpname)-1;
			for (ext = tmpname + offset; offset > 0; ext--, offset--) {
				if (*ext == '.') {
					break;
				}
			}
			if (offset <= 0) { // didn't find a '.'
				offset = strlen(tmpname);
				ext = tmpname + offset;
			}
			while (i < cnt-1) {
				MULTIPART *tpPrevItem = *(*tpMultiPart + i);
				if (tpPrevItem->Filename && strcmp(tpPrevItem->Filename, newname) == 0) {
					j++;
					if (j >= 1000) break;
					if (newname == tmpname) {
						newname = (char *)mem_alloc(sizeof(char)*(strlen(tmpname) + 5)); // _999\n
						str_cpy(newname, tmpname);
					}
					if (newname && newname != tmpname) {
#ifdef UNICODE
						TCHAR dtmp[5];
						char ctmp[5];
						wsprintf(dtmp, TEXT("_%d"), j);
						tchar_to_char(dtmp, ctmp, 5);
						str_join(newname + offset, ctmp, ext, (char *)-1);
#else
						wsprintf(newname + offset, "_%d%s", j, ext);
#endif
					}
					i = -1; // start checking at the beginning
				}
				i++;
			}
			if (newname != tmpname) {
				mem_free(&tmpname);
				tmpname = newname;
			}
		}
		tpMultiPartItem->Filename = tmpname;
		
		// 本文の位置の取得
		tpMultiPartItem->hPos = p;
		tpMultiPartItem->sPos = GetBodyPointa(p);
		if (tpMultiPartItem->sPos == NULL) {
			break;
		}
		// 次のpartの位置を取得
		p = get_next_part(tpMultiPartItem->sPos, Boundary);
		if (*p != '\0') {
			tpMultiPartItem->ePos = p;
			if (tpMultiPartItem->Filename != NULL && *tpMultiPartItem->Filename != '\0') {
				tpMultiPartItem->Forwardable = TRUE;
			}
		}
		if (StopAtTextPart == TRUE && tpMultiPartItem->ContentType != NULL
			&& str_cmp_ni(tpMultiPartItem->ContentType, "text", tstrlen("text")) == 0) {
			break;
		}
	}
	mem_free(&Boundary);
	return cnt;
}

/*
 * multipart_create - マルチパートを作成する (RFC 2046, RFC 2183)
 *                    FwdAttach added by GJC
 */
int multipart_create(TCHAR *Filename, TCHAR *FwdAttach, MAILITEM *tpFwdMailItem, 
					 char *ContentType, char *Encoding, char **RetContentType, 
					 char *body, char **RetBody, int *num_att, char ***EncAtt)
{
#define BREAK_LEN			76
#define CTYPE_RFC822		"Content-Type: message/rfc822\r\n"
#define	CTYPE_MULTIPART		"multipart/mixed;\r\n boundary=\""
#define CONTENT_DIPPOS		"Content-Disposition: attachment;"
#define ENCODING_BASE64		"Content-Transfer-Encoding: base64"
#define ENCODING_QP			"Content-Transfer-Encoding: quoted-printable"
#define CONTENT_TYPE_NAME	";\r\n name=\""
	SYSTEMTIME st;
	TCHAR *fpath, *fname;
	TCHAR *f;
	TCHAR date[15];
	char *Boundary, *ctype;
	char *buf, *ret, *tmp, *prev;
	char *cfname;
	char *p, *ef;
	char *cBuf;
	char *b64str;
	unsigned char digest[16];
	long FileSize;
	int i, len, attnum=0;
	char **tpEncAtt = NULL;
	BOOL have_file, have_fwdatt;
#ifdef UNICODE
	char *cp, *cr, *ftmp;
	TCHAR dtmp[3];
	char ctmp[3];
#endif

	*num_att = 1;
	if (Filename == NULL || *Filename == TEXT('\0')) {
		have_file = FALSE;
	} else {
		have_file = TRUE;
		(*num_att)++;
		for (f = Filename; *f != TEXT('\0'); f++) {
			if (*f == ATTACH_SEP) {
				(*num_att)++;
			}
		}
	}
	if (FwdAttach == NULL || *FwdAttach == TEXT('\0') || tpFwdMailItem == NULL) {
		have_fwdatt = FALSE;
	} else {
		have_fwdatt = TRUE;
		(*num_att)++;
		for (f = FwdAttach+1; *f != TEXT('\0'); f++) {
			if (*f == ATTACH_SEP) {
				(*num_att)++;
			}
		}
	}
	if (have_file == FALSE && have_fwdatt == FALSE) {
		*num_att = 0;
		return MP_NO_ATTACH;
	}
#ifndef WSAASYNC
	if (op.SendAttachIndividually != 0) {
		tpEncAtt = (char **)mem_calloc(sizeof(char *) * (*num_att));
		*EncAtt = tpEncAtt;
	} else
#endif
	{
		*num_att = 0;
	}

	// バウンダリの生成f
	GetLocalTime(&st);
	wsprintf(date, TEXT("%04d%02d%02d%02d%02d%02d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	if (have_file == TRUE && have_fwdatt == TRUE) {
		len = lstrlen(Filename) + lstrlen(FwdAttach);
		fname = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len+1));
		if (fname == NULL) {
#ifndef WSAASYNC
			encatt_free(EncAtt, attnum);
#endif
			return MP_ERROR_ALLOC;
		}
		str_join_t(fname, Filename, FwdAttach, (TCHAR *)-1);
	} else if (have_file == TRUE) {
		fname = Filename;
		len = lstrlen(Filename);
	} else {
		fname = FwdAttach;
		len = lstrlen(FwdAttach);
	}

#ifdef UNICODE
	cp = alloc_tchar_to_char(fname);
	cr = alloc_tchar_to_char(date);
	HMAC_MD5(cp, tstrlen(cp), cr, tstrlen(cr), digest);
	mem_free(&cp);
	mem_free(&cr);
#else
	HMAC_MD5(fname, len, date, lstrlen(date), digest);
#endif
	if (have_file == TRUE && have_fwdatt == TRUE) {
		mem_free(&fname);
	}

	Boundary = (char *)mem_alloc(sizeof(char) * ((16 * 2) + 17 + 1));
	if (Boundary == NULL) {
#ifndef WSAASYNC
		encatt_free(EncAtt, attnum);
#endif
		return MP_ERROR_ALLOC;
	}
	p = str_cpy(Boundary, "-----_MULTIPART_");
	for (i = 0; i < 16; i++) {
#ifdef UNICODE
		wsprintf(dtmp, TEXT("%02X"), digest[i]);
		tchar_to_char(dtmp, ctmp, 3);
		p = str_join(p, ctmp, (char *)-1);
#else
		wsprintf(p, "%02X", digest[i]);
		p += 2;
#endif
	}
	tstrcpy(p, "_");

	// マルチパートの作成
	len = 2 + tstrlen(Boundary) + 2 +
		tstrlen(HEAD_CONTENTTYPE) + 1 + tstrlen(ContentType) + 2 +
		tstrlen(HEAD_ENCODING) + 1 + tstrlen(Encoding) + 4 +
		tstrlen(body) + 4;
	ret = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (ret == NULL) {
#ifndef WSAASYNC
		encatt_free(EncAtt, attnum);
#endif
		mem_free(&Boundary);
		return MP_ERROR_ALLOC;
	}
	str_join(ret, "--", Boundary, "\r\n",
		HEAD_CONTENTTYPE, " ", ContentType, "\r\n",
		HEAD_ENCODING, " ", Encoding, "\r\n\r\n",
		body, "\r\n\r\n", (char *)-1);

	if (Filename != NULL && *Filename != TEXT('\0')) {
		fpath = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(Filename) + 1));
		if (fpath == NULL) {
#ifndef WSAASYNC
			encatt_free(EncAtt, attnum);
#endif
			mem_free(&Boundary);
			mem_free(&ret);
			return MP_ERROR_ALLOC;
		}

		f = Filename;
		while (*f != TEXT('\0')) {
			f = str_cpy_f_t(fpath, f, ATTACH_SEP);
			fname = GetFileNameString(fpath);

			// ファイルを読み込む
			FileSize = file_get_size(fpath);
			if (FileSize < 0 || (cBuf = file_read(fpath, FileSize)) == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&fpath);
				return MP_ERROR_FILE;
			}

			// エンコード
			b64str = (char *)mem_alloc(sizeof(char)*(FileSize * 2 + 4)); // FileSize*4/3 for MIME, plus extra for linebreaks
			if (b64str == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&fpath);
				mem_free(&cBuf);
				return MP_ERROR_ALLOC;
			}
			base64_encode(cBuf, b64str, FileSize, BREAK_LEN);
			mem_free(&cBuf);

			buf = b64str;

			// MIME typeの取得
#ifdef UNICODE
			ftmp = alloc_tchar_to_char(fname);
			ctype = GetMIME2Extension(NULL, ftmp);
			mem_free(&ftmp);
#else
			ctype = GetMIME2Extension(NULL, fname);
#endif
			if (ctype == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&fpath);
				mem_free(&buf);
				return MP_ERROR_ALLOC;
			}

			// ファイル名のエンコード
			p = NULL;
			if (op.EncodeType == 1) {
				// use header encoding, not rfc2231, for filename
				p = MIME_encode(fname, FALSE, op.HeadCharset, op.HeadEncoding, 10); // 10==lstrlen(TEXT(" filename="))
#ifdef UNICODE
				fname = alloc_char_to_tchar(p);
#else
				fname = p;
#endif
			}
			if (fname == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&fpath);
				mem_free(&buf);
				mem_free(&ctype);
				mem_free(&p);
				return MP_ERROR_ALLOC;
			}
			ef = MIME_rfc2231_encode(fname, op.HeadCharset);
			if (ef == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&p);
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&fpath);
				mem_free(&buf);
				mem_free(&ctype);
				return MP_ERROR_ALLOC;
			}

			if (op.EncodeType == 1) {
#ifdef UNICODE
				cfname = alloc_tchar_to_char(fname);
#else
				cfname = fname;
#endif
			}

			// Partの追加
#ifndef WSAASYNC
			if (op.SendAttachIndividually != 0) {
				len = 0;
				prev = NULL;
			} else
#endif
			{
				prev = ret;
			}
			len += (2 + tstrlen(Boundary) + 2 +
				tstrlen(HEAD_CONTENTTYPE) + 1 + tstrlen(ctype) + 2 +
				tstrlen(CONTENT_DIPPOS) + tstrlen(ef) + 2 +
				tstrlen(ENCODING_BASE64) + 4 +
				tstrlen(buf) + 4);
			if (op.EncodeType == 1) {
				len += tstrlen(CONTENT_TYPE_NAME) + tstrlen(cfname) + 1;
			}
			tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
			if (tmp == NULL) {
				mem_free(&p);
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&fpath);
				mem_free(&buf);
				mem_free(&ctype);
				mem_free(&ef);
#ifdef UNICODE
				mem_free(&cfname);
#endif
				return MP_ERROR_ALLOC;
			}
			if (op.EncodeType == 1) {
				str_join(tmp, prev, "--", Boundary, "\r\n",
					HEAD_CONTENTTYPE, " ", ctype, CONTENT_TYPE_NAME, cfname, "\"\r\n",
					CONTENT_DIPPOS, ef, "\r\n",
					ENCODING_BASE64, "\r\n\r\n",
					buf, "\r\n\r\n", (char *)-1);
			} else {
				str_join(tmp, prev, "--", Boundary, "\r\n",
					HEAD_CONTENTTYPE, " ", ctype, "\r\n",
					CONTENT_DIPPOS, ef, "\r\n",
					ENCODING_BASE64, "\r\n\r\n",
					buf, "\r\n\r\n", (char *)-1);
			}

			mem_free(&p);
			mem_free(&ef);
			mem_free(&ctype);
			mem_free(&buf);
#ifdef UNICODE
			mem_free(&cfname);
#endif
#ifndef WSAASYNC
			if (op.SendAttachIndividually != 0) {
				*(tpEncAtt+attnum) = tmp;
				attnum++;
			} else
#endif
			{
				mem_free(&ret);
				ret = tmp;
			}
		}
		mem_free(&fpath);
	}

	// GJC forwarding attachments
	if (FwdAttach != NULL && *FwdAttach != TEXT('\0') && tpFwdMailItem != NULL) {
		if (*FwdAttach == ATTACH_SEP) {
			// forwarding entire message
			int msglen = item_to_string_size(tpFwdMailItem, 2, TRUE, FALSE);
			buf = (char *)mem_alloc(sizeof(char) * (msglen + 1));
			if (buf == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				return MP_ERROR_ALLOC;
			}
			item_to_string(buf, tpFwdMailItem, 2, TRUE, FALSE);

			msglen = QuotedPrintable_encode_length(buf, BODY_ENCODE_LINELEN, TRUE);
			b64str = (char *)mem_alloc(sizeof(char) * msglen);
			if (b64str == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&buf);
				return MP_ERROR_ALLOC;
			}
			QuotedPrintable_encode(buf, b64str, BODY_ENCODE_LINELEN, TRUE);
			mem_free(&buf);
			buf = b64str;
#ifndef WSAASYNC
			if (op.SendAttachIndividually != 0) {
				len = 0;
				prev = NULL;
			} else
#endif
			{
				prev = ret;
			}
			len += (2 + tstrlen(Boundary) + 2 + tstrlen(CTYPE_RFC822) +
				tstrlen(ENCODING_QP) + 4 + msglen + 4);
			tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
			if (tmp == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&buf);
				return MP_ERROR_ALLOC;
			}
			str_join(tmp, prev, "--", Boundary, "\r\n", CTYPE_RFC822,
					ENCODING_QP, "\r\n\r\n", buf, "\r\n\r\n", (char *)-1);
			mem_free(&buf);
#ifndef WSAASYNC
			if (op.SendAttachIndividually != 0) {
				*(tpEncAtt+attnum) = tmp;
				attnum++;
			} else
#endif
			{
				mem_free(&ret);
				ret = tmp;
			}
		} else {
			MULTIPART **tpMultiPart = NULL;
#ifdef UNICODE
			char *ContentType;
#endif
			int cnt;
			int status = MP_ATTACH;

			fpath = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(FwdAttach) + 1));
			if (fpath == NULL) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				return MP_ERROR_ALLOC;
			}

#ifdef UNICODE
			ContentType = alloc_tchar_to_char(tpFwdMailItem->ContentType);
			cnt = multipart_parse(ContentType, tpFwdMailItem->Body, FALSE, &tpMultiPart, 0);
			mem_free(&ContentType);
#else
			cnt = multipart_parse(tpFwdMailItem->ContentType, tpFwdMailItem->Body, FALSE, &tpMultiPart, 0);
#endif

			f = FwdAttach;
			while (*f != TEXT('\0') && status == MP_ATTACH) {
				BOOL found = FALSE;
				f = str_cpy_f_t(fpath, f, ATTACH_SEP);
				tmp = NULL;
				for (i = 0; i < cnt && status == MP_ATTACH; i++) {
#ifdef UNICODE
					fname = alloc_char_to_tchar((*(tpMultiPart + i))->Filename);
#else
					fname = (*(tpMultiPart + i))->Filename;
#endif
					if (fname != NULL && lstrcmp(fname, fpath) == 0) {
						int j, ptlen = ((*(tpMultiPart + i))->ePos - (*(tpMultiPart + i))->hPos);
						found = TRUE;
						// forward only the largest with this name
						for (j = i+1; j < cnt; j++) {
							TCHAR *fn2;
#ifdef UNICODE
							fn2 = alloc_char_to_tchar((*(tpMultiPart + j))->Filename);
#else
							fn2 = (*(tpMultiPart + j))->Filename;
#endif
							if (fn2 != NULL && lstrcmp(fn2, fpath) == 0) {
								int ptlen2 = ((*(tpMultiPart + j))->ePos - (*(tpMultiPart + j))->hPos);
								if (ptlen2 > ptlen) {
									ptlen = ptlen2;
									i = j;
								}
							}
#ifdef UNICODE
							mem_free(&fn2);
#endif
						}
						buf = (char *)mem_alloc(sizeof(char) * (ptlen + 1));
						if (buf == NULL) {
							status = MP_ERROR_ALLOC;
							break;
						}
						str_cpy_n(buf, (*(tpMultiPart + i))->hPos, ptlen + 1);
#ifndef WSAASYNC
						if (op.SendAttachIndividually != 0) {
							len = 0;
							prev = NULL;
						} else
#endif
						{
							prev = ret;
						}
						len += 2 + tstrlen(Boundary) + ptlen + 4;
						tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
						if (tmp == NULL) {
							status = MP_ERROR_ALLOC;
							break;
						}
						str_join(tmp, prev, "--", Boundary, buf, "\r\n\r\n", (char *)-1);
						mem_free(&buf);
#ifndef WSAASYNC
						if (op.SendAttachIndividually != 0) {
							*(tpEncAtt+attnum) = tmp;
							attnum++;
						} else
#endif
						{
							mem_free(&ret);
							ret = tmp;
						}
#ifdef UNICODE
						mem_free(&fname);
#endif
						break;
					}
#ifdef UNICODE
					mem_free(&fname);
#endif
				}
				if (!found) {
					status = MP_ERROR_FILE;
				}
			}
			mem_free(&fpath);
			multipart_free(&tpMultiPart, cnt);
			if (status != MP_ATTACH) {
#ifndef WSAASYNC
				encatt_free(EncAtt, attnum);
#endif
				mem_free(&Boundary);
				mem_free(&ret);
				mem_free(&tmp);
				mem_free(&buf);
				return status;
			}
		}
	}

#ifndef WSAASYNC
	if (op.SendAttachIndividually != 0) {
		len = 0;
		prev = NULL;
	} else
#endif
	{
		prev = ret;
	}
	len += (2 + tstrlen(Boundary) + 2 + 2);
	tmp = (char *)mem_alloc(sizeof(char) * (len + 1));
	if (tmp == NULL) {
#ifndef WSAASYNC
		encatt_free(EncAtt, attnum);
#endif
		mem_free(&Boundary);
		mem_free(&ret);
		return MP_ERROR_ALLOC;
	}
	str_join(tmp, prev, "--", Boundary, "--\r\n", (char *)-1);
#ifndef WSAASYNC
	if (op.SendAttachIndividually != 0) {
		*(tpEncAtt+attnum) = tmp;
		attnum++;
		*RetBody = ret;
	} else
#endif
	{
		mem_free(&ret);
		*RetBody = tmp;
	}

	// Content typeの生成
	*RetContentType = (char *)mem_alloc(
		sizeof(char) * (tstrlen(CTYPE_MULTIPART) + tstrlen(Boundary) + 2));
	if (*RetContentType == NULL) {
#ifndef WSAASYNC
		encatt_free(EncAtt, attnum);
#endif
		mem_free(&Boundary);
		mem_free(&*RetBody);
		return MP_ERROR_ALLOC;
	}
	str_join(*RetContentType, CTYPE_MULTIPART, Boundary, "\"", (char *)-1);
	mem_free(&Boundary);
	return MP_ATTACH;
}

/*
 * convert_cid
 */
char *convert_cid(char *start, char *end, MULTIPART **tpMultiPart, int mpcnt, BOOL open)
{
	//<img width=574 height=155 id="Picture_x0020_1"
	//src="cid:image001.gif@01C8A151.8C0E4EF0" alt=Image>
	// Content-ID: <image001.gif@01C8A151.8C0E4EF0>
	int id, cnt = 0;
#ifdef UNICODE
	unsigned int maxlen = 0;
#else
	int maxlen = 0;
#endif
	char *p, *q, *ret = NULL;
	BOOL add_prefix = (open && op.AttachDelete != 0);

	for (p = start; p < end; p++) {
		if (str_cmp_ni(p, "<img", 4) == 0 && is_white(*(p+4))) {
			p += 4;
			while (p < end && *p != '>') {
				if (str_cmp_ni(p, "src=\"cid:", 9) == 0) {
					cnt++;
					p += 9;
				}
				p++;
			}
		}
	}
	if (cnt == 0) {
		return NULL;
	}
	maxlen = 14; // 8.3\0
	for (id = 0; id < mpcnt; id++) {
		if ((*(tpMultiPart + id))->ContentID != NULL) {
			q = (*(tpMultiPart + id))->Filename;
			if (q != NULL && tstrlen(q) > maxlen) {
				maxlen = tstrlen(q);
			}
		}
	}
	if (add_prefix) {
		maxlen += tstrlen(ATTACH_FILE_A);
	}

	ret = (char *)mem_alloc(sizeof(char) * (tstrlen(start) + cnt*maxlen + 1));
	if (ret == NULL) {
		return NULL;
	}
	p = start;
	q = ret;
	while (p < end) {
		if (str_cmp_ni(p, "<base", 5) == 0) {
			BOOL found = FALSE;
			char *r;
			for (r = p; r < end && *r != '>'; r++) {
				if (found == FALSE && str_cmp_ni(r, "file:", 5) == 0) {
					found = TRUE;
					r += 5;
				}
			}
			r++;
			if (found && r < end) {
				p = r;
			}
		}
		if (str_cmp_ni(p, "<img", 4) == 0 && is_white(*(p+4))) {
			int i;
			for (i = 0; i < 5; i++) {
				*(q++) = *(p++);
			}
			while (p < end && *p != '>') {
				if (str_cmp_ni(p, "src=\"cid:", 9) == 0) {
					char *r, *s;
					for (i = 0; i < 5; i++) {
						*(q++) = *(p++);
					}
					r = s = p + 4;
					while (r < end && *r != '\"') {
						r++;
					}
					if (*r == '\"') {
						int id, len = r - s + 1;
						char *cid = (char *)mem_alloc(sizeof(char) * (len + 2));
						if (cid == NULL) {
							mem_free(&ret);
							return NULL;
						}
						*cid = '<';
						str_cpy_n(cid+1, s, len);
						*(cid + len) = '>';
						*(cid + len + 1) = '\0';
						for (id = 0; id < mpcnt; id++) {
							char *t = (*(tpMultiPart + id))->ContentID;
							if (t != NULL && str_cmp_i(cid, t) == 0) {
								(*(tpMultiPart + id))->EmbeddedImage = TRUE;
								if ((s = (*(tpMultiPart + id))->Filename) != NULL) {
									if (add_prefix) {
										q = str_join(q, ATTACH_FILE_A, s, (char *)-1);
									} else {
										q = str_join(q, s, (char *)-1);
									}
									*q = '\"';
									p = r;
									break;
								} else {
									mem_free(&cid);
									mem_free(&ret);
									return NULL;
								}
							}
						}
						mem_free(&cid);
						if (id >= mpcnt) {
							// didn't find a match
							mem_free(&ret);
							return NULL;
						}
					} else {
						// no closing quote?
						mem_free(&ret);
						return NULL;
					}
				} else {
					*(q++) = *(p++);
				}
			}
		} else {
			*(q++) = *(p++);
		}
	}
	*q = '\0';
	return ret;
}

/* End of source */