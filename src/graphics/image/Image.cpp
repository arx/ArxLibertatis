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

#include "graphics/image/Image.h"

#include <cstring>
#include <il.h>

#include "graphics/Math.h"
#include "io/PakReader.h"

using std::string;
using std::memcpy;
using std::memset;

namespace {

class DevilLib {
	
public:
	
	DevilLib() {
		
		ilInit();
		
		// Set the origin to be used when loading all images, 
		// so that any image with a different origin will be
		// flipped to have the set origin
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
		ilEnable(IL_ORIGIN_SET);
		
	}
	
	~DevilLib() {
		ilShutDown();
	}
	
} gDevilLib;

const unsigned int SIZE_TABLE[Image::Format_Num] = {
	1,  // Format_L8,
	1,  // Format_A8,
	2,  // Format_L8A8,
	3,  // Format_R8G8B8,
	3,  // Format_B8G8R8,
	4,  // Format_R8G8B8A8,
	4,  // Format_B8G8R8A8,
	8,  // Format_DXT1,
	16, // Format_DXT3,
	16, // Format_DXT5,
	0,  // Format_Unknown
};

} // anonymous namespace


Image::Image() : mData(0) {
	Reset();
}

Image::Image(const Image & pOther) : mData(NULL) {
	*this = pOther;
}

Image::~Image() {
	if(mData) {
		delete[] mData;
	}
}

void Image::Reset() {
	
	if(mData) {
		delete[] mData, mData = NULL;
	}
	
	mWidth = 0;
	mHeight = 0;
	mDepth = 0;
	mNumMipmaps = 0;
	mFormat = Format_Unknown;
	mDataSize = 0;
}

const Image& Image::operator=(const Image & pOther) {
	
	// Ignore self copy!
	if(&pOther == this) {
		return *this;
	}
	
	if(mData) {
		delete[] mData;
	}
	
	mWidth      = pOther.mWidth;
	mHeight     = pOther.mHeight;
	mDepth      = pOther.mDepth;
	mNumMipmaps = pOther.mNumMipmaps;
	mFormat     = pOther.mFormat;
	mDataSize   = pOther.mDataSize;
	mData       = new unsigned char[mDataSize];
	
	memcpy( mData, pOther.mData, mDataSize );
	
	return *this;
}

unsigned int Image::GetSize(Image::Format pFormat, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth) {
	
	if(pWidth == 0) {
		pWidth = 1;
	}
	
	if(pHeight == 0) {
		pHeight = 1;
	}
	
	if(pFormat >= Format_DXT1 && pFormat <= Format_DXT5) {
		return ((pWidth+3) >> 2) * ((pHeight+3) >> 2) * SIZE_TABLE[pFormat];
	} else {
		return pWidth * pHeight * SIZE_TABLE[pFormat] * pDepth;
	}
}

unsigned int Image::GetSizeWithMipmaps(Image::Format pFormat, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth, int pMipmapCount) {
	
	unsigned int dataSize = 0;
	
	unsigned int width  = pWidth;
	unsigned int height = pHeight;
	unsigned int depth  = pDepth;
	unsigned int mip    = pMipmapCount == -1 ? 0x7FFFFFFF : pMipmapCount;
	
	while((width || height) && mip != 0) {
		
		dataSize += Image::GetSize( pFormat, width, height, depth );
		
		width  >>= 1;
		height >>= 1;
		
		if(depth != 1) {
			depth  >>= 1;
		}
		
		mip--;
	}
	
	return dataSize;
}

unsigned int Image::GetNumChannels(Image::Format pFormat) {
	
	switch(pFormat) {
		case Format_L8:       return 1;
		case Format_A8:       return 1;
		case Format_L8A8:     return 2;
		case Format_R8G8B8:   return 3;
		case Format_B8G8R8:   return 3;
		case Format_R8G8B8A8: return 4;
		case Format_B8G8R8A8: return 4;
		case Format_DXT1:     return 3;
		case Format_DXT3:     return 4;
		case Format_DXT5:     return 4;
		default:              arx_assert_msg(0, "Invalid image format"); return 0;
	}
}

bool Image::IsCompressed(Image::Format pFormat) {
	return pFormat >= Format_DXT1 && pFormat <= Format_DXT5;
}

bool Image::LoadFromFile(const fs::path & filename) {
	
	size_t size = 0;
	void * pData = resources->readAlloc(filename, size);
	
	if(!pData) {
		return false;
	}
	
	bool ret = LoadFromMemory(pData, size);
	
	free(pData);
	
	return ret;
}

Image::Format GetImageFormat(ILint pImgTextureFormat, ILint pBPP) {
	
	// Convert DevIL image format to our internal format.
	// TODO why test pBPP? shouldn't the IL format be enough?
	switch(pBPP) {
		
		case 1:
			return Image::Format_L8;
			
		case 2: {
			switch(pImgTextureFormat) {
				case IL_LUMINANCE_ALPHA:
					return Image::Format_L8A8;
			}
			break;
		}
		
		case 3: {
			switch(pImgTextureFormat) {
				case IL_RGB:
					return Image::Format_R8G8B8;
				case IL_BGR:
					return Image::Format_B8G8R8;
			}
			break;
		}
		
		case 4: {
			switch(pImgTextureFormat) {
				case IL_RGBA:
					return Image::Format_R8G8B8A8;
				case IL_BGRA:
					return Image::Format_B8G8R8A8;
			}
			break;
		}
	}
	
	return Image::Format_Unknown;
}

bool Image::LoadFromMemory(void * pData, unsigned int size) {
	
	if(!pData) {
		return false;
	}
	
	ILuint imageName;
	ilGenImages(1, &imageName);
	ilBindImage(imageName);
	
	ILboolean bLoaded = ilLoadL(IL_TYPE_UNKNOWN, pData, size);
	if(!bLoaded) {
		return false;
	}
	
	mWidth  = ilGetInteger(IL_IMAGE_WIDTH);
	mHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	mDepth  = 1;
	mNumMipmaps = 1;
	
	ILint imgFormat = ilGetInteger(IL_IMAGE_FORMAT);
	
	// We do not support palettized texture currently, so un-palettize them!
	if(imgFormat == IL_COLOR_INDEX) {
		switch(ilGetInteger(IL_PALETTE_TYPE)) {
			case IL_PAL_RGB24:
			case IL_PAL_RGB32:  imgFormat = IL_RGB; break;
			case IL_PAL_BGR24:
			case IL_PAL_BGR32:  imgFormat = IL_BGR; break;
			case IL_PAL_RGBA32: imgFormat = IL_RGBA; break;
			case IL_PAL_BGRA32: imgFormat = IL_BGRA; break;
			default: arx_assert_msg(0, "Invalid palette type");
		}
		ilConvertImage(imgFormat, IL_UNSIGNED_BYTE);
		imgFormat = ilGetInteger(IL_IMAGE_FORMAT);
	}
	
	ILint bytesPerPixel = ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
	
	mFormat = GetImageFormat(imgFormat, bytesPerPixel);
	unsigned int dataSize = Image::GetSizeWithMipmaps(mFormat, mWidth, mHeight, mDepth, mNumMipmaps);
	
	// Delete previous buffer if size don't match
	if(mData && mDataSize != dataSize) {
		delete[] mData, mData = NULL;
	}
	
	// Create a new buffer if needed
	if(!mData) {
		mData = new unsigned char[dataSize];
	}
	
	// Copy image data to our buffer
	if(mData) {
		mDataSize = dataSize;
		memcpy(mData, ilGetData(), mDataSize);
	}
	
	// Release resources
	ilDeleteImages( 1, &imageName );
	
	return (mData != NULL);
}

void Image::Create(unsigned int pWidth, unsigned int pHeight, Image::Format pFormat, unsigned int pNumMipmaps, unsigned int pDepth) {
	
	arx_assert_msg(pWidth > 0, "[Image::Create] Width is 0!");
	arx_assert_msg(pHeight > 0, "[Image::Create] Width is 0!");
	arx_assert_msg(pFormat < Format_Unknown, "[Image::Create] Unknown texture format!");
	arx_assert_msg(pNumMipmaps > 0, "[Image::Create] Mipmap count must at least be 1!");
	arx_assert_msg(pDepth > 0, "[Image::Create] Image depth must at least be 1!");
	
	mWidth  = pWidth;
	mHeight = pHeight;
	mDepth  = pDepth;
	mFormat = pFormat;
	mNumMipmaps = pNumMipmaps;
	
	unsigned int dataSize = Image::GetSizeWithMipmaps(mFormat, mWidth, mHeight, mDepth, mNumMipmaps);
	if(mData && dataSize != mDataSize) {
		delete[] mData, mData = NULL;
	}
	mDataSize = dataSize;
	if(!mData) {
		mData = new unsigned char[mDataSize];
	}
}

void Image::Clear() {
	memset(mData, 0, mDataSize);
}

bool Image::Copy(const Image & srcImage, unsigned int dstX, unsigned int dstY, unsigned int srcX, unsigned int srcY, unsigned int width, unsigned int height) {
	
	arx_assert_msg( !IsCompressed(), "[Image::Copy] Copy of compressed images not supported yet!" );
	arx_assert_msg( !IsVolume(), "[Image::Copy] Copy of volume images not supported yet!" );
	
	unsigned int bpp = SIZE_TABLE[mFormat];
	
	// Format must match.
	if(srcImage.GetFormat() != mFormat) {
		return false;
	}
	
	// Must fit inside boundaries
	if(dstX + width > mWidth || dstY + height > mHeight) {
		return false;
	}
	
	// Must fit inside boundaries
	if(srcX + width > srcImage.GetWidth() || srcY + height > srcImage.GetHeight()) {
		return false;
	}
	
	// Copy line by line
	unsigned char * dst = &mData[dstY * mWidth * bpp];
	const unsigned char * src = &srcImage.GetData()[srcY * srcImage.GetWidth() * bpp];
	for(unsigned int i = 0; i < height; i++) {
		
		// Copy line
		memcpy( &dst[dstX * bpp], &src[srcX * bpp], width * bpp );
		
		// Advance to next line
		dst += mWidth * bpp;
		src += srcImage.GetWidth() * bpp;
	}
	
	return true;
}

bool Image::Copy(const Image & srcImage, unsigned int destX, unsigned int destY) {
	return Copy(srcImage, destX, destY, 0, 0, srcImage.GetWidth(), srcImage.GetHeight());
}

void Image::ChangeGamma(float pGamma) {
	
	arx_assert_msg(!IsCompressed(), "[Image::ChangeGamma] Gamma change of compressed images not supported yet!");
	arx_assert_msg(!IsVolume(), "[Image::ChangeGamma] Gamma change of volume images not supported yet!");
	
	// This function was taken from a couple engines that I saw,
	// which most likely originated from the Aftershock engine.
	// Kudos to them!  What it does is increase/decrease the intensity
	// of the lightmap so that it isn't so dark.  Quake uses hardware to
	// do this, but we will do it in code.
	unsigned int numComponents = SIZE_TABLE[mFormat];
	unsigned int size = mWidth * mHeight;
	unsigned char * data = mData;
	
	const unsigned int MAX_COMPONENTS = 4;
	const float MAX_COMPONENT_VALUE = 255.0f;
	
	float components[MAX_COMPONENTS];
	
	// Nothing to do in this case!
	if(pGamma == 1.0f) {
		return;
	}
	
	// Go through every pixel in the image
	for(unsigned int i = 0; i < size; i++, data += numComponents) {
		
		float scale = 1.0f;
		float temp = 0.0f;
		
		for(unsigned int j = 0; j < numComponents; j++) {
			
			// Extract the current component value
			components[j] = (float)data[j];
			
			// Multiply the factor by the component value, while keeping it to a 255 ratio
			components[j] *= pGamma;
			
			// Check if the the value went past the highest value
			if(components[j] > MAX_COMPONENT_VALUE && (temp = (MAX_COMPONENT_VALUE/components[j])) < scale) {
				scale = temp;
			}
		}
		
		for(unsigned int j = 0; j < numComponents; j++) {
			
			// Get the scale for this pixel and multiply it by the component value
			components[j] *= scale;
			
			// Assign the new gamma'nized value to the image
			data[j] = (unsigned char)components[j];
		}
	}
}

void Image::ApplyColorKeyToAlpha(const Color3f& colorKey)
{
	arx_assert_msg(!IsCompressed(), "[Image::ApplyColorKeyToAlpha] Not supported yet for compressed textures!");
	arx_assert_msg(!IsVolume(), "[Image::ApplyColorKeyToAlpha] Not supported yet for 3d textures!");

	u8 c1 = Format_R8G8B8 ? colorKey.r : colorKey.b;
	u8 c2 = Format_R8G8B8 ? colorKey.g : colorKey.g;
	u8 c3 = Format_R8G8B8 ? colorKey.b : colorKey.r;

	// For RGB or BGR textures, first check if an alpha channel is really needed, then create it if it's the case
	if(mFormat == Format_R8G8B8 || mFormat == Format_B8G8R8)
	{
		bool needsAlphaChannel = false;
		
		// Check if we've got pixels matching the color key
		unsigned char* pImg = mData;
		for(unsigned int y = 0; y < mHeight; y++)
		{
			for(unsigned int x = 0; x < mWidth; x++)
			{
				if(pImg[0] == c1 && pImg[1] == c2 && pImg[2] == c3)
				{
					needsAlphaChannel = true;
					break;
				}

				pImg += 3;
			}

			if(needsAlphaChannel)
				break;
		}

		// If we need to add an alpha channel
		if(needsAlphaChannel)
		{
			// Create a temp buffer
			unsigned int dataSize = Image::GetSizeWithMipmaps(Format_R8G8B8A8, mWidth, mHeight, mDepth, mNumMipmaps);
			unsigned char* dataTemp = new unsigned char[dataSize];
		
			// Fill temp image and apply color key to alpha channel
			unsigned char* pImgDst = dataTemp;
			pImg = mData;
			for(unsigned int y = 0; y < mHeight; y++)
			{
				for(unsigned int x = 0; x < mWidth; x++)
				{
					pImgDst[0] = pImg[0];
					pImgDst[1] = pImg[1];
					pImgDst[2] = pImg[2];
					pImgDst[3] = pImg[0] == c1 && pImg[1] == c2 && pImg[2] == c3 ? 0 : 0xFF;
					
					pImg += 3;
					pImgDst += 4;
				}
			}

			// Swap data with temp data and ajust internal state
			delete[] mData;
			mData = dataTemp;
			mDataSize = dataSize;
			mFormat = mFormat == Format_R8G8B8 ? Format_R8G8B8A8 : Format_B8G8R8A8;
		}
	}
	// For RGBA or BGRA textures, simply remplace the alpha of pixels matching the color key with 0
	else if(mFormat == Format_R8G8B8A8 || mFormat == Format_B8G8R8A8)
	{
		unsigned char* pImg = mData;
		for(unsigned int y = 0; y < mHeight; y++)
		{
			for(unsigned int x = 0; x < mWidth; x++)
			{
				if(pImg[0] == c1 && pImg[1] == c2 && pImg[2] == c3)
					pImg[3] = 0;

				pImg += 3;
			}
		}
	}
	else
	{
		ARX_DEAD_CODE();
	}
}

bool Image::ToGrayscale() {
	
	int numChannels = GetNumChannels();
	
	if(IsCompressed() || numChannels < 3) {
		return false;
	}
	
	unsigned int size, len;
	unsigned char * src = mData;
	
	size = len = GetSizeWithMipmaps(Format_L8, mWidth, mHeight, mDepth, mNumMipmaps);
	unsigned char * dest = new unsigned char[size];
	
	do {
		*dest++ = (77 * src[0] + 151 * src[1] + 28 * src[2] + 128) >> 8;
		src += numChannels;
	} while (--len);
	
	delete[] mData;
	mData = dest;
	
	mFormat = Format_L8;
	
	return true;
}

bool Image::ToNormalMap() {
	
	if(mFormat != Format_L8) {
		return false;
	}
	
	unsigned char * row0, * row1, * row2;
	unsigned int x,y,w,h,mipmap;
	unsigned int sx, sy, len, predx, succx;
	
	mFormat = Format_R8G8B8;
	mDataSize = GetSizeWithMipmaps(mFormat, mWidth, mHeight, mDepth, mNumMipmaps);
	
	unsigned char * newPixels = new unsigned char[mDataSize];
	unsigned char * dest = newPixels;
	unsigned char * src = mData;
	
	mipmap = 0;
	w = mWidth;
	h = mHeight;
	
	do {
		
		row1 = src + (h - 1) * w;
		row2 = src;
		
		y = h;
		
		do {
			
			row0 = row1;
			row1 = row2;
			row2 += w;
			
			if(y == 1) {
				row2 = src;
			}
			
			x = w - 1;
			succx = 0;
			
			do {
				
				predx = x;
				x = succx++;
				
				if(succx == w) {
					succx = 0;
				}
				
				sx = (row0[predx] + 2 * row1[predx] + row2[predx]) - (row0[succx] + 2 * row1[succx] + row2[succx]);
				sy = (row0[predx] + 2 * row0[x] + row0[succx]) - (row2[predx] + 2 * row2[x] + row2[succx]);
				
				len = (unsigned int)(0x000FF000 * FastRSqrt(float(sx * sx + sy * sy + 256*256)));
				sx *= len;
				sy *= len;
				
				dest[0] = ((sx  + 0x000FF000) >> 13);
				dest[1] = ((sy  + 0x000FF000) >> 13);
				dest[2] = ((len + 0x00000FF0) >> 5);
				dest += 3;
				
			} while(succx);
		} while(--y);
		
		src += w * h;
		
		if(w > 1) {
			w >>= 1;
		}
		
		if(h > 1) {
			h >>= 1;
		}
		
		mipmap++;
	} while(mipmap < mNumMipmaps);
	
	delete[] mData;
	mData = newPixels;
	
	return true;
}

void Image::FlipY() {
	
	unsigned int width  = mWidth;
	unsigned int height = mHeight;
	unsigned int depth  = mDepth;
	
	unsigned int offset = 0;
	
	for(unsigned int i = 0; i < mNumMipmaps && (width || height); i++) {
		
		if(width == 0) {
			width = 1;
		}
		
		if(height == 0) {
			height = 1;
		}
		
		if(depth == 0) {
			depth = 1;
		}
		
		FlipY(mData + offset, width, height, depth);
		offset += Image::GetSize(mFormat, width, height, depth);
		
		width >>= 1;
		height >>= 1;
		depth >>= 1;
	}
}

struct DXTColBlock {
	u16 mCol0;
	u16 mCol1;
	unsigned char	mRow[4];
};

struct DXT3AlphaBlock {
	u16 mRow[4];
};

struct DXT5AlphaBlock {
	unsigned char mAlpha0;
	unsigned char mAlpha1;
	unsigned char mRow[6];
};

void FlipDXT1(unsigned char * data, unsigned int count);
void FlipDXT3(unsigned char * data, unsigned int count);
void FlipDXT5(unsigned char * data, unsigned int count);

void Image::FlipY(unsigned char * pData, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth) {
	
	unsigned int offset;
	
	if(!IsCompressed()) {
		
		unsigned int imageSize = GetSize(mFormat, pWidth, pHeight);
		unsigned int lineSize = imageSize / pHeight;
		
		unsigned char * swapTmp = (unsigned char *)malloc(lineSize);
		arx_assert(swapTmp);
		
		for(unsigned int n = 0; n < pDepth; n++) {
			
			offset = imageSize * n;
			
			unsigned char * top = pData + offset;
			unsigned char * bottom = top + (imageSize-lineSize);
			
			for(unsigned int i = 0; i < (pHeight >> 1); i++) {
				
				memcpy( swapTmp, bottom, lineSize );
				memcpy( bottom, top, lineSize );
				memcpy( top, swapTmp, lineSize );
				
				top += lineSize;
				bottom -= lineSize;
			}
		}
		
		free(swapTmp);
		
	} else {
		
		void (*flipDXTn)(unsigned char *, unsigned int) = NULL;
		
		unsigned int xBlocks = (pWidth+3) / 4;
		unsigned int yBlocks = (pHeight+3) / 4;
		unsigned int blockSize = SIZE_TABLE[mFormat];
		unsigned int lineSize = xBlocks * blockSize;
		
		unsigned char * swapTmp = (unsigned char *)malloc(lineSize);
		arx_assert(swapTmp);
		
		DXTColBlock * top = NULL;
		DXTColBlock * bottom = NULL;
		
		switch(mFormat) {
			
			case Format_DXT1:
				flipDXTn = &FlipDXT1;
				break;
			
			case Format_DXT3:
				flipDXTn = &FlipDXT3;
				break;
			
			case Format_DXT5:
				flipDXTn = &FlipDXT5;
				break;
			
			default:
				arx_assert(flipDXTn);
		}
		
		for(unsigned int j = 0; j < (yBlocks >> 1); j++) {
			
			top = (DXTColBlock*)(pData + j * lineSize);
			bottom = (DXTColBlock*)(pData + (((yBlocks-j)-1) * lineSize));
			
			(*flipDXTn)((unsigned char *)top, xBlocks);
			(*flipDXTn)((unsigned char *)bottom, xBlocks);
			
			memcpy(swapTmp, bottom, lineSize);
			memcpy(bottom, top, lineSize);
			memcpy(top, swapTmp, lineSize);
		}
		
		free(swapTmp);
	}
}


void FlipColorBlock(unsigned char * data) {
	
	unsigned char tmp;
	
	tmp = data[4];
	data[4] = data[7];
	data[7] = tmp;
	
	tmp = data[5];
	data[5] = data[6];
	data[6] = tmp;
}

void FlipSimpleAlphaBlock(u16 * data) {
	
	u16 tmp;
	
	tmp = data[0];
	data[0] = data[3];
	data[3] = tmp;
	
	tmp = data[1];
	data[1] = data[2];
	data[2] = tmp;
}

void ComplexAlphaHelper(unsigned char * Data) {
	
	u16 tmp[2];

	// One 4 pixel line is 12 bit, copy each line into
	// a ushort, swap them and copy back
	tmp[0] = (Data[0] | (Data[1] << 8)) & 0xfff;
	tmp[1] = ((Data[1] >> 4) | (Data[2] << 4)) & 0xfff;
	
	Data[0] = tmp[1];
	Data[1] = (tmp[1] >> 8) | (tmp[0] << 4);
	Data[2] = tmp[0] >> 4;
}

void FlipComplexAlphaBlock(unsigned char * Data) {
	
	unsigned char tmp[3];
	Data += 2; // Skip 'palette'
	
	// Swap upper two rows with lower two rows
	memcpy(tmp, Data, 3);
	memcpy(Data, Data + 3, 3);
	memcpy(Data + 3, tmp, 3);
	
	// Swap 1st with 2nd row, 3rd with 4th
	ComplexAlphaHelper(Data);
	ComplexAlphaHelper(Data + 3);
}

void FlipDXT1(unsigned char * data, unsigned int count) {
	for(unsigned int i = 0; i < count; ++i) {
		FlipColorBlock(data);
		data += 8; // Advance to next block
	}
}

void FlipDXT3(unsigned char * data, unsigned int count) {
	for(unsigned int i = 0; i < count; ++i) {
		FlipSimpleAlphaBlock((u16*)data);
		FlipColorBlock(data + 8);
		data += 16; // Advance to next block
	}
}

void FlipDXT5(unsigned char * data, unsigned int count) {
	for(unsigned int i = 0; i < count; ++i) {
		FlipComplexAlphaBlock(data);
		FlipColorBlock(data + 8);
		data += 16; // Advance to next block
	}
}

void Flip3dc(unsigned char * data, unsigned int count) {
	for(unsigned int i = 0; i < count; ++i) {
		FlipComplexAlphaBlock(data);
		FlipComplexAlphaBlock(data + 8);
		data += 16; // Advance to next block
	}
}

ILenum ARXImageToILFormat[] = {
	IL_LUMINANCE,       // Format_L8
	IL_ALPHA,           // Format_A8
	IL_LUMINANCE_ALPHA, // Format_L8A8
	IL_RGB,             // Format_R8G8B8
	IL_BGR,             // Format_B8G8R8
	IL_RGBA,            // Format_R8G8B8A8
	IL_BGRA,            // Format_B8G8R8A8
	IL_DXT1,            // Format_DXT1
	IL_DXT3,            // Format_DXT3
	IL_DXT5,            // Format_DXT5
};

void Image::save(const fs::path & filename) const {
	
	ILuint imageName;
	ilGenImages(1, &imageName);
	ilBindImage(imageName);
	
	BOOST_STATIC_ASSERT(ARRAY_SIZE(ARXImageToILFormat) == Format_Unknown);
	if(mFormat < 0 || mFormat >= Format_Unknown) {
		return;
	}
	
	ILboolean ret = ilTexImage(mWidth, mHeight, mDepth, GetNumChannels(), ARXImageToILFormat[mFormat], IL_UNSIGNED_BYTE, mData);
	if(ret) {
		ilRegisterOrigin(IL_ORIGIN_UPPER_LEFT);
		ilEnable(IL_FILE_OVERWRITE);
		ret = ilSaveImage(filename.string().c_str());
		arx_assert_msg(ret, "ilSaveImage failed: %d", ilGetError());
	}
	
	ilDeleteImages(1, &imageName);
}
