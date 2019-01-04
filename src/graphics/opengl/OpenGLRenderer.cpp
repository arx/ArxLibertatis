/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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
#include <cstring>

#include <glm/gtc/type_ptr.hpp>

#include "platform/Platform.h"
#include "Configure.h"

#if ARX_HAVE_EPOXY && ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <epoxy/wgl.h>
#endif

#include <boost/algorithm/string/predicate.hpp>

#include "core/Application.h"
#include "core/Config.h"
#include "gui/Credits.h"
#include "graphics/opengl/GLDebug.h"
#include "graphics/opengl/GLTexture.h"
#include "graphics/opengl/GLTextureStage.h"
#include "graphics/opengl/GLVertexBuffer.h"
#include "io/log/Logger.h"
#include "platform/CrashHandler.h"
#include "window/RenderWindow.h"


OpenGLRenderer::OpenGLRenderer()
	: maxTextureStage(0)
	, m_maximumAnisotropy(1.f)
	, m_maximumSupportedAnisotropy(1.f)
	, m_glcull(GL_NONE)
	, m_scissor(Rect::ZERO)
	, m_MSAALevel(0)
	, m_hasMSAA(false)
	, m_hasTextureNPOT(false)
	, m_hasSizedTextureFormats(false)
	, m_hasIntensityTextures(false)
	, m_hasBGRTextureTransfer(false)
	, m_hasMapBuffer(false)
	, m_hasMapBufferRange(false)
	, m_hasBufferStorage(false)
	, m_hasBufferUsageStream(false)
	, m_hasDrawRangeElements(false)
	, m_hasDrawElementsBaseVertex(false)
	, m_hasClearDepthf(false)
	, m_hasVertexFogCoordinate(false)
	, m_hasSampleShading(false)
{ }

OpenGLRenderer::~OpenGLRenderer() {
	
	if(isInitialized()) {
		shutdown();
	}
	
	// TODO textures must be destructed before OpenGLRenderer or not at all
	
}

enum GLTransformMode {
	GL_UnsetTransform,
	GL_NoTransform,
	GL_ModelViewProjectionTransform
};

static GLTransformMode currentTransform;

void OpenGLRenderer::initialize() {
	
	#if ARX_HAVE_EPOXY
	
	#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	epoxy_handle_external_wglMakeCurrent();
	#endif
	
	#elif ARX_HAVE_GLEW
	
	if(glewInit() != GLEW_OK) {
		LogError << "GLEW init failed";
		return;
	}
	
	const GLubyte * glewVersion = glewGetString(GLEW_VERSION);
	LogInfo << "Using GLEW " << glewVersion;
	CrashHandler::setVariable("GLEW version", glewVersion);
	
	#endif
	
	const char * glVersion = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	if(boost::starts_with(glVersion, "OpenGL ES-CL ")) {
		LogError << "OpenGL ES common lite profile detected but arx requires floating point functionality";
	}
	const char * prefix = "OpenGL ";
	if(boost::starts_with(glVersion, prefix)) {
		glVersion += std::strlen(prefix);
	}
	LogInfo << "Using OpenGL " << glVersion;
	CrashHandler::setVariable("OpenGL version", glVersion);
	
	const GLubyte * glVendor = glGetString(GL_VENDOR);
	LogInfo << " ├─ Vendor: " << glVendor;
	CrashHandler::setVariable("OpenGL vendor", glVendor);
	
	const GLubyte * glRenderer = glGetString(GL_RENDERER);
	LogInfo << " ├─ Device: " << glRenderer;
	CrashHandler::setVariable("OpenGL device", glRenderer);
	
	#if defined(GL_CONTEXT_FLAG_DEBUG_BIT) || defined(GL_CONTEXT_FLAG_NO_ERROR_BIT)
	if(ARX_HAVE_GL_VER(3, 0)) {
		GLint flags = 0;
		glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		#ifdef GL_CONTEXT_FLAG_DEBUG_BIT
		if(flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
			LogInfo << " ├─ Context type: debug";
		}
		#endif
		#ifdef GL_CONTEXT_FLAG_NO_ERROR_BIT
		if(flags & GL_CONTEXT_FLAG_NO_ERROR_BIT) {
			LogInfo << " ├─ Context type: no error";
		}
		#endif
	}
	#endif
	
	u64 totalVRAM = 0, freeVRAM = 0;
	{
		#ifdef GL_NVX_gpu_memory_info
		if(ARX_HAVE_GL_EXT(NVX_gpu_memory_info)) {
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
		if(ARX_HAVE_GL_EXT(ATI_meminfo)) {
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
		#if ARX_HAVE_EPOXY
		oss << "libepoxy\n";
		#elif ARX_HAVE_GLEW
		oss << "GLEW " << glewVersion << '\n';
		#endif
		const char * start = glVersion;
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
	
	#if ARX_HAVE_EPOXY
	const bool isES = !epoxy_is_desktop_gl();
	#else
	const bool isES = false;
	#endif
	
	if(isES) {
		if(!ARX_HAVE_GLES_VER(1, 0)) {
			LogError << "OpenGL ES version 1.0 or newer required";
		}
	} else {
		#if ARX_HAVE_EPOXY
		if(!ARX_HAVE_GL_VER(1, 5) && (!ARX_HAVE_GL_VER(1, 4) || !ARX_HAVE_GL_EXT(ARB_vertex_buffer_object))) {
			LogError << "OpenGL version 1.5 or newer or 1.4 + GL_ARB_vertex_buffer_object required";
		}
		#else
		if(!ARX_HAVE_GL_VER(1, 5)) {
			LogError << "OpenGL version 1.5 or newer required";
		}
		#endif
	}
	
	if(isES) {
		m_hasTextureNPOT = ARX_HAVE_GLES_VER(2, 0) || ARX_HAVE_GLES_EXT(OES_texture_npot);
		if(!m_hasTextureNPOT) {
			LogWarning << "Missing OpenGL extension OES_texture_npot.";
		}
		m_hasSizedTextureFormats = ARX_HAVE_GLES_VER(3, 0) || ARX_HAVE_GLES_EXT(OES_required_internalformat);
		m_hasIntensityTextures = false;
		m_hasBGRTextureTransfer = false;
	} else {
		m_hasTextureNPOT = ARX_HAVE_GL_VER(2, 0) || ARX_HAVE_GL_EXT(ARB_texture_non_power_of_two);
		if(!m_hasTextureNPOT) {
			LogWarning << "Missing OpenGL extension ARB_texture_non_power_of_two.";
		} else if(!ARX_HAVE_GL_VER(3, 0)) {
			GLint max = 0;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
			if(max < 8192) {
				LogWarning << "Old hardware detected, ignoring OpenGL extension ARB_texture_non_power_of_two.";
				m_hasTextureNPOT = false;
			}
		}
		m_hasSizedTextureFormats = true;
		m_hasIntensityTextures = true;
		m_hasBGRTextureTransfer = true;
	}
	
	// EXT_texture_filter_anisotropic is available for both OpenGL ES and desktop OpenGL
	if(ARX_HAVE_GL_EXT(EXT_texture_filter_anisotropic) || ARX_HAVE_GL_EXT(ARB_texture_filter_anisotropic)) {
		GLfloat limit;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &limit);
		m_maximumSupportedAnisotropy = limit;
		setMaxAnisotropy(float(config.video.maxAnisotropicFiltering));
	} else {
		m_maximumSupportedAnisotropy = 1.f;
	}
	
	if(isES) {
		// OES_draw_elements_base_vertex requires OpenGL ES 2.0
		// EXT_draw_elements_base_vertex requires OpenGL ES 2.0
		m_hasDrawElementsBaseVertex = ARX_HAVE_GLES_VER(3, 2)
		                              || ARX_HAVE_GLES_EXT(OES_draw_elements_base_vertex)
		                              || ARX_HAVE_GLES_EXT(EXT_draw_elements_base_vertex);
		m_hasDrawRangeElements = ARX_HAVE_GLES_VER(3, 0);
	} else {
		m_hasDrawElementsBaseVertex = ARX_HAVE_GL_VER(3, 2) || ARX_HAVE_GL_EXT(ARB_draw_elements_base_vertex);
		if(!m_hasDrawElementsBaseVertex) {
			LogWarning << "Missing OpenGL extension ARB_draw_elements_base_vertex.";
		}
		m_hasDrawRangeElements = true; // Introduced in OpenGL 1.2
	}
	
	if(isES) {
		// EXT_map_buffer_range requires OpenGL ES 1.1
		m_hasMapBufferRange = ARX_HAVE_GLES_VER(3, 0) || ARX_HAVE_GLES_EXT(EXT_map_buffer_range);
		if(!m_hasMapBufferRange) {
			LogWarning << "Missing OpenGL extension EXT_map_buffer_range.";
		}
		// OES_mapbuffer requires OpenGL ES 1.1
		m_hasMapBuffer = ARX_HAVE_GLES_EXT(OES_mapbuffer);
		if(!m_hasMapBuffer) {
			LogWarning << "Missing OpenGL extension OES_mapbuffer.";
		}
	} else {
		// ARB_map_buffer_range requires OpenGL 2.1
		m_hasMapBufferRange = ARX_HAVE_GL_VER(3, 0) || ARX_HAVE_GL_EXT(ARB_map_buffer_range);
		if(!m_hasMapBufferRange) {
			LogWarning << "Missing OpenGL extension ARB_map_buffer_range.";
		}
		m_hasMapBuffer = true; // Introduced in OpenGL 1.5
	}
	
	if(isES) {
		// EXT_buffer_storage requires OpenGL ES 3.1
		m_hasBufferStorage = ARX_HAVE_GLES_EXT(EXT_buffer_storage);
		m_hasBufferUsageStream = ARX_HAVE_GLES_VER(2, 0);
	} else {
		m_hasBufferStorage = ARX_HAVE_GL_VER(4, 4) || ARX_HAVE_GL_EXT(ARB_buffer_storage);
		m_hasBufferUsageStream = true; // Introduced in OpenGL 1.5
	}
	
	if(isES) {
		m_hasClearDepthf = true;
	} else {
		m_hasClearDepthf = ARX_HAVE_GL_VER(4, 1) || ARX_HAVE_GL_EXT(ARB_ES2_compatibility)
		                   || ARX_HAVE_GL_EXT(OES_single_precision);
	}
	
	// Introduced in OpenGL 1.4, no extension available for OpenGL ES
	m_hasVertexFogCoordinate = !isES;
	
	if(isES) {
		m_hasSampleShading = ARX_HAVE_GLES_VER(3, 2) || ARX_HAVE_GLES_EXT(OES_sample_shading);
	} else {
		#if ARX_HAVE_GLEW
		// The extension and core version have different entry points
		m_hasSampleShading = ARX_HAVE_GL_EXT(ARB_sample_shading);
		#else
		m_hasSampleShading = ARX_HAVE_GL_VER(4, 0) || ARX_HAVE_GL_EXT(ARB_sample_shading);
		#endif
	}
	
	// Synchronize GL state cache
	
	m_MSAALevel = 0;
	{
		GLint buffers = 0;
		glGetIntegerv(GL_SAMPLE_BUFFERS, &buffers);
		if(buffers) {
			GLint samples = 0;
			glGetIntegerv(GL_SAMPLES, &samples);
			m_MSAALevel = samples;
		}
	}
	if(m_MSAALevel > 0) {
		glDisable(GL_MULTISAMPLE);
	}
	m_hasMSAA = false;
	
	m_glcull = GL_BACK;
	m_glstate.setCull(CullNone);
	
	if(isES) {
		#if ARX_HAVE_EPOXY
		glFogx(GL_FOG_MODE, GL_LINEAR);
		#endif
	} else {
		glFogi(GL_FOG_MODE, GL_LINEAR);
		if(ARX_HAVE_GL_EXT(NV_fog_distance)) {
			// TODO Support radial fogs once all vertices are provided in view-space coordinates
			glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_PLANE);
		}
	}
	m_glstate.setFog(false);
	
	glAlphaFunc(GL_GREATER, 0.5f);
	#ifdef GL_VERSION_4_0
	if(hasSampleShading()) {
		#if ARX_HAVE_GLEW
		glMinSampleShadingARB(1.f);
		#else
		glMinSampleShading(1.f);
		#endif
	}
	#endif
	m_glstate.setAlphaCutout(false);
	
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
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	currentTransform = GL_UnsetTransform;
	switchVertexArray(GL_NoArray, 0, 1);
	
	onRendererInit();
	
}

void OpenGLRenderer::shutdown() {
	
	arx_assert(isInitialized());
	
	onRendererShutdown();
	
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		delete m_TextureStages[i];
	}
	m_TextureStages.clear();
	
	m_maximumAnisotropy = 1.f;
	m_maximumSupportedAnisotropy = 1.f;
	
}

static glm::mat4x4 projection(1.f);
static glm::mat4x4 view(1.f);

void OpenGLRenderer::enableTransform() {
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		return;
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(view));
		
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(projection));
	
	if(hasVertexFogCoordinate()) {
		glFogi(GL_FOG_COORDINATE_SOURCE, GL_FRAGMENT_DEPTH);
	}
	
	currentTransform = GL_ModelViewProjectionTransform;
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
	glScalef(2.f / viewport.width(), -2.f / viewport.height(), 1.f);
	
	// Change pixel origins
	glTranslatef(0.5f, 0.5f, 0.f);
	
	if(hasVertexFogCoordinate()) {
		glFogi(GL_FOG_COORDINATE_SOURCE, GL_FOG_COORDINATE);
	}
	
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

void OpenGLRenderer::SetProjectionMatrix(const glm::mat4x4 & matProj) {
	
	if(!memcmp(&projection, &matProj, sizeof(glm::mat4x4))) {
		return;
	}
	
	if(currentTransform == GL_ModelViewProjectionTransform) {
		currentTransform = GL_UnsetTransform;
	}
	
	projection = matProj;
}

void OpenGLRenderer::ReleaseAllTextures() {
	for(TextureList::iterator it = textures.begin(); it != textures.end(); ++it) {
		it->destroy();
	}
}

void OpenGLRenderer::RestoreAllTextures() {
	for(TextureList::iterator it = textures.begin(); it != textures.end(); ++it) {
		it->restore();
	}
}

void OpenGLRenderer::reloadColorKeyTextures() {
	for(TextureList::iterator it = textures.begin(); it != textures.end(); ++it) {
		if(it->hasColorKey()) {
			it->restore();
		}
	}
}

Texture * OpenGLRenderer::createTexture() {
	GLTexture * texture = new GLTexture(this);
	textures.push_back(*texture);
	return texture;
}

void OpenGLRenderer::SetViewport(const Rect & _viewport) {
	
	if(_viewport == viewport) {
		return;
	}
	
	viewport = _viewport;
	
	// TODO maybe it's better to always have the viewport cover the whole window and use glScissor instead?
	
	int height = mainApp->getWindow()->getSize().y;
	
	glViewport(viewport.left, height - viewport.bottom, viewport.width(), viewport.height());
	
	if(currentTransform == GL_NoTransform) {
		currentTransform = GL_UnsetTransform;
	}
}

void OpenGLRenderer::SetScissor(const Rect & rect) {
	
	if(m_scissor == rect) {
		return;
	}
	
	if(rect.isValid()) {
		if(!m_scissor.isValid()) {
			glEnable(GL_SCISSOR_TEST);
		}
		int height = mainApp->getWindow()->getSize().y;
		glScissor(rect.left, height - rect.bottom, rect.width(), rect.height());
	} else {
		if(m_scissor.isValid()) {
			glDisable(GL_SCISSOR_TEST);
		}
	}
	
	m_scissor = rect;
	
}

void OpenGLRenderer::Clear(BufferFlags bufferFlags, Color clearColor, float clearDepth, size_t nrects, Rect * rect) {
	
	GLbitfield buffers = 0;
	
	if(bufferFlags & ColorBuffer) {
		Color4f colorf(clearColor);
		glClearColor(colorf.r, colorf.g, colorf.b, colorf.a);
		buffers |= GL_COLOR_BUFFER_BIT;
	}
	
	if(bufferFlags & DepthBuffer) {
		if(!m_glstate.getDepthWrite()) {
			// glClear() respects the depth mask
			glDepthMask(GL_TRUE);
			m_glstate.setDepthWrite(true);
		}
		#ifdef GL_VERSION_4_1
		if(hasClearDepthf()) {
			glClearDepthf(clearDepth);
		}
		else
		#endif
		{
			// Not available in OpenGL ES
			glClearDepth(GLclampd(clearDepth));
		}
		buffers |= GL_DEPTH_BUFFER_BIT;
	}
	
	if(nrects) {
		
		Rect scissor = m_scissor;
		
		for(size_t i = 0; i < nrects; i++) {
			
			SetScissor(rect[i]);
			
			glClear(buffers);
			
		}
		
		SetScissor(scissor);
		
	} else {
		
		if(m_scissor.isValid()) {
			glDisable(GL_SCISSOR_TEST);
		}
		
		glClear(buffers);
		
		if(m_scissor.isValid()) {
			glEnable(GL_SCISSOR_TEST);
		}
		
	}
}

void OpenGLRenderer::SetFogColor(Color color) {
	Color4f colorf(color);
	GLfloat fogColor[4] = { colorf.r, colorf.g, colorf.b, colorf.a };
	glFogfv(GL_FOG_COLOR, fogColor);
}

void OpenGLRenderer::SetFogParams(float fogStart, float fogEnd) {
	glFogf(GL_FOG_START, fogStart);
	glFogf(GL_FOG_END, fogEnd);
}

void OpenGLRenderer::SetAntialiasing(bool enable) {
	
	if(m_MSAALevel <= 0) {
		return;
	}
	
	if(enable && !config.video.antialiasing) {
		return;
	}
	
	if(enable == m_hasMSAA) {
		return;
	}
	
	// The state used for alpha cutouts can differ between msaa and non-msaa.
	// Clear the old flushed state.
	if(m_glstate.getAlphaCutout()) {
		bool alphaCutout = m_state.getAlphaCutout();
		m_state.setAlphaCutout(false);
		flushState();
		m_state.setAlphaCutout(alphaCutout);
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

Renderer::AlphaCutoutAntialising OpenGLRenderer::getMaxSupportedAlphaCutoutAntialiasing() const {
	
	#ifdef GL_VERSION_4_0
	if(hasSampleShading()) {
		return CrispAlphaCutoutAA;
	}
	#endif
	
	return FuzzyAlphaCutoutAA;
}

template <typename Vertex>
static VertexBuffer<Vertex> * createVertexBufferImpl(OpenGLRenderer * renderer,
                                                     size_t capacity,
                                                     Renderer::BufferUsage usage,
                                                     const std::string & setting) {
	
	bool matched = false;
	
	if(renderer->hasMapBufferRange()) {
		
		#ifdef GL_ARB_buffer_storage
		
		if(renderer->hasBufferStorage()) {
			
			if(setting.empty() || setting == "persistent-orphan") {
				if(usage != Renderer::Static) {
					return new GLPersistentOrphanVertexBuffer<Vertex>(renderer, capacity, usage);
				}
				matched = true;
			}
			if(setting.empty() || setting == "persistent-x3") {
				if(usage == Renderer::Stream) {
					return new GLPersistentFenceVertexBuffer<Vertex, 3>(renderer, capacity, usage, 3);
				}
				matched = true;
			}
			if(setting.empty() || setting == "persistent-x2") {
				if(usage == Renderer::Stream) {
					return new GLPersistentFenceVertexBuffer<Vertex, 3>(renderer, capacity, usage, 2);
				}
				matched = true;
			}
			if(setting == "persistent-nosync") {
				if(usage != Renderer::Static) {
					return new GLPersistentUnsynchronizedVertexBuffer<Vertex>(renderer, capacity, usage);
				}
				matched = true;
			}
			
		}
		
		#endif // GL_ARB_buffer_storage
		
		if(setting.empty() || setting == "maprange" || setting == "maprange+subdata") {
			return new GLMapRangeVertexBuffer<Vertex>(renderer, capacity, usage);
		}
		
	}
	
	if(renderer->hasMapBuffer()) {
		
		if(setting.empty() || setting == "map" || setting == "map+subdata") {
			return new GLMapVertexBuffer<Vertex>(renderer, capacity, usage);
		}
		
	}
	
	if(setting.empty() || setting == "shadow" || setting == "shadow+subdata") {
		return new GLShadowVertexBuffer<Vertex>(renderer, capacity, usage);
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
	return createVertexBufferImpl<TexturedVertex>(this, capacity, usage);
}

VertexBuffer<SMY_VERTEX> * OpenGLRenderer::createVertexBuffer(size_t capacity, BufferUsage usage) {
	return createVertexBufferImpl<SMY_VERTEX>(this, capacity, usage);
}

VertexBuffer<SMY_VERTEX3> * OpenGLRenderer::createVertexBuffer3(size_t capacity, BufferUsage usage) {
	return createVertexBufferImpl<SMY_VERTEX3>(this, capacity, usage);
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
	
	bindBuffer(GL_NONE);
	
	setVertexArray(this, vertices, vertices);
	
	if(hasDrawRangeElements()) {
		glDrawRangeElements(arxToGlPrimitiveType[primitive], 0, nvertices - 1, nindices, GL_UNSIGNED_SHORT, indices);
	} else {
		glDrawElements(arxToGlPrimitiveType[primitive], nindices, GL_UNSIGNED_SHORT, indices);
	}
	
}

bool OpenGLRenderer::getSnapshot(Image & image) {
	
	Vec2i size = mainApp->getWindow()->getSize();
	
	image.create(size_t(size.x), size_t(size.y), Image::Format_R8G8B8);
	
	glReadPixels(0, 0, size.x, size.y, GL_RGB, GL_UNSIGNED_BYTE, image.getData());
	
	image.flipY();
	
	return true;
}

bool OpenGLRenderer::getSnapshot(Image & image, size_t width, size_t height) {
	
	// TODO handle scaling on the GPU so we don't need to download the whole image
	
	Image fullsize;
	getSnapshot(fullsize);
	
	image.resizeFrom(fullsize, width, height);
	
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
		
		bool useA2C = m_hasMSAA && config.video.alphaCutoutAntialiasing == int(FuzzyAlphaCutoutAA);
		if(m_glstate.getAlphaCutout() != m_state.getAlphaCutout()
		   || (useA2C && m_state.getAlphaCutout() && m_glstate.isBlendEnabled() != m_state.isBlendEnabled())) {
			
			/* When rendering alpha cutouts with alpha blending enabled we still
			 * need to 'discard' transparent texels, as blending might not use the src alpha!
			 * On the other hand, we can't use GL_SAMPLE_ALPHA_TO_COVERAGE when blending
			 * as that could result in the src alpha being applied twice (e.g. for text).
			 * So we must toggle between alpha to coverage and alpha test when toggling blending.
			 */
			bool disableA2C = useA2C && !m_glstate.isBlendEnabled()
			                  && (!m_state.getAlphaCutout() || m_state.isBlendEnabled());
			bool enableA2C = useA2C && !m_state.isBlendEnabled()
			                 && (!m_glstate.getAlphaCutout() || m_glstate.isBlendEnabled());
			if(m_glstate.getAlphaCutout()) {
				if(disableA2C) {
					glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				} else if(!m_state.getAlphaCutout() || enableA2C) {
					#ifdef GL_VERSION_4_0
					if(hasSampleShading() && m_hasMSAA
					   && config.video.alphaCutoutAntialiasing == int(CrispAlphaCutoutAA)) {
						glDisable(GL_SAMPLE_SHADING);
					}
					#endif
					glDisable(GL_ALPHA_TEST);
				}
			}
			if(m_state.getAlphaCutout()) {
				if(enableA2C) {
					glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				} else if(!m_glstate.getAlphaCutout() || disableA2C) {
					glEnable(GL_ALPHA_TEST);
					#ifdef GL_VERSION_4_0
					if(hasSampleShading() && m_hasMSAA
					   && config.video.alphaCutoutAntialiasing == int(CrispAlphaCutoutAA)) {
						glEnable(GL_SAMPLE_SHADING);
					}
					#endif
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
