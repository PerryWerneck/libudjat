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
 #include <string>

 namespace Udjat {

	/// @brief Creates and run a child process.
	class UDJAT_API SubProcess {
	private:

		/// @brief Subprocess controller.
		class Controller;
		friend class Controller;

		/// @brief Pid of the subprocess.
		pid_t pid = -1;

		struct {
			bool failed = false;
			int exit = 0;
			int termsig = 0;
		} status;

		struct {
			int fd = -1;
			size_t length = 0;
			char buffer[256];
		} pipes[2];

		/// @brief Read from pipe.
		void read(int id);

		/// @brief The command line to start.
		std::string command;

	protected:

		/// @brief Destroy subprocess (automatic when the subprocess finishes).
		virtual ~SubProcess();

		/// @brief Called on subprocess output.
		virtual void onStdOut(const char *line);

		/// @brief Called on subprocess stderr (redirects to onStdOut if not overrided).
		virtual void onStdErr(const char *line);

		/// @brief Called on subprocess normal exit (redirects to onStdOut if not overrided).
		virtual void onExit(int rc);

		/// @brief Called on subprocess abnormal exit (redirects to onStdOut if not overrided).
		virtual void onSignal(int sig);

	public:
		SubProcess(const SubProcess &) = delete;
		SubProcess(const SubProcess *) = delete;

		/// @brief Create a sub-process.
		SubProcess(const char *command);

		/// @brief Get command line
		inline const char * c_str() const noexcept {
			return command.c_str();
		}

		/// @brief Start sub process; this object will be removed when it exits.
		void start();

		/// @brief Start sub process using the default object.
		static void start(const char *command);

	};

 }
