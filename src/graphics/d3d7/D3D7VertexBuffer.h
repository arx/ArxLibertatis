
#ifndef ARX_GRAPHICS_D3D7_D3D7VERTEXBUFFER_H
#define ARX_GRAPHICS_D3D7_D3D7VERTEXBUFFER_H

#include <d3d.h>

#include "graphics/VertexBuffer.h"
#include "window/D3D7Window.h"

extern const DWORD ARXToDXBufferFlags[];
extern const D3DPRIMITIVETYPE ARXToDXPrimitiveType[];
extern LPDIRECT3DDEVICE7 GD3D7Device;

template <class Vertex>
class D3D7VertexBuffer : public VertexBuffer<Vertex> {
	
public:
	
	D3D7VertexBuffer(D3D7Window * window, DWORD format, size_t capacity) : VertexBuffer<Vertex>(capacity) {
		
		D3DVERTEXBUFFERDESC d3dvbufferdesc;
		d3dvbufferdesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
		
		d3dvbufferdesc.dwCaps = D3DVBCAPS_WRITEONLY;
		if(!(window->getInfo().device.dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)) {
			d3dvbufferdesc.dwCaps |= D3DVBCAPS_SYSTEMMEMORY;
		}
		
		d3dvbufferdesc.dwFVF = format;
		d3dvbufferdesc.dwNumVertices = capacity;
		
		HRESULT hr = window->getD3D()->CreateVertexBuffer(&d3dvbufferdesc, &vb, 0);
		arx_assert_msg(SUCCEEDED(hr), "error creating vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
	}
	
	void setData(const Vertex * vertices, size_t count, size_t offset = 0, BufferFlags flags = 0) {
		
		if(Vertex * buffer = lock(flags | DiscardRange, offset, count)) {
			
			memcpy(buffer, vertices, count * sizeof(Vertex));
			
			unlock();
		}
	}
	
	Vertex * lock(BufferFlags flags, size_t offset, size_t count) {
		ARX_UNUSED(count);
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		LPVOID dest = NULL;
		
		HRESULT hr = vb->Lock(DDLOCK_WRITEONLY | ARXToDXBufferFlags[flags], &dest, NULL);
		arx_assert_msg(SUCCEEDED(hr), "error locking vertex buffer: %08x", hr);
		ARX_UNUSED(hr);
		
		return dest ? reinterpret_cast<Vertex *>(dest) + offset : NULL;
	}
	
	void unlock() {
		vb->Unlock();
	}
	
	void draw(D3D7Renderer::Primitive primitive, size_t count, size_t offset = 0) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		
		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		HRESULT hr = GD3D7Device->DrawPrimitiveVB(type, vb, offset, count, 0);
		arx_assert_msg(SUCCEEDED(hr), "DrawPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
	
	
	void drawIndexed(D3D7Renderer::Primitive primitive, size_t count, size_t offset, unsigned short * indices, size_t nbindices) const {
		
		arx_assert(offset + count <= VertexBuffer<Vertex>::capacity());
		arx_assert(indices != NULL);
		
		D3DPRIMITIVETYPE type = ARXToDXPrimitiveType[primitive];
		HRESULT hr = GD3D7Device->DrawIndexedPrimitiveVB(type, vb, offset, count, indices, nbindices, 0);
		arx_assert_msg(SUCCEEDED(hr), "DrawIndexedPrimitiveVB failed: %08x", hr);
		ARX_UNUSED(hr);
	}
	
	~D3D7VertexBuffer() {
		vb->Release();
	};
	
private:
	
	LPDIRECT3DVERTEXBUFFER7 vb;
	LPDIRECT3DDEVICE7 device;
	
};

#endif // ARX_GRAPHICS_D3D7_D3D7VERTEXBUFFER_H
