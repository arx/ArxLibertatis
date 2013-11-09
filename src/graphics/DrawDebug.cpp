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
#include "game/NPC.h"
#include "game/Player.h"

#include "ai/Paths.h"
#include "font/Font.h"
#include "graphics/effects/Fog.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "physics/Anchors.h"
#include "physics/Collisions.h"

TextureContainer * lightsource_tc = NULL;
EERIE_3DOBJ * fogobj = NULL;
EERIE_3DOBJ * nodeobj = NULL;				// Node 3D Object

const float DebugTextMaxDistance = 1000.f;

void DrawDebugInit() {
	lightsource_tc = TextureContainer::LoadUI("graph/particles/light");
	fogobj = LoadTheObj("editor/obj3d/fog_generator.teo", "node_teo maps");
	nodeobj = LoadTheObj("editor/obj3d/node.teo", "node_teo maps");
}

void DrawDebugRelease() {
	delete fogobj;
	fogobj = NULL;

	delete nodeobj;
	nodeobj = NULL;
}

enum ARX_INTERFACE_EDITION_MODE {
	EDITION_NONE,
	EDITION_Entities,
	EDITION_Paths,
	EDITION_PathFind,
	EDITION_LIGHTS,
	EDITION_FOGS,
	EDITION_CollisionShape,
	EDITION_Portals,
	EDITION_EnumSize
};

static ARX_INTERFACE_EDITION_MODE EDITION = EDITION_NONE;

void DrawDebugToggleDisplayTypes() {

	EDITION = static_cast<ARX_INTERFACE_EDITION_MODE>(EDITION + 1);

	if(EDITION == EDITION_EnumSize) {
		EDITION = EDITION_NONE;
	}
}

extern bool MouseInRect(const float x0, const float y0, const float x1, const float y1);

static void drawDebugBoundingBox(const EERIE_2D_BBOX & box, Color color = Color::white) {
	if(box.valid()) {
		EERIEDraw2DLine(box.min.x, box.min.y, box.max.x, box.min.y, 0.01f, color);
		EERIEDraw2DLine(box.max.x, box.min.y, box.max.x, box.max.y, 0.01f, color);
		EERIEDraw2DLine(box.max.x, box.max.y, box.min.x, box.max.y, 0.01f, color);
		EERIEDraw2DLine(box.min.x, box.max.y, box.min.x, box.min.y, 0.01f, color);
	}
}

static void drawDebugLights() {
	
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	
	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		
		EERIE_LIGHT * light = GLight[i];
		if(!light) {
			continue;
		}
		
		TexturedVertex in, center;
		in.p = light->pos;
		EE_RTP(&in, &center);
		if(MouseInRect(center.p.x - 20, center.p.y - 20, center.p.x + 20, center.p.y + 20)) {
			GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
			EERIE_SPHERE fallstart;
			fallstart.origin = light->pos;
			fallstart.radius = light->fallstart;
			DrawLineSphere(fallstart, Color(Color3<u8>::green, 200));
			EERIE_SPHERE fallend;
			fallend.origin = light->pos;
			fallend.radius = light->fallend;
			DrawLineSphere(fallend, Color(Color3<u8>::red, 200));
		}
		
		GRenderer->SetBlendFunc(Renderer::BlendSrcAlpha, Renderer::BlendSrcAlpha);
		EERIEDrawSprite(&in, 11.f, lightsource_tc, light->rgb.to<u8>(), 2.f);
		
	}
	
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

		EERIEDraw3DLine(epp.v[0].p, epp.v[1].p, color);
		EERIEDraw3DLine(epp.v[1].p, epp.v[3].p, color);
		EERIEDraw3DLine(epp.v[2].p, epp.v[3].p, color);
		EERIEDraw3DLine(epp.v[0].p, epp.v[2].p, color);
		
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
			EERIEDraw3DLine(points[i], points[i + 1], color);
		}
		
		if(path->height > 0) {
			Vec3f offset(0.f, (path->bbmax.y - path->bbmin.y), 0.f);
			for(size_t i = 0; i + 1 < points.size(); i++) {
				EERIEDraw3DLine(points[i] + offset, points[i + 1] + offset, color);
			}
			for(size_t i = 0; i < points.size(); i++) {
				EERIEDraw3DLine(points[i], points[i] + offset, color);
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
				EERIEDraw3DLine(node.pos, other.pos, color1, color2, zbias);
			}
		}
		
		if(node.height != 0.f) {
			Vec3f toppos = node.pos + Vec3f(0.f, node.height, 0.f);
			EERIEDraw3DLine(node.pos, toppos, Color::blue, zbias);
		}
		
	}
	
	// Highlight active paths
	for(size_t i = 1; i < entities.size(); i++) {
		
		const Entity * entity = entities[i];
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
				EERIEDraw3DLine(n0.pos, n1.pos, color0, color1, 2.f * zbias);
			}
		}
		
		// Highlight end nodes
		short k0 = pathfind.list[pathfind.listnb - 1];
		if(k0 >= 0 && k0 < ACTIVEBKG->nbanchors) {
			Anglef angle(0.f, 0.f, 0.f);
			Vec3f scale(0.5f);
			DrawEERIEObjEx(nodeobj, &angle, &ACTIVEBKG->anchors[k0].pos, &scale, Color3f::white);
		}
		
		// Show entity ID at the active node
		if(pathfind.listpos < pathfind.listnb) {
			short k1 = pathfind.list[pathfind.listpos];
			if(k1 >= 0 && k1 < ACTIVEBKG->nbanchors) {
				if(closerThan(ACTIVEBKG->anchors[k1].pos, player.pos, DebugTextMaxDistance)) {
					drawTextAt(hFontDebug, ACTIVEBKG->anchors[k1].pos, entity->long_name());
					GRenderer->SetRenderState(Renderer::DepthTest, true);
				}
			}
		}
		
	}
	
}

static void drawDebugFogs() {
	
	for(long i = 0; i < MAX_FOG; i++) {
		
		FOG_DEF * fog = &fogs[i];
		if(!fog->exist) {
			continue;
		}
		
		if(fogobj) {
			Anglef angle(0.f, 0.f, 0.f);
			Vec3f scale(1.f);
			DrawEERIEObjEx(fogobj, &angle, &fog->pos, &scale, Color3f::white);
		}
		
		if(fog->special & FOG_DIRECTIONAL) {
			EERIEDraw3DLine(fog->pos, fog->pos + fog->move * 50.f, Color::white);
		}
		
		EERIE_SPHERE fogsize;
		fogsize.origin = fog->pos;
		fogsize.radius = fog->size;
		DrawLineSphere(fogsize, Color(Color3<u8>::blue, 200));
		
	}
	
}

extern float GetIOHeight(Entity * io);
extern float GetIORadius(Entity * io);

//! Debug function to show the physical box of an object
static void drawDebugCollisionShape(EERIE_3DOBJ * obj) {
	
	if(!obj || !obj->pbox) {
		return;
	}
	
	EERIE_SPHERE sphere;
	sphere.origin = obj->pbox->vert[0].pos;
	sphere.radius = obj->pbox->radius;
	DrawLineSphere(sphere, Color::white);
	
	GRenderer->SetRenderState(Renderer::DepthTest, false);
	
	Color shapeColor = Color::yellow;
	
	if(obj->pbox->active == 2) {
		shapeColor = Color::green;
	}
	
	for(long k = 0; k + 1 < obj->pbox->nb_physvert; k++) {
		EERIEDraw3DLine(obj->pbox->vert[k].pos, obj->pbox->vert[k+1].pos, shapeColor);
	}
	
}

static void drawDebugEntityPhysicsCylinder(Entity * io) {
	
	if(!(io->ioflags & IO_NPC)) {
		return;
	}
	
	CollisionFlags levitate = 0;
	
	if(ARX_SPELLS_GetSpellOn(io, SPELL_LEVITATE) >= 0) {
		levitate = CFLAG_LEVITATE;
	}
	
	EERIE_CYLINDER cyll;
	cyll.height = GetIOHeight(io);
	cyll.radius = GetIORadius(io);
	cyll.origin = io->physics.startpos;
	EERIEDraw3DCylinder(cyll, Color::green);
	
	if(!(AttemptValidCylinderPos(&cyll, io, levitate | CFLAG_NPC))) {
		cyll.height = -40.f;
		EERIEDraw3DCylinder(cyll, Color::blue);
		cyll.height = GetIOHeight(io);
	}
	
	cyll.origin = io->physics.targetpos;
	EERIEDraw3DCylinder(cyll, Color::red);
	
}

static void drawDebugEntityPhysicsCylinders() {
	for(size_t i = 1; i < entities.size(); i++) {
		Entity * entity = entities[i];
		if(entity) {
			drawDebugCollisionShape(entity->obj);
			drawDebugEntityPhysicsCylinder(entity);
		}
	}
}

static void drawDebugEntities() {
	
	for(size_t i = 1; i < entities.size(); i++) {
		
		Entity * entity = entities[i];
		if(!entity) {
			continue;
		}
		
		bool visible = (entity->show != SHOW_FLAG_HIDDEN)
		               && !(entity->ioflags & IO_CAMERA) && !(entity->ioflags & IO_MARKER);
		
		if(visible) {
			drawDebugBoundingBox(entity->bbox2D, Color::blue);
		}
		
		if(closerThan(entity->pos, player.pos, DebugTextMaxDistance)) {
			if(visible && entity->bbox2D.valid()) {
				int x = (entity->bbox2D.min.x + entity->bbox2D.max.x) / 2;
				int y = entity->bbox2D.min.y - hFontDebug->getLineHeight() - 2;
				UNICODE_ARXDrawTextCenter(hFontDebug, x, y, entity->long_name(), Color::white);
			} else {
				drawTextAt(hFontDebug, entity->pos, entity->long_name());
			}
		}
		
	}
	
}

void RenderAllNodes() {

	Anglef angle(0.f, 0.f, 0.f);
	Vec3f scale(1.f, 1.f, 1.f);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	for(long i=0; i<nodes.nbmax; i++) {
		if (nodes.nodes[i].exist) {
			DrawEERIEObjEx(nodeobj, &angle, &nodes.nodes[i].pos, &scale, Color3f::white);

			/* FIXME DrawEERIEObjEx does not calculate BBOX2D
			nodes.nodes[i].bboxmin.x=(short)BBOX2D.min.x;
			nodes.nodes[i].bboxmin.y=(short)BBOX2D.min.y;
			nodes.nodes[i].bboxmax.x=(short)BBOX2D.max.x;
			nodes.nodes[i].bboxmax.y=(short)BBOX2D.max.y;
			*/

			if(nodeobj->vertexlist[nodeobj->origin].vert.p.z > 0.f && nodeobj->vertexlist[nodeobj->origin].vert.p.z<0.9f) {
				float xx = nodeobj->vertexlist[nodeobj->origin].vert.p.x - 40.f;
				float yy = nodeobj->vertexlist[nodeobj->origin].vert.p.y - 40.f;
				ARX_TEXT_Draw(hFontDebug, xx, yy, nodes.nodes[i].UName, Color::yellow); //font
			}

			if(nodes.nodes[i].selected) {
				EERIEDraw2DLine(nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmin.y, nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmin.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmin.y, nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmax.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(nodes.nodes[i].bboxmax.x, nodes.nodes[i].bboxmax.y, nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmax.y, 0.01f, Color::yellow);
				EERIEDraw2DLine(nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmax.y, nodes.nodes[i].bboxmin.x, nodes.nodes[i].bboxmin.y, 0.01f, Color::yellow);
			}

			for(size_t j = 0; j < MAX_LINKS; j++) {
				if(nodes.nodes[i].link[j]!=-1) {
					EERIEDrawTrue3DLine(nodes.nodes[i].pos, nodes.nodes[nodes.nodes[i].link[j]].pos, Color::green);
				}
			}
		}
	}
}

void DrawDebugRender() {
	
	if(EDITION == EDITION_NONE) {
		return;
	}
	
	std::stringstream ss;
	ss << "Debug Display: ";
	
	switch(EDITION) {
		case EDITION_Entities: {
			ss << "Entities";
			drawDebugEntities();
			break;
		}
		case EDITION_Paths: {
			ss << "Paths and Zones";
			drawDebugPaths();
			break;
		}
		case EDITION_PathFind: {
			ss << "Pathfinding";
			drawDebugPathFinding();
			break;
		}
		case EDITION_LIGHTS: {
			ss << "Lights";
			drawDebugLights();
			break;
		}
		case EDITION_FOGS: {
			drawDebugFogs();
			ss << "Fogs";
			break;
		}
		case EDITION_CollisionShape: {
			ss << "Collision Shapes";
			drawDebugEntityPhysicsCylinders();
			break;
		}
		case EDITION_Portals: {
			ss << "Portals";
			drawDebugPortals();
			break;
		}
		default: return;
	}
	
	ARX_TEXT_Draw(hFontDebug, 100, 2, ss.str(), Color::yellow);
	
}
