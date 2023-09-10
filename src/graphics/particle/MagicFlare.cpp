/*
 * Copyright 2011-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/particle/MagicFlare.h"

#include <cstdio>
#include <sstream>

#include "core/Application.h"
#include "core/Core.h"
#include "core/Config.h"
#include "core/GameTime.h"

#include "input/Input.h"

#include "scene/Light.h"
#include "scene/Interactive.h"

#include "graphics/Math.h"
#include "graphics/Vertex.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/particle/ParticleTextures.h"
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"
#include "math/RandomVector.h"

struct FLARETC {
	TextureContainer* lumignon;
	TextureContainer* lumignon2;
	TextureContainer* plasm;
	TextureContainer* shine[11];
};

static FLARETC g_magicFlareTextures;

class MagicFlare {
public:
	unsigned char exist = 0;
	char type = -1;
	short flags = 0;
	Vec3f p = Vec3f(0.f);
	Vec2f pos = Vec2f(0.f);
	PlatformDuration tolive;
	Color3f rgb;
	float size = 0.f;
	LightHandle dynlight;
	Entity * io = nullptr;
	bool bDrawBitmap = false;
};

class MagicFlareContainer {
public:
	MagicFlareContainer();
	MagicFlare& operator[](size_t element);
	void removeFlare(MagicFlare& flare);
	void addFlare(const Vec2f& pos, float sm, short typ, Entity* io, bool bookDraw);
	long flaggedCount();
	void removeFlareEntityPtr(const Entity* entity);
	void init();
	void removeAll();
	void update();
	void changeColor();

	short shinum = 1;
private:
	static const size_t g_magicFlaresMax = 500;
	MagicFlare m_flares[g_magicFlaresMax];
	long g_magicFlaresCount;
	short m_currentColor;

	size_t findUsableIndex();
};

static MagicFlareContainer g_magicFlares;

MagicFlareContainer::MagicFlareContainer()
	: g_magicFlaresCount(0)
	, m_currentColor(0)
{}

MagicFlare& MagicFlareContainer::operator[](size_t element) {
	
	return m_flares[element];
}

size_t MagicFlareContainer::findUsableIndex() {
	
	size_t oldest = 0;
	size_t i;
	for(i = 0; i < g_magicFlaresMax; i++) {
		if(!g_magicFlares[i].exist) {
			break;
		}
		if(g_magicFlares[i].tolive < g_magicFlares[oldest].tolive) {
			oldest = i;
		}
	}
	if(i >= g_magicFlaresMax) {
		g_magicFlares.removeFlare(g_magicFlares[oldest]);
		i = oldest;
	}
	return i;
}

void MagicFlareContainer::addFlare(const Vec2f& pos, float sm, short typ, Entity* io, bool bookDraw) {
	
	size_t index = findUsableIndex();
	MagicFlare& flare = g_magicFlares[index];
	flare.exist = 1;
	g_magicFlaresCount++;

	flare.bDrawBitmap = bookDraw;

	flare.io = io;
	if(io) {
		flare.flags = 1;
		io->flarecount++;
	} else {
		flare.flags = 0;
	}

	flare.pos.x = pos.x - Random::getf(0.f, 4.f);
	flare.pos.y = pos.y - Random::getf(0.f, 4.f);

	if(!bookDraw) {
		if(io) {
			float vx = -(flare.pos.x - g_size.center().x) * 0.2173913f;
			float vy = (flare.pos.y - g_size.center().y) * 0.1515151515151515f;
			flare.p = io->pos;
			flare.p += angleToVectorXZ(io->angle.getYaw() + vx) * 100.f;
			flare.p.y += std::sin(glm::radians(MAKEANGLE(io->angle.getPitch() + vy))) * 100.f - 150.f;
		} else {
			flare.p = screenToWorldSpace(pos, 75.f);
		}
	} else {
		flare.p = Vec3f(flare.pos.x, flare.pos.y, 0.001f);
	}

	switch(m_currentColor) {
		case 0:
		{
			flare.rgb = Color3f(0.4f, 0.f, 0.4f) + Color3f(2.f / 3, 2.f / 3, 2.f / 3) * randomColor3f();
			break;
		}
		case 1:
		{
			flare.rgb = Color3f(0.5f, 0.5f, 0.f) + Color3f(0.625f, 0.625f, 0.55f) * randomColor3f();
			break;
		}
		case 2:
		{
			flare.rgb = Color3f(0.4f, 0.f, 0.f) + Color3f(2.f / 3, 0.55f, 0.55f) * randomColor3f();
			break;
		}
		default: arx_unreachable();
	}

	if(typ == -1) {
		float zz = eeMousePressed1() ? 0.29f : ((sm > 0.5f) ? Random::getf() : 1.f);
		if(zz < 0.2f) {
			flare.type = 2;
			flare.size = Random::getf(42.f, 84.f);
			flare.tolive = Random::get(1600000us, 3200000us);
		} else if(zz < 0.5f) {
			flare.type = 3;
			flare.size = Random::getf(16.f, 68.f);
			flare.tolive = Random::get(1600000us, 3200000us);
		} else {
			flare.type = 1;
			flare.size = Random::getf(32.f, 56.f) * sm;
			flare.tolive = Random::get(3400000us, 4400000us);
		}
	} else {
		flare.type = (Random::getf() > 0.8f) ? 1 : 4;
		flare.size = Random::getf(64.f, 102.f) * sm;
		flare.tolive = Random::get(3400000us, 4400000us);
	}

	flare.dynlight = {};

	for(unsigned int kk = 0; kk < 3; kk++) {

		if(Random::getf() < 0.5f) {
			continue;
		}

		PARTICLE_DEF* pd = createParticle(true);
		if(!pd) {
			break;
		}

		if(!bookDraw) {
			pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
			if(!io) {
				pd->m_flags |= PARTICLE_NOZBUFFER;
			}
		} else {
			pd->m_flags = FADE_IN_AND_OUT | PARTICLE_2D;
		}

		pd->ov = flare.p + arx::randomVec(-5.f, 5.f);
		pd->move = Vec3f(0.f, 5.f, 0.f);
		pd->sizeDelta = -2.f;
		pd->duration = 1300ms + kk * 100ms + Random::get(0ms, 800ms);
		pd->tc = g_particleTextures.fire2;
		if(kk == 1) {
			pd->move.y = 4.f;
			pd->size = 1.5f;
		} else {
			pd->size = Random::getf(1.f, 2.f);
		}
		pd->rgb = flare.rgb * (2.f / 3);
		pd->m_rotation = 1.2f;

	}
}

void MagicFlareContainer::removeFlare(MagicFlare& flare) {

	if(flare.io && ValidIOAddress(flare.io)) {
		flare.io->flarecount--;
	}

	lightHandleDestroy(flare.dynlight);

	flare.tolive = 0;
	flare.exist = 0;
	g_magicFlaresCount--;

}

long MagicFlareContainer::flaggedCount() {
	if(!g_magicFlaresCount)
		return 0;

	long count = 0;
	for(size_t i = 0; i < g_magicFlaresMax; i++) {
		if(g_magicFlares[i].exist && g_magicFlares[i].flags == 0) {
			count++;
		}
	}

	return count;
}

void MagicFlareContainer::removeFlareEntityPtr(const Entity* entity) {
	
	for(size_t i = 0; i < g_magicFlaresMax; i++) {
		if(g_magicFlares[i].exist && g_magicFlares[i].io == entity) {
			g_magicFlares[i].io = nullptr;
		}
	}
}

void MagicFlareContainer::init() {
	g_magicFlaresCount = 0;
	for(size_t i = 0; i < g_magicFlaresMax; i++) {
		g_magicFlares[i].exist = 0;
	}
}

void MagicFlareContainer::removeAll() {
	
	for(size_t i = 0; i < g_magicFlaresMax; i++) {
		MagicFlare& flare = g_magicFlares[i];
		if(flare.exist) {
			g_magicFlares.removeFlare(flare);
		}
	}

	g_magicFlaresCount = 0;
}

void MagicFlareContainer::update() {

	if(!g_magicFlaresCount)
		return;

	g_magicFlares.shinum++;
	if(g_magicFlares.shinum >= 10) {
		g_magicFlares.shinum = 1;
	}

	PlatformDuration diff = g_platformTime.lastFrameDuration();

	bool key = !GInput->actionPressed(CONTROLS_CUST_MAGICMODE);

	RenderMaterial mat;
	mat.setBlendType(RenderMaterial::Additive);

	EERIE_LIGHT* light = lightHandleGet(torchLightHandle);

	for(long j = 1; j < 5; j++) {

		TextureContainer* surf;
		switch(j) {
			case 2:  surf = g_magicFlareTextures.lumignon; break;
			case 3:  surf = g_magicFlareTextures.lumignon2; break;
			case 4:  surf = g_magicFlareTextures.plasm; break;
			default: surf = g_magicFlareTextures.shine[g_magicFlares.shinum]; break;
		}

		mat.setTexture(surf);

		for(size_t i = 0; i < g_magicFlaresMax; i++) {

			MagicFlare& flare = g_magicFlares[i];

			if(!flare.exist || flare.type != j) {
				continue;
			}

			flare.tolive -= diff * 2;
			if(flare.flags & 1) {
				flare.tolive -= diff * 4;
			} else if(key) {
				flare.tolive -= diff * 6;
			}

			float z = flare.tolive / 4s;
			float size;
			if(flare.type == 1) {
				size = flare.size * 2 * z;
			} else if(flare.type == 4) {
				size = flare.size * 2.f * z * (4.0f / 3.0f);
			} else {
				size = flare.size;
			}

			if(flare.tolive <= 0 || flare.pos.y < -64.f || size < 3.f) {
				g_magicFlares.removeFlare(flare);
				continue;
			}

			if(flare.type == 1 && z < 0.6f) {
				z = 0.6f;
			}

			Color3f color = flare.rgb * z;

			light->rgb = componentwise_max(light->rgb, color);

			EERIE_LIGHT* el = lightHandleGet(flare.dynlight);
			if(el) {
				el->pos = flare.p;
				el->rgb = color;
			}

			mat.setDepthTest(flare.io != nullptr);

			if(flare.bDrawBitmap) {
				Vec3f pos = Vec3f(flare.p.x - size / 2.0f, flare.p.y - size / 2.0f, flare.p.z);
				EERIEAddBitmap(mat, pos, size, size, surf, Color(color));
			} else {
				EERIEAddSprite(mat, flare.p, size * 0.025f + 1.f, Color(color), 2.f);
			}

		}
	}

	light->rgb = componentwise_min(light->rgb, Color3f::white);
}

void MagicFlareContainer::changeColor() {

	m_currentColor++;

	if(m_currentColor > 2)
		m_currentColor = 0;
}

void MagicFlareLoadTextures() {
	
	TextureContainer::TCFlags flags = TextureContainer::NoColorKey;
	
	g_magicFlareTextures.lumignon = TextureContainer::LoadUI("graph/particles/lumignon", flags);
	g_magicFlareTextures.lumignon2 = TextureContainer::LoadUI("graph/particles/lumignon2", flags);
	g_magicFlareTextures.plasm = TextureContainer::LoadUI("graph/particles/plasm", flags);
	
	std::ostringstream oss;
	for(size_t i = 1; i < 10; i++) {
		oss.str(std::string());
		oss << "graph/particles/shine" << i;
		g_magicFlareTextures.shine[i] = TextureContainer::LoadUI(oss.str(), flags);
	}
	
}

void MagicFlareReleaseEntity(const Entity * entity) {
	
	g_magicFlares.removeFlareEntityPtr(entity);
}

long MagicFlareCountNonFlagged() {
	
	return g_magicFlares.flaggedCount();
}

void ARX_MAGICAL_FLARES_FirstInit() {
	
	g_magicFlares.init();
}

void ARX_MAGICAL_FLARES_KillAll() {
	
	g_magicFlares.removeAll();
}

void MagicFlareChangeColor() {
	
	g_magicFlares.changeColor();
}

void AddFlare(const Vec2f & pos, float sm, short typ, Entity * io, bool bookDraw) {
	
	g_magicFlares.addFlare(pos, sm, typ, io, bookDraw);
}

//! Helper for FlareLine
static void AddLFlare(const Vec2f & pos, Entity * io) {
	AddFlare(pos, 0.45f, 1, io);
}

void FlareLine(Vec2f tmpPos0, Vec2f tmpPos1, Entity * io) {
	
	static const int FLARELINESTEP = 7;
	static const int FLARELINERND = 6;
	
	Vec2f d = tmpPos1 - tmpPos0;
	Vec2f ad = glm::abs(d);
	
	if(ad.x > ad.y) {
		
		if(tmpPos0.x > tmpPos1.x) {
			std::swap(tmpPos0, tmpPos1);
		}
		
		float m = d.y / d.x;
		float i = tmpPos0.x;
		
		while(i < tmpPos1.x) {
			long z = Random::get(0, FLARELINERND);
			z += FLARELINESTEP;
			if(!io) {
				z = long(z * g_sizeRatio.y);
			}
			i += z;
			tmpPos0.y += m * z;
			AddLFlare(Vec2f(i, tmpPos0.y), io);
		}
		
	} else {
		
		if(tmpPos0.y > tmpPos1.y) {
			std::swap(tmpPos0, tmpPos1);
		}
		
		float m = d.x / d.y;
		float i = tmpPos0.y;
		
		while(i < tmpPos1.y) {
			long z = Random::get(0, FLARELINERND);
			z += FLARELINESTEP;
			if(!io) {
				z = long(z * g_sizeRatio.y);
			}
			i += z;
			tmpPos0.x += m * z;
			AddLFlare(Vec2f(tmpPos0.x, i), io);
		}
		
	}
}


void ARX_MAGICAL_FLARES_Update() {

	g_magicFlares.update();
}
