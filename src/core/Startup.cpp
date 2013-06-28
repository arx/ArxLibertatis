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


#include <stddef.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <set>
#include <sstream>
#include <algorithm>
#include <cstdlib>

#include "platform/Platform.h"

#if ARX_PLATFORM == ARX_PLATFORM_WIN32
#include <windows.h>
#include <stdlib.h>
#endif

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "core/Config.h"
#include "core/Core.h"
#include "core/Version.h"
#include "io/fs/Filesystem.h"
#include "io/fs/SystemPaths.h"
#include "io/log/CriticalLogger.h"
#include "io/log/FileLogger.h"
#include "io/log/Logger.h"
#include "math/Random.h"
#include "platform/CrashHandler.h"
#include "platform/Environment.h"
#include "platform/ProgramOptions.h"
#include "platform/Time.h"
#include "util/String.h"

namespace {

struct ParsedOption {
	
	enum OptionType {
		Invalid,
		Long,
		Short
	};

	OptionType                  m_type;
	std::string                 m_name;
	std::list<std::string>      m_arguments;
	
	ParsedOption() : m_type(Invalid) {
	}
	
	void reset() {
		m_type = Invalid;
		m_name.clear();
		m_arguments.clear();
	}
	
};

} // anonymous namespace

void ShowHelp() {
	
	// Register all program options in the command line interpreter
	interpreter<std::string> cli;
	BaseOption::registerAll(cli);
	
	std::cout << "Arx Libertatis Options:" << std::endl;
	std::cout << cli << std::endl;
	
	std::exit(EXIT_SUCCESS);
}

ARX_PROGRAM_OPTION("help", "h", "Show supported options", &ShowHelp);

static ExitStatus parseCommandLine(int argc, char ** argv) {
	
	defineSystemDirectories(argv[0]);
	
	// Register all program options in the command line interpreter
	interpreter<std::string> cli;
	BaseOption::registerAll(cli);
	
	std::vector<std::string> args;

	if(argc) {
		std::copy(argv + 1, argv + argc, std::inserter(args, args.end()));
	}
	
	std::list<ParsedOption> allOptions;
	ParsedOption currentOption;
	
	// Pass 1: Split command-line arguments into options and commands
	BOOST_FOREACH(std::string & token, args) {
		
		ParsedOption::OptionType newOptionType(ParsedOption::Invalid);
		
		if(boost::algorithm::starts_with(token, "--")) {                  // handle long options
			token.erase(0, 2);
			newOptionType = ParsedOption::Long;
		} else if(boost::algorithm::starts_with(token, "-")) {            // handle short options
			token.erase(0, 1);
			newOptionType = ParsedOption::Short;
		} else if(currentOption.m_type != ParsedOption::Invalid) {
			currentOption.m_arguments.push_back(token);
		} else {
			// ERROR: invalid command line
			std::cerr << "Error parsing command-line: "
			          << "commands must start with at least one dash: " << token << "\n\n";
			ShowHelp();
			return ExitFailure;
		}
		
		if(newOptionType != ParsedOption::Invalid) {
			
			if(currentOption.m_type != ParsedOption::Invalid) {
				allOptions.push_back(currentOption);
				currentOption.reset();
			}
			
			currentOption.m_type = newOptionType;
			
			std::string::size_type p = token.find('=');
			if(p != token.npos) {
				currentOption.m_name = token.substr(0, p);
				std::string arg = token.substr(p+1);
				if(arg.empty()) {
					// ERROR: invalid command line
				} else {
					currentOption.m_arguments.push_back(arg);
				}
			} else {
				currentOption.m_name = token.substr(0, p);
			}
			
		}
	}
	
	if(currentOption.m_type != ParsedOption::Invalid) {
		allOptions.push_back(currentOption);
		currentOption.reset();
	}
	
	// Pass 2: Process all command line options received
	interpreter<std::string>::type_cast_t tc;
	BOOST_FOREACH(const ParsedOption & option, allOptions) {
		try {
			cli.invoke(option.m_name, option.m_arguments.begin(), option.m_arguments.end(), tc);
		} catch(command_line_exception & e) {
			std::cerr << "Error parsing command-line option ";
			if(option.m_type == ParsedOption::Long) {
				std::cerr << "--";
			} else if(option.m_type == ParsedOption::Short) {
				std::cerr << "-";
			}
			std::cerr << option.m_name;
			BOOST_FOREACH(const std::string & arg, option.m_arguments) {
				std::cerr << ' ';
				std::cerr << util::escapeString(arg, "\\\" '$!");
			}
			std::cerr << ": " << e.what() << "\n\n";
			ShowHelp();
			return ExitFailure;
		}
	}
	
	return RunProgram;
}


#if ARX_PLATFORM != ARX_PLATFORM_WIN32
extern int main(int argc, char ** argv) {
#else
// For windows we can't use main() in a GUI application
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine,
                   INT nCmdShow) {
	ARX_UNUSED(hInstance);
	ARX_UNUSED(hPrevInstance);
	ARX_UNUSED(lpCmdLine);
	ARX_UNUSED(nCmdShow);
	int argc = __argc;
	char ** argv = __argv;
#endif
	
	// Initialize Random now so that the crash handler can use it
	Random::seed();
	
	// Initialize the crash handler
	{
		CrashHandler::initialize();
		CrashHandler::setVariable("Compiler", ARX_COMPILER_VERNAME);
		CrashHandler::setVariable("Boost version", BOOST_LIB_VERSION);
		std::string command_line;
		for(int i = 1; i < argc; i++) {
			command_line += util::escapeString(argv[i], "\\\" '$!");
			command_line += ' ';
		}
		CrashHandler::setVariable("Command line", command_line);
	}
	
	// Also intialize the logging system early as we might need it
	Logger::initialize();
	CrashHandler::registerCrashCallback(Logger::quickShutdown);
	Logger::add(new logger::CriticalErrorDialog);
	
	// Parse the command line and process options
	ExitStatus status = parseCommandLine(argc, argv);
	
	// Setup user, config and data directories
	if(status == RunProgram) {
		status = fs::paths.init();
	}
	
	if(status == RunProgram) {
		
		// Configure the crash report location
		CrashHandler::setReportLocation(fs::paths.user / "crashes");
		CrashHandler::deleteOldReports(/* nb to keep = */5);
		
		// Now that data directories are initialized, create a log file
		{
			fs::path logFile = fs::paths.user / "arx.log";
			Logger::add(new logger::File(logFile));
			CrashHandler::addAttachedFile(logFile);
		}
		
		Time::init();
		
		// Start the game already!
		LogInfo << "Starting " << version;
		runGame();
		
	}
	
	// Shutdown the logging system
	// If there has been a critical error, a dialog will be shown now
	Logger::shutdown();
	
	CrashHandler::shutdown();
	
	return (status == ExitFailure) ? EXIT_FAILURE : EXIT_SUCCESS;
}
