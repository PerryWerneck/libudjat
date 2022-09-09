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
#include <udjat/tools/string.h>
#include <iostream>

using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	Module::Module(const char *n, const ModuleInfo &i) : name(n),handle(nullptr),info(i) {

		if(i.build && i.build < MINIMAL_MODULE_BUILD) {
			cerr << n << "\tThe module build date " << i.build << " is lower than the expected " << MINIMAL_MODULE_BUILD << endl;
			throw system_error(EINVAL,system_category(),"Invalid module build date");
		}

		if(!name) {
			throw system_error(EINVAL,system_category(),"Module name cant be null");
		}

		Controller::getInstance().insert(this);
	}

	Module::~Module() {
#ifdef DEBUG
		cout << name << "\t" <<  __FILE__ << "(" << __LINE__ << ")" << endl;
#endif //
		Controller::getInstance().remove(this);
	}

	void Module::set(const pugi::xml_document UDJAT_UNUSED(&document)) {
	}

	void Module::getInfo(Response &response) {
		Controller::getInstance().getInfo(response);
	}

	const Module * Module::find(const char *name) noexcept {
		return Controller::getInstance().find(name);
	}

	void Module::for_each(std::function<void(Module &module)> method) {
		Controller::getInstance().for_each(method);
	}

	void Module::options(const pugi::xml_node &node, std::function<void(const char *name, const char *value)> call) {

		for(pugi::xml_node child = node.child("option"); child; child = child.next_sibling("option")) {

			const char *name = child.attribute("name").as_string();
			if(!(name && *name)) {
				cerr << "module\tIgnoring unnamed attribute" << endl;
				continue;
			}

			call(
				name,
				String(child.attribute("value").as_string()).expand(child).c_str()
			);

		}

	}

}

