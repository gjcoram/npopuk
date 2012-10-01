/*
 * nPOP
 *
 * mime.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_MIME_H
#define _INC_MIME_H

/* Include Files */
#include <windows.h>
#include "multipart.h"

/* Define */
#define HEAD_LINELEN			800
#define HEAD_ENCODE_LINELEN		76
#define BODY_ENCODE_LINELEN		76
#define ENC_TYPE_7BIT			0
#define ENC_TYPE_8BIT			1
#define ENC_TYPE_BASE64			2
#define ENC_TYPE_Q_PRINT		3

/* Struct */

/* Function Prototypes */
BOOL is_8bit_char_t(TCHAR *str);
char *MIME_charset_encode(const UINT cp, TCHAR *buf, TCHAR *charset);
TCHAR *MIME_charset_decode(const UINT cp, char *buf, TCHAR *charset);
char *MIME_encode(TCHAR *wbuf, BOOL Address, TCHAR *charset_t, int encoding, int headerlen);
char *MIME_encode_opt(TCHAR *wbuf, BOOL Address, TCHAR *charset_t, int encoding, int headerlen);
BOOL MIME_decode(char *buf, TCHAR *ret);
char *MIME_rfc2231_encode(TCHAR *wbuf, TCHAR *charset_t);
char *MIME_rfc2231_decode(char *buf);
BOOL MIME_create_encode_header(TCHAR *charset, int encoding, TCHAR *ctype_in, char *ret_content_type, char *ret_encoding);
char *MIME_body_encode(TCHAR *body, TCHAR *charset_t, int encoding, TCHAR *ctype_in, char *ret_content_type, char *ret_encoding, TCHAR *ErrStr);
char *MIME_body_decode_transfer(MAILITEM *tpMailItem, char *body);
TCHAR *MIME_body_decode(MAILITEM *tpMailItem, BOOL ViewSrc, BOOL StopAtTextPart, MULTIPART ***tpPart, int *cnt, int *TextIndex);
#ifdef UNICODE
TCHAR *alloc_char_to_tchar_check(char *str);
#else
#define alloc_char_to_tchar_check alloc_char_to_tchar
#endif
#endif
/* End of source */
