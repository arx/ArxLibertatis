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

#ifndef ARX_MATH_RECTANGLE_H
#define ARX_MATH_RECTANGLE_H

#include <algorithm>

#include "math/Vector2.h"

// name Rectangle is used in windows headers
template<class T>
class _Rectangle {
	
public:
	
	// TODO can be removed for C++0x
	class DummyVec2 {
		
	public:
		
		T x;
		T y;
		
		operator Vector2<T>() {
			return Vector2<T>(x, y);
		}
		
		DummyVec2 & operator=(const Vector2<T> & vec) {
			x = vec.x, y = vec.y;
			return *this;
		}
		
	};
	
	typedef T Num;
	typedef std::numeric_limits<T> Limits;
	
	union {
		struct {
			T left;
			T top;
		};
		DummyVec2 origin;
	};
	
	union {
		struct {
			T right;
			T bottom;
		};
		DummyVec2 end;
	};
	
	_Rectangle(const _Rectangle & other) : origin(other.origin), end(other.end) { }
	
	_Rectangle() { }
	
	_Rectangle(T _left, T _top, T _right, T _bottom) : left(_left), top(_top), right(_right), bottom(_bottom) { }
	
	_Rectangle(const Vector2<T> & _origin, T width = T(0), T height = T(0)) : left(_origin.x), top(_origin.y), right(_origin.x + width), bottom(_origin.y + height) { }
	
	_Rectangle(const Vector2<T> & _origin, const Vector2<T> & _end) : left(_origin.x), top(_origin.y), right(_end.x), bottom(_end.y) { }
	
	_Rectangle(T width, T height) : left(T(0)), top(T(0)), right(width), bottom(height) { }
	
	bool operator==(const _Rectangle & o) const {
		return (origin == o.origin && end == o.end);
	}
	
	_Rectangle & operator=(const _Rectangle & other) {
		origin = other.origin, end = other.end;
		return *this;
	}
	
	T width() const {
		return right - left;
	}
	
	T height() const {
		return bottom - top;
	}
	
	_Rectangle operator+(const Vector2<T> & offset) const {
		return _Rectangle(origin + offset, end + offset);
	}
	
	_Rectangle & operator+=(const Vector2<T> & offset) {
		origin += offset, end += offset;
		return *this;
	}
	
	void move(T dx, T dy) {
		left += dx, top += dy, right += dx, bottom += dy;
	}
	
	bool contains(const Vector2<T> & point) const {
		return (point.x >= left && point.x < right && point.y >= top && point.y < bottom);
	}
	
	bool contains(T x, T y) const {
		return (x >= left && x < right && y >= top && y < bottom);
	}
	
	bool contains(const _Rectangle & other) const {
		return (other.left >= left && other.right <= right && other.top >= top && other.bottom <= bottom);
	}
	
	bool overlaps(const _Rectangle & other) const {
		return (left < other.right && other.left < right && top < other.bottom && bottom < other.top);
	}
	
	/*!
	 * Calculate a rectangle contained in both this rectangle and the other rectange.
	 * Assumes that both rectangles are valid.
	 */
	_Rectangle operator&(const _Rectangle & other) const {
		_Rectangle result(std::max(left, other.left), std::max(top, other.top), std::min(right, other.right), std::min(bottom, other.bottom));
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
	_Rectangle operator|(const _Rectangle & other) const {
		return _Rectangle(std::min(left, other.left), std::min(top, other.top), std::max(right, other.right), std::max(bottom, other.bottom));
	}
	
	bool empty() const {
		return (left == right || top == bottom);
	}
	
	bool valid() const {
		return (left <= right && top <= bottom);
	}
	
	Vector2<T> center() const {
		return Vector2<T>(left + (right - left) / 2, top + (bottom - top) / 2);
	}
	
	static const _Rectangle ZERO;
	
};

template<class T> const _Rectangle<T> _Rectangle<T>::ZERO(T(0), T(0), T(0));

#endif // ARX_MATH_RECTANGLE_H
