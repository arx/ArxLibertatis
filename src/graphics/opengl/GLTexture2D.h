
#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H

#include "graphics/texture/Texture.h"

class GLTexture2D : public Texture2D {
	
	friend class OpenGLRenderer;
	
	GLTexture2D();
	~GLTexture2D();
	
	bool Create();
	void Upload();
	void Destroy();
	
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
