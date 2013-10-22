/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_UTIL_CMDLINE_DETAIL_INTERPRETER_H
#define ARX_UTIL_CMDLINE_DETAIL_INTERPRETER_H

#include <map>
#include <algorithm>
#include "util/cmdline/detail/LexicalCall.h"

namespace util { namespace cmdline { namespace detail {

template<typename OpName, typename TypeCast>
class interpreter {
	
public:
	
	typedef OpName                          op_name_t;
	typedef typename op_name_t::value_type  string_type;
	typedef TypeCast                        type_cast_t;
	
	template<typename Handler>
	void add(const Handler &, const op_name_t &);
	
	template<typename HndlSign, typename Handler>
	void add(const Handler &, const op_name_t &);
	
	void erase(const string_type & option_name);
	
	template<typename It>
	void invoke(const string_type & option_name, It & args_begin, It args_optend, It args_end,
	            type_cast_t &) const;
	
	template<typename It>
	void invoke(const string_type & option_name, It & args_begin, It args_end,
	            type_cast_t & type_cast) const {
		invoke(option_name, args_begin, args_end, args_end, type_cast);
	}
	
	template<typename Visitor>
	void visit(Visitor & visitor) const {
		do_visit(storage.begin(), storage.end(), visitor);
	}
	
	struct const_iterator;
	
private:
	
	typedef lexical_call_t<void(string_type, string_type, type_cast_t)> function_type;
	
	template<typename Iter, typename Visitor>
	void do_visit(Iter begin, Iter end, Visitor & visitor) const {
		for(; begin != end; ++begin) {
			visitor(begin->second.key);
		}
	}
	
	void do_add(const function_type & handler, const op_name_t & key);
	
	struct ikey_t {
		
		function_type function;
		op_name_t     key;
		
		void key_erase(const string_type & v) {
			typename op_name_t::iterator it(std::find(key.begin(), key.end(), v));
			if(key.end() != it) {
				key.erase(it);
			}
		}
		
		ikey_t(const function_type & function, const op_name_t & key)
			: function(function)
			, key(key)
		{ }
		
	};
	
	typedef std::map<string_type, ikey_t> storage_t;
	storage_t storage;
	
	typedef std::map<string_type, string_type> alt_name_t;
	alt_name_t alt_name;
	
};

///////////////////////////////////////////////////////////////////////////////

template<typename StringType, typename TypeCast>
void interpreter<StringType, TypeCast>::erase(const string_type & option_name) {
	
	typename alt_name_t::iterator it(alt_name.find(option_name));
	if(alt_name.end() == it) {
		return;
	}
	
	if(it->first != it->second) {
		storage.find(it->second)->second.key_erase(it->first);
		alt_name.erase(it);
		return;
	}
	
	typename op_name_t::iterator cur(storage.find(option_name)->second.key.begin());
	typename op_name_t::iterator end(storage.find(option_name)->second.key.end());
	for(; cur != end; ++cur) {
		alt_name.erase(alt_name.find(*cur));
	}
	
	storage.erase(option_name);
}

template<typename StringType, typename TypeCast>
template<typename Handler>
void interpreter<StringType, TypeCast>::add(const Handler & handler,
                                            const op_name_t & key) {
	do_add(function_type::construct(handler), key);
}

template<typename StringType, typename TypeCast>
template<typename HndlSign, typename Handler>
void interpreter<StringType, TypeCast>::add(const Handler & handler,
                                            const op_name_t & key) {
	do_add(function_type::template construct<HndlSign>(handler), key);
}

template<typename StringType, typename TypeCast>
void interpreter<StringType, TypeCast>::do_add(const function_type & handler,
                                               const op_name_t & key) {
	
	if(key.empty())
		return;
	
	typename op_name_t::const_iterator it;
	for(it = key.begin(); it != key.end(); ++it) {
		if(alt_name.end() != alt_name.find(*it)) {
			throw error(error::already_exists);
		}
	}
	
	alt_name_t tmp;
	
	for(it = key.begin(); it != key.end(); ++it) {
		tmp[*it] = key.front();
	}
	
	typename storage_t::iterator itStorage(storage.insert
		(typename storage_t::value_type(*key.begin(), ikey_t(handler, key))).first);
	
	try {
		alt_name.insert(tmp.begin(), tmp.end());
	} catch(...) {
		storage.erase(itStorage);
		throw;
	}
}

template<typename StringType, typename TypeCast>
template<typename It>
void interpreter<StringType, TypeCast>::invoke(const string_type & key,
                                              It & args_begin, It args_optend, It args_end,
                                              type_cast_t & type_cast) const {
	
	typename alt_name_t::const_iterator primary_key = alt_name.find(key);
	
	if(alt_name.end() == primary_key) {
		throw error(error::cmd_not_found);
	}
	
	typename storage_t::const_iterator it(storage.find(primary_key->second));
	it->second.function(args_begin, args_optend, args_end, type_cast);
}

} } } // namespace util::cmdline::detail

#endif // ARX_UTIL_CMDLINE_DETAIL_INTERPRETER_H
