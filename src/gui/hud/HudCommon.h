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

#ifndef ARX_GUI_HUD_HUDCOMMON_H
#define ARX_GUI_HUD_HUDCOMMON_H

#include "graphics/Color.h"
#include "math/Rectangle.h"

class TextureContainer;

class HudItem {
public:
	HudItem()
		: m_scale(1.f)
	{}
	
	void setScale(float scale) {
		m_scale = scale;
	}

	Rectf rect() { return m_rect; }
	
protected:
	float m_scale;
	Rectf m_rect;
};

class HudIconBase : public HudItem {
	
protected:
	
	TextureContainer * m_tex;
	bool m_isSelected;
	
	bool m_haloActive;
	Color3f m_haloColor;
	
public:
	
	HudIconBase();
	
	void draw();
	
};

enum Anchor {
	Anchor_TopLeft,
	Anchor_TopCenter,
	Anchor_TopRight,
	Anchor_LeftCenter,
	Anchor_Center,
	Anchor_RightCenter,
	Anchor_BottomLeft,
	Anchor_BottomCenter,
	Anchor_BottomRight,
};

Vec2f getAnchorPos(const Rectf & rect, Anchor anchor);

Rectf createChild(const Rectf & parent, Anchor parentAnchor, const Vec2f & size, Anchor childAnchor);

#endif // ARX_GUI_HUD_HUDCOMMON_H
