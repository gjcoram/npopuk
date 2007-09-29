/**************************************************************************

	nPOP

	jp.h (Japanese encode)

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_JP_H
#define _INC_JP_H

/**************************************************************************
	Function Prototypes
**************************************************************************/

void sjis_iso2022jp(unsigned char *buf,unsigned char *ret);
char *iso2022jp_sjis(char *buf, char *ret);

#endif
/* End of source */
