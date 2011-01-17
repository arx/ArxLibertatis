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
#ifndef CINEMATIQUE_H
#define CINEMATIQUE_H

#include "EERIEApp.h"
#include "EERIEDraw.h"

/*-----------------------------------------------------------*/
#define		VERSION		((1<<16)|76)

#define		MAX_BITMAP	256
#define		MAX_SOUND	256
#define		MAX_WIDTH_AND_HEIGHT	256

//fx
#define		FX_FADEIN	1
#define		FX_FADEOUT	2
#define		FX_BLUR		3
//prefx
#define		FX_DREAM	(1<<8)
//post fx
#define		FX_FLASH	(1<<16)
#define		FX_APPEAR	(2<<16)
#define		FX_APPEAR2	(3<<16)
//all time
#define		FX_LIGHT	(1<<24)

#define		INTERP_NO		-1
#define		INTERP_BEZIER	0
#define		INTERP_LINEAR	1
#define		INTERP_NO_FADE	2		

#define LARGEURS	512
#define HAUTEURS	384

#define C_MIN_F32	1.175494351e-38F
#define C_EQUAL_F32(f1,f2) (fabs(f1-f2)<C_MIN_F32)
#define C_NEQUAL_F32(f1,f2) (fabs(f1-f2)>=C_MIN_F32)

#define C_LANGUAGE_FRENCH		0
#define C_LANGUAGE_GERMAN		1
#define C_LANGUAGE_SPANISH		2
#define C_LANGUAGE_ENGLISH		3
/*-----------------------------------------------------------*/
void DeleteAllBitmap(LPDIRECT3DDEVICE7 device);
void DeleteAllSound(void);
BOOL DeleteTrack(void);
/*-----------------------------------------------------------*/
//extern char DirectoryChoose[];
/*-----------------------------------------------------------*/
class C_LIGHT
{
	public:
		EERIE_3D	pos;
		float		fallin;
		float		fallout;
		float		r, g, b;
		float		intensite;
		float		intensiternd;
		struct CST_KEY * prev;
		struct CST_KEY * next;
	public:
		C_LIGHT()
		{
			pos.x = pos.y = pos.z = 0.f;
			fallin = 100.f;
			fallout = 200.f;
			r = g = b = 255.f;
			intensite = 1.f;
			intensiternd = 0.2f;
			next = NULL;
		};
		~C_LIGHT() {};
};

class C_LIGHT_1_72
{
	public:
		EERIE_3D	pos;
		float		fallin;
		float		fallout;
		float		r, g, b;
		float		intensite;
		float		intensiternd;
	public:
};

/////// NEW_DRAW
typedef struct
{
	int					bitmapdepx;
	int					bitmapdepy;
	int					bitmapw;
	int					bitmaph;
	int					nbvertexs;
	TextureContainer	* tex;
	int					startind;
	int					nbind;
} C_INDEXED;

typedef struct
{
	unsigned short	i1;
	unsigned short	i2;
	unsigned short	i3;
} C_IND;

typedef struct
{
	EERIE_2D		uv;
	int				indvertex;
} C_UV;

typedef struct
{
	int				nbvertexs;
	int				nbfaces;
	int				nbinds;
	int				nbindsmalloc;
	int				nbuvs;
	int				nbuvsmalloc;
	EERIE_3D	*	vertexs;
	C_UV		*	uvs;
	C_IND		*	inds;
	int				nbmat;
	C_INDEXED	*	mats;
	float			dx;
	float			dy;
	int				nbx;
	int				nby;
	int				echelle;
} C_GRILLE;
/////// END_NEW_DRAW

typedef struct
{
	short		actif, load;
	char	*	dir;
	char	*	name;
	HBITMAP		hbitmap;
	int			w, h;
	int			nbx, nby;
	//	int			nbpoly;
	//	C_INFOPOLY	*listepoly;
	C_GRILLE	grille;
	int			dreaming;
} C_BITMAP;

typedef struct
{
	short		actif, load;
	char	*	dir;
	char	*	name;
	char	*	sound;
	long		idhandle;
} C_SOUND;

typedef struct
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	int			typeinterp;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	float		speed;
} C_KEY_1_59;

typedef struct
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	int			typeinterp;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	int			idsound;
	float		speed;
} C_KEY_1_65;

typedef struct
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	short		typeinterp, force;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	int			idsound;
	float		speed;
} C_KEY_1_70;

typedef struct
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	short		typeinterp, force;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	int			idsound;
	float		speed;
	C_LIGHT		light;
} C_KEY_1_71;

typedef struct CS_KEY
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	short		typeinterp, force;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	int			idsound;
	float		speed;
	C_LIGHT_1_72	light;
	EERIE_3D	posgrille;
	float		angzgrille;
} C_KEY_1_72;

typedef struct
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	short		typeinterp, force;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	int			idsound;
	float		speed;
	C_LIGHT		light;
	EERIE_3D	posgrille;
	float		angzgrille;
} C_KEY_1_74;

typedef struct
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	short		typeinterp, force;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	int			idsound;
	float		speed;
	C_LIGHT		light;
	EERIE_3D	posgrille;
	float		angzgrille;
	float		speedtrack;
} C_KEY_1_75;

typedef struct CST_KEY
{
	int			frame;
	int			numbitmap;
	int			fx;				//fx associé
	short		typeinterp, force;
	EERIE_3D	pos;
	float		angz;
	int			color;
	int			colord;
	int			colorf;
	float		speed;
	C_LIGHT		light;
	EERIE_3D	posgrille;
	float		angzgrille;
	float		speedtrack;
	int			idsound[16];	//16 langues max.
} C_KEY;

typedef struct
{
	int			startframe;
	int			endframe;
	float		currframe;
	float		fps;
	int			nbkey;
	int			pause;
	C_KEY	*	key;
} C_TRACK;

class CINEMATIQUE
{
	public:
 
 
		HRESULT DeleteDeviceObjects();
	public:
		LPDIRECT3DDEVICE7 m_pd3dDevice;

		EERIE_3D	pos;
		float		angz;
		EERIE_3D	possuiv;			//dans le cas d'une non interpolation fadée
		float		angzsuiv;
		int			numbitmap;
		int			numbitmapsuiv;
		float		a;
		int			fx;
		int			fxsuiv;
		BOOL		changekey;
		C_KEY	*	key;
		BOOL		projectload;
		short		ti;
		short		tichoose;
		short		force;
		int			color;
		int			colord;
		int			colorflash;
		float		speed;
		int			colorchoose;
		int			colorchoosed;
		int			colorflashchoose;
		float		speedchoose;
		int			idsound;
		C_LIGHT		light;
		C_LIGHT		lightchoose;
		C_LIGHT		lightd;
		EERIE_3D	posgrille;
		float		angzgrille;
		EERIE_3D	posgrillesuiv;
		float		angzgrillesuiv;
		float		speedtrack;
		float		flTime;
		float		m_flIntensityRND;

		CINEMATIQUE(LPDIRECT3DDEVICE7, int, int);
		BOOL ActiveTexture(int id);
		HRESULT InitDeviceObjects();
		HRESULT OneTimeSceneReInit(void);
		HRESULT Render(float FDIFF);
		HRESULT New();
		void ReInitMapp(int id);
 
};

/*-----------------------------------------------------------*/
BOOL AllocTrack(int sf, int ef, float fps);
BOOL AddKey(C_KEY * key, BOOL writecolor, BOOL writecolord, BOOL writecolorf);
BOOL AddKeyLoad(C_KEY * key);
void AddDiffKey(CINEMATIQUE * c, C_KEY * key, BOOL writecolor, BOOL writecolord, BOOL writecolorf);
BOOL GereTrack(CINEMATIQUE * c, float fpscurr);

void PlayTrack(CINEMATIQUE * c);
int GetCurrentFrame(void);
int GetStartFrame(void);
int GetEndFrame(void);
void SetCurrFrame(int frame);
BOOL GereTrackNoPlay(CINEMATIQUE * c);
float GetTrackFPS(void);

C_KEY * GetKey(int f, int * num);
C_KEY * SearchKey(int f, int * num);
 
float GetTimeKeyFramer(CINEMATIQUE * c);
void InitUndo(void);
void UpDateAllKeyLight(void);

void InitMapLoad(CINEMATIQUE * c);
C_BITMAP * GetFreeBitmap(int * num);
BOOL DeleteFreeBitmap(int num);
BOOL KillTexture(LPDIRECT3DDEVICE7 device, int num);
int CreateAllMapsForBitmap(char * dir, char * name, CINEMATIQUE * c, int num, int pos);
BOOL ActiveAllTexture(CINEMATIQUE * c);

BOOL ReCreateAllMapsForBitmap(int id, int nmax, CINEMATIQUE * c, LPDIRECT3DDEVICE7 device);

int FX_FadeIN(float a, int color, int colord);
int FX_FadeOUT(float a, int color, int colord);
BOOL FX_FlashBlanc(LPDIRECT3DDEVICE7 device, float w, float h, float speed, int color, float fps, float currfps);
BOOL FX_Blur(CINEMATIQUE * c, LPDIRECT3DDEVICE7 device, C_BITMAP * tb);
BOOL SpecialFade(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
BOOL SpecialFadeR(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
void FX_DreamPrecalc(C_BITMAP * bi, float amp, float fps);

void GetPathDirectory(char * dirfile);
void ClearDirectory(char * dirfile);
BOOL OpenDialogRead(char * titlename, HWND hwnd, unsigned long numfilter);
BOOL OpenDialogSave(char * titlename, HWND hwnd, unsigned long numfilter);
int OpenDialogColor(HWND hwnd, int col);
char * OpenDiagDirectory(HWND hwnd, char * title);

BOOL SaveProject(char * dir, char * name);
BOOL LoadProject(CINEMATIQUE * c, char * dir, char * name);
BOOL LoadOldProject(CINEMATIQUE * c, char * dir, char * name);

void InitSound(CINEMATIQUE * c);
C_SOUND * GetFreeSound(int * num);
BOOL DeleteFreeSound(int num);
BOOL AddSoundToList(char * dir, char * name, int id, int pos);
BOOL PlaySoundKeyFramer(int id);
void StopSoundKeyFramer(void);

void DrawGrille(LPDIRECT3DDEVICE7 device, C_GRILLE * grille, int col, int fx, C_LIGHT * light, EERIE_3D * posgrillesuiv, float angzgrillesuiv);
void FillKeyTemp(EERIE_3D * pos, float az, int frame, int numbitmap, int numfx, short ti, int color, int colord, int colorf, float speed, int idsound, short force, C_LIGHT * light, EERIE_3D * posgrille, float angzgrille, float speedtrack);

void ReInitStandardCam(EERIE_CAMERA * cam);
#endif
