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
 * @brief Implements the abstract agent methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/agent.h>
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/intl.h>

namespace Udjat {

	std::recursive_mutex Abstract::Agent::guard;

	Abstract::Agent::Agent(const char *name, const char *label, const char *summary) : Object( (name && *name) ? name : "unnamed") {

		if(label && *label) {
			Object::properties.label = label;
		}

		if(summary && *summary) {
			Object::properties.summary = summary;
		}

		current_state.set(Agent::computeState());

		try {

			update.timer		= Config::get("agent-defaults","update-timer",update.timer);
			update.on_demand	= Config::get("agent-defaults","update-on-demand",update.timer == 0);
			update.next			= time(nullptr) + Config::get("agent-defaults","delay-on-startup",update.timer ? 1 : 0);
			update.failed 		= Config::get("agent-defaults","delay-when-failed",update.failed);

		} catch(const std::exception &e) {

			update.next	= time(nullptr) + update.timer;
			cerr << "Agent\tError '" << e.what() << "' loading defaults" << endl;

		}

	}

	Abstract::Agent::Agent(const pugi::xml_node &node) : Object(node) {
		Controller::setup_properties(*this,node);
	}

	void Abstract::Agent::notify(const Event event) {

		debug("------------------------------->", listeners.size());

		lock_guard<std::recursive_mutex> lock(guard);
		for(Listener &listener : listeners) {

			debug("listener=",listener.activatable->name()," event=",event," listener.event=",listener.event," Activate=",(listener.event & event));

			if((listener.event & event) != 0) {

				auto activatable = listener.activatable;

				push([activatable](std::shared_ptr<Agent> agent){

					try {

						activatable->trigger(*agent);

					} catch(const std::exception &e) {

						activatable->error() << "Error activating on agent '" << agent->name() << "': " << e.what() << endl;

					} catch(...) {

						activatable->error() << "Unexpected error activating on agent '" << agent->name() << "'" << endl;

					}

				});

			}
		}
	}

	Abstract::Agent::~Agent() {

		debug("Cleaning up agent ",name());

		// Remove all associated events.
		Udjat::Event::remove(this);

		// Deleted! My children are now orphans.
		lock_guard<std::recursive_mutex> lock(guard);
		for(auto child : agents()) {
			child->parent = nullptr;
			debug("Releasing agent ",name()," with ",child.use_count()," references");
		}

	}

	void Abstract::Agent::stop() {

		debug("Stopping agent '",name(),"'");

		lock_guard<std::recursive_mutex> lock(guard);

		// Stop children
		for(auto childptr = children.agents.rbegin(); childptr != children.agents.rend(); childptr++) {

			auto agent = *childptr;
			try {

				if(agent->update.running) {

					agent->warning() << "Updating since " << TimeStamp(agent->update.running) << ", waiting" << endl;
					Config::Value<size_t> delay{"agent-controller","delay-wait-on-stop",100};
					Config::Value<size_t> max_wait("agent-controller","max-wait-on-stop",1000);

					ThreadPool::getInstance().wait();

					for(size_t ix = 0; agent->update.running && ix < max_wait; ix++) {
#ifdef _WIN32
						Sleep(delay);
#else
						usleep(delay);
#endif // _WIN32
					}
					if(agent->update.running) {
						agent->error() << "Still updating, giving up" << endl;
					}
				}

				agent->stop();

			} catch(const exception &e) {

				agent->error() << "Error '" << e.what() << "' while stopping" << endl;

			} catch(...) {

				agent->error() << "Unexpected error while stopping" << endl;

			}

		}

		notify(STOPPED);

	}

	void Abstract::Agent::head(ResponseInfo &response) {

		chk4refresh(true);

		time_t now = time(nullptr);

		// Gets the minor time for the next update.
		time_t next{now+Config::Value<time_t>("agent","min-update-time",600)};

		// Gets the major time from the last update.
		time_t updated = 0;

		for_each([&next,&updated](Agent &agent){

			if(agent.update.next) {
				next = std::min(next,agent.update.next);
			}

			if(agent.update.last) {

				if(updated) {
					updated = std::max(updated,agent.update.last);
				} else {
					updated = agent.update.last;
				}
			}

		});

		if(next > now) {
			response.setExpirationTimestamp(next);
		} else {
			response.setExpirationTimestamp(0);
		}

		if(updated >= now) {
			response.setModificationTimestamp(updated);
		}

	}

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	std::shared_ptr<Abstract::State> Abstract::Agent::StateFactory(const pugi::xml_node &node) {
		throw system_error(EPERM,system_category(),string{"Agent '"} + name() + "' doesnt allow states");
	}
	#pragma GCC diagnostic pop

	void Abstract::Agent::push_back(std::shared_ptr<Abstract::Alert> UDJAT_UNUSED(alert)) {
		throw system_error(EPERM,system_category(),string{"Agent '"} + name() + "' doesnt allow alerts");
	}

	void Abstract::Agent::push_back(const pugi::xml_node UDJAT_UNUSED(&node), std::shared_ptr<Abstract::Alert> alert) {
		push_back(alert);
	}

	void Abstract::Agent::push_back(const Abstract::Agent::Event event, std::shared_ptr<Activatable> activatable) {
		lock_guard<std::recursive_mutex> lock(guard);
		listeners.emplace_back(event,activatable);
	}

	void Abstract::Agent::remove(const Abstract::Agent::Event event, std::shared_ptr<Activatable> activatable) {
		lock_guard<std::recursive_mutex> lock(guard);
		listeners.remove_if([event,activatable](Listener &ev){
			return ev.event == event && ev.activatable.get() == activatable.get();
		});
	}

	void Abstract::Agent::remove(std::shared_ptr<Activatable> activatable) {
		lock_guard<std::recursive_mutex> lock(guard);
		listeners.remove_if([activatable](Listener &ev){
			return ev.activatable.get() == activatable.get();
		});
	}

	std::shared_ptr<Abstract::State> Abstract::Agent::computeState() {

		static shared_ptr<Abstract::State> instance;
		if(!instance) {
			debug("Creating agent default state");
			instance = make_shared<Abstract::State>(
							_( "default" ),
							Level::unimportant,
							_( "Normal" ),
							_( "Agent has nothing to report" )
						);

		}
		return instance;

	}

}
