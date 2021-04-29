/*
 * Copyright (C) 2011-2015 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/*!
 * \file
 *
 * Utility functions for dealing with different endiannesses.
 */
#ifndef ARX_PLATFORM_ENDIAN_H
#define ARX_PLATFORM_ENDIAN_H

#include <cstdlib>
#include <cstring>

#include "Configure.h"

#if ARX_HAVE_BSWAP_16 || ARX_HAVE_BSWAP_32 || ARX_HAVE_BSWAP_64
#include <byteswap.h>
#endif

#include "platform/Platform.h"

namespace platform {

namespace detail {

inline u8 byteswap(u8 value) {
	return value;
}

inline s8 byteswap(s8 value) {
	return s8(byteswap(u8(value)));
}

inline u16 byteswap(u16 value) {
#if ARX_HAVE_BUILTIN_BSWAP16
	return __builtin_bswap16(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_ushort(value);
#elif ARX_HAVE_BSWAP_16 \
	&& (!defined(__GLIBC__) || !defined(__GLIBC_MINOR__) \
	    || (__GLIBC__ << 16) + __GLIBC_MINOR__ >= (2 << 16) + 16) \
	    // prevent conversion warnings
	return bswap_16(value);
#else
	return u16((u16(u8(value)) << 8) | u8(value >> 8));
#endif
}

inline s16 byteswap(s16 value) {
	return s16(byteswap(u16(value)));
}

inline u32 byteswap(u32 value) {
#if ARX_HAVE_BUILTIN_BSWAP32
	return __builtin_bswap32(value);
#elif defined(_MSC_VER) && (_MSC_VER >= 1400 || (_MSC_VER >= 1300 && !defined(_DLL)))
	return _byteswap_ulong(value);
#elif ARX_HAVE_BSWAP_32
	return bswap_32(value);
#else
	return (u32(byteswap(u16(value))) << 16) | byteswap(u16(value >> 16));
#endif
}

inline s32 byteswap(s32 value) {
	return s32(byteswap(u32(value)));
}

inline u64 byteswap(u64 value) {
#if ARX_HAVE_BUILTIN_BSWAP64
	return __builtin_bswap64(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_uint64(value);
#elif ARX_HAVE_BSWAP_64
	return bswap_64(value);
#else
	return (u64(byteswap(u32(value))) << 32) | byteswap(u32(value >> 32));
#endif
}

inline s64 byteswap(s64 value) {
	return s64(byteswap(u64(value)));
}

} // namespace detail

//! Load/store functions for a specific endianness.
template <typename Endianness>
struct endianness {
	
	/*!
	 * Load a single integer.
	 *
	 * \param buffer Memory location containing the integer. Will read sizeof(T) bytes.
	 * \return the loaded integer.
	 */
	template <typename T>
	static T load(const char * buffer) {
		if(Endianness::native()) {
			T value;
			std::memcpy(&value, buffer, sizeof(value));
			return value;
		} else {
			return load_alien<T>(buffer);
		}
	}
	
	/*!
	 * Load an array of integers.
	 *
	 * \param buffer Memory location containing the integers (without padding).
	 *               Will read <code>sizeof(T) * count</code> bytes.
	 * \param values Output array for the loaded integers.
	 * \param count  How many integers to load.
	 */
	template <typename T>
	static void load(const char * buffer, T * values, size_t count) {
		if(Endianness::native() || sizeof(*values) == 1) {
			std::memcpy(values, buffer, sizeof(*values) * count);
		} else {
			for(size_t i = 0; i < count; i++, buffer += sizeof(*values)) {
				values[i] = load_alien<T>(buffer);
			}
		}
	}
	
	/*!
	 * Store a single integer.
	 *
	 * \param value  The integer to store.
	 * \param buffer Memory location to receive the integer. Will write sizeof(T) bytes.
	 */
	template <typename T>
	static void store(T value, char * buffer) {
		if(Endianness::native()) {
			std::memcpy(buffer, &value, sizeof(value));
		} else {
			return store_alien(value, buffer);
		}
	}
	
	/*!
	 * Store an array of integers.
	 *
	 * \param values The integers to store.
	 * \param count  How many integers to store.
	 * \param buffer Memory location to receive the integers (without padding).
	 *               Will write <code>sizeof(T) * count</code> bytes.
	 */
	template <typename T>
	static void store(T * values, size_t count, char * buffer) {
		if(Endianness::native() || sizeof(*values) == 1) {
			std::memcpy(buffer, values, sizeof(*values) * count);
		} else {
			for(size_t i = 0; i < count; i++, buffer += sizeof(*values)) {
				store_alien(values[i], buffer);
			}
		}
	}
	
private:
	
	bool reversed() { return false; }
	
	template <typename T>
	static T load_alien(const char * buffer) {
		if(Endianness::reversed()) {
			T value;
			std::memcpy(&value, buffer, sizeof(value));
			return detail::byteswap(value);
		} else {
			return Endianness::template decode<T>(buffer);
		}
	}
	
	template <typename T>
	static void store_alien(T value, char * buffer) {
		if(Endianness::reversed()) {
			value = detail::byteswap(value);
			std::memcpy(buffer, &value, sizeof(value));
		} else {
			Endianness::template encode<T>(value, buffer);
		}
	}
	
};

namespace detail {

inline bool is_little_endian() {
	u32 signature = 0x04030201;
	return (*reinterpret_cast<char *>(&signature) == 1);
}

inline bool is_big_endian() {
	u32 signature = 0x04030201;
	return (*reinterpret_cast<char *>(&signature) == 4);
}

} // namespace detail


//! Load and store little-endian integers.
struct little_endian : endianness<little_endian> {
	
	//! \return true if we are running on a little-endian machine.
	static bool native() { return detail::is_little_endian(); }
	
private:
	
	static bool reversed() { return detail::is_big_endian(); }
	
	template <typename T>
	static T decode(const char * buffer) {
		T value = 0;
		for(size_t i = 0; i < sizeof(T); i++) {
			value = T(value | (T(buffer[i]) << (i * 8)));
		}
		return value;
	}
	
	template <typename T>
	static void encode(T value, char * buffer) {
		for(size_t i = 0; i < sizeof(T); i++) {
			buffer[i] = char((value >> (i * 8)) & 0xff);
		}
	}
	
	friend struct endianness<little_endian>;
};

//! Load and store big-endian integers.
struct big_endian : endianness<big_endian> {
	
	//! \return true if we are running on a big-endian machine.
	static bool native() { return detail::is_big_endian(); }
	
private:
	
	static bool reversed() { return detail::is_little_endian(); }
	
	template <typename T>
	static T decode(const char * buffer) {
		T value = 0;
		for(size_t i = 0; i < sizeof(T); i++) {
			value = T(value | T(buffer[i]) << ((sizeof(T) - i - 1) * 8));
		}
		return value;
	}
	
	template <typename T>
	static void encode(T value, char * buffer) {
		for(size_t i = 0; i < sizeof(T); i++) {
			buffer[i] = char((value >> ((sizeof(T) - i - 1) * 8)) & 0xff);
		}
	}
	
	friend struct endianness<big_endian>;
};

} // namespace platform

#endif // ARX_PLATFORM_ENDIAN_H

