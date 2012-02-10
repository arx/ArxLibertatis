/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_RANDOM_H
#define ARX_PLATFORM_RANDOM_H

#include <limits>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>

/*!
 * Random number generator.
 */
class Random {
public:
	/// Generates a random integer value in the range [0, std::numeric_limits<IntType>::max()]
	template <class IntType>
	static inline IntType get();
	static inline int get();

	/// Generates a random integer value in the range [intMin, intMax).
	template <class IntType>
	static inline IntType get(IntType intMin, IntType intMax);
	static inline int get(int intMin, int intMax);

	/// Generates a random floating point value in the range [realMin, realMax).
	template <class RealType>
	static inline RealType getf(RealType realMin = RealType(0.0), RealType realMax = RealType(1.0));
	static inline float getf(float realMin = 0.0f, float realMax = 1.0f);

	/// Return a random iterator pointing in the range [begin, end).
	template <class Iterator>
	static inline Iterator getIterator(Iterator begin, Iterator end);

	/// Return a random iterator in the given container.
	template <class Container>
	static inline typename Container::iterator getIterator(Container& container);

	/// Return a random const_iterator in the given container.
	template <class Container>
	static inline typename Container::const_iterator getIterator(const Container& container);

	/// Seed the random number generator using the current time.
	static void seed();

	/// Seed the random number generator with the given value.
	static void seed(unsigned int seedVal);

private:
	static boost::random::mt19937 rng;
};

///////////////////////////////////////////////////////////////////////////////

template <class IntType>
IntType Random::get() {
	return boost::random::uniform_int_distribution<IntType>(0, std::numeric_limits<IntType>::max())(rng);
}

int Random::get() {
	return Random::get<int>();
}

template <class IntType>
IntType Random::get(IntType intMin, IntType intMax) {
	return boost::random::uniform_int_distribution<IntType>(intMin, intMax-1)(rng);
}

int Random::get(int intMin, int intMax) {
	return Random::get<int>(intMin, intMax);
}

template <class RealType>
RealType Random::getf(RealType realMin, RealType realMax) {
	return boost::random::uniform_real_distribution<RealType>(realMin, realMax)(rng);
}

float Random::getf(float realMin, float realMax) {
	return Random::getf<float>(realMin, realMax);
}

template <class Iterator>
Iterator Random::getIterator(Iterator begin, Iterator end) {
	typedef std::iterator_traits<Iterator>::difference_type diff_t;

	diff_t dist = std::distance(begin, end);
	diff_t toAdvance = Random::get<diff_t>(0, dist);

	std::advance(begin, toAdvance);
	
	return begin;
}

template <class Container>
typename Container::iterator Random::getIterator(Container& container) {
	return getIterator(container.begin(), container.end());
}

template <class Container>
typename Container::const_iterator Random::getIterator(const Container& container) {
	return getIterator(container.begin(), container.end());
}

#endif // ARX_PLATFORM_RANDOM_H
