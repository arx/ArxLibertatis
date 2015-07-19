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

#ifndef ARX_GUI_HUD_H
#define ARX_GUI_HUD_H

#include "gui/hud/HudCommon.h"
#include "math/Types.h"

extern bool bIsAiming;

/*!
 * \brief the hit strength diamond shown at the bottom of the UI.
 */
class HitStrengthGauge : public HudItem {
private:
	TextureContainer * m_emptyTex;
	TextureContainer * m_fullTex;
	TextureContainer * m_hitTex;
	
	Vec2f m_size;
	Vec2f m_hitSize;
	
	Rectf m_hitRect;
	
	float m_intensity;
	bool bHitFlash;
	unsigned long ulHitFlash;
	float m_flashIntensity;
	
public:
	HitStrengthGauge();
	
	void init();
	void updateRect(const Rectf & parent);
	void update();
	void draw();
	
	void requestFlash(float flashIntensity);
};

class BookIconGui : public HudIconBase {
private:
	Vec2f m_size;
	unsigned long ulBookHaloTime;
	
public:
	BookIconGui();
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	
	void requestHalo();
	void requestFX();
	
private:
	void MakeBookFX(const Vec3f & pos);
};

class BackpackIconGui : public HudIconBase {
public:
	void init();
	void update(const Rectf & parent);
	void updateInput();
	void draw();
};

class StealIconGui : public HudIconBase {
private:
	Vec2f m_size;
	Vec2f m_pos;
	
public:
	void init();
	void updateRect(const Rectf & parent);
	void updateInput();
	void draw();
};

class LevelUpIconGui : public HudIconBase {
private:
	Vec2f m_pos;
	Vec2f m_size;
	bool m_visible;
	
public:
	LevelUpIconGui();
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	void draw();
};

class HudRoot : public HudItem {
public:
	void setScale(float scale);
	
	void init();
	void updateInput();
};

extern HudRoot g_hudRoot;

void purseIconGuiRequestHalo();

void mecanismIconReset();
//! Show the quick save indicator for a short time
void showQuickSaveIcon();
void hitStrengthGaugeRequestFlash(float flashIntensity);
void bookIconGuiRequestFX();
void bookIconGuiRequestHalo();
//! Hide the quick save indicator
void hideQuickSaveIcon();

#endif // ARX_GUI_HUD_H
