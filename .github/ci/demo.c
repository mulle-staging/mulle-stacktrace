#include "mulle-stacktrace.h"

#include <stdio.h>


MULLE_C_NEVER_INLINE
static void   foo( void)
{
   _mulle_stacktrace( NULL, 1, mulle_stacktrace_linefeed, stdout);
}


int   main( void)
{
   printf( "backend: %s\n", mulle_stacktrace_get_backend( NULL));
   foo();
   return( 0);
}
