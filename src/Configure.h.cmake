
#ifndef ARX_CONFIGURE_H
#define ARX_CONFIGURE_H

#cmakedefine HAVE_WINAPI

// Threads
#cmakedefine HAVE_PTHREADS

// Timing
#cmakedefine HAVE_NANOSLEEP
#cmakedefine HAVE_CLOCK_GETTIME

// Audio backend
#cmakedefine HAVE_OPENAL
#cmakedefine HAVE_OPENAL_EFX
#cmakedefine HAVE_DSOUND

// Compiler features
#cmakedefine HAVE_DYNAMIC_STACK_ALLOCATION

#endif // ARX_CONFIGURE_H
