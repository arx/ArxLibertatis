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
// DanaeSaveLoad.H
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		DANAE Save & Load Management
//
// Updates: (date) (person) (update)
//
// Code: Cyril Meynier
//
// Copyright (c) 1999-2000 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef DANAESAVELOAD_H
#define DANAESAVELOAD_H
#include "danae.h"

//Fileformat version
#define CURRENT_VERSION 1.44f

//////////////////////
//Lighting File Header
typedef struct
{
	float	version;
	char	ident[16];
	char	lastuser[256];
	long	time;

	long	nb_lights;
	long	nb_Shadow_Polys;
	long	nb_IGNORED_Polys;
	long	nb_bkgpolys;
	long	pad[256];
	float	fpad[256];
	char	cpad[4096];
	BOOL	bpad[256];
} DANAE_LLF_HEADER; //Aligned 1 2 4

///////////////////////
EERIE_3DOBJ * _LoadTheObj(char * text, char * path);
void SaveIOScript(INTERACTIVE_OBJ * io, long fl);
void LogDirCreation(char * dir);
typedef struct
{
	float	version;
	char	ident[16];
	char	lastuser[256];
	long	time;
	EERIE_3D pos_edit;
	EERIE_3D angle_edit;
	long	nb_scn;
	long	nb_inter;
	long	nb_nodes;
	long	nb_nodeslinks;
	long	nb_zones;
	BOOL	lighting;
	BOOL    Bpad[256];
	long	nb_lights; 
	long	nb_fogs; 

	long	nb_bkgpolys;		
	long	nb_ignoredpolys;	
	long	nb_childpolys;		
	long	nb_paths;			
	long	pad[250];
	EERIE_3D	offset;
	float	fpad[253];
	char	cpad[4096];
	BOOL	bpad[256];
} DANAE_LS_HEADER; // Aligned 1 2 4

#define	SP_IGNORED	1
#define	SP_CHILD	2

typedef struct
{
	char	name[512];
	long	pad[16];
	float	fpad[16];
} DANAE_LS_SCENE; // Aligned 1 2 4 8
void WriteIOInfo(INTERACTIVE_OBJ * io, char * dir);
typedef struct
{
	char		name[512];
	EERIE_3D	pos;
	EERIE_3D	angle;
	long		ident; 
	long		flags; 
	long	pad[14];
	float	fpad[16];
} DANAE_LS_INTER; // Aligned 1 2 4

typedef struct
{
	char		name[64];
	EERIE_3D	pos;
	long		pad[16];
	float		fpad[16];
} DANAE_LS_NODE; 
// Aligned 1 2 4

typedef struct
{
	long		nb_values;
	long		ViewMode;
	long		ModeLight;
	long		pad;
} DANAE_LS_LIGHTINGHEADER; // Aligned 1 2 4 8

typedef struct
{
	long		r;
	long		g;
	long		b;
} DANAE_LS_VLIGHTING; // Aligned 1 2 4

typedef struct
{
	EERIE_3D	pos;
	EERIE_RGB	rgb;
	float		fallstart;
	float		fallend;
	float		intensity;
	float		i;
	EERIE_RGB	ex_flicker;
	float		ex_radius;
	float		ex_frequency;
	float		ex_size;
	float		ex_speed;
	float		ex_flaresize;
	float		fpadd[24];
	long		extras;
	long		lpadd[31];
} DANAE_LS_LIGHT; //ver 1.003f // Aligned 1 2 4

typedef struct
{
	char		name[64];
	short		idx;
	short		flags;
	EERIE_3D	initpos;
	EERIE_3D	pos;
	long		nb_pathways;
	EERIE_RGB	rgb; 
	float		farclip; 
	float		reverb;
	float		amb_max_vol; 
	float		fpadd[26];
	long		height;
	long		lpadd[31];
	char		ambiance[128]; 
	char		cpadd[128];
} DANAE_LS_PATH; // Aligned 1 2 4

typedef struct
{
	EERIE_3D		rpos;
	long			flag;
	unsigned long	time;
	float		fpadd[2];
	long		lpadd[2];
	char		cpadd[32];
} DANAE_LS_PATHWAYS; // Aligned 1 2 4

typedef struct
{
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
	long		blend;
	float		frequency;
	float		fpadd[32];
	long		lpadd[32];
	char		cpadd[256];
} DANAE_LS_FOG; // Aligned 1 2 4

extern EERIE_3D loddpos;
long DanaeSaveLevel(char * fic);
long DanaeLoadLevel(LPDIRECT3DDEVICE7 pd3dDevice, char * fic);
void DanaeClearLevel(long flags = 0);
void DanaeClearAll();
void RestoreLastLoadedLightning();
void LogDirDestruction(char * dir);

void CheckIO_NOT_SAVED();
INTERACTIVE_OBJ * LoadInter_Ex(DANAE_LS_INTER * dli, EERIE_3D * trans);
extern EERIE_3D MSP;
void BIG_PURGE();
void CheckIO_NOT_SAVED();
 
extern long		DanaeSaveLevel(char * fic);
extern long		DanaeLoadLevel(LPDIRECT3DDEVICE7 pd3dDevice, char * fic);

void ARX_SAVELOAD_CheckDLFs();
#endif
