/*
 * Copyright 2013-2016 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Original source is copyright 2010 - 2011. Alexey Tsoy.
 * http://sourceforge.net/projects/interpreter11/
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef ARX_UTIL_CMDLINE_OPTIONAL_H
#define ARX_UTIL_CMDLINE_OPTIONAL_H

namespace util { namespace cmdline {

/*!
 * This class is a kind of pointer storage.
 * It is used to indicate that option parameter isn't required.
 * \param T Type of the elements.
 */
template <typename T>
struct optional {
	
	optional() : m_ptr(0) { }
	
	template <typename U>
	explicit optional(const U & rh) : m_ptr(new T(rh)) { }
	
	optional(const optional & rh)
		: m_ptr(rh.m_ptr ? new T(*rh.m_ptr) : 0) {
	}
	
	template <typename U>
	optional & operator=(const U & rh) {
		swap(optional(rh));
		return *this;
	}
	
	optional & operator=(const optional & rh) {
		swap(optional(rh));
		return *this;
	}
	
	~optional() {
		delete m_ptr;
	}
	
	/*!
	 * This function exchanges the content of the optional by the content
	 * of optional, which is another optional of the same type.
	 *
	 * \param rh
	 */
	void swap(optional & rh) { // never throws
		T * tmp(m_ptr);
		m_ptr = rh.m_ptr;
		rh.m_ptr = tmp ;
	}
	
	const T & operator *() const { // never throws
		return *m_ptr;
	}
	
	T & operator *() { // never throws
		return const_cast<T &>(static_cast<const optional *>(this)->operator*());
	}
	
	const T * operator->() const { // never throws
		return &**this;
	}
	
	T * operator->() { // never throws
		return &**this;
	}
	
private:
	
	struct dummy { void nonnull() {} };
	typedef void (dummy::*safe_bool)();
	
public:
	
	bool operator!() const {
		return m_ptr == 0;
	}
	
	operator safe_bool() const {
		return (!*this) ? 0 : &dummy::nonnull;
	}
	
private:
	
	T * m_ptr;
	
};

template <typename T>
bool operator==(bool lh, optional<T> const & rh) {
	return (lh && rh) || (!lh && !rh);
}

template <typename T>
bool operator==(optional<T> const & lh , bool rh) {
	return rh == lh;
}

} } // namespace util::cmdline

#endif // ARX_UTIL_CMDLINE_OPTIONAL_H
