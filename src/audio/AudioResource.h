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
#include <vector>

#include <boost/noncopyable.hpp>

#include "audio/AudioTypes.h"
#include "platform/Platform.h"

class PakFileHandle;

namespace res { class path; }

namespace audio {

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

template <typename T, typename Handle = size_t>
class ResourceList : private boost::noncopyable {
	
	template <typename Tag, typename IndexType, IndexType InvalidValue>
	static size_t get(HandleType<Tag, IndexType, InvalidValue> handle) {
		return size_t(handle.handleData());
	}
	
	template <typename IndexType>
	static size_t get(IndexType index) {
		return size_t(index);
	}
	
public:
	
	typedef T * const * iterator;
	typedef const T * const * const_iterator;
	
	ResourceList() { }
	~ResourceList() { clear(); }
	
	bool isValid(Handle handle) const {
		return (get(handle) < m_list.size() && m_list[get(handle)]);
	}
	
	T * operator[](Handle handle) { return m_list[get(handle)]; }
	const T * operator[](Handle handle) const { return m_list[get(handle)]; }
	size_t size() const { return m_list.size(); }

	Handle add(T * element);
	void remove(Handle handle);
	void clear();
	
	iterator begin() { return &m_list[0]; }
	iterator end() { return &m_list[0] + m_list.size(); }
	const_iterator begin() const { return &m_list[0]; }
	const_iterator end() const { return &m_list[0] + m_list.size(); }
	
	iterator remove(iterator i);
	
private:
	
	std::vector<T *> m_list;
	
};

template <typename T, typename Handle>
Handle ResourceList<T, Handle>::add(T * element) {
	
	for(size_t i = 0; i < m_list.size(); i++) {
		if(!m_list[i]) {
			m_list[i] = element;
			return Handle(i);
		}
	}
	
	m_list.push_back(element);
	
	return Handle(m_list.size() - 1);
}

template <typename T, typename Handle>
void ResourceList<T, Handle>::remove(Handle handle) {
	
	if(!isValid(handle)) {
		return;
	}
	
	size_t i = get(handle);
	delete m_list[i];
	m_list[i] = NULL;
	
	for(size_t j = i + 1; j < m_list.size(); j++) {
		if(m_list[j]) {
			return;
		}
	}
	
	m_list.resize(i);
	
}

template <typename T, typename Handle>
void ResourceList<T, Handle>::clear() {
	
	for(size_t i = 0; i < m_list.size(); i++) {
		delete m_list[i];
	}
	
	m_list.clear();
	
}

template <typename T, typename Handle>
typename ResourceList<T, Handle>::iterator ResourceList<T, Handle>::remove(iterator i) {
	
	size_t index = i - begin();
	
	remove(Handle(index));
	if(index >= m_list.size()) {
		return end();
	}
	
	return begin() + index + 1;
}

} // namespace audio

#endif // ARX_AUDIO_AUDIORESOURCE_H
