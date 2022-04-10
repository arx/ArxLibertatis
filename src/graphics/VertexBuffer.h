/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_VERTEXBUFFER_H
#define ARX_GRAPHICS_VERTEXBUFFER_H

#include <algorithm>
#include <memory>
#include <utility>

#include "graphics/Renderer.h"
#include "platform/Platform.h"
#include "util/Flags.h"

enum BufferFlag {
	DiscardBuffer = 1 << 0,
	DiscardRange  = 1 << 1,
	NoOverwrite   = 1 << 2
};
DECLARE_FLAGS(BufferFlag, BufferFlags)
DECLARE_FLAGS_OPERATORS(BufferFlags)

template <class Index>
class IndexBuffer {
	
public:
	
	IndexBuffer(const IndexBuffer &) = delete;
	IndexBuffer & operator=(const IndexBuffer &) = delete;
	
	size_t capacity() const { return m_capacity; }
	
	virtual void setData(const Index * vertices, size_t count, size_t offset = 0, BufferFlags flags = 0) = 0;
	
	virtual Index * lock(BufferFlags flags = 0, size_t offset = 0, size_t count = size_t(-1)) = 0;
	virtual void unlock() = 0;
	
	virtual ~IndexBuffer() = default;
	
protected:
	
	explicit IndexBuffer(size_t capacity) : m_capacity(capacity) { }
	
	const size_t m_capacity;
	
};

template <class Vertex>
class VertexBuffer {
	
public:
	
	VertexBuffer(const VertexBuffer &) = delete;
	VertexBuffer & operator=(const VertexBuffer &) = delete;
	
	size_t capacity() const { return m_capacity; }
	
	virtual void setData(const Vertex * vertices, size_t count, size_t offset = 0, BufferFlags flags = 0) = 0;
	
	virtual Vertex * lock(BufferFlags flags = 0, size_t offset = 0, size_t count = size_t(-1)) = 0;
	virtual void unlock() = 0;
	
	virtual void draw(Renderer::Primitive primitive, size_t count, size_t offset = 0) const = 0;
	virtual void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, const unsigned short * indices, size_t nbindices) const = 0;
	
	virtual ~VertexBuffer() = default;
	
protected:
	
	explicit VertexBuffer(size_t capacity) : m_capacity(capacity) { }
	
	const size_t m_capacity;
	
};

template <class Vertex>
class CircularVertexBuffer {
	
public:
	
	CircularVertexBuffer(const CircularVertexBuffer &) = delete;
	CircularVertexBuffer & operator=(const CircularVertexBuffer &) = delete;
	
	std::unique_ptr<VertexBuffer<Vertex>> const m_buffer;
	
	size_t pos;
	
	explicit CircularVertexBuffer(std::unique_ptr<VertexBuffer<Vertex>> buffer)
		: m_buffer(std::move(buffer))
		, pos(0)
	{ }
	
	void draw(Renderer::Primitive primitive, const Vertex * vertices, size_t count) {
		
		const Vertex * src = vertices;
		
		size_t num = std::min(count, m_buffer->capacity());
		
		// Make sure num is a multiple of the primitive's stride.
		if(primitive == Renderer::TriangleList) {
			arx_assert(count % 3 == 0);
			num -= num % 3;
		} else if(primitive == Renderer::LineList) {
			arx_assert((count & 1) == 0);
			num &= ~1;
		} else if(primitive == Renderer::TriangleStrip && num != count) {
			// Draw an even number of triangles so we don't flip front and back faces between draw calls.
			num &= 1;
		}
		
		size_t dst_offset = (pos + num > m_buffer->capacity()) ? 0 : pos;
		
		m_buffer->setData(src, num, dst_offset, dst_offset ? NoOverwrite : DiscardBuffer);
		m_buffer->draw(primitive, num, dst_offset);
		
		pos = dst_offset + num;
		src += num, count -= num;
		
		if(!count) {
			return;
		}
		
		switch(primitive) {
			
			case Renderer::TriangleList: {
				do {
					num = std::min(count, m_buffer->capacity());
					num -= num % 3;
					m_buffer->setData(src, num, 0, DiscardBuffer);
					m_buffer->draw(primitive, num);
					src += num, count -= num, pos = num;
				} while(count);
				break;
			}
			
			case Renderer::TriangleStrip: {
				do {
					count += 2, src -= 2;
					num = std::min(count, m_buffer->capacity());
					if(num != count) {
						// Draw an even number of triangles so we don't flip front and back faces between draw calls.
						num -= num & 1;
					}
					m_buffer->setData(src, num, 0, DiscardBuffer);
					m_buffer->draw(primitive, num);
					src += num, count -= num, pos = num;
				} while(count);
				break;
			}
			
			case Renderer::TriangleFan: {
				do {
					count += 1, src -= 1;
					num = std::min(count, m_buffer->capacity() - 1);
					m_buffer->setData(vertices, 1, 0, DiscardBuffer);
					m_buffer->setData(src, num, 1, NoOverwrite);
					m_buffer->draw(primitive, num + 1);
					src += num, count -= num, pos = num + 1;
				} while(count);
				break;
			}
			
			case Renderer::LineList: {
				do {
					num = std::min(count, m_buffer->capacity()) & ~1;
					m_buffer->setData(src, num, 0, DiscardBuffer);
					m_buffer->draw(primitive, num);
					src += num, count -= num, pos = num;
				} while(count);
				break;
			}
			
			case Renderer::LineStrip: {
				do {
					count += 1, src -= 1;
					num = std::min(count, m_buffer->capacity());
					m_buffer->setData(src, num, 0, DiscardBuffer);
					m_buffer->draw(primitive, num);
					src += num, count -= num, pos = num;
				} while(count);
				break;
			}
			
			default: {
				arx_assert_msg(false, "too large vertex array (%lu) for primitive %d",
				               static_cast<unsigned long>(count + num), primitive);
			}
			
		}
		
	}
	
};

#endif // ARX_GRAPHICS_VERTEXBUFFER_H
