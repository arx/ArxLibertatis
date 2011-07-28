
#ifndef ARX_CONFIGURE_H
#define ARX_CONFIGURE_H

#cmakedefine HAVE_WINAPI

// Filesystem
#cmakedefine HAVE_BOOST_FILESYSTEM_V3
#cmakedefine HAVE_POSIX_FILESYSTEM

// Threads
#cmakedefine HAVE_PTHREADS

// Timing
#cmakedefine HAVE_NANOSLEEP
#cmakedefine HAVE_CLOCK_GETTIME

// Audio backend
#cmakedefine HAVE_OPENAL
#cmakedefine HAVE_OPENAL_EFX
#cmakedefine HAVE_DSOUND

#cmakedefine HAVE_OPENGL

// Compiler features
#cmakedefine HAVE_DYNAMIC_STACK_ALLOCATION

#cmakedefine BUILD_EDITOR

#cmakedefine BUILD_EDIT_LOADSAVE

#endif // ARX_CONFIGURE_H
