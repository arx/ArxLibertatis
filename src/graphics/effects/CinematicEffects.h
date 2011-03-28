
#ifndef ARX_GRAPHICS_EFFECTS_CINEMATICEFFECTS_H
#define ARX_GRAPHICS_EFFECTS_CINEMATICEFFECTS_H

#include "graphics/d3dwrapper.h"


class TextureContainer;
class CinematicBitmap;
class Cinematic;


int FX_FadeIN(float a, int color, int colord);
int FX_FadeOUT(float a, int color, int colord);
bool FX_FlashBlanc(float w, float h, float speed, int color, float fps, float currfps);
bool FX_Blur(Cinematic * c, CinematicBitmap * tb);
bool SpecialFade(TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
bool SpecialFadeR(TextureContainer * mask, float ws, float h, float speed, float fps, float fpscurr);
void FX_DreamPrecalc(CinematicBitmap * bi, float amp, float fps);

#endif // ARX_GRAPHICS_EFFECTS_CINEMATICEFFECTS_H
