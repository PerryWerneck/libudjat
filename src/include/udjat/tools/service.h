/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/moduleinfo.h>
 #include <cstring>

 namespace Udjat {

	/// @brief Service who can be started/stopped.
	class UDJAT_API Service {
	private:

		/// @brief Service state.
		struct {
			/// @brief Is the service active?
			bool active = false;
		} state;

		/// @brief Service module.
		const ModuleInfo &module;

	protected:
		/// @brief Service name.
		const char *service_name = "service";

	public:
		class Controller;
		friend class Controller;

		Service(const Service &src) = delete;
		Service(const Service *src) = delete;

		Service(const char *name, const ModuleInfo &module);
		Service(const ModuleInfo &module);
		virtual ~Service();

		const char * name() const noexcept {
			return service_name;
		}

		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,service_name) == 0;
		}

		inline const char * description() const noexcept {
			return module.description;
		}

		inline const char * version() const noexcept {
			return module.version;
		}

		inline bool isActive() const noexcept {
			return state.active;
		}

		inline bool active() const noexcept {
			return state.active;
		}

		virtual Value & getProperties(Value &properties) const noexcept;

		virtual void start();
		virtual void stop();

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

	};

 }


