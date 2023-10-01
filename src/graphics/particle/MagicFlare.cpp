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
	void init2DPos(const Vec2f& pos);
	void init3DPos(const Vec2f& pos, bool bookDraw);
	TextureContainer* getTexContainerByType(char type);
private:
	short m_currentShineTex = 1;

	void cycleShineTexture();
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

	cycleShineTexture();
}

void MagicFlare::clear() {

	if(io && ValidIOAddress(io)) {
		io->flarecount--;
	}

	lightHandleDestroy(dynlight);

	tolive = 0;
	isValid = false;
	io = nullptr;
	hasIO = false;
	type = -1;
	m_currentShineTex = 1;
}

void MagicFlare::init2DPos(const Vec2f& pos) {

	pos2D.x = pos.x - Random::getf(0.f, 4.f);
	pos2D.y = pos.y - Random::getf(0.f, 4.f);
}

void MagicFlare::init3DPos(const Vec2f& pos, bool bookDraw) {

	if(!bookDraw) {
		if(io) {
			float vx = -(pos2D.x - g_size.center().x) * (5.f / 23.f);
			float vy = (pos2D.y - g_size.center().y) * (5.f / 33.f);
			pos3D = io->pos;
			pos3D += angleToVectorXZ(io->angle.getYaw() + vx) * 100.f;
			pos3D.y += std::sin(glm::radians(MAKEANGLE(io->angle.getPitch() + vy))) * 100.f - 150.f;
		} else {
			pos3D = screenToWorldSpace(pos, 75.f);
		}
	} else {
		pos3D = Vec3f(pos2D.x, pos2D.y, 0.001f);
	}
}

class MagicFlareContainer {
public:
	class iterator {
		using iterator_category = std::forward_iterator_tag;
	public:
		iterator(MagicFlare* ptr) : m_ptr(ptr) {}
		iterator operator++() { ++m_ptr; return *this; } // prefix increment
		iterator operator++(int a) { a++;  iterator tmp = *this; ++(*this); return tmp; } // postfix increment
		MagicFlare* operator->() { return m_ptr; }
		MagicFlare& operator*() const { return *m_ptr; }

		friend bool operator== (const iterator& a, const iterator& b) { return a.m_ptr == b.m_ptr; }
		friend bool operator!= (const iterator& a, const iterator& b) { return a.m_ptr != b.m_ptr; }
	private:
		MagicFlare* m_ptr;
	};
	size_t validFlareCount();
	MagicFlare& newFlare();

	iterator begin() { return iterator(&m_flares[0]); }
	iterator end() { return iterator(&m_flares[m_magicFlaresMax]); }
private:
	static const size_t m_magicFlaresMax = 500;
	MagicFlare m_flares[m_magicFlaresMax];

	size_t findUsableIndex();
};

class MagicFlareHandler {
public:
	MagicFlareHandler();
	void addFlare(const Vec2f& pos, float sm, bool useVariedFlares, Entity* io, bool bookDraw);
	long countWithoutIO();
	void removeEntityPtrFromFlares(const Entity* entity);
	void init();
	void removeAll();
	void update();
	void changeColor();
private:
	MagicFlareContainer m_flares;
	short m_currentColor;

	Color3f newFlareColor();
	void loadTextures();
	void createParticleDefs(const MagicFlare& flare, Entity* io, bool bookDraw);
};

static MagicFlareHandler g_magicFlares;

MagicFlareHandler::MagicFlareHandler()
	: m_currentColor(0)
{}

size_t MagicFlareContainer::validFlareCount() {

	size_t count = 0;
	for(auto& flare : m_flares) {
		if(flare.isValid) {
			count++;
		}
	}
	return count;
}

MagicFlare& MagicFlareContainer::newFlare() {

	size_t index = findUsableIndex();
	return m_flares[index];
}

size_t MagicFlareContainer::findUsableIndex() {
	
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
		m_flares[oldest].clear();
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
	
	MagicFlare& flare = m_flares.newFlare();
	flare.isValid = true;

	flare.bDrawBitmap = bookDraw;

	flare.io = io;
	if(io) {
		flare.hasIO = true;
		io->flarecount++;
	} else {
		flare.hasIO = false;
	}

	flare.init2DPos(pos);
	flare.init3DPos(pos, bookDraw);

	flare.color = newFlareColor();

	if(useVariedFlares) {
		flare.type = 3;
		flare.size = Random::getf(16.f, 68.f);
		flare.tolive = Random::get(1600000us, 3200000us);
	} else {
		flare.type = (Random::getf() > 0.8f) ? 1 : 4;
		flare.size = Random::getf(64.f, 102.f) * sm;
		flare.tolive = Random::get(3400000us, 4400000us);
	}
	flare.currentSize = flare.size;
	flare.dynlight = {};

	createParticleDefs(flare, io, bookDraw);
}

long MagicFlareHandler::countWithoutIO() {
	if(!m_flares.validFlareCount())
		return 0;

	long count = 0;
	for(auto &flare : m_flares) {
		if(flare.isValid && !flare.io) {
			count++;
		}
	}

	return count;
}

void MagicFlareHandler::removeEntityPtrFromFlares(const Entity* entity) {
	
	for(auto& flare : m_flares) {
		if(flare.isValid && flare.io == entity) {
			flare.io = nullptr;
		}
	}
}

void MagicFlareHandler::init() {
	
	for(auto &flare : m_flares) {
		flare.isValid = false;
	}

	loadTextures();
}

void MagicFlareHandler::removeAll() {
	
	for(auto& flare : m_flares) {
		flare.clear();
	}
}

TextureContainer* MagicFlare::getTexContainerByType(char type) {
	
	TextureContainer* tc = nullptr;
	switch(type) {
		case 2:  tc = g_magicFlareTextures.lumignon; break;
		case 3:  tc = g_magicFlareTextures.lumignon2; break;
		case 4:  tc = g_magicFlareTextures.plasm; break;
		default: tc = g_magicFlareTextures.shine[m_currentShineTex]; break;
	}
	return tc;
}

void MagicFlare::cycleShineTexture() {
	m_currentShineTex++;
	if(m_currentShineTex >= 10) {
		m_currentShineTex = 1;
	}
}

void MagicFlareHandler::update() {

	if(!m_flares.validFlareCount())
		return;

	bool isMagicCastKeyPressed = GInput->actionPressed(CONTROLS_CUST_MAGICMODE);

	RenderMaterial mat;
	mat.setBlendType(RenderMaterial::Additive);

	EERIE_LIGHT* light = lightHandleGet(torchLightHandle);

	for(auto& flare : m_flares) {

		if(!flare.isValid) {
			continue;
		}

		TextureContainer* surf = flare.getTexContainerByType(flare.type);

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
