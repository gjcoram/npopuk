/*
 * nPOP
 *
 * code.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_CODE_H
#define _INC_CODE_H

/* Include Files */
#include <windows.h>

/* Define */

/* Struct */

/* Function Prototypes */
char *base64_decode(char *buf, char *ret, BOOL is_body);
void base64_encode(char *buf, char *ret, int size, int breaklen);
#ifdef UNICODE
void base64_encode_t(TCHAR *buf, TCHAR *ret, int size, int breaklen);
#else
#define base64_encode_t base64_encode
#endif

char *QuotedPrintable_decode(char *buf, char *ret, BOOL dummy);
char *Q_decode(char *buf, char *ret);
int QuotedPrintable_encode_length(unsigned char *buf, int break_size, const BOOL body);
void QuotedPrintable_encode(unsigned char *buf, char *ret, int break_size, const BOOL body);

char *URL_decode(char *buf, char *ret);
void URL_encode(unsigned char *buf, char *ret, BOOL sp_conv);

#ifdef UNICODE
TCHAR *URL_decode_t(TCHAR *buf, TCHAR *ret);
void URL_encode_t(TCHAR *buf, TCHAR *ret);
#else
#define URL_decode_t URL_decode
#define URL_encode_t(buf, ret) URL_encode(buf, ret, FALSE)
#endif

#endif
/* End of source */
