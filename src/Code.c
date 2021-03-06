/*
 * nPOP
 *
 * Code.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2012 by Glenn Linderman and Geoffrey Coram.
 * All rights reserved.  Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "Memory.h"
#include "String.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * base64_decode - BASE64 (RFC 2045)
 */
static const signed char db64[256] = {
//0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
 -1, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // 00
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // 10
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, 62, -3, -3, -3, 63, // 20
 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -3, -3, -3, -2, -3, -3, // 30
 -3,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 40
 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -3, -3, -3, -3, -3, // 50
 -3, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 60
 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -3, -3, -3, -3, -3, // 70
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // 80
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // 90
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // A0
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // B0
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // C0
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // D9
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, // E0
 -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3  // F0
};

char *base64_decode(char *buf, char *ret, BOOL is_body)
{
	int c, d;
	unsigned char bb, *bf, *rf;

	if (*buf == '\0') {
		*ret = '\0';
		return ret;
	}
	bf = buf;
	rf = ret;

	while (1) {
		// first char of group never '='; never enough data to output
		do {
			bb = *(bf++);
			c = db64[bb];
		} while (c < -1);
		if ( c < 0 ) break;
		c = c << 2;

		// second char of group never '='
		do {
			bb = *(bf++);
			d = db64[bb];
		} while (d < -1);
		if ( d < 0 ) break;

		*rf = c | (d >> 4); // first output char from group
		if (is_body && *rf == '\n' && (rf == ret || *(rf-1) != '\r')) {
			*(rf++) = '\r';
			*rf = '\n';
		}
		rf++;
		d = (d & 0xF) << 4;

		do {
			bb = *(bf++);
			c = db64[bb];
		} while (c < -2);
		if ( c < 0 ) break;

		*rf = d | (c >> 2); // second output char from group
		if (is_body && *rf == '\n' && *(rf-1) != '\r') {
			*(rf++) = '\r';
			*rf = '\n';
		}
		rf++;
		c = (c & 0x3) << 6;

		do {
			bb = *(bf++);
			d = db64[bb];
		} while (d < -2);
		if ( d < 0 ) break;

		*rf = c | d; // third output char from group
		if (is_body && *rf == '\n' && *(rf-1) != '\r') {
			*(rf++) = '\r';
			*rf = '\n';
		}
		rf++;
	}
	*rf = '\0';
	return rf;
}

/*
 * base64_encode - BASE64 (RFC 2045)
 */
void base64_encode(char *buf, char *ret, int size, int breaklen)
{
	char *r, *bp, *ep;
	int c, d, cnt;
	const char Base[] =
	  // 00000000001111111111222222
	  // 01234567890123456789012345
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"

	  // 22223333333333444444444445
	  // 67890123456789012345678901
		"abcdefghijklmnopqrstuvwxyz"

	  // 555555556666
	  // 234567890123
		"0123456789+/";

	cnt = 0;
	r = ret;
	bp = buf;
	if (size == 0) {
		size = strlen(buf);
	}
	ep = bp + size;
	while (1) {
		if (bp >= ep) {
			break;
		}
		c = *(bp++);
		*(r++) = Base[(c >> 2) & 0x3F];
		cnt++;
		c = (c << 4) & 0x30;

		if (bp >= ep) {
			*(r++) = Base[c];
			*(r++) = '=';
			*(r++) = '=';
			break;
		}
		d = *(bp++);
		*(r++) = Base[c | ((d >> 4) & 0xF)];
		cnt++;
		d = (d << 2) & 0x3C;

		if (bp >= ep) {
			*(r++) = Base[d];
			*(r++) = '=';
			break;
		}

		c = *(bp++);
		*(r++) = Base[d | ((c >> 6) & 0x3)];
		cnt++;

		*(r++) = Base[c & 0x3F];
		cnt++;

		if (breaklen > 0 && cnt >= breaklen) {
			*(r++) = '\r';
			*(r++) = '\n';
			cnt = 0;
		}
	}
	*r = '\0';
}

/*
 * base64_encode_t - BASE64 (UNICODE)
 */
#ifdef UNICODE
void base64_encode_t(TCHAR *buf, TCHAR *ret, int size, int breaklen)
{
	char *b64str;
	char *cret;
	int len;

	//TCHAR conversion to char
	b64str = alloc_tchar_to_char(buf);
	if (b64str == NULL) {
		*ret = TEXT('\0');
		return;
	}

	//From buffer
	cret = (char *)mem_alloc(tstrlen(b64str) * 2 + 4);
	if (cret == NULL) {
		mem_free(&b64str);
		*ret = TEXT('\0');
		return;
	}
	//Base64 encoding
	base64_encode(b64str, cret, 0, 0);
	mem_free(&b64str);

	//char which houses the character string which it converts conversion to TCHAR
	len = char_to_tchar_size(cret);
	char_to_tchar(cret, ret, len);
	mem_free(&cret);
}
#endif

/*
 * hex_val - 16
 */
static int hex_val(int c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 256;
}

/*
 * QuotedPrintable_decode - Quoted Printable‚ (RFC 2045)
 *     '\r' = 13 = 0x0D, '\n' = 10 = 0x0A
 */
char *QuotedPrintable_decode(char *buf, char *ret, BOOL dummy)
{
	char *p, *r;
	int hextmp;

	p = buf;
	r = ret;

	while (*p) {
		if (*p == '=') {
			if (*(p + 1) == '\r' && *(p + 2) == '\n') {
				p += 2;

			} else if (*(p + 1) == '\n') {
				p++;

			} else {
				hextmp = hex_val(*(p + 1)) * 16 + hex_val(*(p + 2));
				if (hextmp > 255) { // preserve bad "digits"
					*(r++) = *p;
					*(r++) = *(p + 1);
					hextmp = *(p + 2);
				}
				*r = hextmp;
				p += 2;
				if (*r == '\n' && *(r-1) != '\r') {
					*(r++) = '\r';
					*r = '\n';
				} else if (*r == '\r') {
					if (*(p+1) == '\r') {
						r--; // discard the first \r
					} else if (*(p+1) != '\n' && *(p+1) != '=' && *(p+2) != '0' && *(p+3) != 'D') {
						r++;
						*r = '\n';
					}
				}
				r++;
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
 * Q_decode - almost quoted-printable, but for headers (RFC 1522) (GJC)
 */
char *Q_decode(char *buf, char *ret)
{
	char *p, *r;
	int hextmp;

	p = buf;
	r = ret;

	while (*p) {
		if (*p == '=') {
			if (*(p + 1) == '\r' && *(p + 2) == '\n') {
				p += 2;

			} else if (*(p + 1) == '\n') {
				p++;

			} else {
				hextmp = hex_val(*(p + 1)) * 16 + hex_val(*(p + 2));
				if (hextmp > 255) { // preserve bad "digits"
					*(r++) = *p;
					*(r++) = *(p + 1);
					hextmp = *(p + 2);
				}
				*r = hextmp;
				p += 2;
				if (*r == '\n' && *(r-1) != '\r') {
					*(r++) = '\r';
					*r = '\n';
#ifdef HANDLE_BARE_SLASH_R
				} else if (*r == '\r' && *p != '\n' && *p != '=' && *(p+1) != '0' && *(p+2) != 'D') {
					r++;
					*r = '\n';
#endif
				}
				r++;
			}
		} else if (*p == '_') {
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
 * QuotedPrintable_encode_length - Quoted Printable (RFC 2045)
 */

int QuotedPrintable_encode_length(unsigned char *buf, int break_size, const BOOL body)
{
	unsigned char *p;
	unsigned char *t;
	int len = 0, i = 0;

	for (p = buf; *p != '\0'; p++) {
		if ((*p >= 0x21 && *p <= 0x7E && *p != '=' && *p != '?' && *p != '_') ||
			*p == '\r' || *p == '\n') {
			len++;
			i++;
			if (*p == '\r') {
				continue;
			}
			if (*p == '\n') {
				i = 0;
			}
		} else {
			for (t = p; *t == ' ' || *t == '\t'; t++);
			if (body == TRUE && (*p == ' ' || *p == '\t') && !(*t == '\r' || *t == '\n')) {
				len++;
				i++;
			} else {
				if (break_size > 0 && (break_size - 1) <= (i + 3)) {
					len += 3;
					i = 0;
				}
				len += 3;
				i += 3;
			}
		}
		if (break_size > 0 && (break_size - 1) <= i) {
			len += 3;
			i = 0;
			if (body == TRUE && *(p + 1) == '.') {
				len++;
				i++;
			}
		}
	}
	len++;
	return len;
}

/*
 * QuotedPrintable_encode - Quoted Printable (RFC 2045)
 */
static const char cHex[] = "0123456789ABCDEF";

void QuotedPrintable_encode(unsigned char *buf, char *ret, int break_size, const BOOL body)
{
	unsigned char *p;
	unsigned char *t;
	char *r;
	int i = 0;

	for (p = buf, r = ret; *p != '\0'; p++) {
		if ((*p >= 0x21 && *p <= 0x7E && *p != '=' && *p != '?' && *p != '_') ||
			*p == '\r' || *p == '\n') {
			*(r++) = *p;
			i++;
			if (*p == '\r') {
				continue;
			}
			if (*p == '\n') {
				i = 0;
			}
		} else {
			for (t = p; *t == ' ' || *t == '\t'; t++);
			if (body == TRUE && (*p == ' ' || *p == '\t') && !(*t == '\r' || *t == '\n')) {
				*(r++) = *p;
				i++;
			} else {
				if (break_size > 0 && (break_size - 1) <= (i + 3)) {
					*(r++) = '=';
					*(r++) = '\r';
					*(r++) = '\n';
					i = 0;
				}
				*(r++) = '=';
				*(r++) = cHex[*p >> 4];
				*(r++) = cHex[*p & 0xF];
				i += 3;
			}
		}
		if (break_size > 0 && (break_size - 1) <= i) {
			*(r++) = '=';
			*(r++) = '\r';
			*(r++) = '\n';
			i = 0;
			if (body == TRUE && *(p + 1) == '.') {
				*(r++) = '.';
				i++;
			}
		}
	}
	*r = '\0';
}

/*
 * URL_decode - URL encoding (RFC 2396)
 */
char *URL_decode(char *buf, char *ret)
{
	char *p, *r;
	int hextmp;

	p = buf;
	r = ret;

	while (*p) {
		if (*p == '%') {
			hextmp = hex_val(*(p + 1)) * 16 + hex_val(*(p + 2));
			if (hextmp > 255) { // preserve bad "digits"
				*(r++) = *p;
				*(r++) = *(p + 1);
				hextmp = *(p + 2);
			}
			*(r++) = hextmp;
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
 * URL_encode - URL encoding (RFC 2396)
 */
void URL_encode(unsigned char *buf, char *ret, BOOL sp_conv)
{
	unsigned char *p;
	char *r;

	for (p = buf, r = ret; *p != '\0'; p++) {
		if ((*p >= 'A' && *p <= 'Z') ||
			(*p >= 'a' && *p <= 'z') ||
			(*p >= '0' && *p <= '9')) {
			*(r++) = *p;
		} else if (sp_conv && *p == ' ') {
			// allowed per RFC 2047
			*(r++) = '_';
		} else {
			*(r++) = '%';
			*(r++) = cHex[*p >> 4];
			*(r++) = cHex[*p & 0xF];
		}
	}
	*r = '\0';
}


#ifdef UNICODE
/*
 * These next two functions are non-standard.  They are used to create fake
 * URL encode/decode operations for TCHAR data, but the encoder only encodes
 * the non-alphanumeric characters < 128 ... this avoids URL parsing problems
 * when URL-reserved punctuation is used in filenames, email addresses, and
 * other parameter values that want to get embedded into the URL, but the
 * result is suitable only for passing around inside nPOPuk,
 * not for use on an actual web page as an actual URL.
 */

// the codepoints from 0x80 to 0x9F differ in Windows-1252 and UTF-16
static TCHAR W1252_to_UTF16[32] = {
	0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
	0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017D, 0x0178};

/*
 * URL_decode_t - URL encoding (RFC 2396)
 */
TCHAR *URL_decode_t(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p, *r;
	int hextmp;

	p = buf;
	r = ret;

	while (*p) {
		if (*p == TEXT('%')) {
			hextmp = hex_val(*(p + 1)) * 16 + hex_val(*(p + 2));
			if (hextmp >= 0x80 && hextmp <= 0x9F) { // difference between Windows-1252 and UTF-16
				hextmp = W1252_to_UTF16[hextmp - 0x80];
			} else if (hextmp > 0xFF) { // preserve bad "digits"
				*(r++) = *p;
				*(r++) = *(p + 1);
				hextmp = *(p + 2);
			}
			*(r++) = hextmp;
			p += 2;
		} else if (*p == TEXT('+')) {
			*(r++) = TEXT(' ');
		} else {
			*(r++) = *p;
		}
		p++;
	}
	*r = TEXT('\0');
	return r;
}

/*
 * URL_encode_t - URL encoding (RFC 2396)
 */
void URL_encode_t(TCHAR *buf, TCHAR *ret)
{
	TCHAR *p;
	TCHAR *r;

	for (p = buf, r = ret; *p != TEXT('\0'); p++) {
		if (*p < 128 ) {
			if ((*p >= TEXT('A') && *p <= TEXT('Z')) ||
			    (*p >= TEXT('a') && *p <= TEXT('z')) ||
			    (*p >= TEXT('0') && *p <= TEXT('9'))) {
				*(r++) = *p;
			} else {
				*(r++) = TEXT('%');
				*(r++) = cHex[*p >> 4];
				*(r++) = cHex[*p & 0xF];
			}
		} else {
			*(r++) = *p;
		}
	}
	*r = TEXT('\0');
}
#endif
/* End of source */
