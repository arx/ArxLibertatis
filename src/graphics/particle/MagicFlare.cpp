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
	float x;
	float y;
	float tolive;
	Color3f rgb;
	float size;
	LightHandle dynlight;
	long move;
	Entity * io;
	bool bDrawBitmap;
};

static const size_t MAX_FLARES = 300;
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

void ARX_MAGICAL_FLARES_KillAll()
{
	for(size_t i = 0; i < MAX_FLARES; i++) {
		FLARES & flare = magicFlares[i];

		if (flare.exist)
		{
			if (flare.io)
			{
				flare.io->flarecount--;
			}

			flare.exist=0;
			flare.tolive=0;
			flarenum--;

			lightHandleDestroy(flare.dynlight);
		}
	}

	flarenum=0;
}

void AddFlare(const Vec2s & pos, float sm, short typ, Entity * io, bool bookDraw) {

	size_t i;
	for(i = 0; i < MAX_FLARES; i++) {
		if(!magicFlares[i].exist) {
			break;
		}
	}
	if(i >= MAX_FLARES) {
		return;
	}

	FLARES * fl = &magicFlares[i];
	fl->exist = 1;
	flarenum++;

	if(!bookDraw)
		fl->bDrawBitmap = 0;
	else
		fl->bDrawBitmap = 1;

	fl->io = io;
	if(io) {
		fl->flags = 1;
		io->flarecount++;
	} else {
		fl->flags = 0;
	}

	fl->x = float(pos.x) - rnd() * 4.f;
	fl->y = float(pos.y) - rnd() * 4.f - 50.f;
	fl->tv.rhw = fl->v.rhw = 1.f;

	if(!bookDraw) {
		EERIE_CAMERA ka = *Kam;
		ka.angle = Anglef(360.f, 360.f, 360.f) - ka.angle;
		EERIE_CAMERA * oldcam = ACTIVECAM;
		SetActiveCamera(&ka);
		PrepareCamera(&ka, g_size);
		fl->v.p += ka.orgTrans.pos;
		EE_RTP(fl->tv.p, &fl->v);
		fl->v.p += ka.orgTrans.pos;

		float vx = -(fl->x - subj.center.x) * 0.2173913f;
		float vy = (fl->y - subj.center.y) * 0.1515151515151515f;
		if(io) {
			fl->v.p.x = io->pos.x - std::sin(radians(MAKEANGLE(io->angle.getPitch() + vx))) * 100.f;
			fl->v.p.y = io->pos.y + std::sin(radians(MAKEANGLE(io->angle.getYaw() + vy))) * 100.f - 150.f;
			fl->v.p.z = io->pos.z + std::cos(radians(MAKEANGLE(io->angle.getPitch() + vx))) * 100.f;
		} else {
			fl->v.p.x = float(pos.x - (g_size.width() / 2)) * 150.f / float(g_size.width());
			fl->v.p.y = float(pos.y - (g_size.height() / 2)) * 150.f / float(g_size.width());
			fl->v.p.z = 75.f;
			ka = *oldcam;
			SetActiveCamera(&ka);
			PrepareCamera(&ka, g_size);
			float temp = (fl->v.p.y * -ka.orgTrans.xsin) + (fl->v.p.z * ka.orgTrans.xcos);
			fl->v.p.y = (fl->v.p.y * ka.orgTrans.xcos) - (-fl->v.p.z * ka.orgTrans.xsin);
			fl->v.p.z = (temp * ka.orgTrans.ycos) - (-fl->v.p.x * ka.orgTrans.ysin);
			fl->v.p.x = (temp * -ka.orgTrans.ysin) + (fl->v.p.x * ka.orgTrans.ycos);
			fl->v.p += oldcam->orgTrans.pos;
		}
		fl->tv.p = fl->v.p;
		SetActiveCamera(oldcam);
		PrepareCamera(oldcam, g_size);
	} else {
		fl->tv.p = Vec3f(fl->x, fl->y, 0.001f);
	}

	switch(PIPOrgb) {
		case 0: {
			fl->rgb = Color3f(rnd() * (2.f/3) + .4f, rnd() * (2.f/3), rnd() * (2.f/3) + .4f);
			break;
		}
		case 1: {
			fl->rgb = Color3f(rnd() * .625f + .5f, rnd() * .625f + .5f, rnd() * .55f);
			break;
		}
		case 2: {
			fl->rgb = Color3f(rnd() * (2.f/3) + .4f, rnd() * .55f, rnd() * .55f);
			break;
		}
	}

	if(typ == -1) {
		float zz = (EERIEMouseButton & 1) ? 0.29f : ((sm > 0.5f) ? rnd() : 1.f);
		if(zz < 0.2f) {
			fl->type = 2;
			fl->size = rnd() * 42.f + 42.f;
			fl->tolive = (800.f + rnd() * 800.f) * FLARE_MUL;
		} else if(zz < 0.5f) {
			fl->type = 3;
			fl->size = rnd() * 52.f + 16.f;
			fl->tolive = (800.f + rnd() * 800.f) * FLARE_MUL;
		} else {
			fl->type = 1;
			fl->size = (rnd() * 24.f + 32.f) * sm;
			fl->tolive = (1700.f + rnd() * 500.f) * FLARE_MUL;
		}
	} else {
		fl->type = (rnd() > 0.8f) ? 1 : 4;
		fl->size = (rnd() * 38.f + 64.f) * sm;
		fl->tolive = (1700.f + rnd() * 500.f) * FLARE_MUL;
	}

	fl->dynlight = InvalidLightHandle;
	fl->move = OPIPOrgb;

	for(long kk = 0; kk < 3; kk++) {

		if(rnd() < 0.5f) {
			continue;
		}

		PARTICLE_DEF * pd = createParticle();
		if(!pd) {
			break;
		}

		if(!bookDraw) {
			pd->special = FADE_IN_AND_OUT | ROTATING | MODULATE_ROTATION | DISSIPATING;
			if(!io) {
				pd->special |= PARTICLE_NOZBUFFER;
			}
		} else {
			pd->special = FADE_IN_AND_OUT;
		}

		pd->ov = fl->v.p + randomVec(-5.f, 5.f);
		pd->move = Vec3f(0.f, 5.f, 0.f);
		pd->scale = Vec3f(-2.f);
		pd->tolive = 1300 + kk * 100 + Random::get(0, 800);
		pd->tc = fire2;
		if(kk == 1) {
			pd->move.y = 4.f;
			pd->siz = 1.5f;
		} else {
			pd->siz = 1.f + rnd();
		}
		pd->rgb = Color3f(fl->rgb.r * (2.f/3), fl->rgb.g * (2.f/3), fl->rgb.b * (2.f/3));
		pd->fparam = 1.2f;

		if(bookDraw)
			pd->is2D = true;
	}
}

//! Helper for FlareLine
void AddLFlare(const Vec2s & pos, Entity * io)
{
	AddFlare(pos, 0.45f, 1, io);
}

static const int FLARELINESTEP = 7;
static const int FLARELINERND = 6;

void FlareLine(const Vec2s & pos0, const Vec2s & pos1, Entity * io)
{
	float m;
	long i;
	long z;

	float x0 = pos0.x;
	float x1 = pos1.x;
	float y0 = pos0.y;
	float y1 = pos1.y;

	float dx = (x1 - x0);
	float adx = EEfabs(dx);
	float dy = (y1 - y0);
	float ady = EEfabs(dy);

	if(adx > ady) {
		if(x0 > x1) {
			z = x1;
			x1 = x0;
			x0 = z;
			z = y1;
			y0 = z;
		}

		if(x0 < x1) {
			m = dy / dx;
			i = x0;

			while(i < x1) {
				z = rnd() * FLARELINERND;
				z += FLARELINESTEP;
				i += z;
				y0 += m * z;
				AddLFlare(Vec2s(i, y0), io);
			}
		} else {
			m = dy / dx;
			i = x1;

			while(i < x0) {
				z = rnd() * FLARELINERND;
				z += FLARELINESTEP;
				i += z;
				y0 += m * z;
				AddLFlare(Vec2s(i, y0), io);
			}
		}
	} else {
		if(y0 > y1) {
			z = x1;
			x0 = z;
			z = y1;
			y1 = y0;
			y0 = z;
		}

		if(y0 < y1) {
			m = dx / dy;
			i = y0;

			while(i < y1) {
				z = rnd() * FLARELINERND;
				z += FLARELINESTEP;
				i += z;
				x0 += m * z;
				AddLFlare(Vec2s(x0, i), io);
			}
		} else {
			m = dx / dy;
			i = y1;

			while(i < y0) {
				z = rnd() * FLARELINERND;
				z += FLARELINESTEP;
				i += z;
				x0 += m * z;
				AddLFlare(Vec2s(x0, i), io);
			}
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

	long TICKS = long(arxtime) - FRAMETICKS;
	FRAMETICKS = (unsigned long)(arxtime);
	if(TICKS < 0) {
		return;
	}

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

			if(flare.tolive <= 0.f || flare.y < -64.f || s < 3.f) {

				if(flare.io && ValidIOAddress(flare.io)) {
					flare.io->flarecount--;
				}

				lightHandleDestroy(flare.dynlight);
				
				flare.exist = 0;
				flarenum--;

				continue;
			}

			if(flare.type == 1 && z < 0.6f)  {
				z = 0.6f;
			}

			Color3f c = flare.rgb * z;
			flare.tv.color = c.toBGR();
			flare.v.p = flare.tv.p;

			light->rgb = componentwise_max(light->rgb, c);

			if(lightHandleIsValid(flare.dynlight)) {
				EERIE_LIGHT * el = lightHandleGet(flare.dynlight);
				el->pos = flare.v.p;
				el->rgb = c;
			}

			mat.setDepthTest(flare.io != NULL);
			
			if(flare.bDrawBitmap) {
				s *= 2.f;
				EERIEAddBitmap(mat, flare.v.p.x, flare.v.p.y, s, s, flare.v.p.z,
								surf, Color::fromBGRA(flare.tv.color));
			} else {
				EERIEAddSprite(mat, flare.v.p, s * 0.025f + 1.f,
				               Color::fromBGRA(flare.tv.color), 2.f);
			}

		}
	}

	light->rgb = componentwise_min(light->rgb, Color3f::white);
}
