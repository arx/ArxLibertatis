
#ifndef ARX_GRAPHICS_DIRECT3D_DX7TEXTURE2D_H
#define ARX_GRAPHICS_DIRECT3D_DX7TEXTURE2D_H

#include <list>

#include <d3d.h>

#include "graphics/texture/Texture.h"

class DX7Texture2D : public Texture2D {
	
	friend class Direct3DRenderer;
	
public:
	
	LPDIRECTDRAWSURFACE7 GetTextureID() const {
		return m_pddsSurface;
	}
	
private:
	
	DX7Texture2D(LPDIRECT3DDEVICE7 device);
	virtual ~DX7Texture2D();
	
	virtual bool Create();
	virtual void Upload();
	virtual void Destroy();
	
	static HRESULT CALLBACK TextureSearchCallback(DDPIXELFORMAT * pddpf, VOID * param);
	
private:
	
	void CopyNextMipLevel(LPDIRECTDRAWSURFACE7 pddsDst, LPDIRECTDRAWSURFACE7 pddsSrc);
	
	LPDIRECTDRAWSURFACE7 m_pddsSurface;
	LPDIRECT3DDEVICE7 device;
};

// TODO-DX7: This should really be an intrusive list!
extern std::list<DX7Texture2D*> g_Textures2D;

bool downloadSurface(LPDIRECTDRAWSURFACE7 surface, Image & image);

#endif // ARX_GRAPHICS_DIRECT3D_DX7TEXTURE2D_H
