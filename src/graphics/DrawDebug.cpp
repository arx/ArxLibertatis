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

#include "ai/Paths.h"
#include "graphics/effects/Fog.h"
#include "scene/Interactive.h"
#include "scene/Light.h"
#include "physics/Collisions.h"

TextureContainer * lightsource_tc = NULL;
EERIE_3DOBJ * fogobj = NULL;
EERIE_3DOBJ * nodeobj = NULL;				// Node 3D Object

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

enum ARX_INTERFACE_EDITION_MODE
{
	EDITION_NONE,
	EDITION_LIGHTS,
	EDITION_FOGS,
	EDITION_BoundingBoxes,
	EDITION_CollisionShape,
	EDITION_Portals,
	EDITION_Paths,
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

void DrawDebugLights() {

	//GRenderer->SetCulling(Renderer::CullNone);
	//GRenderer->SetRenderState(Renderer::DepthTest, true);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	for(size_t i = 0; i < MAX_LIGHTS; i++) {
		EERIE_LIGHT *light = GLight[i];

		if(!light)
			continue;

		TexturedVertex in;
		TexturedVertex center;

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
}

void DrawDebugPortals() {
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

void DrawDebugPaths() {
	GRenderer->SetRenderState(Renderer::Fog, false);
	GRenderer->SetRenderState(Renderer::DepthTest, false);

	for(long i = 0; i < nbARXpaths; i++) {
		ARX_PATH * path = ARXpaths[i];

		if(!path)
			continue;

		bool isBezier = false;
		std::vector<Vec3f> points;

		for(long i = 0; i < path->nb_pathways; i++) {
			ARX_PATHWAY node = path->pathways[i];

			if(node.flag != PATHWAY_STANDARD) {
				isBezier = true;
			}

			points.push_back(path->pos + node.rpos);
		}

		if(isBezier) {
			// TODO Bezier path handling
			continue;
		}

		if(points.size() > 0) {
			points.push_back(points[0]);
		}

		for(size_t i = 0; i + 1 < points.size(); i++) {
			EERIEDraw3DLine(points[i], points[i+1], Color::red);
		}

		if(path->height > 0) {
			Vec3f offset(0.f, -path->height, 0.f);

			for(size_t i = 0; i + 1 < points.size(); i++) {
				EERIEDraw3DLine(points[i] + offset, points[i+1] + offset, Color::red);
			}

			for(size_t i = 0; i < points.size(); i++) {
				EERIEDraw3DLine(points[i], points[i] + offset, Color::red);
			}
		}
	}
}


/**
 * @brief Debug function to show the physical box of an object
 */
void DrawDebugCollisionShape(EERIE_3DOBJ * obj) {

	if(!obj || !obj->pbox)
		return;

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

void DrawDebugFogs() {

	EERIE_QUAT rotation;
	Quat_Init(&rotation);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);

	for(long i = 0; i < MAX_FOG; i++) {
		FOG_DEF *fog = &fogs[i];

		if(!fog->exist)
			continue;

			if(fogobj) {
				Anglef angle(0.f, 0.f, 0.f);
				Vec3f scale(1.f, 1.f, 1.f);
				DrawEERIEObjEx(fogobj, &angle, &fog->pos, &scale, Color3f::white);
			}

//			fog->bboxmin = BBOXMIN;
//			fog->bboxmax = BBOXMAX;

			if(fog->special & FOG_DIRECTIONAL) {
				EERIEDraw3DLine(fog->pos, fog->pos + fog->move * 50.f, Color::white);
			}

//			if(fog->selected) {
//				EERIEDraw2DLine(fog->bboxmin.x, fog->bboxmin.y, fog->bboxmax.x, fog->bboxmin.y, 0.01f, Color::yellow);
//				EERIEDraw2DLine(fog->bboxmax.x, fog->bboxmin.y, fog->bboxmax.x, fog->bboxmax.y, 0.01f, Color::yellow);
//				EERIEDraw2DLine(fog->bboxmax.x, fog->bboxmax.y, fog->bboxmin.x, fog->bboxmax.y, 0.01f, Color::yellow);
//				EERIEDraw2DLine(fog->bboxmin.x, fog->bboxmax.y, fog->bboxmin.x, fog->bboxmin.y, 0.01f, Color::yellow);
//			}
	}
}

extern float GetIOHeight(Entity * io);
extern float GetIORadius(Entity * io);

void debugEntityPhysicsCylinder(Entity * io) {

	if(!(io->ioflags & IO_NPC) )
		return;

	CollisionFlags levitate = 0;

	if(ARX_SPELLS_GetSpellOn(io, SPELL_LEVITATE) >= 0) {
		levitate = CFLAG_LEVITATE;
	}

	EERIE_CYLINDER cyll;
	cyll.height = GetIOHeight(io);
	cyll.radius = GetIORadius(io);
	cyll.origin = io->physics.startpos;
	EERIEDraw3DCylinder(cyll, Color::green);

	if (!(AttemptValidCylinderPos(&cyll, io, levitate | CFLAG_NPC)))
	{
		cyll.height = -40.f;
		EERIEDraw3DCylinder(cyll, Color::blue);
		cyll.height = GetIOHeight(io);
	}

	cyll.origin = io->physics.targetpos;
	EERIEDraw3DCylinder(cyll, Color::red);
}

void DrawDebugScreenBoundingBox(Entity * io) {
	Color color = Color::blue;
	EERIE_2D_BBOX & box = io->bbox2D;
	if(box.min.x != box.max.x && box.min.x < g_size.width()) {
		EERIEDraw2DLine(box.min.x, box.min.y, box.max.x, box.min.y, 0.01f, color);
		EERIEDraw2DLine(box.max.x, box.min.y, box.max.x, box.max.y, 0.01f, color);
		EERIEDraw2DLine(box.max.x, box.max.y, box.min.x, box.max.y, 0.01f, color);
		EERIEDraw2DLine(box.min.x, box.max.y, box.min.x, box.min.y, 0.01f, color);
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
				ARX_TEXT_Draw(hFontInBook, xx, yy, nodes.nodes[i].UName, Color::yellow); //font
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

	if(EDITION == EDITION_NONE)
		return;

	//RenderAllNodes();

	std::stringstream ss;
	ss << "Debug Display: ";

	switch(EDITION) {
	case EDITION_LIGHTS: {
		ss << "Lights";
		DrawDebugLights();
		break;
	}
	case EDITION_FOGS: {
		DrawDebugFogs();
		ss << "Fogs";
		break;
	}
	case EDITION_BoundingBoxes: {
		ss << "Bounding Boxes";
		break;
	}
	case EDITION_CollisionShape: {
		ss << "Collision Shapes";
		break;
	}
	case EDITION_Portals: {
		ss << "Portals";
		DrawDebugPortals();
		break;
	}
	case EDITION_Paths: {
		ss << "Paths";
		DrawDebugPaths();
		break;
	}
	default:
		return;
	}

	for(size_t i = 1; i < entities.size(); i++) {
		Entity * io = entities[i];

		if(!io)
			continue;

		if(EDITION == EDITION_CollisionShape) {
			DrawDebugCollisionShape(io->obj);
			debugEntityPhysicsCylinder(io);
		}

		if(EDITION == EDITION_BoundingBoxes) {
			DrawDebugScreenBoundingBox(io);
		}
	}

	//ss <<  NbIOSelected;
	ARX_TEXT_Draw(hFontInBook, 100, 2, ss.str(), Color::yellow);
}
