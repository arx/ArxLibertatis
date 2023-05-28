/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/effect/Quake.h"

#include "core/Config.h"
#include "core/GameTime.h"
#include "game/Camera.h"
#include "math/RandomVector.h"
#include "scene/GameSound.h"

#define QUAKE_ADJUST 0.4f //adjustment to make quakes look like the original in strength

struct QUAKE_FX_STRUCT {
	float intensity;
	float frequency;
	GameInstant start;
	GameDuration duration;
	bool active;
};

QUAKE_FX_STRUCT QuakeFx = {0.f, 0.f, GameInstant(), GameDuration(), false};

void AddQuakeFX(float intensity, GameDuration duration, float period, bool sound) {

	if(QuakeFx.active) {
		QuakeFx.intensity += intensity;

		QuakeFx.duration += duration;
		QuakeFx.frequency += period;
		QuakeFx.frequency *= .5f;
	} else {
		QuakeFx.intensity = intensity;

		QuakeFx.start = g_gameTime.now();

		QuakeFx.duration = duration;
		QuakeFx.frequency = period;
		QuakeFx.active = true;
	}

	if(!sound) {
		if(QuakeFx.duration > 1500ms)
			QuakeFx.duration = 1500ms;

		if(QuakeFx.intensity > 220)
			QuakeFx.intensity = 220;
	}
}

void RemoveQuakeFX() {
	QuakeFx.intensity = 0.f;
	QuakeFx.active = false;
}

void ManageQuakeFX(Camera * cam) {
	if(QuakeFx.active) {
		GameDuration elapsed = g_gameTime.now() - QuakeFx.start;

		if(elapsed >= QuakeFx.duration) {
			RemoveQuakeFX();
			return;
		}
		
		if(!config.video.screenShake) {
			return;
		}
		
		float itmod = 1.f - (elapsed / QuakeFx.duration);
		
		float truepower = QuakeFx.intensity * itmod * 0.01f * QUAKE_ADJUST;
		float halfpower = truepower * .5f;
		
		cam->m_pos += arx::randomVec(-halfpower, halfpower);
		cam->angle.setPitch(cam->angle.getPitch() + Random::getf(-1.f, 1.f) * truepower - halfpower);
		cam->angle.setYaw(cam->angle.getYaw() + Random::getf(-1.f, 1.f) * truepower - halfpower);
		cam->angle.setRoll(cam->angle.getRoll() + Random::getf(-1.f, 1.f) * truepower - halfpower);
	}
}
