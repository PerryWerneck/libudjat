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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/http/exception.h>
 #include <vector>
 #include <cstdarg>

 using namespace std;

 namespace Udjat {

	Activatable::Activatable(const XML::Node &node) : object_name{String{node,"name"}.as_quark()} {
	}

	Activatable::~Activatable() {
	}

	const char * Activatable::name() const noexcept {
		return object_name;
	}

	bool Activatable::available() const noexcept {
		return true;
	}

	bool Activatable::active(bool value) noexcept {
		if(value) {
			return activate();
		} else {
			return deactivate();
		}
	}

	bool Activatable::activate(const Udjat::Abstract::Object &) noexcept {
		return activate();
	}

	bool Activatable::deactivate() noexcept {
		return false;	// Allways return false if the object cant be deactivated.
	}

	const char * Activatable::payload(const XML::Node &node) {
		String child(node.child_value());
		if(child.empty()) {
			child = node.attribute("payload").as_string();
		}
		child.expand(node);
		if(node.attribute("strip-payload").as_bool(true)) {
			child.strip();
		}
		return child.as_quark();
	}

	int Activatable::exec(Udjat::Value &value, bool except, const std::function<int()> &func) {

		try {

			return func();

		} catch(const HTTP::Exception &e) {

			if(except) {
				throw;
			}
			Logger::String{e.what()}.error(name());
			return e.code();

		} catch(const std::system_error &e) {

			if(except) {
				throw;
			}
			Logger::String{e.what()}.error(name());
			return e.code().value();

		} catch(const std::exception &e) {

			if(except) {
				throw;
			}
			Logger::String{e.what()}.error(name());
			return -1;

		} catch(...) {

			if(except) {
				throw;
			}
			Logger::String{"Unexpected error"}.error(name());
			return -1;

		}

	}

 }
