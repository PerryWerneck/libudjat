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
 #include <udjat/tools/quark.h>
 #include <udjat/alert.h>
 #include <udjat/request.h>
 #include <udjat/tools/logger.h>

 namespace Udjat {

	/// @brief Alert.
	class UDJAT_API Alert : public Logger {
	private:

		/// @brief The Alert controller.
		class Controller;
		friend class Controller;

		/// @brief How many retrys?
		size_t retry = 3;

		/// @brief Alert timers.
		struct {
			time_t start = 0;	///< @brief Seconds to wait before first activation.
			time_t retry = 60;	///< @brief Seconds for retry.
		} timers;

	public:

		/// @brief The Alert factory.
		class UDJAT_API Factory {
		public:
			Factory(const Quark &name);
			virtual ~Factory();

			virtual void parse(Abstract::Agent &agent, const pugi::xml_node &node) const;
			virtual void parse(Abstract::State &state, const pugi::xml_node &node) const;

		};

		Alert(const Quark &name);
		Alert(const char *name);
		Alert(const pugi::xml_node &node);
		virtual ~Alert();

		inline const char * c_str() const noexcept {
			return name.c_str();
		}

		/// @brief Agent value has changed.
		static void set(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, bool level_has_changed);

		/// @brief State state has changed.
		static void set(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, const Abstract::State &state, bool active);

	};

 }

