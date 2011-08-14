
#ifndef ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
#define ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H

#include "graphics/VertexBuffer.h"
#include "graphics/Vertex.h"
#include "graphics/opengl/OpenGLUtil.h"

template <class Vertex>
void setVertexArray(const Vertex * vertex);

template <>
void setVertexArray(const TexturedVertex * vertices) {
	
	glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), &vertices->sx);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), &vertices->color);
	glSecondaryColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), &vertices->specular);
	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), &vertices->tu);
}

template <>
void setVertexArray(const SMY_VERTEX * vertices) {
	
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX), &vertices->x);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX), &vertices->color);
	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX), &vertices->tu);
}

template <>
void setVertexArray(const SMY_VERTEX3 * vertices) {
	
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->x);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX3), &vertices->color);
	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->tu);
	glClientActiveTexture(GL_TEXTURE1);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->tu2);
	glClientActiveTexture(GL_TEXTURE2);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->tu3);
}

GLenum arxToGlBufferUsage[] = {
	GL_STATIC_DRAW,  // Static,
	GL_DYNAMIC_DRAW, // Dynamic,
	GL_STREAM_DRAW   // Stream
};

extern GLenum arxToGlPrimitiveType[];

template <class Vertex>
class GLVertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	GLVertexBuffer(size_t capacity, Renderer::BufferUsage usage) : VertexBuffer<Vertex>(capacity), buffer(0) {
		
		glGenBuffers(1, &buffer);
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(Vertex), NULL, arxToGlBufferUsage[usage]);
		
		CHECK_GL;
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		ARX_UNUSED(flags);
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), count * sizeof(Vertex), vertices);
		
		CHECK_GL;
	}
	
	Vertex * lock(BufferFlags flags) {
		ARX_UNUSED(flags);
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		void * buf = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE); // TODO use flags
		
		CHECK_GL;
		
		return reinterpret_cast<Vertex *>(buf);
	}
	
	void unlock() {
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		glUnmapBuffer(GL_ARRAY_BUFFER);
		
		CHECK_GL;
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		
		setVertexArray<Vertex>(NULL);
		
		glDrawArrays(arxToGlPrimitiveType[primitive], offset, count);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		
		CHECK_GL;
	}
	
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		arx_assert(indices != NULL);
		
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		
		setVertexArray<Vertex>(NULL);
		
		glDrawRangeElements(arxToGlPrimitiveType[primitive], offset, count, nbindices, GL_UNSIGNED_SHORT, indices);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		
		CHECK_GL;
	}
	
	~GLVertexBuffer() {
		glDeleteBuffers(1, &buffer);
		CHECK_GL;
	};
	
private:
	
	GLuint buffer;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
