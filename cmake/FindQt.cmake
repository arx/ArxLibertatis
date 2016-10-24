
set(WITH_QT CACHE STRING "The Qt version to use: 4 or 5 or empty (use any)")

unset(HAVE_QT)

set(_Qt_COMPONENTS Core)
foreach(lib IN LISTS Qt_FIND_COMPONENTS)
	if(lib STREQUAL "Widgets")
		list(APPEND _Qt_COMPONENTS Gui)
	endif()
	list(APPEND _Qt_COMPONENTS ${lib})
endforeach()
list(REMOVE_DUPLICATES _Qt_COMPONENTS)

set(_Qt_FIND_ARGS)
if(NOT WITH_QT)
	# If the user has not requested a specific version, don't spam them
	# when either one is not found.
	set(_Qt_FIND_ARGS QUIET)
else()
	if(Qt_FIND_QUIETLY)
		list(APPEND _Qt_FIND_ARGS QUIET)
	endif()
	if(Qt_FIND_REQUIRED)
		list(APPEND _Qt_FIND_ARGS REQUIRED)
	endif()
endif()

# Try to find Qt 5 modules
if(NOT HAVE_QT AND (NOT WITH_QT OR WITH_QT EQUAL 5))
	
	set(HAVE_QT5 1)
	foreach(lib IN LISTS _Qt_COMPONENTS)
		find_package(Qt5${lib} ${_Qt_FIND_ARGS})
		if(NOT Qt5${lib}_FOUND)
			set(HAVE_QT5 0)
			return()
		endif()
		set(Qt${lib}_LIBRARIES ${Qt5${lib}_LIBRARIES})
		set(Qt${lib}_DEFINITIONS ${Qt5${lib}_DEFINITIONS})
		set(Qt${lib}_INCLUDE_DIRS ${Qt5${lib}_INCLUDE_DIRS})
		set(Qt${lib}_EXECUTABLE_COMPILE_FLAGS ${Qt5${lib}_EXECUTABLE_COMPILE_FLAGS})
		set(Qt${lib}_SOURCES)
	endforeach()
	
	if(HAVE_QT5)
		
		set(QtCore_QTMAIN_LIBRARIES ${Qt5Core_QTMAIN_LIBRARIES})
		
		macro(qt_wrap_ui)
			qt5_wrap_ui(${ARGV})
		endmacro()
		macro(qt_wrap_cpp)
			qt5_wrap_cpp(${ARGV})
		endmacro()
		macro(qt_add_resources)
			qt5_add_resources(${ARGV})
		endmacro()
		
		set(Qt_VERSION ${Qt5Core_VERSION})
		set(HAVE_QT 1)
		
		# Handle static Qt 5 windows builds
		# Even though Qt ships it's own CMake configuration files, they do not add
		# all the required dependencies for their imported static library targets
		# - and those they do add are set with the wrong property :/
		if(WIN32)
			foreach(lib IN LISTS _Qt_COMPONENTS)
				
				if(TARGET Qt5::${lib})
					
					get_property(_Qt_lib TARGET Qt5::${lib} PROPERTY IMPORTED_LOCATION_RELEASE)
					if(NOT _Qt_lib)
						get_property(_Qt_lib TARGET Qt5::${lib} PROPERTY IMPORTED_LOCATION)
					endif()
					get_property(_Qt_implib TARGET Qt5::${lib} PROPERTY IMPORTED_IMPLIB_RELEASE)
					if(NOT _Qt_implib)
						get_property(_Qt_implib TARGET Qt5::${lib} PROPERTY IMPORTED_IMPLIB)
					endif()
					
					if(_Qt_implib OR NOT _Qt_lib OR NOT _Qt_lib MATCHES ".(lib|a)$")
						# Does not look like a static library
					else()
						
						set(_Qt_extra_libs)
						
						# Qt sets this for Qt5::Gui on either ANGLE or opengl
						foreach(var IN ITEMS DEPENDENT_LIBRARIES_RELEASE DEPENDENT_LIBRARIES)
							get_property(_Qt_deps TARGET Qt5::${lib} PROPERTY IMPORTED_LINK_${var})
							if(_Qt_deps)
								list(APPEND _Qt_extra_libs ${_Qt_deps})
							endif()
						endforeach()
						
						if(lib STREQUAL "Core")
							get_filename_component(_Qt_libdir ${_Qt_lib} PATH)
							foreach(ext IN ITEMS lib a)
								if(EXISTS "${_Qt_libdir}/qtpcre.lib")
									list(APPEND _Qt_extra_libs "${_Qt_libdir}/qtpcre.lib")
									break()
								endif()
							endforeach()
							list(APPEND _Qt_extra_libs kernel32 user32 shell32 uuid ole32 advapi32 ws2_32)
						elseif(lib STREQUAL "Concurrent")
							list(APPEND _Qt_extra_libs ${QtCore_LIBRARIES})
						elseif(lib STREQUAL "Gui")
							get_filename_component(_Qt_libdir ${_Qt_lib} PATH)
							foreach(ext IN ITEMS lib a)
								if(EXISTS "${_Qt_libdir}/Qt5PlatformSupport.lib")
									list(APPEND _Qt_extra_libs "${_Qt_libdir}/Qt5PlatformSupport.lib")
									break()
								endif()
							endforeach()
							if(TARGET Qt5::QWindowsIntegrationPlugin)
								list(APPEND _Qt_extra_libs Qt5::QWindowsIntegrationPlugin)
							endif()
							# Just statically linking the plugin is not enough
							set(_Qt_usewinplugin "${PROJECT_BINARY_DIR}/QtUseWindowsIntegration.cpp")
							file(WRITE ${_Qt_usewinplugin} "#include <QtPlugin>\n")
							file(APPEND ${_Qt_usewinplugin} "Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)\n")
							list(APPEND QtGui_SOURCES ${_Qt_usewinplugin})
							list(APPEND _Qt_extra_libs gdi32 comdlg32 oleaut32 imm32 winmm ws2_32 ole32
							                           user32 advapi32 ${Qt5Core_LIBRARIES})
						elseif(lib STREQUAL "Widgets")
							list(APPEND _Qt_extra_libs gdi32 comdlg32 oleaut32 imm32 winmm ws2_32 ole32
							                           user32 advapi32 ${Qt5Core_LIBRARIES}
							                           ${Qt5Gui_LIBRARIES})
						elseif(lib STREQUAL "Network")
							list(APPEND _Qt_extra_libs ws2_32 ${Qt5Core_LIBRARIES})
						else()
							message(WARNING "Don't know how to statically link Qt${lib}: ${_Qt_lib}")
						endif()
						
						if(_Qt_extra_libs)
							set_property(TARGET Qt5::${lib} APPEND PROPERTY INTERFACE_LINK_LIBRARIES
							                    ${_Qt_extra_libs})
						endif()
						
					endif()
					
				endif()
				
			endforeach()
		endif()
		
	endif()
	
endif()

function(get_qt4_component VAR QTCOMPONENT)
	if(QTCOMPONENT STREQUAL "Widgets")
		set(${VAR} "Gui" PARENT_SCOPE)
	elseif(QTCOMPONENT STREQUAL "Concurrent")
		set(${VAR} "Core" PARENT_SCOPE)
	else()
		set(${VAR} ${QTCOMPONENT} PARENT_SCOPE)
	endif()
endfunction()

# Otherwise, look for Qt 4
if(NOT HAVE_QT AND (NOT WITH_QT OR WITH_QT EQUAL 4))
	
	set(QT_USE_IMPORTED_TARGETS 1)
	set(_Qt4_COMPONENTS)
	foreach(lib IN LISTS _Qt_COMPONENTS)
		get_qt4_component(_Qt_LIB ${lib})
		list(APPEND _Qt4_COMPONENTS Qt${_Qt_LIB})
	endforeach()
	list(REMOVE_DUPLICATES _Qt4_COMPONENTS)
	find_package(Qt4 COMPONENTS ${_Qt4_COMPONENTS} ${_Qt_FIND_ARGS})
	
	# QT_VERSION_MAJOR check required because of FindQt4.cmake bug
	if(QT4_FOUND AND QT_VERSION_MAJOR EQUAL 4)
		set(HAVE_QT4 1)
		
		foreach(lib IN LISTS _Qt_COMPONENTS)
			get_qt4_component(_Qt_LIB ${lib})
			string(TOUPPER ${_Qt_LIB} _Qt_LIB)
			set(Qt${lib}_LIBRARIES ${QT_QT${_Qt_LIB}_LIBRARY_RELEASE})
			set(Qt${lib}_DEFINITIONS ${QT_DEFINITIONS})
			set(Qt${lib}_INCLUDE_DIRS ${QT_INCLUDE_DIR} ${QT_QT${_Qt_LIB}_INCLUDE_DIR})
			set(Qt${lib}_EXECUTABLE_COMPILE_FLAGS)
			set(Qt${lib}_SOURCES)
		endforeach()
		set(QtCore_QTMAIN_LIBRARIES ${QT_QTMAIN_LIBRARY_RELEASE})
		
		macro(qt_wrap_ui)
			qt4_wrap_ui(${ARGV})
		endmacro()
		macro(qt_wrap_cpp)
			qt4_wrap_cpp(${ARGV})
		endmacro()
		macro(qt_add_resources results)
			set(_Qt_RESOURCES)
			qt4_add_resources(_Qt_RESOURCES ${ARGN})
			set_property(
				SOURCE ${_Qt_RESOURCES}
				APPEND PROPERTY COMPILE_FLAGS " -Wno-missing-declarations"
			)
			list(APPEND ${results} ${_Qt_RESOURCES})
		endmacro()
		
		set(Qt_VERSION "4.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
		set(HAVE_QT 1)
		
	endif()
	
endif()

# Always link agains release versions of the Qt libraries
# This is important on windows where we would otherwise end up
# with inconsistencies in the stdlib configuration.
if(HAVE_QT AND MSVC AND NOT DEBUG_EXTRA)
	set(_Qt_LIBRARIES)
	foreach(lib IN LISTS _Qt_COMPONENTS)
		list(APPEND _Qt_LIBRARIES ${Qt${lib}_LIBRARIES})
	endforeach()
	list(APPEND _Qt_LIBRARIES ${Qt${lib}_LIBRARIES})
	foreach(lib IN LISTS arxcrashreporter_qt_LIBRARIES)
		if(TARGET ${lib})
			set_property(TARGET ${lib} PROPERTY MAP_IMPORTED_CONFIG_DEBUG "Release")
		endif()
	endforeach()
endif()

# Get a path to display for the config log
unset(Qt_LIBRARY)
foreach(lib IN LISTS QtCore_LIBRARIES)
	if(TARGET ${lib})
		foreach(var IN ITEMS IMPLIB_RELEASE IMPLIB LOCATION_RELEASE LOCATION)
			get_property(Qt_LIBRARY TARGET ${lib} PROPERTY IMPORTED_${var})
			if(Qt_LIBRARY)
				break()
			endif()
		endforeach()
		if(Qt_LIBRARY)
			break()
		endif()
	endif()
endforeach()
if(NOT Qt_LIBRARY)
	set(Qt_LIBRARY ${QtCore_LIBRARIES})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qt REQUIRED_VARS Qt_LIBRARY HAVE_QT
                                  VERSION_VAR Qt_VERSION)
