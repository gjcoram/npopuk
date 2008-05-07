#ifndef _INC_MAIL_NPOP_VER_H
#define _INC_MAIL_NPOP_VER_H

/* First two defines are the major and minor version of nPOPuk.
 *
 * For a real release, use Beta = 100 and patch = 0 or empty
 * If there is ever a need to re-release a real-release, use Beta = 100 and patch > 0
 * For a release candidate, use Beta = 99, Patch = rc#
 * For a beta, use a Beta between 1 and 98, with Patch = 0 or empty
 * For a patched beta, use Beta between 1 and 98, with Patch > 0
 */
#define NPOPMAJOR  2
#define NPOPMINOR 10
#define NPOPBETA  10
#define NPOPPATCH  1

// convert text, blanks, or zeroes to zeroes
#if NPOPMAJOR < 1
#error NPOPMAJOR must be numeric, 1 or greater
#endif
#if NPOPMINOR < 1
#undef NPOPMINOR
#define NPOPMINOR 0
#endif
#if NPOPBETA < 1
#undef NPOPBETA
#define NPOPBETA 0
#endif
#if NPOPPATCH < 1
#undef NPOPPATCH
#define NPOPPATCH 0
#endif

#if nPOPBETA == 100
#if NPOPPATCH
// Re-released (patched) version
#define MKVERSTRHELP(a,b,c,d) #a "." #b "p" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("p") TEXT(#d)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " patch " #d
#else
// Real release
#define MKVERSTRHELP(a,b,c,d) #a "." #b
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b)
#define MKVERLNGHELP(a,b,c,d) #a "." #b
#endif

#elif NPOPBETA == 99
// Beta releases, with and without patch number
#if NPOPPATCH
#define MKVERSTRHELP(a,b,c,d) #a "." #b "rc" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("rc") TEXT(#d)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Release Candidate " #d
#else
#define MKVERSTRHELP(a,b,c,d) #a "." #b "rc" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("rc") TEXT(#d)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Release Candidate " #d
#endif

#else
#if NPOPPATCH
#define MKVERSTRHELP(a,b,c,d) #a "." #b "b" #c "p" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("b") TEXT(#c) TEXT("p") TEXT(#d)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Beta " #c " Patch " #d
#elif NPOPBETA
#define MKVERSTRHELP(a,b,c,d) #a "." #b "b" #c
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("b") TEXT(#c)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Beta " #c

#else
#define MKVERSTRHELP(a,b,c,d) #a "." #b
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b)
#define MKVERLNGHELP(a,b,c,d) #a "." #b
#endif
#endif

#define MKVERSTR(a,b,c,d) MKVERSTRHELP(a,b,c,d)
#define MKVERTXT(a,b,c,d) MKVERTXTHELP(a,b,c,d)
#define MKVERLNG(a,b,c,d) MKVERSTRHELP(a,b,c,d)

#define NPOPVER             NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH
#define NPOPVERSTR MKVERSTR(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH)
#define NPOPVERTXT MKVERTXT(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH)
#define NPOPVERLNG MKVERLNG(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH)

#endif
