/*
 * Copyright 2015-2017 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

/*!
 * \file
 *
 * Compatibility wrapper to deal with annoyances in Microsoft® Windows™.
 * Mostly deals with converting between UTF-8 and UTF-16 input/output.
 * More precisely:
 *  - Converts wide char command-line arguments to UTF-8 and calls utf8_main().
 */
#ifndef ARX_PLATFORM_WINDOWSMAIN_H
#define ARX_PLATFORM_WINDOWSMAIN_H

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32

//! Program entry point that will always receive UTF-8 encoded arguments
int utf8_main(int argc, char * argv[]);

#else

//! The normal main function is already good enough
#define utf8_main main

#endif

#endif // ARX_PLATFORM_WINDOWSMAIN_H
