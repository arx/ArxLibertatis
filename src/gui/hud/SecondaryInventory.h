/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_HUD_SECONDARYINVENTORY_H
#define ARX_GUI_HUD_SECONDARYINVENTORY_H

#include "gui/hud/HudCommon.h"
#include "math/Vector.h"

class Entity;
class TextureContainer;


class SecondaryInventoryPickAllHudIcon : public HudIconBase {
	
	Vec2f m_size;
	
public:
	
	SecondaryInventoryPickAllHudIcon() : m_size(0.f) { }
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	
};

class SecondaryInventoryCloseHudIcon : public HudIconBase {
	
	Vec2f m_size;
	
public:
	
	SecondaryInventoryCloseHudIcon() : m_size(0.f) { }
	
	void init();
	void update(const Rectf & parent);
	void updateInput();
	
};


class SecondaryInventoryHud : public HudItem {
	
	Vec2f m_size;
	TextureContainer * ingame_inventory;
	TextureContainer * m_canNotSteal;
	TextureContainer * m_defaultBackground;
	
	SecondaryInventoryPickAllHudIcon m_pickAllButton;
	SecondaryInventoryCloseHudIcon m_closeButton;
	
public:
	
	SecondaryInventoryHud()
		: m_size(0.f)
		, ingame_inventory(NULL)
		, m_canNotSteal(NULL)
		, m_defaultBackground(NULL)
		, m_fadeDirection(Fade_stable)
		, m_fadePosition(0.f)
	{ }
	
	void init();
	void update();
	void updateRect();
	void draw();
	
	void updateInputButtons();
	
	/*!
	 * \brief Returns true if position is in secondary inventory
	 */
	bool containsPos(const Vec2s & pos);
	Entity * getObj(const Vec2s & pos);
	
	void dropEntity();
	bool dragEntity(Entity * io, const Vec2s & pos);
	
	void close();
	
	void updateFader();
	
	enum Fade {
		Fade_left = -1,
		Fade_stable = 0,
		Fade_right = 1
	};
	
	Fade m_fadeDirection;
	float m_fadePosition;
	
};

extern SecondaryInventoryHud g_secondaryInventoryHud;

#endif // ARX_GUI_HUD_SECONDARYINVENTORY_H
