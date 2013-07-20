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
#include "graphics/particle/ParticleEffects.h"

#include "math/Angle.h"
#include "math/Vector3.h"

#include "platform/Platform.h"

#include "scene/Light.h"
#include "scene/GameSound.h"
#include "scene/Scene.h"
#include "scene/Interactive.h"

using std::min;
using std::max;

extern float dists[];
extern long BH_MODE;

extern TextureContainer TexSpecialColor;

extern long TSU_TEST_NB;
extern long TSU_TEST_NB_LIGHT;

//#define USE_SOFTWARE_CLIPPING
#ifdef USE_SOFTWARE_CLIPPING
	float SOFTNEARCLIPPZ=1.f;
#endif

/* Init bounding box */
inline	static	void	Cedric_ResetBoundingBox(Entity * io)
{
	// resets 2D Bounding Box
	BBOXMIN.y = BBOXMIN.x = 32000;
	BBOXMAX.y = BBOXMAX.x = -32000;
	// Resets 3D Bounding Box
	ResetBBox3D(io);
}

extern float INVISIBILITY_OVERRIDE;
extern bool EXTERNALVIEW;
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





// Animate skeleton
static void Cedric_AnimateObject(Entity *io, EERIE_3DOBJ *eobj, ANIM_USE *animuse)
{
	EERIE_C_DATA	* obj = eobj->c_data;

	std::vector<unsigned char> grps(eobj->nbgroups);

	for(long count = MAX_ANIM_LAYERS - 1; count >= 0; count--) {
		EERIE_QUAT		t, temp;

		if(!io) {
			count = -1;
		} else {
			animuse = &io->animlayer[count];
		}
		
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
		int l = min(eobj->nbgroups - 1, eanim->nb_groups - 1);

		for(int j = l; j >= 0; j--) {
			if(grps[j])
				continue;

			EERIE_GROUP * sGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)];
			EERIE_GROUP * eGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)+eanim->nb_groups];

			if(!eanim->voidgroups[j])
				grps[j] = 1;

			if(eanim->nb_key_frames != 1) {
				Quat_Slerp(&t, &sGroup->quat, &eGroup->quat, animuse->pour);
				Quat_Copy(&temp, &obj->bones[j].quatinit);
				Quat_Multiply(&obj->bones[j].quatinit, &temp, &t);

				Vec3f vect = sGroup->translate + (eGroup->translate - sGroup->translate) * animuse->pour;
				obj->bones[j].transinit = vect + obj->bones[j].transinit_global;

				Vec3f scale = sGroup->zoom + (eGroup->zoom - sGroup->zoom) * animuse->pour;
				if(BH_MODE && j == eobj->fastaccess.head_group) {
					scale += Vec3f::ONE;
				}
				
				obj->bones[j].scaleinit = scale;
			}
		}
	}
}

/* Apply transformations on all bones */
static void Cedric_ConcatenateTM(EERIE_C_DATA *obj, EERIE_QUAT *rotation, Vec3f *pos, Vec3f &ftr, float g_scale) {

	if(!obj)
		return;

	for(int i = 0; i != obj->nb_bones; i++) {
		EERIE_BONE * bone = &obj->bones[i];

		if(bone->father >= 0) { // Child Bones
			EERIE_BONE * parent = &obj->bones[bone->father];
			// Rotation
			Quat_Multiply(&bone->quatanim, &parent->quatanim, &bone->quatinit);

			// Translation
			bone->transanim = bone->transinit * parent->scaleanim;
			TransformVertexQuat(&parent->quatanim, &bone->transanim, &bone->transanim);
			bone->transanim = parent->transanim + bone->transanim;

			/* Scale */
			bone->scaleanim = (bone->scaleinit + Vec3f::ONE) * parent->scaleanim;
		} else { // Root Bone
			// Rotation
			Quat_Multiply(&bone->quatanim, rotation, &bone->quatinit);

			// Translation
			Vec3f vt1 = bone->transinit + ftr;
			TransformVertexQuat(rotation, &vt1, &bone->transanim);
			bone->transanim *= g_scale;
			bone->transanim = *pos + bone->transanim;

			// Compute Global Object Scale AND Global Animation Scale
			bone->scaleanim = (bone->scaleinit + Vec3f::ONE) * g_scale;
		}
	}
}

void EE_RT(Vec3f * in, Vec3f * out);
void EE_P(Vec3f * in, TexturedVertex * out);

/* Transform object vertices  */
void Cedric_TransformVerts(Entity *io, EERIE_3DOBJ *eobj, EERIE_C_DATA *obj, Vec3f *pos) {

 	/* Transform & project all vertices */
	for(long i = 0; i != obj->nb_bones; i++) {
		EERIEMATRIX	 matrix;

		MatrixFromQuat(&matrix, &obj->bones[i].quatanim);
		Vec3f vector = obj->bones[i].transanim;
		
		// Apply Scale
		matrix._11 *= obj->bones[i].scaleanim.x;
		matrix._12 *= obj->bones[i].scaleanim.x;
		matrix._13 *= obj->bones[i].scaleanim.x;
		
		matrix._21 *= obj->bones[i].scaleanim.y;
		matrix._22 *= obj->bones[i].scaleanim.y;
		matrix._23 *= obj->bones[i].scaleanim.y;
		
		matrix._31 *= obj->bones[i].scaleanim.z;
		matrix._32 *= obj->bones[i].scaleanim.z;
		matrix._33 *= obj->bones[i].scaleanim.z;
		
		for(int v = 0; v != obj->bones[i].nb_idxvertices; v++) {
			EERIE_3DPAD * inVert  = &eobj->vertexlocal[obj->bones[i].idxvertices[v]];
			EERIE_VERTEX * outVert = &eobj->vertexlist3[obj->bones[i].idxvertices[v]];
			TransformVertexMatrix(&matrix, inVert, &outVert->v);
			outVert->v += vector;
			outVert->vert.p = outVert->v;
		}
	}

	if(eobj->cdata && eobj->sdata) {
		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			eobj->vertexlist[i].vert.p = eobj->vertexlist3[i].v - *pos;
		}
	}

	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		EERIE_VERTEX * outVert = &eobj->vertexlist3[i];
		AddToBBox3D(io, &outVert->v);
		EE_RT(&outVert->vert.p, &outVert->vworld);
		EE_P(&outVert->vworld, &outVert->vert);

		// Updates 2D Bounding Box
		if(outVert->vert.rhw > 0.f) {
			BBOXMIN.x = min(BBOXMIN.x, outVert->vert.p.x);
			BBOXMAX.x = max(BBOXMAX.x, outVert->vert.p.x);
			BBOXMIN.y = min(BBOXMIN.y, outVert->vert.p.y);
			BBOXMAX.y = max(BBOXMAX.y, outVert->vert.p.y);
		}
	}

	if(io) {
		io->bbox1.x = (short)BBOXMIN.x;
		io->bbox2.x = (short)BBOXMAX.x;
		io->bbox1.y = (short)BBOXMIN.y;
		io->bbox2.y = (short)BBOXMAX.y;
	}
}

extern long TRAP_DETECT;
extern long TRAP_SECRET;

extern float GLOBAL_LIGHT_FACTOR;

//TODO Move somewhere else
void Cedric_ApplyLightingFirstPartRefactor(Entity *io, Color3f &special_color, long &special_color_flag) {

	if(!io)
		return;

		special_color = Color3f::black;
		special_color_flag = 0;

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
			trappercent = (float)TRAP_DETECT - (float)io->_fixdata->trapvalue;
			if(trappercent > 0.f) {
				trappercent = 0.6f + trappercent * ( 1.0f / 100 );
				trappercent = clamp(trappercent, 0.6f, 1.f);
			}
		}

		if((io->ioflags & IO_FIX) && io->secretvalue > -1) {
			secretpercent = (float)TRAP_SECRET - (float)io->secretvalue;
			if(secretpercent > 0.f) {
				secretpercent = 0.6f + secretpercent * ( 1.0f / 100 );
				secretpercent = clamp(secretpercent, 0.6f, 1.f);
			}
		}

		if(poisonpercent > 0.f) {
			special_color_flag = 1;
			special_color = Color3f::green;
		}

		if(trappercent > 0.f) {
			special_color_flag = 1;
			special_color = Color3f(trappercent, 1.f - trappercent, 1.f - trappercent);
		}

		if(secretpercent > 0.f) {
			special_color_flag = 1;
			special_color = Color3f(1.f - secretpercent, 1.f - secretpercent, secretpercent);
		}

		if(io->ioflags & IO_FREEZESCRIPT) {
			special_color = Color3f::blue;
			special_color_flag = 1;
		}

		if(io->sfx_flag & SFX_TYPE_YLSIDE_DEATH) {
			if(io->show == SHOW_FLAG_TELEPORTING) {
				float fTime = io->sfx_time + framedelay;
				io->sfx_time = checked_range_cast<unsigned long>(fTime);

				if (io->sfx_time >= (unsigned long)(arxtime))
					io->sfx_time = (unsigned long)(arxtime);
			} else {
				special_color_flag = 1;
				float elapsed = float(arxtime) - io->sfx_time;

				if(elapsed > 0.f) {
					if(elapsed < 3000.f) { // 5 seconds to red
						float ratio = elapsed * (1.0f / 3000);
						special_color = Color3f(1.f, 1.f - ratio, 1.f - ratio);
						AddRandomSmoke(io, 1);
					} else if(elapsed < 6000.f) { // 5 seconds to White
						float ratio = (elapsed - 3000.f) * (1.0f / 3000);
						special_color = Color3f(1.f, ratio, ratio);
						AddRandomSmoke(io, 2);
					} else { // SFX finish
						special_color_flag = 0;
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
								ARX_INTERACTIVE_DestroyIO(io);
							}
						}
					}
				}
			}
		}
}

/* Object dynamic lighting */
static void Cedric_ApplyLighting(EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, Entity * io, Vec3f * pos, Color3f &special_color, long &special_color_flag) {
		
	Color3f infra = Color3f::black;
	if(Project.improve) {
		infra = (io) ? io->infracolor : Color3f(0.6f, 0.f, 1.f);
	}
	
	/* Get nearest lights */
	Vec3f tv = *pos;
	
	if(io && io->obj->fastaccess.view_attach >= 0 && io->obj->fastaccess.head_group_origin != -1)
		tv.y = io->obj->vertexlist3[io->obj->fastaccess.head_group_origin].v.y + 10;
	else
		tv.y -= 90.f;

	llightsInit();

	for(int i = 0; i < TOTIOPDL; i++) {
		if(IO_PDL[i]->fallend + 500.f < 0)
			continue;

		Insertllight(IO_PDL[i], dist(IO_PDL[i]->pos, tv));
	}

	for(int i = 0; i < TOTPDL; i++) {
		if(PDL[i]->fallend + 500.f < 0)
			continue;

		Insertllight(PDL[i], dist(PDL[i]->pos, tv));
	}

	/* Apply light on all vertices */
	for(int i = 0; i != obj->nb_bones; i++) {

		EERIE_QUAT *qt1 = &obj->bones[i].quatanim;

		/* Get light value for each vertex */
		for(int v = 0; v != obj->bones[i].nb_idxvertices; v++) {
			Color3f tempColor;

			// Ambient light
			if(io && (io->ioflags & (IO_NPC | IO_ITEM)))
				tempColor = Color3f::gray(NPC_ITEMS_AMBIENT_VALUE_255);
			else
				tempColor = ACTIVEBKG->ambient255;

			Vec3f posVert = eobj->vertexlist[obj->bones[i].idxvertices[v]].norm;

			// Dynamic lights
			for(int l = 0; l != MAX_LLIGHTS; l++) {
				EERIE_LIGHT *Cur_llights = llights[l];

				if(!Cur_llights)
					break;

				Vec3f tl = Cur_llights->pos - eobj->vertexlist3[obj->bones[i].idxvertices[v]].v;
				float distance = ffsqrt(tl.lengthSqr());

				if(distance < Cur_llights->fallend) {
					tl *= 1.f / distance;

					Vec3f Cur_vTLights;
					TransformInverseVertexQuat(qt1, &tl, &Cur_vTLights);

					float cosangle = dot(posVert, Cur_vTLights);

					// If light visible
					if(cosangle > 0.f) {
						// Evaluate its intensity depending on the distance Light<->Object
						if(distance <= Cur_llights->fallstart) {
							cosangle *= Cur_llights->precalc;
						} else {
							float p = ((Cur_llights->fallend - distance) * Cur_llights->falldiffmul);

							if(p <= 0.f)
								cosangle = 0.f;
							else
								cosangle *= p * Cur_llights->precalc;
						}

						tempColor.r += Cur_llights->rgb255.r * cosangle;
						tempColor.g += Cur_llights->rgb255.g * cosangle;
						tempColor.b += Cur_llights->rgb255.b * cosangle;
					}
				}
			}

			/* Fake adjust */
			if(Project.improve) {
				tempColor.r *= infra.r;
				tempColor.g *= infra.g;
				tempColor.b *= infra.b;
			}

			if(special_color_flag & 1) {
				tempColor.r *= special_color.r;
				tempColor.g *= special_color.g;
				tempColor.b *= special_color.b;
			} else if(special_color_flag & 4) { // HIGHLIGHT
				tempColor.r += special_color.r;
				tempColor.g += special_color.g;
				tempColor.b += special_color.b;
			}

			u8 ir = clipByte255(tempColor.r);
			u8 ig = clipByte255(tempColor.g);
			u8 ib = clipByte255(tempColor.b);

			eobj->vertexlist3[obj->bones[i].idxvertices[v]].vert.color = (0xFF000000L | (ir << 16) | (ig << 8) | (ib));
		}
	}
}

void Cedric_PrepareHalo(EERIE_3DOBJ * eobj, EERIE_C_DATA * obj) {
	Vec3f cam_vector, t_vector;
	cam_vector.x = -EEsin(radians(ACTIVECAM->angle.b)) * EEcos(radians(ACTIVECAM->angle.a));
	cam_vector.y = EEsin(radians(ACTIVECAM->angle.a));
	cam_vector.z = EEcos(radians(ACTIVECAM->angle.b)) * EEcos(radians(ACTIVECAM->angle.a));

	/* Apply light on all vertices */
	for (long i = 0; i != obj->nb_bones; i++)
	{
		EERIE_QUAT	qt1;
		Quat_Copy(&qt1, &obj->bones[i].quatanim);
		TransformInverseVertexQuat(&qt1, &cam_vector, &t_vector);

		/* Get light value for each vertex */
		for (long v = 0; v != obj->bones[i].nb_idxvertices; v++)
		{
			EERIE_3DPAD *	inVert;
			//inVert  = &eobj->normallocal[obj->bones[i].idxvertices[v]];
			inVert  = (EERIE_3DPAD *)&eobj->vertexlist[obj->bones[i].idxvertices[v]].norm;
			/* Get cos angle between light and vertex norm */
			eobj->vertexlist3[obj->bones[i].idxvertices[v]].norm.z =
			    (inVert->x * t_vector.x + inVert->y * t_vector.y + inVert->z * t_vector.z);

		}
	}
}

#ifdef USE_SOFTWARE_CLIPPING
//-----------------------------------------------------------------------------
void ARX_ClippZ(TexturedVertex * _pA, TexturedVertex * _pB, EERIE_VERTEX * _pVertexA, EERIE_VERTEX * _pVertexB, TexturedVertex * _pOut)
{
	Vec3f e3dTemp;

	float fDenom	=	(SOFTNEARCLIPPZ - _pVertexB->vworld.z) / (_pVertexA->vworld.z - _pVertexB->vworld.z);
	e3dTemp.x		=	(_pVertexA->vworld.x - _pVertexB->vworld.x) * fDenom + _pVertexB->vworld.x;
	e3dTemp.y		=	(_pVertexA->vworld.y - _pVertexB->vworld.y) * fDenom + _pVertexB->vworld.y;
	e3dTemp.z		=	SOFTNEARCLIPPZ ;

	float fRA, fGA, fBA;
	float fRB, fGB, fBB;

	fRA = checked_range_cast<float>((_pA->color >> 16) & 255);
	fGA = checked_range_cast<float>((_pA->color >> 8) & 255);
	fBA = checked_range_cast<float>(_pA->color & 255);
	fRB = checked_range_cast<float>((_pB->color >> 16) & 255);
	fGB = checked_range_cast<float>((_pB->color >> 8) & 255);
	fBB = checked_range_cast<float>(_pB->color & 255);

	float fRC, fGC, fBC;
	fRC = (fRA - fRB) * fDenom + fRB;
	fGC = (fGA - fGB) * fDenom + fGB;
	fBC = (fBA - fBB) * fDenom + fBB;

	_pOut->color	=	(((int)fRC) << 16) | (((int)fGC) << 8) | ((int)fBC);
	_pOut->uv		=	(_pA->uv - _pB->uv) * fDenom + _pB->uv;

	EE_P(&e3dTemp, _pOut);
}

//-----------------------------------------------------------------------------
void ARX_DrawPrimitive_ClippZ(TexturedVertex * _pVertexA, TexturedVertex * _pVertexB, TexturedVertex * _pOut, float _fAdd = 0.f);
void ARX_DrawPrimitive_ClippZ(TexturedVertex * _pVertexA, TexturedVertex * _pVertexB, TexturedVertex * _pOut, float _fAdd)
{
	Vec3f e3dTemp;
	float fDenom = ((SOFTNEARCLIPPZ + _fAdd) - _pVertexB->p.z) / (_pVertexA->p.z - _pVertexB->p.z);
	e3dTemp.x = (_pVertexA->p.x - _pVertexB->p.x) * fDenom + _pVertexB->p.x;
	e3dTemp.y = (_pVertexA->p.y - _pVertexB->p.y) * fDenom + _pVertexB->p.y;
	e3dTemp.z = SOFTNEARCLIPPZ + _fAdd;

	float fRA, fGA, fBA;
	float fRB, fGB, fBB;

	fRA = checked_range_cast<float>((_pVertexA->color >> 16) & 255);
	fGA = checked_range_cast<float>((_pVertexA->color >> 8) & 255);
	fBA = checked_range_cast<float>(_pVertexA->color & 255);
	fRB = checked_range_cast<float>((_pVertexB->color >> 16) & 255);
	fGB = checked_range_cast<float>((_pVertexB->color >> 8) & 255);
	fBB = checked_range_cast<float>(_pVertexB->color & 255);

	float fRC, fGC, fBC;
	fRC = (fRA - fRB) * fDenom + fRB;
	fGC = (fGA - fGB) * fDenom + fGB;
	fBC = (fBA - fBB) * fDenom + fBB;

	_pOut->color	= (((int) fRC) << 16) | (((int)fGC) << 8) | ((int) fBC);
	_pOut->uv		= (_pVertexA->uv - _pVertexB->uv) * fDenom + _pVertexB->uv;

	EE_P(&e3dTemp, _pOut);
}
#endif

//-----------------------------------------------------------------------------
TexturedVertex * GetNewVertexList(EERIE_FACE * _pFace, float _fInvisibility, TextureContainer * _pTex) {
	
	if(!(_pFace->facetype & POLY_TRANS) && !(_fInvisibility > 0.f)) {
		return PushVertexInTableCull(_pTex);
	}
	
	float fTransp;
	
	if(_fInvisibility > 0.f) {
		fTransp = 2.f - _fInvisibility;
	} else {
		fTransp = _pFace->transval;
	}
	
	if(fTransp >= 2.f) { //MULTIPLICATIVE
		return PushVertexInTableCull_TMultiplicative(_pTex);
	} else if(fTransp >= 1.f) { //ADDITIVE
		return PushVertexInTableCull_TAdditive(_pTex);
	} else if(fTransp > 0.f) { //NORMAL TRANS
		return PushVertexInTableCull_TNormalTrans(_pTex);
	} else { //SUBTRACTIVE
		return PushVertexInTableCull_TSubstractive(_pTex);
	}
}

#ifdef USE_SOFTWARE_CLIPPING

int ARX_SoftClippZ(EERIE_VERTEX * _pVertex1, EERIE_VERTEX * _pVertex2, EERIE_VERTEX * _pVertex3, TexturedVertex ** _ptV, EERIE_FACE * _pFace, float _fInvibility, TextureContainer * _pTex, bool _bZMapp, EERIE_3DOBJ * _pObj, int _iNumFace, long * _pInd, Entity * _pioInteractive, bool _bNPC, long _lSpecialColorFlag, Color3f * _pRGB) {
	
	int iPointAdd = 3;
	int iClipp = 0;

	if ((_pVertex1->vworld.z) < SOFTNEARCLIPPZ) iClipp |= 1;

	if ((_pVertex2->vworld.z) < SOFTNEARCLIPPZ) iClipp |= 2;

	if ((_pVertex3->vworld.z) < SOFTNEARCLIPPZ) iClipp |= 4;

	TexturedVertex ClippZ1, ClippZ2;
	TexturedVertex * pPointAdd = NULL;
	TexturedVertex * ptV = *_ptV;

	switch (iClipp)
	{
		case 1:						//pt1 outside
			ARX_ClippZ(&ptV[0], &ptV[1], _pVertex1, _pVertex2, &ClippZ1);
			ARX_ClippZ(&ptV[0], &ptV[2], _pVertex1, _pVertex3, &ClippZ2);
			pPointAdd = GetNewVertexList(_pFace,
			                                _fInvibility,
			                                _pTex);
			ptV = pPointAdd - 3;

			if (pPointAdd)
			{
				pPointAdd[0] = ClippZ1;
				pPointAdd[1] = ptV[1];
				pPointAdd[2] = ClippZ2;
			}

			ptV[0] = ClippZ2;
			iPointAdd = 6;
			break;
		case 2:						//pt2 outside
			ARX_ClippZ(&ptV[1], &ptV[2], _pVertex2, _pVertex3, &ClippZ1);
			ARX_ClippZ(&ptV[1], &ptV[0], _pVertex2, _pVertex1, &ClippZ2);
			pPointAdd = GetNewVertexList(_pFace,
			                                _fInvibility,
			                                _pTex);
			ptV = pPointAdd - 3;

			if (pPointAdd)
			{
				pPointAdd[0] = ptV[2];
				pPointAdd[1] = ClippZ1;
				pPointAdd[2] = ClippZ2;
			}

			ptV[1] = ClippZ2;
			iPointAdd = 6;
			break;
		case 4:						//pt3 outside
			ARX_ClippZ(&ptV[2], &ptV[0], _pVertex3, _pVertex1, &ClippZ1);
			ARX_ClippZ(&ptV[2], &ptV[1], _pVertex3, _pVertex2, &ClippZ2);
			pPointAdd = GetNewVertexList(_pFace,
			                                _fInvibility,
			                                _pTex);
			ptV = pPointAdd - 3;

			if (pPointAdd)
			{
				pPointAdd[0] = ptV[0];
				pPointAdd[1] = ClippZ2;
				pPointAdd[2] = ClippZ1;
			}

			ptV[2] = ClippZ2;
			iPointAdd = 6;
			break;
		case 3:						//pt1_2 outside
			ARX_ClippZ(&ptV[0], &ptV[2], _pVertex1, _pVertex3, &ClippZ1);
			ARX_ClippZ(&ptV[1], &ptV[2], _pVertex2, _pVertex3, &ClippZ2);
			ptV[0] = ClippZ1;
			ptV[1] = ClippZ2;
			break;
		case 5:						//pt1_3 outside
			ARX_ClippZ(&ptV[0], &ptV[1], _pVertex1, _pVertex2, &ClippZ1);
			ARX_ClippZ(&ptV[2], &ptV[1], _pVertex3, _pVertex2, &ClippZ2);
			ptV[0] = ClippZ1;
			ptV[2] = ClippZ2;
			break;
		case 6:						//pt2_3 outside
			ARX_ClippZ(&ptV[2], &ptV[0], _pVertex3, _pVertex1, &ClippZ1);
			ARX_ClippZ(&ptV[1], &ptV[0], _pVertex2, _pVertex1, &ClippZ2);
			ptV[1] = ClippZ1;
			ptV[2] = ClippZ2;
			break;
		case 7:
			return 0;
	}

	if (pPointAdd)
	{
		*_ptV = ptV;

		if (_bZMapp)
		{
			CalculateInterZMapp(_pObj, _iNumFace, _pInd, _pTex, pPointAdd);
		}
	}

	return iPointAdd;
}
#endif

extern long IsInGroup(EERIE_3DOBJ * obj, long vert, long tw);

void ARX_DrawPrimitive(TexturedVertex * _pVertex1, TexturedVertex * _pVertex2, TexturedVertex * _pVertex3, float _fAddZ) {
	
#ifdef USE_SOFTWARE_CLIPPING
	int iClipp = 0;

	if (_pVertex1->p.z < (SOFTNEARCLIPPZ + _fAddZ)) iClipp |= 1;

	if (_pVertex2->p.z < (SOFTNEARCLIPPZ + _fAddZ)) iClipp |= 2;

	if (_pVertex3->p.z < (SOFTNEARCLIPPZ + _fAddZ)) iClipp |= 4;

	TexturedVertex ClippZ1, ClippZ2;
	TexturedVertex pPointAdd[6];
	int			iNbTotVertex = 3;

	switch (iClipp)
	{
		case 0: {
			EE_P(&_pVertex1->p, &pPointAdd[0]);
			EE_P(&_pVertex2->p, &pPointAdd[1]);
			EE_P(&_pVertex3->p, &pPointAdd[2]);
			pPointAdd[0].color = _pVertex1->color;
			pPointAdd[0].specular = _pVertex1->specular;
			pPointAdd[0].uv = _pVertex1->uv;
			pPointAdd[1].specular = _pVertex2->specular;
			pPointAdd[1].uv = _pVertex2->uv;
			pPointAdd[2].color = _pVertex3->color;
			pPointAdd[2].specular = _pVertex3->specular;
			pPointAdd[2].uv = _pVertex3->uv;
			break;
		}
		case 1:						//pt1 outside
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex2, &ClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex3, &ClippZ2, _fAddZ);
			pPointAdd[0] = ClippZ2;
			pPointAdd[1] = *_pVertex2;
			EE_P2(&pPointAdd[1], &pPointAdd[1]);
			pPointAdd[2] = *_pVertex3;
			EE_P2(&pPointAdd[2], &pPointAdd[2]);
			pPointAdd[3] = ClippZ1;
			pPointAdd[4] = pPointAdd[1];
			pPointAdd[5] = ClippZ2;
			iNbTotVertex = 6;
			break;
		case 2:						//pt2 outside
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex3, &ClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex1, &ClippZ2, _fAddZ);
			pPointAdd[0] = *_pVertex1;
			EE_P2(&pPointAdd[0], &pPointAdd[0]);
			pPointAdd[1] = ClippZ2;
			pPointAdd[2] = *_pVertex3;
			EE_P2(&pPointAdd[2], &pPointAdd[2]);
			pPointAdd[3] = pPointAdd[2];
			pPointAdd[4] = ClippZ1;
			pPointAdd[5] = ClippZ2;
			iNbTotVertex = 6;
			break;
		case 4:						//pt3 outside
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex1, &ClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex2, &ClippZ2, _fAddZ);
			pPointAdd[0] = *_pVertex1;
			EE_P2(&pPointAdd[0], &pPointAdd[0]);
			pPointAdd[1] = *_pVertex2;
			EE_P2(&pPointAdd[1], &pPointAdd[1]);
			pPointAdd[2] = ClippZ2;
			pPointAdd[3] = pPointAdd[0];
			pPointAdd[4] = ClippZ2;
			pPointAdd[5] = ClippZ1;
			iNbTotVertex = 6;
			break;
		case 3:						//pt1_2 outside
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex3, &ClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex3, &ClippZ2, _fAddZ);
			pPointAdd[0] = ClippZ1;
			pPointAdd[1] = ClippZ2;
			pPointAdd[2] = *_pVertex3;
			EE_P2(&pPointAdd[2], &pPointAdd[2]);
			break;
		case 5:						//pt1_3 outside
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex2, &ClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex2, &ClippZ2, _fAddZ);
			pPointAdd[0] = ClippZ1;
			pPointAdd[1] = *_pVertex2;
			EE_P2(&pPointAdd[1], &pPointAdd[1]);
			pPointAdd[2] = ClippZ2;
			break;
		case 6:						//pt2_3 outside
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex1, &ClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex1, &ClippZ2, _fAddZ);
			pPointAdd[0] = *_pVertex1;
			EE_P2(&pPointAdd[0], &pPointAdd[0]);
			pPointAdd[1] = ClippZ1;
			pPointAdd[2] = ClippZ2;
			break;
		case 7:
			return;
	}
#else
	
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
#endif

	EERIEDRAWPRIM(Renderer::TriangleList, pPointAdd);
}

//-----------------------------------------------------------------------------
extern long IN_BOOK_DRAW;

/* Render object */
static void Cedric_RenderObject(EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, Entity * io, Vec3f * pos, Vec3f & ftr, float invisibility) {
	
	float MAX_ZEDE = 0.f;

	if(invisibility == 1.f)
		return;

	float ddist = 0.f;
	long need_halo = 0;

	Entity *hio_helmet	= NULL;
	Entity *hio_armor = NULL;
	Entity *hio_leggings = NULL;
	Entity *hio_player = NULL;

	Entity *use_io = io;

	if(!io && IN_BOOK_DRAW && eobj == entities.player()->obj)
		use_io = entities.player();

	if(use_io) {
		if(use_io == entities.player()) {
			if(player.equiped[EQUIP_SLOT_HELMET] != 0 && ValidIONum(player.equiped[EQUIP_SLOT_HELMET])) {
				Entity * tio = entities[player.equiped[EQUIP_SLOT_HELMET]];

				if (tio->halo.flags & HALO_ACTIVE)
					hio_helmet = tio;
			}

			if(player.equiped[EQUIP_SLOT_ARMOR] != 0 && ValidIONum(player.equiped[EQUIP_SLOT_ARMOR])) {
				Entity * tio = entities[player.equiped[EQUIP_SLOT_ARMOR]];

				if (tio->halo.flags & HALO_ACTIVE)
					hio_armor = tio;
			}

			if(player.equiped[EQUIP_SLOT_LEGGINGS] != 0 && ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS])) {
				Entity * tio = entities[player.equiped[EQUIP_SLOT_LEGGINGS]];

				if (tio->halo.flags & HALO_ACTIVE)
					hio_leggings = tio;
			}

			if(use_io->halo.flags & HALO_ACTIVE)
				hio_player = use_io;
		}

		if(hio_player || hio_armor || hio_leggings || hio_helmet || (use_io->halo.flags & HALO_ACTIVE)) {

			Vec3f ftrPos = *pos + ftr;
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
					MAX_ZEDE = max(eobj->vertexlist3[i].vert.p.z, MAX_ZEDE);
			}
		}
	}

	{
	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		TexturedVertex *tv = NULL;

		long paf[3];

		EERIE_FACE *eface = &eobj->facelist[i];

		if((eface->facetype & POLY_HIDE) && !IN_BOOK_DRAW)
			continue;

		//CULL3D
		Vec3f nrm = eobj->vertexlist3[eface->vid[0]].v - ACTIVECAM->orgTrans.pos;

		if(!(eface->facetype & POLY_DOUBLESIDED)) {
			Vec3f normV10;
			Vec3f normV20;
			normV10 = eobj->vertexlist3[eface->vid[1]].v - eobj->vertexlist3[eface->vid[0]].v;
			normV20 = eobj->vertexlist3[eface->vid[2]].v - eobj->vertexlist3[eface->vid[0]].v;
			Vec3f normFace;
			normFace.x = (normV10.y * normV20.z) - (normV10.z * normV20.y);
			normFace.y = (normV10.z * normV20.x) - (normV10.x * normV20.z);
			normFace.z = (normV10.x * normV20.y) - (normV10.y * normV20.x);

			if(dot(normFace, nrm) > 0.f)
				continue;
		}

		if(eobj->facelist[i].texid < 0)
			continue;

		TextureContainer *pTex = eobj->texturecontainer[eobj->facelist[i].texid];
		if(!pTex)
			continue;

		float fTransp = 0;

		if((eobj->facelist[i].facetype & POLY_TRANS) || invisibility > 0.f) {

			fTransp = (invisibility > 0.f) ? 2.f - invisibility : eobj->facelist[i].transval;

			if(fTransp >= 2.f) { //MULTIPLICATIVE
				fTransp *= (1.f/2);
				fTransp += .5f;
				tv = PushVertexInTableCull_TMultiplicative(pTex);
			} else if(fTransp >= 1.f) { //ADDITIVE
				fTransp -= 1.f;
				tv = PushVertexInTableCull_TAdditive(pTex);
			} else if(fTransp > 0.f) { //NORMAL TRANS
				fTransp = 1.f - fTransp;
				tv = PushVertexInTableCull_TNormalTrans(pTex);
			} else { //SUBTRACTIVE
				fTransp = 1.f - fTransp;
				tv = PushVertexInTableCull_TSubstractive(pTex);
			}
		} else {
			tv = PushVertexInTableCull(pTex);
		}

		for(long n = 0 ; n < 3 ; n++) {
			paf[n] = eface->vid[n];
			tv[n].p = eobj->vertexlist3[paf[n]].vert.p;

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

			tv[n].rhw	= eobj->vertexlist3[paf[n]].vert.rhw;
			tv[n].uv.x	= eface->u[n];
			tv[n].uv.y	= eface->v[n];
			tv[n].color = eobj->vertexlist3[paf[n]].vert.color;
		}

		if((eobj->facelist[i].facetype & POLY_TRANS) || invisibility > 0.f) {
			tv[0].color = tv[1].color = tv[2].color = Color::gray(fTransp).toBGR();
		}

#ifdef USE_SOFTWARE_CLIPPING
		if (!(ARX_SoftClippZ(&eobj->vertexlist3[paf[0]],
										   &eobj->vertexlist3[paf[1]],
										   &eobj->vertexlist3[paf[2]],
										   &tv,
										   eface,
										   invisibility,
										   pTex,
										   (io) && (io->ioflags & IO_ZMAP),
										   eobj,
										   i,
										   paf,
										   NULL,
										   true,
										   special_color_flag,
										   &special_color)))
		{
			continue;
		}
#endif

		if(io && (io->ioflags & IO_ZMAP))
			CalculateInterZMapp(eobj, i, paf, pTex, tv);

		////////////////////////////////////////////////////////////////////////
		// HALO HANDLING START
		if(need_halo && io) {

			float	_ffr[3];

			IO_HALO curhalo;
			memcpy(&curhalo, &io->halo, sizeof(IO_HALO));
			int curhaloInitialized = 0;

			long max_c;

			if (use_io == entities.player())
				max_c = 4;
			else
				max_c = 1;

			for(long cnt = 0; cnt < max_c; cnt++) {
				switch (cnt) {
					case 0:
						if(use_io == entities.player()) {
							if(hio_player) {
								memcpy(&curhalo, &use_io->halo, sizeof(IO_HALO));
								++curhaloInitialized;
							}
							else continue;
						} else {
							memcpy(&curhalo, &io->halo, sizeof(IO_HALO));
							++curhaloInitialized;
						}
						break;
					case 1:
						if(hio_helmet && IsInSelection(use_io->obj, paf[0], use_io->obj->fastaccess.sel_head) >= 0) {
							memcpy(&curhalo, &hio_helmet->halo, sizeof(IO_HALO));
							++curhaloInitialized;
						}
						else continue;
						break;
					case 2:
						if(hio_armor && IsInSelection(use_io->obj, paf[0], use_io->obj->fastaccess.sel_chest) >= 0) {
							memcpy(&curhalo, &hio_armor->halo, sizeof(IO_HALO));
							++curhaloInitialized;
						}
						else continue;
						break;
					case 3:
						if(hio_leggings && IsInSelection(use_io->obj, paf[0], use_io->obj->fastaccess.sel_leggings) >= 0){
							memcpy(&curhalo, &hio_leggings->halo, sizeof(IO_HALO)) ;
							++curhaloInitialized;
						}
						else continue;
						break;
				}

				arx_assert(curhaloInitialized > 0);

				TexturedVertex *workon	= tv;

				float tot = 0;
				for(long o = 0; o < 3; o++) {
					float tttz	= EEfabs(eobj->vertexlist3[paf[o]].norm.z) * ( 1.0f / 2 );
					float power = 255.f - (float)(255.f * tttz);
					power *= (1.f - invisibility);

					power = clamp(power, 0.f, 255.f);

					tot += power;
					_ffr[o] = power;

					u8 lfr = curhalo.color.r * power;
					u8 lfg = curhalo.color.g * power;
					u8 lfb = curhalo.color.b * power;
					tv[o].color = ((0xFF << 24) | (lfr << 16) | (lfg << 8) | (lfb));
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
						TexturedVertex *vert = &LATERDRAWHALO[(HALOCUR << 2)];

						if(HALOCUR < ((long)HALOMAX) - 1) {
							HALOCUR++;
						}

						memcpy(&vert[0], &workon[first], sizeof(TexturedVertex));
						memcpy(&vert[1], &workon[first], sizeof(TexturedVertex));
						memcpy(&vert[2], &workon[second], sizeof(TexturedVertex));
						memcpy(&vert[3], &workon[second], sizeof(TexturedVertex));

						float siz = ddist * (curhalo.radius * (EEsin((arxtime.get_frame_time() + i) * .01f) * .1f + 1.f)) * .6f;

						if(io == entities.player() && ddist > 0.8f && !EXTERNALVIEW)
							siz *= 1.5f;

						Vec3f vect1;
						vect1.x = workon[first].p.x - workon[third].p.x;
						vect1.y = workon[first].p.y - workon[third].p.y;
						float len1 = 2.f / ffsqrt(vect1.x * vect1.x + vect1.y * vect1.y);

						if(vect1.x < 0.f)
							len1 *= 1.2f;

						vect1.x *= len1;
						vect1.y *= len1;

						Vec3f vect2;
						vect2.x = workon[second].p.x - workon[third].p.x;
						vect2.y = workon[second].p.y - workon[third].p.y;
						float len2 = 1.f / ffsqrt(vect2.x * vect2.x + vect2.y * vect2.y);

						if(vect2.x < 0.f)
							len2 *= 1.2f;

						vect2.x *= len2;
						vect2.y *= len2;

						vert[1].p.x += (vect1.x + 0.2f - rnd() * 0.1f) * siz;
						vert[1].p.y += (vect1.y + 0.2f - rnd() * 0.1f) * siz;
						vert[1].color = 0xFF000000;

						float valll;
						valll = 0.005f + (EEfabs(workon[first].p.z) - EEfabs(workon[third].p.z))
									   + (EEfabs(workon[second].p.z) - EEfabs(workon[third].p.z));
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

						if(curhalo.flags & HALO_NEGATIVE)
							vert[2].color = 0x00000000;
						else
							vert[2].color = 0xFF000000;
					}
				}
			}

			for(long o = 0; o < 3; o++) {
				paf[o] = eface->vid[o];
				tv[o].color = eobj->vertexlist3[paf[o]].vert.color;
			}
		}
	}
	}
}

void Cedric_BlendAnimation(Entity *io, EERIE_C_DATA *c_data) {

	if(!io || !io->nb_lastanimvertex) {
		return;
	}

	float timm = (arxtime.get_frame_time() - io->lastanimtime) + 0.0001f;

	if(timm >= 300.f) {
		io->nb_lastanimvertex = 0;
		return;
	} else {
		timm *= ( 1.0f / 300 );

		if(timm < 0.f || timm >= 1.f)
			return;
	}

	for(long i = 0; i < c_data->nb_bones; i++) {
		EERIE_BONE * bone = &c_data->bones[i];

		EERIE_QUAT tquat;
		Quat_Copy(&tquat, &bone->quatinit);
		EERIE_QUAT q2;
		Quat_Copy(&q2, &bone->quatlast);

		Quat_Slerp(&bone->quatinit , &q2, &tquat, timm);

		bone->transinit = bone->translast + (bone->transinit - bone->translast) * timm;
	}
}

void Cedric_SaveBlendData(EERIE_C_DATA *c_data) {
	if (c_data)
	{
		for (long i = 0; i < c_data->nb_bones; i++)
		{
			Quat_Copy(&c_data->bones[i].quatlast, &c_data->bones[i].quatinit);
			c_data->bones[i].scalelast = c_data->bones[i].scaleinit;
			c_data->bones[i].translast = c_data->bones[i].transinit;
		}
	}
}
 
void Cedric_ManageExtraRotationsFirst(Entity * io, EERIE_3DOBJ * obj)
{
	for(long i = 0; i != obj->c_data->nb_bones; i++) {
		Quat_Init(&obj->c_data->bones[i].quatinit);
		obj->c_data->bones[i].transinit = obj->c_data->bones[i].transinit_global;
	}

	if(io && (io->ioflags & IO_NPC) && io->_npcdata->ex_rotate) {
		for(long k = 0; k < MAX_EXTRA_ROTATE; k++) {
			long i = io->_npcdata->ex_rotate->group_number[k];

			if(i >= 0) {
				Anglef vt1;
				EERIE_QUAT quat1;
				vt1.a = radians(io->_npcdata->ex_rotate->group_rotate[k].g);
				vt1.b = radians(io->_npcdata->ex_rotate->group_rotate[k].b);
				vt1.g = radians(io->_npcdata->ex_rotate->group_rotate[k].a);
				QuatFromAngles(&quat1, &vt1);
				Quat_Copy(&obj->c_data->bones[i].quatinit, &quat1);
			}
		}
	}
}

bool Cedric_IO_Visible(Vec3f * pos) {

	if(ACTIVEBKG) {
		//TODO maybe readd this
		//if(distSqr(io->pos, ACTIVECAM->orgTrans.pos) > square(ACTIVECAM->cdepth) * square(0.6f))
		//	return false;

		long xx = pos->x * ACTIVEBKG->Xmul;
		long yy = pos->z * ACTIVEBKG->Zmul;

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



/*!
 * \brief Apply animation and draw object
 */
void Cedric_AnimateDrawEntity(EERIE_3DOBJ *eobj, ANIM_USE *animuse, Anglef *angle, Vec3f *pos, Entity *io, Vec3f & ftr, float scale) {
	
	// Init some data
	Cedric_ResetBoundingBox(io);
	
	// Manage Extra Rotations in Local Space
	Cedric_ManageExtraRotationsFirst(io, eobj);

	// Perform animation in Local space
	Cedric_AnimateObject(io, eobj, animuse);

	// Check for Animation Blending in Local space
	if(io) {
		// Is There any Between-Animations Interpolation to make ?
		Cedric_BlendAnimation(io, eobj->c_data);

		Cedric_SaveBlendData(io->obj->c_data);
	}


	EERIE_QUAT	qt2;

	bool isNpc = io && (io->ioflags & IO_NPC);
	worldAngleToQuat(&qt2, angle, isNpc);

	// Build skeleton in Object Space
	Cedric_ConcatenateTM(eobj->c_data, &qt2, pos, ftr, scale);

	/* Display the object */
	EERIE_C_DATA *obj = eobj->c_data;

	if(!obj)
		return;

	Cedric_TransformVerts(io, eobj, obj, pos);
}

void Cedric_AnimateDrawEntityRender(EERIE_3DOBJ *eobj, Vec3f *pos, Vec3f &ftr, Entity *io) {

	float invisibility = Cedric_GetInvisibility(io);

	if(!io && INVISIBILITY_OVERRIDE != 0.f)
		invisibility = INVISIBILITY_OVERRIDE;

	EERIE_C_DATA *obj = eobj->c_data;

	if(!obj)
		return;

	long special_color_flag = 0;
	Color3f special_color = Color3f::black;
	if(io) {
		special_color_flag = io->special_color_flag;
		special_color = io->special_color;
	}

	Cedric_ApplyLighting(eobj, obj, io, pos, special_color, special_color_flag);

	Cedric_RenderObject(eobj, obj, io, pos, ftr, invisibility);



	// Now we can render Linked Objects

	for (long k = 0; k < eobj->nblinked; k++) {
		if((eobj->linked[k].lgroup != -1) && eobj->linked[k].obj) {

			eobj->linked[k].modinfo.rot = Anglef::ZERO;


			Entity * ioo = (Entity *)eobj->linked[k].io;
			EERIE_3DOBJ * obj = (EERIE_3DOBJ *) eobj->linked[k].obj;

			// Store item invisibility flag
			float old = 0.f;
			if(ioo) {
				old = ioo->invisibility;
				ioo->invisibility = invisibility;
			}

			// specific check to avoid drawing player weapon on its back when in subjective view
			if(io == entities.player() &&
				eobj->linked[k].lidx == entities.player()->obj->fastaccess.weapon_attach &&
				!EXTERNALVIEW
			)
				continue;

			long ll = eobj->linked[k].lidx2;
			eobj->linked[k].modinfo.link_position = obj->vertexlist[ll].v - obj->vertexlist[obj->origin].v;

			EERIE_QUAT quat;
			ll = eobj->linked[k].lidx;
			Vec3f * posi = &eobj->vertexlist3[ll].v;
			Quat_Copy(&quat, &eobj->c_data->bones[eobj->linked[k].lgroup].quatanim);

			DrawEERIEInter(obj, &quat, posi, ioo, &eobj->linked[k].modinfo);

			// Restore item invisibility flag
			if(ioo)
				ioo->invisibility = old;
		}
	}
}

void MakeCLight(Entity * io, Color3f * infra, const EERIE_QUAT *qInvert, Vec3f * pos, EERIE_3DOBJ * eobj, Color3f &special_color, long &special_color_flag)
{
	if(Project.improve && !io) {
		infra->r = 0.6f;
		infra->g = 0.f;
		infra->b = 1.f;
	}

	llightsInit();
	Vec3f tv = *pos;

	if(io && (io->ioflags & IO_ITEM))
		tv.y -= 60.f;
	else
		tv.y -= 90.f;

	for(int i = 0; i < TOTIOPDL; i++) {
		if(IO_PDL[i]->fallend + 500.f < 0)
			continue;

		Insertllight(IO_PDL[i], dist(IO_PDL[i]->pos, tv));
	}

	for(int i = 0; i < TOTPDL; i++) {
		if(PDL[i]->fallend + 500.f < 0)
			continue;

		Insertllight(PDL[i], dist(PDL[i]->pos, tv));
	}

	if(io && (io->ioflags & IO_ANGULAR))
		return;
		
	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		Color3f tempColor;

		// Ambient light
		if(io && (io->ioflags & (IO_NPC | IO_ITEM)))
			tempColor = Color3f::gray(NPC_ITEMS_AMBIENT_VALUE_255);
		else
			tempColor = ACTIVEBKG->ambient255;

		Vec3f * posVert = &eobj->vertexlist3[i].v;

		// Dynamic lights
		for(int l = 0; l != MAX_LLIGHTS; l++) {
			EERIE_LIGHT * Cur_llights = llights[l];

			if(!Cur_llights)
				break;

			Vec3f vLight = (llights[l]->pos - *posVert).getNormalized();

			Vec3f Cur_vLights;
			TransformInverseVertexQuat(qInvert, &vLight, &Cur_vLights);

			// Get cos angle between light and vertex norm
			float cosangle = dot(eobj->vertexlist[i].norm, Cur_vLights);

			// If light visible
			if(cosangle > 0.f) {
				float distance = fdist(*posVert, Cur_llights->pos);

				// Evaluate its intensity depending on the distance Light<->Object
				if(distance <= Cur_llights->fallstart) {
					cosangle *= Cur_llights->precalc;
				} else {
					float p = ((Cur_llights->fallend - distance) * Cur_llights->falldiffmul);

					if(p <= 0.f)
						cosangle = 0.f;
					else
						cosangle *= p * Cur_llights->precalc;
				}

				tempColor.r += Cur_llights->rgb255.r * cosangle;
				tempColor.g += Cur_llights->rgb255.g * cosangle;
				tempColor.b += Cur_llights->rgb255.b * cosangle;
			}
		}

		if(Project.improve && !io) {
			tempColor.r *= infra->r;
			tempColor.g *= infra->g;
			tempColor.b *= infra->b;

			tempColor.r += infra->r * 512.f;
			tempColor.g += infra->g;
			tempColor.b += infra->b * 400.f;
		}

		if(special_color_flag & 1) {
			tempColor.r *= special_color.r;
			tempColor.g *= special_color.g;
			tempColor.b *= special_color.b;
		} else if(special_color_flag & 4) { // HIGHLIGHT
			tempColor.r += special_color.r;
			tempColor.g += special_color.g;
			tempColor.b += special_color.b;
		}

		u8 ir = clipByte255(tempColor.r);
		u8 ig = clipByte255(tempColor.g);
		u8 ib = clipByte255(tempColor.b);
		eobj->vertexlist3[i].vert.color = (0xFF000000L | (ir << 16) | (ig << 8) | (ib));
	}
}

void MakeCLight2(Entity *io, Color3f *infra, const EERIE_QUAT *qInvert, Vec3f *pos, EERIE_3DOBJ *eobj, long ii, Color3f &special_color, long &special_color_flag) {

	Vec3f vTLights[32];

	Vec3f tv = *pos;

	if(io && (io->ioflags & IO_ITEM))
		tv.y -= 60.f;
	else
		tv.y -= 90.f;
	
	for(long l = 0; l != MAX_LLIGHTS; l++) {
		if(llights[l]) {
			Vec3f vLight = (llights[l]->pos - tv) / dists[l];
			TransformInverseVertexQuat(qInvert, &vLight, &vTLights[l]);
		} else {
			break;
		}
	}
	
	long paf[3];
	paf[0] = eobj->facelist[ii].vid[0];
	paf[1] = eobj->facelist[ii].vid[1];
	paf[2] = eobj->facelist[ii].vid[2];

	for(long i = 0; i < 3; i++) {
		Color3f tempColor;

		if(io && (io->ioflags & (IO_NPC | IO_ITEM)))
			tempColor = Color3f::gray(NPC_ITEMS_AMBIENT_VALUE_255);
		else
			tempColor = ACTIVEBKG->ambient255;

		Vec3f * posVert = &eobj->vertexlist3[paf[i]].v;

		for(int l = 0; l != MAX_LLIGHTS; l++) {
			EERIE_LIGHT * Cur_llights = llights[l];

			if(!Cur_llights)
				break;

			float oolength = 1.f / fdist(*posVert, Cur_llights->pos);
			Vec3f vLight = (llights[l]->pos - *posVert) * oolength;

			TransformInverseVertexQuat(qInvert, &vLight, &vTLights[l]);
			Vec3f * Cur_vLights = &vTLights[l];

			float cosangle = dot(eobj->facelist[ii].norm, *Cur_vLights);

			cosangle *= 0.5f;

			// If light visible
			if(cosangle > 0.f) {
				float distance = fdist(*posVert, Cur_llights->pos);

				// Evaluate its intensity depending on the distance Light<->Object
				if(distance <= Cur_llights->fallstart) {
					cosangle *= Cur_llights->precalc;
				} else {
					float p = ((Cur_llights->fallend - distance) * Cur_llights->falldiffmul);

					if(p <= 0.f)
						cosangle = 0.f;
					else
						cosangle *= p * Cur_llights->precalc;
				}

				tempColor.r += Cur_llights->rgb255.r * cosangle;
				tempColor.g += Cur_llights->rgb255.g * cosangle;
				tempColor.b += Cur_llights->rgb255.b * cosangle;
			}
		}

		if(Project.improve) {
			tempColor.r *= infra->r;
			tempColor.g *= infra->g;
			tempColor.b *= infra->b;
		}

		if(special_color_flag & 1) {
			tempColor.r *= special_color.r;
			tempColor.g *= special_color.g;
			tempColor.b *= special_color.b;
		} else if(special_color_flag & 4) { // HIGHLIGHT
			tempColor.r += special_color.r;
			tempColor.g += special_color.g;
			tempColor.b += special_color.b;
		}

		u8 ir = clipByte255(tempColor.r);
		u8 ig = clipByte255(tempColor.g);
		u8 ib = clipByte255(tempColor.b);
		eobj->vertexlist3[paf[i]].vert.color = (0xFF000000L | (ir << 16) | (ig << 8) | (ib));
	}
}


void ApplyDynLight(EERIEPOLY * ep)
{
	int nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

	if(TOTPDL == 0) {
		for(int i = 0; i < nbvert; i++)
			ep->tv[i].color = ep->v[i].color;

		return;
	}

	Color3f rgb;

	float epr[4];
	float epg[4];
	float epb[4];

	for(int i = 0; i < nbvert; i++) {
		long c = ep->v[i].color;
		epr[i] = (float)((c >> 16) & 255);
		epg[i] = (float)((c >> 8) & 255);
		epb[i] = (float)(c & 255);
	}

	for(int i = 0; i < TOTPDL; i++) {
		EERIE_LIGHT * el = PDL[i];

		if(el->fallend + 35.f < 0) {
			TSU_TEST_NB_LIGHT ++;
			continue;
		}

		if(distSqr(el->pos, ep->center) <= square(el->fallend + 35.f)) {
			if(Project.improve) {
				rgb.r = el->rgb255.r * 4.f;
				rgb.g = rgb.b = 0.2f;
			} else {
				rgb = el->rgb255;
			}

			for(int j = 0; j < nbvert; j++) {
				Vec3f v(ep->v[j].p.x, ep->v[j].p.y, ep->v[j].p.z);
				if(el->fallend < 0) {
					TSU_TEST_NB ++;
					continue;
				}

				float d = fdist(el->pos, ep->v[j].p);

				if(d <= el->fallend) {
					float divd = 1.f / d;

					Vec3f v1 = (el->pos - ep->v[j].p) * divd;
					float nvalue = dot(v1, ep->nrml[j]) * (1.0f / 2);

					nvalue = clamp(nvalue, 0.f, 1.f);

					if(nvalue > 0.f) {
						if(d <= el->fallstart) {
							d = nvalue * el->precalc;
						} else {
							d -= el->fallstart;
							d = (el->falldiff - d) * el->falldiffmul * nvalue * el->precalc;
						}

						epr[j] += rgb.r * d;
						epg[j] += rgb.g * d;
						epb[j] += rgb.b * d;
					}
				}
				else if(d > el->fallend + 100.f)
					break;
			}
		}
	}

	for(int j = 0; j < nbvert; j++) {
		u8 lepr = clipByte255(epr[j]);
		u8 lepg = clipByte255(epg[j]);
		u8 lepb = clipByte255(epb[j]);
		ep->tv[j].color = (0xFF000000L | (lepr << 16) | (lepg << 8) | (lepb));
	}
}

extern TILE_LIGHTS tilelights[MAX_BKGX][MAX_BKGZ];
//*************************************************************************************

void ApplyDynLight_VertexBuffer_2(EERIEPOLY * ep, short _x, short _y, SMY_VERTEX * _pVertex, unsigned short _usInd0, unsigned short _usInd1, unsigned short _usInd2, unsigned short _usInd3)
{
	// Nuky - 25/01/11 - harmless refactor to understand what is slow.
	//        MASSIVE speed up thanks to "harmless refactor", wtf ?

	TILE_LIGHTS * tls = &tilelights[_x][_y];
	long nbvert = (ep->type & POLY_QUAD) ? 4 : 3;

	if(tls->num == 0) {
		_pVertex[_usInd0].color = ep->v[0].color;
		ep->tv[0].color         = ep->v[0].color;
		_pVertex[_usInd1].color = ep->v[1].color;
		ep->tv[1].color         = ep->v[1].color;
		_pVertex[_usInd2].color = ep->v[2].color;
		ep->tv[2].color         = ep->v[2].color;

		if(nbvert & 4) {
			_pVertex[_usInd3].color = ep->v[3].color;
			ep->tv[3].color         = ep->v[3].color;
		}

		return;
	}

	float epr[4];
	float epg[4];
	float epb[4];
	
	for(long i = 0; i < nbvert; i++) {
		long c = ep->v[i].color;
		epr[i] = (float)(long)((c >> 16) & 255);
		epg[i] = (float)(long)((c >> 8) & 255);
		epb[i] = (float)(long)(c & 255);
	}

	for(long i = 0; i < tls->num; i++) {
		EERIE_LIGHT * el = tls->el[i];
	
		for(long j = 0; j < nbvert; j++) {
			float d = fdist(el->pos, ep->v[j].p);

			if (d < el->fallend) {
				float nvalue;
				
				nvalue =	((el->pos.x - ep->v[j].p.x) * ep->nrml[j].x
				             +	(el->pos.y - ep->v[j].p.y) * ep->nrml[j].y
				             +	(el->pos.z - ep->v[j].p.z) * ep->nrml[j].z
				         ) * 0.5f / d; 
				
				if(nvalue > 0.f) {
					if(d <= el->fallstart) {
						d = el->precalc * nvalue; 
					} else {
						d -= el->fallstart;
						d = (el->falldiff - d) * el->falldiffmul * el->precalc * nvalue; 
					}
					epr[j] += el->rgb255.r * d;
					epg[j] += el->rgb255.g * d;
					epb[j] += el->rgb255.b * d;
				}
			}
			else if (d > el->fallend + 100.f)
				break;
		}	
	}

	u8 lepr, lepg, lepb;

	lepr = clipByte255(epr[0]);
	lepg = clipByte255(epg[0]);
	lepb = clipByte255(epb[0]);
	ep->tv[0].color = _pVertex[_usInd0].color = (0xFF000000L | (lepr << 16) | (lepg << 8) | (lepb));

	lepr = clipByte255(epr[1]);
	lepg = clipByte255(epg[1]);
	lepb = clipByte255(epb[1]);
	ep->tv[1].color = _pVertex[_usInd1].color = (0xFF000000L | (lepr << 16) | (lepg << 8) | (lepb));

	lepr = clipByte255(epr[2]);
	lepg = clipByte255(epg[2]);
	lepb = clipByte255(epb[2]);
	ep->tv[2].color = _pVertex[_usInd2].color = (0xFF000000L | (lepr << 16) | (lepg << 8) | (lepb));

	if(nbvert & 4) {
		lepr = clipByte255(epr[3]);
		lepg = clipByte255(epg[3]);
		lepb = clipByte255(epb[3]);
		ep->tv[3].color = _pVertex[_usInd3].color = (0xFF000000L | (lepr << 16) | (lepg << 8) | (lepb));
	}
}
