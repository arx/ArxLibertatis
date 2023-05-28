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

struct QUAKE_FX_STRUCT {
	float intensity;
	float period;
	GameInstant start;
	GameDuration duration;
	bool active;
	Anglef targetAngle;
	Vec3f targetPos;
	GameInstant lastChange;
};

QUAKE_FX_STRUCT QuakeFx = {0.f, 0.f, GameInstant(), GameDuration(), false, Anglef(), Vec3f(), GameInstant()};

void AddQuakeFX(float intensity, GameDuration duration, float period, bool sound) {

	if(duration <= 0 && period <= 0)
		return;

	if(QuakeFx.active) {
		QuakeFx.intensity += intensity;
		QuakeFx.duration += duration;
		QuakeFx.period += period;
		QuakeFx.period *= .5f;
	} else {
		QuakeFx.intensity = intensity;
		QuakeFx.start = g_gameTime.now();
		QuakeFx.duration = duration;
		QuakeFx.period = period;
		QuakeFx.active = true;
		QuakeFx.lastChange = 0;
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
		
		// adjustment of frequency to linearity because the actual value is nonlinear, using a formula of x/(x + k)
		// values chosen so a scripted earthquake with a frequency of 10 changes direction every ~10ms
		float multiplier = 2 * (1.f - QuakeFx.period / (QuakeFx.period + 10.0f));
		float updateInterval = QuakeFx.period * multiplier;
		
		float itmod = 1.f - (elapsed / QuakeFx.duration);
		
		float power = QuakeFx.intensity * itmod * 0.01f * 0.5f; // 0.5 to compensate for algorithm change

		GameDuration timeFromLastUpdate = g_gameTime.now() - QuakeFx.lastChange;

		if(toMsf(timeFromLastUpdate) >= updateInterval || QuakeFx.lastChange == 0) {
			QuakeFx.targetAngle.setPitch(Random::getf(-1.f, 1.f) * power);
			QuakeFx.targetAngle.setYaw(Random::getf(-1.f, 1.f) * power);
			QuakeFx.targetAngle.setRoll(Random::getf(-1.f, 1.f) * power);
			QuakeFx.targetPos = arx::randomVec(-power, power);
			QuakeFx.lastChange = g_gameTime.now();
		}
		
		cam->m_pos += QuakeFx.targetPos * glm::clamp(toMsf(timeFromLastUpdate) / updateInterval, 0.f, 1.f);
		cam->angle += QuakeFx.targetAngle * glm::clamp(toMsf(timeFromLastUpdate) / updateInterval, 0.f, 1.f);
	}
}
