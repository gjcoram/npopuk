/*
 * nPOP
 *
 * Code.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "jp.h"
#include "kr.h"

/* Define */
#define ESC						0x1B

#define HEAD_LINELEN			800
#define HEAD_ENCODE_LINELEN		15
#define BODY_ENCODE_LINELEN		76

#define CHARSET_US_ASCII		"US-ASCII"
#define CHARSET_ISO2022JP		"ISO-2022-JP"
#define CHARSET_ISO2022KR		"ISO-2022-KR"
#define CHARSET_UTF7			"UTF-7"
#define CHARSET_UTF8			"UTF-8"

#define ENCODE_7BIT				"7bit"
#define ENCODE_8BIT				"8bit"
#define ENCODE_BASE64			"base64"
#define ENCODE_Q_PRINT			"quoted-printable"

#define ENC_TYPE_7BIT			0
#define ENC_TYPE_8BIT			1
#define ENC_TYPE_BASE64			2
#define ENC_TYPE_Q_PRINT		3

#define	Is8bitChar(c)			(((unsigned char)c & (unsigned char)0x80) ? TRUE : FALSE)

/* Global Variables */
// エンコード情報
typedef struct _ENCODE_INFO {
	char *buf;
	BOOL encode;

	struct _ENCODE_INFO *next;
} ENCODE_INFO;

extern OPTION op;

/* Local Function Prototypes */
static char *UtfToMultiByte(char *buf, BOOL UTF8);
static char *MultiByteToUtf(char *buf, BOOL UTF8);
static char *EncodeCharset(char *buf, TCHAR *charset);
static char *DecodeCharset(char *buf, char *charset);
static char *GetToken(char *p, BOOL *encode);
static char *GetAddressToken(char *p, BOOL *encode);
static int GetEncodeBreakLen(char *buf, int len);
static ENCODE_INFO *CreateEncodeInfo(char *buf, TCHAR *charset, BOOL Address);
static void FreeEncodeInfo(ENCODE_INFO *eb);
static int HexVal(int c);
static int GetMIMEencodeSize(ENCODE_INFO *eb);
static int ExtendedEncodeSize(ENCODE_INFO *eb);
static char *DecodeBodyCharset(char *buf, char *ContentType);

/*
 * IsDependenceString - ShiftJISの機種依存文字が含まれているかチェック
 */
int IsDependenceString(TCHAR *buf)
{
#define	IsKanji(c)				(((unsigned char)c >= (unsigned char)0x81 && (unsigned char)c <= (unsigned char)0x9F) || \
									((unsigned char)c >= (unsigned char)0xE0 && (unsigned char)c <= (unsigned char)0xFC))
	unsigned char *p;
	unsigned char c,d;
	int ret = -1;
	int index = 0;
#ifdef UNICODE
	char *cBuf;
#endif

	if (buf == NULL) {
		return ret;
	}
	if ((op.HeadCharset == NULL || lstrcmpi(op.HeadCharset, TEXT(CHARSET_ISO2022JP)) != 0) &&
		(op.BodyCharset == NULL || lstrcmpi(op.BodyCharset, TEXT(CHARSET_ISO2022JP)) != 0)) {
		return ret;
	}

#ifdef UNICODE
	cBuf = AllocTcharToChar(buf);
	if (cBuf == NULL) {
		return ret;
	}
	p = (unsigned char *)cBuf;
#else
	p = buf;
#endif
	while ((c = *(p++)) != '\0') {
		if (IsKanji(c) == TRUE) {
			d = *(p++);
			if (
				// 特殊文字区点コード, 13区
				(c == 0x87 && (d >= 0x40 && d <= 0x9C)) ||

				// NEC選定IBM拡張文字区点コード, 89区～92区
				(c == 0xED && (d >= 0x40 && d <= 0xFF)) ||
				(c == 0xEE && (d >= 0x00 && d <= 0xFC)) ||

				// IBM拡張文字区点コード, 115区～119区
				(c == 0xFA && (d >= 0x40 && d <= 0xFF)) ||
				(c == 0xFB && (d >= 0x00 && d <= 0xFF)) ||
				(c == 0xFC && (d >= 0x00 && d <= 0x4B))
				) {
				ret = index;
				break;
			}
#ifndef UNICODE
			index++;
#endif
		}
		index++;
	}
#ifdef UNICODE
	mem_free(&cBuf);
#endif
	return ret;
}

/*
 * Base64Decode - BASE64のデコード (RFC 2045)
 */
char *Base64Decode(char *buf, char *ret)
{
	int b, c, d, i;
	int *Base;
	unsigned char bb, *bf, *rf;

	if (*buf == '\0') {
		*ret = '\0';
		return ret;
	}

	Base = (int *)mem_calloc(sizeof(int) * 256);
	if (Base == NULL) {
		tstrcpy(ret, buf);
		return ret;
	}
	for (i = 0; i < 26; i++) {
		*(Base + 'A' + i) = i;
		*(Base + 'a' + i) = i + 26;
	}
	for (i = 0; i < 10; i++) {
		*(Base + '0' + i) = i + 52;
	}
	*(Base + '+') = 62;
	*(Base + '/') = 63;

	bf = buf;
	rf = ret;

	for (; *bf != '\0' &&
		(*bf == ' ' || *bf == '\t' || *bf == '\r' || *bf == '\n'); bf++);

	while (1) {
		if ((bb = *bf) == '=' || bb == '\0') {
			break;
		}
		c = *(Base + bb);

		bf++;
		for (; *bf != '\0' &&
			(*bf == ' ' || *bf == '\t' || *bf == '\r' || *bf == '\n'); bf++);
		if ((bb = *bf) == '=' || bb == '\0') {
			break;
		}
        d = *(Base + bb);
        b = c & 0x3;
        *(rf++) = (c << 2) | (d >> 4);

		bf++;
		for (; *bf != '\0' &&
			(*bf == ' ' || *bf == '\t' || *bf == '\r' || *bf == '\n'); bf++);
		if ((bb = *bf) == '=' || bb == '\0') {
			break;
		}
		c = *(Base + bb);
        b = d & 0xF;
        *(rf++) = (b << 4) | (c >> 2);

		bf++;
		for (; *bf != '\0' &&
			(*bf == ' ' || *bf == '\t' || *bf == '\r' || *bf == '\n'); bf++);
		if ((bb = *bf) == '=' || bb == '\0') {
			break;
		}
		d = *(Base + bb);
		bf++;
		for (; *bf != '\0' &&
			(*bf == ' ' || *bf == '\t' || *bf == '\r' || *bf == '\n'); bf++);

        b = c & 0x3;
        *(rf++) = (b << 6) | d;
	}
	*rf = '\0';
	mem_free(&Base);
	return rf;
}

/*
 * Base64Encode - BASE64のエンコード (RFC 2045)
 */
void Base64Encode(char *buf, char *ret, int size)
{
	char tmp, tmp2;
	char *r;
	int c, i;
	const char Base[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	i = 0;
	r = ret;
	while (1) {
		if ((size == 0 && *(buf + i) == '\0') || (size > 0 && size <= i)) {
			break;
		}
		c = (*(buf + i) & 0xFC) >> 2;
		*(r++) = *(Base + c);
		i++;

		if ((size == 0 && *(buf + i) == '\0') || (size > 0 && size <= i)) {
			*(buf + i) = 0;
			tmp2 = (char)(*(buf + i - 1) << 4) & 0x30;
			tmp = (char)(*(buf + i) >> 4) & 0xF;
			c = tmp2 | tmp;
			*(r++) = *(Base + c);
			*(r++) = '=';
			*(r++) = '=';
			break;
		}
		tmp2 = (char)(*(buf + i - 1) << 4) & 0x30;
		tmp = (char)(*(buf + i) >> 4) & 0xF;
		c = tmp2 | tmp;
		*(r++) = *(Base + c);

		if ((size == 0 && *(buf + i + 1) == '\0') || (size > 0 && size <= (i + 1))) {
			*(buf + i + 1) = 0;
			tmp2 = (char)(*(buf + i) << 2) & 0x3C;
			tmp = (char)(*(buf + i + 1) >> 6) & 0x3;
			c = tmp2 | tmp;
			*(r++) = *(Base + c);
			*(r++) = '=';
			break;
		}

		tmp2 = (char)(*(buf + i) << 2) & 0x3C;
		tmp = (char)(*(buf + i + 1) >> 6) & 0x3;
		c = tmp2 | tmp;
		*(r++) = *(Base + c);
		i++;

		c = *(buf + i) & 0x3F;
		*(r++) = *(Base + c);
		i++;
	}
	*r = '\0';
}

/*
 * TBase64Encode - BASE64のエンコード (UNICODE)
 */
#ifdef UNICODE
void TBase64Encode(TCHAR *buf, TCHAR *ret, int size)
{
	char *b64str;
	char *cret;
	int len;

	// TCHAR から char に変換
	b64str = AllocTcharToChar(buf);
	if (b64str == NULL) {
		*ret = TEXT('\0');
		return;
	}
	// 変換した文字列を格納するバッファ
	cret = (char *)mem_alloc(tstrlen(b64str) * 2 + 4);
	if (cret == NULL) {
		mem_free(&b64str);
		*ret = TEXT('\0');
		return;
	}
	// Base64エンコード
	Base64Encode(b64str, cret, 0);
	mem_free(&b64str);
	// char から TCHAR に変換
	len = CharToTcharSize(cret);
	CharToTchar(cret, ret, len);
	mem_free(&cret);
}
#endif

/*
 * QuotedPrintableDecode - Quoted Printableのデコード (RFC 2045)
 */
static int HexVal(int c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
	if (c >= 'a' && c <= 'z') return c - 'a' + 10;
	return 0;
}
char *QuotedPrintableDecode(char *buf, char *ret)
{
	char *p, *r;

	p = buf;
	r = ret;

	while (*p) {
		if (*p == '=') {
			if (*(p + 1) == '\r' && *(p + 2) == '\n') {
				p += 2;

			} else if (*(p + 1) == '\n') {
				p++;

			} else {
				*(r++) = HexVal(*(p + 1)) * 16 + HexVal(*(p + 2));
				p += 2;
			}
		} else {
			*(r++) = *p;
		}
		p++;
	}
	*r = '\0';
	return r;
}

/*
 * QuotedPrintableEncode - Quoted Printableのエンコード (RFC 2045)
 */
static const char cHex[] = "0123456789ABCDEF";
void QuotedPrintableEncode(unsigned char *buf, char *ret, int break_size)
{
	unsigned char *p;
	char *r;
	int i = 0;

	for (p = buf, r = ret; *p != '\0'; p++) {
		if ((*p >= 0x21 && *p <= 0x7F && *p != '=' && *p != '?' && *p != '_') ||
			*p == '\r' || *p == '\n' || *p == '\t') {
			*(r++) = *p;
			i++;
			if (*p == '\r') {
				continue;
			}
			if (*p == '\n') {
				i = 0;
			}
		} else {
			*(r++) = '=';
			*(r++) = cHex[*p >> 4];
			*(r++) = cHex[*p & 0xF];
			i += 3;
		}
		if (break_size > 0 && break_size <= i) {
			*(r++) = '=';
			*(r++) = '\r';
			*(r++) = '\n';
			i = 0;
			if (*(p + 1) == '.') {
				*(r++) = '.';
				i++;
			}
		}
	}
	*r = '\0';
}

/*
 * URLDecode - URL encoding デコード (RFC 2396)
 */
char *URLDecode(char *buf, char *ret)
{
	char *p, *r;

	p = buf;
	r = ret;

	while (*p) {
		if (*p == '%') {
			*(r++) = HexVal(*(p + 1)) * 16 + HexVal(*(p + 2));
			p += 2;
		} else if (*p == '+') {
			*(r++) = ' ';
		} else {
			*(r++) = *p;
		}
		p++;
	}
	*r = '\0';
	return r;
}

/*
 * URLEncode - URL encoding エンコード (RFC 2396)
 */
void URLEncode(unsigned char *buf, char *ret)
{
	unsigned char *p;
	char *r;

	for (p = buf, r = ret; *p != '\0'; p++) {
		if ((*p >= 'A' && *p <= 'Z') ||
			(*p >= 'a' && *p <= 'z') ||
			(*p >= '0' && *p <= '9')) {
			*(r++) = *p;
		} else {
			*(r++) = '%';
			*(r++) = cHex[*p >> 4];
			*(r++) = cHex[*p & 0xF];
		}
	}
	*r = '\0';
}

/*
 * AllocURLDecode - メモリを確保してURL encodingをデコード
 */
TCHAR *AllocURLDecode(TCHAR *buf)
{
	char *cbuf;
	char *tmp;

#ifdef UNICODE
	cbuf = AllocTcharToChar(buf);
#else
	cbuf = buf;
#endif
	if (cbuf == NULL) {
		return NULL;
	}
	tmp = (char *)mem_alloc(tstrlen(cbuf) + 1);
	if (tmp == NULL) {
#ifdef UNICODE
		mem_free(&cbuf);
#endif
		return NULL;
	}
	URLDecode(cbuf, tmp);
#ifdef UNICODE
	mem_free(&cbuf);
	return AllocCharToTchar(tmp);
#else
	return tmp;
#endif
}

/*
 * UtfToMultiByte - UNICODE を MultiByte に変換
 */
static char *UtfToMultiByte(char *buf, BOOL UTF8)
{
#ifdef _WIN32_WCE
	char *ret;

	ret = mem_alloc(tstrlen(buf) + 1);
	if (ret != NULL) {
		tstrcpy(ret, buf);
	}
	return ret;
#else
	wchar_t *wbuf;
	char *ret;
	int len;

	// UTF to ANSI code page
	len = MultiByteToWideChar(((UTF8 == FALSE) ? CP_UTF7 : CP_UTF8), 0, buf, -1, NULL, 0);
	wbuf = mem_alloc(sizeof(wchar_t) * (len + 1));
	if (wbuf == NULL) {
		return NULL;
	}
	MultiByteToWideChar(((UTF8 == FALSE) ? CP_UTF7 : CP_UTF8), 0, buf, -1, wbuf, len);

	// ANSI code page to MultiByte
	len = WideCharToMultiByte(CP_ACP, 0, wbuf, -1, NULL, 0, NULL, NULL);
	ret = mem_alloc((len + 1));
	if (ret == NULL) {
		mem_free(&wbuf);
		return NULL;
	}
	WideCharToMultiByte(CP_ACP, 0, wbuf, -1, ret, len, NULL, NULL);
	mem_free(&wbuf);
	return ret;
#endif
}

/*
 * MultiByteToUtf - MultiByte を UNICODE に変換
 */
static char *MultiByteToUtf(char *buf, BOOL UTF8)
{
#ifdef _WIN32_WCE
	char *ret;

	ret = mem_alloc(tstrlen(buf) + 1);
	if (ret != NULL) {
		tstrcpy(ret, buf);
	}
	return ret;
#else
	wchar_t *wbuf;
	char *ret;
	int len;

	// MultiByte to ANSI code page
	len = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
	wbuf = mem_alloc(sizeof(wchar_t) * (len + 1));
	if (wbuf == NULL) {
		return NULL;
	}
	MultiByteToWideChar(CP_ACP, 0, buf, -1, wbuf, len);

	// ANSI code page to UTF
	len = WideCharToMultiByte(((UTF8 == FALSE) ? CP_UTF7 : CP_UTF8), 0, wbuf, -1, NULL, 0, NULL, NULL);
	ret = mem_alloc((len + 1));
	if (ret == NULL) {
		mem_free(&wbuf);
		return NULL;
	}
	WideCharToMultiByte(((UTF8 == FALSE) ? CP_UTF7 : CP_UTF8), 0, wbuf, -1, ret, len, NULL, NULL);
	mem_free(&wbuf);
	return ret;
#endif
}

/*
 * EncodeCharset - 文字コードをcharsetで指定されたコードに変換
 */
static char *EncodeCharset(char *buf, TCHAR *charset)
{
	char *ret = NULL;

	if (TStrCmpNI(charset, TEXT(CHARSET_ISO2022JP), lstrlen(TEXT(CHARSET_ISO2022JP))) == 0) {
		// SJIS to ISO-2022-JP
		ret = (char *)mem_alloc(tstrlen(buf) * 4 + 1);
		if (ret == NULL) {
			return NULL;
		}
		sjis_iso2022jp(buf, ret);
	} else if (TStrCmpNI(charset, TEXT(CHARSET_ISO2022KR), lstrlen(TEXT(CHARSET_ISO2022KR))) == 0) {
		// KS_C_5601 to ISO-2022-KR
		ret = (char *)mem_alloc(ksc5601_iso2022kr_len(buf) + 1);
		if (ret == NULL) {
			return NULL;
		}
		ksc5601_iso2022kr(buf, ret);
	} else if (TStrCmpNI(charset, TEXT(CHARSET_UTF7), lstrlen(TEXT(CHARSET_UTF7))) == 0) {
		// UTF-7
		ret = MultiByteToUtf(buf, FALSE);
	} else if (TStrCmpNI(charset, TEXT(CHARSET_UTF8), lstrlen(TEXT(CHARSET_UTF8))) == 0) {
		// UTF-8
		ret = MultiByteToUtf(buf, TRUE);
	}
	return ret;
}

/*
 * DecodeCharset - charsetで指定された文字コードを内部コードに変換
 */
static char *DecodeCharset(char *buf, char *charset)
{
	char *ret = NULL;

	if (StrCmpNI(charset, CHARSET_ISO2022JP, tstrlen(CHARSET_ISO2022JP)) == 0) {
		// ISO-2022-JP to SJIS
		ret = (char *)mem_alloc(tstrlen(buf) + 1);
		if (ret == NULL) {
			return NULL;
		}
		iso2022jp_sjis(buf, ret);
	} else if (StrCmpNI(charset, CHARSET_ISO2022KR, tstrlen(CHARSET_ISO2022KR)) == 0) {
		// ISO-2022-KR to KS_C_5601
		ret = (char *)mem_alloc(iso2022kr_ksc5601_len(buf) + 1);
		if (ret == NULL) {
			return NULL;
		}
		iso2022kr_ksc5601(buf, ret);
	} else if (StrCmpNI(charset, CHARSET_UTF7, tstrlen(CHARSET_UTF7)) == 0) {
		// UTF-7
		ret = UtfToMultiByte(buf, FALSE);
	} else if (StrCmpNI(charset, CHARSET_UTF8, tstrlen(CHARSET_UTF8)) == 0) {
		// UTF-8
		ret = UtfToMultiByte(buf, TRUE);
	}
	return ret;
}

/*
 * GetToken - 単語取得
 */
static char *GetToken(char *p, BOOL *encode)
{
	*encode = FALSE;
	for (; *p != '\0'; p++) {
		if (Is8bitChar(*p) == TRUE) {
			// エンコードの必要あり
			*encode = TRUE;
		} else if (*p == ' ') {
			for (; *p == ' '; p++);
			break;
		}
	}
	return p;
}

/*
 * GetAddressToken - メールアドレス用の単語取得
 */
static char *GetAddressToken(char *p, BOOL *encode)
{
	*encode = FALSE;
	switch (*p)
	{
	case '<':
	case '>':
	case '(':
	case ')':
	case ',':
		p++;
		break;

	case '\"':
		for (p++; *p != '\0'; p++) {
			if (Is8bitChar(*p) == TRUE) {
				// エンコードの必要あり
				*encode = TRUE;
			}

#ifdef _WIN32_WCE
			if (MultiByteToWideChar(CP_ACP, 0, p, 2, NULL, 0) == 1) {
#else
			if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
#endif
				p++;
				*encode = TRUE;
				continue;
			}
			if (*p == '\\') {
				p++;
				continue;
			}
			if (*p == '\"') {
				p++;
				break;
			}
		}
		break;

	default:
		for (; *p != '\0'; p++) {
			if (Is8bitChar(*p) == TRUE) {
				// エンコードの必要あり
				*encode = TRUE;
			}
#ifdef _WIN32_WCE
			if (MultiByteToWideChar(CP_ACP, 0, p, 2, NULL, 0) == 1) {
#else
			if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
#endif
				p++;
				*encode = TRUE;
				continue;
			}
			if (*p == '\\') {
				p++;
				continue;
			}
			if (*p == '<' || *p == '>' || *p == '(' || *p == ')' || *p == '\"') {
				break;
			}
		}
		break;
	}
	return p;
}

/*
 * GetEncodeBreakLen - エンコード部の折り返し位置取得
 */
static int GetEncodeBreakLen(char *buf, int len)
{
	char *p;
	int i = 0;

	if ((int)tstrlen(buf) < (int)len) {
		return 0;
	}
	for (p = buf; *p != '\0'; p++, i++) {
		if (i >= len) {
			return i;
		}
#ifdef _WIN32_WCE
		if (MultiByteToWideChar(CP_ACP, 0, p, 2, NULL, 0) == 1) {
#else
		if (IsDBCSLeadByte((BYTE)*p) == TRUE) {
#endif
			p++;
			i++;
		}
	}
	return 0;
}

/*
 * CreateEncodeInfo - エンコード情報の作成
 */
static ENCODE_INFO *CreateEncodeInfo(char *buf, TCHAR *charset, BOOL Address)
{
	ENCODE_INFO top_eb;
	ENCODE_INFO *eb;
	ENCODE_INFO *tmp_eb;
	char *p, *r;
	BOOL encode;
	int len;

	top_eb.next = NULL;
	eb = &top_eb;

	// エンコード情報の作成
	p = buf;
	while (*p != '\0') {
		r = p;
		if (Address == FALSE) {
			p = GetToken(r, &encode);
		} else {
			p = GetAddressToken(r, &encode);
		}

		eb->next = mem_calloc(sizeof(ENCODE_INFO));
		if (eb->next == NULL) {
			FreeEncodeInfo(top_eb.next);
			return NULL;
		}
		eb = eb->next;
		eb->encode = encode;

		eb->buf = (char *)mem_alloc(p - r + 2);
		if (eb->buf == NULL) {
			FreeEncodeInfo(top_eb.next);
			return NULL;
		}
		StrCpyN(eb->buf, r, p - r + 1);
	}

	// マージ
	eb = top_eb.next;
	while (eb->next != NULL) {
		if (eb->encode == eb->next->encode) {
			tmp_eb = eb->next;

			p = mem_alloc(tstrlen(eb->buf) + tstrlen(tmp_eb->buf) + 1);
			if (p == NULL) {
				FreeEncodeInfo(top_eb.next);
				return NULL;
			}
			r = StrCpy(p, eb->buf);
			r = StrCpy(r, tmp_eb->buf);
			mem_free(&eb->buf);
			eb->buf = p;
			eb->next = tmp_eb->next;
			mem_free(&tmp_eb->buf);
			mem_free(&tmp_eb);
		} else {
			eb = eb->next;
		}
	}

	// 折り返し
	for (eb = top_eb.next; eb != NULL; eb = eb->next) {
		len = 0;
		if (eb->encode == TRUE) {
			len = GetEncodeBreakLen(eb->buf, HEAD_ENCODE_LINELEN);
			if ((int)tstrlen(eb->buf) <= (int)len) {
				len = 0;
			}
		} else {
			if (tstrlen(eb->buf) >= HEAD_LINELEN) {
				len = HEAD_LINELEN;
			}
		}
		if (len != 0) {
			tmp_eb = eb->next;

			eb->next = mem_calloc(sizeof(ENCODE_INFO));
			if (eb->next == NULL) {
				FreeEncodeInfo(top_eb.next);
				FreeEncodeInfo(tmp_eb);
				return NULL;
			}
			eb->next->encode = eb->encode;
			eb->next->next = tmp_eb;
			eb->next->buf = (char *)mem_alloc(tstrlen(eb->buf) - len + 1);
			if (eb->next->buf == NULL) {
				FreeEncodeInfo(top_eb.next);
				return NULL;
			}
			StrCpy(eb->next->buf, eb->buf + len);

			p = (char *)mem_alloc(len + 2);
			if (p == NULL) {
				FreeEncodeInfo(top_eb.next);
				return NULL;
			}
			StrCpyN(p, eb->buf, len + 1);
			mem_free(&eb->buf);
			eb->buf = p;
		}
	}

	// エンコード
	for (eb = top_eb.next; eb != NULL; eb = eb->next) {
		if (eb->encode == TRUE) {
			p = EncodeCharset(eb->buf, charset);
			if (p != NULL) {
				mem_free(&eb->buf);
				eb->buf = p;
			}
		}
	}
	return top_eb.next;
}

/*
 * FreeEncodeInfo - エンコード情報の解放
 */
static void FreeEncodeInfo(ENCODE_INFO *eb)
{
	if (eb == NULL) {
		return;
	}
	FreeEncodeInfo(eb->next);

	if (eb->buf != NULL) {
		mem_free(&eb->buf);
	}
	mem_free(&eb);
}

/*
 * MIMEdecode - MIMEのデコード (RFC 2047, RFC 2045)
 */
void MIMEdecode(char *buf, char *ret)
{
	char *CharSet;
	char *Encoding;
	char *Content;
	char *retbuf;
	char *tmp;
	char *p, *r, *s, *t;
	char *wk;
	BOOL JisFlag = FALSE;
	BOOL EncodeFlag = FALSE;

	*ret = '\0';

	p = buf;
	r = s = ret;
	while (*p != '\0') {
		if (*p != '=' || *(p + 1) != '?') {
			if (JisFlag == FALSE &&
				*p == ESC && *(p + 1) == '$' && (*(p + 2) == '@' || *(p + 2) == 'B')) {
				JisFlag = TRUE;
			}
			if (EncodeFlag == TRUE &&
				*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
				EncodeFlag = FALSE;
			}
			*(r++) = *(p++);
			continue;
		}

		// MIMEエンコードされた文字列間の空白は除去する
		if (EncodeFlag == TRUE) {
			r = s;
		}
		wk = p;
		p += 2;

		// キャラクタセット
		CharSet = p;
		for (; *p != '\0' && *p != '?'; p++);
		if (*p == '\0') {
			p = wk;
			*(r++) = *(p++);
			continue;
		}
		p++;

		// エンコード方法
		Encoding = p;
		for (; *p != '\0' && *p != '?'; p++);
		if (*p == '\0') {
			p = wk;
			*(r++) = *(p++);
			continue;
		}
		p++;

		// エンコード部の終了位置の取得
		for (t = p; *t != '\0'; t++) {
			if (*t == '?' && *(t + 1) == '=') {
				break;
			}
		}
		if (*t == '\0') {
			p = wk;
			*(r++) = *(p++);
			continue;
		}

		// 本体のコピー
		Content = (char *)mem_alloc(t - p + 1);
		if (Content == NULL) {
			p = wk;
			*(r++) = *(p++);
			continue;
		}
		for (s = Content; p < t; p++, s++) {
			*s = *p;
		}
		*s = '\0';

		// デコード
		retbuf = (char *)mem_alloc(tstrlen(Content) + 1);
		if (retbuf == NULL) {
			mem_free(&Content);
			p = wk;
			*(r++) = *(p++);
			continue;
		}
		switch (*Encoding)
		{
		case 'b': case 'B':
			Base64Decode(Content, retbuf);
			break;

		case 'q': case 'Q':
			QuotedPrintableDecode(Content, retbuf);
			break;

		default:
			tstrcpy(retbuf, Content);
			break;
		}
		mem_free(&Content);

		// 文字コードの変換
		wk = DecodeCharset(retbuf, CharSet);
		if (wk != NULL) {
			mem_free(&retbuf);
			retbuf = wk;
		}

		// コピー
		for (s = retbuf; *s != '\0'; s++) {
#ifdef _WIN32_WCE
			if (MultiByteToWideChar(CP_ACP, 0, s, 2, NULL, 0) == 1 && *(s + 1) != '\0') {
#else
			if (IsDBCSLeadByte((BYTE)*s) == TRUE && *(s + 1) != '\0') {
#endif
				*(r++) = *(s++);
				*(r++) = *s;
				continue;
			}
			if (*s != '\r' && *s != '\n') {
				*(r++) = *s;
			}
		}
		mem_free(&retbuf);

		EncodeFlag = TRUE;
		p += 2;
		s = r;
	}
	*r = '\0';

	if (JisFlag == TRUE) {
		// JISの場合はJIS->SJISを行う
		tmp = (char *)mem_alloc(tstrlen(ret) + 1);
		iso2022jp_sjis(ret, tmp);
		tstrcpy(ret, tmp);
		mem_free(&tmp);
	}
}

/*
 * GetMIMEencodeSize - MIMEエンコードしたサイズの取得
 */
static int GetMIMEencodeSize(ENCODE_INFO *eb)
{
	int i;
	int len = 0;

	for (; eb != NULL; eb = eb->next) {
		len += 3;			// "\r\n "
		i = tstrlen(eb->buf);

		if (eb->encode == FALSE || op.HeadEncoding == ENC_TYPE_7BIT || op.HeadEncoding == ENC_TYPE_8BIT) {
			len += i;
		} else {
			switch (op.HeadEncoding)
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
			len += lstrlen(op.HeadCharset);
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
 * MIMEencode - MIMEエンコード (RFC 2047, RFC 2045)
 */
TCHAR *MIMEencode(TCHAR *wbuf, BOOL Address)
{
	ENCODE_INFO *top_eb, *eb;
#ifdef UNICODE
	TCHAR *ret;
#endif
	char *buf;
	char *cret;
	char *EncType;
	char *cCharSet;
	char *tmp;
	char *r;
	int i;

#ifdef UNICODE
	// char に変換
	buf = AllocTcharToChar(wbuf);
	if (buf == NULL) {
		return NULL;
	}

	cCharSet = AllocTcharToChar(op.HeadCharset);
	if (cCharSet == NULL) {
		mem_free(&buf);
		return NULL;
	}
#else
	buf = wbuf;
	cCharSet = op.HeadCharset;
#endif

	// エンコード情報の作成
	top_eb = eb = CreateEncodeInfo(buf, op.HeadCharset, Address);
	if (top_eb == NULL) {
#ifdef UNICODE
		mem_free(&buf);
		mem_free(&cCharSet);
#endif
		return NULL;
	}
#ifdef UNICODE
	mem_free(&buf);
#endif

	// エンコード後のバッファ確保
	i = GetMIMEencodeSize(eb);
	cret = (char *)mem_alloc(i + 1);
	if (cret == NULL) {
		FreeEncodeInfo(top_eb);
#ifdef UNICODE
		mem_free(&cCharSet);
#endif
		return NULL;
	}
	r = cret;

	for (; eb != NULL; eb = eb->next) {
		if (top_eb != eb) {
			r = StrCpy(r, "\r\n ");
		}
		if (eb->encode == FALSE || op.HeadEncoding == ENC_TYPE_7BIT || op.HeadEncoding == ENC_TYPE_8BIT) {
			r = StrCpy(r, eb->buf);
		} else {
			switch (op.HeadEncoding)
			{
			case ENC_TYPE_BASE64:
			default:
				// Base64
				tmp = (char *)mem_alloc(tstrlen(eb->buf) * 2 + 4 + 1);
				if (tmp == NULL) {
					FreeEncodeInfo(top_eb);
					mem_free(&cret);
#ifdef UNICODE
					mem_free(&cCharSet);
#endif
					return NULL;
				}
				Base64Encode(eb->buf, tmp, 0);
				mem_free(&eb->buf);
				eb->buf = tmp;
				EncType = "B";
				break;

			case ENC_TYPE_Q_PRINT:
				// quoted-printable
				tmp = (char *)mem_alloc(tstrlen(eb->buf) * 3 + 1);
				if (tmp == NULL) {
					FreeEncodeInfo(top_eb);
					mem_free(&cret);
#ifdef UNICODE
					mem_free(&cCharSet);
#endif
					return NULL;
				}
				QuotedPrintableEncode(eb->buf, tmp, 0);
				mem_free(&eb->buf);
				eb->buf = tmp;
				EncType = "Q";
				break;
			}
			r = StrCpy(r, "=?");
			r = StrCpy(r, cCharSet);
			r = StrCpy(r, "?");
			r = StrCpy(r, EncType);
			r = StrCpy(r, "?");
			r = StrCpy(r, eb->buf);
			r = StrCpy(r, "?=");
		}
	}
	*r = '\0';

	FreeEncodeInfo(top_eb);

#ifdef UNICODE
	mem_free(&cCharSet);

	// TCHAR に変換
	ret = AllocCharToTchar(cret);
	if (ret == NULL) {
		mem_free(&cret);
		return NULL;
	}
	mem_free(&cret);
	return ret;
#else
	return cret;
#endif
}

/*
 * ExtendedDecode - ファイル名のデコード (RFC 2231)
 */
TCHAR *ExtendedDecode(TCHAR *buf)
{
	TCHAR *CharSet;
	TCHAR *lang = NULL;
	TCHAR *r;
#ifdef UNICODE
	TCHAR *ret;
#endif
	char *cCharSet;
	char *cbuf;
	char *tmp;

	CharSet = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(buf) + 1));
	if (CharSet == NULL) {
		return NULL;
	}
	r = TStrCpyF(CharSet, buf, TEXT('\''));
	if (*r != TEXT('\0')) {
		lang = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(r) + 1));
		if (lang == NULL) {
			mem_free(&CharSet);
			return NULL;
		}
		r = TStrCpyF(lang, r, TEXT('\''));
	}
	if (*r == TEXT('\0')) {
		r = buf;
		*CharSet = TEXT('\0');
	}

#ifdef UNICODE
	cCharSet = AllocTcharToChar(CharSet);
	if (cCharSet == NULL) {
		mem_free(&CharSet);
		mem_free(&lang);
		return NULL;
	}
	mem_free(&CharSet);
#else
	cCharSet = CharSet;
#endif

#ifdef UNICODE
	cbuf = AllocTcharToChar(r);
#else
	cbuf = AllocCopy(r);
#endif
	if (cbuf == NULL) {
		mem_free(&cCharSet);
		mem_free(&lang);
		return NULL;
	}

	// URLデコード
	tmp = (char *)mem_alloc(tstrlen(cbuf) + 1);
	if (tmp == NULL) {
		mem_free(&cbuf);
		mem_free(&cCharSet);
		mem_free(&lang);
		return NULL;
	}
	URLDecode(cbuf, tmp);
	mem_free(&cbuf);
	cbuf = tmp;

	// キャラクタセットの変換
	tmp = DecodeCharset(cbuf, cCharSet);
	if (tmp != NULL) {
		mem_free(&cbuf);
		cbuf = tmp;
	}
	mem_free(&cCharSet);
	mem_free(&lang);

#ifdef UNICODE
	ret = AllocCharToTchar(cbuf);
	if (ret == NULL) {
		mem_free(&cbuf);
		return NULL;
	}
	mem_free(&cbuf);
	return ret;
#else
	return cbuf;
#endif
}

/*
 * ExtendedEncodeSize - ファイル名のエンコードのサイズ (RFC 2231)
 */
static int ExtendedEncodeSize(ENCODE_INFO *eb)
{
	int len = 0;

	for (; eb != NULL; eb = eb->next) {
		len += tstrlen(eb->buf) * 3;
		len += (lstrlen(TEXT("\r\n filename**=;")) + 5);
		if (eb->next != NULL) {
			len++;			// ;
		}
	}
	len += lstrlen(op.HeadCharset) + 2;
	return len;
}

/*
 * ExtendedEncode - ファイル名のエンコード (RFC 2231)
 */
TCHAR *ExtendedEncode(TCHAR *wbuf)
{
	ENCODE_INFO *top_eb, *eb;
	TCHAR *ret;
	TCHAR *r, *t;
	char *buf;
	char *tmp;
	char *p;
	int Len;
	int cnt = 0;
	int i;

#ifdef UNICODE
	// char に変換
	buf = AllocTcharToChar(wbuf);
	if (buf == NULL) {
		return NULL;
	}
#else
	buf = wbuf;
#endif

	p = buf;
	while (*p != '\0') {
		if (Is8bitChar(*p) == TRUE) {
			break;
		}
		p++;
	}
	if (*p == '\0') {
#ifdef UNICODE
		mem_free(&buf);
#endif
		i = (lstrlen(TEXT("\r\n filename=\"\"")) + lstrlen(wbuf));
		ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (i + 1));
		if (ret != NULL) {
			wsprintf(ret, TEXT("\r\n filename=\"%s\""), wbuf);
		}
		return ret;
	}

	// エンコード情報の作成
	top_eb = eb = CreateEncodeInfo(buf, op.HeadCharset, FALSE);
	if (top_eb == NULL) {
#ifdef UNICODE
		mem_free(&buf);
#endif
		return NULL;
	}
#ifdef UNICODE
	mem_free(&buf);
#endif

	// エンコード後のバッファ確保
	i = ExtendedEncodeSize(eb);
	ret = (TCHAR *)mem_alloc(sizeof(TCHAR) * (i + 1));
	if (ret == NULL) {
		FreeEncodeInfo(top_eb);
		return NULL;
	}
	r = ret;

	for (; eb != NULL; eb = eb->next) {
		// URLエンコード
		tmp = (char *)mem_alloc(tstrlen(eb->buf) * 3 + 1);
		if (tmp == NULL) {
			FreeEncodeInfo(top_eb);
			return NULL;
		}
		URLEncode(eb->buf, tmp);
		mem_free(&eb->buf);
		eb->buf = tmp;

#ifdef UNICODE
		// TCHAR に変換
		t = AllocCharToTchar(eb->buf);
		if (t == NULL) {
			FreeEncodeInfo(top_eb);
			return NULL;
		}
#else
		t = eb->buf;
#endif
		if (cnt == 0) {
			Len = wsprintf(r, TEXT("\r\n filename*%d*=%s''%s"), cnt, op.HeadCharset, t);
		} else {
			Len = wsprintf(r, TEXT("\r\n filename*%d*=%s"), cnt, t);
		}
#ifdef UNICODE
		mem_free(&t);
#endif
		r += Len;
		cnt++;

		if (eb->next != NULL) {
			*(r++) = TEXT(';');
		}
	}
	FreeEncodeInfo(top_eb);
	return ret;
}

/*
 * DecodeBodyTransfer - Content-Transfer-Encoding にしたがってデコード
 */
char *DecodeBodyTransfer(MAILITEM *tpMailItem, char *body)
{
	char *encBuf, *encRet = NULL;
	int EncodeFlag = 0;

	encBuf = (char *)mem_alloc(tstrlen(body) + 1);
	if (encBuf == NULL) {
		return NULL;
	}
	StrCpy(encBuf, body);

	if (tpMailItem->Encoding == NULL || tpMailItem->ContentType == NULL ||
		TStrCmpNI(tpMailItem->ContentType, TEXT("text"), lstrlen(TEXT("text"))) != 0) {
		// テキストではない
		return encBuf;
	}

	// デコード
	if (TStrCmpNI(tpMailItem->Encoding, TEXT(ENCODE_BASE64), lstrlen(TEXT(ENCODE_BASE64))) == 0) {
		EncodeFlag = ENC_TYPE_BASE64;
	} else if (TStrCmpNI(tpMailItem->Encoding, TEXT(ENCODE_Q_PRINT), lstrlen(TEXT(ENCODE_Q_PRINT))) == 0) {
		EncodeFlag = ENC_TYPE_Q_PRINT;
	}
	if (EncodeFlag != 0) {
		encRet = (char *)mem_alloc(tstrlen(encBuf) + 1);
		if (encRet != NULL) {
			((EncodeFlag == ENC_TYPE_BASE64) ? Base64Decode : QuotedPrintableDecode)(encBuf, encRet);
			mem_free(&encBuf);
			encBuf = encRet;
		}
	}
	mem_free(&tpMailItem->Encoding);
	tpMailItem->Encoding = NULL;
	return encBuf;
}

/*
 * DecodeBodyCharset - ContentTypeで指定された文字コードを内部コードに変換
 */
static char *DecodeBodyCharset(char *buf, char *ContentType)
{
	char *ret;
	char *r;

	r = ContentType;
	while (r != NULL && *r != '\0') {
		for (; *r == ' '; r++);
		if (StrCmpNI(r, "charset", tstrlen("charset")) == 0) {
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
			for (; *r == ' '; r++);

			ret = DecodeCharset(buf, r);
			if (ret != NULL) {
				return ret;
			}
		}
		for (; *r != '\0' && *r != ';'; r++);
		if (*r != '\0') {
			r++;
		}
	}

	ret = mem_alloc(tstrlen(buf) + 1);
	if (ret != NULL) {
		iso2022jp_sjis(buf, ret);
	}
	return ret;
}

/*
 * BodyDecode - 本文のデコード (RFC 822, RFC 2822, RFC 2045)
 */
TCHAR *BodyDecode(MAILITEM *tpMailItem, BOOL ViewSrc, MULTIPART ***tpPart, int *cnt)
{
	TCHAR *mBody = NULL, *buf;
	TCHAR *r;
	char *encBuf, *encRet = NULL;
#ifdef UNICODE
	char *ct;
#endif
	int EncodeFlag = 0;
	int i;
	int TextIndex = -1;

	*cnt = 0;
	if (tpMailItem->Multipart == TRUE && ViewSrc == FALSE) {
		// マルチパートを解析する
		*cnt = MultiPart_Parse(tpMailItem->ContentType, tpMailItem->Body, tpPart, 0);
	}

	if (*cnt == 0 && AddMultiPartInfo(tpPart, 0) != NULL) {
		// マルチパートではない or ソース表示
		if (ViewSrc == FALSE) {
			(**tpPart)->ContentType = AllocCopy(tpMailItem->ContentType);
			(**tpPart)->Encoding = AllocCopy(tpMailItem->Encoding);
		}
		(**tpPart)->sPos = tpMailItem->Body;
		if (ViewSrc == TRUE || (**tpPart)->ContentType == NULL ||
			TStrCmpNI((**tpPart)->ContentType, TEXT("text"), lstrlen(TEXT("text"))) == 0) {
			// テキスト
			TextIndex = 0;
		} else {
			// ファイル名の取得
			(**tpPart)->Filename = GetFilename(tpMailItem->ContentType, TEXT("name"));
		}
		*cnt = 1;
	} else {
		// テキストのパートを検索
		for (i = 0; i < *cnt; i++) {
			if ((*(*tpPart + i))->ContentType == NULL ||
				TStrCmpNI((*(*tpPart + i))->ContentType, TEXT("text"), lstrlen(TEXT("text"))) == 0) {
				TextIndex = i;
				break;
			}
		}
	}

	if (*cnt > 0 && TextIndex != -1) {
		// 本文の取得
		if ((*(*tpPart + TextIndex))->ePos == NULL) {
			mBody = AllocCopy((*(*tpPart + TextIndex))->sPos);
			if (mBody == NULL) {
				return NULL;
			}
		} else {
			i = (*(*tpPart + TextIndex))->ePos - (*(*tpPart + TextIndex))->sPos;
			mBody = (TCHAR *)mem_alloc(sizeof(TCHAR) * (i + 1));
			if (mBody == NULL) {
				return NULL;
			}
			if (i == 0) {
				*mBody = TEXT('\0');
			} else {
				TStrCpyN(mBody, (*(*tpPart + TextIndex))->sPos, i - 1);
			}
		}

#ifdef UNICODE
		encBuf = AllocTcharToChar(mBody);
		if (encBuf == NULL) {
			mem_free(&mBody);
			return NULL;
		}
		mem_free(&mBody);
#else
		encBuf = mBody;
#endif
		// デコード
		if ((*(*tpPart + TextIndex))->Encoding != NULL) {
			if (TStrCmpI((*(*tpPart + TextIndex))->Encoding, TEXT(ENCODE_BASE64)) == 0) {
				EncodeFlag = ENC_TYPE_BASE64;
			} else if (TStrCmpI((*(*tpPart + TextIndex))->Encoding, TEXT(ENCODE_Q_PRINT)) == 0) {
				EncodeFlag = ENC_TYPE_Q_PRINT;
			}
		}
		if (EncodeFlag != 0) {
			encRet = (char *)mem_alloc(tstrlen(encBuf) + 1);
			if (encRet != NULL) {
				((EncodeFlag == ENC_TYPE_BASE64) ? Base64Decode : QuotedPrintableDecode)(encBuf, encRet);
				mem_free(&encBuf);
				encBuf = encRet;
			}
		}

		// キャラクタセットの変換
#ifdef UNICODE
		ct = AllocTcharToChar((*(*tpPart + TextIndex))->ContentType);
		encRet = DecodeBodyCharset(encBuf, ct);
		if (encRet != NULL) {
			mem_free(&encBuf);
			encBuf = encRet;
		}
		if (ct != NULL) {
			mem_free(&ct);
		}
#else
		encRet = DecodeBodyCharset(encBuf, (*(*tpPart + TextIndex))->ContentType);
		if (encRet != NULL) {
			mem_free(&encBuf);
			encBuf = encRet;
		}
#endif

#ifdef UNICODE
		mBody = AllocCharToTchar(encBuf);
		if (mBody == NULL) {
			mem_free(&encBuf);
			return NULL;
		}
		mem_free(&encBuf);
#else
		mBody = encBuf;
#endif
	}

	r = (mBody != NULL) ? mBody : tpMailItem->Body;
	if (r != NULL) {
		buf = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(r) + 1));
		if (buf != NULL) {
			DelDot(r, buf);
			mem_free(&mBody);
			r = buf;
		}
	}
	return r;
}

/*
 * BodyEncode - 本文のエンコード (RFC 822, RFC 2822, RFC 2045)
 */
TCHAR *BodyEncode(TCHAR *body, TCHAR *content_type, TCHAR *encoding, TCHAR *ErrStr)
{
#ifdef UNICODE
	TCHAR *ret;
#endif
	char *cret;
	char *tmp;
	char *cp, *cr;
	int i;
	BOOL mByteFlag = FALSE;

#ifdef UNICODE
	cret = AllocTcharToChar((body != NULL) ? body : TEXT(""));
#else
	cret = AllocCopy((body != NULL) ? body : TEXT(""));
#endif
	if (cret == NULL) {
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}

	for (cp = cret; *cp !='\0'; cp++) {
		if (Is8bitChar(*cp) == TRUE) {
			mByteFlag = TRUE;
			break;
		}
	}

	if (mByteFlag == FALSE) {
		// US-ASCII
		wsprintf(content_type, TEXT("text/plain; charset=\"%s\""), TEXT(CHARSET_US_ASCII));
		lstrcpy(encoding, TEXT(ENCODE_7BIT));

	} else {
		// CONTENT-TYPE
		wsprintf(content_type, TEXT("text/plain; charset=\"%s\""), op.BodyCharset);

		// charsetの変換
		tmp = EncodeCharset(cret, op.BodyCharset);
		if (tmp != NULL) {
			mem_free(&cret);
			cret = tmp;
		}

		switch (op.BodyEncoding) {
		case ENC_TYPE_7BIT:
		default:
			lstrcpy(encoding, TEXT(ENCODE_7BIT));
			break;

		case ENC_TYPE_8BIT:
			lstrcpy(encoding, TEXT(ENCODE_8BIT));
			break;

		case ENC_TYPE_BASE64:
			// Base64でエンコード
			lstrcpy(encoding, TEXT(ENCODE_BASE64));
			tmp = (char *)mem_alloc(tstrlen(cret) * 2 + 4 + 1);
			if (tmp == NULL) {
				mem_free(&cret);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			Base64Encode(cret, tmp, 0);
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
			lstrcpy(encoding, TEXT(ENCODE_Q_PRINT));
			tmp = (char *)mem_alloc(tstrlen(cret) * 4 + 1);
			if (tmp == NULL) {
				mem_free(&cret);
				lstrcpy(ErrStr, STR_ERR_MEMALLOC);
				return FALSE;
			}
			QuotedPrintableEncode(cret, tmp, BODY_ENCODE_LINELEN);
			mem_free(&cret);
			cret = tmp;
			break;
		}
	}
#ifdef UNICODE
	ret = AllocCharToTchar(cret);
	if (ret == NULL) {
		mem_free(&cret);
		lstrcpy(ErrStr, STR_ERR_MEMALLOC);
		return NULL;
	}
	mem_free(&cret);
	return ret;
#else
	return cret;
#endif
}
/* End of source */
