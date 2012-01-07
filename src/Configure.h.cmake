
#ifndef ARX_CONFIGURE_H
#define ARX_CONFIGURE_H

#cmakedefine HAVE_WINAPI

// Threads
#cmakedefine HAVE_PTHREADS

// Audio backend
#cmakedefine HAVE_OPENAL
#cmakedefine HAVE_OPENAL_EFX
#cmakedefine HAVE_DSOUND

// Renderer backend
#cmakedefine HAVE_OPENGL
#cmakedefine HAVE_D3D9

// Input backend
#cmakedefine HAVE_SDL
#cmakedefine HAVE_DINPUT8

// POSIX / Linux features
#cmakedefine HAVE_NANOSLEEP_FUNC // HAVE_NANOSLEEP conflicts with SDL
#cmakedefine HAVE_CLOCK_GETTIME
#cmakedefine HAVE_STRSIGNAL
#cmakedefine HAVE_FORK
#cmakedefine HAVE_READLINK
#cmakedefine HAVE_DUP2
#cmakedefine HAVE_EXECLP
#cmakedefine HAVE_WAITPID
#cmakedefine HAVE_KILL
#cmakedefine HAVE_BACKTRACE
#cmakedefine HAVE_BACKTRACE_SYMBOLS_FD
#cmakedefine HAVE_ISATTY
#cmakedefine HAVE_WORDEXP_H
#cmakedefine HAVE_FPATHCONF
#cmakedefine HAVE_PC_NAME_MAX
#cmakedefine HAVE_NAME_MAX

// Mac OS X features
#cmakedefine HAVE_MACH_CLOCK

// Arx components
#cmakedefine BUILD_EDITOR
#cmakedefine BUILD_EDIT_LOADSAVE

// Build system
#cmakedefine UNITY_BUILD

// Default paths
#cmakedefine DATA_DIR
#ifdef DATA_DIR
#undef DATA_DIR
#define DATA_DIR "${DATA_DIR}"
#endif
#cmakedefine USER_DIR
#ifdef USER_DIR
#undef USER_DIR
#define USER_DIR "${USER_DIR}"
#endif
#cmakedefine DATA_DIR_PREFIXES
#ifdef DATA_DIR_PREFIXES
#undef DATA_DIR_PREFIXES
#define DATA_DIR_PREFIXES "${DATA_DIR_PREFIXES}"
#endif
#cmakedefine USER_DIR_PREFIXES
#ifdef USER_DIR_PREFIXES
#undef USER_DIR_PREFIXES
#define USER_DIR_PREFIXES "${USER_DIR_PREFIXES}"
#endif

#endif // ARX_CONFIGURE_H
