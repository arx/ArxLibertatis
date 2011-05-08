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

#include "scene/CinematicSound.h"

#include <climits>

#include "animation/Cinematic.h"
#include "core/Application.h"
#include "io/FilePath.h"
#include "io/Logger.h"
#include "platform/String.h"
#include "scene/GameSound.h"

using std::string;


CinematicSound		TabSound[MAX_SOUND];
int			NbSound;
/*-----------------------------------------------------------*/
extern HWND HwndPere;
extern char DirectoryChoose[];
extern int	LSoundChoose;

/*-----------------------------------------------------------*/
void InitSound()
{
	CinematicSound	*	ts;
	int			nb;

	ts = TabSound;
	nb = MAX_SOUND;

	while (nb)
	{
		memset((void *)ts, 0, sizeof(CinematicSound));
		ts->idhandle = ARX_SOUND_INVALID_RESOURCE;
		ts++;
		nb--;
	}

	NbSound = 0;
}
/*-----------------------------------------------------------*/
CinematicSound * GetFreeSound(int * num)
{
	CinematicSound	*	ts;
	int			nb;

	ts = TabSound;
	nb = MAX_SOUND;

	while (nb)
	{
		if (!ts->active)
		{
			*num = MAX_SOUND - nb;
			return ts;
		}

		ts++;
		nb--;
	}

	return NULL;
}
/*-----------------------------------------------------------*/
bool DeleteFreeSound(int num)
{
	CinematicSound	*	cs;

	cs = &TabSound[num];

	if (!cs->active) return false;

	if (cs->dir)
	{
		free((void *)cs->dir);
		cs->dir = NULL;
	}

	if (cs->name)
	{
		free((void *)cs->name);
		cs->name = NULL;
	}

	if (cs->sound)
	{
		free((void *)cs->sound);
		cs->sound = NULL;
	}

	cs->active = 0;
	NbSound--;

	return true;
}
/*-----------------------------------------------------------*/
void DeleteAllSound(void)
{
	int	nb;

	nb = MAX_SOUND;

	while (nb)
	{
		DeleteFreeSound(MAX_SOUND - nb);
		nb--;
	}
}

int ExistSound(const string & dir, const string & name) {
	
	CinematicSound * cs;
	
	cs = TabSound;
	int nb = MAX_SOUND;
	
	while (nb)
	{
		if ((cs->active) &&
		        ((cs->active & 0xFF00) == LSoundChoose))
		{
			if (!strcasecmp(dir, cs->dir))
			{
				if (!strcasecmp(name, cs->name))
				{
					return MAX_SOUND - nb;
				}
			}
		}
		
		cs++;
		nb--;
	}

	return -1;
}

int AddSoundToList(const std::string & path) {
	int id = -1;
	
	CinematicSound * cs;
	
	string dir = path;
	RemoveName(dir);
	string name = GetName(path);
	
	int num = ExistSound(dir, name);
	if(num >= 0) {
		return num;
	}

	if (id >= 0)
	{
		cs = &TabSound[id];

		if (!cs->active || cs->load) return -1;

		free((void *)cs->name);
		free((void *)cs->dir);
		NbSound--;
	}
	else
	{
		cs = GetFreeSound(&num);

		if (!cs) return -1;
	}

	cs->dir = strdup(dir.c_str());
	if(!cs->dir) {
		return -1;
	}
	
	cs->name = strdup(name.c_str());
	if(!cs->name) {
		free(cs->dir);
		return -1;
	}
	
	string uppath = path;
	MakeUpcase(uppath);
	LogDebug << "adding cinematic sound " << uppath;
	cs->sound = strdup(uppath.c_str());
	
	cs->load = 1;
	
	int iActive = 1 | LSoundChoose;
	ARX_CHECK_SHORT(iActive);
	
	cs->active = static_cast<short>(iActive);
	
	NbSound++;
	return num;
}
/*-----------------------------------------------------------*/
bool PlaySoundKeyFramer(int id)
{
	CinematicSound * cs;

	cs = &TabSound[id];

	if (!cs->active) return false;

	cs->idhandle = ARX_SOUND_PlayCinematic(cs->sound);

	return true;
}
/*-----------------------------------------------------------*/
void StopSoundKeyFramer(void)
{
	CinematicSound	*	ts;
	int			nb;

	ts = TabSound;
	nb = MAX_SOUND;

	while (nb)
	{
		if (ts->active)
		{
			ARX_SOUND_Stop(ts->idhandle);
			ts->idhandle = ARX_SOUND_INVALID_RESOURCE;
		}

		ts++;
		nb--;
	}
}
