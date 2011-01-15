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
// ARX_FTL
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		ARX FTL file loading and saving
//		FTL files contains Optimised/Pre-computed versions of objects for faster loads
//
// Updates:	(date)		(person)	(update)
//			2001/10/14	Cyril		Some Verifications
//			2001/10/14	Cyril		Now FTL Allocsize is REALLY computed...
//
// Initial Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "ARX_FTL.h"
#include "HERMESMain.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

//***********************************************************************************************
//	FTL FILE Structure:
//***********************************************************************************************
//		ARX_FTL_PRIMARY_HEADER
//		Checksum
//		-->All the following data is then compressed and must be expanded
//		ARX_FTL_SECONDARY_HEADER;
//		-> Then depending on offsets just read data directly.
//***********************************************************************************************
// WARNING: FTL Files uses directly some DANAE internal file formats...
// beware of dependencies...
//***********************************************************************************************

//***********************************************************************************************
// Sets Current FTL Version
#define CURRENT_FTL_VERSION  0.83257f
//***********************************************************************************************

//***********************************************************************************************
// FTL File Structures Definitions
//
typedef struct
{
	char			ident[4]; // FTL
	float			version; // starting with version 1.0f
} ARX_FTL_PRIMARY_HEADER;

typedef struct
{
	long	offset_3Ddata;				// -1 = no
	long	offset_cylinder;			// -1 = no
	long	offset_progressive_data;	// -1 = no
	long	offset_clothes_data;		// -1 = no
	long	offset_collision_spheres;	// -1 = no
	long	offset_physics_box;			// -1 = no
} ARX_FTL_SECONDARY_HEADER;

typedef struct
{
	long	nb_vertex;
} ARX_FTL_PROGRESSIVE_DATA_HEADER;

typedef struct
{
	long	nb_cvert;
	long	nb_springs;
} ARX_FTL_CLOTHES_DATA_HEADER;

typedef struct
{
	long	nb_spheres;
} ARX_FTL_COLLISION_SPHERES_DATA_HEADER;

typedef struct
{
	long	nb_vertex;			// ...
	long	nb_faces;			// ...
	long	nb_maps;			// ...
	long	nb_groups;			// ...
	long	nb_action;			// ...
	long	nb_selections;		// data will follow this order
	long	origin;
	char	name[256];
} ARX_FTL_3D_DATA_HEADER;
// End of Structures definitions
//***********************************************************************************************

extern long NOCHECKSUM;


//***********************************************************************************************
// Saves an FTL File
// Must pass the original name of the theo file
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
BOOL ARX_FTL_Save(char * incomplete_fic, char * complete_fic, EERIE_3DOBJ * obj)
{
	// Need an object to be saved !
	if (obj == NULL) return FALSE;

	// Generate File name/path and create it
	char path[256];
	char gamefic[256];
	sprintf(gamefic, "%sGame%s", Project.workingdir, incomplete_fic);
	SetExt(gamefic, ".FTL");
	strcpy(path, gamefic);
	RemoveName(path);

	if (!CreateFullPath(path)) return NULL;

	// create some usefull vars
	ARX_FTL_PRIMARY_HEADER 	*		afph;
	ARX_FTL_SECONDARY_HEADER 	*		afsh;
	ARX_FTL_CLOTHES_DATA_HEADER 	*	afcdh;
	ARX_FTL_3D_DATA_HEADER 	*		af3Ddh;
	ARX_FTL_COLLISION_SPHERES_DATA_HEADER 	*		afcsdh;
	unsigned char * dat = NULL;
	long pos = 0;
	long allocsize = 8000000; // need to compute it more precisely

	// Compute allocsize...
	allocsize =	sizeof(ARX_FTL_PRIMARY_HEADER)
	            +	512 //checksum
	            +	sizeof(ARX_FTL_SECONDARY_HEADER)
	            +	sizeof(ARX_FTL_3D_DATA_HEADER)
	            +	sizeof(EERIE_OLD_VERTEX) * obj->nbvertex

	            +	sizeof(EERIE_FACE_FTL) * obj->nbfaces
	            +	obj->nbmaps * 256	// texturecontainernames
	            +	sizeof(EERIE_ACTIONLIST) * obj->nbaction
	            +	128000; // just in case...

	if (obj->nbgroups > 0)
	{
		allocsize += sizeof(EERIE_GROUPLIST) * obj->nbgroups;

		for (long i = 0; i < obj->nbgroups; i++)
		{
			if (obj->grouplist[i].nb_index > 0)
			{
				allocsize += sizeof(long) * obj->grouplist[i].nb_index;
			}
		}
	}

	if (obj->nbselections > 0)
	{
		allocsize += sizeof(EERIE_SELECTIONS) * obj->nbselections;

		for (long i = 0; i < obj->nbselections; i++)
		{
			allocsize += sizeof(long) * obj->selections[i].nb_selected;
		}
	}

	if (obj->sdata && obj->sdata->nb_spheres) // Collision Spheres Data
	{
		allocsize += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);
		allocsize += sizeof(COLLISION_SPHERE) * obj->sdata->nb_spheres;
	}

	if (obj->cdata) // Clothes DATA
	{
		allocsize += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);
		allocsize += sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert;
		allocsize += sizeof(EERIE_SPRINGS) * obj->cdata->nb_springs;
	}

	// Finished computing allocsize Now allocate it...
	dat = (unsigned char *)malloc(allocsize);

	if (!dat)
		HERMES_Memory_Emergency_Out();

	memset(dat, 0, allocsize);

	// Primary Header
	afph = (ARX_FTL_PRIMARY_HEADER *)dat;
	pos += sizeof(ARX_FTL_PRIMARY_HEADER);

	if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

	afph->ident[0] = 'F';
	afph->ident[1] = 'T';
	afph->ident[2] = 'L';
	afph->version = CURRENT_FTL_VERSION;

	// Identification
	char check[512];
	MakeUpcase(complete_fic);
	HERMES_CreateFileCheck(complete_fic, check, 512, CURRENT_FTL_VERSION);
	memcpy(dat + pos, check, 512);
	pos += 512;

	if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

	// Secondary Header
	afsh = (ARX_FTL_SECONDARY_HEADER *)(dat + pos);
	pos += sizeof(ARX_FTL_SECONDARY_HEADER);

	if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

	// 3D Data
	afsh->offset_3Ddata = pos; //-1;
	af3Ddh = (ARX_FTL_3D_DATA_HEADER *)(dat + afsh->offset_3Ddata);
	pos += sizeof(ARX_FTL_3D_DATA_HEADER);

	if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

	af3Ddh->nb_vertex = obj->nbvertex;
	af3Ddh->nb_faces = obj->nbfaces;
	af3Ddh->nb_maps = obj->nbmaps;
	af3Ddh->nb_groups = obj->nbgroups;
	af3Ddh->nb_action = obj->nbaction;
	af3Ddh->nb_selections = obj->nbselections;
	af3Ddh->origin = obj->origin;

	// vertexes
	if (af3Ddh->nb_vertex > 0)
	{
		for (long ii = 0; ii < af3Ddh->nb_vertex; ii++)
		{
			memcpy(dat + pos, &obj->vertexlist[ii], sizeof(EERIE_OLD_VERTEX));
			pos += sizeof(EERIE_OLD_VERTEX);
		}

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
	}

	// faces
	if (af3Ddh->nb_faces > 0)
	{
		for (long ii = 0; ii < af3Ddh->nb_faces; ii++)
		{
			EERIE_FACE_FTL * eff = (EERIE_FACE_FTL *)(dat + pos);
			eff->facetype = obj->facelist[ii].facetype;
			eff->texid = obj->facelist[ii].texid;
			eff->transval = obj->facelist[ii].transval;
			eff->temp = obj->facelist[ii].temp;
			memcpy(&eff->norm, &obj->facelist[ii].norm, sizeof(EERIE_3D));

			for (long kk = 0; kk < IOPOLYVERT; kk++)
			{
				memcpy(&eff->nrmls[kk], &obj->facelist[ii].nrmls[kk], sizeof(EERIE_3D));
				eff->vid[kk] = obj->facelist[ii].vid[kk];
				eff->u[kk] = obj->facelist[ii].u[kk];
				eff->v[kk] = obj->facelist[ii].v[kk];
				eff->ou[kk] = obj->facelist[ii].ou[kk];
				eff->ov[kk] = obj->facelist[ii].ov[kk];
				eff->rgb[kk] = 0; 
			}

			pos += sizeof(EERIE_FACE_FTL); 
			if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
		}
	}

	// textures
	for (long i = 0; i < af3Ddh->nb_maps; i++)
	{
		char ficc[256];
		memset(ficc, 0, 256);

		if (obj->texturecontainer[i])
			strcpy(ficc, obj->texturecontainer[i]->m_texName);

		memcpy(dat + pos, ficc, 256);
		pos += 256;

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
	}

	// groups
	if (af3Ddh->nb_groups > 0)
	{
		memcpy(dat + pos, obj->grouplist, sizeof(EERIE_GROUPLIST)*af3Ddh->nb_groups);
		pos += sizeof(EERIE_GROUPLIST) * af3Ddh->nb_groups;

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

		for (int i = 0; i < af3Ddh->nb_groups; i++)
		{
			if (obj->grouplist[i].nb_index > 0)
			{
				memcpy(dat + pos, obj->grouplist[i].indexes, sizeof(long)*obj->grouplist[i].nb_index);
				pos += sizeof(long) * obj->grouplist[i].nb_index;

				if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
			}
		}
	}


	// actionpoints
	if (af3Ddh->nb_action > 0)
	{
		memcpy(dat + pos, obj->actionlist, sizeof(EERIE_ACTIONLIST)*af3Ddh->nb_action);
		pos += sizeof(EERIE_ACTIONLIST) * af3Ddh->nb_action;

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
	}

	// selections
	if (af3Ddh->nb_selections > 0)
	{
		memcpy(dat + pos, obj->selections, sizeof(EERIE_SELECTIONS)*af3Ddh->nb_selections);
		pos += sizeof(EERIE_SELECTIONS) * af3Ddh->nb_selections;

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

		for (int i = 0; i < af3Ddh->nb_selections; i++)
		{
			memcpy(dat + pos, obj->selections[i].selected, sizeof(long)*obj->selections[i].nb_selected);
			pos += sizeof(long) * obj->selections[i].nb_selected;

			if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
		}

		strcpy(af3Ddh->name, obj->file);
	}

	// Progressive DATA
	afsh->offset_progressive_data = -1;
	

	// Collision Spheres Data
	if (obj->sdata && obj->sdata->nb_spheres)
	{
		afsh->offset_collision_spheres = pos; //-1;
		afcsdh = (ARX_FTL_COLLISION_SPHERES_DATA_HEADER *)(dat + afsh->offset_collision_spheres);
		pos += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

		afcsdh->nb_spheres = obj->sdata->nb_spheres;

		memcpy(dat + pos, obj->sdata->spheres, sizeof(COLLISION_SPHERE)*obj->sdata->nb_spheres);
		pos += sizeof(COLLISION_SPHERE) * obj->sdata->nb_spheres;

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
	}
	else afsh->offset_collision_spheres = -1;


	// Clothes DATA
	if (obj->cdata == NULL)
	{
		afsh->offset_clothes_data = -1;
	}
	else
	{
		afsh->offset_clothes_data = pos;
		afcdh = (ARX_FTL_CLOTHES_DATA_HEADER *)(dat + afsh->offset_clothes_data);

		afcdh->nb_cvert = obj->cdata->nb_cvert;
		afcdh->nb_springs = obj->cdata->nb_springs;
		pos += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

		// now save cvert
		memcpy(dat + pos, obj->cdata->cvert, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
		pos += sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert;

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");

		// now saves springs
		memcpy(dat + pos, obj->cdata->springs, sizeof(EERIE_SPRINGS)*obj->cdata->nb_springs);
		pos += sizeof(EERIE_SPRINGS) * obj->cdata->nb_springs;

		if (pos > allocsize) ShowPopup("Invalid Allocsize in ARX_FTL_Save");
	}

	afsh->offset_physics_box = -1;

	afsh->offset_cylinder = -1;

	// Now we can flush our cool FTL file to the hard drive
	unsigned long	handle;
	char _error[512];

	if (pos > allocsize)
	{
		sprintf(_error, "Badly Allocated SaveBuffer...%s", gamefic);
		goto error;
	}

	char * compressed;
	compressed = NULL;
	long cpr_pos;
	cpr_pos = 0;
	compressed = STD_Implode((char *)dat, pos, &cpr_pos);

	// Now Saving Whole Buffer
	if (!(handle = FileOpenWrite(gamefic)))
	{
		sprintf(_error, "Unable to Open %s for Write...", gamefic);
		goto error;
	}

	if (FileWrite(handle, compressed, cpr_pos) != cpr_pos)
	{
		sprintf(_error, "Unable to Write to %s", gamefic);
		goto error;
	}

	FileCloseWrite(handle);
	free(compressed);
	free(dat);
	return TRUE;

error:
	;
	ShowPopup(_error);
	free(dat);

	return FALSE;
}


//***********************************************************************************************
// MESH cache structure definition & Globals
//***********************************************************************************************
typedef struct
{
	char * name;
	char * data;
	long size;
} MCACHE_DATA;

MCACHE_DATA * MCache = NULL;
long MCache_Number = 0;

long MCache_GetSize()
{
	long tot = 0;

	for (long i = 0; i < MCache_Number; i++)
	{
		tot += MCache[i].size;
	}

	return tot;
}
//***********************************************************************************************
// Checks for Mesh file existence in cache
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
long MCache_Get(char * file)
{
	char fic[256];

	File_Standardize(file, fic);

	for (long i = 0; i < MCache_Number; i++)
		if (!stricmp(MCache[i].name, fic)) return i;

	return -1;
}
//***********************************************************************************************
// Pushes a Mesh In Mesh Cache
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
bool MCache_Push(char * file, char * data, long size)
{
	char fic[256];

	File_Standardize(file, fic);

	if (MCache_Get(fic) != -1) return false; // already cached

	MCache = (MCACHE_DATA *)realloc(MCache, sizeof(MCACHE_DATA) * (MCache_Number + 1));
	MCache[MCache_Number].name = (char *)malloc(strlen(file) + 1);
	strcpy(MCache[MCache_Number].name, fic);
	MCache[MCache_Number].data = data;
	MCache[MCache_Number].size = size;
	MCache_Number++;

	return true;
}

//-----------------------------------------------------------------------------
// memory leak
// added tsu 2001/10/16
//-----------------------------------------------------------------------------
void MCache_ClearAll()
{
	for (long i(0); i < MCache_Number; i++)
	{
		free(MCache[i].name), MCache[i].name = NULL;
		free(MCache[i].data), MCache[i].data = NULL;
	}

	free(MCache), MCache = NULL, MCache_Number = 0;
}

//***********************************************************************************************
// Retreives a Mesh File pointer from cache...
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
char * MCache_Pop(char * file, long * size)
{
	long num = MCache_Get(file);

	if (num == -1) return NULL;

	*size = MCache[num].size;
	return MCache[num].data;
}

void EERIE_OBJECT_MakeBH(EERIE_3DOBJ * obj)
{
}

long BH_MODE = 0;

//***********************************************************************************************
// Uses Fast Object Load if FTL file exists
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
EERIE_3DOBJ * ARX_FTL_Load(char * incomplete_fic, char * complete_fic, EERIE_3DOBJ * obj)
{
	// Creates FTL file name
	char gamefic[256];
	sprintf(gamefic, "%sGame%s", Project.workingdir, incomplete_fic);
	SetExt(gamefic, ".FTL");

	// Checks for FTL file existence
	if (!PAK_FileExist(gamefic)) return NULL;

	// Our file exist we can use it
	unsigned char * dat;
	long pos = 0;
	long allocsize;

	ARX_FTL_PRIMARY_HEADER 	*		afph;
	ARX_FTL_SECONDARY_HEADER 	*		afsh;
	ARX_FTL_PROGRESSIVE_DATA_HEADER *	afpdh;
	ARX_FTL_CLOTHES_DATA_HEADER 	*	afcdh;
	ARX_FTL_COLLISION_SPHERES_DATA_HEADER * afcsdh;
	ARX_FTL_3D_DATA_HEADER 	*		af3Ddh;

	char * compressed = NULL;
	long cpr_pos = 0;
	long NOrelease = 1;
	long NoCheck = 0;

	compressed = MCache_Pop(gamefic, &cpr_pos);

	if (!compressed)
	{
		compressed = (char *)PAK_FileLoadMalloc(gamefic, &cpr_pos);
		NOrelease = MCache_Push(gamefic, compressed, cpr_pos) ? 1 : 0;
	}
	else NoCheck = 1;

	if (!compressed) return NULL;

	long DontCheck = 0;

	if (CURRENT_LOADMODE != LOAD_TRUEFILE) DontCheck = 1;

	dat = (unsigned char *)STD_Explode(compressed, cpr_pos, &allocsize);//pos,&cpr_pos);

	if (!dat) return NULL;

	if (!NOrelease) free(compressed);

	// Pointer to Primary Header
	afph = (ARX_FTL_PRIMARY_HEADER *)dat;
	pos += sizeof(ARX_FTL_PRIMARY_HEADER);

	// Verify FTL file Signature
	if ((afph->ident[0] != 'F') && (afph->ident[1] != 'T') && (afph->ident[2] != 'L'))
	{
		free(dat);
		return NULL;
	}

	// Verify FTL file version
	if (afph->version != CURRENT_FTL_VERSION)
	{
		free(dat);
		return NULL;
	}

	// Verify Checksum
	if ((!NOCHECKSUM || NoCheck) && !DontCheck)
	{
		char check[512];

		MakeUpcase(complete_fic);
		HERMES_CreateFileCheck(complete_fic, check, 512, CURRENT_FTL_VERSION);

		char * pouet = (char *)(dat + pos);

		for (long i = 0; i < 512; i++)
			if (check[i] != pouet[i])
			{
				free(dat);
				return NULL;
			}
	}

	// Increases offset by checksum size
	pos += 512;

	// Pointer to Secondary Header
	afsh = (ARX_FTL_SECONDARY_HEADER *)(dat + pos);
	pos += sizeof(ARX_FTL_SECONDARY_HEADER);

	// Check For & Load 3D Data
	if (afsh->offset_3Ddata != -1)
	{
		if (obj) ReleaseEERIE3DObj(obj), obj = NULL;

		//todo free
		obj = (EERIE_3DOBJ *)malloc(sizeof(EERIE_3DOBJ));
		memset(obj, 0, sizeof(EERIE_3DOBJ));

		af3Ddh = (ARX_FTL_3D_DATA_HEADER *)(dat + afsh->offset_3Ddata);
		pos = afsh->offset_3Ddata;
		pos += sizeof(ARX_FTL_3D_DATA_HEADER);

		obj->nbvertex = af3Ddh->nb_vertex;
		obj->nbfaces = af3Ddh->nb_faces;
		obj->nbmaps = af3Ddh->nb_maps;
		obj->nbgroups = af3Ddh->nb_groups;
		obj->nbaction = af3Ddh->nb_action;
		obj->nbselections = af3Ddh->nb_selections;
		obj->origin = af3Ddh->origin;
		strcpy(obj->file, af3Ddh->name);

		// Alloc'n'Copy vertices
		if (obj->nbvertex > 0)
		{
			//todo free
			obj->vertexlist = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX) * obj->nbvertex);

			obj->vertexlist3 = (EERIE_VERTEX *)malloc(sizeof(EERIE_VERTEX) * obj->nbvertex);

			for (long ii = 0; ii < obj->nbvertex; ii++)
			{
				memcpy(&obj->vertexlist[ii], dat + pos, sizeof(EERIE_OLD_VERTEX));
				memcpy(&obj->vertexlist3[ii], dat + pos, sizeof(EERIE_OLD_VERTEX));
				pos += sizeof(EERIE_OLD_VERTEX); 
			}

			obj->point0.x = obj->vertexlist[obj->origin].v.x;
			obj->point0.y = obj->vertexlist[obj->origin].v.y;
			obj->point0.z = obj->vertexlist[obj->origin].v.z;

			for (long i = 0; i < obj->nbvertex; i++)
			{
				obj->vertexlist[i].vert.color = 0xFF000000;
				obj->vertexlist3[i].vert.color = 0xFF000000;
			}
		}

		// Alloc'n'Copy faces
		if (obj->nbfaces > 0)
		{
			//todo free
			obj->facelist = (EERIE_FACE *)malloc(sizeof(EERIE_FACE) * obj->nbfaces);

			for (long ii = 0; ii < af3Ddh->nb_faces; ii++)
			{
				EERIE_FACE_FTL * eff = (EERIE_FACE_FTL *)(dat + pos);
				obj->facelist[ii].facetype = eff->facetype;
				obj->facelist[ii].texid = eff->texid;
				obj->facelist[ii].transval = eff->transval;
				obj->facelist[ii].temp = eff->temp;
				memcpy(&obj->facelist[ii].norm, &eff->norm, sizeof(EERIE_3D));

				for (long kk = 0; kk < IOPOLYVERT; kk++)
				{
					memcpy(&obj->facelist[ii].nrmls[kk], &eff->nrmls[kk], sizeof(EERIE_3D));
					obj->facelist[ii].vid[kk] = eff->vid[kk];
					obj->facelist[ii].u[kk] = eff->u[kk];
					obj->facelist[ii].v[kk] = eff->v[kk];
					obj->facelist[ii].ou[kk] = eff->ou[kk];
					obj->facelist[ii].ov[kk] = eff->ov[kk];
				}

				pos += sizeof(EERIE_FACE_FTL); 
			}
		}

		// Alloc'n'Copy textures
		if (af3Ddh->nb_maps > 0)
		{
			char ficc[256];
			char ficc2[256];
			char ficc3[256];

			//todo free
			obj->texturecontainer = (TextureContainer **)malloc(sizeof(TextureContainer *) * af3Ddh->nb_maps);

			for (long i = 0; i < af3Ddh->nb_maps; i++)
			{
				memcpy(ficc2, dat + pos, 256);
				strcpy(ficc3, Project.workingdir);
				strcat(ficc3, ficc2);
				File_Standardize(ficc3, ficc);

				obj->texturecontainer[i] = D3DTextr_CreateTextureFromFile(ficc, Project.workingdir, 0, 0, EERIETEXTUREFLAG_LOADSCENE_RELEASE);

				if (GDevice && obj->texturecontainer[i] && !obj->texturecontainer[i]->m_pddsSurface)
					obj->texturecontainer[i]->Restore(GDevice);

				MakeUserFlag(obj->texturecontainer[i]);
				pos += 256;
			}
		}

		// Alloc'n'Copy groups
		if (obj->nbgroups > 0)
		{
			//todo free
			obj->grouplist = (EERIE_GROUPLIST *)malloc(sizeof(EERIE_GROUPLIST) * obj->nbgroups);
			memcpy(obj->grouplist, dat + pos, sizeof(EERIE_GROUPLIST) * obj->nbgroups);
			pos += sizeof(EERIE_GROUPLIST) * obj->nbgroups;

			for (long i = 0; i < obj->nbgroups; i++)
				if (obj->grouplist[i].nb_index > 0)
				{
					//TO DO: FREE+++++++++++++++++++++++
					obj->grouplist[i].indexes = (long *)malloc(sizeof(long) * obj->grouplist[i].nb_index);
					memcpy(obj->grouplist[i].indexes, dat + pos, sizeof(long)*obj->grouplist[i].nb_index);
					pos += sizeof(long) * obj->grouplist[i].nb_index;
				}
		}

		// Alloc'n'Copy action points
		if (obj->nbaction > 0)
		{
			//todo free
			obj->actionlist = (EERIE_ACTIONLIST *)malloc(sizeof(EERIE_ACTIONLIST) * obj->nbaction);
			memcpy(obj->actionlist, dat + pos, sizeof(EERIE_ACTIONLIST)*obj->nbaction);
			pos += sizeof(EERIE_ACTIONLIST) * obj->nbaction;
		}

		// Alloc'n'Copy selections
		if (obj->nbselections > 0)
		{
			//todo free++
			obj->selections = (EERIE_SELECTIONS *)malloc(sizeof(EERIE_SELECTIONS) * obj->nbselections);
			memcpy(obj->selections, dat + pos, sizeof(EERIE_SELECTIONS)*obj->nbselections);
			pos += sizeof(EERIE_SELECTIONS) * obj->nbselections;

			for (long i = 0; i < af3Ddh->nb_selections; i++)
			{
				//todo free+++
				obj->selections[i].selected = (long *)malloc(sizeof(long) * obj->selections[i].nb_selected);
				memcpy(obj->selections[i].selected, dat + pos, sizeof(long)*obj->selections[i].nb_selected);
				pos += sizeof(long) * obj->selections[i].nb_selected;
			}
		}

		obj->pbox = NULL;
	}

	if (!obj)
	{
		free(dat);
		return NULL;
	}

	// Alloc'n'Copy Collision Spheres Data
	if (afsh->offset_collision_spheres != -1)
	{
		afcsdh = (ARX_FTL_COLLISION_SPHERES_DATA_HEADER *)(dat + afsh->offset_collision_spheres);
		pos = afsh->offset_collision_spheres;
		pos += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);

		obj->sdata = (COLLISION_SPHERES_DATA *)malloc(sizeof(COLLISION_SPHERES_DATA));
		obj->sdata->nb_spheres = afcsdh->nb_spheres;

		obj->sdata->spheres = (COLLISION_SPHERE *)malloc(sizeof(COLLISION_SPHERE) * obj->sdata->nb_spheres);

		memcpy(obj->sdata->spheres, dat + pos, sizeof(COLLISION_SPHERE)*obj->sdata->nb_spheres);
		pos += sizeof(COLLISION_SPHERE) * obj->sdata->nb_spheres;
	}

	// Alloc'n'Copy Progressive DATA
	if (afsh->offset_progressive_data != -1)
	{
		afpdh = (ARX_FTL_PROGRESSIVE_DATA_HEADER *)(dat + afsh->offset_progressive_data);
		pos = afsh->offset_progressive_data;
		pos += sizeof(ARX_FTL_PROGRESSIVE_DATA_HEADER);
		pos += sizeof(PROGRESSIVE_DATA) * afpdh->nb_vertex;
	}

	// Alloc'n'Copy Clothes DATA
	if (afsh->offset_clothes_data != -1)
	{
		obj->cdata = (CLOTHES_DATA *)malloc(sizeof(CLOTHES_DATA));
		memset(obj->cdata, 0, sizeof(CLOTHES_DATA));

		afcdh = (ARX_FTL_CLOTHES_DATA_HEADER *)(dat + afsh->offset_clothes_data);
		obj->cdata->nb_cvert = (short)afcdh->nb_cvert;
		obj->cdata->nb_springs = (short)afcdh->nb_springs;
		pos = afsh->offset_clothes_data;
		pos += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);

		// now load cvert
		obj->cdata->cvert = (CLOTHESVERTEX *)malloc(sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert);
		obj->cdata->backup = (CLOTHESVERTEX *)malloc(sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert);
		memcpy(obj->cdata->cvert, dat + pos, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
		memcpy(obj->cdata->backup, dat + pos, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
		pos += sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert;

		// now load springs
		obj->cdata->springs = (EERIE_SPRINGS *)malloc(sizeof(EERIE_SPRINGS) * obj->cdata->nb_springs);
		memcpy(obj->cdata->springs, dat + pos, sizeof(EERIE_SPRINGS)*obj->cdata->nb_springs);
		pos += sizeof(EERIE_SPRINGS) * obj->cdata->nb_springs;
	}

	free(dat);

	if (BH_MODE)
		EERIE_OBJECT_MakeBH(obj);

	EERIE_OBJECT_CenterObjectCoordinates(obj);
	EERIE_CreateCedricData(obj);
	EERIEOBJECT_CreatePFaces(obj);
	// Now we can release our cool FTL file
	EERIE_Object_Precompute_Fast_Access(obj);
	return obj;
}
