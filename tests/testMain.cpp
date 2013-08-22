
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "graphics/ColorTest.h"
#include "graphics/GraphicsUtilityTest.h"

int main(int argc, char *argv[]) {
	CppUnit::TextUi::TestRunner testRunner;

	CppUnit::Test* tp = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
	testRunner.addTest(tp);

	bool ok = testRunner.run();

	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
