
#ifndef ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
#define ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H

#include "graphics/VertexBuffer.h"
#include "graphics/Vertex.h"
#include "graphics/opengl/OpenGLUtil.h"
#include "graphics/opengl/OpenGLRenderer.h"

template <class Vertex>
static void setVertexArray(const Vertex * vertex);

enum GLArrayClientState {
	GL_NoArray,
	GL_TexturedVertex,
	GL_SMY_VERTEX,
	GL_SMY_VERTEX3
};

static GLArrayClientState glArrayClientState = GL_NoArray;

static bool switchVertexArray(const void * vertices, GLArrayClientState type) {
	
	if(!vertices) {
		if(glArrayClientState == type) {
			return false;
		}
		glArrayClientState = type;
	} else {
		glArrayClientState = GL_NoArray; // always re-set the array if we don't use a vertex buffer
	}
	
	return true;
}

template <>
void setVertexArray(const TexturedVertex * vertices) {
	
	if(!switchVertexArray(vertices, GL_TexturedVertex)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
	
	// ignore the rhw parameter!
	glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), &vertices->sx);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), &vertices->color);
	glSecondaryColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), &vertices->specular);
	
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), &vertices->tu);
	
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glClientActiveTexture(GL_TEXTURE2);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	CHECK_GL;
}

template <>
void setVertexArray(const SMY_VERTEX * vertices) {
	
	if(!switchVertexArray(vertices, GL_SMY_VERTEX)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX), &vertices->x);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX), &vertices->color);
	
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX), &vertices->tu);
	
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glClientActiveTexture(GL_TEXTURE2);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	CHECK_GL;
}

template <>
void setVertexArray(const SMY_VERTEX3 * vertices) {
	
	if(!switchVertexArray(vertices, GL_SMY_VERTEX3)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->x);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX3), &vertices->color);
	
	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->tu);
	
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->tu2);
	
	glClientActiveTexture(GL_TEXTURE2);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->tu3);
	
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
	
	GLVertexBuffer(OpenGLRenderer * _renderer, size_t capacity, Renderer::BufferUsage usage) : VertexBuffer<Vertex>(capacity), renderer(_renderer), buffer(0) {
		
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
		
		glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), count * sizeof(Vertex), vertices);
		
		CHECK_GL;
	}
	
	Vertex * lock(BufferFlags flags) {
		ARX_UNUSED(flags);
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		void * buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY); 
		
		arx_assert(buf != NULL); // TODO OpenGL doesn't guarantee this
		
		CHECK_GL;
		
		return reinterpret_cast<Vertex *>(buf);
	}
	
	void unlock() {
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		glUnmapBuffer(GL_ARRAY_BUFFER); // TODO handle GL_FALSE return (buffer invalidated)
		
		CHECK_GL;
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		
		arx_assert(offset + count <= capacity());
		
		renderer->beforeDraw<Vertex>();
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		setVertexArray<Vertex>(NULL);
		
		glDrawArrays(arxToGlPrimitiveType[primitive], offset, count);
		
		CHECK_GL;
	}
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= capacity());
		arx_assert(indices != NULL);
		
		renderer->beforeDraw<Vertex>();
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		setVertexArray<Vertex>(NULL);
		
		glDrawRangeElements(arxToGlPrimitiveType[primitive], offset, offset + count - 1, nbindices, GL_UNSIGNED_SHORT, indices);
		
		CHECK_GL;
	}
	
	~GLVertexBuffer() {
		glDeleteBuffers(1, &buffer);
		CHECK_GL;
	};
	
private:
	
	OpenGLRenderer * renderer;
	GLuint buffer;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
