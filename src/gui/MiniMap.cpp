/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
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
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999-2001 ARKANE Studios SA. All rights reserved

#include "gui/MiniMap.h"

#include <cstdio>

#include "core/Core.h"
#include "core/Localisation.h"

#include "game/EntityManager.h"
#include "game/Levels.h"
#include "game/NPC.h"
#include "game/Player.h"

#include "gui/Text.h"
#include "gui/Interface.h"

#include "graphics/Draw.h"
#include "graphics/Math.h"
#include "graphics/data/TextureContainer.h"
#include "graphics/texture/TextureStage.h"
#include "graphics/texture/Texture.h"

#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

#include "scene/Interactive.h"

MiniMap g_miniMap; // TODO: remove this

void MiniMap::getData(int showLevel) {
	
	if(m_data[showLevel].m_texContainer == NULL) {
		
		char name[256];
		char levelMap[256];
		GetLevelNameByNum(showLevel, name);

		sprintf(levelMap, "graph/levels/level%s/map", name);
		m_data[showLevel].m_texContainer = TextureContainer::Load(levelMap);

		if(m_data[showLevel].m_texContainer) { // 4 pix/meter
            
			//TODO-RENDERING: SpecialBorderSurface...
			//SpecialBorderSurface(m_data[showLevel].m_texContainer, m_data[showLevel].m_texContainer->m_dwWidth, m_data[showLevel].m_texContainer->m_dwHeight);

			m_data[showLevel].m_height = static_cast<float>(m_data[showLevel].m_texContainer->m_dwHeight); 
			m_data[showLevel].m_width = static_cast<float>(m_data[showLevel].m_texContainer->m_dwWidth);

			float minX = std::numeric_limits<float>::max();
			float maxX = std::numeric_limits<float>::min();
			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();
			
			EERIEPOLY *ep = NULL;
			EERIE_BKG_INFO *eg = NULL;

			for(int j = 0; j < ACTIVEBKG->Zsize; j++) {
				for(int i = 0; i < ACTIVEBKG->Xsize; i++) {
                    
					eg = &ACTIVEBKG->Backg[i+j*ACTIVEBKG->Xsize];

					for(int k = 0; k < eg->nbpoly; k++) {
                        
						ep = &eg->polydata[k];

						if(ep) {
							minX = min(minX, ep->min.x);
							maxX = max(maxX, ep->max.x);
							minY = min(minY, ep->min.z);
							maxY = max(maxY, ep->max.z);
						}
					}
				}

				m_mapMaxY[showLevel] = maxY;
				m_data[showLevel].m_ratioX = minX;
				m_data[showLevel].m_ratioY = minY;

				for(int l = 0; l < MAX_MINIMAP_DATA; l++) {
					m_data[l].m_offsetX = 0;
					m_data[l].m_offsetY = 0;
				}
			}
		}
	}
}

extern long Book_MapPage;

void MiniMap::validatePos() {
	
	long showLevel = ARX_LEVELS_GetRealNum(CURRENTLEVEL); 

	if((showLevel >= 0) && (showLevel < MAX_MINIMAP_DATA)) {
        
		if(m_data[showLevel].m_texContainer == NULL) {
			getData(showLevel);
		}

		if(m_data[CURRENTLEVEL].m_texContainer == NULL){
			getData(CURRENTLEVEL);
		}

		if(m_data[showLevel].m_texContainer) {
			show(ARX_LEVELS_GetRealNum(CURRENTLEVEL), 2);
		}
	}
}

void MiniMap::validatePlayerPos() {
    
	if(BLOCK_PLAYER_CONTROLS) {
        return;
	}

	float req;

	if((player.Interface & INTER_MAP) && (!(player.Interface & INTER_COMBATMODE)) && (Book_Mode == BOOKMODE_MINIMAP)) {
		req = 20.f;
	}
	else {
        req = 80.f;
	}

	if(fartherThan(Vec2f(m_playerLastPosX, m_playerLastPosZ), Vec2f(player.pos.x, player.pos.z), req)) {
		m_playerLastPosX = player.pos.x;
		m_playerLastPosZ = player.pos.z;
		validatePos();
	}
}

void MiniMap::loadOffsets() {
    
	std::string iniMiniOffsets = "graph/levels/mini_offsets.ini";
	
	PakFile *file = resources->getFile(iniMiniOffsets.c_str());
	
	if(!file) {
		LogError << "Missing " << iniMiniOffsets;
		return;
	}
	
	size_t fileSize = file->size();
	char *dat = new char[fileSize + 2];
	dat[fileSize + 1] = '\0';
	//dat[fileSize] = dat[fileSize + 1]; // TODO remove
	
	file->read(dat);
	
	if(dat) {
        
		size_t pos = 0;
        
		for(int i = 0; i < 29; i++) { // Why 29?
            
            char t[512];
            int nRead = sscanf(dat + pos, "%s %f %f", t, &m_miniOffsetX[i], &m_miniOffsetY[i]);
        
            if(nRead != 3) {
                LogError << "Error parsing line " << i << " of mini_offsets.ini: read " << nRead;
            }
            
            while((pos < fileSize) && (dat[pos] != '\n')) {
                pos++;
            }
            
            pos++;
            
            if(pos >= fileSize) {
                break;
            }
		}
		
		delete[] dat;
	}

	m_miniOffsetX[0] = 0;
	m_miniOffsetY[0] = -0.5;
	m_miniOffsetX[1] = 0;
	m_miniOffsetY[1] = 0;
	m_miniOffsetX[14] = 130;
	m_miniOffsetY[14] = 0;
	m_miniOffsetX[15] = 31;
	m_miniOffsetY[15] = -3.5;
}

void MiniMap::reveal() {
    
	for(int d = 0; d < MAX_MINIMAP_DATA; d++) {
		for(int j = 0; j < MINIMAP_MAX_Z; j++) {
			for(int i = 0; i < MINIMAP_MAX_X; i++) {
				m_data[d].m_revealed[i][j] = 255;
			}
		}
	}
}

void MiniMap::firstInit() {
    
    m_pTexDetect = NULL;
    m_mapMarkerTexCont = NULL;
    
    m_playerLastPosX = -999999.f;
    m_playerLastPosZ = -999999.f;
    
    resetData();
    
	for(int i = 0; i < MAX_MINIMAP_DATA; i++) {
		m_miniOffsetX[i] = 0;
		m_miniOffsetY[i] = 0;
	}

	loadOffsets();
}

void MiniMap::resetData() {
    
    for(int i = 0; i < MAX_MINIMAP_DATA; i++) {
        
        m_data[i].m_texContainer = NULL;
        m_data[i].m_offsetX = 0.f;
        m_data[i].m_offsetY = 0.f;
        m_data[i].m_ratioX = 0.f;
        m_data[i].m_ratioY = 0.f;
        m_data[i].m_width = 0.f;
        m_data[i].m_height = 0.f;
        memset(m_data[i].m_revealed, 0, sizeof(m_data[i].m_revealed[0][0] * MINIMAP_MAX_X * MINIMAP_MAX_Z)); // Sets the whole array to 0
	}
}

void MiniMap::reset() {
    
	purgeTexContainer();
	resetData();
}

void MiniMap::purgeTexContainer() {
    
	for(int i = 0; i < MAX_MINIMAP_DATA; i++) {
        
		delete m_data[i].m_texContainer;
		m_data[i].m_texContainer = NULL;
	}
}

void MiniMap::show(int showLevel, int flag, int fl2) {
	
	// Nuky - centralized some constants and dezoomed ingame minimap
	const int FL2_SIZE = 300;
	const int FL2_LEFT = 390;
	const int FL2_RIGHT = 590;
	const int FL2_TOP = 135;
	const int FL2_BOTTOM = 295;
	const float FL2_PLAYERSIZE = 4.f;
	
	const float DECALY = -150;
	const float DECALX = +40;
	
	if(!m_pTexDetect) {
		m_pTexDetect = TextureContainer::Load("graph/particles/flare");
	}
	
	// First Load Minimap TC & DATA if needed
	if(m_data[showLevel].m_texContainer == NULL) {
		getData(showLevel);
	}

	if(m_data[showLevel].m_texContainer) {
		
		GRenderer->SetRenderState(Renderer::DepthTest, false);
		
		float sstartx, sstarty;
		float startx, starty, casex, casey, ratio;
		float mod_x = (float)MAX_BKGX / (float)MINIMAP_MAX_X;
		float mod_z = (float)MAX_BKGZ / (float)MINIMAP_MAX_Z;

		if(flag == 1) {
		    
			startx = 0;
			starty = 0;
			casex = (900) / ((float)MINIMAP_MAX_X);
			casey = (900) / ((float)MINIMAP_MAX_Z);
			ratio = 900.f / 250.f;
            
			if(fl2) {
				casex = (FL2_SIZE) / ((float)MINIMAP_MAX_X);
				casey = (FL2_SIZE) / ((float)MINIMAP_MAX_Z);
				ratio = FL2_SIZE / 250.f;
			}
		}
		else {
		    
			startx = (140); 
			starty = (120);
			casex = (250) / ((float)MINIMAP_MAX_X);
			casey = (250) / ((float)MINIMAP_MAX_Z);
			ratio = 1.f;
		}

		sstartx = startx;
		sstarty = starty;


		float ofx, ofx2, ofy, ofy2, px, py;
		px = 0.f;
		py = 0.f;

		ofx	= m_miniOffsetX[CURRENTLEVEL];
		ofx2 = m_data[showLevel].m_ratioX;
		ofy	= m_miniOffsetY[CURRENTLEVEL];
		ofy2 = m_data[showLevel].m_ratioY;

		if((showLevel == ARX_LEVELS_GetRealNum(CURRENTLEVEL)) || (flag == 2)) {
			
			// Computes playerpos
			ofx = m_miniOffsetX[CURRENTLEVEL];
			ofx2 = m_data[showLevel].m_ratioX;
			ofy = m_miniOffsetY[CURRENTLEVEL];
			ofy2 = m_data[showLevel].m_ratioY;
		
			px = startx + ((player.pos.x + ofx - ofx2) * ( 1.0f / 100 ) * casex
			               + m_miniOffsetX[CURRENTLEVEL] * ratio * mod_x) / mod_x ; //( 1.0f / 100 )*2;
			py = starty + ((m_mapMaxY[showLevel] - ofy - ofy2) * ( 1.0f / 100 ) * casey
			               - (player.pos.z + ofy - ofy2) * ( 1.0f / 100 ) * casey + m_miniOffsetY[CURRENTLEVEL] * ratio * mod_z) / mod_z ;    //( 1.0f / 100 )*2;

			if(flag == 1) {
				sstartx = startx;
				sstarty = starty;

				startx = 490.f - px;
				starty = 220.f - py;
				px += startx;
				py += starty;
			}
		}

		TexturedVertex verts[4];
		GRenderer->SetTexture(0, m_data[showLevel].m_texContainer);

		for(int k = 0; k < 4; k++) {
            
			verts[k].color = 0xFFFFFFFF;
			verts[k].rhw = 1;
			verts[k].p.z = 0.00001f;
		}

		float div = (1.0f / 25);
		TextureContainer * tc = m_data[showLevel].m_texContainer;
		float dw = 1.f / tc->m_pTexture->getStoredSize().x; 
		float dh = 1.f / tc->m_pTexture->getStoredSize().y;
		
		float vx2 = 4.f * dw * mod_x;
		float vy2 = 4.f * dh * mod_z;

		float _px;
		Rect boundaries;
		float MOD20, MOD20DIV, divXratio, divYratio;

		boundaries.bottom = boundaries.left = boundaries.right = boundaries.top = 0;
		MOD20 = MOD20DIV = divXratio = divYratio = 0.f;

		if(flag != 2) {
			if (flag == 1) {
                
				MOD20 = 20.f * Xratio;
				MOD20DIV = 1.f / (MOD20);
				//@PERF do if(fl2){}else{} to make 4 and not 8 flot op if fl2.

				boundaries.left = checked_range_cast<Rect::Num>((360 + MOD20) * Xratio);
				boundaries.right = checked_range_cast<Rect::Num>((555 - MOD20) * Xratio);
				boundaries.top = checked_range_cast<Rect::Num>((85 + MOD20) * Yratio);
				boundaries.bottom = checked_range_cast<Rect::Num>((355 - MOD20) * Yratio);

				if(fl2) {
					boundaries.left = checked_range_cast<Rect::Num>((FL2_LEFT + MOD20) * Xratio);
					boundaries.right = checked_range_cast<Rect::Num>((FL2_RIGHT - MOD20) * Xratio);
					boundaries.top = checked_range_cast<Rect::Num>((FL2_TOP + MOD20) * Yratio);
					boundaries.bottom = checked_range_cast<Rect::Num>((FL2_BOTTOM - MOD20) * Yratio);
				}
			}

			GRenderer->SetRenderState(Renderer::AlphaBlending, true);
			GRenderer->SetBlendFunc(Renderer::BlendZero, Renderer::BlendInvSrcColor);
			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapClamp);

			if(fl2) {
				GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendInvSrcColor);
			}
		}
		else {
			divXratio = 1.f / Xratio;
			divYratio = 1.f / Yratio;
		}

		for(int j = -2; j < MINIMAP_MAX_Z + 2; j++) {
			for(int i = -2; i < MINIMAP_MAX_X + 2; i++) {
                
				float vx, vy, vxx, vyy;
				vxx = ((float)i * (float)ACTIVEBKG->Xdiv * mod_x);
				vyy = ((float)j * (float)ACTIVEBKG->Zdiv * mod_z);
				vx = (vxx * div) * dw;
				vy = (vyy * div) * dh;

				long okay = 1;
				float posx = (startx + i * casex) * Xratio;
				float posy = (starty + j * casey) * Yratio;

				if(flag == 1) {
                    
					if((posx < 360 * Xratio)
							||	(posx > 555 * Xratio)
							||	(posy < 85 * Yratio)
							||	(posy > 355 * Yratio)) {
						okay = 0;
                    }

					if(fl2) {
                        
						okay = 1;

						if((posx < FL2_LEFT * Xratio)
						        ||	(posx > FL2_RIGHT * Xratio)
						        ||	(posy < FL2_TOP * Yratio)
						        ||	(posy > FL2_BOTTOM * Yratio)) {
							okay = 0;
						}
					}
				}
				else {
                    if((posx > 345 * Xratio) ||	(posy > 290 * Yratio)) {
                        okay = 0;
                    }
				}

				if(okay){
                    
					if((flag == 2)
							&& (i >= 0) && (i < MINIMAP_MAX_X)
							&& (j >= 0) && (j < MINIMAP_MAX_Z)) {
                        
						float d = fdist(Vec2f(posx * divXratio + casex * ( 1.0f / 2 ), posy * divYratio), Vec2f(px, py));

						if(d <= 6.f) {
                        
							long r;
							float vv = (6 - d) * ( 1.0f / 6 );

							if(vv >= 0.5f) {
								vv = 1.f;
							}
							else if (vv > 0.f) {
								vv = vv * 2.f;
							}
							else {
								vv = 0.f;
							}

							r = vv * 255.f;


							long ucLevel =  max(r, (long)m_data[showLevel].m_revealed[i][j]);

							m_data[showLevel].m_revealed[i][j] = checked_range_cast<unsigned char>(ucLevel);
						}
					}

					verts[3].p.x = verts[0].p.x = (posx);
					verts[1].p.y = verts[0].p.y = (posy);
					verts[2].p.x = verts[1].p.x = posx + (casex * Xratio);
					verts[3].p.y = verts[2].p.y = posy + (casey * Yratio);

					verts[3].uv.x = verts[0].uv.x = vx;
					verts[1].uv.y = verts[0].uv.y = vy;
					verts[2].uv.x = verts[1].uv.x = vx + vx2;
					verts[3].uv.y = verts[2].uv.y = vy + vy2;

					if(flag != 2) {
                    
						float v;
						float oo = 0.f;

						if((i < 0) || (i >= MINIMAP_MAX_X) || (j < 0) || (j >= MINIMAP_MAX_Z)) {
                            v = 0;
						}
						else {
                            v = ((float)m_data[showLevel].m_revealed[i][j]) * (1.0f / 255);
						}

						if(flag == 1) {
                            
							int vert = 0;
							_px = verts[vert].p.x - boundaries.left;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = boundaries.right - verts[vert].p.x;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = verts[vert].p.y - boundaries.top;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = boundaries.bottom - verts[vert].p.y;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
                            }
						}

						verts[0].color = Color::gray(fl2 ? v * (1.f/2) : v).toBGR();

						oo += v;

						if((i + 1 < 0) || (i + 1 >= MINIMAP_MAX_X) || (j < 0) || (j >= MINIMAP_MAX_Z)) {
                            v = 0;
						}
						
						else {
                            v = ((float)m_data[showLevel].m_revealed[min((int)i+1, MINIMAP_MAX_X-1)][j]) * (1.0f / 255);
						}

						if(flag == 1) {
                            
							int vert = 1;
							_px = verts[vert].p.x - boundaries.left;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}
							
							_px = boundaries.right - verts[vert].p.x;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}
							
							_px = verts[vert].p.y - boundaries.top;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = boundaries.bottom - verts[vert].p.y;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}
						}

						verts[1].color = Color::gray(fl2 ? v * ( 1.0f / 2 ) : v).toBGR();

						oo += v;

						if((i + 1 < 0) || (i + 1 >= MINIMAP_MAX_X) || (j + 1 < 0) || (j + 1 >= MINIMAP_MAX_Z)) {
                            v = 0;
						}
						else {
                            v = ((float)m_data[showLevel].m_revealed[min((int)i+1, MINIMAP_MAX_X-1)][min((int)j+1, MINIMAP_MAX_Z-1)]) * ( 1.0f / 255 );
						}

						if(flag == 1) {
                            
							int vert = 2;
							_px = verts[vert].p.x - boundaries.left;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = boundaries.right - verts[vert].p.x;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = verts[vert].p.y - boundaries.top;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = boundaries.bottom - verts[vert].p.y;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}
						}

						verts[2].color = Color::gray(fl2 ? v * (1.f/2) : v).toBGR();

						oo += v;

						if((i < 0) || (i >= MINIMAP_MAX_X) || (j + 1 < 0) || (j + 1 >= MINIMAP_MAX_Z)) {
                            v = 0;
						}
						else {
                            v = ((float)m_data[showLevel].m_revealed[i][min((int)j+1, MINIMAP_MAX_Z-1)]) * ( 1.0f / 255 );
						}

						if(flag == 1) {
                            
							int vert = 3;
							_px = verts[vert].p.x - boundaries.left;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
                            }

							_px = boundaries.right - verts[vert].p.x;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = verts[vert].p.y - boundaries.top;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}

							_px = boundaries.bottom - verts[vert].p.y;

							if(_px < 0.f) {
                                v = 0.f;
							}
							else if(_px < MOD20) {
                                v *= _px * MOD20DIV;
							}
						}

						verts[3].color = Color::gray(fl2 ? v * (1.f/2) : v).toBGR();

						oo += v;

						if(oo > 0.f) {
                            
							if(fl2) {
							    
								verts[0].p.x += DECALX * Xratio;
								verts[0].p.y += DECALY * Yratio;
								verts[1].p.x += DECALX * Xratio;
								verts[1].p.y += DECALY * Yratio;
								verts[2].p.x += DECALX * Xratio;
								verts[2].p.y += DECALY * Yratio;
								verts[3].p.x += DECALX * Xratio;
								verts[3].p.y += DECALY * Yratio;
							}

							EERIEDRAWPRIM(Renderer::TriangleFan, verts, 4);
						}
					}
				}
			}
		}

		if(flag != 2) {
            
			GRenderer->GetTextureStage(0)->SetWrapMode(TextureStage::WrapRepeat);
			GRenderer->SetRenderState(Renderer::AlphaBlending, false);

			if(showLevel == ARX_LEVELS_GetRealNum(CURRENTLEVEL)) {
				
				// Now Draws Playerpos/angle
				verts[0].color = 0xFFFF0000;
				verts[1].color = 0xFFFF0000;
				verts[2].color = 0xFFFF0000;
				float val;

				if(flag == 1) {
                    val = 6.f;
				}
				else {
                    val = 3.f;
                }

				if(fl2) {
                    val = FL2_PLAYERSIZE;
				}

				float rx = 0.f;
				float ry = -val * 1.8f;
				float rx2 = -val * (1.0f / 2);
				float ry2 = val;
				float rx3 = val * (1.0f / 2);
				float ry3 = val;

				float angle = radians(player.angle.b);
				float ca = EEcos(angle);
				float sa = EEsin(angle);

				verts[0].p.x = (px + rx2 * ca + ry2 * sa) * Xratio;
				verts[0].p.y = (py + ry2 * ca - rx2 * sa) * Yratio;
				verts[1].p.x = (px + rx * ca + ry * sa) * Xratio;
				verts[1].p.y = (py + ry * ca - rx * sa) * Yratio;
				verts[2].p.x = (px + rx3 * ca + ry3 * sa) * Xratio;
				verts[2].p.y = (py + ry3 * ca - rx3 * sa) * Yratio;

				GRenderer->ResetTexture(0);

				if(fl2) {
                    
					GRenderer->SetRenderState(Renderer::AlphaBlending, true);
					verts[0].p.x += DECALX * Xratio;
					verts[0].p.y += DECALY * Yratio;
					verts[1].p.x += DECALX * Xratio;
					verts[1].p.y += DECALY * Yratio;
					verts[2].p.x += DECALX * Xratio;
					verts[2].p.y += DECALY * Yratio;
				}

				EERIEDRAWPRIM(Renderer::TriangleFan, verts);

				if(fl2) {
                    GRenderer->SetRenderState(Renderer::AlphaBlending, false);
				}
			}
		}

		// tsu
		for(size_t lnpc = 1; lnpc < entities.size(); lnpc++) {
			if((entities[lnpc] != NULL) && (entities[lnpc]->ioflags & IO_NPC)) {
				if(entities[lnpc]->_npcdata->life > 0.f) {
					if (!((entities[lnpc]->gameFlags & GFLAG_MEGAHIDE) || (entities[lnpc]->show == SHOW_FLAG_MEGAHIDE))
							&& (entities[lnpc]->show == SHOW_FLAG_IN_SCENE)) {
						if (!(entities[lnpc]->show == SHOW_FLAG_HIDDEN)) {
							if(entities[lnpc]->_npcdata->fDetect >= 0) {
								if(player.Full_Skill_Etheral_Link >= entities[lnpc]->_npcdata->fDetect) {
                                    
									float fpx;
									float fpy;
								
									fpx = sstartx + ((entities[lnpc]->pos.x - 100 + ofx - ofx2) * ( 1.0f / 100 ) * casex
									                 + m_miniOffsetX[CURRENTLEVEL] * ratio * mod_x) / mod_x; 
									fpy = sstarty + ((m_mapMaxY[showLevel] - ofy - ofy2) * ( 1.0f / 100 ) * casey
									                 - (entities[lnpc]->pos.z + 200 + ofy - ofy2) * ( 1.0f / 100 ) * casey + m_miniOffsetY[CURRENTLEVEL] * ratio * mod_z) / mod_z; 

									if(flag == 1) {

										fpx = startx + ((entities[lnpc]->pos.x - 100 + ofx - ofx2) * ( 1.0f / 100 ) * casex
										                + m_miniOffsetX[CURRENTLEVEL] * ratio * mod_x) / mod_x; 
										fpy = starty + ((m_mapMaxY[showLevel] - ofy - ofy2) * ( 1.0f / 100 ) * casey
										                - (entities[lnpc]->pos.z + 200 + ofy - ofy2) * ( 1.0f / 100 ) * casey + m_miniOffsetY[CURRENTLEVEL] * ratio * mod_z) / mod_z;
									}

									float d = fdist(Vec2f(player.pos.x, player.pos.z), Vec2f(entities[lnpc]->pos.x, entities[lnpc]->pos.z));

									if((d <= 800) && (fabs(entities.player()->pos.y - entities[lnpc]->pos.y) < 250.f)) {
                                        
										float col = 1.f;

										if(d > 600.f) {
											col = 1.f - (d - 600.f) * ( 1.0f / 200 );
										}

										if(!fl2) {
											GRenderer->SetRenderState(Renderer::AlphaBlending, true);
											GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
										}
										else {
											GRenderer->SetRenderState(Renderer::AlphaBlending, true);
										}

										if(fl2) {
											fpx += DECALX * Xratio;
											fpy += (DECALY + 15) * Yratio;
										}

										fpx *= Xratio;
										fpy *= Yratio;
										EERIEDrawBitmap(fpx, fpy, 5.f * ratio, 5.f * ratio, 0, m_pTexDetect,
										                Color3f(col, 0, 0).to<u8>());

										if(!fl2) {
											GRenderer->SetRenderState(Renderer::AlphaBlending, false);
										}
									}
								}
							}
						}
                    }
                }
			}
		}

		if(flag == 0) {
			for(size_t i = 0; i < m_mapMarkers.size(); i++) {
				if(m_mapMarkers[i].m_lvl == showLevel + 1) {
                    
					float pos_x = m_mapMarkers[i].m_x * 8 * ratio * ACTIVEBKG->Xmul * casex + startx;
					float pos_y = m_mapMarkers[i].m_y * 8 * ratio * ACTIVEBKG->Zmul * casey + starty;
					float size = 5.f * ratio;
					verts[0].color = 0xFFFF0000;
					verts[1].color = 0xFFFF0000;
					verts[2].color = 0xFFFF0000;
					verts[3].color = 0xFFFF0000;
					verts[0].p.x = (pos_x - size) * Xratio;
					verts[0].p.y = (pos_y - size) * Yratio;
					verts[1].p.x = (pos_x + size) * Xratio;
					verts[1].p.y = (pos_y - size) * Yratio;
					verts[2].p.x = (pos_x + size) * Xratio;
					verts[2].p.y = (pos_y + size) * Yratio;
					verts[3].p.x = (pos_x - size) * Xratio;
					verts[3].p.y = (pos_y + size) * Yratio;
					verts[0].uv = Vec2f::ZERO;
					verts[1].uv = Vec2f::X_AXIS;
					verts[2].uv = Vec2f::ONE;
					verts[3].uv = Vec2f::Y_AXIS;
					
					if(!fl2 && MouseInRect(verts[0].p.x, verts[0].p.y, verts[2].p.x, verts[2].p.y)) {
						if(!m_mapMarkers[i].m_text.empty()) {
						    
							Rect bRect(140, 290, 140 + 205, 358);

							Rect::Num left = checked_range_cast<Rect::Num>((bRect.left) * Xratio);
							Rect::Num right = checked_range_cast<Rect::Num>((bRect.right) * Xratio);
							Rect::Num top = checked_range_cast<Rect::Num>((bRect.top) * Yratio);
							Rect::Num bottom = checked_range_cast<Rect::Num>((bRect.bottom) * Yratio);
							Rect rRect = Rect(left, top, right, bottom);

							long lLengthDraw = ARX_UNICODE_ForceFormattingInRect(hFontInGameNote, m_mapMarkers[i].m_text, rRect);

							DrawBookTextInRect(hFontInGameNote, float(bRect.left), float(bRect.top), float(bRect.right), m_mapMarkers[i].m_text.substr(0, lLengthDraw), Color::none);
						}
					}

					if(m_mapMarkerTexCont == NULL) {
						m_mapMarkerTexCont = TextureContainer::Load("graph/interface/icons/mapmarker");
					}

					GRenderer->SetTexture(0, m_mapMarkerTexCont);

					if(fl2) {
                        
						verts[0].p.x += DECALX * Xratio;
						verts[0].p.y += DECALY * Yratio;
						verts[1].p.x += DECALX * Xratio;
						verts[1].p.y += DECALY * Yratio;
						verts[2].p.x += DECALX * Xratio;
						verts[2].p.y += DECALY * Yratio;
						verts[3].p.x += DECALX * Xratio;
						verts[3].p.y += DECALY * Yratio;
					}

					EERIEDRAWPRIM(Renderer::TriangleFan, verts, 4);
				}
			}
		}
	}
}

void MiniMap::clearMarkerTexCont() {
    m_mapMarkerTexCont = NULL;
}

void MiniMap::mapMarkerInit() {
	m_mapMarkers.clear();
}

int MiniMap::mapMarkerGetID(const std::string &name) {
	
	for(unsigned int i = 0; i < m_mapMarkers.size(); i++) {
		if(m_mapMarkers[i].m_name == name) {
			return i;
		}
	}
	
	return -1;
}

void MiniMap::mapMarkerAdd(float x, float y, int lvl, const std::string &name) {
	
	int num = mapMarkerGetID(name);
	
	if(num >= 0) {
		// Already exists, update it
		m_mapMarkers[num].m_lvl = lvl;
		m_mapMarkers[num].m_x = x;
		m_mapMarkers[num].m_y = y;
		return;
	}
	
	// Else, create one
	MapMarkerData newMMD;
	newMMD.m_lvl = lvl;
	newMMD.m_x = x;
	newMMD.m_y = y;
	newMMD.m_name = name;
	newMMD.m_text = getLocalised(name);
	m_mapMarkers.push_back(newMMD);
}

void MiniMap::mapMarkerRemove(const std::string &name) {
	
	int num = mapMarkerGetID(name);
	
	if(num < 0) {
		return; // Doesn't exist
	}
	
	m_mapMarkers.erase(m_mapMarkers.begin() + num);
}
