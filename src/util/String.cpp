/*
 * Copyright 2011-2018 Arx Libertatis Team (see the AUTHORS file)
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

#include "util/String.h"

#include <algorithm>

#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

namespace util {

std::string loadString(const char * data, size_t maxLength) {
	return std::string(data, std::find(data, data + maxLength, '\0'));
}

void storeString(char * dst, size_t maxLength, const std::string & src) {
	std::memcpy(dst, src.c_str(), std::min(maxLength, src.length()));
	if(maxLength > src.length()) {
		std::memset(dst + src.length(), 0, maxLength - src.length());
	}
}

struct character_escaper {
	template <typename FinderT>
	std::string operator()(const FinderT & match) const {
		std::string s;
		for(typename FinderT::const_iterator i = match.begin(); i != match.end(); ++i) {
			s += std::string("\\") + *i;
		}
		return s;
	}
};

std::string escapeString(const std::string & text, const char * escapeChars) {
	std::string escapedStr(text);
	boost::find_format_all(escapedStr,
	                       boost::token_finder(boost::is_any_of(escapeChars)),
	                                           character_escaper());
	return escapedStr;
}

std::string unescapeString(const std::string & text) {
	
	std::string result;
	result.reserve(text.size());
	
	std::string::const_iterator begin = text.begin(), end = text.end();
	if(!text.empty() && text[0] == '"') {
		++begin;
		if(begin != end && text[text.size() - 1] == '"') {
			--end;
		}
	}
	
	for(std::string::const_iterator i = begin; i != end; ++i) {
		if(*i == '\\' && ++i == end) {
			break;
		}
		result.push_back(*i);
	}
	
	return result;
}

std::string getDateTimeString() {
	
	boost::posix_time::ptime localTime = boost::posix_time::second_clock::local_time();
	boost::gregorian::date::ymd_type ymd = localTime.date().year_month_day();
	boost::posix_time::time_duration hms = localTime.time_of_day();
	
	std::stringstream localTimeString;
	localTimeString << std::setfill('0')
	                << ymd.year << "."
	                << std::setw(2)
	                << ymd.month.as_number() << "."
	                << std::setw(2)
	                << ymd.day.as_number() << "-"
	                << std::setw(2)
	                << hms.hours() << "."
	                << std::setw(2)
	                << hms.minutes() << "."
	                << std::setw(2)
	                << hms.seconds();
	
	return localTimeString.str();
}


} // namespace util
