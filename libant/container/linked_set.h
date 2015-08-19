// linked_set implementation -*- C++ -*-

// Copyright (C) 2001-2013 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

/**
 * @file container/linked_set.h
 * @brief linked_set implementation.
 */

#ifndef LIBANT_CONTAINER_LINKED_SET_H_
#define LIBANT_CONTAINER_LINKED_SET_H_

#include <functional>

#if __cplusplus >= 201103L
#include <initializer_list>
#include <type_traits>
#endif

#include "internal/stl_tree.h"

namespace ant {

/**
 *  @brief A standard container made up of unique keys, which can be
 *  retrieved in logarithmic time.
 *
 *  @ingroup associative_containers
 *
 *  @tparam _Key  Type of key objects.
 *  @tparam _Compare  Comparison function object type, defaults to less<_Key>.
 *  @tparam _Alloc  Allocator type, defaults to allocator<_Key>.
 *
 *  Meets the requirements of a <a href="tables.html#65">container</a>, a
 *  <a href="tables.html#66">reversible container</a>, and an
 *  <a href="tables.html#69">associative container</a> (using unique keys).
 *
 *  Sets support bidirectional iterators.
 *
 */
template<typename _Key, typename _Compare = std::less<_Key>,
			typename _Alloc = std::allocator<_Key> >
class linked_set {
	// concept requirements
	typedef typename _Alloc::value_type _Alloc_value_type;

	template<typename _Tp>
	struct _Identity : public std::unary_function<_Tp, _Tp> {
		_Tp& operator()(_Tp& __x) const
		{
			return __x;
		}

		const _Tp& operator()(const _Tp& __x) const
		{
			return __x;
		}
	};

public:
	// typedefs:
	//@{
	/// Public typedefs.
	typedef _Key key_type;
	typedef _Key value_type;
	typedef _Compare key_compare;
	typedef _Compare value_compare;
	typedef _Alloc allocator_type;
	//@}

private:
	typedef typename _Alloc::template rebind<_Key>::other _Key_alloc_type;
	typedef _Rb_tree<key_type, value_type, _Identity<value_type>, key_compare, _Key_alloc_type> _Rep_type;

	_Rep_type _M_t;  // Red-black tree representing linked_set.

public:
	//@{
	///  Iterator-related typedefs.
	typedef typename _Key_alloc_type::pointer pointer;
	typedef typename _Key_alloc_type::const_pointer const_pointer;
	typedef typename _Key_alloc_type::reference reference;
	typedef typename _Key_alloc_type::const_reference const_reference;
	typedef typename _Rep_type::const_iterator iterator;
	typedef typename _Rep_type::const_iterator const_iterator;
	typedef typename _Rep_type::const_reverse_iterator reverse_iterator;
	typedef typename _Rep_type::const_reverse_iterator const_reverse_iterator;
	typedef typename _Rep_type::const_insert_order_iterator link_iterator;
	typedef typename _Rep_type::const_insert_order_iterator const_link_iterator;
	typedef typename _Rep_type::const_reverse_insert_order_iterator reverse_link_iterator;
	typedef typename _Rep_type::const_reverse_insert_order_iterator const_reverse_link_iterator;
	typedef typename _Rep_type::size_type size_type;
	typedef typename _Rep_type::difference_type difference_type;
	//@}

	// allocation/deallocation
	/**
	 *  @brief  Default constructor creates no elements.
	 */
	linked_set() : _M_t()
	{
	}

	/**
	 *  @brief  Creates a %linked_set with no elements.
	 *  @param  __comp  Comparator to use.
	 *  @param  __a  An allocator object.
	 */
	explicit linked_set(const _Compare& __comp, const allocator_type& __a = allocator_type())
		: _M_t(__comp, _Key_alloc_type(__a))
	{
	}

	/**
	 *  @brief  Builds a %linked_set from a range.
	 *  @param  __first  An input iterator.
	 *  @param  __last  An input iterator.
	 *
	 *  Create a %linked_set consisting of copies of the elements from
	 *  [__first,__last).  This is linear in N if the range is
	 *  already sorted, and NlogN otherwise (where N is
	 *  distance(__first,__last)).
	 */
	template<typename _InputIterator>
	linked_set(_InputIterator __first, _InputIterator __last) : _M_t()
	{
		_M_t._M_insert_unique(__first, __last);
	}

	/**
	 *  @brief  Builds a %linked_set from a range.
	 *  @param  __first  An input iterator.
	 *  @param  __last  An input iterator.
	 *  @param  __comp  A comparison functor.
	 *  @param  __a  An allocator object.
	 *
	 *  Create a %linked_set consisting of copies of the elements from
	 *  [__first,__last).  This is linear in N if the range is
	 *  already sorted, and NlogN otherwise (where N is
	 *  distance(__first,__last)).
	 */
	template<typename _InputIterator>
	linked_set(_InputIterator __first, _InputIterator __last, const _Compare& __comp,
				const allocator_type& __a = allocator_type())
		: _M_t(__comp, _Key_alloc_type(__a))
	{
		_M_t._M_insert_unique(__first, __last);
	}

	/**
	 *  @brief  %Set copy constructor.
	 *  @param  __x  A %linked_set of identical element and allocator types.
	 *
	 *  The newly-created %linked_set uses a copy of the allocation object used
	 *  by @a __x.
	 */
	linked_set(const linked_set& __x) : _M_t(__x._M_t)
	{
	}

#if __cplusplus >= 201103L
	/**
	 *  @brief %Set move constructor
	 *  @param __x  A %linked_set of identical element and allocator types.
	 *
	 *  The newly-created %linked_set contains the exact contents of @a x.
	 *  The contents of @a x are a valid, but unspecified %linked_set.
	 */
	linked_set(linked_set&& __x) noexcept(std::is_nothrow_copy_constructible<_Compare>::value)
		: _M_t(std::move(__x._M_t))
	{
	}

	/**
	 *  @brief  Builds a %linked_set from an initializer_list.
	 *  @param  __l  An initializer_list.
	 *  @param  __comp  A comparison functor.
	 *  @param  __a  An allocator object.
	 *
	 *  Create a %linked_set consisting of copies of the elements in the list.
	 *  This is linear in N if the list is already sorted, and NlogN
	 *  otherwise (where N is @a __l.size()).
	 */
	linked_set(std::initializer_list<value_type> __l, const _Compare& __comp = _Compare(),
				const allocator_type& __a = allocator_type())
		: _M_t(__comp, _Key_alloc_type(__a))
	{
		_M_t._M_insert_unique(__l.begin(), __l.end());
	}
#endif

	/**
	 *  @brief  %Set assignment operator.
	 *  @param  __x  A %linked_set of identical element and allocator types.
	 *
	 *  All the elements of @a __x are copied, but unlike the copy
	 *  constructor, the allocator object is not copied.
	 */
	linked_set& operator=(const linked_set& __x)
	{
		_M_t = __x._M_t;
		return *this;
	}

#if __cplusplus >= 201103L
	/**
	 *  @brief %Set move assignment operator.
	 *  @param __x  A %linked_set of identical element and allocator types.
	 *
	 *  The contents of @a __x are moved into this %linked_set (without copying).
	 *  @a __x is a valid, but unspecified %linked_set.
	 */
	linked_set& operator=(linked_set&& __x)
	{
		// NB: DR 1204.
		// NB: DR 675.
		this->clear();
		this->swap(__x);
		return *this;
	}

	/**
	 *  @brief  %Set list assignment operator.
	 *  @param  __l  An initializer_list.
	 *
	 *  This function fills a %linked_set with copies of the elements in the
	 *  initializer list @a __l.
	 *
	 *  Note that the assignment completely changes the %linked_set and
	 *  that the resulting %linked_set's size is the same as the number
	 *  of elements assigned.  Old data may be lost.
	 */
	linked_set& operator=(std::initializer_list<value_type> __l)
	{
		this->clear();
		this->insert(__l.begin(), __l.end());
		return *this;
	}
#endif

	// accessors:

	///  Returns the comparison object with which the %linked_set was constructed.
	key_compare key_comp() const
	{
		return _M_t.key_comp();
	}
	///  Returns the comparison object with which the %linked_set was constructed.
	value_compare value_comp() const
	{
		return _M_t.key_comp();
	}
	///  Returns the allocator object with which the %linked_set was constructed.
	allocator_type get_allocator() const _LIBANT_NOEXCEPT
	{
		return allocator_type(_M_t.get_allocator());
	}

	/**
	 *  Returns a read-only (constant) iterator that points to the first
	 *  element in the %linked_set.  Iteration is done in ascending order according
	 *  to the keys.
	 */
	iterator begin() const _LIBANT_NOEXCEPT
	{
		return _M_t.begin();
	}

	/**
	 *  Returns a read-only (constant) iterator that points one past the last
	 *  element in the %linked_set.  Iteration is done in ascending order according
	 *  to the keys.
	 */
	iterator end() const _LIBANT_NOEXCEPT
	{
		return _M_t.end();
	}

	/**
	 *  Returns a read-only (constant) iterator that points to the last
	 *  element in the %linked_set.  Iteration is done in descending order according
	 *  to the keys.
	 */
	reverse_iterator rbegin() const _LIBANT_NOEXCEPT
	{
		return _M_t.rbegin();
	}

	/**
	 *  Returns a read-only (constant) reverse iterator that points to the
	 *  last pair in the %linked_set.  Iteration is done in descending order
	 *  according to the keys.
	 */
	reverse_iterator rend() const _LIBANT_NOEXCEPT
	{
		return _M_t.rend();
	}

#if __cplusplus >= 201103L
	/**
	 *  Returns a read-only (constant) iterator that points to the first
	 *  element in the %linked_set.  Iteration is done in ascending order according
	 *  to the keys.
	 */
	const_iterator cbegin() const noexcept
	{
		return _M_t.begin();
	}

	/**
	 *  Returns a read-only (constant) iterator that points one past the last
	 *  element in the %linked_set.  Iteration is done in ascending order according
	 *  to the keys.
	 */
	const_iterator cend() const noexcept
	{
		return _M_t.end();
	}

	/**
	 *  Returns a read-only (constant) iterator that points to the last
	 *  element in the %linked_set.  Iteration is done in descending order according
	 *  to the keys.
	 */
	const_reverse_iterator crbegin() const noexcept
	{
		return _M_t.rbegin();
	}

	/**
	 *  Returns a read-only (constant) reverse iterator that points to the
	 *  last pair in the %linked_set.  Iteration is done in descending order
	 *  according to the keys.
	 */
	const_reverse_iterator crend() const noexcept
	{
		return _M_t.rend();
	}
#endif

	/**
	 *  Returns a read-only (constant) link_iterator that points to the first element
	 *  inserted into the %linked_set. Iteration is done in insertion order.
	 */
	link_iterator link_begin() const _LIBANT_NOEXCEPT
	{
		return _M_t.link_begin();
	}

	/**
	 *  Returns a read-only (constant) link_iterator that points one past the last
	 *  element inserted into the %linked_set. Iteration is done in insertion order.
	 */
	link_iterator link_end() const _LIBANT_NOEXCEPT
	{
		return _M_t.link_end();
	}

	/**
	 *  Returns a read-only (constant) reverse_link_iterator that points to
	 *  the last element inserted into the %linked_set.
	 *  Iteration is done in reversed insertion order.
	 */
	reverse_link_iterator link_rbegin() const _LIBANT_NOEXCEPT
	{
		return _M_t.link_rbegin();
	}

	/**
	 *  Returns a read-only (constant) reverse_link_iterator that points one past
	 *  the first element inserted into the %linked_set.
	 *  Iteration is done in insertion order.
	 */
	reverse_link_iterator link_rend() const _LIBANT_NOEXCEPT
	{
		return _M_t.link_rend();
	}

#if __cplusplus >= 201103L
	/**
	 *  Returns a read-only (constant) link_iterator that points to the first element
	 *  inserted into the %linked_set. Iteration is done in insertion order.
	 */
	const_link_iterator link_cbegin() const noexcept
	{
		return _M_t.link_begin();
	}

	/**
	 *  Returns a read-only (constant) link_iterator that points one past the last
	 *  element inserted into the %linked_set. Iteration is done in insertion order.
	 */
	const_link_iterator link_cend() const noexcept
	{
		return _M_t.link_end();
	}

	/**
	 *  Returns a read-only (constant) iterator that points to the last
	 *  element in the %linked_set.  Iteration is done in descending order according
	 *  to the keys.
	 */
	const_reverse_link_iterator link_crbegin() const noexcept
	{
		return _M_t.link_rbegin();
	}

	/**
	 *  Returns a read-only (constant) reverse_link_iterator that points one past
	 *  the first element inserted into the %linked_set.
	 *  Iteration is done in insertion order.
	 */
	const_reverse_link_iterator link_crend() const noexcept
	{
		return _M_t.link_rend();
	}
#endif

	///  Returns true if the %linked_set is empty.
	bool empty() const _LIBANT_NOEXCEPT
	{
		return _M_t.empty();
	}

	///  Returns the size of the %linked_set.
	size_type size() const _LIBANT_NOEXCEPT
	{
		return _M_t.size();
	}

	///  Returns the maximum size of the %linked_set.
	size_type max_size() const _LIBANT_NOEXCEPT
	{
		return _M_t.max_size();
	}

	/**
	 *  @brief  Swaps data with another %linked_set.
	 *  @param  __x  A %linked_set of the same element and allocator types.
	 *
	 *  This exchanges the elements between two sets in constant
	 *  time.  (It is only swapping a pointer, an integer, and an
	 *  instance of the @c Compare type (which itself is often
	 *  stateless and empty), so it should be quite fast.)  Note
	 *  that the global std::swap() function is specialized such
	 *  that std::swap(s1,s2) will feed to this function.
	 */
	void swap(linked_set& __x)
	{
		_M_t.swap(__x._M_t);
	}

	// insert/erase
#if __cplusplus >= 201103L
	/**
	 *  @brief Attempts to build and insert an element into the %linked_set.
	 *  @param __args  Arguments used to generate an element.
	 *  @return  A pair, of which the first element is an iterator that points
	 *           to the possibly inserted element, and the second is a bool
	 *           that is true if the element was actually inserted.
	 *
	 *  This function attempts to build and insert an element into the %linked_set.
	 *  A %linked_set relies on unique keys and thus an element is only inserted if
	 *  it is not already present in the %linked_set.
	 *
	 *  Insertion requires logarithmic time.
	 */
	template<typename ... _Args>
	std::pair<iterator, bool> emplace(_Args&&... __args)
	{
		return _M_t._M_emplace_unique(std::forward<_Args>(__args)...);
	}

	/**
	 *  @brief Attempts to insert an element into the %linked_set.
	 *  @param  __pos  An iterator that serves as a hint as to where the
	 *                element should be inserted.
	 *  @param  __args  Arguments used to generate the element to be
	 *                 inserted.
	 *  @return An iterator that points to the element with key equivalent to
	 *          the one generated from @a __args (may or may not be the
	 *          element itself).
	 *
	 *  This function is not concerned about whether the insertion took place,
	 *  and thus does not return a boolean like the single-argument emplace()
	 *  does.  Note that the first parameter is only a hint and can
	 *  potentially improve the performance of the insertion process.  A bad
	 *  hint would cause no gains in efficiency.
	 *
	 *  For more on @a hinting, see:
	 *  http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt07ch17.html
	 *
	 *  Insertion requires logarithmic time (if the hint is not taken).
	 */
	template<typename ... _Args>
	iterator emplace_hint(const_iterator __pos, _Args&&... __args)
	{
		return _M_t._M_emplace_hint_unique(__pos, std::forward<_Args>(__args)...);
	}
#endif

	/**
	 *  @brief Attempts to insert an element into the %linked_set.
	 *  @param  __x  Element to be inserted.
	 *  @return  A pair, of which the first element is an iterator that points
	 *           to the possibly inserted element, and the second is a bool
	 *           that is true if the element was actually inserted.
	 *
	 *  This function attempts to insert an element into the %linked_set.  A %linked_set
	 *  relies on unique keys and thus an element is only inserted if it is
	 *  not already present in the %linked_set.
	 *
	 *  Insertion requires logarithmic time.
	 */
	std::pair<iterator, bool> insert(const value_type& __x)
	{
		std::pair<typename _Rep_type::iterator, bool> __p = _M_t._M_insert_unique(__x);
		return std::pair<iterator, bool>(__p.first, __p.second);
	}

#if __cplusplus >= 201103L
	std::pair<iterator, bool> insert(value_type&& __x)
	{
		std::pair<typename _Rep_type::iterator, bool> __p = _M_t._M_insert_unique(std::move(__x));
		return std::pair<iterator, bool>(__p.first, __p.second);
	}
#endif

	/**
	 *  @brief Attempts to insert an element into the %linked_set.
	 *  @param  __position  An iterator that serves as a hint as to where the
	 *                    element should be inserted.
	 *  @param  __x  Element to be inserted.
	 *  @return An iterator that points to the element with key of
	 *           @a __x (may or may not be the element passed in).
	 *
	 *  This function is not concerned about whether the insertion took place,
	 *  and thus does not return a boolean like the single-argument insert()
	 *  does.  Note that the first parameter is only a hint and can
	 *  potentially improve the performance of the insertion process.  A bad
	 *  hint would cause no gains in efficiency.
	 *
	 *  For more on @a hinting, see:
	 *  http://gcc.gnu.org/onlinedocs/libstdc++/manual/bk01pt07ch17.html
	 *
	 *  Insertion requires logarithmic time (if the hint is not taken).
	 */
	iterator insert(const_iterator __position, const value_type& __x)
	{
		return _M_t._M_insert_unique_(__position, __x);
	}

#if __cplusplus >= 201103L
	iterator insert(const_iterator __position, value_type&& __x)
	{
		return _M_t._M_insert_unique_(__position, std::move(__x));
	}
#endif

	/**
	 *  @brief A template function that attempts to insert a range
	 *  of elements.
	 *  @param  __first  Iterator pointing to the start of the range to be
	 *                   inserted.
	 *  @param  __last  Iterator pointing to the end of the range.
	 *
	 *  Complexity similar to that of the range constructor.
	 */
	template<typename _InputIterator>
	void insert(_InputIterator __first, _InputIterator __last)
	{
		_M_t._M_insert_unique(__first, __last);
	}

#if __cplusplus >= 201103L
	/**
	 *  @brief Attempts to insert a list of elements into the %linked_set.
	 *  @param  __l  A std::initializer_list<value_type> of elements
	 *               to be inserted.
	 *
	 *  Complexity similar to that of the range constructor.
	 */
	void insert(std::initializer_list<value_type> __l)
	{
		this->insert(__l.begin(), __l.end());
	}
#endif

#if __cplusplus >= 201103L
	// DR 130. Associative erase should return an iterator.
	/**
	 *  @brief Erases an element from a %linked_set.
	 *  @param  __position  An iterator pointing to the element to be erased.
	 *  @return An iterator pointing to the element immediately following
	 *          @a __position prior to the element being erased. If no such
	 *          element exists, end() is returned.
	 *
	 *  This function erases an element, pointed to by the given iterator,
	 *  from a %linked_set.  Note that this function only erases the element, and
	 *  that if the element is itself a pointer, the pointed-to memory is not
	 *  touched in any way.  Managing the pointer is the user's
	 *  responsibility.
	 */
	iterator erase(const_iterator __position)
	{
		return _M_t.erase(__position);
	}
#else
	/**
	 *  @brief Erases an element from a %linked_set.
	 *  @param  position  An iterator pointing to the element to be erased.
	 *
	 *  This function erases an element, pointed to by the given iterator,
	 *  from a %linked_set.  Note that this function only erases the element, and
	 *  that if the element is itself a pointer, the pointed-to memory is not
	 *  touched in any way.  Managing the pointer is the user's
	 *  responsibility.
	 */
	void erase(iterator __position)
	{
		_M_t.erase(__position);
	}
#endif

	/**
	 *  @brief Erases elements according to the provided key.
	 *  @param  __x  Key of element to be erased.
	 *  @return  The number of elements erased.
	 *
	 *  This function erases all the elements located by the given key from
	 *  a %linked_set.
	 *  Note that this function only erases the element, and that if
	 *  the element is itself a pointer, the pointed-to memory is not touched
	 *  in any way.  Managing the pointer is the user's responsibility.
	 */
	size_type erase(const key_type& __x)
	{
		return _M_t.erase(__x);
	}

#if __cplusplus >= 201103L
	// DR 130. Associative erase should return an iterator.
	/**
	 *  @brief Erases a [__first,__last) range of elements from a %linked_set.
	 *  @param  __first  Iterator pointing to the start of the range to be
	 *                 erased.

	 *  @param __last Iterator pointing to the end of the range to
	 *  be erased.
	 *  @return The iterator @a __last.
	 *
	 *  This function erases a sequence of elements from a %linked_set.
	 *  Note that this function only erases the element, and that if
	 *  the element is itself a pointer, the pointed-to memory is not touched
	 *  in any way.  Managing the pointer is the user's responsibility.
	 */
	iterator erase(const_iterator __first, const_iterator __last)
	{
		return _M_t.erase(__first, __last);
	}
#else
	/**
	 *  @brief Erases a [first,last) range of elements from a %linked_set.
	 *  @param  __first  Iterator pointing to the start of the range to be
	 *                 erased.
	 *  @param __last Iterator pointing to the end of the range to
	 *  be erased.
	 *
	 *  This function erases a sequence of elements from a %linked_set.
	 *  Note that this function only erases the element, and that if
	 *  the element is itself a pointer, the pointed-to memory is not touched
	 *  in any way.  Managing the pointer is the user's responsibility.
	 */
	void erase(iterator __first, iterator __last)
	{
		_M_t.erase(__first, __last);
	}
#endif

	/**
	 *  Erases all elements in a %linked_set.  Note that this function only erases
	 *  the elements, and that if the elements themselves are pointers, the
	 *  pointed-to memory is not touched in any way.  Managing the pointer is
	 *  the user's responsibility.
	 */
	void clear() _LIBANT_NOEXCEPT
	{
		_M_t.clear();
	}

	// linked_set operations:

	/**
	 *  @brief  Finds the number of elements.
	 *  @param  __x  Element to located.
	 *  @return  Number of elements with specified key.
	 *
	 *  This function only makes sense for linked_multisets;
	 *  for linked_set the result will either be 0
	 *  (not present) or 1 (present).
	 */
	size_type count(const key_type& __x) const
	{
		return _M_t.find(__x) == _M_t.end() ? 0 : 1;
	}

	//@{
	/**
	 *  @brief Tries to locate an element in a %linked_set.
	 *  @param  __x  Element to be located.
	 *  @return  Iterator pointing to sought-after element, or end() if not
	 *           found.
	 *
	 *  This function takes a key and tries to locate the element with which
	 *  the key matches.  If successful the function returns an iterator
	 *  pointing to the sought after element.  If unsuccessful it returns the
	 *  past-the-end ( @c end() ) iterator.
	 */
	iterator find(const key_type& __x)
	{
		return _M_t.find(__x);
	}

	const_iterator find(const key_type& __x) const
	{
		return _M_t.find(__x);
	}
	//@}

	//@{
	/**
	 *  @brief Finds the beginning of a subsequence matching given key.
	 *  @param  __x  Key to be located.
	 *  @return  Iterator pointing to first element equal to or greater
	 *           than key, or end().
	 *
	 *  This function returns the first element of a subsequence of elements
	 *  that matches the given key.  If unsuccessful it returns an iterator
	 *  pointing to the first element that has a greater value than given key
	 *  or end() if no such element exists.
	 */
	iterator lower_bound(const key_type& __x)
	{
		return _M_t.lower_bound(__x);
	}

	const_iterator lower_bound(const key_type& __x) const
	{
		return _M_t.lower_bound(__x);
	}
	//@}

	//@{
	/**
	 *  @brief Finds the end of a subsequence matching given key.
	 *  @param  __x  Key to be located.
	 *  @return Iterator pointing to the first element
	 *          greater than key, or end().
	 */
	iterator upper_bound(const key_type& __x)
	{
		return _M_t.upper_bound(__x);
	}

	const_iterator upper_bound(const key_type& __x) const
	{
		return _M_t.upper_bound(__x);
	}
	//@}

	//@{
	/**
	 *  @brief Finds a subsequence matching given key.
	 *  @param  __x  Key to be located.
	 *  @return  Pair of iterators that possibly points to the subsequence
	 *           matching given key.
	 *
	 *  This function is equivalent to
	 *  @code
	 *    std::make_pair(c.lower_bound(val),
	 *                   c.upper_bound(val))
	 *  @endcode
	 *  (but is faster than making the calls separately).
	 *
	 *  This function probably only makes sense for linked_multisets.
	 */
	std::pair<iterator, iterator> equal_range(const key_type& __x)
	{
		return _M_t.equal_range(__x);
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& __x) const
	{
		return _M_t.equal_range(__x);
	}
	//@}

	template<typename _K1, typename _C1, typename _A1>
	friend bool operator==(const linked_set<_K1, _C1, _A1>&, const linked_set<_K1, _C1, _A1>&);

	template<typename _K1, typename _C1, typename _A1>
	friend bool operator<(const linked_set<_K1, _C1, _A1>&, const linked_set<_K1, _C1, _A1>&);
};

/**
 *  @brief  linked_set equality comparison.
 *  @param  __x  A %linked_set.
 *  @param  __y  A %linked_set of the same type as @a x.
 *  @return  True if the size and elements of the linked_sets are equal.
 *
 *  This is an equivalence relation.  It is linear in the size of the linked_sets.
 *  linked_sets are considered equivalent if their sizes are equal, and if
 *  corresponding elements compare equal.
 */
template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator==(const linked_set<_Key, _Compare, _Alloc>& __x,
						const linked_set<_Key, _Compare, _Alloc>& __y)
{
	return __x._M_t == __y._M_t;
}

/**
 *  @brief  Set ordering relation.
 *  @param  __x  A %linked_set.
 *  @param  __y  A %linked_set of the same type as @a x.
 *  @return  True iff @a __x is lexicographically less than @a __y.
 *
 *  This is a total ordering relation.  It is linear in the size of the
 *  linked_sets.  The elements must be comparable with @c <.
 *
 *  See std::lexicographical_compare() for how the determination is made.
 */
template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator<(const linked_set<_Key, _Compare, _Alloc>& __x,
						const linked_set<_Key, _Compare, _Alloc>& __y)
{
	return __x._M_t < __y._M_t;
}

///  Returns !(x == y).
template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator!=(const linked_set<_Key, _Compare, _Alloc>& __x,
						const linked_set<_Key, _Compare, _Alloc>& __y)
{
	return !(__x == __y);
}

///  Returns y < x.
template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator>(const linked_set<_Key, _Compare, _Alloc>& __x,
						const linked_set<_Key, _Compare, _Alloc>& __y)
{
	return __y < __x;
}

///  Returns !(y < x)
template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator<=(const linked_set<_Key, _Compare, _Alloc>& __x,
						const linked_set<_Key, _Compare, _Alloc>& __y)
{
	return !(__y < __x);
}

///  Returns !(x < y)
template<typename _Key, typename _Compare, typename _Alloc>
inline bool operator>=(const linked_set<_Key, _Compare, _Alloc>& __x,
						const linked_set<_Key, _Compare, _Alloc>& __y)
{
	return !(__x < __y);
}

/// See ant::linked_set::swap().
template<typename _Key, typename _Compare, typename _Alloc>
inline void swap(linked_set<_Key, _Compare, _Alloc>& __x,
					linked_set<_Key, _Compare, _Alloc>& __y)
{
	__x.swap(__y);
}

} // namespace ant

#endif /* LIBANT_CONTAINER_LINKED_SET_H_ */
