
#ifndef ARX_GRAPHICS_TEXTIRE_PACKEDTEXTURE_H
#define ARX_GRAPHICS_TEXTIRE_PACKEDTEXTURE_H

#include <vector>

#include "graphics/Math.h"
#include "graphics/image/Image.h"
#include "math/Rectangle.h"

class Texture2D;

class PackedTexture {
	
public:
	
	PackedTexture(unsigned int pSize, Image::Format pFormat);
	~PackedTexture();
	
	void ClearAll();
	
	void BeginPacking();
	void EndPacking();
	
	bool InsertImage(const Image& pImg, int & pOffsetU, int & pOffsetV, unsigned int & pTextureIndex);
	
	Texture2D & GetTexture(unsigned int pTexture);
	
	unsigned int GetTextureSize() const;
	unsigned int GetTextureCount() const;
	
protected:
	
	class TextureTree {
		
	public:
		
		struct Node {
			
			Node();
			~Node();
			
			Node * InsertImage(const Image & pImg);
			
			Node * mChilds[2];
			Rect mRect;
			bool mInUse;
		};
		
		TextureTree(unsigned int pSize);
		Node* InsertImage(const Image & pImg);
		
	private:
		
		Node mRoot;
		
	};
	
private:
	
	std::vector<Image *> mImages;
	std::vector<Texture2D *> mTextures;
	std::vector<TextureTree *> mTexTrees;
	
	unsigned int mTexSize;
	Image::Format mTexFormat;
	
};


#endif // ARX_GRAPHICS_TEXTIRE_PACKEDTEXTURE_H
