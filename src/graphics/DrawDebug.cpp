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

#include "graphics/DrawDebug.h"

#include <sstream>

#include "core/Core.h"
#include "animation/AnimationRender.h"

#include "graphics/Math.h"
#include "graphics/Draw.h"
#include "graphics/DrawLine.h"
#include "graphics/particle/ParticleEffects.h"

#include "gui/Interface.h"
#include "gui/Text.h"

#include "scene/Object.h"
#include "graphics/data/TextureContainer.h"

#include "game/Entity.h"
#include "game/EntityManager.h"
#include "game/Inventory.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "ai/Paths.h"
#include "font/Font.h"
#include "graphics/effects/Fog.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "physics/Anchors.h"
#include "physics/Collisions.h"

extern bool EXTERNALVIEW; // *sigh*
extern float GetIOHeight(Entity * io);
extern float GetIORadius(Entity * io);

static TextureContainer * g_lightSourceTexture = NULL;
static EERIE_3DOBJ * g_fogObject = NULL;
static EERIE_3DOBJ * g_nodeObject = NULL;

const float DebugTextMaxDistance = 1000.f;
const float DebugPhysicsMaxDistance = 2000.f;

void drawDebugInitialize() {
	g_lightSourceTexture = TextureContainer::LoadUI("graph/particles/light");
	g_fogObject = LoadTheObj("editor/obj3d/fog_generator.teo", "node_teo maps");
	g_nodeObject = LoadTheObj("editor/obj3d/node.teo", "node_teo maps");
}

void drawDebugRelease() {
	delete g_fogObject;
	g_fogObject = NULL;
	delete g_nodeObject;
	g_nodeObject = NULL;
}

enum DebugViewType {
	DebugView_None,
	DebugView_Entities,
	DebugView_Paths,
	DebugView_PathFind,
	DebugView_Lights,
	DebugView_Fogs,
	DebugView_CollisionShapes,
	DebugView_Portals,
	DebugViewCount
};

static DebugViewType g_debugView = DebugView_None;

void drawDebugCycleViews() {
	g_debugView = static_cast<DebugViewType>(g_debugView + 1);
	if(g_debugView == DebugViewCount) {
		g_debugView = DebugView_None;
	}
}

static void drawDebugBoundingBox(const EERIE_2D_BBOX & box, Color color = Color::white) {
	if(box.valid()) {
		drawLine2D(box.min.x, box.min.y, box.max.x, box.min.y, 0.01f, color);
		drawLine2D(box.max.x, box.min.y, box.max.x, box.max.y, 0.01f, color);
		drawLine2D(box.max.x, box.max.y, box.min.x, box.max.y, 0.01f, color);
		drawLine2D(box.min.x, box.max.y, box.min.x, box.min.y, 0.01f, color);
	}
}

static void drawDebugLights() {
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetCulling(Renderer::CullNone);
	
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		
		EERIE_LIGHT * light = GLight[i];
		if(!light) {
			continue;
		}
		
		TexturedVertex center;
		EE_RTP(light->pos, &center);
		
		const Rect mouseTestRect(
		center.p.x - 20,
		center.p.y - 20,
		center.p.x + 20,
		center.p.y + 20
		);
		
		if(mouseTestRect.contains(Vec2i(DANAEMouse))) {
			
			Sphere fallstart;
			fallstart.origin = light->pos;
			fallstart.radius = light->fallstart;
			drawLineSphere(fallstart, Color(Color3<u8>::green, 200));
			Sphere fallend;
			fallend.origin = light->pos;
			fallend.radius = light->fallend;
			drawLineSphere(fallend, Color(Color3<u8>::red, 200));
		}
		
		if(light->m_screenRect.valid())
			drawDebugBoundingBox(light->m_screenRect);
	}
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		
		EERIE_LIGHT * light = GLight[i];
		if(!light) {
			continue;
		}
		
		EERIEDrawSprite(light->pos, 11.f, g_lightSourceTexture, light->rgb.to<u8>(), 0.5f);
	}
	
	GRenderer->SetCulling(Renderer::CullCCW);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	
}

static void drawDebugPortals() {
	
	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	
	for(size_t i = 0; i < portals->portals.size(); i++) {
		
		EERIE_PORTALS & po = portals->portals[i];
		
		Color color = Color::red;
		if(po.useportal == 1) {
			color = Color::green;
		}
		
		EERIEPOLY & epp = po.poly;

		drawLine(epp.v[0].p, epp.v[1].p, color);
		drawLine(epp.v[1].p, epp.v[3].p, color);
		drawLine(epp.v[2].p, epp.v[3].p, color);
		drawLine(epp.v[0].p, epp.v[2].p, color);
		
	}
	
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::Fog, true);
	
}

static void drawDebugPaths() {
	
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	
	for(long i = 0; i < nbARXpaths; i++) {
		
		ARX_PATH * path = ARXpaths[i];
		if(!path) {
			continue;
		}
		
		Vec3f center = Vec3f_ZERO;
		int n = 0;
		
		std::vector<Vec3f> points;
		for(long i = 0; i < path->nb_pathways; i++) {
			const ARX_PATHWAY & node = path->pathways[i];
			Vec3f pos = path->pos + node.rpos;
			points.push_back(pos);
			center += pos, n++;
			if(node.flag == PATHWAY_BEZIER) {
				// Interpolate bezier curve by creating linear segments
				if(i + 2 >= path->nb_pathways) {
					break;
				}
				const size_t nsegments = 20;
				for(size_t j = 0; j < nsegments; j++) {
					points.push_back(path->interpolateCurve(i, float(j) / nsegments));
				}
				i++; // Skip the control point
			}
		}
		
		// Zones only check the bounding box for the y coordinate - adjust display for that
		if(path->height > 0) {
			for(size_t i = 0; i < points.size(); i++) {
				points[i].y = path->bbmin.y;
			}
		}
		
		if(path->height != 0 || ((path->flags & PATH_LOOP) && points.size() > 0)) {
			points.push_back(points[0]);
		}
		
		Color color = (path->height != 0) ? Color::green : Color::red;
		
		for(size_t i = 0; i + 1 < points.size(); i++) {
			drawLine(points[i], points[i + 1], color);
		}
		
		if(path->height > 0) {
			Vec3f offset(0.f, (path->bbmax.y - path->bbmin.y), 0.f);
			for(size_t i = 0; i + 1 < points.size(); i++) {
				drawLine(points[i] + offset, points[i + 1] + offset, color);
			}
			for(size_t i = 0; i < points.size(); i++) {
				drawLine(points[i], points[i] + offset, color);
			}
		}
		
		// Display the name and controlling entity for close zones
		if(!path->name.empty() || !path->controled.empty()) {
			if(path->height > 0) {
				center = (path->bbmin + path->bbmax) / 2.f;
			} else if(n != 0) {
				center /= float(n);
			} else {
				center = path->pos;
			}
			if(closerThan(center, player.pos, DebugTextMaxDistance)) {
				std::string controlledby;
				if(!path->controled.empty()) {
					controlledby = "Controlled by: " + path->controled;
				}
				Color textcolor = color * 0.5f + Color::gray(0.5f);
				drawTextAt(hFontDebug, center, path->name, textcolor, controlledby);
			}
		}
		
	}
	
	GRenderer->SetRenderState(Renderer::DepthTest, true);
	
}

static void drawDebugPathFinding() {
	
	if(!ACTIVEBKG || !ACTIVEBKG->anchors) {
		return;
	}
	
	const float zbias = 0.00001f;
	
	for(long i = 0; i < ACTIVEBKG->nbanchors; i++) {
		
		const ANCHOR_DATA & node = ACTIVEBKG->anchors[i];
		
		Color color1 = (node.flags & ANCHOR_FLAG_BLOCKED) ? Color::blue : Color::green;
		for(long j = 0; j < node.nblinked; j++) {
			long k = node.linked[j];
			if(k >= 0 && k < ACTIVEBKG->nbanchors && i < k) {
				const ANCHOR_DATA & other = ACTIVEBKG->anchors[k];
				Color color2 = (other.flags & ANCHOR_FLAG_BLOCKED) ? Color::blue : Color::green;
				drawLine(node.pos, other.pos, color1, color2, zbias);
			}
		}
		
		if(node.height != 0.f) {
			Vec3f toppos = node.pos + Vec3f(0.f, node.height, 0.f);
			drawLine(node.pos, toppos, Color::blue, zbias);
		}
		
	}
	
	// Highlight active paths
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		const Entity * entity = entities[handle];
		
		if(!entity || !(entity->ioflags & IO_NPC)) {
			continue;
		}
		const IO_PATHFIND & pathfind = entity->_npcdata->pathfind;
		if(pathfind.listnb <= 0 || !pathfind.list) {
			continue;
		}
		
		// Draw visited nodes yello and target nodes as red
		for(long j = 1; j < pathfind.listnb; j++) {
			short k0 = pathfind.list[j - 1], k1 = pathfind.list[j];
			if(k0 >= 0 && k0 < ACTIVEBKG->nbanchors && k1 >= 0 && k1 < ACTIVEBKG->nbanchors) {
				const ANCHOR_DATA & n0 = ACTIVEBKG->anchors[k0], & n1 = ACTIVEBKG->anchors[k1];
				Color color0 = (j     <= pathfind.listpos) ? Color::yellow : Color::red;
				Color color1 = (j + 1 <= pathfind.listpos) ? Color::yellow : Color::red;
				drawLine(n0.pos, n1.pos, color0, color1, 2.f * zbias);
			}
		}
		
		// Highlight end nodes
		short k0 = pathfind.list[pathfind.listnb - 1];
		if(k0 >= 0 && k0 < ACTIVEBKG->nbanchors) {
			Anglef angle(0.f, 0.f, 0.f);
			Vec3f scale(0.5f);
			RenderMaterial mat;
			mat.setBlendType(RenderMaterial::Opaque);
			mat.setDepthTest(true);
			
			Draw3DObject(g_nodeObject, angle, ACTIVEBKG->anchors[k0].pos, scale, Color3f::white, mat);
		}
		
		// Show entity ID at the active node
		if(pathfind.listpos < pathfind.listnb) {
			short k1 = pathfind.list[pathfind.listpos];
			if(k1 >= 0 && k1 < ACTIVEBKG->nbanchors) {
				if(closerThan(ACTIVEBKG->anchors[k1].pos, player.pos, DebugTextMaxDistance)) {
					drawTextAt(hFontDebug, ACTIVEBKG->anchors[k1].pos, entity->idString());
					GRenderer->SetRenderState(Renderer::DepthTest, true);
				}
			}
		}
		
	}
	
}

static void drawDebugFogs() {
	
	for(size_t i = 0; i < MAX_FOG; i++) {
		
		FOG_DEF * fog = &fogs[i];
		if(!fog->exist) {
			continue;
		}
		
			Anglef angle(0.f, 0.f, 0.f);
			Vec3f scale(1.f);
			RenderMaterial mat;
			mat.setBlendType(RenderMaterial::Opaque);
			mat.setDepthTest(true);
			
			Draw3DObject(g_fogObject, angle, fog->pos, scale, Color3f::white, mat);
		
		if(fog->special & FOG_DIRECTIONAL) {
			drawLine(fog->pos, fog->pos + fog->move * 50.f, Color::white);
		}
		
		Sphere fogsize;
		fogsize.origin = fog->pos;
		fogsize.radius = fog->size;
		drawLineSphere(fogsize, Color(Color3<u8>::blue, 200));
		
	}
	
}

//! Debug function to show the physical box of an object
static void drawDebugCollisionShape(EERIE_3DOBJ * obj) {
	
	if(!obj || !obj->pbox) {
		return;
	}
	
	Sphere sphere;
	sphere.origin = obj->pbox->vert[0].pos;
	sphere.radius = obj->pbox->radius;
	drawLineSphere(sphere, Color::white);
	
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	
	Color shapeColor = Color::yellow;
	
	if(obj->pbox->active == 2) {
		shapeColor = Color::green;
	}
	
	for(long k = 0; k + 1 < obj->pbox->nb_physvert; k++) {
		drawLine(obj->pbox->vert[k].pos, obj->pbox->vert[k+1].pos, shapeColor);
	}
	
}

static void drawDebugEntityPhysicsCylinder(Entity * io) {
	
	if(!(io->ioflags & IO_NPC)) {
		return;
	}
	
	CollisionFlags levitate = 0;
	
	if(spells.getSpellOnTarget(io->index(), SPELL_LEVITATE)) {
		levitate = CFLAG_LEVITATE;
	}
	
	Cylinder cyll;
	cyll.height = GetIOHeight(io);
	cyll.radius = GetIORadius(io);
	cyll.origin = io->physics.startpos;
	drawLineCylinder(cyll, Color::green);
	
	if(!(AttemptValidCylinderPos(cyll, io, levitate | CFLAG_NPC))) {
		cyll.height = -40.f;
		drawLineCylinder(cyll, Color::blue);
		cyll.height = GetIOHeight(io);
	}
	
	cyll.origin = io->physics.targetpos;
	drawLineCylinder(cyll, Color::red);
	
}

static void drawDebugEntityPhysicsCylinders() {
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * entity = entities[handle];
		
		if(!entity || !closerThan(entity->pos, player.pos, DebugPhysicsMaxDistance))
			continue;
		
		drawDebugCollisionShape(entity->obj);
		drawDebugEntityPhysicsCylinder(entity);
	}
}

static void drawDebugEntities() {
	
	for(size_t i = 1; i < entities.size(); i++) {
		const EntityHandle handle = EntityHandle(i);
		Entity * entity = entities[handle];
		
		if(!entity) {
			continue;
		}
		
		Color color = Color::white;
		bool visible = true;
		switch(entity->show) {
			case SHOW_FLAG_DESTROYED:    continue; // Don't even display the name
			case SHOW_FLAG_IN_INVENTORY: continue;
			case SHOW_FLAG_ON_PLAYER:    continue;
			case SHOW_FLAG_LINKED:       continue;
			case SHOW_FLAG_NOT_DRAWN:    color = Color::magenta; visible = false; break;
			case SHOW_FLAG_HIDDEN:       color = Color::yellow;  visible = false; break;
			case SHOW_FLAG_MEGAHIDE:     color = Color::green;   visible = false; break;
			case SHOW_FLAG_KILLED:       color = Color::red;     visible = false; break;
			case SHOW_FLAG_IN_SCENE:     color = Color::white;   visible = true;  break;
			case SHOW_FLAG_TELEPORTING:  color = Color::blue;    visible = true;  break;
		}
		if((entity->ioflags & IO_CAMERA) || (entity->ioflags & IO_MARKER)) {
			color = Color::gray(0.7f), visible = false;
		}
		if(DRAGINTER == entity) {
			color = Color::white, visible = true;
		}
		
		if(visible) {
			drawDebugBoundingBox(entity->bbox2D, Color::blue);
		}
		
		if(closerThan(entity->pos, player.pos, DebugTextMaxDistance)) {
			
			if(visible && entity->bbox2D.valid()) {
				int x = (entity->bbox2D.min.x + entity->bbox2D.max.x) / 2;
				int y = entity->bbox2D.min.y - hFontDebug->getLineHeight() - 2;
				UNICODE_ARXDrawTextCenter(hFontDebug, Vec2f(x, y), entity->idString(), color);
			} else {
				drawTextAt(hFontDebug, entity->pos, entity->idString(), color);
			}
			
			if(entity->obj) {
				for(size_t j = 0; j < entity->obj->linked.size(); j++) {
					Vec3f pos = entity->obj->vertexlist3[entity->obj->linked[j].lidx].v;
					Entity * other = entity->obj->linked[j].io;
					drawTextAt(hFontDebug, pos, other->idString(), Color::cyan);
				}
			}
			
		}
		
	}
	
}

void drawDebugRender() {
	
	if(g_debugView == DebugView_None) {
		return;
	}
	
	std::stringstream ss;
	ss << "Debug Display: ";
	
	switch(g_debugView) {
		case DebugView_Entities: {
			ss << "Entities";
			drawDebugEntities();
			break;
		}
		case DebugView_Paths: {
			ss << "Paths and Zones";
			drawDebugPaths();
			break;
		}
		case DebugView_PathFind: {
			ss << "Pathfinding";
			drawDebugPathFinding();
			break;
		}
		case DebugView_Lights: {
			ss << "Lights";
			drawDebugLights();
			break;
		}
		case DebugView_Fogs: {
			drawDebugFogs();
			ss << "Fogs";
			break;
		}
		case DebugView_CollisionShapes: {
			ss << "Collision Shapes";
			drawDebugEntityPhysicsCylinders();
			break;
		}
		case DebugView_Portals: {
			ss << "Portals";
			drawDebugPortals();
			break;
		}
		default: return;
	}
	
	s32 width = hFontDebug->getTextSize(ss.str()).x;
	Vec2f pos = Vec2f(g_size.topRight());
	pos += Vec2f(-10 , 10);
	pos += Vec2f(-width, 0);
	
	ARX_TEXT_Draw(hFontDebug, pos, ss.str(), Color::yellow);
}
