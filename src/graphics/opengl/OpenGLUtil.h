
#include "platform/Platform.h"

// Link statically with GLEW
#if ARX_COMPILER_MSVC
	#define GLEW_STATIC
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include <io/log/Logger.h>

#define CHECK_GL if(GLenum error = glGetError()) LogError << "GL error in " << __func__ << ": " << error << " = " << gluErrorString(error)
