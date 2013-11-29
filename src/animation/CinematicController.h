/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_ANIMATION_CINEMATICCONTROLLER_H
#define ARX_ANIMATION_CINEMATICCONTROLLER_H

#include <string>

enum CinematicState {
	Cinematic_Stopped,
	Cinematic_StartRequested,
	Cinematic_Started
};
extern CinematicState PLAY_LOADED_CINEMATIC;

void cinematicPrepare(std::string name, bool preload);

void DANAE_KillCinematic();

void LaunchWaitingCine();
void DANAE_Manage_Cinematic();

#endif // ARX_ANIMATION_CINEMATICCONTROLLER_H
