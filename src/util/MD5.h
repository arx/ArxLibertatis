/*
 * Copyright 2017 Arx Libertatis Team (see the AUTHORS file)
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

/*!
 * \file
 *
 * MD5 hashing routines.
 */
#ifndef ARX_UTIL_MD5_H
#define ARX_UTIL_MD5_H

#include "platform/Endian.h"
#include "platform/Platform.h"
#include "util/IteratedHash.h"

namespace util {

class md5_transform {
	
public:
	
	typedef u32 hash_word;
	typedef platform::little_endian byte_order;
	static const size_t offset = 0;
	static const size_t block_size = 64;
	static const size_t hash_size = 16;
	
	static void init(hash_word * state);
	
	static void transform(hash_word * digest, const hash_word * data);
};

typedef iterated_hash<md5_transform> md5;

} // namespace util

#endif // ARX_UTIL_MD5_H

