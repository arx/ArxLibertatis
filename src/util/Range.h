/*
 * Copyright 2021 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_UTIL_RANGE_H
#define ARX_UTIL_RANGE_H

#include <type_traits>
#include <utility>

#include "math/Types.h"
#include "platform/Platform.h"

namespace util {

template <typename Base, typename End, typename Filter>
class FilterIterator {
	
	Base m_base;
	End m_end;
	Filter m_filter;
	
public:
	
	struct Sentinel { };
	
	FilterIterator(Base base, End end, Filter filter) noexcept
		: m_base(base)
		, m_end(end)
		, m_filter(filter)
	{
		while(m_base != m_end && !m_filter(*m_base)) {
			++m_base;
		}
	}
	
	[[nodiscard]] decltype(auto) operator*() const noexcept {
		return *m_base;
	}
	
	void operator++() noexcept {
		do {
			++m_base;
		} while(m_base != m_end && !m_filter(*m_base));
	}
	
	[[nodiscard]] bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_base == m_end;
	}
	
	[[nodiscard]] bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_base != m_end;
	}
	
};

template <typename Base, typename End, typename Transform>
class TransformIterator {
	
	Base m_base;
	End m_end;
	Transform m_transform;
	
public:
	
	struct Sentinel { };
	
	TransformIterator(Base && base, End end, Transform transform) noexcept
		: m_base(base)
		, m_end(end)
		, m_transform(transform)
	{ }
	
	[[nodiscard]] decltype(auto) operator*() const noexcept {
		return m_transform(*m_base);
	}
	
	void operator++() noexcept {
		++m_base;
	}
	
	[[nodiscard]] bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_base == m_end;
	}
	
	[[nodiscard]] bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_base != m_end;
	}
	
};

template <typename Base, template <typename B, typename E, typename A> typename Iterator, typename Adaptor>
class RangeAdaptor {
	
	Base m_base;
	Adaptor m_adaptor;
	
public:
	
	typedef decltype(std::declval<Base>().begin()) BaseIterator;
	typedef decltype(std::declval<Base>().end()) BaseEnd;
	typedef Iterator<BaseIterator, BaseEnd, Adaptor> iterator;
	typedef typename iterator::Sentinel sentinel;
	
	RangeAdaptor(Base && base, Adaptor && adaptor)
		: m_base(std::forward<Base>(base)), m_adaptor(std::forward<Adaptor>(adaptor))
	{ }
	
	[[nodiscard]] iterator begin() const {
		return { m_base.begin(), m_base.end(), m_adaptor };
	}
	
	[[nodiscard]] static sentinel end() {
		return { };
	}
	
	[[nodiscard]] bool empty() const {
		return begin() == end();
	}
	
};

template <typename Base, typename Filter>
auto filter(Base && base, Filter && filter) -> RangeAdaptor<Base, FilterIterator, Filter> {
	return { std::forward<Base>(base), std::forward<Filter>(filter) };
}

template <typename Base, typename Transform>
auto transform(Base && base, Transform && transform) -> RangeAdaptor<Base, TransformIterator, Transform> {
	return { std::forward<Base>(base), std::forward<Transform>(transform) };
}

template <typename Base>
auto dereference(Base && base) {
	return transform(std::forward<Base>(base), [](auto * pointer) -> auto & {
		arx_assert(pointer);
		return *pointer;
	});
}

template <typename Vector>
struct GridXYIterator {
	
	static_assert(std::is_integral_v<typename Vector::value_type>);
	typedef Vector value_type;
	
	struct Sentinel { };
	
	constexpr GridXYIterator(Vector begin, Vector end) noexcept
		: m_pos(begin)
		, m_end(end)
		, m_minY(begin.y)
	{
		arx_assume(m_pos.x <= m_end.x && m_pos.y <= m_end.y);
		if(m_minY == m_end.y) {
			m_pos = m_end;
		}
	}
	
	[[nodiscard]] Vector operator*() const noexcept {
		arx_assume(m_pos.x < m_end.x && m_pos.y < m_end.y && m_pos.y >= m_minY);
		return m_pos;
	}
	
	[[nodiscard]] const Vector * operator->() const noexcept {
		return &m_pos;
	}
	
	void operator++() noexcept {
		m_pos.y++;
		if(m_pos.y == m_end.y) {
			m_pos.y = m_minY;
			m_pos.x++;
		}
	}
	
	[[nodiscard]] constexpr bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_pos.x == m_end.x;
	}
	
	[[nodiscard]] constexpr bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_pos.x != m_end.x;
	}
	
private:
	
	Vector m_pos;
	Vector m_end;
	typename Vector::value_type m_minY;
	
};

template <typename Vector>
struct GridYXIterator {
	
	static_assert(std::is_integral_v<typename Vector::value_type>);
	typedef Vector value_type;
	
	struct Sentinel { };
	
	constexpr GridYXIterator(Vector begin, Vector end) noexcept
		: m_pos(begin)
		, m_end(end)
		, m_minX(begin.x)
	{
		arx_assume(m_pos.x <= m_end.x && m_pos.y <= m_end.y);
		if(m_minX == m_end.x) {
			m_pos = m_end;
		}
	}
	
	[[nodiscard]] Vector operator*() const noexcept {
		arx_assume(m_pos.x < m_end.x && m_pos.y < m_end.y && m_pos.x >= m_minX);
		return m_pos;
	}
	
	[[nodiscard]] const Vector * operator->() const noexcept {
		return &m_pos;
	}
	
	void operator++() noexcept {
		m_pos.x++;
		if(m_pos.x == m_end.x) {
			m_pos.x = m_minX;
			m_pos.y++;
		}
	}
	
	[[nodiscard]] constexpr bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_pos.y == m_end.y;
	}
	
	[[nodiscard]] constexpr bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_pos.y != m_end.y;
	}
	
private:
	
	Vector m_pos;
	Vector m_end;
	typename Vector::value_type m_minX;
	
};

template <typename Vector, template <typename T> typename Iterator>
struct GridRange {
	
	typedef Iterator<Vector> iterator;
	typedef Vector value_type;
	typedef typename iterator::Sentinel sentinel;
	
	constexpr GridRange(Vector begin, Vector end) noexcept : m_begin(begin), m_end(end) { }
	
	[[nodiscard]] constexpr iterator begin() const noexcept {
		return { m_begin, m_end };
	}
	
	[[nodiscard]] static constexpr sentinel end() noexcept {
		return { };
	}
	
private:
	
	Vector m_begin;
	Vector m_end;
	
};

template <typename Vector>
auto gridXY(Vector begin, Vector end) {
	return GridRange<Vector, GridXYIterator>(begin, end);
}

template <typename Vector>
auto gridYX(Vector begin, Vector end) {
	return GridRange<Vector, GridYXIterator>(begin, end);
}

template <typename Vector>
auto grid(Vector begin, Vector end) {
	return gridXY(begin, end);
}

} // namespace util

#endif // ARX_UTIL_RANGE_H
