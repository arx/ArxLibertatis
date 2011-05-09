
#ifndef ARX_GRAPHICS_COLOR_H
#define ARX_GRAPHICS_COLOR_H

#include "platform/Platform.h"

class Color {
	
public:
	
	u32 value;
	
	Color() { }
	
	Color(u8 r, u8 g, u8 b) : value(r | (((u32)g) << 8) | (((u32)b) << 16)) { }
	
	Color(u32 color) : value(color) { }
	
	Color & operator=(Color other) {
		value = other.value;
		return *this;
	}
	
	Color & operator=(u32 color) {
		value = color;
		return *this;
	}
	
	operator u32() {
		return value;
	}
	
	u8 r() {
		return value & 0xff;
	}
	
	u8 g() {
		return (value >> 8) & 0xff;
	}
	
	u8 b() {
		return (value >> 16) & 0xff;
	}
	
};

#endif // ARX_GRAPHICS_COLOR_H
