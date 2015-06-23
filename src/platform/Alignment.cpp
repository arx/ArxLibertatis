/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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
#else
#include <stdlib.h>
#endif

namespace platform {

void * alloc_aligned(std::size_t alignment, std::size_t size) {
	void * ptr;
	#if ARX_HAVE_ALIGNED_MALLOC && ARX_HAVE_ALIGNED_FREE
	// alignment needs to be a power of two
	ptr = _aligned_malloc(size, alignment);
	#elif ARX_HAVE_ALIGNED_ALLOC
	// alignment needs to be a power of two
	// size needs to be a multiple of alignment
	if((size & (alignment - 1)) != 0) {
		size += alignment - (size & (alignment - 1));
	}
	ptr = aligned_alloc(alignment, size);
	#elif ARX_HAVE_POSIX_MEMALIGN
	// alignment needs to be a power of two
	// alignment needs to be a multiple of sizeof( void *)
	ptr = 0;
	if(posix_memalign(&ptr, alignment, size)) {
		ptr = 0;
	}
	#else
	#error "No aligned memory allocator available!"
	#endif
	if(!ptr) {
		throw std::bad_alloc();
	}
	return ptr;
}

void free_aligned(void * ptr) {
	#if ARX_HAVE_ALIGNED_MALLOC && ARX_HAVE_ALIGNED_FREE
	_aligned_free(ptr);
	#else
	free(ptr);
	#endif
}

} // namespace platform
