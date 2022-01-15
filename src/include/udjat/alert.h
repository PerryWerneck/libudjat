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

 #include <udjat/defs.h>
 #include <udjat/agent.h>
 #include <udjat/state.h>

 namespace Udjat {

	class UDJAT_API Alert {
	public:

		/// @brief Alert settings
		struct Settings {
			const char *name = "alert";		///< @brief Alert name.
			const char *action = "get";		///< @brief Alert action.
			const char *url = "";			///< @brief Alert URL.
			const char *payload = "";		///< @brief Alert payload.

			time_t start = 0;				///< @brief Seconds to wait before first activation.
			time_t interval = 60;			///< @brief Seconds to wait on every try.

			/// @brief Alarm sents.
			struct {
				size_t min = 1;				///< @brief How many success emissions after deactivation or sleep?
				size_t max = 3;				///< @brief How many retries (success+fails) after deactivation or sleep?
			} limits;

			/// @brief Restart timers.
			struct {
				time_t failed = 14400;		///< @brief Seconds to wait for reactivate after a failed activation.
				time_t success = 86400;		///< @brief Seconds to wait for reactivate after a successfull activation.
			} restart;

			constexpr Settings(const char *n, const char *u, const char *a, const char *p) : name(n), action(a), url(u), payload(p) {
			}

			Settings(const pugi::xml_node &node);

		};

		/// @brief Alert worker.
		class Worker {
		private:

			Settings settings;

			struct {
				size_t success = 0;		///< @brief How many successful sends in the current cycle.
				size_t failed = 0;		///< @brief How many failed sends in the current cycle.
				time_t last = 0;		///< @brief Last try.
				time_t next = 0;		///< @brief Next try (0 = disabled).
			} events;

		protected:

			/// @brief Send alert.
			virtual void send();

		public:
			constexpr Worker(const Settings &s) : settings(s) {
			}

			~Worker();

			/// @brief Restart cycle.
			void reset() {
				events.success = events.failed = 0;
			}

		};

	protected:

		/// @brief Alert settings.
		Settings settings;

	public:
		constexpr Alert(const char *name, const char *url, const char *type="get", const char *payload = nullptr) : settings(name,url,type,payload) {
		}

		Alert(const pugi::xml_node &node);

	};


 }

