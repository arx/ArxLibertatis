/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Alignment.h"

#include "Configure.h"

#if ARX_HAVE_ALIGNED_MALLOC && ARX_HAVE_ALIGNED_FREE
#include <malloc.h>
#elif ARX_HAVE_ALIGNED_ALLOC || ARX_HAVE_POSIX_MEMALIGN
#include <stdlib.h>
#else
#include <cstdlib>
#endif

namespace platform {

#if ARX_HAVE_ALIGNED_MALLOC && ARX_HAVE_ALIGNED_FREE

arx_nodiscard void * alloc_aligned(std::size_t alignment, std::size_t size) {
	// alignment needs to be a power of two
	return _aligned_malloc(size, alignment);
}

void free_aligned(void * ptr) {
	_aligned_free(ptr);
}

#elif ARX_HAVE_ALIGNED_ALLOC

arx_nodiscard void * alloc_aligned(std::size_t alignment, std::size_t size) {
	
	// alignment needs to be a power of two
	// size needs to be a multiple of alignment
	if((size & (alignment - 1)) != 0) {
		size += alignment - (size & (alignment - 1));
	}
	
	return aligned_alloc(alignment, size);
}

void free_aligned(void * ptr) {
	free(ptr);
}

#elif ARX_HAVE_POSIX_MEMALIGN

arx_nodiscard void * alloc_aligned(std::size_t alignment, std::size_t size) {
	
	// alignment needs to be a power of two
	// alignment needs to be a multiple of sizeof(void *)
	void * ptr = 0;
	if(posix_memalign(&ptr, alignment, size)) {
		ptr = 0;
	}
	
	return ptr;
}

void free_aligned(void * ptr) {
	free(ptr);
}

#else

arx_nodiscard void * alloc_aligned(std::size_t alignment, std::size_t size) {
	
	if(alignment < GuaranteedAlignment) {
		alignment = GuaranteedAlignment;
	}
	
	unsigned char * allocation = static_cast<unsigned char *>(std::malloc(size + alignment));
	if(!allocation) {
		return NULL;
	}
	
	size_t offset = alignment - (allocation - static_cast<unsigned char *>(0)) % alignment;
	
	arx_assert(offset - 1 <= 0xff);
	allocation[offset - 1] = static_cast<unsigned char>(offset - 1);
	
	return allocation + offset;
}

void free_aligned(void * ptr) {
	if(ptr) {
		unsigned char * data = static_cast<unsigned char *>(ptr) - 1;
		std::free(data - *data);
	}
}

#endif

} // namespace platform
