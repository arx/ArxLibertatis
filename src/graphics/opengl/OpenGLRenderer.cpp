/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphics/opengl/OpenGLRenderer.h"

#include <algorithm>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "core/Application.h"
#include "core/Config.h"
#include "gui/Credits.h"
#include "graphics/opengl/GLDebug.h"
#include "graphics/opengl/GLNoVertexBuffer.h"
#include "graphics/opengl/GLTexture2D.h"
#include "graphics/opengl/GLTextureStage.h"
#include "graphics/opengl/GLVertexBuffer.h"
#include "io/log/Logger.h"
#include "platform/CrashHandler.h"
#include "window/RenderWindow.h"

static const char vertexShaderSource[] = "void main() {\n"
	"	// Convert pre-transformed D3D vertices to OpenGL vertices.\n"
	"	float w = 1.0 / gl_Vertex.w;\n"
	"	vec4 vertex = vec4(gl_Vertex.xyz * w, w);\n"
	"	// We only need the projection matrix as modelview will always be identity.\n"
	"	gl_Position = gl_ProjectionMatrix * vertex;\n"
	"	gl_FrontColor = gl_BackColor = gl_Color;\n"
	"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"	gl_FogFragCoord = vertex.z;\n"
	"}\n";



OpenGLRenderer::OpenGLRenderer()
	: useVertexArrays(false)
	, useVBOs(false)
	, maxTextureStage(0)
	, shader(0)
	, m_maximumAnisotropy(1.f)
	, m_maximumSupportedAnisotropy(1.f)
	, m_glcull(GL_NONE)
	, m_hasMSAA(false)
	, m_hasTextureNPOT(false)
{ }

OpenGLRenderer::~OpenGLRenderer() {
	
	if(isInitialized()) {
		shutdown();
	}
	
	// TODO textures must be destructed before OpenGLRenderer or not at all
	//for(TextureList::iterator it = textures.begin(); it != textures.end(); ++it) {
	//	LogWarning << "Texture still loaded: " << it->getFileName();
	//}
	
};

enum GLTransformMode {
	GL_UnsetTransform,
	GL_NoTransform,
	GL_ModelViewProjectionTransform
};

static GLTransformMode currentTransform;

static bool checkShader(GLuint object, const char * op, GLuint check) {
	
	GLint status;
	glGetObjectParameterivARB(object, check, &status);
	if(!status) {
		int logLength;
		glGetObjectParameterivARB(object, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);
		char * log = new char[logLength];
		glGetInfoLogARB(object, logLength, NULL, log);
		LogWarning << "Failed to " << op << " vertex shader: " << log;
		delete[] log;
		return false;
	}
	
	return true;
}

static GLuint loadVertexShader(const char * source) {
	
	GLuint shader = glCreateProgramObjectARB();
	if(!shader) {
		LogWarning << "Failed to create program object";
		return 0;
	}
	
	GLuint obj = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	if(!obj) {
		LogWarning << "Failed to create shader object";
		glDeleteObjectARB(shader);
		return 0;
	}
	
	glShaderSourceARB(obj, 1, &source, NULL);
	glCompileShaderARB(obj);
	if(!checkShader(obj, "compile", GL_OBJECT_COMPILE_STATUS_ARB)) {
		glDeleteObjectARB(obj);
		glDeleteObjectARB(shader);
		return 0;
	}
	
	glAttachObjectARB(shader, obj);
	glDeleteObjectARB(obj);
	
	glLinkProgramARB(shader);
	if(!checkShader(shader, "link", GL_OBJECT_LINK_STATUS_ARB)) {
		glDeleteObjectARB(shader);
		return 0;
	}
	
	return shader;
}

void OpenGLRenderer::initialize() {
	
	if(glewInit() != GLEW_OK) {
		LogError << "GLEW init failed";
		return;
	}
	
	const GLubyte * glewVersion = glewGetString(GLEW_VERSION);
	LogInfo << "Using GLEW " << glewVersion;
	CrashHandler::setVariable("GLEW version", glewVersion);
	
	const GLubyte * glVersion = glGetString(GL_VERSION);
	LogInfo << "Using OpenGL " << glVersion;
	CrashHandler::setVariable("OpenGL version", glVersion);
	
	const GLubyte * glVendor = glGetString(GL_VENDOR);
	LogInfo << " ├─ Vendor: " << glVendor;
	CrashHandler::setVariable("OpenGL vendor", glVendor);
	
	const GLubyte * glRenderer = glGetString(GL_RENDERER);
	LogInfo << " ├─ Device: " << glRenderer;
	CrashHandler::setVariable("OpenGL device", glRenderer);
	
	u64 totalVRAM = 0, freeVRAM = 0;
	{
		#ifdef GL_NVX_gpu_memory_info
		if(GLEW_NVX_gpu_memory_info) {
			// Implemented by the NVIDIA blob and radeon drivers in newer Mesa
			GLint tmp = 0;
			glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &tmp);
			totalVRAM = u64(tmp) * 1024;
			glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &tmp);
			freeVRAM = u64(tmp) * 1024;
		}
		#endif
		#if defined(GL_NVX_gpu_memory_info) && defined(GL_ATI_meminfo)
		else
		#endif
		#ifdef GL_ATI_meminfo
		if(GLEW_ATI_meminfo) {
			// Implemented by the AMD blob and radeon drivers in newer Mesa
			GLint info[4];
			glGetIntegerv(GL_VBO_FREE_MEMORY_ATI, info);
			freeVRAM = u64(info[0]) * 1024;
			glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, info);
			freeVRAM = std::max(freeVRAM, u64(info[0]) * 1024);
		}
		#endif
		/*
		 * There is also GLX_MESA_query_renderer but being a GLX extension it's too
		 * anoying to use here.
		 */
	}
	{
		std::ostringstream oss;
		if(totalVRAM == 0 && freeVRAM == 0) {
			oss << "(unknown)";
		} else {
			if(totalVRAM != 0) {
				oss << (totalVRAM / 1024 / 1024) << " MiB";
				CrashHandler::setVariable("VRAM size", totalVRAM);
			}
			if(totalVRAM != 0 && freeVRAM != 0) {
				oss << ", ";
			}
			if(freeVRAM != 0) {
				oss << (freeVRAM / 1024 / 1024) << " MiB free";
				CrashHandler::setVariable("VRAM available", freeVRAM);
			}
		}
		LogInfo << " └─ VRAM: " << oss.str();
	}
	
	{
		std::ostringstream oss;
		oss << "GLEW " << glewVersion << '\n';
		const char * start = reinterpret_cast<const char *>(glVersion);
		while(*start == ' ') {
			start++;
		}
		const char * end = start;
		while(*end != '\0' && *end != ' ') {
			end++;
		}
		oss << "OpenGL ";
		oss.write(start, end - start);
		credits::setLibraryCredits("graphics", oss.str());
	}
	
	gldebug::initialize();
}

void OpenGLRenderer::beforeResize(bool wasOrIsFullscreen) {
	
#if ARX_PLATFORM == ARX_PLATFORM_LINUX || ARX_PLATFORM == ARX_PLATFORM_BSD
	// No re-initialization needed
	ARX_UNUSED(wasOrIsFullscreen);
#else
	
	if(!isInitialized()) {
		return;
	}
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(!wasOrIsFullscreen) {
		return;
	}
	#else
	// By default, always reinit to avoid issues on untested platforms
	ARX_UNUSED(wasOrIsFullscreen);
	#endif
	
	shutdown();
	
#endif
	
}

void OpenGLRenderer::afterResize() {
	if(!isInitialized()) {
		reinit();
	}
}

void OpenGLRenderer::reinit() {
	
	arx_assert(!isInitialized());
	
	m_hasTextureNPOT = GLEW_ARB_texture_non_power_of_two || GLEW_VERSION_2_0;
	if(!m_hasTextureNPOT) {
		LogWarning << "Missing OpenGL extension ARB_texture_non_power_of_two.";
	} else if(!GLEW_VERSION_3_0) {
		GLint max = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if(max < 8192) {
			LogWarning << "Old hardware detected, ignoring OpenGL extension ARB_texture_non_power_of_two.";
			m_hasTextureNPOT = false;
		}
	}
	
	useVertexArrays = true;
	
	if(!GLEW_ARB_draw_elements_base_vertex) {
		LogWarning << "Missing OpenGL extension ARB_draw_elements_base_vertex!";
	}
	
	useVBOs = useVertexArrays;
	if(useVBOs && !GLEW_ARB_map_buffer_range) {
		LogWarning << "Missing OpenGL extension ARB_map_buffer_range, VBO performance will suffer.";
	}

	// Synchronize GL state cache
	
	m_glcull = GL_BACK;
	m_glstate.setCull(CullNone);
	
	glFogi(GL_FOG_MODE, GL_LINEAR);
	m_glstate.setFog(false);
	
	SetAlphaFunc(CmpNotEqual, 0.0f);
	m_glstate.setColorKey(false);
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	m_glstate.setDepthTest(false);
	
	m_glstate.setDepthWrite(true);
	
	glEnable(GL_POLYGON_OFFSET_FILL);
	m_glstate.setDepthOffset(0);
	
	glEnable(GL_BLEND);
	m_glstate.setBlend(BlendOne, BlendZero);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	
	// number of conventional fixed-function texture units
	GLint texunits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &texunits);
	m_TextureStages.resize(texunits, NULL);
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		m_TextureStages[i] = new GLTextureStage(this, i);
	}
	
	// Clear screen
	Clear(ColorBuffer | DepthBuffer);
	
	currentTransform = GL_UnsetTransform;
	switchVertexArray(GL_NoArray, 0, 0);
	
	if(useVertexArrays && useVBOs) {
		if(!GLEW_ARB_shader_objects) {
			LogWarning << "Missing OpenGL extension ARB_shader_objects.";
		} else if(!GLEW_ARB_vertex_program) {
			LogWarning << "Missing OpenGL extension ARB_vertex_program.";
		} else {
			shader = loadVertexShader(vertexShaderSource);
		}
		if(!shader) {
			LogWarning << "Missing vertex shader, cannot use vertex arrays for pre-transformed vertices.";
		}
	}
	
	if(GLEW_EXT_texture_filter_anisotropic) {
		GLfloat limit;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &limit);
		m_maximumSupportedAnisotropy = limit;
		setMaxAnisotropy(float(config.video.maxAnisotropicFiltering));
	}
	
	onRendererInit();
	
}

void OpenGLRenderer::shutdown() {
	
	arx_assert(isInitialized());
	
	onRendererShutdown();
	
	if(shader) {
		glDeleteObjectARB(shader);
	}
	
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		delete m_TextureStages[i];
	}
	m_TextureStages.clear();
	
	m_maximumAnisotropy = 1.f;
	m_maximumSupportedAnisotropy = 1.f;
	
}

static glm::mat4x4 projection;
static glm::mat4x4 view;

void OpenGLRenderer::enableTransform() {
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		return;
	}
	
	if(shader) {
		glUseProgram(0);
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(view));
		
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(projection));
	
	currentTransform = GL_ModelViewProjectionTransform;
}

void OpenGLRenderer::disableTransform() {
	
	if(currentTransform == GL_NoTransform) {
		return;
	}
	
	// D3D doesn't apply any transform for D3DTLVERTEX
	// but we still need to change from D3D to OpenGL coordinates
	
	if(shader) {
		glUseProgram(shader);
	} else {
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Change coordinate system from [0, width] x [0, height] to [-1, 1] x [-1, 1] and flip the y axis
	glTranslatef(-1.f, 1.f, 0.f);
	glScalef(2.f/viewport.width(), -2.f/viewport.height(), 1.f);
	
	// Change the viewport and pixel origins
	glTranslatef(.5f - viewport.left, .5f - viewport.top, 0.f);
	
	currentTransform = GL_NoTransform;
}

void OpenGLRenderer::SetViewMatrix(const glm::mat4x4 & matView) {
	
	if(!memcmp(&view, &matView, sizeof(glm::mat4x4))) {
		return;
	}
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		currentTransform = GL_UnsetTransform;
	}

	view = matView;
}

void OpenGLRenderer::GetViewMatrix(glm::mat4x4 & matView) const {
	matView = view;
}

void OpenGLRenderer::SetProjectionMatrix(const glm::mat4x4 & matProj) {
	
	if(!memcmp(&projection, &matProj, sizeof(glm::mat4x4))) {
		return;
	}
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		currentTransform = GL_UnsetTransform;
	}
	
	projection = matProj;
}

void OpenGLRenderer::GetProjectionMatrix(glm::mat4x4 & matProj) const {
	matProj = projection;
}

void OpenGLRenderer::ReleaseAllTextures() {
	for(TextureList::iterator it = textures.begin(); it != textures.end(); ++it) {
		it->Destroy();
	}
}

void OpenGLRenderer::RestoreAllTextures() {
	for(TextureList::iterator it = textures.begin(); it != textures.end(); ++it) {
		it->Restore();
	}
}

Texture2D * OpenGLRenderer::CreateTexture2D() {
	GLTexture2D * texture = new GLTexture2D(this);
	textures.push_back(*texture);
	return texture;
}

static const GLenum arxToGlPixelCompareFunc[] = {
	GL_NEVER, // CmpNever,
	GL_LESS, // CmpLess,
	GL_EQUAL, // CmpEqual,
	GL_LEQUAL, // CmpLessEqual,
	GL_GREATER, // CmpGreater,
	GL_NOTEQUAL, // CmpNotEqual,
	GL_GEQUAL, // CmpGreaterEqual,
	GL_ALWAYS // CmpAlways
};

void OpenGLRenderer::SetAlphaFunc(PixelCompareFunc func, float ref) {
	glAlphaFunc(arxToGlPixelCompareFunc[func], ref);
}

void OpenGLRenderer::SetViewport(const Rect & _viewport) {
	
	viewport = _viewport;
	
	// TODO maybe it's better to always have the viewport cover the whole window and use glScissor instead?
	
	int height = mainApp->getWindow()->getSize().y;
	
	glViewport(viewport.left, height - viewport.bottom, viewport.width(), viewport.height());
	
	if(currentTransform == GL_NoTransform) {
		currentTransform = GL_UnsetTransform;
	}
}

Rect OpenGLRenderer::GetViewport() {
	return viewport;
}

void OpenGLRenderer::SetScissor(const Rect & rect) {
	
	if(rect.isValid()) {
		glEnable(GL_SCISSOR_TEST);
		int height = mainApp->getWindow()->getSize().y;
		glScissor(rect.left, height - rect.bottom, rect.width(), rect.height());
	} else {
		glDisable(GL_SCISSOR_TEST);
	}
}

void OpenGLRenderer::Clear(BufferFlags bufferFlags, Color clearColor, float clearDepth, size_t nrects, Rect * rect) {
	
	GLbitfield buffers = 0;
	
	if(bufferFlags & ColorBuffer) {
		Color4f col = clearColor.to<float>();
		glClearColor(col.r, col.g, col.b, col.a);
		buffers |= GL_COLOR_BUFFER_BIT;
	}
	
	if(bufferFlags & DepthBuffer) {
		if(!m_glstate.getDepthWrite()) {
			// glClear() respects the depth mask
			glDepthMask(GL_TRUE);
			m_glstate.setDepthWrite(true);
		}
		glClearDepth((GLclampd)clearDepth);
		buffers |= GL_DEPTH_BUFFER_BIT;
	}
	
	if(nrects) {
		
		glEnable(GL_SCISSOR_TEST);
		
		int height = mainApp->getWindow()->getSize().y;
		
		for(size_t i = 0; i < nrects; i++) {
			glScissor(rect[i].left, height - rect[i].bottom, rect[i].width(), rect[i].height());
			glClear(buffers);
		}
		
		glDisable(GL_SCISSOR_TEST);
		
	} else {
		
		glClear(buffers);
		
	}
}

void OpenGLRenderer::SetFogColor(Color color) {
	Color4f colorf = color.to<float>();
	GLfloat fogColor[4]= {colorf.r, colorf.g, colorf.b, colorf.a};
	glFogfv(GL_FOG_COLOR, fogColor);
}

void OpenGLRenderer::SetFogParams(float fogStart, float fogEnd) {
	glFogf(GL_FOG_START, fogStart);
	glFogf(GL_FOG_END, fogEnd);
}

void OpenGLRenderer::SetAntialiasing(bool enable) {
	
	if(enable == m_hasMSAA) {
		return;
	}
	
	// The state used for color keying can differ between msaa and non-msaa.
	// Clear the old flushed state.
	if(m_glstate.getColorKey()) {
		bool colorkey = m_state.getColorKey();
		m_state.setColorKey(false);
		flushState();
		m_state.setColorKey(colorkey);
	}
	
	// This is mostly useless as multisampling must be enabled/disabled at GL context creation.
	if(enable) {
		glEnable(GL_MULTISAMPLE);
	} else {
		glDisable(GL_MULTISAMPLE);
	}
	m_hasMSAA = enable;
}

static const GLenum arxToGlFillMode[] = {
	GL_LINE,  // FillWireframe,
	GL_FILL,  // FillSolid
};

void OpenGLRenderer::SetFillMode(FillMode mode) {
	glPolygonMode(GL_FRONT_AND_BACK, arxToGlFillMode[mode]);
}

void OpenGLRenderer::setMaxAnisotropy(float value) {
	
	float maxAnisotropy = glm::clamp(value, 1.f, m_maximumSupportedAnisotropy);
	if(m_maximumAnisotropy == maxAnisotropy) {
		return;
	}
	
	m_maximumAnisotropy = maxAnisotropy;
	
	for(TextureList::iterator it = textures.begin(); it != textures.end(); ++it) {
		it->updateMaxAnisotropy();
	}
}

template <typename Vertex>
static VertexBuffer<Vertex> * createVertexBufferImpl(OpenGLRenderer * renderer,
                                                     size_t capacity,
                                                     Renderer::BufferUsage usage,
                                                     const std::string & setting) {
	
	bool matched = false;
	
	if(GLEW_ARB_map_buffer_range) {
		
		#ifdef GL_ARB_buffer_storage
		
		if(GLEW_ARB_buffer_storage) {
			
			if(setting.empty() || setting == "persistent-orphan") {
				if(usage != Renderer::Static) {
					return new GLPersistentOrphanVertexBuffer<Vertex>(renderer, capacity);
				}
				matched = true;
			}
			if(setting.empty() || setting == "persistent-x3") {
				if(usage == Renderer::Stream) {
					return new GLPersistentFenceVertexBuffer<Vertex, 3>(renderer, capacity, 3);
				}
				matched = true;
			}
			if(setting.empty() || setting == "persistent-x2") {
				if(usage == Renderer::Stream) {
					return new GLPersistentFenceVertexBuffer<Vertex, 3>(renderer, capacity, 2);
				}
				matched = true;
			}
			if(setting == "persistent-nosync") {
				if(usage != Renderer::Static) {
					return new GLPersistentUnsynchronizedVertexBuffer<Vertex>(renderer, capacity);
				}
				matched = true;
			}
			
		}
		
		#endif // GL_ARB_buffer_storage
		
		if(setting.empty() || setting == "maprange" || setting == "maprange+subdata") {
			return new GLMapRangeVertexBuffer<Vertex>(renderer, capacity, usage);
		}
		
	}
	
	if(setting.empty() || setting == "map" || setting == "map+subdata") {
		return new GLVertexBuffer<Vertex>(renderer, capacity, usage);
	}
	
	static bool warned = false;
	if(!matched && !warned) {
		LogWarning << "Ignoring unsupported video.buffer_upload setting: " << setting;
		warned = true;
	}
	return createVertexBufferImpl<Vertex>(renderer, capacity, usage, std::string());
}

template <typename Vertex>
static VertexBuffer<Vertex> * createVertexBufferImpl(OpenGLRenderer * renderer,
                                                     size_t capacity,
                                                     Renderer::BufferUsage usage) {
	const std::string & setting = config.video.bufferUpload;
	return createVertexBufferImpl<Vertex>(renderer, capacity, usage, setting);
}

VertexBuffer<TexturedVertex> * OpenGLRenderer::createVertexBufferTL(size_t capacity, BufferUsage usage) {
	if(useVBOs && shader) {
		return createVertexBufferImpl<TexturedVertex>(this, capacity, usage); 
	} else {
		return new GLNoVertexBuffer<TexturedVertex>(this, capacity);
	}
}

VertexBuffer<SMY_VERTEX> * OpenGLRenderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	if(useVBOs) {
		return createVertexBufferImpl<SMY_VERTEX>(this, capacity, usage);
	} else {
		return new GLNoVertexBuffer<SMY_VERTEX>(this, capacity);
	}
}

VertexBuffer<SMY_VERTEX3> * OpenGLRenderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	if(useVBOs) {
		return createVertexBufferImpl<SMY_VERTEX3>(this, capacity, usage);
	} else {
		return new GLNoVertexBuffer<SMY_VERTEX3>(this, capacity);
	}
}

const GLenum arxToGlPrimitiveType[] = {
	GL_TRIANGLES, // TriangleList,
	GL_TRIANGLE_STRIP, // TriangleStrip,
	GL_TRIANGLE_FAN, // TriangleFan,
	GL_LINES, // LineList,
	GL_LINE_STRIP // LineStrip
};

void OpenGLRenderer::drawIndexed(Primitive primitive, const TexturedVertex * vertices, size_t nvertices, unsigned short * indices, size_t nindices) {
	
	beforeDraw<TexturedVertex>();
	
	if(useVertexArrays && shader) {
		
		bindBuffer(GL_NONE);
		
		setVertexArray(vertices, vertices);
		
		glDrawRangeElements(arxToGlPrimitiveType[primitive], 0, nvertices - 1, nindices, GL_UNSIGNED_SHORT, indices);
		
	} else {
		
		glBegin(arxToGlPrimitiveType[primitive]);
		
		for(size_t i = 0; i < nindices; i++) {
			renderVertex(vertices[indices[i]]);
		}
		
		glEnd();
		
	}
}

bool OpenGLRenderer::getSnapshot(Image & image) {
	
	Vec2i size = mainApp->getWindow()->getSize();
	
	image.Create(size.x, size.y, Image::Format_R8G8B8);
	
	glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, image.GetData()); 
	
	image.FlipY();
	
	return true;
}

bool OpenGLRenderer::getSnapshot(Image & image, size_t width, size_t height) {
	
	// TODO handle scaling on the GPU so we don't need to download the whole image

	// duplication to ensure use of Image::Format_R8G8B8
	Image fullsize;
	Vec2i size = mainApp->getWindow()->getSize();
	fullsize.Create(size.x, size.y, Image::Format_R8G8B8);
	glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, fullsize.GetData()); 

	image.ResizeFrom(fullsize, width, height, true);

	return true;
}

static const GLenum arxToGlBlendFactor[] = {
	GL_ZERO, // BlendZero,
	GL_ONE, // BlendOne,
	GL_SRC_COLOR, // BlendSrcColor,
	GL_SRC_ALPHA, // BlendSrcAlpha,
	GL_ONE_MINUS_SRC_COLOR, // BlendInvSrcColor,
	GL_ONE_MINUS_SRC_ALPHA, // BlendInvSrcAlpha,
	GL_SRC_ALPHA_SATURATE, // BlendSrcAlphaSaturate,
	GL_DST_COLOR, // BlendDstColor,
	GL_DST_ALPHA, // BlendDstAlpha,
	GL_ONE_MINUS_DST_COLOR, // BlendInvDstColor,
	GL_ONE_MINUS_DST_ALPHA // BlendInvDstAlpha
};

void OpenGLRenderer::flushState() {
	
	if(m_glstate != m_state) {
		
		if(m_glstate.getCull() != m_state.getCull()) {
			if(m_state.getCull() == CullNone) {
				glDisable(GL_CULL_FACE);
			} else {
				if(m_glstate.getCull() == CullNone) {
					glEnable(GL_CULL_FACE);
				}
				GLenum glcull = m_state.getCull() == CullCW ? GL_BACK : GL_FRONT;
				if(m_glcull != glcull) {
					glCullFace(glcull);
					m_glcull = glcull;
				}
			}
		}
		
		if(m_glstate.getFog() != m_state.getFog()) {
			if(m_state.getFog()) {
				glEnable(GL_FOG);
			} else {
				glDisable(GL_FOG);
			}
		}
		
		bool useA2C = m_hasMSAA && config.video.colorkeyAlphaToCoverage;
		if(m_glstate.getColorKey() != m_state.getColorKey()
		   || (useA2C && m_state.getColorKey()
		       && m_glstate.isBlendEnabled() != m_state.isBlendEnabled())) {
			/* When rendering color-keyed textures with alpha blending enabled we still
			 * need to 'discard' transparent texels, as blending might not use the src alpha!
			 * On the other hand, we can't use GL_SAMPLE_ALPHA_TO_COVERAGE when blending
			 * as that could result in the src alpha being applied twice (e.g. for text).
			 * So we must toggle between alpha to coverage and alpha test when toggling blending.
			 */
			bool disableA2C = useA2C && !m_glstate.isBlendEnabled()
				                && (!m_state.getColorKey() || m_state.isBlendEnabled());
			bool enableA2C = useA2C && !m_state.isBlendEnabled()
				               && (!m_glstate.getColorKey() || m_glstate.isBlendEnabled());
			if(m_glstate.getColorKey()) {
				if(disableA2C) {
					glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				} else if(!m_state.getColorKey() || enableA2C) {
					glDisable(GL_ALPHA_TEST);
				}
			}
			if(m_state.getColorKey()) {
				if(enableA2C) {
					glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				} else if(!m_glstate.getColorKey() || disableA2C) {
					glEnable(GL_ALPHA_TEST);
				}
			}
		}
		
		if(m_glstate.getDepthTest() != m_state.getDepthTest()) {
			glDepthFunc(m_state.getDepthTest() ? GL_LEQUAL : GL_ALWAYS);
		}
		
		if(m_glstate.getDepthWrite() != m_state.getDepthWrite()) {
			glDepthMask(m_state.getDepthWrite() ? GL_TRUE : GL_FALSE);
		}
		
		if(m_glstate.getDepthOffset() != m_state.getDepthOffset()) {
			GLfloat depthOffset = -GLfloat(m_state.getDepthOffset());
			glPolygonOffset(depthOffset, depthOffset);
		}
		
		if(m_glstate.getBlendSrc() != m_state.getBlendSrc()
		   || m_glstate.getBlendDst() != m_state.getBlendDst()) {
			GLenum blendSrc = arxToGlBlendFactor[m_state.getBlendSrc()];
			GLenum blendDst = arxToGlBlendFactor[m_state.getBlendDst()];
			glBlendFunc(blendSrc, blendDst);
		}
		
		m_glstate = m_state;
	}
	
	for(size_t i = 0; i <= maxTextureStage; i++) {
		GetTextureStage(i)->apply();
	}
	
}

bool OpenGLRenderer::isFogInEyeCoordinates() {
	return true;
}
