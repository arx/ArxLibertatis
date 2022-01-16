/*
 * Copyright 2018-2021 Arx Libertatis Team (see the AUTHORS file)
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
#ifndef ARX_SCENE_TILES_H
#define ARX_SCENE_TILES_H

#include <bitset>
#include <utility>
#include <vector>

#include "ai/Anchors.h"
#include "graphics/GraphicsTypes.h"
#include "math/Rectangle.h"
#include "math/Types.h"
#include "platform/Platform.h"
#include "util/Range.h"

struct EERIE_LIGHT;

struct BackgroundTileData {
	
	std::vector<EERIEPOLY> polydata;
	std::vector<EERIEPOLY *> polyin;
	float maxy;
	
	BackgroundTileData()
		: maxy(0.f)
	{ }
	
};

constexpr short MAX_BKGX = 160;
constexpr short MAX_BKGZ = 160;

constexpr Vec2f g_backgroundTileSize = Vec2f(100, 100);

struct BackgroundData {
	
	static constexpr Vec2s m_size = Vec2s(MAX_BKGX, MAX_BKGZ);
	static constexpr Vec2f m_mul = 1.f / g_backgroundTileSize;
	
private:
	
	std::bitset<MAX_BKGX * MAX_BKGZ> m_activeTiles;
	BackgroundTileData m_tileData[MAX_BKGX][MAX_BKGZ];
	std::vector<EERIE_LIGHT *> m_tileLights[MAX_BKGX][MAX_BKGZ];
	
	//! Tile data accessor
	template <typename T>
	class TileView {
		
		T * m_base;
		
		[[nodiscard]] auto & data() const noexcept {
			arx_assume(x >= 0 && x < m_size.x && y >= 0 && y < m_size.y);
			return m_base->m_tileData[x][y];
		}
		
	public:
		
		const s16 x;
		const s16 y;
		
		TileView(T * base, Vec2s tile) : m_base(base), x(tile.x), y(tile.y) { }
		
		[[nodiscard]] Vec2s index() const noexcept {
			return { x, y };
		}
		[[nodiscard]] operator Vec2s() const noexcept {
			return index();
		}
		[[nodiscard]] operator Vec2i() const noexcept {
			return index();
		}
		[[nodiscard]] explicit operator Vec2f() const noexcept {
			return index();
		}
		
		[[nodiscard]] Rectf bounds() const noexcept {
			Vec2f min = Vec2f(index()) * g_backgroundTileSize;
			Vec2f max = min + g_backgroundTileSize;
			return { min, max };
		}
		
		[[nodiscard]] Vec2f center() const noexcept {
			return bounds().center();
		}
		
		[[nodiscard]] bool valid() const noexcept {
			return m_base->isTileValid(*this);
		}
		[[nodiscard]] explicit operator bool() const noexcept {
			return valid();
		}
		
		[[nodiscard]] bool active() const noexcept {
			arx_assume(x >= 0 && x < m_size.x && y >= 0 && y < m_size.y);
			return m_base->m_activeTiles.test(size_t(x) * size_t(MAX_BKGZ) + size_t(y));
		}
		void setActive() noexcept {
			arx_assume(x >= 0 && x < m_size.x && y >= 0 && y < m_size.y);
			m_base->m_activeTiles.set(size_t(x) * size_t(MAX_BKGZ) + size_t(y));
		}
		
		[[nodiscard]] auto & polygons() const noexcept {
			return data().polydata;
		}
		
		[[nodiscard]] auto intersectingPolygons() const noexcept {
			return util::dereference(data().polyin);
		}
		
		[[nodiscard]] auto & maxY() const noexcept {
			return data().maxy;
		}
		
		[[nodiscard]] auto & lights() const noexcept {
			arx_assume(x >= 0 && x < m_size.x && y >= 0 && y < m_size.y);
			return m_base->m_tileLights[x][y];
		}
		
		friend struct BackgroundData;
		
	};
	
public:
	
	[[nodiscard]] auto get(Vec2s tile) {
		return TileView(this, tile);
	}
	[[nodiscard]] auto get(Vec2s tile) const {
		return TileView(this, tile);
	}
	
	[[nodiscard]] auto getTile(Vec3f pos) {
		return get(getTileIndex(pos));
	}
	[[nodiscard]] auto getTile(Vec3f pos) const {
		return get(getTileIndex(pos));
	}
	
	[[nodiscard]] static bool isTileValid(Vec2s tile) noexcept {
		return tile.x >= 0 && tile.x < m_size.x && tile.y >= 0 && tile.y < m_size.y;
	}
	
	void resetActiveTiles() noexcept {
		m_activeTiles.reset();
	}
	
	bool isInActiveTile(Vec3f pos) const noexcept {
		auto tile = getTile(pos);
		return tile.valid() && tile.active();
	}
	
	bool isNearActiveTile(Vec3f pos, float extend = std::max(g_backgroundTileSize.x, g_backgroundTileSize.y)) noexcept;
	
private:
	
	[[nodiscard]] static Vec2s getTileIndex(const Vec3f & pos) noexcept {
		return Vec2s(s16(pos.x * m_mul.x), s16(pos.z * m_mul.y));
	}
	
	//! Returns an iterable over all valid tile indices
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] util::GridRange<Vec2s, Iterator> indices() const noexcept {
		return { Vec2s(0), m_size };
	}
	
	//! Returns an iterable over all valid tile indices in [begin.x, end.x) × [begin.y, end.y)
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] util::GridRange<Vec2s, Iterator> indicesIn(Vec2s begin, Vec2s end) const noexcept {
		typedef Vec2s::value_type T;
		return { Vec2s(glm::clamp(begin.x, T(0), m_size.x), glm::clamp(begin.y, T(0), m_size.y)),
		         Vec2s(glm::clamp(end.x, T(0), m_size.x), glm::clamp(end.y, T(0), m_size.y)) };
	}
	
	//! Returns an iterable over all valid tile indices in around and including the given tile
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] util::GridRange<Vec2s, Iterator> indicesAround(Vec2s tile, u16 extend) const noexcept {
		return indicesIn(tile - Vec2s(extend), tile + Vec2s(extend) + Vec2s(1));
	}
	
	//! Returns an iterable over all valid tile indices in around the given position
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] util::GridRange<Vec2s, Iterator> indicesAround(Vec3f pos, float extend) const noexcept {
		return indicesIn(getTileIndex(pos - Vec3f(extend)), getTileIndex(pos + Vec3f(extend)) + Vec2s(1));
	}
	
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesForIndices(util::GridRange<Vec2s, Iterator> range) noexcept {
		return util::transform(std::move(range), [this](Vec2s index) {
			arx_assume(index.x >= 0 && index.x < m_size.x && index.y >= 0 && index.y < m_size.y);
			return get(index);
		});
	}
	
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesForIndices(util::GridRange<Vec2s, Iterator> range) const noexcept {
		return util::transform(std::move(range), [this](Vec2s index) {
			arx_assume(index.x >= 0 && index.x < m_size.x && index.y >= 0 && index.y < m_size.y);
			return get(index);
		});
	}
	
public:
	
	//! Returns an iterable over all valid tiles
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tiles() noexcept {
		return tilesForIndices(indices<Iterator>());
	}
	//! Returns an iterable over all valid tiles
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tiles() const noexcept {
		return tilesForIndices(indices<Iterator>());
	}
	
	//! Returns an iterable over all valid tile indices in around and including the given tile index
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesIn(Vec2s begin, Vec2s end) noexcept {
		return tilesForIndices(indicesIn<Iterator>(begin, end));
	}
	//! Returns an iterable over all valid tile indices in around and including the given tile index
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesIn(Vec2s begin, Vec2s end) const noexcept {
		return tilesForIndices(indicesIn<Iterator>(begin, end));
	}
	
	//! Returns an iterable over all valid tile indices in around and including the given tile index
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesAround(Vec2s tile, u16 extend) noexcept {
		return tilesForIndices(indicesAround<Iterator>(tile, extend));
	}
	//! Returns an iterable over all valid tile indices in around and including the given tile index
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesAround(Vec2s tile, u16 extend) const noexcept {
		return tilesForIndices(indicesAround<Iterator>(tile, extend));
	}
	
	//! Returns an iterable over all valid tile indices in around the given position
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesAround(Vec3f pos, float extend) noexcept {
		return tilesForIndices(indicesAround<Iterator>(pos, extend));
	}
	//! Returns an iterable over all valid tile indices in around the given position
	template <template <typename T> typename Iterator = util::GridXYIterator>
	[[nodiscard]] auto tilesAround(Vec3f pos, float extend) const noexcept {
		return tilesForIndices(indicesAround<Iterator>(pos, extend));
	}
	
	void clear();
	
	void computeIntersectingPolygons();
	
	size_t countVertices();
	
};

extern BackgroundData * g_tiles;

#endif // ARX_SCENE_TILES_H
