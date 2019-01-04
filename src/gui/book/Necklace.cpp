/*
 * Copyright 2014-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "gui/book/Necklace.h"

#include "animation/AnimationRender.h"
#include "core/Application.h"
#include "core/Core.h"
#include "core/GameTime.h"
#include "game/Camera.h"
#include "game/EntityManager.h"
#include "game/Player.h"
#include "gui/Cursor.h"
#include "gui/Interface.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/Renderer.h"
#include "input/Input.h"
#include "scene/GameSound.h"
#include "scene/Light.h"
#include "scene/Object.h"

namespace gui {

ARX_NECKLACE necklace;

void NecklaceInit() {
	
	memset(&necklace, 0, sizeof(ARX_NECKLACE));
	necklace.lacet = loadObject("graph/interface/book/runes/lacet.teo");
	
	necklace.runes[RUNE_AAM] =         loadObject("graph/interface/book/runes/runes_aam.teo");
	necklace.runes[RUNE_CETRIUS] =     loadObject("graph/interface/book/runes/runes_citrius.teo");
	necklace.runes[RUNE_COMUNICATUM] = loadObject("graph/interface/book/runes/runes_comunicatum.teo");
	necklace.runes[RUNE_COSUM] =       loadObject("graph/interface/book/runes/runes_cosum.teo");
	necklace.runes[RUNE_FOLGORA] =     loadObject("graph/interface/book/runes/runes_folgora.teo");
	necklace.runes[RUNE_FRIDD] =       loadObject("graph/interface/book/runes/runes_fridd.teo");
	necklace.runes[RUNE_KAOM] =        loadObject("graph/interface/book/runes/runes_kaom.teo");
	necklace.runes[RUNE_MEGA] =        loadObject("graph/interface/book/runes/runes_mega.teo");
	necklace.runes[RUNE_MORTE] =       loadObject("graph/interface/book/runes/runes_morte.teo");
	necklace.runes[RUNE_MOVIS] =       loadObject("graph/interface/book/runes/runes_movis.teo");
	necklace.runes[RUNE_NHI] =         loadObject("graph/interface/book/runes/runes_nhi.teo");
	necklace.runes[RUNE_RHAA] =        loadObject("graph/interface/book/runes/runes_rhaa.teo");
	necklace.runes[RUNE_SPACIUM] =     loadObject("graph/interface/book/runes/runes_spacium.teo");
	necklace.runes[RUNE_STREGUM] =     loadObject("graph/interface/book/runes/runes_stregum.teo");
	necklace.runes[RUNE_TAAR] =        loadObject("graph/interface/book/runes/runes_taar.teo");
	necklace.runes[RUNE_TEMPUS] =      loadObject("graph/interface/book/runes/runes_tempus.teo");
	necklace.runes[RUNE_TERA] =        loadObject("graph/interface/book/runes/runes_tera.teo");
	necklace.runes[RUNE_VISTA] =       loadObject("graph/interface/book/runes/runes_vista.teo");
	necklace.runes[RUNE_VITAE] =       loadObject("graph/interface/book/runes/runes_vitae.teo");
	necklace.runes[RUNE_YOK] =         loadObject("graph/interface/book/runes/runes_yok.teo");
	
	necklace.pTexTab[RUNE_AAM]         = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_aam[icon]");
	necklace.pTexTab[RUNE_CETRIUS]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_cetrius[icon]");
	necklace.pTexTab[RUNE_COMUNICATUM] = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_comunicatum[icon]");
	necklace.pTexTab[RUNE_COSUM]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_cosum[icon]");
	necklace.pTexTab[RUNE_FOLGORA]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_folgora[icon]");
	necklace.pTexTab[RUNE_FRIDD]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_fridd[icon]");
	necklace.pTexTab[RUNE_KAOM]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_kaom[icon]");
	necklace.pTexTab[RUNE_MEGA]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_mega[icon]");
	necklace.pTexTab[RUNE_MORTE]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_morte[icon]");
	necklace.pTexTab[RUNE_MOVIS]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_movis[icon]");
	necklace.pTexTab[RUNE_NHI]         = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_nhi[icon]");
	necklace.pTexTab[RUNE_RHAA]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_rhaa[icon]");
	necklace.pTexTab[RUNE_SPACIUM]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_spacium[icon]");
	necklace.pTexTab[RUNE_STREGUM]     = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_stregum[icon]");
	necklace.pTexTab[RUNE_TAAR]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_taar[icon]");
	necklace.pTexTab[RUNE_TEMPUS]      = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_tempus[icon]");
	necklace.pTexTab[RUNE_TERA]        = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_tera[icon]");
	necklace.pTexTab[RUNE_VISTA]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_vista[icon]");
	necklace.pTexTab[RUNE_VITAE]       = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_vitae[icon]");
	necklace.pTexTab[RUNE_YOK]         = TextureContainer::LoadUI("graph/obj3d/interactive/items/magic/rune_aam/rune_yok[icon]");
	
	for(size_t i = 0; i < RUNE_COUNT; i++) {
		if(necklace.pTexTab[i]) {
			necklace.pTexTab[i]->getHalo();
		}
	}
}

void ReleaseNecklace() {
	
	delete necklace.lacet, necklace.lacet = NULL;
	
	for(long i = 0; i < RUNE_COUNT; i++) {
		delete necklace.runes[i], necklace.runes[i] = NULL;
		necklace.pTexTab[i] = NULL;
	}
}


static void PlayerBookDrawRune(Rune rune) {
	
	ARX_SPELLS_RequestSymbolDraw2(entities.player(), rune, ARX_SOUND_GetDuration(g_snd.SYMB[rune]));
	ARX_SOUND_PlayInterface(g_snd.SYMB[rune]);
}



void ARX_INTERFACE_ManageOpenedBook_Finish(const Vec2f & mousePos, Rectf rect, float scale)
{
	
	RenderState baseState = render3D().depthTest(false).fog(false);
	
	Vec3f pos = Vec3f(0.f, 0.f, 2100.f);
	
	EERIE_LIGHT * light = lightHandleGet(torchLightHandle);
	
	EERIE_LIGHT tl = *light;
	
	light->pos = Vec3f(500.f, -1960.f, 1590.f);
	light->m_exists = true;
	light->rgb = Color3f(0.6f, 0.7f, 0.9f);
	light->intensity = 1.8f;
	light->fallstart = 4520.f;
	light->fallend = light->fallstart + 600.f;
	RecalcLight(light);
	
	Camera * oldcam = g_camera;
	
	g_culledDynamicLights[0] = light;
	g_culledDynamicLightsCount = 1;
	
	Vec2i tmpPos(0);
	
	GRenderer->SetAntialiasing(true);
	
	float wave = timeWaveSin(g_platformTime.frameStart(), PlatformDurationMsf(1256.6370614f));
	float ptDelta = toMs(g_platformTime.lastFrameDuration());
	
	Camera bookcam;
	bookcam.angle = Anglef();
	bookcam.m_pos = Vec3f(0.f);
	bookcam.focal = 500.f;
	bookcam.cdepth = 2200.f;
	
	for(size_t i = 0; i < RUNE_COUNT; i++) {
		if(!gui::necklace.runes[i])
			continue;
		
		if(!player.hasRune(Rune(i))) {
			continue;
		}
		
		EERIE_3DOBJ * rune = gui::necklace.runes[i];
		
		Vec2i projectionCenter = Vec2i(rect.topLeft() + (Vec2f(285, 36) + Vec2f(tmpPos) * Vec2f(45, 64)) * scale);
		
		PrepareCamera(&bookcam, Rect(rect), projectionCenter);
		
			Anglef angle;
			if(rune->angle.getYaw() != 0.f) {
				if(rune->angle.getYaw() > 300.f) {
					rune->angle.setYaw(300.f);
				}
				angle.setYaw(wave * rune->angle.getYaw() * (1.0f / 40));
			}
			
			rune->angle.setYaw(rune->angle.getYaw() - ptDelta * 0.2f);
			
			if(rune->angle.getYaw() < 0.f)
				rune->angle.setYaw(0.f);
			
			// Now draw the rune
			TransformInfo t2(pos, glm::quat_cast(toRotationMatrix(angle)));
			DrawEERIEInter(rune, t2, NULL, false, 0.f);
			
			Rectf runeBox = UpdateBbox2d(*rune).toRect();
			
			PopAllTriangleListOpaque(baseState);
			
			tmpPos.x++;
			
			if(tmpPos.x > 4) {
				tmpPos.x = 0;
				tmpPos.y++;
			}
			
			// TODO this is a workaround for vertexClipPositions being relative to viewport
			Vec2f mousePosInViewport = mousePos - rect.topLeft();
			
			// Checks for Mouse floating over a rune...
			if(runeBox.contains(mousePosInViewport)) {
				bool r = false;
				
				for(size_t j = 0; j < rune->facelist.size(); j++) {
					float n = PtIn2DPolyProj(rune->vertexClipPositions, &rune->facelist[j], mousePosInViewport.x, mousePosInViewport.y);
					if(n != 0.f) {
						r = true;
						break;
					}
				}
				
				if(r) {
					
					TransformInfo t(pos, glm::quat_cast(toRotationMatrix(angle)));
					DrawEERIEInter(rune, t, NULL, false, 0.f);
					
					rune->angle.setYaw(rune->angle.getYaw() + ptDelta * 2.f);
					
					PopAllTriangleListOpaque(baseState.blendAdditive());
					
					cursorSetInteraction();
					
					if(eeMouseDown1()) {
						PlayerBookDrawRune(Rune(i));
					}
				}
			}
			
			TransformInfo t1(pos, quat_identity());
			DrawEERIEInter(gui::necklace.lacet, t1, NULL, false, 0.f);
			
			PopAllTriangleListOpaque(baseState);
	}
	
	*light = tl;
	
	PrepareCamera(oldcam, g_size);
	
	GRenderer->SetAntialiasing(false);
	
}

} // namespace gui
