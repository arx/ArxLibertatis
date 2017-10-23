/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/

#ifndef ARX_AUDIO_AUDIORESOURCE_H
#define ARX_AUDIO_AUDIORESOURCE_H

#include <stddef.h>
#include <cstdlib>
#include <cstring>

#include "audio/AudioTypes.h"
#include "platform/Platform.h"

class PakFileHandle;

namespace res { class path; }

namespace audio {

PakFileHandle * OpenResource(const res::path & name, const res::path & resource_path);

class ResourceHandle {
	
public:
	
	ResourceHandle() {
		reference_count_ = 0;
	}
	virtual ~ResourceHandle() { }
	
	void reference() {
		++reference_count_;
	}
	void dereference() {
		arx_assert(reference_count_ > 0);
		--reference_count_;
	}
	s32 isReferenced() {
		return reference_count_;
	}
	
private:
	
	s32 reference_count_;
	
};

template <class T>
class ResourceList {
	
public:
	
	static const size_t ALIGNMENT = 16;
	
	typedef T * const * iterator;
	
	ResourceList();
	~ResourceList();
	
	bool isValid(s32 index) { return ((size_t)index < _size && list[index]); }
	T * operator[](s32 index) { return list[index]; }
	size_t size() { return _size; }
	s32 add(T * element);
	void remove(s32 index);
	void clear();
	
	iterator begin() { return list; }
	iterator end() { return list + _size; }
	iterator remove(iterator i);
	
private:
	
	size_t _size;
	T ** list;
	
};

template <class T>
inline ResourceList<T>::ResourceList() : _size(0), list(NULL) { }

template <class T>
inline ResourceList<T>::~ResourceList() {
	clear();
}

template <class T>
inline s32 ResourceList<T>::add(T * element) {
	
	size_t i = 0;
	for(; i < _size; i++) {
		if(!list[i]) {
			list[i] = element;
			return i;
		}
	}
	
	void * ptr = std::realloc(list, (_size + ALIGNMENT) * sizeof(*list));
	if(!ptr) {
		return INVALID_ID;
	}
	
	list = (T **)ptr, _size += ALIGNMENT;
	
	std::memset(&list[i], 0, ALIGNMENT * sizeof(*list));
	list[i] = element;
	
	return i;
}

template <class T>
inline void ResourceList<T>::remove(s32 index) {
	
	if((size_t)index >= _size || !list[index]) {
		return;
	}
	
	T * toDelete = list[index];
	list[index] = NULL;
	
	if(_size <= ALIGNMENT) {
		delete toDelete;
		return;
	}
	
	for(size_t j(_size - ALIGNMENT); j < _size; j++) {
		if(list[j]) {
			delete toDelete;
			return;
		}
	}
	
	list = (T **)std::realloc(list, (_size -= ALIGNMENT) * sizeof(*list));
	delete toDelete;
}

template <class T>
inline void ResourceList<T>::clear() {
	
	for(size_t i = 0; i < _size; i++) {
		if(list[i]) {
			T * toDelete = list[i];
			list[i] = NULL;
			delete toDelete;
		}
	}
	
	std::free(list);
	list = NULL;
	_size = 0;
}

template <class T>
inline typename ResourceList<T>::iterator ResourceList<T>::remove(iterator i) {
	size_t idx = i - begin();
	remove(idx);
	if(idx >= _size) {
		return end();
	}
	return begin() + idx + 1;
}

} // namespace audio

#endif // ARX_AUDIO_AUDIORESOURCE_H
