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

 #include <cstring>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/service.h>
 #include <private/win32/mainloop.h>

 using namespace std;

 namespace Udjat {

	void Win32::MainLoop::push_back(Udjat::MainLoop::Service *service) {
		services.push_back(service);
	}

	void Win32::MainLoop::remove(Udjat::MainLoop::Service *service) {
		services.remove_if([service](Service *s) {
			return s == service;
		});
	}

	void MainLoop::Service::start(std::list<MainLoop::Service *> &services) noexcept {

		ThreadPool::getInstance();

		{
			lock_guard<mutex> lock(Service::guard);
			cout << "mainloop\tStarting " << services.size() << " service(s)" << endl;
			for(auto service : services) {
				if(!service->state.active) {
					try {
						cout << "services\tStarting '" << service->name() << "' (" << service->description() << " " << service->version() << ")" << endl;
						service->start();
						service->state.active = true;
					} catch(const std::exception &e) {
						service->error() << "Error '" << e.what() << "' starting service" << endl;
					} catch(...) {
						service->error() << "Unexpected error starting service" << endl;
					}
				}
			}
		}
	}

	void MainLoop::Service::stop(std::list<MainLoop::Service *> &services) noexcept {

		{
			lock_guard<mutex> lock(Service::guard);

			Logger::String("Stopping ",services.size()," service(s)").write(Logger::Trace,"mainloop");

			// Stop services in reverse order.
			size_t count = 0;
			for(auto srvc = services.rbegin(); srvc != services.rend(); srvc++) {
				Service *service = *srvc;
				if(service->state.active) {
					try {
						Logger::String("Stopping '",service->name(),"' (",(++count),"/",services.size(),")").write(Logger::Trace,"mainloop");
						service->stop();
					} catch(const std::exception &e) {
						service->error() << "Error '" << e.what() << "' stopping service" << endl;
					} catch(...) {
						service->error() << "Unexpected error stopping service" << endl;
					}
					service->state.active = false;
				}
				else {
					Logger::String("Service '",service->name(),"' is already stopped (",(++count),"/",services.size(),")").write(Logger::Trace,"mainloop");
				}
			}
		}

		ThreadPool::getInstance().wait();

	}

 }
