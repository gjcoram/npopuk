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
#include "Memory.h"
#include "String.h"

/* Define */

/* Global Variables */

/* Local Function Prototypes */

/*
 * base64_decode - BASE64 (RFC 2045)
 */
char *base64_decode(char *buf, char *ret)
{
	int b, c, d, i;
	int Base[256];
	unsigned char bb, *bf, *rf;

	if (*buf == '\0') {
		*ret = '\0';
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
	return rf;
}

/*
 * base64_encode - BASE64 (RFC 2045)
 */
void base64_encode(char *buf, char *ret, int size, int breaklen)
{
	char tmp, tmp2;
	char *r;
	int c, i, cnt;
	const char Base[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	i = 0;
	cnt = 0;
	r = ret;
	while (1) {
		if ((size == 0 && *(buf + i) == '\0') || (size > 0 && size <= i)) {
			break;
		}
		c = (*(buf + i) & 0xFC) >> 2;
		*(r++) = *(Base + c);
		i++;
		cnt++;

		if ((size == 0 && *(buf + i) == '\0') || (size > 0 && size <= i)) {
			c = (char)(*(buf + i - 1) << 4) & 0x30;
			*(r++) = *(Base + c);
			*(r++) = '=';
			*(r++) = '=';
			break;
		}
		tmp2 = (char)(*(buf + i - 1) << 4) & 0x30;
		tmp = (char)(*(buf + i) >> 4) & 0xF;
		c = tmp2 | tmp;
		*(r++) = *(Base + c);
		cnt++;

		if ((size == 0 && *(buf + i + 1) == '\0') || (size > 0 && size <= (i + 1))) {
			c = (char)(*(buf + i) << 2) & 0x3C;
			*(r++) = *(Base + c);
			*(r++) = '=';
			break;
		}

		tmp2 = (char)(*(buf + i) << 2) & 0x3C;
		tmp = (char)(*(buf + i + 1) >> 6) & 0x3;
		c = tmp2 | tmp;
		*(r++) = *(Base + c);
		i++;
		cnt++;

		c = *(buf + i) & 0x3F;
		*(r++) = *(Base + c);
		i++;
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
 */
char *QuotedPrintable_decode(char *buf, char *ret)
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
		} else {
			*(r++) = *p;
		}
		p++;
	}
	*r = '\0';
	return r;
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
void URL_encode(unsigned char *buf, char *ret)
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
/* End of source */
