/*
 *   Static libbacktrace configuration for mulle-stacktrace.
 *
 *   Upstream libbacktrace normally generates this file with configure.
 *   mulle-stacktrace embeds libbacktrace without running configure, so this
 *   file derives the same decisions from predefined compiler and OS macros.
 *
 *   Keep this in sync with upstream's configure.ac when merging upstream.
 */

#ifndef MULLE_LIBBACKTRACE_CONFIG_H
#define MULLE_LIBBACKTRACE_CONFIG_H

/*
 * Feature test macros have to be visible before any system header is
 * included. Every libbacktrace source includes "config.h" first.
 */
#if ! defined( _WIN32)
# ifndef _GNU_SOURCE
#  define _GNU_SOURCE   1
# endif
# if defined( __APPLE__)
#  ifndef _DARWIN_C_SOURCE
#   define _DARWIN_C_SOURCE   1
#  endif
# endif
#endif

#define PACKAGE_NAME        "libbacktrace"
#define PACKAGE_TARNAME     "libbacktrace"
#define PACKAGE_VERSION     "version-unused"
#define PACKAGE_STRING      PACKAGE_NAME " " PACKAGE_VERSION
#define PACKAGE_BUGREPORT   ""
#define PACKAGE_URL         ""

#define STDC_HEADERS        1

/*
 * Object format size. Only elf.c reads this, but defining it always keeps
 * the header usable for any selected format.
 */
#if defined( __LP64__) || defined( _WIN64) || \
    (defined( __SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8)
# define BACKTRACE_ELF_SIZE    64
#else
# define BACKTRACE_ELF_SIZE    32
#endif

#define BACKTRACE_XCOFF_SIZE      unused

/*
 * Unwinder. GCC and Clang provide _Unwind_Backtrace and _Unwind_GetIPInfo.
 * MSVC/clang-cl do not, and are handled by the DbgHelp backend instead.
 */
#if defined( __GNUC__) || defined( __clang__)
# define HAVE_GETIPINFO            1
# define HAVE_ATOMIC_FUNCTIONS     1
# define HAVE_SYNC_FUNCTIONS       1
#endif

#if defined( _WIN32)
# define HAVE_WINDOWS_H            1
# define HAVE_DECL__PGMPTR         1
# if defined( __MINGW32__)
#  define HAVE_TLHELP32_H          1
# endif
/* MinGW has strnlen and alloc.c/read.c are used instead of mmap. */
# define HAVE_DECL_STRNLEN         1
#else
/* POSIX: glibc, musl, bionic, macOS and the BSDs. */
# define HAVE_FCNTL                1
# define HAVE_LSTAT                1
# define HAVE_READLINK             1
# define HAVE_DECL_STRNLEN         1
# define HAVE_DECL_GETPAGESIZE     1

# if defined( __APPLE__)
#  define HAVE_MACH_O_DYLD_H       1
# endif

# if defined( __linux__) || defined( __ANDROID__) || \
     defined( __FreeBSD__) || defined( __NetBSD__) || \
     defined( __OpenBSD__) || defined( __DragonFly__) || \
     defined( __ELF__)
#  define HAVE_DL_ITERATE_PHDR     1
#  define HAVE_LINK_H              1
# endif

# if defined( __FreeBSD__) || defined( __DragonFly__)
#  define HAVE_KERN_PROC           1
# endif
# if defined( __NetBSD__)
#  define HAVE_KERN_PROC_ARGS      1
# endif
#endif

#endif
