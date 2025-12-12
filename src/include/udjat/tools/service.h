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

	protected:
		/// @brief Service name.
		const char *service_name = "service";

#ifdef PACKAGE_DESCRIPTION
		const char *service_description = PACKAGE_DESCRIPTION;
#else
		const char *service_description = "Udjat Service";
#endif // PACKAGE_DESCRIPTION

	public:
		class Controller;
		friend class Controller;

		Service(const Service &src) = delete;
		Service(const Service *src) = delete;

#ifdef PACKAGE_DESCRIPTION
		Service(const char *name, const char *description = PACKAGE_DESCRIPTION);
#else
		Service(const char *name, const char *description = PACKAGE_NAME);
#endif

		virtual ~Service();

		const char * name() const noexcept {
			return service_name;
		}

		inline const char * description() const noexcept {
			return service_description;
		}

		inline const char * version() const noexcept {
#ifdef PACKAGE_VERSION
			return PACKAGE_VERSION;
#else
			return "";
#endif
		}

		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,service_name) == 0;
		}

		inline bool isActive() const noexcept {
			return state.active;
		}

		inline bool active() const noexcept {
			return state.active;
		}

		/// @brief Get service by name.
		/// @param name service name.
		/// @return Pointer to service or nullptr if not found.
		static const Service * find(const char *name) noexcept;

		virtual Value & getProperties(Value &properties) const;

		virtual void start();
		virtual void stop();

		static bool for_each(const std::function<bool(const Service &service)> &method);

	};

 }


