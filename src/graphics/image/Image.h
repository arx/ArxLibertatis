/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
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

#include <ostream>

#include "graphics/Color.h"

namespace fs { class path; }
namespace res { class path; }

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
		Format_Unknown,
		Format_Num,
	};
	
	Image();
	Image(const Image & pOther);
	virtual ~Image();
	
	const Image& operator=(const Image & pOther);
	
	bool LoadFromFile(const res::path & filename);
	bool LoadFromMemory(void * pData, unsigned int size,
	                    const char * file = NULL);
	
	void Create(unsigned int width, unsigned int height, Format format, unsigned int numMipmaps = 1);
	
	// Convert 
	bool ConvertTo(Format format);
	
	// reset to fresh constructor state
	void Reset();
	
	// zero image data with memset
	void Clear();
	
	// info accessors
	unsigned int GetWidth() const { return mWidth;      }
	unsigned int GetHeight() const { return mHeight;     }
	unsigned int GetNumMipmaps() const { return mNumMipmaps; }
	Format GetFormat() const { return mFormat;     }
	unsigned int GetDataSize() const { return mDataSize;   }
	unsigned int  GetNumChannels() const { return Image::GetNumChannels( mFormat ); }  
	
	// bool accessors
	bool IsValid() const { return mData != NULL; }
	bool HasAlpha() const {
		return mFormat == Format_A8
		    || mFormat == Format_L8A8
		    || mFormat == Format_R8G8B8A8
		    || mFormat == Format_B8G8R8A8;
	}
	
	//! Access to internal data.
	const unsigned char * GetData() const { return mData; }
	unsigned char * GetData() { return mData; }
	
	// conversions
	
	void FlipY();
	bool ToGrayscale(Format newFormat = Format_L8);
	
	void ResizeFrom(const Image &source, unsigned int width, unsigned int height, bool flip_vertical = false);
	
	//! Set the alpha of pixels matching the color key to 0. Will add an alpha channel if needed.
	void ApplyColorKeyToAlpha(Color colorKey = Color::black, bool antialias = false);
	
	/*!
	 * Extend the image and fill ne new space by sampling at the nearest border
	 * of the original image.
	 */
	void extendClampToEdgeBorder(const Image & srcImage);
	
	/*!
	 * \brief Copy an image into this image's buffer
	 *
	 * Works only with uncompressed formats
	 */
	bool Copy(const Image & srcImage, unsigned int dstX, unsigned int dstY);
	bool Copy(const Image & srcImage, unsigned int dstX, unsigned int dstY, unsigned int srcX, unsigned int srcY, unsigned int width, unsigned int height);
	
	bool save(const fs::path & filename) const;
	
	// processing functions
	// destructively adjust image content
	
	//! blur using gaussian kernel
	void Blur(int radius);
	
	//! scales value and normalizes by max component value
	void QuakeGamma(float pGamma);
	//! Copy the alpha of img to this image.
	void SetAlpha(const Image& img, bool bInvertAlpha);
	
	void AdjustGamma(const float &v);
	void AdjustBrightness(const float &v);
	void AdjustContrast(const float &v);
	
	void ApplyThreshold(unsigned char threshold, int component_mask);
	
	// statics
	static unsigned int	GetSize(Format pFormat, unsigned int pWidth = 1, unsigned int pHeight = 1);
	static unsigned int	GetSizeWithMipmaps(Format pFormat, unsigned int pWidth, unsigned int pHeight, int pMipmapCount = -1);
	static unsigned int	GetNumChannels(Format pFormat);
	
private:
	
	void FlipY(unsigned char* pData, unsigned int pWidth, unsigned int pHeight);
	
	unsigned int mWidth;
	unsigned int mHeight;
	
	unsigned int mNumMipmaps; //!< Number of mipmaps.
	Format mFormat; //!< Image format.
	
	unsigned char* mData; //!< Pointer to image data buffer.
	unsigned int mDataSize; //!< Size of image buffer.
	
};

std::ostream & operator<<(std::ostream & os, Image::Format format);

#endif // ARX_GRAPHICS_IMAGE_IMAGE_H
