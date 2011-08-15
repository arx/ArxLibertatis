
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

static void dump(const EERIEMATRIX & mat) {
	
	printf("%8f  %8f  %8f  %8f\n", mat._11, mat._12, mat._13, mat._14);
	printf("%8f  %8f  %8f  %8f\n", mat._21, mat._22, mat._23, mat._24);
	printf("%8f  %8f  %8f  %8f\n", mat._31, mat._32, mat._33, mat._34);
	printf("%8f  %8f  %8f  %8f\n", mat._41, mat._42, mat._43, mat._44);
	
}

static EERIEMATRIX projection;
static EERIEMATRIX view;

template <class T>
void selectTrasform() {
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1.f, 1.f, -1.f); // switch between LHS and RHS coordnate systems
	glMultMatrixf(&view._11);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1.f, 1.f, 0);
	const Vec2i & s = mainApp->GetWindow()->GetSize();
	glScalef(2.f/s.x, -2.f/s.y, 1.f);
	glMultMatrixf(&projection._11);
	
	CHECK_GL;
}

template <>
void selectTrasform<TexturedVertex>() {
	
	// TODO cache transformations
	
	// D3D doesn't apply any transform for D3DTLVERTEX
	// but we still need to change from D3D to OpenGL coordinates
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1.f, 1.f, -1.f); // switch between LHS and RHS coordnate systems
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glTranslatef(-1.f, 1.f, 0);
	const Vec2i & s = mainApp->GetWindow()->GetSize();
	glScalef(2.f/s.x, -2.f/s.y, 1.f);
	
	CHECK_GL;
}

void OpenGLRenderer::SetViewMatrix(const EERIEMATRIX & matView) {
	
	if(!memcmp(&view, &matView, sizeof(EERIEMATRIX))) {
		return;
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
			setGLState(GL_DEPTH_TEST, false); // TODO
			break;
		}
		
		case DepthWrite: {
			glDepthMask(enable ? GL_TRUE : GL_FALSE);
			break;
		}
		
		case Fog: {
			setGLState(GL_FOG, false); // TODO
			break;
		}
		
		case Lighting: {
			setGLState(GL_LIGHTING, false); // TODO
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
	GL_BACK, // CullCW,
	GL_FRONT, // CullCCW,
};

void OpenGLRenderer::SetCulling(CullingMode mode) {
	//if(mode == CullNone) {
		glDisable(GL_CULL_FACE); // TODO
	//} else {
	//	glEnable(GL_CULL_FACE);
	//	glCullFace(arxToGlCullMode[mode]);
	//}
	CHECK_GL;
}

void OpenGLRenderer::SetDepthBias(int depthBias) {
	
	float bias = -(float)depthBias; // TODO check this
	
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
	return new GLVertexBuffer<TexturedVertex>(this, capacity, usage); 
}

VertexBuffer<SMY_VERTEX> * OpenGLRenderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	return new GLVertexBuffer<SMY_VERTEX>(this, capacity, usage); 
}

VertexBuffer<SMY_VERTEX3> * OpenGLRenderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	return new GLVertexBuffer<SMY_VERTEX3>(this, capacity, usage); 
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
	selectTrasform<TexturedVertex>();
	
#if 1
	
	setVertexArray(vertices);
	
	glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
	
	glDrawRangeElements(arxToGlPrimitiveType[primitive], 0, nvertices - 1, nindices, GL_UNSIGNED_SHORT, indices);
	
#else
	
	ARX_UNUSED(nvertices);
	
	glBegin(arxToGlPrimitiveType[primitive]);
	
	for(size_t i = 0; i < nindices; i++) {
		renderVertex(vertices[indices[i]]);
	}
	
	glEnd();
	
#endif
	
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
