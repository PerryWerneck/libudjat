/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
 * @brief Implements the abstract agent event handling
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/tools/xml.h>
 #include <mutex>

namespace Udjat {

	void Abstract::Agent::notify(const Event event) {
		lock_guard<std::recursive_mutex> lock(guard);
		for(Listener &listener : listeners) {
			if((listener.event & event) != 0) {
				auto activatable = listener.activatable;
				push([activatable](std::shared_ptr<Agent> agent){
					activatable->activate(*agent);
				});
			}
		}
	}

	Abstract::Agent::Event Abstract::Agent::EventFactory(const XML::Node &node, const char *attrname) {

		String name{node,attrname,""};

		if(name.empty()) {
			throw runtime_error(Logger::String{"Required attribute '",attrname,"' is missing or empty"});
		}

		static const struct {
			Event id;
			const char *name;
		} events[] = {
			{ Event::STARTED, 			"started" },
			{ Event::STARTED, 			"start" },
			{ Event::STOPPED, 			"stopped" },
			{ Event::STOPPED, 			"stop" },
			{ Event::STATE_CHANGED,		"state-changed" },
			{ Event::UPDATE_TIMER,		"update-timer" },
			{ Event::VALUE_CHANGED,		"value-changed" },
			{ Event::VALUE_CHANGED,		"changed" },
			{ Event::VALUE_NOT_CHANGED,	"value-not-changed" },
			{ Event::UPDATED,			"updated" },
			{ Event::READY,				"ready" },
			{ Event::NOT_READY,			"not-ready" },
			{ Event::LEVEL_CHANGED,		"level-changed" },
			{ Event::ALL,				"all" },
		};

		for(const auto &event : events) {
			if(!strcasecmp(event.name,name.c_str())) {
				return event.id;
			}
		}

		throw runtime_error(Logger::String{"Unexpected event '",name.c_str(),"'"});

	}

	bool Abstract::Agent::push_back(const XML::Node &node, std::shared_ptr<Activatable> alert) {
		push_back(EventFactory(node,"trigger-event"),alert);
		return true;
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


}
