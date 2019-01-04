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

#include "gui/CinematicBorder.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Player.h"
#include "graphics/Renderer.h"
#include "gui/Interface.h"

CinematicBorder cinematicBorder = CinematicBorder();

CinematicBorder::CinematicBorder()
	: CINEMA_DECAL(0)
	, m_active(false)
	, m_direction(0)
	, m_startTime(0)
{}

bool CinematicBorder::isActive()
{
	return m_active;
}

GameDuration CinematicBorder::elapsedTime() {
	return g_gameTime.now() - m_startTime;
}

void CinematicBorder::reset() {
	m_active = false;
	m_direction = 0;
}

void CinematicBorder::set(bool status, bool smooth) {
	
	if(status) {
		m_active = true;
		m_startTime = g_gameTime.now();
	} else {
		m_active = false;
		m_startTime = 0;
	}
	
	if(m_active) {
		if(smooth) {
			m_direction = 1;
		} else {
			CINEMA_DECAL = 100;
		}
	} else {
		if(smooth) {
			m_direction = -1;
		} else {
			CINEMA_DECAL = 0;
		}
	}
	
	if(player.Interface & INTER_INVENTORY) {
		player.Interface &= ~INTER_INVENTORY;
	}
	
	if(player.Interface & INTER_INVENTORYALL) {
		player.Interface &= ~INTER_INVENTORYALL;
	}
	
}

void CinematicBorder::update() {
	
	if(m_direction == 1) {
		CINEMA_DECAL += g_platformTime.lastFrameDuration() / PlatformDurationMs(10);

		if(CINEMA_DECAL > 100.f) {
			CINEMA_DECAL = 100.f;
			m_direction = 0;
		}
	} else if(m_direction == -1) {
		CINEMA_DECAL -= g_platformTime.lastFrameDuration() / PlatformDurationMs(10);

		if(CINEMA_DECAL < 0.f) {
			CINEMA_DECAL = 0.f;
			m_direction = 0;
		}
	}
}

void CinematicBorder::render() {
	
	if(CINEMA_DECAL != 0.f) {
		Rect rectz[2];
		rectz[0].left = rectz[1].left = 0;
		rectz[0].right = rectz[1].right = g_size.width();
		rectz[0].top = 0;
		s32 lMulResult = checked_range_cast<s32>(CINEMA_DECAL * g_sizeRatio.y);
		rectz[0].bottom = lMulResult;
		rectz[1].top = g_size.height() - lMulResult;
		rectz[1].bottom = g_size.height();
		GRenderer->Clear(Renderer::ColorBuffer | Renderer::DepthBuffer, Color::none, 0.0f, 2, rectz);
		GRenderer->SetScissor(Rect(0, lMulResult, g_size.width(), g_size.height() - lMulResult));
	}
}
