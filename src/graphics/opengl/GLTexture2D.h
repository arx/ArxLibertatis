
#ifndef ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
#define ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H

#include <boost/intrusive/list_hook.hpp>

#include "graphics/opengl/OpenGLUtil.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"

class OpenGLRenderer;
class GLTextureStage;

typedef boost::intrusive::list_base_hook<boost::intrusive::link_mode<boost::intrusive::auto_unlink> > GLTextureListHook;

class GLTexture2D : public Texture2D, public GLTextureListHook {
	
public:
	
	GLTexture2D(OpenGLRenderer * renderer);
	~GLTexture2D();
	
	bool Create();
	void Upload();
	void Destroy();
	
	void apply(GLTextureStage * stage);
	
private:
	
	OpenGLRenderer * renderer;
	
	GLuint tex;
	
	TextureStage::WrapMode wrapMode;
	TextureStage::FilterMode minFilter;
	TextureStage::FilterMode magFilter;
	TextureStage::FilterMode mipFilter;
	
	friend class GLTextureStage;
};

#endif // ARX_GRAPHICS_OPENGL_GLTEXTURE2D_H
