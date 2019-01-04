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

#include "ColorTest.h"

#include <cppunit/TestAssert.h>

#include "tests/math/AssertionTraits.h"

#include "graphics/Color.h"

namespace CppUnit {
	template <typename TAG>
	struct assertion_traits<IntegerColorType<TAG,  u32> > {
		static bool equal(const IntegerColorType<TAG,  u32> & v, const IntegerColorType<TAG,  u32> & other) {
			return v.t == other.t;
		}
		
		static std::string toString(const IntegerColorType<TAG,  u32> & v) {
			std::ostringstream ost;
			ost << std::hex << "0x" << v.t;
			return ost.str();
		}
	};
} // namespace CppUnit


void ColorTest::ColorTypeConversionTests() {
	
	CPPUNIT_ASSERT_EQUAL(ColorBGRA(0xFFFFFFFF), Color3f(1.f, 1.f, 1.f).toBGR(255));
	
	CPPUNIT_ASSERT_EQUAL(Color::fromRGBA(ColorRGBA(0xFF000000)), Color(0, 0, 0, 255));
	CPPUNIT_ASSERT_EQUAL(Color::fromRGBA(ColorRGBA(0x00FF0000)), Color(0, 0, 255, 0));
	CPPUNIT_ASSERT_EQUAL(Color::fromRGBA(ColorRGBA(0x0000FF00)), Color(0, 255, 0, 0));
	CPPUNIT_ASSERT_EQUAL(Color::fromRGBA(ColorRGBA(0x000000FF)), Color(255, 0, 0, 0));
	
	CPPUNIT_ASSERT_EQUAL(Color::fromBGRA(ColorBGRA(0xFF000000)), Color(0, 0, 0, 255));
	CPPUNIT_ASSERT_EQUAL(Color::fromBGRA(ColorBGRA(0x00FF0000)), Color(255, 0, 0, 0));
	CPPUNIT_ASSERT_EQUAL(Color::fromBGRA(ColorBGRA(0x0000FF00)), Color(0, 255, 0, 0));
	CPPUNIT_ASSERT_EQUAL(Color::fromBGRA(ColorBGRA(0x000000FF)), Color(0, 0, 255, 0));
	CPPUNIT_ASSERT_EQUAL(Color::fromBGRA(ColorBGRA(0xFFa8d0df)), Color(168, 208, 223, 255));
}
