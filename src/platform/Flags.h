
#ifndef ARX_PLATFORM_FLAGS_H
#define ARX_PLATFORM_FLAGS_H

#include "platform/Platform.h"

/*!
 * A typesafe way to define flags as a combination of enum values.
 */
template <typename _Enum>
class Flags {
	
	u32 flags;
	
	inline Flags(u32 value) : flags(value) { }
	
public:
	
	typedef void ** Zero;
	typedef _Enum Enum;
	
	inline Flags(Enum flag) : flags(flag) { }
	
	inline Flags(Zero zero) : flags(0) { }
	
	inline Flags(Flags<Enum> o) : flags(o.flags) { }
	
	inline bool has(Enum flag) {
		return (bool)(flags & (u32)flag);
	}
	
	inline bool hasAll(Flags<Enum> o) {
		return (flags & o.flags) == o.flags;
	}
	
	inline Flags<Enum> remove(Enum flag) {
		return Flags<Enum>(flags & ~(u32)flag);
	}
	
	inline operator u32() {
		return flags;
	}
	
	inline Flags<Enum> operator~() {
		return Flags<Enum>(~flags);
	}
	
	inline Flags<Enum> operator&(Flags<Enum> o) {
		return Flags<Enum>(flags & o.flags);
	}
	
	inline Flags<Enum> operator|(Flags<Enum> o) {
		return Flags<Enum>(flags | o.flags);
	}
	
	inline Flags<Enum> operator^(Flags<Enum> o) {
		return Flags<Enum>(flags ^ o.flags);
	}
	
	inline Flags<Enum> & operator&=(Flags<Enum> o) {
		flags &= o.flags;
		return *this;
	}
	
	inline Flags<Enum> & operator|=(Flags<Enum> o) {
		flags |= o.flags;
		return *this;
	}
	
	inline Flags<Enum> & operator^=(Flags<Enum> o) {
		flags ^= o.flags;
		return *this;
	}
	
	inline Flags<Enum> operator&(Enum flag) {
		return Flags<Enum>(flags & (u32)flag);
	}
	
	inline Flags<Enum> operator|(Enum flag) {
		return Flags<Enum>(flags | (u32)flag);
	}
	
	inline Flags<Enum> operator^(Enum flag) {
		return Flags<Enum>(flags ^ (u32)flag);
	}
	
	inline Flags<Enum> & operator&=(Flags<Enum> o) {
		flags &= o.flags;
		return *this;
	}
	
	inline Flags<Enum> & operator|=(Flags<Enum> o) {
		flags |= o.flags;
		return *this;
	}
	
	inline Flags<Enum> & operator^=(Flags<Enum> o) {
		flags ^= o.flags;
		return *this;
	}
	
};

#define DECLARE_FLAGS(Enum, Flagname) typedef Flags<Enum> Flagname;

#define DECLARE_FLAGS_OPERATORS(Flagname) \
	inline Flagname operator|(Flagname::Enum a, Flagname::Enum b) { \
		return Flagname(a) | b; \
	} \
	inline Flagname operator~(Flagname::Enum a) { \
		return ~Flagname(a); \
	}

#endif // ARX_PLATFORM_FLAGS_H
