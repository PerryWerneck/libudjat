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

 #include "private.h"
 #include <udjat/tools/configuration.h>
 #include <udjat/factory.h>

 namespace Udjat {

	mutex Alert::Controller::guard;

	Alert::Controller::Controller() : Worker(Quark::getFromStatic("alerts")), Factory(Quark::getFromStatic("alert")) {

		static const Udjat::ModuleInfo info{
			PACKAGE_NAME,								// The module name.
			"Alert Controller",							// The module description.
			PACKAGE_VERSION "." PACKAGE_RELEASE,		// The module version.
#ifdef PACKAGE_URL
			PACKAGE_URL,
#else
			"",
#endif // PACKAGE_URL
#ifdef PACKAGE_BUG_REPORT
			PACKAGE_BUG_REPORT
#else
			""
#endif // PACKAGE_BUG_REPORT
		};

		Worker::info = &info;

	}

	Alert::Controller::~Controller() {
	}

	Alert::Controller & Alert::Controller::getInstance() {
		lock_guard<mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	void Alert::Controller::work(const Request &request, Response &response) const {

		lock_guard<mutex> lock(guard);


		throw runtime_error("Not implemented");
	}

	void Alert::Controller::remove(const Alert *alert) {
		events.remove_if([alert](std::shared_ptr<Alert::Event> event){
			return event->alert == alert;
		});
	}

	string Alert::Controller::getType(const pugi::xml_node &node) {

		string type =
			Attribute(node,"type",false)
				.as_string(
					Config::Value<string>("alert-default","type","default").c_str()
				);

		return type;

	}

	const string Alert::Controller::getFactoryNameByType(const pugi::xml_node &node) {

		return string{"alert-"}
			+ Attribute(node,"type")
				.as_string(
					Config::Value<string>(
						Alert::getConfigSection(node).c_str(),
						"type","url").c_str()
				);

	}

	void Alert::Controller::parse(Abstract::Agent &parent, const pugi::xml_node &node) const {
		Factory::parse(
			getFactoryNameByType(node).c_str(),
			parent,
			node
		);
	}

	void Alert::Controller::parse(Abstract::State &parent, const pugi::xml_node &node) const {
		Factory::parse(
			getFactoryNameByType(node).c_str(),
			parent,
			node
		);
	}


 }
