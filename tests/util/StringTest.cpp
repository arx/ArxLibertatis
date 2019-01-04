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

#include "StringTest.h"

#include "platform/Platform.h"
#include "util/String.h"

CPPUNIT_TEST_SUITE_REGISTRATION(StringTest);

struct OverflowTestStruct {
	char data[4];
	unsigned char canary[4];
	
	OverflowTestStruct() {
		std::fill_n(data, 4, 0xCC);
		std::fill_n(canary, 4, 0xDD);
	}
	
	bool checkCanary() {
		unsigned char expected[] = {0xDD, 0xDD, 0xDD, 0xDD};
		
		return std::equal(expected, expected + 4, canary);
	}
};

void StringTest::stringStoreEmptyTest() {
	OverflowTestStruct target;
	
	util::storeString(target.data, std::string(""));
	
	char expected[] = {0x00, 0x00, 0x00, 0x00};
	
	CPPUNIT_ASSERT(std::equal(expected, expected + 4, target.data));
	CPPUNIT_ASSERT(target.checkCanary());
}

void StringTest::stringStoreFittingTest() {
	OverflowTestStruct target;
	
	util::storeString(target.data, std::string("123"));
	
	char expected[] = {'1', '2', '3', 0x00};
	
	CPPUNIT_ASSERT(std::equal(expected, expected + 4, target.data));
	CPPUNIT_ASSERT(target.checkCanary());
}

void StringTest::stringStoreTruncatedNullTest() {
	OverflowTestStruct target;
	
	util::storeString(target.data, std::string("1234"));
	
	char expected[] = {'1', '2', '3', '4'};
	
	CPPUNIT_ASSERT(std::equal(expected, expected + 4, target.data));
	CPPUNIT_ASSERT(target.checkCanary());
}

void StringTest::stringStoreOverflowTest() {
	OverflowTestStruct target;
	
	util::storeString(target.data, std::string("123456"));
	
	char expected[] = {'1', '2', '3', '4'};
	
	CPPUNIT_ASSERT(std::equal(expected, expected + 4, target.data));
	CPPUNIT_ASSERT(target.checkCanary());
}


void StringTest::stringStoreTerminatedEmptyTest() {
	OverflowTestStruct target;
	
	util::storeStringTerminated(target.data, std::string(""));
	
	char expected[] = {0x00, 0x00, 0x00, 0x00};
	
	CPPUNIT_ASSERT(std::equal(expected, expected + 4, target.data));
	CPPUNIT_ASSERT(target.checkCanary());
}

void StringTest::stringStoreTerminatedFittingTest() {
	OverflowTestStruct target;
	
	util::storeStringTerminated(target.data, std::string("123"));
	
	char expected[] = {'1', '2', '3', 0x00};
	
	CPPUNIT_ASSERT(std::equal(expected, expected + 4, target.data));
	CPPUNIT_ASSERT(target.checkCanary());
}

void StringTest::stringStoreTerminatedOverflowTest() {
	OverflowTestStruct target;
	
	util::storeStringTerminated(target.data, std::string("123456"));
	
	char expected[] = {'1', '2', '3', 0x00};
	
	CPPUNIT_ASSERT(std::equal(expected, expected + 4, target.data));
	CPPUNIT_ASSERT(target.checkCanary());
}

void StringTest::safeGetExactTest() {
	
	u8 data[] = {0xAA, 0xAA, 0xAA, 0xAA};
	u32 remaining = sizeof(data);
	u8 * dataPtr = &data[0];
	
	s32 resultValue;
	bool resultOk = util::safeGet(resultValue, dataPtr, remaining);
	
	CPPUNIT_ASSERT_EQUAL(true, resultOk);
	CPPUNIT_ASSERT_EQUAL(0u, remaining);
	CPPUNIT_ASSERT_EQUAL(-1431655766, resultValue);
}

void StringTest::safeGetTooSmallTest() {
	
	u8 data[] = {0xAA, 0xAA, 0xAA};
	u32 remaining = sizeof(data);
	u8 * dataPtr = &data[0];
	
	s32 resultValue = -1;
	bool resultOk = util::safeGet(resultValue, dataPtr, remaining);
	
	CPPUNIT_ASSERT_EQUAL(false, resultOk);
	CPPUNIT_ASSERT_EQUAL(3u, remaining);
	CPPUNIT_ASSERT_EQUAL(-1, resultValue);
}
