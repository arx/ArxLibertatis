/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GAME_SPELL_FLYINGEYE_H
#define ARX_GAME_SPELL_FLYINGEYE_H

#include "core/GameTime.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/data/TextureContainer.h"
#include "math/Angle.h"

class FlyingEye {

public:

	enum EyeStatus {
		EYEBALL_INACTIVE = 0,
		EYEBALL_ACTIVE,
		EYEBALL_LAUNCHED,
		EYEBALL_APPEAR,
		EYEBALL_DISAPPEAR
	};

	EyeStatus m_state;
	long m_progress;
	Vec3f m_pos;
	Anglef m_angle;
	Vec3f m_size;

	FlyingEye();
	~FlyingEye();

	void init();
	void release();

	bool isActive();
	bool isInactive();

	void reset();
	void launch();
	void update();
	void drawMagicSightInterface();
	void end();

	void render();

private:
	GameInstant m_timeCreation;
	TextureContainer* m_eyeTex;
	std::unique_ptr<EERIE_3DOBJ> m_eyeballobj; // EyeBall 3D Object
	float m_floatZ;
};

extern float MagicSightFader;
extern FlyingEye eyeball;

#endif // ARX_GAME_SPELL_FLYINGEYE_H
