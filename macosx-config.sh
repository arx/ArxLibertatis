#!/bin/sh

# Run this script from a "build" subdirectory of your ArxLibertatis clone

exec cmake .. '-DCMAKE_C_FLAGS_RELEASE=-DNDEBUG' '-DCMAKE_CXX_FLAGS_RELEASE=-DNDEBUG' '-DCMAKE_INSTALL_PREFIX=/usr/local/Cellar/arx-libertatis/1.1.2' '-DCMAKE_BUILD_TYPE=Release' '-DCMAKE_FIND_FRAMEWORK=LAST' '-DCMAKE_VERBOSE_MAKEFILE=ON' -Wno-dev
