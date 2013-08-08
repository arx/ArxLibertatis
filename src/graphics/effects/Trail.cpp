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
#include "ai/Paths.h" // TODO remove this
#include "math/Random.h"
#include "graphics/Renderer.h"
#include "graphics/effects/SpellEffects.h"

// RUBAN
void Trail::Create(int _iNumThrow, int _iDuration)
{
	iNumThrow = _iNumThrow;

	key = 1;
	duration = _iDuration;
	currduration = 0;

	int nb = 2048;

	while (nb--)
	{
		truban[nb].actif = 0;
	}

	float col = 0.1f + (rnd() * 0.1f);
	float size = 2.f + (2.f * rnd());
	int taille = Random::get(8, 16);
	AddRubanDef(0, size, taille, col, col, col, 0.f, 0.f, 0.f);

}

void Trail::AddRubanDef(int origin, float size, int dec, float r, float g, float b,
						 float r2, float g2, float b2) {

	m_first = -1;
	m_origin = origin;
	m_size = size;
	m_dec = dec;
	m_r = r;
	m_g = g;
	m_b = b;
	m_r2 = r2;
	m_g2 = g2;
	m_b2 = b2;
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

void Trail::AddRuban(int * f, int dec) {

	int num = GetFreeRuban();

	if(num >= 0) {
		truban[num].actif = 1;
		truban[num].pos = Thrown[iNumThrow].position;

		if(*f < 0) {
			*f = num;
			truban[num].next = -1;
		} else {
			truban[num].next = *f;
			*f = num;
		}

		int nb = 0, oldnum = 0;

		while(num != -1) {
			nb++;
			oldnum = num;
			num = truban[num].next;
		}

		if(nb > dec) {
			truban[oldnum].actif = 0;
			num = *f;
			nb -= 2;

			while(nb--) {
				num = truban[num].next;
			}

			truban[num].next = -1;
		}
	}
}

void Trail::Update() {
	if(arxtime.is_paused())
		return;

	AddRuban(&m_first, m_dec);
}

void Trail::DrawRuban(int num, float size, int dec, float r, float g, float b,
					   float r2, float g2, float b2) {

	int numsuiv;

	float dsize = size / (float)(dec + 1);
	int r1 = ((int)(r * 255.f)) << 16;
	int g1 = ((int)(g * 255.f)) << 16;
	int b1 = ((int)(b * 255.f)) << 16;
	int rr2 = ((int)(r2 * 255.f)) << 16;
	int gg2 = ((int)(g2 * 255.f)) << 16;
	int bb2 = ((int)(b2 * 255.f)) << 16;
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
}

void Trail::Render()
{
	GRenderer->SetCulling(Renderer::CullNone);
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendOne);
	GRenderer->ResetTexture(0);

	this->DrawRuban(m_first,
					m_size,
					m_dec,
					m_r, m_g, m_b,
					m_r2, m_g2, m_b2);

	GRenderer->SetRenderState(Renderer::AlphaBlending, false);
	GRenderer->SetBlendFunc(Renderer::BlendOne, Renderer::BlendZero);
}
