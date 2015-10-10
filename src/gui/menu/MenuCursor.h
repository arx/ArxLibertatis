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

#ifndef ARX_GUI_MENU_MENUCURSOR_H
#define ARX_GUI_MENU_MENUCURSOR_H

#include "graphics/Color.h"
#include "math/Vector.h"

struct TexturedVertex;

class CursorTrail {
public:
	CursorTrail();
	
	void reset();
	void add(float time, const Vec2s & pos);
	void draw();
	
private:
	float m_storedTime;
	int   iNbOldCoord;
	int   iMaxOldCoord;
	Vec2s iOldCoord[256];
	
	bool ComputePer(const Vec2s & _psPoint1, const Vec2s & _psPoint2, TexturedVertex * _psd3dv1, TexturedVertex * _psd3dv2, float _fSize);
	void DrawLine2D(float _fSize, Color3f color);
};

class MenuCursor {

public:
	MenuCursor();
	virtual ~MenuCursor();
	
	void reset();
	void update(float time);
	void SetMouseOver();
	void DrawCursor();
	
private:
	void DrawOneCursor(const Vec2s & mousePos);
	
	Vec2s m_size;
	bool exited; //! Has the mouse exited the window
	float				lFrameDiff;
	int					m_currentFrame;
	bool				bMouseOver;
	CursorTrail trail;
};

#endif // ARX_GUI_MENU_MENUCURSOR_H
