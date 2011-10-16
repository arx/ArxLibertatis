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

#include "graphics/texture/PackedTexture.h"

#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"

PackedTexture::PackedTexture(unsigned int pSize, Image::Format pFormat) : mTexSize(pSize), mTexFormat(pFormat) { }

PackedTexture::~PackedTexture() {
	ClearAll();
}

void PackedTexture::ClearAll() {
	
	for(unsigned int i = 0; i < mTexTrees.size(); i++) {
		delete mTexTrees[i];
	}
	mTexTrees.clear();
	
	for(unsigned int i = 0; i < mImages.size(); i++) {
		delete mImages[i];
	}
	mImages.clear();
	
	for(unsigned int i = 0; i < mTextures.size(); i++) {
		delete mTextures[i];
	}
	mTextures.clear();
}

void PackedTexture::BeginPacking() {
	ClearAll();
}

void PackedTexture::EndPacking() {
	
	// Trees are now useless.
	for(unsigned int i = 0; i < mTexTrees.size(); i++) {
		delete mTexTrees[i];
	}
	mTexTrees.clear();
	
	mTextures.resize(mImages.size());
	
	// Create a texture with each images.
	for(unsigned int i = 0; i < mTextures.size(); i++) {
		
		mTextures[i] = GRenderer->CreateTexture2D();
		mTextures[i]->Init(*mImages[i], 0);
		
		// Images are not needed anymore.
		delete mImages[i];
	}
	
	mImages.clear();
}

bool PackedTexture::InsertImage(const Image & pImg, int & pOffsetU, int & pOffsetV, unsigned int & pTextureIndex) {
	
	// Validate image size
	if(pImg.GetWidth() > mTexSize || pImg.GetHeight() > mTexSize) {
		return false;
	}
	
	// Copy to one of the existing image
	TextureTree::Node * node = NULL;
	unsigned int nodeTree = 0;
	
	for(unsigned int i = 0; i < mTexTrees.size(); i++) {
		node = mTexTrees[i]->InsertImage( pImg );
		nodeTree = i;
	}
	
	// No space found, create a new tree
	if(!node) {
		
		mTexTrees.push_back(new TextureTree(mTexSize));
		
		Image* newPage = new Image();
		newPage->Create(mTexSize, mTexSize, mTexFormat);
		newPage->Clear();
		
		mImages.push_back(newPage);
		
		node = mTexTrees[mTexTrees.size() - 1]->InsertImage(pImg);
		nodeTree = mTexTrees.size() - 1;
	}
	
	// A node must have been found.
	arx_assert(node);
	
	// Copy texture there
	if(node) {
		
		mImages[nodeTree]->Copy( pImg, node->mRect.left, node->mRect.top );
		
		// Copy values back into info structure.
		pOffsetU = node->mRect.left;
		pOffsetV = node->mRect.top;
		pTextureIndex = nodeTree;
	}
	
	return node != NULL;
}

Texture2D& PackedTexture::GetTexture(unsigned int pTexture) {
	arx_assert(pTexture < mTextures.size());
	arx_assert(mTextures[pTexture]);
	return *mTextures[pTexture];
}

unsigned int PackedTexture::GetTextureCount() const {
	return mTextures.size();
}

unsigned int PackedTexture::GetTextureSize() const {
	return mTexSize;
}

PackedTexture::TextureTree::Node::Node() {
	mChilds[0] = NULL;
	mChilds[1] = NULL;
	mInUse = 0;
}

PackedTexture::TextureTree::Node::~Node() {
	
	if(mChilds[0]) {
		delete mChilds[0];
	}
	
	if(mChilds[1]) {
		delete mChilds[1];
	}
}

PackedTexture::TextureTree::Node * PackedTexture::TextureTree::Node::InsertImage(const Image & pImg) {
	
	Node * result = NULL;
	
	// We're in a full node/leaf, return immediately.
	if(mInUse) {
		return NULL;
	}
	
	// If we're not a leaf, try inserting in childs
	if(mChilds[0]) {
		
		result = mChilds[0]->InsertImage(pImg);
		
		if(!result) {
			result = mChilds[1]->InsertImage(pImg);
		}
		
		mInUse = mChilds[0]->mInUse && mChilds[1]->mInUse;
		return result;
	}
	
	int diffW = (mRect.width()+1) - pImg.GetWidth();
	int diffH = (mRect.height()+1) - pImg.GetHeight();
	
	// If we're too small, return.
	if(diffW < 0 || diffH < 0) {
		return NULL;
	}
	
	// Perfect match !
	if(diffW == 0 && diffH == 0) {
		mInUse = true;
		return this;
	}
	
	// Otherwise, gotta split this node and create some kids.
	mChilds[0] = new Node();
	mChilds[1] = new Node();
	
	if(diffW > diffH) {
		mChilds[0]->mRect = Rect(mRect.left, mRect.top, mRect.left + pImg.GetWidth() - 1, mRect.bottom);
		mChilds[1]->mRect = Rect(mRect.left + pImg.GetWidth(), mRect.top, mRect.right, mRect.bottom);
	} else {
		mChilds[0]->mRect = Rect(mRect.left, mRect.top, mRect.right, mRect.top + pImg.GetHeight() - 1);
		mChilds[1]->mRect = Rect(mRect.left, mRect.top + pImg.GetHeight(), mRect.right, mRect.bottom);
	}
	
	// Insert into first child we created
	return mChilds[0]->InsertImage(pImg);
}

PackedTexture::TextureTree::TextureTree(unsigned int pSize) {
	mRoot.mRect.left = 0;
	mRoot.mRect.right = pSize-1;
	mRoot.mRect.top = 0;
	mRoot.mRect.bottom = pSize-1;
}

PackedTexture::TextureTree::Node * PackedTexture::TextureTree::InsertImage(const Image & pImg) {
	return mRoot.InsertImage( pImg );
}
