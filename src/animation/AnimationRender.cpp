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
static TexturedVertex tTexturedVertexTab2[4000];

// List of TO-TREAT vertex for MIPMESHING

// TODO: Convert to a RenderBatch & make TextureContainer constructor private
TextureContainer TexSpecialColor("specialcolor_list", TextureContainer::NoInsert);

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

static void PopOneTriangleList(TextureContainer *_pTex) {

	if(!_pTex->count[TextureContainer::Opaque]) {
		return;
	}

	GRenderer->SetTexture(0, _pTex);

	if(_pTex->userflags & POLY_LATE_MIP) {
		const float GLOBAL_NPC_MIPMAP_BIAS = -2.2f;
		GRenderer->GetTextureStage(0)->setMipMapLODBias(GLOBAL_NPC_MIPMAP_BIAS);
	}


	EERIEDRAWPRIM(Renderer::TriangleList, _pTex->list[TextureContainer::Opaque], _pTex->count[TextureContainer::Opaque]);

	_pTex->count[TextureContainer::Opaque] = 0;

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

void PopAllTriangleList() {
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);
	GRenderer->SetCulling(Renderer::CullNone);

	TextureContainer * pTex = GetTextureList();
	while(pTex) {
		PopOneTriangleList(pTex);
		pTex = pTex->m_pNext;
	}
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
}

//-----------------------------------------------------------------------------
void PopOneInterZMapp(TextureContainer *_pTex)
{
	if(!_pTex->TextureRefinement) return;

	GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);

	if(_pTex->TextureRefinement->vPolyInterZMap.size())
	{
		GRenderer->SetTexture(0, _pTex->TextureRefinement);

		int iPos=0;

		std::vector<SMY_ZMAPPINFO>::iterator it;

		for (it = _pTex->TextureRefinement->vPolyInterZMap.begin();
			it != _pTex->TextureRefinement->vPolyInterZMap.end();
			++it)
		{
			SMY_ZMAPPINFO *pSMY = &(*it);

			tTexturedVertexTab2[iPos]        = pSMY->pVertex[0];
			tTexturedVertexTab2[iPos].color  = Color::gray(pSMY->color[0]).toBGR();
			tTexturedVertexTab2[iPos].uv.x   = pSMY->uv[0];
			tTexturedVertexTab2[iPos++].uv.y = pSMY->uv[1];
			tTexturedVertexTab2[iPos]        = pSMY->pVertex[1];
			tTexturedVertexTab2[iPos].color  = Color::gray(pSMY->color[1]).toBGR();
			tTexturedVertexTab2[iPos].uv.x   = pSMY->uv[2];
			tTexturedVertexTab2[iPos++].uv.y = pSMY->uv[3];
			tTexturedVertexTab2[iPos]        = pSMY->pVertex[2];
			tTexturedVertexTab2[iPos].color  = Color::gray(pSMY->color[2]).toBGR();
			tTexturedVertexTab2[iPos].uv.x   = pSMY->uv[4];
			tTexturedVertexTab2[iPos++].uv.y = pSMY->uv[5];
		}

		EERIEDRAWPRIM(Renderer::TriangleList, tTexturedVertexTab2, iPos);

		_pTex->TextureRefinement->vPolyInterZMap.clear();
	}
}

//-----------------------------------------------------------------------------
void PopAllTriangleListTransparency() {

	GRenderer->SetFogColor(Color::none);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false);
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);
	GRenderer->SetAlphaFunc(Renderer::CmpGreater, .5f);

	GRenderer->SetCulling(Renderer::CullNone);

	PopOneTriangleList(&TexSpecialColor);

	TextureContainer * pTex = GetTextureList();

	while(pTex)
	{
		PopOneTriangleListTransparency(pTex);

		//ZMAP
		PopOneInterZMapp(pTex);

		pTex=pTex->m_pNext;
	}

	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
	GRenderer->SetAlphaFunc(Renderer::CmpNotEqual, 0.f);
}





extern long BH_MODE;

extern TextureContainer TexSpecialColor;

extern long ZMAPMODE;

extern bool EXTERNALVIEW;

void EE_RT(const Vec3f & in, Vec3f & out);
void EE_P(Vec3f * in, TexturedVertex * out);

float Cedric_GetScale(Entity * io) {
	if(io) {
		// Scaling Value for this object (Movements will also be scaled)
		return io->scale;
	} else {
		return 1.f;
	}
}

float Cedric_GetInvisibility(Entity *io) {
	if(io) {
		float invisibility = io->invisibility;

		if (invisibility > 1.f)
			invisibility -= 1.f;

		if(io != entities.player() && invisibility > 0.f && !EXTERNALVIEW) {
			long num = ARX_SPELLS_GetSpellOn(io, SPELL_INVISIBILITY);

			if(num >= 0) {
				if(player.Full_Skill_Intuition > spells[num].caster_level * 10) {
					invisibility -= (float)player.Full_Skill_Intuition * (1.0f / 100)
									+ (float)spells[num].caster_level * (1.0f / 10);

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
						EERIE_SPHERE sp;
						sp.origin = io->pos;
						sp.radius = 200.f;
						long count = 6;

						while(count--) {
							SpawnGroundSplat(&sp, &rgb, rnd() * 30.f + 30.f, 1);
							sp.origin.y -= rnd() * 150.f;

							ARX_PARTICLES_Spawn_Splat(sp.origin, 200.f, io->_npcdata->blood_color);

							sp.origin.x = io->pos.x + rnd() * 200.f - 100.f;
							sp.origin.y = io->pos.y + rnd() * 20.f - 10.f;
							sp.origin.z = io->pos.z + rnd() * 200.f - 100.f;
							sp.radius = rnd() * 100.f + 100.f;
						}

						long nn = GetFreeDynLight();
						if(nn >= 0) {
							DynLight[nn].exist = 1;
							DynLight[nn].intensity = 0.7f + 2.f * rnd();
							DynLight[nn].fallend = 600.f;
							DynLight[nn].fallstart = 400.f;
							DynLight[nn].rgb.r = 1.0f;
							DynLight[nn].rgb.g = 0.8f;
							DynLight[nn].rgb.b = .0f;
							DynLight[nn].pos.x = io->pos.x;
							DynLight[nn].pos.y = io->pos.y - 80.f;
							DynLight[nn].pos.z = io->pos.z;
							DynLight[nn].duration = 600;
						}

						if(io->sfx_flag & SFX_TYPE_INCINERATE) {
							io->sfx_flag &= ~SFX_TYPE_INCINERATE;
							io->sfx_flag &= ~SFX_TYPE_YLSIDE_DEATH;
							long num = ARX_SPELLS_GetSpellOn(io, SPELL_INCINERATE);

							if(num < 0)
								num = ARX_SPELLS_GetSpellOn(io, SPELL_MASS_INCINERATE);

							if(num >= 0) {
								spells[num].tolive = 0;
								float damages = 20 * spells[num].caster_level;
								damages = ARX_SPELLS_ApplyFireProtection(io, damages);

								if (ValidIONum(spells[num].caster))
									ARX_DAMAGES_DamageNPC(io, damages, spells[num].caster, 1, &entities[spells[num].caster]->pos);
								else
									ARX_DAMAGES_DamageNPC(io, damages, spells[num].caster, 1, &io->pos);

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

void Cedric_PrepareHalo(EERIE_3DOBJ * eobj, EERIE_C_DATA * obj) {
	Vec3f cam_vector, t_vector;
	cam_vector.x = -std::sin(radians(ACTIVECAM->angle.getPitch())) * std::cos(radians(ACTIVECAM->angle.getYaw()));
	cam_vector.y =  std::sin(radians(ACTIVECAM->angle.getYaw()));
	cam_vector.z =  std::cos(radians(ACTIVECAM->angle.getPitch())) * std::cos(radians(ACTIVECAM->angle.getYaw()));

	// Apply light on all vertices
	for(long i = 0; i != obj->nb_bones; i++) {
		EERIE_QUAT qt1 = obj->bones[i].anim.quat;
		TransformInverseVertexQuat(&qt1, &cam_vector, &t_vector);

		// Get light value for each vertex
		for(long v = 0; v != obj->bones[i].nb_idxvertices; v++) {
			//inVert  = &eobj->normallocal[obj->bones[i].idxvertices[v]];
			EERIE_3DPAD * inVert = (EERIE_3DPAD *)&eobj->vertexlist[obj->bones[i].idxvertices[v]].norm;

			// Get cos angle between light and vertex norm
			eobj->vertexlist3[obj->bones[i].idxvertices[v]].norm.z =
			    (inVert->x * t_vector.x + inVert->y * t_vector.y + inVert->z * t_vector.z);

		}
	}
}

TexturedVertex * GetNewVertexList(TextureContainer * container, EERIE_FACE * face, float invisibility, float & fTransp) {

	fTransp = 0.f;

	if((face->facetype & POLY_TRANS) || invisibility > 0.f) {
		if(invisibility > 0.f)
			fTransp = 2.f - invisibility;
		else
			fTransp = face->transval;

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

void ARX_DrawPrimitive(TexturedVertex * _pVertex1, TexturedVertex * _pVertex2, TexturedVertex * _pVertex3, float _fAddZ) {
	
	ARX_UNUSED(_fAddZ);
	
	TexturedVertex pPointAdd[3];
	EE_P(&_pVertex1->p, &pPointAdd[0]);
	EE_P(&_pVertex2->p, &pPointAdd[1]);
	EE_P(&_pVertex3->p, &pPointAdd[2]);
	pPointAdd[0].color = _pVertex1->color;
	pPointAdd[0].specular = _pVertex1->specular;
	pPointAdd[0].uv = _pVertex1->uv;
	pPointAdd[1].color = _pVertex2->color;
	pPointAdd[1].specular = _pVertex2->specular;
	pPointAdd[1].uv = _pVertex2->uv;
	pPointAdd[2].color = _pVertex3->color;
	pPointAdd[2].specular = _pVertex3->specular;
	pPointAdd[2].uv = _pVertex3->uv;

	EERIEDRAWPRIM(Renderer::TriangleList, pPointAdd);
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
static void Cedric_ApplyLighting(EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, const ColorMod & colorMod) {

	/* Apply light on all vertices */
	for(int i = 0; i != obj->nb_bones; i++) {

		EERIE_QUAT *quat = &obj->bones[i].anim.quat;

		/* Get light value for each vertex */
		for(int v = 0; v != obj->bones[i].nb_idxvertices; v++) {
			size_t vertexIndex = obj->bones[i].idxvertices[v];

			Vec3f & position = eobj->vertexlist3[vertexIndex].v;
			Vec3f & normal = eobj->vertexlist[vertexIndex].norm;

			eobj->vertexlist3[vertexIndex].vert.color = ApplyLight(quat, position, normal, colorMod);
		}
	}
}

void MakeCLight(const EERIE_QUAT *quat, EERIE_3DOBJ * eobj, const ColorMod & colorMod) {
		
	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {

		Vec3f & position = eobj->vertexlist3[i].v;
		Vec3f & normal = eobj->vertexlist[i].norm;

		eobj->vertexlist3[i].vert.color = ApplyLight(quat, position, normal, colorMod);
	}
}

void MakeCLight2(const EERIE_QUAT *quat, EERIE_3DOBJ *eobj, long ii, const ColorMod & colorMod) {
	
	for(long i = 0; i < 3; i++) {
		size_t vertexIndex = eobj->facelist[ii].vid[i];

		Vec3f & position = eobj->vertexlist3[vertexIndex].v;
		Vec3f & normal = eobj->facelist[ii].norm;

		eobj->vertexlist3[vertexIndex].vert.color = ApplyLight(quat, position, normal, colorMod, 0.5f);
	}
}

void CalculateInterZMapp(EERIE_3DOBJ * _pobj3dObj, long lIdList, long * _piInd,
						 TextureContainer * _pTex, TexturedVertex * _pVertex) {

	SMY_ZMAPPINFO sZMappInfo;

	if(!ZMAPMODE || !_pTex->TextureRefinement)
		return;

	bool bUp = false;

	if(fabs(_pobj3dObj->vertexlist[_piInd[0]].norm.y) >= .9f
	   || fabs(_pobj3dObj->vertexlist[_piInd[1]].norm.y) >= .9f
	   || fabs(_pobj3dObj->vertexlist[_piInd[2]].norm.y) >= .9f) {
		bUp = true;
	}

	for(int iI=0; iI<3; iI++) {
		if(bUp) {
			sZMappInfo.uv[iI<<1]=(_pobj3dObj->vertexlist3[_piInd[iI]].v.x*( 1.0f / 50 ));
			sZMappInfo.uv[(iI<<1)+1]=(_pobj3dObj->vertexlist3[_piInd[iI]].v.z*( 1.0f / 50 ));
		} else {
			sZMappInfo.uv[iI<<1]=(_pobj3dObj->facelist[lIdList].u[iI]*4.f);
			sZMappInfo.uv[(iI<<1)+1]=(_pobj3dObj->facelist[lIdList].v[iI]*4.f);
		}

		float fDist = fdist(ACTIVECAM->orgTrans.pos, _pobj3dObj->vertexlist3[_piInd[iI]].v) - 80.f;

		if(fDist < 10.f)
			fDist = 10.f;

		sZMappInfo.color[iI] = (150.f - fDist) * 0.006666666f;

		if(sZMappInfo.color[iI] < 0.f)
			sZMappInfo.color[iI] = 0.f;

		sZMappInfo.pVertex[iI]=_pVertex[iI];
	}

	//optim
	if(sZMappInfo.color[0] != 0.f || sZMappInfo.color[1] != 0.f || sZMappInfo.color[2] != 0.f) {
		_pTex->TextureRefinement->vPolyInterZMap.push_back(sZMappInfo);
	}
}

void UpdateBbox3d(EERIE_3DOBJ *eobj, EERIE_3D_BBOX & box3D) {

	box3D.reset();

	for(size_t i = 0 ; i < eobj->vertexlist.size(); i++) {
		box3D.add(eobj->vertexlist3[i].v);
	}
}

void UpdateBbox2d(EERIE_3DOBJ *eobj, EERIE_2D_BBOX & box2D) {

	box2D.reset();

	for(size_t i = 0 ; i < eobj->vertexlist.size(); i++) {
		// Memorizes 2D Bounding Box using vertex min/max x,y pos
		if(eobj->vertexlist[i].vert.rhw > 0.f) {

			if ((eobj->vertexlist[i].vert.p.x >= -32000) &&
				(eobj->vertexlist[i].vert.p.x <= 32000) &&
				(eobj->vertexlist[i].vert.p.y >= -32000) &&
				(eobj->vertexlist[i].vert.p.y <= 32000))
			{
				box2D.add(eobj->vertexlist[i].vert.p);
			}
		}
	}
}

void DrawEERIEInter_ModelTransform(EERIE_3DOBJ *eobj, const TransformInfo &t) {

	for(size_t i = 0 ; i < eobj->vertexlist.size(); i++) {

		Vec3f temp = eobj->vertexlist[i].v;

		temp -= t.offset;
		temp *= t.scale;
		temp = TransformVertexQuat(t.rotation, temp);
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

void DrawEERIEInter_Render(EERIE_3DOBJ *eobj, const TransformInfo &t, Entity *io, float invisibility) {

	ColorMod colorMod;
	colorMod.updateFromEntity(io, !io);

	Vec3f tv = t.pos;

	if(io && (io->ioflags & IO_ITEM))
		tv.y -= 60.f;
	else
		tv.y -= 90.f;

	UpdateLlights(tv);


	// Precalc local lights for this object then interpolate
	if(!(io && (io->ioflags & IO_ANGULAR))) {
		MakeCLight(&t.rotation, eobj, colorMod);
	}

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		EERIE_FACE *eface = &eobj->facelist[i];

		long paf[3];
		paf[0]=eface->vid[0];
		paf[1]=eface->vid[1];
		paf[2]=eface->vid[2];

		//CULL3D
		Vec3f nrm = eobj->vertexlist3[paf[0]].v - ACTIVECAM->orgTrans.pos;

		if(!(eface->facetype & POLY_DOUBLESIDED)) {
			Vec3f normV10 = eobj->vertexlist3[paf[1]].v - eobj->vertexlist3[paf[0]].v;
			Vec3f normV20 = eobj->vertexlist3[paf[2]].v - eobj->vertexlist3[paf[0]].v;
			Vec3f normFace;
			normFace.x = (normV10.y * normV20.z) - (normV10.z * normV20.y);
			normFace.y = (normV10.z * normV20.x) - (normV10.x * normV20.z);
			normFace.z = (normV10.x * normV20.y) - (normV10.y * normV20.x);

			if(glm::dot(normFace, nrm) > 0.f)
				continue;
		}

		if(eface->texid < 0)
			continue;

		TextureContainer *pTex = eobj->texturecontainer[eface->texid];
		if(!pTex)
			continue;

		if(io && (io->ioflags & IO_ANGULAR))
			MakeCLight2(&t.rotation, eobj, i, colorMod);

		float fTransp = 0.f;
		TexturedVertex *tvList = GetNewVertexList(pTex, eface, invisibility, fTransp);

		tvList[0]=eobj->vertexlist[paf[0]].vert;
		tvList[1]=eobj->vertexlist[paf[1]].vert;
		tvList[2]=eobj->vertexlist[paf[2]].vert;

		tvList[0].uv.x=eface->u[0];
		tvList[0].uv.y=eface->v[0];
		tvList[1].uv.x=eface->u[1];
		tvList[1].uv.y=eface->v[1];
		tvList[2].uv.x=eface->u[2];
		tvList[2].uv.y=eface->v[2];

		// Treat WATER Polys (modify UVs)
		if(eface->facetype & POLY_WATER) {
			for(long k = 0; k < 3; k++) {
				tvList[k].uv.x=eface->u[k];
				tvList[k].uv.y=eface->v[k];
				ApplyWaterFXToVertex(&eobj->vertexlist[eface->vid[k]].v, &tvList[k], 0.3f);
			}
		}

		if(eface->facetype & POLY_GLOW) { // unaffected by light
			tvList[0].color=tvList[1].color=tvList[2].color=0xffffffff;
		} else { // Normal Illuminations
			for(long j = 0; j < 3; j++) {
				tvList[j].color=eobj->vertexlist3[paf[j]].vert.color;
			}
		}

		if(io && Project.improve) {
			int to=3;

			for(long k = 0; k < to; k++) {
				long lr=(tvList[k].color>>16) & 255;
				float ffr=(float)(lr);

				float dd = tvList[k].rhw;

				dd = clamp(dd, 0.f, 1.f);

				Vec3f & norm = eobj->vertexlist[paf[k]].norm;

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
				tvList[k].color = (0xff000000L | (lfr << 16) | (lfg << 8) | (lfb));
			}
		}

		for(long j = 0; j < 3; j++)
			eface->color[j]=Color::fromBGRA(tvList[j].color);

		// Transparent poly: storing info to draw later
		if((eface->facetype & POLY_TRANS) || invisibility > 0.f) {
			tvList[0].color = tvList[1].color = tvList[2].color = Color::gray(fTransp).toBGR();
		}

		if(io && (io->ioflags & IO_ZMAP)) {
			CalculateInterZMapp(eobj,i,paf,pTex,tvList);
		}

		// HALO HANDLING START
		if(io && (io->halo.flags & HALO_ACTIVE)) {

			float mdist=ACTIVECAM->cdepth;
			float ddist = mdist-fdist(t.pos, ACTIVECAM->orgTrans.pos);
			ddist = ddist/mdist;
			ddist = std::pow(ddist, 6);

			ddist = clamp(ddist, 0.25f, 0.9f);

			float tot=0;
			float _ffr[3];

			for(long o = 0; o < 3; o++) {
				Vec3f temporary3D;
				temporary3D = TransformVertexQuat(t.rotation, eobj->vertexlist[paf[o]].norm);

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
					TexturedVertex *vert = Halo_AddVertex();

					vert[0] = tvList[first];
					vert[1] = tvList[first];
					vert[2] = tvList[second];
					vert[3] = tvList[second];

					float siz = ddist * (io->halo.radius * 1.5f * (EEsin((arxtime.get_frame_time() + i) * .01f) * .1f + .7f)) * .6f;

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
				}
			}
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
		UpdateBbox2d(eobj, io->bbox2D);
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

	if(player.equiped[slot] != 0 && ValidIONum(player.equiped[slot])) {
		Entity * tio = entities[player.equiped[slot]];

		if(tio->halo.flags & HALO_ACTIVE) {
			halos.push_back(HaloRenderInfo(&tio->halo, selection));
		}
	}
}

//-----------------------------------------------------------------------------
extern long IN_BOOK_DRAW;

/* Render object */
static void Cedric_RenderObject(EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, Entity * io, const Vec3f & pos, float invisibility) {

	float MAX_ZEDE = 0.f;

	if(invisibility == 1.f)
		return;

	float ddist = 0.f;
	long need_halo = 0;

	Entity *use_io = io;

	if(!io && IN_BOOK_DRAW && eobj == entities.player()->obj)
		use_io = entities.player();

	arx_assert(use_io);

	std::vector<HaloRenderInfo> halos;

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
		ddist = mdist-fdist(ftrPos, ACTIVECAM->orgTrans.pos);
		ddist = ddist/mdist;
		ddist = std::pow(ddist, 6);

		ddist = clamp(ddist, 0.25f, 0.9f);

		Cedric_PrepareHalo(eobj, obj);
		need_halo = 1;

		MAX_ZEDE = 0.f;
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			if(eobj->vertexlist3[i].vert.rhw > 0.f)
				MAX_ZEDE = std::max(eobj->vertexlist3[i].vert.p.z, MAX_ZEDE);
		}
	}

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		EERIE_FACE *eface = &eobj->facelist[i];

		long paf[3];
		paf[0]=eface->vid[0];
		paf[1]=eface->vid[1];
		paf[2]=eface->vid[2];

		if((eface->facetype & POLY_HIDE) && !IN_BOOK_DRAW)
			continue;

		//CULL3D
		Vec3f nrm = eobj->vertexlist3[paf[0]].v - ACTIVECAM->orgTrans.pos;

		if(!(eface->facetype & POLY_DOUBLESIDED)) {
			Vec3f normV10 = eobj->vertexlist3[paf[1]].v - eobj->vertexlist3[paf[0]].v;
			Vec3f normV20 = eobj->vertexlist3[paf[2]].v - eobj->vertexlist3[paf[0]].v;
			Vec3f normFace;
			normFace.x = (normV10.y * normV20.z) - (normV10.z * normV20.y);
			normFace.y = (normV10.z * normV20.x) - (normV10.x * normV20.z);
			normFace.z = (normV10.x * normV20.y) - (normV10.y * normV20.x);

			if(glm::dot(normFace, nrm) > 0.f)
				continue;
		}

		if(eface->texid < 0)
			continue;

		TextureContainer *pTex = eobj->texturecontainer[eface->texid];
		if(!pTex)
			continue;

		float fTransp = 0.f;
		TexturedVertex *tvList = GetNewVertexList(pTex, eface, invisibility, fTransp);

		for(long n = 0 ; n < 3 ; n++) {
			tvList[n].p = eobj->vertexlist3[paf[n]].vert.p;

			// Nuky - this code takes 20% of the whole game performance O_O
			//        AFAIK it allows to correctly display the blue magic effects
			//        when one's hands are inside a wall. I've only managed to do that
			//        while in combat mode, looking straight down, and touching a wall
			//        So, for the greater good I think it's best to simply skip this test
			//const float IN_FRONT_DIVIDER = 0.75f;
			//const float IN_FRONT_DIVIDER_FEET = 0.998f;
			//if (FORCE_FRONT_DRAW)
			//{
			//	if (IsInGroup(eobj, paf[n], 1) != -1)
			//		tv[n].sz *= IN_FRONT_DIVIDER;
			//	else
			//		tv[n].sz *= IN_FRONT_DIVIDER_FEET;
			//}

			tvList[n].rhw	= eobj->vertexlist3[paf[n]].vert.rhw;
			tvList[n].uv.x	= eface->u[n];
			tvList[n].uv.y	= eface->v[n];
			tvList[n].color = eobj->vertexlist3[paf[n]].vert.color;
		}

		if((eface->facetype & POLY_TRANS) || invisibility > 0.f) {
			tvList[0].color = tvList[1].color = tvList[2].color = Color::gray(fTransp).toBGR();
		}

		if(io && (io->ioflags & IO_ZMAP))
			CalculateInterZMapp(eobj, i, paf, pTex, tvList);

		////////////////////////////////////////////////////////////////////////
		// HALO HANDLING START
		if(need_halo) {

			IO_HALO * curhalo = NULL;

			for(size_t h = 0; h < halos.size(); h++) {
				if(halos[h].selection == -1 || IsInSelection(eobj, paf[0], halos[h].selection) >= 0) {
					curhalo = halos[h].halo;
					break;
				}
			}

			if(!curhalo)
				continue;

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
					TexturedVertex *vert = Halo_AddVertex();

					vert[0] = tvList[first];
					vert[1] = tvList[first];
					vert[2] = tvList[second];
					vert[3] = tvList[second];

					vert[0].color = colors[first];
					vert[1].color = colors[first];
					vert[2].color = colors[second];
					vert[3].color = colors[second];

					float siz = ddist * (curhalo->radius * (EEsin((arxtime.get_frame_time() + i) * .01f) * .1f + 1.f)) * .6f;

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
				}
			}
		}
	}
}

void Cedric_AnimateDrawEntityRender(EERIE_3DOBJ *eobj, const Vec3f & pos, Entity *io, float invisibility) {

	EERIE_C_DATA *obj = eobj->c_data;

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

	UpdateLlights(tv);

	Cedric_ApplyLighting(eobj, obj, colorMod);

	Cedric_RenderObject(eobj, obj, io, pos, invisibility);

	// Now we can render Linked Objects
	for (long k = 0; k < eobj->nblinked; k++) {
		EERIE_LINKED & link = eobj->linked[k];

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
			eobj->c_data->bones[link.lgroup].anim.quat,
			link.io ? link.io->scale : 1.f,
			link.obj->vertexlist[link.lidx2].v - link.obj->vertexlist[link.obj->origin].v);

		DrawEERIEInter(link.obj, t, link.io, true, invisibility);
	}
}

static void CalcTranslation(ANIM_USE * animuse, Vec3f & ftr) {
	if(!animuse || !animuse->cur_anim)
		return;

	EERIE_ANIM	*eanim = animuse->cur_anim->anims[animuse->altidx_cur];

	if(!eanim)
		return;

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
		EERIE_FRAME *sFrame = &eanim->frames[animuse->fr];
		EERIE_FRAME *eFrame = &eanim->frames[animuse->fr+1];

		// Linear interpolation of object translation (MOVE)
		ftr = sFrame->translate + (eFrame->translate - sFrame->translate) * animuse->pour;
	}
}

static void StoreEntityMovement(Entity * io, Vec3f & ftr, float scale) {

	if(!io)
		return;

	Vec3f ftr2 = Vec3f_ZERO;

	if(ftr != Vec3f_ZERO) {
		ftr *= scale;

		float temp;
		if (io == entities.player()) {
			temp = radians(MAKEANGLE(180.f - player.angle.getPitch()));
		} else {
			temp = radians(MAKEANGLE(180.f - io->angle.getPitch()));
		}

		YRotatePoint(&ftr, &ftr2, std::cos(temp), std::sin(temp));

		// stores Translations for a later use
		io->move = ftr2;
	}

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
static void Cedric_AnimateObject(EERIE_C_DATA * obj, ANIM_USE * animlayer)
{
	std::vector<unsigned char> grps(obj->nb_bones);

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
		int l = std::min(obj->nb_bones - 1, eanim->nb_groups - 1);

		for(int j = l; j >= 0; j--) {
			if(grps[j])
				continue;

			EERIE_GROUP * sGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)];
			EERIE_GROUP * eGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)+eanim->nb_groups];

			if(!eanim->voidgroups[j])
				grps[j] = 1;

			if(eanim->nb_key_frames != 1) {
				EERIE_BONE & bone = obj->bones[j];

				BoneTransform temp;

				temp.quat = Quat_Slerp(sGroup->quat, eGroup->quat, animuse->pour);
				temp.trans = sGroup->translate + (eGroup->translate - sGroup->translate) * animuse->pour;
				temp.scale = sGroup->zoom + (eGroup->zoom - sGroup->zoom) * animuse->pour;

				bone.init.quat = Quat_Multiply(bone.init.quat, temp.quat);
				bone.init.trans = temp.trans + bone.transinit_global;
				bone.init.scale = temp.scale;
			}
		}
	}
}

void Cedric_BlendAnimation(EERIE_C_DATA & rig, AnimationBlendStatus * animBlend) {

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

	for(long i = 0; i < rig.nb_bones; i++) {
		EERIE_BONE * bone = &rig.bones[i];

		bone->init.quat = Quat_Slerp(bone->last.quat, bone->init.quat, timm);

		bone->init.trans = bone->last.trans + (bone->init.trans - bone->last.trans) * timm;
		bone->init.scale = bone->last.scale + (bone->init.scale - bone->last.scale) * timm;
	}
}

/*!
 * Apply transformations on all bones
 */
static void Cedric_ConcatenateTM(EERIE_C_DATA & rig, const TransformInfo & t) {

	for(int i = 0; i != rig.nb_bones; i++) {
		EERIE_BONE * bone = &rig.bones[i];

		if(bone->father >= 0) { // Child Bones
			EERIE_BONE * parent = &rig.bones[bone->father];
			// Rotation
			bone->anim.quat = Quat_Multiply(parent->anim.quat, bone->init.quat);

			// Translation
			bone->anim.trans = bone->init.trans * parent->anim.scale;
			bone->anim.trans = TransformVertexQuat(parent->anim.quat, bone->anim.trans);
			bone->anim.trans = parent->anim.trans + bone->anim.trans;

			// Scale
			bone->anim.scale = (bone->init.scale + Vec3f_ONE) * parent->anim.scale;
		} else { // Root Bone
			// Rotation
			bone->anim.quat = Quat_Multiply(t.rotation, bone->init.quat);

			// Translation
			Vec3f vt1 = bone->init.trans + t.offset;
			bone->anim.trans = TransformVertexQuat(t.rotation, vt1);
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

	EERIE_C_DATA & rig = *eobj->c_data;

	// Transform & project all vertices
	for(long i = 0; i != rig.nb_bones; i++) {
		EERIE_BONE & bone = rig.bones[i];

		EERIEMATRIX	 matrix;

		MatrixFromQuat(&matrix, &bone.anim.quat);

		// Apply Scale
		matrix._11 *= bone.anim.scale.x;
		matrix._12 *= bone.anim.scale.x;
		matrix._13 *= bone.anim.scale.x;

		matrix._21 *= bone.anim.scale.y;
		matrix._22 *= bone.anim.scale.y;
		matrix._23 *= bone.anim.scale.y;

		matrix._31 *= bone.anim.scale.z;
		matrix._32 *= bone.anim.scale.z;
		matrix._33 *= bone.anim.scale.z;

		Vec3f vector = bone.anim.trans;

		for(int v = 0; v != bone.nb_idxvertices; v++) {
			long index = bone.idxvertices[v];

			EERIE_3DPAD * inVert  = &eobj->vertexlocal[index];
			EERIE_VERTEX * outVert = &eobj->vertexlist3[index];

			TransformVertexMatrix(&matrix, inVert, &outVert->v);
			outVert->v += vector;

			outVert->vert.p = outVert->v;
		}
	}

	if(eobj->sdata) {
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			eobj->vertexlist[i].vert.p = eobj->vertexlist3[i].v - pos;
		}
	}
}

void Cedric_ViewProjectTransform(Entity *io, EERIE_3DOBJ *eobj) {

	EERIE_2D_BBOX box2D;
	box2D.reset();

	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		EERIE_VERTEX * outVert = &eobj->vertexlist3[i];

		Vec3f tempWorld;
		EE_RT(outVert->v, tempWorld);
		EE_P(&tempWorld, &outVert->vert);

		// Updates 2D Bounding Box
		if(outVert->vert.rhw > 0.f) {
			box2D.add(outVert->vert.p);
		}
	}

	if(io) {
		io->bbox2D = box2D;
	}
}

/*!
 * \brief Apply animation and draw object
 */
void Cedric_AnimateDrawEntity(EERIE_C_DATA & rig, ANIM_USE * animlayer, EERIE_EXTRA_ROTATE * extraRotation, AnimationBlendStatus * animBlend, EERIE_EXTRA_SCALE & extraScale) {

	// Initialize the rig
	for(long i = 0; i != rig.nb_bones; i++) {
		EERIE_BONE & bone = rig.bones[i];

		Quat_Init(&bone.init.quat);
		bone.init.trans = bone.transinit_global;
	}

	// Apply Extra Rotations in Local Space
	if(extraRotation) {
		for(long k = 0; k < MAX_EXTRA_ROTATE; k++) {
			long i = extraRotation->group_number[k];

			if(i >= 0) {
				Anglef vt1;
				vt1.setYaw(radians(extraRotation->group_rotate[k].getRoll()));
				vt1.setPitch(radians(extraRotation->group_rotate[k].getPitch()));
				vt1.setRoll(radians(extraRotation->group_rotate[k].getYaw()));

				QuatFromAngles(&rig.bones[i].init.quat, &vt1);
			}
		}
	}

	// Perform animation in Local space
	Cedric_AnimateObject(&rig, animlayer);

	if(extraScale.groupIndex != -1) {
		EERIE_BONE & bone = rig.bones[extraScale.groupIndex];

		bone.init.scale += extraScale.scale;
	}

	// Check for Animation Blending in Local space
	if(animBlend) {
		// Is There any Between-Animations Interpolation to make ?
		Cedric_BlendAnimation(rig, animBlend);

		for(long i = 0; i < rig.nb_bones; i++) {
			rig.bones[i].last = rig.bones[i].init;
		}
	}
}

void EERIEDrawAnimQuatUpdate(EERIE_3DOBJ *eobj, ANIM_USE * animlayer,const Anglef & angle, const Vec3f & pos, unsigned long time, Entity *io, bool update_movement) {

	if(io) {
		float speedfactor = io->basespeed + io->speed_modif;

		if(speedfactor < 0)
			speedfactor = 0;

		time = time * speedfactor;
	}

	if(time > 0) {
		for(size_t count = 0; count < MAX_ANIM_LAYERS; count++) {
			ANIM_USE * animuse = &animlayer[count];
			if(animuse->cur_anim)
				PrepareAnim(animuse, time, io);
		}
	}

	// Reset Frame Translate
	Vec3f ftr = Vec3f_ZERO;

	// Set scale and invisibility factors
	float scale = Cedric_GetScale(io);

	// Only layer 0 controls movement
	CalcTranslation(&animlayer[0], ftr);


	if(update_movement)
		StoreEntityMovement(io, ftr, scale);

	if(io && io != entities.player() && !Cedric_IO_Visible(io->pos))
		return;

	EERIE_QUAT rotation;

	bool isNpc = io && (io->ioflags & IO_NPC);
	worldAngleToQuat(&rotation, angle, isNpc);


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

	arx_assert(eobj->c_data);
	EERIE_C_DATA & skeleton = *eobj->c_data;

	Cedric_AnimateDrawEntity(skeleton, animlayer, extraRotation, animBlend, extraScale);

	// Build skeleton in Object Space
	TransformInfo t(pos, rotation, scale, ftr);
	Cedric_ConcatenateTM(skeleton, t);

	Cedric_TransformVerts(eobj, pos);
	if(io) {
		UpdateBbox3d(eobj, io->bbox3D);
	}

	Cedric_ViewProjectTransform(io, eobj);
}

void EERIEDrawAnimQuatRender(EERIE_3DOBJ *eobj, const Vec3f & pos, Entity *io, bool render, float invisibility) {

	if(io && io != entities.player() && !Cedric_IO_Visible(io->pos))
		return;

	bool isFightingNpc = io &&
						 (io->ioflags & IO_NPC) &&
						 (io->_npcdata->behavior & BEHAVIOUR_FIGHT) &&
						 closerThan(io->pos, player.pos, 240.f);

	if(!isFightingNpc && ARX_SCENE_PORTAL_ClipIO(io, pos))
		return;

	if(render)
		Cedric_AnimateDrawEntityRender(eobj, pos, io, invisibility);
}

void EERIEDrawAnimQuat(EERIE_3DOBJ *eobj, ANIM_USE * animlayer,const Anglef & angle, const Vec3f & pos, unsigned long time, Entity *io, bool render, bool update_movement, float invisibility) {

	EERIEDrawAnimQuatUpdate(eobj, animlayer,angle, pos, time, io, update_movement);
	EERIEDrawAnimQuatRender(eobj, pos, io, render, invisibility);
}

void AnimatedEntityUpdate(Entity * entity, float time) {
	EERIEDrawAnimQuatUpdate(entity->obj, entity->animlayer, entity->angle,
							entity->pos, time, entity, true);
}

void AnimatedEntityRender(Entity * entity, float invisibility) {

	Cedric_ViewProjectTransform(entity, entity->obj);
	EERIEDrawAnimQuatRender(entity->obj, entity->pos, entity, true, invisibility);
}
