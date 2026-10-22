#
# Embedded libbacktrace
#
# This is not a standalone library. It builds an OBJECT library that the
# mulle-stacktrace target folds into itself via OTHER_LIBRARY_OBJECT_FILES.
# Keeping the files out of src/ means mulle-sde reflect does not compile them
# independently.
#
# configure is replaced by the static config.h and backtrace-supported.h in
# this directory. The source selection mirrors configure.ac:
#
#    common   : atomic dwarf fileline posix print sort state
#    format   : elf | macho | pecoff
#    view     : mmapio | read
#    alloc    : mmap | alloc
#    unwind   : backtrace + simple | nounwind
#

if( NOT MINGW AND (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC"))
   message( STATUS "libbacktrace: not built for MSVC/clang-cl, using DbgHelp")
   return()
endif()

set( LIBBACKTRACE_COMMON_SOURCES
   atomic.c
   dwarf.c
   fileline.c
   posix.c
   print.c
   sort.c
   state.c
)

if( WIN32)
   set( LIBBACKTRACE_PLATFORM_SOURCES
      pecoff.c
      read.c
      alloc.c
      backtrace.c
      simple.c
   )
elseif( APPLE)
   set( LIBBACKTRACE_PLATFORM_SOURCES
      macho.c
      mmapio.c
      mmap.c
      backtrace.c
      simple.c
   )
else()
   set( LIBBACKTRACE_PLATFORM_SOURCES
      elf.c
      mmapio.c
      mmap.c
      backtrace.c
      simple.c
   )
endif()

add_library( mulle-libbacktrace OBJECT
   ${LIBBACKTRACE_COMMON_SOURCES}
   ${LIBBACKTRACE_PLATFORM_SOURCES}
)

target_include_directories( mulle-libbacktrace PRIVATE
   "${CMAKE_CURRENT_SOURCE_DIR}"
)

if( MSVC)
   target_compile_options( mulle-libbacktrace PRIVATE
      "/FI${CMAKE_CURRENT_SOURCE_DIR}/mulle-backtrace-prefix.h"
   )
else()
   target_compile_options( mulle-libbacktrace PRIVATE
      "-include" "${CMAKE_CURRENT_SOURCE_DIR}/mulle-backtrace-prefix.h"
   )
endif()

if( BUILD_SHARED_LIBS)
   set_property( TARGET mulle-libbacktrace
                 PROPERTY POSITION_INDEPENDENT_CODE TRUE)
endif()
