/*
 * Copyright 2021-2022 Arx Libertatis Team (see the AUTHORS file)
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

#include <boost/range/counting_range.hpp>

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
	
	TransformIterator(Base base, End end, Transform transform) noexcept
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

template <typename Base>
auto filter(Base && base) {
	return filter(std::forward<Base>(base), [](const auto & entry) {
		return !!entry;
	});
}

template <typename Base, typename Transform>
auto transform(Base && base, Transform && transform) -> RangeAdaptor<Base, TransformIterator, Transform> {
	return { std::forward<Base>(base), std::forward<Transform>(transform) };
}

template <typename Base>
auto dereference(Base && base) {
	return transform(std::forward<Base>(base), [](auto & pointer) -> auto & {
		arx_assert(pointer);
		return *pointer;
	});
}

template <typename Base>
auto nonnull(Base && base) {
	return dereference(filter(std::forward<Base>(base)));
}

template <typename Container>
class StableIndexedIterator {
	
	Container * m_container;
	size_t m_i = 0;
	
public:
	
	class Sentinel { };
	
	explicit constexpr StableIndexedIterator(const Container * container) noexcept
		: m_container(container)
	{ }
	
	[[nodiscard]] constexpr auto & operator*() const noexcept {
		return (*m_container)[m_i];
	}
	
	constexpr void operator++() noexcept {
		++m_i;
	}
	
	[[nodiscard]] constexpr bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_i >= m_container->size();
	}
	
	[[nodiscard]] constexpr bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_i < m_container->size();
	}
	
};

template <typename Container>
class StableIndexedRange {
	
	Container m_container;
	
public:
	
	typedef StableIndexedIterator<std::remove_reference_t<Container>> iterator;
	typedef typename iterator::Sentinel sentinel;
	
	explicit constexpr StableIndexedRange(Container && container) noexcept
		: m_container(std::forward<Container>(container))
	{ }
	
	[[nodiscard]] constexpr iterator begin() const noexcept {
		return iterator(&m_container);
	}
	
	[[nodiscard]] constexpr static sentinel end() noexcept {
		return { };
	}
	
	[[nodiscard]] constexpr bool empty() const noexcept {
		return begin() == end();
	}
	
};

template <typename Base>
auto entries(Base && base) {
	return nonnull(StableIndexedRange<Base>(std::forward<Base>(base)));
}

template <typename Container>
auto indices(const Container & container) {
	return boost::counting_range(size_t(0), std::size(container));
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
		return &**this;
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
		return &**this;
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

template <typename Vector>
struct GridDiagonalIterator {
	
	static_assert(std::is_integral_v<typename Vector::value_type>);
	typedef Vector value_type;
	
	struct Sentinel { };
	
	constexpr GridDiagonalIterator(Vector begin, Vector end) noexcept
		: m_pos(begin)
		, m_begin(begin)
		, m_end(end)
	{
		arx_assume(m_pos.x <= m_end.x && m_pos.y <= m_end.y);
		if(m_begin.x == m_end.x) {
			m_pos = m_end;
		}
	}
	
	[[nodiscard]] Vector operator*() const noexcept {
		arx_assume(m_pos.x < m_end.x && m_pos.y < m_end.y && m_pos.x >= m_begin.x && m_pos.y >= m_begin.y);
		return m_pos;
	}
	
	[[nodiscard]] const Vector * operator->() const noexcept {
		return &**this;
	}
	
	void operator++() noexcept {
		m_pos.x++;
		m_pos.y--;
		if(m_pos.y < m_begin.y || m_pos.x >= m_end.x) {
			m_pos.y = m_pos.y + (m_pos.x - m_begin.x) + 1;
			m_pos.x = m_begin.x;
		}
	}
	
	[[nodiscard]] constexpr bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_pos.y >= m_end.y;
	}
	
	[[nodiscard]] constexpr bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_pos.y < m_end.y;
	}
	
private:
	
	Vector m_pos;
	Vector m_begin;
	Vector m_end;
	
};

template <typename Vector>
struct GridXYZIterator {
	
	static_assert(std::is_integral_v<typename Vector::value_type>);
	typedef Vector value_type;
	
	struct Sentinel { };
	
	constexpr GridXYZIterator(Vector begin, Vector end) noexcept
		: m_pos(begin)
		, m_end(end)
		, m_minY(begin.y)
		, m_minZ(begin.z)
	{
		arx_assume(m_pos.x <= m_end.x && m_pos.y <= m_end.y && m_pos.z <= m_end.z);
		if(m_minY == m_end.y || m_minZ == m_end.z) {
			m_pos = m_end;
		}
	}
	
	[[nodiscard]] Vector operator*() const noexcept {
		arx_assume(m_pos.x < m_end.x && m_pos.y < m_end.y && m_pos.z < m_end.z
		           && m_pos.y >= m_minY && m_pos.z >= m_minZ);
		return m_pos;
	}
	
	[[nodiscard]] const Vector * operator->() const noexcept {
		return &**this;
	}
	
	void operator++() noexcept {
		m_pos.z++;
		if(m_pos.z == m_end.z) {
			m_pos.z = m_minZ;
			m_pos.y++;
			if(m_pos.y == m_end.y) {
				m_pos.y = m_minY;
				m_pos.x++;
			}
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
	typename Vector::value_type m_minZ;
	
};

template <typename Vector>
struct GridZXYIterator {
	
	static_assert(std::is_integral_v<typename Vector::value_type>);
	typedef Vector value_type;
	
	struct Sentinel { };
	
	constexpr GridZXYIterator(Vector begin, Vector end) noexcept
		: m_pos(begin)
		, m_end(end)
		, m_minX(begin.x)
		, m_minY(begin.y)
	{
		arx_assume(m_pos.x <= m_end.x && m_pos.y <= m_end.y && m_pos.z <= m_end.z);
		if(m_minX == m_end.x || m_minY == m_end.y) {
			m_pos = m_end;
		}
	}
	
	[[nodiscard]] Vector operator*() const noexcept {
		arx_assume(m_pos.x < m_end.x && m_pos.y < m_end.y && m_pos.z < m_end.z
		           && m_pos.x >= m_minX && m_pos.y >= m_minY);
		return m_pos;
	}
	
	[[nodiscard]] const Vector * operator->() const noexcept {
		return &**this;
	}
	
	void operator++() noexcept {
		m_pos.y++;
		if(m_pos.y == m_end.y) {
			m_pos.y = m_minY;
			m_pos.x++;
			if(m_pos.x == m_end.x) {
				m_pos.x = m_minX;
				m_pos.z++;
			}
		}
	}
	
	[[nodiscard]] constexpr bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_pos.z == m_end.z;
	}
	
	[[nodiscard]] constexpr bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_pos.z != m_end.z;
	}
	
private:
	
	Vector m_pos;
	Vector m_end;
	typename Vector::value_type m_minX;
	typename Vector::value_type m_minY;
	
};

template <typename Vector>
struct GridZYXIterator {
	
	static_assert(std::is_integral_v<typename Vector::value_type>);
	typedef Vector value_type;
	
	struct Sentinel { };
	
	constexpr GridZYXIterator(Vector begin, Vector end) noexcept
		: m_pos(begin)
		, m_end(end)
		, m_minX(begin.x)
		, m_minY(begin.y)
	{
		arx_assume(m_pos.x <= m_end.x && m_pos.y <= m_end.y && m_pos.z <= m_end.z);
		if(m_minX == m_end.x || m_minY == m_end.y) {
			m_pos = m_end;
		}
	}
	
	[[nodiscard]] Vector operator*() const noexcept {
		arx_assume(m_pos.x < m_end.x && m_pos.y < m_end.y && m_pos.z < m_end.z
		           && m_pos.x >= m_minX && m_pos.y >= m_minY);
		return m_pos;
	}
	
	[[nodiscard]] const Vector * operator->() const noexcept {
		return &**this;
	}
	
	void operator++() noexcept {
		m_pos.x++;
		if(m_pos.x == m_end.x) {
			m_pos.x = m_minX;
			m_pos.y++;
			if(m_pos.y == m_end.y) {
				m_pos.y = m_minY;
				m_pos.z++;
			}
		}
	}
	
	[[nodiscard]] constexpr bool operator==(Sentinel /* sentinel */) const noexcept {
		return m_pos.z == m_end.z;
	}
	
	[[nodiscard]] constexpr bool operator!=(Sentinel /* sentinel */) const noexcept {
		return m_pos.z != m_end.z;
	}
	
private:
	
	Vector m_pos;
	Vector m_end;
	typename Vector::value_type m_minX;
	typename Vector::value_type m_minY;
	
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
auto gridXYZ(Vector begin, Vector end) {
	return GridRange<Vector, GridXYZIterator>(begin, end);
}

template <typename Vector>
auto gridZXY(Vector begin, Vector end) {
	return GridRange<Vector, GridZXYIterator>(begin, end);
}

template <typename Vector>
auto gridZYX(Vector begin, Vector end) {
	return GridRange<Vector, GridZYXIterator>(begin, end);
}

template <typename T, glm::qualifier Q>
auto grid(glm::vec<2, T, Q> begin, glm::vec<2, T, Q> end) {
	return gridXY(begin, end);
}

template <typename T, glm::qualifier Q>
auto grid(glm::vec<3, T, Q> begin, glm::vec<3, T, Q> end) {
	return gridXYZ(begin, end);
}

template <typename Range, typename Predicate>
auto unordered_remove_if(Range & range, Predicate predicate) {
	
	auto a = range.begin();
	auto b = range.end();
	
	while(a != b) {
		
		while(a != b && !predicate(*a)) {
			a++;
		}
		if(a == b) {
			break;
		}
		arx_assert(predicate(*a));
		
		do {
			b--;
		} while(a != b && predicate(*b));
		if(a == b) {
			break;
		}
		arx_assert(!predicate(*b));
		
		*a = std::move(*b);
		a++;
		
	}
	
	range.erase(a, range.end());
	
}

template <typename Range, typename It>
auto unordered_erase(Range & range, It it) {
	
	if(it + 1 != range.end()) {
		*it = std::move(*(range.end() - 1));
	}
	
	range.resize(range.size() - 1);
	
}

} // namespace util

#endif // ARX_UTIL_RANGE_H
