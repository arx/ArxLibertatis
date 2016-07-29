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

/*!
 * Morphological antialisaing algorithm for black-white channels.
 *
 * All initial pixels with color > 127 are considered covered, while pixels <= 127
 * are considered transparent. This algorithm discards any existing anti-alising
 * information and only distinguishes between "covered" and "transparent" pixels.
 * It should therefore only be applied to source images/channels with two colors,
 * such as the alpha channel of color-keyed images.
 *
 * Guarantees:
 * - Previously "covered" pixels will receive a value > 127.
 * - Previously "transparent" pixels will receive a value <= 127.
 * - Pixels whose "covered" state is the same as all their neigboring pixels are unchanged.
 * - Hard corners are preserved.
 * - If initially all pixels next to pixels with value > 0 have a valid color (the value
 *   of the remaining channels), the algorithm will ensure that remains true.
 *
 * \tparam Channels Number of color channels in the image.
 * \tparam Channel  Index of the color channel to antialias.
 */
template <size_t Channels, size_t Channel>
class BlackWhiteAntialiaser {
	
private:
	
	//! Test if pixel (x, y) is "covered".
	static bool sample(u8 * d, size_t w, size_t h, int x, int y) {
		
		if(x < 0 || y < 0 || size_t(x) >= w || size_t(y) >= h) {
			return false; // (x + ix, y + iy) is out of bounds → assume "transparent".
		}
		
		const size_t sx = Channels;
		size_t sy = w * sx;
		
		return d[x * sx + y * sy] > 127;
	}
	
	//! Ensure pixel (x + ix, y + iy) has a valid color. Copy from p == (x, y) if needed.
	static void newPixel(u8 * p, int x, int y, int ix, int iy, size_t w, size_t h) {
		
		if(Channels == 1) {
			return; // No color to copy.
		}
		
		int nx = x + ix;
		int ny = y + iy;
		if(nx < 0 || ny < 0 || size_t(nx) >= w || size_t(ny) >= h) {
			return; // (x + ix, y + iy) is out of bounds.
		}
		
		const int sx = Channels;
		int sy = w * sx;
		
		u8 * n = p + sx * ix + sy * iy;
		if(*n == 0) {
			// For transparent pixels, use the color of an opaque bordering pixel,
			// so that linear filtering won't produce black borders.
			// TODO with premultiplied alpha this wouldn't be needed
			for(size_t c = 0; c < Channels; c++) {
				if(c != Channel) {
					int offset = int(c) - int(Channel);
					n[offset] = p[offset];
				}
			}
		}
		
	}
	
	/*!
	 * Process corner pixels.
	 *
	 * ???
	 * ?0X
	 * ?XX
	 */
	static void processCorner(u8 * d, size_t w, size_t h, size_t x, size_t y, bool filled) {
		
		const size_t sx = Channels;
		size_t sy = w * sx;
		
		// Find the pixel "!" that was different from the other three
		int ox = 0, oy = 0;
		for(size_t dy = 0; dy < 2; dy++) {
			for(size_t dx = 0; dx < 2; dx++) {
				u8 alpha = *(d + sx * (x + dx) + sy * (y + dy));
				if((alpha > 127) == filled) {
					ox = dx, oy = dy;
				}
			}
		}
		x += ox, y += oy;
		u8 * p = d + sx * x + sy * y;
		
		// 2. Classification
		// Look at surrounding pixels to guess the intended shape
		
		int ix = (ox ? 1 : -1), iy = (oy ? 1 : -1); // Direction from ! to untested pixels
		
		bool x0 = (sample(d, w, h, x + ix, y) == filled);
		bool y0 = (sample(d, w, h, x, y + iy) == filled);
		if(!x0 || !y0) {
			// ?X?
			// 0!X
			// ?XX
			//
			// ?X?
			// X!X
			// ?XX
			return; // ignore weird cases
		}
		
		bool x1 = (sample(d, w, h, x + ix, y - iy) == filled);
		bool y1 = (sample(d, w, h, x - ix, y + iy) == filled);
		if(!x1 && !y1) {
			// ?oX
			// o!X
			// XXX
			return; // leave hard corners as-is
		}
		
		if(x1 && y1) {
			
			// Optimized case for diagonal edges
			
			// ?oo
			// o!X
			// oXX
			// assume 1:1 slope
			u8 area = u8(0.5f * 0.5f / 2 * 255);
			*p = (filled ? 255 - area : area);
			
			if(!filled) {
				newPixel(p, x, y, ix, iy, w, h);
			}
			
			return;
		}
		
		// ?..
		// ?oX
		// o!X
		// oXX
		
		// 3. Walk edge until we find the next corner
		
		unsigned n = 2;
		
		int dx = (x1 ? 0 : ix), dy = (x1 ? iy : 0); // Edge direction, away from !
		
		for(int wx = x + dx + dx, wy = y + dy + dy; ; wx += dx, wy += dy) {
			if(sample(d, w, h, wx, wy) != filled) {
				n /= 2;
				break;
			}
			if(sample(d, w, h, wx + dx - ix, wy + dy - iy) == filled) {
				break;
			}
			n++;
		}
		
		u8 area = u8(n * 0.5f * 0.5f / 2 * 255);
		for(unsigned i = 0; i < n; i += 2) {
			
			u8 remaining = u8((n - i - 2) * 0.5f * float(n - i - 2) / n * 0.5f / 2 * 255);
			u8 current = area - remaining;
			if(current > 127) {
				// ensure (pixel > 127) remains constant
				current = 127;
			}
			
			*p = (filled ? 255 - current : current);
			
			if(!filled) {
				newPixel(p, x, y, ix, iy, w, h);
			}
			
			p += sx * dx + sy * dy;
			area = remaining;
		}
		
	}
	
public:
	
	static void process(u8 * data, size_t w, size_t h) {
		
		data += Channel;
		
		const size_t sx = Channels;
		size_t sy = w * sx;
		
		for(size_t y = 0; y < h - 1; y++) {
			for(size_t x = 0; x < w - 1; x++) {
				u8 * d = data + sx * x + sy * y;
				
				// 1. Corner detection
				// Find 2x2 pixel regions where one pixel is different from the others
				
				unsigned coverage = 0;
				for(size_t dy = 0; dy < 2; dy++) {
					for(size_t dx = 0; dx < 2; dx++) {
						u8 alpha = *(d + sx * dx + sy * dy);
						coverage += (alpha > 127);
					}
				}
				
				if(coverage == 3) {
					
					// 0X
					// XX
					
					processCorner(data, w, h, x, y, false);
					
				} else if(coverage == 1) {
					
					// X0
					// 00
				
					processCorner(data, w, h, x, y, true);
					
				} else if(coverage == 2 && *d == *(d + sx + sy)) {
					
					// X0
					// 0X
					
					// assume 45° line
					
					bool filled = (*d > 127);
					
					u8 area = u8(0.5f * 0.5f / 2 * 255);
					*d = *(d + sx + sy) = (filled ? 255 - area : area);
					*(d + sx) = *(d + sy) = (filled ? area : 255 - area);
					
					if(!filled) {
						newPixel(d, x, y, -1, -1, w, h);
						newPixel(d + sx + sy, x + 1, y + 1, 1, 1, w, h);
					} else {
						newPixel(d + sx, x + 1, y, 1, -1, w, h);
						newPixel(d + sy, x, y + 1, -1, 1, w, h);
					}
					
				} else {
					
					// X0
					// X0
					
					// 00
					// 00
					
					// XX
					// XX
					
					// no anti-aliasing needed
					
				}
				
			}
		}
		
	}
	
};

static bool sampleColorKey(const u8 * src, int w, int h, int x, int y, u8 * dst, Color key) {
	if(x >= 0 && x < w && y >= 0 && y < h) {
		const u8 * s = src + (y * w + x) * 3;
		if(s[0] != key.r || s[1] != key.g || s[2] != key.b) {
			dst[0] = s[0], dst[1] = s[1], dst[2] = s[2];
			return true;
		}
	}
	return false;
}

void Image::ApplyColorKeyToAlpha(Color key, bool antialias) {
	
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
				// TODO with premultiplied alpha this wouldn't be needed
				if(   !sampleColorKey(mData, mWidth, mHeight, int(x)    , int(y) - 1, dst, key)
				   && !sampleColorKey(mData, mWidth, mHeight, int(x) + 1, int(y)    , dst, key)
				   && !sampleColorKey(mData, mWidth, mHeight, int(x)    , int(y) + 1, dst, key)
				   && !sampleColorKey(mData, mWidth, mHeight, int(x) - 1, int(y)    , dst, key)
				   && !sampleColorKey(mData, mWidth, mHeight, int(x) - 1, int(y) - 1, dst, key)
				   && !sampleColorKey(mData, mWidth, mHeight, int(x) + 1, int(y) - 1, dst, key)
				   && !sampleColorKey(mData, mWidth, mHeight, int(x) + 1, int(y) + 1, dst, key)
				   && !sampleColorKey(mData, mWidth, mHeight, int(x) - 1, int(y) + 1, dst, key)) {
					dst[0] = dst[1] = dst[2] = 0;
				}
			}
			
			img += 3;
			dst += 4;
		}
	}
	
	if(antialias && mWidth > 1 && mHeight > 1) {
		BlackWhiteAntialiaser<4, 3 /* alpha */>::process(dataTemp, mWidth, mHeight);
	}
	
	// Swap data with temp data and ajust internal state
	delete[] mData;
	mData = dataTemp;
	mDataSize = dataSize;
	mFormat = (mFormat == Format_R8G8B8) ? Format_R8G8B8A8 : Format_B8G8R8A8;
}
