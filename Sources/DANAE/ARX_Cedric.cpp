/*
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
#include "EERIEAnim.h"
#include "HermesMain.h"
#include "Arx_Collisions.h"
#include "EERIEClothes.h"
#include "EERIEObject.h"
#include "EERIEMath.h"
#include "EERIELight.h"
#include "EERIEPoly.h"
#include "EERIEDraw.h"
#include <Arx_Sound.h>
#include <Arx_Scene.h>
#include <Arx_Equipment.h>
#include <Arx_Damages.h>
#include <Arx_Particles.h>
#include <ARX_NPC.h>
#include "ARX_Time.h"
#include "danae.h"
#include <dinput.h>
#include "arx_menu2.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include 	"arx_cedric.h"


#if	CEDRIC

extern		BOOL 		MIPM;
extern		float 		vdist;
extern 		float 		FORCED_REDUCTION_VALUE;
extern		unsigned char * grps;
extern		long 		max_grps;
extern		long		FORCE_NO_HIDE;
extern		long		USEINTERNORM;
extern 		long 		INTER_DRAW;

extern		float		dists[];
extern bool bZBUFFER;
extern bool bGATI8500;
extern bool bSoftRender;
extern bool bALLOW_BUMP;
extern long BH_MODE;
extern int iHighLight;

extern float IN_FRONT_DIVIDER;
extern float IN_FRONT_DIVIDER_FEET;
extern bool bRenderInterList;
extern TextureContainer TexSpecialColor;

 
extern long FLAG_ALLOW_CLOTHES;
 
long TSU_TEST = 0;
extern long TSU_TEST_NB;
extern long TSU_TEST_NB_LIGHT;
extern D3DMATRIX ProjectionMatrix;
extern float fZFogStart;

extern CDirectInput * pGetInfoDirectInput;

float SOFTNEARCLIPPZ=1.f;
#define SOFTNEARCLIPPTANDLZ (60.f)

void CalculateInterZMappTANDL(EERIE_3DOBJ * _pobj3dObj, long lIdList, long * _piInd, TextureContainer * _pTex);
void PushInterBumpTANDL(TextureContainer * _pTex, D3DTLVERTEX * _pVertex, EERIE_3D * _pee3d0, EERIE_3D * _pee3d1, EERIE_3D * _pee3d2);

void EE_P2(D3DTLVERTEX * in, D3DTLVERTEX * out);

 

__inline void ResetBBox3D(INTERACTIVE_OBJ * io)
{
	if (io)
	{
		io->bbox3D.min.x = 99999999.f;
		io->bbox3D.min.y = 99999999.f;
		io->bbox3D.min.z = 99999999.f;
		io->bbox3D.max.x = -99999999.f;
		io->bbox3D.max.y = -99999999.f;
		io->bbox3D.max.z = -99999999.f;
	}
}
__inline void AddToBBox3D(INTERACTIVE_OBJ * io, EERIE_3D * pos)
{
	if (io)
	{
		io->bbox3D.min.x = __min(io->bbox3D.min.x, pos->x);
		io->bbox3D.min.y = __min(io->bbox3D.min.y, pos->y);
		io->bbox3D.min.z = __min(io->bbox3D.min.z, pos->z);
		io->bbox3D.max.x = __max(io->bbox3D.max.x, pos->x);
		io->bbox3D.max.y = __max(io->bbox3D.max.y, pos->y);
		io->bbox3D.max.z = __max(io->bbox3D.max.z, pos->z);
	}
}


/* Init bounding box */
inline	static	void	Cedric_ResetBoundingBox(INTERACTIVE_OBJ * io)
{
	// resets 2D Bounding Box
	BBOXMIN.y = BBOXMIN.x = 32000;
	BBOXMAX.y = BBOXMAX.x = -32000;
	// Resets 3D Bounding Box
	ResetBBox3D(io);
}

//-----------------------------------------------------------------------------

__forceinline float GetMaxManhattanDistance(const EERIE_3D * _e1, const EERIE_3D * _e2)
{
	return 0;

	if (!TSU_TEST) return 0;

	register float fMaxX(0);
	register float fMaxY(0);
	register float fMaxZ(0);

	fMaxX = _e1->x - _e2->x;
	
	fMaxX = (fMaxX < 0) ? -fMaxX : fMaxX;

	fMaxY = _e1->y - _e2->y;
	
	fMaxY = (fMaxY < 0) ? -fMaxY : fMaxY;

	fMaxZ = _e1->z - _e2->z;
	
	fMaxZ = (fMaxZ < 0) ? -fMaxZ : fMaxZ;

	fMaxX = (fMaxX < fMaxY) ? fMaxY : fMaxX;
	fMaxX = (fMaxX < fMaxZ) ? fMaxZ : fMaxX;

	return fMaxX;
}

extern float INVISIBILITY_OVERRIDE;
extern long EXTERNALVIEW;
static	void	Cedric_GetScale(float & scale, float & invisibility, INTERACTIVE_OBJ * io)
{
	if (io)
	{
		invisibility = io->invisibility;

		if (invisibility > 1.f) invisibility -= 1.f;

		if ((io != inter.iobj[0]) && (invisibility > 0.f) && (!EXTERNALVIEW))
		{
			long num = ARX_SPELLS_GetSpellOn(io, SPELL_INVISIBILITY);

			if (num >= 0)
			{
				if (player.Full_Skill_Intuition > spells[num].caster_level * 10)
				{
					invisibility -= (float)player.Full_Skill_Intuition * DIV100 + (float)spells[num].caster_level * DIV10;

					if (invisibility < 0.1f) invisibility = 0.1f;
					else if (invisibility > 1.f) invisibility = 1.f;
				}
			}
		}

		// Scaling Value for this object (Movements will also be scaled)
		scale = io->scale;
	}
	else
	{
		if (INVISIBILITY_OVERRIDE != 0.f)
		{
			invisibility = INVISIBILITY_OVERRIDE;

			if (invisibility > 1.f) invisibility -= 1.f;
		}
		else
		{
			invisibility = 0.f;
		}

		scale = 1.f;
	}
}


static	void	Cedric_GetTime(float & timm, INTERACTIVE_OBJ * io, long typ)
{
	if ((io) && (!(typ & ANIMQUATTYPE_FIRST_PERSON)))
	{
		if (io->nb_lastanimvertex)
		{
			timm = (FrameTime - io->lastanimtime) + 0.0001f;

			if (timm >= 300.f)
			{
				timm = 0.f;
				io->nb_lastanimvertex = 0;
			}
			else
			{
				timm *= DIV300;

				if (timm >= 1.f) timm = 0.f;
				else if (timm < 0.f) timm = 0.f;
			}
		}
		else timm = 0.f;
	}
	else timm = 0.f;
}


#define ANIMQUATTYPE_NO_COMPUTATIONS	8


/* Evaluate main entity translation */
static	void	Cedric_AnimCalcTranslation(INTERACTIVE_OBJ * io, ANIM_USE * animuse, float scale, long typ, EERIE_3D & ftr, EERIE_3D & ftr2)
{
	// Resets Frame Translate
	Vector_Init(&ftr);
	Vector_Init(&ftr2);


	// Fill frame translate values with multi-layer translate informations...
	for (int count = MAX_ANIM_LAYERS - 1; count >= 0; count--)
	{
		EERIE_ANIM	*	eanim;

		if (!io)
		{
			count = -1;
		}
		else
		{
			animuse = &io->animlayer[count];
		}

		if (!animuse) continue;

		if (!animuse->cur_anim) continue;

		eanim = animuse->cur_anim->anims[animuse->altidx_cur];

		if (!eanim) continue;

		//Avoiding impossible cases
		if (animuse->fr < 0)
		{
			animuse->fr = 0;
			animuse->pour = 0.f;
		}
		else if (animuse->fr >= eanim->nb_key_frames - 1)
		{
			animuse->fr = eanim->nb_key_frames - 2;
			animuse->pour = 1.f;
		}
		else if (animuse->pour > 1.f) animuse->pour = 1.f;
		else if (animuse->pour < 0.f) animuse->pour = 0.f;


		// FRAME TRANSLATE : Gives the Virtual pos of Main Object
		if (((eanim->frames[animuse->fr].f_translate) && (!(animuse->flags & EA_STATICANIM))))
		{
			EERIE_FRAME * sFrame = &eanim->frames[animuse->fr];
			EERIE_FRAME * eFrame = &eanim->frames[animuse->fr+1];

			// Linear interpolation of object translation (MOVE)
			ftr.x = sFrame->translate.x + (eFrame->translate.x - sFrame->translate.x) * animuse->pour;
			ftr.y = sFrame->translate.y + (eFrame->translate.y - sFrame->translate.y) * animuse->pour;
			ftr.z = sFrame->translate.z + (eFrame->translate.z - sFrame->translate.z) * animuse->pour;

			if ((io) && !(typ & ANIMQUATTYPE_NO_RENDER))
			{
				ftr.x *= scale;
				ftr.y *= scale;
				ftr.z *= scale;

				float temp = DEG2RAD(MAKEANGLE(180.f - io->angle.b));

				if (io == inter.iobj[0]) temp = DEG2RAD(MAKEANGLE(180.f - player.angle.b));

				_YRotatePoint(&ftr, &ftr2, (float)EEcos(temp), (float)EEsin(temp));

				// stores Translations for a later use
				io->move.x = ftr2.x;
				io->move.y = ftr2.y;
				io->move.z = ftr2.z;
			}
		}
	}

	if ((io) && (io->animlayer[0].cur_anim) && !(typ & ANIMQUATTYPE_NO_RENDER))
	{
		// Use calculated value to notify the Movement engine of the translation to do
		if (io->ioflags & IO_NPC)
		{
			Vector_Init(&ftr);
			io->move.x -= io->lastmove.x;
			io->move.y -= io->lastmove.y;
			io->move.z -= io->lastmove.z;
		}
		// Must recover translations for NON-NPC IO
		else
		{
			if (io->GameFlags & GFLAG_ELEVATOR)
			{
				PushIO_ON_Top(io, io->move.y - io->lastmove.y);
			}
		}

		io->lastmove.x = ftr2.x;
		io->lastmove.y = ftr2.y;
		io->lastmove.z = ftr2.z;
	}
}


long Looking_At = -1;

// Animate skeleton
static	void	Cedric_AnimateObject(INTERACTIVE_OBJ * io, EERIE_3DOBJ * eobj, ANIM_USE * animuse)
{
	int				j, l;
	EERIE_ANIM 	*	eanim = animuse->cur_anim->anims[animuse->altidx_cur];
	EERIE_C_DATA	* obj = eobj->c_data;

	for (long count = MAX_ANIM_LAYERS - 1; count >= 0; count--)
	{
		EERIE_QUAT		t, temp;
		EERIE_3D		vect;
		EERIE_3D		scale;

		if (!io)
		{
			count = -1;

			if (animuse->cur_anim == NULL)
				continue;

			eanim = animuse->cur_anim->anims[animuse->altidx_cur];
		}
		else
		{
			animuse = &io->animlayer[count];

			if (animuse->cur_anim == NULL) continue;

			eanim = animuse->cur_anim->anims[animuse->altidx_cur];
		}

		if (!animuse) continue;

		if (!eanim) continue;

		if (animuse->fr < 0)
		{
			animuse->fr = 0;
			animuse->pour = 0.f;
		}
		else if (animuse->fr >= eanim->nb_key_frames - 1)
		{
			animuse->fr = eanim->nb_key_frames - 2;
			animuse->pour = 1.f;
		}
		else if (animuse->pour > 1.f) animuse->pour = 1.f;
		else if (animuse->pour < 0.f) animuse->pour = 0.f;

		// Now go for groups rotation/translation/scaling, And transform Linked objects by the way
		l = __min(eobj->nbgroups - 1, eanim->nb_groups - 1);

		for (j = l; j >= 0; j--)
		{
			if (grps[j])
				continue;

			EERIE_GROUP * sGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)];
			EERIE_GROUP * eGroup = &eanim->groups[j+(animuse->fr*eanim->nb_groups)+eanim->nb_groups];

			if (!eanim->voidgroups[j])
				grps[j] = 1;

			if (eanim->nb_key_frames != 1)
			{
				Quat_Slerp(&t, &sGroup->quat, &eGroup->quat, animuse->pour);
				Quat_Copy(&temp, &obj->bones[j].quatinit);
				Quat_Multiply(&obj->bones[j].quatinit, &temp, &t);

				vect.x = sGroup->translate.x + (eGroup->translate.x - sGroup->translate.x) * animuse->pour;
				vect.y = sGroup->translate.y + (eGroup->translate.y - sGroup->translate.y) * animuse->pour;
				vect.z = sGroup->translate.z + (eGroup->translate.z - sGroup->translate.z) * animuse->pour;
				Vector_Add(&obj->bones[j].transinit, &vect, &obj->bones[j].transinit_global);

				scale.x = sGroup->zoom.x + (eGroup->zoom.x - sGroup->zoom.x) * animuse->pour;
				scale.y = sGroup->zoom.y + (eGroup->zoom.y - sGroup->zoom.y) * animuse->pour;
				scale.z = sGroup->zoom.z + (eGroup->zoom.z - sGroup->zoom.z) * animuse->pour;

				if (BH_MODE)
				{
					if (j == eobj->fastaccess.head_group)
					{
						scale.x += 1.f;
						scale.y += 1.f;
						scale.z += 1.f;
					}
				}

				Vector_Copy(&obj->bones[j].scaleinit, &scale);
			}
		}
	}
}



/* Apply transformations on all bones */
void	Cedric_ConcatenateTM(INTERACTIVE_OBJ * io, EERIE_C_DATA * obj, EERIE_3D * angle, EERIE_3D * pos, EERIE_3D & ftr, float g_scale)
{
	int i;

	if (!obj)
		return;

	for (i = 0; i != obj->nb_bones; i++)
	{
		EERIE_QUAT	qt2;
		EERIE_3D	vt1;

		if (obj->bones[i].father >= 0) // Child Bones
		{
			// Rotation
			Quat_Multiply(&obj->bones[i].quatanim, &obj->bones[obj->bones[i].father].quatanim, &obj->bones[i].quatinit);

			// Translation
			obj->bones[i].transanim.x = obj->bones[i].transinit.x * obj->bones[obj->bones[i].father].scaleanim.x;
			obj->bones[i].transanim.y = obj->bones[i].transinit.y * obj->bones[obj->bones[i].father].scaleanim.y;
			obj->bones[i].transanim.z = obj->bones[i].transinit.z * obj->bones[obj->bones[i].father].scaleanim.z;
			TransformVertexQuat(&obj->bones[obj->bones[i].father].quatanim, &obj->bones[i].transanim, &obj->bones[i].transanim);
			Vector_Add(&obj->bones[i].transanim, &obj->bones[obj->bones[i].father].transanim, &obj->bones[i].transanim);

			/* Scale */
			obj->bones[i].scaleanim.x = (obj->bones[i].scaleinit.x + 1.f) * obj->bones[obj->bones[i].father].scaleanim.x;
			obj->bones[i].scaleanim.y = (obj->bones[i].scaleinit.y + 1.f) * obj->bones[obj->bones[i].father].scaleanim.y;
			obj->bones[i].scaleanim.z = (obj->bones[i].scaleinit.z + 1.f) * obj->bones[obj->bones[i].father].scaleanim.z;

		}
		else // Root Bone
		{
			// Rotation
			if ((io) && !(io->ioflags & IO_NPC))
			{
				// To correct invalid angle in Animated FIX/ITEMS
				EERIE_3D ang;
				Vector_Copy(&ang, angle);
				ang.a = (360 - ang.a);
				ang.b = (ang.b);
				ang.g = (ang.g);
				EERIEMATRIX mat;
				EERIE_3D vect, up;
				Vector_Init(&vect, 0, 0, 1);
				Vector_Init(&up, 0, 1, 0);
				VRotateY(&vect, ang.b);
				VRotateX(&vect, ang.a);
				VRotateZ(&vect, ang.g);
				VRotateY(&up, ang.b);
				VRotateX(&up, ang.a);
				VRotateZ(&up, ang.g);
				MatrixSetByVectors(&mat, &vect, &up);
				QuatFromMatrix(qt2, mat);
				Quat_Multiply(&obj->bones[i].quatanim, &qt2, &obj->bones[i].quatinit);
			}
			else
			{
				Vector_Copy(&vt1, angle);
				vt1.x *= EEdef_DEGTORAD;
				vt1.y *= EEdef_DEGTORAD;
				vt1.z *= EEdef_DEGTORAD;
				QuatFromAngles(&qt2, &vt1);
				Quat_Multiply(&obj->bones[i].quatanim, &qt2, &obj->bones[i].quatinit);
			}

			// Translation
			Vector_Add(&vt1, &obj->bones[i].transinit, &ftr);
			TransformVertexQuat(&qt2, &vt1, &obj->bones[i].transanim);
			obj->bones[i].transanim.x *= g_scale;
			obj->bones[i].transanim.y *= g_scale;
			obj->bones[i].transanim.z *= g_scale;
			Vector_Add(&obj->bones[i].transanim, pos, &obj->bones[i].transanim);

			// Compute Global Object Scale AND Global Animation Scale
			obj->bones[i].scaleanim.x = (obj->bones[i].scaleinit.x + 1.f) * g_scale;
			obj->bones[i].scaleanim.y = (obj->bones[i].scaleinit.y + 1.f) * g_scale;
			obj->bones[i].scaleanim.z = (obj->bones[i].scaleinit.z + 1.f) * g_scale;
		}
	}
}

void EE_RT(D3DTLVERTEX * in, EERIE_3D * out);
void EE_P(EERIE_3D * in, D3DTLVERTEX * out);

extern long INTERPOLATE_BETWEEN_BONES;

/* Transform object vertices  */
int		Cedric_TransformVerts(INTERACTIVE_OBJ * io, EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, EERIE_3D * pos, EERIE_3D * angle)
{
	int				i, v;

	EERIE_3DPAD * inVert;
	EERIE_VERTEX * outVert;

 	/* Transform & project all vertices */
	for (i = 0; i != obj->nb_bones; i++)
	{
		EERIEMATRIX	 matrix;
		EERIE_3D	vector;

		MatrixFromQuat(&matrix, &obj->bones[i].quatanim);
		Vector_Copy(&vector, &obj->bones[i].transanim);

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


		for (v = 0; v != obj->bones[i].nb_idxvertices; v++)
		{
			inVert  = &eobj->vertexlocal[obj->bones[i].idxvertices[v]];
			outVert = &eobj->vertexlist3[obj->bones[i].idxvertices[v]];

			TransformVertexMatrix(&matrix, (EERIE_3D *)inVert, &outVert->v);

			Vector_Add(&outVert->v, &vector, &outVert->v);
			outVert->vert.sx = outVert->v.x;
			outVert->vert.sy = outVert->v.y;
			outVert->vert.sz = outVert->v.z;
		}
	}

	if (FLAG_ALLOW_CLOTHES && eobj->cdata && eobj->sdata)
	{
		for (i = 0; i < eobj->nbvertex; i++)
		{
			eobj->vertexlist[i].vert.sx = eobj->vertexlist3[i].v.x - pos->x;
			eobj->vertexlist[i].vert.sy = eobj->vertexlist3[i].v.y - pos->y;
			eobj->vertexlist[i].vert.sz = eobj->vertexlist3[i].v.z - pos->z;
		}
	}

	for (i = 0; i < eobj->nbvertex; i++)
	{
		outVert = &eobj->vertexlist3[i];
		AddToBBox3D(io, &outVert->v);
		EE_RT(&outVert->vert, &outVert->vworld);
		EE_P(&outVert->vworld, &outVert->vert);

		// Updates 2D Bounding Box
		if (outVert->vert.rhw > 0.f)
		{
			BBOXMIN.x = __min(BBOXMIN.x, outVert->vert.sx);
			BBOXMAX.x = __max(BBOXMAX.x, outVert->vert.sx);
			BBOXMIN.y = __min(BBOXMIN.y, outVert->vert.sy);
			BBOXMAX.y = __max(BBOXMAX.y, outVert->vert.sy);
		}
	}

	if ((io)
	        &&	(io->ioflags & IO_NPC)
	        &&	(io->_npcdata->behavior & BEHAVIOUR_FIGHT)
	        &&	(EEDistance3D(&io->pos, &player.pos) < 240.f))
		return true;

	if ((io != inter.iobj[0])
	        &&	(!EXTERNALVIEW)
	        &&	(!eobj->cdata)
	        &&	((BBOXMIN.x >= DANAESIZX - 1)
	             ||	(BBOXMAX.x <= 1)
	             ||	(BBOXMIN.y >= DANAESIZY - 1)
	             ||	(BBOXMAX.y <= 1))
	   )
	{
		return false;
	}

	if (ARX_SCENE_PORTAL_ClipIO(io, eobj, pos, &BBOXMIN, &BBOXMAX))
		return false;

	return true;
}
extern INTERACTIVE_OBJ * DESTROYED_DURING_RENDERING;
long special_color_flag = 0;
EERIE_RGB special_color;
extern long DEBUG_PATHFAIL;
extern long FINAL_RELEASE;
extern long TRAP_DETECT;
extern long TRAP_SECRET;
extern long FRAME_COUNT;
extern long ALTERNATE_LIGHTING;

extern float GLOBAL_LIGHT_FACTOR;

/* Object dynamic lighting */
bool	Cedric_ApplyLighting(EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, INTERACTIVE_OBJ * io, EERIE_3D * pos, long typ)
{
	EERIE_RGB		infra;
	int				i, v, l;
	EERIE_3D		tv;
	EERIE_3D		vTLights[32];//MAX_LLIGHTS];				/* Same as above but in bone space (for faster calculation) */



	ZeroMemory(&infra, sizeof(EERIE_RGB));


	special_color_flag = 0;

	if (io)
	{
		float poisonpercent = 0.f;
		float trappercent = 0.f;
		float secretpercent = 0.f;

		if (io->ioflags & IO_NPC)
		{
			if ((DEBUG_PATHFAIL) && (!FINAL_RELEASE))
			{
				if (io->_npcdata->pathfind.listnb == -2)
					trappercent = 1;

				if (io->_npcdata->pathfind.pathwait)
					poisonpercent = 1;
			}

			if (io->_npcdata->poisonned > 0.f)
			{
				poisonpercent = io->_npcdata->poisonned * DIV20;

				if (poisonpercent > 1.f) poisonpercent = 1.f;
			}
		}

		if ((io->ioflags & IO_ITEM) && (io->poisonous > 0.f) && (io->poisonous_count != 0))
		{
			poisonpercent = (float)io->poisonous * DIV20;

			if (poisonpercent > 1.f) poisonpercent = 1.f;
		}

		if ((io->ioflags & IO_FIX) && (io->_fixdata->trapvalue > -1))
		{
			trappercent = (float)TRAP_DETECT - (float)io->_fixdata->trapvalue;

			if (trappercent > 0.f)
			{
				trappercent = 0.6f + trappercent * DIV100; 

				if (trappercent < 0.6f) trappercent = 0.6f;

				if (trappercent > 1.f) trappercent = 1.f;
			}
		}

		if ((io->ioflags & IO_FIX) && (io->secretvalue > -1))
		{
			secretpercent = (float)TRAP_SECRET - (float)io->secretvalue;

			if (secretpercent > 0.f)
			{
				secretpercent = 0.6f + secretpercent * DIV100; 
				if (secretpercent < 0.6f) secretpercent = 0.6f;
				else if (secretpercent > 1.f) secretpercent = 1.f;
			}
		}

		if (poisonpercent > 0.f)
		{
			special_color_flag = 1;
			special_color.r = 0.f; 
			special_color.g = 1.f;
			special_color.b = 0.f; 
		}

		if (trappercent > 0.f)
		{
			special_color_flag = 1;
			special_color.r = trappercent;
			special_color.g = 1.f - trappercent;
			special_color.b = 1.f - trappercent;
		}

		if (secretpercent > 0.f)
		{
			special_color_flag = 1;
			special_color.r = 1.f - secretpercent;
			special_color.g = 1.f - secretpercent;
			special_color.b = secretpercent;
		}

		if (io->ioflags & IO_FREEZESCRIPT)
		{
			special_color_flag = 1;
			special_color.r = 0.f;
			special_color.g = 0.f;
			special_color.b = 1.f;
		}

		if (io->sfx_flag & SFX_TYPE_YLSIDE_DEATH)
		{
			if (io->show == SHOW_FLAG_TELEPORTING)
			{

				float fTime = io->sfx_time + FrameDiff;
				ARX_CHECK_ULONG(fTime);

				io->sfx_time = 	ARX_CLEAN_WARN_CAST_LONG(fTime);

				if (io->sfx_time >= ARXTimeUL())
					io->sfx_time = ARXTimeUL();


			}
			else
			{
				special_color_flag = 1;
				float elapsed = ARXTime - io->sfx_time;

				if (elapsed > 0.f)
				{
					if (elapsed < 3000.f) // 5 seconds to red
					{
						float ratio = elapsed * DIV3000;
						special_color.r = 1.f;
						special_color.g = 1.f - ratio;
						special_color.b = 1.f - ratio;
						AddRandomSmoke(io, 1);
					}
					else if (elapsed < 6000.f) // 5 seconds to White
					{
						float ratio = (elapsed - 3000.f) * DIV3000;
						special_color.r = ratio;
						special_color_flag = 2;
						AddRandomSmoke(io, 2);
					}
					else // SFX finish
					{
						special_color_flag = 0;

						io->sfx_time = 0;

						if (io->ioflags & IO_NPC)
						{
							MakePlayerAppearsFX(io);
							AddRandomSmoke(io, 50);
							EERIE_RGB rgb;
							unsigned long color = io->_npcdata->blood_color;
							rgb.r = (float)((long)((color >> 16) & 255)) * DIV255;
							rgb.g = (float)((long)((color >> 8) & 255)) * DIV255;
							rgb.b = (float)((long)((color) & 255)) * DIV255;
							EERIE_SPHERE sp;
							sp.origin.x = io->pos.x;
							sp.origin.y = io->pos.y;
							sp.origin.z = io->pos.z;
							sp.radius = 200.f;
							long count = 6;

							while (count--)
							{
								SpawnGroundSplat(&sp, &rgb, rnd() * 30.f + 30.f, 1);
								sp.origin.y -= rnd() * 150.f;

								if (count == 0)
									ARX_PARTICLES_Spawn_Splat(&sp.origin, 200, io->_npcdata->blood_color, 0, io, 1);
								else
									ARX_PARTICLES_Spawn_Splat(&sp.origin, 200, io->_npcdata->blood_color, 0, io, 0);

								sp.origin.x = io->pos.x + rnd() * 200.f - 100.f;
								sp.origin.y = io->pos.y + rnd() * 20.f - 10.f;
								sp.origin.z = io->pos.z + rnd() * 200.f - 100.f;
								sp.radius = rnd() * 100.f + 100.f;
							}

							long nn = GetFreeDynLight();

							if (nn >= 0)
							{
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

							if (io->sfx_flag & SFX_TYPE_INCINERATE)
							{
								io->sfx_flag &= ~SFX_TYPE_INCINERATE;
								io->sfx_flag &= ~SFX_TYPE_YLSIDE_DEATH;
								long num = ARX_SPELLS_GetSpellOn(io, SPELL_INCINERATE);

								if (num < 0)
									num = ARX_SPELLS_GetSpellOn(io, SPELL_MASS_INCINERATE);

								if (num >= 0)
								{
									spells[num].tolive = 0;
									float damages = 20 * spells[num].caster_level;
									damages = ARX_SPELLS_ApplyFireProtection(io, damages);

									if (ValidIONum(spells[num].caster))
										ARX_DAMAGES_DamageNPC(io, damages, spells[num].caster, 1, &inter.iobj[spells[num].caster]->pos);
									else
										ARX_DAMAGES_DamageNPC(io, damages, spells[num].caster, 1, &io->pos);

									ARX_SOUND_PlaySFX(SND_SPELL_FIRE_HIT, &io->pos);
								}
							}
							else
							{
								io->sfx_flag &= ~SFX_TYPE_YLSIDE_DEATH;
								ARX_INTERACTIVE_DestroyIO(io);
								DESTROYED_DURING_RENDERING = io;
								return false;
							}
						}
					}
				}
			}
		}
	}

	if (eobj->drawflags & DRAWFLAG_HIGHLIGHT)
	{
		special_color_flag	=	4;

		special_color.r		=	ARX_CLEAN_WARN_CAST_FLOAT(iHighLight);   //100.f;
		special_color.g		=	ARX_CLEAN_WARN_CAST_FLOAT(iHighLight);   //100.f;
		special_color.b		=	ARX_CLEAN_WARN_CAST_FLOAT(iHighLight);   //100.f;

	}

	if (FRAME_COUNT > 0) return true;

	if (Project.improve)
	{
		if (io != NULL)
		{
			infra.r = io->infracolor.r;
			infra.g = io->infracolor.g;
			infra.b = io->infracolor.b;
		}
		else
		{
			infra.r = 0.6f;
			infra.g = 0.f;
			infra.b = 1.f;
		}
	}

	/* Get nearest lights */
	tv = *pos;

	if (typ & ANIMQUATTYPE_FIRST_PERSON)
	{
		tv.x = subj.pos.x;
		tv.y = subj.pos.y;
		tv.z = subj.pos.z;
	}

	if ((io) && (io->obj->fastaccess.view_attach >= 0) && (io->obj->fastaccess.head_group_origin != -1))
	{
		tv.y = io->obj->vertexlist3[io->obj->fastaccess.head_group_origin].v.y + 10;
	}
	else tv.y -= 90.f;

	llightsInit();

	for (i = 0; i < TOTIOPDL; i++)
	{
		if (!(GetMaxManhattanDistance(&IO_PDL[i]->pos, &tv) <= IO_PDL[i]->fallend + 500.f))
			continue;

		Insertllight(IO_PDL[i], TRUEEEDistance3D(&IO_PDL[i]->pos, &tv));
	}

	for (i = 0; i < TOTPDL; i++)
	{
		if (!(GetMaxManhattanDistance(&PDL[i]->pos, &tv) <= PDL[i]->fallend + 500.f))
			continue;

		Insertllight(PDL[i], TRUEEEDistance3D(&PDL[i]->pos, &tv));//-PDL[i]->fallstart);
	}

	if (!USEINTERNORM)
	{
		/* Apply light on all vertices */
		for (i = 0; i != obj->nb_bones; i++)
		{
			/* Get light value for each vertex */
			for (v = 0; v != obj->bones[i].nb_idxvertices; v++)
			{
				EERIE_3DPAD *	inVert;
				EERIE_3D	*	posVert;
				float			r, g, b;
				long	ir, ig, ib;

				if (io)
				{
					inVert  = (EERIE_3DPAD *)&io->obj->vertexlist[obj->bones[i].idxvertices[v]].norm; 
					posVert  = (EERIE_3D *)&io->obj->vertexlist3[obj->bones[i].idxvertices[v]].v;
				}
				else
				{
					inVert  = (EERIE_3DPAD *)&eobj->vertexlist[obj->bones[i].idxvertices[v]].norm;
					posVert  = (EERIE_3D *)&eobj->vertexlist3[obj->bones[i].idxvertices[v]].v;
				}

				/* Ambient light */
				if ((io) && (io->ioflags & (IO_NPC | IO_ITEM)))
				{
					r = g = b = NPC_ITEMS__AMBIENT_VALUE_255;
				}
				else
				{
					r = ACTIVEBKG->ambient255.r;
					g = ACTIVEBKG->ambient255.g;
					b = ACTIVEBKG->ambient255.b;
				}

				/* Dynamic lights */
				for (l = 0 ; l != MAX_LLIGHTS; l++)
				{
					EERIE_LIGHT * Cur_llights = llights[l];

					if (Cur_llights)
					{
						// tsu
						if (!(GetMaxManhattanDistance(&Cur_llights->pos, (EERIE_3D *)&posVert) <= Cur_llights->fallend))
						{
							TSU_TEST_NB_LIGHT ++;
							continue;
						}

						float	cosangle;
						float distance = EEDistance3D((EERIE_3D *)posVert, &Cur_llights->pos);

						/* Evaluate its intensity depending on the distance Light<->Object */
						if (distance <= Cur_llights->fallstart)
							cosangle = Cur_llights->intensity * GLOBAL_LIGHT_FACTOR;
						else
						{
							float p = ((Cur_llights->fallend - distance) * Cur_llights->falldiffmul);

							if (p <= 0.f)
								cosangle = 0.f;
							else
								cosangle = p * Cur_llights->precalc; //>intensity*GLOBAL_LIGHT_FACTOR;
						}

						r += Cur_llights->rgb255.r * cosangle;
						g += Cur_llights->rgb255.g * cosangle;
						b += Cur_llights->rgb255.b * cosangle;
					}
					else
						break;
				}

				if (special_color_flag)
				{
					if (special_color_flag & 1)
					{
						r *= special_color.r;
						g *= special_color.g;
						b *= special_color.b;
					}
					else if (special_color_flag & 2)
					{
						r = 1.f;
						g = 0.f;
						b = 0.f;
					}
					else if (special_color_flag & 4) // HIGHLIGHT
					{
						r += special_color.r;
						g += special_color.g;
						b += special_color.b;
					}
				}

				/* PACK color */
				F2L(r, &ir);
				F2L(g, &ig);
				F2L(b, &ib);

				ir = clipByte255(ir);
				ig = clipByte255(ig);
				ib = clipByte255(ib);
	
				eobj->vertexlist3[obj->bones[i].idxvertices[v]].vert.color = (0xFF000000L | ((ir) << 16) | ((ig) << 8) | (ib));
			}
		}
	}
	
	else
	{
		/* Apply light on all vertices */
		for (i = 0; i != obj->nb_bones; i++)
		{
			EERIE_QUAT	qt1;
		
			EERIEMATRIX matrix;//,omatrix;
			Quat_Copy(&qt1, &obj->bones[i].quatanim);
			Quat_Reverse(&qt1);
			MatrixFromQuat(&matrix, &qt1);
			//	FMatrixInvert(matrix,omatrix);

			/* Get light value for each vertex */
			for (v = 0; v != obj->bones[i].nb_idxvertices; v++)
			{
				EERIE_3DPAD *	inVert;

				float			r, g, b;
				long	ir, ig, ib;

				inVert  = (EERIE_3DPAD *)&eobj->vertexlist[obj->bones[i].idxvertices[v]].norm;

				/* Ambient light */
				if ((io) && (io->ioflags & (IO_NPC | IO_ITEM)))
				{
					r = g = b = NPC_ITEMS__AMBIENT_VALUE_255;
				}
				else
				{
					r = ACTIVEBKG->ambient255.r;
					g = ACTIVEBKG->ambient255.g;
					b = ACTIVEBKG->ambient255.b;
				}


				/* Dynamic lights */
				for (l = 0 ; l != MAX_LLIGHTS; l++)
				{
					EERIE_LIGHT * Cur_llights = llights[l];

					if (Cur_llights)
					{
						float	cosangle;
						EERIE_3D * Cur_vTLights = &vTLights[l];
						EERIE_3D tl;
						tl.x = (Cur_llights->pos.x - eobj->vertexlist3[obj->bones[i].idxvertices[v]].v.x);
						tl.y = (Cur_llights->pos.y - eobj->vertexlist3[obj->bones[i].idxvertices[v]].v.y);
						tl.z = (Cur_llights->pos.z - eobj->vertexlist3[obj->bones[i].idxvertices[v]].v.z);
						float dista = Vector_Magnitude(&tl);

						if (dista < Cur_llights->fallend)
						{
							float divv = 1.f / dista;
							tl.x *= divv;
							tl.y *= divv;
							tl.z *= divv;

							VectorMatrixMultiply(Cur_vTLights, &tl, &matrix);

							cosangle = (inVert->x * Cur_vTLights->x +
							            inVert->y * Cur_vTLights->y +
							            inVert->z * Cur_vTLights->z);

							/* If light visible */
							if (cosangle > 0.0f)
							{
								/* Evaluate its intensity depending on the distance Light<->Object */
								if (dista <= Cur_llights->fallstart)
									cosangle *= Cur_llights->precalc; 
								else
								{
									float p = ((Cur_llights->fallend - dista) * Cur_llights->falldiffmul);

									if (p <= 0.f)
										cosangle = 0.f;
									else
										cosangle *= p * Cur_llights->precalc; 
								}

								r += Cur_llights->rgb255.r * cosangle;
								g += Cur_llights->rgb255.g * cosangle;
								b += Cur_llights->rgb255.b * cosangle;
							}
						}
					}
					else
						break;
				}

				/* Fake adjust */
				if (Project.improve)
				{
					r *= infra.r;
					g *= infra.g;
					b *= infra.b;
				}

				if (special_color_flag)
				{
					if (special_color_flag & 1)
					{
						r *= special_color.r;
						g *= special_color.g;
						b *= special_color.b;
					}
					else if (special_color_flag & 2)
					{
						r = 1.f;
						g = 0.f;
						b = 0.f;
					}
					else if (special_color_flag & 4) // HIGHLIGHT
					{
						r += special_color.r;
						g += special_color.g;
						b += special_color.b;
					}
				}

				/* PACK color */
				F2L(r, &ir);
				F2L(g, &ig);
				F2L(b, &ib);

				ir = clipByte255(ir);
				ig = clipByte255(ig);
				ib = clipByte255(ib);

				eobj->vertexlist3[obj->bones[i].idxvertices[v]].vert.color = (0xFF000000L | ((ir) << 16) | ((ig) << 8) | (ib));
			}
		}
	}

	return true;
}

void Cedric_PrepareHalo(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, INTERACTIVE_OBJ * io, EERIE_3D * pos, EERIE_3D & ftr)
{
	EERIE_3D cam_vector, t_vector;
	cam_vector.x = -EEsin(DEG2RAD(ACTIVECAM->angle.b)) * EEcos(DEG2RAD(ACTIVECAM->angle.a));
	cam_vector.y = EEsin(DEG2RAD(ACTIVECAM->angle.a));
	cam_vector.z = EEcos(DEG2RAD(ACTIVECAM->angle.b)) * EEcos(DEG2RAD(ACTIVECAM->angle.a));

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

//-----------------------------------------------------------------------------
__forceinline void ARX_ClippZ(D3DTLVERTEX * _pD3DA, D3DTLVERTEX * _pD3DB, EERIE_VERTEX * _pVertexA, EERIE_VERTEX * _pVertexB, D3DTLVERTEX * _pOut)
{
	EERIE_3D e3dTemp;

	float fDenom	=	(SOFTNEARCLIPPZ - _pVertexB->vworld.z) / (_pVertexA->vworld.z - _pVertexB->vworld.z);
	e3dTemp.x		=	(_pVertexA->vworld.x - _pVertexB->vworld.x) * fDenom + _pVertexB->vworld.x;
	e3dTemp.y		=	(_pVertexA->vworld.y - _pVertexB->vworld.y) * fDenom + _pVertexB->vworld.y;
	e3dTemp.z		=	SOFTNEARCLIPPZ ;

	float fRA, fGA, fBA;
	float fRB, fGB, fBB;


	fRA = ARX_CLEAN_WARN_CAST_FLOAT((_pD3DA->color >> 16) & 255);
	fGA = ARX_CLEAN_WARN_CAST_FLOAT((_pD3DA->color >> 8) & 255);
	fBA = ARX_CLEAN_WARN_CAST_FLOAT(_pD3DA->color & 255);
	fRB = ARX_CLEAN_WARN_CAST_FLOAT((_pD3DB->color >> 16) & 255);
	fGB = ARX_CLEAN_WARN_CAST_FLOAT((_pD3DB->color >> 8) & 255);
	fBB = ARX_CLEAN_WARN_CAST_FLOAT(_pD3DB->color & 255);

	float fRC, fGC, fBC;
	fRC = (fRA - fRB) * fDenom + fRB;
	fGC = (fGA - fGB) * fDenom + fGB;
	fBC = (fBA - fBB) * fDenom + fBB;

	_pOut->color	=	(((int)fRC) << 16) | (((int)fGC) << 8) | ((int)fBC);
	_pOut->tu		=	(_pD3DA->tu - _pD3DB->tu) * fDenom + _pD3DB->tu;
	_pOut->tv		=	(_pD3DA->tv - _pD3DB->tv) * fDenom + _pD3DB->tv;

	EE_P(&e3dTemp, _pOut);
}

//-----------------------------------------------------------------------------
void ARX_DrawPrimitive_ClippZ(D3DTLVERTEX * _pVertexA, D3DTLVERTEX * _pVertexB, D3DTLVERTEX * _pOut, float _fAdd = 0.f);
void ARX_DrawPrimitive_ClippZ(D3DTLVERTEX * _pVertexA, D3DTLVERTEX * _pVertexB, D3DTLVERTEX * _pOut, float _fAdd)
{
	EERIE_3D e3dTemp;
	float fDenom = ((SOFTNEARCLIPPZ + _fAdd) - _pVertexB->sz) / (_pVertexA->sz - _pVertexB->sz);
	e3dTemp.x = (_pVertexA->sx - _pVertexB->sx) * fDenom + _pVertexB->sx;
	e3dTemp.y = (_pVertexA->sy - _pVertexB->sy) * fDenom + _pVertexB->sy;
	e3dTemp.z = SOFTNEARCLIPPZ + _fAdd;

	float fRA, fGA, fBA;
	float fRB, fGB, fBB;


	fRA = ARX_CLEAN_WARN_CAST_FLOAT((_pVertexA->color >> 16) & 255);
	fGA = ARX_CLEAN_WARN_CAST_FLOAT((_pVertexA->color >> 8) & 255);
	fBA = ARX_CLEAN_WARN_CAST_FLOAT(_pVertexA->color & 255);
	fRB = ARX_CLEAN_WARN_CAST_FLOAT((_pVertexB->color >> 16) & 255);
	fGB = ARX_CLEAN_WARN_CAST_FLOAT((_pVertexB->color >> 8) & 255);
	fBB = ARX_CLEAN_WARN_CAST_FLOAT(_pVertexB->color & 255);


	float fRC, fGC, fBC;
	fRC = (fRA - fRB) * fDenom + fRB;
	fGC = (fGA - fGB) * fDenom + fGB;
	fBC = (fBA - fBB) * fDenom + fBB;

	_pOut->color	= (((int) fRC) << 16) | (((int)fGC) << 8) | ((int) fBC);
	_pOut->tu		= (_pVertexA->tu - _pVertexB->tu) * fDenom + _pVertexB->tu;
	_pOut->tv		= (_pVertexA->tv - _pVertexB->tv) * fDenom + _pVertexB->tv;

	EE_P(&e3dTemp, _pOut);
}

//-----------------------------------------------------------------------------
D3DTLVERTEX * GetNewVertexList(EERIE_FACE * _pFace, float _fInvisibility, TextureContainer * _pTex)
{
	if ((_pFace->facetype & POLY_TRANS) ||
	        (_fInvisibility > 0.f))
	{
		float fTransp;

		if (_fInvisibility > 0.f)
			fTransp = 2.f - _fInvisibility;
		else
			fTransp = _pFace->transval;

		if (fTransp >= 2.f) //MULTIPLICATIVE
		{
			fTransp *= DIV2;
			fTransp += 0.5f;

			return PushVertexInTableCull_TMultiplicative(_pTex);
		}
		else
		{
			if (fTransp >= 1.f) //ADDITIVE
			{
				fTransp -= 1.f;
				return PushVertexInTableCull_TAdditive(_pTex);
			}
			else
			{
				if (fTransp > 0.f) //NORMAL TRANS
				{
					fTransp = 1.f - fTransp;
					return PushVertexInTableCull_TNormalTrans(_pTex);
				}
				else  //SUBTRACTIVE
				{
					fTransp = 1.f - fTransp;
					return PushVertexInTableCull_TSubstractive(_pTex);
				}
			}
		}
	}
	else
	{
		return PushVertexInTableCull(_pTex);
	}
}

//-----------------------------------------------------------------------------
int ARX_SoftClippZ(EERIE_VERTEX * _pVertex1, EERIE_VERTEX * _pVertex2, EERIE_VERTEX * _pVertex3, D3DTLVERTEX ** _ptV, EERIE_FACE * _pFace, float _fInvibility, TextureContainer * _pTex, bool _bBump, bool _bZMapp, EERIE_3DOBJ * _pObj, int _iNumFace, long * _pInd, INTERACTIVE_OBJ * _pioInteractive, bool _bNPC, long _lSpecialColorFlag, EERIE_RGB * _pRGB, bool _bPassTANDL)
{
	int iPointAdd = 3;
	int iClipp = 0;

	if ((_pVertex1->vworld.z) < SOFTNEARCLIPPZ) iClipp |= 1;

	if ((_pVertex2->vworld.z) < SOFTNEARCLIPPZ) iClipp |= 2;

	if ((_pVertex3->vworld.z) < SOFTNEARCLIPPZ) iClipp |= 4;

	D3DTLVERTEX D3DClippZ1, D3DClippZ2;
	D3DTLVERTEX * pD3DPointAdd = NULL;
	D3DTLVERTEX * ptV = *_ptV;

	switch (iClipp)
	{
		case 1:						//pt1 outside
			ARX_ClippZ(&ptV[0], &ptV[1], _pVertex1, _pVertex2, &D3DClippZ1);
			ARX_ClippZ(&ptV[0], &ptV[2], _pVertex1, _pVertex3, &D3DClippZ2);
			pD3DPointAdd = GetNewVertexList(_pFace,
			                                _fInvibility,
			                                _pTex);
			ptV = pD3DPointAdd - 3;

			if (pD3DPointAdd)
			{
				pD3DPointAdd[0] = D3DClippZ1;
				pD3DPointAdd[1] = ptV[1];
				pD3DPointAdd[2] = D3DClippZ2;
			}

			ptV[0] = D3DClippZ2;
			iPointAdd = 6;
			break;
		case 2:						//pt2 outside
			ARX_ClippZ(&ptV[1], &ptV[2], _pVertex2, _pVertex3, &D3DClippZ1);
			ARX_ClippZ(&ptV[1], &ptV[0], _pVertex2, _pVertex1, &D3DClippZ2);
			pD3DPointAdd = GetNewVertexList(_pFace,
			                                _fInvibility,
			                                _pTex);
			ptV = pD3DPointAdd - 3;

			if (pD3DPointAdd)
			{
				pD3DPointAdd[0] = ptV[2];
				pD3DPointAdd[1] = D3DClippZ1;
				pD3DPointAdd[2] = D3DClippZ2;
			}

			ptV[1] = D3DClippZ2;
			iPointAdd = 6;
			break;
		case 4:						//pt3 outside
			ARX_ClippZ(&ptV[2], &ptV[0], _pVertex3, _pVertex1, &D3DClippZ1);
			ARX_ClippZ(&ptV[2], &ptV[1], _pVertex3, _pVertex2, &D3DClippZ2);
			pD3DPointAdd = GetNewVertexList(_pFace,
			                                _fInvibility,
			                                _pTex);
			ptV = pD3DPointAdd - 3;

			if (pD3DPointAdd)
			{
				pD3DPointAdd[0] = ptV[0];
				pD3DPointAdd[1] = D3DClippZ2;
				pD3DPointAdd[2] = D3DClippZ1;
			}

			ptV[2] = D3DClippZ2;
			iPointAdd = 6;
			break;
		case 3:						//pt1_2 outside
			ARX_ClippZ(&ptV[0], &ptV[2], _pVertex1, _pVertex3, &D3DClippZ1);
			ARX_ClippZ(&ptV[1], &ptV[2], _pVertex2, _pVertex3, &D3DClippZ2);
			ptV[0] = D3DClippZ1;
			ptV[1] = D3DClippZ2;
			break;
		case 5:						//pt1_3 outside
			ARX_ClippZ(&ptV[0], &ptV[1], _pVertex1, _pVertex2, &D3DClippZ1);
			ARX_ClippZ(&ptV[2], &ptV[1], _pVertex3, _pVertex2, &D3DClippZ2);
			ptV[0] = D3DClippZ1;
			ptV[2] = D3DClippZ2;
			break;
		case 6:						//pt2_3 outside
			ARX_ClippZ(&ptV[2], &ptV[0], _pVertex3, _pVertex1, &D3DClippZ1);
			ARX_ClippZ(&ptV[1], &ptV[0], _pVertex2, _pVertex1, &D3DClippZ2);
			ptV[1] = D3DClippZ1;
			ptV[2] = D3DClippZ2;
			break;
		case 7:
			return 0;
	}

	if (pD3DPointAdd)
	{
		*_ptV = ptV;

		if (_bBump)
		{
			PushInterBump(_pTex, pD3DPointAdd);
		}

		if (_bZMapp)
		{
			CalculateInterZMapp(_pObj, _iNumFace, _pInd, _pTex, pD3DPointAdd);
		}

		if ((_pFace->facetype & POLY_METAL)
		        || ((_pTex) && (_pTex->userflags & POLY_METAL)))
		{
			if (_bNPC)
			{
				D3DTLVERTEX * tv2;

				if (_lSpecialColorFlag & 2)
				{
					tv2 = PushVertexInTableCull(&TexSpecialColor);
					memcpy((void *)tv2, (void *)pD3DPointAdd, sizeof(D3DTLVERTEX) * 3);
					tv2[0].color = tv2[1].color = tv2[2].color = _EERIERGB(_pRGB->r);
				}

				tv2 = PushVertexInTableCull_TMetal(_pTex);
				unsigned long * pulNbVertexList_TMetal = &_pTex->ulNbVertexListCull_TMetal;

				memcpy((void *)tv2, (void *)pD3DPointAdd, sizeof(D3DTLVERTEX) * 3);

				long r, g, b;
				long todo = 0;

				for (long j = 0; j < 3; j++)
				{
					r = (tv2[j].color >> 16) & 255;
					g = (tv2[j].color >> 8) & 255;
					b = tv2[j].color & 255;

					if (r > 192 || g > 192 || b > 192)
					{
						todo++;
					}

					r -= 192;

					if (r < 0.f) r = 0;

					g -= 192;

					if (g < 0.f) g = 0;

					b -= 192;

					if (b < 0.f) b = 0;

					tv2[j].color = 0xFF000000 | (r << 18) | (g << 10) | (b << 2);
				}

				if (!todo)
				{
					*pulNbVertexList_TMetal -= 3;
				}
			}
			else
			{
				ARX_D3DVERTEX * vert_list_metal			= PushVertexInTableCull_TMetal(_pTex);
				unsigned long * pulNbVertexList_TMetal	= &_pTex->ulNbVertexListCull_TMetal;

				memcpy((void *)vert_list_metal, (void *)pD3DPointAdd, sizeof(D3DTLVERTEX) * 3);
				D3DTLVERTEX * tl = vert_list_metal;

				long r, g, b;
				long todo = 0;

				r = g = b = 0 ;

				for (long j = 0 ; j < 3 ; j++)
				{
					r = (tl->color >> 16) & 255;
					g = (tl->color >> 8) & 255;
					b = tl->color & 255;

					if (r > 192 || g > 192 || b > 192)
					{
						todo++;
					}

					r -= 192;

					if (r < 0.f) r = 0;

					g -= 192;

					if (g < 0.f) g = 0;

					b -= 192;

					if (b < 0.f) b = 0;

					tl->color = 0xFF000000 | (r << 18) | (g << 10) | (b << 2);
					tl++;
				}

				if (todo)
				{
					if ((todo > 2) && (rnd() > 0.997f))
					{
						if (_pioInteractive)
							SpawnMetalShine((EERIE_3D *)&_pObj->vertexlist3[_pObj->facelist[_iNumFace].vid[0]].vert, r, g, b, GetInterNum(_pioInteractive));
					}
				}
				else
				{
					*pulNbVertexList_TMetal -= 3;
				}
			}
		}
	}

	return iPointAdd;
}
extern long IsInGroup(EERIE_3DOBJ * obj, long vert, long tw);
//-----------------------------------------------------------------------------
bool ARX_DrawPrimitive_SoftClippZ(D3DTLVERTEX * _pVertex1, D3DTLVERTEX * _pVertex2, D3DTLVERTEX * _pVertex3, float _fAddZ)
{
	int iClipp = 0;

	if (_pVertex1->sz < (SOFTNEARCLIPPZ + _fAddZ)) iClipp |= 1;

	if (_pVertex2->sz < (SOFTNEARCLIPPZ + _fAddZ)) iClipp |= 2;

	if (_pVertex3->sz < (SOFTNEARCLIPPZ + _fAddZ)) iClipp |= 4;

	D3DTLVERTEX D3DClippZ1, D3DClippZ2;
	D3DTLVERTEX pD3DPointAdd[6];
	int			iNbTotVertex = 3;

	switch (iClipp)
	{
		case 0:
			EERIE_3D e3dTemp;
			e3dTemp.x = _pVertex1->sx;
			e3dTemp.y = _pVertex1->sy;
			e3dTemp.z = _pVertex1->sz;
			EE_P(&e3dTemp, &pD3DPointAdd[0]);
			e3dTemp.x = _pVertex2->sx;
			e3dTemp.y = _pVertex2->sy;
			e3dTemp.z = _pVertex2->sz;
			EE_P(&e3dTemp, &pD3DPointAdd[1]);
			e3dTemp.x = _pVertex3->sx;
			e3dTemp.y = _pVertex3->sy;
			e3dTemp.z = _pVertex3->sz;
			EE_P(&e3dTemp, &pD3DPointAdd[2]);
			pD3DPointAdd[0].color = _pVertex1->color;
			pD3DPointAdd[0].specular = _pVertex1->specular;
			pD3DPointAdd[0].tu = _pVertex1->tu;
			pD3DPointAdd[0].tv = _pVertex1->tv;
			pD3DPointAdd[1].color = _pVertex2->color;
			pD3DPointAdd[1].specular = _pVertex2->specular;
			pD3DPointAdd[1].tu = _pVertex2->tu;
			pD3DPointAdd[1].tv = _pVertex2->tv;
			pD3DPointAdd[2].color = _pVertex3->color;
			pD3DPointAdd[2].specular = _pVertex3->specular;
			pD3DPointAdd[2].tu = _pVertex3->tu;
			pD3DPointAdd[2].tv = _pVertex3->tv;
			break;
		case 1:						//pt1 outside
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex2, &D3DClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex3, &D3DClippZ2, _fAddZ);
			pD3DPointAdd[0] = D3DClippZ2;
			pD3DPointAdd[1] = *_pVertex2;
			EE_P2(&pD3DPointAdd[1], &pD3DPointAdd[1]);
			pD3DPointAdd[2] = *_pVertex3;
			EE_P2(&pD3DPointAdd[2], &pD3DPointAdd[2]);
			pD3DPointAdd[3] = D3DClippZ1;
			pD3DPointAdd[4] = pD3DPointAdd[1];
			pD3DPointAdd[5] = D3DClippZ2;
			iNbTotVertex = 6;
			break;
		case 2:						//pt2 outside
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex3, &D3DClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex1, &D3DClippZ2, _fAddZ);
			pD3DPointAdd[0] = *_pVertex1;
			EE_P2(&pD3DPointAdd[0], &pD3DPointAdd[0]);
			pD3DPointAdd[1] = D3DClippZ2;
			pD3DPointAdd[2] = *_pVertex3;
			EE_P2(&pD3DPointAdd[2], &pD3DPointAdd[2]);
			pD3DPointAdd[3] = pD3DPointAdd[2];
			pD3DPointAdd[4] = D3DClippZ1;
			pD3DPointAdd[5] = D3DClippZ2;
			iNbTotVertex = 6;
			break;
		case 4:						//pt3 outside
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex1, &D3DClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex2, &D3DClippZ2, _fAddZ);
			pD3DPointAdd[0] = *_pVertex1;
			EE_P2(&pD3DPointAdd[0], &pD3DPointAdd[0]);
			pD3DPointAdd[1] = *_pVertex2;
			EE_P2(&pD3DPointAdd[1], &pD3DPointAdd[1]);
			pD3DPointAdd[2] = D3DClippZ2;
			pD3DPointAdd[3] = pD3DPointAdd[0];
			pD3DPointAdd[4] = D3DClippZ2;
			pD3DPointAdd[5] = D3DClippZ1;
			iNbTotVertex = 6;
			break;
		case 3:						//pt1_2 outside
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex3, &D3DClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex3, &D3DClippZ2, _fAddZ);
			pD3DPointAdd[0] = D3DClippZ1;
			pD3DPointAdd[1] = D3DClippZ2;
			pD3DPointAdd[2] = *_pVertex3;
			EE_P2(&pD3DPointAdd[2], &pD3DPointAdd[2]);
			break;
		case 5:						//pt1_3 outside
			ARX_DrawPrimitive_ClippZ(_pVertex1, _pVertex2, &D3DClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex2, &D3DClippZ2, _fAddZ);
			pD3DPointAdd[0] = D3DClippZ1;
			pD3DPointAdd[1] = *_pVertex2;
			EE_P2(&pD3DPointAdd[1], &pD3DPointAdd[1]);
			pD3DPointAdd[2] = D3DClippZ2;
			break;
		case 6:						//pt2_3 outside
			ARX_DrawPrimitive_ClippZ(_pVertex3, _pVertex1, &D3DClippZ1, _fAddZ);
			ARX_DrawPrimitive_ClippZ(_pVertex2, _pVertex1, &D3DClippZ2, _fAddZ);
			pD3DPointAdd[0] = *_pVertex1;
			EE_P2(&pD3DPointAdd[0], &pD3DPointAdd[0]);
			pD3DPointAdd[1] = D3DClippZ1;
			pD3DPointAdd[2] = D3DClippZ2;
			break;
		case 7:
			return false;
	}

	EERIEDRAWPRIM(GDevice,
	              D3DPT_TRIANGLELIST,
	              D3DFVF_TLVERTEX,
	              pD3DPointAdd,
					iNbTotVertex,
					0, (bSoftRender?EERIE_USEVB:0)  );
	return true;
}
long FORCE_FRONT_DRAW = 0;

//-----------------------------------------------------------------------------
extern long IN_BOOK_DRAW;
/* Render object */
void	Cedric_RenderObject2(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, INTERACTIVE_OBJ * io, EERIE_3D * pos, EERIE_3D & ftr, float invisibility)
{
	int			i;
	float		MAX_ZEDE = 0.f;

	// Sets IO BBox to calculated BBox :)
	if (io)
	{
		io->bbox1.x = (short) BBOXMIN.x;
		io->bbox2.x = (short) BBOXMAX.x;
		io->bbox1.y = (short) BBOXMIN.y;
		io->bbox2.y = (short) BBOXMAX.y;
	}

	if (invisibility == 1.f)
		return;

	float	ddist		= 0.f;
	long	need_halo;

	need_halo = 0;

	INTERACTIVE_OBJ * hio_helmet	= NULL;
	INTERACTIVE_OBJ * hio_armor		= NULL;
	INTERACTIVE_OBJ * hio_leggings	= NULL;
	INTERACTIVE_OBJ * hio_player	= NULL;
	INTERACTIVE_OBJ * use_io		= io;

	if ((!io)
	        &&	(IN_BOOK_DRAW)
	        &&	(eobj == inter.iobj[0]->obj))
		use_io = inter.iobj[0];

	if (use_io)
	{
		if (use_io == inter.iobj[0])
		{
			if ((player.equiped[EQUIP_SLOT_HELMET] != 0)
			        && ValidIONum(player.equiped[EQUIP_SLOT_HELMET]))
			{
				INTERACTIVE_OBJ * tio = inter.iobj[player.equiped[EQUIP_SLOT_HELMET]];

				if (tio->halo.flags & HALO_ACTIVE)
					hio_helmet = tio;
			}

			if ((player.equiped[EQUIP_SLOT_ARMOR] != 0)
			        &&	ValidIONum(player.equiped[EQUIP_SLOT_ARMOR]))
			{
				INTERACTIVE_OBJ * tio = inter.iobj[player.equiped[EQUIP_SLOT_ARMOR]];

				if (tio->halo.flags & HALO_ACTIVE)
					hio_armor = tio;
			}

			if ((player.equiped[EQUIP_SLOT_LEGGINGS] != 0)
			        &&	ValidIONum(player.equiped[EQUIP_SLOT_LEGGINGS]))
			{
				INTERACTIVE_OBJ * tio = inter.iobj[player.equiped[EQUIP_SLOT_LEGGINGS]];

				if (tio->halo.flags & HALO_ACTIVE)
					hio_leggings = tio;
			}

			if (use_io->halo.flags & HALO_ACTIVE)
				hio_player = use_io;
		}

		if (hio_player
		        ||	hio_armor
		        ||	hio_leggings
		        ||	hio_helmet
		        ||	(use_io->halo.flags & HALO_ACTIVE))
		{

			float mdist = ACTIVECAM->cdepth * DIV2;

			ddist = mdist - Distance3D(pos->x + ftr.x, pos->y + ftr.y, pos->z + ftr.z, ACTIVECAM->pos.x, ACTIVECAM->pos.y, ACTIVECAM->pos.z);

			ddist = (ddist / mdist);  //*0.1f;
			ddist *= ddist * ddist * ddist * ddist * ddist;

			if (ddist <= 0.25f) ddist = 0.25f;

			else if (ddist > 0.9f) ddist = 0.9f;

			if ((use_io) && (use_io->halo.flags & HALO_DYNLIGHT))     
				HALO_IO_DynLight_Update(io);

			Cedric_PrepareHalo(pd3dDevice, eobj, obj, use_io, pos, ftr);
			need_halo	= 1;
			MAX_ZEDE	= 0.f;

			for (long i = 0 ; i < eobj->nbvertex ; i++)
			{
				if (eobj->vertexlist3[i].vert.rhw > 0.f)
					MAX_ZEDE = __max(eobj->vertexlist3[i].vert.sz, MAX_ZEDE);
			}
			
		}
	}

	{
		bool bPassInTANDL;
		bool bBumpOnIO;
		float fDist		= EEDistance3D(pos, &ACTIVECAM->pos);
		bPassInTANDL	= false;

		bBumpOnIO		= ( bALLOW_BUMP ) && ( io ) && ( io->ioflags & IO_BUMP ) && ( fDist < __min( __max( 0.f, ( ACTIVECAM->cdepth * fZFogStart ) - 200.f ), 600.f ) ) ? true : false ;

		for (i = 0 ; i < eobj->nbfaces ; i++)
		{
			D3DTLVERTEX		tv_static[3];
			ARX_D3DVERTEX	* tv			= NULL;

 
			EERIE_FACE	*	eface;
			long 			paf[3];

			eface = &eobj->facelist[i];

			if ((eface->facetype & POLY_HIDE) && (!FORCE_NO_HIDE))
				continue;

			if (bGATI8500)
			{
				long OUTSIDE = 0;

				if (eobj->vertexlist3[eface->vid[0]].vworld.z < SOFTNEARCLIPPTANDLZ) OUTSIDE++;

				if (eobj->vertexlist3[eface->vid[1]].vworld.z < SOFTNEARCLIPPTANDLZ) OUTSIDE++;

				if (eobj->vertexlist3[eface->vid[2]].vworld.z < SOFTNEARCLIPPTANDLZ) OUTSIDE++;

				if (OUTSIDE)
				{
					bPassInTANDL = true;
				}
				else
				{
					bPassInTANDL = false;
				}
			}

			//CULL3D
			EERIE_3D nrm;
			nrm.x = eobj->vertexlist3[eface->vid[0]].v.x - ACTIVECAM->pos.x;
			nrm.y = eobj->vertexlist3[eface->vid[0]].v.y - ACTIVECAM->pos.y;
			nrm.z = eobj->vertexlist3[eface->vid[0]].v.z - ACTIVECAM->pos.z;

			if (!(eface->facetype & POLY_DOUBLESIDED))
			{
				EERIE_3D normV10;
				EERIE_3D normV20;
				normV10.x = eobj->vertexlist3[eface->vid[1]].v.x - eobj->vertexlist3[eface->vid[0]].v.x;
				normV10.y = eobj->vertexlist3[eface->vid[1]].v.y - eobj->vertexlist3[eface->vid[0]].v.y;
				normV10.z = eobj->vertexlist3[eface->vid[1]].v.z - eobj->vertexlist3[eface->vid[0]].v.z;
				normV20.x = eobj->vertexlist3[eface->vid[2]].v.x - eobj->vertexlist3[eface->vid[0]].v.x;
				normV20.y = eobj->vertexlist3[eface->vid[2]].v.y - eobj->vertexlist3[eface->vid[0]].v.y;
				normV20.z = eobj->vertexlist3[eface->vid[2]].v.z - eobj->vertexlist3[eface->vid[0]].v.z;

				EERIE_3D normFace;
				normFace.x = (normV10.y * normV20.z) - (normV10.z * normV20.y);
				normFace.y = (normV10.z * normV20.x) - (normV10.x * normV20.z);
				normFace.z = (normV10.x * normV20.y) - (normV10.y * normV20.x);

				if ((DOTPRODUCT(normFace , nrm) > 0.f)) continue;
			}

			TextureContainer * pTex;

			if ((eobj->facelist[i].texid < 0) ||
			        (!(pTex = eobj->texturecontainer[eobj->facelist[i].texid]))) continue;

			float			fTransp = 0;
			unsigned long	* pNb;

			if ((eobj->facelist[i].facetype & POLY_TRANS) ||
			        (invisibility > 0.f))
			{
				if (invisibility > 0.f)
					fTransp = 2.f - invisibility;
				else
					fTransp = eobj->facelist[i].transval;

				if (fTransp >= 2.f)    //MULTIPLICATIVE
				{
					fTransp *= DIV2;
					fTransp += 0.5f;

					tv	= PushVertexInTableCull_TMultiplicative(pTex);
					pNb	= &pTex->ulNbVertexListCull_TMultiplicative;
				}
				else
				{
					if (fTransp >= 1.f)  //ADDITIVE
					{
						fTransp -= 1.f;

						tv		 = PushVertexInTableCull_TAdditive(pTex);
						pNb		 = &pTex->ulNbVertexListCull_TAdditive;
					}
					else
					{
						if (fTransp > 0.f)   //NORMAL TRANS
						{
							fTransp = 1.f - fTransp;

							tv		= PushVertexInTableCull_TNormalTrans(pTex);
							pNb		= &pTex->ulNbVertexListCull_TNormalTrans;
						}
						else  //SUBTRACTIVE
						{
							fTransp	= 1.f - fTransp;

							tv		= PushVertexInTableCull_TSubstractive(pTex);
							pNb		= &pTex->ulNbVertexListCull_TSubstractive;
						}
					}
				}
			}
			else
			{
				tv	= PushVertexInTableCull(pTex);
				pNb	= &pTex->ulNbVertexListCull;
			}

			for (long n = 0 ; n < 3 ; n++)
			{
				paf[n]		= eface->vid[n];
				tv[n].sx	= eobj->vertexlist3[paf[n]].vert.sx;
				tv[n].sy	= eobj->vertexlist3[paf[n]].vert.sy;
				tv[n].sz	= eobj->vertexlist3[paf[n]].vert.sz;

				if (FORCE_FRONT_DRAW)
				{
					if (IsInGroup(eobj, paf[n], 1) != -1)
						tv[n].sz *= IN_FRONT_DIVIDER;
					else
						tv[n].sz *= IN_FRONT_DIVIDER_FEET;
				}

				tv[n].rhw	= eobj->vertexlist3[paf[n]].vert.rhw;
				tv[n].tu	= eface->u[n];
				tv[n].tv	= eface->v[n];
				tv[n].color = eobj->vertexlist3[paf[n]].vert.color;
			}

			if (special_color_flag)
			{
				if (special_color_flag & 1)
				{
					for (long j = 0 ; j < 3 ; j++)
					{
						tv[j].color = (0xFF000000L
						               | (((long)((float)((long)((tv[j].color >> 16) & 255)) * (special_color.r)) & 255) << 16)
						               | (((long)((float)((long)((tv[j].color >> 8) & 255)) * special_color.g) & 255) << 8)
						               | (long)((float)((long)(tv[j].color & 255)) * (special_color.b)) & 255);
					}
				}
				else if (special_color_flag & 2)
				{
					for (long j = 0 ; j < 3 ; j++)
					{
						tv[j].color = 0xFFFF0000;
					}
				}
			}
		

			if ((eobj->facelist[i].facetype & POLY_TRANS)
			        || (invisibility > 0.f))
			{
				tv[0].color = D3DRGB(fTransp, fTransp, fTransp);
				tv[1].color = D3DRGB(fTransp, fTransp, fTransp);
				tv[2].color = D3DRGB(fTransp, fTransp, fTransp);
			}

			if (!bPassInTANDL)
				ComputeFog(tv, 3);
			else
			{
				memcpy(tv_static, tv, sizeof(D3DTLVERTEX) * 3);
			}

			int iNbPointAdd;

			if (!(iNbPointAdd = ARX_SoftClippZ(&eobj->vertexlist3[paf[0]],
			                                   &eobj->vertexlist3[paf[1]],
			                                   &eobj->vertexlist3[paf[2]],
			                                   &tv,
			                                   eface,
			                                   invisibility,
			                                   pTex,
			                                   bBumpOnIO,
			                                   (io) && (io->ioflags & IO_ZMAP),
			                                   eobj,
			                                   i,
			                                   paf,
			                                   NULL,
			                                   true,
			                                   special_color_flag,
			                                   &special_color,
			                                   bPassInTANDL)))
			{
				*pNb -= 3;
				continue;
			}


			if (!bPassInTANDL)
			{
				if (bBumpOnIO)
				{
					if (bPassInTANDL)
					{
						PushInterBumpTANDL(pTex, tv_static, &eobj->vertexlist3[paf[0]].v, &eobj->vertexlist3[paf[1]].v, &eobj->vertexlist3[paf[2]].v);
					}
					else
					{
						PushInterBump(pTex, tv);
					}
				}

				if ((io) && (io->ioflags & IO_ZMAP))
				{
					if (bPassInTANDL)
					{
						CalculateInterZMappTANDL(eobj, i, paf, pTex);
					}
					else
					{
						CalculateInterZMapp(eobj, i, paf, pTex, tv);
					}
				}
			}

			////////////////////////////////////////////////////////////////////////
			// HALO HANDLING START
			if ( need_halo && io )
			{
				long	lfr, lfg, lfb;
				float	ffr, ffg, ffb;
				float	tot	=	0;
				float	_ffr[3];

				IO_HALO curhalo;
				memcpy(&curhalo, &io->halo, sizeof(IO_HALO));
				int curhaloInitialized = 0;

				long max_c;

				if (use_io == inter.iobj[0])
					max_c = 4;
				else
					max_c = 1;

				for (long cnt = 0 ; cnt < max_c ; cnt++)
				{
					switch (cnt)
					{
						case 0:

							if (use_io == inter.iobj[0])
							{
								if (hio_player)
								{
									memcpy(&curhalo, &use_io->halo, sizeof(IO_HALO));
									++curhaloInitialized;
								}
								else continue;
							}
							else
							{
								memcpy(&curhalo, &io->halo, sizeof(IO_HALO));
								++curhaloInitialized;
							}

							break;
						case 1:

							if ((hio_helmet)
							        &&	(IsInSelection(use_io->obj, paf[0], use_io->obj->fastaccess.sel_head) >= 0))
							{
								memcpy(&curhalo, &hio_helmet->halo, sizeof(IO_HALO));
								++curhaloInitialized;
							}
							else continue;

							break;
						case 2:

							if ((hio_armor)
							        &&	(IsInSelection(use_io->obj, paf[0], use_io->obj->fastaccess.sel_chest) >= 0))
							{
								memcpy(&curhalo, &hio_armor->halo, sizeof(IO_HALO));
								++curhaloInitialized;
							}
							else continue;

							break;
						case 3:

							if ((hio_leggings)
							        &&	(IsInSelection(use_io->obj, paf[0], use_io->obj->fastaccess.sel_leggings) >= 0))
							{
								memcpy(&curhalo, &hio_leggings->halo, sizeof(IO_HALO)) ;
								++curhaloInitialized;
							}
							else continue;

							break;
					}

					ARX_CHECK(curhaloInitialized > 0);

					D3DTLVERTEX * workon;
					workon	= tv;

					long o;

					for (o = 0 ; o < 3 ; o++)
					{
						float tttz	= EEfabs(eobj->vertexlist3[paf[o]].norm.z) * DIV2;
						float power	=	255.f - (float)(255.f * tttz);
						power		*=	(1.f - invisibility);

						if (power > 255.f) power = 255.f;
						else if (power < 0.f) power = 0.f;

						ffr			=	curhalo.color.r * power;
						ffg			=	curhalo.color.g * power;
						ffb			=	curhalo.color.b * power;
						tot			+=	power;
						_ffr[o]		=	power;
						F2L(ffr, &lfr);
						F2L(ffg, &lfg);
						F2L(ffb, &lfb);
						tv[o].color = (0xFF000000L | (((lfr) & 255) << 16) |	(((lfg) & 255) << 8) | (lfb) & 255);
					}

					//SETCULL(pd3dDevice,D3DCULL_NONE);
					if (tot > 260)   //260.f)
					{
						long first;
						long second;
						long third;

						if ((_ffr[0] >= _ffr[1]) && (_ffr[1] >= _ffr[2]))
						{
							first = 0;
							second = 1;
							third = 2;
						}
						else if ((_ffr[0] >= _ffr[2]) && (_ffr[2] >= _ffr[1]))
						{
							first = 0;
							second = 2;
							third = 1;
						}
						else if ((_ffr[1] >= _ffr[0]) && (_ffr[0] >= _ffr[2]))
						{
							first = 1;
							second = 0;
							third = 2;
						}
						else if ((_ffr[1] >= _ffr[2]) && (_ffr[2] >= _ffr[0]))
						{
							first = 1;
							second = 2;
							third = 0;
						}
						else if ((_ffr[2] >= _ffr[0]) && (_ffr[0] >= _ffr[1]))
						{
							first = 2;
							second = 0;
							third = 1;
						}
						else
						{
							first = 2;
							second = 1;
							third = 0;
						}


						if ((_ffr[first] > 150.f) && (_ffr[second] > 110.f)) 
						{
							EERIE_3D		vect1, vect2;
							D3DTLVERTEX *	vert = &LATERDRAWHALO[(HALOCUR << 2)];

							HALOCUR++;

							if (HALOCUR >= HALOMAX) HALOCUR--;

							memcpy(&vert[0], &workon[first], sizeof(D3DTLVERTEX));
							memcpy(&vert[1], &workon[first], sizeof(D3DTLVERTEX));
							memcpy(&vert[2], &workon[second], sizeof(D3DTLVERTEX));
							memcpy(&vert[3], &workon[second], sizeof(D3DTLVERTEX));

							float siz = ddist * (curhalo.radius * (EEsin((float)(FrameTime + i) * DIV100) * DIV10 + 1.f)) * 0.6f;

							if ((io == inter.iobj[0]) && (ddist > 0.8f) && !EXTERNALVIEW)
								siz *= 1.5f;

							vect1.x = workon[first].sx - workon[third].sx;
							vect1.y = workon[first].sy - workon[third].sy;
							float len1 = 2.f / EEsqrt(vect1.x * vect1.x + vect1.y * vect1.y);

							if (vect1.x < 0.f) len1 *= 1.2f;

							vect1.x *= len1;
							vect1.y *= len1;
							vect2.x	 = workon[second].sx - workon[third].sx;
							vect2.y	 = workon[second].sy - workon[third].sy;

							float len2 = 1.f / EEsqrt(vect2.x * vect2.x + vect2.y * vect2.y);

							if (vect2.x < 0.f) len2 *= 1.2f;

							vect2.x		*= len2;
							vect2.y		*= len2;
							vert[1].sx	+= (vect1.x + 0.2f - rnd() * 0.1f) * siz;  
							vert[1].sy	+= (vect1.y + 0.2f - rnd() * 0.1f) * siz; 
							vert[1].color = 0xFF000000;
							float valll;

							if (bZBUFFER)
							{
								valll = 0.005f + (EEfabs(workon[first].sz) - EEfabs(workon[third].sz)
								                  + EEfabs(workon[second].sz) - EEfabs(workon[third].sz));   

								valll = 0.0001f + valll * DIV10;

								if (valll < 0.f) valll = 0.f;

								vert[1].sz	+= valll;
								vert[2].sz	+= valll;
								vert[0].sz	+= 0.0001f;
								vert[3].sz	+= 0.0001f;//*DIV2;
								vert[1].rhw	*= .98f;
								vert[2].rhw	*= .98f;
								vert[0].rhw	*= .98f;
								vert[3].rhw	*= .98f;
							}
							else
							{
								vert[1].rhw	*= .98f;
								vert[2].rhw	*= .98f;
								vert[0].rhw	*= .98f;
								vert[3].rhw	*= .98f;
							}

							vert[2].sx += (vect2.x + 0.2f - rnd() * 0.1f) * siz;  
							vert[2].sy += (vect2.y + 0.2f - rnd() * 0.1f) * siz;  

							vert[1].sz = (vert[1].sz + MAX_ZEDE) * DIV2;
							vert[2].sz = (vert[2].sz + MAX_ZEDE) * DIV2;

							if (curhalo.flags & HALO_NEGATIVE)
								vert[2].color = 0x00000000;
							else
								vert[2].color = 0xFF000000;
						}
					}
				}

				for (long o = 0 ; o < 3 ; o++)
				{
					paf[o]		= eface->vid[o];
					tv[o].color = eobj->vertexlist3[paf[o]].vert.color;
				}
			}

			////////////////////////////////////////////////////////////////////////
			// HALO HANDLING END
			////////////////////////////////////////////////////////////////////////

			if ((eobj->facelist[i].facetype & POLY_TRANS)
			        || (invisibility > 0.f))
			{
				if (bPassInTANDL)
				{
					D3DTLVERTEX * ptvTL;

					if (invisibility > 0.f)
						fTransp = 2.f - invisibility;
					else
						fTransp = eobj->facelist[i].transval;

					if (fTransp >= 2.f)    //MULTIPLICATIVE
					{
						fTransp *= DIV2;
						fTransp += 0.5f;

						ptvTL	 = PushVertexInTableCull_TMultiplicativeH(pTex);
					}
					else
					{
						if (fTransp >= 1.f)  //ADDITIVE
						{
							fTransp -= 1.f;

							ptvTL = PushVertexInTableCull_TAdditiveH(pTex);
						}
						else
						{
							if (fTransp > 0.f)   //NORMAL TRANS
							{
								fTransp = 1.f - fTransp;

								ptvTL = PushVertexInTableCull_TNormalTrans(pTex);
							}
							else  //SUBTRACTIVE
							{
								fTransp = 1.f - fTransp;

								ptvTL = PushVertexInTableCull_TSubstractive(pTex);
							}
						}
					}

					ptvTL[0].sx		= eobj->vertexlist3[paf[0]].v.x;
					ptvTL[0].sy		= eobj->vertexlist3[paf[0]].v.y;
					ptvTL[0].sz		= eobj->vertexlist3[paf[0]].v.z;
					ptvTL[0].color	= tv_static[0].color;
					ptvTL[0].tu		= tv_static[0].tu;
					ptvTL[0].tv		= tv_static[0].tv;
					ptvTL[1].sx		= eobj->vertexlist3[paf[1]].v.x;
					ptvTL[1].sy		= eobj->vertexlist3[paf[1]].v.y;
					ptvTL[1].sz		= eobj->vertexlist3[paf[1]].v.z;
					ptvTL[1].color	= tv_static[1].color;
					ptvTL[1].tu		= tv_static[1].tu;
					ptvTL[1].tv		= tv_static[1].tv;
					ptvTL[2].sx		= eobj->vertexlist3[paf[2]].v.x;
					ptvTL[2].sy		= eobj->vertexlist3[paf[2]].v.y;
					ptvTL[2].sz		= eobj->vertexlist3[paf[2]].v.z;
					ptvTL[2].color	= tv_static[2].color;
					ptvTL[2].tu		= tv_static[2].tu;
					ptvTL[2].tv		= tv_static[2].tv;

					*pNb -= iNbPointAdd;

				}

				continue;
			}

			if (special_color_flag & 2)
			{
				D3DTLVERTEX * tv2;
				{
					tv2 = PushVertexInTableCull(&TexSpecialColor);
				}
				memcpy((void *) tv2, (void *)tv, sizeof(D3DTLVERTEX) * 3);

				unsigned long v = _EERIERGB(special_color.r);
				tv2[0].color	= v;
				tv2[1].color	= v;
				tv2[2].color	= v;
			}

			// Add a little bit of Fake Metal Specular if needed :p
			if ((eface->facetype & POLY_METAL)
			        || ((pTex) && (pTex->userflags & POLY_METAL)))
			{
				D3DTLVERTEX	* tv2;
				unsigned long	* pulNbVertexList_TMetal;

				if (bPassInTANDL)
				{
					tv2						= PushVertexInTableCull_TMetalH(pTex);
					pulNbVertexList_TMetal	= &pTex->ulNbVertexListCull_TMetalH;

					tv2[0].sx				= eobj->vertexlist3[paf[0]].v.x;
					tv2[0].sy				= eobj->vertexlist3[paf[0]].v.y;
					tv2[0].sz				= eobj->vertexlist3[paf[0]].v.z;
					tv2[0].color			= tv_static[0].color;
					tv2[0].tu				= tv_static[0].tu;
					tv2[0].tv				= tv_static[0].tv;
					tv2[1].sx				= eobj->vertexlist3[paf[1]].v.x;
					tv2[1].sy				= eobj->vertexlist3[paf[1]].v.y;
					tv2[1].sz				= eobj->vertexlist3[paf[1]].v.z;
					tv2[1].color			= tv_static[1].color;
					tv2[1].tu				= tv_static[1].tu;
					tv2[1].tv				= tv_static[1].tv;
					tv2[2].sx				= eobj->vertexlist3[paf[2]].v.x;
					tv2[2].sy				= eobj->vertexlist3[paf[2]].v.y;
					tv2[2].sz				= eobj->vertexlist3[paf[2]].v.z;
					tv2[2].color			= tv_static[2].color;
					tv2[2].tu				= tv_static[2].tu;
					tv2[2].tv				= tv_static[2].tv;
				}
				else
				{
					tv2						=	PushVertexInTableCull_TMetal(pTex);
					pulNbVertexList_TMetal	=	&pTex->ulNbVertexListCull_TMetal;
					memcpy((void *)tv2, (void *)tv, sizeof(D3DTLVERTEX) * 3);
				}


				long r, g, b;
				long todo = 0;

				for (long j = 0 ; j < 3 ; j++)
				{
					r = (tv2[j].color >> 16) & 255;
					g = (tv2[j].color >> 8) & 255;
					b = tv2[j].color & 255;

					if (r > 192 || g > 192 || b > 192)
					{
						todo++;
					}

					r -= 192;

					if (r < 0.f) r = 0;

					g -= 192;

					if (g < 0.f) g = 0;

					b -= 192;

					if (b < 0.f) b = 0;

					tv2[j].color = 0xFF000000 | (r << 18) | (g << 10) | (b << 2);
				}

				if (!todo)
				{
					*pulNbVertexList_TMetal -= 3;
				}
			}

			if (bPassInTANDL)
			{
				D3DTLVERTEX * ptvTL;

				ptvTL			= PushVertexInTableCullH(pTex);

				ptvTL[0].sx		= eobj->vertexlist3[paf[0]].v.x;
				ptvTL[0].sy		= eobj->vertexlist3[paf[0]].v.y;
				ptvTL[0].sz		= eobj->vertexlist3[paf[0]].v.z;
				ptvTL[0].color	= tv_static[0].color;
				ptvTL[0].tu		= tv_static[0].tu;
				ptvTL[0].tv		= tv_static[0].tv;
				ptvTL[1].sx		= eobj->vertexlist3[paf[1]].v.x;
				ptvTL[1].sy		= eobj->vertexlist3[paf[1]].v.y;
				ptvTL[1].sz		= eobj->vertexlist3[paf[1]].v.z;
				ptvTL[1].color	= tv_static[1].color;
				ptvTL[1].tu		= tv_static[1].tu;
				ptvTL[1].tv		= tv_static[1].tv;
				ptvTL[2].sx		= eobj->vertexlist3[paf[2]].v.x;
				ptvTL[2].sy		= eobj->vertexlist3[paf[2]].v.y;
				ptvTL[2].sz		= eobj->vertexlist3[paf[2]].v.z;
				ptvTL[2].color	= tv_static[2].color;
				ptvTL[2].tu		= tv_static[2].tu;
				ptvTL[2].tv		= tv_static[2].tv;

				*pNb -= iNbPointAdd;
			}

		}
	}
}


/* Render object */
void	Cedric_RenderObject(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_3DOBJ * eobj, EERIE_C_DATA * obj, INTERACTIVE_OBJ * io, EERIE_3D * pos, EERIE_3D & ftr, float invisibility)
{
	if (bRenderInterList)
	{
		Cedric_RenderObject2(pd3dDevice,
		                     eobj,
		                     obj,
		                     io,
		                     pos,
		                     ftr,
		                     invisibility);
		return;
	}

	int			i;

	// Finally we can draw polys !!!
	SETCULL(pd3dDevice, D3DCULL_NONE);

	// Sets IO BBox to calculated BBox :)
	if (io)
	{
		io->bbox1.x = (short)BBOXMIN.x;
		io->bbox2.x = (short)BBOXMAX.x;
		io->bbox1.y = (short)BBOXMIN.y;
		io->bbox2.y = (short)BBOXMAX.y;
	}

	if (invisibility == 1.f)
		return;

	float ddist = 0;
	long need_halo;
	need_halo = 0;

	if ((io) && (io->halo.flags & HALO_ACTIVE))
	{
		ddist = 500.f - Distance3D(pos->x + ftr.x, pos->y + ftr.y, pos->z + ftr.z, ACTIVECAM->pos.x, ACTIVECAM->pos.y, ACTIVECAM->pos.z);

		if (ddist > 0.f)
		{
			ddist = (ddist * DIV500);
		}
		else if (ddist < 0.f)
		{
			ddist = 0.f;
		}

		else if (ddist > 1.f)
		{
			ddist = 1.f;
		}



		if ((io) && (io->halo.flags & HALO_DYNLIGHT))//(io->halo.flags & HALO_ACTIVE)
			HALO_IO_DynLight_Update(io);

		if ((io) && (io->halo.flags & HALO_ACTIVE) && (ddist > 0.01f))
		{
			Cedric_PrepareHalo(pd3dDevice, eobj, obj, io, pos, ftr);
			need_halo = 1;
		}
	}


	{
		for (i = 0; i < eobj->nbfaces; i++)
		{
			D3DTLVERTEX tv[3];
 
			EERIE_FACE 	 *  eface;
			long 			paf[3];

			eface = &eobj->facelist[i];

			if ((eface->facetype & POLY_HIDE) && (!FORCE_NO_HIDE))
				continue;

			if ((eobj->vertexlist3[eface->vid[0]].vert.rhw < 0)
			        &&	(eobj->vertexlist3[eface->vid[1]].vert.rhw < 0)
			        &&	(eobj->vertexlist3[eface->vid[2]].vert.rhw < 0))
				continue; // To avoid reverse-flip draw.

			long OUTSIDE = 0;

			for (long n = 0; n < 3; n++)
			{
				paf[n] = eface->vid[n];
				tv[n].sx = eobj->vertexlist3[paf[n]].vert.sx;
				tv[n].sy = eobj->vertexlist3[paf[n]].vert.sy;
				tv[n].sz = eobj->vertexlist3[paf[n]].vert.sz;

				if (tv[n].sz <= 0.f)
					OUTSIDE++;

				tv[n].rhw = eobj->vertexlist3[paf[n]].vert.rhw;
				tv[n].tu = eface->u[n];
				tv[n].tv = eface->v[n];
				tv[n].color = eobj->vertexlist3[paf[n]].vert.color;
			}

			if (OUTSIDE == 3) continue;

			////////////////////////////////////////////////////////////////////////
			// HALO HANDLING START
			if (need_halo)
			{
				long lfr, lfg, lfb;
				float ffr, ffg, ffb;
				float tot = 0;
				float _ffr[3];

				D3DTLVERTEX * workon = tv;

				for (long o = 0; o < 3; o++)
				{
					float tttz = eobj->vertexlist3[paf[o]].norm.z;

					if (tttz < 0.f) tttz *= -0.5f;
					else tttz *= 0.5f;

					float power = 255.f - (float)(255.f * tttz);
					power *= (1.f - invisibility);

					if (power > 255.f) power = 255.f;
					else if (power < 0.f) power = 0.f;

					ffr = io->halo.color.r * power;
					ffg = io->halo.color.g * power;
					ffb = io->halo.color.b * power;
					tot += power;
					_ffr[o] = power;
					F2L(ffr, &lfr);
					F2L(ffg, &lfg);
					F2L(ffb, &lfb);
					tv[o].color = (0xFF000000L | (((lfr) & 255) << 16) |	(((lfg) & 255) << 8) | (lfb) & 255);
				}

				if (tot > 260.f)
				{
					long first;
					long second;
					long third;

					if ((_ffr[0] >= _ffr[1]) && (_ffr[1] >= _ffr[2]))
					{
						first = 0;
						second = 1;
						third = 2;
					}
					else if ((_ffr[0] >= _ffr[2]) && (_ffr[2] >= _ffr[1]))
					{
						first = 0;
						second = 2;
						third = 1;
					}
					else if ((_ffr[1] >= _ffr[0]) && (_ffr[0] >= _ffr[2]))
					{
						first = 1;
						second = 0;
						third = 2;
					}
					else if ((_ffr[1] >= _ffr[2]) && (_ffr[2] >= _ffr[0]))
					{
						first = 1;
						second = 2;
						third = 0;
					}
					else if ((_ffr[2] >= _ffr[0]) && (_ffr[0] >= _ffr[1]))
					{
						first = 2;
						second = 0;
						third = 1;
					}
					else
					{
						first = 2;
						second = 1;
						third = 0;
					}

					if ((_ffr[first] > 150.f) && (_ffr[second] > 110.f)) // && (workon[0].sz>0.f) && (workon[3].sz>0.f))
					{
						EERIE_3D		vect1, vect2;
						D3DTLVERTEX	* vert = &LATERDRAWHALO[(HALOCUR << 2)];

						HALOCUR++;

						if (HALOCUR >= HALOMAX) HALOCUR--;

						memcpy(&vert[0], &workon[first], sizeof(D3DTLVERTEX));
						memcpy(&vert[1], &workon[first], sizeof(D3DTLVERTEX));
						memcpy(&vert[2], &workon[second], sizeof(D3DTLVERTEX));
						memcpy(&vert[3], &workon[second], sizeof(D3DTLVERTEX));

						float siz = ddist * (io->halo.radius * (EEsin((float)(FrameTime + i) * DIV100) * DIV50 + 1.f));

						vect1.x = workon[first].sx - workon[third].sx;
						vect1.y = workon[first].sy - workon[third].sy;
						float len1 = 1.f / EEsqrt(vect1.x * vect1.x + vect1.y * vect1.y);
						vect1.x *= len1;
						vect1.y *= len1;
						vect2.x = workon[second].sx - workon[third].sx;
						vect2.y = workon[second].sy - workon[third].sy;
						float len2 = 1.f / EEsqrt(vect2.x * vect2.x + vect2.y * vect2.y);
						vect2.x *= len2;
						vect2.y *= len2;
						vert[1].sx += (vect1.x + 0.2f - rnd() * 0.1f) * siz; //+len1;
						vert[1].sy += (vect1.y + 0.2f - rnd() * 0.1f) * siz; //+len2;
						vert[1].color = 0xFF000000;
						float valll; 

						if (bZBUFFER)
						{
							valll = (EEfabs(workon[first].sz - workon[third].sz)
							         + EEfabs(workon[second].sz - workon[third].sz)) * 1.8f;

							valll = 0.01f + valll;

							if (valll < 0.f) valll = 0.f;

							vert[1].sz += valll;
							vert[2].sz += valll;
							vert[0].sz += 0.0001f;
							vert[3].sz += 0.0001f; //*DIV2;
						}
						else
						{
							vert[1].rhw *= .98f;
							vert[2].rhw *= .98f;
							vert[0].rhw *= .98f;
							vert[3].rhw *= .98f; //*DIV2;
						}

						vert[2].sx += (vect2.x + 0.2f - rnd() * 0.1f) * siz; //+len2;
						vert[2].sy += (vect2.y + 0.2f - rnd() * 0.1f) * siz; //+len1;

						if (io->halo.flags & HALO_NEGATIVE) vert[2].color = 0x00000000;
						else vert[2].color = 0xFF000000;
					}
				}

				for (long n = 0; n < 3; n++)
				{
					paf[n] = eface->vid[n];
					tv[n].color = eobj->vertexlist3[paf[n]].vert.color;
				}
			}

			if (special_color_flag)
			{
				if (special_color_flag & 1)
				{
					for (long j = 0; j < 3; j++)
					{
						tv[j].color = (0xFF000000L
						               | (((long)((float)((long)((tv[j].color >> 16) & 255)) * (special_color.r)) & 255) << 16)
						               | (((long)((float)((long)((tv[j].color >> 8) & 255)) * special_color.g) & 255) << 8)
						               | (long)((float)((long)(tv[j].color & 255)) * (special_color.b)) & 255);
					}
				}
				else if (special_color_flag & 2)
				{
					for (long j = 0; j < 3; j++)
					{
						tv[j].color = 0xFFFF0000;
					}
				}
			}

			// HALO HANDLING END
			////////////////////////////////////////////////////////////////////////
	const int EERIE_FLAG = bSoftRender?EERIE_USEVB:0; //Should we use Vertex Buffer

			// Backface culling if required
			if (eface->facetype & POLY_DOUBLESIDED)
				SETCULL(pd3dDevice, D3DCULL_NONE);
			else
				SETCULL(pd3dDevice, D3DCULL_CW);

			// Is Transparent?
			if (eface->facetype & POLY_TRANS)
			{
				memcpy(&InterTransPol[INTERTRANSPOLYSPOS], &tv, sizeof(D3DTLVERTEX) * 3);
				InterTransFace[INTERTRANSPOLYSPOS] = eface;
				InterTransTC[INTERTRANSPOLYSPOS] = eobj->texturecontainer[eface->texid];
				INTERTRANSPOLYSPOS++;

				if (INTERTRANSPOLYSPOS >= MAX_INTERTRANSPOL)
					INTERTRANSPOLYSPOS = MAX_INTERTRANSPOL - 1;

				continue;
			}

			// Set texture
			if ((eface->texid == -1) || (eobj->texturecontainer[eface->texid] == NULL))
				SETTC(pd3dDevice, NULL);
			else
				SETTC(pd3dDevice, eobj->texturecontainer[eface->texid]);

			if (invisibility > 0.f)
			{
				memcpy(&InterTransPol[INTERTRANSPOLYSPOS], &tv, sizeof(D3DTLVERTEX) * 3);
				InterTransFace[INTERTRANSPOLYSPOS] = eface;
				InterTransFace[INTERTRANSPOLYSPOS]->transval = 2.f - invisibility;
				InterTransTC[INTERTRANSPOLYSPOS] = eobj->texturecontainer[eface->texid];
				INTERTRANSPOLYSPOS++;

				if (INTERTRANSPOLYSPOS >= MAX_INTERTRANSPOL)
					INTERTRANSPOLYSPOS = MAX_INTERTRANSPOL - 1;

				continue;
			}

			EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX | D3DFVF_DIFFUSE , &tv, 3,  0, EERIE_FLAG );

			if (special_color_flag & 2)
			{
				pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR);
				pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
				SETALPHABLEND(pd3dDevice, TRUE);
				SETZWRITE(pd3dDevice, FALSE);
				SETTC(pd3dDevice, NULL);
				unsigned long v = _EERIERGB(special_color.r);

				for (long j = 0; j < 3; j++)
				{
					tv[j].color = v;
				}
				EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , &tv, 3,  0, EERIE_FLAG  );
				EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE , &tv, 3,  0, EERIE_FLAG  );//duplicate ???? @TBR ?
				SETALPHABLEND(pd3dDevice, FALSE);
				SETZWRITE(pd3dDevice, TRUE);
			}

		// Add a little bit of Fake Metal Specular if needed
			if ((eface->facetype & POLY_METAL)
			        || ((eobj->texturecontainer[eface->texid]) && (eobj->texturecontainer[eface->texid]->userflags & POLY_METAL)))
			{
				long r, g, b;
				long todo = 0;

				for (long j = 0; j < 3; j++)
				{
					r = (tv[j].color >> 16) & 255;
					g = (tv[j].color >> 8) & 255;
					b = tv[j].color & 255;

					if (r > 192 || g > 192 || b > 192)
					{
						todo++;
					}

					r -= 192;

					if (r < 0.f) r = 0;

					g -= 192;

					if (g < 0.f) g = 0;

					b -= 192;

					if (b < 0.f) b = 0;

					tv[j].color = 0xFF000000 | (r << 18) | (g << 10) | (b << 2);
				}

				if (todo)
				{
					pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_DESTCOLOR);
					pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
					SETALPHABLEND(pd3dDevice, TRUE);
					SETZWRITE(pd3dDevice, FALSE);
					EERIEDRAWPRIM(pd3dDevice,D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX| D3DFVF_DIFFUSE, &tv, 3, 0, EERIE_FLAG );
					SETALPHABLEND(pd3dDevice, FALSE);
					SETZWRITE(pd3dDevice, TRUE);
				}
			}
		}
	}
}




void Cedric_BlendAnimation(INTERACTIVE_OBJ * io, EERIE_3DOBJ * eobj, float timm, EERIE_3D * angle)
{
	for (long i = 0; i < eobj->c_data->nb_bones; i++)
	{
		EERIE_QUAT tquat;
		Quat_Copy(&tquat, &eobj->c_data->bones[i].quatinit);
		EERIE_QUAT q2;
		Quat_Copy(&q2, &eobj->c_data->bones[i].quatlast);

		Quat_Slerp(&eobj->c_data->bones[i].quatinit , &q2, &tquat, timm);

		eobj->c_data->bones[i].transinit.x = eobj->c_data->bones[i].translast.x + (eobj->c_data->bones[i].transinit.x - eobj->c_data->bones[i].translast.x) * timm;
		eobj->c_data->bones[i].transinit.y = eobj->c_data->bones[i].translast.y + (eobj->c_data->bones[i].transinit.y - eobj->c_data->bones[i].translast.y) * timm;
		eobj->c_data->bones[i].transinit.z = eobj->c_data->bones[i].translast.z + (eobj->c_data->bones[i].transinit.z - eobj->c_data->bones[i].translast.z) * timm;
	}
}

void Cedric_SaveBlendData(INTERACTIVE_OBJ * io, EERIE_3DOBJ * eobj, EERIE_3D * angle)
{
	if (io->obj->c_data)
	{
		for (long i = 0; i < io->obj->c_data->nb_bones; i++)
		{
			Quat_Copy(&io->obj->c_data->bones[i].quatlast, &io->obj->c_data->bones[i].quatinit);
			Vector_Copy(&io->obj->c_data->bones[i].scalelast, &io->obj->c_data->bones[i].scaleinit);
			Vector_Copy(&io->obj->c_data->bones[i].translast, &io->obj->c_data->bones[i].transinit);
		}
	}
}
 
void Cedric_ManageExtraRotationsFirst(INTERACTIVE_OBJ * io, EERIE_3DOBJ * obj)
{
	for (long i = 0; i != obj->c_data->nb_bones; i++)
	{
		Quat_Init(&obj->c_data->bones[i].quatinit);
		Vector_Copy(&obj->c_data->bones[i].transinit, &obj->c_data->bones[i].transinit_global);
	}

	if ((io) && (io->ioflags & IO_NPC) && (io->_npcdata->ex_rotate))
	{
		for (long k = 0; k < MAX_EXTRA_ROTATE; k++)
		{
			long i = io->_npcdata->ex_rotate->group_number[k];

			if (i >= 0)
			{
				EERIE_3D vt1;
				EERIE_QUAT quat1;
				vt1.x = DEG2RAD(io->_npcdata->ex_rotate->group_rotate[k].g);
				vt1.y = DEG2RAD(io->_npcdata->ex_rotate->group_rotate[k].b);
				vt1.z = DEG2RAD(io->_npcdata->ex_rotate->group_rotate[k].a);
				QuatFromAngles(&quat1, &vt1);
				Quat_Copy(&obj->c_data->bones[i].quatinit, &quat1);
			}
		}
	}
}

extern long LOOK_AT_TARGET;

extern long ForceIODraw;
bool Cedric_IO_Visible(INTERACTIVE_OBJ * io, long typ)
{
	if (io == inter.iobj[0]) return true;

	if ((!ForceIODraw)
	    &&	(ACTIVEBKG != NULL)
	    &&	(io)
	    &&	(!(typ & 1)))
	{
		if (EEDistance3D(&io->pos, &ACTIVECAM->pos) > ACTIVECAM->cdepth * 0.6f)
			return false;

		long xx, yy;
		F2L(((io->pos.x)*ACTIVEBKG->Xmul), &xx);
		F2L(((io->pos.z)*ACTIVEBKG->Zmul), &yy);

		if ((xx >= 1) && (yy >= 1) && (xx < ACTIVEBKG->Xsize - 1) && (yy < ACTIVEBKG->Zsize - 1))
		{
			for (long ky = yy - 1; ky <= yy + 1; ky++)
				for (long kx = xx - 1; kx <= xx + 1; kx++)
				{
					FAST_BKG_DATA * feg = (FAST_BKG_DATA *)&ACTIVEBKG->fastdata[kx][ky];

					if (feg->treat)
						return true;
				}

			return false;
		}
	}

	return true;
}

extern long __MUST_DRAW;
extern long EXTERNALVIEW;

/* Apply animation and draw object */
void	Cedric_AnimateDrawEntity(LPDIRECT3DDEVICE7 pd3dDevice,
                                 EERIE_3DOBJ * eobj,
                                 ANIM_USE * animuse,
                                 EERIE_3D * angle,
                                 EERIE_3D * pos,
                                 INTERACTIVE_OBJ * io,
                                 D3DCOLOR col,
                                 long typ)
{
	float 				invisibility;
	float 				scale;
	float				timm;
	EERIE_3D			ftr, ftr2;
	EERIE_C_DATA	*	obj;

	
	// Init some data
	Cedric_ResetBoundingBox(io);

	// Set scale and invisibility factors
	Cedric_GetScale(scale, invisibility, io);

	// Flag linked objects
	//Cedric_FlagLinkedObjects(eobj); ???

	// Is There any Between-Animations Interpolation to make ? timm>0.f
	Cedric_GetTime(timm, io, typ);


	// Buffer size check
	if (eobj->nbgroups > max_grps)
	{
		//todo free
		grps = (unsigned char *)realloc(grps, eobj->nbgroups);
		max_grps = eobj->nbgroups;
	}

	memset(grps, 0, eobj->nbgroups);


	Cedric_AnimCalcTranslation(io, animuse, scale, typ, ftr, ftr2);

	if (Cedric_IO_Visible(io, typ))
	{

		// Manage Extra Rotations in Local Space
		Cedric_ManageExtraRotationsFirst(io, eobj);
		Looking_At = -1;

		// Perform animation in Local space
		Cedric_AnimateObject(io, eobj, animuse);

		// Check for Animation Blending in Local space
		if (io)
		{
			if (timm > 0.f)
			{
				Cedric_BlendAnimation(io, eobj, timm, angle);
				Cedric_SaveBlendData(io, eobj, angle);
			}
			else
				Cedric_SaveBlendData(io, eobj, angle);
		}

		// Build skeleton in Object Space
		Cedric_ConcatenateTM(io, eobj->c_data, angle, pos, ftr, scale);

		/* Display the object */
		obj = eobj->c_data;

		if (!obj)
			return;


		if ((Cedric_TransformVerts(io, eobj, obj, pos, angle)) && (!(typ & ANIMQUATTYPE_NO_RENDER)))
		{
			INTER_DRAW++;

			if (!Cedric_ApplyLighting(eobj, obj, io, pos, typ)) return;

			Cedric_RenderObject(pd3dDevice, eobj, obj, io, pos, ftr, invisibility);

			if (io)
			{
				io->bbox1.x = (short)BBOXMIN.x;
				io->bbox2.x = (short)BBOXMAX.x;
				io->bbox1.y = (short)BBOXMIN.y;
				io->bbox2.y = (short)BBOXMAX.y;
			}

			// Now we can render Linked Objects
			for (long k = 0; k < eobj->nblinked; k++)
			{
				if (k == 1)
					k = k;

				if ((eobj->linked[k].lgroup != -1) && eobj->linked[k].obj)
				{
					eobj->linked[k].modinfo.rot.a = 0;
					eobj->linked[k].modinfo.rot.b = 0;
					eobj->linked[k].modinfo.rot.g = 0;

					float old = 0.f;
					INTERACTIVE_OBJ * ioo = (INTERACTIVE_OBJ *)eobj->linked[k].io;
					EERIE_3DOBJ * obj = (EERIE_3DOBJ *) eobj->linked[k].obj;

					// Store item invisibility flag
					if (io && ioo)
					{
						old = ioo->invisibility;

						if (io == inter.iobj[0])
						{
							ioo->invisibility = INVISIBILITY_OVERRIDE;
						}
						else
						{
							INVISIBILITY_OVERRIDE = 0.f;
							ioo->invisibility = invisibility;
						}
					}
					else
					{
						if (ioo)
						{
							INVISIBILITY_OVERRIDE = 0.f;
							ioo->invisibility = invisibility;
						}
						else
							INVISIBILITY_OVERRIDE = invisibility;
					}

					if (ioo)
					{
						if ((ioo->ignition > 0.f) || (ioo->ioflags & IO_FIERY))
							ManageIgnition(ioo);
					}

					__MUST_DRAW = 1;

					// specific check to avoid drawing player weapon on its back when in subjective view
					if ((io == inter.iobj[0]) &&
					        (eobj->linked[k].lidx == inter.iobj[0]->obj->fastaccess.weapon_attach)
					        && (!EXTERNALVIEW))
						continue;

					long ll = eobj->linked[k].lidx2;
					eobj->linked[k].modinfo.link_position.x = obj->vertexlist[ll].v.x - obj->vertexlist[obj->origin].v.x;
					eobj->linked[k].modinfo.link_position.y = obj->vertexlist[ll].v.y - obj->vertexlist[obj->origin].v.y;
					eobj->linked[k].modinfo.link_position.z = obj->vertexlist[ll].v.z - obj->vertexlist[obj->origin].v.z;

					EERIE_QUAT quat;
					ll = eobj->linked[k].lidx;
					EERIE_3D * posi = &eobj->vertexlist3[ll].v;
					Quat_Copy(&quat, &eobj->c_data->bones[eobj->linked[k].lgroup].quatanim);
					
					EERIEMATRIX	 matrix;
					MatrixFromQuat(&matrix, &quat);
					DrawEERIEInterMatrix(pd3dDevice, obj, &matrix, posi, ioo, NULL, &eobj->linked[k].modinfo);
					INVISIBILITY_OVERRIDE = 0.f;

					// Restore item invisibility flag
					if (ioo)
						ioo->invisibility = old;

					__MUST_DRAW = 0;

				}
			}

		}
	}
}

void MakeCLight(INTERACTIVE_OBJ * io, EERIE_RGB * infra, EERIE_3D * angle, EERIE_3D * pos, EERIE_3DOBJ * eobj, EERIEMATRIX * BIGMAT, EERIE_QUAT * BIGQUAT)
{
	if ((Project.improve) && (!io))
	{
		infra->r = 0.6f;
		infra->g = 0.f;
		infra->b = 1.f;
	}

	llightsInit();
	EERIE_3D tv;

	if ((io) && (io->ioflags & IO_ITEM))
		Vector_Init(&tv, pos->x, pos->y - 60.f, pos->z);
	else
		Vector_Init(&tv, pos->x, pos->y - 90.f, pos->z);

	for (long i = 0; i < TOTIOPDL; i++)
	{
		if (!(GetMaxManhattanDistance(&IO_PDL[i]->pos, &tv) <= IO_PDL[i]->fallend + 500.f))
			continue;

		Insertllight(IO_PDL[i], EEDistance3D(&IO_PDL[i]->pos, &tv)); 
	}

	for (int i = 0; i < TOTPDL; i++)
	{
		if (!(GetMaxManhattanDistance(&PDL[i]->pos, &tv) <= PDL[i]->fallend + 500.f))
			continue;

		Insertllight(PDL[i], EEDistance3D(&PDL[i]->pos, &tv)); 
	}

	Preparellights(&tv);

	if ((io) && (io->ioflags & IO_ANGULAR)) return;

	EERIE_3D		vLight;
	EERIE_3D		vTLights[32];
	EERIE_QUAT		qInvert;

	if (BIGMAT != NULL)
	{
		QuatFromMatrix(qInvert, *BIGMAT);
	}
	else
	{
		if (BIGQUAT != NULL)
		{
			qInvert = *BIGQUAT;
		}
		else
		{
			//FIX LIGHT 
			EERIE_3D	vt1;

			if (angle)
			{
				Vector_Copy(&vt1, angle);
			}
			else
			{
				if (io) 
					Vector_Copy(&vt1, &io->angle);
				else
					Vector_Copy(&vt1, &eobj->angle);
			}

			vt1.x = MAKEANGLE(-vt1.z) * EEdef_DEGTORAD;
			vt1.y = MAKEANGLE(vt1.y) * EEdef_DEGTORAD;
			vt1.z = MAKEANGLE(vt1.x) * EEdef_DEGTORAD;
			QuatFromAngles(&qInvert, &vt1);
		}
	}


		
		for (int i = 0; i < eobj->nbvertex; i++)
		{
			float r, g, b;
			long ir, ig, ib;

			if ((io) && (io->ioflags & (IO_NPC | IO_ITEM)))
			{
				r = g = b = NPC_ITEMS__AMBIENT_VALUE_255;
			}
			else
			{
				r = ACTIVEBKG->ambient255.r;
				g = ACTIVEBKG->ambient255.g;
				b = ACTIVEBKG->ambient255.b;
			}

			EERIE_3D * posVert = &eobj->vertexlist3[i].v;

			for (long l = 0 ; l != MAX_LLIGHTS; l++)
			{
				EERIE_LIGHT * Cur_llights = llights[l];

				if (Cur_llights)
				{
					float	cosangle;
					
					vLight.x = (llights[l]->pos.x - posVert->x);  
					vLight.y = (llights[l]->pos.y - posVert->y);  
					vLight.z = (llights[l]->pos.z - posVert->z);  
					TRUEVector_Normalize(&vLight);

					TransformInverseVertexQuat(&qInvert, &vLight, &vTLights[l]);
					EERIE_3D * Cur_vLights = &vTLights[l];

					// Get cos angle between light and vertex norm
					cosangle = (eobj->vertexlist[i].norm.x * Cur_vLights->x +
					            eobj->vertexlist[i].norm.y * Cur_vLights->y +
					            eobj->vertexlist[i].norm.z * Cur_vLights->z);

					// If light visible
					if (cosangle > 0.f)
					{
						float distance = EEDistance3D((EERIE_3D *)posVert, &Cur_llights->pos);

						// Evaluate its intensity depending on the distance Light<->Object
						if (distance <= Cur_llights->fallstart)
							cosangle *= Cur_llights->precalc; 
						else
						{
							float p = ((Cur_llights->fallend - distance) * Cur_llights->falldiffmul);

							if (p <= 0.f)
								cosangle = 0.f;
							else
								cosangle *= p * Cur_llights->precalc;
						}

						r += Cur_llights->rgb255.r * cosangle;
						g += Cur_llights->rgb255.g * cosangle;
						b += Cur_llights->rgb255.b * cosangle;
					}
				}
				else
					break;
			}

			if (eobj->drawflags & DRAWFLAG_HIGHLIGHT)
			{
				r += iHighLight; 
				g += iHighLight; 
				b += iHighLight; 
			}

			if ((Project.improve) &&  (!io)) 
			{
					r *= infra->r;
					g *= infra->g;
					b *= infra->b;
					r += infra->r * 512.f;
					g += infra->g;
					b += infra->b * 400.f;
			}

			F2L(r, &ir);
			ir = clipByte255(ir);
			F2L(g, &ig);
			ig = clipByte255(ig);
			F2L(b, &ib);
			ib = clipByte255(ib);
			eobj->vertexlist3[i].vert.color = (0xff000000L | (((ir) & 255) << 16) |	(((ig) & 255) << 8) | (ib) & 255);
		}
}

void MakeCLight2(INTERACTIVE_OBJ * io, EERIE_RGB * infra, EERIE_3D * angle, EERIE_3D * pos, EERIE_3DOBJ * eobj, EERIEMATRIX * BIGMAT, EERIE_QUAT * BIGQUAT, long ii)
{
	EERIE_3D		vLight;
	EERIE_3D		vTLights[32];
	EERIE_QUAT		qInvert;

	if (BIGMAT != NULL)
	{
		QuatFromMatrix(qInvert, *BIGMAT);
	}
	else
	{
		if (BIGQUAT != NULL)
		{
			qInvert = *BIGQUAT;
		}
		else
		{
			EERIE_3D	vt1;

			if (angle)
			{
				Vector_Copy(&vt1, angle);
			}
			else
			{
				if (io) 
					Vector_Copy(&vt1, &io->angle);
				else
					Vector_Copy(&vt1, &eobj->angle);
			}

			vt1.x *= EEdef_DEGTORAD;
			vt1.y *= EEdef_DEGTORAD;
			vt1.z *= EEdef_DEGTORAD;
			QuatFromAngles(&qInvert, &vt1);
		}
	}

	EERIE_3D tv;

	if ((io) && (io->ioflags & IO_ITEM))
		Vector_Init(&tv, pos->x, pos->y - 60.f, pos->z);
	else
		Vector_Init(&tv, pos->x, pos->y - 90.f, pos->z);


	for (long l = 0; l != MAX_LLIGHTS; l++)
	{
		if (llights[l])
		{
			float		oolength = 1.f / dists[l];
			vLight.x = (llights[l]->pos.x - tv.x) * oolength;
			vLight.y = (llights[l]->pos.y - tv.y) * oolength;
			vLight.z = (llights[l]->pos.z - tv.z) * oolength;

			TransformInverseVertexQuat(&qInvert, &vLight, &vTLights[l]);
		}
		else
			break;
	}

	long paf[3];
	paf[0] = eobj->facelist[ii].vid[0];
	paf[1] = eobj->facelist[ii].vid[1];
	paf[2] = eobj->facelist[ii].vid[2];

	for (long i = 0; i < 3; i++)
	{
		float r, g, b;
		long ir, ig, ib;

		if ((io) && (io->ioflags & (IO_NPC | IO_ITEM)))
		{
			r = g = b = NPC_ITEMS__AMBIENT_VALUE_255;
		}
		else
		{
			r = ACTIVEBKG->ambient255.r;
			g = ACTIVEBKG->ambient255.g;
			b = ACTIVEBKG->ambient255.b;
		}

		EERIE_3D * posVert = &eobj->vertexlist3[paf[i]].v;

		for (int l = 0 ; l != MAX_LLIGHTS; l++)
		{
			EERIE_LIGHT * Cur_llights = llights[l];

			if (Cur_llights)
			{
				float	cosangle;
				float		oolength = 1.f / EEDistance3D((EERIE_3D *)posVert, &Cur_llights->pos); //dists[l];
				vLight.x = (llights[l]->pos.x - posVert->x) * oolength;
				vLight.y = (llights[l]->pos.y - posVert->y) * oolength;
				vLight.z = (llights[l]->pos.z - posVert->z) * oolength;

				TransformInverseVertexQuat(&qInvert, &vLight, &vTLights[l]);
				EERIE_3D * Cur_vLights = &vTLights[l];
	
				cosangle = (eobj->facelist[ii].norm.x * Cur_vLights->x +
				            eobj->facelist[ii].norm.y * Cur_vLights->y +
				            eobj->facelist[ii].norm.z * Cur_vLights->z) * DIV2;

				// If light visible
				if (cosangle > 0.f)
				{
					float distance = EEDistance3D((EERIE_3D *)posVert, &Cur_llights->pos);

					// Evaluate its intensity depending on the distance Light<->Object
					if (distance <= Cur_llights->fallstart)
						cosangle *= Cur_llights->precalc; 
					else
					{
						float p = ((Cur_llights->fallend - distance) * Cur_llights->falldiffmul);

						if (p <= 0.f)
							cosangle = 0.f;
						else
							cosangle *= p * Cur_llights->precalc; 
					}

					r += Cur_llights->rgb255.r * cosangle;
					g += Cur_llights->rgb255.g * cosangle;
					b += Cur_llights->rgb255.b * cosangle;
				}
			}
			else
				break;
		}

		if (eobj->drawflags & DRAWFLAG_HIGHLIGHT)
		{
			r += iHighLight; 
			g += iHighLight; 
			b += iHighLight;
		}

		if (Project.improve)
		{
			r *= infra->r;
			g *= infra->g;
			b *= infra->b;
		}

		F2L(r, &ir);
		ir = clipByte255(ir);
		F2L(g, &ig);
		ig = clipByte255(ig);
		F2L(b, &ib);
		ib = clipByte255(ib);
		eobj->vertexlist3[paf[i]].vert.color = (0xff000000L | (((ir) & 255) << 16) |	(((ig) & 255) << 8) | (ib) & 255);
	}
}


extern long DYNAMIC_NORMALS;

void ApplyDynLight(EERIEPOLY * ep)
{
	long nbvert, i;

	if (ep->type & POLY_QUAD) nbvert = 4;
	else nbvert = 3;

	if (TOTPDL == 0)
	{
		for (i = 0; i < nbvert; i++)
			ep->tv[i].color = ep->v[i].color;

		return;
	}

	EERIE_RGB rgb;
	long j;

	register float epr[4];
	register float epg[4];
	register float epb[4];

	for (i = 0; i < nbvert; i++)
	{
		long c = ep->v[i].color;
		epr[i] = (float)((c >> 16) & 255);
		epg[i] = (float)((c >> 8) & 255);
		epb[i] = (float)(c & 255);
	}

	for (i = 0; i < TOTPDL; i++)
	{
		EERIE_LIGHT * el = PDL[i];

		if (!(GetMaxManhattanDistance(&el->pos, (EERIE_3D *)&ep->center) <= el->fallend + 35.f))
		{
			TSU_TEST_NB_LIGHT ++;
			continue;
		}

		float d = EEDistance3D(&el->pos, (EERIE_3D *)&ep->center);

		if (d <= el->fallend + 35.f)
		{
			if (Project.improve)
			{
				rgb.r = el->rgb255.r * 4.f;
				rgb.g = rgb.b = 0.2f;
			}
			else
			{
				rgb.r = el->rgb255.r;
				rgb.g = el->rgb255.g;
				rgb.b = el->rgb255.b;
			}

			for (j = 0; j < nbvert; j++)
			{
				if (!(GetMaxManhattanDistance(&el->pos, (EERIE_3D *)&ep->v[j]) <= el->fallend))
				{
					TSU_TEST_NB ++;
					continue;
				}

				d = EEDistance3D(&el->pos, (EERIE_3D *)&ep->v[j]);

				if (d <= el->fallend)
				{
					float divd = 1.f / d;
					float nvalue;

					if (DYNAMIC_NORMALS)
					{
						EERIE_3D v1;
						v1.x = (el->pos.x - ep->v[j].sx) * divd;
						v1.y = (el->pos.y - ep->v[j].sy) * divd;
						v1.z = (el->pos.z - ep->v[j].sz) * divd;
						nvalue = Vector_DotProduct(&v1, &ep->nrml[j]) * DIV2;

						if (nvalue > 1.f) nvalue = 1.f;
						else if (nvalue < 0.f) nvalue = 0.f;
					}
					else nvalue = 1.f;

					if (nvalue > 0.f)
					{
						////
						if (d <= el->fallstart)
						{
							d = nvalue * el->precalc;
						}
						else
						{
							d -= el->fallstart;
							d = (el->falldiff - d) * el->falldiffmul * nvalue * el->precalc;
						}

						epr[j] += rgb.r * d;
						epg[j] += rgb.g * d;
						epb[j] += rgb.b * d;
					}
				}
				else if (d > el->fallend + 100.f) break;
			}
		}
	}

	long lepr, lepg, lepb;

	for (j = 0; j < nbvert; j++)
	{
		F2L(epr[j], &lepr);
		lepr = clipByte255(lepr);
		F2L(epg[j], &lepg);
		lepg = clipByte255(lepg);
		F2L(epb[j], &lepb);
		lepb = clipByte255(lepb);
		ep->tv[j].color = (0xFF000000L  |
		                   (lepr << 16) |
		                   (lepg << 8) |
		                   (lepb));
	}
}
 
float TOTAL_CHRONO = 0.f;
//*************************************************************************************
void ApplyDynLight_VertexBuffer(EERIEPOLY * ep, SMY_D3DVERTEX * _pVertex, unsigned short _usInd0, unsigned short _usInd1, unsigned short _usInd2, unsigned short _usInd3)
{
	long nbvert, i;

	if (ep->type & POLY_QUAD) nbvert = 4;
	else nbvert = 3;

	if (TOTPDL == 0)
	{

		ep->tv[0].color = _pVertex[_usInd0].color = ep->v[0].color;
		ep->tv[1].color = _pVertex[_usInd1].color = ep->v[1].color;
		ep->tv[2].color = _pVertex[_usInd2].color = ep->v[2].color;

		if (nbvert & 4)
		{
			ep->tv[3].color = _pVertex[_usInd3].color = ep->v[3].color;
		}

		return;
	}

	long j;
	register float d;
	register float epr[4];
	register float epg[4];
	register float epb[4];
	
	// To keep...
	//	register float DVAL;
	//	if (LIGHTPOWERUP) DVAL=LPpower*100.f;
	//	else
	//	DVAL=300.f;
	//register float nvalue;


	for (i = 0; i < nbvert; i++)
	{
		long c = ep->v[i].color;
		epr[i] = (float)(long)((c >> 16) & 255);
		epg[i] = (float)(long)((c >> 8) & 255);
		epb[i] = (float)(long)(c & 255);
	}

	for (i = 0; i < TOTPDL; i++)
	{
		EERIE_LIGHT * el = PDL[i];

		for (j = 0; j < nbvert; j++)
		{
			d = EEDistance3D(&el->pos, (EERIE_3D *)&ep->v[j]);

			if (d < el->fallend)
			{
				float nvalue;

				nvalue =	((el->pos.x - ep->v[j].sx) * ep->nrml[j].x
				             +	(el->pos.y - ep->v[j].sy) * ep->nrml[j].y
				             +	(el->pos.z - ep->v[j].sz) * ep->nrml[j].z
				         ) * 0.5f / d; 

				if (nvalue > 0.f)
				{
					if (d <= el->fallstart)
					{
						d = el->precalc * nvalue; 
					}
					else
					{
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

	long lepr, lepg, lepb;

	F2L(epr[0], &lepr);
	lepr = clipByte255(lepr);
	F2L(epg[0], &lepg);
	lepg = clipByte255(lepg);
	F2L(epb[0], &lepb);
	lepb = clipByte255(lepb);
	ep->tv[0].color = _pVertex[_usInd0].color =	(0xFF000000L  |
	                  (lepr << 16) |
	                  (lepg << 8) |
	                  (lepb));
	F2L(epr[1], &lepr);
	lepr = clipByte255(lepr);
	F2L(epg[1], &lepg);
	lepg = clipByte255(lepg);
	F2L(epb[1], &lepb);
	lepb = clipByte255(lepb);
	ep->tv[1].color = _pVertex[_usInd1].color =	(0xFF000000L  |
	                  (lepr << 16) |
	                  (lepg << 8) |
	                  (lepb));

	F2L(epr[2], &lepr);
	lepr = clipByte255(lepr);
	F2L(epg[2], &lepg);
	lepg = clipByte255(lepg);
	F2L(epb[2], &lepb);
	lepb = clipByte255(lepb);
	ep->tv[2].color = _pVertex[_usInd2].color =	(0xFF000000L  |
	                  (lepr << 16) |
	                  (lepg << 8) |
	                  (lepb));

	if (nbvert & 4)
	{
		F2L(epr[3], &lepr);
		lepr = clipByte255(lepr);
		F2L(epg[3], &lepg);
		lepg = clipByte255(lepg);
		F2L(epb[3], &lepb);
		lepb = clipByte255(lepb);
		ep->tv[3].color = _pVertex[_usInd3].color =	(0xFF000000L  |
		                  (lepr << 16) |
		                  (lepg << 8) |
		                  (lepb));
	}
}

extern TILE_LIGHTS tilelights[MAX_BKGX][MAX_BKGZ];
//*************************************************************************************

void ApplyDynLight_VertexBuffer_2(EERIEPOLY * ep, short _x, short _y, SMY_D3DVERTEX * _pVertex, unsigned short _usInd0, unsigned short _usInd1, unsigned short _usInd2, unsigned short _usInd3)
{
	long nbvert, i;

	if (ep->type & POLY_QUAD) nbvert = 4;
	else nbvert = 3;

	TILE_LIGHTS * tls = &tilelights[_x][_y];

	if (tls->num == 0)
	{
		ep->tv[0].color = _pVertex[_usInd0].color = ep->v[0].color;
		ep->tv[1].color = _pVertex[_usInd1].color = ep->v[1].color;
		ep->tv[2].color = _pVertex[_usInd2].color = ep->v[2].color;

		if (nbvert & 4)
		{
			ep->tv[3].color = _pVertex[_usInd3].color = ep->v[3].color;
		}

		return;
	}

	long j;
	register float d;
	register float epr[4];
	register float epg[4];
	register float epb[4];
	
	// To keep...
	//	register float DVAL;
	//	if (LIGHTPOWERUP) DVAL=LPpower*100.f;
	//	else
	//	DVAL=300.f;
	//register float nvalue;


	for (i = 0; i < nbvert; i++)
	{
		long c = ep->v[i].color;
		epr[i] = (float)(long)((c >> 16) & 255);
		epg[i] = (float)(long)((c >> 8) & 255);
		epb[i] = (float)(long)(c & 255);
	}

	for (i = 0; i < tls->num; i++)
	{
		EERIE_LIGHT * el = tls->el[i];
	
		for (j = 0; j < nbvert; j++)
		{
		
			d = EEDistance3D(&el->pos, (EERIE_3D *)&ep->v[j]);

			if (d < el->fallend)
			{
				float nvalue;
				
				nvalue =	((el->pos.x - ep->v[j].sx) * ep->nrml[j].x
				             +	(el->pos.y - ep->v[j].sy) * ep->nrml[j].y
				             +	(el->pos.z - ep->v[j].sz) * ep->nrml[j].z
				         ) * 0.5f / d; 
				
				if (nvalue > 0.f)
				{
					if (d <= el->fallstart)
					{
						
						d = el->precalc * nvalue; 
					}
					else
					{
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

	long lepr, lepg, lepb;

	F2L(epr[0], &lepr);
	lepr = clipByte255(lepr);
	F2L(epg[0], &lepg);
	lepg = clipByte255(lepg);
	F2L(epb[0], &lepb);
	lepb = clipByte255(lepb);
	ep->tv[0].color = _pVertex[_usInd0].color =	(0xFF000000L  |
	                  (lepr << 16) |
	                  (lepg << 8) |
	                  (lepb));
	F2L(epr[1], &lepr);
	lepr = clipByte255(lepr);
	F2L(epg[1], &lepg);
	lepg = clipByte255(lepg);
	F2L(epb[1], &lepb);
	lepb = clipByte255(lepb);
	ep->tv[1].color = _pVertex[_usInd1].color =	(0xFF000000L  |
	                  (lepr << 16) |
	                  (lepg << 8) |
	                  (lepb));

	F2L(epr[2], &lepr);
	lepr = clipByte255(lepr);
	F2L(epg[2], &lepg);
	lepg = clipByte255(lepg);
	F2L(epb[2], &lepb);
	lepb = clipByte255(lepb);
	ep->tv[2].color = _pVertex[_usInd2].color =	(0xFF000000L  |
	                  (lepr << 16) |
	                  (lepg << 8) |
	                  (lepb));

	if (nbvert & 4)
	{
		F2L(epr[3], &lepr);
		lepr = clipByte255(lepr);
		F2L(epg[3], &lepg);
		lepg = clipByte255(lepg);
		F2L(epb[3], &lepb);
		lepb = clipByte255(lepb);
		ep->tv[3].color = _pVertex[_usInd3].color =	(0xFF000000L  |
		                  (lepr << 16) |
		                  (lepg << 8) |
		                  (lepb));
	}

}

#endif
