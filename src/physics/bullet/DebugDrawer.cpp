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

#include "physics/bullet/DebugDrawer.h"
#include "io/log/Logger.h"
#include "gui/Text.h"
#include "graphics/Draw.h"
#include "physics/bullet/BulletTypes.h"


ArxBulletDebugDrawer::ArxBulletDebugDrawer() {
}

void ArxBulletDebugDrawer::drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & color) {
	Color col = convColor(color);
	
	push(conv(from), col);
	push(conv(to), col);
}

void ArxBulletDebugDrawer::drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & fromColor, const btVector3 & toColor) {
	push(conv(from), convColor(fromColor));
	push(conv(to), convColor(toColor));
}

void ArxBulletDebugDrawer::drawContactPoint(const btVector3 & PointOnB, const btVector3 & normalOnB, btScalar distance, int lifeTime, const btVector3 & color) {
	ARX_UNUSED(PointOnB);
	ARX_UNUSED(normalOnB);
	ARX_UNUSED(distance);
	ARX_UNUSED(lifeTime);
	ARX_UNUSED(color);
}

void ArxBulletDebugDrawer::reportErrorWarning(const char* warningString) {
	LogError << "Bullet " << warningString;
}


void ArxBulletDebugDrawer::draw3dText(const btVector3& location, const char* textString) {

	drawTextAt(hFontDebug, conv(location), std::string(textString), Color::white);
}


void ArxBulletDebugDrawer::renderIt() {
	
	//GRenderer->SetRenderState(Renderer::DepthTest, false);
	//GRenderer->SetDepthBias(30);
	//GRenderer->ResetTexture(0);
	
	EERIEDRAWPRIM(Renderer::LineList, &vertices[0], vertices.size());
	vertices.clear();
}

void ArxBulletDebugDrawer::push(Vec3f pos, Color col) {
	
	TexturedVertex out;
	worldToClipSpace(pos, out);
	
	out.color = col.toRGBA();
	vertices.push_back(out);
}
