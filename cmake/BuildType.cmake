
include(CompileCheck)

option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)

if(MSVC)
	
	if(SET_WARNING_FLAGS)
		
		# Disable deprecation warnings
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
		add_definitions(-D_SCL_SECURE_NO_WARNINGS)
		# 'func': name was marked as #pragma deprecated
		add_definitions(/wd4995)
		
		# TEMP - disable warning caused by the F2L removal
		# Conversion from 'float' to 'long', possible loss of data
		add_definitions(/wd4244)
		
		# TEMP - disable warning caused by conversion from a 64-bit type to a 32-bit one...
		if(CMAKE_CL_64)
			# Conversion from 'size_t' to 'xxx', possible loss of data
			add_definitions(/wd4267)
		endif()
		
		# warning C4127: conditional expression is constant
		add_definitions(/wd4127)
		# warning C4201: nonstandard extension used : nameless struct/union
		add_definitions(/wd4201)
		# warning C4503: 'xxx' : decorated name length exceeded, name was truncated
		add_definitions(/wd4503)
		
	endif(SET_WARNING_FLAGS)
	
	if(NOT DEBUG_EXTRA)
		add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
		add_definitions(-D_SECURE_SCL=0)
		add_definitions(-D_SECURE_SCL_THROWS=0)
		add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
	endif(NOT DEBUG_EXTRA)
	
	if(SET_OPTIMIZATION_FLAGS)
		
		# Disable exceptions & rtti
		add_definitions(/GR-)           # No RTTI
		
		# Enable multiprocess build
		add_definitions(/MP)
		
	endif(SET_OPTIMIZATION_FLAGS)
	
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
		
		# Disable Run time checks
		string(REGEX REPLACE "/RTC1" "" ${flag_var} "${${flag_var}}")
		
		# Change runtime library from "Multi-threaded Debug DLL" to "Multi-threaded DLL"
		string(REGEX REPLACE "/MDd" "/MD" ${flag_var} "${${flag_var}}")
		
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
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}
	    /Ox /Oi /Ot /GL /arch:SSE2 /GS- /fp:fast")
	
	# Always build with debug information
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	
	# Enable linker optimization in release
	#  /OPT:REF   Eliminate unreferenced code
	#  /OPT:ICF   COMDAT folding (merge functions generating the same code)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}
	    /OPT:REF /OPT:ICF /LTCG")
	
else(MSVC)
	
	if(SET_WARNING_FLAGS)
		
		# GCC (and compatible)
		add_cxxflag("-Wall")
		add_cxxflag("-Wextra")
		add_cxxflag("-Wformat=2")
		add_cxxflag("-Wundef")
		add_cxxflag("-Wpointer-arith")
		add_cxxflag("-Wcast-qual")
		add_cxxflag("-Woverloaded-virtual")
		add_cxxflag("-Wlogical-op")
		add_cxxflag("-Woverflow")
		
		# TODO consider adding these
		# add_cxxflag("-Wconversion") # very noisy
		# add_cxxflag("-Wsign-conversion") # very noisy
		# to catch functions that should be marked as static:
		# add_cxxflag("-Wmissing-declarations")
		# to catch extern definitions in .cpp files (with UNITYBUILD):
		# add_cxxflag("-Wredundant-decls")
		
		# clang
		add_cxxflag("-Wliteral-conversion")
		add_cxxflag("-Wshift-overflow")
		add_cxxflag("-Wbool-conversions")
		
		if(NOT DEBUG_EXTRA)
			
			# icc
			if(${CMAKE_CXX_COMPILER} MATCHES "(^|/)icp?c$")
				# '... was declared but never referenced'
				# While normally a sensible warning, it also fires when a member isn't used for
				# *all* instantiations of a template class, making the warning too annoying to
				# be useful
				add_cxxflag("-wd177")
				# 'external function definition with no prior declaration'
				# This gets annoying fast with small inline/template functions.
				add_cxxflag("-wd1418")
			endif()
			
			# -Wuninitialized causes too many false positives
			add_cxxflag("-Wno-uninitialized")
			
			# (clang only) Conflicts with using const variables for configuration.
			add_cxxflag("-Wno-constant-logical-operand")
			
			# Xcode does not support -isystem yet
			if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
				add_cxxflag("-Wno-undef")
			endif()
			
		endif(NOT DEBUG_EXTRA)
		
	endif(SET_WARNING_FLAGS)
	
	if(DEBUG_EXTRA)
		add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
		add_cxxflag("-fbounds-checking")
		add_cxxflag("-fcatch-undefined-behavior")
		add_cxxflag("-Wstrict-aliasing=1")
	endif(DEBUG_EXTRA)
	
	if(CMAKE_BUILD_TYPE STREQUAL "")
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_definitions(-D_DEBUG)
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		
		# Link as few libraries as possible
		# This is much easier than trying to decide which libraries are needed for each system
		add_ldflag("-Wl,--as-needed")
	
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
			
		endif()
		
	endif(SET_OPTIMIZATION_FLAGS)
	
endif(MSVC)
