
#include "graphics/opengl/OpenGLRenderer.h"

#include <cstdio>

#include "core/Application.h"
#include "core/RenderWindow.h"
#include "graphics/Math.h"
#include "graphics/GraphicsUtility.h"
#include "graphics/opengl/GLNoVertexBuffer.h"
#include "graphics/opengl/GLTexture2D.h"
#include "graphics/opengl/GLTextureStage.h"
#include "graphics/opengl/GLVertexBuffer.h"
#include "io/Logger.h"

OpenGLRenderer::OpenGLRenderer() { };

OpenGLRenderer::~OpenGLRenderer() { };

void OpenGLRenderer::Initialize() {
	
	if(glewInit() != GLEW_OK) {
		LogError << "GLEW init failed";
	}
	
	if(!GLEW_ARB_vertex_array_bgra) {
		LogError << "Missing OpenGL extension ARB_vertex_array_bgra";
	}
	
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	
	glEnable(GL_TEXTURE_2D);
	
	glDepthFunc(GL_LESS);
	
	// glShadeModel(GL_SMOOTH);
	
	// TODO get the supported texture stage count
	m_TextureStages.resize(3);
	
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		m_TextureStages[i] = new GLTextureStage(i);
	}
	
	SetRenderState(ColorKey, true);
	
	// Clear screen
	Clear(ColorBuffer | DepthBuffer);
	
	LogInfo << "OpenGL renderer initialized: " << glGetError();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	
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

static void transpose(EERIEMATRIX & mat) {
	
	std::swap(mat._12, mat._21);
	std::swap(mat._13, mat._31);
	std::swap(mat._14, mat._41);
	std::swap(mat._23, mat._32);
	std::swap(mat._24, mat._42);
	std::swap(mat._34, mat._43);
	
}

static void dump(const EERIEMATRIX & mat) {
	
	printf("%8f  %8f  %8f  %8f\n", mat._11, mat._12, mat._13, mat._14);
	printf("%8f  %8f  %8f  %8f\n", mat._21, mat._22, mat._23, mat._24);
	printf("%8f  %8f  %8f  %8f\n", mat._31, mat._32, mat._33, mat._34);
	printf("%8f  %8f  %8f  %8f\n", mat._41, mat._42, mat._43, mat._44);
	
}

void OpenGLRenderer::SetViewMatrix(const EERIEMATRIX & matView) {
	
	if(!memcmp(&view, &matView, sizeof(EERIEMATRIX))) {
		return;
	}
	
	view = matView;
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/*
	EERIEMATRIX mat;
	
	EERIEMATRIX a;
	a._11 = 1.f, a._21 = 0.f, a._31 = 0.f, a._41 = 0.f;
	a._12 = 0.f, a._22 = 1.f, a._32 = 0.f, a._42 = 0.f;
	a._13 = 0.f, a._23 = 0.f, a._33 = -1.f, a._43 = 0.f;
	a._14 = 0.f, a._24 = 0.f, a._34 = 0.f, a._44 = 1.f;
	
	MatrixMultiply(&mat, &matView, &a);
	*/
	//glLoadMatrixf(&matView._11);
	
	CHECK_GL;
}

void OpenGLRenderer::GetViewMatrix(EERIEMATRIX & matView) const {
	matView = view;
}

void OpenGLRenderer::SetProjectionMatrix(const EERIEMATRIX & matProj) {
	
	if(!memcmp(&projection, &matProj, sizeof(EERIEMATRIX))) {
		return;
	}
	
	projection = matProj;
	
	glMatrixMode(GL_PROJECTION);
	
	LogInfo << "projection";
	
	dump(matProj);
	
	EERIEMATRIX mat;
	
	EERIEMATRIX a;
	const Vec2i & s = mainApp->GetWindow()->GetSize();
	a._11 = 2.f, a._21 = 0.f, a._31 = 0.f, a._41 = -1.f;
	a._12 = 0.f, a._22 = -2.f, a._32 = 0.f, a._42 = 1.f;
	a._13 = 0.f, a._23 = 0.f, a._33 = -1.f, a._43 = 0.f;
	a._14 = 0.f, a._24 = 0.f, a._34 = 0.f, a._44 = 1.f;
	
	MatrixMultiply(&mat, &a, &matProj);
	
	LogInfo << "---------";
	
	dump(mat);
	
	glLoadMatrixf(&a._11);
	
	CHECK_GL;
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
	return new GLTexture2D;
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
		
		case ColorKey: {
			// TODO implement
			break;
		}
		
		case DepthTest: {
			setGLState(GL_DEPTH_TEST, enable); // TODO
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
	GL_ZERO, // BlendZero,              //!< Zero
	GL_ONE, // BlendOne,               //!< One
	GL_SRC_COLOR, // BlendSrcColor,          //!< Source color
	GL_SRC_ALPHA, // BlendSrcAlpha,          //!< Source alpha
	GL_ONE_MINUS_SRC_COLOR, // BlendInvSrcColor,       //!< Inverse source color
	GL_ONE_MINUS_SRC_ALPHA, // BlendInvSrcAlpha,       //!< Inverse source alpha
	GL_SRC_ALPHA_SATURATE, // BlendSrcAlphaSaturate,  //!< Source alpha saturate
	GL_DST_COLOR, // BlendDstColor,          //!< Destination color
	GL_DST_ALPHA, // BlendDstAlpha,          //!< Destination alpha
	GL_ONE_MINUS_DST_COLOR, // BlendInvDstColor,       //!< Inverse destination color
	GL_ONE_MINUS_DST_ALPHA // BlendInvDstAlpha        //!< Inverse destination alpha
};

void OpenGLRenderer::SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor) {
	glBlendFunc(arxToGlBlendFactor[srcFactor], arxToGlBlendFactor[dstFactor]);
	CHECK_GL;
}

void OpenGLRenderer::SetViewport(const Rect & viewport) {
	glViewport(viewport.left, viewport.top, viewport.width(), viewport.height());
	CHECK_GL;
}

Rect OpenGLRenderer::GetViewport() {
	
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	CHECK_GL;
	
	return Rect(Vec2i(viewport[0], viewport[1]), viewport[2], viewport[3]);
}

void OpenGLRenderer::Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar) {
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(left, right, bottom, top, zNear, zFar);
	
	EERIEMATRIX mat;
	glGetFloatv(GL_PROJECTION_MATRIX, &mat._11);
	LogInfo << "2d proj";
	dump(mat);
	
	CHECK_GL;
}

void OpenGLRenderer::End2DProjection() {
	
	SetProjectionMatrix(projection);
	
	SetViewMatrix(view);
	
	CHECK_GL;
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
		
		for(size_t i = 0; i < nrects; i++) {
			glScissor(rect[i].left, rect[i].top, rect[i].width(), rect[i].height());
			glClear(buffers);
		}
		
		glDisable(GL_SCISSOR_TEST);
		
	} else {
		
		glClear(buffers);
		
	}
	
	CHECK_GL;
}

void OpenGLRenderer::SetFogColor(Color color) {
	ARX_UNUSED(color); // TODO implement
}

void OpenGLRenderer::SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity) {
	ARX_UNUSED(fogMode), ARX_UNUSED(fogStart), ARX_UNUSED(fogEnd), ARX_UNUSED(fogDensity);
	// TODO implement
}

void OpenGLRenderer::SetAntialiasing(bool enable) {
	
	setGLState(GL_POLYGON_SMOOTH, enable);
	setGLState(GL_LINE_SMOOTH, enable);
	setGLState(GL_POINT_SMOOTH, enable);
	
	CHECK_GL;
}

static const GLenum arxToGlCullMode[] = {
	-1, // CullNone,
	GL_FRONT, // CullCCW,
	GL_BACK, // CullCW,
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
	
	float bias = -(float)depthBias; // TODO this seems to be what wine is doing
	
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
	
	float maximumAnistropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnistropy);
	
	CHECK_GL;
	
	return maximumAnistropy;
}

void OpenGLRenderer::DrawTexturedRect(float x, float y, float w, float h, float uStart, float vStart, float uEnd, float vEnd, Color color) {
	
	applyTextureStages();
	
	glColor4ub(color.r, color.g, color.b, color.a);
	
	glSecondaryColor3ub(0, 0, 0);
	
	x -= 0.5f;
	y -= 0.5f;
	
	glBegin(GL_QUADS);
		
		glTexCoord2f(uStart, vStart);
		glVertex3f(x, y, 0);
		
		glTexCoord2f(uEnd, vStart);
		glVertex3f(x + w, y, 0);
		
		glTexCoord2f(uEnd, vEnd);
		glVertex3f(x + w, y + h, 0);
		
		glTexCoord2f(uStart, vEnd);
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
	
	applyTextureStages();
	
	/*
	
	setVertexArray(vertices);
	
	glDrawRangeElements(arxToGlPrimitiveType[primitive], 0, nvertices - 1, nindices, GL_UNSIGNED_SHORT, indices);
	
	*/
	
	glBegin(arxToGlPrimitiveType[primitive]);
	
	for(size_t i = 0; i < nvertices; i++) {
		renderVertex(vertices[indices[i]]);
	}
	
	glEnd();
	
	CHECK_GL;
}

bool OpenGLRenderer::getSnapshot(Image & image) {
	ARX_UNUSED(image);
	return false; // TODO implement
}

bool OpenGLRenderer::getSnapshot(Image & image, size_t width, size_t height) {
	ARX_UNUSED(image), ARX_UNUSED(width), ARX_UNUSED(height);
	return false; // TODO implement
}

void OpenGLRenderer::setGamma(float brightness, float contrast, float gamma) {
	ARX_UNUSED(brightness), ARX_UNUSED(contrast), ARX_UNUSED(gamma); // TODO implement
}

void OpenGLRenderer::applyTextureStages() {
	
	std::vector<TextureStage *>::const_iterator i = m_TextureStages.begin();
	for(; i != m_TextureStages.end(); ++i) {
		reinterpret_cast<GLTextureStage *>(*i)->apply();
	}
}
