/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_MATH_RANDOM_H
#define ARX_MATH_RANDOM_H

#include <limits>

#include <boost/type_traits.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

#include "platform/Platform.h"
#include "platform/ThreadLocal.h"

/*!
 * Random number generator.
 */
class Random {
	
public:
	
	//! Generates a random integer value in the range [intMin, intMax].
	template <typename IntType> static IntType get();
	template <typename IntType> static IntType get(IntType min, IntType max);
	static int get(int min = 0, int max = std::numeric_limits<int>::max());
	static unsigned int getu(unsigned int min, unsigned int max);
	
	//! Generates a random floating point value in the range [realMin, realMax).
	template <class RealType> static RealType getf();
	template <class RealType> static RealType getf(RealType realMin, RealType realMax);
	static float getf(float realMin = 0.0f, float realMax = 1.0f);
	
	//! Return a random iterator pointing in the range [begin, end).
	template <class Iterator>
	static Iterator getIterator(Iterator begin, Iterator end);
	
	//! Return a random iterator in the given container.
	template <class Container>
	static typename Container::iterator getIterator(Container & container);
	
	//! Return a random const_iterator in the given container.
	template <class Container>
	static typename Container::const_iterator getIterator(const Container & container);
	
	//! Seed the random number generator using the current time.
	static void seed();
	
	//! Seed the random number generator with the given value.
	static void seed(unsigned int seedVal);
	
	//! Release all resources held by this threads random generator
	static void shutdown();
	
private:
	
	typedef boost::random::mt19937 Generator;
	
	static ARX_THREAD_LOCAL Generator * rng;
};

///////////////////////////////////////////////////////////////////////////////

template <class IntType>
IntType Random::get(IntType min, IntType max) {
	ARX_STATIC_ASSERT(boost::is_integral<IntType>::value, "get must be called with ints");
	
	return typename boost::random::uniform_int_distribution<IntType>(min, max)(*rng);
}

template <class IntType>
IntType Random::get() {
	return Random::get<IntType>(0, std::numeric_limits<IntType>::max());
}

inline int Random::get(int min, int max) {
	return Random::get<int>(min, max);
}

inline unsigned int Random::getu(unsigned int min, unsigned int max) {
	return Random::get<unsigned int>(min, max);
}

template <class RealType>
RealType Random::getf(RealType min, RealType max) {
	ARX_STATIC_ASSERT(boost::is_float<RealType>::value, "getf must be called with floats");
	
	return typename boost::random::uniform_real_distribution<RealType>(min, max)(*rng);
}

template <class RealType>
RealType Random::getf() {
	return Random::getf<RealType>(RealType(0.0), RealType(1.0));
}

inline float Random::getf(float min, float max) {
	return Random::getf<float>(min, max);
}

template <class Iterator>
Iterator Random::getIterator(Iterator begin, Iterator end) {
	typedef typename std::iterator_traits<Iterator>::difference_type diff_t;
	
	diff_t dist = std::distance(begin, end);
	diff_t toAdvance = Random::get<diff_t>(0, dist - 1);
	
	std::advance(begin, toAdvance);
	
	return begin;
}

template <class Container>
typename Container::iterator Random::getIterator(Container & container) {
	return getIterator(container.begin(), container.end());
}

template <class Container>
typename Container::const_iterator Random::getIterator(const Container & container) {
	return getIterator(container.begin(), container.end());
}

#endif // ARX_MATH_RANDOM_H
