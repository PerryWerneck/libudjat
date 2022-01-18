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

	/// @brief Alert private data.
	struct Alert::PrivateData {
		Alert *alert;
		const char *name;
		string url;
		string payload;

		PrivateData(Alert *alert);
		PrivateData(Alert *alert, const string &payload);

	};

	/// @brief Singleton for alert emission.
	class Alert::Controller : public Alert::Worker {
	private:

		/// @brief Mutex for serialization
		static mutex guard;

		/// @brief List of active workers.
		list<Alert::PrivateData> alerts;

		/// @brief Alert workers.
		list<const Alert::Worker *> workers;

		Controller();

		/// @brief Emit pending alerts.
		void emit() noexcept;

		/// @brief Reset update timer.
		/// @param seconds Seconds for the next 'emit()'.
		void reset(time_t seconds) noexcept;

	public:
		static Controller & getInstance();
		~Controller();

		void activate(Alert *alert);
		void activate(Alert *alert, const string &payload);
		void deactivate(Alert *alert);

		/// @brief Update timer.
		void refresh() noexcept;

		/// @brief Insert worker.
		void insert(const Alert::Worker *worker);

		/// @brief Remove worker.
		void remove(const Alert::Worker *worker);

		/// @brief Get workers.
		const Alert::Worker * getWorker(const char *name) const;

	};

 }

