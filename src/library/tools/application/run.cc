/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/commandlineparser.h>
 #include <udjat/agent/abstract.h>
 #include <udjat/tools/intl.h>
 #include <udjat/module/abstract.h>
 #include <udjat/ui/console.h>
 #include <udjat/tools/intl.h>
 #include <udjat/ui/status.h>
 #include <string>
 #include <udjat/agent/abstract.h>
 #include <private/agent.h>
 #include <private/service.h>

 #undef LOG_DOMAIN
 #define LOG_DOMAIN Application::Name();
 #include <udjat/tools/logger.h>

 #include <iostream>     // std::cout, std::ostream, std::ios
 #include <fstream>      // std::filebuf

 #ifdef _WIN32
	#include <private/win32/mainloop.h>
	#include <udjat/win32/exception.h>
	#include <private/event.h>
 #else
	#include <sys/resource.h>
	#include <csignal>
	#include <udjat/tools/event.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	void Application::root(std::shared_ptr<Abstract::Agent>) {
	}

	bool Application::setProperty(const char *name, const char *value) {

		debug("Property: '",name,"'('",(value ? value : "NULL"),"')");

#ifdef _WIN32
		if(!SetEnvironmentVariable(name,value)) {
			throw Win32::Exception(_("Unable to set environment variable"));
		}
#else
		if(setenv(name, value, 1)) {
			throw std::system_error(errno,std::system_category(),_("Unable to set environment variable"));
		}
#endif // _WIN32

		return true;
	}

	static void dump(std::shared_ptr<Abstract::Agent> agent, size_t level = 0) {

		Logger::String{
			string(level*2, ' ').c_str(),
			agent->label(),
			" - ",
			agent->summary()
		}.write((Logger::Level) (Logger::Debug+1),agent->name());

		for(const auto child : *agent) {
			dump(child, level + 1);
		}

	}

	void Application::parse(const char *path) {

		// XML parse will run in a thread.
		ThreadPool::getInstance().push([this,path](){

			// Load new configuration, if it fails, the old configuration will be kept.
			shared_ptr<Abstract::Agent> root;

			try {

				Logger::String{"Loading ",path}.trace();
				state( _("Loading configuration") );

				// TODO: Load XML definitions.
				root = RootFactory();
				time_t refresh = root->parse(path);

				if(refresh) {

					time_t seconds = refresh - time(0);
					Logger::String{"Update of ",path," scheduled to ",TimeStamp{refresh}.to_string().c_str()," (",seconds," seconds from now)"}.info(name());

					reload_timer = MainLoop::getInstance().TimerFactory(seconds * 1000,[this,path]() -> bool {
						Logger::String{"Reloading ",path," by timer action"}.info(name());
						reload_timer = nullptr;
						this->parse(path);
						return false;
					});

				} else {

					Logger::String{"Automatic update of ",path," disabled"}.trace(name());

				}

			} catch(const std::exception &e) {

				Logger::String{"Error while loading ",path,": ",e.what()}.error(name());
				return;

			} catch(...) {

				Logger::String{"Unexpected error while loading ",path}.error(name());
				return;

			}

			// New root agent is ready, activate it.
			try {

				// Activate the new root agent.
				state( _("Activating new configuration") );

				if(Logger::enabled(Logger::Trace)) {
					dump(root);
				}

				// Set the root agent for application.
				this->root(root);

				// Set the root agent on controller.
				Abstract::Agent::Controller::getInstance().set(root);

				// Inform the modules about the new root agent.
                Module::for_each([root](const Module &module){
					const_cast<Module &>(module).set(root);
					return false;
                });

				state( _("Starting services") );
				Service::Controller::getInstance().start();

			} catch(const std::exception &e) {

				state(e.what());
				MainLoop::getInstance().quit(e.what());

			} catch(...) {

				state(_("Unexpected error while activating new configuration"));
				MainLoop::getInstance().quit("Unexpected error");
			}

		});

	}

	int Application::run(const char *definitions) {

		if(has_argument('h',"help",true)) {
			help();
			cout << "\n";
			Logger::help();
			return 0;
		}

		// Parse command line arguments.
		CommandLineParser::setup(argc,argv);

		if(!MainLoop::getInstance()) {
			return -1;
		}

		{
			string argvalue;

			if(get_argument(argc,argv,'T',"timer",argvalue)) {
				MainLoop::getInstance().TimerFactory(((time_t) TimeStamp{argvalue.c_str()}) * 1000,[](){
					MainLoop::getInstance().quit("Timer expired, exiting");
					return false;
				});
			}

		}

#ifdef _WIN32
			Udjat::Event::ConsoleHandler(this,CTRL_C_EVENT,[](){
				MainLoop::getInstance().quit("Terminating by ctrl-c event");
				return false;
			});

			Udjat::Event::ConsoleHandler(this,CTRL_CLOSE_EVENT,[](){
				MainLoop::getInstance().quit("Terminating by close event");
				return false;
			});

			Udjat::Event::ConsoleHandler(this,CTRL_SHUTDOWN_EVENT,[](){
				MainLoop::getInstance().quit("Terminating by shutdown event");
				return false;
			});
#endif // _WIN32

		try {

			if(definitions && *definitions) {

				const char *path = String{definitions}.expand(true).as_quark();
				
#ifndef _WIN32
				// Sighup force reconfiguration.
				Event::SignalHandler(this,SIGHUP,[this,path]() -> bool {
					Logger::String{"Catched SIGHUP, reloading ",path}.info(name());
					parse(path);
					return true;
				});

				Logger::String{
					"Signal '",(const char *) strsignal(SIGHUP),"' (SIGHUP) will reload settings from ",path
				}.info(name());
#endif // _WIN32

				parse(path);

			} else {

				state( _("Starting services") );
				Service::Controller::getInstance().start();

			}

			// Start the main loop.
			debug("----> Starting mainloop");
			int rc = MainLoop::getInstance().run();
			debug("----> MainLoop exited with code ",rc);
	
			Service::Controller::getInstance().stop();

			return rc;

		} catch(const std::exception &e) {

			error() << e.what() << endl;

		} catch(...) {

			error() << "Unexpected error" << endl;

		}

		Service::Controller::getInstance().stop();

		return -1;

	}

 }


