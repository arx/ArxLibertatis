/*
 * Copyright 2016-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TESTS_IO_FS_FILEPATHTEST_H
#define ARX_TESTS_IO_FS_FILEPATHTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class FilePathTest : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE(FilePathTest);
	CPPUNIT_TEST(pathTest);
	CPPUNIT_TEST(resolveTest);
	CPPUNIT_TEST(parentTest);
	CPPUNIT_TEST_SUITE_END();
	
public:
	
	void testPath(const char * input, const char * expected);
	void testResolve(const char * left, const char * right, const char * expected);
	void testParent(const char * input, const char * expected);
	
	void pathTest();
	void resolveTest();
	void parentTest();
	
};

#endif // ARX_TESTS_IO_FS_FILEPATHTEST_H

