
#ifndef ARX_GRAPHICS_OPENGL_GLNOVERTEXBUFFER_H
#define ARX_GRAPHICS_OPENGL_GLNOVERTEXBUFFER_H

#include "graphics/VertexBuffer.h"
#include "graphics/Vertex.h"
#include "graphics/opengl/OpenGLUtil.h"
#include "graphics/opengl/OpenGLRenderer.h"

template <class Vertex>
void renderVertex(const Vertex & vertex);

template <>
void renderVertex(const TexturedVertex & vertex) {
	
	Color c = Color::fromBGRA(vertex.color);
	glColor4ub(c.r, c.g, c.b, c.a);
	
	Color s = Color::fromBGRA(vertex.specular);
	glSecondaryColor3ub(s.r, s.g, s.b);
	
	glMultiTexCoord2f(GL_TEXTURE0, vertex.tu, vertex.tv);
	
	GLfloat w = 1.0f / vertex.rhw; 
	glVertex4f(vertex.sx * w, vertex.sy * w, vertex.sz * w, w);
}

template <>
void renderVertex(const SMY_VERTEX & vertex) {
	
	Color c = Color::fromBGRA(vertex.color);
	glColor4ub(c.r, c.g, c.b, c.a);
	
	glSecondaryColor3ub(0, 0, 0);
	
	glMultiTexCoord2f(GL_TEXTURE0, vertex.tu, vertex.tv);
	
	glVertex3f(vertex.x, vertex.y, vertex.z);
}

template <>
void renderVertex(const SMY_VERTEX3 & vertex) {
	
	Color c = Color::fromBGRA(vertex.color);
	glColor4ub(c.r, c.g, c.b, c.a);
	
	glSecondaryColor3ub(0, 0, 0);
	
	glMultiTexCoord2f(GL_TEXTURE0, vertex.tu, vertex.tv);
	
	glMultiTexCoord2f(GL_TEXTURE1, vertex.tu2, vertex.tv2);
	
	glMultiTexCoord2f(GL_TEXTURE2, vertex.tu3, vertex.tv3);
	
	glVertex3f(vertex.x, vertex.y, vertex.z);
}

extern const GLenum arxToGlPrimitiveType[];

template <class Vertex>
class GLNoVertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	using VertexBuffer<Vertex>::capacity;
	
	GLNoVertexBuffer(OpenGLRenderer * _renderer, size_t capacity) : VertexBuffer<Vertex>(capacity), renderer(_renderer), buffer(new Vertex[capacity]) { }
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		ARX_UNUSED(flags);
		
		arx_assert(offset + count <= capacity());
		
		std::copy(vertices, vertices + count, buffer + offset);
	}
	
	Vertex * lock(BufferFlags flags) {
		ARX_UNUSED(flags);
		return buffer;
	}
	
	void unlock() {
		// nothing to do
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		
		arx_assert(offset + count <= capacity());
		
		renderer->beforeDraw<Vertex>();
		
		glBegin(arxToGlPrimitiveType[primitive]);
		
		for(size_t i = 0; i < count; i++) {
			renderVertex(buffer[offset + i]);
		}
		
		glEnd();
		
		CHECK_GL;
	}
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		ARX_UNUSED(count), ARX_UNUSED(offset);
		
		arx_assert(offset + count <= capacity());
		arx_assert(indices != NULL);
		
		renderer->beforeDraw<Vertex>();

		Vertex * pBuf = buffer + offset;
		
		glBegin(arxToGlPrimitiveType[primitive]);
		
		for(size_t i = 0; i < nbindices; i++) {
			renderVertex(pBuf[indices[i]]);
		}
		
		glEnd();
		
		CHECK_GL;
	}
	
	~GLNoVertexBuffer() {
		delete[] buffer;
	};
	
private:
	
	OpenGLRenderer * renderer;
	Vertex * buffer;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLNOVERTEXBUFFER_H
