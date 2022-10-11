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
 #include <udjat/tools/logger.h>

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

		debug("Agent '",name(),"' update set to ", TimeStamp(update.next));

		if(update.next < saved) {
			debug("Agent timer needs reset");
			Controller::getInstance().reset();
		}
		return update.next;
	}

 }
