
include(CompileCheck)

option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_NOISY_WARNING_FLAGS "Enable noisy compiler warnings" OFF)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)


if(MSVC)
	
	if(SET_WARNING_FLAGS AND NOT SET_NOISY_WARNING_FLAGS)
		
		# Disable deprecation warnings
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
		add_definitions(-D_SCL_SECURE_NO_WARNINGS)
		
		# TODO TEMP - disable very noisy warning
		# Conversion from 'A' to 'B', possible loss of data
		add_definitions(/wd4244)
		
		# TODO TEMP - disable warning caused by conversion from a 64-bit type to a 32-bit one...
		if(CMAKE_CL_64)
			# Conversion from 'size_t' to 'xxx', possible loss of data
			add_definitions(/wd4267)
		endif()
		
		# warning C4127: conditional expression is constant
		add_definitions(/wd4127)
		# warning C4250: 'xxx' : inherits 'std::basic_{i,o}stream::...' via dominance
		add_definitions(/wd4250) # harasses you when inheriting from std::basic_{i,o}stream
		# warning C4503: 'xxx' : decorated name length exceeded, name was truncated
		add_definitions(/wd4503)
		
	endif()
	
	if(NOT DEBUG_EXTRA)
		add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
		add_definitions(-D_SECURE_SCL=0)
		add_definitions(-D_SECURE_SCL_THROWS=0)
		add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		
		# Disable exceptions & rtti
		add_definitions(/GR-) # No RTTI
		foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
			string(REGEX REPLACE "/GR( |$)" "/GR-\\1" ${flag_var} "${${flag_var}}")
		endforeach(flag_var)
		
		# Enable multiprocess build
		add_definitions(/MP)
		
	endif()
	
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
		
		# Disable Run time checks
		if(NOT DEBUG_EXTRA)
			string(REGEX REPLACE "/RTC1" "" ${flag_var} "${${flag_var}}")
		endif()
		
		# Change runtime library from "Multi-threaded Debug DLL" to "Multi-threaded DLL"
		if(NOT DEBUG_EXTRA)
			string(REGEX REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")
		endif()
		
		# Remove definition of _DEBUG as it might conflict with libs we're linking with
		string(REGEX REPLACE "/D_DEBUG" "/DNDEBUG" ${flag_var} "${${flag_var}}")
		
		# Force to always compile with warning level 3
		string(REGEX REPLACE "/W[0-4]" "/W3" ${flag_var} "${${flag_var}}")
		
	endforeach(flag_var)
	
	# Avoid warning during link
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
	
	# Disable randomized base address (for better callstack matching)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DYNAMICBASE:NO")
	
	# Always generate a PDB file
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
	
	# Enable compiler optimization in release
	set(CMAKE_CXX_FLAGS_RELEASE
	    "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Oi /Ot /GL /GS- /fp:fast")
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /arch:SSE2")
	endif()
	
	# Always build with debug information
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	
	# Enable linker optimization in release
	#  /OPT:REF   Eliminate unreferenced code
	#  /OPT:ICF   COMDAT folding (merge functions generating the same code)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE
		"${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /LTCG")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE
		"${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF /LTCG")
	set(CMAKE_STATIC_LINKER_FLAGS_RELEASE
		"${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
	
else(MSVC)
	
	if(SET_WARNING_FLAGS)
		
		# GCC or Clang (and compatible)
		
		add_cxxflag("-Wall")
		add_cxxflag("-Wextra")
		
		add_cxxflag("-Warray-bounds=2")
		add_cxxflag("-Wcast-qual")
		add_cxxflag("-Wcatch-value=3")
		add_cxxflag("-Wdocumentation")
		add_cxxflag("-Wdouble-promotion")
		add_cxxflag("-Wduplicated-cond")
		add_cxxflag("-Wextra-semi")
		add_cxxflag("-Wformat=2")
		add_cxxflag("-Wheader-guard")
		add_cxxflag("-Wlogical-op")
		add_cxxflag("-Wmissing-declarations")
		add_cxxflag("-Woverflow")
		add_cxxflag("-Woverloaded-virtual")
		add_cxxflag("-Wpessimizing-move")
		add_cxxflag("-Wpointer-arith")
		add_cxxflag("-Wredundant-decls")
		add_cxxflag("-Wshift-overflow")
		add_cxxflag("-Wstringop-overflow=4")
		add_cxxflag("-Wundef")
		add_cxxflag("-Wunused-const-variable=1")
		add_cxxflag("-Wunused-macros")
		add_cxxflag("-Wvla")
		
		add_cxxflag("-Wliteral-conversion") # part of -Wconversion
		add_cxxflag("-Wbool-conversion") # part of -Wconversion
		add_cxxflag("-Wfloat-conversion") # part of -Wconversion
		add_cxxflag("-Wstring-conversion") # part of -Wconversion
		
		if(SET_NOISY_WARNING_FLAGS OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
			# TODO for some reason this warns in /usr/include/boost/type_traits/alignment_of.hpp for -m32 builds
			add_cxxflag("-Wduplicated-branches")
		endif()
		
		if(SET_NOISY_WARNING_FLAGS)
			
			# These are too noisy to enable right now but we still want to track new warnings.
			# TODO enable by default as soon as most are silenced
			add_cxxflag("-Wconversion") # very noisy
			# add_cxxflag("-Wsign-conversion") # part of -Wconversion
			# add_cxxflag("-Wshorten-64-to-32") # part of -Wconversion
			add_cxxflag("-Wshadow") # very noisy
			add_cxxflag("-Wstrict-aliasing=1") # has false positives
			add_cxxflag("-Wuseless-cast") # has false positives
			# add_cxxflag("-Wnull-dereference") not that useful without deduction path
			
		else()
			
			# icc
			if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
				# '... was declared but never referenced'
				# While normally a sensible warning, it also fires when a member isn't used for
				# *all* instantiations of a template class, making the warning too annoying to
				# be useful
				add_cxxflag("-wd177")
				# 'external function definition with no prior declaration'
				# This gets annoying fast with small inline/template functions.
				add_cxxflag("-wd1418")
				# 'printf/scanf format not a string literal and no format arguments'
				# balks when passing NULL as the format string
				add_cxxflag("-wd2279")
			endif()
			
			# -Wuninitialized causes too many false positives in older gcc versions
			if(CMAKE_COMPILER_IS_GNUCXX)
				# GCC is 'clever' and silently accepts -Wno-*  - check for the non-negated variant
				check_compiler_flag(FLAG_FOUND "-Wmaybe-uninitialized")
				if(FLAG_FOUND)
					add_cxxflag("-Wno-maybe-uninitialized")
				endif()
				if(NOT FLAG_FOUND)
					add_cxxflag("-Wno-uninitialized")
				endif()
			endif()
			
			# Xcode does not support -isystem yet
			if(MACOS)
				add_cxxflag("-Wno-undef")
			endif()
			
		endif()
		
	endif(SET_WARNING_FLAGS)
	
	if(DEBUG_EXTRA)
		add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
		add_cxxflag("-fbounds-checking")
		add_cxxflag("-fcatch-undefined-behavior")
		add_cxxflag("-fstack-protector-all")
		add_cxxflag("-fsanitize=address")
		add_cxxflag("-fsanitize=thread")
		add_cxxflag("-fsanitize=leak")
		add_cxxflag("-fsanitize=undefined")
	endif(DEBUG_EXTRA)
	
	if(CMAKE_BUILD_TYPE STREQUAL "")
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		
		add_cxxflag("-fno-rtti")
		
		if(MACOS)
			# TODO For some reason this check succeeds on macOS, but then
			# flag causes the actual build to fail :(
		else()
			# Link as few libraries as possible
			# This is much easier than trying to decide which libraries are needed for each
			# system
			add_ldflag("-Wl,--as-needed")
		endif()
		
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			
			# set debug symbol level to -g3
			check_compiler_flag(RESULT "-g3")
			if(NOT RESULT STREQUAL "")
				string(REGEX REPLACE "-g(|[0-9]|gdb)" "" CMAKE_CXX_FLAGS_DEBUG
				       "${CMAKE_CXX_FLAGS_DEBUG}")
				set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
			endif()
			
			# disable optimizations
			check_compiler_flag(RESULT "-O0")
			string(REGEX REPLACE "-O[0-9]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
			
		elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
			
			if((NOT CMAKE_CXX_FLAGS MATCHES "-g(|[0-9]|gdb)")
			   AND (NOT CMAKE_CXX_FLAGS_RELEASE MATCHES "-g(|[0-9]|gdb)"))
				add_cxxflag("-g2")
			endif()
			
			add_cxxflag("-ffast-math")
			
		endif()
		
	endif(SET_OPTIMIZATION_FLAGS)
	
endif(MSVC)

set(BUILD_TYPES ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
list(REMOVE_DUPLICATES BUILD_TYPES)
