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

 namespace Udjat {

	/// @brief Abstract class for system services.
	class UDJAT_API SystemService {
	protected:

#ifdef _WIN32

		/// @brief Install win32 service.
		virtual void install();
		virtual void install(const char *display_name);

		/// @brief Uninstall win32 service.
		virtual void uninstall();

#endif // _WIN32

	public:
		SystemService();
		virtual ~SystemService();

		/// @brief Initialize service.
		virtual void init();

		/// @brief Deinitialize service.
		virtual void deinit();

		/// @brief Stop service.
		virtual void stop();

		/// @brief Service main loop
		virtual int run();

		/// @brief Parse command line arguments and run service.
		virtual int run(int argc, char **argv);

	};

 }


