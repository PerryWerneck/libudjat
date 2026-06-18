/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/timestamp.h>
 #include <private/logger.h>
 #include <mutex>
 #include <cstring>

 #ifdef HAVE_SYSLOG
	#include <syslog.h>
 #endif

 #ifndef _WIN32
	#include <dlfcn.h>
 #endif // !_WIN32

 using namespace std;

 namespace Udjat {

	Logger::Controller::Controller() {

#ifdef HAVE_SYSLOG
		// Have syslog, allways enable it.
		{
			::openlog(NULL, LOG_PID, LOG_DAEMON);
			insert("syslog",BackEnd::SysLog,[this](Logger::Level level, const char *, const char *domain, const char *text){

				static const int priority[] = {
					LOG_ERR,		// Error
					LOG_WARNING,	// Warning
					LOG_NOTICE,		// Notice
					LOG_INFO,		// Info
					LOG_DEBUG,		// Trace
					LOG_DEBUG,		// Debug
				};
				::syslog(priority[verbosity()],"%s: %s",domain,text);

			});	
		}
#endif // HAVE_SYSLOG

#ifndef _WIN32
		// Check for GLib (possible GUI app).
		{

			typedef void (*GLogFunc)(const char *log_domain, GLogLevelFlags log_level, const char *message, void *user_data);

			dlerror();


			void (*g_log_set_default_handler)(GLogFunc, void *)
					= (void (*)(GLogFunc,void *)) dlsym(RTLD_DEFAULT, "g_log_set_default_handler");

			if(!dlerror()) {
				// GLib is present, capture logs, disable console output
				g_log_set_default_handler(g_logger,this);
				console(false);
			} else {
				// GLib is not present, enable console output.
				console(true);
			}

		}
#else

#endif // !_WIN32

	}

	Logger::Controller::~Controller() {

#ifdef HAVE_SYSLOG
		closelog();
#endif

	}

	Logger::Controller & Logger::Controller::getInstance() {
		static Controller instance;
		return instance;	
	}

	bool Logger::Controller::enabled(BackEnd::Type type) const noexcept {
		for(const auto &backend : backends) {
			if(backend.type == type) {
				return true;
			}
		}
		return false;
	}

	void Logger::Controller::insert(const char *name, const BackEnd::Type type,const std::function<void(Logger::Level level, const char *timestamp, const char *domain, const char *text)> &call) {
		lock_guard<recursive_mutex> lock(guard);
		remove(name);
		backends.emplace_back(name,type,call);
	}

	void Logger::Controller::remove(const char *name) {
		lock_guard<recursive_mutex> lock(guard);
		backends.remove_if([name](BackEnd &backend){
			return strcasecmp(name,backend.name) == 0;
		});
	}

	void Logger::Controller::write(Level level, const char *domain, const char *text) {

		if(!(level & enabled_levels)) {
			return;
		}

		auto timestamp = TimeStamp{}.to_string();
		
		lock_guard<recursive_mutex> lock(guard);
		for(const auto &backend : backends) {
			try {
				backend.call(level,timestamp.c_str(),domain,text);
			} catch(const std::exception &e) {
#ifdef HAVE_SYSLOG
				::syslog(LOG_ERR,"Error write log: %s",e.what());
#endif
			} catch(...) {
#ifdef HAVE_SYSLOG
				::syslog(LOG_ERR,"Unexpected error writing log");
#endif
			}
		}

	}

#ifndef _WIN32
	void Logger::Controller::g_logger(const char *domain, GLogLevelFlags level, const char *message, void *userdata) {

		auto &controller = getInstance();

		static const struct Type {
				GLogLevelFlags          level;
				Udjat::Logger::Level    lvl;
		} types[] = {
				{ G_LOG_FLAG_RECURSION, Udjat::Logger::Info             },
				{ G_LOG_FLAG_FATAL,     Udjat::Logger::Error            },

				// GLib log levels
				{ G_LOG_LEVEL_ERROR,    Udjat::Logger::Error            },
				{ G_LOG_LEVEL_CRITICAL, Udjat::Logger::Error            },
				{ G_LOG_LEVEL_WARNING,  Udjat::Logger::Warning          },
				{ G_LOG_LEVEL_MESSAGE,  Udjat::Logger::Info             },
				{ G_LOG_LEVEL_INFO,     Udjat::Logger::Info             },
				{ G_LOG_LEVEL_DEBUG,    Udjat::Logger::Debug            },
		};

		if(!domain) {
			domain = "gtk";
		}

		for(const auto &type : types) {
			if(type.level == level) {
				controller.write(type.lvl, domain, message);
				return;
			}
		}

		controller.write(Error, domain, message);

	}
#endif // !_WIN32

 }