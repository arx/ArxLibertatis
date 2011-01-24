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
// HERMESMain
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		HUM...hum...
//
// Updates: (date) (person) (update)
//
// Code: Sébastien Scieux
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_HERMES_PAKMANAGER_H
#define ARX_HERMES_PAKMANAGER_H

#include <stddef.h>
#include <vector>

class PakFileHandle;
class PakReader;
class PakDirectory;

void * PAK_FileLoadMalloc(const char * name, size_t * sizeLoaded = NULL);
void * PAK_FileLoadMallocZero(const char * name, size_t * sizeLoaded = NULL);

bool PAK_AddPak(const char * pakfile);

<<<<<<< HEAD
#define LOAD_TRUEFILE			1
#define LOAD_PACK				2
#define LOAD_PACK_THEN_TRUEFILE	3
#define LOAD_TRUEFILE_THEN_PACK	4

extern long CURRENT_LOADMODE;
void * PAK_FileLoadMalloc( const std::string& name, long& SizeLoadMalloc );
void * PAK_FileLoadMallocZero( const std::string& name,long& SizeLoadMalloc );

// use only for READ !!!!
void PAK_SetLoadMode(long mode, const char * pakfile);
FILE * PAK_fopen(const char * filename, const char * mode );
std::size_t PAK_fread(void * buffer, std::size_t size, std::size_t count, FILE * stream );
int PAK_fclose(FILE * stream);
long PAK_ftell(FILE * stream);
long PAK_DirectoryExist( const std::string& name);
long PAK_FileExist( const std::string& name);
int PAK_fseek(FILE * fic,long offset,int origin);

=======
PakFileHandle * PAK_fopen(const char * filename);
size_t PAK_fread(void * buffer, size_t size, size_t count, PakFileHandle * stream);
int PAK_fclose(PakFileHandle * stream);
long PAK_ftell(PakFileHandle * stream);
bool PAK_DirectoryExist(const char * name);
bool PAK_FileExist(const char * name);
int PAK_fseek(PakFileHandle * fic, int offset, long origin);
>>>>>>> 5073e39f879cb51c7b8bd3bb33a399c5a309171c

void PAK_Close();

class PakManager {
	
private:
	
	std::vector<PakReader*> loadedPaks;
	
public:
	
	PakManager();
	~PakManager();

<<<<<<< HEAD
	bool AddPak( const std::string& pakname );
	bool RemovePak( const std::string& pakname );
	bool Read( const std::string& filename, void* buffer );
	void* ReadAlloc( const std::string& filename, int* sizeRead );
	int GetSize( const std::string& filename );
	PakFileHandle * fOpen( const std::string& filename );
	int fClose(PakFileHandle * fh);
	int fRead(void * buffer, int size, int count, PakFileHandle * fh);
	int fSeek(PakFileHandle * fh, int offset, int whence );
	int fTell(PakFileHandle * fh);
	std::vector<PakDirectory*> * ExistDirectory( const std::string& name);
	bool ExistFile( const std::string& _lpszName);
=======
	bool AddPak(const char * pakname);
	bool RemovePak(const char * pakname);
	bool Read(const char * filename, void * buffer);
	void * ReadAlloc(const char * filenme, size_t * sizeRead);
	size_t GetSize(const char * filename);
	PakFileHandle * fOpen(const char * filename);
	int fClose(PakFileHandle * fh);
	size_t fRead(void * buffer, size_t size, size_t count, PakFileHandle * fh);
	int fSeek(PakFileHandle * fh, int offset, long whence);
	int fTell(PakFileHandle * fh);
	std::vector<PakDirectory*> * ExistDirectory(const char * name);
	bool ExistFile(const char * name);
	
>>>>>>> 5073e39f879cb51c7b8bd3bb33a399c5a309171c
};

#endif // ARX_HERMES_PAKMANAGER_H
