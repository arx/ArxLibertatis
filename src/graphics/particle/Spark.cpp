/*
 * Copyright 2016-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include <cmath>

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Camera.h"
#include "graphics/Color.h"
#include "graphics/GlobalFog.h"
#include "graphics/Math.h"
#include "graphics/RenderBatcher.h"
#include "math/RandomVector.h"
#include "platform/profiler/Profiler.h"

struct SparkParticle {
	u32 m_duration;
	Vec3f m_pos;
	Vec3f move;
	long timcreation;
	ColorRGBA rgb;
	float m_tailLength;
	
	SparkParticle()
		: m_duration(0)
		, m_pos(0.f)
		, move(0.f)
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

void ParticleSparkSpawnContinous(const Vec3f & pos, unsigned rate, SpawnSparkType type) {
	
	float amount = float(rate) * (g_platformTime.lastFrameDuration() / PlatformDurationMsf(66.666666f));
	
	unsigned count = unsigned(amount);
	if(Random::getf() < (amount - float(count))) {
		count++;
	}
	
	ParticleSparkSpawn(pos, count, type);
}

void ParticleSparkSpawn(const Vec3f & pos, unsigned int count, SpawnSparkType type) {
	
	if(g_gameTime.isPaused()) {
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
		
		spark.timcreation = toMsi(g_gameTime.now());
		spark.m_pos = pos + arx::randomVec(-5.f, 5.f);
		spark.move = arx::randomVec(-6.f, 6.f);
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
	
	const GameInstant now = g_gameTime.now();
	
	RenderMaterial sparkMaterial;
	sparkMaterial.setBlendType(RenderMaterial::Additive);
	
	for(size_t i = 0; i < g_sparkParticlesMax; i++) {

		SparkParticle & spark = g_sparkParticles[i];
		if(spark.m_duration == 0) {
			continue;
		}

		long framediff = spark.timcreation + spark.m_duration - toMsi(now);
		long framediff2 = toMsi(now) - spark.timcreation;
		
		if(framediff2 < 0) {
			continue;
		}
		
		if(framediff <= 0) {
			spark.m_duration = 0;
			g_sparkParticlesCount--;
			continue;
		}
		
		float val = (spark.m_duration - framediff) * 0.01f;
		
		Vec3f in = spark.m_pos + spark.move * val;
		
		Vec3f tailDirection = glm::normalize(-spark.move);
		
		TexturedVertex tv[3];
		tv[0].color = spark.rgb;
		tv[1].color = Color::gray(0.4f).toRGBA();
		tv[2].color = Color::black.toRGBA();
		
		worldToClipSpace(in, tv[0]);
		
		if(tv[0].w < 0 || tv[0].p.z > g_camera->cdepth * fZFogEnd * tv[0].w) {
			continue;
		}
		
		Vec3f temp1 = in + Vec3f(Random::getf(0.f, 0.5f), 0.8f, Random::getf(0.f, 0.5f));
		Vec3f temp2 = in + tailDirection * spark.m_tailLength;
		
		worldToClipSpace(temp1, tv[1]);
		worldToClipSpace(temp2, tv[2]);
		
		g_renderBatcher.add(sparkMaterial, tv);
	}
}
