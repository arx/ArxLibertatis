
#include "io/log/Logger.h"

#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <boost/unordered/unordered_map.hpp>

#include "io/log/ConsoleLogger.h"
#include "io/log/LogBackend.h"
#include "io/log/MsvcLogger.h"
#include "platform/Platform.h"

#include "Configure.h"

using std::string;

namespace logger {

namespace {

struct Manager {
	
	static const Logger::LogLevel defaultLevel;
	
	//! note: using the pointer value of a string constant as a hash map index.
	typedef boost::unordered_map<const char *, Source> Sources;
	static Sources sources;
	
	typedef std::vector<Backend *> Backends;
	static Backends backends;
	
	typedef boost::unordered_map<string, Logger::LogLevel> Rules;
	static Rules rules;
	
	~Manager();
	
};

const Logger::LogLevel Manager::defaultLevel = Logger::Info;
Manager::Sources Manager::sources;
Manager::Backends Manager::backends;
Manager::Rules Manager::rules;

Manager::~Manager() {
	for(Manager::Backends::const_iterator i = Manager::backends.begin();
	    i != Manager::backends.end(); ++i) {
		delete *i;
	}
	backends.clear();
}

} // anonymous namespace

} // namespace logger

void Logger::add(logger::Backend * backend) {
	if(backend != NULL) {
		logger::Manager::backends.push_back(backend);
	}
}

void Logger::remove(logger::Backend * backend) {
	logger::Manager::backends.erase(std::remove(logger::Manager::backends.begin(),
	                                            logger::Manager::backends.end(),
	                                            backend),
	                                logger::Manager::backends.end());
}

const logger::Source * Logger::get(const char * file, LogLevel level) {
	
	logger::Source & source = logger::Manager::sources[file];
	
	if(source.name.empty()) {
		
		source.file = file;
		source.level = logger::Manager::defaultLevel;
		
		const char * end = file + strlen(file);
		bool first = true;
		for(const char * p = end; p != file; p--) {
			if(*p == '/' || *p == '\\') {
				
				string component(p + 1, end);
				
				if(first) {
					size_t pos = component.find_last_of('.');
					if(pos != string::npos) {
						component.resize(pos);
					}
					source.name = component;
					first = false;
				}
				
				logger::Manager::Rules::const_iterator i = logger::Manager::rules.find(component);
				if(i != logger::Manager::rules.end()) {
					source.level = i->second;
					break;
				}
				
				if(component == "src" || component == "tools") {
					break;
				}
				
				end = p;
			}
		}
		
	}
	
	return (source.level <= level) ? &source : NULL;
}

void Logger::log(const logger::Source & file, int line, LogLevel level, const string & str) {
	if(level != None) {
		for(logger::Manager::Backends::const_iterator i = logger::Manager::backends.begin();
			  i != logger::Manager::backends.end(); ++i) {
			(*i)->log(file, line, level, str);
		}
	}
}

void Logger::set(const string & prefix, Logger::LogLevel level) {
	logger::Manager::rules[prefix] = level;
	logger::Manager::sources.clear();
}

void Logger::reset(const string & prefix) {
	logger::Manager::rules.erase(prefix);
	logger::Manager::sources.clear();
}

void Logger::flush() {
	for(logger::Manager::Backends::const_iterator i = logger::Manager::backends.begin();
	    i != logger::Manager::backends.end(); ++i) {
		(*i)->flush();
	}
}

void Logger::configure(const string config) {
	
	size_t start = 0;
	
	while(start < config.length()) {
		
		size_t pos = config.find(',', start);
		if(pos == string::npos) {
			pos = config.length();
		}
		if(pos == start) {
			start++;
			continue;
		}
		
		string entry = config.substr(start, pos - start);
		start = pos + 1;
		
		size_t eq = entry.find('=');
		string level;
		if(eq != string::npos) {
			level = entry.substr(eq + 1), entry.resize(eq);
		}
		
		if(level.empty() || level == "debug" || level == "d" || level == "D") {
			set(entry, Debug);
		} else if(level == "info" || level == "i" || level == "I") {
			set(entry, Info);
		} else if(level == "warning" || level == "warn", level == "w" || level == "W") {
			set(entry, Warning);
		} else if(level == "error" || level == "w" || level == "E") {
			set(entry, Error);
		} else if(level == "none" || level == "n" || level == "N") {
			set(entry, None);
		} else if(level == "reset" || level == "r" || level == "R" || level == "-") {
			reset(entry);
		}
		
	}
	
}

void Logger::init() {
	
	Logger::add(logger::Console::get());
	
#ifdef HAVE_WINAPI
	Logger::add(logger::MsvcDebugger::get());
#endif
	
	const char * arxdebug = getenv("ARXDEBUG");
	if(arxdebug) {
		configure(arxdebug);
	}
}
