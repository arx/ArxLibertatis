
#include "graphics/opengl/OpenGLRenderer.h"

#include <GL/gl.h>

#include "io/Logger.h"

//OpenGLRenderer instance;

void OpenGLRenderer::SetRenderState(Renderer::RenderState state, bool enable) {
	
	switch ( state )
	{
		case AlphaBlending:
		{
			enable ?
				glEnable( GL_ALPHA_TEST ) :
				glDisable( GL_ALPHA_TEST );
				LogDebug << "Chaning alpha blending" << enable;
			return;
		}
		case DepthTest:
		{
			enable ?
				glEnable( GL_DEPTH_TEST ) :
				glDisable( GL_DEPTH_TEST );
				LogDebug << "Changing depth test " << enable;
			return;
		}
		case DepthWrite:
		{
			glDepthMask( enable ? GL_TRUE : GL_FALSE );
			LogDebug << "Changing depth writing " << enable;
			return;
		}
		case Fog:
		{
			enable ?
				glEnable( GL_FOG ) :
				glDisable( GL_FOG );
			LogDebug << "Changing Fog " << enable;
			return;
		}
		case Lighting:
		{
			enable ?
				glEnable( GL_LIGHTING ) :
				glDisable( GL_LIGHTING );
			LogDebug << "Changing lighting " << enable;
			return;
		}
		case ZBias:
		{
			enable ?
				glEnable( GL_POLYGON_OFFSET_FILL ) :
				glDisable( GL_POLYGON_OFFSET_FILL );
			LogDebug << "Changing depth bias" << enable;
			return;
		}
		default:
			LogWarning << "unsupported render state: " << state;
	}
}
