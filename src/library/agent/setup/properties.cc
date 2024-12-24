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
 * @file
 *
 * @brief Implements the agent properties loader.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/agent.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/event.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/logger.h>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Abstract::Agent::Controller::setup_properties(Abstract::Agent &agent, const XML::Node &root) noexcept {

		const char *section = root.attribute("settings-from").as_string("agent-defaults");

		agent.update.timer = getAttribute(root,section,"update-timer",(unsigned int) agent.update.timer);
		agent.update.on_demand = getAttribute(root,section,"update-on-demand",agent.update.timer == 0);

		time_t delay = getAttribute(root,section,"delay-on-startup",(unsigned int) (agent.update.timer ? 1 : 0));
		if(delay)
			agent.update.next = time(nullptr) + delay;

#ifndef _WIN32
		{
			// Check for signal based update.
			const char *signame = root.attribute("update-signal").as_string();
			if(*signame && strcasecmp(signame,"none")) {

				// Agent has signal based update.
				agent.update.sigdelay = (short) getAttribute(root,section,"update-signal-delay",(unsigned int) 0);

				Udjat::Event &event = Udjat::Event::SignalHandler(&agent, signame, [&agent](){
					agent.sched_update(agent.update.sigdelay);
					return true;
				});

				if(agent.update.sigdelay) {
					agent.info()	<< "An agent update with a "
									<< agent.update.sigdelay
									<< " second(s) delay will be triggered by signal '"
									<< event.to_string() << "'"
									<< endl;
				} else {
					agent.info() << signame << " (" << event.to_string() << ") will trigger an agent update" << endl;
				}

			} else {
				agent.update.sigdelay = -1;
			}

		}
#endif // !_WIN32

	}

}
