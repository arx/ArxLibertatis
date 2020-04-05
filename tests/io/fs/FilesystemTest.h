/*
 * Copyright 2020 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_TESTS_IO_FS_FILESYSTEMTEST_H
#define ARX_TESTS_IO_FS_FILESYSTEMTEST_H

#include <ctime>
#include <string>
#include <map>
#include <set>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "io/fs/Filesystem.h"
#include "io/fs/FilePath.h"

class FilesystemTest : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE(FilesystemTest);
	CPPUNIT_TEST(testRelative);
	CPPUNIT_TEST(testAbsolute);
	CPPUNIT_TEST_SUITE_END();
	
	struct Entry {
		
		fs::FileType type;
		std::string contents;
		std::time_t mtime;
		
		Entry()
			: type(fs::DoesNotExist)
			, mtime(0)
		{ }
		
		Entry(fs::FileType type_, const std::string & contents_ = std::string(), std::time_t mtime_ = 0)
			: type(type_)
			, contents(contents_)
			, mtime(mtime_)
		{ }
		
	};
	
	typedef std::map<std::string, Entry> Entries;
	
	fs::path m_relroot;
	fs::path m_absroot;
	Entries m_expected;
	std::set<std::string> m_all;
	
public:
	
	void setUp();
	void tearDown();
	
	std::time_t checkTime(const fs::path & root, const fs::path & path, std::time_t before);
	
	void iterate(const fs::path & path, const std::string & prefix, Entries & result);
	
	void checkState(const fs::path & root);
	
	void run(const fs::path & root);
	
	void testRelative();
	void testAbsolute();
	
};

#endif // ARX_TESTS_IO_FS_FILESYSTEMTEST_H

