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
 #include <udjat/defs.h>
 #include <private/factory.h>
 #include <iostream>
 #include <udjat/moduleinfo.h>

 using namespace std;

 namespace Udjat {

	recursive_mutex Factory::Controller::guard;

	Factory::Controller & Factory::Controller::getInstance() {
		lock_guard<recursive_mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	void Factory::Controller::insert(Factory *factory) {
		lock_guard<recursive_mutex> lock(guard);
		cout << "factories\tRegister '" << factory->name() << "' (" << factory->module.description << ")" << endl;
		factories.push_back(factory);
	}

	Factory * Factory::Controller::find(const char *name) {
		lock_guard<recursive_mutex> lock(guard);

		if(name && *name) {
//#ifdef DEBUG
//			cout << "factories\tSearching for '" << name << "' factory" << endl;
//#endif // DEBUG

			for(auto factory : factories) {
				if(!strcasecmp(factory->name(),name)) {
					return factory;
				}
			}
		}

		return nullptr;

	}

	void Factory::Controller::remove(Factory *factory) {

		cout << "factories\tUnregister '" << factory->name() << "' (" << factory->module.description << ")" << endl;

		lock_guard<recursive_mutex> lock(guard);
		factories.remove_if([factory](const Factory *obj){
			return factory == obj;
		});

	}

	bool Factory::Controller::for_each(const char *name, const std::function<bool(Factory &factory)> &func) {
		lock_guard<recursive_mutex> lock(guard);
		for(auto factory : factories) {
			if(!(name && *name && strcasecmp(factory->name(),name))) {
				if(func(*factory)) {
					return true;
				}
			}
		}
		return false;
	}

 }
