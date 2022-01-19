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
 #include <memory>

 namespace Udjat {

	class UDJAT_API Alert {
	public:

		class UDJAT_API Factory {
		private:
			const char *name = "";
			const ModuleInfo *info = nullptr;

		public:
			constexpr Factory(const char *n, const ModuleInfo *i) : name(n), info(i) {
			}

			virtual ~Factory() {
			}

		};

	private:
		class Activation;

		class Controller;
		friend class Controller;

	protected:

		/// @brief Alert name.
		const char *name = "alert";

		/// @brief Alert limits.
		struct {
			size_t min = 1;				///< @brief How many success emissions after deactivation or sleep?
			size_t max = 3;				///< @brief How many retries (success+fails) after deactivation or sleep?
		} retry;

		/// @brief Alert timers.
		struct {
			time_t start = 0;			///< @brief Seconds to wait before first activation.
			time_t interval = 60;		///< @brief Seconds to wait on every try.
			time_t busy = 60;			///< @brief Seconds to wait if the alert is busy when activated.
		} timers;

		/// @brief Restart timers.
		struct {
			time_t failed = 14400;		///< @brief Seconds to wait for reactivate after a failed activation.
			time_t success = 0;			///< @brief Seconds to wait for reactivate after a successful activation.
		} restart;

	public:

		constexpr Alert(const char *n) : name(n) {
		}

		/**
		 * @brief Create alert for xml description.
		 * @param node XML node with the alert description.
		 * @param defaults Section on configuration file for the alert default options (can be overrided by xml attribute 'settings-from'.
		 */
		Alert(const pugi::xml_node &node, const char *defaults = "alert-defaults");
		virtual ~Alert();

		static void initialize();

		inline const char * c_str() const noexcept {
			return name;
		}

		/// @brief Emit alert.
		virtual void emit() const;

		static void activate(std::shared_ptr<Alert> alert);
		static void activate(const char *name, const char *url, const char *action="get", const char *payload = "");

		void deactivate();

	};


 }

