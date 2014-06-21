/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_DRAWLINE_H
#define ARX_GRAPHICS_DRAWLINE_H

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/GraphicsTypes.h"

#include "graphics/Draw.h"

void EERIEDraw2DLine(float x0, float y0, float x1, float y1, float z, Color col);

void EERIEDraw2DRect(const Rectf & rect, float z, Color col);
void EERIEDrawFill2DRectDegrad(float x0, float y0, float x1, float y1, float z, Color cold, Color cole);

void DrawLineSphere(const Sphere & sphere, Color color);
void EERIEDraw3DCylinder(const EERIE_CYLINDER & cyl, Color col);
void EERIEDrawTrue3DLine(const Vec3f & orgn, const Vec3f & dest, Color col);
void EERIEDraw3DLine(const Vec3f & orgn, const Vec3f & dest, Color col, float zbias = 0.f);
void EERIEDraw3DLine(const Vec3f & orgn, const Vec3f & dest, Color color1, Color color2, float zbias = 0.f);

#endif // ARX_GRAPHICS_DRAWLINE_H
