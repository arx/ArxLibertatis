
#ifndef ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
#define ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H

#include "graphics/VertexBuffer.h"
#include "graphics/Vertex.h"
#include "graphics/Math.h"
#include "graphics/opengl/OpenGLRenderer.h"
#include "graphics/opengl/OpenGLUtil.h"

template <class Vertex>
static void setVertexArray(const Vertex * vertex, const void * ref);

// cached vertex array definitions
enum GLArrayClientState {
	GL_NoArray,
	GL_TexturedVertex,
	GL_SMY_VERTEX,
	GL_SMY_VERTEX3
};

static GLArrayClientState glArrayClientState = GL_NoArray;
static const void * glArrayClientStateRef = NULL;
static int glArrayClientStateTexCount = 0;

static void setVertexArrayTexCoord(int index, const float * coord, size_t stride) {
	
	glClientActiveTexture(GL_TEXTURE0 + index);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, stride, coord);
	
}

static bool switchVertexArray(GLArrayClientState type, const void * ref, int texcount) {
	
	if(glArrayClientState == type && glArrayClientStateRef == ref) {
		return false;
	}
	
	if(glArrayClientState != type) {
		if(glArrayClientState == GL_TexturedVertex) {
			glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
		}
		for(int i = texcount; i < glArrayClientStateTexCount; i++) {
			glClientActiveTexture(GL_TEXTURE0 + i);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glArrayClientStateTexCount = texcount;
	}
	
	glArrayClientState = type;
	glArrayClientStateRef = ref;
	
	return true;
}

template <>
void setVertexArray(const TexturedVertex * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_TexturedVertex, ref, 1)) {
		return;
	}
	
	// TODO ignoring the rhw parameter!
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), &vertices->sx);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), &vertices->color);
	
	// TODO(broken-GLEW) work around a bug in older GLEW versions (fix is in 1.6.0)
	GLvoid * ptr = const_cast<ColorBGRA *>(&vertices->specular);
	glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
	glSecondaryColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), ptr);
	
	setVertexArrayTexCoord(0, &vertices->tu, sizeof(TexturedVertex));
	
	CHECK_GL;
}

template <>
void setVertexArray(const SMY_VERTEX * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_SMY_VERTEX, ref, 1)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX), &vertices->x);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX), &vertices->color);
	
	setVertexArrayTexCoord(0, &vertices->tu, sizeof(SMY_VERTEX));
	
	CHECK_GL;
}

template <>
void setVertexArray(const SMY_VERTEX3 * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_SMY_VERTEX3, ref, 3)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->x);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX3), &vertices->color);
	
	setVertexArrayTexCoord(0, &vertices->tu, sizeof(SMY_VERTEX3));
	setVertexArrayTexCoord(1, &vertices->tu2, sizeof(SMY_VERTEX3));
	setVertexArrayTexCoord(2, &vertices->tu3, sizeof(SMY_VERTEX3));
	
	CHECK_GL;
}

static const GLenum arxToGlBufferUsage[] = {
	GL_STATIC_DRAW,  // Static,
	GL_DYNAMIC_DRAW, // Dynamic,
	GL_STREAM_DRAW   // Stream
};

extern const GLenum arxToGlPrimitiveType[];

template <class Vertex>
class GLVertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	using VertexBuffer<Vertex>::capacity;
	
	GLVertexBuffer(OpenGLRenderer * _renderer, size_t capacity, Renderer::BufferUsage _usage) : VertexBuffer<Vertex>(capacity), renderer(_renderer), buffer(0), usage(_usage) {
		
		glGenBuffers(1, &buffer);
		
		arx_assert(buffer != GL_NONE);
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(Vertex), NULL, arxToGlBufferUsage[usage]);
		
		CHECK_GL;
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		ARX_UNUSED(flags);
		
		arx_assert(offset + count <= capacity());
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		if(flags & DiscardContents) {
			// avoid waiting if GL is still using the old buffer contents
			glBufferData(GL_ARRAY_BUFFER, capacity() * sizeof(Vertex), NULL, arxToGlBufferUsage[usage]);
		}
		
		glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), count * sizeof(Vertex), vertices);
		
		CHECK_GL;
	}
	
	Vertex * lock(BufferFlags flags) {
		ARX_UNUSED(flags);
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		if(flags & DiscardContents) {
			// avoid waiting if GL is still using the old buffer contents
			glBufferData(GL_ARRAY_BUFFER, capacity() * sizeof(Vertex), NULL, arxToGlBufferUsage[usage]);
		}
		
		void * buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
		
		arx_assert(buf != NULL); // TODO OpenGL doesn't guarantee this
		
		CHECK_GL;
		
		return reinterpret_cast<Vertex *>(buf);
	}
	
	void unlock() {
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		GLboolean ret = glUnmapBuffer(GL_ARRAY_BUFFER);
		
		if(ret == GL_FALSE) {
			// TODO handle GL_FALSE return (buffer invalidated)
			LogWarning << "vertex buffer invalidated";
		}
		
		CHECK_GL;
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		
		arx_assert(offset + count <= capacity());
		
		renderer->beforeDraw<Vertex>();
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		setVertexArray<Vertex>(NULL, this);
		
		glDrawArrays(arxToGlPrimitiveType[primitive], offset, count);
		
		CHECK_GL;
	}
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		// needs GL_ARB_draw_elements_base_vertex!
		
		arx_assert(offset + count <= capacity());
		arx_assert(indices != NULL);
		
		renderer->beforeDraw<Vertex>();
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		setVertexArray<Vertex>(NULL, this);
		
		glDrawRangeElementsBaseVertex(arxToGlPrimitiveType[primitive], 0, count - 1, nbindices, GL_UNSIGNED_SHORT, indices, offset);
		
		CHECK_GL;
	}
	
	~GLVertexBuffer() {
		glDeleteBuffers(1, &buffer);
		CHECK_GL;
	};
	
private:
	
	OpenGLRenderer * renderer;
	GLuint buffer;
	Renderer::BufferUsage usage;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
