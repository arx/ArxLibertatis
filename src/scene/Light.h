/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//       Didier Pedreno (update Light Model version 1)
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved

#ifndef ARX_SCENE_LIGHT_H
#define ARX_SCENE_LIGHT_H

#include <stddef.h>

#include "core/TimeTypes.h"
#include "audio/AudioTypes.h"
#include "graphics/BaseGraphicsTypes.h"
#include "graphics/Color.h"
#include "math/Angle.h"
#include "math/Types.h"
#include "math/Quantizer.h"
#include "util/Flags.h"
#include "util/HandleType.h"

struct EERIE_LIGHT;
struct EERIEPOLY;
struct SMY_VERTEX;
class Entity;

const size_t g_staticLightsMax = 1200;
const size_t g_dynamicLightsMax = 500;

extern EERIE_LIGHT * g_culledDynamicLights[g_dynamicLightsMax];
extern EERIE_LIGHT * g_staticLights[g_staticLightsMax];
extern EERIE_LIGHT g_dynamicLights[g_dynamicLightsMax];
extern size_t g_culledDynamicLightsCount;

void culledStaticLightsReset();

typedef HandleType<struct LightHandleTag, long, -1> LightHandle;

enum EERIE_TYPES_EXTRAS_MODE
{
	EXTRAS_SEMIDYNAMIC       = 0x00000001,
	EXTRAS_EXTINGUISHABLE    = 0x00000002,
	EXTRAS_STARTEXTINGUISHED = 0x00000004,
	EXTRAS_SPAWNFIRE         = 0x00000008,
	EXTRAS_SPAWNSMOKE        = 0x00000010,
	EXTRAS_OFF               = 0x00000020,
	EXTRAS_COLORLEGACY       = 0x00000040,
	EXTRAS_NOCASTED          = 0x00000080,
	EXTRAS_FIXFLARESIZE      = 0x00000100,
	EXTRAS_FIREPLACE         = 0x00000200,
	EXTRAS_NO_IGNIT          = 0x00000400,
	EXTRAS_FLARE             = 0x00000800
};
DECLARE_FLAGS(EERIE_TYPES_EXTRAS_MODE, ExtrasType)
DECLARE_FLAGS_OPERATORS(ExtrasType)

struct EERIE_LIGHT {
	
	bool m_exists;
	bool m_isIgnitionLight; // TODO refactor special case
	bool m_isVisible;
	ExtrasType extras;
	bool m_ignitionStatus; // on/off
	Vec3f pos;
	float fallstart;
	float fallend;
	float falldiffmul;
	Color3f rgb255;
	float intensity;
	Color3f rgb;
	
	Rectf m_screenRect;
	
	float m_flareFader;
	Color3f ex_flicker;
	float ex_radius;
	float ex_frequency;
	float ex_size;
	float ex_speed;
	float ex_flaresize;
	LightHandle m_ignitionLightHandle;
	GameInstant creationTime;
	
	// will start to fade before the end of duration...
	GameDuration duration;
	
	audio::SourcedSample sample;
	math::Quantizer m_storedFlameTime;
	
	EERIE_LIGHT()
		: m_exists(false)
		, m_isIgnitionLight(false)
		, m_isVisible(false)
		, extras(0)
		, m_ignitionStatus(false)
		, pos(0)
		, fallstart(0.f)
		, fallend(0.f)
		, falldiffmul(0.f)
		, rgb255(Color3f::black)
		, intensity(0.f)
		, rgb(Color3f::black)
		, m_screenRect(Rectf::ZERO)
		, m_flareFader(0.f)
		, ex_flicker(Color3f::black)
		, ex_radius(0.f)
		, ex_frequency(0.f)
		, ex_size(0.f)
		, ex_speed(0.f)
		, ex_flaresize(0.f)
	{ }
	
};

struct ColorMod {

	void updateFromEntity(Entity * io, bool inBook = false);

	Color3f ambientColor;
	Color3f factor;
	Color3f term;
};

void RecalcLight(EERIE_LIGHT * el);

void EERIE_LIGHT_GlobalInit();
long EERIE_LIGHT_Create();
void PrecalcIOLighting(const Vec3f & pos, float radius);

const LightHandle torchLightHandle = LightHandle(0);

EERIE_LIGHT * lightHandleGet(LightHandle lightHandle);

LightHandle GetFreeDynLight();

EERIE_LIGHT * dynLightCreate(LightHandle & handle);
EERIE_LIGHT * dynLightCreate();

void lightHandleDestroy(LightHandle & handle);
void endLightDelayed(LightHandle & handle, GameDuration delay);

void resetDynLights();

void ClearDynLights();
void PrecalcDynamicLighting(const Vec3f & camPos, float camDepth);


struct ShaderLight {
	
	Vec3f pos;
	float fallstart;
	float fallend;
	float falldiffmul;
	float intensity;
	Color3f rgb;
	Color3f rgb255;
	
	ShaderLight()
		: pos(0.f)
		, fallstart(0.f)
		, fallend(0.f)
		, falldiffmul(0.f)
		, intensity(0.f)
		, rgb(Color3f::black)
		, rgb255(Color3f::black)
	{ }
	
};

static const size_t llightsSize = 16;

void setMaxLLights(size_t count);
void UpdateLlights(ShaderLight lights[], size_t & lightsCount, Vec3f pos, bool forPlayerColor);

void InitTileLights();
void ComputeTileLights(short x, short z);
void ClearTileLights();

float GetColorz(const Vec3f & pos);

ColorRGBA ApplyLight(ShaderLight lights[], size_t lightsCount, const glm::quat & quat, const Vec3f & position,
                     const Vec3f & normal, const ColorMod & colorMod, float materialDiffuse = 1.f);

void ApplyTileLights(EERIEPOLY * ep, const Vec2s & pos);

void TreatBackgroundDynlights();

#endif // ARX_SCENE_LIGHT_H
