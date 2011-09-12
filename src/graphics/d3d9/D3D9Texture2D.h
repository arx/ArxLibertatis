
#ifndef ARX_GRAPHICS_D3D9_D3D9TEXTURE2D_H
#define ARX_GRAPHICS_D3D9_D3D9TEXTURE2D_H

#include <list>

#include <d3d9.h>

#include "graphics/texture/Texture.h"

class DX9Texture2D : public Texture2D {
	
	friend class D3D9Renderer;
	
public:
	
	LPDIRECT3DTEXTURE9 GetTextureID() const {
		return m_pTexture;
	}

	static void Copy_L8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch);
	static void Copy_A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch);
	static void Copy_L8A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch);
	static void Copy_R8G8B8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch);
	static void Copy_B8G8R8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch);
	static void Copy_R8G8B8A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch);
	static void Copy_B8G8R8A8(BYTE* __restrict pDst, BYTE* __restrict pSrc, DWORD imageWidth, DWORD imageHeight, D3DFORMAT d3dFormat, DWORD dstPixelSize, DWORD srcPixelSize, DWORD dstPitch, DWORD srcPitch);
	
private:
	
	DX9Texture2D();
	virtual ~DX9Texture2D();
	
	virtual bool Create();
	virtual void Upload();
	virtual void Destroy();
	
private:

	LPDIRECT3DTEXTURE9 m_pTexture;
};

// TODO-DX9: This should really be an intrusive list!
extern std::list<DX9Texture2D*> g_Textures2D;

#endif // ARX_GRAPHICS_D3D9_D3D9TEXTURE2D_H
