/*
 * Copyright 2015-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/hud/HudCommon.h"

#include "game/Entity.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "gui/Interface.h"


HudIconBase::HudIconBase()
	: m_tex(NULL)
	, m_isSelected(false)
	, m_haloActive(false)
	, m_haloColor(Color3f::white)
{ }

void HudIconBase::draw() {
	arx_assert(m_tex);
	
	if(m_haloActive && m_tex->getHalo()) {
		ARX_INTERFACE_HALO_Render(m_haloColor, HALO_ACTIVE, m_tex->getHalo(), m_rect.topLeft(), Vec2f(m_scale));
	}
	
	EERIEDrawBitmap(m_rect, 0.001f, m_tex, Color::white);
	
	if(m_isSelected) {
		UseRenderState state(render2D().blendAdditive());
		EERIEDrawBitmap(m_rect, 0.001f, m_tex, Color::white);
	}
	
}


Vec2f getAnchorPos(const Rectf & rect, const Anchor anchor) {
	switch(anchor) {
		case Anchor_TopLeft:      return rect.topLeft();
		case Anchor_TopCenter:    return rect.topCenter();
		case Anchor_TopRight:     return rect.topRight();
		case Anchor_LeftCenter:   return Vec2f(rect.left, rect.top + rect.height() / 2);
		case Anchor_Center:       return rect.center();
		case Anchor_RightCenter:  return Vec2f(rect.right, rect.top + rect.height() / 2);
		case Anchor_BottomLeft:   return rect.bottomLeft();
		case Anchor_BottomCenter: return rect.bottomCenter();
		case Anchor_BottomRight:  return rect.bottomRight();
		default: return rect.topLeft();
	}
}

Rectf createChild(const Rectf & parent, const Anchor parentAnchor, const Vec2f & size, const Anchor childAnchor) {
	
	Vec2f parentPos = getAnchorPos(parent, parentAnchor);
	
	Rectf child(size.x, size.y);
	Vec2f childPos = getAnchorPos(child, childAnchor);
	child.move(parentPos.x, parentPos.y);
	child.move(-childPos.x, -childPos.y);
	return child;
}
