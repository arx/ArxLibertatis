
#ifndef ARX_CONFIGURE_H
#define ARX_CONFIGURE_H

#cmakedefine HAVE_WINAPI

// Threads
#cmakedefine HAVE_PTHREADS
#cmakedefine HAVE_PRCTL
#cmakedefine HAVE_PR_SET_NAME
#cmakedefine HAVE_PTHREAD_SETNAME_NP
#cmakedefine HAVE_PTHREAD_SET_NAME_NP

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

// Crash handler
#cmakedefine HAVE_CRASHHANDLER_POSIX
#cmakedefine HAVE_CRASHHANDLER_WINDOWS
#cmakedefine BUGTRACKER_PASSWD

// POSIX / Linux features
#cmakedefine HAVE_NANOSLEEP_FUNC // HAVE_NANOSLEEP conflicts with SDL
#cmakedefine HAVE_CLOCK_GETTIME
#cmakedefine HAVE_FORK
#cmakedefine HAVE_READLINK
#cmakedefine HAVE_DUP2
#cmakedefine HAVE_EXECL
#cmakedefine HAVE_EXECLP
#cmakedefine HAVE_WAITPID
#cmakedefine HAVE_BACKTRACE
#cmakedefine HAVE_ISATTY
#cmakedefine HAVE_WORDEXP_H
#cmakedefine HAVE_FPATHCONF
#cmakedefine HAVE_PATHCONF
#cmakedefine HAVE_PC_NAME_MAX
#cmakedefine HAVE_NAME_MAX
#cmakedefine HAVE_SCHED_GETSCHEDULER
#cmakedefine HAVE_SYS_STAT_H
#cmakedefine HAVE_UNAME
#cmakedefine HAVE_GETRUSAGE
#cmakedefine HAVE_POPEN
#cmakedefine HAVE_PCLOSE
#cmakedefine HAVE_SYSCONF
#cmakedefine HAVE_SIGACTION
#cmakedefine HAVE_DIRFD
#cmakedefine HAVE_FSTATAT

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

#endif // ARX_CONFIGURE_H
