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

/**
 *
 * @brief Implements the state alert factory.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	std::shared_ptr<Abstract::Alert> Abstract::State::AlertFactory(const pugi::xml_node &node, const char *name) {

		std::shared_ptr<Abstract::Alert> alert;

		if(!name) {
			name = getAttribute(node,"alert-defaults","type","default");
		}

#ifdef DEBUG
		info() << "Creating alert '" << name << "'" << endl;
#endif // DEBUG

		if(strcasecmp(name,"default")) {

			// It's not the default alert, search for factory.
			if(!Factory::for_each(name,[this,&alert,node](const Factory &factory){

				try {

					alert = factory.AlertFactory(node);
					if(alert) {
						alerts.push_back(alert);
						return true;
					}

				} catch(const std::exception &e) {

					factory.error() << "Error '" << e.what() << "' creating alert" << endl;

				} catch(...) {

					factory.error() << "Unexpected error creating alert" << endl;

				}

				return false;

			})) {

				error() << "Unable to create the required alert" << endl;

			}

		} else {

			// First, try the node and state parent names.
			const char *names[] = { node.name(), node.parent().name(), node.parent().parent().name() };

			for(size_t ix = 0; !alert && ix < (sizeof(names)/sizeof(names[0])); ix++) {

				const Factory * factory = Factory::find(names[ix]);
				if(factory) {
					alert = factory->AlertFactory(node);
					if(alert) {
						info() << "Using alert engine from '" << factory->name() << "'" << endl;
						break;
					}
				}
			}

			if(!alert) {
				info() << "Using the default alert engine" << endl;
				alert = make_shared<Udjat::Alert>(node);
			}

			alerts.push_back(alert);

		}

		return alert;

	}

}
