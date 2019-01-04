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

#ifndef ARX_TESTS_UTIL_STRINGTEST_H
#define ARX_TESTS_UTIL_STRINGTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class StringTest : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE(StringTest);	
	CPPUNIT_TEST(stringStoreEmptyTest);
	CPPUNIT_TEST(stringStoreFittingTest);
	CPPUNIT_TEST(stringStoreTruncatedNullTest);
	CPPUNIT_TEST(stringStoreOverflowTest);
	
	CPPUNIT_TEST(stringStoreTerminatedEmptyTest);
	CPPUNIT_TEST(stringStoreTerminatedFittingTest);
	CPPUNIT_TEST(stringStoreTerminatedOverflowTest);
	
	CPPUNIT_TEST(safeGetExactTest);
	CPPUNIT_TEST(safeGetTooSmallTest);
	
	CPPUNIT_TEST_SUITE_END();
	
public:
	
	void stringStoreEmptyTest();
	void stringStoreFittingTest();
	void stringStoreTruncatedNullTest();
	void stringStoreOverflowTest();
	
	void stringStoreTerminatedEmptyTest();
	void stringStoreTerminatedFittingTest();
	void stringStoreTerminatedOverflowTest();
	
	void safeGetExactTest();
	void safeGetTooSmallTest();
	
};

#endif // ARX_TESTS_UTIL_STRINGTEST_H
