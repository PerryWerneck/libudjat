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
 #include <udjat/agent/abstract.h>
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

	Abstract::Agent::Agent(const char *name, const char *label, const char *summary) : Object{(name && *name) ? name : "unnamed"} {

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

	Abstract::Agent::Agent(const XML::Node &node) : Object{node} {

		update.timer = XML::AttributeFactory(node,"update-timer").as_uint((unsigned int) update.timer);
		update.on_demand = XML::AttributeFactory(node,"update-on-demand").as_bool(update.timer == 0);

		time_t delay = XML::AttributeFactory(node,"delay-on-startup").as_uint((unsigned int) (update.timer ? 1 : 0));
		if(delay)
			update.next = time(nullptr) + delay;

#ifndef _WIN32
		{
			// Check for signal based update.
			const char *signame = XML::AttributeFactory(node,"update-signal").as_string();
			if(*signame && strcasecmp(signame,"none")) {

				// Agent has signal based update.
				update.sigdelay = (short) XML::AttributeFactory(node,"update-signal-delay").as_uint(0);

				Udjat::Event &event = Udjat::Event::SignalHandler(this, signame, [this](){
					sched_update(update.sigdelay);
					return true;
				});

				if(update.sigdelay) {
					info()	<< "An agent update with a "
							<< update.sigdelay
							<< " second(s) delay will be triggered by signal '"
							<< event.to_string() << "'"
							<< endl;
				} else {
					info() << signame << " (" << event.to_string() << ") will trigger an agent update" << endl;
				}

			} else {
				update.sigdelay = -1;
			}

		}
#endif // !_WIN32

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

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	std::shared_ptr<Abstract::State> Abstract::Agent::StateFactory(const XML::Node &node) {
		throw system_error(EPERM,system_category(),string{"Agent '"} + name() + "' doesnt allow states");
	}
	#pragma GCC diagnostic pop

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

	std::shared_ptr<Abstract::State> Abstract::Agent::StateFactory(const char *name, const Udjat::Level level, const char *summary, const char *body) {

		class State : public Abstract::State {
		private:
			String summary;
			String body;

		public:
			State(const Udjat::Object &object, const char *name, const Udjat::Level level, const char *s, const char *b) 
			 : Abstract::State{name,level}, summary{s}, body{b} {

				summary.expand(object);
				body.expand(object);
				
				Abstract::State::properties.body = this->body.c_str();
				Object::properties.summary = this->summary.c_str();
			}
		};

		return make_shared<State>(*this, name, level,summary,body);

	}

}
