#ifndef _RENDERER_H_
#define _RENDERER_H_


#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"

// TEMP - move back to .cpp once types are all abstracted
#include <windows.h>
#include <d3d.h>

class Renderer
{
public:
	//! Render states
	enum RenderState
    {
        AlphaBlending,
		DepthTest,
		DepthWrite,
        Lighting,
    };

	//! Pixel comparison functions
    enum PixelCompareFunc
    {
        CmpNever,               //!< Never
        CmpLess,                //!< Less
        CmpEqual,               //!< Equal
        CmpLessEqual,           //!< Less Equal
        CmpGreater,             //!< Greater
        CmpNotEqual,            //!< Not Equal
        CmpGreaterEqual,        //!< Greater Equal
        CmpAlways               //!< Always
    };

    //! Pixel blending factor
    enum PixelBlendingFactor
    {
        BlendZero,              //!< Zero    
        BlendOne,               //!< One
        BlendSrcColor,          //!< Source color
        BlendSrcAlpha,          //!< Source alpha
        BlendInvSrcColor,       //!< Inverse source color
        BlendInvSrcAlpha,       //!< Inverse source alpha
        BlendSrcAlphaSaturate,  //!< Source alpha saturate
        BlendDstColor,          //!< Destination color
        BlendDstAlpha,          //!< Destination alpha
        BlendInvDstColor,       //!< Inverse destination color
        BlendInvDstAlpha        //!< Inverse destination alpha
    };

	//! Culling 
	enum CullingMode
    {
        CullNone,
        CullCW,
        CullCCW
    };

	//! Target surface
	enum BufferType
    {
        ColorBuffer     = 0x00000001,
        DepthBuffer     = 0x00000002,
        StencilBuffer   = 0x00000004
    };

	struct Viewport
	{
		int x;
		int y;
		int width;
		int height;
	};

	Renderer();
	~Renderer();

	// Texture management
	void ReleaseAllTextures();
	void RestoreAllTextures();

	// Factory
	Texture1D*	CreateTexture1D();
	Texture2D*	CreateTexture2D();
	Texture3D*	CreateTexture3D();
	Cubemap*	CreateCubemap();

	// Render states
    void SetRenderState(RenderState renderState, bool enable);

    // Alphablending & Transparency
    void SetAlphaFunc(PixelCompareFunc func, float fef);    // Ref = [0.0f, 1.0f]
    void SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor);

	// Viewport
    void SetViewport(const Viewport& viewport);
    Viewport GetViewport();

	// Culling & Clipping
	void SetCulling( CullingMode mode );
	
	// Projection
	void Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar);
    void End2DProjection();

	// Render Target
	void Clear(int bufferFlags, COLORREF clearColor = 0, float clearDepth = 1.0f, unsigned int rectCount = 0, D3DRECT* pRects = 0);

	// Texturing
	unsigned int GetTextureStageCount() const;
	TextureStage* GetTextureStage(unsigned int textureStage);
    void ResetTexture(unsigned int textureStage);
    void SetTexture(unsigned int textureStage, Texture& pTexture);
	
	float GetMaxAnisotropy() const;

	// Utilities...
	void DrawTexturedRect( float pX, float pY, float pW, float pH, float pUStart, float pVStart, float pUEnd, float pVEnd, COLORREF pColor );

private:
	std::vector<TextureStage*>	m_TextureStages;
};

extern Renderer* GRenderer;


#endif // _RENDERER_H_
