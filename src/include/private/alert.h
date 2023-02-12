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
 #include <udjat/alert/abstract.h>
 #include <udjat/worker.h>
 #include <udjat/factory.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/timer.h>
 #include <udjat/tools/service.h>
 #include <udjat/alert/abstract.h>
 #include <mutex>
 #include <list>
 #include <iostream>

 using namespace std;

 namespace Udjat {

 	/// @brief Singleton for alert emission.
	class Alert::Controller : private Service, private Udjat::Worker, private MainLoop::Timer {
	private:
		/// @brief Mutex for serialization
		static mutex guard;

		/// @brief List of active workers.
		list<shared_ptr<Udjat::Alert::Activation>> activations;

		Controller();

		void on_timer() override;

		/// @brief Reset timer.
		/// @param seconds Seconds for the next 'emit()'.
		void reset(time_t interval) noexcept;

		/// @brief Emit pending alerts.
		void emit() noexcept;

		/// @brief Disable active alerts.
		void clear() noexcept;

	protected:
		void stop() override;

		/// @brief How many activations?
		size_t running() const noexcept;

	public:
		static Controller & getInstance();
		virtual ~Controller();

		void push_back(shared_ptr<Udjat::Alert::Activation> activation);
		void remove(const Abstract::Alert *alert);
		bool active(const Abstract::Alert *alert);
		bool get(Request &request, Response &response) const override;

	};


 }

