
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include <io/Logger.h>

#define CHECK_GL if(GLenum error = glGetError()) LogError << "GL error in " << __func__ << ": " << error << " = " << gluErrorString(error)
