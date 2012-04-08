/*
 * nPOP
 *
 * Profile.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_PROFILE_H
#define _INC_PROFILE_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
BOOL profile_initialize(const TCHAR *file_path, const BOOL pw_only);
BOOL profile_flush(const TCHAR *file_path);
void profile_free(void);
long profile_get_string(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *default_str, TCHAR *ret, const long size);
TCHAR *profile_alloc_string(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *default_str);
void profile_free_string(TCHAR *buf);
int profile_get_int(const TCHAR *section_name, const TCHAR *key_name, const int default_str);
BOOL profile_write_string(const TCHAR *section_name, const TCHAR *key_name, const TCHAR *str);
BOOL profile_write_int(const TCHAR *section_name, const TCHAR *key_name, const int num);
BOOL profile_delete_key(const TCHAR *section_name, const TCHAR *key_name);
BOOL profile_find_section(const TCHAR *section_name);
BOOL profile_clear_section(const TCHAR *section_name);

#endif
/* End of source */