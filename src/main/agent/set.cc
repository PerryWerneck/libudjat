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
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	bool Abstract::Agent::assign(const char *value) {
		throw system_error(ENOTSUP,system_category(),string{"Agent '"} + name() + "' doesnt allow assign method");;
	}
	#pragma GCC diagnostic pop

	void Abstract::Agent::setOndemand() noexcept {

		update.on_demand = true;

		if(update.timer) {
			cout << name() << "Disabling timer update (" << update.timer << " seconds)" << endl;
			update.timer = 0;
		}
	}

	time_t Abstract::Agent::reset(time_t timestamp) {

		time_t saved = update.next;
		update.next = timestamp;

		debug("Next update for ",name()," set to ",TimeStamp(update.next).to_string().c_str());

		if(update.next < saved) {

			debug("Agent timer needs reset");

			if(root()) {

				time_t now = time(nullptr);
				time_t next{now+Config::Value<time_t>("agent","min-update-time",600)};

				root()->for_each([&next](std::shared_ptr<Agent> agent) {
					if(agent->update.next) {
						next = std::min(next,agent->update.next);
					}
				});

				debug("Next agent update will be ",TimeStamp(next));

				if(now > next) {
					Controller::getInstance().reset( (now-next) * 1000);
				} else {
					Controller::getInstance().reset(0);
				}
			}

		}
		return update.next;
	}

	time_t Abstract::Agent::sched_update(time_t seconds) {
		debug("Agent '",name(),"' will wait for ",seconds," seconds");
		return reset(time(nullptr) + seconds);
	}

	void Abstract::Agent::requestRefresh(time_t seconds) {
		sched_update(seconds);
	}

 }
