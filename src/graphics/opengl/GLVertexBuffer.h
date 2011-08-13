
#ifndef ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
#define ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H

#include "graphics/VertexBuffer.h"

template <class Vertex>
class GLVertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	GLVertexBuffer(size_t capacity, Renderer::BufferUsage usage) : VertexBuffer<Vertex>(capacity), buffer(new Vertex[capacity]) {
		ARX_UNUSED(usage); // TODO implement
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		ARX_UNUSED(flags);
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		std::copy(vertices, vertices + count, buffer + offset); // TODO implement
	}
	
	Vertex * lock(BufferFlags flags) {
		ARX_UNUSED(flags);
		return buffer; // TODO implement
	}
	
	void unlock() {
		// TODO implement
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		ARX_UNUSED(primitive);
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		// TODO implement
	}
	
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		ARX_UNUSED(primitive), ARX_UNUSED(nbindices);
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		arx_assert(indices != NULL);
		
		// TODO implement
	}
	
	~GLVertexBuffer() {
		delete[] buffer; // TODO implement
	};
	
private:
	
	Vertex * buffer;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
