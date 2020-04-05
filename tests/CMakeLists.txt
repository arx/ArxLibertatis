
find_package(CppUnit REQUIRED)

include_directories(
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}
)

set(arxtest_TESTCASES
	
	tests/TestMain.cpp
	
	tests/graphics/ColorTest.h
	tests/graphics/ColorTest.cpp
	
	tests/io/fs/FilePathTest.h
	tests/io/fs/FilePathTest.cpp
	
	tests/io/fs/FilesystemTest.h
	tests/io/fs/FilesystemTest.cpp
	
	tests/io/resource/ResourcePathTest.h
	tests/io/resource/ResourcePathTest.cpp
	
	tests/io/IniTest.h
	tests/io/IniTest.cpp
	
	tests/math/AssertionTraits.h
	tests/math/LegacyMath.h
	tests/math/LegacyMathTest.cpp
	
	tests/util/StringTest.h
	tests/util/StringTest.cpp
	
)

# CppUnit requires RTTI and does not work with libstdc++'s debug mode
add_source_flags("${arxtest_TESTCASES}" "${enable_rtti} ${disable_libstdcxx_debug}")

set(arxtest_SOURCES
	
	${BASE_SOURCES}
	
	src/game/Camera.cpp
	src/graphics/Math.cpp
	src/graphics/Renderer.cpp
	src/io/IniSection.cpp
	src/io/resource/ResourcePath.cpp
	
	${arxtest_TESTCASES}
	
)

set(arxtest_INCLUDES
	${CppUnit_INCLUDE_DIR}
)

set(arxtest_LIBRARIES ${BASE_LIBRARIES} ${CppUnit_LIBRARIES})

add_executable_shared(arxtest "${arxtest_SOURCES}" "${arxtest_LIBRARIES}")
add_binary_includes(arxtest ${arxtest_INCLUDES})
set_binary_installdir(arxtest "")
