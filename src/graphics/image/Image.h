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

#ifndef ARX_GRAPHICS_IMAGE_IMAGE_H
#define ARX_GRAPHICS_IMAGE_IMAGE_H

#include "graphics/Color.h"

namespace fs { class path; }

class Image {
	
public:
	
	enum Format {
		Format_L8,
		Format_A8,
		Format_L8A8, 
		Format_R8G8B8,
		Format_B8G8R8,
		Format_R8G8B8A8,
		Format_B8G8R8A8,
		Format_DXT1,
		Format_DXT3,
		Format_DXT5,
		Format_Unknown,
		Format_Num,
		Format_MAX = 0xFF
	};
	
	Image();
	Image(const Image & pOther);
	virtual ~Image();
	
	const Image& operator=(const Image & pOther);
	
	bool LoadFromFile(const fs::path & filename);
	bool LoadFromMemory(void * pData, unsigned int size);
	
	void Create(unsigned int width, unsigned int height, Format format, unsigned int numMipmaps = 1, unsigned int depth = 1);
	
	void Clear();
	void Reset();
	
	bool IsValid() const { return mData != NULL; }
	unsigned int GetWidth() const { return mWidth;      }
	unsigned int GetHeight() const { return mHeight;     }
	unsigned int GetDepth() const { return mDepth;      }
	unsigned int GetNumMipmaps() const { return mNumMipmaps; }
	Format GetFormat() const { return mFormat;     }
	unsigned int GetDataSize() const { return mDataSize;   }
	
	unsigned int  GetNumChannels() const { return Image::GetNumChannels( mFormat ); }  
	bool IsCompressed() const { return Image::IsCompressed( mFormat ); }
	bool IsVolume() const { return mDepth > 1;  }
	bool HasAlpha() const { return mFormat == Format_A8 || mFormat == Format_L8A8 || mFormat == Format_R8G8B8A8 || mFormat == Format_B8G8R8A8 || mFormat == Format_DXT3 || mFormat == Format_DXT5; }
	
	//! Access to internal data.
	inline const unsigned char * GetData() const { return mData; }
	inline unsigned char * GetData() { return mData; }
	
	void FlipY();
	
	//! Copy an image into this image's buffer.
	//! Works only with uncompressed formats
	bool Copy(const Image & srcImage, unsigned int dstX, unsigned int dstY);
	bool Copy(const Image & srcImage, unsigned int dstX, unsigned int dstY, unsigned int srcX, unsigned int srcY, unsigned int width, unsigned int height);
	
	void ChangeGamma(float pGamma);

	// Set the alpha of pixels matching the color key to 0. Will add an alpha channel if needed.
	void ApplyColorKeyToAlpha(const Color3f& colorKey = Color3f::black);
	
	bool ToGrayscale();
	bool ToNormalMap();
	
	void save(const fs::path & filename) const;
	
	static unsigned int	GetSize(Format pFormat, unsigned int pWidth = 1, unsigned int pHeight = 1, unsigned int pDepth = 1);
	static unsigned int	GetSizeWithMipmaps(Format pFormat, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth = 1, int pMipmapCount = -1);
	static unsigned int	GetNumChannels(Format pFormat);
	static bool IsCompressed(Format pFormat);
	
private:
	
	void FlipY(unsigned char* pData, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth);
	
	unsigned int mWidth; //!< Image width.
	unsigned int mHeight; //!< Image height.
	unsigned int mDepth; //!< Image depth. Depth == 1 for normal images, > 1 for depth images.
	
	unsigned int mNumMipmaps; //!< Number of mipmaps.
	Format mFormat; //!< Image format.
	
	unsigned char* mData; //!< Pointer to image data buffer.
	unsigned int mDataSize; //!< Size of image buffer.
	
};

#endif // ARX_GRAPHICS_IMAGE_IMAGE_H
