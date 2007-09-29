/**************************************************************************

	nPOP

	Profile.h

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_PROFILE_H
#define _INC_PROFILE_H

/**************************************************************************
	Include Files
**************************************************************************/

/**************************************************************************
	Define
**************************************************************************/

#define BUFSIZE						256					// バッファサイズ


/**************************************************************************
	Global Variables
**************************************************************************/

/**************************************************************************
	Struct
**************************************************************************/

struct TPKEY {
	TCHAR KeyName[BUFSIZE];
	int hash;
	TCHAR *String;
	BOOL CommentFlag;
};

struct TPSECTION {
	TCHAR SectionName[BUFSIZE];
	int hash;
	struct TPKEY *tpKey;
	int KeyCnt;
	int KeyAllocCnt;
};

/**************************************************************************
	Function Prototypes
**************************************************************************/

BOOL Profile_Initialize(TCHAR *path, BOOL ReadFlag);
BOOL Profile_Flush(TCHAR *path);
void Profile_Free(void);
long Profile_GetString(TCHAR *Section, TCHAR *Key, TCHAR *Default, TCHAR *ret, long size, TCHAR *File);
int Profile_GetInt(TCHAR *Section, TCHAR *Key, int Default, TCHAR *File);
void Profile_WriteString(TCHAR *Section, TCHAR *Key, TCHAR *str, TCHAR *File);
void Profile_WriteInt(TCHAR *Section, TCHAR *Key, int num, TCHAR *File);

#endif
/* End of source */
