#include "graphics/texture/PackedTexture.h"

#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"

	
PackedTexture::PackedTexture( unsigned int pSize, Image::Format pFormat )
    : mTexSize(pSize)
    , mTexFormat(pFormat)
{
}

PackedTexture::~PackedTexture()
{
    ClearAll();
}

void PackedTexture::ClearAll()
{
    for( unsigned int i = 0; i < mTexTrees.size(); i++ )
    {
        delete mTexTrees[i];
    }
    mTexTrees.clear();

    for( unsigned int i = 0; i < mImages.size(); i++ )
    {
        delete mImages[i];
    }
    mImages.clear();

    for( unsigned int i = 0; i < mTextures.size(); i++ )
    {
        delete mTextures[i];
    }
    mTextures.clear();
}

void PackedTexture::BeginPacking()
{
    ClearAll();
}

void PackedTexture::EndPacking()
{
    // Trees are now useless.
    for( unsigned int i = 0; i < mTexTrees.size(); i++ )
    {
        delete mTexTrees[i];
    }
    mTexTrees.clear();

    mTextures.resize( mImages.size() );

    // Create a texture with each images.
    for( unsigned int i = 0; i < mTextures.size(); i++ )
    {
        mTextures[i] = GRenderer->CreateTexture2D();
        mTextures[i]->Create( *mImages[i], false );
        mTextures[i]->Init();        
                
        mTextures[i]->SetWrapMode( Texture::Wrap_S, Texture::Wrap_Clamp );
        mTextures[i]->SetWrapMode( Texture::Wrap_T, Texture::Wrap_Clamp );
        mTextures[i]->SetMinFilter( Texture::MinFilter_Linear );
        mTextures[i]->SetMagFilter( Texture::MagFilter_Linear );
        
        // Images are not needed anymore.
        delete mImages[i];
    }

    mImages.clear();
}

bool PackedTexture::InsertImage( const Image& pImg, int& pOffsetU, int& pOffsetV, unsigned int& pTextureIndex )
{
    // Validate image size
    if( pImg.GetWidth() > mTexSize || pImg.GetHeight() > mTexSize )
        return false;

    // Copy to one of the existing image
    TextureTree::Node* node = NULL;
    unsigned int nodeTree = 0;

    for( unsigned int i = 0; i < mTexTrees.size(); i++ )
    {
        node = mTexTrees[i]->InsertImage( pImg );
        nodeTree = i;
    }
    
    // No space found, create a new tree
    if( !node )
    {
        mTexTrees.push_back( new TextureTree(mTexSize) );
        mImages.push_back( new Image() );

        node = mTexTrees[mTexTrees.size()-1]->InsertImage( pImg );
        nodeTree = mTexTrees.size()-1;

        mImages[mImages.size()-1]->Create( mTexSize, mTexSize, mTexFormat );
    }

    // A node must have been found.
    arx_assert( node );
    
    // Copy texture there
    if( node )
    {
        mImages[nodeTree]->Copy( pImg, node->mRect.mLeft, node->mRect.mTop );

        // Copy values back into info structure.
        pOffsetU = node->mRect.mLeft;
		pOffsetV = node->mRect.mTop;
		pTextureIndex = nodeTree;
    }

    return node != NULL;
}

Texture2D& PackedTexture::GetTexture( unsigned int pTexture )
{
    arx_assert( pTexture < mTextures.size() );
    arx_assert( mTextures[pTexture] );
    return *mTextures[pTexture];
}

unsigned int PackedTexture::GetTextureCount() const
{
	return mTextures.size();
}

unsigned int PackedTexture::GetTextureSize() const
{
	return mTexSize;
}

PackedTexture::TextureTree::Node::Node()
{
    mChilds[0] = NULL;
    mChilds[1] = NULL;
    mInUse     = 0;
}

PackedTexture::TextureTree::Node::~Node()
{
    if( mChilds[0] )
        delete mChilds[0];

    if( mChilds[1] )
        delete mChilds[1];
}

PackedTexture::TextureTree::Node* PackedTexture::TextureTree::Node::InsertImage( const Image& pImg )
{
    Node* result = NULL;

    // We're in a full node/leaf, return immediately.
    if( mInUse )
        return NULL;

    // If we're not a leaf, try inserting in childs
    if( mChilds[0] )
    {
        result = mChilds[0]->InsertImage( pImg );
        
        if( !result )
            result = mChilds[1]->InsertImage( pImg );

        mInUse = mChilds[0]->mInUse && mChilds[1]->mInUse;
        return result;
    }

    int diffW = (mRect.mRight-mRect.mLeft+1) - pImg.GetWidth();
    int diffH = (mRect.mBottom-mRect.mTop+1) - pImg.GetHeight();
    
    // If we're too small, return.
    if( diffW < 0 || diffH < 0 )
        return NULL;

    // Perfect match !
    if( diffW == 0 && diffH == 0 )
    {
        mInUse = true;
        return this;
    }

    // Otherwise, gotta split this node and create some kids.
    mChilds[0] = new Node();
    mChilds[1] = new Node();
    
    if( diffW > diffH )
    {
        mChilds[0]->mRect = Rect( mRect.mLeft, mRect.mTop, mRect.mLeft + pImg.GetWidth() - 1, mRect.mBottom );
        mChilds[1]->mRect = Rect( mRect.mLeft + pImg.GetWidth(), mRect.mTop, mRect.mRight, mRect.mBottom );
    }
    else
    {
        mChilds[0]->mRect = Rect( mRect.mLeft, mRect.mTop, mRect.mRight, mRect.mTop + pImg.GetHeight() - 1 );
        mChilds[1]->mRect = Rect( mRect.mLeft, mRect.mTop + pImg.GetHeight(), mRect.mRight, mRect.mBottom );
    }
    
    // Insert into first child we created
    return mChilds[0]->InsertImage( pImg );
}

PackedTexture::TextureTree::TextureTree( unsigned int pSize )
{
    mRoot.mRect.mLeft = 0;
    mRoot.mRect.mRight = pSize-1;
    mRoot.mRect.mTop = 0;
    mRoot.mRect.mBottom = pSize-1;
}

PackedTexture::TextureTree::Node* PackedTexture::TextureTree::InsertImage( const Image& pImg )
{
    return mRoot.InsertImage( pImg );
}
