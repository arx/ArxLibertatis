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

#include "animation/AnimationRender.h"

#include <stddef.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include "animation/Animation.h"

#include "core/Application.h"
#include "core/GameTime.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/Equipment.h"
#include "game/EntityManager.h"
#include "game/NPC.h"
#include "game/Player.h"
#include "game/Spells.h"

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/Vertex.h"
#include "graphics/data/Mesh.h"
#include "graphics/data/MeshManipulation.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/particle/ParticleEffects.h"
#include "graphics/effects/Halo.h"

#include "math/Angle.h"
#include "math/Vector.h"

#include "physics/Collisions.h"

#include "platform/Platform.h"

#include "scene/Light.h"
#include "scene/GameSound.h"
#include "scene/Scene.h"
#include "scene/Interactive.h"

extern Color ulBKGColor;

// List of TO-TREAT vertex for MIPMESHING

// TODO: Convert to a RenderBatch & make TextureContainer constructor private
static TextureContainer TexSpecialColor("specialcolor_list", TextureContainer::NoInsert);

TexturedVertex * PushVertexInTable(TextureContainer *pTex, TextureContainer::TransparencyType type)
{
	if(pTex->count[type] + 3 > pTex->max[type]) {
		pTex->max[type] += 20 * 3;
		pTex->list[type] = (TexturedVertex *)realloc(pTex->list[type], pTex->max[type] * sizeof(TexturedVertex));

		if(!pTex->list[type]) {
			pTex->max[type] = 0;
			pTex->count[type] = 0;
			return NULL;
		}
	}

	pTex->count[type] += 3;
	return &pTex->list[type][pTex->count[type] - 3];
}

static void PopOneTriangleList(TextureContainer * _pTex, bool clear) {

	if(!_pTex->count[TextureContainer::Opaque]) {
		return;
	}

	GRenderer->SetTexture(0, _pTex);

	if(_pTex->userflags & POLY_LATE_MIP) {
		const float GLOBAL_NPC_MIPMAP_BIAS = -2.2f;
		GRenderer->GetTextureStage(0)->setMipMapLODBias(GLOBAL_NPC_MIPMAP_BIAS);
	}


	EERIEDRAWPRIM(Renderer::TriangleList, _pTex->list[TextureContainer::Opaque], _pTex->count[TextureContainer::Opaque]);
	
	if(clear) {
		_pTex->count[TextureContainer::Opaque] = 0;
	}

	if(_pTex->userflags & POLY_LATE_MIP) {
		float biasResetVal = 0;
		GRenderer->GetTextureStage(0)->setMipMapLODBias(biasResetVal);
	}

}

static void PopOneTriangleListTransparency(TextureContainer *_pTex) {

	if(!_pTex->count[TextureContainer::Blended]
	   && !_pTex->count[TextureContainer::Additive]
	   && !_pTex->count[TextureContainer::Subtractive]
	   && !_pTex->count[TextureContainer::Multiplicative]) {
		return;
	}

	GRenderer->SetTexture(0, _pTex);

	if(_pTex->count[TextureContainer::Blended]) {
		GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendSrcColor);
		if(_pTex->count[TextureContainer::Blended]) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->list[TextureContainer::Blended],
						  _pTex->count[TextureContainer::Blended]);
			_pTex->count[TextureContainer::Blended]=0;
		}
	}

	if(_pTex->count[TextureContainer::Additive]) {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		if(_pTex->count[TextureContainer::Additive]) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->list[TextureContainer::Additive],
						  _pTex->count[TextureContainer::Additive]);
			_pTex->count[TextureContainer::Additive]=0;
		}
	}

	if(_pTex->count[TextureContainer::Subtractive]) {
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
		if(_pTex->count[TextureContainer::Subtractive]) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->list[TextureContainer::Subtractive],
						  _pTex->count[TextureContainer::Subtractive]);
			_pTex->count[TextureContainer::Subtractive]=0;
		}
	}

	if(_pTex->count[TextureContainer::Multiplicative]) {
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
		if(_pTex->count[TextureContainer::Multiplicative]) {
			EERIEDRAWPRIM(Renderer::TriangleList, _pTex->list[TextureContainer::Multiplicative],
						  _pTex->count[TextureContainer::Multiplicative]);
			_pTex->count[TextureContainer::Multiplicative] = 0;
		}
	}
}

void PopAllTriangleList(bool clear) {
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);
	GRenderer->SetCulling(Renderer::CullNone);

	TextureContainer * pTex = GetTextureList();
	while(pTex) {
		PopOneTriangleList(pTex, clear);
		pTex = pTex->m_pNext;
	}
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
}

void PopAllTriangleListTransparency() {

	GRenderer->SetFogColor(Color::none);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);

	GRenderer->SetCulling(Renderer::CullNone);

	PopOneTriangleList(&TexSpecialColor, true);

	TextureContainer * pTex = GetTextureList();
	while(pTex) {
		PopOneTriangleListTransparency(pTex);
		pTex = pTex->m_pNext;
	}

	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
}





extern long BH_MODE;

extern bool EXTERNALVIEW;

float Cedric_GetInvisibility(Entity *io) {
	if(io) {
		float invisibility = io->invisibility;

		if (invisibility > 1.f)
			invisibility -= 1.f;

		if(io != entities.player() && invisibility > 0.f && !EXTERNALVIEW) {
			SpellBase * spell = spells.getSpellOnTarget(io->index(), SPELL_INVISIBILITY);

			if(spell) {
				if(player.m_skillFull.intuition > spell->m_level * 10) {
					invisibility -= (float)player.m_skillFull.intuition * (1.0f / 100)
									+ (float)spell->m_level * (1.0f / 10);

					invisibility = clamp(invisibility, 0.1f, 1.f);
				}
			}
		}
		return invisibility;
	} else {
		return 0.f;
	}
}

//TODO Move somewhere else
void Cedric_ApplyLightingFirstPartRefactor(Entity *io) {

	if(!io)
		return;

	io->special_color = Color3f::white;

	float poisonpercent = 0.f;
	float trappercent = 0.f;
	float secretpercent = 0.f;

	if((io->ioflags & IO_NPC) && io->_npcdata->poisonned > 0.f) {
		poisonpercent = io->_npcdata->poisonned * ( 1.0f / 20 );
		if(poisonpercent > 1.f)
			poisonpercent = 1.f;
	}

	if((io->ioflags & IO_ITEM) && io->poisonous > 0.f && io->poisonous_count) {
		poisonpercent = (float)io->poisonous * (1.0f / 20);
		if(poisonpercent > 1.f)
			poisonpercent = 1.f;
	}

	if((io->ioflags & IO_FIX) && io->_fixdata->trapvalue > -1) {
		trappercent = player.TRAP_DETECT - (float)io->_fixdata->trapvalue;
		if(trappercent > 0.f) {
			trappercent = 0.6f + trappercent * ( 1.0f / 100 );
			trappercent = clamp(trappercent, 0.6f, 1.f);
		}
	}

	if((io->ioflags & IO_FIX) && io->secretvalue > -1) {
		secretpercent = player.TRAP_SECRET - (float)io->secretvalue;
		if(secretpercent > 0.f) {
			secretpercent = 0.6f + secretpercent * ( 1.0f / 100 );
			secretpercent = clamp(secretpercent, 0.6f, 1.f);
		}
	}

	if(poisonpercent > 0.f) {
		io->special_color = Color3f::green;
	}

	if(trappercent > 0.f) {
		io->special_color = Color3f(trappercent, 1.f - trappercent, 1.f - trappercent);
	}

	if(secretpercent > 0.f) {
		io->special_color = Color3f(1.f - secretpercent, 1.f - secretpercent, secretpercent);
	}

	if(io->ioflags & IO_FREEZESCRIPT) {
		io->special_color = Color3f::blue;
	}

	if(io->sfx_flag & SFX_TYPE_YLSIDE_DEATH) {
		if(io->show == SHOW_FLAG_TELEPORTING) {
			float fTime = io->sfx_time + framedelay;
			io->sfx_time = checked_range_cast<unsigned long>(fTime);

			if (io->sfx_time >= (unsigned long)(arxtime))
				io->sfx_time = (unsigned long)(arxtime);
		} else {
			float elapsed = float(arxtime) - io->sfx_time;

			if(elapsed > 0.f) {
				if(elapsed < 3000.f) { // 5 seconds to red
					float ratio = elapsed * (1.0f / 3000);
					io->special_color = Color3f(1.f, 1.f - ratio, 1.f - ratio);
					AddRandomSmoke(io, 1);
				} else if(elapsed < 6000.f) { // 5 seconds to White
					float ratio = (elapsed - 3000.f) * (1.0f / 3000);
					io->special_color = Color3f(1.f, ratio, ratio);
					AddRandomSmoke(io, 2);
				} else { // SFX finish
					io->sfx_time = 0;

					if(io->ioflags & IO_NPC) {
						MakePlayerAppearsFX(io);
						AddRandomSmoke(io, 50);
						Color3f rgb = io->_npcdata->blood_color.to<float>();
						Sphere sp;
						sp.origin = io->pos;
						sp.radius = 200.f;
						long count = 6;

						while(count--) {
							Sphere splatSphere;
							splatSphere.origin = sp.origin;
							splatSphere.radius = rnd() * 30.f + 30.f;
							SpawnGroundSplat(splatSphere, rgb, 1);
							sp.origin.y -= rnd() * 150.f;

							ARX_PARTICLES_Spawn_Splat(sp.origin, 200.f, io->_npcdata->blood_color);

							sp.origin.x = io->pos.x + rnd() * 200.f - 100.f;
							sp.origin.y = io->pos.y + rnd() * 20.f - 10.f;
							sp.origin.z = io->pos.z + rnd() * 200.f - 100.f;
							sp.radius = rnd() * 100.f + 100.f;
						}

						LightHandle nn = GetFreeDynLight();
						if(lightHandleIsValid(nn)) {
							EERIE_LIGHT * light = lightHandleGet(nn);
							
							light->intensity = 0.7f + 2.f * rnd();
							light->fallend = 600.f;
							light->fallstart = 400.f;
							light->rgb = Color3f(1.0f, 0.8f, 0.f);
							light->pos = io->pos + Vec3f(0.f, -80.f, 0.f);
							light->duration = 600;
						}

						if(io->sfx_flag & SFX_TYPE_INCINERATE) {
							io->sfx_flag &= ~SFX_TYPE_INCINERATE;
							io->sfx_flag &= ~SFX_TYPE_YLSIDE_DEATH;
							SpellBase * spell = spells.getSpellOnTarget(io->index(), SPELL_INCINERATE);

							if(!spell)
								spell = spells.getSpellOnTarget(io->index(), SPELL_MASS_INCINERATE);

							if(spell) {
								spell->m_tolive = 0;
								float damages = 20 * spell->m_level;
								damages = ARX_SPELLS_ApplyFireProtection(io, damages);

								if (ValidIONum(spell->m_caster))
									ARX_DAMAGES_DamageNPC(io, damages, spell->m_caster, 1, &entities[spell->m_caster]->pos);
								else
									ARX_DAMAGES_DamageNPC(io, damages, spell->m_caster, 1, &io->pos);

								ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &io->pos);
							}
						} else {
							io->sfx_flag &= ~SFX_TYPE_YLSIDE_DEATH;
							ARX_INTERACTIVE_DestroyIOdelayed(io);
						}
					}
				}
			}
		}
	}
}

Vec3f angleToVecForCedricHalo(const Anglef & angle) {
	Vec3f cam_vector;
	cam_vector.x = -std::sin(radians(angle.getPitch())) * std::cos(radians(angle.getYaw()));
	cam_vector.y =  std::sin(radians(angle.getYaw()));
	cam_vector.z =  std::cos(radians(angle.getPitch())) * std::cos(radians(angle.getYaw()));
	
	return cam_vector;
}

void Cedric_PrepareHalo(EERIE_3DOBJ * eobj, Skeleton * obj) {
	Vec3f cam_vector = angleToVecForCedricHalo(ACTIVECAM->angle);
	
	// Apply light on all vertices
	for(size_t i = 0; i != obj->bones.size(); i++) {
		const Bone & bone = obj->bones[i];
		
		glm::quat qt1 = bone.anim.quat;
		Vec3f t_vector = glm::inverse(qt1) * cam_vector;

		// Get light value for each vertex
		for(size_t v = 0; v != bone.idxvertices.size(); v++) {
			long vertIndex = bone.idxvertices[v];
			const Vec3f & inVert = eobj->vertexlist[vertIndex].norm;

			// Get cos angle between light and vertex norm
			eobj->vertexlist3[vertIndex].norm.z =
			    (inVert.x * t_vector.x + inVert.y * t_vector.y + inVert.z * t_vector.z);

		}
	}
}

TexturedVertex * GetNewVertexList(TextureContainer * container, const EERIE_FACE & face, float invisibility, float & fTransp) {

	fTransp = 0.f;

	if((face.facetype & POLY_TRANS) || invisibility > 0.f) {
		if(invisibility > 0.f)
			fTransp = 2.f - invisibility;
		else
			fTransp = face.transval;

		if(fTransp >= 2.f) { //MULTIPLICATIVE
			fTransp *= (1.f / 2);
			fTransp += 0.5f;
			return PushVertexInTable(container, TextureContainer::Multiplicative);
		} else if(fTransp >= 1.f) { //ADDITIVE
			fTransp -= 1.f;
			return PushVertexInTable(container, TextureContainer::Additive);
		} else if(fTransp > 0.f) { //NORMAL TRANS
			fTransp = 1.f - fTransp;
			return PushVertexInTable(container, TextureContainer::Blended);
		} else { //SUBTRACTIVE
			fTransp = 1.f - fTransp;
			return PushVertexInTable(container, TextureContainer::Subtractive);
		}
	} else {
		return PushVertexInTable(container, TextureContainer::Opaque);
	}
}

// TODO remove this function, use drawTriangle directly!
void ARX_DrawPrimitive(TexturedVertex * v0, TexturedVertex * v1, TexturedVertex * v2) {
	TexturedVertex vertices[3];
	vertices[0] = *v0, vertices[1] = *v1, vertices[2] = *v2;
	drawTriangle(RenderMaterial::getCurrent(), vertices);
}

void drawTriangle(const RenderMaterial & mat, const TexturedVertex * vertices) {
	
	TexturedVertex projected[3];
	EE_P(&vertices[0].p, &projected[0]);
	EE_P(&vertices[1].p, &projected[1]);
	EE_P(&vertices[2].p, &projected[2]);
	projected[0].color = vertices[0].color;
	projected[0].uv = vertices[0].uv;
	projected[1].color = vertices[1].color;
	projected[1].uv = vertices[1].uv;
	projected[2].color = vertices[2].color;
	projected[2].uv = vertices[2].uv;
	
	RenderBatcher::getInstance().add(mat, projected);
}

bool Cedric_IO_Visible(const Vec3f & pos) {

	if(ACTIVEBKG) {
		//TODO maybe readd this
		//if(fartherThan(io->pos, ACTIVECAM->orgTrans.pos, ACTIVECAM->cdepth * 0.6f))
		//	return false;

		long xx = pos.x * ACTIVEBKG->Xmul;
		long yy = pos.z * ACTIVEBKG->Zmul;

		if(xx >= 1 && yy >= 1 && xx < ACTIVEBKG->Xsize-1 && yy < ACTIVEBKG->Zsize-1) {
			for(long ky = yy - 1; ky <= yy + 1; ky++)
				for(long kx = xx - 1; kx <= xx + 1; kx++) {
					FAST_BKG_DATA * feg = (FAST_BKG_DATA *)&ACTIVEBKG->fastdata[kx][ky];
					if(feg->treat)
						return true;
				}

			return false;
		}
	}

	return true;
}

/* Object dynamic lighting */
static void Cedric_ApplyLighting(EERIE_3DOBJ * eobj, Skeleton * obj, const ColorMod & colorMod) {

	/* Apply light on all vertices */
	for(size_t i = 0; i != obj->bones.size(); i++) {

		glm::quat *quat = &obj->bones[i].anim.quat;

		/* Get light value for each vertex */
		for(size_t v = 0; v != obj->bones[i].idxvertices.size(); v++) {
			size_t vertexIndex = obj->bones[i].idxvertices[v];

			Vec3f & position = eobj->vertexlist3[vertexIndex].v;
			Vec3f & normal = eobj->vertexlist[vertexIndex].norm;

			eobj->vertexlist3[vertexIndex].vert.color = ApplyLight(quat, position, normal, colorMod);
		}
	}
}

void UpdateBbox3d(EERIE_3DOBJ *eobj, EERIE_3D_BBOX & box3D) {

	box3D.reset();

	for(size_t i = 0 ; i < eobj->vertexlist.size(); i++) {
		box3D.add(eobj->vertexlist3[i].v);
	}
}

void UpdateBbox2d(const EERIE_3DOBJ & eobj, EERIE_2D_BBOX & box2D) {

	box2D.reset();

	for(size_t i = 0; i < eobj.vertexlist.size(); i++) {
		const EERIE_VERTEX & vertex = eobj.vertexlist[i];

		if(   vertex.vert.rhw > 0.f
		   && vertex.vert.p.x >= -32000
		   && vertex.vert.p.x <=  32000
		   && vertex.vert.p.y >= -32000
		   && vertex.vert.p.y <=  32000
		) {
			box2D.add(vertex.vert.p);
		}
	}
}

void DrawEERIEInter_ModelTransform(EERIE_3DOBJ *eobj, const TransformInfo &t) {

	for(size_t i = 0 ; i < eobj->vertexlist.size(); i++) {

		Vec3f temp = eobj->vertexlist[i].v;

		temp -= t.offset;
		temp *= t.scale;
		temp = t.rotation * temp;
		temp += t.pos;

		eobj->vertexlist3[i].v = temp;
	}
}

void DrawEERIEInter_ViewProjectTransform(EERIE_3DOBJ *eobj) {
	for(size_t i = 0 ; i < eobj->vertexlist.size(); i++) {

		Vec3f tempWorld;
		EE_RT(eobj->vertexlist3[i].v, tempWorld);
		EE_P(&tempWorld, &eobj->vertexlist[i].vert);
	}
}

bool CullFace(const EERIE_3DOBJ * eobj, const EERIE_FACE & face) {

	if(!(face.facetype & POLY_DOUBLESIDED)) {
		Vec3f normV10 = eobj->vertexlist3[face.vid[1]].v - eobj->vertexlist3[face.vid[0]].v;
		Vec3f normV20 = eobj->vertexlist3[face.vid[2]].v - eobj->vertexlist3[face.vid[0]].v;
		Vec3f normFace;
		normFace.x = (normV10.y * normV20.z) - (normV10.z * normV20.y);
		normFace.y = (normV10.z * normV20.x) - (normV10.x * normV20.z);
		normFace.z = (normV10.x * normV20.y) - (normV10.y * normV20.x);

		Vec3f nrm = eobj->vertexlist3[face.vid[0]].v - ACTIVECAM->orgTrans.pos;

		if(glm::dot(normFace, nrm) > 0.f)
			return true;
	}

	return false;
}

void AddFixedObjectHalo(const EERIE_FACE & face, const TransformInfo & t, const Entity * io, TexturedVertex * tvList, const EERIE_3DOBJ * eobj)
{
	float mdist=ACTIVECAM->cdepth;
	float ddist = mdist-fdist(t.pos, ACTIVECAM->orgTrans.pos);
	ddist = ddist/mdist;
	ddist = std::pow(ddist, 6);

	ddist = clamp(ddist, 0.25f, 0.9f);

	float tot=0;
	float _ffr[3];

	for(long o = 0; o < 3; o++) {
		Vec3f temporary3D;
		temporary3D = t.rotation * eobj->vertexlist[face.vid[o]].norm;

		float power = 255.f-(float)EEfabs(255.f*(temporary3D.z)*( 1.0f / 2 ));

		power = clamp(power, 0.f, 255.f);

		tot += power;
		_ffr[o] = power;

		u8 lfr = io->halo.color.r * power;
		u8 lfg = io->halo.color.g * power;
		u8 lfb = io->halo.color.b * power;
		tvList[o].color = ((0xFF << 24) | (lfr << 16) | (lfg << 8) | (lfb));
	}

	if(tot > 150.f) {
		long first;
		long second;
		long third;

		if(_ffr[0] >= _ffr[1] && _ffr[1] >= _ffr[2]) {
			first = 0;
			second = 1;
			third = 2;
		} else if(_ffr[0] >= _ffr[2] && _ffr[2] >= _ffr[1]) {
			first = 0;
			second = 2;
			third = 1;
		} else if(_ffr[1] >= _ffr[0] && _ffr[0] >= _ffr[2]) {
			first = 1;
			second = 0;
			third = 2;
		} else if(_ffr[1] >= _ffr[2] && _ffr[2] >= _ffr[0]) {
			first = 1;
			second = 2;
			third = 0;
		} else if(_ffr[2] >= _ffr[0] && _ffr[0] >= _ffr[1]) {
			first = 2;
			second = 0;
			third = 1;
		} else {
			first = 2;
			second = 1;
			third = 0;
		}

		if(_ffr[first] > 70.f && _ffr[second] > 60.f) {
			TexturedVertex vert[4];

			vert[0] = tvList[first];
			vert[1] = tvList[first];
			vert[2] = tvList[second];
			vert[3] = tvList[second];

			float siz = ddist * (io->halo.radius * 1.5f * (std::sin(arxtime.get_frame_time() * .01f) * .1f + .7f)) * .6f;

			Vec3f vect1;
			vect1.x = tvList[first].p.x - tvList[third].p.x;
			vect1.y = tvList[first].p.y - tvList[third].p.y;
			float len1 = 1.f / ffsqrt(vect1.x * vect1.x + vect1.y * vect1.y);

			if(vect1.x < 0.f)
				len1 *= 1.2f;

			vect1.x *= len1;
			vect1.y *= len1;

			Vec3f vect2;
			vect2.x = tvList[second].p.x - tvList[third].p.x;
			vect2.y = tvList[second].p.y - tvList[third].p.y;
			float len2 = 1.f / ffsqrt(vect2.x * vect2.x + vect2.y * vect2.y);

			if(vect2.x < 0.f)
				len2 *= 1.2f;

			vect2.x *= len2;
			vect2.y *= len2;

			vert[1].p.x += (vect1.x + 0.2f - rnd() * 0.1f) * siz;
			vert[1].p.y += (vect1.y + 0.2f - rnd() * 0.1f) * siz;
			vert[1].color = 0xFF000000;

			vert[0].p.z += 0.0001f;
			vert[3].p.z += 0.0001f;
			vert[1].rhw *= .8f;
			vert[2].rhw *= .8f;

			vert[2].p.x += (vect2.x + 0.2f - rnd() * 0.1f) * siz;
			vert[2].p.y += (vect2.y + 0.2f - rnd() * 0.1f) * siz;

			if(io->halo.flags & HALO_NEGATIVE)
				vert[2].color = 0x00000000;
			else
				vert[2].color = 0xFF000000;

			Halo_AddVertices(vert);
		}
	}
}

void DrawEERIEInter_Render(EERIE_3DOBJ *eobj, const TransformInfo &t, Entity *io, float invisibility) {

	ColorMod colorMod;
	colorMod.updateFromEntity(io, !io);

	Vec3f tv = t.pos;

	if(io && (io->ioflags & IO_ITEM))
		tv.y -= 60.f;
	else
		tv.y -= 90.f;

	UpdateLlights(tv, false);

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		const EERIE_FACE & face = eobj->facelist[i];

		if(CullFace(eobj, face))
			continue;

		if(face.texid < 0)
			continue;

		TextureContainer *pTex = eobj->texturecontainer[face.texid];
		if(!pTex)
			continue;

		float fTransp = 0.f;
		TexturedVertex *tvList = GetNewVertexList(pTex, face, invisibility, fTransp);

		for(size_t n = 0; n < 3; n++) {

			if(io && (io->ioflags & IO_ANGULAR)) {
				const Vec3f & position = eobj->vertexlist3[face.vid[n]].v;
				const Vec3f & normal = face.norm;

				eobj->vertexlist3[face.vid[n]].vert.color = ApplyLight(&t.rotation, position, normal, colorMod, 0.5f);
			} else {
				Vec3f & position = eobj->vertexlist3[face.vid[n]].v;
				Vec3f & normal = eobj->vertexlist[face.vid[n]].norm;

				eobj->vertexlist3[face.vid[n]].vert.color = ApplyLight(&t.rotation, position, normal, colorMod);
			}

			tvList[n] = eobj->vertexlist[face.vid[n]].vert;
			tvList[n].uv.x = face.u[n];
			tvList[n].uv.y = face.v[n];

			// Treat WATER Polys (modify UVs)
			if(face.facetype & POLY_WATER) {
				tvList[n].uv += getWaterFxUvOffset(eobj->vertexlist[face.vid[n]].v, 0.3f);
			}

			if(face.facetype & POLY_GLOW) {
				// unaffected by light
				tvList[n].color = 0xffffffff;
			} else {
				// Normal Illuminations
				tvList[n].color = eobj->vertexlist3[face.vid[n]].vert.color;
			}

			// TODO copy-paste
			if(io && Project.improve) {
				long lr=(tvList[n].color>>16) & 255;
				float ffr=(float)(lr);

				float dd = tvList[n].rhw;

				dd = clamp(dd, 0.f, 1.f);

				Vec3f & norm = eobj->vertexlist[face.vid[n]].norm;

				float fb=((1.f-dd)*6.f + (EEfabs(norm.x) + EEfabs(norm.y))) * 0.125f;
				float fr=((.6f-dd)*6.f + (EEfabs(norm.z) + EEfabs(norm.y))) * 0.125f;

				if(fr < 0.f)
					fr = 0.f;
				else
					fr = std::max(ffr, fr * 255.f);

				fr=std::min(fr,255.f);
				fb*=255.f;
				fb=std::min(fb,255.f);
				u8 lfr = fr;
				u8 lfb = fb;
				u8 lfg = 0x1E;
				tvList[n].color = (0xff000000L | (lfr << 16) | (lfg << 8) | (lfb));
			}

			// Transparent poly: storing info to draw later
			if((face.facetype & POLY_TRANS) || invisibility > 0.f) {
				tvList[n].color = Color::gray(fTransp).toBGR();
			}
		}

		// HALO HANDLING START
		if(io && (io->halo.flags & HALO_ACTIVE)) {
			AddFixedObjectHalo(face, t, io, tvList, eobj);
		}
	}
}

void DrawEERIEInter(EERIE_3DOBJ *eobj, const TransformInfo &t, Entity *io, bool forceDraw, float invisibility) {

	if(!eobj)
		return;

	// Avoids To treat an object that isn't Visible
	if(!forceDraw && io && io != entities.player() && !Cedric_IO_Visible(t.pos))
		return;

	DrawEERIEInter_ModelTransform(eobj, t);
	if(io) {
		UpdateBbox3d(eobj, io->bbox3D);
	}

	DrawEERIEInter_ViewProjectTransform(eobj);
	if(io) {
		UpdateBbox2d(*eobj, io->bbox2D);
	}

	if(!forceDraw && ARX_SCENE_PORTAL_ClipIO(io, t.pos))
		return;

	DrawEERIEInter_Render(eobj, t, io, invisibility);
}


struct HaloRenderInfo {

	HaloRenderInfo(IO_HALO * halo, long selection)
		: halo(halo)
		, selection(selection)
	{}

	explicit HaloRenderInfo(IO_HALO * halo)
		: halo(halo)
		, selection(-1)
	{}

	IO_HALO * halo;
	short selection;
};

void pushSlotHalo(std::vector<HaloRenderInfo> & halos, EquipmentSlot slot, short selection) {

	if(player.equiped[slot] != PlayerEntityHandle && ValidIONum(player.equiped[slot])) {
		Entity * tio = entities[player.equiped[slot]];

		if(tio->halo.flags & HALO_ACTIVE) {
			halos.push_back(HaloRenderInfo(&tio->halo, selection));
		}
	}
}

struct HaloInfo {
	bool need_halo;
	float MAX_ZEDE;
	float ddist;

	HaloInfo()
		: need_halo(0)
		, MAX_ZEDE(0.f)
		, ddist(0.f)
	{}

	std::vector<HaloRenderInfo> halos;
};

//-----------------------------------------------------------------------------
extern long IN_BOOK_DRAW;

void PrepareAnimatedObjectHalo(HaloInfo & haloInfo, const Vec3f& pos, Skeleton* obj, Entity *use_io, EERIE_3DOBJ* eobj)
{
	std::vector<HaloRenderInfo> & halos = haloInfo.halos;

	if(use_io == entities.player()) {
		pushSlotHalo(halos, EQUIP_SLOT_HELMET,   eobj->fastaccess.sel_head);
		pushSlotHalo(halos, EQUIP_SLOT_ARMOR,    eobj->fastaccess.sel_chest);
		pushSlotHalo(halos, EQUIP_SLOT_LEGGINGS, eobj->fastaccess.sel_leggings);
	}

	if(use_io->halo.flags & HALO_ACTIVE) {
		halos.push_back(HaloRenderInfo(&use_io->halo));
	}

	if(halos.size() > 0) {

		Vec3f ftrPos = pos;
		//TODO copy-pase
		float mdist = ACTIVECAM->cdepth;
		mdist *= ( 1.0f / 2 );

		float ddist = mdist-fdist(ftrPos, ACTIVECAM->orgTrans.pos);
		ddist = ddist/mdist;
		ddist = std::pow(ddist, 6);
		ddist = clamp(ddist, 0.25f, 0.9f);

		haloInfo.ddist = ddist;

		Cedric_PrepareHalo(eobj, obj);

		haloInfo.MAX_ZEDE = 0.f;
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			if(eobj->vertexlist3[i].vert.rhw > 0.f)
				haloInfo.MAX_ZEDE = std::max(eobj->vertexlist3[i].vert.p.z, haloInfo.MAX_ZEDE);
		}

		haloInfo.need_halo = true;
	}
}

void AddAnimatedObjectHalo(HaloInfo & haloInfo, const unsigned short * paf, float invisibility, EERIE_3DOBJ* eobj, Entity* io, TexturedVertex *tvList) {


	float & ddist = haloInfo.ddist;
	float & MAX_ZEDE = haloInfo.MAX_ZEDE;
	std::vector<HaloRenderInfo> & halos = haloInfo.halos;

	IO_HALO * curhalo = NULL;

	for(size_t h = 0; h < halos.size(); h++) {
		if(halos[h].selection == -1 || IsInSelection(eobj, paf[0], halos[h].selection) >= 0) {
			curhalo = halos[h].halo;
			break;
		}
	}

	if(!curhalo)
		return;

	float tot = 0;
	float _ffr[3];
	ColorBGRA colors[3];

	for(size_t o = 0; o < 3; o++) {
		float tttz	= EEfabs(eobj->vertexlist3[paf[o]].norm.z) * ( 1.0f / 2 );
		float power = 255.f - (float)(255.f * tttz);
		power *= (1.f - invisibility);

		power = clamp(power, 0.f, 255.f);

		tot += power;
		_ffr[o] = power;

		u8 lfr = curhalo->color.r * power;
		u8 lfg = curhalo->color.g * power;
		u8 lfb = curhalo->color.b * power;
		colors[o] = ((0xFF << 24) | (lfr << 16) | (lfg << 8) | (lfb));
	}

	if(tot > 260) {
		long first;
		long second;
		long third;

		if(_ffr[0] >= _ffr[1] && _ffr[1] >= _ffr[2]) {
			first = 0;
			second = 1;
			third = 2;
		} else if(_ffr[0] >= _ffr[2] && _ffr[2] >= _ffr[1]) {
			first = 0;
			second = 2;
			third = 1;
		} else if(_ffr[1] >= _ffr[0] && _ffr[0] >= _ffr[2]) {
			first = 1;
			second = 0;
			third = 2;
		} else if(_ffr[1] >= _ffr[2] && _ffr[2] >= _ffr[0]) {
			first = 1;
			second = 2;
			third = 0;
		} else if(_ffr[2] >= _ffr[0] && _ffr[0] >= _ffr[1]) {
			first = 2;
			second = 0;
			third = 1;
		} else {
			first = 2;
			second = 1;
			third = 0;
		}

		if(_ffr[first] > 150.f && _ffr[second] > 110.f) {
			TexturedVertex vert[4];

			vert[0] = tvList[first];
			vert[1] = tvList[first];
			vert[2] = tvList[second];
			vert[3] = tvList[second];

			vert[0].color = colors[first];
			vert[1].color = colors[first];
			vert[2].color = colors[second];
			vert[3].color = colors[second];

			float siz = ddist * (curhalo->radius * (std::sin(arxtime.get_frame_time() * .01f) * .1f + 1.f)) * .6f;

			if(io == entities.player() && ddist > 0.8f && !EXTERNALVIEW)
				siz *= 1.5f;

			Vec3f vect1;
			vect1.x = tvList[first].p.x - tvList[third].p.x;
			vect1.y = tvList[first].p.y - tvList[third].p.y;
			float len1 = 2.f / ffsqrt(vect1.x * vect1.x + vect1.y * vect1.y);

			if(vect1.x < 0.f)
				len1 *= 1.2f;

			vect1.x *= len1;
			vect1.y *= len1;

			Vec3f vect2;
			vect2.x = tvList[second].p.x - tvList[third].p.x;
			vect2.y = tvList[second].p.y - tvList[third].p.y;
			float len2 = 1.f / ffsqrt(vect2.x * vect2.x + vect2.y * vect2.y);

			if(vect2.x < 0.f)
				len2 *= 1.2f;

			vect2.x *= len2;
			vect2.y *= len2;

			vert[1].p.x += (vect1.x + 0.2f - rnd() * 0.1f) * siz;
			vert[1].p.y += (vect1.y + 0.2f - rnd() * 0.1f) * siz;
			vert[1].color = 0xFF000000;

			float valll;
			valll = 0.005f + (EEfabs(tvList[first].p.z) - EEfabs(tvList[third].p.z))
						   + (EEfabs(tvList[second].p.z) - EEfabs(tvList[third].p.z));
			valll = 0.0001f + valll * ( 1.0f / 10 );

			if(valll < 0.f)
				valll = 0.f;

			vert[1].p.z	+= valll;
			vert[2].p.z	+= valll;

			vert[0].p.z	+= 0.0001f;
			vert[3].p.z	+= 0.0001f;//*( 1.0f / 2 );
			vert[1].rhw	*= .98f;
			vert[2].rhw	*= .98f;
			vert[0].rhw	*= .98f;
			vert[3].rhw	*= .98f;

			vert[2].p.x += (vect2.x + 0.2f - rnd() * 0.1f) * siz;
			vert[2].p.y += (vect2.y + 0.2f - rnd() * 0.1f) * siz;

			vert[1].p.z = (vert[1].p.z + MAX_ZEDE) * ( 1.0f / 2 );
			vert[2].p.z = (vert[2].p.z + MAX_ZEDE) * ( 1.0f / 2 );

			if(curhalo->flags & HALO_NEGATIVE)
				vert[2].color = 0x00000000;
			else
				vert[2].color = 0xFF000000;

			Halo_AddVertices(vert);
		}
	}
}

static void Cedric_RenderObject(EERIE_3DOBJ * eobj, Skeleton * obj, Entity * io, const Vec3f & pos, float invisibility) {

	if(invisibility == 1.f)
		return;

	Entity *use_io = io;

	if(!io && IN_BOOK_DRAW && eobj == entities.player()->obj)
		use_io = entities.player();

	arx_assert(use_io);

	HaloInfo haloInfo;
	PrepareAnimatedObjectHalo(haloInfo, pos, obj, use_io, eobj);

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		const EERIE_FACE & face = eobj->facelist[i];

		if((face.facetype & POLY_HIDE) && !IN_BOOK_DRAW)
			continue;

		if(CullFace(eobj, face))
			continue;

		if(face.texid < 0)
			continue;

		TextureContainer *pTex = eobj->texturecontainer[face.texid];
		if(!pTex)
			continue;

		float fTransp = 0.f;
		TexturedVertex *tvList = GetNewVertexList(pTex, face, invisibility, fTransp);

		for(size_t n = 0; n < 3; n++) {
			tvList[n].p     = eobj->vertexlist3[face.vid[n]].vert.p;
			tvList[n].rhw   = eobj->vertexlist3[face.vid[n]].vert.rhw;
			tvList[n].color = eobj->vertexlist3[face.vid[n]].vert.color;
			tvList[n].uv.x  = face.u[n];
			tvList[n].uv.y  = face.v[n];
		}

		if((face.facetype & POLY_TRANS) || invisibility > 0.f) {
			tvList[0].color = tvList[1].color = tvList[2].color = Color::gray(fTransp).toBGR();
		}

		if(haloInfo.need_halo) {
			AddAnimatedObjectHalo(haloInfo, face.vid, invisibility, eobj, io, tvList);
		}
	}
}

void Cedric_AnimateDrawEntityRender(EERIE_3DOBJ *eobj, const Vec3f & pos, Entity *io, float invisibility) {

	Skeleton *obj = eobj->m_skeleton;

	if(!obj)
		return;

	ColorMod colorMod;
	colorMod.updateFromEntity(io);

	/* Get nearest lights */
	Vec3f tv = pos;

	if(io && io->obj->fastaccess.view_attach >= 0 && io->obj->fastaccess.head_group_origin != -1)
		tv.y = io->obj->vertexlist3[io->obj->fastaccess.head_group_origin].v.y + 10;
	else
		tv.y -= 90.f;

	UpdateLlights(tv, false);

	Cedric_ApplyLighting(eobj, obj, colorMod);

	Cedric_RenderObject(eobj, obj, io, pos, invisibility);

	// Now we can render Linked Objects
	for(size_t k = 0; k < eobj->linked.size(); k++) {
		const EERIE_LINKED & link = eobj->linked[k];

		if(link.lgroup == -1 || !link.obj)
			continue;

		// specific check to avoid drawing player weapon on its back when in subjective view
		if(io == entities.player() &&
			link.lidx == entities.player()->obj->fastaccess.weapon_attach &&
			!EXTERNALVIEW
		)
			continue;


		TransformInfo t(
			eobj->vertexlist3[link.lidx].v,
			eobj->m_skeleton->bones[link.lgroup].anim.quat,
			link.io ? link.io->scale : 1.f,
			link.obj->vertexlist[link.lidx2].v - link.obj->vertexlist[link.obj->origin].v);

		DrawEERIEInter(link.obj, t, link.io, true, invisibility);
	}
}

static Vec3f CalcTranslation(ANIM_USE * animuse) {
	
	if(!animuse || !animuse->cur_anim) {
		return Vec3f_ZERO;
	}
	
	EERIE_ANIM	*eanim = animuse->cur_anim->anims[animuse->altidx_cur];
	
	if(!eanim) {
		return Vec3f_ZERO;
	}
	
	//Avoiding impossible cases
	if(animuse->fr < 0) {
		animuse->fr = 0;
		animuse->pour = 0.f;
	} else if(animuse->fr >= eanim->nb_key_frames - 1) {
		animuse->fr = eanim->nb_key_frames - 2;
		animuse->pour = 1.f;
	}
	animuse->pour = clamp(animuse->pour, 0.f, 1.f);
	
	// FRAME TRANSLATE : Gives the Virtual pos of Main Object
	if(eanim->frames[animuse->fr].f_translate && !(animuse->flags & EA_STATICANIM)) {
		EERIE_FRAME * sFrame = &eanim->frames[animuse->fr];
		EERIE_FRAME * eFrame = &eanim->frames[animuse->fr+1];
		// Linear interpolation of object translation (MOVE)
		return sFrame->translate + (eFrame->translate - sFrame->translate) * animuse->pour;
	}
	
	return Vec3f_ZERO;
}

static void StoreEntityMovement(Entity * io, Vec3f & ftr, float scale) {

	if(!io)
		return;

	Vec3f ftr2 = Vec3f_ZERO;

	ftr *= scale;

	float temp;
	if (io == entities.player()) {
		temp = MAKEANGLE(180.f - player.angle.getPitch());
	} else {
		temp = MAKEANGLE(180.f - io->angle.getPitch());
	}

	ftr2 = VRotateY(ftr, temp);

	// stores Translations for a later use
	io->move = ftr2;

	if(io->animlayer[0].cur_anim) {

		// Use calculated value to notify the Movement engine of the translation to do
		if(io->ioflags & IO_NPC) {
			ftr = Vec3f_ZERO;
			io->move -= io->lastmove;
		} else if (io->gameFlags & GFLAG_ELEVATOR) {
			// Must recover translations for NON-NPC IO
			PushIO_ON_Top(io, io->move.y - io->lastmove.y);
		}

		io->lastmove = ftr2;
	}
}

/*!
 * Animate skeleton
 */
static void Cedric_AnimateObject(Skeleton * obj, ANIM_USE * animlayer)
{
	std::vector<unsigned char> grps(obj->bones.size());

	for(long count = MAX_ANIM_LAYERS - 1; count >= 0; count--) {

		ANIM_USE * animuse = &animlayer[count];

		if(!animuse || !animuse->cur_anim)
			continue;

		EERIE_ANIM *eanim = animuse->cur_anim->anims[animuse->altidx_cur];
		if(!eanim)
			continue;

		if(animuse->fr < 0) {
			animuse->fr = 0;
			animuse->pour = 0.f;
		} else if(animuse->fr >= eanim->nb_key_frames - 1) {
			animuse->fr = eanim->nb_key_frames - 2;
			animuse->pour = 1.f;
		}
		animuse->pour = clamp(animuse->pour, 0.f, 1.f);

		// Now go for groups rotation/translation/scaling, And transform Linked objects by the way
		int l = std::min(long(obj->bones.size() - 1), eanim->nb_groups - 1);

		for(int j = l; j >= 0; j--) {
			if(grps[j])
				continue;

			EERIE_GROUP * sGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)];
			EERIE_GROUP * eGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)+eanim->nb_groups];

			if(!eanim->voidgroups[j])
				grps[j] = 1;

			if(eanim->nb_key_frames != 1) {
				Bone & bone = obj->bones[j];

				BoneTransform temp;

				temp.quat = Quat_Slerp(sGroup->quat, eGroup->quat, animuse->pour);
				temp.trans = sGroup->translate + (eGroup->translate - sGroup->translate) * animuse->pour;
				temp.scale = sGroup->zoom + (eGroup->zoom - sGroup->zoom) * animuse->pour;

				bone.init.quat = bone.init.quat * temp.quat;
				bone.init.trans = temp.trans + bone.transinit_global;
				bone.init.scale = temp.scale;
			}
		}
	}
}

void Cedric_BlendAnimation(Skeleton & rig, AnimationBlendStatus * animBlend) {

	if(!animBlend->nb_lastanimvertex) {
		return;
	}

	float timm = (arxtime.get_frame_time() - animBlend->lastanimtime) + 0.0001f;

	if(timm >= 300.f) {
		animBlend->nb_lastanimvertex = 0;
		return;
	} else {
		timm *= ( 1.0f / 300 );

		if(timm < 0.f || timm >= 1.f)
			return;
	}

	for(size_t i = 0; i < rig.bones.size(); i++) {
		Bone * bone = &rig.bones[i];

		bone->init.quat = Quat_Slerp(bone->last.quat, bone->init.quat, timm);

		bone->init.trans = bone->last.trans + (bone->init.trans - bone->last.trans) * timm;
		bone->init.scale = bone->last.scale + (bone->init.scale - bone->last.scale) * timm;
	}
}

/*!
 * Apply transformations on all bones
 */
static void Cedric_ConcatenateTM(Skeleton & rig, const TransformInfo & t) {

	for(size_t i = 0; i != rig.bones.size(); i++) {
		Bone * bone = &rig.bones[i];

		if(bone->father >= 0) { // Child Bones
			Bone * parent = &rig.bones[bone->father];
			// Rotation
			bone->anim.quat = parent->anim.quat * bone->init.quat;

			// Translation
			bone->anim.trans = bone->init.trans * parent->anim.scale;
			bone->anim.trans = parent->anim.quat * bone->anim.trans;
			bone->anim.trans = parent->anim.trans + bone->anim.trans;

			// Scale
			bone->anim.scale = (bone->init.scale + Vec3f_ONE) * parent->anim.scale;
		} else { // Root Bone
			// Rotation
			bone->anim.quat = t.rotation * bone->init.quat;

			// Translation
			Vec3f vt1 = bone->init.trans + t.offset;
			bone->anim.trans = t.rotation * vt1;
			bone->anim.trans *= t.scale;
			bone->anim.trans += t.pos;

			// Compute Global Object Scale AND Global Animation Scale
			bone->anim.scale = (bone->init.scale + Vec3f_ONE) * t.scale;
		}
	}
}

/*!
 * Transform object vertices
 */
void Cedric_TransformVerts(EERIE_3DOBJ *eobj, const Vec3f & pos) {

	Skeleton & rig = *eobj->m_skeleton;

	// Transform & project all vertices
	for(size_t i = 0; i != rig.bones.size(); i++) {
		Bone & bone = rig.bones[i];

		glm::mat4x4 matrix = glm::toMat4(bone.anim.quat);
		
		// Apply Scale
		matrix[0][0] *= bone.anim.scale.x;
		matrix[0][1] *= bone.anim.scale.x;
		matrix[0][2] *= bone.anim.scale.x;

		matrix[1][0] *= bone.anim.scale.y;
		matrix[1][1] *= bone.anim.scale.y;
		matrix[1][2] *= bone.anim.scale.y;

		matrix[2][0] *= bone.anim.scale.z;
		matrix[2][1] *= bone.anim.scale.z;
		matrix[2][2] *= bone.anim.scale.z;

		Vec3f vector = bone.anim.trans;

		for(size_t v = 0; v != bone.idxvertices.size(); v++) {
			long index = bone.idxvertices[v];

			Vec3f & inVert = eobj->vertexlocal[index];
			EERIE_VERTEX & outVert = eobj->vertexlist3[index];
			
			outVert.v = Vec3f(matrix * Vec4f(inVert, 1.f));
			outVert.v += vector;
			
			outVert.vert.p = outVert.v;
		}
	}

	if(eobj->sdata) {
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			eobj->vertexlist[i].vert.p = eobj->vertexlist3[i].v - pos;
		}
	}
}

void Cedric_ViewProjectTransform(EERIE_3DOBJ *eobj) {

	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		EERIE_VERTEX * outVert = &eobj->vertexlist3[i];

		Vec3f tempWorld;
		EE_RT(outVert->v, tempWorld);
		EE_P(&tempWorld, &outVert->vert);
	}
}

void Cedric_UpdateBbox2d(const EERIE_3DOBJ & eobj, EERIE_2D_BBOX & box2D) {

	box2D.reset();

	for(size_t i = 0; i < eobj.vertexlist.size(); i++) {
		const EERIE_VERTEX & vertex = eobj.vertexlist3[i];
		
		if(   vertex.vert.rhw > 0.f
		   && vertex.vert.p.x >= -32000
		   && vertex.vert.p.x <=  32000
		   && vertex.vert.p.y >= -32000
		   && vertex.vert.p.y <=  32000
		) {
			box2D.add(vertex.vert.p);
		}
	}
}

/*!
 * \brief Apply animation and draw object
 */
void Cedric_AnimateDrawEntity(Skeleton & skeleton, ANIM_USE * animlayer, EERIE_EXTRA_ROTATE * extraRotation, AnimationBlendStatus * animBlend, EERIE_EXTRA_SCALE & extraScale) {

	// Initialize the rig
	for(size_t i = 0; i != skeleton.bones.size(); i++) {
		Bone & bone = skeleton.bones[i];

		bone.init.quat = glm::quat();
		bone.init.trans = bone.transinit_global;
	}

	// Apply Extra Rotations in Local Space
	if(extraRotation) {
		for(long k = 0; k < MAX_EXTRA_ROTATE; k++) {
			long i = extraRotation->group_number[k];

			if(i >= 0) {
				skeleton.bones[i].init.quat = angleToQuatForExtraRotation(extraRotation->group_rotate[k]);
			}
		}
	}

	// Perform animation in Local space
	Cedric_AnimateObject(&skeleton, animlayer);

	if(extraScale.groupIndex != -1) {
		Bone & bone = skeleton.bones[extraScale.groupIndex];

		bone.init.scale += extraScale.scale;
	}

	// Check for Animation Blending in Local space
	if(animBlend) {
		// Is There any Between-Animations Interpolation to make ?
		Cedric_BlendAnimation(skeleton, animBlend);

		for(size_t i = 0; i < skeleton.bones.size(); i++) {
			skeleton.bones[i].last = skeleton.bones[i].init;
		}
	}
}

void EERIEDrawAnimQuatUpdate(EERIE_3DOBJ *eobj, ANIM_USE * animlayer,const Anglef & angle, const Vec3f & pos, unsigned long time, Entity *io, bool update_movement) {

	if(io) {
		float speedfactor = io->basespeed + io->speed_modif;

		if(speedfactor < 0)
			speedfactor = 0;

		float tim = (float)time * speedfactor;

		if(tim<=0.f)
			time=0;
		else
			time=(unsigned long)tim;

		io->frameloss += tim - time;

		if(io->frameloss > 1.f) { // recover lost time...
			long tt = io->frameloss;
			io->frameloss -= tt;
			time += tt;
		}
	}

	if(time > 0) {
		for(size_t count = 0; count < MAX_ANIM_LAYERS; count++) {
			ANIM_USE * animuse = &animlayer[count];
			if(animuse->cur_anim)
				PrepareAnim(animuse, time, io);
		}
	}

	// Set scale and invisibility factors
	// Scaling Value for this object (Movements will also be scaled)
	float scale = (io) ? io->scale : 1.f;

	// Only layer 0 controls movement
	Vec3f ftr = CalcTranslation(&animlayer[0]);


	if(update_movement)
		StoreEntityMovement(io, ftr, scale);

	if(io && io != entities.player() && !Cedric_IO_Visible(io->pos))
		return;

	glm::quat rotation;

	bool isNpc = io && (io->ioflags & IO_NPC);
	if(!isNpc) {
		// To correct invalid angle in Animated FIX/ITEMS
		rotation = glm::toQuat(toRotationMatrix(angle));
	} else {
		rotation = QuatFromAngles(angle);
	}

	EERIE_EXTRA_ROTATE * extraRotation = NULL;
	AnimationBlendStatus * animBlend = NULL;

	if(io && (io->ioflags & IO_NPC) && io->_npcdata->ex_rotate) {
		extraRotation = io->_npcdata->ex_rotate;
	}

	if(io) {
		animBlend = &io->animBlend;
	}

	EERIE_EXTRA_SCALE extraScale;

	if(BH_MODE && eobj->fastaccess.head_group != -1) {
		extraScale.groupIndex = eobj->fastaccess.head_group;
		extraScale.scale = Vec3f_ONE;
	}

	arx_assert(eobj->m_skeleton);
	Skeleton & skeleton = *eobj->m_skeleton;

	Cedric_AnimateDrawEntity(skeleton, animlayer, extraRotation, animBlend, extraScale);

	// Build skeleton in Object Space
	TransformInfo t(pos, rotation, scale, ftr);
	Cedric_ConcatenateTM(skeleton, t);

	Cedric_TransformVerts(eobj, pos);
	if(io) {
		UpdateBbox3d(eobj, io->bbox3D);
	}

	Cedric_ViewProjectTransform(eobj);
	if(io) {
		Cedric_UpdateBbox2d(*eobj, io->bbox2D);
	}
}

void EERIEDrawAnimQuatRender(EERIE_3DOBJ *eobj, const Vec3f & pos, Entity *io, float invisibility) {

	if(io && io != entities.player() && !Cedric_IO_Visible(io->pos))
		return;

	bool isFightingNpc = io &&
						 (io->ioflags & IO_NPC) &&
						 (io->_npcdata->behavior & BEHAVIOUR_FIGHT) &&
						 closerThan(io->pos, player.pos, 240.f);

	if(!isFightingNpc && ARX_SCENE_PORTAL_ClipIO(io, pos))
		return;

	Cedric_AnimateDrawEntityRender(eobj, pos, io, invisibility);
}

void EERIEDrawAnimQuat(EERIE_3DOBJ *eobj, ANIM_USE * animlayer,const Anglef & angle, const Vec3f & pos, unsigned long time, Entity *io, bool update_movement, float invisibility) {

	EERIEDrawAnimQuatUpdate(eobj, animlayer,angle, pos, time, io, update_movement);
	EERIEDrawAnimQuatRender(eobj, pos, io, invisibility);
}

void AnimatedEntityUpdate(Entity * entity, float time) {
	EERIEDrawAnimQuatUpdate(entity->obj, entity->animlayer, entity->angle,
							entity->pos, time, entity, true);
}

void AnimatedEntityRender(Entity * entity, float invisibility) {

	Cedric_ViewProjectTransform(entity->obj);
	Cedric_UpdateBbox2d(*entity->obj, entity->bbox2D);

	EERIEDrawAnimQuatRender(entity->obj, entity->pos, entity, invisibility);
}
