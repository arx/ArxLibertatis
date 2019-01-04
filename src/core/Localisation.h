/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_CORE_LOCALISATION_H
#define ARX_CORE_LOCALISATION_H

#include <string>

/*!
 * Initializes the localisation hashmap based on the current chosen locale
 */
bool initLocalisation();

/*!
 * Returns the localized string for the given key name
 * \param name The string to be looked up
 * \return The localized string based on the currently loaded locale file
 */
std::string getLocalised(const std::string & name);

/*!
 * Returns the localized string for the given key name
 * \param name The string to be looked up
 * \return The localized string based on the currently loaded locale file
 */
std::string getLocalised(const std::string & name, const std::string & default_value);

long getLocalisedKeyCount(const std::string & sectionname);

#endif // ARX_CORE_LOCALISATION_H
