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
// ARX_Paths
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX Paths Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////
#ifndef ARX_PATHS_H
#define ARX_PATHS_H

#include "EERIETypes.h"
#include "EERIEPoly.h"

class CRuban;

//-----------------------------------------------------------------------------
typedef struct
{
	EERIE_3D		rpos; //relative pos
	long			flag;
	float			_time;
} ARX_PATHWAY;

typedef struct
{
	char			name[64];
	short			idx;
	long			flags;
	EERIE_3D		initpos;
	EERIE_3D		pos;
	long			nb_pathways;
	ARX_PATHWAY	*	pathways;
	long			height; // 0 NOT A ZONE
	char 			controled[64];

	char			ambiance[128];
	EERIE_RGB		rgb;
	float			farclip;
	float			reverb;
	float			amb_max_vol;
	EERIE_3D		bbmin;
	EERIE_3D		bbmax;
} ARX_PATH;

typedef struct
{
	ARX_PATH	*	path;
	float			_starttime;
	float			_curtime;
	long			aupflags;
	EERIE_3D		initpos;
	long			lastWP;
} ARX_USE_PATH;

typedef struct
{
	long				exist; // 2== want to change to want_vars...
	INTERACTIVE_OBJ *	io;
	ARX_USE_PATH	*	aup;
	EERIE_CAMERA	*	cam;
	INTERACTIVE_OBJ *	want_io;
	ARX_USE_PATH	*	want_aup;
	EERIE_CAMERA	*	want_cam;
} MASTER_CAMERA_STRUCT;

//-----------------------------------------------------------------------------
#define PATHWAY_STANDARD				0
#define PATHWAY_BEZIER					1
#define PATHWAY_BEZIER_CONTROLPOINT		2

#define ARX_PATH_MOD_NONE		0
#define ARX_PATH_MOD_POSITION	1
#define ARX_PATH_MOD_FLAGS		2
#define ARX_PATH_MOD_TIME		4
#define ARX_PATH_MOD_TRANSLATE	8
#define ARX_PATH_MOD_ALL		ARX_PATH_MOD_POSITION | ARX_PATH_MOD_FLAGS | ARX_PATH_MOD_TIME
#define ARX_PATH_HIERARCHY		16

// ARX_PATH@flags values
#define PATH_LOOP		1
#define PATH_AMBIANCE	2
#define PATH_RGB		4
#define PATH_FARCLIP	8
#define PATH_REVERB		16

#define ARX_USEPATH_FLAG_FINISHED		1
#define ARX_USEPATH_WORM_SPECIFIC		2
#define ARX_USEPATH_FOLLOW_DIRECTION	4
#define ARX_USEPATH_FORWARD				8
#define ARX_USEPATH_BACKWARD			16
#define ARX_USEPATH_PAUSE				32
#define	ARX_USEPATH_FLAG_ADDSTARTPOS	64

//-----------------------------------------------------------------------------
extern MASTER_CAMERA_STRUCT MasterCamera;
extern ARX_USE_PATH USE_CINEMATICS_PATH;
extern ARX_PATH ** ARXpaths;
extern ARX_PATH * ARX_PATHS_FlyingOverAP;
extern ARX_PATH * ARX_PATHS_SelectedAP;
extern long	ARX_PATHS_SelectedNum;
extern long ARX_PATHS_HIERARCHYMOVE;
extern long USE_CINEMATICS_CAMERA;
extern long	nbARXpaths;
extern long	ARX_PATHS_FlyingOverNum;

//-----------------------------------------------------------------------------
void ARX_PATH_UpdateAllZoneInOutInside();
long ARX_PATH_IsPosInZone(ARX_PATH * ap, float x, float y, float z);
void ARX_PATH_ClearAllUsePath();
void ARX_PATH_ReleaseAllPath();
ARX_PATH * ARX_PATH_GetAddressByName(char * name);
void ARX_PATH_ClearAllControled();
void ARX_PATH_ComputeAllBoundingBoxes();

void ARX_PATHS_ChangeName(ARX_PATH * ap, char * newname);
ARX_PATH * ARX_PATHS_ExistName(char * name);
void ARX_PATHS_Delete(ARX_PATH * ap);
void ARX_PATHS_RedrawAll(LPDIRECT3DDEVICE7 pd3dDevice);
ARX_PATH * ARX_PATHS_Create(char * name, EERIE_3D * pos);
ARX_PATH * ARX_PATHS_AddNew(EERIE_3D * pos);
void ARX_PATHS_Delete(ARX_PATH * ap);
long ARX_PATHS_AddPathWay(ARX_PATH * ap, long insert);
void ARX_PATHS_ModifyPathWay(ARX_PATH * ap, long num, long mods, EERIE_3D * pos, long flags, unsigned long time);
void ARX_PATHS_DeletePathWay(ARX_PATH * ap, long del);
void ARX_PATHS_DrawPath(ARX_PATH * ap, LPDIRECT3DDEVICE7 pd3dDevice);
long ARX_PATHS_Interpolate(ARX_USE_PATH * aup, EERIE_3D * pos);


//////////////////////////////////////////////////////////////////////
#define ATO_EXIST		1
#define ATO_MOVING		2
#define ATO_UNDERWATER	4
#define ATO_FIERY		8

#define ATO_TYPE_ARROW	1

typedef struct
{
	long		flags;
	EERIE_3D	vector;
	EERIE_3D	upvect;
	EERIE_QUAT	quat;
	EERIE_3D	initial_position;
	float		velocity;
	EERIE_3D	position;
	float		damages;
	EERIE_3DOBJ *	obj;
	long		source;
	unsigned long creation_time;
	float		poisonous;
	CRuban	*	pRuban;

} ARX_THROWN_OBJECT;

#define MAX_THROWN_OBJECTS	100

extern ARX_THROWN_OBJECT Thrown[MAX_THROWN_OBJECTS];
extern long Thrown_Count;

class CRuban
{
	private:
		short		key;
		int			duration;
		int			currduration;
		int			num;
		int			iNumThrow;

		typedef struct ST_RUBAN
		{
			int				actif;
			EERIE_3D		pos;
			int				next;
		} T_RUBAN;
		T_RUBAN truban[2048];

		typedef struct
		{
			int		first;
			int		origin;
			float	size;
			int		dec;
			float	r, g, b;
			float	r2, g2, b2;
		} T_RUBAN_DEF;

		int			nbrubandef;
		T_RUBAN_DEF trubandef[256];

		int GetFreeRuban(void);
		void AddRuban(int * f, int id, int dec);
		void DrawRuban(LPDIRECT3DDEVICE7 device, int num, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
	public:
		CRuban() {};
		void	AddRubanDef(int origin, float size, int dec, float r, float g, float b, float r2, float g2, float b2);
		void	Create(int numinteractive, int duration);
		void	Update(unsigned long);
		float	Render(LPDIRECT3DDEVICE7);
 
};


long ARX_THROWN_OBJECT_GetFree();
long ARX_THROWN_OBJECT_Throw(long type, long source, EERIE_3D * position, EERIE_3D * vect, EERIE_3D * upvect, EERIE_QUAT * quat, float velocity, float damages, float poisonous);
void ARX_THROWN_OBJECT_KillAll();
void ARX_THROWN_OBJECT_Manage(unsigned long time_offset);
void EERIE_PHYSICS_BOX_Launch_NOCOL(INTERACTIVE_OBJ * io, EERIE_3DOBJ * obj, EERIE_3D * pos, EERIE_3D * vect, long flags = 0, EERIE_3D * angle = NULL);


#endif
