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
#include <stdio.h>
#include "arx_c_cinematique.h"
#include "resource.h"
#include "arx_sound.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

/*-----------------------------------------------------------*/
C_SOUND		TabSound[MAX_SOUND];
int			NbSound;
/*-----------------------------------------------------------*/
extern char AllTxt[];
extern HWND HwndPere;
extern char DirectoryChoose[];
extern int	LSoundChoose;

extern char		DirectoryAbs[];

extern ULONG g_pak_workdir_len;

void ClearAbsDirectory(char * pT, char * d);
void AddDirectory(char * pT, char * dir);
/*-----------------------------------------------------------*/
void InitSound(CINEMATIQUE * c)
{
	C_SOUND	*	ts;
	int			nb;

	ts = TabSound;
	nb = MAX_SOUND;

	while (nb)
	{
		memset((void *)ts, 0, sizeof(C_SOUND));
		ts->idhandle = ARX_SOUND_INVALID_RESOURCE;
		ts++;
		nb--;
	}

	NbSound = 0;
}
/*-----------------------------------------------------------*/
C_SOUND * GetFreeSound(int * num)
{
	C_SOUND	*	ts;
	int			nb;

	ts = TabSound;
	nb = MAX_SOUND;

	while (nb)
	{
		if (!ts->actif)
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
BOOL DeleteFreeSound(int num)
{
	C_SOUND	*	cs;
	int			l;

	cs = &TabSound[num];

	if (!cs->actif) return FALSE;

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

	cs->actif = 0;
	NbSound--;

	return TRUE;
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
void CutAndAddString(char * pText, char * pDebText)
{
	int	i = strlen(pText);
	int j = strlen(pDebText);
	bool bOk = false;

	while (i--)
	{
		if (!strnicmp(pText, pDebText, j))
		{
			bOk = true;
			break;
		}

		pText++;
	}

	if (bOk)
	{
		strcat(AllTxt, pText);
	}
}
/*-----------------------------------------------------------*/
int ExistSound(char * dir, char * name)
{
	C_SOUND * cs;

	cs = TabSound;
	int nb = MAX_SOUND;

	while (nb)
	{
		if ((cs->actif) &&
		        ((cs->actif & 0xFF00) == LSoundChoose))
		{
			if (!stricmp(dir, cs->dir))
			{
				if (!stricmp(name, cs->name))
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
void PatchReplace()
{
	char CopyTxt[256];
	int j = strlen(AllTxt);
	char * pT = AllTxt;

	while (j--)
	{
		if (!strnicmp(pT, "uk", strlen("uk")))
		{
			*pT = 0;
			strcpy(CopyTxt, pT + 3);
			strcat(AllTxt, "english\\");
			strcat(AllTxt, CopyTxt);
			break;
		}

		if (!strnicmp(pT, "fr", strlen("fr")))
		{
			*pT = 0;
			strcpy(CopyTxt, pT + 3);
			strcat(AllTxt, "francais\\");
			strcat(AllTxt, CopyTxt);
			break;
		}

		pT++;
	}

	ClearAbsDirectory(AllTxt, "arx\\");
	AddDirectory(AllTxt, DirectoryAbs);

	//on enleve "sfx"
	bool bFound = false;
	pT = AllTxt;
	j = strlen((const char *)pT);

	while (j)
	{
		if (!strnicmp((const char *)pT, "sfx\\speech\\", strlen((const char *)"sfx\\speech\\")))
		{
			bFound = true;
			break;
		}

		j--;
		pT++;
	}

	if (bFound)
	{
		memmove((void *)pT, (const void *)(pT + 4), strlen((const char *)(pT + 4)) + 1);
	}

	//UNIQUEMENT EN MODE GAME!!!!!!
	char * pcTxt = strstr(AllTxt, "speech\\");

	if (pcTxt)
	{
		pcTxt += strlen("speech\\");
		char * pcTxt2 = strdup(pcTxt);
		char * pcTxt3 = pcTxt2;

		while (*pcTxt3 != '\\')
		{
			pcTxt3++;
		}

		*pcTxt = 0;
		strcat(pcTxt, Project.localisationpath);
		strcat(pcTxt, "\\");
		strcat(pcTxt, pcTxt3 + 1);

		free((void *)pcTxt2);
	}
}

/*-----------------------------------------------------------*/
int AddSoundToList(char * dir, char * name, int id, int pos)
{
	C_SOUND * cs;
	int		num;

	if ((num = ExistSound(dir, name)) >= 0)
	{
		return num;
	}

	if (id >= 0)
	{
		cs = &TabSound[id];

		if (!cs->actif || cs->load) return -1;

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

	strcpy(AllTxt, "\\\\Arkaneserver\\public\\ARX\\");
	CutAndAddString(dir, "sfx");
	strcat(AllTxt, name);
	PatchReplace();

	strupr(AllTxt);

	if (strstr(AllTxt, "SFX"))
	{
		cs->sound = strdup(AllTxt);
	}
	else
	{
		char szTemp[1024];
		ZeroMemory(szTemp, 1024);

		sprintf(szTemp, "%sspeech\\%s\\%s", Project.workingdir, Project.localisationpath, name);
		cs->sound = strdup(szTemp);
	}

	cs->load = 1;



	int iActif = 1 | LSoundChoose;
	ARX_CHECK_SHORT(iActif);

	cs->actif = ARX_CLEAN_WARN_CAST_SHORT(iActif);


	NbSound++;
	return num;
}
/*-----------------------------------------------------------*/
BOOL PlaySoundKeyFramer(int id)
{
	C_SOUND * cs;

	cs = &TabSound[id];

	if (!cs->actif) return FALSE;

	cs->idhandle = ARX_SOUND_PlayCinematic(cs->sound + g_pak_workdir_len);

	return TRUE;
}
/*-----------------------------------------------------------*/
void StopSoundKeyFramer(void)
{
	C_SOUND	*	ts;
	int			nb;

	ts = TabSound;
	nb = MAX_SOUND;

	while (nb)
	{
		if (ts->actif)
		{
			ARX_SOUND_Stop(ts->idhandle);
			ts->idhandle = ARX_SOUND_INVALID_RESOURCE;
		}

		ts++;
		nb--;
	}
}
