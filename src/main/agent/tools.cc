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

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	void Abstract::Agent::expand(std::string &text) const {

		Udjat::expand(text,[this](const char *key) {

			// Agent value
			if( !(strcasecmp(key,"value") && strcasecmp(key,"agent.value")) ) {
				return to_string();
			}

			// Agent properties.
			{
				struct {
					const char *key;
					const char *value;
				} values[] = {
					{ "agent.name",		this->getName()	},
					{ "agent.label",	this->label		},
					{ "agent.summary",	this->summary	},
					{ "agent.uri", 		this->uri		},
					{ "agent.icon", 	this->icon		}
				};

				for(size_t ix = 0; ix < (sizeof(values)/sizeof(values[0]));ix++) {

					if(!strcasecmp(values[ix].key,key)) {
						return string(values[ix].value);
					}

				}
			}

			if(this->state) {

				struct {
					const char *key;
					const Quark &agent;
					const Quark &state;
				} values[] = {
					{
						"summary",
						this->summary,
						this->state->getSummary()
					},
					{
						"uri",
						this->uri,
						this->state->getUri()
					}
				};

				for(size_t ix = 0; ix < (sizeof(values)/sizeof(values[0]));ix++) {

					if(!strcasecmp(values[ix].key,key)) {

						if(values[ix].state) {
							return string(values[ix].state.c_str());
						}

						return string(values[ix].agent.c_str());

					}

				}

			}

#ifdef DEBUG
			cout << "Can't find key '" << key << "'" << endl;
#endif // DEBUG
			return string{"${}"};

		});

	}

 }
