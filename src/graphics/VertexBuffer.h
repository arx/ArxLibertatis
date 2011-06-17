
#ifndef ARX_GRAPHICS_VERTEXBUFFER_H
#define ARX_GRAPHICS_VERTEXBUFFER_H

#include <algorithm>

#include "platform/Flags.h"
#include "graphics/Renderer.h"

enum BufferFlag {
	DiscardContents,
	NoOverwrite
};
DECLARE_FLAGS(BufferFlag, BufferFlags);
DECLARE_FLAGS_OPERATORS(BufferFlags);

template <class Vertex>
class VertexBuffer {
	
public:
	
	size_t capacity() const { return _capacity; }
	
	virtual void setData(const Vertex * vertices, size_t count, size_t offset = 0, BufferFlags flags = 0) = 0;
	
	virtual void draw(Renderer::Primitive primitive, size_t count, size_t offset = 0) const = 0;
	virtual void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const = 0;
	
	virtual ~VertexBuffer() { };
	
protected:
	
	VertexBuffer(size_t capacity) : _capacity(capacity) { }
	
	const size_t _capacity;
	
};

template <class Vertex>
class CircularVertexBuffer {
	
public:
	
	VertexBuffer<Vertex> * const vb;
	
	size_t pos;
	
	size_t nbindices;
	unsigned short * const indices; // TODO not all users of this class need this
	
	CircularVertexBuffer(VertexBuffer<Vertex> * _vb) : vb(_vb), pos(0), nbindices(0), indices(new unsigned short[4 * _vb->capacity()]) { }
	
	void draw(Renderer::Primitive primitive, const Vertex * vertices, size_t count) {
		
		nbindices = 0;
		
		while(count) {
			
			size_t num = std::min(count, vb->capacity());
			size_t offset = (pos + num > vb->capacity()) ? 0 : pos;
			
			vb->setData(vertices, num, offset, offset ? NoOverwrite : DiscardContents);
			
			vb->draw(primitive, num, offset);
			
			pos = offset + num;
			vertices += num, count -= num;
		}
		
	}
	
	~CircularVertexBuffer() {
		delete vb;
		delete[] indices;
	}
	
};

#endif // ARX_GRAPHICS_VERTEXBUFFER_H;
