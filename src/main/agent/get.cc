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
	Udjat::Value & Abstract::Agent::get(Udjat::Value &value) {
		return value;
	}
	#pragma GCC diagnostic pop

	std::string Abstract::Agent::to_string() const {
		return "";
	}

	void Abstract::Agent::get(const Request &request, Report &report) {
		throw system_error(ENOENT,system_category(),"No available reports on this path");
	}

	void Abstract::Agent::get(const Request &request, Response &response) {
		get(getDetails(response)["value"]);
	}

	Value & Abstract::Agent::getDetails(Value &value) const {

		value["name"] = this->getName();
		value["summary"] = this->summary;
		value["label"] = this->label;
		value["uri"] = this->uri;
		value["icon"] = this->icon;

		// Get agent state
		auto &state = value["state"];
		this->state.active->get(state);

		if(this->state.activation) {
			state["activation"] = TimeStamp(this->state.activation);
		} else {
			state["activation"] = false;
		}

		return value;

	}

 }
