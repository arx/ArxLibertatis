
# Extended / compatibility wrapper for GNUInstallDirs.cmake
#
# Defines the following GNUInstallDirs vars for CMake < 2.8.5:
#  CMAKE_INSTALL_DATAROOTDIR  CMAKE_INSTALL_FULL_DATAROOTDIR
#  CMAKE_INSTALL_BINDIR       CMAKE_INSTALL_FULL_BINDIR
#  CMAKE_INSTALL_LIBEXECDIR   CMAKE_INSTALL_FULL_LIBEXECDIR
#  CMAKE_INSTALL_MANDIR       CMAKE_INSTALL_FULL_MANDIR
#
# Defines the following additional configuration variables:
#  ICONDIR      install location for icon files
#  APPDIR       install location for applications launcher
#  GAMESBINDIR  install location for game binaries
#  SCRIPTDIR    install location for helper scripts
#
# Also defines default values for these to build a Mac .app bundle hierarchy
# if CREATE_MAC_APP_BUNDLE is set to true (enabled by default on Mac OS X).
#


# Install destinations
set(default_CREATE_MAC_APP_BUNDLE OFF)
if(MACOSX)
	set(default_CREATE_MAC_APP_BUNDLE ON)
endif()
if(CREATE_MAC_APP_BUNDLE)
	set(CMAKE_INSTALL_DATAROOTDIR "Contents/Resources" CACHE
			STRING "read-only architecture-independent data root (share) (relative to prefix)")
	set(CMAKE_INSTALL_BINDIR "Contents/MacOS" CACHE
			STRING "user executables (bin) (relative to prefix)")
	set(CMAKE_INSTALL_LIBEXECDIR "${CMAKE_INSTALL_BINDIR}" CACHE
			STRING "program executables (libexec) (relative to prefix)")
elseif(${CMAKE_VERSION} VERSION_LESS 2.8.5)
	set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE
			STRING "read-only architecture-independent data root (share) (relative to prefix)")
	set(CMAKE_INSTALL_BINDIR "bin" CACHE
			STRING "user executables (bin) (relative to prefix)")
	set(CMAKE_INSTALL_LIBEXECDIR "libexec" CACHE
			STRING "program executables (libexec) (relative to prefix)")
else()
	include(GNUInstallDirs)
endif()
if(CREATE_MAC_APP_BUNDLE OR ${CMAKE_VERSION} VERSION_LESS 2.8.5)
	set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_DATAROOTDIR}/man" CACHE
			STRING "man documentation (DATAROOTDIR/man) (relative to prefix)")
	foreach(dir BINDIR LIBEXECDIR DATAROOTDIR MANDIR)
		if(NOT IS_ABSOLUTE ${CMAKE_INSTALL_${dir}})
			set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_${dir}}")
		else()
			set(CMAKE_INSTALL_FULL_${dir} "${CMAKE_INSTALL_${dir}}")
		endif()
	endforeach()
	mark_as_advanced(
		CMAKE_INSTALL_DATAROOTDIR
		CMAKE_INSTALL_BINDIR
		CMAKE_INSTALL_LIBEXECDIR
		CMAKE_INSTALL_MANDIR
	)
endif()
if(CREATE_MAC_APP_BUNDLE)
	set(ICONDIR "${CMAKE_INSTALL_DATAROOTDIR}" CACHE
	    STRING "Install location for icons (relative to prefix)")
	set(APPDIR "Contents" CACHE
	    STRING "Install location for .desktop files (relative to prefix)")
else()
	set(ICONDIR "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor" CACHE
	    STRING "Install location for icons (relative to prefix)")
	set(APPDIR "${CMAKE_INSTALL_DATAROOTDIR}/applications" CACHE
	    STRING "Install location for .desktop files (relative to prefix)")
endif()
set(GAMESBINDIR "${CMAKE_INSTALL_BINDIR}" CACHE
    STRING "Install location for game executables (relative to prefix)")
set(SCRIPTDIR "${CMAKE_INSTALL_BINDIR}" CACHE
    STRING "Where to install the data install script (relative to prefix)")
mark_as_advanced(
	ICONDIR
	APPDIR
	GAMESBINDIR
	SCRIPTDIR
)
