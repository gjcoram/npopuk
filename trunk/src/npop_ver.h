#ifndef _INC_MAIL_NPOP_VER_H
#define _INC_MAIL_NPOP_VER_H

/* First two defines define what the 3rd part of the version number should
 * be, for non-beta versions.
 *
 * The next two defines are the major and minor version of nPOPuk.
 *
 * For a real release, use patch = 0 or empty
 * If there is ever a need to re-release a real-release, use patch > 0
 * For a release candidate, use Beta=99, Patch = rc# (should be > 0)
 * For a beta, use a Beta between 1 and 98, with Patch = 0 or empty
 * For a patched beta, use Beta between 1 and 98, with Patch > 0
 */

#define NPOPBETA_REAL_RELEASE 100
#define NPOPBETA_RELEASE_CAND (NPOPBETA_REAL_RELEASE-1)

#define NPOPMAJOR  2
#define NPOPMINOR 16
#define NPOPBETA  6
#define NPOPPATCH 0


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

#if defined(ARM)
#define BUILDARCH TEXT("arm")
#elif defined(MIPS)
#define BUILDARCH TEXT("mips")
#elif defined(SH3)
#define BUILDARCH TEXT("sh3")
#elif defined(SH4)
#define BUILDARCH TEXT("sh4")
#elif defined(x86)
#define BUILDARCH TEXT("x86")
//#else
//#define BUILDARCH ??
#endif

#ifdef _WIN32_WCE
#  if defined(_WIN32_WCE_SP)
#    define BUILDSTR TEXT("wm")
#  elif defined(_WIN32_WCE_PPC)
#    if WIN32_PLATFORM_PSPC >= 310
#      define BUILDSTR TEXT("ppc2002")
#    else
#      define BUILDSTR TEXT("ppc2000")
#    endif
#  else
#    if _WIN32_WCE >= 300
#      define BUILDSTR TEXT("wce30") BUILDARCH
#    elif _WIN32_WCE >= 211
#      define BUILDSTR TEXT("wce211") BUILDARCH
#    else // _WIN32_WCE <= 200
#      define BUILDSTR TEXT("wce20") BUILDARCH
#    endif
#  endif
#else
#  if defined( _MSC_VER) && (_MSC_VER <= 1200) // using VC++6 to build win98 versions
#    ifdef UNICODE
#      define BUILDSTR TEXT("win98u")
#    else
#      define BUILDSTR TEXT("win98")
#    endif
#  else
#    ifdef UNICODE
#      define BUILDSTR TEXT("win32u")
#    else
#      define BUILDSTR TEXT("win32")
#    endif
#  endif
#endif


#if NPOPBETA == NPOPBETA_REAL_RELEASE
#if NPOPPATCH
// Re-released (patched) version
#define MKVERSTRHELP(a,b,c,d) #a "." #b "p" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("p") TEXT(#d)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " patch " #d
#define MKBLDSTR(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("p") TEXT(#d) TEXT("_") BUILDSTR TEXT(".zip")
#define MKBLDSTRSSL(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("p") TEXT(#d) TEXT("ssl_") BUILDSTR TEXT(".zip")
#else
// Real release
#define MKVERSTRHELP(a,b,c,d) #a "." #b
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b)
#define MKVERLNGHELP(a,b,c,d) #a "." #b
#define MKBLDSTR(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("_") BUILDSTR TEXT(".zip")
#define MKBLDSTRSSL(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("ssl_") BUILDSTR TEXT(".zip")
#endif

#elif NPOPBETA == NPOPBETA_RELEASE_CAND
// Release candidates
#define MKVERSTRHELP(a,b,c,d) #a "." #b "rc" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("rc") TEXT(#d)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Release Candidate " #d
#define MKBLDSTR(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("rc") TEXT(#d) TEXT("_") BUILDSTR TEXT(".zip")
#define MKBLDSTRSSL(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("rc") TEXT(#d) TEXT("ssl_") BUILDSTR TEXT(".zip")

#else
// Beta releases, with and without patch number
#if NPOPPATCH
//#define MKVERSTRHELP(a,b,c,d) #a "." #b "b" #c "t" #d
#define MKVERSTRHELP(a,b,c,d) #a "." #b "b" #c "p" #d
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("b") TEXT(#c) TEXT("p") TEXT(#d)
//#define MKVERLNGHELP(a,b,c,d) #a "." #b " Beta " #c " Test " #d
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Beta " #c " Patch " #d
#define MKBLDSTR(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("b") TEXT(#c) TEXT("p") TEXT(#d) TEXT("_") BUILDSTR TEXT(".zip")
#define MKBLDSTRSSL(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("b") TEXT(#c) TEXT("p") TEXT(#d) TEXT("ssl_") BUILDSTR TEXT(".zip")
#elif NPOPBETA
#define MKVERSTRHELP(a,b,c,d) #a "." #b "b" #c
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b) TEXT("b") TEXT(#c)
#define MKVERLNGHELP(a,b,c,d) #a "." #b " Beta " #c
#define MKBLDSTR(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("b") TEXT(#c) TEXT("_") BUILDSTR TEXT(".zip")
#define MKBLDSTRSSL(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("b") TEXT(#c) TEXT("ssl_") BUILDSTR TEXT(".zip")

#else
// not a patch, not a beta, not a real release, not a release candidate ... what is it?
#define MKVERSTRHELP(a,b,c,d) #a "." #b
#define MKVERTXTHELP(a,b,c,d) TEXT(#a) TEXT(".") TEXT(#b)
#define MKVERLNGHELP(a,b,c,d) #a "." #b
#define MKBLDSTR(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("_") BUILDSTR TEXT(".zip")
#define MKBLDSTRSSL(a,b,c,d) TEXT("npopuk") TEXT(#a) TEXT(#b) TEXT("ssl_") BUILDSTR TEXT(".zip")
#endif
#endif

#define MKVERSTR(a,b,c,d) MKVERSTRHELP(a,b,c,d)
#define MKVERTXT(a,b,c,d) MKVERTXTHELP(a,b,c,d)
#define MKVERLNG(a,b,c,d) MKVERLNGHELP(a,b,c,d)
#define MKVERWEBHELP(a,b) TEXT(#a) TEXT(".") TEXT(#b)
#define MKVERWEBADDR(a,b) MKVERWEBHELP(a,b)
#define MKBUILD(a,b,c,d)  MKBLDSTR(a,b,c,d)
#define MKBUILDSSL(a,b,c,d) MKBLDSTRSSL(a,b,c,d)

#define NPOPVER             NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH
#define NPOPVERSTR MKVERSTR(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH)
#define NPOPVERTXT MKVERTXT(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH)
#define NPOPVERLNG MKVERLNG(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH)
#define NPOPWEBVER MKVERWEBADDR(NPOPMAJOR, NPOPMINOR)

#define NPOPBUILD     MKBUILD(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH) 
#define NPOPBUILDSSL  MKBUILDSSL(NPOPMAJOR, NPOPMINOR, NPOPBETA, NPOPPATCH) 

#endif
