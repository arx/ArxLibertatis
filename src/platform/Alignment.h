/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
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

/*!
 * \file
 *
 * Utilities for aligning heap-allocated objects.
 */
#ifndef ARX_PLATFORM_ALIGNMENT_H
#define ARX_PLATFORM_ALIGNMENT_H

#include <cstddef>
#include <new>
#include <memory>
#include <limits>
#include <utility>

#include <boost/preprocessor/punctuation/comma.hpp>

#include "platform/PlatformConfig.h"

#if ARX_HAVE_CXX11_INTEGRAL_CONSTANT
#include <type_traits>
#endif

#if !ARX_HAVE_CXX11_MAX_ALIGN_T
#include <boost/integer/static_min_max.hpp>
#endif

#include "platform/Platform.h"


/*!
 * \def ARX_ALIGNOF(T)
 * \brief Get the required alignment of a type.
 */
#if ARX_HAVE_CXX11_ALIGNOF
	#define ARX_ALIGNOF(T) alignof(T)
#elif ARX_HAVE_GCC_ALIGNOF
	#define ARX_ALIGNOF(T) __alignof__(T)
#elif ARX_HAVE_MSVC_ALIGNOF
	#define ARX_ALIGNOF(T) __alignof(T)
#else
	#error "No way to determine required alignment!"
#endif

/*!
 * \def ARX_ALIGNAS(N)
 * \brief Set the required alignment of a type or member variable.
 *
 * Usage:
 * \code struct ARX_ALIGNAS(128) Name { … } \endcode
 *
 * \note Because of __declspec(align) restrictions / incomplete alignas support in MSVC,
 *       N must be a number literal - it can *not* be an arbitrary constant expression
 *       like \ref ARX_ALIGNOF(T).
 */
#if ARX_HAVE_CXX11_ALIGNAS
	#define ARX_ALIGNAS(N) alignas(N)
#elif ARX_HAVE_ATTRIBUTE_ALIGNED
	#define ARX_ALIGNAS(N) __attribute__((aligned(N)))
#elif ARX_HAVE_DECLSPEC_ALIGN
	#define ARX_ALIGNAS(N) __declspec(align(N))
#else
	#error "No way to specify required alignment!"
#endif

/*!
 * \def arx_is_aligned(Pointer, aligned)
 * \brief Check if a pointer is aligned.
 */
#define arx_is_aligned(Pointer, Alignment) \
	((reinterpret_cast<char *>(Pointer) - reinterpret_cast<char *>(0)) % (Alignment) == 0)

/*!
 * \def arx_return_aligned(Alignment)
 * \brief Declare that the pointer returned by a function has a specific alignment
 */
#if ARX_HAVE_ATTRIBUTE_ASSUME_ALIGNED
#define arx_return_aligned_impl(Alignment) __attribute__((assume_aligned(Alignment)))
#endif
#ifdef arx_return_aligned_impl
#define arx_return_aligned(Alignment) arx_return_aligned_impl(Alignment)
#else
#define arx_return_aligned(Alignment)
#endif

/*!
 * \def arx_assume_aligned(Pointer, Alignment)
 * \brief Assume that a pointer is aligned.
 *
 * In debug builds, alignment is checked using \ref arx_assert().
 *
 * Unlike arx_assert(Expression) this macro also tells the compiler to assume the pointer is aligned
 * in release builds.
 *
 * Depending on the compilter the alignment of the pointer is only assumed for the return value of the macro:
 * \code
 * const char * ptr = …;
 * const char * aligned = arx_assume_aligned(ptr, 16);
 * // Use aligned instead of ptr
 * \endcode
 */
#ifdef ARX_DEBUG
	template <typename T>
	T * checkAlignment(T * pointer, size_t alignment, const char * file, unsigned line) {
		if(!arx_is_aligned(pointer, alignment)) {
			assertionFailed("unaligned pointer", file, line, NULL);
			arx_trap();
		}
		return pointer;
	}
	#define arx_assume_aligned(Pointer, Alignment) \
		checkAlignment((Pointer), (Alignment), ARX_FILE, __LINE__)
#elif ARX_HAVE_BUILTIN_ASSUME_ALIGNED && ARX_HAVE_CXX11_DECLTYPE
	#define arx_assume_aligned(Pointer, Alignment) \
		static_cast<decltype(Pointer)>(__builtin_assume_aligned((Pointer), (Alignment)))
#elif ARX_HAVE_BUILTIN_ASSUME_ALIGNED && ARX_HAVE_GCC_TYPEOF
	#define arx_assume_aligned(Pointer, Alignment) \
		static_cast<__typeof__(Pointer)>(__builtin_assume_aligned((Pointer), (Alignment)))
#elif defined(arx_assume_impl) || defined(arx_return_aligned_impl)
	// TODO Use lambda
	template <size_t Alignment, typename T>
	arx_return_aligned(Alignment)
	arx_force_inline
	T * assumeAlignment(T * pointer) {
		arx_assume_impl(arx_is_aligned(pointer, Alignment));
		return pointer;
	}
	#define arx_assume_aligned(Pointer, Alignment) \
		assumeAlignment<(Alignment)>((Pointer))
#else
	#define arx_assume_aligned(Pointer, Alignment) (Pointer)
#endif

/*!
 * \def arx_alloc_align(SizeArg, AlignArg)
 * \brief Annotate a function that returns a pointer whose alignment is given by parameter with index
 *        AlignArg and that doesn't alias with anything and points to uninitialized or zeroed memory of
 *        size given by the function parameter with index SizeArg
 */
#if ARX_HAVE_ATTRIBUTE_ALLOC_ALIGN
#define arx_alloc_align(SizeArg, AlignArg) arx_alloc(SizeArg) __attribute__((alloc_align(AlignArg)))
#else
#define arx_alloc_align(SizeArg, AlignArg) arx_alloc(SizeArg)
#endif

#define arx_alloc_align_static(SizeArg, Alignment) arx_alloc(SizeArg) arx_return_aligned(Alignment)

namespace platform {

#if ARX_HAVE_CXX11_MAX_ALIGN_T
enum AlignmentInfo_ { GuaranteedAlignment = ARX_ALIGNOF(std::max_align_t) };
#else
enum AlignmentInfo_ {
	GuaranteedAlignment = boost::static_unsigned_max<
		#if ARX_HAVE_CXX11_LONG_LONG
		ARX_ALIGNOF(long long),
		#else
		ARX_ALIGNOF(long),
		#endif
		boost::static_unsigned_max<
			ARX_ALIGNOF(double),
			ARX_ALIGNOF(long double)
		>::value
	>::value
};
#endif

/*!
 * Allocate a buffer with a specific allignment.
 *
 * This is only needed if the required alignment is greater than \c GuaranteedAlignment.
 *
 * \param alignment The required alignment. This must be a power of two and
 *                  a multiple of \code sizeof( void *) \endcode.
 * \param size      The required buffer size.
 */
arx_alloc_align(2, 1)
void * alloc_aligned(std::size_t alignment, std::size_t size);

/*!
 * Free a pointer that was allocated with \ref alloc_aligned.
 *
 * \param ptr The pointer to free. This must either be \c nullptr or a pointer
 *            that was previously returned from \ref alloc_aligned and not yet freed.
 */
void free_aligned(void * ptr);

/*!
 * Allocation helper that uses aligned allocation if required but otherwise uses \c new.
 *
 * \tparam Alignment The required alignment. This must be a power of two and
 *                   a multiple of \code sizeof( void *) \endcode.
 */
template <size_t Alignment, bool NeedsManualAlignment = (Alignment > GuaranteedAlignment)>
struct AlignedAllocator {
	static const void * void_type;
	ARX_STATIC_ASSERT(Alignment % sizeof(void_type) == 0,
	                  "Alignment must be a multiple of sizeof(void *)");
	ARX_STATIC_ASSERT((Alignment & (Alignment - 1)) == 0,
	                  "Alignment must be a power of two");
	arx_alloc_align_static(1, Alignment)
	static void * alloc_object(std::size_t size) {
		void * ptr = arx_assume_aligned(alloc_aligned(Alignment, size), Alignment);
		if(!ptr) {
			throw std::bad_alloc();
		}
		return ptr;
	}
	static void free_object(void * ptr) {
		free_aligned(ptr);
	}
	arx_alloc_align_static(1, Alignment)
	static void * alloc_array(std::size_t size) {
		return alloc_object(size);
	}
	static void free_array(void * ptr) {
		free_object(ptr);
	}
};

template <size_t Alignment>
struct AlignedAllocator<Alignment, false> {
	arx_alloc_align_static(1, GuaranteedAlignment)
	static void * alloc_object(std::size_t size) {
		return ::operator new(size);
	}
	static void free_object(void * ptr) {
		::operator delete(ptr);
	}
	arx_alloc_align_static(1, GuaranteedAlignment)
	static void * alloc_array(std::size_t size) {
		return ::operator new[](size);
	}
	static void free_array(void * ptr) {
		::operator delete[](ptr);
	}
};

/*!
 * C++ allocator with alignment support.
 *
 * \tparam T         The type to allocate.
 * \tparam Alignment The required alignment. Will use the alignment of T if not specified.
 */
template <typename T, size_t Alignment = ARX_ALIGNOF(T)>
struct aligned_allocator {
	
	typedef T value_type;
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T & reference;
	typedef const T & const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	template <class U> struct rebind { typedef std::allocator<U> other; };
	
	#if ARX_HAVE_CXX11_INTEGRAL_CONSTANT
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type is_always_equal;
	#endif
	
	aligned_allocator() ARX_NOEXCEPT { }
	aligned_allocator(const aligned_allocator & o) ARX_NOEXCEPT { ARX_UNUSED(o); }
	template <typename U>
	aligned_allocator(const aligned_allocator<U> & o) ARX_NOEXCEPT { ARX_UNUSED(o); }
	~aligned_allocator() ARX_NOEXCEPT { }
	
	pointer address(reference x) const ARX_NOEXCEPT { return &x; }
	const_pointer address(const_reference x) const ARX_NOEXCEPT { return &x; }
	
	size_type max_size() const {
		return std::numeric_limits<size_type>::max() / sizeof(value_type);
	}
	
	pointer allocate(size_type n, const void * hint = 0) {
		ARX_UNUSED(hint);
		if(n > max_size()) {
			throw std::bad_alloc();
		}
		return static_cast<pointer>(AlignedAllocator<Alignment>::alloc_array(n * sizeof(value_type)));
	}
	
	void deallocate(pointer p, size_type n) {
		ARX_UNUSED(n);
		AlignedAllocator<Alignment>::free_array(p);
	}
	
	#if ARX_HAVE_CXX11_VARIADIC_TEMPLATES && ARX_HAVE_CXX11_FORWARD
	
	template <class U, class... Args>
	void construct(U * p, Args &&... args) {
		::new(static_cast<void *>(p)) U(std::forward<Args>(args)...);
	}
	
	template <class U>
	void destroy(U * p) {
		p->~U();
	}
	
	#else
	
	#if ARX_COMPILER_MSVC
	/*
	 * MSVC's default allocator has partial support for variadic construction even for
	 * compilers that don't support variadic templates yet. Other parts of the stdlib
	 * depend on this.
	 */
	template <class U>
	void construct(U * p) {
		::new(static_cast<void *>(p)) U();
	}
	template <class U, class V0>
	void construct(U * p, V0 && v0) {
		::new(static_cast<void *>(p)) U(v0);
	}
	#endif
	
	void construct(pointer p, const_reference val) {
		::new(static_cast<void *>(p)) value_type(val);
	}
	
	void destroy(pointer p) {
		p->~value_type();
	}
	
	#endif
	
	friend bool operator!=(const aligned_allocator & a, const aligned_allocator & b) {
		ARX_UNUSED(a), ARX_UNUSED(b);
		return false;
	}
	
	friend bool operator==(const aligned_allocator & a, const aligned_allocator & b) {
		ARX_UNUSED(a), ARX_UNUSED(b);
		return true;
	}
	
};

//! Check if a pointer has aparticular alignment.
inline bool is_aligned_on(const void * p, size_t alignment) {
	return alignment == 1 || (size_t(p) % alignment == 0);
}

//! Check if a pointer is aligned for a specific type.
template <class T>
bool is_aligned(const void * p) {
	return is_aligned_on(p, ARX_ALIGNOF(T));
}

} // namespace platform


/*!
 * \def ARX_USE_ALIGNED_NEW_N(Alignment)
 * Overrride the new, new[], delete and delete[] operators to use a specific alignment.
 * 
 * This should go in the body of a class to only override the operators for that class.
 * 
 * \param Alignment The alignment to use. This must be a power of two and a multiple of
 *                  sizeof(void *).
 */
#define ARX_USE_ALIGNED_NEW_N(Alignment) \
	void * operator new(std::size_t size) { \
		return ::platform::AlignedAllocator<Alignment>::alloc_object(size); \
	} \
	void * operator new[](std::size_t size) { \
		return ::platform::AlignedAllocator<Alignment>::alloc_array(size); \
	} \
	void operator delete(void * ptr) { \
		::platform::AlignedAllocator<Alignment>::free_object(ptr); \
	} \
	void operator delete[](void * ptr) { \
		::platform::AlignedAllocator<Alignment>::free_array(ptr); \
	}

/*!
 * \def ARX_USE_ALIGNED_NEW(Class)
 * Overrride the new, new[], delete and delete[] operators to use the alignment requred
 * by a class.
 * 
 * This should only be used in the global namespace.
 * 
 * \param Class The class whose alignment requirements should be used.
 */
#define ARX_USE_ALIGNED_NEW(Class) ARX_USE_ALIGNED_NEW_N(ARX_ALIGNOF(Class))

#define ARX_USE_ALIGNED_ALLOCATOR_T(Template, Class, Alignment) \
	namespace std { \
	template <Template> \
	struct allocator<Class> : public ::platform::aligned_allocator<Class, Alignment> { \
		allocator() ARX_NOEXCEPT { } \
		allocator(const allocator & o) ARX_NOEXCEPT \
			: ::platform::aligned_allocator<Class, Alignment>(o) { } \
		template <typename U> \
		allocator(const allocator<U> & o) ARX_NOEXCEPT \
			: ::platform::aligned_allocator<Class, Alignment>(o) { } \
		~allocator() ARX_NOEXCEPT { } \
	}; \
	}

/*!
 * \def ARX_USE_ALIGNED_ALLOCATOR_N(Class, Alignment)
 * Overrride the default std::allocator for a class to use a specific alignment.
 * 
 * This should only be used in the global namespace.
 * 
 * \param Class     The user-defined class to overrride std::allocator for.
 * \param Alignment The alignment to use. This must be a power of two and a multiple of
 *                  sizeof(void *).
 */
#define ARX_USE_ALIGNED_ALLOCATOR_N(Class, Alignment) \
	ARX_USE_ALIGNED_ALLOCATOR_T(typename Key, std::pair<Key BOOST_PP_COMMA() Class>, Alignment) \
	ARX_USE_ALIGNED_ALLOCATOR_T(typename Value, std::pair<Class BOOST_PP_COMMA() Value>, Alignment) \
	ARX_USE_ALIGNED_ALLOCATOR_T(, Class, Alignment)

/*!
 * \def ARX_USE_ALIGNED_ALLOCATOR_N(Class, Alignment)
 * Overrride the default std::allocator for a class to use the required alignment.
 * 
 * This should only be used in the global namespace.
 * 
 * \param Class     The user-defined class to overrride std::allocator for.
 */
#define ARX_USE_ALIGNED_ALLOCATOR(Class) \
	ARX_USE_ALIGNED_ALLOCATOR_N(Class, ARX_ALIGNOF(Class))


#endif // ARX_PLATFORM_ALIGNMENT_H
