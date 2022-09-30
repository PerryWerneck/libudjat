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
 #include <udjat/tools/application.h>
 #include <udjat/agent/state.h>
 #include <udjat/tools/timer.h>
 #include <memory>
 #include <list>

 namespace Udjat {

	/// @brief Abstract class for system services.
	class UDJAT_API SystemService : public Udjat::Application {
	private:
		/// @brief Service instance.
		static SystemService *instance;

		/// @brief Path for the xml file(s) with service definitions.
		const char * definitions = nullptr;

		/// @brief Command line parser.
		int cmdline(int argc, const char **argv);

		/// @brief Set service state message.
		void notify(const char *state) noexcept;

		/// @brief Set service state message to the root agent.
		void notify() noexcept;

#ifdef _WIN32
		void registry(const char *name, const char *value);
#endif // _WIN32

	protected:

		/// @brief Service mode.
		enum Mode : uint8_t {
			SERVICE_MODE_DEFAULT,		///< @brief Standard service mode based on OS.
			SERVICE_MODE_NONE,			///< @brief Doesn't run, just quit after parameter parsing.
			SERVICE_MODE_FOREGROUND,	///< @brief Run in foreground as an application.
			SERVICE_MODE_DAEMON			///< @brief Run as daemon (Linux only).
		} mode = SERVICE_MODE_DEFAULT;

		/// @brief Reconfigure application from XML files.
		/// @param force Do a reconfiguration even if the file hasn't change.
		/// @param pathname Path for a xml file or folder with xml files.
		virtual void reconfigure(const char *pathname, bool force) noexcept;

		/// @brief Factory for the application root.
		virtual std::shared_ptr<Abstract::Agent> RootFactory() const;

		/// @brief Check '--param=value' command line options.
		/// @param key Option name.
		/// @param value Option value (can be nullptr).
		/// @retval 0 Normal exit.
		/// @retval -2 Continue as a service.
		/// @retval ENOENT Invalid option.
		virtual int cmdline(const char *key, const char *value = nullptr);

		/// @brief Check '-p value' command line options.
		/// @param key Option name.
		/// @param value Option value (can be nullptr).
		/// @return result.
		/// @retval 0 Normal exit.
		/// @retval -2 Continue as a service.
		/// @retval ENOENT Invalid option.
		virtual int cmdline(const char key, const char *value = nullptr);

#ifdef _WIN32

		/// @brief Install win32 service.
		/// @return 0 when success.
		virtual int install();

		virtual int install(const char *display_name);

		/// @brief Uninstall win32 service.
		/// @return 0 when success.
		virtual int uninstall();

#endif // _WIN32

		/// @brief Send usage help to std::cout
		virtual void usage() const noexcept;

		SystemService(const char *definitions = nullptr);

	public:

		virtual ~SystemService();

		/// @brief Get Service state.
		std::shared_ptr<Abstract::State> state() const;

		/// @brief Get current service instance.
		static SystemService * getInstance();

		/// @brief Initialize service.
		virtual void init();

		/// @brief Deinitialize service.
		virtual void deinit();

		/// @brief Stop service.
		virtual void stop();

		/// @brief Service main loop
		virtual int run() noexcept;

		/// @brief Parse command line arguments and run service.
		virtual int run(int argc, char **argv);

	};

 }


