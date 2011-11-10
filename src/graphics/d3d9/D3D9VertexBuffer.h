/*
 * Copyright 2011 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_D3D9_D3D9VERTEXBUFFER_H
#define ARX_GRAPHICS_D3D9_D3D9VERTEXBUFFER_H

#include <d3d9.h>

#include "graphics/VertexBuffer.h"
#include "window/D3D9Window.h"

extern const DWORD ARXToDXBufferFlags[];
extern const D3DPRIMITIVETYPE ARXToDXPrimitiveType[];
extern LPDIRECT3DDEVICE9 GD3D9Device;

extern UINT GetNumberOfPrimitives(Renderer::Primitive primitive, UINT nindices);

template <class Index>
class D3DIndexBuffer : public IndexBuffer<Index> {

public:

	D3DIndexBuffer(size_t capacity, Renderer::BufferUsage usage) : IndexBuffer<Index>(capacity) {

		D3DCAPS9 deviceCaps;
		GD3D9Device->GetDeviceCaps(&deviceCaps);

		D3DPOOL poolType = D3DPOOL_MANAGED;
		DWORD dwUsage = D3DUSAGE_WRITEONLY;

		if(usage & (Renderer::Dynamic | Renderer::Stream) ) {
			poolType = D3DPOOL_DEFAULT;
			dwUsage |= D3DUSAGE_DYNAMIC;
		}

		if(!(deviceCaps.DeviceType & D3DDEVTYPE_HAL))
			dwUsage |= D3DUSAGE_SOFTWAREPROCESSING;

		D3DFORMAT format = sizeof(Index) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32;

		HRESULT hr = GD3D9Device->CreateIndexBuffer(capacity * sizeof(Index), dwUsage, format, poolType, &ib, 0);
		arx_assert_msg(SUCCEEDED(hr), "error creating index buffer: %08x", hr);
		ARX_UNUSED(hr);
	}

	void setData(const Index * indices, size_t count, size_t offset = 0, BufferFlags flags = 0) {
		arx_assert(offset + count <= IndexBuffer<Index>::capacity());

		Index * dest = lock(flags, offset, count);
		memcpy(dest, indices, count * sizeof(Index));
		unlock();
	}

	Index * lock(BufferFlags flags, size_t offset, size_t count) {
		Index * dest = NULL;
		
		HRESULT hr = ib->Lock(offset * sizeof(Index), count * sizeof(Index), (LPVOID*)&dest, ARXToDXBufferFlags[flags]);
		arx_assert_msg(SUCCEEDED(hr), "error locking index buffer: %08x", hr);
		ARX_UNUSED(hr);
		
		return dest;
	}

	void unlock() {
		ib->Unlock();
	}

	~D3DIndexBuffer() {
		ib->Release();
	};
	
	LPDIRECT3DINDEXBUFFER9	ib;
};

template <class Vertex>
class D3D9VertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	D3D9VertexBuffer(DWORD format, size_t capacity, Renderer::BufferUsage usage) 
		: VertexBuffer<Vertex>(capacity)
		, ib(capacity*4, Renderer::Dynamic) {
		
		D3DCAPS9 deviceCaps;
		GD3D9Device->GetDeviceCaps(&deviceCaps);
		
		D3DPOOL poolType = D3DPOOL_MANAGED;
		dwUsage = D3DUSAGE_WRITEONLY;

		if(usage & (Renderer::Dynamic | Renderer::Stream) ) {
			poolType = D3DPOOL_DEFAULT;
			dwUsage |= D3DUSAGE_DYNAMIC;
		}
		
		if(!(deviceCaps.DeviceType & D3DDEVTYPE_HAL))
			dwUsage |= D3DUSAGE_SOFTWAREPROCESSING;
				
		dwFVF = format;
		
		HRESULT hr = GD3D9Device->CreateVertexBuffer(capacity * sizeof(Vertex), dwUsage, dwFVF, poolType, &vb, 0);
		arx_assert_msg(SUCCEEDED(hr), "error creating vertex buffer: %08x", hr);
		ARX_UNUSED(hr);		
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset = 0, BufferFlags flags = 0) {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		Vertex * dest = lock(flags, offset, count);
		memcpy(dest, vertices, count * sizeof(Vertex));
		unlock();
	}
	
	Vertex * lock(BufferFlags flags, size_t offset, size_t count) {
		Vertex * dest = NULL;
		
		if(count == (size_t)-1)
			count = VertexBuffer<Vertex>::capacity();

		HRESULT hr = vb->Lock(offset * sizeof(Vertex), count * sizeof(Vertex), (LPVOID*)&dest, ARXToDXBufferFlags[flags]);
		arx_assert_msg(SUCCEEDED(hr), "error locking vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
		return dest;
	}
	
	void unlock() {
		vb->Unlock();
	}
	
	void draw(D3D9Renderer::Primitive primitive, size_t count, size_t offset = 0) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		GD3D9Device->SetStreamSource( 0, vb, offset * sizeof(Vertex), sizeof(Vertex));
		GD3D9Device->SetFVF(dwFVF);

		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		UINT nbPrimitives = GetNumberOfPrimitives(primitive, count);

		HRESULT hr = GD3D9Device->DrawPrimitive(type, 0, nbPrimitives);

		arx_assert_msg(SUCCEEDED(hr), "DrawPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
		
	void drawIndexed(D3D9Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		arx_assert(indices != NULL);
		
		ib.setData(indices, nbindices, 0, DiscardBuffer);
		
		GD3D9Device->SetIndices(ib.ib);
		GD3D9Device->SetStreamSource( 0, vb, offset * sizeof(Vertex), sizeof(Vertex));
		GD3D9Device->SetFVF(dwFVF);
		
		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		UINT nbPrimitives = GetNumberOfPrimitives(primitive, nbindices);
		
		HRESULT hr = GD3D9Device->DrawIndexedPrimitive(type, 0, 0, count, 0, nbPrimitives);
		
		arx_assert_msg(SUCCEEDED(hr), "DrawIndexedPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
	
	~D3D9VertexBuffer() {
		vb->Release();
	};
	
private:
	mutable D3DIndexBuffer<unsigned short> ib;
	LPDIRECT3DVERTEXBUFFER9 vb;
	DWORD dwFVF;
	DWORD dwUsage;
};

#endif // ARX_GRAPHICS_D3D9_D3D9VERTEXBUFFER_H
