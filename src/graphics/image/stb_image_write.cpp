/* Copyright: stbiw-0.92 - public domain - http://nothings.org/stb/stb_image_write.h
   writes out PNG/BMP/TGA images to C stdio - Sean Barrett 2010
                            no warranty implied; use at your own risk
*/

#include "graphics/image/stb_image_write.h"

#include <stdio.h>

#include "platform/Platform.h"

namespace stbi {

static int write2(FILE * f, int x) {
	unsigned char b[2] = { (unsigned char)x, (unsigned char)(x >> 8) };
	return fwrite(b, 2, 1, f);
}

static int write3(FILE * f, unsigned char a, unsigned char b, unsigned char c) {
	unsigned char arr[3];
	arr[0] = a, arr[1] = b, arr[2] = c;
	return fwrite(arr, 3, 1, f);
}

static int write4(FILE * f, u32 x) {
	unsigned char b[4] = { (unsigned char)x, (unsigned char)(x >> 8),
	                       (unsigned char)(x >> 16), (unsigned char)(x >> 24) };
	return fwrite(b, 4, 1, f);
}

static int write_pixels(FILE * f, int x, int y, int comp, const void * data, int scanline_pad) {
	
	if(y <= 0) {
		return 1;
	}
	
	for(int j = y - 1; j != -1; j--) {
		
		for(int i = 0; i < x; i++) {
			const unsigned char * d = (const unsigned char *)data + (j * x + i) * comp;
			
			switch(comp) {
				
				case 1:
				case 2: {
					if(!write3(f, d[0], d[0], d[0])) {
						return 0;
					}
					break;
				}
				
				case 3: {
					if(!write3(f, d[2], d[1], d[0])) {
						return 0;
					}
					break;
				}
				
				case 4: {
					// Composite against pink background
					const unsigned char bg[3] = { 255, 0, 255 };
					unsigned char px[3];
					for(int k = 0; k < 3; ++k) {
						px[k] = bg[k] + ((d[k] - bg[k]) * d[3]) / 255;
					}
					if(!write3(f, px[2], px[1], px[0])) {
						return 0;
					}
					break;
				}
				
			}
			
		}
		
		u32 zero = 0;
		if(scanline_pad > 0 && !fwrite(&zero, scanline_pad, 1, f)) {
			return 0;
		}
		
	}
	
	return 1;
}

int stbi_write_bmp(char const * filename, int x, int y, int comp, const void * data) {
	
	if(y < 0 || x < 0) {
		return 0;
	}
	
	FILE * f = fopen(filename, "wb");
	if(!f) {
		return 0;
	}
	
	// File header
	fputc('B', f);
	fputc('M', f);
	int pad = (-x * 3) & 3;
	write4(f, u32(14 + 40 + (x * 3 + pad) * y));
	write2(f, 0);
	write2(f, 0);
	write4(f, 14 + 40);
	
	// Bitmap header
	write4(f, 40);
	write4(f, x);
	write4(f, y);
	write2(f, 1);
	write2(f, 24);
	write4(f, 0);
	write4(f, 0);
	write4(f, 0);
	write4(f, 0);
	write4(f, 0);
	write4(f, 0);
	
	int ret = write_pixels(f, x, y, comp, data, pad);
	
	fclose(f);
	
	return ret;
}

} // namespace stbi
