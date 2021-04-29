/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
#define ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H

#include <cstring>

#include "graphics/VertexBuffer.h"
#include "graphics/Vertex.h"
#include "graphics/Math.h"
#include "graphics/opengl/OpenGLRenderer.h"

#include "io/log/Logger.h"

template <class Vertex>
static void setVertexArray(const Vertex * vertex, const void * ref);

// cached vertex array definitions
enum GLArrayClientState {
	GL_NoArray,
	GL_TexturedVertex,
	GL_SMY_VERTEX,
	GL_SMY_VERTEX3
};

void bindBuffer(GLuint buffer);
void unbindBuffer(GLuint buffer);
void setVertexArrayTexCoord(int index, const void * coord, size_t stride);
bool switchVertexArray(GLArrayClientState type, const void * ref, int texcount);
void clearVertexArray(const void * ref);

template <>
inline void setVertexArray(const TexturedVertex * vertices, const void * ref) {
	
	ARX_UNUSED(vertices);

	if(!switchVertexArray(GL_TexturedVertex, ref, 1)) {
		return;
	}
	
	// rhw -> w conversion is done in a vertex shader
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, sizeof(TexturedVertex), &vertices->p);

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), &vertices->color);

	setVertexArrayTexCoord(0, &vertices->uv, sizeof(TexturedVertex));
}

template <>
inline void setVertexArray(const SMY_VERTEX * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_SMY_VERTEX, ref, 1)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX), &vertices->p.x);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX), &vertices->color);
	
	setVertexArrayTexCoord(0, &vertices->uv, sizeof(SMY_VERTEX));
}

template <>
inline void setVertexArray(const SMY_VERTEX3 * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_SMY_VERTEX3, ref, 3)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->p.x);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX3), &vertices->color);
	
	setVertexArrayTexCoord(0, &vertices->uv[0], sizeof(SMY_VERTEX3));
	setVertexArrayTexCoord(1, &vertices->uv[1], sizeof(SMY_VERTEX3));
	setVertexArrayTexCoord(2, &vertices->uv[2], sizeof(SMY_VERTEX3));
}

extern const GLenum arxToGlPrimitiveType[];

extern std::vector<GLushort> glShortIndexBuffer;
extern std::vector<GLuint> glIntIndexBuffer;

template <class Vertex>
class BaseGLVertexBuffer : public VertexBuffer<Vertex> {
	
	typedef VertexBuffer<Vertex> Base;
	
public:
	
	using Base::capacity;
	
	BaseGLVertexBuffer(OpenGLRenderer * renderer, size_t capacity)
		: Base(capacity)
		, m_renderer(renderer)
		, m_buffer(GL_NONE)
		, m_offset(0)
	{
		glGenBuffers(1, &m_buffer);
		arx_assert(m_buffer != GL_NONE);
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		
		arx_assert(offset < capacity());
		arx_assert(offset + count <= capacity());
		
		m_renderer->beforeDraw<Vertex>();
		
		bindBuffer(m_buffer);
		
		setVertexArray<Vertex>(NULL, this);
		
		glDrawArrays(arxToGlPrimitiveType[primitive], m_offset + offset, count);
		
	}
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset,
	                 unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset < capacity());
		arx_assert(offset + count <= capacity());
		arx_assert(indices != NULL);
		
		offset += m_offset;
		
		m_renderer->beforeDraw<Vertex>();
		
		bindBuffer(m_buffer);
		
		setVertexArray<Vertex>(NULL, this);
		
		if(GLEW_ARB_draw_elements_base_vertex) {
			glDrawRangeElementsBaseVertex(arxToGlPrimitiveType[primitive], 0, count - 1,
			                              nbindices, GL_UNSIGNED_SHORT, indices, offset);
		} else if(offset + count - 1 <= std::numeric_limits<GLushort>::max()) {
			glShortIndexBuffer.resize(std::max(glShortIndexBuffer.size(), nbindices));
			for(size_t i = 0; i < nbindices; i++) {
				glShortIndexBuffer[i] = GLushort(indices[i] + offset);
			}
			glDrawRangeElements(arxToGlPrimitiveType[primitive], offset, offset + count - 1,
			                    nbindices, GL_UNSIGNED_SHORT, &glShortIndexBuffer[0]);
		} else {
			glIntIndexBuffer.resize(std::max(glIntIndexBuffer.size(), nbindices));
			for(size_t i = 0; i < nbindices; i++) {
				glIntIndexBuffer[i] = GLuint(indices[i] + offset);
			}
			glDrawRangeElements(arxToGlPrimitiveType[primitive], offset, offset + count - 1,
			                    nbindices, GL_UNSIGNED_INT, &glIntIndexBuffer[0]);
		}
		
	}
	
	~BaseGLVertexBuffer() {
		clearVertexArray(this);
		unbindBuffer(m_buffer);
		glDeleteBuffers(1, &m_buffer);
	}
	
protected:
	
	OpenGLRenderer * m_renderer;
	GLuint m_buffer;
	size_t m_offset;
	
};

static const GLenum arxToGlBufferUsage[] = {
	GL_STATIC_DRAW,  // Static,
	GL_DYNAMIC_DRAW, // Dynamic,
	GL_STREAM_DRAW   // Stream
};

template <class Vertex>
class GLVertexBuffer : public BaseGLVertexBuffer<Vertex> {
	
	typedef BaseGLVertexBuffer<Vertex> Base;
	
public:
	
	using Base::capacity;
	
	GLVertexBuffer(OpenGLRenderer * renderer, size_t capacity, Renderer::BufferUsage usage)
		: Base(renderer, capacity)
		, m_usage(usage)
		, m_initialized(false)
	{ }
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		
		arx_assert(offset < capacity());
		arx_assert(offset + count <= capacity());
		
		bindBuffer(m_buffer);
		
		if(offset == 0 && count == capacity()) {
			initialize(vertices);
			return;
		}
		
		if(!m_initialized || (flags & DiscardBuffer)) {
			// Orphan buffer to avoid waiting if the GL is still using the old contents
			initialize();
		}
		
		if(count != 0) {
			size_t obytes = offset * sizeof(Vertex);
			size_t nbytes = count * sizeof(Vertex);
			glBufferSubData(GL_ARRAY_BUFFER, obytes, nbytes, vertices);
		}
		
	}
	
	Vertex * lock(BufferFlags flags, size_t offset, size_t count) {
		ARX_UNUSED(count);
		
		arx_assert(offset < capacity());
		
		bindBuffer(m_buffer);
		
		if(!m_initialized || (flags & DiscardBuffer)) {
			// Orphan buffer to avoid waiting if the GL is still using the old contents
			initialize();
		}
		
		void * buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if(!buf) {
			LogError << "Could not map vertex buffer!";
		}
		
		return reinterpret_cast<Vertex *>(buf) + offset;
	}
	
	void unlock() {
		
		bindBuffer(m_buffer);
		
		GLboolean ret = glUnmapBuffer(GL_ARRAY_BUFFER);
		if(ret == GL_FALSE) {
			LogWarning << "Vertex buffer invalidated";
		}
		
	}
	
protected:
	
	void initialize(const Vertex * data = NULL) {
		GLenum usage = arxToGlBufferUsage[m_usage];
		glBufferData(GL_ARRAY_BUFFER, capacity() * sizeof(Vertex), data, usage);
		m_initialized = true;
	}
	
	using Base::m_buffer;
	Renderer::BufferUsage m_usage;
	bool m_initialized;
	
};

template <class Vertex>
class GLMapRangeVertexBuffer : public GLVertexBuffer<Vertex> {
	
	typedef GLVertexBuffer<Vertex> Base;
	
public:
	
	using Base::capacity;
	
	GLMapRangeVertexBuffer(OpenGLRenderer * renderer, size_t capacity,
	                       Renderer::BufferUsage usage)
		: Base(renderer, capacity, usage)
	{ }
	
	Vertex * lock(BufferFlags flags, size_t offset, size_t count) {
		
		arx_assert(offset < capacity());
		
		bindBuffer(m_buffer);
		
		if(!m_initialized) {
			initialize();
		}
		
		GLbitfield glflags = GL_MAP_WRITE_BIT;
		
		if(flags & DiscardBuffer) {
			glflags |= GL_MAP_INVALIDATE_BUFFER_BIT;
		}
		if(flags & DiscardRange) {
			glflags |= GL_MAP_INVALIDATE_RANGE_BIT;
		}
		if(flags & NoOverwrite) {
			glflags |= GL_MAP_UNSYNCHRONIZED_BIT;
		}
		
		size_t obytes = offset * sizeof(Vertex);
		size_t nbytes = std::min(count, capacity() - offset) * sizeof(Vertex);
		
		void * buf = glMapBufferRange(GL_ARRAY_BUFFER, obytes, nbytes, glflags);
		if(!buf) {
			LogError << "Could not map vertex buffer!";
		}
		
		return reinterpret_cast<Vertex *>(buf);
	}
	
protected:
	
	using Base::initialize;
	
	using Base::m_buffer;
	using Base::m_initialized;
	
};

#ifdef GL_ARB_buffer_storage

template <class Vertex>
class BaseGLPersistentVertexBuffer : public BaseGLVertexBuffer<Vertex> {
	
	typedef BaseGLVertexBuffer<Vertex> Base;
	
public:
	
	using Base::capacity;
	
	BaseGLPersistentVertexBuffer(OpenGLRenderer * renderer, size_t capacity,
	                             size_t multiplier = 1)
		: Base(renderer, capacity)
		, m_multiplier(multiplier)
		, m_mapping(NULL)
	{
		
		bindBuffer(m_buffer);
		m_mapping = create();
		
	}
	
protected:
	
	size_t size() const { return capacity() * sizeof(Vertex) * m_multiplier; }
	
	Vertex * map(GLbitfield flags) const {
		flags |= GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		return reinterpret_cast<Vertex *>(glMapBufferRange(GL_ARRAY_BUFFER, 0, size(), flags));
	}
	
	Vertex * create() const {
		
		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
		glBufferStorage(GL_ARRAY_BUFFER, size(), NULL, flags);
		
		return map(GL_MAP_UNSYNCHRONIZED_BIT);
	}
	
	virtual void discardBuffer() = 0;
	
	void syncPersistent(BufferFlags flags) {
		
		if(flags & DiscardBuffer) {
			discardBuffer();
		} else if(!(flags & NoOverwrite)) {
			LogWarning << "Blocking buffer upload, use DiscardBuffer or NoOverwrite!";
			glFinish();
		}
		
	}
	
public:
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		
		arx_assert(offset < capacity());
		arx_assert(offset + count <= capacity());
		
		syncPersistent(flags);
		
		memcpy(m_mapping + m_offset + offset, vertices, count * sizeof(Vertex));
		
	}
	
	Vertex * lock(BufferFlags flags, size_t offset, size_t count) {
		ARX_UNUSED(count);
		
		arx_assert(offset < capacity());
		
		syncPersistent(flags);
		
		return m_mapping + m_offset + offset;
	}
	
	void unlock() { }
	
	~BaseGLPersistentVertexBuffer() {
		
		if(m_mapping) {
			bindBuffer(m_buffer);
			GLboolean ret = glUnmapBuffer(GL_ARRAY_BUFFER);
			if(ret == GL_FALSE) {
				LogWarning << "Persistently mapped vertex buffer invalidated";
			}
		}
		
	}
	
protected:
	
	using Base::m_buffer;
	using Base::m_offset;
	
	size_t m_multiplier;
	
	Vertex * m_mapping;
	
};

template <class Vertex>
class GLPersistentUnsynchronizedVertexBuffer
	: public BaseGLPersistentVertexBuffer<Vertex>
{
	
	typedef BaseGLPersistentVertexBuffer<Vertex> Base;
	
public:
	
	GLPersistentUnsynchronizedVertexBuffer(OpenGLRenderer * renderer, size_t capacity)
		: Base(renderer, capacity)
	{ }
	
	void discardBuffer() {
		// Assume the GL already rendered far enough
	}
	
protected:
	
	using Base::map;
	
	using Base::m_buffer;
	using Base::m_mapping;
	
};

template <class Vertex>
class GLPersistentOrphanVertexBuffer
	: public BaseGLPersistentVertexBuffer<Vertex>
{
	
	typedef BaseGLPersistentVertexBuffer<Vertex> Base;
	
public:
	
	GLPersistentOrphanVertexBuffer(OpenGLRenderer * renderer, size_t capacity)
		: Base(renderer, capacity)
	{ }
	
	void discardBuffer() {
		
		bindBuffer(m_buffer);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		m_mapping = map(GL_MAP_INVALIDATE_BUFFER_BIT);
		
	}
	
protected:
	
	using Base::map;
	
	using Base::m_buffer;
	using Base::m_mapping;
	
};

template <class Vertex, size_t MaxFences>
class GLPersistentFenceVertexBuffer
	: public BaseGLPersistentVertexBuffer<Vertex>
{
	
	typedef BaseGLPersistentVertexBuffer<Vertex> Base;
	
public:
	
	using Base::capacity;
	
	GLPersistentFenceVertexBuffer(OpenGLRenderer * renderer, size_t capacity,
	                              size_t fenceCount)
		: Base(renderer, capacity, fenceCount)
	{
		
		m_position = 0;
		
		arx_assert(m_multiplier <= MaxFences);
		for(size_t i = 0; i < m_multiplier; i++) {
			m_fences[i] = 0;
		}
		
	}
	
	void discardBuffer() {
		
		// Create a fence for the current buffer
		arx_assert(!m_fences[m_position]);
		m_fences[m_position] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		
		// Switch to the next buffer and wait for its fence
		m_position = (m_position + 1) % m_multiplier;
		if(m_fences[m_position]) {
			GLenum ret = GL_UNSIGNALED;
			while(ret != GL_ALREADY_SIGNALED && ret != GL_CONDITION_SATISFIED) {
				ret = glClientWaitSync(m_fences[m_position], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
			}
			glDeleteSync(m_fences[m_position]);
			m_fences[m_position] = 0;
		}
		
		m_offset = m_position * capacity();
		
	}
	
	~GLPersistentFenceVertexBuffer() {
		
		for(size_t i = 0; i < m_multiplier; i++) {
			if(m_fences[i]) {
				glDeleteSync(m_fences[i]);
			}
		}
		
	}
	
protected:
	
	using Base::m_offset;
	using Base::m_multiplier;
	
	size_t m_position;
	GLsync m_fences[MaxFences];
	
};

#endif // GL_ARB_buffer_storage

#endif // ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
