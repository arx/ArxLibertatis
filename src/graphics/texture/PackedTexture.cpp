/*
 * Copyright 2011-2021 Arx Libertatis Team (see the AUTHORS file)
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

#include <utility>

#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"
#include "io/log/Logger.h"

PackedTexture::PackedTexture(size_t textureSize, Image::Format textureFormat)
	: m_textureSize(textureSize)
	, m_textureFormat(textureFormat)
{ }

void PackedTexture::clear() {
	m_textures.clear();
}

void PackedTexture::upload() {
	for(auto & tree : m_textures) {
		if(tree->dirty) {
			tree->texture->upload();
			tree->dirty = false;
		}
	}
}

PackedTexture::TextureTree::TextureTree(size_t textureSize, Image::Format textureFormat)
	: texture(GRenderer->createTexture())
{
	root.rect = Rect(0, 0, s32(textureSize - 1), s32(textureSize - 1));
	
	if(!texture->create(textureSize, textureSize, textureFormat)) {
		LogError << "Could not create texture for size " << textureSize << " and format " << textureFormat;
		texture.reset();
		dirty = false;
		return;
	}
	
	texture->getImage().clear();
	dirty = true;
}

PackedTexture::TextureTree::Node * PackedTexture::TextureTree::insertImage(const Image & image) {
	
	Node * node = root.insertImage(image);
	
	if(node != nullptr) {
		texture->getImage().copy(image, size_t(node->rect.left), size_t(node->rect.top));
		dirty = true;
	}
	
	return node;
}

bool PackedTexture::insertImage(const Image & image, size_t & textureIndex, Vec2i & offset) {
	
	// Validate image size
	if(image.getWidth() > m_textureSize || image.getHeight() > m_textureSize) {
		return false;
	}
	
	// Copy to one of the existing textures
	TextureTree::Node * node = nullptr;
	size_t nodeTree = 0;
	
	for(size_t i = 0; i < m_textures.size(); i++) {
		node = m_textures[i]->insertImage(image);
		nodeTree = i;
		if(node) {
			break;
		}
	}
	
	// No space found, create a new texture
	if(!node) {
		std::unique_ptr<TextureTree> newTree = std::make_unique<TextureTree>(m_textureSize, m_textureFormat);
		if(!newTree->texture) {
			return false;
		}
		m_textures.emplace_back(std::move(newTree));
		node = m_textures[m_textures.size() - 1]->insertImage(image);
		nodeTree = m_textures.size() - 1;
	}
	
	// A node must have been found
	arx_assert(node);
	
	// Copy texture there
	if(node) {
		// Copy values back into info structure
		offset = node->rect.topLeft();
		textureIndex = nodeTree;
	}
	
	return node != nullptr;
}

Texture & PackedTexture::getTexture(size_t index) {
	arx_assert(index < m_textures.size());
	arx_assert(m_textures[index]->texture);
	return *m_textures[index]->texture;
}

PackedTexture::TextureTree::Node::Node()
	: used(false)
{
	children[0] = nullptr;
	children[1] = nullptr;
}

PackedTexture::TextureTree::Node * PackedTexture::TextureTree::Node::insertImage(const Image & image) {
	
	// We're in a full node/leaf, return immediately.
	if(used) {
		return nullptr;
	}
	
	// If we're not a leaf, try inserting in childs
	if(children[0]) {
		
		Node * result = children[0]->insertImage(image);
		
		if(!result) {
			result = children[1]->insertImage(image);
		}
		
		used = children[0]->used && children[1]->used;
		return result;
	}
	
	s32 diffW = (rect.width() + 1) - s32(image.getWidth());
	s32 diffH = (rect.height() + 1) - s32(image.getHeight());
	
	// If we're too small, return.
	if(diffW < 0 || diffH < 0) {
		return nullptr;
	}
	
	// Perfect match !
	if(diffW == 0 && diffH == 0) {
		used = true;
		return this;
	}
	
	// Otherwise, gotta split this node and create some kids
	children[0] = std::make_unique<Node>();
	children[1] = std::make_unique<Node>();
	
	if(diffW > diffH) {
		children[0]->rect = Rect(rect.left, rect.top, rect.left + s32(image.getWidth()) - 1, rect.bottom);
		children[1]->rect = Rect(rect.left + s32(image.getWidth()), rect.top, rect.right, rect.bottom);
	} else {
		children[0]->rect = Rect(rect.left, rect.top, rect.right, rect.top + s32(image.getHeight()) - 1);
		children[1]->rect = Rect(rect.left, rect.top + s32(image.getHeight()), rect.right, rect.bottom);
	}
	
	// Insert into first child we created
	return children[0]->insertImage(image);
}
