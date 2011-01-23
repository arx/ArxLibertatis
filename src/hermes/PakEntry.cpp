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
#include <hermes/PakEntry.h>
#include <hermes/HashMap.h>

#include <cstring>
#include <cassert>


static char * GetFirstDir(const char * dir, size_t * l);

PakFile::PakFile(const char * n)
{
	this->name = NULL;
	this->size = 0;
	this->offset = this->flags = this->uncompressedSize = 0;
	this->prev = this->next = NULL;

	if (n)
	{
		char * nc = new char[strlen(n)+1];
		strcpy(nc, n);
		this->name = nc;
		return;
	}
}
//#############################################################################
PakFile::~PakFile()
{
	if (this->name) delete[] this->name;

	if (this->prev)
	{
		this->prev->next = this->next;
	}

	if (this->next)
	{
		this->next->prev = this->prev;
	}
}
//#############################################################################
//#############################################################################
//                             PakDirectory
//#############################################################################
//#############################################################################
PakDirectory::PakDirectory(PakDirectory * p, const char * n)
{
	filesMap = NULL;
	this->prev = this->next = NULL;
	this->parent = p;
	this->children = NULL;
	this->files = NULL;
	this->nbsousreps = this->nbfiles = 0;

	if (n)
	{
		size_t l;
		this->name = GetFirstDir(n, &l);

		if (this->name)
		{
			if (l != strlen((const char *)n))
			{
				this->nbsousreps = 1;
				this->children = new PakDirectory(this, n + l);
			}

			return;
		}
	}

	this->name = NULL;
}
//#############################################################################
PakDirectory::~PakDirectory()
{
	if(name) {
		delete[] name;
		name = NULL;
	}
	
	if(filesMap) {
		delete filesMap;
		filesMap = NULL;
	}
	
	PakFile * f = files;
	while(f) {
		PakFile * fnext = f->next;
		delete f;
		f = fnext;
	}
	files = NULL;
	
	PakDirectory * r = children;
	while(r) {
		PakDirectory * rnext = r->next;
		delete r;
		r = rnext;
	}
	children = NULL;
}
//#############################################################################
PakDirectory * PakDirectory::addDirectory(const char * sname)
{
	unsigned int nbs = this->nbsousreps;
	size_t l;
	PakDirectory	* rf = this->children;

	const char * fdir = GetFirstDir(sname, &l);

	while (nbs--)
	{
		if (!strcasecmp(fdir, rf->name))
		{
			delete[] fdir;
			return rf->addDirectory(sname + l);
		}

		rf = rf->next;
	}

	delete[] fdir;
	this->nbsousreps++;
	rf = new PakDirectory(this, sname);
	rf->prev = NULL;
	rf->next = this->children;

	if (this->children) this->children->prev = rf;

	this->children = rf;
	
	return rf;
}
//#############################################################################
bool PakDirectory::removeDirectory(const char * sname)
{
	unsigned int nbs = this->nbsousreps;
	size_t l;
	PakDirectory * rf = this->children;
	
	const char * fdir = GetFirstDir(sname, &l);
	
	while (nbs--)
	{
		if (!strcasecmp(fdir, rf->name))
		{
			delete[] fdir;
			bool ok;

			if (l == strlen(sname))
			{
				ok = true;
			}
			else
			{
				ok = rf->removeDirectory(sname + l);
				return ok;
			}

			if (ok)
			{
				if (rf == this->children)
				{
					this->children = rf->next;

					if (this->children) this->children->prev = NULL;
				}
				else
				{
					rf->prev->next = rf->next;

					if (rf->next)
					{
						rf->next->prev = rf->prev;
					}
				}

				this->nbsousreps--;
				delete rf;
			}

			return ok;
		}

		rf = rf->next;
	}

	delete[] fdir;
	return false;
}

#include <stdio.h>

//#############################################################################
PakDirectory * PakDirectory::getDirectory(const char * sname)
{
	unsigned int nbs = this->nbsousreps;
	size_t l;
	PakDirectory	* rf = this->children;
	
	// TODO this can be done without allocating
	const char * fdir = GetFirstDir(sname, &l);
	
	while (nbs--)
	{
		
		if (!strcasecmp(fdir, rf->name))
		{
			delete[] fdir;

			if (l == strlen(sname))
			{
				return rf;
			}
			else
			{
				return rf->getDirectory(sname + l);
			}
		}

		rf = rf->next;
	}

	delete[] fdir;
	return NULL;
}
//#############################################################################
PakFile * PakDirectory::addFile(const char * name) {
	
	PakFile * f = files;
	while(f) {
		if(!strcasecmp(f->name, name)) {
			// File already exists.
			return NULL;
		}
		f = f->next;
	}
	
	f = new PakFile(name);
	if(!f) {
		return NULL;
	}
	
	// Add file to hash map.
	if(filesMap) {
		if(!filesMap->add((char *)name, (void *)f)) {
			delete f;
			return NULL;
		}
	} else {
		printf("file added befor initializing files map\n");
	}
	
	// Link file into list.
	f->prev = NULL;
	f->next = files;
	if(files) {
		files->prev = f;
	}
	files = f;
	
	nbfiles++;
	
	return f;
}

static size_t getFileNamePosition(const char * dirplusname) {
	
	size_t i = strlen(dirplusname);
	
	while(i != 0) {
		i--;
		if(dirplusname[i] == '\\' || dirplusname[i] == '/') {
			// Found dir seperator.
			return i + 1;
		}
	}
	
	return i;
}

PakFile * PakDirectory::getFile(const char * name) {
	
	PakDirectory * d = this;
	
	// Get the directory.
	size_t fpos = getFileNamePosition(name);
	if(fpos) {
		char * dir = new char[fpos + 1]; // TODO this can be done without allocating
		memcpy(dir, name, fpos);
		dir[fpos] = '\0';
		d = this->getDirectory(dir);
		delete[] dir;
		if(!d) {
			// directory not found
			return NULL;
		}
	}
	
	if(!d->filesMap) {
		assert(d->files == NULL);
		// Empty directory.
		return NULL;
	}
	
	const char * file = name + fpos;
	return (PakFile *)d->filesMap->get(file);
}

// TODO is this even used?
void Kill(PakDirectory * r)
{
	PakFile * f = r->files;

	while (r->nbfiles--)
	{
		PakFile * fnext = f->next;
		delete f;
		f = fnext;
	}

	r->files = NULL;

	PakDirectory * brep = r->children;
	int nb = r->nbsousreps;

	while (nb--)
	{
		PakDirectory * brepnext = brep->next;
		Kill(brep);
		brep = brepnext;
	}

	r->children = NULL;
	delete r;
}

static char * GetFirstDir(const char * dir, size_t * l)
{
	const char * dirc = dir;

	*l = 1;

	while ((*dirc) &&
	        (*dirc != '\\' && *dirc != '/'))
	{
		*l += 1;
		dirc++;
	}

	char * fdir = new char[*l+1];

	if (!fdir) return NULL;

	strncpy(fdir, dir, *l);
	fdir[*l] = 0;
	fdir[*l - 1] = '\\'; // TODO use "/" seperator

	return fdir;
}
