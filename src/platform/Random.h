
#ifndef ARX_PLATFORM_RANDOM_H

#include <stddef.h>

/*!
 * A simple but very fast random number generator.
 */
class Random {
	
	static const size_t MODULO = 2147483647;
	static const size_t FACTOR = 16807;
	static const size_t SHIFT = 91;
	static const size_t SEED = 43;
	
	static size_t current;
	
public:
	
	inline static size_t get() {
		return current = (current * FACTOR + SHIFT) % MODULO;
	}
	
	inline static float getf() {
		return get() * (1.f / MODULO);
	}
	
	static void seed();
	
};

#endif // ARX_PLATFORM_RANDOM_H
