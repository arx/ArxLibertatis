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

void bindBuffer(GLuint buffer);
void unbindBuffer(GLuint buffer);
void setVertexArrayTexCoord(int index, const void * coord, size_t stride);
bool switchVertexArray(GLArrayClientState type, const void * ref, int texcount);

template <>
inline void setVertexArray(const TexturedVertex * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_TexturedVertex, ref, 1)) {
		return;
	}
	
	// rhw -> w conversion is done in a vertex shader
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, sizeof(TexturedVertex), &vertices->p);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(TexturedVertex), &vertices->color);
	
	setVertexArrayTexCoord(0, &vertices->uv, sizeof(TexturedVertex));
	
	CHECK_GL;
}

template <>
inline void setVertexArray(const SMY_VERTEX * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_SMY_VERTEX, ref, 1)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX), &vertices->p.x);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX), &vertices->color);
	
	setVertexArrayTexCoord(0, &vertices->uv, sizeof(SMY_VERTEX));
	
	CHECK_GL;
}

template <>
inline void setVertexArray(const SMY_VERTEX3 * vertices, const void * ref) {
	
	if(!switchVertexArray(GL_SMY_VERTEX3, ref, 3)) {
		return;
	}
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(SMY_VERTEX3), &vertices->p.x);
	
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(GL_BGRA, GL_UNSIGNED_BYTE, sizeof(SMY_VERTEX3), &vertices->color);
	
	setVertexArrayTexCoord(0, &vertices->uv[0], sizeof(SMY_VERTEX3));
	setVertexArrayTexCoord(1, &vertices->uv[1], sizeof(SMY_VERTEX3));
	setVertexArrayTexCoord(2, &vertices->uv[2], sizeof(SMY_VERTEX3));
	
	CHECK_GL;
}

static const GLenum arxToGlBufferUsage[] = {
	GL_STATIC_DRAW,  // Static,
	GL_DYNAMIC_DRAW, // Dynamic,
	GL_STREAM_DRAW   // Stream
};

extern const GLenum arxToGlPrimitiveType[];

extern std::vector<GLushort> glShortIndexBuffer;
extern std::vector<GLuint> glIntIndexBuffer;

template <class Vertex>
class GLVertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	using VertexBuffer<Vertex>::capacity;
	
	GLVertexBuffer(OpenGLRenderer * _renderer, size_t capacity, Renderer::BufferUsage _usage) : VertexBuffer<Vertex>(capacity), renderer(_renderer), buffer(0), usage(_usage) {
		
		glGenBuffers(1, &buffer);
		
		arx_assert(buffer != GL_NONE);
		
		bindBuffer(buffer);
		glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(Vertex), NULL, arxToGlBufferUsage[usage]);
		
		CHECK_GL;
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset, BufferFlags flags) {
		ARX_UNUSED(flags);
		
		arx_assert(offset + count <= capacity());
		
		bindBuffer(buffer);
		
		if(GLEW_ARB_map_buffer_range && count != 0) {
			
			GLbitfield glflags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT;
			
			if(flags & DiscardBuffer) {
				glflags |= GL_MAP_INVALIDATE_BUFFER_BIT;
			}
			if(flags & NoOverwrite) {
				glflags |= GL_MAP_UNSYNCHRONIZED_BIT;
			}
			
			size_t obytes = offset * sizeof(Vertex);
			size_t nbytes = count * sizeof(Vertex);
			void * buf = glMapBufferRange(GL_ARRAY_BUFFER, obytes, nbytes, glflags);
			
			if(buf) {
				
				memcpy(buf, vertices, nbytes);
				
				GLboolean ret = glUnmapBuffer(GL_ARRAY_BUFFER);
				if(ret == GL_FALSE) {
					// TODO handle GL_FALSE return (buffer invalidated)
					LogWarning << "Vertex buffer invalidated";
				}
				
			}
			
		} else {
			
			if(flags & DiscardBuffer) {
				// avoid waiting if GL is still using the old buffer contents
				glBufferData(GL_ARRAY_BUFFER, capacity() * sizeof(Vertex), NULL, arxToGlBufferUsage[usage]);
			}
			
			if(count != 0) {
				glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), count * sizeof(Vertex), vertices);
			}
			
		}
		
		CHECK_GL;
	}
	
	Vertex * lock(BufferFlags flags, size_t offset, size_t count) {
		ARX_UNUSED(flags);
		
		bindBuffer(buffer);
		
		Vertex * buf;
		
		if(GLEW_ARB_map_buffer_range) {
			
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
			
			buf = reinterpret_cast<Vertex *>(glMapBufferRange(GL_ARRAY_BUFFER, obytes, nbytes, glflags));
			
		} else {
			
			if(flags & DiscardBuffer) {
				// avoid waiting if GL is still using the old buffer contents
				glBufferData(GL_ARRAY_BUFFER, capacity() * sizeof(Vertex), NULL, arxToGlBufferUsage[usage]);
			}
			
			buf = reinterpret_cast<Vertex *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
			if(buf) {
				buf += offset;
			}
		}
		
		CHECK_GL;
		
		arx_assert(buf != NULL); // TODO OpenGL doesn't guarantee this
		
		return buf;
	}
	
	void unlock() {
		
		bindBuffer(buffer);
		
		GLboolean ret = glUnmapBuffer(GL_ARRAY_BUFFER);
		
		if(ret == GL_FALSE) {
			// TODO handle GL_FALSE return (buffer invalidated)
			LogWarning << "Vertex buffer invalidated";
		}
		
		CHECK_GL;
	}
	
	void draw(Renderer::Primitive primitive, size_t count, size_t offset) const {
		
		arx_assert(offset + count <= capacity());
		
		renderer->beforeDraw<Vertex>();
		
		bindBuffer(buffer);
		
		setVertexArray<Vertex>(NULL, this);
		
		glDrawArrays(arxToGlPrimitiveType[primitive], offset, count);
		
		CHECK_GL;
	}
	
	void drawIndexed(Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= capacity());
		arx_assert(indices != NULL);
		
		renderer->beforeDraw<Vertex>();
		
		bindBuffer(buffer);
		
		setVertexArray<Vertex>(NULL, this);
		
		if(GLEW_ARB_draw_elements_base_vertex) {
			
			glDrawRangeElementsBaseVertex(arxToGlPrimitiveType[primitive], 0, count - 1, nbindices, GL_UNSIGNED_SHORT, indices, offset);
			
			CHECK_GL;
			
		} else if(offset + count - 1 <= std::numeric_limits<GLushort>::max()) {
			
			glShortIndexBuffer.resize(std::max(glShortIndexBuffer.size(), nbindices));
			for(size_t i = 0; i < nbindices; i++) {
				glShortIndexBuffer[i] = GLushort(indices[i] + offset);
			}
			
			glDrawRangeElements(arxToGlPrimitiveType[primitive], offset, offset + count - 1, nbindices, GL_UNSIGNED_SHORT, &glShortIndexBuffer[0]);
			
			CHECK_GL;
			
		} else {
			
			glIntIndexBuffer.resize(std::max(glIntIndexBuffer.size(), nbindices));
			for(size_t i = 0; i < nbindices; i++) {
				glIntIndexBuffer[i] = GLuint(indices[i] + offset);
			}
			
			glDrawRangeElements(arxToGlPrimitiveType[primitive], offset, offset + count - 1, nbindices, GL_UNSIGNED_INT, &glIntIndexBuffer[0]);
			
			CHECK_GL;
		}
	}
	
	~GLVertexBuffer() {
		unbindBuffer(buffer);
		glDeleteBuffers(1, &buffer);
		CHECK_GL;
	};
	
private:
	
	OpenGLRenderer * renderer;
	GLuint buffer;
	Renderer::BufferUsage usage;
	
};

#endif // ARX_GRAPHICS_OPENGL_GLVERTEXBUFFER_H
