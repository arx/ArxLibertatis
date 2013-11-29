/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#if defined _WIN32 || defined __CYGWIN__
	#ifdef __GNUC__
		#define LIB_PUBLIC __attribute__ ((dllexport))
	#else
		#define LIB_PUBLIC __declspec(dllexport)
	#endif
#else
	#if __GNUC__ >= 4
		#define LIB_PUBLIC __attribute__ ((visibility ("default")))
	#endif
#endif

extern "C" {

LIB_PUBLIC void ArxIO_init();

LIB_PUBLIC void ArxIO_getError(char * outMessage, int size);
LIB_PUBLIC int  ArxIO_getLogLine(char * outMessage, int size);

LIB_PUBLIC int  ArxIO_ftlLoad(char * filePath);
LIB_PUBLIC int  ArxIO_ftlRelease();
LIB_PUBLIC int  ArxIO_ftlGetRawDataSize();
LIB_PUBLIC int  ArxIO_ftlGetRawData(char * outData, int size);

}

#endif // ARX_LIB_ARXIO_H
