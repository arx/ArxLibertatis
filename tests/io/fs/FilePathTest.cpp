/*
 * Copyright 2016-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "tests/io/fs/FilePathTest.h"

#include <algorithm>
#include <string>

#include "io/fs/FilePath.h"

CPPUNIT_TEST_SUITE_REGISTRATION(FilePathTest);

void FilePathTest::testPath(const char * input, const char * expected) {
	
	std::string target = expected;
	std::replace(target.begin(), target.end(), '/', fs::path::dir_sep);
	
	fs::path path = input;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(input), target, path.string());
	
}

void FilePathTest::pathTest() {
	
	testPath(".", ".");
	testPath("./", ".");
	testPath(".////./././//././/", ".");
	testPath("./a", "a");
	testPath(".////./././//././/a", "a");
	testPath("./..", "..");
	testPath(".////./././//././/..", "..");
	testPath("./a/..", ".");
	
	testPath("/a", "/a");
	testPath("/a/b", "/a/b");
	testPath("/a/b/c", "/a/b/c");
	testPath("/a/", "/a");
	testPath("/a/b", "/a/b");
	testPath("/a/b/c/./", "/a/b/c");
	
	testPath("/", "/");
	testPath("/..", "/..");
	testPath("/../", "/..");
	testPath("/..//", "/..");
	testPath("/../..", "/../..");
	testPath("/..//..", "/../..");
	testPath("/../../", "/../..");
	testPath("/..//../", "/../..");
	testPath("/../..//", "/../..");
	testPath("/..//..//", "/../..");
	
	testPath("/a", "/a");
	testPath("/a/..", "/");
	testPath("/a/../", "/");
	testPath("/a/..//", "/");
	testPath("/a/../..", "/..");
	testPath("/a/..//..", "/..");
	testPath("/a/../../", "/..");
	testPath("/a/..//../", "/..");
	testPath("/a/../..//", "/..");
	testPath("/a/..//..//", "/..");
	
	testPath("/abcd", "/abcd");
	testPath("/abcd/..", "/");
	testPath("/abcd/../", "/");
	testPath("/abcd/..//", "/");
	testPath("/abcd/../..", "/..");
	testPath("/abcd/..//..", "/..");
	testPath("/abcd/../../", "/..");
	testPath("/abcd/..//../", "/..");
	testPath("/abcd/../..//", "/..");
	testPath("/abcd/..//..//", "/..");
	
	testPath("/../a", "/../a");
	testPath("/../a/", "/../a");
	testPath("/../a//", "/../a");
	testPath("/../a/..", "/..");
	testPath("/../a//..", "/..");
	testPath("/../a/../", "/..");
	testPath("/../a//../", "/..");
	testPath("/../a/..//", "/..");
	testPath("/../a//..//", "/..");
	
	testPath("a", "a");
	testPath("a/b", "a/b");
	testPath("a/", "a");
	testPath("a/b/", "a/b");
	
	testPath("", "");
	testPath("..", "..");
	testPath("../", "..");
	testPath("..//", "..");
	testPath("../..", "../..");
	testPath("..//..", "../..");
	testPath("../../", "../..");
	testPath("..//../", "../..");
	testPath("../..//", "../..");
	testPath("..//..//", "../..");
	
	testPath("a", "a");
	testPath("a/..", ".");
	testPath("a/../", ".");
	testPath("a/..//", ".");
	testPath("a/../..", "..");
	testPath("a/..//..", "..");
	testPath("a/../../", "..");
	testPath("a/..//../", "..");
	testPath("a/../..//", "..");
	testPath("a/..//..//", "..");
	
	testPath("abcd", "abcd");
	testPath("abcd/..", ".");
	testPath("abcd/../", ".");
	testPath("abcd/..//", ".");
	testPath("abcd/../..", "..");
	testPath("abcd/..//..", "..");
	testPath("abcd/../../", "..");
	testPath("abcd/..//../", "..");
	testPath("abcd/../..//", "..");
	testPath("abcd/..//..//", "..");
	
	testPath("../a", "../a");
	testPath("../a/", "../a");
	testPath("../a//", "../a");
	testPath("../a/..", "..");
	testPath("../a//..", "..");
	testPath("../a/../", "..");
	testPath("../a//../", "..");
	testPath("../a/..//", "..");
	testPath("../a//..//", "..");
	
	testPath("/.", "/");
	
}

void FilePathTest::testResolve(const char * left, const char * right, const char * expected) {
	
	std::string target = expected;
	std::replace(target.begin(), target.end(), '/', fs::path::dir_sep);
	
	std::string message = "\"" + std::string(left) + "\" / \"" + std::string(right) + "\"";
	
	fs::path result = fs::path(left) / right;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(message, target, result.string());
	
	fs::path temp = left;
	temp /= right;
	CPPUNIT_ASSERT_EQUAL_MESSAGE(message, target, temp.string());
	
}

void FilePathTest::resolveTest() {
	
	testResolve(".", "a", "a");
	testResolve(".", "a/b", "a/b");
	testResolve(".", "..", "..");
	testResolve(".", "a", "a");
	testResolve("a", "..", ".");
	testResolve("a/b", "..", "a");
	testResolve("a/b", "../..", ".");
	
	testResolve(".", "", ".");
	testResolve(".", "a/..", ".");
	
	testResolve("/a", "b", "/a/b");
	testResolve("/a", "b/c", "/a/b/c");
	testResolve("/a/b", "c", "/a/b/c");
	
	testResolve("..", "..", "../..");
	testResolve("/..", "..", "/../..");
	testResolve("../..", "..", "../../..");
	testResolve("/../..", "..", "/../../..");
	testResolve("..", "../..", "../../..");
	testResolve("/..", "../..", "/../../..");
	
	testResolve("../a", "../..", "../..");
	testResolve("/../a", "../..", "/../..");
	
	testResolve("/a", "..", "/");
	testResolve("/a", "../..", "/..");
	testResolve("/a/..", "..", "/..");
	
	testResolve("/abcd", "..", "/");
	testResolve("/abcd", "../..", "/..");
	testResolve("/abcd/..", "..", "/..");
	
	testResolve("/" , "../a", "/../a");
	testResolve("/..", "a", "/../a");
	testResolve("/", "../a/..", "/..");
	testResolve("/..", "a/..", "/..");
	testResolve("/../a", "..", "/..");
	
	testResolve(".", "/a", "/a");
	testResolve(".", "/a/b", "/a/b");
	testResolve(".", "/..", "/..");
	testResolve(".", "/a", "/a");
	testResolve("a", "/..", "/..");
	testResolve("a/b", "/..", "/..");
	testResolve("a/b", "/../..", "/../..");
	
	testResolve(".", "/", "/");
	testResolve(".", "/a/..", "/");
	
	testResolve("/a", "/b", "/b");
	testResolve("/a", "/b/c", "/b/c");
	testResolve("/a/b", "/c", "/c");
	
	testResolve("..", "/..", "/..");
	testResolve("/..", "/..", "/..");
	
	testResolve("/a", "/..", "/..");
	testResolve("/a", "/../..", "/../..");
	testResolve("/a/..", "/..", "/..");
	
	testResolve("/abcd", "/..", "/..");
	testResolve("/abcd", "/../..", "/../..");
	testResolve("/abcd/..", "/..", "/..");
	
	testResolve("/" , "/../a", "/../a");
	testResolve("/..", "/a", "/a");
	testResolve("/", "/../a/..", "/..");
	testResolve("/..", "/a/..", "/");
	testResolve("/../a", "/..", "/..");
	
}

void FilePathTest::testParent(const char * input, const char * expected) {
	
	std::string target = expected;
	std::replace(target.begin(), target.end(), '/', fs::path::dir_sep);
	
	fs::path parent = fs::path(input).parent();
	CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(input), target, parent.string());
	
	fs::path temp = input;
	temp.up();
	CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(input), target, temp.string());
	
}

void FilePathTest::parentTest() {
	
	testParent("", "..");
	testParent(".", "..");
	testParent("a", ".");
	testParent("a/b", "a");
	testParent("/", "/..");
	testParent("/a", "/");
	testParent("/a/b", "/a");
	testParent("..", "../..");
	testParent("../..", "../../..");
	
}
