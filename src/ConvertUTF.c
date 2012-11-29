// ConvertUTF
// This source code adapted from ConvertUTF.h and ConvertUTF.c
// as posted to StackOverflow
// http://stackoverflow.com/questions/2867123/convert-utf-16-to-utf-8-under-windows-and-linux-in-c
// including the copyright notice below.

/* --------------------------------------------------------------------- */
/*
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------

	Conversions between UTF32, UTF-16, and UTF-8. Source code file.
	Author: Mark E. Davis, 1994.
	Rev History: Rick McGowan, fixes & updates May 2001.
	Sept 2001: fixed const & error conditions per
	mods suggested by S. Parent & A. Lillich.
	June 2002: Tim Dodd added detection and handling of incomplete
	source sequences, enhanced error detection, added casts
	to eliminate compiler warnings.
	July 2003: slight mods to back out aggressive FFFE detection.
	Jan 2004: updated switches in from-UTF8 conversions.
	Oct 2004: updated to use UNI_MAX_LEGAL_UTF32 in UTF-32 conversions.

------------------------------------------------------------------------ */

#include "General.h"

typedef unsigned long UTF32; /* at least 32 bits */

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR	(UTF32)0x0000FFFD
#define UNI_MAX_BMP				(UTF32)0x0000FFFF
#define UNI_MAX_UTF16			(UTF32)0x0010FFFF
#define UNI_MAX_UTF32			(UTF32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32		(UTF32)0x0010FFFF

static const int halfShift  = 10; /* used for shifting by 10 bits */

static const UTF32 halfBase = 0x0010000UL;
static const UTF32 halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END	(UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END	 (UTF32)0xDFFF

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
 * left as-is for anyone who may want to do such conversion, which was
 * allowed in earlier algorithms.
 */
static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
			 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
 * for *legal* UTF-8 will be 4 or fewer bytes total.
 */
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */

/* --------------------------------------------------------------------- */

int ConvertUTF16toUTF8(const WCHAR *wstr, char *dest, int len, BOOL strict)
{
	const WCHAR *source = wstr, *sourceEnd = source + lstrlen(source);
	char *target = dest, *targetEnd = NULL;
	int retlen = 1; // includes \0
	if (target != NULL) {
		targetEnd = target + len;
	}
	while (source < sourceEnd) {
		UTF32 ch;
		unsigned short bytesToWrite = 0;
		const UTF32 byteMask = 0xBF;
		const UTF32 byteMark = 0x80; 
		ch = *source++;
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
			/* If the 16 bits following the high surrogate are in the source buffer... */
			if (source < sourceEnd) {
				UTF32 ch2 = *source;
				/* If it's a low surrogate, convert to UTF32. */
				if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
					ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
					+ (ch2 - UNI_SUR_LOW_START) + halfBase;
					++source;
				} else if (strict) { /* it's an unpaired high surrogate */
					break;
				}
			} else { /* We don't have the 16 bits following the high surrogate. */
				break;
			}
		} else if (strict) {
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
				break;
			}
		}
		/* Figure out how many bytes the result will require */
		if (ch < (UTF32)0x80) {
			bytesToWrite = 1;
		} else if (ch < (UTF32)0x800) {
			bytesToWrite = 2;
		} else if (ch < (UTF32)0x10000) {
			bytesToWrite = 3;
		} else if (ch < (UTF32)0x110000) {
			bytesToWrite = 4;
		} else {
			bytesToWrite = 3;
			ch = UNI_REPLACEMENT_CHAR;
		}
		retlen += bytesToWrite;

		if (target) {
			if (target + bytesToWrite > targetEnd) {
				break;
			}
			target += bytesToWrite;
			switch (bytesToWrite) { /* note: everything falls through. */
				case 4: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 3: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 2: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
				case 1: *--target = (unsigned char) (ch | firstByteMark[bytesToWrite]);
			}
			target += bytesToWrite;
		}
	}
	if (target) *target = '\0';
	return retlen;
}

/* --------------------------------------------------------------------- */

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *  length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

static BOOL isLegalUTF8(const unsigned char *source, int length) {
	unsigned char a;
	const unsigned char *srcptr = source+length;
	switch (length) {
		default: return FALSE;
		/* Everything else falls through when "true"... */
		case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return FALSE;
		case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return FALSE;
		case 2: if ((a = (*--srcptr)) > 0xBF) return FALSE;

		switch (*source) {
			/* no fall-through in this inner switch */
			case 0xE0: if (a < 0xA0) return FALSE; break;
			case 0xED: if (a > 0x9F) return FALSE; break;
			case 0xF0: if (a < 0x90) return FALSE; break;
			case 0xF4: if (a > 0x8F) return FALSE; break;
			default:   if (a < 0x80) return FALSE;
		}

		case 1: if (*source >= 0x80 && *source < 0xC2) return FALSE;
	}
	if (*source > 0xF4) return FALSE;
	return TRUE;
}

/* --------------------------------------------------------------------- */

int ConvertUTF8toUTF16(const unsigned char *str, WCHAR *dest, int len, BOOL strict)
{
	const unsigned char *source = str, *sourceEnd = source + strlen(source);
	WCHAR *target = dest, *targetEnd = NULL;
	int retlen = 1; // includes \0
	if (target != NULL) {
		targetEnd = target + len;
	}
	while (source < sourceEnd) {
		UTF32 ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd) {
			break;
		}
		/* Do this check whether lenient or strict */
		if (! isLegalUTF8(source, extraBytesToRead+1)) {
			break;
		}
		/*
		 * The cases all fall through. See "Note A" below.
		 */
		switch (extraBytesToRead) {
			case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
			case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
			case 3: ch += *source++; ch <<= 6;
			case 2: ch += *source++; ch <<= 6;
			case 1: ch += *source++; ch <<= 6;
			case 0: ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
				if (strict) {
					break;
				} else {
					retlen++;
					if (target) *target++ = UNI_REPLACEMENT_CHAR;
				}
			} else {
				/* normal case */
				retlen++;
				if (target) *target++ = (WCHAR)ch;
			}
		} else if (ch > UNI_MAX_UTF16) {
			if (strict) {
				break; /* Bail out; shouldn't continue */
			} else {
				retlen++;
				if (target) *target++ = UNI_REPLACEMENT_CHAR;
			}
		} else {
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			if (target && target + 1 >= targetEnd) {
				break;
			}
			ch -= halfBase;
			retlen += 2;
			if (target) {
				*target++ = (WCHAR)((ch >> halfShift) + UNI_SUR_HIGH_START);
				*target++ = (WCHAR)((ch & halfMask) + UNI_SUR_LOW_START);
			}
		}
		if (target && target >= targetEnd) {
			break;
		}
	}
	if (target) *target = TEXT('\0');
	return retlen;
}

/* ---------------------------------------------------------------------

	Note A.
	The fall-through switches in UTF-8 reading code save a
	temp variable, some decrements & conditionals.  The switches
	are equivalent to the following loop:
	{
		int tmpBytesToRead = extraBytesToRead+1;
		do {
		ch += *source++;
		--tmpBytesToRead;
		if (tmpBytesToRead) ch <<= 6;
		} while (tmpBytesToRead > 0);
	}
	In UTF-8 writing code, the switches on "bytesToWrite" are
	similarly unrolled loops.

   --------------------------------------------------------------------- */