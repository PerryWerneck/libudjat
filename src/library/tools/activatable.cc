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
 #include <udjat/tools/activatable.h>
 #include <udjat/tools/string.h>

 namespace Udjat {

	Activatable::Activatable(const XML::Node &node) : object_name{String{node,"name"}.as_quark()} {
	}

	Activatable::~Activatable() {
	}

	bool Activatable::activate(const Udjat::Abstract::Object &) noexcept {
		return activate();
	}

	bool Activatable::deactivate() noexcept {
		return false;	// Allways return false if the object cant be deactivated.
	}

 }


/*
 #include <udjat/tools/factory.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/alert.h>

 namespace Udjat {

	std::shared_ptr<Activatable> Activatable::Factory(const Abstract::Object &parent, const XML::Node &node) {

		std::shared_ptr<Activatable> activatable;

		if(Udjat::Factory::for_each([&parent,&activatable,&node](Udjat::Factory &factory){

			if(factory == node.attribute("type").as_string("default")) {
				activatable = factory.ActivatableFactory(parent,node);
				return (bool) activatable;
			}

			return false;

		})) {

			return activatable;

		}

		return Abstract::Alert::Factory(parent,node);

	}

	void Activatable::activate(const Abstract::Object UDJAT_UNUSED(&object)) {
	}

	void Activatable::deactivate() {
	}

	void Activatable::trigger(const Abstract::Object &object) {

		if(activated()) {
			deactivate();
		}

		activate(object);

	}

 }

*/
