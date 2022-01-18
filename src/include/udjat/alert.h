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
 #include <udjat/state.h>

 namespace Udjat {

	class UDJAT_API Alert {
	private:
		class Controller;
		friend class Controller;

		class PrivateData;

		void emit(const PrivateData &priv) noexcept;
		void checkForSleep(const char *msg) noexcept;
		void next() noexcept;

	public:

		/// @brief Alert worker.
		class UDJAT_API Worker {
		private:
			const char *name = "";
			friend class Controller;

			constexpr Worker() {
			}

		protected:

			/// @brief Worker module info.
			const ModuleInfo *info = nullptr;

		public:
			Worker(const char *name);
			Worker(const char *name, const ModuleInfo *info);

			virtual ~Worker();

			virtual void send(const Alert &alert, const std::string &url, const std::string &payload) const;

		};

		/// @brief Alert settings
		struct Settings {
			const char *name = "alert";		///< @brief Alert name.
			const char *action = "get";		///< @brief Alert action.
			const char *url = "";			///< @brief Alert URL.
			const char *payload = "";		///< @brief Alert payload.

			constexpr Settings(const char *n, const char *u, const char *a, const char *p) : name(n), action(a), url(u), payload(p) {
			}

			Settings(const pugi::xml_node &node);

		};

	protected:

		/// @brief Alert settings.
		Settings settings;

		/// @brief Alert worker.
		const Worker *worker = nullptr;

		/// @brief Alert limits.
		struct {
			size_t min = 1;				///< @brief How many success emissions after deactivation or sleep?
			size_t max = 3;				///< @brief How many retries (success+fails) after deactivation or sleep?
		} retry;

		/// @brief Alert activations.
		struct {
			time_t last = 0;
			time_t next = 0;
			unsigned int success = 0;
			unsigned int failed = 0;
		} activations;

		/// @brief Alert timers.
		struct {
			time_t start = 0;			///< @brief Seconds to wait before first activation.
			time_t interval = 60;		///< @brief Seconds to wait on every try.
			time_t busy = 60;			///< @brief Seconds to wait if the alert is busy when activated.
		} timers;

		/// @brief Restart timers.
		struct {
			time_t failed = 14400;		///< @brief Seconds to wait for reactivate after a failed activation.
			time_t success = 86400;		///< @brief Seconds to wait for reactivate after a successful activation.
		} restart;

		bool restarting = false;
		time_t running = 0;

	public:

		constexpr Alert(const char *name, const char *url, const char *type="get", const char *payload = "") : settings(name,url,type,payload) {
		}

		Alert(const pugi::xml_node &node);
		~Alert();

		static void initialize();

		inline const char * name() const noexcept {
			return settings.name;
		}

		inline const char * action() const noexcept {
			return settings.action;
		}

		inline const char * url() const noexcept {
			return settings.url;
		}

		inline const char * payload() const noexcept {
			return settings.payload;
		}

		void activate(const std::string &url, const std::string &payload);
		void activate(const std::string &payload);
		void activate();
		void deactivate();

	};


 }

