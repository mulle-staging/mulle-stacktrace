/*
 *   Prefix all libbacktrace symbols so the embedded copy cannot collide with
 *   another libbacktrace in the same process.
 *
 *   This file is force-included for the libbacktrace sources and included by
 *   mulle-stacktrace-backtrace.c before <backtrace.h>.
 */

#ifndef MULLE_LIBBACKTRACE_PREFIX_H
#define MULLE_LIBBACKTRACE_PREFIX_H

#define backtrace_alloc                          mulle_backtrace_alloc
#define backtrace_close                          mulle_backtrace_close
#define backtrace_create_state                   mulle_backtrace_create_state
#define backtrace_dwarf_add                      mulle_backtrace_dwarf_add
#define backtrace_free                           mulle_backtrace_free
#define backtrace_full                           mulle_backtrace_full
#define backtrace_get_view                       mulle_backtrace_get_view
#define backtrace_initialize                     mulle_backtrace_initialize
#define backtrace_open                           mulle_backtrace_open
#define backtrace_pcinfo                         mulle_backtrace_pcinfo
#define backtrace_print                          mulle_backtrace_print
#define backtrace_qsort                          mulle_backtrace_qsort
#define backtrace_release_view                   mulle_backtrace_release_view
#define backtrace_simple                         mulle_backtrace_simple
#define backtrace_syminfo                        mulle_backtrace_syminfo
#define backtrace_syminfo_to_full_callback       mulle_backtrace_syminfo_to_full_callback
#define backtrace_syminfo_to_full_error_callback mulle_backtrace_syminfo_to_full_error_callback
#define backtrace_uncompress_lzma                mulle_backtrace_uncompress_lzma
#define backtrace_uncompress_zdebug              mulle_backtrace_uncompress_zdebug
#define backtrace_uncompress_zstd                mulle_backtrace_uncompress_zstd
#define backtrace_vector_finish                  mulle_backtrace_vector_finish
#define backtrace_vector_grow                    mulle_backtrace_vector_grow
#define backtrace_vector_release                 mulle_backtrace_vector_release

#endif
