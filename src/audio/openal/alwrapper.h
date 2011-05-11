
#ifndef ARX_AUDIO_ALWRAPPER_H
#define ARX_AUDIO_ALWRAPPER_H

#if defined(__WINE__) && defined(_WIN32)
	// We link to a native OpenAL when building with winelib.
	#define _OLD_WIN32 1
	#undef _WIN32
#endif

#include "Configure.h"

#include <al.h>
#include <alc.h>
#ifdef HAVE_OPENAL_EFX
	#include <efx.h>
#endif

#ifdef _OLD_WIN32
	#define _WIN32 1
#endif

#define AL_CHECK_ERROR(desc) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error; \
		return AAL_ERROR_SYSTEM; \
	}} \

#define AL_CHECK_ERROR_N(desc, todo) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error; \
		todo \
	}} \

#endif // ARX_AUDIO_ALWRAPPER_H
