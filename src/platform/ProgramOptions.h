/*
 * Copyright 2013 Arx Libertatis Team (see the AUTHORS file)
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

#include "platform/Platform.h"
#include "util/cmdline/Interpreter.h"

// Linked list of statically defined options (no memory allocation)
class BaseOption : public boost::intrusive::list_base_hook<
	boost::intrusive::link_mode<boost::intrusive::auto_unlink>
> {
	
public:
	
	static void registerAll(util::cmdline::interpreter<std::string> & l);
		
protected:
	
	BaseOption(const char* longName, const char* shortName, const char* description);
	
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

template<typename Handler>
class Option : public BaseOption {
	
public:
	
	Option(const char * longName, const char * shortName, const char * description,
	       const Handler & handler, const char * argNames)
		: BaseOption(longName, shortName, description)
		, m_handler(handler), m_argNames(argNames) { }
	
	virtual void registerOption(util::cmdline::interpreter<std::string> & l) {
		std::string shortName = (m_shortName == NULL || *m_shortName == 0) ? "" : std::string("-") + m_shortName;
		std::string longName = (m_longName == NULL || *m_longName == 0) ? "" : std::string("--") + m_longName;
		
		l.add(m_handler,
			  util::cmdline::interpreter<std::string>::op_name_t(shortName)(longName)
			  .description(m_description)
			  .arg_count(boost::function_types::function_arity<Handler>::value)
			  .arg_names(m_argNames)
		);
	}
	
private:
	
	Handler m_handler;
	const char * m_argNames;
	
};


#define UNIQUE_NAME(X) BOOST_PP_CAT(X,__LINE__)

#ifdef ARX_COMPILER_HAS_CXX11_AUTO
	template<typename Handler>
	Option<Handler> make_option(const char * longName, const char * shortName,
	                            const char * desc, const Handler & funcHandler,
	                            const char * argDesc = NULL) {
		return Option<Handler>(longName, shortName, desc, funcHandler, argDesc);
	}
	#define ARX_PROGRAM_OPTION(longOpt, shortOpt, description, handler, ...) \
		static auto UNIQUE_NAME(optionRegistrator) \
			= make_option(longOpt, shortOpt, description, handler, ##__VA_ARGS__);
#else
	#define ARX_PROGRAM_OPTION(longOpt, shortOpt, description, handler, ...) \
		template<typename Handler> \
		static BaseOption * UNIQUE_NAME(declare_option)(const char * longName, \
		                                                const char * shortName, \
		                                                const char * desc, \
		                                                const Handler & funcHandler, \
		                                                const char * argDesc = NULL) { \
			static Option<Handler> s_handler(longName, shortName, desc, funcHandler, argDesc); \
			return &s_handler; \
		} \
		static BaseOption * UNIQUE_NAME(optionRegistrator) \
			= UNIQUE_NAME(declare_option)(longOpt, shortOpt, description, handler, ##__VA_ARGS__)
#endif

#endif // ARX_PLATFORM_PROGRAMOPTIONS_H
