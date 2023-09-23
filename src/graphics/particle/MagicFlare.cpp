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
	TextureContainer* shine[10];
};

static FLARETC g_magicFlareTextures;

class MagicFlare {
public:
	bool isValid;
	char type = -1;
	bool hasIO = false;
	Vec3f pos3D = Vec3f(0.f);
	Vec2f pos2D = Vec2f(0.f);
	PlatformDuration tolive;
	Color3f color;
	Color3f currentColor;
	float size = 0.f;
	float currentSize;
	LightHandle dynlight;
	Entity * io = nullptr;
	bool bDrawBitmap = false;

	void update(bool isMagicCastKeyPressed);
	void clear();
};

void MagicFlare::update(bool isMagicCastKeyPressed) {

	PlatformDuration lastFrameDuration = g_platformTime.lastFrameDuration();
	
	tolive -= lastFrameDuration * 2;
	if(hasIO) {
		tolive -= lastFrameDuration * 4;
	} else if(!isMagicCastKeyPressed) {
		tolive -= lastFrameDuration * 6;
	}

	float decayRate = tolive / 4s;
	switch(type) {
		case 1:
			currentSize = size * 2 * decayRate;
			break;
		case 4:
			currentSize = size * 2.f * decayRate * (4.0f / 3.0f);
			break;
		default:
			currentSize = size;
			break;
	}

	if(tolive <= 0 || pos2D.y < -64.f || currentSize < 3.f) {
		clear();
		return;
	}

	if(type == 1 && decayRate < 0.6f) {
		decayRate = 0.6f;
	}

	currentColor = color * decayRate;
}

void MagicFlare::clear() {

	if(io && ValidIOAddress(io)) {
		io->flarecount--;
	}

	lightHandleDestroy(dynlight);

	tolive = 0;
	isValid = false;
}

class MagicFlareHandler {
public:
	MagicFlareHandler();
	MagicFlare& operator[](size_t element);
	void removeFlare(MagicFlare& flare);
	void addFlare(const Vec2f& pos, float sm, bool useVariedFlares, Entity* io, bool bookDraw);
	long countWithoutIO();
	void removeEntityPtrFromFlares(const Entity* entity);
	void init();
	void removeAll();
	void update();
	void changeColor();

	short m_currentShineTex = 1;
private:
	static const size_t m_magicFlaresMax = 500;
	MagicFlare m_flares[m_magicFlaresMax];
	long m_flareCount;
	short m_currentColor;

	size_t findUsableIndex();
	Color3f newFlareColor();
	TextureContainer* getTexContainerByType(char type);
	void loadTextures();
	void cycleShineTexture();
	void createParticleDefs(const MagicFlare& flare, Entity* io, bool bookDraw);
};

static MagicFlareHandler g_magicFlares;

MagicFlareHandler::MagicFlareHandler()
	: m_flareCount(0)
	, m_currentColor(0)
{}

MagicFlare& MagicFlareHandler::operator[](size_t element) {
	
	return m_flares[element];
}

size_t MagicFlareHandler::findUsableIndex() {
	
	size_t oldest = 0;
	size_t i;
	for(i = 0; i < m_magicFlaresMax; i++) {
		if(!m_flares[i].isValid) {
			break;
		}
		if(m_flares[i].tolive < m_flares[oldest].tolive) {
			oldest = i;
		}
	}
	if(i >= m_magicFlaresMax) {
		removeFlare(m_flares[oldest]);
		i = oldest;
	}
	return i;
}

Color3f MagicFlareHandler::newFlareColor() {
	
	Color3f newColor;
	switch(m_currentColor) {
		case 0:
		{
			newColor = Color3f(0.4f, 0.f, 0.4f) + Color3f(2.f / 3, 2.f / 3, 2.f / 3) * randomColor3f();
			break;
		}
		case 1:
		{
			newColor = Color3f(0.5f, 0.5f, 0.f) + Color3f(0.625f, 0.625f, 0.55f) * randomColor3f();
			break;
		}
		case 2:
		{
			newColor = Color3f(0.4f, 0.f, 0.f) + Color3f(2.f / 3, 0.55f, 0.55f) * randomColor3f();
			break;
		}
		default: arx_unreachable();
	}
	return newColor;
}

void MagicFlareHandler::createParticleDefs(const MagicFlare& flare, Entity *io, bool bookDraw) {
	
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

		pd->ov = flare.pos3D + arx::randomVec(-5.f, 5.f);
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
		pd->rgb = flare.color * (2.f / 3);
		pd->m_rotation = 1.2f;

	}
}

void MagicFlareHandler::addFlare(const Vec2f& pos, float sm, bool useVariedFlares, Entity* io, bool bookDraw) {
	
	size_t index = findUsableIndex();
	MagicFlare& flare = m_flares[index];
	flare.isValid = true;
	m_flareCount++;

	flare.bDrawBitmap = bookDraw;

	flare.io = io;
	if(io) {
		flare.hasIO = true;
		io->flarecount++;
	} else {
		flare.hasIO = false;
	}

	flare.pos2D.x = pos.x - Random::getf(0.f, 4.f);
	flare.pos2D.y = pos.y - Random::getf(0.f, 4.f);

	if(!bookDraw) {
		if(io) {
			float vx = -(flare.pos2D.x - g_size.center().x) * (5.f/23.f);
			float vy = (flare.pos2D.y - g_size.center().y) * (5.f/33.f);
			flare.pos3D = io->pos;
			flare.pos3D += angleToVectorXZ(io->angle.getYaw() + vx) * 100.f;
			flare.pos3D.y += std::sin(glm::radians(MAKEANGLE(io->angle.getPitch() + vy))) * 100.f - 150.f;
		} else {
			flare.pos3D = screenToWorldSpace(pos, 75.f);
		}
	} else {
		flare.pos3D = Vec3f(flare.pos2D.x, flare.pos2D.y, 0.001f);
	}

	flare.color = newFlareColor();

	if(useVariedFlares) {
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
	flare.currentSize = flare.size;
	flare.dynlight = {};

	createParticleDefs(flare, io, bookDraw);
}

void MagicFlareHandler::removeFlare(MagicFlare& flare) {

	flare.clear();
	m_flareCount--;

}

long MagicFlareHandler::countWithoutIO() {
	if(!m_flareCount)
		return 0;

	long count = 0;
	for(size_t i = 0; i < m_magicFlaresMax; i++) {
		if(m_flares[i].isValid && !m_flares[i].io) {
			count++;
		}
	}

	return count;
}

void MagicFlareHandler::removeEntityPtrFromFlares(const Entity* entity) {
	
	for(size_t i = 0; i < m_magicFlaresMax; i++) {
		if(m_flares[i].isValid && m_flares[i].io == entity) {
			m_flares[i].io = nullptr;
		}
	}
}

void MagicFlareHandler::init() {
	m_flareCount = 0;
	for(size_t i = 0; i < m_magicFlaresMax; i++) {
		m_flares[i].isValid = false;
	}

	loadTextures();
}

void MagicFlareHandler::removeAll() {
	
	for(size_t i = 0; i < m_magicFlaresMax; i++) {
		MagicFlare& flare = m_flares[i];
		removeFlare(flare);
	}

	m_flareCount = 0;
}

TextureContainer* MagicFlareHandler::getTexContainerByType(char type) {
	
	TextureContainer* tc = nullptr;
	switch(type) {
		case 2:  tc = g_magicFlareTextures.lumignon; break;
		case 3:  tc = g_magicFlareTextures.lumignon2; break;
		case 4:  tc = g_magicFlareTextures.plasm; break;
		default: tc = g_magicFlareTextures.shine[m_currentShineTex]; break;
	}
	return tc;
}

void MagicFlareHandler::cycleShineTexture() {
	m_currentShineTex++;
	if(m_currentShineTex >= 10) {
		m_currentShineTex = 1;
	}
}

void MagicFlareHandler::update() {

	if(!m_flareCount)
		return;

	cycleShineTexture();

	bool isMagicCastKeyPressed = GInput->actionPressed(CONTROLS_CUST_MAGICMODE);

	RenderMaterial mat;
	mat.setBlendType(RenderMaterial::Additive);

	EERIE_LIGHT* light = lightHandleGet(torchLightHandle);

	for(size_t i = 0; i < m_magicFlaresMax; i++) {

		MagicFlare& flare = m_flares[i];

		if(!flare.isValid) {
			continue;
		}

		TextureContainer* surf = getTexContainerByType(flare.type);

		mat.setTexture(surf);

		flare.update(isMagicCastKeyPressed);

		light->rgb = componentwise_max(light->rgb, flare.currentColor);

		EERIE_LIGHT* el = lightHandleGet(flare.dynlight);
		if(el) {
			el->pos = flare.pos3D;
			el->rgb = flare.currentColor;
		}

		mat.setDepthTest(flare.io != nullptr);

		if(flare.bDrawBitmap) {
			Vec3f pos = Vec3f(flare.pos3D.x - flare.currentSize / 2.0f, flare.pos3D.y - flare.currentSize / 2.0f, flare.pos3D.z);
			EERIEAddBitmap(mat, pos, flare.currentSize, flare.currentSize, surf, Color(flare.currentColor));
		} else {
			EERIEAddSprite(mat, flare.pos3D, flare.currentSize * 0.025f + 1.f, Color(flare.currentColor), 2.f);
		}
	}

	light->rgb = componentwise_min(light->rgb, Color3f::white);
}

void MagicFlareHandler::changeColor() {

	m_currentColor++;

	if(m_currentColor > 2)
		m_currentColor = 0;
}

void MagicFlareHandler::loadTextures() {
	
	TextureContainer::TCFlags flags = TextureContainer::NoColorKey;
	
	g_magicFlareTextures.lumignon = TextureContainer::LoadUI("graph/particles/lumignon", flags);
	g_magicFlareTextures.lumignon2 = TextureContainer::LoadUI("graph/particles/lumignon2", flags);
	g_magicFlareTextures.plasm = TextureContainer::LoadUI("graph/particles/plasm", flags);
	
	std::ostringstream oss;
	for(size_t i = 0; i < 10; i++) {
		oss.str(std::string());
		oss << "graph/particles/shine" << i;
		g_magicFlareTextures.shine[i] = TextureContainer::LoadUI(oss.str(), flags);
	}
	
}

void MagicFlareReleaseEntity(const Entity * entity) {
	
	g_magicFlares.removeEntityPtrFromFlares(entity);
}

long MagicFlareCountWithoutEntity() {
	
	return g_magicFlares.countWithoutIO();
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

void AddFlare(const Vec2f & pos, float sm, bool useVariedFlares, Entity * io, bool bookDraw) {
	
	g_magicFlares.addFlare(pos, sm, useVariedFlares, io, bookDraw);
}

//! Helper for FlareLine
static void AddLFlare(const Vec2f & pos, Entity * io) {
	AddFlare(pos, 0.45f, false, io);
}

void FlareLine(Vec2f fromPos, Vec2f toPos, Entity * io) {
	
	static const int FLARELINESTEP = 7;
	static const int FLARELINERND = 6;
	
	Vec2f dist = toPos - fromPos;
	Vec2f absDist = glm::abs(dist);
	
	if(absDist.x > absDist.y) {
		
		if(fromPos.x > toPos.x) {
			std::swap(fromPos, toPos);
		}
		
		float m = dist.y / dist.x;
		float currentPos = fromPos.x;
		
		while(currentPos < toPos.x) {
			long step = Random::get(0, FLARELINERND);
			step += FLARELINESTEP;
			if(!io) {
				step = long(step * g_sizeRatio.y);
			}
			currentPos += step;
			fromPos.y += m * step;
			AddLFlare(Vec2f(currentPos, fromPos.y), io);
		}
		
	} else {
		
		if(fromPos.y > toPos.y) {
			std::swap(fromPos, toPos);
		}
		
		float m = dist.x / dist.y;
		float currentPos = fromPos.y;
		
		while(currentPos < toPos.y) {
			long step = Random::get(0, FLARELINERND);
			step += FLARELINESTEP;
			if(!io) {
				step = long(step * g_sizeRatio.y);
			}
			currentPos += step;
			fromPos.x += m * step;
			AddLFlare(Vec2f(fromPos.x, currentPos), io);
		}
		
	}
}


void ARX_MAGICAL_FLARES_Update() {

	g_magicFlares.update();
}
