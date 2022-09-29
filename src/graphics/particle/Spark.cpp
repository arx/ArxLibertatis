/*
 * Copyright 2016-2021 Arx Libertatis Team (see the AUTHORS file)
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
#include <type_traits>
#include <vector>

#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Camera.h"
#include "graphics/GlobalFog.h"
#include "graphics/Math.h"
#include "graphics/RenderBatcher.h"
#include "math/RandomVector.h"
#include "platform/profiler/Profiler.h"
#include "util/Range.h"

struct alignas(16) SparkParticle {
	
	Vec3f pos;
	ShortGameDuration elapsed;
	Vec3f move;
	ShortGameDuration duration;
	ColorRGBA color;
	float tailLength;
	
	SparkParticle()
		: pos(0.f)
		, move(0.f)
		, color(Color::black.toRGB())
		, tailLength(0.f)
	{ }
	
};

static_assert(std::is_trivially_copyable_v<SparkParticle>);

static std::vector<SparkParticle> g_sparkParticles;

void ParticleSparkClear() {
	g_sparkParticles.clear();
}

size_t ParticleSparkCount() {
	return g_sparkParticles.size();
}

void ParticleSparkSpawnContinous(const Vec3f & pos, unsigned rate, ColorRGBA color) {
	
	float amount = float(rate) * (g_platformTime.lastFrameDuration() / (PlatformDuration(200ms) / 3));
	
	unsigned count = unsigned(amount);
	if(Random::getf() < (amount - float(count))) {
		count++;
	}
	
	ParticleSparkSpawn(pos, count, color);
}

void ParticleSparkSpawn(const Vec3f & pos, unsigned int count, ColorRGBA color) {
	
	if(g_gameTime.isPaused()) {
		return;
	}
	
	u32 len = glm::clamp(count / 3, 3u, 8u);
	
	for(unsigned int k = 0; k < count; k++) {
		
		SparkParticle & spark = g_sparkParticles.emplace_back();
		
		spark.pos = pos + arx::randomVec(-5.f, 5.f);
		spark.move = arx::randomVec(-6.f, 6.f);
		spark.duration = std::chrono::milliseconds(len * 90 + count);
		spark.color = color;
		spark.tailLength = len + Random::getf() * len;
		
	}
	
}

void ParticleSparkUpdate() {
	
	ARX_PROFILE_FUNC();
	
	if(g_sparkParticles.empty()) {
		return;
	}
	
	ShortGameDuration delta(g_gameTime.lastFrameDuration());
	arx_assume(delta <= ShortGameDuration::max() / 2);
	
	RenderMaterial sparkMaterial;
	sparkMaterial.setBlendType(RenderMaterial::Additive);
	
	for(SparkParticle & spark : g_sparkParticles) {
		
		arx_assume(spark.duration > 0 && spark.duration <= ShortGameDuration::max() / 2);
		arx_assume(spark.elapsed >= 0 && spark.elapsed <= spark.duration);
		
		float t = spark.elapsed / 100ms;
		spark.elapsed += delta;
		
		Vec3f in = spark.pos + spark.move * t;
		
		Vec3f tailDirection = glm::normalize(-spark.move);
		
		TexturedVertex tv[3];
		tv[0].color = spark.color;
		tv[1].color = Color::gray(0.4f).toRGBA();
		tv[2].color = Color::black.toRGBA();
		
		worldToClipSpace(in, tv[0]);
		
		if(tv[0].w < 0 || tv[0].p.z > g_camera->cdepth * fZFogEnd * tv[0].w) {
			continue;
		}
		
		Vec3f temp1 = in + Vec3f(Random::getf(0.f, 0.5f), 0.8f, Random::getf(0.f, 0.5f));
		Vec3f temp2 = in + tailDirection * spark.tailLength;
		
		worldToClipSpace(temp1, tv[1]);
		worldToClipSpace(temp2, tv[2]);
		
		g_renderBatcher.add(sparkMaterial, tv);
		
	}
	
	util::unordered_remove_if(g_sparkParticles, [](const SparkParticle & spark) {
		return spark.elapsed > spark.duration;
	});
	
}
