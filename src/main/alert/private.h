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
 #include <udjat/factory.h>
 #include <udjat/url.h>
 #include <list>
 #include <mutex>

 using namespace std;

 namespace Udjat {

	class Alert::Controller : private Worker, private Factory {
	private:
		Controller();

		/// @brief Mutex for serialization.
		static mutex guard;

		/// @brief List of active alerts.
		list<std::shared_ptr<Alert::Event>> events;

		/// @brief Build alert from XML
		shared_ptr<Alert> build(const pugi::xml_node &node);

		static const string getFactoryNameByType(const pugi::xml_node &node);

		void onTimer(time_t now) noexcept;

	public:
		static Controller & getInstance();
		~Controller();

		static string getType(const pugi::xml_node &node);

		void work(const Request &request, Response &response) const override;

		/// @brief Create Agent alert.
		void parse(Abstract::Agent &parent, const pugi::xml_node &node) const override;

		/// @brief Create State alert.
		void parse(Abstract::State &parent, const pugi::xml_node &node) const override;

		/// @brief Activate alert;
		void insert(Alert *alert, std::shared_ptr<Alert::Event> event);

		/// @brief Deactivate alert.
		void remove(const Alert *alert);

		/// @brief Remove event.
		void remove(const Alert::Event *event);

	};

	class URLAlert : public Udjat::Alert {
	private:

		/// @brief The URL request method.
		URL::Method method;

		/// @brief The URL.
		Quark url;

		/// @brief Connection timeout.
		time_t timeout = 60;

		/// @brief Mimetype.
		string mimetype = "application/json; charset=utf-8";

	public:

		URLAlert(const pugi::xml_node &node);
		virtual ~URLAlert();

	};


 }

