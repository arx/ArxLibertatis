
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
