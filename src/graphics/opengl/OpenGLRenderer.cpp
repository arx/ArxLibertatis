
#include "graphics/opengl/OpenGLRenderer.h"

#include <cstdio>

#include "core/Application.h"
#include "graphics/Math.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/opengl/GLNoVertexBuffer.h"
#include "graphics/opengl/GLTexture2D.h"
#include "graphics/opengl/GLTextureStage.h"
#include "graphics/opengl/GLVertexBuffer.h"
#include "io/Logger.h"
#include "window/RenderWindow.h"

OpenGLRenderer::OpenGLRenderer() { };

OpenGLRenderer::~OpenGLRenderer() { };

enum GLTransformMode {
	GL_UnsetTransform,
	GL_NoTransform,
	GL_ModelViewProjectionTransform
};

static GLTransformMode currentTransform;

void OpenGLRenderer::Initialize() {
	
	if(glewInit() != GLEW_OK) {
		LogError << "GLEW init failed";
	}
	
	if(!GLEW_ARB_vertex_array_bgra) {
		LogWarning << "Missing OpenGL extension ARB_vertex_array_bgra";
	}
	
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POLYGON_OFFSET_FILL);
	
	glDepthFunc(GL_LEQUAL);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	
	// TODO get the supported texture stage count
	m_TextureStages.resize(3);
	
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		m_TextureStages[i] = new GLTextureStage(this, i);
	}
	
	SetRenderState(ColorKey, true);
	
	// Clear screen
	Clear(ColorBuffer | DepthBuffer);
	
	LogInfo << "Using OpenGL " << glGetString(GL_VERSION);
	LogInfo << "Vendor: " << glGetString(GL_VENDOR);
	LogInfo << "Device: " << glGetString(GL_RENDERER);
	
	currentTransform = GL_UnsetTransform;
	
	CHECK_GL;
}

bool OpenGLRenderer::BeginScene() {
	return true; // nothing to do?
}

bool OpenGLRenderer::EndScene() {
	
	glFlush();
	
	return true;
}

static EERIEMATRIX projection;
static EERIEMATRIX view;

void OpenGLRenderer::enableTransform() {
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		return;
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&view._11);
		
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&projection._11);
	
	currentTransform = GL_ModelViewProjectionTransform;
	
	CHECK_GL;
}

void OpenGLRenderer::disableTransform() {
	
	if(currentTransform == GL_NoTransform) {
		return;
	}
	
	// D3D doesn't apply any transform for D3DTLVERTEX
	// but we still need to change from D3D to OpenGL coordinates
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Change coordinate system from [0, width] x [0, height] to [-1, 1] x [-1, 1] and flip the y axis
	glTranslatef(-1.f, 1.f, 0.f);
	glScalef(2.f/viewport.width(), -2.f/viewport.height(), 1.f);
	
	// Change the viewport and pixel origins
	glTranslatef(.5f - viewport.left, .5f - viewport.top, 0.f);
	
	currentTransform = GL_NoTransform;
	
	CHECK_GL;
}

void OpenGLRenderer::SetViewMatrix(const EERIEMATRIX & matView) {
	
	if(!memcmp(&view, &matView, sizeof(EERIEMATRIX))) {
		return;
	}
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		currentTransform = GL_UnsetTransform;
	}

	view = matView;
}

void OpenGLRenderer::GetViewMatrix(EERIEMATRIX & matView) const {
	matView = view;
}

void OpenGLRenderer::SetProjectionMatrix(const EERIEMATRIX & matProj) {
	
	if(!memcmp(&projection, &matProj, sizeof(EERIEMATRIX))) {
		return;
	}
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		currentTransform = GL_UnsetTransform;
	}

	projection = matProj;
}

void OpenGLRenderer::GetProjectionMatrix(EERIEMATRIX & matProj) const {
	matProj = projection;
}

void OpenGLRenderer::ReleaseAllTextures() {
	// TODO implement
}

void OpenGLRenderer::RestoreAllTextures() {
	// TODO implement
}

Texture2D * OpenGLRenderer::CreateTexture2D() {
	return new GLTexture2D(this);
}

static inline void setGLState(GLenum state, bool enable) {
	if(enable) {
		glEnable(state);
	} else {
		glDisable(state);
	}
}

void OpenGLRenderer::SetRenderState(RenderState renderState, bool enable) {
	
	switch(renderState) {
		
		case AlphaBlending: {
			setGLState(GL_BLEND, enable);
			break;
		}
		
		case AlphaTest: {
			setGLState(GL_ALPHA_TEST, enable);
			break;
		}

		case ColorKey: {
			SetRenderState(AlphaTest, enable);
			if(enable)
				SetAlphaFunc(CmpNotEqual, 0.0f);
			break;
		}
		
		case DepthTest: {
			setGLState(GL_DEPTH_TEST, enable);
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
			LogWarning << "unsupported render state: " << renderState;
	}
	
	CHECK_GL;
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
	CHECK_GL;
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

void OpenGLRenderer::SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor) {
	glBlendFunc(arxToGlBlendFactor[srcFactor], arxToGlBlendFactor[dstFactor]);
	CHECK_GL;
}

void OpenGLRenderer::SetViewport(const Rect & _viewport) {
	
	viewport = _viewport;
	
	// TODO maybe it's better to always have the viewport cover the whole window and use glScissor instead?
	
	int height = mainApp->GetWindow()->GetSize().y;
	
	glViewport(viewport.left, height - viewport.bottom, viewport.width(), viewport.height());
	
	if(currentTransform == GL_NoTransform) {
		currentTransform = GL_UnsetTransform;
	}
	
	CHECK_GL;
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
		
		int height = mainApp->GetWindow()->GetSize().y;
		
		for(size_t i = 0; i < nrects; i++) {
			glScissor(rect[i].left, height - rect[i].bottom, rect[i].width(), rect[i].height());
			glClear(buffers);
		}
		
		glDisable(GL_SCISSOR_TEST);
		
	} else {
		
		glClear(buffers);
		
	}
	
	CHECK_GL;
}

void OpenGLRenderer::SetFogColor(Color color) {
	Color4f colorf = color.to<float>();
	GLfloat fogColor[4]= {colorf.r, colorf.g, colorf.b, colorf.a};
	glFogfv(GL_FOG_COLOR, fogColor);
	CHECK_GL;
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
	
	CHECK_GL;
}

void OpenGLRenderer::SetAntialiasing(bool enable) {
	
	setGLState(GL_POLYGON_SMOOTH, enable);
	setGLState(GL_LINE_SMOOTH, enable);
	setGLState(GL_POINT_SMOOTH, enable);
	
	CHECK_GL;
}

static const GLenum arxToGlCullMode[] = {
	-1, // CullNone,
	GL_BACK, // CullCW,
	GL_FRONT, // CullCCW,
};

void OpenGLRenderer::SetCulling(CullingMode mode) {
	if(mode == CullNone) {
		glDisable(GL_CULL_FACE);
	} else {
		glEnable(GL_CULL_FACE);
		glCullFace(arxToGlCullMode[mode]);
	}
	CHECK_GL;
}

void OpenGLRenderer::SetDepthBias(int depthBias) {
	
	float bias = -(float)depthBias;
	
	glPolygonOffset(bias, bias);
	
	CHECK_GL;
}

static const GLenum arxToGlFillMode[] = {
	GL_POINT, // FillPoint,
	GL_LINE,  // FillWireframe,
	GL_FILL,  // FillSolid
};

void OpenGLRenderer::SetFillMode(FillMode mode) {
	glPolygonMode(GL_FRONT_AND_BACK, arxToGlFillMode[mode]);
	CHECK_GL;
}

float OpenGLRenderer::GetMaxAnisotropy() const {
	
	float maximumAnistropy = 1.f;
	
	if(GLEW_EXT_texture_filter_anisotropic) {
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnistropy);
		CHECK_GL;
	}
	
	return maximumAnistropy;
}

void OpenGLRenderer::DrawTexturedRect(float x, float y, float w, float h, float uStart, float vStart, float uEnd, float vEnd, Color color) {
	
	applyTextureStages();
	disableTransform();
	
	x -= .5f;
	y -= .5f;
	
	glColor4ub(color.r, color.g, color.b, color.a);
	
	glSecondaryColor3ub(0, 0, 0);
	
	glBegin(GL_QUADS);
		
		glMultiTexCoord2f(GL_TEXTURE0, uStart, vStart);
		glVertex3f(x, y, 0);
		
		glMultiTexCoord2f(GL_TEXTURE0, uEnd, vStart);
		glVertex3f(x + w, y, 0);
		
		glMultiTexCoord2f(GL_TEXTURE0, uEnd, vEnd);
		glVertex3f(x + w, y + h, 0);
		
		glMultiTexCoord2f(GL_TEXTURE0, uStart, vEnd);
		glVertex3f(x, y + h, 0);
		
	glEnd();
	
	CHECK_GL;
}

VertexBuffer<TexturedVertex> * OpenGLRenderer::createVertexBufferTL(size_t capacity, BufferUsage usage) {
	return new GLNoVertexBuffer<TexturedVertex>(this, capacity, usage); 
}

VertexBuffer<SMY_VERTEX> * OpenGLRenderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	return new GLNoVertexBuffer<SMY_VERTEX>(this, capacity, usage); 
}

VertexBuffer<SMY_VERTEX3> * OpenGLRenderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	return new GLNoVertexBuffer<SMY_VERTEX3>(this, capacity, usage); 
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
	
	if(GLEW_ARB_vertex_array_bgra) {
		
		glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
		
		setVertexArray(vertices);
		
		glDrawRangeElements(arxToGlPrimitiveType[primitive], 0, nvertices - 1, nindices, GL_UNSIGNED_SHORT, indices);
		
	} else {
		
		ARX_UNUSED(nvertices);
		
		glBegin(arxToGlPrimitiveType[primitive]);
		
		for(size_t i = 0; i < nindices; i++) {
			renderVertex(vertices[indices[i]]);
		}
		
		glEnd();
		
	}
	
	CHECK_GL;
}

bool OpenGLRenderer::getSnapshot(Image & image) {
	
	Vec2i size = mainApp->GetWindow()->GetSize();
	
	image.Create(size.x, size.y, Image::Format_R8G8B8);
	
	glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, image.GetData()); 
	
	CHECK_GL;
	
	return true;
}

bool OpenGLRenderer::getSnapshot(Image & image, size_t width, size_t height) {
	
	// TODO handle scaling on the GPU so we don't need to download the whole image
	
	Image fullsize;
	
	getSnapshot(fullsize);
	
 	image.Create(width, height, Image::Format_R8G8B8);
	
	GLint ret = gluScaleImage(GL_RGB, fullsize.GetWidth(), fullsize.GetHeight(), GL_UNSIGNED_BYTE,
	                          fullsize.GetData(), width, height, GL_UNSIGNED_BYTE, image.GetData());
	
	if(ret) {
		LogWarning << "Failed to scaled down screen capture: " << ret << " = " << gluErrorString(ret);
		return false;
	}
	
	return true;
}

void OpenGLRenderer::applyTextureStages() {
	
	std::vector<TextureStage *>::const_iterator i = m_TextureStages.begin();
	for(; i != m_TextureStages.end(); ++i) {
		reinterpret_cast<GLTextureStage *>(*i)->apply();
	}
}
