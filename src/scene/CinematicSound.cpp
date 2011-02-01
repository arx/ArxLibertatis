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

#include "animation/Cinematic.h"
#include "core/Application.h"
#include "scene/GameSound.h"
#include "io/IO.h"


/*-----------------------------------------------------------*/
CinematicSound		TabSound[MAX_SOUND];
int			NbSound;
/*-----------------------------------------------------------*/
extern HWND HwndPere;
extern char DirectoryChoose[];
extern int	LSoundChoose;

/*-----------------------------------------------------------*/
void InitSound(Cinematic * c)
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
	int			l;

	cs = &TabSound[num];

	if (!cs->active) return false;

	l = 0;

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
/*-----------------------------------------------------------*/
void CutAndAddString(std::string & dest, char * pText, const char * pDebText)
{
	int	i = strlen(pText);
	int j = strlen(pDebText);
	bool bOk = false;

	while (i--)
	{
		if (!strncasecmp(pText, pDebText, j))
		{
			bOk = true;
			break;
		}

		pText++;
	}

	if (bOk)
	{
		dest += pText;
	}
}
/*-----------------------------------------------------------*/
int ExistSound(char * dir, char * name)
{
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

/*-----------------------------------------------------------*/
/* TODO Figure out what this is supposed to do */
void PatchReplace(std::string & path)
{
	char CopyTxt[256];
	int j = path.length();
	//char * pT = path.c_str();
/*
	while (j--)
	{
		if (!strncasecmp(path.c_str(), "uk", strlen("uk")))
		{
			*pT = 0;
			strcpy(CopyTxt, pT + 3);
			strcat(path, "english\\");
			strcat(path, CopyTxt);
			break;
		}

		if (!strncasecmp(pT, "fr", strlen("fr")))
		{
			*pT = 0;
			strcpy(CopyTxt, pT + 3);
			strcat(path, "francais\\");
			strcat(path, CopyTxt);
			break;
		}

		//pT++;
	//}
*/
	ClearAbsDirectory(path, "ARX\\");

	//on enleve "sfx"
	bool bFound = false;
	//pT = path;
	//j = strlen((const char *)pT);
/*
	while (j)
	{
		if (!strncasecmp((const char *)pT, "sfx\\speech\\", strlen((const char *)"sfx\\speech\\")))
		{
			bFound = true;
			break;
		}
*/
		//j--;
		//pT++;
	//}

	/*if (bFound)
	{
		memmove((void *)pT, (const void *)(pT + 4), strlen((const char *)(pT + 4)) + 1);
	}*/

	//UNIQUEMENT EN MODE GAME!!!!!!
	std::string pcTxt = strstr(path.c_str(), "speech\\");

	if (!pcTxt.empty())
	{
		pcTxt = pcTxt.substr( strlen("speech\\") );
		char * pcTxt2 = strdup(pcTxt.c_str());
		char * pcTxt3 = pcTxt2;

		while (*pcTxt3 != '\\')
		{
			pcTxt3++;
		}

		pcTxt.clear();
		pcTxt += Project.localisationpath;
		pcTxt += "\\";
		pcTxt += (pcTxt3 + 1);

		free((void *)pcTxt2);
	}
}

/*-----------------------------------------------------------*/
int AddSoundToList(char * dir, char * name, int id, int pos)
{
	CinematicSound * cs;
	int		num;

	if ((num = ExistSound(dir, name)) >= 0)
	{
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

	cs->dir = (char *)malloc(strlen(dir) + 1);

	if (!cs->dir) return -1;

	strcpy(cs->dir, dir);

	cs->name = (char *)malloc(strlen(name) + 1);

	if (!cs->name)
	{
		free((void *)cs->dir);
		return -1;
	}

	strcpy(cs->name, name);

	std::string path = "\\\\Arkaneserver\\public\\ARX\\";
	CutAndAddString(path, dir, "sfx");
	path += name;
	PatchReplace(path);

	MakeUpcase(path);

	if ( path.find("SFX") != std::string::npos )
	{
		cs->sound = strdup(path.c_str());
	}
	else
	{
		char szTemp[1024];
		ZeroMemory(szTemp, 1024);

		sprintf(szTemp, "speech\\%s\\%s", Project.localisationpath.c_str(), name);
		cs->sound = strdup(szTemp);
	}

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
