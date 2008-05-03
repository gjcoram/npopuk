#ifndef _INC_MAIL_NPOP_VER_H
#define _INC_MAIL_NPOP_VER_H

/* First two defines are the major and minor version of nPOPuk.
 * The next two defines should be blank for real releases.
 * Beta and Patch numbers can be placed there when appropriate.
 */
#define NPOPMAJOR  2
#define NPOPMINOR 10
#define NPOPBETA   9
#define NPOPPATCH  4


#if NPOPPATCH
#define NPOPCALCBETA NPOPBETA
#define NPOPCALCPATCH NPOPPATCH
#define MKVERSTRHELP(a,b,c,d) #a "." #b "b" #c "p" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("b") TEXT(#c) TEXT("p") TEXT(#d)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Beta " #c " Patch " #d
#define MKVERSTR(a,b,c,d) MKVERSTRHELP(a,b,c,d)
#define MKVERTXT(a,b,c,d) MKVERTXTHELP(a,b,c,d)
#define MKVERLNG(a,b,c,d) MKVERSTRHELP(a,b,c,d)
#elif NPOPBETA
#define NPOPCALCBETA NPOPBETA
#define NPOPCALCPATCH 0
#define MKVERSTRHELP(a,b,c,d) #a "." #b "b" #c
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("b") TEXT(#c)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Beta " #c
#define MKVERSTR(a,b,c,d) MKVERSTRHELP(a,b,c,d)
#define MKVERTXT(a,b,c,d) MKVERTXTHELP(a,b,c,d)
#define MKVERLNG(a,b,c,d) MKVERSTRHELP(a,b,c,d)

#else
#define NPOPCALCBETA 0
#define NPOPCALCPATCH 0
#define MKVERSTRHELP(a,b,c,d) #a "." #b
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b)
#define MKVERLNGHELP(a,b,c,d) #a "." #b
#define MKVERSTR(a,b,c,d) MKVERSTRHELP(a,b,c,d)
#define MKVERTXT(a,b,c,d) MKVERTXTHELP(a,b,c,d)
#define MKVERLNG(a,b,c,d) MKVERSTRHELP(a,b,c,d)
#endif

#define NPOPVER             NPOPMAJOR, NPOPMINOR, NPOPCALCBETA, NPOPCALCPATCH
#define NPOPVERSTR MKVERSTR(NPOPMAJOR, NPOPMINOR, NPOPCALCBETA, NPOPCALCPATCH)
#define NPOPVERTXT MKVERTXT(NPOPMAJOR, NPOPMINOR, NPOPCALCBETA, NPOPCALCPATCH)
#define NPOPVERLNG MKVERLNG(NPOPMAJOR, NPOPMINOR, NPOPCALCBETA, NPOPCALCPATCH)

#endif
