/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_AUDIO_OPENAL_OPENALUTILS_H
#define ARX_AUDIO_OPENAL_OPENALUTILS_H

#include <boost/math/special_functions/fpclassify.hpp>

#include <al.h>

#include "math/Vector.h"

const char * getAlcErrorString(ALenum error);

const char * getAlErrorString(ALenum error);

#define AL_CHECK_ERROR_C(desc, todo) { ALenum error = alGetError(); \
	if(error != AL_NO_ERROR) { \
		ALError << "Error " desc << ": " << error << " = " << getAlErrorString(error); \
		todo \
	}}

#define AL_CHECK_ERROR_N(desc) AL_CHECK_ERROR_C(desc, (void)0;)
#define AL_CHECK_ERROR(desc) AL_CHECK_ERROR_C(desc, return AAL_ERROR_SYSTEM;)

#endif // ARX_AUDIO_OPENAL_OPENALUTILS_H
