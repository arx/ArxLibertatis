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

#ifndef ARX_GRAPHICS_COLORTEST_H
#define ARX_GRAPHICS_COLORTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "graphics/Color.h"

class ColorTest : public CppUnit::TestFixture {
	CPPUNIT_TEST_SUITE(ColorTest);
	CPPUNIT_TEST(ColorTypeConversionTests);
	CPPUNIT_TEST_SUITE_END();

public:
	void ColorTypeConversionTests();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ColorTest);

#endif
