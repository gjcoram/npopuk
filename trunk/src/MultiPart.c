/*
 * nPOP
 *
 * Multipart.c
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
#include "code.h"
#include "mime.h"
#include "multipart.h"
extern TCHAR *AppDir;

/* Define */

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
 * multipart_scan - quick check if mail has real attachments (GJC)
 *       OBSOLETE: now, multipart/alternative <==> MULTIPART_HTML
 */
#ifdef DO_MULTIPART_SCAN
int multipart_scan(char *ContentType, char *buf) {
	char *Boundary, *Content = NULL, *p;

	// Content-Type: multipart was found before calling this function,
	// so default return value is MULTIPART_ATTACH
	int retval = MULTIPART_ATTACH;

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
				retval = MULTIPART_ATTACH;
				break;
			}
			get_content(p, HEAD_CONTENTTYPE, &Content);
			if (Content == NULL) {
				// imcomplete message -> assume has attachment
				retval = MULTIPART_ATTACH;
				break;
			}
			if (get_content_value(Content, "name", NULL) == TRUE) {
				retval = MULTIPART_ATTACH;
				break;
			} else if (str_cmp_ni(Content, "text", tstrlen("text")) != 0) {
				retval = MULTIPART_ATTACH;
				break;
			} else {
				char *Dispo;
				get_content(p, HEAD_DISPOSITION, &Dispo);
				if (Dispo != NULL && get_content_value(Dispo, "filename", NULL) == TRUE) {
					retval = MULTIPART_ATTACH;
					break;
				}
				mem_free(&Dispo);
				if (str_cmp_ni(Content, "text/html", tstrlen("text/html")) == 0) {
					retval = MULTIPART_HTML;
				}
			}
			mem_free(&Content);
			p = GetBodyPointa(p);
			if (p == NULL || *p == '\0') {
				retval = MULTIPART_ATTACH;
				break;
			}
			p = get_next_part(p, Boundary);
		}
		mem_free(&Content);
	}
	mem_free(&Boundary);
	return retval;
}
#endif

/*
 * multipart_parse - Partを解析する (RFC 2046)
 */
int multipart_parse(char *ContentType, char *buf, BOOL StopAtTextPart, MULTIPART ***tpMultiPart, int cnt)
{
	MULTIPART *tpMultiPartItem;
	char *Boundary;
	char *p;
	char *Content, *sPos;

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
		if (Content != NULL &&
			str_cmp_ni(Content, "multipart", tstrlen("multipart")) == 0) {
			// 階層になっている場合は再帰する
			sPos = GetBodyPointa(p);
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
		get_content(p, HEAD_ENCODING, &tpMultiPartItem->Encoding);
		get_content(p, HEAD_DISPOSITION, &Content);

		// ファイル名の取得
		if ((tpMultiPartItem->Filename = multipart_get_filename_rfc2231(Content)) == NULL &&
			(tpMultiPartItem->Filename = multipart_get_filename(Content, "filename")) == NULL &&
			(tpMultiPartItem->Filename = multipart_get_filename(tpMultiPartItem->ContentType, "name")) == NULL) {
		}
		mem_free(&Content);

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
			if (tpMultiPartItem->Filename != NULL && *tpMultiPartItem->Filename != '\0' &&
				(tpMultiPartItem->Encoding == NULL || (*tpMultiPartItem->Encoding != '\0' &&
				str_cmp_i(tpMultiPartItem->Encoding, "base64") == 0))) {
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
int multipart_create(TCHAR *Filename, TCHAR *FwdAttach, MAILITEM *tpFwdMailItem, char *ContentType, char *Encoding, char **RetContentType, char *body, char **RetBody, int *num_att, char ***EncAtt)
{
#define BREAK_LEN			76
#define	CTYPE_MULTIPART		"multipart/mixed;\r\n boundary=\""
#define CONTENT_DIPPOS		"Content-Disposition: attachment;"
#define ENCODING_BASE64		"Content-Transfer-Encoding: base64"
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
	char *cp, *cr;
	TCHAR *wtmp;
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
		for (f = FwdAttach; *f != TEXT('\0'); f++) {
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
		wsprintf(fname, TEXT("%s%s"), Filename, FwdAttach);
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
			b64str = (char *)mem_alloc(FileSize * 2 + 4); // FileSize*4/3 for MIME, plus extra for linebreaks
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
			wtmp = GetMIME2Extension(NULL, fname);
			ctype = alloc_tchar_to_char(wtmp);
			mem_free(&wtmp);
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
#ifdef UNICODE
				wtmp = MIME_encode(fname, FALSE, op.HeadCharset, op.HeadEncoding);
				fname = wtmp;
#else
				p = MIME_encode(fname, FALSE, op.HeadCharset, op.HeadEncoding);
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
#ifdef UNICODE
			wtmp = MIME_rfc2231_encode(fname, op.HeadCharset);
			ef = alloc_tchar_to_char(wtmp);
			mem_free(&wtmp);
#else
			ef = MIME_rfc2231_encode(fname, op.HeadCharset);
#endif
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

#ifdef UNICODE
			cfname = alloc_tchar_to_char(fname);
#else
			cfname = fname;
#endif

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
					int ptlen = ((*(tpMultiPart + i))->ePos - (*(tpMultiPart + i))->hPos);
					found = TRUE;
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
		sizeof(TCHAR) * (tstrlen(CTYPE_MULTIPART) + tstrlen(Boundary) + 2));
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
/* End of source */