#ifndef _ARX_IMAGE_H_
#define _ARX_IMAGE_H_

#include "platform/Platform.h"

class Image
{
public:
    enum Format
    {
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

public:
    Image();
    Image( const Image& pOther );
    virtual ~Image();

    const Image& operator = ( const Image& pOther );

	bool LoadFromFile( const std::string& filename );
	bool LoadFromMemory( void* pData, unsigned int size );

    void Create( unsigned int width, unsigned int height, Format format, unsigned int numMipmaps = 1, unsigned int depth = 1 );
	
	void Clear();
	void Reset();
	
	bool IsValid() const					{ return mData != NULL; }
    unsigned int GetWidth() const			{ return mWidth;      }
    unsigned int GetHeight() const			{ return mHeight;     }
    unsigned int GetDepth() const			{ return mDepth;      }
    unsigned int GetNumMipmaps() const		{ return mNumMipmaps; }
    Format		 GetFormat() const			{ return mFormat;     }
    unsigned int GetDataSize() const		{ return mDataSize;   }
    
    unsigned int  GetNumChannels() const	{ return Image::GetNumChannels( mFormat ); }  
    bool IsCompressed() const				{ return Image::IsCompressed( mFormat ); }
    bool IsVolume() const					{ return mDepth > 1;  }
	bool HasAlpha() const					{ return mFormat == Format_A8 || mFormat == Format_L8A8 || mFormat == Format_R8G8B8A8 || mFormat == Format_B8G8R8A8 || mFormat == Format_DXT3 || mFormat == Format_DXT5; }

    //! Access to internal data.
    const unsigned char* GetData() const;
    unsigned char* GetData();

    void FlipY();

    //! Copy an image into this image's buffer.
    //! Works only with uncompressed formats
    bool Copy( const Image& srcImage, unsigned int dstX, unsigned int dstY );
	bool Copy( const Image& srcImage, unsigned int dstX, unsigned int dstY, unsigned int srcX, unsigned int srcY, unsigned int width, unsigned int height );

    void ChangeGamma( float pGamma );
    
    bool ToGrayscale();
    bool ToNormalMap();

	void save( const std::string& pFilename ) const;

public:
    static unsigned int	GetSize( Format pFormat, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth = 1 );
    static unsigned int	GetSizeWithMipmaps( Format pFormat, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth = 1, int pMipmapCount = -1 );
    static unsigned int	GetNumChannels( Format pFormat );
    static bool IsCompressed( Format pFormat );

private:
    void FlipY( unsigned char* pData, unsigned int pWidth, unsigned int pHeight, unsigned int pDepth );
    
private:
    unsigned int mWidth;		//!< Image width.
    unsigned int mHeight;       //!< Image height.
    unsigned int mDepth;        //!< Image depth. Depth == 1 for normal images, > 1 for depth images.

    unsigned int mNumMipmaps;   //!< Number of mipmaps.
    Format mFormat;				//!< Image format.

    unsigned char* mData;		//!< Pointer to image data buffer.
    unsigned int mDataSize;     //!< Size of image buffer.
};


#endif	// _ARX_IMAGE_H_
