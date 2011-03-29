
#ifndef ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H
#define ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H

#include "graphics/GraphicsTypes.h" // for EERIE_2D and EERIE_3D
#include "graphics/texture/Texture.h" // For Texture2D

class TextureContainer;
class Cinematic;

// TODO better name
struct C_INDEXED {
	int bitmapdepx;
	int bitmapdepy;
	int bitmapw;
	int bitmaph;
	int nbvertexs;
	Texture2D* tex;
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
	EERIE_2D uv;
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
	std::vector<C_INDEXED> mats;
	float dx;
	float dy;
	int nbx;
	int nby;
	int echelle;
};

class CinematicBitmap {
public:
	~CinematicBitmap();

public:
	int w, h;
	int nbx, nby;
	CinematicGrid grid;
	int dreaming;
};

CinematicBitmap* CreateCinematicBitmap(const std::string & path, int scale);

#endif // ARX_GRAPHICS_DATA_CINEMATICTEXTURE_H
