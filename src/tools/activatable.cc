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
 #include <udjat/tools/activatable.h>
 #include <udjat/factory.h>
 #include <udjat/alert/abstract.h>
 #include <udjat/alert.h>

 namespace Udjat {

	std::shared_ptr<Activatable> Activatable::Factory(const Abstract::Object &parent, const pugi::xml_node &node, const char *type) {

		std::shared_ptr<Activatable> activatable;

		if(!(type && *type)) {
			type = "default";
		}

		if(Udjat::Factory::search(node,[&parent,&activatable](const Udjat::Factory &factory, const pugi::xml_node &node){

			activatable = factory.ActivatableFactory(parent,node);
			if(activatable) {
				return true;
			}
			return false;


		},type)) {
			return activatable;
		}

		return AlertFactory(parent,node,type);

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

