/*
 * Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "tests/io/IniTest.h"

#include "io/IniSection.h"

CPPUNIT_TEST_SUITE_REGISTRATION(IniTest);

void IniTest::intTest() {
	static const int INVALID = 999999;
	
	CPPUNIT_ASSERT_EQUAL(-1, IniKey("b", "-1").getValue(INVALID));
	CPPUNIT_ASSERT_EQUAL( 0, IniKey("b", "0").getValue(INVALID));
	CPPUNIT_ASSERT_EQUAL( 1, IniKey("b", "1").getValue(INVALID));
	
	CPPUNIT_ASSERT_EQUAL( 1, IniKey("b", "1.4").getValue(INVALID));
	CPPUNIT_ASSERT_EQUAL( 1, IniKey("b", "1,5").getValue(INVALID));
}

void IniTest::floatTest() {
	static const float INVALID = 999999.f;
	
	CPPUNIT_ASSERT_EQUAL(-1.f, IniKey("b", "-1").getValue(INVALID));
	CPPUNIT_ASSERT_EQUAL( 0.f, IniKey("b", "0").getValue(INVALID));
	CPPUNIT_ASSERT_EQUAL( 1.f, IniKey("b", "1").getValue(INVALID));
	
	CPPUNIT_ASSERT_EQUAL( 1.12f, IniKey("b", "1.12").getValue(INVALID));
	CPPUNIT_ASSERT_EQUAL(INVALID, IniKey("b", "1,12").getValue(INVALID));
}

void IniTest::boolTest() {
	
	// Should convert properly
	CPPUNIT_ASSERT_EQUAL(true,  IniKey("a", "true" ).getValue(false));
	CPPUNIT_ASSERT_EQUAL(false, IniKey("b", "false").getValue(true));
	CPPUNIT_ASSERT_EQUAL(true,  IniKey("a", "1" ).getValue(false));
	CPPUNIT_ASSERT_EQUAL(false, IniKey("b", "0").getValue(true));
	
	// Should not
	CPPUNIT_ASSERT_EQUAL(false, IniKey("a", "True" ).getValue(false));
	CPPUNIT_ASSERT_EQUAL(true,  IniKey("b", "False").getValue(true));
	CPPUNIT_ASSERT_EQUAL(false, IniKey("a", "TRUE" ).getValue(false));
	CPPUNIT_ASSERT_EQUAL(true,  IniKey("b", "FALSE").getValue(true));
}


