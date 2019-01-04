/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

void EERIEDrawFill2DRectDegrad(Vec2f a, Vec2f b, float z, Color cold, Color cole);

void drawLine(const Vec2f & from, const Vec2f & to, float z, Color col);
void drawLine(const Vec3f & orgn, const Vec3f & dest, Color col, float zbias = 0.f);
void drawLine(const Vec3f & orgn, const Vec3f & dest, Color color1, Color color2, float zbias = 0.f);
void drawLineRectangle(const Rectf & rect, float z, Color col);
void drawLineSphere(const Sphere & sphere, Color color);
void drawLineCylinder(const Cylinder & cyl, Color col);
void drawLineCross(Vec2f v, float z, Color color, float size);
void drawLineCross(Vec3f v, Color c, float size = 1.f);
void drawLineTriangle(Vec3f v0, Vec3f v1, Vec3f v2, Color color);

#endif // ARX_GRAPHICS_DRAWLINE_H
