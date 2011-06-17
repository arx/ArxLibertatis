#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "graphics/BaseGraphicsTypes.h"
#include "graphics/data/Texture.h"
#include "graphics/Color.h"
#include "graphics/texture/Texture.h"
#include "graphics/texture/TextureStage.h"

class Renderer
{
public:
	//! Render states
	enum RenderState
	{
		AlphaBlending,
		ColorKey,
		DepthTest,
		DepthWrite,
		Fog,
		Lighting,
		ZBias
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

	enum FillMode
	{
		FillPoint,
		FillWireframe,
		FillSolid
	};

	//! Fog
	enum FogMode
	{
		FogNone,
		FogExp,
		FogExp2,
		FogLinear
	};

	//! Target surface
	enum BufferType {
		ColorBuffer   = (1<<0),
		DepthBuffer   = (1<<1),
		StencilBuffer = (1<<2)
	};
	DECLARE_FLAGS(BufferType, BufferFlags);

	Renderer();
	virtual ~Renderer();

	void Initialize();

	// Scene begin/end...
	virtual bool BeginScene();
	virtual bool EndScene();

    // Matrices
	virtual void SetViewMatrix(const EERIEMATRIX& matView);
	virtual void SetViewMatrix(const Vec3f & vPosition, const Vec3f & vDir, const Vec3f & vUp);
	virtual void GetViewMatrix(EERIEMATRIX& matView) const;
	virtual void SetProjectionMatrix(const EERIEMATRIX& matProj);
	virtual void GetProjectionMatrix(EERIEMATRIX& matProj) const;

	// Texture management
	virtual void ReleaseAllTextures();
	virtual void RestoreAllTextures();

	// Factory
	virtual Texture2D*	CreateTexture2D();

	// Render states
	virtual void SetRenderState(RenderState renderState, bool enable);

	// Alphablending & Transparency
	virtual void SetAlphaFunc(PixelCompareFunc func, float fef);    // Ref = [0.0f, 1.0f]
	virtual void SetBlendFunc(PixelBlendingFactor srcFactor, PixelBlendingFactor dstFactor);

	// Viewport
	virtual void SetViewport(const Rect & viewport);
	virtual Rect GetViewport();

	// Projection
	virtual void Begin2DProjection(float left, float right, float bottom, float top, float zNear, float zFar);
	virtual void End2DProjection();

	// Render Target
	virtual void Clear(BufferFlags bufferFlags, Color clearColor = Color::none, float clearDepth = 1.f, size_t nrects = 0, Rect * rect = 0);

	// Fog
	virtual void SetFogColor(Color color);
	virtual void SetFogParams(FogMode fogMode, float fogStart, float fogEnd, float fogDensity = 1.0f);
		
	// Rasterizer
	virtual void SetAntialiasing(bool enable);
	virtual void SetCulling(CullingMode mode);
	virtual void SetDepthBias(int depthBias);
	virtual void SetFillMode(FillMode mode);
	
	// Texturing
	virtual unsigned int GetTextureStageCount() const;
	virtual TextureStage* GetTextureStage(unsigned int textureStage);
	virtual void ResetTexture(unsigned int textureStage);
	virtual void SetTexture(unsigned int textureStage, Texture* pTexture);
	virtual void SetTexture(unsigned int textureStage, TextureContainer* pTextureContainer);
	
	virtual float GetMaxAnisotropy() const;

	// Utilities...
	virtual void DrawTexturedRect(float x, float y, float w, float h, float uStart, float vStart, float uEnd, float vEnd, Color color);

private:
	std::vector<TextureStage*>	m_TextureStages;
};

DECLARE_FLAGS_OPERATORS(Renderer::BufferFlags)

extern Renderer* GRenderer;


#endif // _RENDERER_H_
