
# Try to find the CppUnit library and include path.
# Once done this will define
#
# CppUnit_FOUND
# CppUnit_INCLUDE_DIR   Where to find cppunit/*.h
# CppUnit_LIBRARIES     The cppunit library
# CppUnit_DEFINITIONS   Definitions to use when compiling code that uses cppunit
#
# Typical usage could be something like:
#   find_package(CppUnit REQUIRED)
#   include_directories(SYSTEM ${CppUnit_INCLUDE_DIR})
#   add_definitions(${CppUnit_DEFINITIONS})
#   ...
#   target_link_libraries(myexe ${CppUnit_LIBRARIES})
#
# The following additional options can be defined before the find_package() call:
# CppUnit_USE_STATIC_LIBS  Statically link against cppunit (default: OFF)

if(UNIX)
	find_package(PkgConfig QUIET)
	pkg_check_modules(_PC_CppUnit cppunit)
endif()

include(UseStaticLibs)
use_static_libs(CppUnit _PC_CppUnit)


set(_CppUnit_SEARCH_PATHS
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
)


find_path(CppUnit_INCLUDE_DIR cppunit/TestCase.h
	HINTS
		${_PC_CppUnit_INCLUDE_DIRS}
	PATH_SUFFIXES include
	PATHS ${_CppUnit_SEARCH_PATHS}
	DOC "The directory where cppunit/*.h resides"
)
mark_as_advanced(CppUnit_INCLUDE_DIR)

# Prefer libraries in the same prefix as the include files
string(REGEX REPLACE "(.*)/include/?" "\\1" CppUnit_BASE_DIR "${CppUnit_INCLUDE_DIR}")

find_library(CppUnit_LIBRARY
	NAMES cppunit
	HINTS
		${_PC_CppUnit_LIBRARY_DIRS}
		"${CppUnit_BASE_DIR}/lib"
	PATH_SUFFIXES lib64 lib
	PATHS ${_CppUnit_SEARCH_PATHS}
	DOC "The CppUnit library"
)
mark_as_advanced(CppUnit_LIBRARY)

use_static_libs_restore()

set(CppUnit_DEFINITIONS -DGL_GLEXT_PROTOTYPES)
if(WIN32 AND CppUnit_USE_STATIC_LIBS)
	list(APPEND CppUnit_DEFINITIONS -DCppUnit_STATIC)
endif()

foreach(header IN ITEMS "config-auto.h" "Portability.h")
	set(_cppunit_VERSION_HEADER "${CppUnit_INCLUDE_DIR}/cppunit/${header}")
	if(CppUnit_INCLUDE_DIR AND EXISTS "${_cppunit_VERSION_HEADER}")
		file(STRINGS "${_cppunit_VERSION_HEADER}" _cppunit_VERSION_DEFINE REGEX "#define[ \t]+CPPUNIT_VERSION[ \t]+")
		if(_cppunit_VERSION_DEFINE MATCHES "#define[ \t]+CPPUNIT_VERSION[ \t]+\"([0-9]+\\.[0-9]+(\\.[0-9]+)?)")
			set(CppUnit_VERSION_STRING ${CMAKE_MATCH_1})
			break()
		endif()
	endif()
endforeach()
unset(_cppunit_VERSION_DEFINE)
unset(_cppunit_VERSION_HEADER)
unset(_cppunit_VERSION_HEADERS)
if(UNIX AND NOT DEFINED CppUnit_VERSION_STRING AND _PC_CppUnit_FOUND)
	set(CppUnit_VERSION_STRING "${_PC_CppUnit_VERSION}")
endif()

# handle the QUIETLY and REQUIRED arguments and set CppUnit_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CppUnit REQUIRED_VARS CppUnit_LIBRARY CppUnit_INCLUDE_DIR
                                  VERSION_VAR CppUnit_VERSION_STRING)

if(CPPUNIT_FOUND)
	set(CppUnit_LIBRARIES ${CppUnit_LIBRARY})
endif()
