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
 #include <private/mainloop.h>
 #include <cstring>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>

 using namespace std;

 namespace Udjat {

	mutex MainLoop::Service::guard;

	void MainLoop::start() noexcept {

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

	void MainLoop::stop() noexcept {

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

	MainLoop::Service::Service(const char *name, const ModuleInfo &i) : module(i), service_name(name) {
		lock_guard<mutex> lock(guard);
		if(!service_name) {
			service_name = strrchr(module.name,'-');
			if(service_name) {
				service_name++;
			} else {
				service_name = module.name;
			}
		}
		MainLoop::getInstance().services.push_back(this);
	}

	MainLoop::Service::Service(const ModuleInfo &module) : Service(nullptr,module) {
	}

	MainLoop::Service::~Service() {
		lock_guard<mutex> lock(guard);
		MainLoop::getInstance().services.remove_if([this](Service *s) {
			return s == this;
		});
	}

	void MainLoop::Service::start() {
		state.active = true;
	}

	void MainLoop::Service::stop() {
		state.active = false;
	}

	std::ostream & MainLoop::Service::info() const {
		cout << name() << "\t";
		return cout;
	}

	std::ostream & MainLoop::Service::warning() const {
		clog << name() << "\t";
		return clog;
	}

	std::ostream & MainLoop::Service::error() const {
		cerr << name() << "\t";
		return cerr;
	}

	void MainLoop::Service::getInfo(Response &response) {

		lock_guard<mutex> lock(guard);
		response.reset(Value::Array);

		for(auto service : MainLoop::getInstance().services) {

			Value &object = response.append(Value::Object);

			object["name"] = service->service_name;
			object["active"] = service->state.active;

			service->module.get(object);


		}
	}

 }
