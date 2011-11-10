
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

// Arx components
#cmakedefine BUILD_EDITOR
#cmakedefine BUILD_EDIT_LOADSAVE

// Build system
#cmakedefine UNITY_BUILD

#endif // ARX_CONFIGURE_H
