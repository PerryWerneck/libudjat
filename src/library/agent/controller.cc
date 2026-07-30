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
 * @file src/main/agent/controller.cc
 *
 * @brief Implements the agent controller.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #define LOG_DOMAIN "agent"

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/file.h>
 #include <udjat/agent.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/agent/state.h>
 #include <udjat/tools/http/exception.h>
 #include <unistd.h>

 #include <udjat/tools/logger.h>

 #include <udjat/tools/intl.h>

 using namespace std;

 namespace Udjat {

	Abstract::Agent::Controller::Controller() 
		: Service{"agents"}, Abstract::Object::Factory{"agent"}, Interface{"agent",Authentication::None} {
		Logger::String{"Initializing controller"}.trace();
	}

	Abstract::Agent::Controller::~Controller() {
		Logger::String{"Deinitializing controller"}.trace();
	}

	bool Abstract::Agent::Controller::schema(const HTTP::Method method, const char *path, OutputSchema &schema) const noexcept {

		debug("Getting output schema for '",Interface::name(),"' at '",path,"'");

		if(!(root && (path && *path))) {
			// Return default output schema.
			debug("Returning default output schema for '",Interface::name(),"'");
			return Abstract::Agent{}.schema(method,"",schema);
		}

		auto agent = root;
		if(path && *path) {
			agent = root->find(path,false,false);
			if(!agent) {
				debug("Cant find agent '",path,"' searching for output schema for '",Interface::name(),"'");
				return false;
			}
		}

		debug("Returning agent output schema for '",Interface::name(),"'");
		return agent->schema(method,"",schema);
	}

	bool Abstract::Agent::Controller::schema(const HTTP::Method method, const char *path, InputSchema &schema) const noexcept {

		if(!(root && (path && *path))) {
			// No root or no path, return the default 'No-schema'.
			return false;
		}

		auto agent = root;
		if(path && *path) {
			agent = root->find(path,false,false);
			if(!agent) {
				return false;
			}
		}

		return agent->schema(method,"",schema);

	}

	void Abstract::Agent::Controller::set(std::shared_ptr<Abstract::Agent> root) {

		if(root && root->parent) {
			throw logic_error("Child agent cant be promoted to root");
		}

		if(this->root) {
			Logger::String{
				"Root agent ",
				this->root->name(),
				" (",to_hex_string(this->root.get()).c_str(),
				") was demoted"
			}.trace("agents");
		}

		if(!root) {

			this->root.reset();

		} else {

			this->root = root;

			Logger::String{
				"Agent ",
				this->root->name(),
				" (",to_hex_string(this->root.get()).c_str(),
				") was promoted to root"
			}.trace("agents");

		}

	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::get() const {
		if(this->root)
			return this->root;
		throw logic_error(_("Root agent is not available"));
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::Controller::find(const char *path, bool required) const {

		auto root = get();

		if(path && *path)
			return root->find(path,required);

		return root;

	}

	void Abstract::Agent::root(std::shared_ptr<Abstract::Agent> agent) {
		Abstract::Agent::Controller::getInstance().set(agent);
	}

	void Abstract::Agent::deinit() {
		Abstract::Agent::Controller::getInstance().set(std::shared_ptr<Abstract::Agent>());
	}

	std::shared_ptr<Abstract::Agent> Abstract::Agent::root() {
		return Abstract::Agent::Controller::getInstance().get();
	}

	void Abstract::Agent::Controller::start() noexcept {

		if(root) {

			try {

				root->start();

				// Setup next update on all children.
				root->for_each([](std::shared_ptr<Agent> agent) {
					if(agent->update.timer && !agent->update.next) {
						agent->update.next = time(0) + agent->update.timer;
					}
				});

			} catch(const std::exception &e) {
				Logger::String{"Error '",e.what(),"' starting agents"}.error(root->name());
				return;
			}

		} else {

			Logger::String{"Starting controller without root agent"}.warning("agent");

		}

		Logger::String{"Starting controller"}.trace("agent");

		MainLoop::Timer::reset(1000);
		MainLoop::Timer::enable();

	}

	void Abstract::Agent::Controller::stop() noexcept {

		debug("---- Stopping agent controller ----");

		MainLoop::Timer::disable();

		if(root) {

			try {
				debug("---- Stopping children ----");
				root->stop();
				debug("---- Cleaning children ----");
				root->clear();
				debug("---- Root agent cleanup is complete ----")
			} catch(const std::exception &e) {
				Logger::String{"Error '",e.what(),"' stopping root agent"}.error(root->name());
			} catch(...) {
				Logger::String{"Unexpected error stopping root agent"}.error(root->name());
			}

			root.reset();

			debug("Waiting for tasks (agent)");
			ThreadPool::getInstance().wait();
			debug("Wait for tasks complete");

		} else {

			Logger::String{"Stopping empty controller"}.trace();

		}

		debug("---- Agent controller stopped ----");
	}

	void Abstract::Agent::Controller::update_agents() {

		time_t now{time(0)};
		time_t next{now+Config::Value<time_t>("agent","min-update-time",600)};

		std::vector<std::shared_ptr<Agent>> updatelist;

		root->for_each([now,this,&next,&updatelist](std::shared_ptr<Agent> agent) {

			// Ignore agents with on_demand flag active, without 'next' or with forwarded state.
			if(agent->update.on_demand || !agent->update.next || agent->current_state.forwarded()) {
				return;
			}

			// Do the agent requires an update?
			if(agent->update.next <= now) {

				lock_guard<std::recursive_mutex> lock(agent->guard);
				if(agent->update.running) {

					//
					// Agent still updating.
					//
					Logger::String{
						"Update is active since ",TimeStamp(agent->update.running).to_string().c_str()
					 }.warning(agent->name());
					agent->update.next = now + 60;
					next = std::min(next,agent->update.next);

				} else {

					//
					// Queue agent update.
					//
					debug("Agent='",agent->name(),"' is expired by ", (now - agent->update.next)," seconds, adding to the update list");
					agent->update.running = time(0);
					updatelist.push_back(agent);

					if(agent->update.timer) {
						agent->update.next = now + agent->update.timer;
						next = std::min(next,agent->update.next);
					} else {
						agent->update.next = 0;
					}

				}

			} else {
				next = std::min(next,agent->update.next);
				// debug(
				// 	"Agent='",agent->name(),
				// 	"' update set to '",TimeStamp(agent->update.next),
				// 	", global update set to ",TimeStamp(next)
				// );
			}
		});

		//
		// Enqueue agent updates
		//
		// debug(updatelist.size()," agent(s) to update, next update will be ",TimeStamp(next));

		if(now < next) {
			MainLoop::Timer::reset((next-now) * 1000);
		} else {
			MainLoop::Timer::reset(1000);
		}

		for(auto agent : updatelist) {

			agent->push([](std::shared_ptr<Agent> agent){

				try {

					debug("Scheduled update of '",agent->name(),"' begin");

					agent->notify(UPDATE_TIMER);

					if(agent->refresh(false)) {
						debug("Agent was changed");
						agent->updated(true);
					} else {
						agent->updated(false);
					}

					debug("Scheduled update of '",agent->name(),"' complete");

				} catch(const exception &e) {

					agent->failed("Agent update failed",e);

				} catch(...) {

					agent->failed("Unexpected error when updating");

				}

				{
					lock_guard<std::recursive_mutex> lock(agent->guard);
					agent->update.running = 0;
				}

			});

		}
	}

	void Abstract::Agent::Controller::on_timer() {

		if(!root) {
			debug("No root agent!!");
			return;
		}

		ThreadPool::getInstance().push("agent-updates",[this]() {

			{
				time_t now = time(0);

				lock_guard<std::recursive_mutex> lock(Abstract::Agent::guard);
				if(updating) {
					if(updating < now) {
						Logger::String{"Updating since ",TimeStamp(updating).to_string().c_str()}.error("agent");
					}
					reset(500);
					return;
				}
				updating = now;
			}

			try {

				update_agents();

			} catch(const std::exception &e) {

				Logger::String{"Error '",e.what(),"' updating agents"}.error("agent");

			} catch(...) {

				Logger::String{"Unexpected error updating agents"}.error("agent");

			}

			{
				lock_guard<std::recursive_mutex> lock(Abstract::Agent::guard);
				updating = 0;
			}

		});


	}

	std::shared_ptr<Abstract::Object> Abstract::Agent::Controller::ObjectFactory(const Properties &props) const {

		auto child = Abstract::Agent::Factory::build(props);
		return child;
	}

	bool Abstract::Agent::Controller::process(Request &request, Response &response) const noexcept {

		debug("Response type is ",std::to_string(response.mimetype()));
		
		if(!this->root) {
			request.error(Interface::name(),"Root agent is not available");
			response = HTTP::Unavailable;
			return true;
		}

		debug("Searching for agent '",request.path(),"'");
		auto agent = Abstract::Agent::Controller::getInstance().find(request.path(),false);
		if(!agent) {
			response = HTTP::NotFound;
			return true;
		}

		debug("Found agent '",agent->name(),"'");

		time_t timestamp = agent->last_modified();
		if(timestamp) {
			debug("last-modified: ",TimeStamp{timestamp}.to_string().c_str());
			response.last_modified(timestamp);
			if(request.cached(timestamp)) {
				response = HTTP::NotModified;
				return true;
			}
		}

		if(agent->update.next) {
			response.expires(agent->update.next);
		}

		// Set state info on header X-agent-state: 
		{
			auto state = agent->state();
			if(state) {
				response.state(
					"agent",
					std::to_string(state->level()),
					state->summary()
				);
			}
		}

		auto method = request.method();
		if(method == HTTP::Head) {
			// Header was already set, just return.
			response = HTTP::NoContent;
			return true;
		}

		if(method != HTTP::Get) {
			response = HTTP::MethodNotAllowed;
			return true;
		}

		OutputSchema out;
		if(schema(request.method(),request.path(),out)) {
			for(const auto &item : out) {
				debug("Getting value for '",agent->name(),".",item.name(),"'");
				if(!agent->get_property(item.name(),response[item.name()])) {
					response = HTTP::SystemError;
					response.failed(
						String{"Unable to get value for '",item.name(),"'"}.c_str()
					);
					return true;
				}
			}

			debug("Got agent '",agent->name(),"' properties using outputschema");
			return true;
		}

		// No schema, get all properties.
		agent->get_properties(response);

		return true;
	}

	// std::shared_ptr<Action> Abstract::Agent::Controller::ActionFactory(const Properties &) const {

	// 	debug("Build agent action");

	// 	/// @brief Action to get agent properties.
	// 	class AgentProperties : public Udjat::Action {
	// 	public:
	// 		AgentProperties() : Udjat::Action{"agent",_("Get agent properties")} {
	// 		} 

	// 		bool schema(OutputSchema &schema) const noexcept override {

	// 			schema.append(
	// 				Schema::Item{ "icon",		Schema::Icon	},
	// 				Schema::Item{ "label",		Schema::String	},
	// 				Schema::Item{ "name",		Schema::String	},
	// 				Schema::Item{ "state",		Schema::String	},
	// 				Schema::Item{ "summary",	Schema::String	},
	// 				Schema::Item{ "system", 	Schema::String	},
	// 				Schema::Item{ "url", 		Schema::Url		},
	// 				Schema::Item{ "value",		Schema::String	}
	// 			);

	// 			return true;
	// 		}

	// 		int call(Udjat::Request &request, Udjat::Response &response, bool except) override {

	// 			return exec(response, except, [&]() {

	// 				auto agent = Abstract::Agent::Controller::getInstance().find(request.path(),true);

	// 				time_t timestamp = agent->last_modified();
	// 				if(timestamp) {
	// 					debug("last-modified: ",TimeStamp{timestamp}.to_string().c_str());
	// 					response.last_modified(timestamp);
	// 					if(request.cached(timestamp)) {
	// 						response.failed(HTTP::NotModified);
	// 						return 0;
	// 					}
	// 				}

	// 				agent->get_properties(response);

	// 				if(agent->update.next) {
	// 					response.expires(agent->update.next);
	// 				}

	// 				response.message(agent->state()->to_string().c_str());

	// 				return 0;
	// 			});


	// 		}

	// 	};

	// 	static std::shared_ptr<Action> instance;
	// 	if(!instance) {
	// 		Logger::String{"Building singleton for agent actions"}.trace();
	// 		instance = make_shared<AgentProperties>();
	// 	}

	// 	return instance;

	// }

}

