/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "tests/io/resource/ResourcePathTest.h"

#include <string>

#include "io/resource/ResourcePath.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ResourcePathTest);

void ResourcePathTest::testLoad(const char * input, const char * expected) {
	res::path path = res::path::load(input);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(input), std::string(expected), path.string());
}

void ResourcePathTest::loadTest() {
	
	testLoad("", "");
	testLoad(".", "");
	testLoad("./", "");
	testLoad(".////./././//././/", "");
	testLoad("./a", "a");
	testLoad(".////./././//././/a", "a");
	testLoad("./..", "..");
	testLoad(".////./././//././/..", "..");
	testLoad("./a/..", "");
	
	testLoad("/a", "a");
	testLoad("/a/b", "a/b");
	testLoad("/a/b/c", "a/b/c");
	testLoad("/a/", "a");
	testLoad("/a/b", "a/b");
	testLoad("/a/b/c/./", "a/b/c");
	
	testLoad("/", "");
	testLoad("/..", "..");
	testLoad("/../", "..");
	testLoad("/..//", "..");
	testLoad("/../..", "../..");
	testLoad("/..//..", "../..");
	testLoad("/../../", "../..");
	testLoad("/..//../", "../..");
	testLoad("/../..//", "../..");
	testLoad("/..//..//", "../..");
	
	testLoad("/a", "a");
	testLoad("/a/..", "");
	testLoad("/a/../", "");
	testLoad("/a/..//", "");
	testLoad("/a/../..", "..");
	testLoad("/a/..//..", "..");
	testLoad("/a/../../", "..");
	testLoad("/a/..//../", "..");
	testLoad("/a/../..//", "..");
	testLoad("/a/..//..//", "..");
	
	testLoad("/abcd", "abcd");
	testLoad("/abcd/..", "");
	testLoad("/abcd/../", "");
	testLoad("/abcd/..//", "");
	testLoad("/abcd/../..", "..");
	testLoad("/abcd/..//..", "..");
	testLoad("/abcd/../../", "..");
	testLoad("/abcd/..//../", "..");
	testLoad("/abcd/../..//", "..");
	testLoad("/abcd/..//..//", "..");
	
	testLoad("/../a", "../a");
	testLoad("/../a/", "../a");
	testLoad("/../a//", "../a");
	testLoad("/../a/..", "..");
	testLoad("/../a//..", "..");
	testLoad("/../a/../", "..");
	testLoad("/../a//../", "..");
	testLoad("/../a/..//", "..");
	testLoad("/../a//..//", "..");
	
	testLoad("a", "a");
	testLoad("a/b", "a/b");
	testLoad("a/", "a");
	testLoad("a/b/", "a/b");
	
	testLoad("", "");
	testLoad("..", "..");
	testLoad("../", "..");
	testLoad("..//", "..");
	testLoad("../..", "../..");
	testLoad("..//..", "../..");
	testLoad("../../", "../..");
	testLoad("..//../", "../..");
	testLoad("../..//", "../..");
	testLoad("..//..//", "../..");
	
	testLoad("a", "a");
	testLoad("a/..", "");
	testLoad("a/../", "");
	testLoad("a/..//", "");
	testLoad("a/../..", "..");
	testLoad("a/..//..", "..");
	testLoad("a/../../", "..");
	testLoad("a/..//../", "..");
	testLoad("a/../..//", "..");
	testLoad("a/..//..//", "..");
	
	testLoad("abcd", "abcd");
	testLoad("abcd/..", "");
	testLoad("abcd/../", "");
	testLoad("abcd/..//", "");
	testLoad("abcd/../..", "..");
	testLoad("abcd/..//..", "..");
	testLoad("abcd/../../", "..");
	testLoad("abcd/..//../", "..");
	testLoad("abcd/../..//", "..");
	testLoad("abcd/..//..//", "..");
	
	testLoad("../a", "../a");
	testLoad("../a/", "../a");
	testLoad("../a//", "../a");
	testLoad("../a/..", "..");
	testLoad("../a//..", "..");
	testLoad("../a/../", "..");
	testLoad("../a//../", "..");
	testLoad("../a/..//", "..");
	testLoad("../a//..//", "..");
	
	testLoad("/.", "");
	
}

void ResourcePathTest::testResolve(const char * left, const char * right, const char * expected) {
	
	std::string message = "\"" + std::string(left) + "\" / \"" + std::string(right) + "\"";
	
	res::path result = res::path::load(left) / res::path::load(right);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(message, std::string(expected), result.string());
	
	res::path temp = res::path::load(left);
	temp /= res::path::load(right);
	CPPUNIT_ASSERT_EQUAL_MESSAGE(message, std::string(expected), temp.string());
	
}

void ResourcePathTest::resolveTest() {
	
	testResolve("", "a", "a");
	testResolve("", "a/b", "a/b");
	testResolve("", "..", "..");
	testResolve("", "a", "a");
	testResolve("a", "..", "");
	testResolve("a/b", "..", "a");
	testResolve("a/b", "../..", "");
	
	testResolve("", "a/..", "");
	
	testResolve("/a", "b", "a/b");
	testResolve("/a", "b/c", "a/b/c");
	testResolve("/a/b", "c", "a/b/c");
	
	testResolve("..", "..", "../..");
	testResolve("../..", "..", "../../..");
	testResolve("..", "../..", "../../..");
	
	testResolve("../a", "../..", "../..");
	
	testResolve("a", "..", "");
	testResolve("a", "../..", "..");
	testResolve("a/..", "..", "..");
	
	testResolve("abcd", "..", "");
	testResolve("abcd", "../..", "..");
	testResolve("abcd/..", "..", "..");
	
	testResolve("" , "../a", "../a");
	testResolve("..", "a", "../a");
	testResolve("", "../a/..", "..");
	testResolve("..", "a/..", "..");
	testResolve("../a", "..", "..");
	
	testResolve("", "", "");
	testResolve("a", "", "a");
	testResolve("a/b", "", "a/b");
	testResolve("..", "", "..");
	testResolve("../a", "", "../a");
	
}

void ResourcePathTest::testParent(const char * input, const char * expected) {
	
	res::path path = res::path::load(input);
	
	res::path parent = path.parent();
	CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(input), std::string(expected), parent.string());
	
	res::path temp = path;
	temp.up();
	CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(input), std::string(expected), temp.string());
	
}

void ResourcePathTest::parentTest() {
	
	testParent("", "..");
	testParent(".", "..");
	testParent("a", "");
	testParent("a/b", "a");
	testParent("/", "..");
	testParent("/a", "");
	testParent("/a/b", "a");
	testParent("..", "../..");
	testParent("../..", "../../..");
	
}

