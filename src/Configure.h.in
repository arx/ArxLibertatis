
#ifndef ARX_CONFIGURE_H
#define ARX_CONFIGURE_H

#cmakedefine ARX_HAVE_WINAPI

// Threads
#cmakedefine ARX_HAVE_PTHREADS
#cmakedefine ARX_HAVE_PRCTL
#cmakedefine ARX_HAVE_PR_SET_NAME
#cmakedefine ARX_HAVE_PTHREAD_SETNAME_NP
#cmakedefine ARX_HAVE_PTHREAD_SET_NAME_NP

// Audio backend
#cmakedefine ARX_HAVE_OPENAL
#cmakedefine ARX_HAVE_OPENAL_EFX
#cmakedefine ARX_HAVE_DSOUND

// Renderer backend
#cmakedefine ARX_HAVE_OPENGL
#cmakedefine ARX_HAVE_D3D9

// Input backend
#cmakedefine ARX_HAVE_SDL
#cmakedefine ARX_HAVE_DINPUT8

// Crash handler
#cmakedefine ARX_HAVE_CRASHHANDLER_POSIX
#cmakedefine ARX_HAVE_CRASHHANDLER_WINDOWS

// POSIX / Linux features
#cmakedefine ARX_HAVE_NANOSLEEP
#cmakedefine ARX_HAVE_CLOCK_GETTIME
#cmakedefine ARX_HAVE_FORK
#cmakedefine ARX_HAVE_READLINK
#cmakedefine ARX_HAVE_DUP2
#cmakedefine ARX_HAVE_EXECL
#cmakedefine ARX_HAVE_EXECLP
#cmakedefine ARX_HAVE_WAITPID
#cmakedefine ARX_HAVE_BACKTRACE
#cmakedefine ARX_HAVE_ISATTY
#cmakedefine ARX_HAVE_WORDEXP_H
#cmakedefine ARX_HAVE_FPATHCONF
#cmakedefine ARX_HAVE_PATHCONF
#cmakedefine ARX_HAVE_PC_NAME_MAX
#cmakedefine ARX_HAVE_PC_CASE_SENSITIVE
#cmakedefine ARX_HAVE_NAME_MAX
#cmakedefine ARX_HAVE_SCHED_GETSCHEDULER
#cmakedefine ARX_HAVE_UNAME
#cmakedefine ARX_HAVE_GETRUSAGE
#cmakedefine ARX_HAVE_POPEN
#cmakedefine ARX_HAVE_PCLOSE
#cmakedefine ARX_HAVE_SYSCONF
#cmakedefine ARX_HAVE_SIGACTION
#cmakedefine ARX_HAVE_DIRFD
#cmakedefine ARX_HAVE_FSTATAT

// Mac OS X features
#cmakedefine ARX_HAVE_MACH_CLOCK

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
#cmakedefine CONFIG_DIR
#ifdef CONFIG_DIR
#undef CONFIG_DIR
#define CONFIG_DIR "${CONFIG_DIR}"
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
#cmakedefine CONFIG_DIR_PREFIXES
#ifdef CONFIG_DIR_PREFIXES
#undef CONFIG_DIR_PREFIXES
#define CONFIG_DIR_PREFIXES "${CONFIG_DIR_PREFIXES}"
#endif

#cmakedefine CMAKE_INSTALL_FULL_LIBEXECDIR
#ifdef CMAKE_INSTALL_FULL_LIBEXECDIR
#undef CMAKE_INSTALL_FULL_LIBEXECDIR
#define CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}"
#endif

#endif // ARX_CONFIGURE_H
