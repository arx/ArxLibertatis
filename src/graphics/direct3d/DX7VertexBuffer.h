
#ifndef ARX_GRAPHICS_DIRECT3D_DX7VERTEXBUFFER_H
#define ARX_GRAPHICS_DIRECT3D_DX7VERTEXBUFFER_H

#include <d3d.h>

#include "core/Core.h"
#include "graphics/VertexBuffer.h"

extern const DWORD ARXToDXBufferFlags[];
extern const D3DPRIMITIVETYPE ARXToDXPrimitiveType[];

template <class Vertex>
class DX7VertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	DX7VertexBuffer(DWORD format, size_t capacity) : VertexBuffer<Vertex>(capacity) {
		
		D3DVERTEXBUFFERDESC d3dvbufferdesc;
		d3dvbufferdesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
		
		d3dvbufferdesc.dwCaps = D3DVBCAPS_WRITEONLY;
		if(!(danaeApp.m_pDeviceInfo->ddDeviceDesc.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)) {
			d3dvbufferdesc.dwCaps |= D3DVBCAPS_SYSTEMMEMORY;
		}
		
		d3dvbufferdesc.dwFVF = format;
		d3dvbufferdesc.dwNumVertices = capacity;
		
		HRESULT hr = danaeApp.m_pD3D->CreateVertexBuffer(&d3dvbufferdesc, &vb, 0);
		arx_assert_msg(SUCCEEDED(hr), "error creating vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset = 0, BufferFlags flags = 0) {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		Vertex * dest;
		
		HRESULT hr = vb->Lock(DDLOCK_WRITEONLY | ARXToDXBufferFlags[flags], (LPVOID*)&dest, NULL);
		arx_assert_msg(SUCCEEDED(hr), "error locking vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
		memcpy(dest + offset, vertices, count * sizeof(Vertex));
		
		vb->Unlock();
	}
	
	Vertex * lock(BufferFlags flags = 0) {
		
		Vertex * dest;
		
		HRESULT hr = vb->Lock(DDLOCK_WRITEONLY | ARXToDXBufferFlags[flags], (LPVOID*)&dest, NULL);
		arx_assert_msg(SUCCEEDED(hr), "error locking vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
		return dest;
	}
	
	void unlock() {
		vb->Unlock();
	}
	
	void draw(Direct3DRenderer::Primitive primitive, size_t count, size_t offset = 0) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		HRESULT hr = GDevice->DrawPrimitiveVB(type, vb, offset, count, 0);
		arx_assert_msg(SUCCEEDED(hr), "DrawPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
	
	
	void drawIndexed(Direct3DRenderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		arx_assert(indices != NULL);
		
		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		HRESULT hr = GDevice->DrawIndexedPrimitiveVB(type, vb, offset, count, indices, nbindices, 0);
		arx_assert_msg(SUCCEEDED(hr), "DrawIndexedPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
	
	~DX7VertexBuffer() {
		vb->Release();
	};
	
private:
	
	LPDIRECT3DVERTEXBUFFER7 vb;
	
};

#endif // ARX_GRAPHICS_DIRECT3D_D3DVERTEXBUFFER_H
