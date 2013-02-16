#ifndef ARX_GRAPHICS_GRAPHICSUTILITYTEST_H
#define ARX_GRAPHICS_GRAPHICSUTILITYTEST_H

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include "graphics/GraphicsUtility.h"

class GraphicsUtilityTest : public CppUnit::TestCase {
	CPPUNIT_TEST_SUITE(GraphicsUtilityTest);
	CPPUNIT_TEST(front);
	CPPUNIT_TEST(back);
	CPPUNIT_TEST(edgeCase1);
	CPPUNIT_TEST(edgeCase2);
	CPPUNIT_TEST(translation);
	CPPUNIT_TEST(combined);
	CPPUNIT_TEST_SUITE_END();
public:
	GraphicsUtilityTest() : CppUnit::TestCase("GraphicsUtilityTest") {}

	void front();
	void back();
	void edgeCase1();
	void edgeCase2();
	void translation();
	void combined();

private:
	EERIE_TRANSFORM transform;
	EERIEMATRIX matrix;
	EERIEMATRIX expected;
};

#endif
