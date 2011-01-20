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


using std::size_t;


static char * GetFirstDir(const char * dir, size_t * l);

PakFile::PakFile(const char * n)
{
	this->name = NULL;
	this->size = 0;
	this->offset = this->flags = this->uncompressedSize = 0;
	this->fprev = this->fnext = NULL;

	if (n)
	{
		this->name = new char[strlen(n)+1];
		strcpy(this->name, n);
		return;
	}
}
//#############################################################################
PakFile::~PakFile()
{
	if (this->name) delete[] this->name;

	if (this->fprev)
	{
		this->fprev->fnext = this->fnext;
	}

	if (this->fnext)
	{
		this->fnext->fprev = this->fprev;
	}
}
//#############################################################################
//#############################################################################
//                             PakDirectory
//#############################################################################
//#############################################################################
PakDirectory::PakDirectory(PakDirectory * p, const char * n)
{
	pHachage = NULL;
	this->param = 0;
	this->brotherprev = this->brothernext = NULL;
	this->parent = p;
	this->fils = NULL;
	this->fichiers = NULL;
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
				this->fils = new PakDirectory(this, n + l);
			}

			return;
		}
	}

	this->name = NULL;
}
//#############################################################################
PakDirectory::~PakDirectory()
{
	if (this->name)
	{
		delete[] this->name;
	}

	if (pHachage)
	{
		delete pHachage;
		pHachage = NULL;
	}

	PakFile * f = fichiers;

	while (nbfiles--)
	{
		PakFile * fnext = f->fnext;
		delete f;
		f = fnext;
	}

	fichiers = NULL;

	PakDirectory * r = this->fils;
	unsigned int nb = this->nbsousreps;

	while (nb--)
	{
		PakDirectory * rnext = r->brothernext;
		delete r;
		r = rnext;
	}
}
//#############################################################################
PakDirectory * PakDirectory::AddSousRepertoire(const char * sname)
{
	unsigned int nbs = this->nbsousreps;
	size_t l;
	PakDirectory	* rf = this->fils;

	const char * fdir = GetFirstDir(sname, &l);

	while (nbs--)
	{
		if (!strcasecmp(fdir, rf->name))
		{
			delete[] fdir;
			return rf->AddSousRepertoire(sname + l);
		}

		rf = rf->brothernext;
	}

	delete[] fdir;
	this->nbsousreps++;
	rf = new PakDirectory(this, sname);
	rf->brotherprev = NULL;
	rf->brothernext = this->fils;

	if (this->fils) this->fils->brotherprev = rf;

	this->fils = rf;
	
	return rf;
}
//#############################################################################
bool PakDirectory::DelSousRepertoire(const char * sname)
{
	unsigned int nbs = this->nbsousreps;
	size_t l;
	PakDirectory * rf = this->fils;
	
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
				ok = rf->DelSousRepertoire(sname + l);
				return ok;
			}

			if (ok)
			{
				if (rf == this->fils)
				{
					this->fils = rf->brothernext;

					if (this->fils) this->fils->brotherprev = NULL;
				}
				else
				{
					rf->brotherprev->brothernext = rf->brothernext;

					if (rf->brothernext)
					{
						rf->brothernext->brotherprev = rf->brotherprev;
					}
				}

				this->nbsousreps--;
				delete rf;
			}

			return ok;
		}

		rf = rf->brothernext;
	}

	delete[] fdir;
	return false;
}

#include <stdio.h>

//#############################################################################
PakDirectory * PakDirectory::GetSousRepertoire(const char * sname)
{
	unsigned int nbs = this->nbsousreps;
	size_t l;
	PakDirectory	* rf = this->fils;

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
				return rf->GetSousRepertoire(sname + l);
			}
		}

		rf = rf->brothernext;
	}

	delete[] fdir;
	return NULL;
}
//#############################################################################
PakFile * PakDirectory::AddFileToSousRepertoire(const char * sname, const char * name)
{
	PakDirectory * r;

	if (!sname)
	{
		r = this;
	}
	else
	{
		r = this->GetSousRepertoire(sname);

		if (!r)
		{
			return NULL;
		}
	}

	PakFile * f = r->fichiers;
	unsigned int nb = r->nbfiles;

	while (nb--)
	{
		if (!strcasecmp(f->name, name)) return NULL;

		f = f->fnext;
	}

	f = new PakFile(name);

	if (!f) return NULL;

	f->fprev = NULL;
	f->fnext = r->fichiers;

	if (f->fnext) f->fnext->fprev = f;

	r->fichiers = f;
	r->nbfiles++;

	//on l'insert dans la table de hachage
	if (r->pHachage)
	{
		r->pHachage->AddString((char *)name, (void *)f);
	}

	return f;
}
//#############################################################################
void PakDirectory::ConstructFullNameRepertoire(char * t)
{
	if (parent)
	{
		parent->ConstructFullNameRepertoire(t);
	}

	if (name)
	{
		strcat((char *)t, (const char *)name);
	}
}
//#############################################################################
void Kill(PakDirectory * r)
{
	PakFile * f = r->fichiers;

	while (r->nbfiles--)
	{
		PakFile * fnext = f->fnext;
		delete f;
		f = fnext;
	}

	r->fichiers = NULL;

	PakDirectory * brep = r->fils;
	int nb = r->nbsousreps;

	while (nb--)
	{
		PakDirectory * brepnext = brep->brothernext;
		Kill(brep);
		brep = brepnext;
	}

	r->fils = NULL;
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
//#############################################################################
char * EVEF_GetDirName(const char * dirplusname)
{
	size_t l = strlen(dirplusname);

	char * dir = new char[l+1];
	char * dirc = dir;

	if (!dir) return NULL;

	strcpy(dir, dirplusname);

	dirc += l;

	while ((l--) &&
	        (*dirc != '\\' && *dirc != '/'))
	{
		dirc--;
	}

	if (l)
	{
		*dirc = 0;
	}
	else
	{
		delete[] dir;
		return NULL;
	}

	char * dirf = new char[strlen(dir)+1];

	if (!dirf)
	{
		delete[] dir;
		return NULL;
	}

	strcpy(dirf, dir);

	delete[] dir;
	return dirf;
}
//#############################################################################
char * EVEF_GetFileName(const char * dirplusname)
{
	size_t l = strlen(dirplusname);
	dirplusname += l;

	while ((--l) &&
	        (*dirplusname != '\\' && *dirplusname != '/'))
	{
		dirplusname--;
	}

	if (l >= 0) dirplusname++;

	char * fname = new char[strlen(dirplusname)+1];

	if (!fname) return NULL;

	strcpy(fname, dirplusname);

	return fname;
}
