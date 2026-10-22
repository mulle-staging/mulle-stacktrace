//
//  mulle-stacktrace-dbghelp.c
//  mulle-stacktrace
//
//  Copyright (c) 2026 Nat! - Mulle kybernetiK.
//  Copyright (c) 2026 Codeon GmbH.
//  All rights reserved.
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
// DbgHelp backend for MSVC and clang-cl. libbacktrace cannot be used here
// because there is no _Unwind_Backtrace and DbgHelp reads PDBs, not DWARF.
//
#include "include-private.h"

#include "mulle-stacktrace.h"

#if MULLE_STRACKTRACE_BACKEND == MULLE_STRACKTRACE_BACKEND_DBGHELP

#include <windows.h>
#include <dbghelp.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


// Trim leading address or path from symbol string
static char   *_trim_belly_fat( char *s)
{
   char   *sym;

   sym = strchr( s, ' ');
   if( sym)
   {
      while( *sym == ' ')
         ++sym;
      if( *sym)
         return( sym);
      return( s);
   }
   return( s);
}


// Trim trailing offset or address from symbol string
static int   _trim_arse_fat( char *s)
{
   char   *offset;
   int    len;

   offset = strstr( s, " + ");
   if( offset)
      return( (int) (offset - s));
   len = (int) strlen( s);
   return( len);
}


#define stracktrace_has_prefix( s, prefix)   ! strncmp( s, prefix, sizeof( prefix) - 1)


static int   _trim_boring_functions( char *s, int size)
{
   if( size == -1)
      size = (int) strlen( s);

   if( size == 3 && ! strncmp( s, "0x0", 3))
      return( 1);

   if( stracktrace_has_prefix( s, "test_calloc_or_raise"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_realloc_or_raise"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_realloc"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_calloc"))
      return( 1);
   if( stracktrace_has_prefix( s, "test_free"))
      return( 1);
   if( stracktrace_has_prefix( s, "libmulle-testallocator"))
      return( 1);
   if( stracktrace_has_prefix( s, "mulle_objc"))
      return( 1);
   if( stracktrace_has_prefix( s, "_mulle_objc"))
      return( 1);

   return( 0);
}


static char   *_symbolize_nothing( void *address,
                                   size_t max,
                                   char *buf,
                                   size_t len,
                                   void **userinfo)
{
   MULLE_C_UNUSED( address);
   MULLE_C_UNUSED( max);
   MULLE_C_UNUSED( buf);
   MULLE_C_UNUSED( len);
   MULLE_C_UNUSED( userinfo);
   return( NULL);
}


static char   *_keep_belly_fat( char *s)
{
   return( s);
}


static int   _keep_arse_fat( char *s)
{
   return( (int) strlen( s));
}


static int   keep_boring_functions( char *s, int size)
{
   MULLE_C_UNUSED( s);
   MULLE_C_UNUSED( size);
   return( 0);
}


static struct mulle_stacktrace   dummy =
{
   .symbolize      = _symbolize_nothing,
   .trim_belly_fat = _trim_belly_fat,
   .trim_arse_fat  = _trim_arse_fat,
   .is_boring      = _trim_boring_functions,
   .backend        = "dbghelp"
};


// DbgHelp is not thread-safe, so symbolization is serialized.
static SRWLOCK   dbghelp_lock = SRWLOCK_INIT;
static INIT_ONCE dbghelp_once = INIT_ONCE_STATIC_INIT;


static BOOL CALLBACK   dbghelp_init( PINIT_ONCE once, PVOID param, PVOID *ctx)
{
   MULLE_C_UNUSED( once);
   MULLE_C_UNUSED( param);
   MULLE_C_UNUSED( ctx);

   SymSetOptions( SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
   return( SymInitialize( GetCurrentProcess(), NULL, TRUE));
}


static int   dbghelp_is_ready( void)
{
   return( InitOnceExecuteOnce( &dbghelp_once, dbghelp_init, NULL, NULL));
}


// Fill buf with "0xADDR function at file:line" or "0xADDR (no symbol...)"
static void   format_frame( struct mulle_stacktrace *stacktrace,
                            void *address,
                            char *buf,
                            size_t bufsize,
                            char *function_buf,
                            size_t function_bufsize,
                            char *filename_buf,
                            size_t filename_bufsize,
                            unsigned long *line_number)
{
   char                symbol_buffer[sizeof( SYMBOL_INFO) + MAX_SYM_NAME];
   char                user_buf[512];
   char                *s;
   char                *function;
   char                *filename;
   DWORD64             displacement;
   DWORD               line_displacement;
   IMAGEHLP_LINE64     line;
   PSYMBOL_INFO        symbol;
   void                *userinfo;
   int                 has_line;
   int                 has_symbol;

   symbol                  = (PSYMBOL_INFO) symbol_buffer;
   symbol->SizeOfStruct    = sizeof( SYMBOL_INFO);
   symbol->MaxNameLen      = MAX_SYM_NAME;
   displacement            = 0;
   line_displacement       = 0;
   memset( &line, 0, sizeof( line));
   line.SizeOfStruct       = sizeof( IMAGEHLP_LINE64);

   has_symbol = SymFromAddr( GetCurrentProcess(), (DWORD64) address,
                             &displacement, symbol);
   has_line   = SymGetLineFromAddr64( GetCurrentProcess(), (DWORD64) address,
                                      &line_displacement, &line);

   function = has_symbol ? symbol->Name : "";
   filename = has_line ? line.FileName : "";

   snprintf( function_buf, function_bufsize, "%s", function);
   snprintf( filename_buf, filename_bufsize, "%s", filename);
   *line_number = has_line ? (unsigned long) line.LineNumber : 0;

   if( has_symbol || has_line)
   {
      snprintf( buf, bufsize, "0x%p %s at %s:%lu",
                address,
                has_symbol ? function : "(unknown function)",
                has_line ? filename : "(unknown file)",
                *line_number);
   }
   else
   {
      snprintf( buf, bufsize, "0x%p (no symbol information)", address);
   }

   userinfo = NULL;
   s = stacktrace->symbolize( address, 0, user_buf, sizeof( user_buf),
                              &userinfo);
   if( s && strcmp( s, function))
   {
      snprintf( buf, bufsize, "0x%p %s", address, s);
   }
}


void   _mulle_stacktrace( struct mulle_stacktrace      *stacktrace,
                          int                          offset,
                          enum mulle_stacktrace_format format,
                          FILE                         *fp)
{
   char            *delimchar;
   char            *delim;
   char            *s;
   char            buf[512];
   char            filename_buf[512];
   char            function_buf[512];
   unsigned long   line_number;
   void            *callstack[256];
   int             frames;
   int             i;
   int             size;

   if( ! stacktrace)
      stacktrace = &dummy;
   if( ! fp)
      fp = stderr;

   switch( format)
   {
   case mulle_stacktrace_normal:
   case mulle_stacktrace_trimmed:
      fprintf( fp, " : [");
      delimchar = " |";
      break;

   case mulle_stacktrace_linefeed:
      delimchar = "\n";
      break;

   default:
      fprintf( fp, "address,segment_offset,symbol_offset,symbol_address,symbol_name,segment_address,segment_name\n");
      delimchar = "\n";
      break;
   }

   frames = CaptureStackBackTrace( 0, 256, callstack, NULL);
   if( offset < 0)
      offset = 0;
   if( offset > frames)
      offset = frames;

   AcquireSRWLockExclusive( &dbghelp_lock);
   if( dbghelp_is_ready())
   {
      delim = "";
      for( i = offset; i < frames; i++)
      {
         format_frame( stacktrace, callstack[ i], buf, sizeof( buf),
                       function_buf, sizeof( function_buf),
                       filename_buf, sizeof( filename_buf),
                       &line_number);

         if( format == mulle_stacktrace_trimmed)
         {
            // trim the address first, so prefix based boring checks see the
            // symbol
            s = stacktrace->trim_belly_fat( buf);
            if( stacktrace->is_boring( s, -1))
               continue;

            size = stacktrace->trim_arse_fat( s);
            fprintf( fp, "%s%.*s", delim, size, s);
         }
         else
         {
            if( format != mulle_stacktrace_normal)
            {
               if( stacktrace->is_boring( buf, -1))
                  continue;
            }

            if( format == mulle_stacktrace_csv)
            {
               fprintf( fp, "%p,,,%s,,%s\n",
                        callstack[ i], function_buf, filename_buf);
               continue;
            }

            fprintf( fp, "%s%s", delim, buf);
         }
         delim = delimchar;
      }
   }
   ReleaseSRWLockExclusive( &dbghelp_lock);

   switch( format)
   {
   case mulle_stacktrace_normal:
   case mulle_stacktrace_trimmed:
      fputc( ']', fp);
      break;

   case mulle_stacktrace_linefeed:
      fputc( '\n', fp);
      break;

   default:
      break;
   }
}


int   mulle_stacktrace_count_frames( void)
{
   void   *callstack[256];

   return( CaptureStackBackTrace( 0, 256, callstack, NULL));
}


void   _mulle_stacktrace_init_default( struct mulle_stacktrace *stacktrace)
{
   stacktrace->symbolize      = _symbolize_nothing;
   stacktrace->trim_belly_fat = _trim_belly_fat;
   stacktrace->trim_arse_fat  = _trim_arse_fat;
   stacktrace->is_boring      = _trim_boring_functions;
   stacktrace->backend        = "dbghelp";
}


void   _mulle_stacktrace_init( struct mulle_stacktrace *stacktrace,
                               mulle_stacktrace_symbolizer_t *p_symbolize,
                               char *(*p_trim_belly_fat)( char *),
                               int (*p_trim_arse_fat)( char *),
                               int (*p_is_boring)( char *, int size))
{
   stacktrace->symbolize      = p_symbolize       ? p_symbolize       : _symbolize_nothing;
   stacktrace->trim_belly_fat = p_trim_belly_fat  ? p_trim_belly_fat  : _keep_belly_fat;
   stacktrace->trim_arse_fat  = p_trim_arse_fat   ? p_trim_arse_fat   : _keep_arse_fat;
   stacktrace->is_boring      = p_is_boring       ? p_is_boring       : keep_boring_functions;
   stacktrace->backend        = "dbghelp";
}

#endif
