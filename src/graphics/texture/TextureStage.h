/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GRAPHICS_TEXTURE_TEXTURESTAGE_H
#define ARX_GRAPHICS_TEXTURE_TEXTURESTAGE_H

class Texture;

class TextureStage {
	
public:
	
	//! Texture blending operations
	enum TextureOp {
		OpDisable,    //!< Disables output from this texture stage and all stages with a higher index.
		OpSelectArg1, //!< Use this texture stage's first color or alpha argument, unmodified, as the output.
		OpSelectArg2, //!< Use this texture stage's second color or alpha argument, unmodified, as the output.
		OpModulate,   //!< Multiply the components of the arguments together.
		OpModulate2X, //!< Multiply the components of the arguments, and shift the products to the left 1 bit.
		OpModulate4X, //!< Multiply the components of the arguments, and shift the products to the left 2 bits.
		OpAddSigned   //!< Add args with -0.5 bias
	};
	
	//! Texture blending arguments
	enum TextureArg {
		ArgDiffuse    = 0x00000,
		ArgCurrent    = 0x00001,
		ArgTexture    = 0x00002,
		ArgMask       = 0x0000F,
		ArgComplement = 0x00010
	};
	
	//! Texture wrapping/addressing mode
	enum WrapMode {
		WrapRepeat, //!< Tile the texture at every integer junction. For example, for u values between 0 and 3, the texture is repeated three times; no mirroring is performed.
		WrapMirror, //!< Similar to WrapRepeat, except that the texture is flipped at every integer junction.
		WrapClamp   //!< Texture coordinates outside the range [0.0, 1.0] are set to the texture color at 0.0 or 1.0, respectively.
	};
	
	//! Minification/Magnification/Mipmap filter
	enum FilterMode {
		FilterNone,    //!< Only valid for mip filtering.
		FilterNearest, //!< Point filtering. The texel with coordinates nearest to the desired pixel value is used. 
		FilterLinear   //!< Bilinear interpolation filtering. A weighted average of a 2×2 area of texels surrounding the desired pixel is used.
	};
	
	explicit TextureStage(unsigned int stage);
	virtual ~TextureStage() { }
	
	virtual void setTexture(Texture * pTexture) = 0;
	virtual void resetTexture() = 0;
	
	virtual void setColorOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) = 0;
	virtual void setColorOp(TextureOp textureOp) = 0;
	inline void setColorOp(TextureArg texArg);
	inline void disableColor();
	
	virtual void setAlphaOp(TextureOp textureOp, TextureArg texArg1, TextureArg texArg2) = 0;
	virtual void setAlphaOp(TextureOp textureOp) = 0;
	inline void setAlphaOp(TextureArg texArg);
	inline void disableAlpha();
	
	virtual void setWrapMode(WrapMode wrapMode) = 0;
	
	virtual void setMinFilter(FilterMode filterMode) = 0;
	virtual void setMagFilter(FilterMode filterMode) = 0;
	virtual void setMipFilter(FilterMode filterMode) = 0;
	
	//! Level of detail bias for mipmaps. Can be used to make textures appear more chunky or more blurred.
	/* Each unit bias (+/-1.0) biases the selection by exactly one MIP map level. 
	 * A negative bias causes the use of larger MIP map levels, resulting in a sharper but more aliased image. 
	 * A positive bias causes the use of smaller MIP map levels, resulting in a blurrier image. 
	 * Applying a negative bias also results in the referencing of a smaller amount of texture data, which can boost performance on some systems.
	 */
	virtual void setMipMapLODBias(float bias) = 0;
	
protected:
	
	unsigned int mStage;
	
};


inline void TextureStage::setColorOp(TextureArg texArg) {
	setColorOp(OpSelectArg1, texArg, ArgCurrent);
}

inline void TextureStage::disableColor() {
	setColorOp(OpDisable, ArgCurrent, ArgCurrent);
}

inline void TextureStage::setAlphaOp(TextureArg texArg) {
	setAlphaOp(OpSelectArg1, texArg, ArgCurrent);
}

inline void TextureStage::disableAlpha() {
	setAlphaOp(OpDisable, ArgCurrent, ArgCurrent);
}

#endif // ARX_GRAPHICS_TEXTURE_TEXTURESTAGE_H
