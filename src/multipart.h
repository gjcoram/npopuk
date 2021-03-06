/*
 * nPOP
 *
 * multipart.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_MULTIPART_H
#define _INC_MULTIPART_H

/* Include Files */
#include <windows.h>

/* Define */

/* Struct */
typedef struct _MULTIPART {
	char *ContentType;
	char *Filename;
	char *Encoding;
	char *ContentID;
	char *hPos;
	char *sPos;
	char *ePos;
	BOOL Forwardable;
	BOOL IsDigestMsg;
	BOOL EmbeddedImage;
} MULTIPART;

/* Function Prototypes */
MULTIPART *multipart_add(MULTIPART ***tpMultiPart, int cnt);
void multipart_free(MULTIPART ***tpMultiPart, int cnt);
#ifndef WSAASYNC
void encatt_free(char ***EncAtt, int cnt);
#endif
char *multipart_get_filename(char *buf, char *Attribute);
int multipart_verify(char *ContentType, char *buf);
int multipart_parse(char *ContentType, char *buf, BOOL StopAtTextPart, MULTIPART ***tpMultiPart, int cnt);
int multipart_create(TCHAR *Filename, TCHAR *FwdAttach, MAILITEM *tpFwdMailItem, char *ContentType, char *Encoding, char **RetContentType, char *body, char **RetBody, int *num_att, char ***EncAtt);
char *convert_cid(char *start, char *end, MULTIPART **tpMultiPart, int mpcnt, BOOL open);

#endif
/* End of source */
