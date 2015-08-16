
# FindGLM - attempts to locate the glm matrix/vector library.
#
# This module defines the following variables (on success):
#   GLM_INCLUDE_DIRS  - where to find glm/glm.hpp
#   GLM_FOUND         - if the library was successfully located
#
# It is trying a few standard installation locations, but can be customized
# with the following variables:
#   GLM_ROOT_DIR      - root directory of a glm installation
#                       Headers are expected to be found in either:
#                       <GLM_ROOT_DIR>/glm/glm.hpp           OR
#                       <GLM_ROOT_DIR>/include/glm/glm.hpp
#                       This variable can either be a cmake or environment
#                       variable. Note however that changing the value
#                       of the environment varible will NOT result in
#                       re-running the header search and therefore NOT
#                       adjust the variables set by this module.

#=============================================================================
# Copyright 2012 Carsten Neumann
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# default search dirs
set(_glm_HEADER_SEARCH_DIRS
	"/usr/include"
	"/usr/local/include"
)

# check environment variable
set(_glm_ENV_ROOT_DIR "$ENV{GLM_ROOT_DIR}")

if(NOT GLM_ROOT_DIR AND _glm_ENV_ROOT_DIR)
	set(GLM_ROOT_DIR "${_glm_ENV_ROOT_DIR}")
endif(NOT GLM_ROOT_DIR AND _glm_ENV_ROOT_DIR)

# put user specified location at beginning of search
if(GLM_ROOT_DIR)
	set(_glm_HEADER_SEARCH_DIRS
		"${GLM_ROOT_DIR}"
		"${GLM_ROOT_DIR}/include"
		${_glm_HEADER_SEARCH_DIRS}
	)
endif(GLM_ROOT_DIR)

# locate header
find_path(GLM_INCLUDE_DIR "glm/glm.hpp" PATHS ${_glm_HEADER_SEARCH_DIRS})

set(_glm_VERSION_HEADER "${GLM_INCLUDE_DIR}/glm/detail/setup.hpp")
if(GLM_INCLUDE_DIR AND NOT EXISTS "${_glm_VERSION_HEADER}")
	set(_glm_VERSION_HEADER "${GLM_INCLUDE_DIR}/glm/core/setup.hpp")
endif()
if(GLM_INCLUDE_DIR AND EXISTS "${_glm_VERSION_HEADER}")
	file(STRINGS "${_glm_VERSION_HEADER}" _glm_VERSION_DEFINES REGEX "#define.*")
	foreach(component IN ITEMS "MAJOR" "MINOR" "PATCH" "REVISION")
		set(_glm_VERSION_PATTERN ".*#define GLM_VERSION_${component}[ \t]+([0-9]+).*")
		if("${_glm_VERSION_DEFINES}" MATCHES "${_glm_VERSION_PATTERN}")
			string(REGEX REPLACE "${_glm_VERSION_PATTERN}" "\\1"
			       _glm_${component} "${_glm_VERSION_DEFINES}")
		endif()
	endforeach()
	if(DEFINED _glm_MAJOR AND DEFINED _glm_MINOR AND DEFINED _glm_PATCH
	   AND DEFINED _glm_REVISION)
		set(GLM_VERSION_STRING "${_glm_MAJOR}.${_glm_MINOR}.${_glm_PATCH}.${_glm_REVISION}")
	endif()
	unset(_glm_VERSION_DEFINES)
endif()

set(GLM_INCLUDE_SUBDIR "${GLM_INCLUDE_DIR}/glm")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLM REQUIRED_VARS GLM_INCLUDE_SUBDIR GLM_INCLUDE_DIR
                                  VERSION_VAR GLM_VERSION_STRING)

if(GLM_FOUND)
	set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}")
endif(GLM_FOUND)
