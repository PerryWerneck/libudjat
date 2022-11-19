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
 #include <udjat/tools/logger.h>
 #include <udjat/factory.h>
 #include <udjat/alert/url.h>
 #include <udjat/alert/script.h>
 #include <udjat/alert/file.h>
 #include <udjat/tools/object.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::Alert> AlertFactory(const Abstract::Object &parent, const pugi::xml_node &node, const char *type) {

		std::shared_ptr<Abstract::Alert> alert;

		if(!(type && *type)) {
			// TODO: Scan for global alert definition.
			type = "default";
		}

		// debug("Creating alert '",type,"'");

		//
		// First, try using the type name (even for 'default').
		//
		if(Factory::search(node,[&parent,&alert](const Factory &factory, const pugi::xml_node &node){
			alert = factory.AlertFactory(parent,node);
			if(alert) {
				if(alert->verbose()) {
					alert->info() << "Using alert engine from '" << factory.name() << "'" << endl;
				}
				return true;
			}
			return false;
		},type)) {
			return alert;
		}

		//
		// Try defined types
		//
		if(!strcasecmp(type,"url")) {
			return make_shared<Udjat::Alert::URL>(node);
		}

		if(!strcasecmp(type,"script")) {
			return make_shared<Udjat::Alert::Script>(node);
		}

		if(!strcasecmp(type,"file")) {
			return make_shared<Udjat::Alert::File>(node);
		}

		if(!strcasecmp(type,"default")) {

			//
			// Check node attributes.
			//
			if(node.attribute("url")) {
				return make_shared<Udjat::Alert::URL>(node);
			}

			if(node.attribute("script")) {
				return make_shared<Udjat::Alert::Script>(node);
			}

			if(node.attribute("filename")) {
				return make_shared<Udjat::Alert::Script>(node);
			}

			//
			// Do an upsearch.
			//
			if(Object::getAttribute(node,"url")) {
				return make_shared<Udjat::Alert::URL>(node);
			}

			if(Object::getAttribute(node,"script")) {
				return make_shared<Udjat::Alert::Script>(node);
			}

			if(Object::getAttribute(node,"filename")) {
				return make_shared<Udjat::Alert::File>(node);
			}

		}

		throw runtime_error(Logger::Message("Unable to create an alert type '{}'",type));

	}

 }
