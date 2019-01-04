/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_TIME_H
#define ARX_PLATFORM_TIME_H

#include "platform/Platform.h"
#include "core/TimeTypes.h"

namespace platform {

/*!
 * \brief Initalize the time subsystem
 *
 * Must be called before any other time functions.
 */
void initializeTime();

/*!
 * \brief Get the current time
 *
 * \return The current time relative to some unspecified starting point.
 */
PlatformInstant getTime();

} // namespace platform

#endif // ARX_PLATFORM_TIME_H
