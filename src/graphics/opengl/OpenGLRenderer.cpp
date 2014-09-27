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

#include <glm/gtc/type_ptr.hpp>

#include "core/Application.h"
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
	, maximumAnisotropy(1.f)
	, m_hasMSAA(false)
	, m_hasColorKey(false)
	, m_hasBlend(false)
	, m_hasTextureNPOT(false)
{
	resetStateCache();
}

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
	
	LogInfo << "Using GLEW " << glewGetString(GLEW_VERSION);
	CrashHandler::setVariable("GLEW version", glewGetString(GLEW_VERSION));
	
	LogInfo << "Using OpenGL " << glGetString(GL_VERSION);
	CrashHandler::setVariable("OpenGL version", glGetString(GL_VERSION));
	
	LogInfo << " ├─ Vendor: " << glGetString(GL_VENDOR);
	CrashHandler::setVariable("OpenGL vendor", glGetString(GL_VENDOR));
	
	LogInfo << " └─ Device: " << glGetString(GL_RENDERER);
	CrashHandler::setVariable("OpenGL device", glGetString(GL_RENDERER));
	
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

	resetStateCache();
	
	SetRenderState(ZBias, true);
	
	glEnable(GL_DEPTH_TEST);
	SetRenderState(DepthTest, false);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	
	// number of conventional fixed-function texture units
	GLint texunits = 0;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &texunits);
	m_TextureStages.resize(texunits, NULL);
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		m_TextureStages[i] = new GLTextureStage(this, i);
	}
	
	SetRenderState(ColorKey, true);
	
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
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
	}
	
	onRendererInit();
	
}

void OpenGLRenderer::resetStateCache() {
	m_cachedStates.clear();
	m_cachedSrcBlend = BlendOne;
	m_cachedDstBlend = BlendZero;
	m_cachedDepthBias = 0;
	m_cachedCullMode = CullNone;
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
	
	maximumAnisotropy = 1.f;
	
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

bool OpenGLRenderer::getGLState(GLenum state) const {
	BoolStateCache::iterator it = m_cachedStates.find(state);
	
	if(it != m_cachedStates.end()) {
		return it->second;
	}

	bool isEnabled = glIsEnabled(state) == GL_TRUE;
	m_cachedStates[state] = isEnabled;

	return isEnabled;
}

void OpenGLRenderer::setGLState(GLenum state, bool enable) {
	BoolStateCache::iterator it = m_cachedStates.find(state);

	// No change ?
	if(it == m_cachedStates.end() || it->second != enable) {
		if(enable) {
			glEnable(state);
		} else {
			glDisable(state);
		}
		m_cachedStates[state] = enable;
	}
}

bool OpenGLRenderer::GetRenderState(RenderState renderState) const {

	switch(renderState) {
		
		case AlphaBlending: {
			return m_hasBlend;
		}
		
		case ColorKey: {
			return m_hasColorKey;
		}
		
		case DepthTest: {
			return getGLState(GL_DEPTH_TEST);
		}
				
		case Fog: {
			return getGLState(GL_FOG);
		}
		
		case Lighting: {
			return getGLState(GL_LIGHTING);
		}
		
		case ZBias: {
			return getGLState(GL_POLYGON_OFFSET_FILL);
		}
		
		default:
			LogWarning << "Unsupported render state: " << renderState;
	}

	return false;
}

void OpenGLRenderer::SetRenderState(RenderState renderState, bool enable) {
	
	switch(renderState) {
		
		case AlphaBlending: {
			if(m_hasBlend == enable) {
				return;
			}
			/*
			 * When rendering color-keyed textures with GL_BLEND enabled we still
			 * need to 'discard' transparent texels, as blending might not use the src alpha!
			 * On the other hand, we can't use GL_SAMPLE_ALPHA_TO_COVERAGE when blending
			 * as that could result in the src alpha being applied twice (e.g. for text).
			 * So we must toggle between alpha to coverage and alpha test when toggling blending.
			 * TODO Fix it so that the apha channel from the color-keyed textures is
			 *      always used as part of the blending factor.
			 */
			bool colorkey = m_hasColorKey;
			if(colorkey && m_hasMSAA) {
				SetRenderState(ColorKey, false);
			}
			m_hasBlend = enable;
			if(colorkey && m_hasMSAA) {
				SetRenderState(ColorKey, true);
			}
			setGLState(GL_BLEND, enable);
			break;
		}
		
		case ColorKey: {
			if(m_hasColorKey == enable) {
				return;
			}
			m_hasColorKey = enable;
			if(m_hasMSAA && !m_hasBlend) {
				// TODO(option-video) add a config option for this
				setGLState(GL_SAMPLE_ALPHA_TO_COVERAGE, enable);
			} else {
				setGLState(GL_ALPHA_TEST, enable);
				if(enable) {
					SetAlphaFunc(CmpNotEqual, 0.0f);
				}
			}
			break;
		}
		
		case DepthTest: {
			glDepthFunc(enable ? GL_LEQUAL : GL_ALWAYS);
			break;
		}
		
		case DepthWrite: {
			glDepthMask(enable ? GL_TRUE : GL_FALSE);
			break;
		}
		
		case Fog: {
			setGLState(GL_FOG, enable);
			break;
		}
		
		case Lighting: {
			setGLState(GL_LIGHTING, enable);
			break;
		}
		
		case ZBias: {
			setGLState(GL_POLYGON_OFFSET_FILL, enable);
			break;
		}
		
		default:
			LogWarning << "Unsupported render state: " << renderState;
	}
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

void OpenGLRenderer::GetBlendFunc(PixelBlendingFactor& srcFactor, PixelBlendingFactor& dstFactor) const {
	srcFactor = m_cachedSrcBlend;
	dstFactor = m_cachedDstBlend;
}

void OpenGLRenderer::SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor) {
	if(srcFactor != m_cachedSrcBlend || dstFactor != m_cachedDstBlend) {
		glBlendFunc(arxToGlBlendFactor[srcFactor], arxToGlBlendFactor[dstFactor]);
		m_cachedSrcBlend = srcFactor;
		m_cachedDstBlend = dstFactor;
	}
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

void OpenGLRenderer::Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar) {
	ARX_UNUSED(left), ARX_UNUSED(right), ARX_UNUSED(bottom), ARX_UNUSED(top), ARX_UNUSED(zNear), ARX_UNUSED(zFar);
	// Do nothing!
}

void OpenGLRenderer::End2DProjection() {
	// Do nothing!
}

void OpenGLRenderer::Clear(BufferFlags bufferFlags, Color clearColor, float clearDepth, size_t nrects, Rect * rect) {
	
	GLbitfield buffers = 0;
	
	if(bufferFlags & ColorBuffer) {
		Color4f col = clearColor.to<float>();
		glClearColor(col.r, col.g, col.b, col.a);
		buffers |= GL_COLOR_BUFFER_BIT;
	}
	
	if(bufferFlags & DepthBuffer) {
		glClearDepth(clearDepth);
		buffers |= GL_DEPTH_BUFFER_BIT;
	}
	
	if(bufferFlags & StencilBuffer) {
		buffers |= GL_STENCIL_BUFFER_BIT;
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

static const GLint arxToGlFogMode[] = {
	-1, // FogNone, TODO(unused) why is there a FogNone if there is also a separate Fog render state?
	GL_EXP, // FogExp,
	GL_EXP2, // FogExp2,
	GL_LINEAR, // FogLinear
};


void OpenGLRenderer::SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity) {
	
	glFogi(GL_FOG_MODE, arxToGlFogMode[fogMode]);
	
	glFogf(GL_FOG_START, fogStart);
	glFogf(GL_FOG_END, fogEnd);
	glFogf(GL_FOG_DENSITY, fogDensity);
}

void OpenGLRenderer::SetAntialiasing(bool enable) {
	
	bool colorkey = m_hasColorKey;
	if(colorkey) {
		SetRenderState(ColorKey, false);
	}
	
	// This is mostly useless as multisampling must be enabled/disabled at GL context creation.
	setGLState(GL_MULTISAMPLE, enable);
	
	m_hasMSAA = enable;
	
	if(colorkey) {
		SetRenderState(ColorKey, true);
	}
}

static const GLenum arxToGlCullMode[] = {
	(GLenum)-1, // CullNone,
	GL_BACK, // CullCW,
	GL_FRONT, // CullCCW,
};

Renderer::CullingMode OpenGLRenderer::GetCulling() const {
	return m_cachedCullMode;
}

void OpenGLRenderer::SetCulling(CullingMode mode) {
	if(mode == m_cachedCullMode)
		return;

	m_cachedCullMode = mode;

	if(mode == CullNone) {
		setGLState(GL_CULL_FACE, false);
	} else {
		setGLState(GL_CULL_FACE, true);
		glCullFace(arxToGlCullMode[mode]);
	}
}

int OpenGLRenderer::GetDepthBias() const {
	return m_cachedDepthBias;
}

void OpenGLRenderer::SetDepthBias(int depthBias) {
	if(depthBias == m_cachedDepthBias)
		return;
	
	m_cachedDepthBias = depthBias;

	float bias = -(float)m_cachedDepthBias;
	
	glPolygonOffset(bias, bias);
}

static const GLenum arxToGlFillMode[] = {
	GL_POINT, // FillPoint,
	GL_LINE,  // FillWireframe,
	GL_FILL,  // FillSolid
};

void OpenGLRenderer::SetFillMode(FillMode mode) {
	glPolygonMode(GL_FRONT_AND_BACK, arxToGlFillMode[mode]);
}

VertexBuffer<TexturedVertex> * OpenGLRenderer::createVertexBufferTL(size_t capacity, BufferUsage usage) {
	if(useVBOs && shader) {
		return new GLVertexBuffer<TexturedVertex>(this, capacity, usage); 
	} else {
		return new GLNoVertexBuffer<TexturedVertex>(this, capacity); 
	}
}

VertexBuffer<SMY_VERTEX> * OpenGLRenderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	if(useVBOs) {
		return new GLVertexBuffer<SMY_VERTEX>(this, capacity, usage);
	} else {
		return new GLNoVertexBuffer<SMY_VERTEX>(this, capacity);
	}
}

VertexBuffer<SMY_VERTEX3> * OpenGLRenderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	if(useVBOs) {
		return new GLVertexBuffer<SMY_VERTEX3>(this, capacity, usage);
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

void OpenGLRenderer::applyTextureStages() {
	for(size_t i = 0; i <= maxTextureStage; i++) {
		GetTextureStage(i)->apply();
	}
}

bool OpenGLRenderer::isFogInEyeCoordinates() {
	return true;
}
