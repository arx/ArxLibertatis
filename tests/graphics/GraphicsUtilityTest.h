#ifndef ARX_GRAPHICS_GRAPHICSUTILITYTEST_H
#define ARX_GRAPHICS_GRAPHICSUTILITYTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "graphics/GraphicsUtility.h"

class GraphicsUtilityTest : public CppUnit::TestCase {
	CPPUNIT_TEST_SUITE(GraphicsUtilityTest);
	CPPUNIT_TEST(front);
	CPPUNIT_TEST(back);
	CPPUNIT_TEST(translation);
	CPPUNIT_TEST(testMatrix1);
	CPPUNIT_TEST(testMatrix2);
	CPPUNIT_TEST(testMatrix3);
	CPPUNIT_TEST(testMatrix4);
	CPPUNIT_TEST(testMatrix5);
	CPPUNIT_TEST(testMatrix6);
	CPPUNIT_TEST(testMatrix7);
	CPPUNIT_TEST_SUITE_END();
public:
	GraphicsUtilityTest() : CppUnit::TestCase("GraphicsUtilityTest") {}

	void front();
	void back();
	void translation();
	
	void testMatrix1();
	void testMatrix2();
	void testMatrix3();
	void testMatrix4();
	void testMatrix5();
	void testMatrix6();
	void testMatrix7();

private:
	EERIE_TRANSFORM transform;
	EERIEMATRIX matrix;
	EERIEMATRIX expected;
};

#endif
