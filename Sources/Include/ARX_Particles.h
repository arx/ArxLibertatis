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
// ARX_Particles
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Particles Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_PARTICLES_H
#define ARX_PARTICLES_H


#include <EERIETypes.h>
#include <EERIEPoly.h>
#include <tchar.h>

//-----------------------------------------------------------------------------
typedef struct
{
	unsigned char	exist;
	char			type;
	short			flags;
	D3DTLVERTEX		v;
	D3DTLVERTEX		tv;
	float			x;
	float			y;
	float			tolive;
	float			r;
	float			g;
	float			b;
	float			size;
	long			dynlight;
	long			move;
	INTERACTIVE_OBJ * io;
	BOOL		bDrawBitmap;
} FLARES;

//-----------------------------------------------------------------------------
typedef struct
{
	EERIE_3D		pos;
	long			exist;
} BOOM;

//-----------------------------------------------------------------------------
typedef struct
{
	long				exist;
	short				tx;
	short				tz;
	EERIEPOLY 	*		ep;
	float				u[4];
	float				v[4];
	EERIE_RGB			rgb;
	TextureContainer *	tc;
	unsigned long		timecreation;
	unsigned long		tolive;
	short				type;
	short				nbvert;
} POLYBOOM;

//-----------------------------------------------------------------------------
typedef struct
{
	BOOL		exist;
	long		type;
	EERIE_3D	ov;
	EERIE_3D	move;
	EERIE_3D	scale;
	EERIE_3D	oldpos;
	float		siz;
	long		zdec;
	long		timcreation;
	unsigned long	tolive;
	unsigned long	delay;
	TextureContainer * tc;
	float		r;
	float		g;
	float		b;
	long		special;
	float		fparam;
	long		mask;
	EERIE_3D  * source;
	long		sourceionum;
	short		sval;
	char		cval1;
	char		cval2;
} PARTICLE_DEF;

//-----------------------------------------------------------------------------
typedef struct
{
	BOOL		exist;
	long		selected;
	EERIE_3D	pos;
	EERIE_RGB	rgb;
	float		size;
	long		special;
	float		scale;
	EERIE_3D	move;
	EERIE_3D	angle;
	float		speed;
	float		rotatespeed;
	long		tolive;
	EERIE_3D	bboxmin;
	EERIE_3D	bboxmax;
	long		blend;
	float		frequency;
	unsigned long lastupdate;
} FOG_DEF;

//-----------------------------------------------------------------------------
typedef struct
{
	TextureContainer * lumignon;
	TextureContainer * lumignon2;
	TextureContainer * plasm;
	TextureContainer * shine[11];
} FLARETC;

//-----------------------------------------------------------------------------
#define MAX_FOG 100
#define FOG_DIRECTIONAL 1
#define MAX_POLYBOOM 4000
#define MAX_BOOMS 20
#define PARTICLE_2D	256
#define MAX_FLARES 300
#define MAX_FLARELIFE 4000
#define FLARE_MUL 2

const unsigned long MAX_PARTICLES(2200);

enum ARX_PARTICLES_TYPE_FLAG
{
	FIRE_TO_SMOKE       = 0x00000001,
	ROTATING            = 0x00000002,
	FADE_IN_AND_OUT     = 0x00000004,
	MODULATE_ROTATION   = 0x00000008,
	DISSIPATING         = 0x00000010,
	GRAVITY             = 0x00000020,
	SUBSTRACT           = 0x00000040,
	FIRE_TO_SMOKE2      = 0x00000080,
	PARTICLE_ETINCELLE  = 0x00000100, //gère l'acceleration et la gravité
	FOLLOW_SOURCE       = 0x00000200,
	FOLLOW_SOURCE2      = 0x00000400,
	DELAY_FOLLOW_SOURCE = 0x00000800,
	NO_TRANS            = 0x00001000,
	PARTICLE_ANIMATED   = 0x00002000,
	PARTICLE_SPARK		= 0x00004000,
	SPLAT_GROUND		= 0x00008000,
	SPLAT_WATER			= 0x00010000,
	PARTICLE_SUB2		= 0x00020000,
	PARTICLE_GOLDRAIN	= 0x00040000,
	PARTICLE_NOZBUFFER	= 0x80000000
};

//-----------------------------------------------------------------------------
#define BOOM_RADIUS 420.f
#define BOOM_RADIUS2 250.f
#define MAX_OBJFX			30
#define SPECIAL_RAYZ		1
#define FLARELINESTEP		7
#define FLARELINERND		6
#define MAX_EXPLO			24 

//-----------------------------------------------------------------------------
extern TextureContainer * explo[MAX_EXPLO];
extern TextureContainer * blood_splat;
extern FLARES flare[MAX_FLARES];
extern long flarenum;
extern short OPIPOrgb;
extern short PIPOrgb;
extern long currboom;
extern PARTICLE_DEF particle[MAX_PARTICLES];
extern BOOM booms[MAX_BOOMS];
extern long BoomCount;
extern POLYBOOM polyboom[MAX_POLYBOOM];
extern FOG_DEF fogs[MAX_FOG];
extern TextureContainer * smokeparticle;
extern TextureContainer * bloodsplatter;
extern TextureContainer * healing;
extern TextureContainer * tzupouf;
extern TextureContainer * fire2;
extern long NewSpell;
extern FLARETC flaretc;
extern long ParticleCount;

//-----------------------------------------------------------------------------
void MagFX(float posx, float posy, float posz);
void RestoreAllLightsInitialStatus();
void TreatBackgroundActions();
void TreatBackgroundDynlights();
void MakeBookFX(float posx, float posy, float posz);
void UpdateObjFx(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_CAMERA * cam) ;
void Add3DBoom(EERIE_3D * position, EERIE_3DOBJ * obj = NULL);
void AddRandomSmoke(INTERACTIVE_OBJ * io, long amount = 1);
void AddFlare(EERIE_S2D * pos, float sm, short typ, INTERACTIVE_OBJ * io = NULL);
void AddFlare2(EERIE_S2D * pos, float sm, short typ, INTERACTIVE_OBJ * io);
void AddLFlare(float x, float y, INTERACTIVE_OBJ * io = NULL) ;
void FlareLine(EERIE_S2D * pos0, EERIE_S2D * pos1, INTERACTIVE_OBJ * io = NULL);
void LaunchDummyParticle();
void ManageTorch();

void ARX_GenereSpheriqueEtincelles(EERIE_3D * pos, float r, TextureContainer * tc, float rr, float g, float b, int mask);
void MakePlayerAppearsFX(INTERACTIVE_OBJ * io);
void MakeCoolFx(EERIE_3D * pos);
void SpawnGroundSplat(EERIE_SPHERE * sp, EERIE_RGB * rgb, float size, long flags);

void ARX_FOGS_FirstInit();
long ARX_FOGS_Count();
void ARX_FOGS_Clear();
void ARX_FOGS_TranslateSelected(EERIE_3D * trans);
void ARX_FOGS_UnselectAll();
void ARX_FOGS_Select(long n);
long ARX_FOGS_GetFree();
void ARX_FOGS_KillSelected();
void ARX_FOGS_Render(long init);

void ARX_PARTICLES_FirstInit();
void ARX_PARTICLES_ClearAll();
long ARX_PARTICLES_GetFree();
void ARX_PARTICLES_Render(LPDIRECT3DDEVICE7 pd3dDevice, EERIE_CAMERA * cam);
void ARX_PARTICLES_Spawn_Blood(EERIE_3D * pos, EERIE_3D * vect, float dmgs, long source);
void ARX_PARTICLES_Spawn_Blood2(EERIE_3D * pos, float dmgs, D3DCOLOR col, long vert, INTERACTIVE_OBJ * io);
void ARX_PARTICLES_Spawn_Lava_Burn(EERIE_3D * pos, float power, INTERACTIVE_OBJ * io = NULL);
void ARX_PARTICLES_Add_Smoke(EERIE_3D * pos, long flags, long amount, EERIE_RGB * rgb = NULL); // flag 1 = randomize pos
void ARX_PARTICLES_Spawn_Spark(EERIE_3D * pos, float dmgs, long flags);
void ARX_PARTICLES_Spawn_Splat(EERIE_3D * pos, float dmgs, D3DCOLOR col, long vert, INTERACTIVE_OBJ * io, long flags = 1);
void ARX_PARTICLES_SpawnWaterSplash(EERIE_3D *);

void ARX_BOOMS_ClearAllPolyBooms();
void ARX_BOOMS_ClearAll();
void ARX_BOOMS_Add(EERIE_3D * pos, long type = 0);
 

void ARX_MAGICAL_FLARES_FirstInit();
void ARX_MAGICAL_FLARES_KillAll();
void ARX_MAGICAL_FLARES_Draw(LPDIRECT3DDEVICE7 pd3dDevice, long FRAMETICKS);

void LaunchFireballBoom(EERIE_3D * poss, float level, EERIE_3D * direction = NULL, EERIE_RGB * rgb = NULL);
void SpawnFireballTail(EERIE_3D *, EERIE_3D *, float, long);
void SpawnMetalShine(EERIE_3D * pos, long r, long g, long b, long num);

#endif
