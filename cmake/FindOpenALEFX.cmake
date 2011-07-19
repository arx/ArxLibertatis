
# Hints and paths from FindOpenAL.cmake
find_path(OPENAL_EFX_INCLUDE_DIR efx.h
  HINTS
  $ENV{OPENALDIR}
  PATH_SUFFIXES include/AL include/OpenAL include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  [HKEY_LOCAL_MACHINE\\SOFTWARE\\Creative\ Labs\\OpenAL\ 1.1\ Software\ Development\ Kit\\1.00.0000;InstallDir]
)

# handle the QUIETLY and REQUIRED arguments and set OPENAL_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenALEFX DEFAULT_MSG OPENAL_EFX_INCLUDE_DIR)

mark_as_advanced(OPENAL_EFX_INCLUDE_DIR)
