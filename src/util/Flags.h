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
// Inspired by QFlags from Qt

#ifndef ARX_UTIL_FLAGS_H
#define ARX_UTIL_FLAGS_H

#include "platform/Platform.h"

/*!
 * A typesafe way to define flags as a combination of enum values.
 * 
 * This type should not be used directly, only through DECLARE_FLAGS.
 */
template <typename Enum_>
class Flags {
	
	typedef void ** Zero;
	u32 m_flags;
	
	Flags(u32 flags, bool dummy) : m_flags(flags) { ARX_UNUSED(dummy); }
	
public:
	
	typedef Enum_ Enum;
	
	/* implicit */ Flags(Enum flag) : m_flags(flag) { }
	
	/* implicit */ Flags(Zero /* zero */ = 0) : m_flags(0) { }
	
	static Flags load(u32 flags) {
		return Flags(flags, true);
	}
	
	bool has(Enum flag) const {
		return !!(m_flags & u32(flag));
	}
	
	bool hasAll(Flags o) const {
		return (m_flags & o.m_flags) == o.m_flags;
	}
	
	Flags except(Enum flag) const {
		Flags r;
		r.m_flags = m_flags & ~u32(flag);
		return r;
	}
	
	void remove(Enum flag) {
		m_flags &= ~u32(flag);
	}
	
	operator u32() const {
		return m_flags;
	}
	
	Flags operator~() const {
		Flags r;
		r.m_flags = ~m_flags;
		return r;
	}
	
	bool operator!() const {
		return (m_flags == 0);
	}
	
	Flags operator&(Flags o) const {
		Flags r;
		r.m_flags = m_flags & o.m_flags;
		return r;
	}
	
	Flags operator|(Flags o) const {
		Flags r;
		r.m_flags = m_flags | o.m_flags;
		return r;
	}
	
	Flags operator^(Flags o) const {
		Flags r;
		r.m_flags = m_flags ^ o.m_flags;
		return r;
	}
	
	Flags & operator&=(Flags o) {
		m_flags &= o.m_flags;
		return *this;
	}
	
	Flags & operator|=(Flags o) {
		m_flags |= o.m_flags;
		return *this;
	}
	
	Flags & operator^=(Flags o) {
		m_flags ^= o.m_flags;
		return *this;
	}
	
	Flags operator&(Enum flag) const {
		Flags r;
		r.m_flags = m_flags & u32(flag);
		return r;
	}
	
	Flags operator|(Enum flag) const {
		Flags r;
		r.m_flags = m_flags | u32(flag);
		return r;
	}
	
	Flags operator^(Enum flag) const {
		Flags r;
		r.m_flags = m_flags ^ u32(flag);
		return r;
	}
	
	Flags & operator&=(Enum flag) {
		m_flags &= u32(flag);
		return *this;
	}
	
	Flags & operator|=(Enum flag) {
		m_flags |= u32(flag);
		return *this;
	}
	
	Flags & operator^=(Enum flag) {
		m_flags ^= u32(flag);
		return *this;
	}
	
	static Flags all() {
		return ~Flags(0);
	}
	
};

/*!
 * Helper class used by DECLARE_FLAGS_OPERATORS to prevent some automatic casts.
 */
class IncompatibleFlag {
	
public:
	
	explicit IncompatibleFlag(u32 flag) { ARX_UNUSED(flag); }
	
};

/*!
 * Declare a flag type using values from a given enum.
 * This should always be used instead of using Flags&lt;Enum&gt; directly.
 * 
 * \param Enum should be an enum with values that have exactly one bit set.
 * \param Flagname is the name for the flag type to be defined.
 */
#define DECLARE_FLAGS(Enum, Flagname) typedef Flags<Enum> Flagname;

/*!
 * Declare overloaded operators for a given flag type.
 */
#define DECLARE_FLAGS_OPERATORS(Flagname) \
	inline Flagname operator|(Flagname::Enum a, Flagname::Enum b) { \
		return Flagname(a) | b; \
	} \
	inline Flagname operator|(Flagname::Enum a, Flagname b) { \
		return b | a; \
	} \
	inline IncompatibleFlag operator|(Flagname::Enum a, u32 b) { \
		return IncompatibleFlag(u32(a) | b); \
	} \
	inline IncompatibleFlag operator|(u32 a, Flagname::Enum b) { \
		return IncompatibleFlag(a | u32(b)); \
	} \
	inline IncompatibleFlag operator&(Flagname a, u32 b) { \
		return IncompatibleFlag(u32(a) & b); \
	} \
	inline IncompatibleFlag operator&(u32 a, Flagname b) { \
		return IncompatibleFlag(a & u32(b)); \
	} \
	inline Flagname operator~(Flagname::Enum a) { \
		return ~Flagname(a); \
	}

#endif // ARX_UTIL_FLAGS_H
