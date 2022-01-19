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

	/// @brief Alert activation (the object who really send the alerts).
	class Alert::Activation {
	private:
		friend class Alert::Controller;

		std::shared_ptr<Alert> alertptr;

		struct {
			time_t last = 0;
			time_t next = 0;
		} timers;

		struct {
			unsigned int success = 0;
			unsigned int failed = 0;
		} count;

		void checkForSleep(const char *msg) noexcept;

		bool restarting = false;
		time_t running = 0;

	public:
		Activation(std::shared_ptr<Alert> alert);

		virtual ~Activation();

		/// @brief Schedule next alert.
		void next() noexcept;
		void success() noexcept;
		void failed() noexcept;

		inline const char * name() const noexcept {
			return alertptr->c_str();
		}

		inline std::shared_ptr<Alert> alert() const {
			return alertptr;
		};

	};;

	/// @brief Singleton for alert emission.
	class Alert::Controller : public Udjat::Factory {
	private:

		/// @brief Mutex for serialization
		static mutex guard;

		/// @brief List of active workers.
		list<shared_ptr<Alert::Activation>> activations;

		/// @brief Alert workers.
		// list<const Alert::Worker *> workers;

		Controller();

		/// @brief Emit pending alerts.
		void emit() noexcept;

		/// @brief Reset update timer.
		/// @param seconds Seconds for the next 'emit()'.
		void reset(time_t seconds) noexcept;

	public:
		static Controller & getInstance();
		~Controller();

		/// @brief Update timer.
		void refresh() noexcept;

		/// @brief Insert worker.
		// void insert(const Alert::Worker *worker);

		/// @brief Remove worker.
		// void remove(const Alert::Worker *worker);

		/// @brief Insert activation.
		void insert(const std::shared_ptr<Alert::Activation> activation);

		/// @brief Insert URL activation.
		void insert(const char *name, const char *url, const char *action, const char *payload);

		/// @brief Remove alert activation.
		void remove(const Alert *alert);

		/// @brief Create State child.
		bool parse(Abstract::State &parent, const pugi::xml_node &node) const override;

	};

 }

