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

#ifndef ARX_GUI_HUD_PLAYERINVENTORY_H
#define ARX_GUI_HUD_PLAYERINVENTORY_H

#include "gui/hud/HudCommon.h"
#include "math/Vector.h"

class Entity;
class TextureContainer;

extern bool bInventorySwitch;
extern float fDecPulse;

class PlayerInventoryHud : public HudItem {
	
	TextureContainer * m_heroInventory;
	TextureContainer * m_heroInventoryLink;
	TextureContainer * m_heroInventoryUp;
	TextureContainer * m_heroInventoryDown;
	
	Vec2f m_arrowsAnchor;
	
	Vec2f m_slotSize;
	Vec2f m_slotSpacing;
	
	Vec2f m_bagSize;
	
	bool m_isClosing;
	long m_inventoryY;
	short m_currentBag;
	
public:
	
	PlayerInventoryHud()
		: m_heroInventory(NULL)
		, m_heroInventoryLink(NULL)
		, m_heroInventoryUp(NULL)
		, m_heroInventoryDown(NULL)
		, m_arrowsAnchor(0.f)
		, m_slotSize(0.f)
		, m_slotSpacing(0.f)
		, m_bagSize(0.f)
		, m_isClosing(false)
		, m_inventoryY(0)
		, m_currentBag(0)
	{ }
	
	void init();
	Vec2f anchorPosition();
	void update();
	void updateRect();
	bool updateInput();
	void draw();
	
	void nextBag();
	void previousBag();
	void setCurrentBag(short bag);
	
	bool containsPos(const Vec2s & pos);
	Entity * getObj(const Vec2s & pos);
	
	void dropEntity();
	void dragEntity(Entity * io, const Vec2s & pos);
	
	void close();
	bool isClosing();
	void resetPos();
	
private:
	
	void CalculateInventoryCoordinates();
	void drawBag(size_t bag, Vec2i i);
	bool InPlayerInventoryBag(const Vec2s & pos);
	
};

extern PlayerInventoryHud g_playerInventoryHud;

#endif // ARX_GUI_HUD_PLAYERINVENTORY_H
