/*
 * Copyright 2014-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_CINEMATICBORDER_H
#define ARX_GUI_CINEMATICBORDER_H

#include "core/TimeTypes.h"

class CinematicBorder {
public:
	CinematicBorder();
	
	bool isActive();
	GameDuration elapsedTime();
	
	void reset();
	
	void update();
	void set(bool status, bool smooth);
	
	void render();
	
	float CINEMA_DECAL;
	
private:
	bool m_active;
	long m_direction;
	GameInstant m_startTime;
};

extern CinematicBorder cinematicBorder;

#endif // ARX_GUI_CINEMATICBORDER_H
