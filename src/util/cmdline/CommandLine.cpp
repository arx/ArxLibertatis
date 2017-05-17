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

#include "util/cmdline/CommandLine.h"

#include <iostream>

#include "core/Version.h"
#include "io/log/Logger.h"
#include "platform/Environment.h"
#include "platform/ProgramOptions.h"
#include "util/cmdline/Parser.h"

static void showCommandLineHelp(const util::cmdline::interpreter<std::string> & options,
                                std::ostream & os) {
	
	std::string command = platform::getCommandName();
	os << "Usage: " << (command.empty() ? "arx" : command.c_str()) << " [options]\n\n";
	
	os << arx_name << " Options:\n";
	os << options << std::endl;
	
}

static void handleHelpOption() {
	
	// Register all program options in the command line interpreter
	util::cmdline::interpreter<std::string> cli;
	BaseOption::registerAll(cli);
	
	showCommandLineHelp(cli, std::cout);
	
	std::exit(EXIT_SUCCESS);
}

ARX_PROGRAM_OPTION("help", "h", "Show supported options", &handleHelpOption)

ExitStatus parseCommandLine(int argc, char ** argv) {
	
	platform::initializeEnvironment(argv[0]);
	
	// Register all program options in the command line interpreter
	util::cmdline::interpreter<std::string> cli;
	BaseOption::registerAll(cli);
	
	try {
		
		util::cmdline::parse(cli, argc, argv);
		
	} catch(util::cmdline::error & e) {
		
		LogCritical << e.what();
		std::cerr << "\n";
		showCommandLineHelp(cli, std::cerr);
		
		return ExitFailure;
	}
	
	return RunProgram;
}
