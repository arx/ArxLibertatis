
include(CompileCheck)

if(NOT MSVC AND NOT CMAKE_VERSION VERSION_LESS 2.8.6)
	include(CheckCXXSymbolExists)
	check_cxx_symbol_exists(_LIBCPP_VERSION "cstddef" IS_LIBCXX)
	if(IS_LIBCXX AND (DEBUG OR DEBUG_EXTRA))
		check_cxx_symbol_exists(_LIBCPP_ENABLE_ASSERTIONS "version" ARX_HAVE_LIBCPP_ENABLE_ASSERTIONS)
	endif()
else()
	set(IS_LIBCXX OFF)
endif()


option(DEBUG_EXTRA "Expensive debug options" OFF)
option(SET_WARNING_FLAGS "Adjust compiler warning flags" ON)
option(SET_NOISY_WARNING_FLAGS "Enable noisy compiler warnings" OFF)
option(SET_OPTIMIZATION_FLAGS "Adjust compiler optimization flags" ON)

set(conservative_warnings)
set(enable_rtti)
set(disable_libstdcxx_debug)

if(MSVC)
	
	if(USE_LTO)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")
		set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
	endif()
	
	if(FASTLINK)
		
		# Optimize for link speed in developer builds
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG:FASTLINK")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG:FASTLINK")
		
	elseif(SET_OPTIMIZATION_FLAGS)
		
		# Merge symbols and discard unused symbols
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
		
	endif()
	
	if(SET_WARNING_FLAGS AND NOT SET_NOISY_WARNING_FLAGS)
		
		# Disable deprecation warnings
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
		add_definitions(-D_SCL_SECURE_NO_WARNINGS)
		
		# TODO TEMP - disable very noisy warning
		# Conversion from 'A' to 'B', possible loss of data
		add_definitions(/wd4244)
		add_definitions(/wd4267)
		
		# warning C4127: conditional expression is constant
		add_definitions(/wd4127)
		# warning C4201: nonstandard extension used: nameless struct/union
		add_definitions(/wd4201) # used in GLM
		# warning C4324: 'xxx': structure was padded due to alignment specifier
		add_definitions(/wd4324)
		# warning C4701: potentially uninitialized local variable 'xxx' used
		add_definitions(/wd4701)
		# warning C4703: potentially uninitialized local pointer variable 'xxx' used
		add_definitions(/wd4703)
		
	endif()
	
	if(WERROR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
	endif()
	
	if(NOT DEBUG_EXTRA)
		add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
		add_definitions(-D_SECURE_SCL=0)
		add_definitions(-D_SECURE_SCL_THROWS=0)
		add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		
		# Enable exceptions
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
		
		# Disable RTTI
		add_definitions(/GR-) # No RTTI
		if(CMAKE_VERSION VERSION_LESS 3.20)
			foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
				string(REGEX REPLACE "/GR( |$)" "" ${flag_var} "${${flag_var}}")
				set(${flag_var} "${${flag_var}} /GR-")
			endforeach(flag_var)
		endif()
		list(APPEND enable_rtti /GR)
		
	endif()
	
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
		
		# Disable Run time checks
		if(NOT DEBUG_EXTRA)
			string(REGEX REPLACE "(^| )/RTC1( |$)" "\\1" ${flag_var} "${${flag_var}}")
		endif()
		
		# Change runtime library from "Multi-threaded Debug DLL" to "Multi-threaded DLL"
		if(NOT DEBUG_EXTRA)
			string(REGEX REPLACE "(^| )/MDd( |$)" "\\1" ${flag_var} "${${flag_var}}")
			set(${flag_var} "${${flag_var}} /MD")
			set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
		endif()
		
		# Remove definition of _DEBUG as it might conflict with libs we're linking with
		string(REGEX REPLACE "(^| )/D_DEBUG( |$)" "\\1" ${flag_var} "${${flag_var}}")
		
		if(DEBUG)
			string(REGEX REPLACE "(^| )/DNDEBUG( |$)" "\\1" ${flag_var} "${${flag_var}}")
		else()
			set(${flag_var} "${${flag_var}} /DNDEBUG")
		endif()
		
		# Force compiler warning level
		if(SET_WARNING_FLAGS)
			string(REGEX REPLACE "(^| )/W[0-4]( |$)" "\\1" ${flag_var} "${${flag_var}}")
			set(${flag_var} "${${flag_var}} /W4")
		endif()
		
	endforeach(flag_var)
	
	add_definitions(/utf-8)
	
	# Turn on standards compliant mode
	# /Za is not compatible with /fp:fast, leave it off
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
	# /permissive- enables /Zc:twoPhase which would be good if two phase lookup wasn't still broken in VS 2017
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:twoPhase-")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:inline")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:throwingNew")
	if(NOT MSVC_VERSION LESS 1914)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
	endif()
	
	# Avoid warning during link
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
	
	# Always build with debug information
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG")
	
	# Enable compiler optimization in release
	set(CMAKE_CXX_FLAGS_RELEASE
	    "${CMAKE_CXX_FLAGS_RELEASE} /Ox /Oi /Ot /GS- /fp:fast")
	
else(MSVC)
	
	set(linker_used)
	if(NOT linker_used AND USE_LD STREQUAL "mold")
		# Does not really support LTO yet
		add_ldflag("-fuse-ld=mold")
		if(FLAG_FOUND)
			set(linker_used "mold")
		elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
			message(FATAL_ERROR "Requested linker is not available")
		endif()
	endif()
	if(NOT linker_used AND (USE_LD STREQUAL "lld" OR
	                        (USE_LD STREQUAL "best" AND (NOT USE_LTO OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))))
		# Only supports LTO with LLVM-based compilers and old versions are unstable
		if(USE_LD STREQUAL "best")
			execute_process(COMMAND ${CMAKE_CXX_COMPILER} "-fuse-ld=lld" "-Wl,-version"
			                OUTPUT_VARIABLE _LLD_Version ERROR_QUIET)
		endif()
		if(USE_LD STREQUAL "best" AND _LLD_Version MATCHES "LLD [0-8]\\.[0-9\\.]*")
			message(STATUS "Not using ancient ${CMAKE_MATCH_0}")
		elseif(USE_LD STREQUAL "best" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
		       CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
			message(STATUS "Not using ancient Clang ${CMAKE_CXX_COMPILER_VERSION}")
		else()
			add_ldflag("-fuse-ld=lld")
			if(FLAG_FOUND)
				set(linker_used "lld")
			elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
				message(FATAL_ERROR "Requested linker is not available")
			endif()
		endif()
	endif()
	if(NOT linker_used AND (USE_LD STREQUAL "gold" OR USE_LD STREQUAL "best"))
		add_ldflag("-fuse-ld=gold")
		if(FLAG_FOUND)
			set(linker_used "gold")
		elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
			message(FATAL_ERROR "Requested linker is not available")
		endif()
	endif()
	if(NOT linker_used AND (USE_LD STREQUAL "bfd"))
		add_ldflag("-fuse-ld=bfd")
		if(FLAG_FOUND)
			set(linker_used "bfd")
		elseif(STRICT_USE AND NOT USE_LD STREQUAL "best")
			message(FATAL_ERROR "Requested linker is not available")
		endif()
	endif()
	
	if(USE_LTO)
		add_cxxflag("-flto=auto")
		if(NOT FLAG_FOUND)
			add_cxxflag("-flto")
		endif()
		# TODO set CMAKE_INTERPROCEDURAL_OPTIMIZATION instead
		add_ldflag("-fuse-linker-plugin")
	endif()
	
	if(FASTLINK)
		
		# Optimize for link speed in developer builds
		if(linker_used STREQUAL "mold" OR linker_used STREQUAL "lld")
			# mold and lld are fast enough without -gsplit-dwarf that we don't need to deal with its issues
		else()
			add_cxxflag("-gsplit-dwarf")
			add_cxxflag("-gdwarf-4") # -gsplit-dwarf is broken with DWARF 5
		endif()
		
	elseif(SET_OPTIMIZATION_FLAGS)
		
		# Merge symbols and discard unused symbols
		add_ldflag("-Wl,--gc-sections")
		add_ldflag("-Wl,--icf=all")
		add_cxxflag("-fmerge-all-constants")
		
	endif()
	
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
		add_cxxflag("-Winit-self")
		add_cxxflag("-Wkeyword-macro")
		add_cxxflag("-Wlogical-op")
		add_cxxflag("-Wmissing-declarations")
		add_cxxflag("-Wnoexcept")
		add_cxxflag("-Woverflow")
		add_cxxflag("-Woverloaded-virtual")
		add_cxxflag("-Wpessimizing-move")
		add_cxxflag("-Wpointer-arith")
		add_cxxflag("-Wredundant-decls")
		add_cxxflag("-Wreserved-id-macro")
		add_cxxflag("-Wshift-overflow")
		add_cxxflag("-Wstrict-null-sentinel")
		add_cxxflag("-Wstringop-overflow=2")
		add_cxxflag("-Wundef")
		add_cxxflag("-Wunused-const-variable=1")
		add_cxxflag("-Wunused-macros")
		add_cxxflag("-Wvla")
		
		add_cxxflag("-Wliteral-conversion") # part of -Wconversion
		add_cxxflag("-Wbool-conversion") # part of -Wconversion
		add_cxxflag("-Wfloat-conversion") # part of -Wconversion
		add_cxxflag("-Wstring-conversion") # part of -Wconversion
		add_cxxflag("-Wenum-conversion") # part of -Wconversion
		
		if(NOT UNITY_BUILD)
			add_cxxflag("-Wheader-hygiene")
		endif()
		
		if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
			# GCC <= 8 wants override even for member functions declared final
		else()
			add_cxxflag("-Wsuggest-override")
		endif()
		
		if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
			add_cxxflag("-Wsign-promo")
			add_cxxflag("-Wold-style-cast")
			if(FLAG_FOUND AND NOT SET_NOISY_WARNING_FLAGS)
				list(APPEND conservative_warnings "-Wno-old-style-cast")
			endif()
		endif()
		
		if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5
		   AND NOT SET_NOISY_WARNING_FLAGS)
			# In older GCC versions this warning is too strict
		elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5
		       AND NOT SET_NOISY_WARNING_FLAGS)
			# In older Clang verstions this warns on BOOST_SCOPE_EXIT
		elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND NOT SET_NOISY_WARNING_FLAGS)
			# For icc this warning is too strict
		else()
			add_cxxflag("-Wshadow")
		endif()
		
		if(SET_NOISY_WARNING_FLAGS OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
			# TODO for some reason this warns in /usr/include/boost/type_traits/alignment_of.hpp for -m32 builds
			add_cxxflag("-Wduplicated-branches")
		endif()
		
		if(DEBUG_EXTRA)
			# Address sanitizer cannot be linked into shared libraries so there will be undefined symbols
			# This is by design as asan needs to be preloaded for shared libraries
		else()
			add_ldflag("-Wl,--no-undefined")
		endif()
		
		if(SET_NOISY_WARNING_FLAGS)
			
			# These are too noisy to enable right now but we still want to track new warnings.
			# TODO enable by default as soon as most are silenced
			add_cxxflag("-Wconditionally-supported") # warns on casting from pointer to function pointer
			add_cxxflag("-Wconversion") # very noisy
			# add_cxxflag("-Wsign-conversion") # part of -Wconversion
			# add_cxxflag("-Wshorten-64-to-32") # part of -Wconversion
			add_cxxflag("-Wstrict-aliasing=1") # has false positives
			add_cxxflag("-Wuseless-cast") # has false positives
			add_cxxflag("-Wsign-promo")
			add_cxxflag("-Wsuggest-final-types")
			add_cxxflag("-Wsuggest-final-methods")
			# add_cxxflag("-Wnull-dereference") not that useful without deduction path
			
			# Possible optimization opportunities
			add_cxxflag("-Wdisabled-optimization")
			add_cxxflag("-Wpadded")
			add_cxxflag("-Wunsafe-loop-optimizations")
			
			# Levels 3 and above may result in warnings for safe code
			add_cxxflag("-Wstringop-overflow=4")
			
			# There are some cases we want 0 instead of nullptr (e.g. Flags)
			add_cxxflag("-Wzero-as-null-pointer-constant")
			
			if(NOT DEBUG_EXTRA OR NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
				add_ldflag("-Wl,--detect-odr-violations")
			endif()
			
			if(UNITY_BUILD)
				add_cxxflag("-Wunused-const-variable=2")
			endif()
			
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
				# 'offsetof applied to non-POD (Plain Old Data) types is nonstandard'
				# This triggers Qt moc-generated headers
				add_cxxflag("-wd1875")
				# 'printf/scanf format not a string literal and no format arguments'
				# balks when passing NULL as the format string
				add_cxxflag("-wd2279")
				if(DEBUG)
					# 'missing return statement at end of non-void function' even with arx_unreachable()
					# ICC does no handle the noreturn attribute with the comma operator
					add_cxxflag("-wd1011")
				endif()
			endif()
			
			# -Wuninitialized causes too many false positives in older gcc versions
			if(CMAKE_COMPILER_IS_GNUCXX)
				# GCC is 'clever' and silently accepts -Wno-*  - check for the non-negated variant
				check_compiler_flag(FLAG_FOUND "-Wmaybe-uninitialized")
				if(FLAG_FOUND)
					add_cxxflag("-Wno-maybe-uninitialized")
				else()
					add_cxxflag("-Wno-uninitialized")
				endif()
			endif()
			
			# Xcode does not support -isystem yet
			if(MACOS)
				add_cxxflag("-Wno-undef")
			endif()
			
		endif()
		
		if(IS_LIBCXX)
			add_definitions(-D_LIBCPP_ENABLE_NODISCARD)
			add_definitions(-D_LIBCPP_ENABLE_THREAD_SAFETY_ANNOTATIONS)
		endif()
		
	endif(SET_WARNING_FLAGS)
	
	if(WERROR)
		add_cxxflag("-Werror")
	endif()
	
	if(DEBUG_EXTRA)
		add_cxxflag("-ftrapv") # to add checks for (undefined) signed integer overflow
		add_cxxflag("-fbounds-checking")
		add_cxxflag("-fcatch-undefined-behavior")
		add_cxxflag("-fstack-protector-all")
		add_cxxflag("-fsanitize=address")
		add_cxxflag("-fsanitize-address-use-after-scope")
		add_cxxflag("-fsanitize-address-use-after-return=always")
		# add_cxxflag("-fsanitize=thread") does not work together with -fsanitize=address
		add_cxxflag("-fsanitize=leak")
		add_cxxflag("-fsanitize=undefined")
		if(ARX_HAVE_LIBCPP_ENABLE_ASSERTIONS)
			# libc++ 15+ - Full debug mode is now a compile-time option and all -D_LIBCPP_DEBUG=1 does is
			# generate an #error if the library was not built in debug mode :|
			add_definitions(-D_LIBCPP_ENABLE_ASSERTIONS=1)
		elseif(IS_LIBCXX)
			# older libc++
			add_definitions(-D_LIBCPP_DEBUG=1)
		else()
			# libstdc++
			add_definitions(-D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_SANITIZE_VECTOR)
			set(disable_libstdcxx_debug "-U_GLIBCXX_DEBUG -U_GLIBCXX_DEBUG_PEDANTIC")
		endif()
	elseif(DEBUG)
		if(ARX_HAVE_LIBCPP_ENABLE_ASSERTIONS)
			# libc++ 15+
			add_definitions(-D_LIBCPP_ENABLE_ASSERTIONS=1)
		elseif(IS_LIBCXX)
			# older libc++ - 0 means light checks only, it does not mean no checks
			add_definitions(-D_LIBCPP_DEBUG=0)
		else()
			# libstdc++
			add_definitions(-D_GLIBCXX_ASSERTIONS=1)
		endif()
	elseif(SET_OPTIMIZATION_FLAGS)
		if(IS_LIBCXX)
			add_cxxflag("-Wp,-D_LIBCPP_ENABLE_ASSERTIONS=0")
		else()
			add_cxxflag("-Wp,-U_GLIBCXX_ASSERTIONS")
		endif()
	endif()
	
	if(CMAKE_BUILD_TYPE STREQUAL "")
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	
	if(SET_OPTIMIZATION_FLAGS)
		
		add_cxxflag("-fno-rtti")
		if(FLAG_FOUND)
			list(APPEND enable_rtti "-frtti")
		endif()
		
		if(MACOS)
			# TODO For some reason this check succeeds on macOS, but then
			# flag causes the actual build to fail :(
		else()
			# Link as few libraries as possible
			# This is much easier than trying to decide which libraries are needed for each
			# system
			add_ldflag("-Wl,--as-needed")
		endif()
		
		foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE)
			
			if(DEBUG)
				string(REGEX REPLACE "(^| )-DNDEBUG( |$)" "\\1" ${flag_var} "${${flag_var}}")
				set(${flag_var} "${${flag_var}}")
			else()
				set(${flag_var} "${${flag_var}} -DNDEBUG")
			endif()
			
		endforeach(flag_var)
		
		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			
			# set debug symbol level to -g3
			check_compiler_flag(RESULT "-g3")
			if(NOT RESULT STREQUAL "")
				string(REGEX REPLACE "(^| )-g(|[0-9]|gdb)( |$)" "\\1" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
				set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
			endif()
			
			# disable optimizations
			check_compiler_flag(RESULT "-Og")
			if(NOT RESULT)
				check_compiler_flag(RESULT "-O0")
			endif()
			string(REGEX REPLACE "(^| )-O[0-9g]( |$)" "\\1" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
			set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RESULT}")
			
		elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
			
			if((NOT CMAKE_CXX_FLAGS MATCHES "-g(|[0-9]|gdb)")
			   AND (NOT CMAKE_CXX_FLAGS_RELEASE MATCHES "-g(|[0-9]|gdb)"))
				add_cxxflag("-g2")
			endif()
			
			add_cxxflag("-ffast-math")
			if(FLAG_FOUND)
				# Prevent linking crtfastmath.o into shared libraries loaded by other applications
				# that might not expect denormals to be disabled.
				set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-fast-math")
			endif()
			
		endif()
		
	endif(SET_OPTIMIZATION_FLAGS)
	
endif(MSVC)

set(BUILD_TYPES ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
list(REMOVE_DUPLICATES BUILD_TYPES)
