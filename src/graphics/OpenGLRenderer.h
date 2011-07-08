#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "graphics/Renderer.h"
#include "Configure.h"

#ifdef HAVE_OPENGL

class OpenGLRenderer : Renderer
{

	virtual void SetRenderState( RenderState state, bool enable );

};

#endif // HAVE_OPENGL

#endif // OPENGL_RENDERER_H
