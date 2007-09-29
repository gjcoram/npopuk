/*
 * nPOP
 *
 * Memory.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include <windows.h>
#include <tchar.h>

/* Define */

/* Global Variables */

/* Local Function Prototypes */
#ifdef _DEBUG
static long all_alloc_size = 0;

#define MEM_CHECK
#ifdef MEM_CHECK
#define ADDRESS_CNT		100000
#define DEBUG_ADDRESS	0
static long address[ADDRESS_CNT];
static int address_index;
#endif	// MEM_CHECK

#endif	// _DEBUG

/*
 * mem_alloc - バッファを確保
 */
void *mem_alloc(const DWORD size)
{
#ifdef _DEBUG
	void *mem;

	mem = LocalAlloc(LMEM_FIXED, size);
	all_alloc_size += LocalSize(mem);
#ifdef MEM_CHECK
	if (address_index < ADDRESS_CNT) {
		if (address_index == DEBUG_ADDRESS) {
			address[address_index] = (long)mem;
		} else {
			address[address_index] = (long)mem;
		}
		address_index++;
	}
#endif	// MEM_CHECK
	return mem;
#else	// _DEBUG
	return LocalAlloc(LMEM_FIXED, size);
#endif	// _DEBUG
}

/*
 * mem_calloc - 初期化してバッファを確保
 */
void *mem_calloc(const DWORD size)
{
#ifdef _DEBUG
	void *mem;

	mem = LocalAlloc(LPTR, size);
	all_alloc_size += LocalSize(mem);
#ifdef MEM_CHECK
	if (address_index < ADDRESS_CNT) {
		if (address_index == DEBUG_ADDRESS) {
			address[address_index] = (long)mem;
		} else {
			address[address_index] = (long)mem;
		}
		address_index++;
	}
#endif	// MEM_CHECK
	return mem;
#else	// _DEBUG
	return LocalAlloc(LPTR, size);
#endif	// _DEBUG
}

/*
 * mem_free - バッファを解放
 */
void mem_free(void **mem)
{
	if (*mem != NULL) {
#ifdef _DEBUG
		all_alloc_size -= LocalSize(*mem);
#ifdef MEM_CHECK
		{
			int i;

			for (i = 0; i < ADDRESS_CNT; i++) {
				if (address[i] == (long)*mem) {
					address[i] = 0;
					break;
				}
			}
		}
#endif	// MEM_CHECK
#endif	// _DEBUG
		LocalFree(*mem);
		*mem = NULL;
	}
}

/*
 * mem_debug - メモリ情報の表示
 */
#ifdef _DEBUG
void mem_debug(void)
{
	TCHAR buf[256];

	if (all_alloc_size == 0) {
		return;
	}

	wsprintf(buf, TEXT("Memory leak: %lu bytes"), all_alloc_size);
	MessageBox(NULL, buf, TEXT("debug"), 0);
#ifdef MEM_CHECK
	{
		int i, leaks=0;

		for (i = 0; i < ADDRESS_CNT; i++) {
			if (address[i] != 0) {
				if (leaks == 0) {
					wsprintf(buf, TEXT("first leak address: %u, %lu, %lx"), i, address[i], address[i]);
					MessageBox(NULL, buf, TEXT("debug"), MB_OK);
					MessageBox(NULL, (TCHAR *)address[i], TEXT("debug"), MB_OK);
				}
				leaks++;
			}
		}
		wsprintf(buf, TEXT("total leaks: %d"), leaks);
		MessageBox(NULL, buf, TEXT("debug"), MB_OK);
	}
#endif	// MEM_CHECK
}
#endif	// _DEBUG
/* End of source */
