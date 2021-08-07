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

#include <utility>

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

template <typename Base, typename Adaptor, typename Iterator>
class RangeAdaptor {
	
	Base m_base;
	Adaptor m_adaptor;
	
public:
	
	typedef Iterator iterator;
	typedef typename Iterator::Sentinel sentinel;
	
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
auto filter(Base && base, Filter && filter) {
	typedef FilterIterator<decltype(base.begin()), decltype(base.end()), Filter> Iterator;
	return RangeAdaptor<Base, Filter, Iterator>{ std::forward<Base>(base), std::forward<Filter>(filter) };
}

template <typename Base, typename Transform>
auto transform(Base && base, Transform && transform) {
	typedef TransformIterator<decltype(base.begin()), decltype(base.end()), Transform> Iterator;
	return RangeAdaptor<Base, Transform, Iterator>{ std::forward<Base>(base), std::forward<Transform>(transform) };
}

template <typename Base>
auto dereference(Base && base) {
	return transform(std::forward<Base>(base), [](auto * pointer) -> auto & {
		arx_assert(pointer);
		return *pointer;
	});
}

} // namespace util

#endif // ARX_UTIL_RANGE_H
