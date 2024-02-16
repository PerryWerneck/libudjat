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
 #include <udjat/module/abstract.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/timer.h>
 #include <udjat/module/abstract.h>
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

	int Application::install(const char *) {
		return ENOTSUP;
	}

	int Application::uninstall() {
		return ENOTSUP;
	}

	time_t Application::initialize(std::shared_ptr<Abstract::Agent> root, const char *pathname, bool startup) {

		if(startup && !Module::preload()) {
			throw runtime_error("Module preload has failed");
		}

		Updater updater{pathname,startup};

		if(updater.refresh()) {
			if(!updater.load(root)) {
				root->error() << "Update failed, agent " << hex << root.get() << dec << " will not be promoted to root" << endl;
				auto old = Abstract::Agent::root();
				if(old) {
					old->warning() << "Keeping agent " << hex << old.get() << dec << " as root" << endl;
				}
			}
		}

		return updater.wait();
	}

	void Application::setup(const char *pathname, bool startup) {

		auto root = RootFactory();
		time_t timer = initialize(root,pathname,startup);

		if(Abstract::Agent::root().get() == root.get()) {
			Logger::String{"Root agent has changed"}.trace(name());
			this->root(root);
		}

		if(!timer) {
			Logger::String{"Auto update is disabled"}.trace(name());
			return;
		}

		Logger::String{"Auto update set to ",TimeStamp{time(0)+timer}.to_string()}.info(name());

		this->timer = Timer::Factory(timer*1000,[this,pathname](){
			this->timer = nullptr;
			Logger::String{"Requesting auto update"}.info(name());
			ThreadPool::getInstance().push([this,pathname](){
				setup(pathname,false);
			});
			return false;
		});

	}

 }
