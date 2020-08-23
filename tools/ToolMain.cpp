/*
 * Copyright 2020 Arx Libertatis Team (see the AUTHORS file)
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

#include <iostream>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "io/fs/FilePath.h"
#include "platform/WindowsMain.h"

#include "savetool/SaveTool.h"
#include "unpak/UnPak.h"

static int run_main(const char * tool, int argc, char ** argv) {
	
	if(boost::equals(tool, "save")) {
		return arxsavetool_main(argc, argv);
	} else if(boost::equals(tool, "unpak")) {
		return arxunpak_main(argc, argv);
	}
	
	return -1;
}

int utf8_main(int argc, char ** argv) {
	
	std::string tool;
	if(argc > 0) {
		tool = boost::to_lower_copy(fs::path(argv[0]).basename());
		if(boost::starts_with(tool, "arx")) {
			tool.erase(0, 3);
		}
		if(boost::ends_with(tool, "tool")) {
			tool.resize(tool.size() - 4);
		}
	}
	
	int ret = -1;
	if(!tool.empty()) {
		ret = run_main(tool.c_str(), argc, argv);
	}
	
	if(ret == -1 && argc > 1) {
		ret = run_main(argv[1], argc - 1, argv + 1);
	}
	
	if(ret == -1) {
		std::cerr << "Usage: arxtool <tool> ...\n\nWhere <tool> is one of:\n- save\n- unpak\n";
		ret = 1;
	}
	
	return ret;
}
