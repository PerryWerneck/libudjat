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
 #include <udjat/agent/abstract.h>
 #include <udjat/agent.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/response/table.h>
 #include <udjat/tools/response/value.h>
 #include <list>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	time_t Abstract::Agent::last_modified() const noexcept {
		if(update.last) {
			return update.last;
		}
		return current_state.timestamp;
	}

	time_t Abstract::Agent::expires() const noexcept {
		if(update.next > time(0)) {
			return update.next;
		}
		return 0;
	}

	Udjat::Value & Abstract::Agent::get(Udjat::Value &value) const {
		value.set(to_string());
		return value;
	}

	bool Abstract::Agent::get(Udjat::Response::Value &response) const {

		response.last_modified(last_modified());
		response.expires(expires());
		getProperties((Udjat::Value &) response);
		return true;

	}

	bool Abstract::Agent::get(Udjat::Response::Table &response) const {

		response.start(
			"icon",
			"name",
			"label",
			"state",
			"summary",
			"body",

			nullptr
		);

		response.last_modified(last_modified());
		response.expires(expires());

		for(const auto &child : children.agents) {

			auto state = child->state();

			response.last_modified(child->last_modified());

			response.push_back(child->icon(),Value::Icon);
			response.push_back(child->name());
			response.push_back(child->label());
			response.push_back(std::to_string(state->level()));
			response.push_back(child->summary());
			response.push_back(state->body());

		}

		return true;
	}

	void Abstract::Agent::getStates(Udjat::Response::Table &report) const {

		report.last_modified(last_modified());
		report.expires(expires());

		report.start(
			"name",
			"active",
			"value",
			"summary",
			"label",
			"url",
			"icon",
			"body",
			"level",
			NULL
		);

		for_each([&report,this](const Abstract::State &state) {

			report.push_back(state.name());
			report.push_back(&state == this->state().get());
			report.push_back(state.value());
			report.push_back(state.summary());
			report.push_back(state.label());
			report.push_back(state.url());
			report.push_back(state.icon());
			report.push_back(state.body());
			report.push_back(std::to_string(state.level()));

		});

	}

	std::string Abstract::Agent::to_string() const noexcept {
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

	size_t Abstract::Agent::push(const std::function<void(std::shared_ptr<Agent> agent)> &method) {

		if(!parent) {

			auto agent = root();

			if(agent.get() == this) {

				// Root agent, no parent.

				return ThreadPool::getInstance().push(name(),[agent,method]() {

					try {

						method(agent);

					} catch(const std::exception &e) {

						agent->failed("Background task failed",e);

					} catch(...) {

						agent->failed("Unexpected background task fail","A generic background task has failed in this agent");
					}

				});
			}


			throw system_error(EINVAL,system_category(),string{"Unable to push background tasks to orphaned agent '"} + name() + "'.");
		}

		auto agent = to_shared_ptr();
		return ThreadPool::getInstance().push(name(),[agent,method]() {

			try {

				method(agent);

			} catch(const std::exception &e) {

				agent->failed("Background task failed",e);

			} catch(...) {

				agent->failed("Unexpected background task fail","A generic background task has failed in this agent");
			}

		});

	}

	Value & Abstract::Agent::getProperties(Value &value) const {

		Object::getProperties(value);

		try {

			// Get agent value.
			get(value["value"]);

			// Get agent state.
			auto &state = value["state"];
			this->current_state.selected->getProperties(state);
			state["activation"] = TimeStamp(this->current_state.timestamp);

			switch(current_state.activation) {
			case current_state.Activation::StateWasSet:
				state["mode"] = "set";
				break;

			case this->current_state.Activation::StateWasActivated:
				state["mode"] = "activated";
				break;

			case this->current_state.Activation::StateWasForwarded:
				state["mode"] = "forwarded";
				break;

			}

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

	const char * Abstract::Agent::summary() const noexcept {

		const char *str = Udjat::Object::summary();
		if(str && *str) {
			return str;
		}

		return current_state.selected->summary();

	}

	const char * Abstract::Agent::label() const noexcept {

		const char *str = Udjat::Object::label();
		if(str && *str) {
			return str;
		}

		return current_state.selected->label();

	}

	const char * Abstract::Agent::icon() const noexcept {

		if(Object::properties.icon && *Object::properties.icon) {
			return Object::properties.icon;
		}

		return current_state.selected->icon();

	}

 }
