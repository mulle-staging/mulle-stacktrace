cmake_minimum_required( VERSION 3.15)

project( mulle_stacktrace_ci C)

set( CMAKE_C_STANDARD 11)
set( CMAKE_C_EXTENSIONS OFF)

if( NOT CMAKE_BUILD_TYPE)
   set( CMAKE_BUILD_TYPE Debug)
endif()

get_filename_component( ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)

if( NOT MULLE_DEPS_DIR)
   set( MULLE_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps/include")
endif()

set( LIBBACKTRACE_DIR "${ROOT}/libbacktrace")

if( NOT MSVC)
   add_library( libbacktrace OBJECT
      "${LIBBACKTRACE_DIR}/atomic.c"
      "${LIBBACKTRACE_DIR}/dwarf.c"
      "${LIBBACKTRACE_DIR}/fileline.c"
      "${LIBBACKTRACE_DIR}/posix.c"
      "${LIBBACKTRACE_DIR}/print.c"
      "${LIBBACKTRACE_DIR}/sort.c"
      "${LIBBACKTRACE_DIR}/state.c"
   )

   if( WIN32)
      target_sources( libbacktrace PRIVATE
         "${LIBBACKTRACE_DIR}/pecoff.c"
         "${LIBBACKTRACE_DIR}/read.c"
         "${LIBBACKTRACE_DIR}/alloc.c"
         "${LIBBACKTRACE_DIR}/backtrace.c"
         "${LIBBACKTRACE_DIR}/simple.c"
      )
   elseif( APPLE)
      target_sources( libbacktrace PRIVATE
         "${LIBBACKTRACE_DIR}/macho.c"
         "${LIBBACKTRACE_DIR}/mmapio.c"
         "${LIBBACKTRACE_DIR}/mmap.c"
         "${LIBBACKTRACE_DIR}/backtrace.c"
         "${LIBBACKTRACE_DIR}/simple.c"
      )
   else()
      target_sources( libbacktrace PRIVATE
         "${LIBBACKTRACE_DIR}/elf.c"
         "${LIBBACKTRACE_DIR}/mmapio.c"
         "${LIBBACKTRACE_DIR}/mmap.c"
         "${LIBBACKTRACE_DIR}/backtrace.c"
         "${LIBBACKTRACE_DIR}/simple.c"
      )
   endif()

   target_include_directories( libbacktrace PRIVATE
      "${LIBBACKTRACE_DIR}"
      "${MULLE_DEPS_DIR}"
   )
   target_compile_options( libbacktrace PRIVATE
      "-include" "${LIBBACKTRACE_DIR}/mulle-backtrace-prefix.h"
   )
endif()

set( STACKTRACE_SOURCES
   "${ROOT}/src/mulle-stacktrace.c"
)

if( MSVC)
   list( APPEND STACKTRACE_SOURCES "${ROOT}/src/mulle-stacktrace-dbghelp.c")
else()
   list( APPEND STACKTRACE_SOURCES "${ROOT}/src/mulle-stacktrace-backtrace.c")
endif()

add_library( stacktrace OBJECT ${STACKTRACE_SOURCES})

target_include_directories( stacktrace PRIVATE
   "${ROOT}/src"
   "${ROOT}/src/generic"
   "${ROOT}/src/reflect"
   "${LIBBACKTRACE_DIR}"
   "${MULLE_DEPS_DIR}"
)

if( MSVC)
   add_executable( ci-demo
      "${CMAKE_CURRENT_SOURCE_DIR}/demo.c"
      $<TARGET_OBJECTS:stacktrace>
   )
   target_link_libraries( ci-demo PRIVATE dbghelp)
else()
   add_executable( ci-demo
      "${CMAKE_CURRENT_SOURCE_DIR}/demo.c"
      $<TARGET_OBJECTS:stacktrace>
      $<TARGET_OBJECTS:libbacktrace>
   )
endif()

target_include_directories( ci-demo PRIVATE
   "${ROOT}/src"
   "${ROOT}/src/generic"
   "${ROOT}/src/reflect"
   "${LIBBACKTRACE_DIR}"
   "${MULLE_DEPS_DIR}"
)
