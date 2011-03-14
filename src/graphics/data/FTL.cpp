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
// Initial Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include "graphics/data/FTL.h"

#include <cstdlib>
#include <cassert>

#include "core/Application.h"

#include "graphics/data/FTLFormat.h"
#include "graphics/data/Texture.h"

#include "io/FilePath.h"
#include "io/PakManager.h"
#include "io/Filesystem.h"
#include "io/Logger.h"
#include "io/Blast.h"
#include "io/Implode.h"
#include "io/IO.h"
#include "io/String.h"

#include "scene/Object.h"

extern long NOCHECKSUM;

bool ARX_FTL_Save(const char * file, const EERIE_3DOBJ * obj) {
	
	LogWarning << "ARX_FTL_Save " << file;
	
	if(!obj) {
		return false;
	}
	
	// Generate File name/path and create it
	string gamefic = "Game\\";
	gamefic += file;
	SetExt(gamefic, ".FTL");
	
	if(!CreateFullPath(gamefic)) {
		return NULL;
	}
	
	// Compute allocsize...
	size_t allocsize = sizeof(ARX_FTL_PRIMARY_HEADER)
	                   + 512 //checksum
	                   + sizeof(ARX_FTL_SECONDARY_HEADER)
	                   + sizeof(ARX_FTL_3D_DATA_HEADER)
	                   + sizeof(EERIE_OLD_VERTEX) * obj->vertexlist.size()
	                   + sizeof(EERIE_FACE_FTL) * obj->facelist.size()
	                   + sizeof(Texture_Container_FTL) * obj->texturecontainer.size()
	                   + sizeof(EERIE_ACTIONLIST_FTL) * obj->actionlist.size()
	                   + 128000; // TODO just in case...
	
	if(obj->nbgroups > 0) {
		allocsize += sizeof(EERIE_GROUPLIST_FTL) * obj->nbgroups;
		for(long i = 0; i < obj->nbgroups; i++) {
			 allocsize += sizeof(long) * obj->grouplist[i].indexes.size();
		}
	}
	
	if(!obj->selections.empty()) {
		allocsize += sizeof(EERIE_SELECTIONS_FTL) * obj->selections.size();
		for(size_t i = 0; i < obj->selections.size(); i++) {
			allocsize += sizeof(long) * obj->selections[i].selected.size();
		}
	}
	
	if(obj->sdata && !obj->sdata->spheres.empty()) {
		allocsize += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);
		allocsize += sizeof(COLLISION_SPHERE_FTL) * obj->sdata->spheres.size();
	}
	
	if(obj->cdata) {
		allocsize += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);
		allocsize += sizeof(CLOTHESVERTEX_FTL) * obj->cdata->nb_cvert;
		allocsize += sizeof(EERIE_SPRINGS_FTL) * obj->cdata->springs.size();
	}
	
	// Finished computing allocsize Now allocate it...
	char * dat = new char[allocsize];
	size_t pos = 0;
	
	memset(dat, 0, allocsize);
	
	// Primary Header
	{
		ARX_FTL_PRIMARY_HEADER afph;
		
		afph.ident[0] = 'F';
		afph.ident[1] = 'T';
		afph.ident[2] = 'L';
		afph.ident[3] = '0';
		afph.version = CURRENT_FTL_VERSION;
		
		memcpy(dat + pos, &afph, sizeof(ARX_FTL_PRIMARY_HEADER));
		pos += sizeof(ARX_FTL_PRIMARY_HEADER);
	}
	
	// Identification
	char check[512];
	string filename = file;
	MakeUpcase(filename);
	HERMES_CreateFileCheck(filename.c_str(), check, 512, CURRENT_FTL_VERSION);
	memcpy(dat + pos, check, 512);
	pos += 512;
	
	// Secondary Header
	ARX_FTL_SECONDARY_HEADER*afsh = (ARX_FTL_SECONDARY_HEADER *)(dat + pos);
	pos += sizeof(ARX_FTL_SECONDARY_HEADER);

	if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");

	// 3D Data
	afsh->offset_3Ddata = pos; //-1;
	ARX_FTL_3D_DATA_HEADER * af3Ddh = (ARX_FTL_3D_DATA_HEADER *)(dat + afsh->offset_3Ddata);
	pos += sizeof(ARX_FTL_3D_DATA_HEADER);

	if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");

	af3Ddh->nb_vertex = obj->vertexlist.size();
	af3Ddh->nb_faces = obj->facelist.size();
	af3Ddh->nb_maps = obj->texturecontainer.size();
	af3Ddh->nb_groups = obj->nbgroups;
	af3Ddh->nb_action = obj->actionlist.size();
	af3Ddh->nb_selections = obj->selections.size();
	af3Ddh->origin = obj->origin;

	// vertexes
	if (af3Ddh->nb_vertex > 0) {
		std::copy(obj->vertexlist.begin(), obj->vertexlist.end(), (EERIE_OLD_VERTEX*)(dat + pos));
		pos += sizeof(EERIE_OLD_VERTEX) * obj->vertexlist.size();

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");
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
			if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");
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

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");
	}

	// groups
	if (af3Ddh->nb_groups > 0)
	{
		std::copy(obj->grouplist, obj->grouplist + obj->nbgroups, (EERIE_GROUPLIST_FTL*)(dat + pos));
		pos += sizeof(EERIE_GROUPLIST_FTL) * af3Ddh->nb_groups;

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");

		for(int i = 0; i < af3Ddh->nb_groups; i++) {
			if(!obj->grouplist[i].indexes.empty()) {
				std::copy(obj->grouplist[i].indexes.begin(), obj->grouplist[i].indexes.end(), (s32*)(dat + pos));
				pos += sizeof(s32) * obj->grouplist[i].indexes.size();
			}
		}
	}

	// actionpoints
	if(af3Ddh->nb_action > 0) {
		std::copy(obj->actionlist.begin(), obj->actionlist.end(), (EERIE_ACTIONLIST_FTL*)(dat + pos));
		pos += sizeof(EERIE_ACTIONLIST_FTL) * af3Ddh->nb_action;

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");
	}

	// selections
	if (af3Ddh->nb_selections > 0)
	{
		std::copy(obj->selections.begin(), obj->selections.end(), (EERIE_SELECTIONS_FTL*)(dat + pos));
		pos += sizeof(EERIE_SELECTIONS_FTL) * af3Ddh->nb_selections;

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");

		for (int i = 0; i < af3Ddh->nb_selections; i++) {
			std::copy(obj->selections[i].selected.begin(), obj->selections[i].selected.end(), (s32*)(dat + pos));
			pos += sizeof(s32) * obj->selections[i].selected.size();

			if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");
		}

		strcpy(af3Ddh->name, obj->file.c_str());
	}

	// Progressive DATA
	afsh->offset_progressive_data = -1;
	

	// Collision Spheres Data
	if (obj->sdata && !obj->sdata->spheres.empty())
	{
		afsh->offset_collision_spheres = pos; //-1;
		ARX_FTL_COLLISION_SPHERES_DATA_HEADER 	*		afcsdh = (ARX_FTL_COLLISION_SPHERES_DATA_HEADER *)(dat + afsh->offset_collision_spheres);
		pos += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");

		afcsdh->nb_spheres = obj->sdata->spheres.size();

		std::copy(obj->sdata->spheres.begin(), obj->sdata->spheres.end(), (COLLISION_SPHERE_FTL*)(dat + pos));
		pos += sizeof(COLLISION_SPHERE_FTL) * obj->sdata->spheres.size();

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");
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
		ARX_FTL_CLOTHES_DATA_HEADER * afcdh = (ARX_FTL_CLOTHES_DATA_HEADER *)(dat + afsh->offset_clothes_data);

		afcdh->nb_cvert = obj->cdata->nb_cvert;
		afcdh->nb_springs = obj->cdata->springs.size();
		pos += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");

		// now save cvert
		std::copy(obj->cdata->cvert, obj->cdata->cvert + obj->cdata->nb_cvert, (CLOTHESVERTEX_FTL*)(dat + pos));
		pos += sizeof(CLOTHESVERTEX_FTL) * obj->cdata->nb_cvert;

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");

		// now saves springs
		std::copy(obj->cdata->springs.begin(), obj->cdata->springs.end(), (EERIE_SPRINGS_FTL*)(dat + pos));
		pos += sizeof(EERIE_SPRINGS_FTL) * obj->cdata->springs.size();

		if (pos > allocsize) LogError << ("Invalid Allocsize in ARX_FTL_Save");
	}
	
	afsh->offset_physics_box = -1;
	
	afsh->offset_cylinder = -1;
	
	// Now we can flush our cool FTL file to the hard drive
	
	if(pos > allocsize) {
		LogError << "Badly Allocated SaveBuffer... " << gamefic;
		free(dat);
		return false;
	}
	
	size_t cpr_pos = 0;
	char * compressed = implodeAlloc(dat, pos, cpr_pos);
	
	// Now Saving Whole Buffer
	FileHandle handle;
	if(!(handle = FileOpenWrite(gamefic.c_str()))) {
		LogError << "Unable to Open " << gamefic << " for Write...";
		free(dat);
		return false;
	}
	
	if((size_t)FileWrite(handle, compressed, cpr_pos) != cpr_pos) {
		LogError <<  "Unable to Write to " << gamefic;
		free(dat);
		return false;
	}
	
	FileClose(handle);
	free(compressed);
	free(dat);
	return true;
	
}


// MESH cache structure definition & Globals
struct MCACHE_DATA {
	std::string name;
	char* data;
	size_t size;
};
static vector<MCACHE_DATA> meshCache;

// Checks for Mesh file existence in cache
static long MCache_Get( const std::string file)
{
    std::string fic;

    File_Standardize(file, fic);

    for (size_t i = 0; i < meshCache.size(); i++)
        if ( !strcasecmp(meshCache[i].name.c_str(), fic.c_str() ) ) return i;

    return -1;
}

// Pushes a Mesh In Mesh Cache
static bool MCache_Push( const std::string& file, char * data, size_t size)
{
	std::string fic;

	File_Standardize(file, fic);

	if (MCache_Get(fic.c_str()) != -1) return false; // already cached

	LogDebug << fic  << " " << file << " #" << meshCache.size();

	MCACHE_DATA newMesh;
	newMesh.size = size;
	newMesh.data = data;
	newMesh.name = fic;
	meshCache.push_back(newMesh);

	return true;
}

void MCache_ClearAll(){
	meshCache.erase(meshCache.begin(),meshCache.end());
}

// Retreives a Mesh File pointer from cache...
static char* MCache_Pop( const std::string& file, size_t& size)
{
	long num = MCache_Get(file);

	if (num == -1) return NULL;

	size = meshCache[num].size;
	return meshCache[num].data;
}

EERIE_3DOBJ * ARX_FTL_Load(const char * file) {
	
	// Creates FTL file name
	std::string filename = "Game\\";
	filename += file;
	SetExt(filename, ".FTL");
	
	// Checks for FTL file existence
	if(!PAK_FileExist(filename.c_str())) {
		LogError << "ARX_FTL_Load: not found in PAK " << filename;
		return NULL;
	}
	
	size_t compressedSize = 0;
	char * compressedData = MCache_Pop(filename, compressedSize);
	LogDebug << "File name check " << filename;
	
	bool NOrelease = true;
	if(!compressedData) {
		compressedData = (char *)PAK_FileLoadMalloc(filename, compressedSize);
		NOrelease = MCache_Push(filename, compressedData, compressedSize) ? 1 : 0;
	}
	
	if(!compressedData) {
		LogError << "ARX_FTL_Load: error loading from PAK/cache " << filename;
		return NULL;
	}
	
	size_t allocsize; // The size of the data
	char * dat = blastMemAlloc(compressedData, compressedSize, allocsize);//pos,&cpr_pos);
	// TODO size ignored
	
	if(!dat) {
		LogError << "ARX_FTL_Load: error decompressing " << filename;
		return NULL;
	}
	
	if(!NOrelease) {
		free(compressedData);
	}
	
	size_t pos = 0; // The position within the data
	
	// Pointer to Primary Header
	ARX_FTL_PRIMARY_HEADER * afph = (ARX_FTL_PRIMARY_HEADER *)(dat + pos);
	pos += sizeof(ARX_FTL_PRIMARY_HEADER);
	
	// Verify FTL file Signature
	if(afph->ident[0] != 'F' || afph->ident[1] != 'T' || afph->ident[2] != 'L' || afph->ident[3] != '0') {
		LogError << "ARX_FTL_Load: wrong magic number in " << filename;
		free(dat);
		return NULL;
	}
	
	// Verify FTL file version
	if(afph->version != CURRENT_FTL_VERSION) {
		LogError << "ARX_FTL_Load: wring version " << afph->version << ", expected "
		         << CURRENT_FTL_VERSION << " in " << filename;
		free(dat);
		return NULL;
	}
	
	// Increases offset by checksum size
	pos += 512;
	
	// Pointer to Secondary Header
	ARX_FTL_SECONDARY_HEADER * afsh = (ARX_FTL_SECONDARY_HEADER *)(dat + pos);
	pos += sizeof(ARX_FTL_SECONDARY_HEADER);
	
	if(afsh->offset_3Ddata == -1) {
		LogError << "ARX_FTL_Load: error loading data from " << filename;
		free(dat);
		return NULL;
	}
	
	// Available from here in whole function
	EERIE_3DOBJ * obj = new EERIE_3DOBJ();
	
	pos = afsh->offset_3Ddata;
	ARX_FTL_3D_DATA_HEADER * af3Ddh = reinterpret_cast<ARX_FTL_3D_DATA_HEADER*>(dat + pos);
	pos += sizeof(ARX_FTL_3D_DATA_HEADER);
	
	obj->vertexlist.resize(af3Ddh->nb_vertex);
	obj->facelist.resize(af3Ddh->nb_faces);
	obj->texturecontainer.resize(af3Ddh->nb_maps);
	obj->nbgroups = af3Ddh->nb_groups;
	obj->actionlist.resize(af3Ddh->nb_action);
	obj->selections.resize(af3Ddh->nb_selections);
	obj->origin = af3Ddh->origin;
	obj->file = af3Ddh->name;
	
	// Alloc'n'Copy vertices
	if(!obj->vertexlist.empty()) {
		
		// Copy the vertex data in
		for(size_t ii = 0; ii < obj->vertexlist.size(); ii++) {
			
			// Vertices stored as EERIE_OLD_VERTEX, copy in to new one
			obj->vertexlist[ii] = *reinterpret_cast<EERIE_OLD_VERTEX*>(dat + pos);
			pos += sizeof(EERIE_OLD_VERTEX);
			
			obj->vertexlist[ii].vert.color = 0xFF000000;
		}
		
		// Set the origin point of the mesh
		obj->point0.x = obj->vertexlist[obj->origin].v.x;
		obj->point0.y = obj->vertexlist[obj->origin].v.y;
		obj->point0.z = obj->vertexlist[obj->origin].v.z;
		
		obj->vertexlist3 = obj->vertexlist;
	}
	
	// Alloc'n'Copy faces
	if(!obj->facelist.empty()) {
		
		// Copy the face data in
		for(long ii = 0; ii < af3Ddh->nb_faces; ii++) {
			
			EERIE_FACE_FTL * eff = reinterpret_cast<EERIE_FACE_FTL*>(dat + pos);
			pos += sizeof(EERIE_FACE_FTL); 
			
			obj->facelist[ii].facetype = eff->facetype;
			obj->facelist[ii].texid = eff->texid;
			obj->facelist[ii].transval = eff->transval;
			obj->facelist[ii].temp = eff->temp;
			obj->facelist[ii].norm = eff->norm;
			
			// Copy in all the texture and normals data
			assert(IOPOLYVERT_FTL == IOPOLYVERT);
			for(size_t kk = 0; kk < IOPOLYVERT_FTL; kk++) {
				obj->facelist[ii].nrmls[kk] = eff->nrmls[kk];
				obj->facelist[ii].vid[kk] = eff->vid[kk];
				obj->facelist[ii].u[kk] = eff->u[kk];
				obj->facelist[ii].v[kk] = eff->v[kk];
				obj->facelist[ii].ou[kk] = eff->ou[kk];
				obj->facelist[ii].ov[kk] = eff->ov[kk];
			}
			
		}
	}
	
	// Alloc'n'Copy textures
	if(af3Ddh->nb_maps > 0) {
		
		// Copy in the texture containers
		for(long i = 0; i < af3Ddh->nb_maps; i++) {
			
			Texture_Container_FTL * tex = reinterpret_cast<Texture_Container_FTL*>(dat + pos);
			pos += sizeof(Texture_Container_FTL);
			
			std::string name;
			File_Standardize( tex->name, name );
			
			// Create the texture and put it in the container list
			obj->texturecontainer[i] = D3DTextr_CreateTextureFromFile( name, 0, 0, EERIETEXTUREFLAG_LOADSCENE_RELEASE);
			
			if(GDevice && obj->texturecontainer[i] && !obj->texturecontainer[i]->m_pddsSurface) {
				obj->texturecontainer[i]->Restore(GDevice);
			}
			
			MakeUserFlag(obj->texturecontainer[i]);
			
		}
	}
	
	// Alloc'n'Copy groups
	if(obj->nbgroups > 0) {
		
		// Alloc the grouplists
		obj->grouplist = new EERIE_GROUPLIST[obj->nbgroups];
		
		// Copy in the grouplist data
		for(long i = 0 ; i < obj->nbgroups ; i++) {
			
			EERIE_GROUPLIST_FTL* group = reinterpret_cast<EERIE_GROUPLIST_FTL*>(dat + pos);
			pos += sizeof(EERIE_GROUPLIST_FTL);
			
			obj->grouplist[i].name = group->name;
			obj->grouplist[i].origin = group->origin;
			obj->grouplist[i].indexes.resize(group->nb_index);
			obj->grouplist[i].siz = group->siz;
			
		}
		
		// Copy in the group index data
		for(long i = 0; i < obj->nbgroups; i++) {
			if(!obj->grouplist[i].indexes.empty()) {
				size_t oldpos = pos;
				pos += sizeof(s32) * obj->grouplist[i].indexes.size(); // Advance to the next index block
				std::copy((s32*)(dat+oldpos), (s32*)(dat + pos), obj->grouplist[i].indexes.begin());
			}
		}
	}
	
	// Copy in the action points data
	for(size_t i = 0 ; i < obj->actionlist.size(); i++) {
		obj->actionlist[i] = *reinterpret_cast<EERIE_ACTIONLIST_FTL*>(dat + pos);
		pos += sizeof(EERIE_ACTIONLIST_FTL);
	}
	
	// Copy in the selections data
	for(size_t i = 0 ; i < obj->selections.size(); i++) {
		
		EERIE_SELECTIONS_FTL * selection = reinterpret_cast<EERIE_SELECTIONS_FTL*>(dat + pos);
		pos += sizeof(EERIE_SELECTIONS_FTL);
		
		obj->selections[i].name = selection->name;
		obj->selections[i].selected.resize(selection->nb_selected);
	}
	
	// Copy in the selections selected data
	for(long i = 0; i < af3Ddh->nb_selections; i++) {
		std::copy((s32*)(dat + pos), (s32*)(dat + pos) + obj->selections[i].selected.size(), obj->selections[i].selected.begin() );
		pos += sizeof(s32) * obj->selections[i].selected.size(); // Advance to the next selection data block
	}
	
	obj->pbox = NULL; // Reset physics
	
	// Alloc'n'Copy Collision Spheres Data
	if(afsh->offset_collision_spheres != -1) {
		
		// Cast to header
		ARX_FTL_COLLISION_SPHERES_DATA_HEADER * afcsdh;
		pos = afsh->offset_collision_spheres;
		afcsdh = reinterpret_cast<ARX_FTL_COLLISION_SPHERES_DATA_HEADER*>(dat + pos);
		pos += sizeof(ARX_FTL_COLLISION_SPHERES_DATA_HEADER);
		
		// Alloc the collision sphere data object
		obj->sdata = new COLLISION_SPHERES_DATA();
		obj->sdata->spheres.resize(afcsdh->nb_spheres);
		
		// Alloc the collision speheres
		COLLISION_SPHERE_FTL * begin = (COLLISION_SPHERE_FTL*)(dat + pos);
		pos += sizeof(COLLISION_SPHERE_FTL) * obj->sdata->spheres.size();
		COLLISION_SPHERE_FTL * end = (COLLISION_SPHERE_FTL*)(dat + pos);
		std::copy(begin, end, obj->sdata->spheres.begin());
	}
	
	// Alloc'n'Copy Progressive DATA
	if(afsh->offset_progressive_data != -1) {
		ARX_FTL_PROGRESSIVE_DATA_HEADER *	afpdh;
		afpdh = (ARX_FTL_PROGRESSIVE_DATA_HEADER *)(dat + afsh->offset_progressive_data);
		pos = afsh->offset_progressive_data;
		pos += sizeof(ARX_FTL_PROGRESSIVE_DATA_HEADER);
		pos += sizeof(PROGRESSIVE_DATA) * afpdh->nb_vertex;
	}
	
	// Alloc'n'Copy Clothes DATA
	if(afsh->offset_clothes_data != -1) {
		
		obj->cdata = new CLOTHES_DATA();
		
		ARX_FTL_CLOTHES_DATA_HEADER 	*	afcdh;
		afcdh = reinterpret_cast<ARX_FTL_CLOTHES_DATA_HEADER*>(dat + afsh->offset_clothes_data);
		obj->cdata->nb_cvert = (short)afcdh->nb_cvert;
		obj->cdata->springs.resize(afcdh->nb_springs);
		pos = afsh->offset_clothes_data;
		pos += sizeof(ARX_FTL_CLOTHES_DATA_HEADER);
		
		// now load cvert
		obj->cdata->cvert = new CLOTHESVERTEX[obj->cdata->nb_cvert];
		obj->cdata->backup = new CLOTHESVERTEX[obj->cdata->nb_cvert];
		std::copy((CLOTHESVERTEX_FTL*)(dat + pos), (CLOTHESVERTEX_FTL*)(dat + pos) + obj->cdata->nb_cvert, obj->cdata->cvert);
		memcpy(obj->cdata->backup, obj->cdata->cvert, sizeof(CLOTHESVERTEX)*obj->cdata->nb_cvert);
		pos += sizeof(CLOTHESVERTEX_FTL) * obj->cdata->nb_cvert;
		
		// now load springs
		EERIE_SPRINGS_FTL * begin = (EERIE_SPRINGS_FTL*)(dat + pos);
		pos += sizeof(EERIE_SPRINGS_FTL) * obj->cdata->springs.size();
		EERIE_SPRINGS_FTL * end = (EERIE_SPRINGS_FTL*)(dat + pos);
		std::copy(begin, end, obj->cdata->springs.begin());
	}
	
	// Free the loaded file memory
	free(dat);
	
	EERIE_OBJECT_CenterObjectCoordinates(obj);
	EERIE_CreateCedricData(obj);
	EERIEOBJECT_CreatePFaces(obj);
	// Now we can release our cool FTL file
	EERIE_Object_Precompute_Fast_Access(obj);
	
	LogInfo << "ARX_FTL_Load: loaded object " << filename;
	
	return obj;
}
