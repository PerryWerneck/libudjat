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
 #include <udjat/tools/intl.h>
 #include <udjat/tools/factory.h>
 #include <udjat/alert/url.h>
 #include <udjat/alert/script.h>
 #include <udjat/alert/file.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	static const char * get_alert_type(const XML::Node &node) {

		const char *type;
		const char *attrname = (strcasecmp(node.name(),"alert") == 0 ? "type" : "alert-type");

		type = node.attribute(attrname).as_string("");
		if(type[0]) {
			return type;
		}

		if(!strcasecmp(node.parent().name(),"agent")) {
			// The parent node is an agent, use their type.
			type = node.parent().attribute("type").as_string();
			if(type[0]) {
				return type;
			}
		}

		for(auto parent = node.parent();parent;parent = node.parent()) {

			type = parent.attribute("alert-type").as_string();
			if(type[0]) {
				return type;
			}

		}

		throw runtime_error(Logger::Message{_("Unable to determine alert type for node <{}>"),node.name()});

	}

	std::shared_ptr<Abstract::Alert> Abstract::Alert::Factory(const Abstract::Object &parent, const XML::Node &node) {

		static const struct {
			const char *attrname;
			const char *type;
			const std::function<std::shared_ptr<Abstract::Alert>(const XML::Node &node)> factory;
		} internal_types[] = {
			{
				"cmdline", "script",
				[](const XML::Node &node){
					return make_shared<Udjat::Alert::Script>(node);
				}
			},
			{
				"url", "url",
				[](const XML::Node &node){
					return make_shared<Udjat::Alert::URL>(node);
				}
			},
			{
				"filename", "file",
				[](const XML::Node &node){
					return make_shared<Udjat::Alert::File>(node);
				}
			},
		};

		const char *type = get_alert_type(node);

		if(!strcasecmp(type,"internal")) {

			// Try to identify internal alert using attributes.
			for(auto internal_type : internal_types) {
				if(node.attribute(internal_type.attrname)) {
					Logger::String{"Building internal '",internal_type.type,"' alert."}.trace("alert");
					return internal_type.factory(node);
				}
			}

			throw runtime_error(Logger::Message{_("Unable to determine internal alert type for node <{}>"),node.name()});

		}

		// Check factories.
		{
			std::shared_ptr<Abstract::Alert> alert;
			if(Udjat::Factory::for_each([&parent,&node,&alert,type](Udjat::Factory &factory) {

				if(factory == type) {
					alert = factory.AlertFactory(parent,node);
					if(alert) {
						if(Logger::enabled(Logger::Trace)) {
							alert->trace() << "Using '" << factory.name() << "' alert engine" << endl;
						}
						return true;
					}
				}
				return false;

			})) {
				return alert;
			};
		}

		// Try internal types.
		for(auto internal_type : internal_types) {
			if(strcasecmp(internal_type.type,type) == 0) {
				Logger::String{"Building internal '",internal_type.type,"' alert."}.trace("alert");
				return internal_type.factory(node);
			}
		}

		// Last chance, try to identify internal alert using attributes.
		for(auto internal_type : internal_types) {
			if(node.attribute(internal_type.attrname)) {
				Logger::String{"Building internal '",internal_type.type,"' alert."}.trace("alert");
				return internal_type.factory(node);
			}
		}

		throw runtime_error(Logger::Message{_("Unable to create alert for node <{}>"),node.name()});

	}

 }
