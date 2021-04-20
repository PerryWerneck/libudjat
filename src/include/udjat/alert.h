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

		bool active = false;
		bool disable_when_failed = false;
		bool reset_when_activated = true;

		/// @brief Activates on every value change.
		bool activate_on_value_change = true;

		struct {
			size_t limit = 3;		///< @brief How many retries for activation?
			time_t start = 0;		///< @brief Seconds to wait before first activation.
			time_t interval = 60;	///< @brief Seconds to wait on every try.
			time_t restart = 86400;	///< @brief Seconds to wait for reactivate after maximum tries.

			size_t current = 0;		///< @brief How many retries I did in the current activation?
			time_t last = 0;		///< @brief Last try.
			time_t next = 0;		///< @brief Next try.
		} retry;

	public:

		/// @brief Get configuration file section for default values.
		static std::string getConfigSection(const pugi::xml_node &node);

		Alert(const Quark &name);
		Alert(const char *name);
		Alert(const pugi::xml_node &node);
		virtual ~Alert();

		/// @brief Initialize alert subsystem.
		static void init();

		inline const char * c_str() const noexcept {
			return name.c_str();
		}

		/// @brief Agent value has changed.
		static void set(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, bool level_has_changed);

		/// @brief State state has changed.
		static void set(std::shared_ptr<Alert> alert, const Abstract::Agent &agent, const Abstract::State &state, bool active);

	};

 }

