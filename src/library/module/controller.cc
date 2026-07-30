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

#define LOG_DOMAIN "module"
#include <config.h>
#include <udjat/tools/interface.h>
#include <udjat/authentication.h>
#include <private/module.h>
#include <udjat/module.h>
#include <udjat/tools/container.h>
#include <udjat/module.h>
#include <iostream>
#include <udjat/tools/logger.h>
#include <udjat/tools/request.h>
#include <udjat/tools/response.h>
#include <udjat/tools/intl.h>

using namespace std;

#ifdef DEBUG
	#define REQUIRED_ROLE Authentication::None
#else
	#define REQUIRED_ROLE Authentication::Admin
#endif

namespace Udjat {

	void Module::initialize() noexcept {
		Controller::getInstance();
	}

	Module::Controller & Module::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	Module::Controller::Controller() : Properties::ObjectBuilder{"module"}, Interface{"module",REQUIRED_ROLE} {
		Logger::String{"Starting controller"}.trace();
	}

	Module::Controller::~Controller() {
		Logger::String{"Stopping controller"}.trace();
		unload();
	}

	bool Module::Controller::for_each(const std::function<bool(Module &module)> &method) {
		return modules.for_each(method);
	}

	bool Module::Controller::process(Request &request, Response &response) const noexcept {

		if(request.root()) {
			response = HTTP::BadRequest;
			return true;	
		}

		// TODO: Get info about the module on path.

		Logger::String{"Module information is incomplete"}.error();
		response = HTTP::SystemError;

		return true;
	}

	bool Module::Controller::for_each(const std::function<bool(const Udjat::Value &value)> &func) const noexcept {
		for(const auto module : modules) {
			Value value;
			value["name"] = module->module_name;
			value["description"] = module->info.description;
			value["version"] = module->info.version;
			value["filename"] = module->filename();
			if(func(value)) {
				return true;
			}
		}
		return false;
	}
}

