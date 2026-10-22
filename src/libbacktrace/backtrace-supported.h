/*
 *   Static backtrace-supported.h for the embedded libbacktrace copy.
 *   Upstream normally generates this with configure.
 */

#ifndef MULLE_LIBBACKTRACE_SUPPORTED_H
#define MULLE_LIBBACKTRACE_SUPPORTED_H

#define BACKTRACE_SUPPORTED           1

#if defined( _WIN32)
# define BACKTRACE_USES_MALLOC        1
# define BACKTRACE_SUPPORTS_DATA      0
#else
# define BACKTRACE_USES_MALLOC        0
# define BACKTRACE_SUPPORTS_DATA      1
#endif

#define BACKTRACE_SUPPORTS_THREADS    1

#endif
