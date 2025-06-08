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
 #include <cstdint>
 #include <udjat/tools/application.h>

 #ifdef _WIN32
	#include <udjat/win32/service.h>
 #endif // _WIN32

 namespace Udjat {

	class UDJAT_API SystemService : public Udjat::Application, private MainLoop::Timer {
	private:

		static SystemService *instance;

#ifdef _WIN32
		const char *definitions = nullptr;

		Win32::Service::Status service_status;
		SERVICE_STATUS_HANDLE hStatus = 0;

		void set(DWORD state, DWORD wait = 0) {
			service_status.set(hStatus,state,wait);
		}

		static void WINAPI handler(DWORD CtrlCmd);
		static void dispatcher();

#endif // _WIN32

	protected:

		typedef Udjat::SystemService super;

		/// @brief Watchdog timer.
		void on_timer() override;

		/// @brief Set root agent.
		/// @param agent The new root agent.
		void root(std::shared_ptr<Abstract::Agent> agent) override;

		/// @brief Reconfigure service.
		//void setup(const char *pathname, bool startup) noexcept override;

		/// @brief Show help text to stdout.
		void help(size_t width = 20) const noexcept override;

		//int setup(const char *definitions = nullptr) override;

	public:
		SystemService(const SystemService&) = delete;
		SystemService& operator=(const SystemService &) = delete;
		SystemService(SystemService &&) = delete;
		SystemService & operator=(SystemService &&) = delete;

		static SystemService & getInstance();

		SystemService(int argc, char **argv);
		virtual ~SystemService();

		/// @brief Parse command line options, run service.
		/// @param definitions Path to a single xml file or a folder with xml files.
		int run(const char *definitions = nullptr);

		/// @brief Set service state.
		/// @param level The current service level.
		/// @param message The message to be shown.
		/// @return Dialog status.
		Dialog::Status & state(const Level level, const char *message) noexcept override;

		/// @brief Initialize service.
		//int init(const char *definitions = nullptr) override;

		/// @brief Deinitialize service.
		//int deinit(const char *definitions = nullptr) override;

		/// @brief Install service.
		/// @return 0 when success, errno if failed.
		/// @retval ENOTSUP No support for this method.
		int install(const char *description = nullptr) override;

		/// @brief Uninstall service.
		/// @return 0 when success, errno if failed.
		/// @retval ENOTSUP No support for this method.
		int uninstall() override;

		/// @brief Start service.
		virtual int start();

		/// @brief Stop service.
		virtual int stop();

	};


 }


