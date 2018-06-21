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

#include "platform/ProgramOptions.h"

#include <list>
#include <boost/foreach.hpp>

BaseOption::List & BaseOption::getOptionsList() {
	// Local static to ensure initialization order is not causing us any issue.
	static List s_options;
	return s_options;
}

void BaseOption::registerAll(util::cmdline::interpreter<std::string> & l) {
	for(BaseOption & opt : getOptionsList()) {
		opt.registerOption(l);
	}
}

BaseOption::BaseOption(const char * longName, const char * shortName,
                       const char * description)
	: m_longName(longName)
	, m_shortName(shortName)
	, m_description(description) {
		getOptionsList().push_back(*this);
}
