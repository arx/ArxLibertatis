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

#include "graphics/particle/MagicFlare.h"

#include <cstdio>

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
#include "graphics/Draw.h"
#include "graphics/Renderer.h"
#include "graphics/data/TextureContainer.h"

long			flarenum=0;

struct FLARES {
	unsigned char exist;
	char type;
	short flags;
	TexturedVertex v;
	TexturedVertex tv;
	Vec2f pos;
	float tolive;
	Color3f rgb;
	float size;
	LightHandle dynlight;
	Entity * io;
	bool bDrawBitmap;
};

static const size_t MAX_FLARES = 500;
FLARES magicFlares[MAX_FLARES];

struct FLARETC
{
	TextureContainer * lumignon;
	TextureContainer * lumignon2;
	TextureContainer * plasm;
	TextureContainer * shine[11];
};

FLARETC flaretc;

void MagicFlareLoadTextures() {

	flaretc.lumignon=	TextureContainer::LoadUI("graph/particles/lumignon");
	flaretc.lumignon2=	TextureContainer::LoadUI("graph/particles/lumignon2");
	flaretc.plasm=		TextureContainer::LoadUI("graph/particles/plasm");

	char temp[256];

	for(long i = 1; i < 10; i++) {
		sprintf(temp,"graph/particles/shine%ld", i);
		flaretc.shine[i]=TextureContainer::LoadUI(temp);
	}
}

EERIE_CAMERA * Kam;

static short shinum = 1;

void MagicFlareSetCamera(EERIE_CAMERA * camera) {
	Kam = camera;
}

void MagicFlareReleaseEntity(Entity * io) {
	for(size_t i = 0; i < MAX_FLARES; i++) {
		if(magicFlares[i].exist && magicFlares[i].io == io)
			magicFlares[i].io = NULL;
	}
}

long MagicFlareCountNonFlagged() {
	
	if(!flarenum)
		return 0;
	
	long count = 0;
	for(size_t i = 0; i < MAX_FLARES; i++) {
		if(magicFlares[i].exist && magicFlares[i].flags == 0) {
			count++;
		}
	}

	return count;
}

void ARX_MAGICAL_FLARES_FirstInit() {
	flarenum = 0;
	for(size_t i = 0; i < MAX_FLARES; i++) {
		magicFlares[i].exist = 0;
	}
}

static void removeFlare(FLARES & flare) {
	
	if(flare.io && ValidIOAddress(flare.io)) {
		flare.io->flarecount--;
	}

	lightHandleDestroy(flare.dynlight);
	
	flare.tolive = 0;
	flare.exist = 0;
	flarenum--;
	
}

void ARX_MAGICAL_FLARES_KillAll() {
	
	for(size_t i = 0; i < MAX_FLARES; i++) {
		FLARES & flare = magicFlares[i];
		if(flare.exist) {
			removeFlare(flare);
		}
	}
	
	flarenum=0;
}

short PIPOrgb = 0;

void MagicFlareChangeColor() {
	PIPOrgb++;

	if(PIPOrgb > 2)
		PIPOrgb = 0;
}

void AddFlare(const Vec2s & pos, float sm, short typ, Entity * io, bool bookDraw) {
	
	int oldest = 0;
	size_t i;
	for(i = 0; i < MAX_FLARES; i++) {
		if(!magicFlares[i].exist) {
			break;
		}
		if(magicFlares[i].tolive < magicFlares[oldest].tolive) {
			oldest = i;
		}
	}
	if(i >= MAX_FLARES) {
		removeFlare(magicFlares[oldest]);
		i = oldest;
	}

	FLARES & flare = magicFlares[i];
	flare.exist = 1;
	flarenum++;

	if(!bookDraw)
		flare.bDrawBitmap = 0;
	else
		flare.bDrawBitmap = 1;

	flare.io = io;
	if(io) {
		flare.flags = 1;
		io->flarecount++;
	} else {
		flare.flags = 0;
	}

	flare.pos.x = float(pos.x) - Random::getf(0.f, 4.f);
	flare.pos.y = float(pos.y) - Random::getf(0.f, 4.f) - 50.f;
	flare.tv.rhw = flare.v.rhw = 1.f;

	if(!bookDraw) {
		EERIE_CAMERA ka = *Kam;
		ka.angle = Anglef(360.f, 360.f, 360.f) - ka.angle;
		EERIE_CAMERA * oldcam = ACTIVECAM;
		SetActiveCamera(&ka);
		PrepareCamera(&ka, g_size);
		flare.v.p += ka.orgTrans.pos;
		EE_RTP(flare.tv.p, flare.v);
		flare.v.p += ka.orgTrans.pos;

		float vx = -(flare.pos.x - subj.center.x) * 0.2173913f;
		float vy = (flare.pos.y - subj.center.y) * 0.1515151515151515f;
		if(io) {
			flare.v.p = io->pos;
			flare.v.p += angleToVectorXZ(io->angle.getPitch() + vx) * 100.f;
			flare.v.p.y += std::sin(glm::radians(MAKEANGLE(io->angle.getYaw() + vy))) * 100.f - 150.f;
		} else {
			flare.v.p.x = 1.0f  * float(pos.x - (g_size.width()  / 2)) * 156.f / (640.f * g_sizeRatio.y);
			flare.v.p.y = 0.75f * float(pos.y - (g_size.height() / 2)) * 156.f / (480.f * g_sizeRatio.y);
			flare.v.p.z = 75.f;
			ka = *oldcam;
			SetActiveCamera(&ka);
			PrepareCamera(&ka, g_size);
			float temp = (flare.v.p.y * -ka.orgTrans.xsin) + (flare.v.p.z * ka.orgTrans.xcos);
			flare.v.p.y = (flare.v.p.y * ka.orgTrans.xcos) - (-flare.v.p.z * ka.orgTrans.xsin);
			flare.v.p.z = (temp * ka.orgTrans.ycos) - (-flare.v.p.x * ka.orgTrans.ysin);
			flare.v.p.x = (temp * -ka.orgTrans.ysin) + (flare.v.p.x * ka.orgTrans.ycos);
			flare.v.p += oldcam->orgTrans.pos;
		}
		flare.tv.p = flare.v.p;
		SetActiveCamera(oldcam);
		PrepareCamera(oldcam, g_size);
	} else {
		flare.tv.p = Vec3f(flare.pos.x, flare.pos.y, 0.001f);
	}

	switch(PIPOrgb) {
		case 0: {
			flare.rgb = Color3f(.4f, 0.f, .4f) + Color3f(2.f/3, 2.f/3, 2.f/3) * randomColor3f();
			break;
		}
		case 1: {
			flare.rgb = Color3f(.5f, .5f, 0.f) + Color3f(.625f, .625f, .55f) * randomColor3f();
			break;
		}
		case 2: {
			flare.rgb = Color3f(.4f, 0.f, 0.f) + Color3f(2.f/3, .55f, .55f) * randomColor3f();
			break;
		}
	}

	if(typ == -1) {
		float zz = eeMousePressed1() ? 0.29f : ((sm > 0.5f) ? Random::getf() : 1.f);
		if(zz < 0.2f) {
			flare.type = 2;
			flare.size = Random::getf(42.f, 84.f);
			flare.tolive = Random::getf(800.f, 1600.f) * FLARE_MUL;
		} else if(zz < 0.5f) {
			flare.type = 3;
			flare.size = Random::getf(16.f, 68.f);
			flare.tolive = Random::getf(800.f, 1600.f) * FLARE_MUL;
		} else {
			flare.type = 1;
			flare.size = Random::getf(32.f, 56.f) * sm;
			flare.tolive = Random::getf(1700.f, 2200.f) * FLARE_MUL;
		}
	} else {
		flare.type = (Random::getf() > 0.8f) ? 1 : 4;
		flare.size = Random::getf(64.f, 102.f) * sm;
		flare.tolive = Random::getf(1700.f, 2200.f) * FLARE_MUL;
	}

	flare.dynlight = LightHandle();

	for(long kk = 0; kk < 3; kk++) {

		if(Random::getf() < 0.5f) {
			continue;
		}

		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}

		if(!bookDraw) {
			pd->m_flags = FADE_IN_AND_OUT | ROTATING | DISSIPATING;
			if(!io) {
				pd->m_flags |= PARTICLE_NOZBUFFER;
			}
		} else {
			pd->m_flags = FADE_IN_AND_OUT;
		}

		pd->ov = flare.v.p + randomVec(-5.f, 5.f);
		pd->move = Vec3f(0.f, 5.f, 0.f);
		pd->scale = Vec3f(-2.f);
		pd->tolive = 1300 + kk * 100 + Random::get(0, 800);
		pd->tc = fire2;
		if(kk == 1) {
			pd->move.y = 4.f;
			pd->siz = 1.5f;
		} else {
			pd->siz = Random::getf(1.f, 2.f);
		}
		pd->rgb = Color3f(flare.rgb.r * (2.f/3), flare.rgb.g * (2.f/3), flare.rgb.b * (2.f/3));
		pd->m_rotation = 1.2f;

		if(bookDraw)
			pd->is2D = true;
	}
}

//! Helper for FlareLine
static void AddLFlare(const Vec2s & pos, Entity * io) {
	AddFlare(pos, 0.45f, 1, io);
}

static const int FLARELINESTEP = 7;
static const int FLARELINERND = 6;

void FlareLine(const Vec2s & pos0, const Vec2s & pos1, Entity * io)
{
	Vec2f tmpPos0 = Vec2f(pos0);
	Vec2f tmpPos1 = Vec2f(pos1);
	
	Vec2f d = tmpPos1 - tmpPos0;
	Vec2f ad = glm::abs(d);
	
	if(ad.x > ad.y) {
		
		if(tmpPos0.x > tmpPos1.x) {
			std::swap(tmpPos0, tmpPos1);
		}
		
		float m = d.y / d.x;
		long i = tmpPos0.x;
		
		while(i < tmpPos1.x) {
			long z = Random::get(0, FLARELINERND);
			z += FLARELINESTEP;
			if(!io) {
				z *= g_sizeRatio.y;
			}
			i += z;
			tmpPos0.y += m * z;
			AddLFlare(Vec2s(i, tmpPos0.y), io);
		}
		
	} else {
		
		if(tmpPos0.y > tmpPos1.y) {
			std::swap(tmpPos0, tmpPos1);
		}
		
		float m = d.x / d.y;
		long i = tmpPos0.y;
		
		while(i < tmpPos1.y) {
			long z = Random::get(0, FLARELINERND);
			z += FLARELINESTEP;
			if(!io) {
				z *= g_sizeRatio.y;
			}
			i += z;
			tmpPos0.x += m * z;
			AddLFlare(Vec2s(tmpPos0.x, i), io);
		}
		
	}
}

static unsigned long FRAMETICKS=0;

void ARX_MAGICAL_FLARES_Update() {

	if(!flarenum)
		return;

	shinum++;
	if(shinum >= 10) {
		shinum = 1;
	}
	
	const ArxInstant now = arxtime.now();
	const unsigned long TICKS = now - FRAMETICKS;
	FRAMETICKS = now;
	
	bool key = !GInput->actionPressed(CONTROLS_CUST_MAGICMODE);

	RenderMaterial mat;
	mat.setBlendType(RenderMaterial::Additive);
	
	EERIE_LIGHT * light = lightHandleGet(torchLightHandle);
	
	for(long j = 1; j < 5; j++) {

		TextureContainer * surf;
		switch(j) {
			case 2:  surf = flaretc.lumignon; break;
			case 3:  surf = flaretc.lumignon2; break;
			case 4:  surf = flaretc.plasm; break;
			default: surf = flaretc.shine[shinum]; break;
		}

		mat.setTexture(surf);

		for(size_t i = 0; i < MAX_FLARES; i++) {

			FLARES & flare = magicFlares[i];

			if(!flare.exist || flare.type != j) {
				continue;
			}

			flare.tolive -= float(TICKS * 2);
			if(flare.flags & 1) {
				flare.tolive -= float(TICKS * 4);
			} else if (key) {
				flare.tolive -= float(TICKS * 6);
			}

			float z = (flare.tolive * 0.00025f);
			float s;
			if(flare.type == 1) {
				s = flare.size * 2 * z;
			} else if(flare.type == 4) {
				s = flare.size * 2.f * z + 10.f;
			} else {
				s = flare.size;
			}

			if(flare.tolive <= 0.f || flare.pos.y < -64.f || s < 3.f) {
				removeFlare(flare);
				continue;
			}

			if(flare.type == 1 && z < 0.6f)  {
				z = 0.6f;
			}

			Color3f c = flare.rgb * z;
			flare.tv.color = c.toRGB();
			flare.v.p = flare.tv.p;

			light->rgb = componentwise_max(light->rgb, c);

			if(lightHandleIsValid(flare.dynlight)) {
				EERIE_LIGHT * el = lightHandleGet(flare.dynlight);
				el->pos = flare.v.p;
				el->rgb = c;
			}

			mat.setDepthTest(flare.io != NULL);
			
			if(flare.bDrawBitmap) {
				s *= 2.f * minSizeRatio();
				EERIEAddBitmap(mat, flare.v.p, s, s, surf, Color::fromRGBA(flare.tv.color));
			} else {
				EERIEAddSprite(mat, flare.v.p, s * 0.025f + 1.f,
				               Color::fromRGBA(flare.tv.color), 2.f);
			}

		}
	}

	light->rgb = componentwise_min(light->rgb, Color3f::white);
}
