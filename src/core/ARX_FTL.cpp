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

#include "core/ARX_FTL.h"
#include "io/HERMESMain.h"
#include "io/PakManager.h"
#include "io/Filesystem.h"
#include "io/Logger.h"
#include "io/blast.h"

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

struct Texture_Container_FTL
{
	char name[256];
};

typedef struct
{
	long		facetype;	// 0 = flat  1 = text
							// 2 = Double-Side
	D3DCOLOR	rgb[IOPOLYVERT];
	unsigned short		vid[IOPOLYVERT];
	short		texid; 
	float		u[IOPOLYVERT];
	float		v[IOPOLYVERT];
	short		ou[IOPOLYVERT];
	short		ov[IOPOLYVERT];

	float		transval;
	EERIE_3D	norm;
	EERIE_3D	nrmls[IOPOLYVERT];
	float		temp;

} EERIE_FACE_FTL; // Aligned 1 2 4

typedef struct
{
	char name[256];
	long		origin;
	long		nb_index;
	long 	*	indexes;
	float		siz;
} EERIE_GROUPLIST_FTL; // Aligned 1 2 4

typedef struct
{
	char name[256];
	long			idx; //index vertex;
	long			act; //action
	long			sfx; //sfx
} EERIE_ACTIONLIST_FTL; // Aligned 1 2 4

typedef struct
{
	char	name[64];
	long	nb_selected;
	long *	selected;
} EERIE_SELECTIONS_FTL; // Aligned 1 2 4

typedef struct
{
	short			idx;
	short			flags;
	float			radius;
} COLLISION_SPHERE_FTL; // Aligned 1 2 4

// End of Structures definitions
//***********************************************************************************************

extern long NOCHECKSUM;


//***********************************************************************************************
// Saves an FTL File
// Must pass the original name of the theo file
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
bool ARX_FTL_Save(const char * file, EERIE_3DOBJ * obj)
{
    // Need an object to be saved !
    if (obj == NULL) return false;

    // Generate File name/path and create it
    std::string gamefic;
    gamefic = "Game\\";
    gamefic += file;
    SetExt(gamefic, ".FTL");
    std::string path = gamefic;
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
    std::string filename = file;
    MakeUpcase(filename);
	HERMES_CreateFileCheck(filename.c_str(), check, 512, CURRENT_FTL_VERSION);
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
			strcpy(ficc, obj->texturecontainer[i]->m_texName.c_str());

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

		strcpy(af3Ddh->name, obj->file.c_str());
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
	FileHandle handle;
	char _error[512];

	if (pos > allocsize)
	{
		sprintf(_error, "Badly Allocated SaveBuffer...%s", gamefic.c_str());
		goto error;
	}

	char * compressed;
	compressed = NULL;
	long cpr_pos;
	cpr_pos = 0;
	LogError <<"IMPLODE NOT IMPLEMENTED";
	// TODO fix
	//compressed = STD_Implode((char *)dat, pos, &cpr_pos);

	// Now Saving Whole Buffer
	if (!(handle = FileOpenWrite(gamefic.c_str())))
	{
		sprintf(_error, "Unable to Open %s for Write...", gamefic.c_str());
		goto error;
	}

	if (FileWrite(handle, compressed, cpr_pos) != cpr_pos)
	{
		sprintf(_error, "Unable to Write to %s", gamefic.c_str());
		goto error;
	}

	FileCloseWrite(handle);
	free(compressed);
	free(dat);
	return true;

error:
	;
	ShowPopup(_error);
	free(dat);

	return false;
}


//***********************************************************************************************
// MESH cache structure definition & Globals
//***********************************************************************************************
typedef struct
{
	std::string name;
	char * data;
	size_t size;
} MeshData;

vector<MeshData> meshCache;

long MCache_GetSize()
//Calculate size of all meshes
{
	long tot = 0;

	for (long i = 0; i < meshCache.size(); i++)
	{
		tot += meshCache[i].size;
	}

	return tot;
}
//***********************************************************************************************
// Checks for Mesh file existence in cache
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
long MCache_Get( const std::string file)
{
    std::string fic;

    File_Standardize(file, fic);

    for (long i = 0; i < meshCache.size(); i++)
        if ( !strcasecmp(meshCache[i].name.c_str(), fic.c_str() ) ) return i;

    return -1;
}
//***********************************************************************************************
// Pushes a Mesh In Mesh Cache
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
bool MCache_Push( const std::string& file, char * data, size_t size)
{
	std::string fic;

	File_Standardize(file, fic);

	if (MCache_Get(fic.c_str()) != -1) return false; // already cached

	LogDebug << fic  << " " << file << " #" << meshCache.size();

//	MCache = (MCACHE_DATA *)realloc(MCache, sizeof(MCACHE_DATA) * (MCache_Number + 1));
//	MCache[MCache_Number].size = size;
//	MCache[MCache_Number].data = data;
//	MCache[MCache_Number].name = fic;
//	MCache_Number++;
	MeshData newMesh;
	newMesh.size = size;
	newMesh.data = data;
	newMesh.name = fic;
	meshCache.push_back(newMesh);

	return true;
}

//-----------------------------------------------------------------------------
// memory leak
// added tsu 2001/10/16
//-----------------------------------------------------------------------------
void MCache_ClearAll()
{
	meshCache.erase(meshCache.begin(),meshCache.end());
}

//***********************************************************************************************
// Retreives a Mesh File pointer from cache...
//-----------------------------------------------------------------------------------------------
// VERIFIED (Cyril 2001/10/15)
//***********************************************************************************************
char * MCache_Pop( const std::string& file, size_t& size)
{
	long num = MCache_Get(file);

	if (num == -1) return NULL;

	size = meshCache[num].size;
	return meshCache[num].data;
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
EERIE_3DOBJ * ARX_FTL_Load(const char * file)
{
	// Creates FTL file name
	std::string filename;
	filename += "Game\\";
	filename += file;
	SetExt(filename, ".FTL");

	// Checks for FTL file existence
	if(!PAK_FileExist(filename.c_str())) {
		LogError << "ARX_FTL_Load: not found in PAK" << filename;
		return NULL;
	}

	// Our file exist we can use it
	unsigned char * dat; // Holds uncompressed data
	long pos = 0; // The position within the data
	size_t allocsize; // The size of the data

	ARX_FTL_PRIMARY_HEADER 	*		afph;
	ARX_FTL_SECONDARY_HEADER 	*		afsh;
	ARX_FTL_3D_DATA_HEADER 	*		af3Ddh;

	char * compressedData = NULL;
	size_t compressedSize = 0;
	long NOrelease = 1;
	long NoCheck = 0;

	compressedData = MCache_Pop(filename, compressedSize);
	LogDebug << "File name check " << filename;

	if (!compressedData)
	{
		compressedData = (char *)PAK_FileLoadMalloc(filename, compressedSize);
		NOrelease = MCache_Push(filename, compressedData, compressedSize) ? 1 : 0;
	}
	else NoCheck = 1;

	if(!compressedData) {
		LogError << "ARX_FTL_Load: error loading from PAK/cache " << filename;
		return NULL;
	}

	long DontCheck = 0;

	DontCheck = 1;

	dat = (unsigned char *)blastMemAlloc(compressedData, compressedSize, allocsize);//pos,&cpr_pos);
	// TODO size ignored

	if(!dat) {
		LogError << "ARX_FTL_Load: error decompressing " << filename;
		return NULL;
	}

	if (!NOrelease) free(compressedData);

	// Pointer to Primary Header
	afph = (ARX_FTL_PRIMARY_HEADER *)dat;
	pos += sizeof(ARX_FTL_PRIMARY_HEADER);

	// Verify FTL file Signature
	if ((afph->ident[0] != 'F') && (afph->ident[1] != 'T') && (afph->ident[2] != 'L'))
	{
		LogError << "ARX_FTL_Load: wrong magic number in " << filename;
		free(dat);
		return NULL;
	}

	// Verify FTL file version
	// TODO Fix float comparison
	if (afph->version != CURRENT_FTL_VERSION)
	{
		LogError << "ARX_FTL_Load: wring version " << afph->version << ", expected "
		         << CURRENT_FTL_VERSION << " in " << filename;
		free(dat);
		return NULL;
	}

	// Verify Checksum
	if ((!NOCHECKSUM || NoCheck) && !DontCheck)
	{
		char check[512];

		HERMES_CreateFileCheck(file, check, 512, CURRENT_FTL_VERSION);

		char * pouet = (char *)(dat + pos);

		for (long i = 0; i < 512; i++)
			if (check[i] != pouet[i])
			{
				LogError << "ARX_FTL_Load: checksum error in " << filename;
				free(dat);
				return NULL;
			}
	}

	// Increases offset by checksum size
	pos += 512;

	// Pointer to Secondary Header
	afsh = (ARX_FTL_SECONDARY_HEADER *)(dat + pos);
	pos += sizeof(ARX_FTL_SECONDARY_HEADER);

	// Available from here in whole function
	EERIE_3DOBJ * obj =0;
	
	// Check For & Load 3D Data
	if (afsh->offset_3Ddata != -1)
	{
		// Alloc the EERIE_3DOBJ if header checks out
		obj = new EERIE_3DOBJ();

		af3Ddh = reinterpret_cast<ARX_FTL_3D_DATA_HEADER*>(dat + afsh->offset_3Ddata);
		pos = afsh->offset_3Ddata;
		pos += sizeof(ARX_FTL_3D_DATA_HEADER);

		obj->nbvertex = af3Ddh->nb_vertex;
		obj->nbfaces = af3Ddh->nb_faces;
		obj->nbmaps = af3Ddh->nb_maps;
		obj->nbgroups = af3Ddh->nb_groups;
		obj->nbaction = af3Ddh->nb_action;
		obj->nbselections = af3Ddh->nb_selections;
		obj->origin = af3Ddh->origin;
		obj->file = af3Ddh->name;

		// Alloc'n'Copy vertices
		if (obj->nbvertex > 0)
		{
			// Alloc the vertices
			obj->vertexlist = new EERIE_VERTEX[obj->nbvertex];
			obj->vertexlist3 = new EERIE_VERTEX[obj->nbvertex];;

			// Copy the vertex data in
			for (long ii = 0; ii < obj->nbvertex; ii++)
			{
				// Vertices stored as EERIE_OLD_VERTEX, copy in to new one
				obj->vertexlist[ii] = *reinterpret_cast<EERIE_OLD_VERTEX*>(dat+pos);
				obj->vertexlist3[ii] = *reinterpret_cast<EERIE_OLD_VERTEX*>(dat+pos);
				pos += sizeof(EERIE_OLD_VERTEX); // Advance position
			}

			// Set the origin point of the mesh
			obj->point0.x = obj->vertexlist[obj->origin].v.x;
			obj->point0.y = obj->vertexlist[obj->origin].v.y;
			obj->point0.z = obj->vertexlist[obj->origin].v.z;

			// Color all the vertices
			for (long i = 0; i < obj->nbvertex; i++)
			{
				obj->vertexlist[i].vert.color = 0xFF000000;
				obj->vertexlist3[i].vert.color = 0xFF000000;
			}
		}

		// Alloc'n'Copy faces
		if (obj->nbfaces > 0)
		{
			// Alloc the list of faces
			obj->facelist = new EERIE_FACE[obj->nbfaces];

			// Copy the face data in
			for (long ii = 0; ii < af3Ddh->nb_faces; ii++)
			{
				EERIE_FACE_FTL* eff = reinterpret_cast<EERIE_FACE_FTL*>(dat + pos);
				obj->facelist[ii].facetype = eff->facetype;
				obj->facelist[ii].texid = eff->texid;
				obj->facelist[ii].transval = eff->transval;
				obj->facelist[ii].temp = eff->temp;
				obj->facelist[ii].norm = eff->norm;

				// Copy in all the texture and normals data
				for (long kk = 0; kk < IOPOLYVERT; kk++)
				{
					obj->facelist[ii].nrmls[kk] = eff->nrmls[kk];
					obj->facelist[ii].vid[kk] = eff->vid[kk];
					obj->facelist[ii].u[kk] = eff->u[kk];
					obj->facelist[ii].v[kk] = eff->v[kk];
					obj->facelist[ii].ou[kk] = eff->ou[kk];
					obj->facelist[ii].ov[kk] = eff->ov[kk];
				}

				// Advance to the next face
				pos += sizeof(EERIE_FACE_FTL); 
			}
		}

		// Alloc'n'Copy textures
		if (af3Ddh->nb_maps > 0)
		{
			// Alloc the TextureContainers
			obj->texturecontainer = new TextureContainer*[af3Ddh->nb_maps];

			// Copy in the texture containers
			for (long i = 0; i < af3Ddh->nb_maps; i++)
			{
				// Get the name of the texture to load
				Texture_Container_FTL* tex = reinterpret_cast<Texture_Container_FTL*>(dat+pos);
				std::string name;
				File_Standardize( tex->name, name );

				// Create the texture and put it in the container list
				obj->texturecontainer[i] = D3DTextr_CreateTextureFromFile( name, 0, 0, EERIETEXTUREFLAG_LOADSCENE_RELEASE);

				if (GDevice && obj->texturecontainer[i] && !obj->texturecontainer[i]->m_pddsSurface)
					obj->texturecontainer[i]->Restore(GDevice);

				MakeUserFlag(obj->texturecontainer[i]);
				pos += sizeof(Texture_Container_FTL);
			}
		}

		// Alloc'n'Copy groups
		if (obj->nbgroups > 0)
		{
			// Alloc the grouplists
			obj->grouplist = new EERIE_GROUPLIST[obj->nbgroups];

			// Copy in the grouplist data
			for ( long i = 0 ; i < obj->nbgroups ; i++ )
			{
				EERIE_GROUPLIST_FTL* group = reinterpret_cast<EERIE_GROUPLIST_FTL*>(dat+pos);
				obj->grouplist[i].name = group->name;
				obj->grouplist[i].origin = group->origin;
				obj->grouplist[i].nb_index = group->nb_index;
				obj->grouplist[i].indexes = 0; // Nullpointer unless changed
				obj->grouplist[i].siz = group->siz;
				pos += sizeof(EERIE_GROUPLIST_FTL); // Advance to next group
			}

			// Copy in the group index data
			for (long i = 0; i < obj->nbgroups; i++)
				if (obj->grouplist[i].nb_index > 0)
				{
					// Alloc space for the index block
					obj->grouplist[i].indexes = new long[obj->grouplist[i].nb_index];
					std::copy( (long*)(dat+pos),
					           (long*)(dat+pos + sizeof(long) * obj->grouplist[i].nb_index),
					           obj->grouplist[i].indexes );
					pos += sizeof(long) * obj->grouplist[i].nb_index; // Advance to the next index block
				}
		}

		// Alloc'n'Copy action points
		if (obj->nbaction > 0)
		{
			// Alloc the action points
			obj->actionlist = new EERIE_ACTIONLIST[obj->nbaction];

			// Copy in the action points data
			for ( long i = 0 ; i < obj->nbaction ; i++ )
			{
				EERIE_ACTIONLIST_FTL* action = reinterpret_cast<EERIE_ACTIONLIST_FTL*>(dat+pos);
				obj->actionlist[i].name = action->name;
				obj->actionlist[i].idx = action->idx;
				obj->actionlist[i].act = action->act;
				obj->actionlist[i].sfx = action->sfx;

				pos += sizeof(EERIE_ACTIONLIST_FTL);
			}
		}

		// Alloc'n'Copy selections
		if (obj->nbselections > 0)
		{
			// Alloc the selections
			obj->selections = new EERIE_SELECTIONS[obj->nbselections];
			//obj->selections = (EERIE_SELECTIONS *)malloc(sizeof(EERIE_SELECTIONS) * obj->nbselections);
			
			// Copy in the selections data
			for ( long i = 0 ; i < obj->nbselections ; i++ )
			{
				EERIE_SELECTIONS_FTL* selection = reinterpret_cast<EERIE_SELECTIONS_FTL*>(dat+pos);
				obj->selections[i].name = selection->name;
				obj->selections[i].nb_selected = selection->nb_selected;
				obj->selections[i].selected = 0; // Null unless changed

				pos += sizeof(EERIE_SELECTIONS_FTL);
			}

			// Copy in the selections selected data
			for (long i = 0; i < af3Ddh->nb_selections; i++)
			{
				obj->selections[i].selected = new long[obj->selections[i].nb_selected];
				std::copy( (long*)(dat+pos),
				           (long*)(dat+pos + sizeof(long)*obj->selections[i].nb_selected),
				           obj->selections[i].selected );
				pos += sizeof(long) * obj->selections[i].nb_selected; // Advance to the next selection data block
			}
		}

		obj->pbox = NULL; // Reset physics
	}
	
	if ( !obj ) // Never created obj object
	{
		LogError << "ARX_FTL_Load: error loading data from " << filename;
		free(dat);
		return 0;
	}

	// Alloc'n'Copy Collision Spheres Data
	if (afsh->offset_collision_spheres != -1)
	{
		// Cast to header
		ARX_FTL_COLLISION_SPHERES_DATA_HEADER * afcsdh;
		afcsdh = reinterpret_cast<ARX_FTL_COLLISION_SPHERES_DATA_HEADER*>(dat + afsh->offset_collision_spheres);
		pos = afsh->offset_collision_spheres;
		pos += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);

		// Alloc the collision sphere data object
		obj->sdata = new COLLISION_SPHERES_DATA();
		obj->sdata->nb_spheres = afcsdh->nb_spheres;

		// Alloc the collision speheres
		obj->sdata->spheres = new COLLISION_SPHERE[obj->sdata->nb_spheres];

		// TODO Replace with copy
		memcpy(obj->sdata->spheres, dat + pos, sizeof(COLLISION_SPHERE)*obj->sdata->nb_spheres);
		pos += sizeof(COLLISION_SPHERE) * obj->sdata->nb_spheres;
	}

	// Alloc'n'Copy Progressive DATA
	if (afsh->offset_progressive_data != -1)
	{
		ARX_FTL_PROGRESSIVE_DATA_HEADER *	afpdh;
		afpdh = (ARX_FTL_PROGRESSIVE_DATA_HEADER *)(dat + afsh->offset_progressive_data);
		pos = afsh->offset_progressive_data;
		pos += sizeof(ARX_FTL_PROGRESSIVE_DATA_HEADER);
		pos += sizeof(PROGRESSIVE_DATA) * afpdh->nb_vertex;
	}

	// Alloc'n'Copy Clothes DATA
	if (afsh->offset_clothes_data != -1)
	{
		obj->cdata = new CLOTHES_DATA();

		ARX_FTL_CLOTHES_DATA_HEADER 	*	afcdh;
		afcdh = reinterpret_cast<ARX_FTL_CLOTHES_DATA_HEADER*>(dat + afsh->offset_clothes_data);
		obj->cdata->nb_cvert = (short)afcdh->nb_cvert;
		obj->cdata->nb_springs = (short)afcdh->nb_springs;
		pos = afsh->offset_clothes_data;
		pos += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);

		// now load cvert
		obj->cdata->cvert = new CLOTHESVERTEX[obj->cdata->nb_cvert];
		obj->cdata->backup = new CLOTHESVERTEX[obj->cdata->nb_cvert];
		memcpy(obj->cdata->cvert, dat + pos, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
		memcpy(obj->cdata->backup, dat + pos, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
		pos += sizeof(CLOTHESVERTEX) * obj->cdata->nb_cvert;

		// now load springs
		obj->cdata->springs = new EERIE_SPRINGS[obj->cdata->nb_springs];
		memcpy(obj->cdata->springs, dat + pos, sizeof(EERIE_SPRINGS)*obj->cdata->nb_springs);
		pos += sizeof(EERIE_SPRINGS) * obj->cdata->nb_springs;
	}

	// Free the loaded file memory
	free(dat);

	if (BH_MODE)
		EERIE_OBJECT_MakeBH(obj);

	EERIE_OBJECT_CenterObjectCoordinates(obj);
	EERIE_CreateCedricData(obj);
	EERIEOBJECT_CreatePFaces(obj);
	// Now we can release our cool FTL file
	EERIE_Object_Precompute_Fast_Access(obj);
	LogInfo << "ARX_FTL_Load: loaded object " << filename;
	return obj;
}
