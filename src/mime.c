/*
 * nPOP
 *
 * mime.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2016 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"
#include "jp.h"
#include "Charset.h"
#include "code.h"
#include "mime.h"
#include "multipart.h"

/* Define */
#define ESC						0x1B

/* Global Variables */
extern OPTION op;
extern int font_charset;

// エンコード情報
typedef struct _ENCODE_INFO {
	TCHAR *buf;
	char *encode_buf;
	int buflen, enclen;
	BOOL encode;

	struct _ENCODE_INFO *next;
} ENCODE_INFO;

/* Local Function Prototypes */
static TCHAR *get_token(TCHAR *p, BOOL *encode, int *enc_len);
static TCHAR *get_token_address(TCHAR *p, BOOL *encode, int *enc_len);
static int get_encode_wrap_len(TCHAR *buf, int len, int *enc_len);
static ENCODE_INFO *encode_info_create(TCHAR *buf, TCHAR *charset, BOOL Address, int prefixlen);
static void encode_info_free(ENCODE_INFO *eb);
static int MIME_encode_size(ENCODE_INFO *eb, TCHAR *charset_t, int encoding);
static int MIME_rfc2231_encode_size(ENCODE_INFO *eb, TCHAR *charset_t);
static TCHAR *MIME_body_decode_charset(char *buf, char *ContentType);

/*
 * is_8bit_char_t - 8bit文字が含まれるかチェック
 */
BOOL is_8bit_char_t(TCHAR *str)
{
#ifdef UNICODE
	return ((*str & 0xFF80) ? TRUE : FALSE);
#else
	return (((unsigned char)*str & (unsigned char)0x80) ? TRUE : FALSE);
#endif
}

/*
 * MIME_charset_encode - 文字コードをcharsetで指定されたコードに変換
 */
char *MIME_charset_encode(const UINT cp, TCHAR *buf, TCHAR *charset)
{
	char *ret = NULL;

	if (lstrcmpi(charset, TEXT(CHARSET_US_ASCII)) == 0) {
		ret = NULL;
	} else if (lstrcmpi(charset, TEXT(CHARSET_ISO_2022_JP)) == 0) {
		// SJIS to ISO-2022-JP
#ifdef UNICODE
		char *cbuf;

		CP_int = 932; // Japanese codepage
		cbuf = alloc_tchar_to_char(buf);
		CP_int = CP_UTF8;
		if (cbuf == NULL) {
			return NULL;
		}
		ret = (char *)mem_alloc(tstrlen(cbuf) * 4 + 1);
		if (ret == NULL) {
			mem_free(&cbuf);
			return NULL;
		}
		sjis_iso2022jp(cbuf, ret);
		mem_free(&cbuf);
#else
		ret = (char *)mem_alloc(tstrlen(buf) * 4 + 1);
		if (ret == NULL) {
			return NULL;
		}
		sjis_iso2022jp(buf, ret);
#endif
	} else {
#ifndef _WCE_OLD
#ifdef UNICODE
		ret = charset_encode(charset, buf, -1);
#else
		WCHAR *wcharset;
		WCHAR *wbuf;

		wcharset = alloc_char_to_wchar(CP_int, charset);
		wbuf = alloc_char_to_wchar(cp, buf);
		ret = charset_encode(wcharset, wbuf, -1);
		mem_free(&wcharset);
		mem_free(&wbuf);
#endif
#else
		// not supported in WinCE2.00
		// ought to pop a warning box, but don't have hWnd in this context
		ret = NULL;
#endif
	}
	return ret;
}

/*
 * MIME_charset_decode - charsetで指定された文字コードを内部コードに変換
 */
TCHAR *MIME_charset_decode(const UINT cp, char *buf, TCHAR *charset)
{
	TCHAR *ret = NULL;

	if (lstrcmpi(charset, TEXT(CHARSET_US_ASCII)) == 0) {
		ret = NULL;
	} else if (lstrcmpi(charset, TEXT(CHARSET_ISO_2022_JP)) == 0) {
		// ISO-2022-JP to SJIS
#ifdef UNICODE
		char *cret = (char *)mem_alloc(tstrlen(buf) + 1);
		if (cret == NULL) {
			return NULL;
		}
		iso2022jp_sjis(buf, cret);
		CP_int = 932; // Japanese codepage
		ret = alloc_char_to_tchar(cret);
		CP_int = CP_UTF8;
		mem_free(&cret);
#else
		ret = (char *)mem_alloc(tstrlen(buf) + 1);
		if (ret == NULL) {
			return NULL;
		}
		iso2022jp_sjis(buf, ret);
#endif
	} else {
#ifndef _WCE_OLD
#ifdef UNICODE
		ret = charset_decode(charset, buf, -1);
#else
		WCHAR *wcharset;
		WCHAR *wret;

		wcharset = alloc_char_to_wchar(CP_int, charset);
		wret = charset_decode(wcharset, buf, -1);
		mem_free(&wcharset);
		ret = alloc_wchar_to_char(cp, wret);
		mem_free(&wret);
#endif
#else
		// not supported in WinCE2.00
		// ought to pop a warning box, but don't have hWnd in this context
		return NULL;
#endif
	}
	return ret;
}

/*
 * get_token - 単語取得
 */
static TCHAR *get_token(TCHAR *p, BOOL *encode, int *enc_len)
{
	*encode = FALSE;
	*enc_len = 0;
	for (; *p != TEXT('\0'); p++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
			(*enc_len) += 6; // overkill?
			*encode = TRUE;
			continue;
		}
#endif
		if ((*p >= TEXT('a') && *p <= TEXT('z'))
			|| (*p >= TEXT('A') && *p <= TEXT('Z'))
			|| (*p >= TEXT('0') && *p <= TEXT('9'))) {
			(*enc_len)++;
		} else if (*p == TEXT(' ')) {
			(*enc_len)++;
			for (; *p == TEXT(' '); p++, (*enc_len)++);
			break;
		} else {
			(*enc_len) += 3;
			if (*encode == FALSE && is_8bit_char_t(p) == TRUE) {
				// エンコードの必要あり
				*encode = TRUE;
			}
		}
	}
	return p;
}

/*
 * get_token_address - メールアドレス用の単語取得
 */
static TCHAR *get_token_address(TCHAR *p, BOOL *encode, int *enc_len)
{
	*encode = FALSE;
	*enc_len = 0;
	switch (*p)
	{
	case TEXT('<'):
	case TEXT('>'):
	case TEXT('('):
	case TEXT(')'):
	case TEXT(','):
		p++;
		(*enc_len) += 3;
		break;

	case TEXT('\"'):
		for (p++; *p != TEXT('\0'); p++) {
#ifdef UNICODE
			if (WCtoMB(CP_int, 0, p, 1, NULL, 0, NULL, NULL) > 1) {
#else
			if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
#endif
				p++;
				(*enc_len) += 6; // overkill?
				*encode = TRUE;
				continue;
			}
			if ((*p >= TEXT('a') && *p <= TEXT('z'))
				|| (*p >= TEXT('A') && *p <= TEXT('Z'))
				|| (*p >= TEXT('0') && *p <= TEXT('9'))) {
				(*enc_len)++;
			} else {
				(*enc_len) += 3;
				if (*encode == FALSE && is_8bit_char_t(p) == TRUE) {
					*encode = TRUE;
				}
				if (*p == TEXT('\\')) {
					p++;
					(*enc_len) += 3;
					continue;
				}
				if (*p == TEXT('\"')) {
					p++;
					(*enc_len) += 3;
					break;
				}
			}
		}
		break;

	default:
		for (; *p != TEXT('\0'); p++) {
#ifdef UNICODE
			if (WCtoMB(CP_int, 0, p, 1, NULL, 0, NULL, NULL) > 1) {
#else
			if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
#endif
				p++;
				(*enc_len) += 6; // overkill?
				*encode = TRUE;
				continue;
			}
			if ((*p >= TEXT('a') && *p <= TEXT('z'))
				|| (*p >= TEXT('A') && *p <= TEXT('Z'))
				|| (*p >= TEXT('0') && *p <= TEXT('9'))) {
				(*enc_len)++;
			} else {
				(*enc_len) += 3;
				if (*encode == FALSE && is_8bit_char_t(p) == TRUE) {
					*encode = TRUE;
				}
				if (*p == TEXT('\\')) {
					p++;
					(*enc_len) += 3;
					continue;
				}
				if (*p == TEXT('<') || *p == TEXT('>') || *p == TEXT('(') || *p == TEXT(')') || *p == TEXT('\"')) {
					break;
				}
			}
		}
		break;
	}
	return p;
}

/*
 * get_encode_wrap_len - エンコード部の折り返し位置取得
 */
static int get_encode_wrap_len(TCHAR *buf, int len, int *enc_len)
{
	TCHAR *p;
	int i = 0, nb = 1;
        *enc_len = 0;

	if ((int)lstrlen(buf) < (int)len) {
		return 0;
	}
	for (p = buf; *p != TEXT('\0'); p++, i++) {
		if (i >= len) {
			return i;
		}
#ifdef UNICODE
		if ( (nb = WCtoMB(CP_int, 0, p, 1, NULL, 0, NULL, NULL)) > 1) {
#else
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
#endif
			p++;
			i += nb;
		}
		if ((*p >= TEXT('a') && *p <= TEXT('z'))
			|| (*p >= TEXT('A') && *p <= TEXT('Z'))
			|| (*p >= TEXT('0') && *p <= TEXT('9'))) {
			(*enc_len)++;
		} else {
			(*enc_len) += 3;
		}
	}
	return 0;
}

/*
 * encode_info_create - エンコード情報の作成
 */
static ENCODE_INFO *encode_info_create(TCHAR *buf, TCHAR *charset, BOOL Address, int prefixlen)
{
	ENCODE_INFO top_eb;
	ENCODE_INFO *eb;
	ENCODE_INFO *tmp_eb;
	TCHAR *p, *r, *t;
	BOOL encode, tmp_encode;
	int clen = lstrlen(charset) + 7; // "=??Q??="
	int len, maxlen = HEAD_ENCODE_LINELEN - prefixlen;
	int elen, tmp_elen;

	top_eb.next = NULL;
	eb = &top_eb;

	// break on spaces (except "in addresses")
	p = buf;
	while (*p != TEXT('\0')) {
		r = p;
		if (Address == TRUE) {
			p = get_token_address(p, &encode, &elen);
		} else {
			p = get_token(p, &encode, &elen);
		}
		if (elen < maxlen) {
			while (*p != TEXT('\0')) {
				// try to add the next token
				if (Address == TRUE) {
					t = get_token_address(p, &tmp_encode, &tmp_elen);
					if (encode != tmp_encode) {
						// From: =?koi8-r?B?IuzJzNEi?= <umw@bluebeatmusic.com>
						break;
					}
				} else {
					t = get_token(p, &tmp_encode, &tmp_elen);
				}
				if (encode || tmp_encode) {
					if ((elen + tmp_elen) > maxlen - clen) {
						break;
					}
				} else {
					if ((t - r) > maxlen) {
						break;
					}
				}
				p = t;
				elen += tmp_elen;
				encode |= tmp_encode;
			}
		}
		len = p - r + 1;

		eb->next = mem_calloc(sizeof(ENCODE_INFO));
		if (eb->next == NULL) {
			encode_info_free(top_eb.next);
			return NULL;
		}
		eb = eb->next;
		eb->encode = encode;
		eb->buflen = len;
		eb->enclen = elen;

		eb->buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 1));
		if (eb->buf == NULL) {
			encode_info_free(top_eb.next);
			return NULL;
		}
		str_cpy_n_t(eb->buf, r, len);
	}

	// split buffers if they're too long (eg first token itself was too long)
	for (eb = top_eb.next; eb != NULL; eb = eb->next) {
		len = elen = 0;
		if (eb->encode == TRUE) {
			if (eb->enclen > HEAD_ENCODE_LINELEN) {
				// get_encode_wrap_len isn't very clever
				len = get_encode_wrap_len(eb->buf, (HEAD_ENCODE_LINELEN/3), &elen);
				if (eb->buflen <= len) {
					len = 0;
				}
			}
		} else {
			if (eb->buflen >= HEAD_LINELEN) {
				len = HEAD_LINELEN;
			}
		}
		if (len != 0) {
			tmp_eb = eb->next;

			eb->next = mem_calloc(sizeof(ENCODE_INFO));
			if (eb->next == NULL) {
				encode_info_free(top_eb.next);
				encode_info_free(tmp_eb);
				return NULL;
			}
			eb->next->encode = eb->encode;
			eb->next->next = tmp_eb;
			eb->next->buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (eb->buflen - len + 1));
			if (eb->next->buf == NULL) {
				encode_info_free(top_eb.next);
				return NULL;
			}
			eb->next->buflen = eb->buflen - len;
			eb->next->enclen = eb->enclen - elen;
			lstrcpy(eb->next->buf, eb->buf + len);

			p = (TCHAR *)mem_alloc(sizeof(TCHAR) * (len + 2));
			if (p == NULL) {
				encode_info_free(top_eb.next);
				return NULL;
			}
			str_cpy_n_t(p, eb->buf, len + 1);
			mem_free(&eb->buf);
			eb->buf = p;
			eb->buflen = len + 1;
			eb->enclen = elen + 1;
		}
	}

	// encode each piece
	for (eb = top_eb.next; eb != NULL; eb = eb->next) {
		if (eb->encode == TRUE && (eb->encode_buf = MIME_charset_encode(CP_int, eb->buf, charset)) != NULL) {
			mem_free(&eb->buf);
		} else {
#ifdef UNICODE
			eb->encode_buf = alloc_tchar_to_char(eb->buf);
			mem_free(&eb->buf);
#else
			eb->encode_buf = eb->buf;
#endif
		}
		eb->buf = NULL;
	}
	return top_eb.next;
}

/*
 * encode_info_free - エンコード情報の解放
 */
static void encode_info_free(ENCODE_INFO *eb)
{
	if (eb == NULL) {
		return;
	}
	encode_info_free(eb->next);

	mem_free(&eb->buf);
	mem_free(&eb->encode_buf);
	mem_free(&eb);
}

/*
 * MIME_encode_size - MIMEエンコードしたサイズの取得
 */
static int MIME_encode_size(ENCODE_INFO *eb, TCHAR *charset_t, int encoding)
{
	int i;
	int len = 0;

	for (; eb != NULL; eb = eb->next) {
		if (eb->encode_buf == NULL) {
			continue;
		}
		len += 3;			// "\r\n "

		if (eb->encode == FALSE || encoding == ENC_TYPE_7BIT || encoding == ENC_TYPE_8BIT) {
			len += tstrlen(eb->encode_buf);
		} else {
			i = tstrlen(eb->encode_buf);
			switch (encoding)
			{
			case ENC_TYPE_BASE64:
			default:
				// Base64
				i = i * 2 + 4;
				break;

			case ENC_TYPE_Q_PRINT:
				// quoted-printable
				i = i * 3;
				break;
			}
			len += 2;		// =?
			len += lstrlen(charset_t);
			len++;			// ?
			len++;			// B or Q
			len++;			// ?
			len += i;
			len += 2;		// ?=
		}
	}
	return len;
}

/*
 * MIME_encode - MIMEエンコード (RFC 2047, RFC 2045)
 */
char *MIME_encode(TCHAR *wbuf, BOOL Address, TCHAR *charset_t, int encoding, int headerlen)
{
	ENCODE_INFO *top_eb, *eb;
	char *ret;
	char *EncType;
	char *charset;
	char *tmp;
	char *r;
	int i;

#ifdef UNICODE
	// char に変換
	charset = alloc_tchar_to_char(charset_t);
	if (charset == NULL) {
		return NULL;
	}
#else
	charset = charset_t;
#endif

	// エンコード情報の作成
	top_eb = eb = encode_info_create(wbuf, charset_t, Address, headerlen);
	if (top_eb == NULL) {
#ifdef UNICODE
		mem_free(&charset);
#endif
		return NULL;
	}

	// エンコード後のバッファ確保
	i = MIME_encode_size(eb, charset_t, encoding);
	ret = (char *)mem_alloc(i + 1);
	if (ret == NULL) {
		encode_info_free(top_eb);
#ifdef UNICODE
		mem_free(&charset);
#endif
		return NULL;
	}
	r = ret;

	for (; eb != NULL; eb = eb->next) {
		if (eb->encode_buf == NULL) {
			continue;
		}
		if (top_eb != eb) {
			r = str_cpy(r, "\r\n ");
		}
		if (eb->encode == FALSE || encoding == ENC_TYPE_7BIT || encoding == ENC_TYPE_8BIT) {
			r = str_cpy(r, eb->encode_buf);
		} else {
			switch (encoding)
			{
			case ENC_TYPE_BASE64:
			default:
				// Base64
				tmp = (char *)mem_alloc(tstrlen(eb->encode_buf) * 2 + 4 + 1);
				if (tmp == NULL) {
					encode_info_free(top_eb);
					mem_free(&ret);
#ifdef UNICODE
					mem_free(&charset);
#endif
					return NULL;
				}
				base64_encode(eb->encode_buf, tmp, 0, 0);
				mem_free(&eb->encode_buf);
				eb->encode_buf = tmp;
				EncType = "?B?";
				break;

			case ENC_TYPE_Q_PRINT:
				// quoted-printable
				tmp = (char *)mem_alloc(tstrlen(eb->encode_buf) * 3 + 1);
				if (tmp == NULL) {
					encode_info_free(top_eb);
					mem_free(&ret);
#ifdef UNICODE
					mem_free(&charset);
#endif
					return NULL;
				}
				QuotedPrintable_encode(eb->encode_buf, tmp, 0, FALSE);
				mem_free(&eb->encode_buf);
				eb->encode_buf = tmp;
				EncType = "?Q?";
				break;
			}
			r = str_cpy(r, "=?");
			r = str_cpy(r, charset);
			r = str_cpy(r, EncType);
			r = str_cpy(r, eb->encode_buf);
			r = str_cpy(r, "?=");
		}
	}
	*r = '\0';

	encode_info_free(top_eb);
#ifdef UNICODE
	mem_free(&charset);
#endif

	return ret;
}

/*
 * MIME_encode_opt - encode if needed
 */
char *MIME_encode_opt(TCHAR *wbuf, BOOL Address, TCHAR *charset_t, int encoding, int headerlen)
{
	char *ret = NULL;
	TCHAR *p;

	for (p = wbuf; *p != TEXT('\0'); p++) {
		if (is_8bit_char_t(p) == TRUE) {
			break;
		}
		// if IsDBCSLeadByte((BYTE)*p) then it must be 8bit
	}

	if (*p == TEXT('\0')) {
		// got to the end of wbuf without finding an 8-bit char, no encoding necessary
		ret = alloc_tchar_to_char(wbuf);
	} else {
		ret = MIME_encode(wbuf, Address, charset_t, encoding, headerlen);
	}
	return ret;
}

/*
 * MIME_decode - MIMEのデコード (RFC 2047, RFC 2045)
 *
 * =?charset?B?content?=
 * =?charset?Q?content?=
 */
int MIME_decode(char *buf, TCHAR *ret)
{
	char *p, *t;
	TCHAR *r, *s;
	char *non_enc_pt;
	char *enc_st;
	char *charset_st, *charset_en;
	TCHAR *charset;
	char *encoding;
	char *content;
	char *dec_buf;
	TCHAR *retbuf;
	BOOL jis_flag = FALSE;
	BOOL enc_flag = FALSE;
	BOOL enc_err = FALSE;
	BOOL non_ascii = FALSE;
	int ret_len = 0;

	p = buf;
	r = ret;
	non_enc_pt = p;
	while (*p != '\0') {
		if (*p != '=' || *(p + 1) != '?' || enc_err == TRUE) {
			enc_err = FALSE;
			if (jis_flag == FALSE &&
				*p == ESC && *(p + 1) == '$' && (*(p + 2) == '@' || *(p + 2) == 'B')) {
				// JISコードが含まれている
				jis_flag = TRUE;
			}
			if (enc_flag == TRUE &&
				*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
				enc_flag = FALSE;
			}
			if (*p < 0) {
				non_ascii = TRUE;
			}
			p++;
			continue;
		}
		// if we got here, we must have hit =?
		if (enc_flag == FALSE && p != non_enc_pt) {
			if (ret != NULL) {
				// convert any non-encoded portion that preceeds the =?
#ifdef UNICODE
				if (non_ascii == TRUE && op.ViewCharset != NULL && *op.ViewCharset != TEXT('\0')) {
					// found a character not in the 0-127 range, convert it according to ViewCharset
					char hold = *p;
					*p = '\0'; // stop decoding here
					retbuf = MIME_charset_decode(CP_int, non_enc_pt, op.ViewCharset);
					// #ifdef UNICODE, charset_to_cp always returns CP_int
					// retbuf = MIME_charset_decode(charset_to_cp((BYTE)font_charset), non_enc_pt, op.ViewCharset);
					*p = hold;
					if (retbuf != NULL) {
						str_cpy_n_t(r, retbuf, (p - non_enc_pt) + 1);
						mem_free(&retbuf);
					}
					non_ascii = FALSE;
				} else {
					MBtoWC(CP_int, 0, non_enc_pt, (p - non_enc_pt), r, (p - non_enc_pt));
				}
#else
				str_cpy_n_t(r, non_enc_pt, (p - non_enc_pt) + 1);
#endif
				r += p - non_enc_pt;
			}
			ret_len += p - non_enc_pt;
			non_enc_pt = p;
		}
		enc_st = p;
		p += 2;

		// 文字セット
		charset_st = p;
		for (; *p != '\0' && *p != '?'; p++);
		if (*p == '\0') {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		charset_en = p;
		p++;
		// エンコード方法
		encoding = p;
		for (; *p != '\0' && *p != '?'; p++);
		if (*p == '\0') {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		p++;
		// エンコード終了位置
		for (t = p; *t != '\0' && !(*t == '?' && *(t + 1) == '='); t++);
		if (*t == '\0') {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		// エンコード部分のコピー
		content = (char *)mem_alloc(sizeof(char) * (t - p + 1));
		if (content == NULL) {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
		str_cpy_n(content, p, (t - p) + 1);
		// デコード
		switch (*encoding) {
		case 'b': case 'B':
			dec_buf = (char *)mem_alloc(sizeof(char) * (t - p + 1));
			if (dec_buf == NULL) {
				p = enc_st;
				enc_err = TRUE;
				continue;
			}
			base64_decode(content, dec_buf, FALSE);
			mem_free(&content);
			break;
		case 'q': case 'Q':
			dec_buf = (char *)mem_alloc(sizeof(char) * (t - p + 1));
			if (dec_buf == NULL) {
				p = enc_st;
				enc_err = TRUE;
				continue;
			}
			Q_decode(content, dec_buf);
			mem_free(&content);
			break;
		default:
			dec_buf = content;
			break;
		}
		p = t + 2;

		// 文字セットのコピー
		charset = mem_alloc(sizeof(TCHAR) * ((charset_en - charset_st) + 1));
		if (charset == NULL) {
			p = enc_st;
			enc_err = TRUE;
			continue;
		}
#ifdef UNICODE
		MBtoWC(CP_int, 0, charset_st, (charset_en - charset_st), charset, (charset_en - charset_st));
		*(charset + (charset_en - charset_st)) = TEXT('\0');
#else
		str_cpy_n_t(charset, charset_st, (charset_en - charset_st) + 1);
#endif
		// 文字コードの変換
		retbuf = MIME_charset_decode(CP_int, dec_buf, charset);
		if (retbuf != NULL) {
			mem_free(&dec_buf);
		} else {
#ifdef UNICODE
			retbuf = alloc_char_to_tchar(dec_buf);
			mem_free(&dec_buf);
#else
			retbuf = dec_buf;
#endif
		}
		mem_free(&charset);

		// 文字列のコピー
		for (s = retbuf; *s != TEXT('\0'); s++) {
#ifndef UNICODE
			if (IsDBCSLeadByte((BYTE)*s) == TRUE && *(s + 1) != TEXT('\0')) {
				if (ret != NULL) {
					*(r++) = *(s++);
					*(r++) = *s;
				}
				ret_len += 2;
				continue;
			}
#endif
			if (*s != TEXT('\r') && *s != TEXT('\n')) {
				if (ret != NULL) {
					*(r++) = *s;
				}
				ret_len++;
			}
		}
		mem_free(&retbuf);

		enc_flag = TRUE;
		non_enc_pt = p;
	}
	if (enc_flag == FALSE && p != non_enc_pt) {
		if (ret != NULL) {
			// コピー
#ifdef UNICODE
			if (non_ascii == TRUE && op.ViewCharset != NULL && *op.ViewCharset != TEXT('\0')) {
				// found a character not in the 0-127 range, convert it according to ViewCharset
				char hold = *p;
				*p = '\0'; // stop decoding here
				retbuf = MIME_charset_decode(CP_int, non_enc_pt, op.ViewCharset);
				// #ifdef UNICODE, charset_to_cp always returns CP_int
				// retbuf = MIME_charset_decode(charset_to_cp((BYTE)font_charset), non_enc_pt, op.ViewCharset);
				*p = hold;
				if (retbuf != NULL) {
					str_cpy_n_t(r, retbuf, (p - non_enc_pt) + 1);
					mem_free(&retbuf);
				}
			} else {
				MBtoWC(CP_int, 0, non_enc_pt, (p - non_enc_pt), r, (p - non_enc_pt));
			}
#else
			str_cpy_n_t(r, non_enc_pt, (p - non_enc_pt) + 1);
#endif
			r += p - non_enc_pt;
		}
		ret_len += p - non_enc_pt;
	}
	if (ret == NULL) {
		return ret_len;
	}
	*r = TEXT('\0');

	if (jis_flag == TRUE) {
		// JISの場合はJIS->SJISを行う
#ifdef UNICODE
		char *cret, *cbuf;
		int len;

		cbuf = alloc_tchar_to_char(ret);
		len = tstrlen(cbuf);
		if (cbuf != NULL) {
			cret = mem_alloc(len + 1);
			if (cret != NULL) {
				iso2022jp_sjis(cbuf, cret);
				CP_int = 932; // Japanese codepage
				char_to_tchar(cret, ret, tstrlen(cret));
				CP_int = CP_UTF8;
				mem_free(&cret);
			}
			mem_free(&cbuf);
		}
#else
		char *tmp;
		tmp = (char *)mem_alloc(tstrlen(ret) + 1);
		if (tmp != NULL) {
			iso2022jp_sjis(ret, tmp);
			tstrcpy(ret, tmp);
			mem_free(&tmp);
		}
#endif
	}
	return ret_len;
}

/*
 * MIME_rfc2231_encode_size - ファイル名のエンコードのサイズ (RFC 2231)
 */
static int MIME_rfc2231_encode_size(ENCODE_INFO *eb, TCHAR *charset_t)
{
	int len = 0;

	for (; eb != NULL; eb = eb->next) {
		len += tstrlen(eb->encode_buf) * 3; // overkill, not all chars will need encoding
		len += (lstrlen(TEXT("\r\n filename**=;")) + 5);
		if (eb->next != NULL) {
			len++;			// ;
		}
	}
	len += lstrlen(charset_t) + 2;
	return len;
}

/*
 * MIME_rfc2231_encode - ファイル名のエンコード (RFC 2231)
 */
char *MIME_rfc2231_encode(TCHAR *wbuf, TCHAR *charset_t)
{
	ENCODE_INFO *top_eb, *eb;
	char *ret, *tmp, *r, *charset;
	TCHAR *p, *t;
	int cnt = 0;
	int i;

	for (p = wbuf; *p != TEXT('\0'); p++) {
		if (is_8bit_char_t(p) == TRUE) {
			break;
		}
		// if IsDBCSLeadByte((BYTE)*p) then it must be 8bit
	}

	if (*p == TEXT('\0')) {
		// got to the end of wbuf without finding an 8-bit char, no encoding necessary
#ifdef UNICODE
		tmp = alloc_tchar_to_char(wbuf);
		if (tmp == NULL) {
			return NULL;
		}
#else
		tmp = wbuf;
#endif
		i = (tstrlen("\r\n filename=\"\"") + tstrlen(tmp));
		ret = (char *)mem_alloc(sizeof(char) * (i + 1));
		if (ret != NULL) {
			str_join(ret, "\r\n filename=\"", tmp, "\"", (char *)-1);
		}
#ifdef UNICODE
		mem_free(&tmp);
#endif
		return ret;
	}

	// break filename into manageable chunks
	top_eb = eb = encode_info_create(wbuf, charset_t, -1, lstrlen(TEXT(" filename*0*='';")));
	if (top_eb == NULL) {
		return NULL;
	}

	// compute encoded string length
	i = MIME_rfc2231_encode_size(eb, charset_t);
	ret = (char *)mem_alloc(sizeof(char) * (i + 1));
	if (ret == NULL) {
		encode_info_free(top_eb);
		return NULL;
	}
	r = ret;
	charset = alloc_tchar_to_char(charset_t);

	for (; eb != NULL; eb = eb->next) {
		// URLエンコード
		tmp = (char *)mem_alloc(tstrlen(eb->encode_buf) * 3 + 1);
		if (tmp == NULL) {
			encode_info_free(top_eb);
			mem_free(&charset);
			return NULL;
		}
		URL_encode(eb->encode_buf, tmp, TRUE);
		mem_free(&eb->encode_buf);
		eb->encode_buf = tmp;

#ifdef UNICODE
		// TCHAR に変換
		t = alloc_char_to_tchar(eb->encode_buf);
		if (t == NULL) {
			encode_info_free(top_eb);
			mem_free(&charset);
			return NULL;
		}
#else
		t = eb->encode_buf;
#endif
		if (cnt == 0) {
			r = str_join(r, "\r\n filename*0*=", charset, "''", t, (char *)-1);
		} else {
#ifdef _WCE_OLD
			TCHAR cnt_str_t[12];
			char *cnt_str;
			wsprintf(cnt_str_t, TEXT("%d"), cnt);
			cnt_str = alloc_tchar_to_char(cnt_str_t);
			r = str_join(r, "\r\n filename*", cnt_str, "*=", t, (char *)-1);
			mem_free(&cnt_str);
#else
			char cnt_str[12];
			sprintf_s(cnt_str, 9, "%d", cnt, "\0");
			r = str_join(r, "\r\n filename*", cnt_str, "*=", t, (char *)-1);
#endif
		}
#ifdef UNICODE
		mem_free(&t);
#endif
		cnt++;

		if (eb->next != NULL) {
			*(r++) = ';';
		}
	}
	encode_info_free(top_eb);
	mem_free(&charset);
	return ret;
}

/*
 * MIME_rfc2231_decode - ファイル名のデコード (RFC 2231)
 */
char *MIME_rfc2231_decode(char *buf)
{
#ifdef UNICODE
	TCHAR *wcharset;
	TCHAR *wret;
#endif
	char *charset;
	char *lang = NULL;
	char *r;
	char *ret;
	char *tmp;

	charset = (char *)mem_alloc(sizeof(char) * (tstrlen(buf) + 1));
	if (charset == NULL) {
		return NULL;
	}
	r = str_cpy_f(charset, buf, '\'');
	if (*r != '\0') {
		lang = (char *)mem_alloc(sizeof(char) * (tstrlen(r) + 1));
		if (lang == NULL) {
			mem_free(&charset);
			return NULL;
		}
		r = str_cpy_f(lang, r, '\'');
	}
	if (*r == '\0') {
		r = buf;
		*charset = '\0';
	}

	// URLデコード
	tmp = (char *)mem_alloc(tstrlen(r) + 1);
	if (tmp == NULL) {
		mem_free(&charset);
		mem_free(&lang);
		return NULL;
	}
	URL_decode(r, tmp);

	// キャラクタセットの変換
#ifdef UNICODE
	wcharset = alloc_char_to_tchar(charset);
	if (wcharset == NULL) {
		mem_free(&charset);
		mem_free(&lang);
		return NULL;
	}
	wret = MIME_charset_decode(CP_int, tmp, wcharset);
	mem_free(&wcharset);
	ret = alloc_tchar_to_char(wret);
	mem_free(&wret);
#else
	ret = MIME_charset_decode(CP_int, tmp, charset);
#endif
	if (ret != NULL) {
		mem_free(&tmp);
	} else {
		ret = tmp;
	}
	mem_free(&charset);
	mem_free(&lang);
	return ret;
}

/*
 * MIME_create_encode_header - エンコードヘッダ作成
 */
BOOL MIME_create_encode_header(TCHAR *charset, int encoding, TCHAR *ctype_in, char *ret_content_type, char *ret_encoding)
{
	TCHAR *content_type;
#ifdef UNICODE
	TCHAR buf[BUF_SIZE];
#endif
	
	if (ctype_in != NULL && *ctype_in != TEXT('\0')) {
		content_type = ctype_in;
	} else {
		content_type = TEXT("text/plain");
	}

#ifdef UNICODE
	// Content-Type
	wsprintf(buf, TEXT("%s; charset=\"%s\""), content_type, charset);
	tchar_to_char(buf, ret_content_type, BUF_SIZE);
#else
	// Content-Type
	wsprintf(ret_content_type, TEXT("%s; charset=\"%s\""), content_type, charset);
#endif

	// Content-Transfer-Encoding
	switch (encoding) {
	case ENC_TYPE_7BIT:
	default:
		tstrcpy(ret_encoding, ENCODE_7BIT);
		break;

	case ENC_TYPE_8BIT:
		tstrcpy(ret_encoding, ENCODE_8BIT);
		break;

	case ENC_TYPE_BASE64:
		// Base64
		tstrcpy(ret_encoding, ENCODE_BASE64);
		break;

	case ENC_TYPE_Q_PRINT:
		// quoted-printable
		tstrcpy(ret_encoding, ENCODE_Q_PRINT);
		break;
	}
	return TRUE;
}

/*
 * MIME_body_encode - 本文のエンコード (RFC 822, RFC 2822, RFC 2045)
 */
char *MIME_body_encode(TCHAR *body, TCHAR *charset_t, int encoding, TCHAR *ctype_in, char *ret_content_type, char *ret_encoding, TCHAR *ErrStr)
{
	TCHAR *buf;
	char *ret;
	TCHAR *p;
	char *cret;
	char *tmp;
	char *cp, *cr;
	int i;
	BOOL encode = FALSE;

	buf = (body != NULL) ? body : TEXT("");
	for (p = buf, i = 0; *p != TEXT('\0'); p++, i++) {
		if (is_8bit_char_t(p) == TRUE || i > 1000) {
			encode = TRUE;
			break;
		}
#ifndef UNICODE
		// need to check this, in case the trail byte is \r or \n
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
		}
#endif
		if (*p == TEXT('\r') && *(p+1) == TEXT('\n')) {
			i = 0;
			p++;
		}
	}

	if (encode == FALSE) {
		// Assume US-ASCII (could be national variant)
		MIME_create_encode_header(TEXT(CHARSET_US_ASCII), 0, ctype_in, ret_content_type, ret_encoding);
		ret = alloc_tchar_to_char(buf);
	} else {
		if (encoding == ENC_TYPE_7BIT) {
			// can't send 8bit data with 7bit encoding (unless you convert � to ue or "u ?)
			encoding = ENC_TYPE_Q_PRINT;
		}
		//if (lstrcmpi(charset_t, TEXT(CHARSET_US_ASCII)) == 0) {
			// that's a lie if there's 8-bit data ...
		//}
		MIME_create_encode_header(charset_t, encoding, ctype_in, ret_content_type, ret_encoding);

		// charsetの変換
#ifdef UNICODE
		cret = MIME_charset_encode(CP_int, buf, charset_t);
		if (cret == NULL) {
			cret = alloc_tchar_to_char(buf);
		}
#else
		cret = MIME_charset_encode(charset_to_cp((BYTE)font_charset), buf, charset_t);
		if (cret == NULL) {
			cret = alloc_copy_t(buf);
		}
#endif
		switch (encoding) {
		case ENC_TYPE_7BIT:
		case ENC_TYPE_8BIT:
		default:
			break;

		case ENC_TYPE_BASE64:
			// Base64でエンコード
			tmp = (char *)mem_alloc(tstrlen(cret) * 2 + 4 + 1);
			if (tmp == NULL) {
				mem_free(&cret);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			base64_encode(cret, tmp, 0, 0);
			mem_free(&cret);

			// 折り返し
			cret = (char *)mem_alloc(tstrlen(tmp) + (tstrlen(tmp) / BODY_ENCODE_LINELEN * 2) + 1);
			if (cret == NULL) {
				mem_free(&tmp);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			for (cp = tmp, cr = cret, i = 0; *cp != '\0'; cp++, i++) {
				if (i >= BODY_ENCODE_LINELEN) {
					i = 0;
					*(cr++) = '\r';
					*(cr++) = '\n';
				}
				*(cr++) = *cp;
			}
			*cr = '\0';
			mem_free(&tmp);
			break;

		case ENC_TYPE_Q_PRINT:
			// quoted-printable
			i = QuotedPrintable_encode_length(cret, BODY_ENCODE_LINELEN, TRUE);
			tmp = (char *)mem_alloc(sizeof(char) * i);
			if (tmp == NULL) {
				mem_free(&cret);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			QuotedPrintable_encode(cret, tmp, BODY_ENCODE_LINELEN, TRUE);
			mem_free(&cret);
			cret = tmp;
			break;
		}
		ret = cret;
	}
	return ret;
}

/*
 * MIME_body_decode_charset - ContentTypeで指定された文字コードを内部コードに変換
 */
static TCHAR *MIME_body_decode_charset(char *buf, char *ContentType)
{
	TCHAR *charset;
	TCHAR *p;
	char *r;
	TCHAR *ret = NULL;

	r = ContentType;
	while (r != NULL && *r != '\0') {
		for (; *r == ' '; r++);
		if (str_cmp_ni(r, "charset", tstrlen("charset")) != 0) {
			for (; *r != '\0' && *r != ';'; r++);
			if (*r != '\0') {
				r++;
			}
			continue;
		}
		r += tstrlen("charset");
		for (; *r == ' '; r++);
		if (*r != '=') {
			break;
		}
		r++;
		for (; *r == ' '; r++);
		if (*r == '\"') {
			r++;
		}
		for (; *r == TEXT(' '); r++);

		charset = alloc_char_to_tchar(r);
		if (charset == NULL) {
			return NULL;
		}
		for (p = charset; *p != TEXT('\0') && *p != TEXT('\"') && *p != TEXT(';'); p++);
		*p = TEXT('\0');

#ifdef UNICODE
		ret = MIME_charset_decode(CP_int, buf, charset);
#else
		ret = MIME_charset_decode(charset_to_cp((BYTE)font_charset), buf, charset);
#endif
		if (ret != NULL) {
			mem_free(&charset);
			return ret;
		}
		mem_free(&charset);
		break;
	}
	if (lstrcmpi(STR_DEFAULT_BODY_CHARSET, TEXT(CHARSET_ISO_2022_JP)) == 0) {
#ifdef UNICODE
		char *cret;

		cret = mem_alloc(tstrlen(buf) + 1);
		if (cret != NULL) {
			iso2022jp_sjis(buf, cret);
		}
		CP_int = 932; // Japanese codepage
		ret = alloc_char_to_tchar(cret);
		CP_int = CP_UTF8;
		mem_free(&cret);
#else
		ret = mem_alloc(tstrlen(buf) + 1);
		if (ret != NULL) {
			iso2022jp_sjis(buf, ret);
		}
#endif
	} else {
		ret = alloc_char_to_tchar_check(buf);
	}
	return ret;
}

/*
 * MIME_body_decode_transfer - Content-Transfer-Encoding にしたがってデコード
 */
char *MIME_body_decode_transfer(MAILITEM *tpMailItem, char *body)
{
	char *ret;
	BOOL check_slash_n = TRUE;

	// body is a pointer into the full wire-form buffer, need to take a copy to modify
	ret = alloc_copy(body);
	if (ret == NULL) {
		return NULL;
	}

	if (tpMailItem->ContentType != NULL &&
		str_cmp_ni_t(tpMailItem->ContentType, TEXT("text"), lstrlen(TEXT("text"))) != 0) {
		// don't try to decode non-text
		return ret;
	} else if (tpMailItem->Encoding != NULL) {
		// decoding always takes fewer characters, so do it in-place
		if (str_cmp_ni_t(tpMailItem->Encoding, TEXT(ENCODE_BASE64), lstrlen(TEXT(ENCODE_BASE64))) == 0) {
			base64_decode(ret, ret, TRUE);
			check_slash_n = FALSE; // handled in base64_decode
		} else if (str_cmp_ni_t(tpMailItem->Encoding, TEXT(ENCODE_Q_PRINT), lstrlen(TEXT(ENCODE_Q_PRINT))) == 0) {
			QuotedPrintable_decode(ret, ret, TRUE);
			check_slash_n = FALSE; // handled in QP_decode
		}
		// else encoding is assumed 7bit or 8bit
	}
	if (check_slash_n == TRUE) {
		FixCRLF(&ret);
	}
	// now saving wire-form; still need the Encoding
	// mem_free(&tpMailItem->Encoding);
	// tpMailItem->Encoding = NULL;
	return ret;
}

/*
 * MIME_body_decode - 本文のデコード (RFC 822, RFC 2822, RFC 2045)
 */
TCHAR *MIME_body_decode(MAILITEM *tpMailItem, BOOL ViewSrc, BOOL StopAtTextPart, MULTIPART ***tpPart, int *cnt, int *TextIndex)
{
#ifdef UNICODE
	char *ContentType;
#endif
	char *body = NULL;
	TCHAR *buf;
	TCHAR *r;
	TCHAR *wenc_ret = NULL;
	char *enc_ret = NULL;
	int i;
	*TextIndex = -1;

	*cnt = 0;
	if (ViewSrc == TRUE) {
		char *head;
		body = tpMailItem->Body;
#ifdef UNICODE
		ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
		buf = MIME_body_decode_charset(body, ContentType);
		mem_free(&ContentType);
#else
		buf = MIME_body_decode_charset(body, tpMailItem->ContentType);
#endif
		if (buf == NULL) {
#ifdef UNICODE
			buf = alloc_char_to_tchar(body);
#else
			buf = body;
#endif
		}

		// put full headers in for view source
		// this is goofy, because item_to_string converts everything to char,
		// and we really want TCHAR here
		i = item_to_string_size(tpMailItem, 2, FALSE, FALSE);
		head = (char *)mem_alloc(sizeof(char) * (i + 1));
		if (head != NULL) {
			*head = '\0';
			item_to_string(head, tpMailItem, 2, FALSE, FALSE);
#ifdef UNICODE
			wenc_ret = alloc_char_to_tchar(head);
			mem_free(&head);
#else
			wenc_ret = head;
#endif
		}

		if (wenc_ret != NULL && *wenc_ret!= TEXT('\0')) {
			if (buf != NULL && *buf != TEXT('\0')) {
				i = lstrlen(wenc_ret) + lstrlen(buf) + 3;
				r = (TCHAR *)mem_alloc(sizeof(TCHAR) * i);
				str_join_t(r, wenc_ret, TEXT("\r\n"), buf, (TCHAR *)-1);
				mem_free(&wenc_ret);
				mem_free(&buf);
				buf = r;
			} else {
				buf = wenc_ret;
			}
		}

		if (buf != NULL && *buf != TEXT('\0')) {
			// DelDot(buf, buf);
			return buf;
		}
	}
	if (tpMailItem->Multipart > MULTIPART_ATTACH && ViewSrc == FALSE) {
		// マルチパートを解析する
#ifdef UNICODE
		ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
		*cnt = multipart_parse(ContentType, tpMailItem->Body, StopAtTextPart, tpPart, 0);
		mem_free(&ContentType);
#else
		*cnt = multipart_parse(tpMailItem->ContentType, tpMailItem->Body, StopAtTextPart, tpPart, 0);
#endif
	}

	if (*cnt == 0 && multipart_add(tpPart, 0) != NULL) {
		// not actually a multipart message
		char *p;
		if (ViewSrc == FALSE) {
			(**tpPart)->ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
			(**tpPart)->Encoding = alloc_tchar_to_char(tpMailItem->Encoding);
		}
		if (tpMailItem->WireForm != NULL) {
			(**tpPart)->sPos = (**tpPart)->hPos = tpMailItem->WireForm;
			p = GetBodyPointa((**tpPart)->sPos);
			if (p != NULL) {
				(**tpPart)->sPos = p;
			}
		} else {
			(**tpPart)->sPos = (**tpPart)->hPos = tpMailItem->Body;
			if (tpMailItem->HasHeader && tpMailItem->Multipart <= MULTIPART_ATTACH && (**tpPart)->sPos != NULL) {
				p = GetBodyPointa((**tpPart)->sPos);
				if (p != NULL) {
					(**tpPart)->sPos = p;
				}
			}
		}
		if (ViewSrc == TRUE || (**tpPart)->ContentType == NULL ||
			str_cmp_ni((**tpPart)->ContentType, "text", tstrlen("text")) == 0) {
			// テキスト
			*TextIndex = 0;
		} else {
			// ファイル名の取得
#ifdef UNICODE
			ContentType = alloc_tchar_to_char(tpMailItem->ContentType);
			(**tpPart)->Filename = multipart_get_filename(ContentType, "name");
			mem_free(&ContentType);
#else
			(**tpPart)->Filename = multipart_get_filename(tpMailItem->ContentType, "name");
#endif
		}
		*cnt = 1;
	} else {
		// テキストのパートを検索
		for (i = 0; i < *cnt; i++) {
			if ((*(*tpPart + i))->ContentType == NULL ||
				str_cmp_ni((*(*tpPart + i))->ContentType, "text", tstrlen("text")) == 0) {
				*TextIndex = i;
				break;
			}
		}
	}

	if (*cnt > 0 && *TextIndex != -1) {
		wenc_ret = MIME_text_extract_decode(*(*tpPart + *TextIndex), ViewSrc);
	} else if (*TextIndex == -1 && tpMailItem->Body != NULL && op.ViewShowAttach) {
		wenc_ret = alloc_copy_t(STR_MSG_NOTEXTPART);
	}

	if (wenc_ret == NULL && tpMailItem->Body != NULL) {
		char *p = tpMailItem->Body, *q;
		if (ViewSrc == FALSE && tpMailItem->HasHeader) {
			q = GetBodyPointa(p);
			if (q != NULL) {
				p = q;
			}
		}
#ifdef UNICODE
		r = wenc_ret = alloc_char_to_tchar(p);
#else
		r = alloc_copy(p);
#endif
	} else {
		r = wenc_ret;
	}
	if (r != NULL) {
		DelDot(r, r); // process in place
	}
	return r;
}

/*
 * MIME_text_extract_decode - extract text part and decode if necessary
 */
TCHAR *MIME_text_extract_decode(MULTIPART *tpPart, BOOL ViewSrc) {
	TCHAR *ret = NULL;
	char *spos, *body;
	int encode = 0;
	
	if (ViewSrc == FALSE) {
		spos = tpPart->sPos;
	} else {
		spos = tpPart->hPos;
	}

	if (tpPart->ePos == NULL) {
		body = alloc_copy(spos);
		if (body == NULL) {
			return NULL;
		}
	} else {
		int len = tpPart->ePos - spos;
		body = (char *)mem_alloc(sizeof(char) * (len + 1));
		if (body == NULL) {
			return NULL;
		}
		if (len == 0) {
			*body = '\0';
		} else {
			str_cpy_n(body, spos, len - 1);
		}
	}

	if (tpPart->Encoding != NULL) {
		if (str_cmp_i(tpPart->Encoding, ENCODE_BASE64) == 0) {
			encode = ENC_TYPE_BASE64;
		} else if (str_cmp_i(tpPart->Encoding, ENCODE_Q_PRINT) == 0) {
			encode = ENC_TYPE_Q_PRINT;
		}
	}
	if (encode != 0) {
		char *enc_ret = (char *)mem_alloc(tstrlen(body) + 1);
		if (enc_ret != NULL) {
			((encode == ENC_TYPE_BASE64) ? base64_decode : QuotedPrintable_decode)(body, enc_ret, TRUE);
			mem_free(&body);
			body = enc_ret;
		}
	}

	ret = MIME_body_decode_charset(body, tpPart->ContentType);
	if (ret == NULL) {
#ifdef UNICODE
		ret = alloc_char_to_tchar(body);
		mem_free(&body);
#else
		ret = body;
#endif
	} else {
		mem_free(&body);
	}

	return ret;
}


/*
 * alloc_char_to_tchar_check - check for non-ASCII characters before conversion
 */
#ifdef UNICODE
TCHAR *alloc_char_to_tchar_check(char *str)
{
	TCHAR *tchar = NULL;
	if (op.ViewCharset != NULL && *op.ViewCharset != TEXT('\0')) {
		// check for characters not in the ASCII range (0-127 or 0x00-0x7F)
		unsigned char *p;
		for (p = str; *p != '\0'; p++) {
			if (*p > 0x7F) {
				tchar = MIME_charset_decode(CP_int, str, op.ViewCharset);
				// #ifdef UNICODE, charset_to_cp always returns CP_int
				//tchar = MIME_charset_decode(charset_to_cp((BYTE)font_charset), str, op.ViewCharset);
				break;
			}
		}
	}
	if (tchar == NULL) {
		tchar = alloc_char_to_tchar(str);
	}
	return tchar;
}
#endif
/* End of source */