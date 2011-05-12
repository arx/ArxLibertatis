
#ifndef ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H
#define ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H

#include <windows.h> // TODO for HBITMAP

#include "graphics/GraphicsTypes.h" // for EERIE_3D
#include "platform/math/Vector2.h"


#define MAX_WIDTH_AND_HEIGHT 256
#define MAX_BITMAP 256


class TextureContainer;
class Cinematic;


// TODO better name
struct C_INDEXED {
	int bitmapdepx;
	int bitmapdepy;
	int bitmapw;
	int bitmaph;
	int nbvertexs;
	TextureContainer * tex;
	int startind;
	int nbind;
};

// TODO better name
struct C_IND {
	unsigned short i1;
	unsigned short i2;
	unsigned short i3;
};

struct C_UV {
	Vec2f uv;
	int indvertex;
};

struct CinematicGrid {
	int nbvertexs;
	int nbfaces;
	int nbinds;
	int nbindsmalloc;
	int nbuvs;
	int nbuvsmalloc;
	EERIE_3D * vertexs;
	C_UV * uvs;
	C_IND * inds;
	int nbmat;
	C_INDEXED * mats;
	float dx;
	float dy;
	int nbx;
	int nby;
	int echelle;
};

struct CinematicBitmap {
	short actif, load;
	char * dir;
	char * name;
	HBITMAP hbitmap;
	int w, h;
	int nbx, nby;
	CinematicGrid grid;
	int dreaming;
};


void DeleteAllBitmap();

void InitMapLoad();
CinematicBitmap * GetFreeBitmap(int * num);
bool DeleteFreeBitmap(int num);
int CreateAllMapsForBitmap(const std::string & path, Cinematic * c);
bool ActiveAllTexture(Cinematic * c);

bool ReCreateAllMapsForBitmap(int id, int nmax, Cinematic * c);

#endif // ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H
