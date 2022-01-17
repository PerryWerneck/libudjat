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
 #include <udjat/tools/configuration.h>
 #include <udjat/alert.h>
 #include <udjat/worker.h>
 #include <mutex>
 #include <list>

 using namespace std;

 namespace Udjat {

	/// @brief Singleton for alert emission.
	class Alert::Controller {
	private:

		/// @brief Mutex for serialization
		static mutex guard;

		struct Active {
			Alert *alert;
			const char *name;
			string url;
			string payload;

			Active(Alert *alert);

		};

		/// @brief List of active workers.
		list<Active> alerts;

		Controller();

		/// @brief Emit pending alerts.
		void emit() noexcept;

	public:
		static Controller & getInstance();
		~Controller();

		void activate(Alert *alert);
		void deactivate(Alert *alert);

	};

 }

