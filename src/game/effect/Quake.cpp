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

#include "game/effect/Quake.h"

#include "core/GameTime.h"
#include "scene/GameSound.h"

struct QUAKE_FX_STRUCT {
	float intensity;
	float frequency;
	unsigned long start;
	unsigned long duration;
	bool sound;
};

QUAKE_FX_STRUCT QuakeFx;

void AddQuakeFX(float intensity, float duration, float period, bool sound) {

	if(QuakeFx.intensity > 0.f) {
		QuakeFx.intensity += intensity;

		QuakeFx.duration += (unsigned long)duration;
		QuakeFx.frequency += period;
		QuakeFx.frequency *= .5f;
		QuakeFx.sound = QuakeFx.sound || sound;

		if(sound)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);
	} else {
		QuakeFx.intensity = intensity;

		QuakeFx.start = checked_range_cast<unsigned long>(arxtime.get_frame_time());

		QuakeFx.duration = (unsigned long)duration;
		QuakeFx.frequency = period;
		QuakeFx.sound = sound;

		if(sound)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);
	}

	if(!sound) {
		if(QuakeFx.duration > 1500)
			QuakeFx.duration = 1500;

		if(QuakeFx.intensity > 220)
			QuakeFx.intensity = 220;
	}
}

void RemoveQuakeFX() {

	QuakeFx.intensity=0.f;
}

void ManageQuakeFX(EERIE_CAMERA * cam) {
	if(QuakeFx.intensity>0.f) {
		float tim=(float)arxtime.get_frame_time()-(float)QuakeFx.start;

		if(tim >= QuakeFx.duration) {
			QuakeFx.intensity=0.f;
			return;
		}

		float itmod=1.f-(tim/QuakeFx.duration);
		float periodicity=std::sin((float)arxtime.get_frame_time()*QuakeFx.frequency*( 1.0f / 100 ));

		if(periodicity > 0.5f && QuakeFx.sound)
			ARX_SOUND_PlaySFX(SND_QUAKE, NULL, 1.0F - 0.5F * QuakeFx.intensity);

		float truepower = periodicity * QuakeFx.intensity * itmod * 0.01f;
		float halfpower = truepower * .5f;
		cam->orgTrans.pos += randomVec(-halfpower, halfpower);
		cam->angle.setPitch(cam->angle.getPitch() + Random::getf() * truepower - halfpower);
		cam->angle.setYaw(cam->angle.getYaw() + Random::getf() * truepower - halfpower);
		cam->angle.setRoll(cam->angle.getRoll() + Random::getf() * truepower - halfpower);
	}
}
