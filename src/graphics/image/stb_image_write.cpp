/* Copyright: stbiw-0.92 - public domain - http://nothings.org/stb/stb_image_write.h
   writes out PNG/BMP/TGA images to C stdio - Sean Barrett 2010
                            no warranty implied; use at your own risk
*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "graphics/image/stb_image_write.h"

namespace stbi {

typedef unsigned int stbiw_uint32;
typedef int stb_image_write_test[sizeof(stbiw_uint32)==4 ? 1 : -1];

static int writefv(FILE *f, const char *fmt, va_list v)
{
   while (*fmt) {
      switch (*fmt++) {
         case ' ': break;
         case '1': { unsigned char x = (unsigned char) va_arg(v, int); fputc(x,f); break; }
         case '2': { int x = va_arg(v,int); unsigned char b[2];
                     b[0] = (unsigned char) x; b[1] = (unsigned char) (x>>8);
                     if(!fwrite(b,2,1,f)) { return 0; } break; }
         case '4': { stbiw_uint32 x = va_arg(v,int); unsigned char b[4];
                     b[0]=(unsigned char)x; b[1]=(unsigned char)(x>>8);
                     b[2]=(unsigned char)(x>>16); b[3]=(unsigned char)(x>>24);
                     if(!fwrite(b,4,1,f)) { return 0; } break; }
         default:
            assert(0);
            return 1;
      }
   }
   return 1;
}

static int write3(FILE *f, unsigned char a, unsigned char b, unsigned char c)
{
   unsigned char arr[3];
   arr[0] = a, arr[1] = b, arr[2] = c;
   return fwrite(arr, 3, 1, f);
}

static int write_pixels(FILE *f, int rgb_dir, int vdir, int x, int y, int comp, const void *data, int write_alpha, int scanline_pad)
{
   unsigned char bg[3] = { 255, 0, 255 }, px[3];
   stbiw_uint32 zero = 0;
   int i,j,k, j_end;

   if (y <= 0)
      return 1;

   if (vdir < 0)
      j_end = -1, j = y-1;
   else
      j_end =  y, j = 0;

   for (; j != j_end; j += vdir) {
      for (i=0; i < x; ++i) {
         const unsigned char *d = (const unsigned char *) data + (j*x+i)*comp;
         if (write_alpha < 0)
            if(!fwrite(&d[comp-1], 1, 1, f)) return 0;
         switch (comp) {
            case 1:
            case 2: if(!write3(f, d[0],d[0],d[0])) return 0;
                    break;
            case 4:
               if (!write_alpha) {
                  // composite against pink background
                  for (k=0; k < 3; ++k)
                     px[k] = bg[k] + ((d[k] - bg[k]) * d[3])/255;
                  if(!write3(f, px[1-rgb_dir],px[1],px[1+rgb_dir])) return 0;
                  break;
               }
               /* FALLTHROUGH */
            case 3:
               if(!write3(f, d[1-rgb_dir],d[1],d[1+rgb_dir])) return 0;
               break;
         }
         if (write_alpha > 0)
            if(!fwrite(&d[comp-1], 1, 1, f)) return 0;
      }
      if(scanline_pad > 0 && !fwrite(&zero,scanline_pad,1,f)) return 0;
   }
   return 1;
}

static int outfile(char const *filename, int rgb_dir, int vdir, int x, int y, int comp, const void *data, int alpha, int pad, const char *fmt, ...)
{
   FILE *f;
   if (y < 0 || x < 0) return 0;
   f = fopen(filename, "wb");
   if (f) {
      va_list v;
      va_start(v, fmt);
      int ret = writefv(f, fmt, v);
      va_end(v);
      if(ret) ret = write_pixels(f,rgb_dir,vdir,x,y,comp,data,alpha,pad);
      fclose(f);
      if(!ret) return 0;
   }
   return f != NULL;
}

extern "C" int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   int pad = (-x*3) & 3;
   return outfile(filename,-1,-1,x,y,comp,data,0,pad,
           "11 4 22 4" "4 44 22 444444",
           'B', 'M', 14+40+(x*3+pad)*y, 0,0, 14+40,  // file header
            40, x,y, 1,24, 0,0,0,0,0,0);             // bitmap header
}

extern "C" int stbi_write_tga(char const *filename, int x, int y, int comp, const void *data)
{
   int has_alpha = !(comp & 1);
   return outfile(filename, -1,-1, x, y, comp, data, has_alpha, 0,
                  "111 221 2222 11", 0,0,2, 0,0,0, 0,0,x,y, 24+8*has_alpha, 8*has_alpha);
}

} // namespace stbi
