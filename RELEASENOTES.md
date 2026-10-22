### 0.6.0

feature: embed libbacktrace, no configure or external dependency

* embedded a configure-free libbacktrace copy under `src/libbacktrace`, built as an OBJECT library and folded into mulle-stacktrace
* file:line stacktraces on Linux, Android, macOS, BSD and Windows MinGW
* added a DbgHelp backend for MSVC/clang-cl, so Windows works with either toolchain
* prefixed all exported libbacktrace symbols with `mulle_` to avoid collisions
* removed the libbacktrace dependency, craftinfo and configure step
* fixed `mulle_stacktrace_count_frames` using an uninitialized context, made state creation race-safe, defined trimmed formatting (no address prefix, no startup frames) and stopped repeated libbacktrace errors from interleaving with the trace

### 0.5.3











* DllMain is emitted again for Windows debug dynamic builds after mulle-core renamed `MULLE__CORE__ALL_LOAD_BUILD` to `MULLE__CORE__ALL__LOAD_BUILD`

### 0.5.2





* backend macro renamed to ``MULLE_STRACKTRACE_BACKEND`` so the execinfo backend is now actually enabled on Apple/BSD/Linux (previously ``MULLE_STRACKTRACE_STYLE`` was defined and ignored)
* new ``MULLE_STACKTRACE_NO_LIBBACKTRACE`` define to force-disable the libbacktrace backend at build time



* added missing license headers to internal include files
* corrected project name and copyright years in source file headers
* added API summary documentation link to README

### 0.5.1


feature: add Windows cross-compilation support

* add unwind library dependency for Windows platforms
* move libbacktrace include from public to private headers
* guard libbacktrace usage behind `_WIN32` check for non-Windows only
* rename export macro from `DEMO_GLOBAL/BUILD` to `MULLE__STACKTRACE_GLOBAL`
