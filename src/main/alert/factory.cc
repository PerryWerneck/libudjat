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
 #include <udjat/factory.h>
 #include <udjat/alert.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>
 #include <iostream>
 #include <udjat/moduleinfo.h>

 using namespace std;

 namespace Udjat {

	std::shared_ptr<Abstract::Alert> AlertFactory(const pugi::xml_node &node, const char *name) {

		std::shared_ptr<Abstract::Alert> alert;

		if(Factory::search(node,[&alert](const Factory &factory, const pugi::xml_node &node){
			alert = factory.AlertFactory(node);
			if(alert) {
				alert->info() << "Using alert engine from '" << factory.name() << "'" << endl;
				return true;
			}
			return false;
		},name)) {
			return alert;
		}

		alert = make_shared<Udjat::Alert>(node);
		if(alert->verbose()) {
			alert->info() << "Using the default alert engine" << endl;
		}

		return alert;

	}

 }
