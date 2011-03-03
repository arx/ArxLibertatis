#include "Image.h"

#include "graphics/Math.h"

#include <map>
#include <IL/il.h>

const unsigned int SIZE_TABLE[Image::Format_Num] =
{
        1,          // Format_L8,
        1,          // Format_A8,
        2,          // Format_L8A8,
        3,          // Format_R8G8B8,
        4,          // Format_R8G8B8A8,
        8,          // Format_DXT1,
        16,         // Format_DXT3,
        16,         // Format_DXT5,
        0,          // Format_Unknown
};


Image::Image()
    : mWidth(0)
    , mHeight(0)
    , mDepth(0)
    , mNumMipmaps(0)
    , mFormat(Format_Unknown)
    , mData(NULL)
    , mDataSize(0)
{
}

Image::Image( const Image& pOther )
    : mData(NULL)
{
    *this = pOther;
}

Image::~Image()
{
    if( mData )
        delete[] mData;
}

const Image& Image::operator = ( const Image& pOther )
{
    // Ignore self copy!
    if( &pOther == this )
        return *this;

    if( mData )
        delete[] mData;

    mWidth          = pOther.mWidth;
    mHeight         = pOther.mHeight;
    mDepth          = pOther.mDepth;
    mNumMipmaps     = pOther.mNumMipmaps;
    mFormat         = pOther.mFormat;
    mDataSize       = pOther.mDataSize;
    mData           = new unsigned char[mDataSize];

    memcpy( mData, pOther.mData, mDataSize );

    return *this;
}

unsigned int Image::GetSize( Image::Format pFormat, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth )
{
    if (pWidth == 0)
        pWidth = 1;

    if (pHeight == 0)
        pHeight = 1;

    if( pFormat >= Format_DXT1 && pFormat <= Format_DXT5 )
        return ((pWidth+3) >> 2) * ((pHeight+3) >> 2) * SIZE_TABLE[pFormat];
    else
        return pWidth * pHeight * SIZE_TABLE[pFormat] * pDepth;
}

unsigned int Image::GetSizeWithMipmaps( Image::Format pFormat, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth, int pMipmapCount )
{
    unsigned int dataSize = 0;

    unsigned int width  = pWidth;
    unsigned int height = pHeight;
    unsigned int depth  = pDepth;
    unsigned int mip    = pMipmapCount == -1 ? 0x7FFFFFFF : pMipmapCount;

    while( (width || height) && mip != 0 )
    {
        dataSize += Image::GetSize( pFormat, width, height, depth );

        width  >>= 1;
        height >>= 1;

        if( depth != 1 )
            depth  >>= 1;

        mip--;
    }

    return dataSize;
}

unsigned int Image::GetNumChannels( Image::Format pFormat )
{
    switch( pFormat )
    {
        case Format_L8:         return 1;
        case Format_A8:         return 1;
        case Format_L8A8:       return 2;
        case Format_R8G8B8:     return 3;
        case Format_R8G8B8A8:   return 4;
        case Format_DXT1:       return 3;
        case Format_DXT3:       return 4;
        case Format_DXT5:       return 4;
        default:                return 0;
    }
}

bool Image::IsCompressed( Image::Format pFormat )
{
    return pFormat >= Format_DXT1 && pFormat <= Format_DXT5;
}

void Image::Create( unsigned int pWidth, unsigned int pHeight, Image::Format pFormat, unsigned int pNumMipmaps, unsigned int pDepth )
{
    arx_assert_msg( pWidth > 0, "[Image::Create] Width is 0!" );
    arx_assert_msg( pHeight > 0, "[Image::Create] Width is 0!" );
    arx_assert_msg( pFormat < Format_Unknown, "[Image::Create] Unknown texture format!" );
    arx_assert_msg( pNumMipmaps > 0, "[Image::Create] Mipmap count must at least be 1!" );
    arx_assert_msg( pDepth > 0, "[Image::Create] Image depth must at least be 1!" );

    if( mData )
        delete[] mData;

    mWidth  = pWidth;
    mHeight = pHeight;
    mDepth  = pDepth;
    mFormat = pFormat;
    mNumMipmaps = pNumMipmaps;

    mDataSize = Image::GetSizeWithMipmaps( mFormat, mWidth, mHeight, mDepth, mNumMipmaps );
    mData = new unsigned char[mDataSize];
}

void Image::Clear()
{
	memset(mData, 0, mDataSize);
}

const unsigned char* Image::GetData() const
{
    return mData;
}

unsigned char* Image::GetData()
{
    return mData;
}

bool Image::Copy( const Image& pImage, unsigned int pX, unsigned int pY )
{
    arx_assert_msg( !IsCompressed(), "[Image::Copy] Copy of compressed images not supported yet!" );
    arx_assert_msg( !IsVolume(), "[Image::Copy] Copy of volume images not supported yet!" );

    unsigned int bpp = SIZE_TABLE[mFormat];

    // Format must match.
    if( pImage.GetFormat() != mFormat )
        return false;

    // Must fit inside boundaries
    if( pX + pImage.GetWidth() > mWidth || pY + pImage.GetHeight() > mHeight )
        return false;

    // Copy scanline by scanline
    unsigned char* dst = &mData[pY * mWidth * bpp];
    const unsigned char* src = pImage.GetData();
    for( unsigned int i = 0; i < pImage.GetHeight(); i++ )
    {
        memcpy( &dst[pX * bpp], src, pImage.GetWidth() * bpp );

        dst += mWidth * bpp;
        src += pImage.GetWidth() * bpp;
    }

    return true;
}

void Image::ChangeGamma( float pGamma )
{
    arx_assert_msg( !IsCompressed(), "[Image::ChangeGamma] Gamma change of compressed images not supported yet!" );
    arx_assert_msg( !IsVolume(), "[Image::ChangeGamma] Gamma change of volume images not supported yet!" );

    // This function was taken from a couple engines that I saw,
    // which most likely originated from the Aftershock engine.
    // Kudos to them!  What it does is increase/decrease the intensity
    // of the lightmap so that it isn't so dark.  Quake uses hardware to
    // do this, but we will do it in code.
    unsigned int numComponents = SIZE_TABLE[mFormat];
    unsigned int size = mWidth * mHeight;
    unsigned char* data = mData;

    const unsigned int MAX_COMPONENTS = 4;
    const float MAX_COMPONENT_VALUE = 255.0f;

    float components[MAX_COMPONENTS];

    // Nothing to do in this case!
    if( pGamma == 1.0f )
        return;

    // Go through every pixel in the image
    for( unsigned int i = 0; i < size; i++, data += numComponents )
    {
        float scale = 1.0f;
        float temp  = 0.0f;

        for( unsigned int j = 0; j < numComponents; j++ )
        {
            // Extract the current component value
            components[j] = (float)data[j];

            // Multiply the factor by the component value, while keeping it to a 255 ratio
            components[j] *= pGamma;

            // Check if the the value went past the highest value
            if( components[j] > MAX_COMPONENT_VALUE && (temp = (MAX_COMPONENT_VALUE/components[j])) < scale )
                scale = temp;
        }

        for( unsigned int j = 0; j < numComponents; j++ )
        {
            // Get the scale for this pixel and multiply it by the component value
            components[j] *= scale;

            // Assign the new gamma'nized value to the image
            data[j] = (unsigned char)components[j];
        }
    }
}

bool Image::ToGrayscale()
{
    int numChannels = GetNumChannels();

    if( IsCompressed() || numChannels < 3 )
        return false;

    unsigned int size, len;
    unsigned char* src = mData;

    size = len = GetSizeWithMipmaps(Format_L8, mWidth, mHeight, mDepth, mNumMipmaps);
    unsigned char* dest = new unsigned char[size];

    do
    {
        *dest++ = (77 * src[0] + 151 * src[1] + 28 * src[2] + 128) >> 8;
        src += numChannels;
    } while (--len);

    delete[] mData;
    mData = dest;

    mFormat = Format_L8;

	return true;
}

bool Image::ToNormalMap()
{
    if( mFormat != Format_L8 )
        return false;

    unsigned char *row0, *row1, *row2;
    unsigned int x,y,w,h,mipmap;
    unsigned int sx, sy, len, predx, succx;

    mFormat = Format_R8G8B8;
    mDataSize = GetSizeWithMipmaps(mFormat, mWidth, mHeight, mDepth, mNumMipmaps);

	unsigned char* newPixels = new unsigned char[mDataSize];
    unsigned char* dest = newPixels;
    unsigned char* src = mData;

    mipmap = 0;
    w = mWidth;
    h = mHeight;

    do
    {
        row1 = src + (h - 1) * w;
        row2 = src;

        y = h;

        do
        {
            row0 = row1;
            row1 = row2;
            row2 += w;

            if( y == 1 )
                row2 = src;

            x = w - 1;
            succx = 0;

            do
            {
                predx = x;
                x = succx++;

                if(succx == w)
                    succx = 0;

                sx = (row0[predx] + 2 * row1[predx] + row2[predx]) - (row0[succx] + 2 * row1[succx] + row2[succx]);
                sy = (row0[predx] + 2 * row0[x]     + row0[succx]) - (row2[predx] + 2 * row2[x]     + row2[succx]);

                len = (unsigned int)(0x000FF000 * FastRSqrt(float(sx * sx + sy * sy + 256*256)));
                sx *= len;
                sy *= len;

                dest[0] = ((sx  + 0x000FF000) >> 13);
                dest[1] = ((sy  + 0x000FF000) >> 13);
                dest[2] = ((len + 0x00000FF0) >> 5);
                dest += 3;
            } while (succx);
        } while (--y);

        src += w * h;

        if( w > 1 )
            w >>= 1;

        if( h > 1 )
            h >>= 1;

        mipmap++;
    } while (mipmap < mNumMipmaps);

    delete[] mData;
    mData = newPixels;

	return true;
}

void Image::FlipY()
{
    unsigned int width  = mWidth;
    unsigned int height = mHeight;
    unsigned int depth  = mDepth;

    unsigned int offset = 0;

    for( unsigned int i = 0; i < mNumMipmaps && (width || height); i++ )
    {
        if( width == 0 )
            width = 1;

        if( height == 0 )
            height = 1;

        if( depth == 0 )
            depth = 1;

        FlipY( mData + offset, width, height, depth );
        offset += Image::GetSize( mFormat, width, height, depth );

        width  >>= 1;
        height >>= 1;
        depth  >>= 1;
    }
}

struct DXTColBlock
{
    u16 mCol0;
    u16 mCol1;
    unsigned char	mRow[4];
};

struct DXT3AlphaBlock
{
    u16 mRow[4];
};

struct DXT5AlphaBlock
{
    unsigned char  mAlpha0;
    unsigned char  mAlpha1;
    unsigned char  mRow[6];
};

void FlipDXT1(unsigned char* data, unsigned int count);
void FlipDXT3(unsigned char* data, unsigned int count);
void FlipDXT5(unsigned char* data, unsigned int count);

void Image::FlipY( unsigned char* pData, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth )
{
    unsigned int offset;

    if( !IsCompressed() )
    {
        unsigned int imageSize = GetSize(mFormat, pWidth, pHeight);
        unsigned int lineSize  = imageSize / pHeight;

        unsigned char* swapTmp = (unsigned char*)malloc(lineSize);
		arx_assert(swapTmp);

        for( unsigned int n = 0; n < pDepth; n++ )
        {
            offset = imageSize * n;

            unsigned char* top    = pData + offset;
            unsigned char* bottom = top + (imageSize-lineSize);

            for( unsigned int i = 0; i < (pHeight >> 1); i++ )
            {
                memcpy( swapTmp, bottom, lineSize );
                memcpy( bottom, top, lineSize );
                memcpy( top, swapTmp, lineSize );

                top += lineSize;
                bottom -= lineSize;
            }
        }

		free(swapTmp);
    }
    else
    {
        void (*flipDXTn)(unsigned char*, unsigned int) = NULL;

        unsigned int xBlocks   = (pWidth+3) / 4;
        unsigned int yBlocks   = (pHeight+3) / 4;
        unsigned int blockSize = SIZE_TABLE[mFormat];
        unsigned int lineSize  = xBlocks * blockSize;

		unsigned char* swapTmp = (unsigned char*)malloc(lineSize);
		arx_assert(swapTmp);

        DXTColBlock* top    = NULL;
        DXTColBlock* bottom = NULL;

		switch(mFormat)
		{
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

        for( unsigned int j = 0; j < (yBlocks >> 1); j++ )
        {
            top = (DXTColBlock*)(pData + j * lineSize);
            bottom = (DXTColBlock*)(pData + (((yBlocks-j)-1) * lineSize));

            (*flipDXTn)( (unsigned char*)top, xBlocks );
            (*flipDXTn)( (unsigned char*)bottom, xBlocks );

            memcpy( swapTmp, bottom, lineSize );
            memcpy( bottom, top, lineSize );
            memcpy( top, swapTmp, lineSize );
        }

		free(swapTmp);
    }
}


void FlipColorBlock(unsigned char *data)
{
    unsigned char tmp;

    tmp = data[4];
    data[4] = data[7];
    data[7] = tmp;

    tmp = data[5];
    data[5] = data[6];
    data[6] = tmp;
}

void FlipSimpleAlphaBlock(u16 *data)
{
	u16 tmp;

	tmp = data[0];
	data[0] = data[3];
	data[3] = tmp;

	tmp = data[1];
	data[1] = data[2];
	data[2] = tmp;
}

void ComplexAlphaHelper(unsigned char* Data)
{
	u16 tmp[2];

	// One 4 pixel line is 12 bit, copy each line into
	// a ushort, swap them and copy back
	tmp[0] = (Data[0] | (Data[1] << 8)) & 0xfff;
	tmp[1] = ((Data[1] >> 4) | (Data[2] << 4)) & 0xfff;

	Data[0] = tmp[1];
	Data[1] = (tmp[1] >> 8) | (tmp[0] << 4);
	Data[2] = tmp[0] >> 4;
}

void FlipComplexAlphaBlock(unsigned char *Data)
{
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

void FlipDXT1(unsigned char* data, unsigned int count)
{
	for(unsigned int i = 0; i < count; ++i)
	{
		FlipColorBlock(data);
		data += 8; // Advance to next block
	}
}

void FlipDXT3(unsigned char* data, unsigned int count)
{
	for(unsigned int i = 0; i < count; ++i)
	{
		FlipSimpleAlphaBlock((u16*)data);
		FlipColorBlock(data + 8);
		data += 16; // Advance to next block
	}
}

void FlipDXT5(unsigned char* data, unsigned int count)
{
	for(unsigned int i = 0; i < count; ++i)
	{
		FlipComplexAlphaBlock(data);
		FlipColorBlock(data + 8);
		data += 16; // Advance to next block
	}
}

void Flip3dc(unsigned char* data, unsigned int count)
{
	for(unsigned int i = 0; i < count; ++i)
	{
		FlipComplexAlphaBlock(data);
		FlipComplexAlphaBlock(data + 8);
		data += 16; // Advance to next block
	}
}

ILenum ARXImageToILFormat[] = 
	{
		IL_LUMINANCE,		// Format_L8
        IL_ALPHA,			// Format_A8
        IL_LUMINANCE_ALPHA, // Format_L8A8
        IL_RGB,				// Format_R8G8B8
        IL_RGBA,			// Format_R8G8B8A8
		IL_DXT1,			// Format_DXT1
        IL_DXT3,			// Format_DXT3
        IL_DXT5,			// Format_DXT5
	};

void Image::Dump( const std::string& pFilename ) const
{
	ILuint imageName;
    ilGenImages(1, &imageName);
    ilBindImage(imageName);

	ILboolean ret = ilTexImage(mWidth, mHeight, mDepth, GetNumChannels(), ARXImageToILFormat[mFormat], IL_UNSIGNED_BYTE, mData);
	if(ret)
	{
		ret = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		ilEnable(IL_FILE_OVERWRITE);

		ret = ilSaveImage(pFilename.c_str());
		arx_assert(ret);
	}

	 ilDeleteImages(1, &imageName);
}
