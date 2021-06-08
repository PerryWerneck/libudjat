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

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	void Abstract::Agent::get(const char *name, Json::Value &value) {
		// It's just a placeholder here. Don't set any value.
	}
	#pragma GCC diagnostic pop

	std::string Abstract::Agent::to_string() const {
		throw system_error(ENOTSUP,system_category(),string{"Can't get value for agent'"} + getName() + "'");;
	}

	void Abstract::Agent::get(Json::Value &value, const bool children, const bool state) {

		get("value",value);

		value["name"] = this->getName();
		value["summary"] = this->summary;
		value["label"] = this->label;
		value["uri"] = this->uri;
		value["icon"] = this->icon;

		// Get
		if(state) {
			auto state = Json::Value(Json::objectValue);
			this->state.active->get(state);
			value["state"] = state;

			if(this->state.activation) {
				value["state"]["activation"] = TimeStamp(this->state.activation).to_string(TIMESTAMP_FORMAT_JSON);
			} else {
				value["state"]["activation"] = false;
			}
 		}

		// Get children values
		if(children) {
			auto cvalues = Json::Value(Json::objectValue);
			for(auto child : this->children) {
				auto values = Json::Value(Json::objectValue);
				child->get(values,false,true);
				cvalues[child->getName()] = values;
			}
			value["children"] = cvalues;
		}

	}

	void Abstract::Agent::get(const Request &request, Report &report) {
		throw system_error(ENOENT,system_category(),"No available reports on this path");
	}

	void Abstract::Agent::get(const Request &request, Response &response) {
		get( (Json::Value &) response, false, true);
	}

 }
