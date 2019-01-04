/*
 * Copyright 2011-2017 Arx Libertatis Team (see the AUTHORS file)
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

#include "game/EntityId.h"

#include <stddef.h>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include "game/Entity.h"
#include "io/resource/ResourcePath.h"

const EntityId EntityId::self("self", 0);

EntityId::EntityId(const res::path & classPath, EntityInstance instance)
	: m_className(classPath.filename())
	, m_instance(instance)
{ }

EntityId::EntityId(const std::string & id)
	: m_instance(-1)
{
	if(id.empty() || id == "none") {
		
		// empty className, instance = -1
		
	} else {
		
		size_t sep = id.find_last_of('_');
		
		m_instance = 0;
		if(sep != std::string::npos) {
			try {
				m_instance = boost::lexical_cast<EntityInstance>(id.c_str() + sep + 1);
			} catch(...) {
				m_className = id;
				m_instance = -1;
				return;
			}
		}
		
		m_className = id.substr(0, sep);
	}
}

std::string EntityId::string() const {
	return boost::lexical_cast<std::string>(*this);
}

std::ostream & operator<<(std::ostream & os, const EntityId & id) {
	if(id.className().empty()) {
		os << "none";
	} else {
		os << id.className();
		if(id.instance() > 0) {
			os << '_';
			char c = os.fill('0');
			std::ios_base::fmtflags f = os.flags(std::ios_base::dec);
			std::streamsize w = os.width(4);
			os << id.instance();
			os.width(w);
			os.flags(f);
			os.fill(c);
		}
	}
	return os;
}
