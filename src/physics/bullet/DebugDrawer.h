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

#ifndef ARX_PHYSICS_BULLET_DEBUGDRAWER_H
#define ARX_PHYSICS_BULLET_DEBUGDRAWER_H

#include <vector>

#include <bullet/LinearMath/btIDebugDraw.h>

#include "graphics/data/Mesh.h"

class ArxBulletDebugDrawer : public btIDebugDraw {
public:
	ArxBulletDebugDrawer();
	
	virtual int getDebugMode() const { return m_debugMode; }
	virtual void setDebugMode(int debugMode) { m_debugMode = debugMode; }
	
	virtual void drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & color);
	virtual void drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & fromColor, const btVector3 & toColor);
	
	virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
	virtual void draw3dText(const btVector3& location, const char* textString);
	
	virtual void reportErrorWarning(const char* warningString);
	
	void renderIt();
	
private:
	int m_debugMode;
	
	void push(Vec3f pos, Color col);
	std::vector<TexturedVertex> vertices;
};

#endif
