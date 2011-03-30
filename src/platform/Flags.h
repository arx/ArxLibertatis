
#ifndef ARX_PLATFORM_FLAGS_H
#define ARX_PLATFORM_FLAGS_H

#include "platform/Platform.h"

// Based on QFlags from Qt

class Flag {
	
	u32 value;
	
public:
	
	Flag(u32 flag) : value(flag) { }
	
	operator u32() {
		return value;
	}
	
};

/*!
 * A typesafe way to define flags as a combination of enum values.
 */
template <typename _Enum>
class Flags {
	
	typedef void ** Zero;
	u32 flags;
	
public:
	
	typedef _Enum Enum;
	
	inline Flags(Enum flag) : flags(flag) { }
	
	inline Flags(Zero = 0) : flags(0) { }
	
	inline Flags(const Flags & o) : flags(o.flags) { }
	
	inline Flags(Flag flag) : flags(flag) { }
	
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

class IncompatibleFlag {
	
	u32 value;
	
public:
	
	IncompatibleFlag(u32 flag) : value(flag) { }
	
};

#define DECLARE_FLAGS(Enum, Flagname) typedef Flags<Enum> Flagname;

#define DECLARE_FLAGS_OPERATORS(Flagname) \
	inline Flagname operator|(Flagname::Enum a, Flagname::Enum b) { \
		return Flagname(a) | b; \
	} \
	inline Flagname operator|(Flagname::Enum a, Flagname b) { \
		return b | a; \
	} \
	inline IncompatibleFlag operator|(Flagname::Enum a, u32 b) { \
		return IncompatibleFlag(u32(b) | a); \
	} \
	inline Flagname operator~(Flagname::Enum a) { \
		return ~Flagname(a); \
	}

#endif // ARX_PLATFORM_FLAGS_H
