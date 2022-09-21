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

/**
 * @file main/load/xml.cc
 *
 * @brief Implements the application settings loader.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/misc.h>
 #include <sys/stat.h>
 #include <udjat/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/timer.h>
 #include <udjat/module.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
 #include <private/updater.h>
 #include <list>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 using namespace std;

 namespace Udjat {

	UDJAT_API time_t reconfigure(const char *pathname, bool force) {

		Updater update(pathname);

		if(update || force) {
			return update.set(RootAgentFactory());
		}

		Application::info() << "Reconfiguration is not necessary" << endl;
		return update.time();

	}

	UDJAT_API time_t reconfigure(std::shared_ptr<Abstract::Agent> agent, const char *pathname, bool force) {

		Updater update(pathname);

		if(update || force) {
			update.set(agent);
		} else {
			Application::info() << "No changes, reconfiguration is not necessary" << endl;
		}

#ifdef HAVE_SYSTEMD
		sd_notifyf(0,"STATUS=%s",Abstract::Agent::root()->state()->to_string().c_str());
#endif // HAVE_SYSTEMD

		return update.time();

	}

	void SystemService::reconfigure(const char *pathname, bool force) noexcept {

		// Create reconfig timer.
		class ReconfigTimer : public MainLoop::Timer {
		protected:
			void on_timer() override {
				ThreadPool::getInstance().push("system-reconfigure",[]{
					if(instance) {
						instance->reconfigure(instance->definitions,false);
					}
				});
			}

		public:
			ReconfigTimer() = default;

			void set(time_t seconds) {

				if(seconds) {

					reset(seconds * 1000);
					enable();

					TimeStamp next(time(0)+seconds);
					Application::info() << "Auto reconfiguration set to " << next << endl;
#ifdef _WIN32
					SystemService::getInstance()->registry("auto-reconfig",next.to_string().c_str());
#endif // _WIN32

				} else {

					disable();

					Application::info() << "Auto reconfiguration is not enabled" << endl;
#ifdef _WIN32
					SystemService::getInstance()->registry("auto-reconfig","disabled");
#endif // _WIN32
				}
			}

		};

		static ReconfigTimer timer;

		try {

			Updater update(pathname);

			if(update || force) {

				auto agent = RootFactory();
				update.set(agent);

#ifdef _WIN32
				registry("last_reconfig",TimeStamp().to_string().c_str());
#endif // _WIN32

			} else {

				info() << "Reconfiguration is not necessary" << endl;
			}

			timer.set(update.time());

		} catch(const std::exception &e ) {

			error() << "Reconfiguration has failed: " << e.what() << endl;
			timer.set(Config::Value<time_t>("service","reconfig-time-when-failed",120000));

		} catch(...) {

			error() << "Unexpected error during reconfiguration" << endl;
			timer.set(Config::Value<time_t>("service","reconfig-time-when-failed",120000));

		}

		notify(Abstract::Agent::root()->state()->to_string().c_str());

	}

 }
