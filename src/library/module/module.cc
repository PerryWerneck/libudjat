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
#include <udjat/module/abstract.h>
#include <udjat/tools/string.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/intl.h>
#include <iostream>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const char *n, const ModuleInfo &i) : module_name(n),handle(nullptr),_info(i) {

		if(!(module_name && *module_name)) {
			throw system_error(EINVAL,system_category(),"Cant create unnamed module");
		}

		if(i.build < MINIMAL_MODULE_BUILD) {
			error()<< "The module build date " << i.build << " is lower than the expected " << MINIMAL_MODULE_BUILD << endl;
			throw system_error(EINVAL,system_category(),"Invalid module build date");
		}

		Controller::getInstance().push_back(this);
	}

	Module::~Module() {
		Controller::getInstance().remove(this);
	}

	void Module::finalize() {
	}

	Value & Module::getProperties(Value &properties) const {
		properties["name"] = module_name;
		properties["filename"] = filename();
		return _info.getProperties(properties);
	}

	const Module * Module::find(const char *name) noexcept {
		return Controller::getInstance().find(name);
	}

	void * Module::dlsym(const char *symbol, bool required) const {
		return Controller::getSymbol(handle, symbol, required);
	}

	bool Module::for_each(const std::function<bool(Module &module)> &method) {
		for(auto &module : Controller::getInstance()) {
			if(method(*module)) {
				return true;
			}
		}
		return false;
	}

	void Module::set(std::shared_ptr<Abstract::Agent>) {
	}

	bool Module::getProperty(const char *key, std::string &value) const {

		if(!strcasecmp(key,"filename")) {
			value = filename();
			return true;
		}

		return _info.getProperty(key,value);
	}

	std::string Module::operator[](const char *property_name) const noexcept {
		std::string value;
		getProperty(property_name,value);
		return value;
	}

	void Module::trace_paths(const char *) const noexcept {
	}

	void Module::exec(Udjat::Value &response, const char *name,...) const {
		va_list args;
		va_start(args, name);
		try {
			exec(response,name,args);
		} catch(...) {
			va_end(args);
			throw;
		}
		va_end(args);
	}

	void Module::exec(const char *module_name, Udjat::Value &response, const char *name, ...) {
		const Module *module = find(module_name);
		if(!module) {
			throw system_error(EINVAL,system_category(),Logger::Message(_("Module '{}' is not loaded"),module_name));
		}

		va_list args;
		va_start(args, name);
		try {
			module->exec(response,name,args);
		} catch(...) {
			va_end(args);
			throw;
		}
		va_end(args);

	}

	void Module::exec(Udjat::Value UDJAT_UNUSED(&response), const char *name, va_list UDJAT_UNUSED(args)) const {
		throw system_error(ENOTSUP,system_category(),Logger::Message(_("I dont know how to execute '{}'"),name));
	}

	void Module::options(const XML::Node &node, std::function<void(const char *name, const char *value)> call) {
		XML::options(node,call);
	}

	std::ostream & Module::info() const {
		return cout << module_name << "\t";
	}

	std::ostream & Module::warning() const {
		return clog << module_name << "\t";
	}

	std::ostream & Module::error() const {
		return cerr << module_name << "\t";
	}

}

