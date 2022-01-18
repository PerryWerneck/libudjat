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
 #include <udjat/tools/logger.h>
 #include <udjat/alert.h>
 #include <udjat/worker.h>
 #include <udjat/factory.h>
 #include <mutex>
 #include <list>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	/// @brief Alert private data.
	struct Alert::PrivateData {

		shared_ptr<Alert> alert;
		string url;
		string payload;

		PrivateData(shared_ptr<Alert> alert);
		PrivateData(shared_ptr<Alert> alert, const string &url, const string &payload);

		inline const char *name() const noexcept {
			return alert->name();
		}

	};

	/// @brief Singleton for alert emission.
	class Alert::Controller : public Alert::Worker, public Udjat::Factory {
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

		void insert(std::shared_ptr<Alert> alert, const std::string &url, const std::string &payload);
		void remove(std::shared_ptr<Alert> alert);

		/// @brief Update timer.
		void refresh() noexcept;

		/// @brief Insert worker.
		void insert(const Alert::Worker *worker);

		/// @brief Remove worker.
		void remove(const Alert::Worker *worker);

		/// @brief Get workers.
		const Alert::Worker * getWorker(const char *name) const;

		/// @brief Create State child.
		bool parse(Abstract::State &parent, const pugi::xml_node &node) const override;

	};

 }

