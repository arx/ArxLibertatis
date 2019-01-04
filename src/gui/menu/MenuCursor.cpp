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

#include "gui/menu/MenuCursor.h"

#include "core/Core.h"
#include "core/GameTime.h"

#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/Renderer.h"
#include "graphics/texture/TextureStage.h"

#include "gui/Interface.h"

#include "input/Input.h"

extern TextureContainer * scursor[];

CursorTrail::CursorTrail()
	: m_storedTime(0)
	, iNbOldCoord(0)
	, iMaxOldCoord(40)
{ }

void CursorTrail::reset() {
	iNbOldCoord = 0;
}

void CursorTrail::add(PlatformDuration time, const Vec2s & pos)
{
	iOldCoord[iNbOldCoord] = pos;
	
	const s64 targetFPS = 61;
	const PlatformDuration targetDelay = PlatformDurationUs((1000 * 1000) / targetFPS);
	
	m_storedTime += time;
	if(m_storedTime > targetDelay) {
		m_storedTime = std::min(targetDelay, m_storedTime - targetDelay);
		
		iNbOldCoord++;
		
		if(iNbOldCoord >= iMaxOldCoord) {
			iNbOldCoord = iMaxOldCoord - 1;
			memmove(iOldCoord, iOldCoord + 1, sizeof(Vec2s) * iNbOldCoord);
		}
	}
}

void CursorTrail::draw() {
	DrawLine2D(10.f, Color3f(.725f, .619f, 0.56f));
}

bool CursorTrail::ComputePer(const Vec2s & p1, const Vec2s & p2, TexturedVertex * v1, TexturedVertex * v2, float size) {
	
	Vec2f p1f = Vec2f(p1);
	
	Vec2f sTemp = Vec2f(p2) - p1f;
	float fTemp = sTemp.x;
	sTemp.x = -sTemp.y;
	sTemp.y = fTemp;
	float fMag = glm::length(sTemp);
	if(fMag < std::numeric_limits<float>::epsilon()) {
		return false;
	}

	fMag = size / fMag;

	v1->p.x = sTemp.x * fMag;
	v1->p.y = sTemp.y * fMag;
	v2->p.x = p1f.x - v1->p.x;
	v2->p.y = p1f.y - v1->p.y;
	v1->p.x += p1f.x;
	v1->p.y += p1f.y;

	return true;
}

void CursorTrail::DrawLine2D(float _fSize, Color3f color) {
	
	if(iNbOldCoord < 2) {
		return;
	}
	
	float incSize = _fSize / iNbOldCoord;
	float currentSize = incSize;
	
	Color3f incColor = Color3f(color.r / iNbOldCoord, color.g / iNbOldCoord, color.b / iNbOldCoord);
	
	Color3f currentColor = incColor;
	
	UseRenderState state(render2D().blend(BlendDstColor, BlendInvDstColor));
	GRenderer->ResetTexture(0);
	
	TexturedVertex v[4];
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = 0.f;
	v[0].w = v[1].w = v[2].w = v[3].w = 1.f;
	
	v[0].color = v[2].color = currentColor.toRGB();
	
	if(!ComputePer(iOldCoord[0], iOldCoord[1], &v[0], &v[2], currentSize)) {
		v[0].p.x = v[2].p.x = iOldCoord[0].x;
		v[0].p.y = v[2].p.y = iOldCoord[1].y;
	}
	
	for(int i = 1; i < iNbOldCoord - 1; i++) {
		
		currentSize += incSize;
		currentColor += incColor;
		
		if(ComputePer(iOldCoord[i], iOldCoord[i + 1], &v[1], &v[3], currentSize)) {
			
			v[1].color = v[3].color = currentColor.toRGB();
			EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
			
			v[0].p.x = v[1].p.x;
			v[0].p.y = v[1].p.y;
			v[0].color = v[1].color;
			v[2].p.x = v[3].p.x;
			v[2].p.y = v[3].p.y;
			v[2].color = v[3].color;
		}
		
	}
	
}


MenuCursor::MenuCursor()
	: m_size(Vec2s(64, 64))
{
	exited = true;
	
	bMouseOver = false;
	
	m_currentFrame = 0;
	lFrameDiff = 0;
}

MenuCursor::~MenuCursor() { }

void MenuCursor::SetMouseOver() {
	bMouseOver = true;
}

void MenuCursor::DrawOneCursor(const Vec2s & mousePos) {
	
	if(!GInput->isMouseInWindow()) {
		return;
	}
	
	UseTextureState textureState(getInterfaceTextureFilter(), TextureStage::WrapClamp);
	
	float scale = getInterfaceScale(config.interface.cursorScale, config.interface.cursorScaleInteger);
	EERIEDrawBitmap(Rectf(Vec2f(mousePos), m_size.x * scale, m_size.y * scale),
	                0.00000001f, scursor[m_currentFrame], Color::white);
	
}

void MenuCursor::reset() {
	trail.reset();
}

void MenuCursor::update() {
	
	bool inWindow = GInput->isMouseInWindow();
	if(inWindow && exited) {
		// Mouse is re-entering the window - reset the cursor trail
		reset();
	}
	exited = !inWindow;
	
	Vec2s iDiff = m_size / Vec2s(2);
	
	trail.add(g_platformTime.lastFrameDuration(), GInput->getMousePosition() + iDiff);
}


void MenuCursor::DrawCursor() {
	
	trail.draw();
	
	DrawOneCursor(GInput->getMousePosition());
	
	lFrameDiff += g_platformTime.lastFrameDuration();
	
	if(lFrameDiff > PlatformDurationMs(70)) {
		if(bMouseOver) {
			if(m_currentFrame < 4) {
				m_currentFrame++;
			} else {
				if(m_currentFrame > 4) {
					m_currentFrame--;
				}
			}
			bMouseOver = false;
		} else {
			if(m_currentFrame > 0) {
				m_currentFrame++;
				if(m_currentFrame > 7) {
					m_currentFrame = 0;
				}
			}
		}
		lFrameDiff = 0;
	}
	
}

MenuCursor * pMenuCursor = NULL;


ThumbnailCursor g_thumbnailCursor;

void ThumbnailCursor::render() {
	
	if(m_renderTexture) {
		
		Vec2f offset = Vec2f(0, 0);
		
		if(DANAEMouse.y + config.interface.thumbnailSize.y > g_size.height()) {
			offset.y -= config.interface.thumbnailSize.y;
		}
		
		Vec2f pos = Vec2f(DANAEMouse) + offset;
		
		Rectf rect = Rectf(pos, config.interface.thumbnailSize.x, config.interface.thumbnailSize.y);
		
		EERIEDrawBitmap(rect, 0.001f, m_loadTexture, Color::white);
		drawLineRectangle(rect, 0.01f, Color::white);

		m_renderTexture = NULL;
	}
}

void ThumbnailCursor::clear() {
	m_renderTexture = NULL;
}
