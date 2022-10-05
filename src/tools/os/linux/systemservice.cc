/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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
 #include <private/misc.h>
 #include <udjat/tools/systemservice.h>
 #include <iostream>
 #include <system_error>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module.h>
 #include <unistd.h>
 #include <fstream>
 #include <sys/time.h>
 #include <sys/resource.h>
 #include <locale.h>
 #include <udjat/agent.h>
 #include <csignal>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/application.h>
 #include <syslog.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	void SystemService::notify(const char *message) noexcept {

		if(message && *message) {

#ifdef HAVE_SYSTEMD
			sd_notifyf(0,"STATUS=%s",message);
#endif // HAVE_SYSTEMD

			Logger::Writer * writer = dynamic_cast<Logger::Writer *>(cout.rdbuf());

			if(writer && writer->get_console() && (getenv("TERM") != NULL)) {
				writer->write(1,"\x1b[96m");
				writer->timestamp(1);
				for(size_t ix = 0; ix < 15;ix++) {
					writer->write(1," ");
				}
				writer->write(1,message);
				writer->write(1,"\x1b[0m\n");
			}

			syslog(LOG_NOTICE,"%s",message);

		}

	}

	void SystemService::init() {

		string appconfig;
		if(!definitions) {
			Config::Value<string> config("service","definitions","");
			if(config.empty()) {
				definitions = Quark(Application::DataDir("xml.d")).c_str();
			} else {
				definitions = Quark(config).c_str();
			}
		}

		if(!Module::preload(definitions)) {
			throw runtime_error("Module preload has failed, aborting service");
		}

		if(definitions[0] && strcasecmp(definitions,"none")) {

			info() << "Loading service definitions from " << definitions << endl;

			reconfigure(definitions,true);

			Config::Value<string> signame("service","signal-reconfigure","SIGHUP");
			if(!signame.empty() && strcasecmp(signame.c_str(),"none")) {
				Udjat::Event &reconfig = Udjat::Event::SignalHandler(this,signame.c_str(),[this](){
					reconfigure(definitions,false);
					return true;
				});
				info() << signame << " (" << reconfig.to_string() << ") triggers a conditional reload" << endl;
			}

		}

	}

	void SystemService::deinit() {
		Udjat::Event::remove(this);
	}

	void SystemService::stop() {
		MainLoop::getInstance().quit();
	}

	void SystemService::usage() const noexcept {
		cout 	<< "Usage: " << endl << "  " << name() << " [options]" << endl << endl
				<< "  --core\t\tenable coredumps" << endl
				<< "  --timer=seconds\tTerminate " << name() << " after 'seconds'" << endl
				<< "  --daemon\t\tRun " << name() << " service in the background" << endl
				<< "  --foreground\t\tRun " << name() << " service as application (foreground)" << endl;
	}

	int SystemService::cmdline(char key, const char *value) {

		switch(key) {
		case 'T':	// Auto quit
			{
				if(!value) {
					throw system_error(EINVAL,system_category(),_( "Invalid timer value" ));
				}

				int seconds = atoi(value);
				if(!seconds) {
					throw system_error(EINVAL,system_category(),_( "Invalid timer value" ));
				}

				MainLoop::getInstance().TimerFactory(seconds * 1000,[](){
					Application::warning() << "Exiting by timer request" << endl;
					MainLoop::getInstance().quit();
					return false;
				});

			}
			return 0;

		case 'C':	// Enable core dumps.
			{
				if(optarg && *optarg) {
					ofstream ofs;
					ofs.open("/proc/sys/kernel/core_pattern",ofstream::out);
					ofs << optarg;
					ofs.close();
				}

				// Enable cores
				struct rlimit core_limits;
				core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;

				if(setrlimit(RLIMIT_CORE, &core_limits)) {
					error() << "Error \"" << strerror(errno) << "\" activating coredumps" << endl;
				} else {
					info() << "Coredumps are active" << endl;
				}
			}
			return -2;

		}

		return ENOENT;
	}

	int SystemService::cmdline(const char *key, const char *value) {

		static const struct {
			char option;
			const char *key;
		} options[] = {
			{ 'C', "core" },
			{ 'd', "daemon" },
			{ 'f', "foreground" },
			{ 'T', "timer" }
		};

		for(size_t option = 0; option < (sizeof(options)/sizeof(options[0])); option++) {
			if(!strcasecmp(key,options[option].key)) {
				return cmdline(options[option].option,value);
			}
		}

		return ENOENT;
	}

	int SystemService::run(int argc, char **argv) {

		int rc = 0;

		if(argc > 1) {
			rc = cmdline(argc,(const char **) argv);
			if(rc) {
				mode = SERVICE_MODE_NONE;
			}
		}

		Logger::redirect(mode == SERVICE_MODE_FOREGROUND ? true : false);

		if(mode == SERVICE_MODE_DAEMON) {
			if(daemon(0,0)) {
				error() << strerror(errno) << endl;
				return -1;
			}
		}

		if(mode != SERVICE_MODE_NONE) {

			try {

				init();
				rc = run();
				deinit();

			} catch(const std::exception &e) {

				error() << e.what() << endl;
				rc = -1;

			}
		}

		return rc;

	}

 }

