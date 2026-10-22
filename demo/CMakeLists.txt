cmake_minimum_required( VERSION 3.13...99.99)

project( demo C)

set( MULLE_PROJECT_TYPE "Executable")

### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/reflect")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


include( Environment)

include( Files)

### Embedded libbacktrace (via the parent project)

if( NOT MINGW AND (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC"))
   # MSVC/clang-cl use the DbgHelp backend instead of libbacktrace
else()
   list( FILTER SOURCES EXCLUDE REGEX "[.][.]/src/libbacktrace/.*[.]c$")
   add_subdirectory( ../src/libbacktrace "${CMAKE_CURRENT_BINARY_DIR}/libbacktrace")
   set( OTHER_EXECUTABLE_OBJECT_FILES $<TARGET_OBJECTS:mulle-libbacktrace>)
endif()

# include( IDESupport OPTIONAL)

include( Dependencies OPTIONAL)

foreach( SOURCE ${SOURCES})
   # get rid off src/main- prefix and .m suffix
   #             offset 9  length -9 - 2
   # ignore library "main.c" and "main.m" files, just in case
   #
   if( NOT (SOURCE MATCHES ".*/main\.[cm]$"))
      if( SOURCE MATCHES "^src/main-")
         list( APPEND DEMO_MAIN_SOURCES "${SOURCE}")
      else()
         list( APPEND DEMO_OTHER_SOURCES "${SOURCE}")
      endif()
   endif()
endforeach()

foreach( MAIN_SOURCE ${DEMO_MAIN_SOURCES})
   string( LENGTH "${MAIN_SOURCE}" MAIN_SOURCE_LENGTH)
   math( EXPR MAIN_SOURCE_LENGTH "${MAIN_SOURCE_LENGTH} - 11")
   string( SUBSTRING "${MAIN_SOURCE}" 9 ${MAIN_SOURCE_LENGTH} EXECUTABLE_NAME)

   set( SOURCES "${MAIN_SOURCE}" ${DEMO_OTHER_SOURCES})
   include( Executable)
   if( NOT MINGW AND (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC"))
      target_link_libraries( "${EXECUTABLE_LINK_TARGET}" PUBLIC dbghelp)
   endif()
   include( InstallExecutable)
   include( FinalOutput OPTIONAL)
   # only for Executable
   include( Motd OPTIONAL)
endforeach()

