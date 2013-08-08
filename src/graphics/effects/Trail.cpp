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

#include "graphics/effects/Trail.h"

#include "core/GameTime.h"
#include "math/Random.h"
#include "graphics/Renderer.h"
#include "graphics/effects/SpellEffects.h"


Trail::Trail(Vec3f & initialPosition)
{
	m_nextPosition = initialPosition;

	int nb = 2048;

	while (nb--)
	{
		truban[nb].actif = 0;
	}

	float col = 0.1f + (rnd() * 0.1f);
	float size = 2.f + (2.f * rnd());
	int taille = Random::get(8, 16);

	m_first = -1;
	m_origin = 0;
	m_size = size;
	m_dec = taille;
	m_r = col;
	m_g = col;
	m_b = col;
	m_r2 = 0.f;
	m_g2 = 0.f;
	m_b2 = 0.f;

	Update();
}

void Trail::SetNextPosition(Vec3f & nextPosition)
{
	m_nextPosition = nextPosition;
}

int Trail::GetFreeRuban()
{
	int nb = 2048;

	while(nb--) {
		if(!truban[nb].actif)
			return nb;
	}

	return -1;
}

void Trail::Update() {
	if(arxtime.is_paused())
		return;

	int num = GetFreeRuban();

	if(num >= 0) {
		truban[num].actif = 1;
		truban[num].pos = m_nextPosition;

		if(m_first < 0) {
			m_first = num;
			truban[num].next = -1;
		} else {
			truban[num].next = m_first;
			m_first = num;
		}

		int nb = 0, oldnum = 0;

		while(num != -1) {
			nb++;
			oldnum = num;
			num = truban[num].next;
		}

		if(nb > m_dec) {
			truban[oldnum].actif = 0;
			num = m_first;
			nb -= 2;

			while(nb--) {
				num = truban[num].next;
			}

			truban[num].next = -1;
		}
	}
}

void Trail::Render()
{
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->ResetTexture(0);

	int num = m_first;
	float size = m_size;
	int dec = m_dec;

	int numsuiv;

	float dsize = size / (float)(dec + 1);
	int r1 = ((int)(m_r * 255.f)) << 16;
	int g1 = ((int)(m_g * 255.f)) << 16;
	int b1 = ((int)(m_b * 255.f)) << 16;
	int rr2 = ((int)(m_r2 * 255.f)) << 16;
	int gg2 = ((int)(m_g2 * 255.f)) << 16;
	int bb2 = ((int)(m_b2 * 255.f)) << 16;
	int dr = (rr2 - r1) / dec;
	int dg = (gg2 - g1) / dec;
	int db = (bb2 - b1) / dec;

	for(;;) {
		numsuiv = truban[num].next;

		if(num >= 0 && numsuiv >= 0) {
			Draw3DLineTex2(truban[num].pos, truban[numsuiv].pos, size,
						   Color(r1 >> 16, g1 >> 16, b1 >> 16, 255),
						   Color((r1 + dr) >> 16, (g1 + dg) >> 16, (b1 + db) >> 16, 255));
			r1 += dr;
			g1 += dg;
			b1 += db;
			size -= dsize;
		} else {
			break;
		}

		num = numsuiv;
	}

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
}
