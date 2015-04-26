/*
 * Copyright 2011-2014 Arx Libertatis Team (see the AUTHORS file)
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

inline static bool sample(const u8 * src, int w, int h, int x, int y, u8 * dst, Color key) {
	if(x >= 0 && x < w && y >= 0 && y < h) {
		const u8 * s = src + (y * w + x) * 3;
		if(s[0] != key.r || s[1] != key.g || s[2] != key.b) {
			dst[0] = s[0], dst[1] = s[1], dst[2] = s[2];
			return true;
		}
	}
	return false;
}

void Image::ApplyColorKeyToAlpha(Color key) {
	
	arx_assert(!IsCompressed(), "ApplyColorKeyToAlpha Not supported for compressed textures!");
	arx_assert(!IsVolume(), "ApplyColorKeyToAlpha Not supported for 3d textures!");
	
	if(mFormat != Format_R8G8B8 && mFormat != Format_B8G8R8) {
		arx_assert(false, "ApplyColorKeyToAlpha not supported for format %d", mFormat);
		return;
	}
	
	if(mFormat == Format_B8G8R8) {
		std::swap(key.r, key.b);
	}
	
	// For RGB or BGR textures, first check if an alpha channel is really needed,
	// then create it if it's the case
	
	// Check if we've got pixels matching the color key
	const u8 * img = mData;
	bool needsAlphaChannel = false;
	for(size_t i = 0; i < (mHeight * mHeight); i++, img += 3) {
		if(img[0] == key.r && img[1] == key.g && img[2] == key.b) {
			needsAlphaChannel = true;
			break;
		}
	}
	if(!needsAlphaChannel) {
		return;
	}
	
	// If we need to add an alpha channel
	
	// Create a temp buffer
	size_t dataSize = GetSizeWithMipmaps(Format_R8G8B8A8, mWidth, mHeight, mDepth, mNumMipmaps);
	u8 * dataTemp = new unsigned char[dataSize];
	
	// Fill temp image and apply color key to alpha channel
	u8 * dst = dataTemp;
	img = mData;
	for(size_t y = 0; y < mHeight; y++) {
		for(size_t x = 0; x < mWidth; x++) {
			
			dst[3] = (img[0] == key.r && img[1] == key.g && img[2] == key.b) ? 0 : 0xff;
			
			if(dst[3]) {
				
				dst[0] = img[0];
				dst[1] = img[1];
				dst[2] = img[2];
				
			} else {
				// For transparent pixels, use the color of an opaque bordering pixel,
				// so that linear filtering won't produce black borders.
				if(   !sample(mData, mWidth, mHeight, int(x)    , int(y) - 1, dst, key)
				   && !sample(mData, mWidth, mHeight, int(x) + 1, int(y)    , dst, key)
				   && !sample(mData, mWidth, mHeight, int(x)    , int(y) + 1, dst, key)
				   && !sample(mData, mWidth, mHeight, int(x) - 1, int(y)    , dst, key)
				   && !sample(mData, mWidth, mHeight, int(x) - 1, int(y) - 1, dst, key)
				   && !sample(mData, mWidth, mHeight, int(x) + 1, int(y) - 1, dst, key)
				   && !sample(mData, mWidth, mHeight, int(x) + 1, int(y) + 1, dst, key)
				   && !sample(mData, mWidth, mHeight, int(x) - 1, int(y) + 1, dst, key)) {
					dst[0] = dst[1] = dst[2] = 0;
				}
			}
			
			img += 3;
			dst += 4;
		}
	}
	
	// Swap data with temp data and ajust internal state
	delete[] mData;
	mData = dataTemp;
	mDataSize = dataSize;
	mFormat = (mFormat == Format_R8G8B8) ? Format_R8G8B8A8 : Format_B8G8R8A8;
}
