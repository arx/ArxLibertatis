
#ifndef ARX_PLATFORM_FLAGS_H
#define ARX_PLATFORM_FLAGS_H

#include "platform/Platform.h"

// Based on QFlags from Qt

/*!
 * A typesafe way to define flags as a combination of enum values.
 * 
 * This type should not be used directly, only through DECLARE_FLAGS.
 */
template <typename _Enum>
class Flags {
	
	typedef void ** Zero;
	u32 flags;
	
	inline Flags(u32 flag, bool dummy) : flags(flag) { ARX_UNUSED(dummy); }
	
public:
	
	typedef _Enum Enum;
	
	inline Flags(Enum flag) : flags(flag) { }
	
	inline Flags(Zero = 0) : flags(0) { }
	
	inline Flags(const Flags & o) : flags(o.flags) { }
	
	static inline Flags load(u32 flags) {
		return Flags(flags, true);
	}
	
	inline bool has(Enum flag) const {
		return (bool)(flags & (u32)flag);
	}
	
	inline bool hasAll(Flags o) const {
		return (flags & o.flags) == o.flags;
	}
	
	inline Flags except(Enum flag) const {
		Flags r;
		r.flags = flags & ~(u32)flag;
		return r;
	}
	
	inline void remove(Enum flag) const {
		flags &= ~(u32)flag;
	}
	
	inline operator u32() const {
		return flags;
	}
	
	inline Flags operator~() const {
		Flags r;
		r.flags = ~flags;
		return r;
	}
	
	inline bool operator!() const {
		return (flags == 0);
	}
	
	inline Flags operator&(Flags o) const {
		Flags r;
		r.flags = flags & o.flags;
		return r;
	}
	
	inline Flags operator|(Flags o) const {
		Flags r;
		r.flags = flags | o.flags;
		return r;
	}
	
	inline Flags operator^(Flags o) const {
		Flags r;
		r.flags = flags ^ o.flags;
		return r;
	}
	
	inline Flags & operator&=(const Flags & o) {
		flags &= o.flags;
		return *this;
	}
	
	inline Flags & operator|=(Flags o) {
		flags |= o.flags;
		return *this;
	}
	
	inline Flags & operator^=(Flags o) {
		flags ^= o.flags;
		return *this;
	}
	
	inline Flags operator&(Enum flag) const {
		Flags r;
		r.flags = flags & (u32)flag;
		return r;
	}
	
	inline Flags operator|(Enum flag) const {
		Flags r;
		r.flags = flags | (u32)flag;
		return r;
	}
	
	inline Flags operator^(Enum flag) const {
		Flags r;
		r.flags = flags ^ (u32)flag;
		return r;
	}
	
	inline Flags & operator&=(Enum flag) {
		flags &= (u32)flag;
		return *this;
	}
	
	inline Flags & operator|=(Enum flag) {
		flags |= (u32)flag;
		return *this;
	}
	
	inline Flags & operator^=(Enum flag) {
		flags ^= (u32)flag;
		return *this;
	}
	
	inline Flags & operator=(Flags o) {
		flags = o.flags;
		return *this;
	}
	
	static inline Flags all() {
		return ~Flags(0);
	}
	
};

/*!
 * Helper class used by DECLARE_FLAGS_OPERATORS to prevent some automatic casts.
 */
class IncompatibleFlag {
	
	u32 value;
	
public:
	
	IncompatibleFlag(u32 flag) : value(flag) { }
	
};

/*!
 * Declare a flag type using values from a given enum.
 * This should always be used instead of using Flags&lt;Enum&gt; directly.
 * 
 * @param Enum should be an enum with values that have exactly one bit set.
 * @param Flagname is the name for the flag type to be defined.
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

#endif // ARX_PLATFORM_FLAGS_H
