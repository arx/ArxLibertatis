
#ifndef ARX_AUDIO_OPENAL_OPENALUTILS_H
#define ARX_AUDIO_OPENAL_OPENALUTILS_H

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

#endif // ARX_AUDIO_OPENAL_OPENALUTILS_H
