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
 #include <private/agent.h>
 #include <list>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	Udjat::Value & Abstract::Agent::get(Udjat::Value &value) const {
		value.set(to_string());
		return value;
	}

	std::string Abstract::Agent::to_string() const {
		return name();
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::to_shared_ptr() {

		if(!parent) {
			throw system_error(EINVAL,system_category(),"Cant get pointer on orphaned agent");
		}

		for(auto ptr : *parent) {
			if(ptr.get() == this) {
				return ptr;
			}
		}

		throw system_error(EINVAL,system_category(),"Cant get pointer to an invalid agent");
	}

	void Abstract::Agent::get(const Request UDJAT_UNUSED(&request), Report UDJAT_UNUSED(&report)) {
		error() << "Rejecting 'report' request - Not available in this agent" << endl;
		throw system_error(ENOENT,system_category(),"No available reports on this path");
	}

	void Abstract::Agent::get(Response &response) {
		getProperties(response);
	}

	void Abstract::Agent::get(const Request UDJAT_UNUSED(&request), Response UDJAT_UNUSED(&response)) {
		getProperties(response);
	}

	Value & Abstract::Agent::getProperties(Value &value) const noexcept {

		Object::getProperties(value);

		try {

			// Get agent value.
			get(value["value"]);

			// Get agent state.
			auto &state = value["state"];
			this->current_state.active->getProperties(state);
			state["activation"] = TimeStamp(this->current_state.activation);

		} catch(const std::exception &e) {

			error() << "Error '" << e.what() << "' getting agent properties" << endl;

		} catch(...) {

			error() << "Unexpected error getting agent properties" << endl;

		}

		return value;

	}

	std::string Abstract::Agent::path() const {

		std::list<std::string> names;

		const Abstract::Agent *agent = this;
		while(agent->parent) {
			names.push_front(agent->name());
			agent = agent->parent;
		}

		string rc;

		for(auto name : names) {
			rc += "/";
			rc += name;
		}

		return rc;

	}

 }
