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
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////                                                                                     
//////////////////////////////////////////////////////////////////////////////////////
// EERIEAnim
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		Animation funcs
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "animation/Animation.h"

#include <cstdio>
#include <cassert>
#include <climits>

#include "animation/AnimationRender.h"

#include "core/Time.h"
#include "core/Core.h"

#include "game/Damage.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "graphics/GraphicsEnum.h"
#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/Mesh.h"
#include "graphics/particle/ParticleEffects.h"

#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Logger.h"

#include "physics/Clothes.h"
#include "physics/Collisions.h"

#include "scene/Object.h"
#include "scene/Light.h"
#include "scene/GameSound.h"
#include "scene/Scene.h"
#include "scene/Interactive.h"

using std::min;
using std::max;

//-----------------------------------------------------------------------------
void PushInterBump(TextureContainer *_pTex,D3DTLVERTEX *_pVertex);
void PopOneInterBump(TextureContainer *_pTex);
//-----------------------------------------------------------------------------

#define MAX_DIST_BUMP_INTER (400.f)
extern float IN_FRONT_DIVIDER_ITEMS;
long MAX_LLIGHTS=18;
//-----------------------------------------------------------------------------
extern long FINAL_RELEASE;
extern EERIE_CAMERA TCAM[32];
extern QUAKE_FX_STRUCT QuakeFx;
extern float _framedelay;
extern float FORCED_REDUCTION_VALUE;
extern float METALdecal;
extern long ForceIODraw;
extern long INTER_DRAW;
extern long INTER_COMPUTE;
extern long FRAME_COUNT;
extern unsigned long ulBKGColor;
extern bool bALLOW_BUMP;
extern CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBuffer;
extern CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBufferTransform;
extern CMY_DYNAMIC_VERTEXBUFFER *pDynamicVertexBuffer_TLVERTEX;	// VB using TLVERTEX format.
extern CMY_DYNAMIC_VERTEXBUFFER* pDynamicVertexBufferBump;		// VB for Bump Mapping (duplicate pDynamicVertexBuffer).
extern long ZMAPMODE;
extern float fZFogStart;
//-----------------------------------------------------------------------------
ANIM_HANDLE animations[MAX_ANIMATIONS];
bool MIPM;
D3DTLVERTEX LATERDRAWHALO[HALOMAX*4];
EERIE_LIGHT * llights[32];
EERIE_QUAT * BIGQUAT;
EERIEMATRIX * BIGMAT;
float		dists[32];
float		values[32];
float vdist;
long __MUST_DRAW=0;
long FORCE_NO_HIDE=0;
long DEBUG_PATHFAIL=1;
long LOOK_AT_TARGET=0;
#if !CEDRIC
	static unsigned char * grps=NULL;
	static long max_grps=0;	
#else
	unsigned char * grps=NULL;
	long max_grps=0;	
#endif
long TRAP_DETECT=-1;
long TRAP_SECRET=-1;
long USEINTERNORM=1;
long HALOCUR=0;
long anim_power[] = { 100, 20, 15, 12, 8, 6, 5, 4, 3, 2, 2, 1, 1, 1, 1 };

//-----------------------------------------------------------------------------
int iNbD3DTLVERTEXTab;
SMY_D3DVERTEX3_T tD3DTLVERTEXTab[4000];
D3DTLVERTEX tD3DTLVERTEXTab2[4000];

void CalculTriangleBump( const D3DTLVERTEX& v0, const D3DTLVERTEX& v1, const D3DTLVERTEX& v2, float *du, float *dv );

extern long EXTERNALVIEW;
void EERIE_ANIM_Get_Scale_Invisibility(INTERACTIVE_OBJ * io,float &invisibility,float &scale)
{
	if (io)
	{
		invisibility=io->invisibility;

		if (invisibility>1.f) invisibility-=1.f;

		if ((io!=inter.iobj[0]) && (invisibility>0.f) && (!EXTERNALVIEW))
		{
			long num=ARX_SPELLS_GetSpellOn(io,SPELL_INVISIBILITY);

			if (num>=0)
			{
				if (player.Full_Skill_Intuition>spells[num].caster_level*10)
				{
					invisibility-=(float)player.Full_Skill_Intuition*( 1.0f / 100 )+(float)spells[num].caster_level*( 1.0f / 10 );

					if (invisibility<0.1f) invisibility=0.1f;
					else if (invisibility>1.f) invisibility=1.f;
				}
			}
		}

		// Scaling Value for this object (Movements will also be scaled)
		scale=io->scale;
	}
	else 
	{
		invisibility=0.f;
		scale=1.f;
	}
}	

//*************************************************************************************
// ANIMATION HANDLES handling
//*************************************************************************************

//-----------------------------------------------------------------------------
short ANIM_GetAltIdx(ANIM_HANDLE * ah,long old)
{

	if (ah->alt_nb==1) return 0;	

	float tot=(float)anim_power[0];

	for (long i=1;i<ah->alt_nb;i++)
	{
		tot+=anim_power[min(i,14L)];
	}

	while (1)
	{
		for (short i=0;i<ah->alt_nb;i++)
		{
			float rnd=rnd()*tot;

			if ((rnd<anim_power[min((int)i,14)]) && (i!=old))
				return i;
		}
	}
}

//-----------------------------------------------------------------------------
void ANIM_Set(ANIM_USE * au,ANIM_HANDLE * anim)
{
	if (	(!au)
		||	(!anim) )
		return;

	au->cur_anim=anim;
	au->altidx_cur=ANIM_GetAltIdx(anim,au->altidx_cur);

	if (au->altidx_cur>au->cur_anim->alt_nb) 
		au->altidx_cur=0;

	au->ctime=0;
	au->lastframe=-1;
	au->flags&=~EA_PAUSED;
	au->flags&=~EA_ANIMEND;
	au->flags&=~EA_LOOP;
	au->flags&=~EA_FORCEPLAY;	
}

//-----------------------------------------------------------------------------
void EERIE_ANIMMANAGER_Init()
{
	memset(animations,0,sizeof(ANIM_HANDLE)*MAX_ANIMATIONS);
}

//-----------------------------------------------------------------------------

void EERIE_ANIMMANAGER_PurgeUnused()
{
	for (long i=0;i<MAX_ANIMATIONS;i++)
	{
		if (	(animations[i].path[0]!=0)
			&&	(animations[i].locks==0)	)
		{
			for (long k=0;k<animations[i].alt_nb;k++)
			{
				ReleaseAnim(animations[i].anims[k]);
				animations[i].anims[k]=NULL;					
			}

			if (animations[i].anims)
				free(animations[i].anims);

			animations[i].anims=NULL;

			if (animations[i].sizes)
				free(animations[i].sizes);

			animations[i].sizes=NULL;
			animations[i].path[0]=0;				
		}
	}
}

//-----------------------------------------------------------------------------
void EERIE_ANIMMANAGER_ReleaseHandle(ANIM_HANDLE * anim)
{
	if (anim)
	{
		anim->locks--;

		if (anim->locks<0) anim->locks=0;
	}
}

//-----------------------------------------------------------------------------
ANIM_HANDLE * EERIE_ANIMMANAGER_GetHandle(const char * path)
{
	for (long i=0;i<MAX_ANIMATIONS;i++)
	{
		if (!strcasecmp(animations[i].path,path))
		{
			return &animations[i];
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
long EERIE_ANIMMANAGER_AddAltAnim(ANIM_HANDLE * ah, char * path) {
	
	if(!ah || !ah->path[0]) {
		return 0;
	}
	
	size_t FileSize;
	unsigned char * adr = (unsigned char *)PAK_FileLoadMalloc( path, FileSize );
	if(!adr) {
		return 0;
	}
	
	EERIE_ANIM * temp = TheaToEerie(adr,FileSize,path);
	free(adr);
	if(!temp) {
		return 0;
	}
	
	ah->alt_nb++;
	ah->anims=(EERIE_ANIM **)realloc(ah->anims,sizeof(EERIE_ANIM *)*ah->alt_nb);
	ah->sizes=(long *)realloc(ah->sizes,sizeof(long)*ah->alt_nb);
	ah->anims[ah->alt_nb-1]=temp;
	ah->sizes[ah->alt_nb-1]=FileSize;		
	return 1;
}

//-----------------------------------------------------------------------------
ANIM_HANDLE * EERIE_ANIMMANAGER_Load( const std::string& _path)
{
	std::string path = _path;

	ANIM_HANDLE * handl=EERIE_ANIMMANAGER_GetHandle(path.c_str());

	if (handl) 
	{
		handl->locks++;
		return handl;
	}

	unsigned char * adr;
	size_t FileSize;
	char path2[256];
	int pathcount = 2;

	for (long i=0;i<MAX_ANIMATIONS;i++)
	{
		if (animations[i].path[0]==0)
		{				
			if ((adr=(unsigned char *)PAK_FileLoadMalloc(path,FileSize))!=NULL)
			{
				animations[i].anims=(EERIE_ANIM **)malloc(sizeof(EERIE_ANIM *));
				animations[i].sizes=(long *)malloc(sizeof(long));

				animations[i].anims[0]=TheaToEerie(adr,FileSize,path.c_str());
				animations[i].sizes[0]=FileSize;
				animations[i].alt_nb=1;
				free(adr);

				if (animations[i].anims[0]==NULL) return NULL;			

				strcpy(animations[i].path,path.c_str());				
				animations[i].locks=1;

				//remove extension
				path = path.substr(0, path.size()-4);

				sprintf(path2,"%s%d.tea",path.c_str(),pathcount);

				while (EERIE_ANIMMANAGER_AddAltAnim(&animations[i],path2))
				{
					pathcount++;
					sprintf(path2,"%s%d.tea",path.c_str(),pathcount);
				}

				return &animations[i];
			}				
			
			return NULL;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// tex Must be of sufficient size...
long EERIE_ANIMMANAGER_Count( std::string& tex, long * memsize)
{
	char temp[512];
	long count=0;
	*memsize=0;

	for (long i=0;i<MAX_ANIMATIONS;i++)
	{
		if (animations[i].path[0]!=0)
		{
			count++;
			char txx[256];
			strcpy(txx,animations[i].path);
			GetName(txx);
			long totsize=0;

			for (long k=0;k<animations[i].alt_nb;k++)
				totsize+=animations[i].sizes[k];

			sprintf(temp,"%3ld[%3ld] %s size %ld Locks %ld Alt %d\r\n",count,i,txx,totsize,animations[i].locks,animations[i].alt_nb-1);
			memsize+=totsize;
			tex += temp;
		}
	}	

	return count;
}

//*************************************************************************************
// Fill "pos" with "eanim" total translation
//*************************************************************************************
void GetAnimTotalTranslate( ANIM_HANDLE * eanim,long alt_idx,EERIE_3D * pos)
{

	if (	(!eanim)
		||	(!pos)
		||	(!eanim->anims[alt_idx])
		||	(!eanim->anims[alt_idx]->frames)
		||	(eanim->anims[alt_idx]->nb_key_frames<=0) )
	{
		pos->x=0;
		pos->y=0;
		pos->z=0;
		return;
	}

	EERIE_3D * e3D=&eanim->anims[alt_idx]->frames[eanim->anims[alt_idx]->nb_key_frames-1].translate;
	pos->x=e3D->x;
	pos->y=e3D->y;
	pos->z=e3D->z;	
}

//*************************************************************************************
// Main Procedure to draw an animated object
//------------------------------------------
// Needs some update...
//  EERIE_3DOBJ * eobj				main object data
//  EERIE_ANIM * eanim				Animation data
//  EERIE_3D * angle				Object Angle
//  EERIE_3D  * pos					Object Position
//  unsigned long time				Time increment to current animation in Ms
//  INTERACTIVE_OBJ * io			Referrence to Interactive Object (NULL if no IO)
//  D3DCOLOR col					Color
//  long typ						Misc Type 0=World View 1=1st Person View
//*************************************************************************************

//-----------------------------------------------------------------------------
void PrepareAnim(EERIE_3DOBJ * eobj, ANIM_USE * eanim,unsigned long time,
							INTERACTIVE_OBJ * io)
{
	long tcf,tnf;
	long fr;
	float pour;
	long tim;
	
	if (	(!eobj)
		||	(!eanim)	)
		return;

	if (eanim->flags & EA_PAUSED) time=0;

	if ((io) && (io->ioflags & IO_FREEZESCRIPT)) time=0;

	if (eanim->altidx_cur>= eanim->cur_anim->alt_nb) eanim->altidx_cur=0;

	if (!(eanim->flags & EA_EXCONTROL))
		eanim->ctime+=time;

	eanim->flags&=~EA_ANIMEND;

	if (	(eanim->flags & EA_STOPEND)
		&&	(eanim->ctime > eanim->cur_anim->anims[eanim->altidx_cur]->anim_time)	)
	{
		eanim->ctime = eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
	}

	if ((eanim->flags & EA_LOOP) || (io && (
				(eanim->cur_anim==io->anims[ANIM_WALK])
			||	(eanim->cur_anim==io->anims[ANIM_WALK2])
			||	(eanim->cur_anim==io->anims[ANIM_WALK3])
			||	(eanim->cur_anim==io->anims[ANIM_RUN])
			||	(eanim->cur_anim==io->anims[ANIM_RUN2])
			||	(eanim->cur_anim==io->anims[ANIM_RUN3]) ))

		)
	{
		
		if (eanim->ctime > eanim->cur_anim->anims[eanim->altidx_cur]->anim_time)
		{	
			long lost=(long)((long)eanim->ctime - (long)eanim->cur_anim->anims[eanim->altidx_cur]->anim_time);

			if (eanim->next_anim==NULL) 
			{			
				long t = eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
				eanim->ctime= eanim->ctime % t;

					if (io) FinishAnim(io,eanim->cur_anim);
					
				}				
			else 
			{			
				if (io) 
				{
					FinishAnim(io,eanim->cur_anim);

					if (io->lastanimtime!=0) AcquireLastAnim(io);
					else io->lastanimtime=1;
				}

				eanim->cur_anim=eanim->next_anim;
				eanim->altidx_cur=ANIM_GetAltIdx(eanim->next_anim,eanim->altidx_cur);
				eanim->next_anim=NULL;
				ResetAnim(eanim);
				eanim->ctime = lost; 
				eanim->flags=eanim->nextflags;
				eanim->flags&=~EA_ANIMEND;
				goto suite;
			}
		}
	}
	else if (eanim->ctime > eanim->cur_anim->anims[eanim->altidx_cur]->anim_time) 
	{				
		if (io)
		{
			long lost=(long)((long)eanim->ctime - (long)eanim->cur_anim->anims[eanim->altidx_cur]->anim_time);

			if (eanim->next_anim!=NULL)
			{
				
				FinishAnim(io,eanim->cur_anim);

				if (io->lastanimtime!=0) AcquireLastAnim(io);
				else io->lastanimtime=1;

				eanim->cur_anim=eanim->next_anim;
				eanim->altidx_cur=ANIM_GetAltIdx(eanim->next_anim,eanim->altidx_cur);				
				eanim->next_anim=NULL;
				ResetAnim(eanim);
				eanim->ctime = lost; 
				eanim->flags=eanim->nextflags;
				eanim->flags&=~EA_ANIMEND;
				goto suite;
			}
			else
			{
				eanim->ctime=(long)eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
				eanim->flags&=~EA_ANIMEND;
			}
		}

		eanim->flags|=EA_ANIMEND;
		eanim->ctime=(unsigned long)eanim->cur_anim->anims[eanim->altidx_cur]->anim_time;
	}

suite:
	;

	if (!eanim->cur_anim)
		return;

	if (eanim->flags & EA_REVERSE) 
		tim=(unsigned long)eanim->cur_anim->anims[eanim->altidx_cur]->anim_time - eanim->ctime;
	else	
		tim=eanim->ctime;

	eanim->fr=eanim->cur_anim->anims[eanim->altidx_cur]->nb_key_frames-2;
	eanim->pour=1.f;

	for (long i=1;i<eanim->cur_anim->anims[eanim->altidx_cur]->nb_key_frames;i++) 
	{
		tcf=(long)eanim->cur_anim->anims[eanim->altidx_cur]->frames[i-1].time;
		tnf=(long)eanim->cur_anim->anims[eanim->altidx_cur]->frames[i].time;

		if (tcf == tnf) return; 

		if (((tim<tnf) && (tim>=tcf)) 
			|| ((i==eanim->cur_anim->anims[eanim->altidx_cur]->nb_key_frames-1) && (tim==tnf)))
		{
			fr=i-1;
			tim-=tcf;
			pour=(float)((float)tim/((float)tnf-(float)tcf));			
			
			// Frame Sound Management
			if (!(eanim->flags & EA_ANIMEND) && time && 
					(eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].sample!=-1) && (eanim->lastframe!=fr))
			{
				if ((eanim->lastframe<fr) && (eanim->lastframe!=-1)) 
				{
					for (long n=eanim->lastframe+1;n<=fr;n++)
						ARX_SOUND_PlayAnim(eanim->cur_anim->anims[eanim->altidx_cur]->frames[n].sample, io ? &io->pos : NULL);
				}
				else 
				{
					ARX_SOUND_PlayAnim(eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].sample, io ? &io->pos : NULL);
				}
			}

			// Frame Flags Management
			if (!(eanim->flags & EA_ANIMEND) && time && 
					(eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].flag>0) && (eanim->lastframe!=fr))
			{
				if (io!=inter.iobj[0])
				{
					if ((eanim->lastframe<fr) && (eanim->lastframe!=-1)) 
					{
						for (long n=eanim->lastframe+1;n<=fr;n++)
						{
							if (eanim->cur_anim->anims[eanim->altidx_cur]->frames[n].flag==9) 
								ARX_NPC_NeedStepSound(io, &io->pos);
						}
					}
					else if (eanim->cur_anim->anims[eanim->altidx_cur]->frames[fr].flag == 9)
						ARX_NPC_NeedStepSound(io, &io->pos);
				}
			}
			
			// Memorize this frame as lastframe.
			eanim->lastframe=fr;	
			eanim->fr=fr;
			eanim->pour=pour;
			break;
		}	
	}
}
INTERACTIVE_OBJ * DESTROYED_DURING_RENDERING=NULL;							
extern long USE_CEDRIC_ANIM;
//-----------------------------------------------------------------------------
void EERIEDrawAnimQuat(	EERIE_3DOBJ * eobj,
						ANIM_USE * eanim,
						EERIE_3D * angle,
						EERIE_3D  * pos,
						unsigned long time,
						INTERACTIVE_OBJ * io,
						long typ
						)
{
	if (	(io)
		&&	(io!=inter.iobj[0])	)
	{
		float speedfactor = io->basespeed+io->speed_modif;

		if (speedfactor < 0) speedfactor = 0;

		float tim=(float)time*(speedfactor);

		if (tim<=0.f) time=0;
		else time=(unsigned long)tim;

		io->frameloss+=tim-time;

		if (io->frameloss>1.f) // recover lost time...
		{
			long tt = io->frameloss;
			io->frameloss-=tt;
			time+=tt;
		}
	}

	if (time <= 0) goto suite; 

	if (time>200) time=200; // TO REMOVE !!!!!!!!!

	PrepareAnim(eobj,eanim,time,io);
	
	if (io)
	for (long count=1;count<MAX_ANIM_LAYERS;count++)
	{
		ANIM_USE * animuse=&io->animlayer[count];

		if (animuse->cur_anim)
			PrepareAnim(eobj,animuse,time,io);
	}

suite:
	;

	DESTROYED_DURING_RENDERING=NULL;

	if (USE_CEDRIC_ANIM)
		Cedric_AnimateDrawEntity(eobj, eanim, angle, pos, io, typ);
}

#define ANIMQUATTYPE_FIRST_PERSON	2
#define ANIMQUATTYPE_NO_RENDER		4
#define ANIMQUATTYPE_NO_COMPUTATIONS	8
extern float GLOBAL_LIGHT_FACTOR;


//*************************************************************************************
// Procedure for drawing Interactive Objects (Not Animated)
//*************************************************************************************

void DrawEERIEInterMatrix(EERIE_3DOBJ * eobj,
					EERIEMATRIX * mat,EERIE_3D  * poss,INTERACTIVE_OBJ * io,EERIE_MOD_INFO * modinfo)
{
	BIGQUAT=NULL;
	BIGMAT=mat;

	if (BIGMAT==NULL) return;
	
	DrawEERIEInter(eobj,NULL,poss,io,modinfo);
	BIGMAT=NULL;
}
// List of TO-TREAT vertex for MIPMESHING

void specialEE_P(D3DTLVERTEX *in,D3DTLVERTEX *out);

// TODO: Convert to a RenderBatch & make TextureContainer constructor private 
TextureContainer TexSpecialColor("SPECIALCOLOR_LIST", TextureContainer::NoInsert);

//-----------------------------------------------------------------------------
ARX_D3DVERTEX * PushVertexInTableCull(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull+3)>pTex->ulMaxVertexListCull)
	{
		pTex->ulMaxVertexListCull+=10*3;
		pTex->pVertexListCull=(ARX_D3DVERTEX*)realloc(pTex->pVertexListCull,pTex->ulMaxVertexListCull*sizeof(ARX_D3DVERTEX));
	}

	pTex->ulNbVertexListCull+=3;
	return &pTex->pVertexListCull[pTex->ulNbVertexListCull-3];
}

//-----------------------------------------------------------------------------
ARX_D3DVERTEX * PushVertexInTableCull_TNormalTrans(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TNormalTrans+3)>pTex->ulMaxVertexListCull_TNormalTrans)
	{
		pTex->ulMaxVertexListCull_TNormalTrans+=20*3;
		pTex->pVertexListCull_TNormalTrans=(ARX_D3DVERTEX*)realloc(pTex->pVertexListCull_TNormalTrans,pTex->ulMaxVertexListCull_TNormalTrans*sizeof(ARX_D3DVERTEX));

		if (!pTex->pVertexListCull_TNormalTrans)
		{
			pTex->ulMaxVertexListCull_TNormalTrans=0;
			pTex->ulNbVertexListCull_TNormalTrans=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TNormalTrans+=3;
	return &pTex->pVertexListCull_TNormalTrans[pTex->ulNbVertexListCull_TNormalTrans-3];
}

//-----------------------------------------------------------------------------
ARX_D3DVERTEX * PushVertexInTableCull_TAdditive(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TAdditive+3)>pTex->ulMaxVertexListCull_TAdditive)
	{
		pTex->ulMaxVertexListCull_TAdditive+=20*3;
		pTex->pVertexListCull_TAdditive=(ARX_D3DVERTEX*)realloc(pTex->pVertexListCull_TAdditive,pTex->ulMaxVertexListCull_TAdditive*sizeof(ARX_D3DVERTEX));

		if (!pTex->pVertexListCull_TAdditive)
		{
			pTex->ulMaxVertexListCull_TAdditive=0;
			pTex->ulNbVertexListCull_TAdditive=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TAdditive+=3;
	return &pTex->pVertexListCull_TAdditive[pTex->ulNbVertexListCull_TAdditive-3];
}

//-----------------------------------------------------------------------------
ARX_D3DVERTEX * PushVertexInTableCull_TSubstractive(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TSubstractive+3)>pTex->ulMaxVertexListCull_TSubstractive)
	{
		pTex->ulMaxVertexListCull_TSubstractive+=20*3;
		pTex->pVertexListCull_TSubstractive=(ARX_D3DVERTEX*)realloc(pTex->pVertexListCull_TSubstractive,pTex->ulMaxVertexListCull_TSubstractive*sizeof(ARX_D3DVERTEX));

		if (!pTex->pVertexListCull_TSubstractive)
		{
			pTex->ulMaxVertexListCull_TSubstractive=0;
			pTex->ulNbVertexListCull_TSubstractive=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TSubstractive+=3;
	return &pTex->pVertexListCull_TSubstractive[pTex->ulNbVertexListCull_TSubstractive-3];
}

//-----------------------------------------------------------------------------
ARX_D3DVERTEX * PushVertexInTableCull_TMultiplicative(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TMultiplicative+3)>pTex->ulMaxVertexListCull_TMultiplicative)
	{
		pTex->ulMaxVertexListCull_TMultiplicative+=20*3;
		pTex->pVertexListCull_TMultiplicative=(ARX_D3DVERTEX*)realloc(pTex->pVertexListCull_TMultiplicative,pTex->ulMaxVertexListCull_TMultiplicative*sizeof(ARX_D3DVERTEX));

		if (!pTex->pVertexListCull_TMultiplicative)
		{
			pTex->ulMaxVertexListCull_TMultiplicative=0;
			pTex->ulNbVertexListCull_TMultiplicative=0;
			return NULL;
		}
	}

	pTex->ulNbVertexListCull_TMultiplicative+=3;
	return &pTex->pVertexListCull_TMultiplicative[pTex->ulNbVertexListCull_TMultiplicative-3];
}

//-----------------------------------------------------------------------------
ARX_D3DVERTEX * PushVertexInTableCull_TMetal(TextureContainer *pTex)
{
	if((pTex->ulNbVertexListCull_TMetal+3)>pTex->ulMaxVertexListCull_TMetal)
	{
		pTex->ulMaxVertexListCull_TMetal+=20*3;
		pTex->pVertexListCull_TMetal=(ARX_D3DVERTEX*)realloc(pTex->pVertexListCull_TMetal,pTex->ulMaxVertexListCull_TMetal*sizeof(ARX_D3DVERTEX));
	}

	pTex->ulNbVertexListCull_TMetal+=3;
	return &pTex->pVertexListCull_TMetal[pTex->ulNbVertexListCull_TMetal-3];
}

void PopOneTriangleListClipp(D3DTLVERTEX *_pVertex,int *_piNbVertex);


//-----------------------------------------------------------------------------
void PopOneTriangleList(TextureContainer *_pTex,bool _bUpdate)
{
	if(	!_pTex->ulNbVertexListCull )
	{
		return;
	}

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetTexture(0, _pTex);

	if (_pTex->userflags & POLY_LATE_MIP)
	{
		const float GLOBAL_NPC_MIPMAP_BIAS = -2.2f;
		GRenderer->GetTextureStage(0)->SetMipMapLODBias(GLOBAL_NPC_MIPMAP_BIAS);
	}
	
	if( _pTex->ulNbVertexListCull )
	{
		EERIEDRAWPRIM(	D3DPT_TRIANGLELIST,
						D3DFVF_TLVERTEX,
						_pTex->pVertexListCull,
						_pTex->ulNbVertexListCull,
						0, 
						0 );

		if(_bUpdate) 
			_pTex->ulNbVertexListCull = 0;
	}
	
	if (_pTex->userflags & POLY_LATE_MIP)
	{
		float biasResetVal = 0;
		GRenderer->GetTextureStage(0)->SetMipMapLODBias(biasResetVal);
	}
}

//-----------------------------------------------------------------------------
void PopOneTriangleListTransparency(TextureContainer *_pTex)
{
	if(	!_pTex->ulNbVertexListCull_TNormalTrans&&
		!_pTex->ulNbVertexListCull_TAdditive&&
		!_pTex->ulNbVertexListCull_TSubstractive&&
		!_pTex->ulNbVertexListCull_TMultiplicative&&
		!_pTex->ulNbVertexListCull_TMetal
		) return;

	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetTexture(0, _pTex);

	if(	_pTex->ulNbVertexListCull_TNormalTrans )
	{
		GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendSrcColor);

		if(_pTex->ulNbVertexListCull_TNormalTrans)
		{
			EERIEDRAWPRIM(D3DPT_TRIANGLELIST, 
				D3DFVF_TLVERTEX,
				_pTex->pVertexListCull_TNormalTrans,
				_pTex->ulNbVertexListCull_TNormalTrans,
				0, 0 );
			_pTex->ulNbVertexListCull_TNormalTrans=0;
		}
	}
	
	if(	_pTex->ulNbVertexListCull_TAdditive )
	{
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

		if(_pTex->ulNbVertexListCull_TAdditive)
		{
			EERIEDRAWPRIM(D3DPT_TRIANGLELIST, 
				D3DFVF_TLVERTEX,
				_pTex->pVertexListCull_TAdditive,
				_pTex->ulNbVertexListCull_TAdditive,
				0, 0 );
			_pTex->ulNbVertexListCull_TAdditive=0;
		}
	}

	if (_pTex->ulNbVertexListCull_TSubstractive)
	{
		GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);	

		if(_pTex->ulNbVertexListCull_TSubstractive)
		{
			EERIEDRAWPRIM(D3DPT_TRIANGLELIST, 
				D3DFVF_TLVERTEX,
				_pTex->pVertexListCull_TSubstractive,
				_pTex->ulNbVertexListCull_TSubstractive,
				0, 0 );
			_pTex->ulNbVertexListCull_TSubstractive=0;
		}
	}

	if (_pTex->ulNbVertexListCull_TMultiplicative)
	{
		GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);

		if(_pTex->ulNbVertexListCull_TMultiplicative)
		{
			EERIEDRAWPRIM(D3DPT_TRIANGLELIST, 
				D3DFVF_TLVERTEX,
				_pTex->pVertexListCull_TMultiplicative,
				_pTex->ulNbVertexListCull_TMultiplicative,
				0, 0 );
			_pTex->ulNbVertexListCull_TMultiplicative=0;
		}
	}

	if(	_pTex->ulNbVertexListCull_TMetal )
	{
		GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);

		if(_pTex->ulNbVertexListCull_TMetal)
		{
			EERIEDRAWPRIM(D3DPT_TRIANGLELIST, 
				D3DFVF_TLVERTEX,
				_pTex->pVertexListCull_TMetal,
				_pTex->ulNbVertexListCull_TMetal,
				0, 0 );
			_pTex->ulNbVertexListCull_TMetal=0;
		}
	}
}

//-----------------------------------------------------------------------------
void PopAllTriangleList(bool _bUpdate)
{
	D3DMATRIX matbase;

	TextureContainer *pTex=GetTextureList();
	while(pTex)
	{
		PopOneTriangleList(pTex,_bUpdate);
		pTex=pTex->m_pNext;
	}
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
 
		vector<SMY_ZMAPPINFO>::iterator it;

		for (it = _pTex->TextureRefinement->vPolyInterZMap.begin();
			it != _pTex->TextureRefinement->vPolyInterZMap.end();
			++it)
		{
			SMY_ZMAPPINFO *pSMY = &(*it);
			float fColor;
			
			tD3DTLVERTEXTab2[iPos]			= pSMY->pD3DVertex[0];
			fColor							= pSMY->color[0];
			tD3DTLVERTEXTab2[iPos].color	= D3DRGB(fColor,fColor,fColor);
			tD3DTLVERTEXTab2[iPos].tu		= pSMY->uv[0];
			tD3DTLVERTEXTab2[iPos++].tv		= pSMY->uv[1];
			tD3DTLVERTEXTab2[iPos]			= pSMY->pD3DVertex[1];
			fColor							= pSMY->color[1];
			tD3DTLVERTEXTab2[iPos].color	= D3DRGB(fColor,fColor,fColor);
			tD3DTLVERTEXTab2[iPos].tu		= pSMY->uv[2];
			tD3DTLVERTEXTab2[iPos++].tv		= pSMY->uv[3];
			tD3DTLVERTEXTab2[iPos]			= pSMY->pD3DVertex[2];
			fColor							= pSMY->color[2];
			tD3DTLVERTEXTab2[iPos].color	= D3DRGB(fColor,fColor,fColor);
			tD3DTLVERTEXTab2[iPos].tu		= pSMY->uv[4];
			tD3DTLVERTEXTab2[iPos++].tv		= pSMY->uv[5];
		}

		EERIEDRAWPRIM(	D3DPT_TRIANGLELIST, 
						D3DFVF_TLVERTEX,
						tD3DTLVERTEXTab2,
						iPos,
						0, 0 );

		_pTex->TextureRefinement->vPolyInterZMap.clear();
	}
}

//-----------------------------------------------------------------------------
void PopAllTriangleListTransparency()
{
	TextureContainer *pTex=GetTextureList();

	D3DMATRIX matbase;

	GRenderer->SetFogColor(0);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetRenderState(Renderer::DepthWrite, false); 
	GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);	

	PopOneTriangleList(&TexSpecialColor);

	pTex=GetTextureList();

	while(pTex)
	{
		PopOneTriangleListTransparency(pTex);

		//ZMAP
		PopOneInterZMapp(pTex);

		//BUMP
		PopOneInterBump(pTex);

		pTex=pTex->m_pNext;
	}

	GRenderer->SetFogColor(ulBKGColor);
	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetRenderState(Renderer::DepthWrite, true);
}

//-----------------------------------------------------------------------------
void PushInterBump(TextureContainer *_pTex,D3DTLVERTEX *_pVertex)
{
	_pTex->vPolyInterBump.push_back(_pVertex[0]);
	_pTex->vPolyInterBump.push_back(_pVertex[1]);
	_pTex->vPolyInterBump.push_back(_pVertex[2]);
}

//-----------------------------------------------------------------------------
void PopOneInterBump(TextureContainer *_pTex)
{
	//BUMP
	if( _pTex->vPolyInterBump.size() )
	{
		//----------------------------------------------------------------------------------
		//																		Initializing
		GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendSrcColor);	
		GRenderer->SetTexture(0, _pTex->m_pTextureBump);
		GRenderer->SetTexture(1, _pTex->m_pTextureBump);
		GRenderer->GetTextureStage(1)->SetTextureCoordIndex(1);
		
		GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::ArgTexture);
		GRenderer->GetTextureStage(1)->SetColorOp(TextureStage::OpAddSigned, (TextureStage::TextureArg)(TextureStage::ArgTexture | TextureStage::ArgComplement), TextureStage::ArgCurrent);
		GRenderer->GetTextureStage(1)->DisableAlpha();
		GRenderer->GetTextureStage(2)->DisableColor();

		//----------------------------------------------------------------------------------
		//																				Loop
		iNbD3DTLVERTEXTab	=	0;
		vector<D3DTLVERTEX>::iterator iT;

		for( iT = _pTex->vPolyInterBump.begin() ; iT < _pTex->vPolyInterBump.end() ;  )
		{
			D3DTLVERTEX pVertexS[3];

			pVertexS[0]	=	*iT;	iT++;
			pVertexS[1]	=	*iT;	iT++;
			pVertexS[2]	=	*iT;	iT++;
			
			if(	!pVertexS[0].color&&
				!pVertexS[1].color&&
				!pVertexS[2].color) continue;
			
			float fDu, fDv;
			CalculTriangleBump( pVertexS[0], pVertexS[1], pVertexS[2], &fDu, &fDv );
			fDu			*=	.8f;
			fDv			*=	.8f;

			SMY_D3DVERTEX3_T *pVertex;
			pVertex=&tD3DTLVERTEXTab[iNbD3DTLVERTEXTab];
			iNbD3DTLVERTEXTab	+=	3;
			
			if( iNbD3DTLVERTEXTab > 4000 )
			{
				//----------------------------------------------------------------------------------
				//																			Flushing
				iNbD3DTLVERTEXTab-=3;
				
				EERIEDRAWPRIM(	D3DPT_TRIANGLELIST, 
								FVF_D3DVERTEX3_T,
								tD3DTLVERTEXTab,
								iNbD3DTLVERTEXTab,
								0, 0 );
			
				iNbD3DTLVERTEXTab	=	3;
				pVertex				=	tD3DTLVERTEXTab;
			}
			
			for (int idx = 0 ; idx < 3 ; ++idx)
			{
				pVertex->x		=	pVertexS[idx].sx;
				pVertex->y		=	pVertexS[idx].sy;
				pVertex->z		=	pVertexS[idx].sz;
				pVertex->rhw	=	pVertexS[idx].rhw;
				pVertex->color	=	pVertexS[idx].color;
				pVertex->tu		=	pVertexS[idx].tu;
				pVertex->tv		=	pVertexS[idx].tv;
				pVertex->tu2	=	pVertexS[idx].tu + fDu;
				pVertex->tv2	=	pVertexS[idx].tv + fDv;
				pVertex++;
			}
		}
		
		//----------------------------------------------------------------------------------
		//																	Drawing BUMP tex
		if( iNbD3DTLVERTEXTab )
		{
			EERIEDRAWPRIM(	D3DPT_TRIANGLELIST, 
							FVF_D3DVERTEX3_T,
							tD3DTLVERTEXTab,
							iNbD3DTLVERTEXTab,
							0, 0 );

			iNbD3DTLVERTEXTab	=	0;
		}

		//----------------------------------------------------------------------------------
		//																			  Ending
		GRenderer->GetTextureStage(0)->SetColorOp(TextureStage::OpModulate, TextureStage::ArgTexture, TextureStage::ArgDiffuse);
		GRenderer->GetTextureStage(1)->DisableColor();
		GRenderer->GetTextureStage(1)->SetTextureCoordIndex(0);
		_pTex->vPolyInterBump.clear();
	}
}

float INVISIBILITY_OVERRIDE=0.f;
extern void MakeCLight(INTERACTIVE_OBJ * io,EERIE_RGB * infra,EERIE_3D * angle,EERIE_3D * pos,EERIE_3DOBJ * eobj,EERIEMATRIX * BIGMAT,EERIE_QUAT *BIGQUAT);
extern void MakeCLight2(INTERACTIVE_OBJ * io,EERIE_RGB * infra,EERIE_3D * angle,EERIE_3D * pos,EERIE_3DOBJ * eobj,EERIEMATRIX * BIGMAT,EERIE_QUAT *BIGQUAT,long i);

//-----------------------------------------------------------------------------
void CalculateInterZMapp(EERIE_3DOBJ *_pobj3dObj,long lIdList,long *_piInd,TextureContainer *_pTex,D3DTLVERTEX *_pD3DVertex)
{
    SMY_ZMAPPINFO sZMappInfo;

	if( (!ZMAPMODE)||
		(!_pTex->TextureRefinement) ) return;

	bool bUp=false;

	if(	(fabs(_pobj3dObj->vertexlist[_piInd[0]].norm.y)>=.9f)||
		(fabs(_pobj3dObj->vertexlist[_piInd[1]].norm.y)>=.9f)||
		(fabs(_pobj3dObj->vertexlist[_piInd[2]].norm.y)>=.9f) )
	{
		bUp=true;
	}

	for(int iI=0;iI<3;iI++)
	{
		if(bUp)
		{
			sZMappInfo.uv[iI<<1]=(_pobj3dObj->vertexlist3[_piInd[iI]].v.x*( 1.0f / 50 ));
			sZMappInfo.uv[(iI<<1)+1]=(_pobj3dObj->vertexlist3[_piInd[iI]].v.z*( 1.0f / 50 ));
		}
		else
		{
			sZMappInfo.uv[iI<<1]=(_pobj3dObj->facelist[lIdList].u[iI]*4.f);
			sZMappInfo.uv[(iI<<1)+1]=(_pobj3dObj->facelist[lIdList].v[iI]*4.f);
		}

		float fDist=EEDistance3D(&ACTIVECAM->pos,(EERIE_3D *)&_pobj3dObj->vertexlist3[_piInd[iI]].v)-80.f;

		if (fDist<10.f) fDist=10.f;

		sZMappInfo.color[iI] = (150.f - fDist) * 0.006666666f;

		if (sZMappInfo.color[iI]<0.f) sZMappInfo.color[iI]=0.f;
	
		sZMappInfo.pD3DVertex[iI]=_pD3DVertex[iI];
	}

	//optim
	if(	(sZMappInfo.color[0]!=0.f)||
		(sZMappInfo.color[1]!=0.f)||
		(sZMappInfo.color[2]!=0.f) )
	{
		_pTex->TextureRefinement->vPolyInterZMap.push_back(sZMappInfo);
	}
}

//-----------------------------------------------------------------------------
int ARX_SoftClippZ(EERIE_VERTEX *_pVertex1,EERIE_VERTEX *_pVertex2,EERIE_VERTEX *_pVertex3,D3DTLVERTEX **_ptV,EERIE_FACE *_pFace,float _fInvibility,TextureContainer *_pTex,bool _bBump,bool _bZMapp,EERIE_3DOBJ *_pObj,int _iNumFace,long *_pInd,INTERACTIVE_OBJ *_pioInteractive,bool _bNPC,long _lSpecialColorFlag,EERIE_RGB *_pRGB);
extern EERIEMATRIX ProjectionMatrix;
extern long FORCE_FRONT_DRAW;
//-----------------------------------------------------------------------------
void DrawEERIEInter2(EERIE_3DOBJ * eobj,
					EERIE_3D * angle, EERIE_3D  * poss, INTERACTIVE_OBJ * io, EERIE_MOD_INFO * modinfo )
{
	if ( !eobj ) return;

	EERIE_3D pos;	
	// Resets 2D Bounding Box
	BBOXMIN.y	=	BBOXMIN.x	=	32000;
	BBOXMAX.y	=	BBOXMAX.x	=	-32000;
	pos.x		=	poss->x;
	pos.y		=	poss->y;
	pos.z		=	poss->z;
	
	// Avoids To treat an object that isn't Visible
	if (	( io )
		&&	( io != inter.iobj[0] )
		&&	( !modinfo ) 
			&&	(!ForceIODraw)
		&&	( !__MUST_DRAW )
			&&	(ACTIVEBKG))   
	{		
		long xx, yy;
		xx = (pos.x) * ACTIVEBKG->Xmul;
		yy = (pos.z) * ACTIVEBKG->Zmul;

		if ( ( xx >= 1 ) && ( yy >= 1 ) && ( xx < ACTIVEBKG->Xsize - 1 ) && ( yy < ACTIVEBKG->Zsize - 1 ) )
		{
			long ok = 0;

			for (long ky = yy - 1 ; ky <= yy + 1 ; ky++ )
			{
				for ( long kx = xx - 1 ; kx <= xx + 1 ; kx++ )
				{
					FAST_BKG_DATA * feg	=	(FAST_BKG_DATA *)&ACTIVEBKG->fastdata[kx][ky];

					if ( feg->treat ) 
					{
						ok = 1;
						break;
					}
				}

				if ( ok ) 
					break;
			}

			if ( !ok ) return;
		}
	}

	float					Xcos = 0, Ycos = 0, Zcos = 0, Xsin = 0, Ysin = 0, Zsin = 0;
	D3DTLVERTEX vert_list_static[4];
	long					k;
	
	long			lfr, lfg, lfb;		
	EERIE_3D		temporary3D;
	EERIE_CAMERA	Ncam;
	EERIE_RGB		infra;
	float			invisibility;
	float			scale;

	EERIE_ANIM_Get_Scale_Invisibility( io, invisibility, scale );

	if ( ( !io ) && ( INVISIBILITY_OVERRIDE != 0.f ) )
		invisibility = INVISIBILITY_OVERRIDE;
	
	// Precalc rotations
	if ( angle != NULL )
	{
		if ( modinfo )
			Zsin = (float)radians( MAKEANGLE( angle->a + modinfo->rot.a ) );
		else
			Zsin = (float)radians( angle->a );

		Ncam.Xcos	=	Xcos	=	(float)EEcos( Zsin );
		Ncam.Xsin	=	Xsin	=	(float)EEsin( Zsin );

		if ( modinfo )
			Zsin	=	(float)radians( MAKEANGLE( angle->b + modinfo->rot.b ) );
		else
			Zsin	=	(float)radians( angle->b );

		Ncam.Ycos	=	Ycos	=	(float)EEcos( Zsin );
		Ncam.Ysin	=	Ysin	=	(float)EEsin( Zsin );
		
		if ( modinfo )
			Zsin	=	(float)radians( MAKEANGLE( angle->g + modinfo->rot.g ) );
		else
			Zsin	=	(float)radians( angle->g );

		Ncam.Zcos	=	Zcos	=	(float)EEcos( Zsin );
		Ncam.Zsin	=	Zsin	=	(float)EEsin( Zsin );
	}
	
	// Test for Mipmeshing then pre-computes vertices
	MIPM	=	0;
	vdist	=	0.f;
	{
		ResetBBox3D( io );

		for(size_t i = 0 ; i < eobj->vertexlist.size() ; i++ ) 
		{
			if ( (modinfo) && !angle && BIGMAT )
			{
				vert_list_static[0].sx	=	(eobj->vertexlist[i].v.x-modinfo->link_position.x) * scale;
				vert_list_static[0].sy	=	(eobj->vertexlist[i].v.y-modinfo->link_position.y) * scale;
				vert_list_static[0].sz	=	(eobj->vertexlist[i].v.z-modinfo->link_position.z) * scale;
			}
			else if (scale != 1.f)
			{
				vert_list_static[0].sx	=	eobj->vertexlist[i].v.x * scale;
				vert_list_static[0].sy	=	eobj->vertexlist[i].v.y * scale;
				vert_list_static[0].sz	=	eobj->vertexlist[i].v.z * scale;
			}
			else
			{
				vert_list_static[0].sx	=	eobj->vertexlist[i].v.x;
				vert_list_static[0].sy	=	eobj->vertexlist[i].v.y;
				vert_list_static[0].sz	=	eobj->vertexlist[i].v.z;
			}

			if ( !angle )
			{
				if ( ( io != NULL ) && ( modinfo == NULL ) )
				{
					
					vert_list_static[0].sx	-=	io->obj->pbox->vert[0].initpos.x * scale-io->obj->point0.x;
					vert_list_static[0].sy	-=	io->obj->pbox->vert[0].initpos.y * scale-io->obj->point0.y;
					vert_list_static[0].sz	-=	io->obj->pbox->vert[0].initpos.z * scale-io->obj->point0.z;
				}

				if ( BIGQUAT == NULL )
					VertexMatrixMultiply( (EERIE_3D *)&vert_list_static[1], (EERIE_3D *)&vert_list_static[0], BIGMAT );
				else 
					TransformVertexQuat ( BIGQUAT, (EERIE_3D *)&vert_list_static[0], (EERIE_3D *)&vert_list_static[1] );

				eobj->vertexlist3[i].v.x	=	vert_list_static[1].sx	+=	pos.x;
				eobj->vertexlist3[i].v.y	=	vert_list_static[1].sy	+=	pos.y;
				eobj->vertexlist3[i].v.z	=	vert_list_static[1].sz	+=	pos.z;
	
				specialEE_RT( &vert_list_static[1], &eobj->vertexlist[i].vworld );
				specialEE_P( &eobj->vertexlist[i].vworld, &eobj->vertexlist[i].vert );
			}
			else
			{
				_YRotatePoint( (EERIE_3D *)&vert_list_static[0], (EERIE_3D *)&vert_list_static[1], Ycos, Ysin );
				_XRotatePoint( (EERIE_3D *)&vert_list_static[1], (EERIE_3D *)&vert_list_static[0], Xcos, Xsin );

				// Misc Optim to avoid 1 infrequent rotation around Z
				if ( Zsin == 0.f ) 
				{
					eobj->vertexlist3[i].v.x	=	vert_list_static[0].sx	+=	pos.x;
					eobj->vertexlist3[i].v.y	=	vert_list_static[0].sy	+=	pos.y;
					eobj->vertexlist3[i].v.z	=	vert_list_static[0].sz	+=	pos.z;
					
					specialEE_RT(&vert_list_static[0],&eobj->vertexlist[i].vworld);
					specialEE_P(&eobj->vertexlist[i].vworld,&eobj->vertexlist[i].vert);
				}
				else 
				{			
					_ZRotatePoint( (EERIE_3D *) &vert_list_static[0], (EERIE_3D *)&vert_list_static[1], Zcos, Zsin );
					eobj->vertexlist3[i].v.x	=	vert_list_static[1].sx	+=	pos.x;
					eobj->vertexlist3[i].v.y	=	vert_list_static[1].sy	+=	pos.y;
					eobj->vertexlist3[i].v.z	=	vert_list_static[1].sz	+=	pos.z;
				
					specialEE_RT( &vert_list_static[1], &eobj->vertexlist[i].vworld);
					specialEE_P( &eobj->vertexlist[i].vworld, &eobj->vertexlist[i].vert);
				} 
			}

			// Memorizes 2D Bounding Box using vertex min/max x,y pos
			if (eobj->vertexlist[i].vert.rhw>0.f) 
			{
				if ((eobj->vertexlist[i].vert.sx >= -32000) &&
					(eobj->vertexlist[i].vert.sx <= 32000) &&
					(eobj->vertexlist[i].vert.sy >= -32000) &&
					(eobj->vertexlist[i].vert.sy <= 32000))
				{
					BBOXMIN.x=min(BBOXMIN.x,eobj->vertexlist[i].vert.sx);
					BBOXMAX.x=max(BBOXMAX.x,eobj->vertexlist[i].vert.sx);
					BBOXMIN.y=min(BBOXMIN.y,eobj->vertexlist[i].vert.sy);
					BBOXMAX.y=max(BBOXMAX.y,eobj->vertexlist[i].vert.sy);
				}
			}

			AddToBBox3D(io,&eobj->vertexlist3[i].v);
		}
	}
	
	if (	(BBOXMAX.x<=1)
		||	(BBOXMIN.x>=DANAESIZX-1)
		||	(BBOXMAX.y<=1) 
		||	(BBOXMIN.y>=DANAESIZY-1)		
		)
		goto finish;

	if ((!modinfo) && (ARX_SCENE_PORTAL_ClipIO(io,&pos)))
		return;
	
	
	// Precalc local lights for this object then interpolate
	if (FRAME_COUNT <= 0)
	{	
		MakeCLight(io,&infra,angle,&pos,eobj,BIGMAT,BIGQUAT);
	}

	INTER_DRAW++;
	float ddist;
	ddist=0;
	long need_halo;
	need_halo=0;

	if (	(io)
		&&	(io->halo.flags & HALO_ACTIVE) )
	{
		float mdist=ACTIVECAM->cdepth;
		ddist=mdist-EEDistance3D(&pos,&ACTIVECAM->pos);		
		ddist=(ddist/mdist);
		ddist*=ddist*ddist*ddist*ddist*ddist;

		if (ddist<=0.25f) ddist=0.25f;
		else if (ddist>0.9f) ddist=0.9f;

		need_halo=1;
	}
	
	long		special_color_flag;
				special_color_flag	=	0;
	EERIE_RGB	special_color;
				special_color.r		=	special_color.g		=	special_color.b		=	0; 

	if ( io ) 
	{
		float poisonpercent	=	0.f;
		float trappercent	=	0.f;
		float secretpercent	=	0.f;

		if ( io->ioflags & IO_NPC )
		{
			if ( (DEBUG_PATHFAIL) && (!FINAL_RELEASE) )
			{
				if ( io->_npcdata->pathfind.listnb == -2)
					trappercent = 1;

				if ( io->_npcdata->pathfind.pathwait)
					poisonpercent = 1;
			}

			if ( io->_npcdata->poisonned > 0.f )
			{
				poisonpercent = io->_npcdata->poisonned * ( 1.0f / 20 );

				if ( poisonpercent > 1.f ) poisonpercent = 1.f;
			}
		}

		if ((io->ioflags & IO_ITEM) && (io->poisonous>0.f) && (io->poisonous_count!=0)) 
		{
			poisonpercent=(float)io->poisonous*( 1.0f / 20 );

			if (poisonpercent>1.f) poisonpercent=1.f;
		}

		if ((io->ioflags & IO_FIX) && (io->_fixdata->trapvalue>-1)) 
		{
			trappercent=(float)TRAP_DETECT-(float)io->_fixdata->trapvalue;

			if (trappercent>0.f)
			{
				trappercent = 0.6f + trappercent * ( 1.0f / 100 ); 

				if (trappercent<0.6f) trappercent=0.6f;

				if (trappercent>1.f) trappercent=1.f;
			}
		}

		if ((io->ioflags & IO_FIX) && (io->secretvalue>-1)) 
		{
			secretpercent=(float)TRAP_SECRET-(float)io->secretvalue;

			if (secretpercent>0.f)
			{
				secretpercent = 0.6f + secretpercent * ( 1.0f / 100 ); 

				if (secretpercent<0.6f) secretpercent=0.6f;

				if (secretpercent>1.f) secretpercent=1.f;
			}
		}

		if ( poisonpercent > 0.f )
		{
			special_color_flag	=	1;
			special_color.r		=	0.f;
			special_color.g		=	1.f;
			special_color.b		=	0.f;
		}

		if ( trappercent > 0.f )
		{
			special_color_flag	=	1;
			special_color.r		=	trappercent;
			special_color.g		=	1.f - trappercent;
			special_color.b		=	1.f - trappercent;
		}

		if ( secretpercent > 0.f )
		{
			special_color_flag	=	1;
			special_color.r		=	1.f - secretpercent;
			special_color.g		=	1.f - secretpercent;
			special_color.b		=	secretpercent;
		}

		if (io->sfx_flag & SFX_TYPE_YLSIDE_DEATH)
		{
			if (io->show==SHOW_FLAG_TELEPORTING)
			{
				float fCalc = io->sfx_time + FrameDiff ;
				ARX_CHECK_ULONG(fCalc);

				io->sfx_time = ARX_CLEAN_WARN_CAST_ULONG(fCalc) ;

				if (io->sfx_time >= ARXTimeUL())
					io->sfx_time = ARXTimeUL();
			}
			else
			{
						special_color_flag	=	1;
				float	elapsed				=	ARXTime - io->sfx_time;

				if ( elapsed > 0.f )
				{
					if ( elapsed < 3000.f ) // 5 seconds to red
					{
						float ratio		=	elapsed * ( 1.0f / 3000 );
						special_color.r	=	1.f;
						special_color.g	=	1.f - ratio;
						special_color.b	=	1.f - ratio;
						AddRandomSmoke( io, 1 );
					}
					else if ( elapsed < 6000.f ) // 5 seconds to White				
					{
						float ratio			=	( elapsed - 3000.f ) * ( 1.0f / 3000 );
						special_color.r		=	1.f;
						special_color.g		=	ratio;
						special_color.b		=	ratio;
						special_color_flag	=	2;
						AddRandomSmoke( io, 2 );
					}
					else if ( elapsed < 8000.f ) // 5 seconds to White				
					{
						float ratio			=	( elapsed - 6000.f ) * ( 1.0f / 2000 );
						special_color.r		=	ratio;
						special_color.g		=	ratio;
						special_color.b		=	ratio;
						special_color_flag	=	2;
						AddRandomSmoke( io, 2 );
					}
					else // SFX finish
					{
						special_color_flag	=	0;
						
						io->sfx_time=0;

						if (io->ioflags & IO_NPC)
						{
							MakePlayerAppearsFX(io);
							AddRandomSmoke(io,50);
							EERIE_RGB rgb;
							unsigned long color=io->_npcdata->blood_color;
							rgb.r=(float)((long)((color>>16) & 255))*( 1.0f / 255 );
							rgb.g=(float)((long)((color>>8) & 255))*( 1.0f / 255 );
							rgb.b=(float)((long)((color) & 255))*( 1.0f / 255 );
							EERIE_SPHERE sp;
							sp.origin.x=io->pos.x;
							sp.origin.y=io->pos.y;
							sp.origin.z=io->pos.z;
							sp.radius=200.f;
							long count=6;

							while (count--)
							{							
								SpawnGroundSplat(&sp,&rgb,rnd()*30.f+30.f,1);
								sp.origin.y-=rnd()*150.f;

								ARX_PARTICLES_Spawn_Splat(&sp.origin,200,io->_npcdata->blood_color);

								sp.origin.x=io->pos.x+rnd()*200.f-100.f;
								sp.origin.y=io->pos.y+rnd()*20.f-10.f;
								sp.origin.z=io->pos.z+rnd()*200.f-100.f;
								sp.radius=rnd()*100.f+100.f;							
							}

							if (io->sfx_flag & SFX_TYPE_INCINERATE)
							{
								io->sfx_flag&=~SFX_TYPE_INCINERATE;
								io->sfx_flag&=~SFX_TYPE_YLSIDE_DEATH;
								long num=ARX_SPELLS_GetSpellOn(io, SPELL_INCINERATE);

								while (num>=0)
								{
									spells[num].tolive=0;
									ARX_DAMAGES_DamageNPC(io,20,0,1,&inter.iobj[0]->pos);
									num=ARX_SPELLS_GetSpellOn(io, SPELL_INCINERATE);
								}
							}
							else
							{
								io->sfx_flag&=~SFX_TYPE_YLSIDE_DEATH;
								ARX_INTERACTIVE_DestroyIO(io);
								DESTROYED_DURING_RENDERING=io;
								return;
							}
							
						}
					}
				}
				else
				{
					ARX_CHECK_NO_ENTRY(); //To avoid using special_color when it is not defined, currently equal 0
				}
			}
		}
	}

	bool bBumpOnIO;
	float fDist;
	fDist=EEDistance3D(&pos,&ACTIVECAM->pos);
	bBumpOnIO=(bALLOW_BUMP)&&(io)&&(io->ioflags&IO_BUMP)&&(fDist<min(max(0.f,(ACTIVECAM->cdepth*fZFogStart)-200.f),600.f))?true:false;

	float prec;
	prec=1.f/(ACTIVECAM->cdepth*ACTIVECAM->Zmul);

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		long paf[3];
		paf[0]=eobj->facelist[i].vid[0];
		paf[1]=eobj->facelist[i].vid[1];
		paf[2]=eobj->facelist[i].vid[2];

		//CULL3D
		EERIE_3D nrm;
		nrm.x=eobj->vertexlist3[paf[0]].v.x-ACTIVECAM->pos.x;
		nrm.y=eobj->vertexlist3[paf[0]].v.y-ACTIVECAM->pos.y;
		nrm.z=eobj->vertexlist3[paf[0]].v.z-ACTIVECAM->pos.z;

		if(!(eobj->facelist[i].facetype&POLY_DOUBLESIDED))
		{
			EERIE_3D normV10;
			EERIE_3D normV20;
			normV10.x=eobj->vertexlist3[paf[1]].v.x-eobj->vertexlist3[paf[0]].v.x;
			normV10.y=eobj->vertexlist3[paf[1]].v.y-eobj->vertexlist3[paf[0]].v.y;
			normV10.z=eobj->vertexlist3[paf[1]].v.z-eobj->vertexlist3[paf[0]].v.z;
			normV20.x=eobj->vertexlist3[paf[2]].v.x-eobj->vertexlist3[paf[0]].v.x;
			normV20.y=eobj->vertexlist3[paf[2]].v.y-eobj->vertexlist3[paf[0]].v.y;
			normV20.z=eobj->vertexlist3[paf[2]].v.z-eobj->vertexlist3[paf[0]].v.z;
			EERIE_3D normFace;
			normFace.x=(normV10.y*normV20.z)-(normV10.z*normV20.y);
			normFace.y=(normV10.z*normV20.x)-(normV10.x*normV20.z);
			normFace.z=(normV10.x*normV20.y)-(normV10.y*normV20.x);

			if((DOTPRODUCT( normFace , nrm )>0.f) ) continue;
		}

		ARX_D3DVERTEX		*vert_list;
		TextureContainer	*pTex;

		if(	(eobj->facelist[i].texid<0)|| 
			(!(pTex=eobj->texturecontainer[eobj->facelist[i].texid])) ) continue;

		if ((io) && (io->ioflags & IO_ANGULAR))
			MakeCLight2(io,&infra,angle,&pos,eobj,BIGMAT,BIGQUAT,i);

		unsigned long*	pNbVertexPush;
		float			fTransp = 0.f;

		if(	(eobj->facelist[i].facetype&POLY_TRANS) ||
			(invisibility>0.f) )
		{
			if(invisibility>0.f) 
				fTransp=2.f-invisibility;
			else
				fTransp=eobj->facelist[i].transval;
			
			if (fTransp>=2.f)  //MULTIPLICATIVE
			{
				fTransp*=( 1.0f / 2 );
				fTransp+=0.5f;
				
				{
					vert_list=PushVertexInTableCull_TMultiplicative(pTex);
					pNbVertexPush=&pTex->ulNbVertexListCull_TMultiplicative;
				}
			}
			else
			{
				if(fTransp>=1.f) //ADDITIVE
				{	
					fTransp-=1.f;

					{
						vert_list=PushVertexInTableCull_TAdditive(pTex);
						pNbVertexPush=&pTex->ulNbVertexListCull_TAdditive;
					}
				}
				else
				{
					if(fTransp>0.f)  //NORMAL TRANS
					{
						fTransp=1.f-fTransp;

						{
							vert_list=PushVertexInTableCull_TNormalTrans(pTex);
							pNbVertexPush=&pTex->ulNbVertexListCull_TNormalTrans;
						}
					}
					else  //SUBTRACTIVE
					{
						fTransp=1.f-fTransp;
		
						{
							vert_list=PushVertexInTableCull_TSubstractive(pTex);
							pNbVertexPush=&pTex->ulNbVertexListCull_TSubstractive;
						}
					}
				}				
			}
		}
		else
		{
			{
				vert_list=PushVertexInTableCull(pTex);
				pNbVertexPush=&pTex->ulNbVertexListCull;
			}
		}

		vert_list[0]=eobj->vertexlist[paf[0]].vert;
		vert_list[1]=eobj->vertexlist[paf[1]].vert;
		vert_list[2]=eobj->vertexlist[paf[2]].vert;

		vert_list[0].tu=eobj->facelist[i].u[0];
		vert_list[0].tv=eobj->facelist[i].v[0];
		vert_list[1].tu=eobj->facelist[i].u[1];
		vert_list[1].tv=eobj->facelist[i].v[1];
		vert_list[2].tu=eobj->facelist[i].u[2];
		vert_list[2].tv=eobj->facelist[i].v[2];	

		if (FORCE_FRONT_DRAW)
		{
			vert_list[0].sz*=IN_FRONT_DIVIDER_ITEMS;
			vert_list[1].sz*=IN_FRONT_DIVIDER_ITEMS;
			vert_list[2].sz*=IN_FRONT_DIVIDER_ITEMS;
		}

		// Treat WATER Polys (modify UVs)
		if (eobj->facelist[i].facetype & POLY_WATER)
		{
			for (k=0;k<3;k++) 
			{ 
				vert_list[k].tu=eobj->facelist[i].u[k];
				vert_list[k].tv=eobj->facelist[i].v[k];
				ApplyWaterFXToVertex((EERIE_3D *)&eobj->vertexlist[eobj->facelist[i].vid[k]].v,&vert_list[k],0.3f);
			}
		}

	if (FRAME_COUNT<=0)
	{
		if (io)
		{
			// Frozen status override all other colors
			if (io->ioflags & IO_FREEZESCRIPT)
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color=0xFF0000FF;
			}
			// Is it a Glowing Poly ?
			else if (eobj->facelist[i].facetype & POLY_GLOW)
				vert_list[0].color=vert_list[1].color=vert_list[2].color=0xffffffff;
			// Are we using Normal Illuminations ?
			else if (USEINTERNORM) 
			{
				for (long j=0;j<3;j++)
				{			
					vert_list[j].color=eobj->vertexlist3[paf[j]].vert.color;
				}					
			}
			// Are we using IMPROVED VISION view ?
			else if (Project.improve) 
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color
					=EERIERGB(io->infracolor.r,io->infracolor.g,io->infracolor.b);
			}
			else 
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color=0xFFFFFFFF;
			}

			if(Project.improve)
			{
				int to=3;

				for (long k=0;k<to;k++) 
				{
					long lfr,lfb;
					float fr,fb;
					long lr=(vert_list[k].color>>16) & 255;
					float ffr=(float)(lr);
					
					float dd=(vert_list[k].rhw*prec);

					if (dd>1.f) dd=1.f;

					if (dd<0.f) dd=0.f;
					
					fb=((1.f-dd)*6.f + (EEfabs(eobj->vertexlist[paf[k]].norm.x)+EEfabs(eobj->vertexlist[paf[k]].norm.y)))*0.125f;
					fr=((0.6f-dd)*6.f + (EEfabs(eobj->vertexlist[paf[k]].norm.z)+EEfabs(eobj->vertexlist[paf[k]].norm.y)))*0.125f;						

					if (fr<0.f) fr=0.f;
					else fr=max(ffr,fr*255.f);

					fr=min(fr,255.f);
					fb*=255.f;
					fb=min(fb,255.f);
					lfr = fr;
					lfb = fb;
					vert_list[k].color=( 0xff001E00L | ( (lfr & 255) << 16) | (lfb & 255) );
				}
			}
		}
		else
		{
			// Treating GLOWING POLY (unaffected by light)
			if (eobj->facelist[i].facetype & POLY_GLOW)
				vert_list[0].color=vert_list[1].color=vert_list[2].color=0xffffffff;
				else if (USEINTERNORM) // using INTERNORM lighting
			{
				for (long j=0;j<3;j++)
				{			
						vert_list[j].color = eobj->vertexlist3[paf[j]].vert.color; 
				}					
			}
			else if (Project.improve) // using IMPROVED VISION view
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color=EERIERGB(0.6f,0.f,1.f);
			}
			else // using default white
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color=EERIECOLOR_WHITE;
			}
		}

		if ( special_color_flag & 1)
		{
			for ( long j = 0 ; j < 3 ; j++ )
			{	
				vert_list[j].color = 0xFF000000L 
				                     | (((long)((float)((long)((vert_list[j].color >> 16) & 255)) * (special_color.r ) ) & 255 ) << 16 ) 
				                     | (((long)((float)((long)((vert_list[j].color >> 8) & 255))*special_color.g)&255) << 8) 
				                     | ((long)((float)((long) (vert_list[j].color & 255))*(special_color.b))&255);
			}
		}
	}			

	if (FRAME_COUNT!=0)
	for (long j=0;j<3;j++)		
		vert_list[j].color=eobj->facelist[i].color[j];		
	else 
	for (long j=0;j<3;j++)		
		eobj->facelist[i].color[j]=vert_list[j].color;		

	// Transparent poly: storing info to draw later
	if ((eobj->facelist[i].facetype & POLY_TRANS) 
		|| (invisibility>0.f))
	{
		vert_list[0].color	=	D3DRGB( fTransp, fTransp, fTransp );
		vert_list[1].color	=	D3DRGB( fTransp, fTransp, fTransp );
		vert_list[2].color	=	D3DRGB( fTransp, fTransp, fTransp );

	}

	int iNbPointAdd;
	if(!(iNbPointAdd=ARX_SoftClippZ(	&eobj->vertexlist[paf[0]],
						&eobj->vertexlist[paf[1]],
						&eobj->vertexlist[paf[2]],
						&vert_list,
						&eobj->facelist[i],
						invisibility,
						pTex,
						bBumpOnIO,
						(io)&&(io->ioflags&IO_ZMAP),
						eobj,
						i,
						paf,
						io,
						false,
						0,
						NULL)))
	{
		*pNbVertexPush-=3;
		continue;
	}

	if(bBumpOnIO)
	{
		PushInterBump(pTex,vert_list);
	}

	if((io)&&(io->ioflags&IO_ZMAP))
	{
		CalculateInterZMapp(eobj,i,paf,pTex,vert_list);
	}

	// Add some fake specular to Metallic polys
	if ((eobj->facelist[i].facetype & POLY_METAL)
		|| ((pTex) && (pTex->userflags & POLY_METAL)) )				
	{
		ARX_D3DVERTEX *vert_list_metal;

		unsigned long *pulNbVertexList_TMetal;
		{
			vert_list_metal=PushVertexInTableCull_TMetal(pTex);
			pulNbVertexList_TMetal=&pTex->ulNbVertexListCull_TMetal;
		}

		memcpy((void*)vert_list_metal,(void*)vert_list,sizeof(D3DTLVERTEX)*3);
		D3DTLVERTEX * tl=vert_list_metal;

		long r = 0, g = 0, b = 0;
		long todo	=	0;

		for ( long j = 0 ; j < 3 ; j++ )
		{ 
			r	=	( tl->color >> 16 ) & 255;
			g	=	( tl->color >> 8 ) & 255;
			b	=	tl->color & 255;
			
			if ( r > 192 || g > 192 || b > 192 )
			{
				todo++;
			}

			r -= 192;

			if ( r < 0.f ) r = 0;

			g -= 192;

			if ( g < 0.f ) g = 0;

			b -= 192;

			if ( b < 0.f ) b = 0;

			tl->color = 0xFF000000 | ( r << 18 ) | ( g << 10 ) | ( b << 2 );
			tl++;
		}

		if ( todo )
		{
			if ( ( todo > 2 ) && ( rnd() > 0.997f ) )
			{
				if ( io )
					SpawnMetalShine( (EERIE_3D *)&eobj->vertexlist3[eobj->facelist[i].vid[0]].vert, r, g, b, GetInterNum( io ) );
			}
		}
		else
		{
			*pulNbVertexList_TMetal -= 3;
		}
	}


	////////////////////////////////////////////////////////////////////////
	// HALO HANDLING START
	if	(need_halo)			
	{
		Ncam.Xcos = 1.f;
		Ncam.Xsin = 0.f;
		Ncam.Zcos = 1.f;
		Ncam.Zsin = 0.f;
		float power=radians(MAKEANGLE(subj.angle.b));
		Ncam.Ycos = (float)EEcos(power);	
		Ncam.Ysin = (float)EEsin(power);
		float tot=0;
		float _ffr[3];
			
		ARX_D3DVERTEX * workon=vert_list;

		for (long o=0;o<3;o++)
		{
			if (BIGMAT!=NULL)
				VertexMatrixMultiply(&temporary3D, (EERIE_3D *)&eobj->vertexlist[paf[o]].norm, BIGMAT );
			else 
				_YXZRotatePoint(&eobj->vertexlist[paf[o]].norm,&temporary3D,&Ncam);
	
			power=255.f-(float)EEfabs(255.f*(temporary3D.z)*( 1.0f / 2 ));

			if (power>255.f) 
			{
				power=255.f;					
			}
			else if (power<0.f) 
				power=0.f;

			tot+=power;
			
			_ffr[o]=power;
			lfr = io->halo.color.r * power;
			lfg = io->halo.color.g * power;
			lfb = io->halo.color.b * power;
			workon[o].color = (0xFF << 24) | ((lfr & 0xFF) << 16) | ((lfg & 0xFF) << 8) | (lfb & 0xFF);
		}

			if (tot>150.f)
			{
				long first;
				long second;
				long third;

				if ( (_ffr[0]>=_ffr[1]) && (_ffr[1]>=_ffr[2]))
			{
				first = 0;
				second = 1;
				third = 2;
			}
				else if ( (_ffr[0]>=_ffr[2]) && (_ffr[2]>=_ffr[1]))
			{
				first = 0;
				second = 2;
				third = 1;
			}
				else if ( (_ffr[1]>=_ffr[0]) && (_ffr[0]>=_ffr[2]))
			{
				first = 1;
				second = 0;
				third = 2;
			}
				else if ( (_ffr[1]>=_ffr[2]) && (_ffr[2]>=_ffr[0]))
			{
				first = 1;
				second = 2;
				third = 0;
			}
				else if ( (_ffr[2]>=_ffr[0]) && (_ffr[0]>=_ffr[1]))
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

			if ((_ffr[first] > 70.f) && (_ffr[second] > 60.f)) 
				{
					EERIE_3D vect1,vect2;
					D3DTLVERTEX * vert=&LATERDRAWHALO[(HALOCUR<<2)];						

					if (HALOCUR<(HALOMAX-1)) HALOCUR++;

					memcpy(&vert[0],&workon[first],sizeof(D3DTLVERTEX));
					memcpy(&vert[1],&workon[first],sizeof(D3DTLVERTEX));
					memcpy(&vert[2],&workon[second],sizeof(D3DTLVERTEX));
					memcpy(&vert[3],&workon[second],sizeof(D3DTLVERTEX));						

					float siz=ddist*(io->halo.radius*1.5f*(EEsin((float)(FrameTime+i)*( 1.0f / 100 ))*( 1.0f / 10 )+0.7f))*0.6f;
					vect1.x=workon[first].sx-workon[third].sx;
					vect1.y=workon[first].sy-workon[third].sy;						
					float len1=1.f/EEsqrt(vect1.x*vect1.x+vect1.y*vect1.y);

					if (vect1.x<0.f) len1*=1.2f;

					vect1.x*=len1;
					vect1.y*=len1;
					vect2.x=workon[second].sx-workon[third].sx;
					vect2.y=workon[second].sy-workon[third].sy;
					float len2=1.f/EEsqrt(vect2.x*vect2.x+vect2.y*vect2.y);

					if (vect2.x<0.f) len2*=1.2f;

					vect2.x*=len2;
					vect2.y*=len2;
				vert[1].sx += (vect1.x + 0.2f - rnd() * 0.1f) * siz; 
				vert[1].sy += (vect1.y + 0.2f - rnd() * 0.1f) * siz; 
					vert[1].color=0xFF000000;

					vert[0].sz += 0.0001f;
					vert[3].sz += 0.0001f; 

					vert[1].rhw*=.8f;
					vert[2].rhw*=.8f;
				vert[2].sx += (vect2.x + 0.2f - rnd() * 0.1f) * siz; 
				vert[2].sy += (vect2.y + 0.2f - rnd() * 0.1f) * siz; 

					if (io->halo.flags & HALO_NEGATIVE) 
						vert[2].color=0x00000000;					
					else 
						vert[2].color=0xFF000000;
				}
			}
		}
	}

// HALO HANDLING END
////////////////////////////////////////////////////////////////////////

finish:

	// storing 2D Bounding Box info
	if (io) 
	{
		io->bbox1.x=(short)BBOXMIN.x;
		io->bbox2.x=(short)BBOXMAX.x;
		io->bbox1.y=(short)BBOXMIN.y;
		io->bbox2.y=(short)BBOXMAX.y;
	}
}

extern bool bRenderInterList;
//-----------------------------------------------------------------------------
void DrawEERIEInter(EERIE_3DOBJ * eobj,
					EERIE_3D * angle,EERIE_3D  * poss,INTERACTIVE_OBJ * io,EERIE_MOD_INFO * modinfo)
{
	DESTROYED_DURING_RENDERING=NULL;

	if(bRenderInterList)
	{
		DrawEERIEInter2(eobj,
							angle,
							poss,
							io,
							modinfo);
	 return;
	}

	if (eobj==NULL) return;

	EERIE_3D pos;	
	// Resets 2D Bounding Box
	BBOXMIN.y=BBOXMIN.x=32000;
	BBOXMAX.y=BBOXMAX.x=-32000;
	pos.x=poss->x;
	pos.y=poss->y;
	pos.z=poss->z;
	
	// Avoids To treat an object that isn't Visible
	if (io!=inter.iobj[0])
		if ((!ForceIODraw) && (!__MUST_DRAW) && (ACTIVEBKG != NULL) && (io != NULL)) 
	{
		
		long xx,yy;
		xx = pos.x * ACTIVEBKG->Xmul;
		yy = pos.z * ACTIVEBKG->Zmul;

		if ( (xx>=1) && (yy>=1) && (xx<ACTIVEBKG->Xsize-1) && (yy<ACTIVEBKG->Zsize-1) )
		{
			long ok=0;

			for (long ky=yy-1;ky<=yy+1;ky++)
			for (long kx=xx-1;kx<=xx+1;kx++)
			{
				FAST_BKG_DATA * feg=(FAST_BKG_DATA *)&ACTIVEBKG->fastdata[kx][ky];

						if (feg->treat) ok = 1; 
			}

				if (!ok) return; 
		}

	}
	
	

	float					Xcos = 0, Ycos = 0, Zcos = 0, Xsin = 0, Ysin = 0, Zsin = 0;
	D3DTLVERTEX vert_list[4];
	long k;
	
	long					lfr, lfg, lfb;		
	float					dd, ffr, ffg, ffb;
	EERIE_3D				temporary3D;
	EERIE_CAMERA			Ncam;

	EERIE_RGB				infra;
	float					dc, p;
	float					invisibility;
	float					scale;
	EERIE_ANIM_Get_Scale_Invisibility( io, invisibility, scale );

	infra.r		=	infra.g		=	infra.b		=	0; 
	
	// Precalc rotations
	if ( angle != NULL )
	{
		if ( modinfo )
			Zsin	=	(float) radians( MAKEANGLE( angle->a + modinfo->rot.a ) );
		else
			Zsin	=	(float)radians( angle->a );

		Ncam.Xcos	=	Xcos	=	(float)EEcos( Zsin );
		Ncam.Xsin	=	Xsin	=	(float)EEsin( Zsin );

		if ( modinfo )
			Zsin	=	(float)radians( MAKEANGLE( angle->b + modinfo->rot.b ) );
		else
			Zsin	=	(float)radians( angle->b );

		Ncam.Ycos	=	Ycos	=	(float)EEcos( Zsin );
		Ncam.Ysin	=	Ysin	=	(float)EEsin( Zsin );
		
		if ( modinfo )
			Zsin	=	(float)radians( MAKEANGLE( angle->g + modinfo->rot.g ) );
		else
			Zsin	=	(float)radians( angle->g );

		Ncam.Zcos	=	Zcos	=	(float)EEcos( Zsin );
		Ncam.Zsin	=	Zsin	=	(float)EEsin( Zsin );
	}
	
	// Test for Mipmeshing then pre-computes vertices
	MIPM=0;
	vdist=0.f;
	
	ResetBBox3D(io);

	// Transforms vertex
	for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
		if ((MIPM) && (!eobj->pdata[i].need_computing)) continue;

		if ((modinfo) && !angle && BIGMAT)
		{
			vert_list[0].sx=(eobj->vertexlist[i].v.x-modinfo->link_position.x)*scale;
			vert_list[0].sy=(eobj->vertexlist[i].v.y-modinfo->link_position.y)*scale;
			vert_list[0].sz=(eobj->vertexlist[i].v.z-modinfo->link_position.z)*scale;
		}
		else if (scale != 1.f)
		{
			vert_list[0].sx=eobj->vertexlist[i].v.x*scale;
			vert_list[0].sy=eobj->vertexlist[i].v.y*scale;
			vert_list[0].sz=eobj->vertexlist[i].v.z*scale;
		}
		else
		{
			vert_list[0].sx = eobj->vertexlist[i].v.x; 
			vert_list[0].sy = eobj->vertexlist[i].v.y; 
			vert_list[0].sz = eobj->vertexlist[i].v.z; 
		}

		if (!angle)
		{
			if ((io!=NULL) && (modinfo==NULL))
			{
				
				vert_list[0].sx-=io->obj->pbox->vert[0].initpos.x*scale-io->obj->point0.x;
				vert_list[0].sy-=io->obj->pbox->vert[0].initpos.y*scale-io->obj->point0.y;
				vert_list[0].sz-=io->obj->pbox->vert[0].initpos.z*scale-io->obj->point0.z;
			}

			if (BIGQUAT==NULL)
				VertexMatrixMultiply( (EERIE_3D *)&vert_list[1], (EERIE_3D *)&vert_list[0], BIGMAT );
			else 
				TransformVertexQuat ( BIGQUAT, (EERIE_3D *)&vert_list[0], (EERIE_3D *)&vert_list[1] );

			eobj->vertexlist3[i].v.x=vert_list[1].sx+=pos.x;
			eobj->vertexlist3[i].v.y=vert_list[1].sy+=pos.y;
			eobj->vertexlist3[i].v.z=vert_list[1].sz+=pos.z;
			specialEE_RTP(&vert_list[1],&eobj->vertexlist[i].vert);
		}
		else
		{
			_YRotatePoint((EERIE_3D *)&vert_list[0],(EERIE_3D *)&vert_list[1],Ycos,Ysin);
			_XRotatePoint((EERIE_3D *)&vert_list[1],(EERIE_3D *)&vert_list[0],Xcos,Xsin);

			// Misc Optim to avoid 1 infrequent rotation around Z
			if (Zsin==0.f) 
			{
				eobj->vertexlist3[i].v.x=vert_list[0].sx+=pos.x;
				eobj->vertexlist3[i].v.y=vert_list[0].sy+=pos.y;
				eobj->vertexlist3[i].v.z=vert_list[0].sz+=pos.z;
				specialEE_RTP(&vert_list[0],&eobj->vertexlist[i].vert);
			}
			else 
			{			
				_ZRotatePoint((EERIE_3D *)&vert_list[0],(EERIE_3D *)&vert_list[1],Zcos,Zsin);
				eobj->vertexlist3[i].v.x=vert_list[1].sx+=pos.x;
				eobj->vertexlist3[i].v.y=vert_list[1].sy+=pos.y;
				eobj->vertexlist3[i].v.z=vert_list[1].sz+=pos.z;
				specialEE_RTP(&vert_list[1],&eobj->vertexlist[i].vert);
			} 
		}

		// Memorizes 2D Bounding Box using vertex min/max x,y pos
		if (eobj->vertexlist[i].vert.rhw>0.f) 
		{
			BBOXMIN.x=min(BBOXMIN.x,eobj->vertexlist[i].vert.sx);
			BBOXMAX.x=max(BBOXMAX.x,eobj->vertexlist[i].vert.sx);
			BBOXMIN.y=min(BBOXMIN.y,eobj->vertexlist[i].vert.sy);
			BBOXMAX.y=max(BBOXMAX.y,eobj->vertexlist[i].vert.sy);

		}

		AddToBBox3D(io,&eobj->vertexlist3[i].v);
	}

	if (	(BBOXMAX.x<=1)
		||	(BBOXMIN.x>=DANAESIZX-1)
		||	(BBOXMAX.y<=1) 
		||	(BBOXMIN.y>=DANAESIZY-1)		
		)
		goto finish;

	if (ARX_SCENE_PORTAL_ClipIO(io,&pos))
		return;

	// Precalc local lights for this object then interpolate
	if ((FRAME_COUNT<=0) && USEINTERNORM)
	{	
		llightsInit();

		if (Project.improve) 
		{
			if (io!=NULL) 
			{
				infra.r=io->infracolor.r;
				infra.g=io->infracolor.g;
				infra.b=io->infracolor.b;
			}
			else 
			{
				infra.r=0.6f;
				infra.g=0.f;
				infra.b=1.f;
			}
		}

		for(long i = 0; i < TOTIOPDL; i++) {
			Insertllight(IO_PDL[i], EEDistance3D(&IO_PDL[i]->pos, &pos)); 
		}

		for(long i = 0; i < TOTPDL; i++) {
			Insertllight(PDL[i], EEDistance3D(&PDL[i]->pos, &pos)); 
		}

		EERIE_3D l_pos(pos.x, pos.y - 60.f, pos.z);
		Preparellights(&l_pos);	

		for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
			if ((MIPM) && (!eobj->pdata[i].need_computing)) continue;
			
			if (BIGMAT!=NULL)
				VertexMatrixMultiply( &temporary3D, (EERIE_3D *)&eobj->vertexlist[i].norm, BIGMAT );
			else 
				_YXZRotatePoint(&eobj->vertexlist[i].norm,&temporary3D,&Ncam);
		
			if ((io) && (io->ioflags & (IO_NPC | IO_ITEM)))
			{
				ffr = ffg = ffb = NPC_ITEMS__AMBIENT_VALUE_255;
			}
			else
			{
				ffr=ACTIVEBKG->ambient255.r;
				ffg=ACTIVEBKG->ambient255.g;
				ffb=ACTIVEBKG->ambient255.b;
			}

			for (long k=0;k<MAX_LLIGHTS;k++)
			{
				EERIE_LIGHT * el=llights[k];

				if (el!=NULL)
				{
					dd=EEDistance3D(&el->pos,&eobj->vertexlist3[i].v);

					if (dd<el->fallend) 
					{
						_YXZRotateNorm(&temporary3D,&eobj->vertexlist3[i].norm,&TCAM[k]);
						dc=eobj->vertexlist3[i].norm.z;

						if (dd<=el->fallstart) 
						{
							dc *= el->precalc; 
						}
						else 
						{
							p=((el->fallend-dd)*el->falldiffmul);

							if (p<0.f) dc=0.f;
							else dc *= p * el->precalc; 
						}					

						ffr=max(ffr,el->rgb255.r*dc);
						ffg=max(ffg,el->rgb255.g*dc);
						ffb=max(ffb,el->rgb255.b*dc);
					}
				}

				if (eobj->drawflags & DRAWFLAG_HIGHLIGHT)
				{
					ffr+=100.f;
					ffg+=100.f;
					ffb+=100.f;
				}

				if (Project.improve) 
				{
					lfr = ffr*infra.r;

					lfr=clipByte255(lfr);
					lfg=ffg*infra.g;
					lfg=clipByte255(lfg);

					lfb = ffb*infra.b;
					lfb=clipByte255(lfb);
				}
				else
				{
					lfr = ffr;
					
					lfr=clipByte255(lfr);
					lfg=ffg;

					lfg=clipByte255(lfg);
					lfb = ffb;
				
					lfb=clipByte255(lfb);
				}

				eobj->vertexlist3[i].vert.color = 0xff000000L | (((lfr)&255) << 16)
				                                  | (((lfg)&255) << 8) | ((lfb)&255);
				
			}
		}
	}

		if ( MIPM )
		{
			float r1;

			for(size_t i = 0; i < eobj->vertexlist.size(); i++) {
				if (eobj->pdata[i].collapse_ratio!=0.f)
				{
					// Full collapse
					if (eobj->pdata[i].collapse_ratio==1.f)
					{
						memcpy(&eobj->vertexlist[i].vert,&eobj->vertexlist[eobj->pdata[i].actual_collapse].vert,sizeof(D3DTLVERTEX));
					}
					// Interpolated collapse
					else
					{
						r1=1.f-eobj->pdata[i].collapse_ratio;
						eobj->vertexlist[i].vert.sx=eobj->vertexlist[i].vert.sx*r1 +eobj->vertexlist[eobj->pdata[i].actual_collapse].vert.sx*eobj->pdata[i].collapse_ratio;
						eobj->vertexlist[i].vert.sy=eobj->vertexlist[i].vert.sy*r1 +eobj->vertexlist[eobj->pdata[i].actual_collapse].vert.sy*eobj->pdata[i].collapse_ratio;
						eobj->vertexlist[i].vert.sz=eobj->vertexlist[i].vert.sz*r1 +eobj->vertexlist[eobj->pdata[i].actual_collapse].vert.sz*eobj->pdata[i].collapse_ratio;
					}
				}	
			}
		}

	INTER_DRAW++;
	long lm;
	long ppooss;
	

	float ddist;
	ddist=0;
	long need_halo;
	need_halo=0;

	if ((io) && (io->halo.flags & HALO_ACTIVE))
	{
		ddist=500.f-Distance3D(pos.x,pos.y,pos.z,ACTIVECAM->pos.x,ACTIVECAM->pos.y,ACTIVECAM->pos.z);

		if (ddist>0.f) ddist=(ddist*( 1.0f / 500 ));	

		if ((io) && (io->halo.flags & HALO_ACTIVE) && (ddist>0.01f) )
			need_halo=1;
	}
	
	long		special_color_flag;
				special_color_flag	=	0;
	EERIE_RGB	special_color;
				special_color.r		=	special_color.g		=	special_color.b		=	0; 

	if (io) 
	{
		float poisonpercent=0.f;
		float trappercent=0.f;
		float secretpercent=0.f;

		if (io->ioflags & IO_NPC)
		{
			if ((DEBUG_PATHFAIL) && (!FINAL_RELEASE))
			{
				if (io->_npcdata->pathfind.listnb==-2)
					trappercent=1;

				if (io->_npcdata->pathfind.pathwait)
					poisonpercent=1;
			}

			if (io->_npcdata->poisonned>0.f)
			{
				poisonpercent=io->_npcdata->poisonned*( 1.0f / 20 );

				if (poisonpercent>1.f) poisonpercent=1.f;
			}
		}

		if ((io->ioflags & IO_ITEM) && (io->poisonous>0.f) && (io->poisonous_count!=0)) 
		{
			poisonpercent=(float)io->poisonous*( 1.0f / 20 );

			if (poisonpercent>1.f) poisonpercent=1.f;
		}

		if ((io->ioflags & IO_FIX) && (io->_fixdata->trapvalue>-1)) 
		{
			trappercent=(float)TRAP_DETECT-(float)io->_fixdata->trapvalue;

			if (trappercent>0.f)
			{
				trappercent = 0.6f + trappercent * ( 1.0f / 100 ); 

				if (trappercent<0.6f) trappercent=0.6f;

				if (trappercent>1.f) trappercent=1.f;
			}
		}

		if ((io->ioflags & IO_FIX) && (io->secretvalue>-1)) 
		{
			secretpercent=(float)TRAP_SECRET-(float)io->secretvalue;

			if (secretpercent>0.f)
			{
				secretpercent = 0.6f + secretpercent * ( 1.0f / 100 ); 

				if (secretpercent<0.6f) secretpercent=0.6f;

				if (secretpercent>1.f) secretpercent=1.f;
			}
		}

		if (poisonpercent>0.f)
		{
			special_color_flag=1;
			special_color.r = 0.f; 
			special_color.g = 1.f; 
			special_color.b = 0.f; 
		}

		if (trappercent>0.f)
		{
			special_color_flag=1;
			special_color.r=trappercent;
			special_color.g=1.f-trappercent;
			special_color.b=1.f-trappercent;
		}

		if (secretpercent>0.f)
		{
			special_color_flag=1;
			special_color.r=1.f-secretpercent;
			special_color.g=1.f-secretpercent;
			special_color.b=secretpercent;
		}

		if (io->sfx_flag & SFX_TYPE_YLSIDE_DEATH)
		{
			if (io->show==SHOW_FLAG_TELEPORTING)
			{
				float fCalc = io->sfx_time+FrameDiff;
				ARX_CHECK_ULONG(fCalc);

				io->sfx_time = ARX_CLEAN_WARN_CAST_ULONG(fCalc);

				if (io->sfx_time >= ARXTimeUL())
					io->sfx_time =  ARXTimeUL();
			}
			else
			{
				special_color_flag=1;
				float elapsed=ARXTime-io->sfx_time;

				if (elapsed>0.f)
				{
					if (elapsed<3000.f) // 5 seconds to red
					{
						float ratio=elapsed*( 1.0f / 3000 );
						special_color.r=1.f;
						special_color.g=1.f-ratio;
						special_color.b=1.f-ratio;
						AddRandomSmoke(io,1);
					}
					else if (elapsed<6000.f) // 5 seconds to White				
					{
						float ratio=(elapsed-3000.f)*( 1.0f / 3000 );
						special_color.r=1.f;
						special_color.g=ratio;
						special_color.b=ratio;
						special_color_flag=2;
						AddRandomSmoke(io,2);
					}
					else if (elapsed<8000.f) // 5 seconds to White				
					{
						float ratio=(elapsed-6000.f)*( 1.0f / 2000 );
						special_color.r=ratio;
						special_color.g=ratio;
						special_color.b=ratio;
						special_color_flag=2;
						AddRandomSmoke(io,2);
					}
					else // SFX finish
					{
						special_color_flag=0;
						io->sfx_time = 0;
						
						if (io->ioflags & IO_NPC)
						{
							MakePlayerAppearsFX(io);
							AddRandomSmoke(io,50);
							EERIE_RGB rgb;
							unsigned long color=io->_npcdata->blood_color;
							rgb.r=(float)((long)((color>>16) & 255))*( 1.0f / 255 );
							rgb.g=(float)((long)((color>>8) & 255))*( 1.0f / 255 );
							rgb.b=(float)((long)((color) & 255))*( 1.0f / 255 );
							EERIE_SPHERE sp;
							sp.origin.x=io->pos.x;
							sp.origin.y=io->pos.y;
							sp.origin.z=io->pos.z;
							sp.radius=200.f;
							long count=6;

							while (count--)
							{							
								SpawnGroundSplat(&sp,&rgb,rnd()*30.f+30.f,1);
								sp.origin.y-=rnd()*150.f;

								ARX_PARTICLES_Spawn_Splat(&sp.origin,200,io->_npcdata->blood_color);

								sp.origin.x=io->pos.x+rnd()*200.f-100.f;
								sp.origin.y=io->pos.y+rnd()*20.f-10.f;
								sp.origin.z=io->pos.z+rnd()*200.f-100.f;
								sp.radius=rnd()*100.f+100.f;							
							}

							if (io->sfx_flag & SFX_TYPE_INCINERATE)
							{
								io->sfx_flag&=~SFX_TYPE_INCINERATE;
								io->sfx_flag&=~SFX_TYPE_YLSIDE_DEATH;
								long num=ARX_SPELLS_GetSpellOn(io, SPELL_INCINERATE);

								while (num>=0)
								{
									spells[num].tolive=0;
									ARX_DAMAGES_DamageNPC(io,20,0,1,&inter.iobj[0]->pos);
									num=ARX_SPELLS_GetSpellOn(io, SPELL_INCINERATE);
								}
							}
							else
							{
								io->sfx_flag&=~SFX_TYPE_YLSIDE_DEATH;
								ARX_INTERACTIVE_DestroyIO(io);
								DESTROYED_DURING_RENDERING=io;
								return;
							}
							
						}
					}
				}
				else
				{
					ARX_CHECK_NO_ENTRY(); //assert if enter here : special_color is not defined properly.
				}
			}
		}
	}

	for(size_t i = 0; i < eobj->facelist.size(); i++) {
		if (MIPM)
		{
			for (lm=0;lm<3;lm++)				
			{
				if (eobj->pdata[eobj->facelist[i].vid[lm]].collapse_ratio==1.f)
				{
					ppooss=eobj->pdata[eobj->facelist[i].vid[lm]].actual_collapse;

					if (	((ppooss==eobj->facelist[i].vid[0]) && (lm!=0)) 
						||	((ppooss==eobj->facelist[i].vid[1]) && (lm!=1))
						||  ((ppooss==eobj->facelist[i].vid[2]) && (lm!=2)) ) goto dontdraw;
				}
			}	
		}

		long paf[3];
		paf[0]=eobj->facelist[i].vid[0];
		paf[1]=eobj->facelist[i].vid[1];
		paf[2]=eobj->facelist[i].vid[2];

		if (		(eobj->vertexlist3[paf[0]].vert.rhw<0)
				&&	(eobj->vertexlist3[paf[1]].vert.rhw<0)
				&&	(eobj->vertexlist3[paf[2]].vert.rhw<0)	)
			continue; 

		vert_list[0]=eobj->vertexlist[paf[0]].vert;
		vert_list[1]=eobj->vertexlist[paf[1]].vert;
		vert_list[2]=eobj->vertexlist[paf[2]].vert;

		if (	(vert_list[0].sz<=0.f)
			&&	(vert_list[1].sz<=0.f)
			&&	(vert_list[2].sz<=0.f) )
				continue;

	
		if (eobj->facelist[i].facetype & POLY_DOUBLESIDED)
			GRenderer->SetCulling(Renderer::CullNone);
		else
		{
			GRenderer->SetCulling(Renderer::CullCW);
		}

////////////////////////////////////////////////////////////////////////
// HALO HANDLING START
			if (need_halo)
			{
			Ncam.Xcos = 1.f;
			Ncam.Xsin = 0.f;
			Ncam.Zcos = 1.f;
			Ncam.Zsin = 0.f;
				float power=radians(MAKEANGLE(subj.angle.b));
			Ncam.Ycos = (float)EEcos(power);
			Ncam.Ysin = (float)EEsin(power);
				float tot=0;
				float _ffr[3];
				
				D3DTLVERTEX * workon=vert_list;

				for (long o=0;o<3;o++)
				{
					if (BIGMAT!=NULL)
						VertexMatrixMultiply(&temporary3D, (EERIE_3D *)&eobj->vertexlist[paf[o]].norm, BIGMAT );
					else _YXZRotatePoint(&eobj->vertexlist[paf[o]].norm,&temporary3D,&Ncam);
		
					power=255.f-(float)(255.f*(temporary3D.z));

					if (power>255.f) power=255.f;
					else if (power<0.f) power=0.f;

					tot+=power;
					_ffr[o]=power;
					lfr = io->halo.color.r * power;
					lfg = io->halo.color.g * power;
					lfb = io->halo.color.b * power;
					workon[o].color = 0xFF000000L | (((lfr)&255) << 16)
					                  | (((lfg)&255) << 8) | ((lfb)&255);
				}

				if ((tot>250.f) )
				{
					long first;
					long second;
					long third;

					if ( (_ffr[0]>=_ffr[1]) && (_ffr[1]>=_ffr[2]))
				{
					first = 0;
					second = 1;
					third = 2;
				}
					else if ( (_ffr[0]>=_ffr[2]) && (_ffr[2]>=_ffr[1]))
				{
					first = 0;
					second = 2;
					third = 1;
				}
					else if ( (_ffr[1]>=_ffr[0]) && (_ffr[0]>=_ffr[2]))
				{
					first = 1;
					second = 0;
					third = 2;
				}
					else if ( (_ffr[1]>=_ffr[2]) && (_ffr[2]>=_ffr[0]))
				{
					first = 1;
					second = 2;
					third = 0;
				}
					else if ( (_ffr[2]>=_ffr[0]) && (_ffr[0]>=_ffr[1]))
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
						EERIE_3D vect1,vect2;
						D3DTLVERTEX * vert=&LATERDRAWHALO[(HALOCUR<<2)];
						
						HALOCUR++;

						if (HALOCUR>=HALOMAX) HALOCUR--;

						memcpy(&vert[0],&workon[first],sizeof(D3DTLVERTEX));
						memcpy(&vert[1],&workon[first],sizeof(D3DTLVERTEX));
						memcpy(&vert[2],&workon[second],sizeof(D3DTLVERTEX));
						memcpy(&vert[3],&workon[second],sizeof(D3DTLVERTEX));						
						float siz=ddist*(io->halo.radius*(EEsin((float)(FrameTime+i)*( 1.0f / 100 ))*( 1.0f / 50 )+0.7f));
						vect1.x=workon[first].sx-workon[third].sx;
						vect1.y=workon[first].sy-workon[third].sy;						
						float len1=1.f/EEsqrt(vect1.x*vect1.x+vect1.y*vect1.y);
						vect1.x*=len1;
						vect1.y*=len1;
						vect2.x=workon[second].sx-workon[third].sx;
						vect2.y=workon[second].sy-workon[third].sy;
						float len2=1.f/EEsqrt(vect2.x*vect2.x+vect2.y*vect2.y);
						vect2.x*=len2;
						vect2.y*=len2;
						vert[1].sx+=(vect1.x+0.2f-rnd()*0.1f)*siz;
						vert[1].sy+=(vect1.y+0.2f-rnd()*0.1f)*siz;
						vert[1].color=0xFF000000;

						float valll = 0.005f + (EEfabs(workon[first].sz-workon[third].sz) + EEfabs(workon[second].sz - workon[third].sz));
						vert[1].sz+=valll;
						vert[2].sz+=valll;
						
						vert[2].sx += (vect2.x + 0.2f - rnd() * 0.1f) * siz; 
						vert[2].sy += (vect2.y + 0.2f - rnd() * 0.1f) * siz; 

						if (io->halo.flags & HALO_NEGATIVE) vert[2].color=0x00000000;					
						else vert[2].color=0xFF000000;
					}
				}
			}

		vert_list[0].tu=eobj->facelist[i].u[0];
		vert_list[0].tv=eobj->facelist[i].v[0];
		vert_list[1].tu=eobj->facelist[i].u[1];
		vert_list[1].tv=eobj->facelist[i].v[1];
		vert_list[2].tu=eobj->facelist[i].u[2];
		vert_list[2].tv=eobj->facelist[i].v[2];	
	
	if (FRAME_COUNT<=0)
	{
		if (io)
		{
			// Frozen status override all other colors
			if (io->ioflags & IO_FREEZESCRIPT)
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color=0xFF0000FF;
			}
			// Is it a Glowing Poly ?
			else if (eobj->facelist[i].facetype & POLY_GLOW)
				vert_list[0].color=vert_list[1].color=vert_list[2].color=0xffffffff;
			// Are we using Normal Illuminations ?
			else if (USEINTERNORM) 
			{
				for (long j=0;j<3;j++)
				{			
						vert_list[j].color = eobj->vertexlist3[paf[j]].vert.color; 
				}					
			}
			// Are we using IMPROVED VISION view ?
			else if (Project.improve) 
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color
					=EERIERGB(io->infracolor.r,io->infracolor.g,io->infracolor.b);
			}
			else 
			{
					vert_list[0].color = vert_list[1].color = vert_list[2].color = 0xFFFFFFFF; 
			}
		}
		else
		{
			// Treating GLOWING POLY (unaffected by light)
			if (eobj->facelist[i].facetype & POLY_GLOW)
				vert_list[0].color=vert_list[1].color=vert_list[2].color=0xffffffff;
				else if (USEINTERNORM) // using INTERNORM lighting
			{
				for (long j=0;j<3;j++)
				{			
						vert_list[j].color = eobj->vertexlist3[paf[j]].vert.color; 
				}					
			}
			else if (Project.improve) // using IMPROVED VISION view
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color=EERIERGB(0.6f,0.f,1.f);
			}
			else // using default white
			{
				vert_list[0].color=vert_list[1].color=vert_list[2].color=EERIECOLOR_WHITE;
			}
		}

		if(special_color_flag & 1) {
			for(long j = 0; j < 3; j++) {
				long r = (long)((float)((long)((vert_list[j].color >> 16) & 255)) * special_color.r);
				long g = (long)((float)((long)((vert_list[j].color >> 8) & 255)) * special_color.g);
				long b = (long)((float)((long) (vert_list[j].color & 255))*(special_color.b));
				vert_list[j].color = 0xFF000000L | ((r&255) << 16) | ((g&255) << 8) | (b&255);
			}
		}
	}

		// Treat WATER Polys (modify UVs)
		if (eobj->facelist[i].facetype & POLY_WATER)
		{
			for (k=0;k<3;k++) 
			{ 
				vert_list[k].tu=eobj->facelist[i].u[k];
				vert_list[k].tv=eobj->facelist[i].v[k];
				ApplyWaterFXToVertex((EERIE_3D *)&eobj->vertexlist[eobj->facelist[i].vid[k]].v,&vert_list[k],0.3f);
			}
		}

		// Transparent poly: storing info to draw later
		if ((eobj->facelist[i].facetype & POLY_TRANS) 
			|| (invisibility>0.f))
		{
			if (eobj->facelist[i].texid>=0)
			{
				memcpy(&InterTransPol[INTERTRANSPOLYSPOS],&vert_list,sizeof(D3DTLVERTEX)*3);
				InterTransFace[INTERTRANSPOLYSPOS]=&eobj->facelist[i];

				if (invisibility>0.f)
					InterTransFace[INTERTRANSPOLYSPOS]->transval=2.f-invisibility;

				InterTransTC[INTERTRANSPOLYSPOS]=eobj->texturecontainer[eobj->facelist[i].texid];
				INTERTRANSPOLYSPOS++;

				if (INTERTRANSPOLYSPOS>=MAX_INTERTRANSPOL) INTERTRANSPOLYSPOS=MAX_INTERTRANSPOL-1;
			}

			continue;
		}
			
		// Set Texture for Poly
		if ((eobj->facelist[i].texid>=0) 
			&& (eobj->texturecontainer[eobj->facelist[i].texid]!=NULL))
				GRenderer->SetTexture(0, eobj->texturecontainer[eobj->facelist[i].texid]);
		else  // Set to Flat if invalid
				GRenderer->ResetTexture(0);

		if (FRAME_COUNT!=0)
		for (long j=0;j<3;j++)		
			vert_list[j].color=eobj->facelist[i].color[j];		
		else 
		for (long j=0;j<3;j++)		
			eobj->facelist[i].color[j]=vert_list[j].color;		

		// Finally render our primitive			
		EERIEDRAWPRIM(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX,&vert_list, 3,  0, 0 );
			

		// Add some fake specular to Metallic polys
		if (	( eobj->facelist[i].facetype & POLY_METAL )
			|| ( ( eobj->texturecontainer[eobj->facelist[i].texid] ) && ( eobj->texturecontainer[eobj->facelist[i].texid]->userflags & POLY_METAL ) ) )
		{				
			long r = 0, g = 0, b = 0;
				long todo	=	0;

				for ( long j = 0 ; j < 3 ; j++ )
				{ 
					D3DTLVERTEX * tl = &vert_list[j];

					r = ( tl->color >> 16 ) & 255;
					g = ( tl->color >> 8 ) & 255;
					b = tl->color  & 255;
						
					if ( r > 192 || g > 192 || b > 192 )
					{
						todo++;
					}

					r -=192;

					if ( r < 0.f ) r = 0;

					g -= 192;

					if ( g < 0.f ) g = 0;

					b -= 192;

					if ( b < 0.f ) b = 0;

					tl->color = 0xFF000000 | ( r << 18 ) | ( g << 10 ) | ( b << 2 );
				}

				if ( todo )
				{
					if ( ( todo > 2 ) && ( rnd() > 0.997f ) )
					{
						if ( io )
							SpawnMetalShine( (EERIE_3D *)&eobj->vertexlist3[eobj->facelist[i].vid[0]].vert, r, g, b, GetInterNum( io ) );
					}

					GRenderer->SetBlendFunc(Renderer::BlendDstColor, Renderer::BlendOne);	
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);			
					GRenderer->SetRenderState(Renderer::DepthWrite, false);
					EERIEDRAWPRIM( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, &vert_list, 3, 0, 0 );
					GRenderer->SetRenderState(Renderer::AlphaBlending, false);			
					GRenderer->SetRenderState(Renderer::DepthWrite, true);
				}				
	}

	dontdraw:
		;
	}	

	// storing 2D Bounding Box info
finish:
	;

	if (io) 
	{
		io->bbox1.x=(short)BBOXMIN.x;
		io->bbox2.x=(short)BBOXMAX.x;
		io->bbox1.y=(short)BBOXMIN.y;
		io->bbox2.y=(short)BBOXMAX.y;
	}
}

//*************************************************************************************
//*************************************************************************************
void ResetAnim(ANIM_USE * eanim)
{
	if (eanim==NULL) return;

	eanim->ctime=0;
	eanim->lastframe=-1;
	eanim->flags&=~EA_PAUSED;
	eanim->flags&=~EA_ANIMEND;
	eanim->flags&=~EA_LOOP;
	eanim->flags&=~EA_FORCEPLAY;
}
long LAST_LLIGHT_COUNT=0;
//*************************************************************************************
//*************************************************************************************
void llightsInit()
{
	for (long i = 0; i < MAX_LLIGHTS; i++)
	{
		llights[i]=NULL;
		dists[i]=999999999.f;
		values[i]=999999999.f;
	}	

	LAST_LLIGHT_COUNT=0;
}
//*************************************************************************************
// Inserts Light in the List of Nearest Lights
//*************************************************************************************
void Insertllight(EERIE_LIGHT * el,float dist)
{	
	if (el==NULL) return;

	float threshold=el->fallend+560.f;
	if (dist > threshold) return; 

	{
		float val = dist - el->fallend; 

		if (val<0) val=0;

		for (long i=0;i<MAX_LLIGHTS;i++) 
		{			
			if (llights[i]==NULL)
			{
				llights[i]=el;
				dists[i]=dist;
				values[i]=val;
				LAST_LLIGHT_COUNT++;
				return;
			}
			else if (val <= values[i])  // Inserts light at the right place
			{				
				for (long j=MAX_LLIGHTS-2;j>=i;j--)
				{
					if (llights[j])
					{
						llights[j+1]=llights[j];
						dists[j+1]=dists[j];
						values[j+1]=values[j];
					}
				}

				llights[i]=el;
				dists[i]=dist;
				values[i]=val;
				LAST_LLIGHT_COUNT++;
				return;
			}
		}
	}
}

//*************************************************************************************
// Precalcs some misc things for lights
//*************************************************************************************
void Preparellights(EERIE_3D * pos)
{
	for (long i=0;i<MAX_LLIGHTS;i++) 
	{
		EERIE_LIGHT * el=llights[i];

		if (el)
		{
			TCAM[i].pos.x=el->pos.x;			
			TCAM[i].pos.y=el->pos.y;
			TCAM[i].pos.z=el->pos.z;
			SetTargetCamera(&TCAM[i],pos->x,pos->y,pos->z);
			F_PrepareCamera(&TCAM[i]);
			
		}		 
	}
}

void EERIE_ANIMMANAGER_Clear(long i)
{
	for (long k=0;k<animations[i].alt_nb;k++)
	{
		ReleaseAnim(animations[i].anims[k]);
		animations[i].anims[k]=NULL;					
	}

	if (animations[i].anims)
		free(animations[i].anims);

	animations[i].anims=NULL;

	if (animations[i].sizes)
		free(animations[i].sizes);

	animations[i].sizes=NULL;
	animations[i].path[0]=0;
}
//-----------------------------------------------------------------------------
void EERIE_ANIMMANAGER_ClearAll()
{
	for (long i=0;i<MAX_ANIMATIONS;i++)
	{
		if (animations[i].path[0]!=0)
		{
			EERIE_ANIMMANAGER_Clear(i);			
		}
	}

	if (grps)
	{
		free (grps);
		grps = NULL;
	}

}
void EERIE_ANIMMANAGER_ReloadAll()
{
	for (long i=0;i<inter.nbmax;i++)
	{
		if (inter.iobj[i])
		{
			INTERACTIVE_OBJ * io=inter.iobj[i];

			for (long j=0;j<MAX_ANIMS;j++)
			{
				EERIE_ANIMMANAGER_ReleaseHandle(io->anims[j]);
				io->anims[j]=NULL;
			}

			for (long count=0;count<MAX_ANIM_LAYERS;count++)
			{
				memset(&io->animlayer[count],0,sizeof(ANIM_USE));
				io->animlayer[count].cur_anim=NULL;
				io->animlayer[count].next_anim=NULL;
			}
		}
	}
		
	for ( int i=0;i<MAX_ANIMATIONS;i++)
	{
		if (animations[i].path[0]!=0)
		{
			char pathhh[256];
			strcpy(pathhh,animations[i].path);
			EERIE_ANIMMANAGER_Clear(i);
			EERIE_ANIMMANAGER_Load(pathhh);			
		}
	}
}
