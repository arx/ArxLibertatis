/*
 * Copyright 2012-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CORE_URLCONSTANTS_H
#define ARX_CORE_URLCONSTANTS_H

namespace url {

typedef const char * const str_t;

//! Documentation on how to get the game data
static str_t help_get_data = "http://arx.vg/data";

//! Documentation on how and where to install the game data
static str_t help_install_data = "http://arx.vg/paths";

//! Where users can report bugs
static str_t bug_report = "http://arx.vg/bug";

} // namespace url

#endif // ARX_CORE_URLCONSTANTS_H
