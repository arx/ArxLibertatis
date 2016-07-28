/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/particle/Spark.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/Color.h"
#include "graphics/GraphicsModes.h"
#include "graphics/Math.h"
#include "graphics/RenderBatcher.h"
#include "platform/profiler/Profiler.h"

struct SparkParticle {
	u32 m_duration;
	Vec3f ov;
	Vec3f move;
	long timcreation;
	ColorRGBA rgb;
	float m_tailLength;
	
	SparkParticle()
		: m_duration(0)
		, ov(Vec3f_ZERO)
		, move(Vec3f_ZERO)
		, timcreation(0)
		, rgb(Color::black.toRGB())
		, m_tailLength(0.f)
	{ }
};

static const size_t g_sparkParticlesMax = 500;
static SparkParticle g_sparkParticles[g_sparkParticlesMax];
static long g_sparkParticlesCount = 0;

void ParticleSparkClear() {
	std::fill(g_sparkParticles, g_sparkParticles + g_sparkParticlesMax, SparkParticle());
	g_sparkParticlesCount = 0;
}

long ParticleSparkCount() {
	return g_sparkParticlesCount;
}

void ParticleSparkSpawn(const Vec3f & pos, unsigned int count, SpawnSparkType type) {
	
	if(arxtime.is_paused()) {
		return;
	}
	
	u32 len = glm::clamp(count / 3, 3u, 8u);
	
	for(unsigned int k = 0; k < count; k++) {
		
		int sparkSlot = -1;
		for(size_t i = 0; i < g_sparkParticlesMax; i++) {
			if(g_sparkParticles[i].m_duration == 0) {
				sparkSlot = int(i);
				break;
			}
		}
		
		if(sparkSlot == -1) {
			return;
		}
		
		SparkParticle & spark = g_sparkParticles[sparkSlot];
		
		g_sparkParticlesCount++;
		
		spark.timcreation = arxtime.now();
		spark.ov = pos + randomVec(-5.f, 5.f);
		spark.move = randomVec(-6.f, 6.f);
		spark.m_duration = len * 90 + count;
		
		switch(type) {
			case SpawnSparkType_Default:
				spark.rgb = Color3f(.3f, .3f, 0.f).toRGB();
				break;
			case SpawnSparkType_Failed:
				spark.rgb = Color3f(.2f, .2f, .1f).toRGB();
				break;
			case SpawnSparkType_Success:
				spark.rgb = Color3f(.45f, .1f, 0.f).toRGB();
				break;
		}
		
		spark.m_tailLength = len + Random::getf() * len;
	}
}

void ParticleSparkUpdate() {
	
	ARX_PROFILE_FUNC();
	
	if(g_sparkParticlesCount == 0) {
		return;
	}
	
	EERIE_CAMERA * cam = &subj;
	
	const ArxInstant now = arxtime.now();
	
	RenderMaterial sparkMaterial;
	sparkMaterial.setBlendType(RenderMaterial::Additive);
	
	for(size_t i = 0; i < g_sparkParticlesMax; i++) {

		SparkParticle & spark = g_sparkParticles[i];
		if(spark.m_duration == 0) {
			continue;
		}

		long framediff = spark.timcreation + spark.m_duration - now;
		long framediff2 = now - spark.timcreation;
		
		if(framediff2 < 0) {
			continue;
		}
		
		if(framediff <= 0) {
			spark.m_duration = 0;
			g_sparkParticlesCount--;
			continue;
		}
		
		float val = (spark.m_duration - framediff) * 0.01f;
		
		Vec3f in = spark.ov + spark.move * val;
		
		TexturedVertex out;
		EE_RTP(in, out);
		
		if(out.rhw < 0 || out.p.z > cam->cdepth * fZFogEnd) {
			continue;
		}
		
		Vec3f tailDirection = glm::normalize(-spark.move);
		
		TexturedVertex tv[3];
		tv[0].color = spark.rgb;
		tv[1].color = Color(102, 102, 102, 255).toRGBA();
		tv[2].color = Color(0, 0, 0, 255).toRGBA();
		tv[0].p = out.p;
		tv[0].rhw = out.rhw;
		
		Vec3f temp1 = in + Vec3f(Random::getf(0.f, 0.5f), 0.8f, Random::getf(0.f, 0.5f));
		Vec3f temp2 = in + tailDirection * spark.m_tailLength;
		
		EE_RTP(temp1, tv[1]);
		EE_RTP(temp2, tv[2]);
		
		RenderBatcher::getInstance().add(sparkMaterial, tv);
	}
}
