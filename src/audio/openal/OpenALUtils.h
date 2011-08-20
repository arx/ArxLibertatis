
#ifndef ARX_AUDIO_OPENAL_OPENALUTILS_H
#define ARX_AUDIO_OPENAL_OPENALUTILS_H

#include <boost/math/special_functions/fpclassify.hpp>

#include <al.h>

#include "math/Vector3.h"

const char * getAlcErrorString(ALenum error);

const char * getAlErrorString(ALenum error);

#define AL_CHECK_ERROR(desc) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error << " = " << getAlErrorString(error); \
		return AAL_ERROR_SYSTEM; \
	}}

#define AL_CHECK_ERROR_N(desc, todo) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error << " = " << getAlErrorString(error); \
		todo \
	}}

template <class T>
inline bool isallfinite(Vector3<T> vec) {
	return (boost::math::isfinite)(vec.x) && (boost::math::isfinite)(vec.y)  && (boost::math::isfinite)(vec.z);
}

#endif // ARX_AUDIO_OPENAL_OPENALUTILS_H
