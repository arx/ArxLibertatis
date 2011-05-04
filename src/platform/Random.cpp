
#include "platform/Random.h"

#include <ctime>

size_t Random::current;

void Random::seed() {
	current = (size_t)std::time(NULL);
}
