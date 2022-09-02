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
 #include <udjat/alerts/script.h>
 #include <udjat/tools/subprocess.h>

 using namespace std;

 namespace Udjat {

	Alert::Script::Script(const pugi::xml_node &node, const char *defaults) : Abstract::Alert(node) {

		const char *section = node.attribute("settings-from").as_string(defaults);

		cmdline = getAttribute(node,section,"cmdline","");

		if(!(cmdline && *cmdline)) {
			throw runtime_error(string{"Required attribute 'cmdline' is missing on alert '"} + name() + "'");
		}

	}

	std::shared_ptr<Abstract::Alert::Activation> Alert::Script::ActivationFactory() const {
		return make_shared<Activation>(this);
	}

	Value & Alert::Script::getProperties(Value &value) const noexcept {
		Abstract::Alert::getProperties(value);
		value["cmdline"] = cmdline;
		return value;
	}

	Value & Alert::Script::Activation::getProperties(Value &value) const noexcept {
		Abstract::Alert::Activation::getProperties(value);
		value["cmdline"] = cmdline.c_str();
		return value;
	}

	Alert::Script::Activation::Activation(const Udjat::Alert::Script *alert) : Abstract::Alert::Activation(alert), cmdline(alert->cmdline) {
		cmdline.expand(*alert,true,false);
	}

	void Alert::Script::Activation::emit() {

		cmdline.expand();

		if(verbose()) {
			if(description.empty()) {
				info() << "Emitting " << cmdline << endl;
			} else {
				info() << description << ": " << cmdline << endl;
			}
		}

		SubProcess::run(cmdline.c_str());

	}

	void Alert::Script::Activation::set(const Abstract::Object &object) {
		cmdline.expand(object);
	}

 }


