#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "graphics/Renderer.h"

class OpenGLRenderer : Renderer
{

	virtual void SetRenderState( RenderState state, bool enable );

};

#endif // OPENGL_RENDERER_H
