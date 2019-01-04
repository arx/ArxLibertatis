/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:

  blast.h -- interface for blast.c
  Copyright (C) 2003 Mark Adler
  version 1.1, 16 Feb 2003

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the author be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Mark Adler    madler@alumni.caltech.edu
 */

#ifndef ARX_IO_BLAST_H
#define ARX_IO_BLAST_H

#include <stddef.h>

#include <string>

/*
 * blast() decompresses the PKWare Data Compression Library (DCL) compressed
 * format.  It provides the same functionality as the explode() function in
 * that library.  (Note: PKWare overused the "implode" verb, and the format
 * used by their library implode() function is completely different and
 * incompatible with the implode compression method supported by PKZIP.)
 */

/* Definitions for input/output functions passed to blast().  See below for
 * what the provided functions need to do.
 */
typedef size_t (*blast_in)(void * how, const unsigned char ** buf);
typedef int (*blast_out)(void * how, unsigned char * buf, size_t len);

enum BlastResult {
	BLAST_TRUNCATED_INPUT = 2, // ran out of input before completing decompression
	BLAST_OUTPUT_ERROR = 1, // output error before completing decompression
	BLAST_SUCCESS = 0, // successful decompression
	BLAST_INVALID_LITERAL_FLAG = -1, // literal flag not zero or one
	BLAST_INVALID_DIC_SIZE = -2, // dictionary size not in 4..6
	BLAST_INVALID_OFFSET = -3, // distance is too far back
};

/*! Decompress input to output using the provided infun() and outfun() calls.
 * On success, the return value of blast() is zero.  If there is an error in
 * the source data, i.e. it is not in the proper format, then a negative value
 * is returned.  If there is not enough input available or there is not enough
 * output space, then a positive error is returned.
 *
 * The input function is invoked: len = infun(how, &buf), where buf is set by
 * infun() to point to the input buffer, and infun() returns the number of
 * available bytes there.  If infun() returns zero, then blast() returns with
 * an input error.  (blast() only asks for input if it needs it.)  inhow is for
 * use by the application to pass an input descriptor to infun(), if desired.
 *
 * The output function is invoked: err = outfun(how, buf, len), where the bytes
 * to be written are buf[0..len-1].  If err is not zero, then blast() returns
 * with an output error.  outfun() is always called with len <= 4096.  outhow
 * is for use by the application to pass an output descriptor to outfun(), if
 * desired.
 *
 * At the bottom of blast.c is an example program that uses blast() that can be
 * compiled to produce a command-line decompression filter by defining TEST.
 */
BlastResult blast(blast_in infun, void * inhow, blast_out outfun, void * outhow);

// Convenience implementations.

struct BlastMemOutBuffer {
	
	char * buf;
	
	size_t size;
	
	BlastMemOutBuffer(char * b, size_t s) : buf(b), size(s) { }
	
};

struct BlastMemInBuffer {
	
	const char * buf;
	
	size_t size;
	
	BlastMemInBuffer(const char * b, size_t s) : buf(b), size(s) { }
	
};

struct BlastMemOutString {
	
	std::string & buffer;
	
	explicit BlastMemOutString(std::string & destination) : buffer(destination) { }
	
};

/*!
 * Writes data to a BlastMemOutBuffer.
 * Advances the buf pointer and decreases size.
 */
int blastOutMem(void * param, unsigned char * buf, size_t len);

/*!
 * Reads data from a BlastMemInBuffer.
 * Advances the buf pointer and decrises size.
 */
size_t blastInMem(void * param, const unsigned char ** buf);

/*!
 * Writes data to a BlastMemOutString.
 */
int blastOutString(void * param, unsigned char * buf, size_t len);

/*!
 * Decompress data.
 */
std::string blast(const char * from, size_t fromSize, size_t toSizeHint = size_t(-1));

/*!
 * Decompress data.
 */
inline std::string blast(const std::string & from, size_t toSizeHint = size_t(-1)) {
	return blast(from.data(), from.size(), toSizeHint);
}

#endif // ARX_IO_BLAST_H
