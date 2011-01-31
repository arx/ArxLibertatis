
#ifndef ARX_GRAPHICS_EFFECTS_CINEMATICEFFECTS_H
#define ARX_GRAPHICS_EFFECTS_CINEMATICEFFECTS_H

#include "graphics/d3dwrapper.h" // for LPDIRECT3DDEVICE7


class TextureContainer;
class CinematicBitmap;
class Cinematic;


int FX_FadeIN(float a, int color, int colord);
int FX_FadeOUT(float a, int color, int colord);
bool FX_FlashBlanc(LPDIRECT3DDEVICE7 device, float w, float h, float speed, int color, float fps, float currfps);
bool FX_Blur(Cinematic * c, LPDIRECT3DDEVICE7 device, CinematicBitmap * tb);
bool SpecialFade(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
bool SpecialFadeR(LPDIRECT3DDEVICE7 device, TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
void FX_DreamPrecalc(CinematicBitmap * bi, float amp, float fps);

#endif // ARX_GRAPHICS_EFFECTS_CINEMATICEFFECTS_H
