
#include <cppunit/TestCase.h>
#include "graphics/Math.h"

class VectorTest : public CppUnit::TestCase {
public:
  VectorTest( std::string name ) : CppUnit::TestCase( name ) {}

  void runTest() {
   
    EERIEMATRIX a, b;
    
    MatrixReset(&a);
	MatrixReset(&b);
	CPPUNIT_ASSERT( a._11 == b._11 );
  }
};


int main(int argc, char *argv[]) {
  VectorTest foo("bar");
  foo.runTest();
}
