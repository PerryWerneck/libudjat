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
		/// @brief Service instance.
		static SystemService *instance;

		/// @brief Path for the xml file(s) with service definitions.
		const char * definitions = nullptr;

		/// @brief Command line parser.
		int cmdline(const char *appname, int argc, const char **argv);

	protected:

		/// @brief Reconfigure application from XML files.
		/// @param pathname Path for a xml file or folder with xml files.
		virtual void reconfigure(const char *pathname) noexcept;

		/// @brief Factory for the application root.
		virtual std::shared_ptr<Abstract::Agent> RootFactory() const;

		/// @brief Check '--param=value' command line options.
		/// @param key Option name.
		/// @param value Option value (can be nullptr).
		/// @retval 0 Normal exit.
		/// @retval -2 Continue as a service.
		/// @retval ENOENT Invalid option.
		virtual int cmdline(const char *appname, const char *key, const char *value = nullptr);

		/// @brief Check '-p value' command line options.
		/// @param key Option name.
		/// @param value Option value (can be nullptr).
		/// @return result.
		/// @retval 0 Normal exit.
		/// @retval -2 Continue as a service.
		/// @retval ENOENT Invalid option.
		virtual int cmdline(const char *appname, const char key, const char *value = nullptr);

#ifdef _WIN32

		/// @brief Install win32 service.
		virtual int install();
		virtual int install(const char *display_name);

		/// @brief Uninstall win32 service.
		virtual int uninstall();

#else
		static void onReloadSignal(int signal) noexcept;

#endif // _WIN32

		/// @brief Send usage help to std::cout
		virtual void usage(const char *appname) const noexcept;

		SystemService(const char *definitions = nullptr);

	public:

		virtual ~SystemService();

		/// @brief Get current service instance.
		static SystemService * getInstance();

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


