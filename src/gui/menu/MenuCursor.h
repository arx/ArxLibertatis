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

#ifndef ARX_GUI_MENU_MENUCURSOR_H
#define ARX_GUI_MENU_MENUCURSOR_H

#include "core/TimeTypes.h"
#include "graphics/Color.h"
#include "math/Vector.h"

struct TexturedVertex;

class CursorTrail {
	
public:
	
	CursorTrail();
	
	void reset();
	void add(PlatformDuration time, const Vec2s & pos);
	void draw();
	
private:
	
	PlatformDuration m_storedTime;
	int iNbOldCoord;
	int iMaxOldCoord;
	Vec2s iOldCoord[256];
	
	bool ComputePer(const Vec2s & p1, const Vec2s & p2, TexturedVertex * v1, TexturedVertex * v2, float size);
	void DrawLine2D(float _fSize, Color3f color);
	
};

class MenuCursor {
	
public:
	
	MenuCursor();
	virtual ~MenuCursor();
	
	void reset();
	void update();
	void SetMouseOver();
	void DrawCursor();
	
private:
	
	void DrawOneCursor(const Vec2s & mousePos);
	
	Vec2s m_size;
	bool exited; //! Has the mouse exited the window
	PlatformDuration lFrameDiff;
	int m_currentFrame;
	bool bMouseOver;
	CursorTrail trail;
	
};

extern MenuCursor * pMenuCursor;

class TextureContainer;

class ThumbnailCursor {
	
public:
	
	ThumbnailCursor()
		: m_renderTexture(NULL)
		, m_loadTexture(NULL)
	{ }
	
	void render();
	
	void clear();
	
	TextureContainer * m_renderTexture;
	TextureContainer * m_loadTexture;
	
};

extern ThumbnailCursor g_thumbnailCursor;

#endif // ARX_GUI_MENU_MENUCURSOR_H
