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
#include <private/module.h>
#include <iostream>

#define LOG_DOMAIN "module"
#include <udjat/tools/logger.h>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Controller & Module::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	Module::Controller::Controller() : XML::Parser{"module"} {
		Logger::String{"Starting controller"}.trace();
	}

	Module::Controller::~Controller() {
		clear();
	}

	void Module::Controller::push_back(Module *module) {
		lock_guard<mutex> lock(guard);
		handlers.emplace_back(module);
	}

	void Module::Controller::remove(Module *module) {
		lock_guard<mutex> lock(guard);
		for(auto handler : handlers) {
			if(handler.module == module) {
				handlers.remove(handler);
				return;
			}
		}
	}

	Module::Controller::Handler & Module::Controller::handler(Module *module) {
		lock_guard<mutex> lock(guard);
		for(auto &handler : handlers) {
			if(handler.module == module) {
				return handler;
			}
		}
		throw logic_error("Unexpected error: The requested module was not found");
	}

}

