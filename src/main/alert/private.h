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

 #pragma once

 #include <config.h>
 #include <udjat/tools/quark.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>
 #include <udjat/alert.h>
 #include <udjat/worker.h>
 #include <list>
 #include <mutex>

 using namespace std;

 namespace Udjat {

	class Alert::Controller : private Worker {
	private:
		Controller();

		/// @brief Mutex for serialization.
		static mutex guard;

		/// @brief Alert description.
		struct ActiveAlert {
			shared_ptr<Alert>		alert;		///< @brief Pointer to the associated alert.
			Abstract::State::Level	level;		///< @brief Alert level.
			string					value;		///< @brief Agent value.
			Quark					summary;	///< @brief Message summary.
			Quark					body;		///< @brief Message body.
			Quark					uri;		///< @brief Web link to this state (Usually used for http exporters).
		};

		/// @brief List of active alerts.
		list<ActiveAlert> alerts;

	public:
		static Controller & getInstance();
		~Controller();

		void work(const Request &request, Response &response) const override;

		/// @brief Agent value has changed.
		void deactivate(std::shared_ptr<Alert> alert);

		/// @brief Activate alert.
		void activate(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, const Abstract::State &state);

	};

 }
