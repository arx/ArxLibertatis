/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/CinematicBorder.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Player.h"
#include "gui/Interface.h"

float				CINEMA_DECAL=0.f;

CinematicBorder cinematicBorder = CinematicBorder();

CinematicBorder::CinematicBorder()
	: CINEMASCOPE(false)
	, CINEMA_INC(0)
	, g_TimeStartCinemascope(0)
{}

bool CinematicBorder::isActive()
{
	return CINEMASCOPE;
}

float CinematicBorder::elapsedTime() {
	float dwCurrTime = arxtime.get_updated();
	return (dwCurrTime - g_TimeStartCinemascope);
}

void CinematicBorder::reset() {
	CINEMA_INC=0;
}
void CinematicBorder::reset2() {
	CINEMASCOPE = false;
}

void CinematicBorder::set(bool status, bool smooth)
{
	if(status) {
		CINEMASCOPE = true;//++;
		g_TimeStartCinemascope = arxtime.get_updated();
	} else {
		CINEMASCOPE = false;//--;
		g_TimeStartCinemascope = 0;
	}
	
	if(CINEMASCOPE) {
		if(smooth)
			CINEMA_INC=1;
		else
			CINEMA_DECAL=100;
	} else {
		if(smooth)
			CINEMA_INC=-1;
		else
			CINEMA_DECAL=0;
	}

	if(player.Interface & INTER_INVENTORY)
		player.Interface &=~ INTER_INVENTORY;

	if(player.Interface & INTER_INVENTORYALL)
		player.Interface &=~ INTER_INVENTORYALL;
}


void CinematicBorder::update() {
	
	if(CINEMA_INC == 1) {
		CINEMA_DECAL += (float)Original_framedelay*( 1.0f / 10 );

		if(CINEMA_DECAL > 100.f) {
			CINEMA_DECAL = 100.f;
			CINEMA_INC = 0;
		}
	} else if(CINEMA_INC == -1) {
		CINEMA_DECAL -= (float)Original_framedelay*( 1.0f / 10 );

		if(CINEMA_DECAL < 0.f) {
			CINEMA_DECAL = 0.f;
			CINEMA_INC = 0;
		}
	}
}
