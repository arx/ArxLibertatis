/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include <cppunit/TextTestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/extensions/HelperMacros.h>

#include "platform/Platform.h"

int main(int argc, char *argv[]) {
	ARX_UNUSED(argc);
	ARX_UNUSED(argv);
	
	CppUnit::TextTestRunner runner;
	
	runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cout, "%f:%l "));
	runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
	
	runner.run("");
	CppUnit::TestResultCollector & result = runner.result();
	
	return result.wasSuccessful() ? EXIT_SUCCESS : EXIT_FAILURE;
}
