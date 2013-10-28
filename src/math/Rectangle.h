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

#ifndef ARX_MATH_RECTANGLE_H
#define ARX_MATH_RECTANGLE_H

#include <algorithm>

#include "math/Vector.h"

// name Rectangle is used in windows headers
template<class T>
class Rectangle_ {
	
public:
	
	// TODO can be removed for C++0x
	class DummyVec2 {
		
	public:
		
		T x;
		T y;
		
		typename vec2_traits<T>::type toVec2() const {
			return typename vec2_traits<T>::type(x, y);
		}
		
		DummyVec2 & operator=(const typename vec2_traits<T>::type & vec) {
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
	
	Rectangle_(const Rectangle_ & other) : origin(other.origin), end(other.end) { }
	
	Rectangle_() { }
	
	Rectangle_(T _left, T _top, T _right, T _bottom)
		: left(_left), top(_top), right(_right), bottom(_bottom) { }
	
	Rectangle_(const typename vec2_traits<T>::type & _origin, T width = T(0), T height = T(0))
		: left(_origin.x), top(_origin.y), right(_origin.x + width), bottom(_origin.y + height) { }
	
	Rectangle_(const typename vec2_traits<T>::type & _origin, const typename vec2_traits<T>::type & _end)
		: left(_origin.x), top(_origin.y), right(_end.x), bottom(_end.y) {}
	
	Rectangle_(T width, T height) : left(T(0)), top(T(0)), right(width), bottom(height) { }
	
	bool operator==(const Rectangle_ & o) const {
		return (origin == o.origin && end == o.end);
	}
	
	Rectangle_ & operator=(const Rectangle_ & other) {
		origin = other.origin, end = other.end;
		return *this;
	}
	
	T width() const {
		return right - left;
	}
	
	T height() const {
		return bottom - top;
	}
	
	Rectangle_ operator+(const typename vec2_traits<T>::type & offset) const {
		return Rectangle_(origin + offset, end + offset);
	}
	
	Rectangle_ & operator+=(const typename vec2_traits<T>::type & offset) {
		origin += offset, end += offset;
		return *this;
	}
	
	void move(T dx, T dy) {
		left += dx, top += dy, right += dx, bottom += dy;
	}
    
	bool contains(const typename vec2_traits<T>::type & point) const {
		return (point.x >= left && point.x < right && point.y >= top && point.y < bottom);
	}
	
	bool contains(T x, T y) const {
		return (x >= left && x < right && y >= top && y < bottom);
	}
	
	bool contains(const Rectangle_ & other) const {
		return (other.left >= left && other.right <= right && other.top >= top && other.bottom <= bottom);
	}
	
	bool overlaps(const Rectangle_ & other) const {
		return (left < other.right && other.left < right && top < other.bottom && bottom < other.top);
	}
	
	/*!
	 * Calculate a rectangle contained in both this rectangle and the other rectange.
	 * Assumes that both rectangles are valid.
	 */
	Rectangle_ operator&(const Rectangle_ & other) const {
		Rectangle_ result(
				std::max(left, other.left), std::max(top, other.top),
				std::min(right, other.right), std::min(bottom, other.bottom)
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
	Rectangle_ operator|(const Rectangle_ & other) const {
		return Rectangle_(
			std::min(left, other.left), std::min(top, other.top),
			std::max(right, other.right), std::max(bottom, other.bottom)
		);
	}
	
	bool empty() const {
		return (left == right || top == bottom);
	}
	
	bool valid() const {
		return (left <= right && top <= bottom);
	}
	
	typename vec2_traits<T>::type center() const {
		return typename vec2_traits<T>::type(left + (right - left) / 2, top + (bottom - top) / 2);
	}
	
	static const Rectangle_ ZERO;
	
};

template<class T> const Rectangle_<T> Rectangle_<T>::ZERO(T(0), T(0), T(0), T(0));

#endif // ARX_MATH_RECTANGLE_H
