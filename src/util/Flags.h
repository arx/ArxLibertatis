/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include <type_traits>

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
	
	constexpr Flags(u32 flags, bool /* dummy */) noexcept : m_flags(flags) { }
	
public:
	
	typedef Enum_ Enum;
	
	/* implicit */ constexpr Flags(Enum flag) noexcept : m_flags(flag) { arx_assert(Enum(m_flags) == flag); }
	
	/* implicit */ constexpr Flags(Zero /* zero */ = 0) noexcept : m_flags(0) { }
	
	template <typename NullPtr, typename = std::enable_if_t<
		std::is_same_v<std::remove_cv_t<std::remove_reference_t<NullPtr>>, decltype(nullptr)>>
	>
	/* implicit */ Flags(NullPtr /* nullptr */) = delete;
	
	[[nodiscard]] static constexpr Flags load(u32 flags) noexcept {
		return Flags(flags, true);
	}
	
	[[nodiscard]] constexpr bool has(Enum flag) const noexcept {
		return hasAny(flag);
	}
	
	[[nodiscard]] constexpr bool hasAny(Flags o) const noexcept {
		return !!(*this & o);
	}
	
	[[nodiscard]] constexpr bool hasAll(Flags o) const noexcept {
		return (*this & o) == o;
	}
	
	[[nodiscard]] constexpr Flags except(Flags o) const noexcept {
		return *this & ~o;
	}
	
	constexpr void remove(Flags o) noexcept {
		*this &= ~o;
	}
	
	[[nodiscard]] constexpr operator u32() const noexcept {
		return m_flags;
	}
	
	[[nodiscard]] constexpr Flags operator~() const noexcept {
		Flags r;
		r.m_flags = ~m_flags;
		return r;
	}
	
	[[nodiscard]] constexpr bool operator!() const noexcept {
		return (m_flags == 0);
	}
	
	[[nodiscard]] constexpr Flags operator&(Flags o) const noexcept {
		Flags r;
		r.m_flags = m_flags & o.m_flags;
		return r;
	}
	
	[[nodiscard]] constexpr Flags operator|(Flags o) const noexcept {
		Flags r;
		r.m_flags = m_flags | o.m_flags;
		return r;
	}
	
	[[nodiscard]] constexpr Flags operator^(Flags o) const noexcept {
		Flags r;
		r.m_flags = m_flags ^ o.m_flags;
		return r;
	}
	
	constexpr Flags & operator&=(Flags o) noexcept {
		m_flags &= o.m_flags;
		return *this;
	}
	
	constexpr Flags & operator|=(Flags o) noexcept {
		m_flags |= o.m_flags;
		return *this;
	}
	
	constexpr Flags & operator^=(Flags o) noexcept {
		m_flags ^= o.m_flags;
		return *this;
	}
	
	[[nodiscard]] constexpr Flags operator&(Enum flag) const noexcept {
		return *this & Flags(flag);
	}
	
	[[nodiscard]] constexpr Flags operator|(Enum flag) const noexcept {
		return *this | Flags(flag);
	}
	
	[[nodiscard]] constexpr Flags operator^(Enum flag) const noexcept {
		return *this ^ Flags(flag);
	}
	
	constexpr Flags & operator&=(Enum flag) noexcept {
		return *this &= Flags(flag);
	}
	
	constexpr Flags & operator|=(Enum flag) noexcept {
		return *this |= Flags(flag);
	}
	
	constexpr Flags & operator^=(Enum flag) noexcept {
		return *this ^= Flags(flag);
	}
	
	[[nodiscard]] static constexpr Flags all() noexcept {
		return ~Flags(0);
	}
	
};

/*!
 * Declare a flag type using values from a given enum.
 * This should always be used instead of using Flags&lt;Enum&gt; directly.
 * 
 * \param Enum should be an enum with values that have exactly one bit set.
 * \param Flagname is the name for the flag type to be defined.
 */
#define DECLARE_FLAGS(Enum, Flagname) typedef Flags<Enum> Flagname; \
	static_assert(std::is_trivially_copyable_v<Flagname>); /* can be passed in registers in sane ABIs */ \
	static_assert(std::is_standard_layout_v<Flagname>); \

/*!
 * Declare overloaded operators for a given flag type.
 */
#define DECLARE_FLAGS_OPERATORS(Flagname) \
	[[nodiscard]] inline constexpr Flagname operator|(Flagname::Enum a, Flagname::Enum b) noexcept { \
		return Flagname(a) | b; \
	} \
	[[nodiscard]] inline constexpr Flagname operator|(Flagname::Enum a, Flagname b) noexcept { \
		return b | a; \
	} \
	inline void operator|(Flagname::Enum a, u32 b) = delete; \
	inline void operator|(u32 a, Flagname::Enum b) = delete; \
	inline void operator&(Flagname a, u32 b) = delete; \
	inline void operator&(u32 a, Flagname b) = delete; \
	[[nodiscard]] inline constexpr Flagname operator~(Flagname::Enum a) noexcept { \
		return ~Flagname(a); \
	}

#endif // ARX_UTIL_FLAGS_H
