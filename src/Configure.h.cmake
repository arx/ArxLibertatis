
#ifndef ARX_CONFIGURE_H
#define ARX_CONFIGURE_H

#cmakedefine HAVE_WINAPI

// Threads
#cmakedefine HAVE_PTHREADS

// Timing
#cmakedefine HAVE_NANOSLEEP_FUNC // HAVE_NANOSLEEP conflicts with SDL
#cmakedefine HAVE_CLOCK_GETTIME

// Audio backend
#cmakedefine HAVE_OPENAL
#cmakedefine HAVE_OPENAL_EFX
#cmakedefine HAVE_DSOUND

// Renderer backend
#cmakedefine HAVE_OPENGL
#cmakedefine HAVE_D3D7

// Input backend
#cmakedefine HAVE_SDL
#cmakedefine HAVE_DINPUT7

// Debugging features
#cmakedefine HAVE_UNISTD_H_
#cmakedefine HAVE_SIGNAL_H

// Compiler features
#cmakedefine HAVE_DYNAMIC_STACK_ALLOCATION

#cmakedefine BUILD_EDITOR

#cmakedefine BUILD_EDIT_LOADSAVE

#endif // ARX_CONFIGURE_H
