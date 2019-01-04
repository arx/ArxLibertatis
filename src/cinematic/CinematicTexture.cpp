/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
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

#include "cinematic/CinematicTexture.h"

#include <stddef.h>
#include <string>
#include <cstdlib>

#include <boost/foreach.hpp>

#include "graphics/Math.h"
#include "graphics/Renderer.h"
#include "graphics/texture/Texture.h"

#include "io/resource/ResourcePath.h"
#include "io/resource/PakReader.h"
#include "io/log/Logger.h"

static const Vec2i cinMaxSize = Vec2i(256, 256);

CinematicBitmap::~CinematicBitmap()
{
	grid.FreeGrille();
}

CinematicBitmap * CreateCinematicBitmap(const res::path & path, int scale) {
	
	std::string name = path.basename();
	if(name.empty()) {
		return 0;
	}
	
	CinematicBitmap * bi = new CinematicBitmap();
	if(!bi)
		return 0;

	LogDebug("loading cinematic texture " << path);

	res::path filename = path;
	bool foundPath = g_resources->getFile(filename.set_ext(".bmp")) != NULL;
	foundPath = foundPath || g_resources->getFile(filename.set_ext("tga"));
	if(!foundPath) {
		LogError << path << " not found";
		delete bi;
		return 0;
	}
	
	Image cinematicImage;
	cinematicImage.load(filename);
	
	Vec2i size = Vec2i(cinematicImage.getWidth(), cinematicImage.getHeight());
	Vec2i nb = size / cinMaxSize;
	
	if(size.x % cinMaxSize.x)
		nb.x++;

	if(size.y % cinMaxSize.y)
		nb.y++;
	
	bi->m_size = size;
	bi->m_count = nb;
	
	bi->grid.AllocGrille(bi->m_count, Vec2f(bi->m_size), Vec2f(((bi->m_size.x > cinMaxSize.x) ? cinMaxSize.x : bi->m_size.x), ((bi->m_size.y > cinMaxSize.y) ? cinMaxSize.y : bi->m_size.y)), scale);

	int h = bi->m_size.y;

	while(nb.y) {

		int h2;

		if((h - cinMaxSize.y) < 0)
			h2 = h;
		else
			h2 = cinMaxSize.y;

		int w = bi->m_size.x;
		int nbxx = nb.x;

		while(nbxx) {
			int w2 = (w - cinMaxSize.x) < 0 ? w : cinMaxSize.x;

			Texture * tex = GRenderer->createTexture();
			tex->create(size_t(w2), size_t(h2), cinematicImage.getFormat());
			tex->getImage().copy(cinematicImage, 0, 0,
			                     size_t(bi->m_size.x - w), size_t(bi->m_size.y - h), size_t(w2), size_t(h2));
			tex->upload();
			
			{
			Vec2i depc = Vec2i((bi->m_count.x - nbxx) * scale, (bi->m_count.y - nb.y) * scale);
			Vec2i tc = Vec2i(scale, scale);
			Vec2i bitmappos = Vec2i(bi->m_size.x - w, bi->m_size.y - h);
			Vec2i bitmapw = Vec2i(w2, h2);
			
			bi->grid.AddQuadUVs(depc, tc, bitmappos, bitmapw, tex);
			}

			w -= cinMaxSize.x;

			nbxx--;
		}

		h -= cinMaxSize.y;
		nb.y--;
	}

	bi->grid.m_scale = scale;

	bi->grid.ReajustUV();

	return bi;
}

bool CinematicGrid::AllocGrille(Vec2i nb, Vec2f t, Vec2f d, int scale) {
	
	m_scale = scale;
	Vec2i oldnb = nb + Vec2i(1);
	nb *= scale;
	Vec2f oldd = d;
	d /= float(scale);
	
	m_nbvertexs = size_t(nb.x + 1) * size_t(nb.y + 1);
	m_vertexs.reserve(m_nbvertexs);
	
	m_count = nb;
	t *= .5f;
	Vec2f dep = -t;
	
	float olddyy = oldd.y;

	while(oldnb.y--) {
		nb.y = scale;

		if(!oldnb.y)
			nb.y = 1;

		while(nb.y--) {
			float olddxx = oldd.x;
			float depxx = dep.x;
			float dxx = d.x;
			int oldnbxx = oldnb.x;

			while(oldnbxx--) {
				nb.x = scale;

				if (!oldnbxx) nb.x = 1;

				while(nb.x--) {
					m_vertexs.push_back(Vec2f(depxx, dep.y));
					depxx += dxx;
				}

				olddxx += oldd.x;

				if(olddxx > (t.x * 2.f)) {
					dxx = t.x - depxx;
					dxx /= float(scale);
				}
				
			}

			dep.y += d.y;
		}
		
			olddyy += oldd.y;

			if(olddyy > (t.y * 2.f)) {
				d.y = t.y - dep.y;
				d.y /= float(scale);
			}
		
	}

	return true;
}

void CinematicGrid::FreeGrille() {
	
	m_vertexs.clear();
	m_uvs.clear();
	m_inds.clear();
	
	BOOST_FOREACH(C_INDEXED & mat, m_mats) {
		delete mat.tex;
	}
	
	m_mats.clear();
}

void CinematicGrid::AddQuadUVs(Vec2i depc, Vec2i tc, Vec2i bitmappos, Vec2i bitmapw, Texture * tex) {
	size_t matIdx = AddMaterial(tex);
	m_mats[matIdx].bitmapdep = bitmappos;
	m_mats[matIdx].bitmap = bitmapw;
	m_mats[matIdx].nbvertexs = (tc.x + 1) * (tc.y + 1);
	
	float v = 0.f;
	float du = 0.999999f / float(tc.x);
	float dv = 0.999999f / float(tc.y);
	tc.y++;
	tc.x++;
	int tcyy = tc.y;
	int depcyy = depc.y;

	while(tcyy--) {
		float u = 0.f;
		int depcxx = depc.x;
		int tcxx = tc.x;

		while(tcxx--) {
			int i0, i1, i2, i3;
			GetIndNumCube(depcxx, depcyy, &i0, &i1, &i2, &i3);
			C_UV cuv;
			cuv.indvertex = i0;
			cuv.uv.x = u;
			cuv.uv.y = v;
			m_uvs.push_back(cuv);
			depcxx++;
			u += du;
		}

		depcyy++;
		v += dv;
	}

	tc.x--;
	tc.y--;

	while(tc.y--) {
		int depcxx = depc.x;
		int tcxx = tc.x;

		while(tcxx--) {
			int i0, i1, i2, i3;
			GetIndNumCube(depcxx, depc.y, &i0, &i1, &i2, &i3);

			AddPoly(matIdx, i0, i1, i2);
			AddPoly(matIdx, i1, i2, i3);
			depcxx++;
		}

		depc.y++;
	}
}

void CinematicGrid::ReajustUV() {
	
	C_UV * uvs = m_uvs.data();
	
	for(std::vector<C_INDEXED>::iterator mat = m_mats.begin(); mat != m_mats.end(); ++mat) {
		
		Texture * tex = mat->tex;
		if(!tex) {
			return;
		}
		
		Vec2i size = tex->getStoredSize();
		
		if(size.x != mat->bitmap.x || size.y != mat->bitmap.y) {
			float dx = (0.999999f * float(mat->bitmap.x)) / float(size.x);
			float dy = (0.999999f * float(mat->bitmap.y)) / float(size.y);

			int nb2 = mat->nbvertexs;

			while(nb2--) {
				uvs->uv.x *= dx;
				uvs->uv.y *= dy;
				uvs++;
			}
		} else {
			uvs += mat->nbvertexs;
		}
	}
}

void CinematicGrid::GetIndNumCube(int cx, int cy, int * i1, int * i2, int * i3, int * i4) {
	*i1 = cy * (m_count.x + 1) + cx;
	*i2 = *i1 + 1;
	*i3 = *i1 + (m_count.x + 1);
	*i4 = *i3 + 1;
}

size_t CinematicGrid::AddMaterial(Texture * tex) {
	
	size_t matIdx = m_mats.size();
	m_mats.resize(matIdx + 1);
	
	int deb = (matIdx == 0) ? 0 : m_mats[matIdx - 1].startind + m_mats[matIdx - 1].nbind;
	
	m_mats[matIdx].tex = tex;
	m_mats[matIdx].startind = deb;
	m_mats[matIdx].nbind = 0;

	return matIdx;
}

void CinematicGrid::AddPoly(size_t matIdx, int i0, int i1, int i2) {
	
	C_IND ind;
	ind.i1 = checked_range_cast<unsigned short>(i0);
	ind.i2 = checked_range_cast<unsigned short>(i1);
	ind.i3 = checked_range_cast<unsigned short>(i2);
	m_inds.push_back(ind);
	
	m_mats[matIdx].nbind += 3;
}
