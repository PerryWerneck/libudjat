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

 #include <cstring>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/mainloop.h>
 #include <private/service.h>

 using namespace std;

 namespace Udjat {

	void Service::Controller::start() noexcept {

		ThreadPool::getInstance();

		lock_guard<mutex> lock(guard);
		Logger::String{"Starting ", objects.size(), " service(s)"}.trace("services");
		for(auto service : objects) {
			if(!service->state.active) {
				try {
					Logger::String{"Starting ", service->description(), " ", service->version()}.info(service->name());
					service->start();
					service->state.active = true;
				} catch(const std::exception &e) {
					Logger::String{"Error '", e.what(), "' starting service"}.error(service->name());
				} catch(...) {
					Logger::String{"Unexpected error starting service"}.error(service->name());
				}
			}
		}

	}

	void Service::Controller::stop() noexcept {

		{
			lock_guard<mutex> lock(guard);

			Logger::String{"Stopping ",objects.size()," service(s)"}.trace("services");

			// Stop services in reverse order.
			size_t count = 0;
			for(auto srvc = objects.rbegin(); srvc != objects.rend(); srvc++) {
				Service *service = *srvc;
				if(service->state.active) {
					try {
						Logger::String{"Stopping '",service->name(),"' (",(++count),"/",objects.size(),")"}.trace("services");
						service->stop();
					} catch(const std::exception &e) {
						Logger::String{"Error '",e.what(),"' stopping service"}.error(service->name());
					} catch(...) {
						Logger::String{"Unexpected error stopping service"}.error(service->name());
					}
					service->state.active = false;
				}
				else {
					Logger::String("Service '",service->name(),"' is already stopped (",(++count),"/",objects.size(),")").trace("services");
				}
			}
		}

		ThreadPool::getInstance().wait();

	}

 }
