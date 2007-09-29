/*
 * nPOP
 *
 * Memory.h
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_MEMORY_H
#define _INC_MEMORY_H

/* Include Files */

/* Define */

/* Struct */

/* Function Prototypes */
void *mem_alloc(const DWORD size);
void *mem_calloc(const DWORD size);
void mem_free(void **mem);
#ifdef _DEBUG
void mem_debug(void);
#endif

#endif
/* End of source */
