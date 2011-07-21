
#ifndef ARX_AUDIO_OPENAL_OPENALUTILS_H
#define ARX_AUDIO_OPENAL_OPENALUTILS_H

#include <boost/math/special_functions/fpclassify.hpp>

#include "math/Vector3.h"

#define AL_CHECK_ERROR(desc) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error; \
		return AAL_ERROR_SYSTEM; \
	}}

#define AL_CHECK_ERROR_N(desc, todo) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "error " desc ": " << error; \
		todo \
	}}

template <class T>
inline bool isallfinite(Vector3<T> vec) {
	return (boost::math::isfinite)(vec.x) && (boost::math::isfinite)(vec.y)  && (boost::math::isfinite)(vec.z);
}

#endif // ARX_AUDIO_OPENAL_OPENALUTILS_H
