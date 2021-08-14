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

#ifndef ARX_MATH_RECTANGLE_H
#define ARX_MATH_RECTANGLE_H

#include <algorithm>

#include "math/Vector.h"

// name Rectangle is used in windows headers
template <class T>
class Rectangle_ {
	
public:
	
	typedef T Num;
	typedef std::numeric_limits<T> Limits;
	typedef typename vec_traits<T, 2>::type Vec2;
	
	T left;
	T top;
	T right;
	T bottom;
	
	template <class U>
	explicit constexpr Rectangle_(Rectangle_<U> const & other) noexcept
		: left(T(other.left))
		, top(T(other.top))
		, right(T(other.right))
		, bottom(T(other.bottom))
	{ }
	
	constexpr Rectangle_() noexcept = default;
	
	constexpr Rectangle_(T _left, T _top, T _right, T _bottom) noexcept
		: left(_left)
		, top(_top)
		, right(_right)
		, bottom(_bottom)
	{ }
	
	constexpr Rectangle_(const Vec2 & _origin, T width, T height) noexcept
		: left(_origin.x)
		, top(_origin.y)
		, right(_origin.x + width)
		, bottom(_origin.y + height)
	{ }
	
	constexpr Rectangle_(const Vec2 & topLeft, const Vec2 & bottomRight) noexcept
		: left(topLeft.x)
		, top(topLeft.y)
		, right(bottomRight.x)
		, bottom(bottomRight.y)
	{ }
	
	constexpr Rectangle_(T width, T height) noexcept
		: left(T(0))
		, top(T(0))
		, right(width)
		, bottom(height)
	{ }
	
	[[nodiscard]] constexpr bool operator==(const Rectangle_ & o) const noexcept {
		return left   == o.left
		    && top    == o.top
		    && right  == o.right
		    && bottom == o.bottom;
	}
	
	[[nodiscard]] constexpr T width() const noexcept {
		return right - left;
	}
	
	[[nodiscard]] constexpr T height() const noexcept {
		return bottom - top;
	}
	
	constexpr Rectangle_ operator+(const Vec2 & offset) const noexcept {
		return Rectangle_(topLeft() + offset, bottomRight() + offset);
	}
	
	constexpr Rectangle_ & operator+=(const Vec2 & offset) noexcept {
		left   += offset.x;
		top    += offset.y;
		right  += offset.x;
		bottom += offset.y;
		
		return *this;
	}
	
	constexpr void move(T dx, T dy) noexcept {
		left   += dx;
		top    += dy;
		right  += dx;
		bottom += dy;
	}
	
	constexpr void move(const Vec2 & offset) noexcept {
		move(offset.x, offset.y);
	}
	
	constexpr void moveTo(const Vec2 & position) noexcept {
		move(position - topLeft());
	}
	
	[[nodiscard]] constexpr bool contains(const Vec2 & point) const noexcept {
		return point.x >= left
		    && point.x <  right
		    && point.y >= top
		    && point.y <  bottom;
	}
	
	[[nodiscard]] constexpr bool contains(T x, T y) const noexcept {
		return x >= left
		    && x <  right
		    && y >= top
		    && y <  bottom;
	}
	
	[[nodiscard]] constexpr bool contains(const Rectangle_ & other) const noexcept {
		return other.left   >= left
		    && other.right  <= right
		    && other.top    >= top
		    && other.bottom <= bottom;
	}
	
	[[nodiscard]] constexpr bool overlaps(const Rectangle_ & other) const noexcept {
		return left       < other.right
		    && other.left < right
		    && top        < other.bottom
		    && bottom     > other.top;
	}
	
	/*!
	 * Calculate a rectangle contained in both this rectangle and the other rectangle.
	 * Assumes that both rectangles are valid.
	 */
	[[nodiscard]] constexpr Rectangle_ operator&(const Rectangle_ & other) const noexcept {
		Rectangle_ result(
			std::max(left,   other.left),
			std::max(top,    other.top),
			std::min(right,  other.right),
			std::min(bottom, other.bottom)
		);
		if(result.left > result.right) {
			result.left = result.right = T(0);
		}
		if(result.top > result.bottom) {
			result.top = result.bottom = T(0);
		}
		return result;
	}
	
	/*!
	 * Calculate a bounding rectangle containing both this rectangle and the other rectangle.
	 * Assumes that both rectangles are valid.
	 */
	[[nodiscard]] constexpr Rectangle_ operator|(const Rectangle_ & other) const noexcept {
		return Rectangle_(
			std::min(left,   other.left),
			std::min(top,    other.top),
			std::max(right,  other.right),
			std::max(bottom, other.bottom)
		);
	}
	
	[[nodiscard]] constexpr bool empty() const noexcept {
		return (left == right || top == bottom);
	}
	
	[[nodiscard]] constexpr bool isValid() const noexcept {
		return left < right && top < bottom;
	}
	
	
	[[nodiscard]] constexpr Vec2 topLeft() const noexcept {
		return Vec2(left, top);
	}
	[[nodiscard]] constexpr Vec2 topCenter() const noexcept {
		return Vec2(left + (right - left) / 2, top);
	}
	[[nodiscard]] constexpr Vec2 topRight() const noexcept {
		return Vec2(right, top);
	}
	
	[[nodiscard]] constexpr Vec2 centerLeft() const noexcept {
		return Vec2(left, top + (bottom - top) / 2);
	}
	[[nodiscard]] constexpr Vec2 center() const noexcept {
		return Vec2(left + (right - left) / 2, top + (bottom - top) / 2);
	}
	[[nodiscard]] constexpr Vec2 centerRight() const noexcept {
		return Vec2(right, top + (bottom - top) / 2);
	}
	
	[[nodiscard]] constexpr Vec2 bottomLeft() const noexcept {
		return Vec2(left, bottom);
	}
	[[nodiscard]] constexpr Vec2 bottomCenter() const noexcept {
		return Vec2(left + (right - left) / 2, bottom);
	}
	[[nodiscard]] constexpr Vec2 bottomRight() const noexcept {
		return Vec2(right, bottom);
	}
	
	
	[[nodiscard]] constexpr Vec2 size() const noexcept {
		return Vec2(right - left, bottom - top);
	}
	
	static const Rectangle_ ZERO;
	
};

template <class T> const Rectangle_<T> Rectangle_<T>::ZERO(T(0), T(0), T(0), T(0));

#endif // ARX_MATH_RECTANGLE_H
