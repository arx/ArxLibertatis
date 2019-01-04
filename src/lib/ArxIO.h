/*
 * Copyright 2013-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_LIB_ARXIO_H
#define ARX_LIB_ARXIO_H

#include <stddef.h>

#if defined _WIN32 || defined __CYGWIN__
	#ifdef __GNUC__
		#define ARX_LIB_PUBLIC __attribute__ ((dllexport))
	#else
		#define ARX_LIB_PUBLIC __declspec(dllexport)
	#endif
#else
	#if __GNUC__ >= 4
		#define ARX_LIB_PUBLIC __attribute__ ((visibility ("default")))
	#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

ARX_LIB_PUBLIC void ArxIO_init();

ARX_LIB_PUBLIC void ArxIO_getError(char * outMessage, int size);
ARX_LIB_PUBLIC int  ArxIO_getLogLine(char * outMessage, int size);

ARX_LIB_PUBLIC void ArxIO_unpack_alloc(const char * in, size_t inSize, char ** out, size_t * outSize);
ARX_LIB_PUBLIC void ArxIO_unpack_free(char * buffer);

#ifdef __cplusplus
}
#endif

#endif // ARX_LIB_ARXIO_H
