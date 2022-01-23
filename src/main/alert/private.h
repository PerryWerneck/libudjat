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
 #include <udjat/tools/mainloop.h>
 #include <mutex>
 #include <list>
 #include <iostream>

 using namespace std;

 namespace Udjat {

	/// @brief Singleton for alert emission.
	class Abstract::Alert::Controller : private MainLoop::Service {
	private:

		/// @brief Mutex for serialization
		static mutex guard;

		/// @brief List of active workers.
		list<shared_ptr<Abstract::Alert::Activation>> activations;

		Controller();

		/// @brief Emit pending alerts.
		void emit() noexcept;

		/// @brief Reset update timer.
		/// @param seconds Seconds for the next 'emit()'.
		void reset(time_t seconds) noexcept;

	protected:
		void stop() override;

	public:
		static Controller & getInstance();
		~Controller();

		/// @brief Update timer.
		void refresh() noexcept;

		//void insert(const std::shared_ptr<Abstract::Alert::Activation> activation);

		/// @brief Activate an alert.
		void activate(std::shared_ptr<Alert> alert, const std::function<void(std::string &str)> &expander);

		/// @brief Remove alert activation.
		void remove(const Abstract::Alert *alert);

	};

 }

