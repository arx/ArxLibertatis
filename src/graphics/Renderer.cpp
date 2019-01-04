/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/Renderer.h"

#include <algorithm>

#include <boost/foreach.hpp>

#include "graphics/texture/TextureStage.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/Texture.h"
#include "platform/CrashHandler.h"

Renderer * GRenderer;

TextureStage * Renderer::GetTextureStage(size_t textureStage) {
	return (textureStage < m_TextureStages.size()) ? m_TextureStages[textureStage] : NULL;
}

const TextureStage * Renderer::GetTextureStage(size_t textureStage) const {
	return (textureStage < m_TextureStages.size()) ? m_TextureStages[textureStage] : NULL;
}

void Renderer::ResetTexture(unsigned int textureStage) {
	GetTextureStage(textureStage)->resetTexture();
}

Texture * Renderer::GetTexture(unsigned int textureStage) const {
	return GetTextureStage(textureStage)->getTexture();
}

void Renderer::SetTexture(unsigned int textureStage, Texture * pTexture) {
	GetTextureStage(textureStage)->setTexture(pTexture);
}

void Renderer::SetTexture(unsigned int textureStage, TextureContainer * pTextureContainer) {
	
	if(pTextureContainer && pTextureContainer->m_pTexture) {
		GetTextureStage(textureStage)->setTexture(pTextureContainer->m_pTexture);
	} else {
		GetTextureStage(textureStage)->resetTexture();
	}
}

Renderer::Renderer()
	: m_initialized(false)
{ }

Renderer::~Renderer() {
	if(isInitialized()) {
		onRendererShutdown();
	}
	for(size_t i = 0; i < m_TextureStages.size(); ++i) {
		delete m_TextureStages[i];
	}
}

void Renderer::addListener(Listener * listener) {
	m_listeners.push_back(listener);
}

void Renderer::removeListener(Listener * listener) {
	m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener),
	                  m_listeners.end());
}

void Renderer::onRendererInit() {
	m_initialized = true;
	BOOST_FOREACH(Listener * listener, m_listeners) {
		listener->onRendererInit(*this);
	}
}

void Renderer::onRendererShutdown() {
	BOOST_FOREACH(Listener * listener, m_listeners) {
		listener->onRendererShutdown(*this);
	}
	m_initialized = false;
}
