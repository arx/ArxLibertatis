/*
 * Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_NOTIFICATION_H
#define ARX_GUI_NOTIFICATION_H

#include <string>

void ARX_SPEECH_Init();
void ARX_SPEECH_ClearAll();

/*!
 * Add a raw text message to the "system" log (top of the screen).
 * This message will be displayed as-is.
 */
void ARX_SPEECH_Add(const std::string & text);

void ARX_SPEECH_Check();

#endif // ARX_GUI_NOTIFICATION_H
