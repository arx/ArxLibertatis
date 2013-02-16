/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#include <cppunit/TestCase.h>
#include "graphics/Math.h"

class VectorTest : public CppUnit::TestCase {
public:
  VectorTest( std::string name ) : CppUnit::TestCase( name ) {}

  void runTest() {
   
    EERIEMATRIX a, b;
    
	//MatrixReset(&a);
	//MatrixReset(&b);
	CPPUNIT_ASSERT( a._11 == b._11 );
  }
};
