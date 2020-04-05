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

#include "io/fs/FilesystemTest.h"

#include <boost/foreach.hpp>

CppUnit::AutoRegisterSuite<FilesystemTest> g_registerFilesystemTest;

void FilesystemTest::setUp() {
	m_relroot = "filesystem-test-tmp";
	m_absroot = fs::current_path() / m_relroot;
	fs::remove_all(m_relroot);
	fs::create_directories(m_relroot);
	CPPUNIT_ASSERT(m_relroot.is_relative());
	CPPUNIT_ASSERT(m_absroot.is_absolute());
	m_expected.clear();
	m_all.clear();
}

void FilesystemTest::tearDown() {
	fs::remove_all(m_relroot);
	m_expected.clear();
	checkState(m_relroot);
}

std::time_t FilesystemTest::checkTime(const fs::path & root, const fs::path & path, std::time_t before) {
	std::time_t time = fs::last_write_time(root / path);
	CPPUNIT_ASSERT_MESSAGE(path.string(), time >= before);
	CPPUNIT_ASSERT_MESSAGE(path.string(), time <= std::time(NULL));
	return time;
}

void FilesystemTest::iterate(const fs::path & path, const std::string & prefix, Entries & result) {
	
	for(fs::directory_iterator it(path); !it.end(); ++it) {
		
		std::string name = prefix + it.name();
		CPPUNIT_ASSERT(result.find(name) == result.end());
		fs::FileType type = it.type();
		if(type == fs::RegularFile) {
			result[name] = Entry(type, std::string('\0', it.file_size()), it.last_write_time());
		} else {
			result[name] = Entry(type);
		}
		
		if(type == fs::Directory) {
			iterate(path / it.name(), name + "/", result);
		}
		
	}
	
}

void FilesystemTest::checkState(const fs::path & root) {
	
	// Check fs::directory_iterator
	Entries iterated;
	iterate(root, std::string(), iterated);
	BOOST_FOREACH(const Entries::value_type & entry, m_expected) {
		Entries::iterator it = m_expected.find(entry.first);
		CPPUNIT_ASSERT_MESSAGE(entry.first, it != m_expected.end());
		CPPUNIT_ASSERT_EQUAL_MESSAGE(entry.first, entry.second.type, it->second.type);
		if(entry.second.type == fs::RegularFile) {
			CPPUNIT_ASSERT_EQUAL_MESSAGE(entry.first, entry.second.contents.length(), it->second.contents.length());
			CPPUNIT_ASSERT_EQUAL_MESSAGE(entry.first, entry.second.mtime, it->second.mtime);
		}
	}
	
	// Check that there are no unexpected files
	BOOST_FOREACH(const std::string & path, m_all) {
		if(m_expected.find(path) == m_expected.end()) {
			CPPUNIT_ASSERT_MESSAGE(path, !fs::exists(root / path));
			CPPUNIT_ASSERT_EQUAL_MESSAGE(path, fs::get_type(root / path), fs::DoesNotExist);
			CPPUNIT_ASSERT_MESSAGE(path, iterated.find(path) == iterated.end());
		}
	}
	
	// Check all expected files
	BOOST_FOREACH(const Entries::value_type & entry, m_expected) {
		CPPUNIT_ASSERT_MESSAGE(entry.first, m_all.find(entry.first) != m_all.end());
		fs::path path = root / entry.first;
		fs::FileType type = fs::get_type(path);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(entry.first, type, entry.second.type);
		if(type == fs::RegularFile) {
			CPPUNIT_ASSERT_EQUAL_MESSAGE(entry.first, fs::file_size(path), u64(entry.second.contents.length()));
			CPPUNIT_ASSERT_EQUAL_MESSAGE(entry.first, fs::read(path), entry.second.contents);
			CPPUNIT_ASSERT_EQUAL_MESSAGE(entry.first, fs::last_write_time(path), entry.second.mtime);
		}
		CPPUNIT_ASSERT_MESSAGE(entry.first, iterated.find(entry.first) != iterated.end());
	}
	
}

void FilesystemTest::run(const fs::path & root) {
	
	m_all.insert("a");
	m_all.insert("a/b");
	m_all.insert("a/b/c");
	m_all.insert("a/b/c/filed.sav");
	m_all.insert("a/d");
	m_all.insert("a/e");
	m_all.insert("a/e/f");
	m_all.insert("a/fileb.bmp");
	m_all.insert("a/z");
	m_all.insert("filea.sav");
	m_all.insert("filec.sav");
	m_all.insert("filed.sav");
	m_all.insert("filez.sav");
	m_all.insert("g");
	m_all.insert("g/empty");
	m_all.insert("missingparent");
	m_all.insert("missingparent/filea.sav");
	m_all.insert("missingparent/subdir");
	m_all.insert("z");
	m_all.insert("z/empty");
	checkState(root);
	
	// Create directory tree
	CPPUNIT_ASSERT(fs::create_directories(root / "a/b/c"));
	m_expected["a"] = Entry(fs::Directory);
	m_expected["a/b"] = Entry(fs::Directory);
	m_expected["a/b/c"] = Entry(fs::Directory);
	checkState(root);
	
	// Create directory tree with existing parent
	CPPUNIT_ASSERT(fs::create_directories(root / "a/e/f"));
	m_expected["a/e"] = Entry(fs::Directory);
	m_expected["a/e/f"] = Entry(fs::Directory);
	checkState(root);
	
	// Create existing directory (noop)
	CPPUNIT_ASSERT(fs::create_directory(root / "a"));
	checkState(root);
	
	// Create new subdirectory
	CPPUNIT_ASSERT(fs::create_directory(root / "a/d"));
	m_expected["a/d"] = Entry(fs::Directory);
	checkState(root);
	
	// Created new directory
	CPPUNIT_ASSERT(fs::create_directory(root / "g"));
	m_expected["g"] = Entry(fs::Directory);
	checkState(root);
	
	// Fail to create subdirectory of missing parent
	CPPUNIT_ASSERT(!fs::create_directory(root / "missingparent/subdir"));
	checkState(root);
	
	// Fail to create file where a directory exists
	CPPUNIT_ASSERT(!fs::write(root / "a", std::string("tset")));
	checkState(root);
	
	// Create a file from a string
	std::string filea = "file a contents\n";
	filea += '\0';
	std::time_t before = std::time(NULL);
	CPPUNIT_ASSERT(fs::write(root / "filea.sav", filea));
	m_expected["filea.sav"] = Entry(fs::RegularFile, filea, checkTime(root, "filea.sav", before));
	checkState(root);
	
	// Create a file from a char array
	before = std::time(NULL);
	char fileb[] = { '\0', 'T', 'S', '\r', 'e', 'T' };
	CPPUNIT_ASSERT(fs::write(root / "a/fileb.bmp", fileb, sizeof(fileb)));
	m_expected["a/fileb.bmp"] = Entry(fs::RegularFile, std::string(fileb, sizeof(fileb)),
	                                  checkTime(root, "a/fileb.bmp", before));
	checkState(root);
	
	// Create an empty file
	before = std::time(NULL);
	CPPUNIT_ASSERT(fs::write(root / "g/empty", std::string()));
	m_expected["g/empty"] = Entry(fs::RegularFile, std::string(), checkTime(root, "g/empty", before));
	checkState(root);
	
	// Fail to copy file that does not exist
	CPPUNIT_ASSERT(!fs::copy_file(root / "filec.sav", root / "filec.sav", false));
	checkState(root);
	
	// Fail to copy file to name of existing directory
	CPPUNIT_ASSERT(!fs::copy_file(root / "filea.sav", root / "a", false));
	CPPUNIT_ASSERT(!fs::copy_file(root / "filea.sav", root / "a", true));
	checkState(root);
	
	// Fail to copy file to path with missing parent
	CPPUNIT_ASSERT(!fs::copy_file(root / "filea.sav", root / "missingparent/filea.sav", false));
	CPPUNIT_ASSERT(!fs::copy_file(root / "filea.sav", root / "missingparent/filea.sav", true));
	checkState(root);
	
	// Copy file to new target
	before = std::time(NULL);
	CPPUNIT_ASSERT(fs::copy_file(root / "filea.sav", root / "filec.sav", false));
	m_expected["filec.sav"] = m_expected["filea.sav"];
	m_expected["filec.sav"].mtime = checkTime(root, "filec.sav", before);
	checkState(root);
	
	// Copy file to new target
	before = std::time(NULL);
	CPPUNIT_ASSERT(fs::copy_file(root / "filea.sav", root / "filed.sav", true));
	m_expected["filed.sav"] = m_expected["filea.sav"];
	m_expected["filed.sav"].mtime = checkTime(root, "filed.sav", before);
	checkState(root);
	
	// Fail to copy file to new existing target
	CPPUNIT_ASSERT(!fs::copy_file(root / "a/fileb.bmp", root / "filed.sav", false));
	checkState(root);
	
	// Copy file to new existing target and overwrite
	before = std::time(NULL);
	CPPUNIT_ASSERT(fs::copy_file(root / "a/fileb.bmp", root / "filed.sav", true));
	m_expected["filed.sav"] = m_expected["a/fileb.bmp"];
	m_expected["filed.sav"].mtime = checkTime(root, "filed.sav", before);
	checkState(root);
	
	// Fail to rename a file that does not exist
	CPPUNIT_ASSERT(!fs::rename(root / "missingparent", root / "z"));
	checkState(root);
	
	// Fail to rename directory to name of existing directory
	CPPUNIT_ASSERT(!fs::rename(root / "g", root / "a"));
	checkState(root);
	
	// Fail to rename directory to subdirectory of itself
	CPPUNIT_ASSERT(!fs::rename(root / "a", root / "a/z"));
	checkState(root);
	
	// Fail to rename directory to name of existing file
	CPPUNIT_ASSERT(!fs::rename(root / "g", root / "filea.sav"));
	checkState(root);
	
	// Fail to rename directory to path with missing parent
	CPPUNIT_ASSERT(!fs::rename(root / "filea.sav", root / "missingparent/filea.sav"));
	checkState(root);
	
	// Rename directory to new target
	CPPUNIT_ASSERT(fs::rename(root / "g", root / "z"));
	m_expected["z"] = m_expected["g"];
	m_expected["z/empty"] = m_expected["g/empty"];
	m_expected.erase("g/empty");
	m_expected.erase("g");
	checkState(root);
	
	// Fail to rename file to name of existing directory
	CPPUNIT_ASSERT(!fs::rename(root / "filea.sav", root / "a"));
	checkState(root);
	
	// Fail to rename file to path with missing parent
	CPPUNIT_ASSERT(!fs::rename(root / "filea.sav", root / "missingparent/filea.sav"));
	checkState(root);
	
	// Rename file in same directory
	CPPUNIT_ASSERT(fs::rename(root / "filea.sav", root / "filez.sav"));
	m_expected["filez.sav"] = m_expected["filea.sav"];
	m_expected.erase("filea.sav");
	checkState(root);
	
	// Fail to rename file because the target exists
	CPPUNIT_ASSERT(!fs::rename(root / "filec.sav", root / "filed.sav", false));
	checkState(root);
	
	// Rename file and overwrite
	CPPUNIT_ASSERT(fs::rename(root / "filec.sav", root / "filed.sav", true));
	m_expected["filed.sav"] = m_expected["filec.sav"];
	m_expected.erase("filec.sav");
	checkState(root);
	
	// Move file to new parent
	CPPUNIT_ASSERT(fs::rename(root / "filed.sav", root / "a/b/c/filed.sav", true));
	m_expected["a/b/c/filed.sav"] = m_expected["filed.sav"];
	m_expected.erase("filed.sav");
	checkState(root);
	
	// Fail to remove a file that is a directory
	CPPUNIT_ASSERT(!fs::remove(root / "a/e/f"));
	checkState(root);
	
	// Remove a file where the parent is a file (noop)
	CPPUNIT_ASSERT(fs::remove(root / "filez.sav/a"));
	checkState(root);
	
	// Fail to remove a directory that is a file
	CPPUNIT_ASSERT(!fs::remove_directory(root / "filez.sav"));
	checkState(root);
	
	// Remove a directory where the parent is a file (noop)
	CPPUNIT_ASSERT(fs::remove_directory(root / "filez.sav/a"));
	checkState(root);
	
	// Remove a directory
	CPPUNIT_ASSERT(fs::remove_directory(root / "a/e/f"));
	m_expected.erase("a/e/f");
	checkState(root);
	
	// Remove a directory that does not exist (noop)
	CPPUNIT_ASSERT(fs::remove_directory(root / "a/e/f"));
	checkState(root);
	
	// Remove a directory where the parent does not exist
	CPPUNIT_ASSERT(fs::remove_directory(root / "missingparent/subdir"));
	checkState(root);
	
	// Remove a file
	CPPUNIT_ASSERT(fs::remove(root / "filez.sav"));
	m_expected.erase("filez.sav");
	checkState(root);
	
	// Remove a file that does not exist (noop)
	CPPUNIT_ASSERT(fs::remove(root / "filez.sav"));
	checkState(root);
	
	// Remove a file whered the parent does not exist
	CPPUNIT_ASSERT(fs::remove(root / "missingparent/filea.sav"));
	checkState(root);
	
	// Remove a directory tree
	CPPUNIT_ASSERT(fs::remove_all(root / "a/b"));
	m_expected.erase("a/b/c/filed.sav");
	m_expected.erase("a/b/c");
	m_expected.erase("a/b");
	checkState(root);
	
	// Remove a file
	CPPUNIT_ASSERT(fs::remove_all(root / "z/empty"));
	m_expected.erase("z/empty");
	checkState(root);
	
	// Remove a file that does not exist
	CPPUNIT_ASSERT(fs::remove_all(root / "z/empty"));
	m_expected.erase("z/empty");
	checkState(root);
	
	// Remove a directory where the parent does not exist
	CPPUNIT_ASSERT(fs::remove_all(root / "missingparent/subdir"));
	checkState(root);
	
	// TODO test symbolic links
	
}

void FilesystemTest::testRelative() {
	run(m_relroot);
	checkState(m_absroot);
}

void FilesystemTest::testAbsolute() {
	run(m_absroot);
	checkState(m_relroot);
}
