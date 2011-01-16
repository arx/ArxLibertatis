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
#include <stdlib.h>
#include "arx_c_cinematique.h"
#include "arx_time.h"
#include "Resource.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

/*----------------------------------------------------------------------*/
//undo/redo
#define UNDOPILE	256*2
#define PILE_ADD	1
#define PILE_DEL	2
#define PILE_MOD	3

typedef struct
{
	int		action;
	C_KEY	key;
} C_UNDOPILE;
/*----------------------------------------------------------------------*/
extern HWND HwndPere;
extern BOOL ProjectModif;
/*----------------------------------------------------------------------*/
static BOOL GereTrackNoPlay2(C_KEY * k, int frame);
static BOOL DeleteKey2(CINEMATIQUE * c, int frame);
/*----------------------------------------------------------------------*/
C_TRACK	* CKTrack;

int UndoPile;
static int TotUndoPile, FillUndo;
static C_UNDOPILE UndoKey[UNDOPILE];
extern int LSoundChoose;
/*----------------------------------------------------------------------*/
BOOL AllocTrack(int sf, int ef, float fps)
{
	if (CKTrack) return FALSE;

	CKTrack = (C_TRACK *)malloc(sizeof(C_TRACK));

	if (!CKTrack) return FALSE;

	CKTrack->startframe = sf;
	CKTrack->endframe = ef;
	CKTrack->currframe = 0.f;
	CKTrack->fps = fps;
	CKTrack->nbkey = 0;
	CKTrack->key = NULL;
	CKTrack->pause = TRUE;

	InitUndo();

	return TRUE;
}
/*----------------------------------------------------------------------*/
BOOL DeleteTrack(void)
{
	if (!CKTrack) return FALSE;

	if (CKTrack->key) free((void *)CKTrack->key);

	free((void *)CKTrack);
	CKTrack = NULL;

	return TRUE;
}
/*----------------------------------------------------------------------*/
static C_KEY * SearchAndMoveKey(int f)
{
	int		nb;
	C_KEY	* k;

	k = CKTrack->key + CKTrack->nbkey - 1;
	nb = CKTrack->nbkey;

	while (nb)
	{
		if (f > k->frame) break;

		k--;
		nb--;
	}

	nb = CKTrack->nbkey - nb;

	if (nb)
	{
		memmove((void *)(k + 2), (void *)(k + 1), sizeof(C_KEY)*nb);
	}

	return k + 1;
}
/*----------------------------------------------------------------------*/
C_KEY * SearchKey(int f, int * num)
{
	int		nb;
	C_KEY	* k;

	if (!CKTrack || !CKTrack->nbkey) return NULL;

	k = CKTrack->key;
	nb = CKTrack->nbkey;

	while (nb)
	{
		if (f == k->frame)
		{
			*num = CKTrack->nbkey - nb;
			return k;
		}

		k++;
		nb--;
	}

	return NULL;
}
/*----------------------------------------------------------------------*/
static void AddPileUndo(C_KEY * k, int type)
{
	C_UNDOPILE	* up;

	if (!FillUndo) return;

	if (UndoPile >= UNDOPILE)
	{
		memmove(UndoKey, UndoKey + 2, sizeof(C_UNDOPILE)*(UNDOPILE - 2));
		UndoPile -= 2;
	}

	up = &UndoKey[UndoPile];
	UndoPile++;

	TotUndoPile = UndoPile;
	up->action = type;
	up->key = *k;
}
/*----------------------------------------------------------------------*/
void InitUndo(void)
{
	int			nb;
	C_UNDOPILE	* up;

	UndoPile = TotUndoPile = 0;
	up = UndoKey;
	nb = UNDOPILE;

	while (nb)
	{
		up->action = 0;
		up++;
		nb--;
	}

	FillUndo = TRUE;
}
 
/*----------------------------------------------------------------------*/
void UpDateKeyLight(int frame)
{
	C_KEY	* k, *klightprev, *klightnext, *klightprev2, *klightnext2;
	int		num;

	k = SearchKey(frame, &num);
	klightprev = klightnext = k;
	
	C_KEY * kbase = k;
	int num2 = num;

	//on cherche le range de deux lights
	//prev
	while (num2)
	{
		k--;

		if ((k->fx & 0xFF000000) == FX_LIGHT)
		{
			klightprev = k;
			break;
		}

		num2--;
	}

	//next
	k = kbase;
	num2 = num;

	while (num2 < (CKTrack->nbkey - 1))
	{
		k++;

		if ((k->fx & 0xFF000000) == FX_LIGHT)
		{
			klightnext = k;
			break;
		}

		num2++;
	}

	//on crépie le range avec ces deux valeurs
	kbase->light.prev = klightprev;
	kbase->light.next = klightnext;

	if ((kbase->fx & 0xFF000000) == FX_LIGHT)
	{
		klightprev2 = klightnext2 = kbase;
	}
	else
	{
		kbase->light.intensite = -1.f;
		klightprev2 = klightprev;
		klightnext2 = klightnext;
	}

	//prev
	k = kbase - 1;

	while (k >= CKTrack->key)
	{
		if (klightprev == kbase)
		{
			k->light.intensite = -1.f;
		}

		k->light.next = klightnext2;

		if ((k->fx & 0xFF000000) == FX_LIGHT) break;

		k->light.prev = klightprev;
		k--;
	}

	//next
	k = kbase + 1;

	while (k < (CKTrack->key + CKTrack->nbkey))
	{
		if (klightnext == kbase)
		{
			k->light.intensite = -1.f;
		}

		k->light.prev = klightprev2;

		if ((k->fx & 0xFF000000) == FX_LIGHT) break;

		k->light.next = klightnext;
		k++;
	}
}
/*----------------------------------------------------------------------*/
void UpDateAllKeyLight(void)
{
	C_KEY	* kk;
	int		nb;

	//update les lights
	kk = CKTrack->key;
	nb = CKTrack->nbkey;

	while (nb--)
	{
		UpDateKeyLight(kk->frame);
		kk++;
	}
}
/*----------------------------------------------------------------------*/
BOOL AddKey(C_KEY * key, BOOL writecolor, BOOL writecolord, BOOL writecolorf)
{
	C_KEY	*	k;
	int			num;

	if (!CKTrack || (key->frame < CKTrack->startframe) || (key->frame > CKTrack->endframe)) return FALSE;

	if (!(k = SearchKey(key->frame, &num)))
	{
		if (!CKTrack->nbkey)
		{
			CKTrack->key = k = (C_KEY *)malloc(sizeof(C_KEY));
		}
		else
		{
			CKTrack->key = (C_KEY *)realloc(CKTrack->key, sizeof(C_KEY) * (CKTrack->nbkey + 1));
			k = SearchAndMoveKey(key->frame);
		}

		CKTrack->nbkey++;

		k->frame = key->frame;
		AddPileUndo(k, PILE_ADD);
	}
	else
	{
		AddPileUndo(k, PILE_MOD);
	}

	if (key->numbitmap > -2) k->numbitmap = key->numbitmap;

	if (key->fx > -2)
	{
		if ((key->fx > 255) && (k->fx > 0))
		{
			k->fx |= key->fx;
		}
		else
		{
			if ((k->fx >= 255) && (key->fx >= 0))
			{
				k->fx |= key->fx;
			}
			else
			{
				k->fx = key->fx;
			}
		}
	}

	if (key->speed > -1.f)
	{
		k->speed = key->speed;
	}

	if (writecolor) k->color = key->color;

	if (writecolord) k->colord = key->colord;

	if (writecolorf) k->colorf = key->colorf;

	if (key->idsound[LSoundChoose>>8] > -2) k->idsound[LSoundChoose>>8] = key->idsound[LSoundChoose>>8];

	if (key->force > -2) k->force = key->force;

	k->frame = key->frame;
	k->pos = key->pos;
	k->angz = key->angz;

	if (key->typeinterp > -2) k->typeinterp = key->typeinterp;

	float a = -2.f;

	if (C_NEQUAL_F32(key->light.intensite, a))
	{
		k->light = key->light;
	}

	k->posgrille = key->posgrille;
	k->angzgrille = key->angzgrille;
	k->speedtrack = key->speedtrack;

	UpDateAllKeyLight();

	AddPileUndo(k, PILE_ADD);

	ProjectModif = TRUE;
	return TRUE;
}
/*----------------------------------------------------------------------*/
BOOL AddKeyLoad(C_KEY * key)
{
	C_KEY	*	k;
	int			num;

	if (!CKTrack || (key->frame < CKTrack->startframe) || (key->frame > CKTrack->endframe)) return FALSE;

	if (!(k = SearchKey(key->frame, &num)))
	{
		if (!CKTrack->nbkey)
		{
			CKTrack->key = k = (C_KEY *)malloc(sizeof(C_KEY));
		}
		else
		{
			CKTrack->key = (C_KEY *)realloc(CKTrack->key, sizeof(C_KEY) * (CKTrack->nbkey + 1));
			k = SearchAndMoveKey(key->frame);
		}

		CKTrack->nbkey++;

	}

	k->numbitmap = key->numbitmap;
	k->fx = key->fx;
	k->speed = key->speed;
	k->color = key->color;
	k->colord = key->colord;
	k->colorf = key->colorf;
	k->frame = key->frame;
	k->pos = key->pos;
	k->angz = key->angz;
	k->typeinterp = key->typeinterp;

	memcpy(k->idsound, key->idsound, 16 * 4);
	k->force = key->force;
	k->light = key->light;
	k->posgrille = key->posgrille;
	k->angzgrille = key->angzgrille;
	k->speedtrack = key->speedtrack;

	return TRUE;
}
/*----------------------------------------------------------------------*/
static BOOL DiffKey(C_KEY * key1, C_KEY * key2)
{
	return((key1->pos.x != key2->pos.x) || (key1->pos.y != key2->pos.y) || (key1->pos.z != key2->pos.z) ||
	       (key1->angz != key2->angz) ||
	       (key1->numbitmap != key2->numbitmap) ||
	       (key1->fx != key2->fx) ||
	       (key1->typeinterp != key2->typeinterp) ||
	       (key1->color != key2->color) || (key1->colord != key2->colord) || (key1->colorf != key2->colorf) ||
	       (key1->speed != key2->speed) ||
	       (key1->idsound[LSoundChoose>>8] != key2->idsound[LSoundChoose>>8]) ||
	       (key1->force != key2->force) ||
	       (key1->light.pos.x != key2->light.pos.x) || (key1->light.pos.y != key2->light.pos.y) || (key1->light.pos.z != key2->light.pos.z) ||
	       (key1->light.fallin != key2->light.fallin) || (key1->light.fallout != key2->light.fallout) ||
	       (key1->light.r != key2->light.r) || (key1->light.g != key2->light.g) || (key1->light.b != key2->light.b) ||
	       (key1->light.intensite != key2->light.intensite) || (key1->light.intensiternd != key2->light.intensiternd) ||
	       (key1->posgrille.x != key2->posgrille.x) || (key1->posgrille.y != key2->posgrille.y) || (key1->posgrille.z != key2->posgrille.z) ||
	       (key1->angzgrille != key2->angzgrille) ||
	       (key1->speedtrack != key2->speedtrack)
	      );
}
/*----------------------------------------------------------------------*/
void AddDiffKey(CINEMATIQUE * c, C_KEY * key, BOOL writecolor, BOOL writecolord, BOOL writecolorf)
{
	C_KEY	* k, *ksuiv;
	int		num;

	if (!CKTrack || !CKTrack->pause) return;

	k = GetKey((int)CKTrack->currframe, &num);

	if (!k) return;

	ksuiv = (num == CKTrack->nbkey) ? ksuiv = k : ksuiv = k + 1;

	if (DiffKey(k, key))
	{
		if (DiffKey(ksuiv, key))
		{
			key->frame = (int)CKTrack->currframe;
			AddKey(key, writecolor, writecolord, writecolorf);
			GereTrackNoPlay(c);
		}
	}
}
/*----------------------------------------------------------------------*/

C_KEY * GetKey(int f, int * num)
{
	int		nb;
	C_KEY	* k;

	if (!CKTrack || !CKTrack->key) return NULL;

	k = CKTrack->key + CKTrack->nbkey - 1;
	nb = CKTrack->nbkey;

	while (nb)
	{
		if (f >= k->frame)
		{
			*num = nb;
			return k;
		}

		k--;
		nb--;
	}

	return NULL;
}
/*----------------------------------------------------------------------*/
float GetAngleInterpolation(float d, float e)
{
	float	da;

	da = e - d;

	if (fabs(da) > 180.f)
	{
		if (da > 0.f) da -= 360.f;
		else da += 360.f;
	}

	return da;
}
extern char AllTxt[];
/*----------------------------------------------------------------------*/
BOOL GereTrack(CINEMATIQUE * c, float fpscurr)
{
	C_KEY	* k, *ksuiv;
	float	a, unmoinsa, alight = 0, unmoinsalight = 0;
	int		num;
	C_KEY	* kprec, *ksuivsuiv;
	float	t1, t2, t3, f0, f1, f2, f3, p0, p1, temp;
	C_KEY	* lightprec, *lightnext;

	if (!CKTrack || !CKTrack->nbkey) return FALSE;

	if (CKTrack->pause) return TRUE;

	k = GetKey((int)CKTrack->currframe, &num);
	ksuiv = (num == CKTrack->nbkey) ? ksuiv = k : ksuiv = k + 1;

	if (ksuiv->frame != k->frame)
		a = (CKTrack->currframe - (float)k->frame) / ((float)(ksuiv->frame - k->frame));
	else
		a = 1.f;

	c->a = unmoinsa = 1.0f - a;

	c->numbitmap		= k->numbitmap;
	c->numbitmapsuiv	= ksuiv->numbitmap;
	c->ti				= k->typeinterp;
	c->fx				= k->fx;
	c->fxsuiv			= ksuiv->fx;
	c->color			= k->color;
	c->colord			= k->colord;
	c->colorflash		= k->colorf;
	c->speed			= k->speed;
	c->idsound			= k->idsound[LSoundChoose>>8];
	c->force			= k->force;

	if ((k->fx & 0xFF000000) == FX_LIGHT)
	{
		lightprec = k;
	}
	else
	{
		lightprec = k->light.prev;
	}

	lightnext = k->light.next;
	c->lightd = lightnext->light;

	if ((lightprec != lightnext))
	{
		alight = (CKTrack->currframe - (float)lightprec->frame) / ((float)(lightnext->frame - lightprec->frame));

		if (alight > 1.f) alight = 1.f;

		unmoinsalight = 1.0f - alight;
	}
	else
	{
		if (k == (CKTrack->key + CKTrack->nbkey - 1))
		{
			alight			= 1.f;
			unmoinsalight	= 0.f;
		}
		else
		{

			//alight can't be used because it is not initialized
//ARX_BEGIN: jycorbel (2010-07-19) - Set light coeff to 0 to keep null all possibly light created from uninitialyzed var.
/*
alight = unmoinsalight = 0.f; //default values needed when : k->typeinterp == INTERP_BEZIER (0) || k->typeinterp == INTERP_LINEAR (1)
consequences on light :
				c->light : position = (0,0,0);
				c->light : color	= (0,0,0); == BLACK
				c->light : fallin	= fallout		= 0;
				c->light : intensite = intensiternd	= 0;
			ARX_CHECK( k->typeinterp != INTERP_BEZIER && k->typeinterp != INTERP_LINEAR );
*/
//ARX_END: jycorbel (2010-07-19)
			//ARX_END: jycorbel (2010-06-28)
			c->lightd = k->light;
			lightnext = lightprec = k;
		}
	}

	c->posgrille	  = k->posgrille;
	c->angzgrille	  = k->angzgrille;
	c->posgrillesuiv  = ksuiv->posgrille;
	c->angzgrillesuiv = ksuiv->angzgrille;

	switch (k->typeinterp)
	{
		case INTERP_NO:
			c->pos = k->pos;
			c->angz = k->angz;
			c->possuiv = ksuiv->pos;
			c->angzsuiv = ksuiv->angz;
			c->light = lightprec->light;
			c->speedtrack = k->speedtrack;
			break;
		case INTERP_LINEAR:
			c->pos.x = a * ksuiv->pos.x + unmoinsa * k->pos.x;
			c->pos.y = a * ksuiv->pos.y + unmoinsa * k->pos.y;
			c->pos.z = a * ksuiv->pos.z + unmoinsa * k->pos.z;

			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);

			c->speedtrack = a * ksuiv->speedtrack + unmoinsa * k->speedtrack;

			{
				C_LIGHT ldep;
				C_LIGHT lend;

				if (lightprec->light.intensite < 0.f)
				{
					c->light.intensite = -1;
					break;
				}
				else
				{
					ldep = lightprec->light;
				}

				if (c->lightd.intensite < 0.f)
				{
					break;
				}
				else
				{
					lend = c->lightd;
				}

				c->light.pos.x		= alight * lend.pos.x + unmoinsalight * ldep.pos.x;
				c->light.pos.y		= alight * lend.pos.y + unmoinsalight * ldep.pos.y;
				c->light.pos.z		= alight * lend.pos.z + unmoinsalight * ldep.pos.z;
				c->light.fallin		= alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout	= alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.r			= alight * lend.r + unmoinsalight * ldep.r;
				c->light.g			= alight * lend.g + unmoinsalight * ldep.g;
				c->light.b			= alight * lend.b + unmoinsalight * ldep.b;
				c->light.intensite	= alight * lend.intensite + unmoinsalight * ldep.intensite;
				c->light.intensiternd = alight * lend.intensiternd + unmoinsalight * ldep.intensiternd;
			}
			break;
		case INTERP_BEZIER:
			c->light = k->light;

			ksuivsuiv = ((num + 1) < CKTrack->nbkey) ? ksuiv + 1 : ksuiv;
			kprec = (num > 1) ? k - 1 : k;

			t1 = a;
			t2 = t1 * t1;
			t3 = t2 * t1;
			f0 = 2.f * t3 - 3.f * t2 + 1.f;
			f1 = -2.f * t3 + 3.f * t2;
			f2 = t3 - 2.f * t2 + t1;
			f3 = t3 - t2;

			temp = ksuiv->pos.x;
			p0 = 0.5f * (temp - kprec->pos.x);
			p1 = 0.5f * (ksuivsuiv->pos.x - k->pos.x);
			c->pos.x = f0 * k->pos.x + f1 * temp + f2 * p0 + f3 * p1;
			temp = ksuiv->pos.y;
			p0 = 0.5f * (temp - kprec->pos.y);
			p1 = 0.5f * (ksuivsuiv->pos.y - k->pos.y);
			c->pos.y = f0 * k->pos.y + f1 * temp + f2 * p0 + f3 * p1;
			temp = ksuiv->pos.z;
			p0 = 0.5f * (temp - kprec->pos.z);
			p1 = 0.5f * (ksuivsuiv->pos.z - k->pos.z);
			c->pos.z = f0 * k->pos.z + f1 * temp + f2 * p0 + f3 * p1;

			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);

			temp = ksuiv->speedtrack;
			p0 = 0.5f * (temp - kprec->speedtrack);
			p1 = 0.5f * (ksuivsuiv->speedtrack - k->speedtrack);
			c->speedtrack = f0 * k->speedtrack + f1 * temp + f2 * p0 + f3 * p1;

			{
				C_LIGHT ldep;
				C_LIGHT lend;

				if (lightprec->light.intensite < 0.f)
				{
					c->light.intensite = -1;
					break;
				}
				else
				{
					ldep = lightprec->light;
				}

				if (c->lightd.intensite < 0.f)
				{
					break;
				}
				else
				{
					lend = c->lightd;
				}

				c->light.pos.x = alight * lend.pos.x + unmoinsalight * ldep.pos.x;
				c->light.pos.y = alight * lend.pos.y + unmoinsalight * ldep.pos.y;
				c->light.pos.z = alight * lend.pos.z + unmoinsalight * ldep.pos.z;
				c->light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.r = alight * lend.r + unmoinsalight * ldep.r;
				c->light.g = alight * lend.g + unmoinsalight * ldep.g;
				c->light.b = alight * lend.b + unmoinsalight * ldep.b;
				c->light.intensite = alight * lend.intensite + unmoinsalight * ldep.intensite;
				c->light.intensiternd = alight * lend.intensiternd + unmoinsalight * ldep.intensiternd;
			}
			break;
	}

	if (k != c->key)
	{
		c->key = k;
		c->changekey = TRUE;
	}

	c->flTime += fpscurr;
	CKTrack->currframe = (((float)(c->flTime)) / 1000.f) * ((float)(GetEndFrame() - GetStartFrame())) / (float)GetTimeKeyFramer(c);

	if (CKTrack->currframe > (float)CKTrack->endframe)
	{
		CKTrack->currframe = (float)CKTrack->startframe;
		c->key = NULL;
		c->flTime = ARX_TIME_Get();
	}

	return TRUE;
}
/*----------------------------------------------------------------------*/
BOOL GereTrackNoPlay(CINEMATIQUE * c)
{
	C_KEY	* k, *ksuiv;
	float	a, unmoinsa, alight = 0, unmoinsalight = 0;
	int		num;
	C_KEY	* kprec, *ksuivsuiv;
	float	t1, t2, t3, f0, f1, f2, f3, p0, p1, temp;
	C_KEY	* lightprec, *lightnext;

	if (!CKTrack || !CKTrack->nbkey || !CKTrack->pause) return FALSE;

	k = GetKey((int) CKTrack->currframe, &num);

	if (!k) return FALSE;

	ksuiv = (num == CKTrack->nbkey) ? ksuiv = k : ksuiv = k + 1;

	if (ksuiv->frame != k->frame)
		a = (CKTrack->currframe - (float)k->frame) / ((float)(ksuiv->frame - k->frame));
	else
		a = 1.f;

	c->a = unmoinsa = 1.0f - a;

	c->numbitmap		= k->numbitmap;
	c->numbitmapsuiv	= ksuiv->numbitmap;
	c->ti				= k->typeinterp;
	c->fx				= k->fx;
	c->fxsuiv			= ksuiv->fx;
	c->color			= k->color;
	c->colord			= k->colord;
	c->colorflash		= k->colorf;
	c->speed			= k->speed;
	c->idsound			= k->idsound[LSoundChoose>>8];
	c->force			= k->force;

	if ((k->fx & 0xFF000000) == FX_LIGHT)
	{
		lightprec = k;
	}
	else
	{
		lightprec = k->light.prev;
	}

	lightnext = k->light.next;
	c->lightd = lightnext->light;

	if ((lightprec != lightnext))
	{
		alight = (CKTrack->currframe - (float)lightprec->frame) / ((float)(lightnext->frame - lightprec->frame));

		if (alight > 1.f) alight = 1.f;

		unmoinsalight = 1.0f - alight;
	}
	else
	{
		if (k == (CKTrack->key + CKTrack->nbkey - 1))
		{
			alight			= 1.f;
			unmoinsalight	= 0.f;
		}
		else
		{
			//ARX_BEGIN: jycorbel (2010-06-28) - clean warning 'variable used without having been initialized'. @BUG
			//alight can't be used because it is not initialized but the game used un initialized alight...
//ARX_BEGIN: jycorbel (2010-07-19) - Set light coeff to 0 to keep null all possibly light created from uninitialyzed var.
/*
	alight = unmoinsalight = 0.f; //default values needed when : k->typeinterp == INTERP_BEZIER (0) || k->typeinterp == INTERP_LINEAR (1)
	consequences on light :
			c->light : position = (0,0,0);
			c->light : color	= (0,0,0); == BLACK
			c->light : fallin	= fallout		= 0;
			c->light : intensite = intensiternd	= 0;
			ARX_CHECK( k->typeinterp != INTERP_BEZIER && k->typeinterp != INTERP_LINEAR );
*/
//ARX_END: jycorbel (2010-07-19)
			//ARX_END: jycorbel (2010-06-28)
			c->lightd = k->light;
			lightnext = lightprec = k;
		}
	}

	c->posgrille		= k->posgrille;
	c->angzgrille		= k->angzgrille;
	c->posgrillesuiv	= ksuiv->posgrille;
	c->angzgrillesuiv	= ksuiv->angzgrille;

	if ((k->numbitmap < 0) || (ksuiv->numbitmap < 0)) return FALSE;

	switch (k->typeinterp)
	{
		case INTERP_NO:
			c->pos		= k->pos;
			c->angz		= k->angz;
			c->possuiv	= ksuiv->pos;
			c->angzsuiv	= ksuiv->angz;

			c->light	= lightprec->light;
			c->speedtrack = k->speedtrack;
			break;
		case INTERP_LINEAR:
			c->pos.x = a * ksuiv->pos.x + unmoinsa * k->pos.x;
			c->pos.y = a * ksuiv->pos.y + unmoinsa * k->pos.y;
			c->pos.z = a * ksuiv->pos.z + unmoinsa * k->pos.z;

			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);

			c->speedtrack = a * ksuiv->speedtrack + unmoinsa * k->speedtrack;

			{
				C_LIGHT ldep;
				C_LIGHT lend;

				if (lightprec->light.intensite < 0.f)
				{
					c->light.intensite = -1.f;
					break;
				}
				else
				{
					ldep = lightprec->light;
				}

				if (c->lightd.intensite < 0.f)
				{
					break;
				}
				else
				{
					lend = c->lightd;
				}

				c->light.pos.x  = alight * lend.pos.x + unmoinsalight * ldep.pos.x;
				c->light.pos.y  = alight * lend.pos.y + unmoinsalight * ldep.pos.y;
				c->light.pos.z  = alight * lend.pos.z + unmoinsalight * ldep.pos.z;
				c->light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.r		= alight * lend.r + unmoinsalight * ldep.r;
				c->light.g		= alight * lend.g + unmoinsalight * ldep.g;
				c->light.b		= alight * lend.b + unmoinsalight * ldep.b;
				c->light.intensite = alight * lend.intensite + unmoinsalight * ldep.intensite;
				c->light.intensiternd = alight * lend.intensiternd + unmoinsalight * ldep.intensiternd;
			}
			break;
		case INTERP_BEZIER:
			ksuivsuiv = ((num + 1) < CKTrack->nbkey) ? ksuiv + 1 : ksuiv;
			kprec = (num > 1) ? k - 1 : k;

			t1 = a;
			t2 = t1 * t1;
			t3 = t2 * t1;
			f0 = 2.f * t3 - 3.f * t2 + 1.f;
			f1 = -2.f * t3 + 3.f * t2;
			f2 = t3 - 2.f * t2 + t1;
			f3 = t3 - t2;

			temp = ksuiv->pos.x;
			p0 = 0.5f * (temp - kprec->pos.x);
			p1 = 0.5f * (ksuivsuiv->pos.x - k->pos.x);
			c->pos.x = f0 * k->pos.x + f1 * temp + f2 * p0 + f3 * p1;
			temp = ksuiv->pos.y;
			p0 = 0.5f * (temp - kprec->pos.y);
			p1 = 0.5f * (ksuivsuiv->pos.y - k->pos.y);
			c->pos.y = f0 * k->pos.y + f1 * temp + f2 * p0 + f3 * p1;
			temp = ksuiv->pos.z;
			p0 = 0.5f * (temp - kprec->pos.z);
			p1 = 0.5f * (ksuivsuiv->pos.z - k->pos.z);
			c->pos.z = f0 * k->pos.z + f1 * temp + f2 * p0 + f3 * p1;

			c->angz = k->angz + a * GetAngleInterpolation(k->angz, ksuiv->angz);

			temp = ksuiv->speedtrack;
			p0 = 0.5f * (temp - kprec->speedtrack);
			p1 = 0.5f * (ksuivsuiv->speedtrack - k->speedtrack);
			c->speedtrack = f0 * k->speedtrack + f1 * temp + f2 * p0 + f3 * p1;

			{
				C_LIGHT ldep;
				C_LIGHT lend;

				if (lightprec->light.intensite < 0.f)
				{
					c->light.intensite = -1;
					break;
				}
				else
				{
					ldep = lightprec->light;
				}

				if (c->lightd.intensite < 0.f)
				{
					break;
				}
				else
				{
					lend = c->lightd;
				}

				c->light.pos.x = alight * lend.pos.x + unmoinsalight * ldep.pos.x;
				c->light.pos.y = alight * lend.pos.y + unmoinsalight * ldep.pos.y;
				c->light.pos.z = alight * lend.pos.z + unmoinsalight * ldep.pos.z;
				c->light.fallin = alight * lend.fallin + unmoinsalight * ldep.fallin;
				c->light.fallout = alight * lend.fallout + unmoinsalight * ldep.fallout;
				c->light.r = alight * lend.r + unmoinsalight * ldep.r;
				c->light.g = alight * lend.g + unmoinsalight * ldep.g;
				c->light.b = alight * lend.b + unmoinsalight * ldep.b;
				c->light.intensite = alight * lend.intensite + unmoinsalight * ldep.intensite;
				c->light.intensiternd = alight * lend.intensiternd + unmoinsalight * ldep.intensiternd;
			}
			break;
	}

	if (k != c->key)
	{
		c->key = k;
		c->changekey = TRUE;
	}

	return TRUE;
}

/*----------------------------------------------------------------------*/
void PlayTrack(CINEMATIQUE * c)
{
	if (!CKTrack || !CKTrack->pause) return;

	CKTrack->pause = FALSE;
	c->flTime = 0; 
}
/*----------------------------------------------------------------------*/
int GetCurrentFrame(void)
{
	if (!CKTrack) return -1;

	return (int)CKTrack->currframe;
}
/*----------------------------------------------------------------------*/
float GetTimeKeyFramer(CINEMATIQUE * c)
{
	if (!CKTrack) return 0.f;

	float t = 0.f;
	C_KEY * k = CKTrack->key, *ksuiv;
	int nb = CKTrack->nbkey - 1;

	while (nb--)
	{
		ksuiv = k + 1;
		t += ((float)(ksuiv->frame - k->frame)) / (CKTrack->fps * k->speedtrack);
		k++;
	}

	return t;
}
/*----------------------------------------------------------------------*/
int GetStartFrame(void)
{
	if (!CKTrack) return -1;

	return CKTrack->startframe;
}
/*----------------------------------------------------------------------*/
int GetEndFrame(void)
{
	if (!CKTrack) return -1;

	return CKTrack->endframe;
}
/*----------------------------------------------------------------------*/
float GetTrackFPS(void)
{
	if (!CKTrack) return -1;

	return CKTrack->fps;
}
/*----------------------------------------------------------------------*/
void SetCurrFrame(int frame)
{
	if (!CKTrack) return ;

	CKTrack->currframe = (float)CKTrack->startframe + (float)frame;
}
