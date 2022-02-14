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
 #include <memory>

 namespace Udjat {

	/// @brief Abstract class for system services.
	class UDJAT_API SystemService {
	private:
		void load() noexcept;
		void reconfigure() noexcept;

	protected:

		/// @brief Path for the xml file(s) with service definitions.
		const char * definitions = nullptr;

		/// @brief Factory for the application root.
		virtual std::shared_ptr<Abstract::Agent> RootFactory() const;

#ifdef _WIN32

		/// @brief Install win32 service.
		virtual int install();
		virtual int install(const char *display_name);

		/// @brief Uninstall win32 service.
		virtual int uninstall();

#else
		static SystemService *instance;
		static void onReloadSignal(int signal) noexcept;

#endif // _WIN32

		/// @brief Send usage help to std::cout
		virtual void usage(const char *appname) const noexcept;

	public:

		constexpr SystemService() {
		}

		constexpr SystemService(const char *d) : definitions(d) {
		}

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


