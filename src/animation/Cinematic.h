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


#include <windows.h> // for HRESULT

#include "graphics/d3dwrapper.h" // for LPDIRECT3DDEVICE7
#include "graphics/GraphicsTypes.h" // for EERIE_2D and EERIE_3D

#ifndef ARX_ANIMATION_CINEMATIC_H
#define ARX_ANIMATION_CINEMATIC_H

#define MAX_BITMAP 256
#define MAX_SOUND 256

//fx
#define FX_FADEIN 1
#define FX_FADEOUT 2
#define FX_BLUR    3
//prefx
#define FX_DREAM   (1<<8)
//post fx
#define FX_FLASH   (1<<16)
#define FX_APPEAR  (2<<16)
#define FX_APPEAR2 (3<<16)
//all time
#define FX_LIGHT   (1<<24)

#define INTERP_NO -1
#define INTERP_BEZIER 0
#define INTERP_LINEAR 1
#define INTERP_NO_FADE 2

#define LARGEURS 512
#define HAUTEURS 384

#define C_MIN_F32 1.175494351e-38F
#define C_EQUAL_F32(f1,f2) (fabs(f1-f2)<C_MIN_F32)
#define C_NEQUAL_F32(f1,f2) (fabs(f1-f2)>=C_MIN_F32)

#define C_LANGUAGE_FRENCH   0
#define C_LANGUAGE_GERMAN   1
#define C_LANGUAGE_SPANISH  2
#define C_LANGUAGE_ENGLISH  3


// TODO cleanup
class C_KEY;
class CinematicBitmap;
class CinematicSound;
class CinematicGrid;
class EERIE_CAMERA;


// TODO used for loading
class CinematicLight {
	
public:
	
	EERIE_3D pos;
	float fallin;
	float fallout;
	float r, g, b;
	float intensity;
	float intensiternd;
	C_KEY * prev;
	C_KEY * next;
	
	CinematicLight() {
		pos.x = pos.y = pos.z = 0.f;
		fallin = 100.f;
		fallout = 200.f;
		r = g = b = 255.f;
		intensity = 1.f;
		intensiternd = 0.2f;
		next = NULL;
	};
	
};


class Cinematic {
	
public:
	
	LPDIRECT3DDEVICE7 m_pd3dDevice;
	
	EERIE_3D pos;
	float angz;
	EERIE_3D possuiv; // in the case of a non-fade interpolation
	float angzsuiv;
	int numbitmap;
	int numbitmapsuiv;
	float a;
	int fx;
	int fxsuiv;
	bool changekey;
	C_KEY * key;
	bool projectload;
	short ti;
	short tichoose;
	short force;
	int color;
	int colord;
	int colorflash;
	float speed;
	int colorchoose;
	int colorchoosed;
	int colorflashchoose;
	float speedchoose;
	int idsound;
	CinematicLight light;
	CinematicLight lightchoose;
	CinematicLight lightd;
	EERIE_3D posgrille;
	float angzgrille;
	EERIE_3D posgrillesuiv;
	float angzgrillesuiv;
	float speedtrack;
	float flTime;
	float m_flIntensityRND;
	
	Cinematic(LPDIRECT3DDEVICE7, int, int);
	bool ActiveTexture(int id);
	HRESULT InitDeviceObjects();
	HRESULT OneTimeSceneReInit();
	HRESULT Render(float framediff);
	HRESULT New();
	void ReInitMapp(int id);
	HRESULT DeleteDeviceObjects();
	
};

void DeleteAllBitmap(LPDIRECT3DDEVICE7 device);
void DeleteAllSound(void);
bool DeleteTrack(void);

bool AllocTrack(int sf, int ef, float fps);
bool AddKey(C_KEY * key, bool writecolor, bool writecolord, bool writecolorf);
bool AddKeyLoad(C_KEY * key);
void AddDiffKey(Cinematic * c, C_KEY * key, bool writecolor, bool writecolord, bool writecolorf);
bool GereTrack(Cinematic * c, float fpscurr);

void PlayTrack(Cinematic * c);
int GetCurrentFrame();
int GetStartFrame();
int GetEndFrame();
void SetCurrFrame(int frame);
bool GereTrackNoPlay(Cinematic * c);
float GetTrackFPS();

C_KEY * GetKey(int f, int * num);
C_KEY * SearchKey(int f, int * num);
 
float GetTimeKeyFramer(Cinematic * c);
void InitUndo();
void UpDateAllKeyLight();

void InitMapLoad(Cinematic * c);
CinematicBitmap * GetFreeBitmap(int * num);
bool DeleteFreeBitmap(int num);
bool KillTexture(LPDIRECT3DDEVICE7 device, int num);
int CreateAllMapsForBitmap(char * dir, char * name, Cinematic * c, int num, int pos);
bool ActiveAllTexture(Cinematic * c);

bool ReCreateAllMapsForBitmap(int id, int nmax, Cinematic * c, LPDIRECT3DDEVICE7 device);

int FX_FadeIN(float a, int color, int colord);
int FX_FadeOUT(float a, int color, int colord);
bool FX_FlashBlanc(LPDIRECT3DDEVICE7 device, float w, float h, float speed, int color, float fps, float currfps);
bool FX_Blur(Cinematic * c, LPDIRECT3DDEVICE7 device, CinematicBitmap * tb);
bool SpecialFade(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
bool SpecialFadeR(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
void FX_DreamPrecalc(CinematicBitmap * bi, float amp, float fps);

void GetPathDirectory(char * dirfile);
void ClearDirectory(char * dirfile);
bool OpenDialogRead(char * titlename, HWND hwnd, unsigned long numfilter);
bool OpenDialogSave(char * titlename, HWND hwnd, unsigned long numfilter);
int OpenDialogColor(HWND hwnd, int col);
char * OpenDiagDirectory(HWND hwnd, char * title);

bool SaveProject(char * dir, char * name);
bool LoadProject(Cinematic * c, const char * dir, const char * name);
bool LoadOldProject(Cinematic * c, const char * dir, const char * name);

void InitSound(Cinematic * c);
CinematicSound * GetFreeSound(int * num);
bool DeleteFreeSound(int num);
int AddSoundToList(char * dir, char * name, int id, int pos);
bool PlaySoundKeyFramer(int id);
void StopSoundKeyFramer(void);

void DrawGrille(LPDIRECT3DDEVICE7 device, CinematicGrid * grille, int col, int fx, CinematicLight * light, EERIE_3D * posgrillesuiv, float angzgrillesuiv);
void FillKeyTemp(EERIE_3D * pos, float az, int frame, int numbitmap, int numfx, short ti, int color, int colord, int colorf, float speed, int idsound, short force, CinematicLight * light, EERIE_3D * posgrille, float angzgrille, float speedtrack);

void ClearAbsDirectory(char * pT, const char * d);
void AddDirectory(char * pT, const char * dir);

void ReInitStandardCam(EERIE_CAMERA * cam);

#endif // ARX_ANIMATION_CINEMATIC_H
