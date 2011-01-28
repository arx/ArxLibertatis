#include <cppunit/TestCase.h>
#include "renderer/EERIEMath.h"

class VectorTest : public CppUnit::TestCase {
public:
  VectorTest( std::string name ) : CppUnit::TestCase( name ) {}

  void runTest() {

    CPPUNIT_ASSERT( 0 == 0 );
  }
};


int main(int argc, char *argv[]) {
  VectorTest foo("bar");
  foo.runTest();
}
