/*
 * Copyright 2015 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/texture/TextureStage.h"

#include "input/Input.h"

extern TextureContainer * scursor[];

CursorTrail::CursorTrail() {
	m_storedTime = 0;
	iNbOldCoord = 0;
	iMaxOldCoord = 40;
}

void CursorTrail::reset() {
	iNbOldCoord = 0;
}

void CursorTrail::add(float time, const Vec2s & pos)
{
	iOldCoord[iNbOldCoord] = pos;
	
	const float targetFPS = 61.f;
	const float targetDelay = 1000.f / targetFPS;
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

bool CursorTrail::ComputePer(const Vec2s & _psPoint1, const Vec2s & _psPoint2, TexturedVertex * _psd3dv1, TexturedVertex * _psd3dv2, float _fSize) {
	
	Vec2f sTemp((float)(_psPoint2.x - _psPoint1.x), (float)(_psPoint2.y - _psPoint1.y));
	float fTemp = sTemp.x;
	sTemp.x = -sTemp.y;
	sTemp.y = fTemp;
	float fMag = glm::length(sTemp);
	if(fMag < std::numeric_limits<float>::epsilon()) {
		return false;
	}

	fMag = _fSize / fMag;

	_psd3dv1->p.x=(sTemp.x*fMag);
	_psd3dv1->p.y=(sTemp.y*fMag);
	_psd3dv2->p.x=((float)_psPoint1.x)-_psd3dv1->p.x;
	_psd3dv2->p.y=((float)_psPoint1.y)-_psd3dv1->p.y;
	_psd3dv1->p.x+=((float)_psPoint1.x);
	_psd3dv1->p.y+=((float)_psPoint1.y);

	return true;
}

void CursorTrail::DrawLine2D(float _fSize, Color3f color) {
	
	if(iNbOldCoord < 2) {
		return;
	}
	
	float fSize = _fSize / iNbOldCoord;
	float fTaille = fSize;
	
	float fDColorRed = color.r / iNbOldCoord;
	float fColorRed = fDColorRed;
	float fDColorGreen = color.g / iNbOldCoord;
	float fColorGreen = fDColorGreen;
	float fDColorBlue = color.b / iNbOldCoord;
	float fColorBlue = fDColorBlue;
	
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendInvDstColor);
	GRenderer->ResetTexture(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	TexturedVertex v[4];
	v[0].p.z = v[1].p.z = v[2].p.z = v[3].p.z = 0.f;
	v[0].rhw = v[1].rhw = v[2].rhw = v[3].rhw = 1.f;
	
	v[0].color = v[2].color = Color3f(fColorRed, fColorGreen, fColorBlue).toRGB();
	
	if(!ComputePer(iOldCoord[0], iOldCoord[1], &v[0], &v[2], fTaille)) {
		v[0].p.x = v[2].p.x = iOldCoord[0].x;
		v[0].p.y = v[2].p.y = iOldCoord[1].y;
	}
	
	for(int i = 1; i < iNbOldCoord - 1; i++) {
		
		fTaille += fSize;
		fColorRed += fDColorRed;
		fColorGreen += fDColorGreen;
		fColorBlue += fDColorBlue;
		
		if(ComputePer(iOldCoord[i], iOldCoord[i + 1], &v[1], &v[3], fTaille)) {
			
			v[1].color = v[3].color = Color3f(fColorRed, fColorGreen, fColorBlue).toRGB();
			EERIEDRAWPRIM(Renderer::TriangleStrip, v, 4);
			
			v[0].p.x = v[1].p.x;
			v[0].p.y = v[1].p.y;
			v[0].color = v[1].color;
			v[2].p.x = v[3].p.x;
			v[2].p.y = v[3].p.y;
			v[2].color = v[3].color;
		}
		
	}
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}


MenuCursor::MenuCursor()
	: m_size(Vec2s(64, 64))
{
	exited = true;
	
	bMouseOver=false;
	
	m_currentFrame=0;
	lFrameDiff = 0.f;
}

MenuCursor::~MenuCursor()
{
}

void MenuCursor::SetMouseOver() {
	bMouseOver=true;
}

void MenuCursor::DrawOneCursor(const Vec2s& mousePos) {
	
	if(!GInput->isMouseInWindow()) {
		return;
	}
	
	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterNearest);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapClamp);

	EERIEDrawBitmap2(Rectf(Vec2f(mousePos), m_size.x, m_size.y),
	                 0.00000001f, scursor[m_currentFrame], Color::white);

	GRenderer->GetTextureStage(0)->setMinFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setMagFilter(TextureStage::FilterLinear);
	GRenderer->GetTextureStage(0)->setWrapMode(TextureStage::WrapRepeat);
}

void MenuCursor::reset() {
	trail.reset();
}

void MenuCursor::update(float time) {
	
	bool inWindow = GInput->isMouseInWindow();
	if(inWindow && exited) {
		// Mouse is re-entering the window - reset the cursor trail
		reset();
	}
	exited = !inWindow;
	
	Vec2s iDiff = m_size / Vec2s(2);
	
	trail.add(time, GInput->getMousePosAbs() + iDiff);
}


extern float ARXDiffTimeMenu;

void MenuCursor::DrawCursor() {
	
	trail.draw();
	
	DrawOneCursor(GInput->getMousePosAbs());

	lFrameDiff += ARXDiffTimeMenu;

	if(lFrameDiff > 70.f) {
		if(bMouseOver) {
			if(m_currentFrame < 4) {
				m_currentFrame++;
			} else {
				if(m_currentFrame > 4) {
					m_currentFrame--;
				}
			}
			bMouseOver=false;
		} else {
			if(m_currentFrame > 0) {
				m_currentFrame++;

				if(m_currentFrame > 7)
					m_currentFrame=0;
			}
		}

		lFrameDiff = 0.f;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
}
