/*
 * Copyright 2013-2017 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_PLATFORM_PROGRAMOPTIONS_H
#define ARX_PLATFORM_PROGRAMOPTIONS_H

#include <string>

#include <boost/intrusive/list.hpp>
#include <boost/function_types/function_arity.hpp>

#include "platform/Platform.h"
#include "util/cmdline/Interpreter.h"
#include "util/cmdline/Optional.h"

// Linked list of statically defined options (no memory allocation)
class BaseOption : public boost::intrusive::list_base_hook<
	boost::intrusive::link_mode<boost::intrusive::auto_unlink>
> {
	
public:
	
	static void registerAll(util::cmdline::interpreter<std::string> & l);
		
protected:
	
	BaseOption(const char * longName, const char * shortName, const char * description);
	
private:
	
	typedef boost::intrusive::list<
		BaseOption,
		boost::intrusive::constant_time_size<false>
	> List;
	
	static List & getOptionsList();
	
	virtual void registerOption(util::cmdline::interpreter<std::string> & l) = 0;
	
protected:
	
	const char * m_longName;
	const char * m_shortName;
	const char * m_description;
	
};

#define ARX_PROGRAM_OPTION_ARGS \
	const char * longName, const char * shortName, const char * description, \
	const Handler & handler, const char * args

namespace detail {

template <typename T>
struct is_arg_optional {
	static const bool value = false;
};

template <typename T>
struct is_arg_optional<void(util::cmdline::optional<T>)> {
	static const bool value = true;
};

template <typename T>
struct is_arg_optional<void(*)(util::cmdline::optional<T>)> {
	static const bool value = true;
};

} // namespace detail

template <typename Handler>
class Option : public BaseOption {
	
public:
	
	explicit Option(ARX_PROGRAM_OPTION_ARGS)
		: BaseOption(longName, shortName, description)
		, m_handler(handler), m_argNames(args) { }
	
	virtual void registerOption(util::cmdline::interpreter<std::string> & l) {
		std::string shortName = (m_shortName == NULL || *m_shortName == 0) ? "" : std::string("-") + m_shortName;
		std::string longName = (m_longName == NULL || *m_longName == 0) ? "" : std::string("--") + m_longName;
		if(shortName.empty() && longName.empty()) {
			longName = "--";
		}
		
		l.add(m_handler,
			  util::cmdline::interpreter<std::string>::op_name_t(shortName)(longName)
			  .description(m_description)
			  .arg_count(boost::function_types::function_arity<Handler>::value)
			  .arg_names(m_argNames)
				.arg_optional(detail::is_arg_optional<Handler>::value)
		);
	}
	
private:
	
	Handler m_handler;
	const char * m_argNames;
	
};


/*!
 * \def ARX_PROGRAM_OPTION(longName, shortName, description, handler, args)
 * \brief Register a program option
 */
#if ARX_HAVE_CXX11_AUTO
	template <typename Handler>
	Option<Handler> makeProgramOption(ARX_PROGRAM_OPTION_ARGS) {
		return Option<Handler>(longName, shortName, description, handler, args);
	}
	#define ARX_PROGRAM_OPTION_ARG(longName, shortName, description, handler, args) \
		static auto ARX_UNIQUE_SYMBOL(programOptionRegistrator) = makeProgramOption( \
			longName, shortName, description, handler, args \
		);
#else
	#define ARX_PROGRAM_OPTION_ARG(longName, shortName, description, handler, args) \
		template <typename Handler> \
		static BaseOption * ARX_UNIQUE_SYMBOL(makeProgramOption)(const Handler &); \
		static BaseOption * ARX_UNIQUE_SYMBOL(programOptionRegistrator) \
			= ARX_UNIQUE_SYMBOL(makeProgramOption)(handler); \
		template <typename Handler> \
		static BaseOption * ARX_UNIQUE_SYMBOL(makeProgramOption)(const Handler &) { \
			ARX_UNUSED(ARX_UNIQUE_SYMBOL(programOptionRegistrator)); \
			static Option<Handler> s_handler( \
				longName, shortName, description, handler, args \
			); \
			return &s_handler; \
		}
#endif
#define ARX_PROGRAM_OPTION(longName, shortName, description, handler) \
	ARX_PROGRAM_OPTION_ARG(longName, shortName, description, handler, NULL)

#undef ARX_PROGRAM_OPTION_ARGS

#endif // ARX_PLATFORM_PROGRAMOPTIONS_H
