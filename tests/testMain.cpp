
#include <cppunit/ui/text/TestRunner.h>

#include "graphics/GraphicsUtilityTest.h"

int main(int argc, char *argv[]) {
	CppUnit::TextUi::TestRunner testRunner;
	testRunner.addTest(new GraphicsUtilityTest());

	bool ok = testRunner.run();
	//return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
