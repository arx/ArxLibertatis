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

#ifndef ARX_PLATFORM_PROGRAMOPTIONS_H
#define ARX_PLATFORM_PROGRAMOPTIONS_H

#include <string>

#include <boost/intrusive/list.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/function_types/function_arity.hpp>

#include "util/cmdline/Interpreter.h"

// Linked list of statically defined options (no memory allocation)
class BaseOption : public boost::intrusive::list_base_hook<> {
public:
	static void registerAll(interpreter<std::string>& l);
	
protected:
	BaseOption(const char* longName, const char* shortName, const char* description);

private:
	static boost::intrusive::list<BaseOption>& getOptionsList();

	virtual void registerOption(interpreter<std::string>& l) = 0;

protected:
	const char* m_longName;
	const char* m_shortName;
	const char* m_description;
};

template<typename Handler>
class Option : public BaseOption {
public:
	Option(const char* longName, const char* shortName, const char* description, Handler const& handler) 
		: BaseOption(longName, shortName, description)
		, m_handler(handler) {
	}

	virtual void registerOption(interpreter<std::string>& l) {
		l.add(m_handler, interpreter<std::string>::op_name_t(m_shortName)
		                                                    (m_longName)
		                                                    .description(m_description)
		                                                    .arg_count(boost::function_types::function_arity<Handler>::value));
	}

private:
	Handler m_handler;
};


#define UNIQUE_NAME(X) BOOST_PP_CAT(X,__LINE__)

#ifdef ARX_COMPILER_HAS_CXX11_AUTO
	template<typename Handler>
	Option<Handler> make_option(const char* longName, const char* shortName, const char* description, Handler const& funcHandler)
	{
		return Option<Handler>(longName, shortName, description, funcHandler);
	}

	#define ARX_PROGRAM_OPTION(longOpt, shortOpt, description, handler) \
		static auto UNIQUE_NAME(optionRegistrator) = make_option(longOpt, shortOpt, description, handler);
#else
	#define ARX_PROGRAM_OPTION(longOpt, shortOpt, description, handler)                                                                                            \
		template<typename Handler>                                                                                                                                  \
		static BaseOption* UNIQUE_NAME(declare_option)(const char* longName, const char* shortName, const char* desc, Handler const& funcHandler) \
		{                                                                                                                                                           \
			static Option<Handler> s_handler(longName, shortName, desc, funcHandler);                                                                        \
			return &s_handler;                                                                                                                                      \
		}                                                                                                                                                           \
		static BaseOption* UNIQUE_NAME(optionRegistrator) = UNIQUE_NAME(declare_option)(longOpt, shortOpt, description, handler)
#endif

#endif // ARX_PLATFORM_PROGRAMOPTIONS_H
