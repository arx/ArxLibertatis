
#ifndef ARX_AUDIO_ALWRAPPER_H
#define ARX_AUDIO_ALWRAPPER_H

#if defined(__WINE__) && defined(_WIN32)
	#define _OLD_WIN32 1
	#undef _WIN32
#endif

#include <al.h>
#include <alc.h>
#include <efx.h>

#ifdef _OLD_WIN32
	#define _WIN32 1
#endif

#endif // ARX_AUDIO_ALWRAPPER_H
