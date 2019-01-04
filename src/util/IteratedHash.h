/*
 * Copyright 2017-2018 Arx Libertatis Team (see the AUTHORS file)
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
 * Generic hashing utilities.
 */
#ifndef ARX_UTIL_ITERATEDHASH_H
#define ARX_UTIL_ITERATEDHASH_H

// Taken from Crypto++ and modified to fit the project.

#include <cstring>
#include <string>
#include <ostream>
#include <iomanip>

#include <boost/range/size.hpp>

#include "platform/Alignment.h"
#include "platform/Platform.h"

namespace util {

namespace detail {

template <bool overflow>
struct safe_shifter {
	
	template <class T>
	static T right_shift(T /* value */, unsigned int /* bits */) {
		return 0;
	}
	
};

template <>
struct safe_shifter<false> {
	
	template <class T>
	static T right_shift(T value, unsigned int bits) {
		return value >> bits;
	}
	
};

} // namespace detail

//! Right-shift a value without shifting past the size of the type or return 0.
template <unsigned int bits, class T>
T safe_right_shift(T value) {
	return detail::safe_shifter<(bits >= (8 * sizeof(T)))>::right_shift(value, bits);
}

template <size_t N>
struct checksum {
	char data[N];
};

template <class T>
class iterated_hash {
	
public:
	
	typedef T transform;
	typedef typename transform::hash_word hash_word;
	typedef typename transform::byte_order byte_order;
	static const size_t block_size = transform::block_size;
	static const size_t hash_size = transform::hash_size / sizeof(hash_word);
	static const size_t size = transform::hash_size;
	typedef util::checksum<size> checksum;
	
	void init() { count_lo = count_hi = 0; transform::init(state); }
	
	void update(const char * input, size_t length);
	
	void finalize(char * result);
	
	checksum finalize() {
		checksum result;
		finalize(result.data);
		return result;
	}
	
	static checksum compute(const char * input, size_t length) {
		iterated_hash<T> hasher;
		hasher.init();
		hasher.update(input, length);
		return hasher.finalize();
	}
	
	static checksum compute(const std::string & input) {
		return compute(input.data(), input.length());
	}
	
private:

	size_t hash(const char * input, size_t length);
	void pad(size_t last_block_size, char pad_first = '\x80');
	
	hash_word bit_count_hi() const {
		return (count_lo >> (8 * sizeof(hash_word) - 3)) + (count_hi << 3);
	}
	hash_word bit_count_lo() const { return count_lo << 3; }
	
	char data[block_size];
	hash_word state[hash_size];
	
	hash_word count_lo, count_hi;
	
};

template <class T>
void iterated_hash<T>::update(const char * input, size_t length) {
	
	hash_word old_count_lo = count_lo;
	
	if((count_lo = old_count_lo + hash_word(length)) < old_count_lo) {
		count_hi++; // carry from low to high
	}
	
	count_hi += hash_word(util::safe_right_shift<8 * sizeof(hash_word)>(length));
	
	size_t num = old_count_lo % size_t(block_size);
	
	if(num != 0) { // process left over data
		if(num + length >= block_size) {
			std::memcpy(data + num, input, block_size - num);
			hash(data, block_size);
			input += (block_size - num);
			length -= (block_size - num);
			// drop through and do the rest
		} else {
			std::memcpy(data + num, input, length);
			return;
		}
	}
	
	// now process the input data in blocks of BlockSize bytes and save the leftovers to m_data
	if(length >= block_size) {
		size_t leftOver = hash(input, length);
		input += (length - leftOver);
		length = leftOver;
	}
	
	if(length) {
		memcpy(data, input, length);
	}
}

template <class T>
size_t iterated_hash<T>::hash(const char * input, size_t length) {
	
	if(byte_order::native() && platform::is_aligned<T>(input)) {
		
		do {
			
			transform::transform(state, reinterpret_cast<const hash_word *>(input));
			
			input += block_size;
			length -= block_size;
			
		} while(length >= block_size);
		
	} else {
		
		do {
			
			hash_word buffer[block_size / sizeof(hash_word)];
			byte_order::load(input, buffer, size_t(boost::size(buffer)));
			
			transform::transform(state, buffer);
			
			input += block_size;
			length -= block_size;
			
		} while(length >= block_size);
		
	}
	
	return length;
}

template <class T>
void iterated_hash<T>::pad(size_t last_block_size, char pad_first) {
	
	size_t num = count_lo % size_t(block_size);
	
	data[num++] = pad_first;
	
	if(num <= last_block_size) {
		memset(data + num, 0, last_block_size - num);
	} else {
		memset(data + num, 0, block_size - num);
		hash(data, block_size);
		memset(data, 0, last_block_size);
	}
}

template <class T>
void iterated_hash<T>::finalize(char * result) {
	
	size_t order = transform::offset * sizeof(hash_word);
	
	size_t padSize = block_size - 2 * sizeof(hash_word);
	pad(padSize);
	byte_order::store(bit_count_lo(), data + padSize + order);
	byte_order::store(bit_count_hi(), data + padSize + sizeof(hash_word) - order);
	
	hash(data, block_size);
	
	byte_order::store(state, hash_size, result);
	
}

template <size_t N>
inline std::ostream & operator<<(std::ostream & os, const checksum<N> & c) {
	
	std::ios_base::fmtflags old = os.flags();
	char oldfill = os.fill('0');
	
	os << std::hex;
	for(size_t i = 0; i < sizeof(c.data); i++) {
		os << std::setw(2) << int(u8(c.data[i]));
	}
	
	os.fill(oldfill);
	os.setf(old, std::ios_base::basefield);
	return os;
}

} // namespace util

#endif // ARX_UTIL_ITERATEDHASH_H

