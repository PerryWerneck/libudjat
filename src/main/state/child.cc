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
 #include <udjat/tools/factory.h>
 #include <udjat/agent/state.h>
 #include <iostream>
 #include <udjat/alert/abstract.h>

//---[ Implement ]------------------------------------------------------------------------------------------

 using namespace std;

 namespace Udjat {

	bool Abstract::State::push_back(const XML::Node &node) {
		return push_back(node.name(),node);
	}

	bool Abstract::State::push_back(const char *type, const XML::Node &node) {

		if(!strcasecmp(type,"alert")) {
			auto alert = Udjat::Abstract::Alert::Factory(*this, node);
			if(alert) {
				listeners.push_back(alert);
				return true;
			}
		}

		return Udjat::Factory::for_each(type,[this,&node](const Udjat::Factory &factory) {

			try {

				auto alert = factory.AlertFactory(*this,node);
				if(alert) {
					if(alert->verbose()) {
						alert->info() << "Using alert engine from '" << factory.name() << "'" << endl;
					}
					listeners.push_back(alert);
					return true;
				}

				return false;

			} catch(const std::exception &e) {

				factory.error() << "Error '" << e.what() << "' parsing node <" << node.name() << ">" << endl;

			} catch(...) {

				factory.error() << "Unexpected error parsing node <" << node.name() << ">" << endl;

			}

			return false;

		});

	}

 }
