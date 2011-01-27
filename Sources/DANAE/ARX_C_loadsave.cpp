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

#include <stdlib.h>
#include "arx_c_cinematique.h"
#include "HERMES_PAK.h"
#include "resource.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

/*----------------------------------------------------------------------*/
void DrawInfoTrack(void);
/*----------------------------------------------------------------------*/
static FILE * FCurr;
static char Dir[256];
static char Name[256];
/*----------------------------------------------------------------------*/
extern char		AllTxt[];
extern int		NbBitmap;
extern C_BITMAP	TabBitmap[];
extern int		NbBitmap;
extern C_TRACK	* CKTrack;
char FileNameChoose[256];
extern BOOL Restore;
extern HWND HwndPere;
extern int		NbSound;
extern C_SOUND	TabSound[];
extern C_KEY KeyTemp;
extern int LSoundChoose;

void ClearDirectory(char * dirfile);
void GetPathDirectory(char * dirfile);
void ClearAbsDirectory(char * pT, char * d);
void AddDirectory(char * pT, char * dir);

/*----------------------------------------------------------------------*/
void ReadString(char * d)
{
	while (1)
	{
		PAK_fread(d, 1, 1, FCurr);

		if (!*d) break;

		d++;
	}
}
/*----------------------------------------------------------------------*/
BOOL LoadProject(CINEMATIQUE * c, char * dir, char * name)
{
	int		nb, version;
	C_TRACK	t;
	C_KEY		k, *kk;
	C_KEY_1_59	k159;
	C_KEY_1_65	k165;
	C_KEY_1_70	k170;
	C_KEY_1_71	k171;
	C_KEY_1_72	k172;
	C_KEY_1_74	k174;
	C_KEY_1_75	k175;
	char	txt[4];

	InitMapLoad(c);
	InitSound(c);

	strcpy(AllTxt, dir);
	strcat(AllTxt, name);
	FCurr = PAK_fopen(AllTxt, "rb");

	if (!FCurr) return FALSE;

	ReadString(txt);

	if (strcmp(txt, "KFA"))
	{
		PAK_fclose(FCurr);
		FCurr = NULL;
		c->New();
		return FALSE;
	}

	PAK_fread(&version, 4, 1, FCurr);

	if (version > VERSION)
	{
		PAK_fclose(FCurr);
		FCurr = NULL;
		c->New();
		return FALSE;
	}

	if (version >= ((1 << 16) | 61))
	{
		char txt[256];
		ReadString(txt);
	}

	//chargement image
	PAK_fread(&nb, 1, 4, FCurr);

	while (nb)
	{
		int echelle = 0;

		if (version >= ((1 << 16) | 71))
		{
			PAK_fread((void *)&echelle, 4, 1, FCurr);
		}

		ReadString(AllTxt);
		strcpy(Dir, AllTxt);
		GetPathDirectory(Dir);
		strcpy(Name, AllTxt);
		ClearDirectory(Name);
		strcpy(Name, FileNameChoose);

		int id = CreateAllMapsForBitmap(Dir, Name, c, -1, 0);

		if (TabBitmap[id].load)
		{
			if (echelle > 1)
			{
				TabBitmap[id].grille.echelle = echelle;
				c->ReInitMapp(id);
			}
			else
			{
				TabBitmap[id].grille.echelle = 1;
			}
		}
		else
		{
			TabBitmap[id].grille.echelle = 1;
		}

		nb--;
	}

	//chargement son
	LSoundChoose = C_LANGUAGE_FRENCH;

	if (version >= ((1 << 16) | 60))
	{
		PAK_fread(&nb, 1, 4, FCurr);

		while (nb)
		{
			if (version >= ((1 << 16) | 76))
			{
				short il;
				PAK_fread((void *)&il, 1, 2, FCurr);
				LSoundChoose = il;
			}

			ReadString(AllTxt);
			strcpy(Dir, AllTxt);
			GetPathDirectory(Dir);
			strcpy(Name, AllTxt);
			ClearDirectory(Name);
			strcpy(Name, FileNameChoose);

			AddSoundToList(Dir, Name, -1, 0);
			nb--;
		}
	}

	//chargement track + key
	PAK_fread(&t, 1, sizeof(C_TRACK) - 4, FCurr);
	AllocTrack(t.startframe, t.endframe, t.fps);

	nb = t.nbkey;

	while (nb)
	{
		if (version <= ((1 << 16) | 59))
		{
			PAK_fread(&k159, 1, sizeof(C_KEY_1_59), FCurr);
			k.angz = k159.angz;
			k.color = k159.color;
			k.colord = k159.colord;
			k.colorf = k159.colorf;
			k.frame = k159.frame;
			k.fx = k159.fx;
			k.numbitmap = k159.numbitmap;
			k.pos = k159.pos;
			k.speed = k159.speed;

			ARX_CHECK_SHORT(k159.typeinterp);
			k.typeinterp = ARX_CLEAN_WARN_CAST_SHORT(k159.typeinterp);
			k.force = 1;
			k.idsound[C_LANGUAGE_FRENCH] = -1;
			k.light.intensite = -1.f;
			k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
			k.angzgrille = 0.f;
			k.speedtrack = 1.f;
		}
		else
		{
			if (version <= ((1 << 16) | 65))
			{
				PAK_fread(&k165, 1, sizeof(C_KEY_1_65), FCurr);
				k.angz = k165.angz;
				k.color = k165.color;
				k.colord = k165.colord;
				k.colorf = k165.colorf;
				k.frame = k165.frame;
				k.fx = k165.fx;
				k.numbitmap = k165.numbitmap;
				k.pos = k165.pos;
				k.speed = k165.speed;

				ARX_CHECK_SHORT(k165.typeinterp);
				k.typeinterp = ARX_CLEAN_WARN_CAST_SHORT(k165.typeinterp);
				k.force = 1;
				k.idsound[C_LANGUAGE_FRENCH] = k165.idsound;
				k.light.intensite = -1.f;
				k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
				k.angzgrille = 0.f;
				k.speedtrack = 1.f;
			}
			else
			{
				if (version <= ((1 << 16) | 70))
				{
					PAK_fread(&k170, 1, sizeof(C_KEY_1_70), FCurr);
					k.angz = k170.angz;
					k.color = k170.color;
					k.colord = k170.colord;
					k.colorf = k170.colorf;
					k.frame = k170.frame;
					k.fx = k170.fx;
					k.numbitmap = k170.numbitmap;
					k.pos = k170.pos;
					k.speed = k170.speed;
					k.typeinterp = k170.typeinterp;
					k.force = k170.force;
					k.idsound[C_LANGUAGE_FRENCH] = k170.idsound;
					k.light.intensite = -1.f;
					k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
					k.angzgrille = 0.f;
					k.speedtrack = 1.f;
				}
				else
				{
					if (version <= ((1 << 16) | 71))
					{
						PAK_fread(&k171, 1, sizeof(C_KEY_1_71), FCurr);
						k.angz = k171.angz;
						k.color = k171.color;
						k.colord = k171.colord;
						k.colorf = k171.colorf;
						k.frame = k171.frame;
						k.fx = k171.fx;
						k.numbitmap = k171.numbitmap;
						k.pos = k171.pos;
						k.speed = k171.speed;
						k.typeinterp = k171.typeinterp;
						k.force = k171.force;
						k.idsound[C_LANGUAGE_FRENCH] = k171.idsound;
						k.light = k171.light;

						if ((k.fx & 0xFF000000) != FX_LIGHT)
						{
							k.light.intensite = -1.f;
						}

						k.posgrille.x = k.posgrille.y = k.posgrille.z = 0.f;
						k.angzgrille = 0.f;
						k.speedtrack = 1.f;
					}
					else
					{
						if (version <= ((1 << 16) | 72))
						{
							PAK_fread(&k172, 1, sizeof(C_KEY_1_72), FCurr);
							k.angz = k172.angz;
							k.color = k172.color;
							k.colord = k172.colord;
							k.colorf = k172.colorf;
							k.frame = k172.frame;
							k.fx = k172.fx;
							k.numbitmap = k172.numbitmap;
							k.pos = k172.pos;
							k.speed = k172.speed;
							k.typeinterp = k172.typeinterp;
							k.force = k172.force;
							k.idsound[C_LANGUAGE_FRENCH] = k172.idsound;
							k.light.pos = k172.light.pos;
							k.light.fallin = k172.light.fallin;
							k.light.fallout = k172.light.fallout;
							k.light.r = k172.light.r;
							k.light.g = k172.light.g;
							k.light.b = k172.light.b;
							k.light.intensite = k172.light.intensite;
							k.light.intensiternd = k172.light.intensiternd;
							k.posgrille.x = k172.posgrille.x;
							k.posgrille.y = k172.posgrille.y;
							k.posgrille.z = k172.posgrille.z;
							k.angzgrille = k172.angzgrille;
							k.speedtrack = 1.f;

							if ((k.fx & 0xFF000000) != FX_LIGHT)
							{
								k.light.intensite = -1.f;
							}
						}
						else
						{
							if (version <= ((1 << 16) | 74))
							{
								PAK_fread(&k174, 1, sizeof(C_KEY_1_74), FCurr);
								k.angz = k174.angz;
								k.color = k174.color;
								k.colord = k174.colord;
								k.colorf = k174.colorf;
								k.frame = k174.frame;
								k.fx = k174.fx;
								k.numbitmap = k174.numbitmap;
								k.pos = k174.pos;
								k.speed = k174.speed;
								k.typeinterp = k174.typeinterp;
								k.force = k174.force;
								k.idsound[C_LANGUAGE_FRENCH] = k174.idsound;
								k.light.pos = k174.light.pos;
								k.light.fallin = k174.light.fallin;
								k.light.fallout = k174.light.fallout;
								k.light.r = k174.light.r;
								k.light.g = k174.light.g;
								k.light.b = k174.light.b;
								k.light.intensite = k174.light.intensite;
								k.light.intensiternd = k174.light.intensiternd;
								k.posgrille.x = k174.posgrille.x;
								k.posgrille.y = k174.posgrille.y;
								k.posgrille.z = k174.posgrille.z;
								k.angzgrille = k174.angzgrille;
								k.speedtrack = 1.f;
							}
							else
							{
								if (version <= ((1 << 16) | 75))
								{
									PAK_fread(&k175, 1, sizeof(C_KEY_1_75), FCurr);
									k.angz = k175.angz;
									k.color = k175.color;
									k.colord = k175.colord;
									k.colorf = k175.colorf;
									k.frame = k175.frame;
									k.fx = k175.fx;
									k.numbitmap = k175.numbitmap;
									k.pos = k175.pos;
									k.speed = k175.speed;
									k.typeinterp = k175.typeinterp;
									k.force = k175.force;
									k.idsound[C_LANGUAGE_FRENCH] = k175.idsound;
									k.light.pos = k175.light.pos;
									k.light.fallin = k175.light.fallin;
									k.light.fallout = k175.light.fallout;
									k.light.r = k175.light.r;
									k.light.g = k175.light.g;
									k.light.b = k175.light.b;
									k.light.intensite = k175.light.intensite;
									k.light.intensiternd = k175.light.intensiternd;
									k.posgrille.x = k175.posgrille.x;
									k.posgrille.y = k175.posgrille.y;
									k.posgrille.z = k175.posgrille.z;
									k.angzgrille = k175.angzgrille;
									k.speedtrack = k175.speedtrack;
								}
								else
								{
									PAK_fread(&k, 1, sizeof(C_KEY), FCurr);
								}
							}
						}
					}
				}
			}
		}

		if (version <= ((1 << 16) | 67))
		{
			if (k.typeinterp == INTERP_NO_FADE)
			{
				k.typeinterp = INTERP_NO;
				k.force = 1;
			}
		}

		if (version <= ((1 << 16) | 73))
		{
			k.light.pos.x = k.light.pos.y = k.light.pos.z = 0.f;
		}

		if (version <= ((1 << 16) | 75))
		{
			for (int i = 1; i < 16; i++) k.idsound[i] = -1;
		}

		if (k.force < 0) k.force = 1;

		FillKeyTemp(&k.pos, k.angz, k.frame, k.numbitmap, k.fx, k.typeinterp, k.color, k.colord, k.colorf, k.speed, -1, k.force, &k.light, &k.posgrille, k.angzgrille, k.speedtrack);
		memcpy(&KeyTemp.idsound, &k.idsound, 16 * 4);
		AddKeyLoad(&KeyTemp);

		if (!(t.nbkey - nb))
		{
			c->pos = k.pos;
			c->angz = k.angz;
			c->numbitmap = k.numbitmap;
			c->fx = k.fx;
			c->ti = c->tichoose = k.typeinterp;
			c->color = c->colorchoose = k.color;
			c->colord = c->colorchoosed = k.colord;
			c->colorflash = c->colorflashchoose = k.colorf;
			c->speed = c->speedchoose = k.speed;
			c->idsound = k.idsound[C_LANGUAGE_FRENCH];
			c->force = k.force;
			c->light = c->lightchoose = k.light;
			c->posgrille = k.posgrille;
			c->angzgrille = k.angzgrille;
			c->speedtrack = k.speedtrack;
		}

		nb--;
	}

	UpDateAllKeyLight();

	PAK_fclose(FCurr);
	FCurr = NULL;

	ActiveAllTexture(c);

	SetCurrFrame(0);

	GereTrackNoPlay(c);
	c->projectload = TRUE;

	InitUndo();

	//precalc
	if (version < ((1 << 16) | 71))
	{
		kk = CKTrack->key;
		nb = CKTrack->nbkey;

		while (nb)
		{
			switch (kk->fx & 0x0000FF00)
			{
				case FX_DREAM:
					TabBitmap[kk->numbitmap].grille.echelle = 4;
					c->ReInitMapp(kk->numbitmap);
					break;
			}

			nb--;
			kk++;
		}
	}

	LSoundChoose = C_LANGUAGE_ENGLISH << 8;

	return TRUE;
}
