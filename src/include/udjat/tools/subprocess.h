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
 #include <udjat/tools/object.h>
 #include <string>

 namespace Udjat {

	/// @brief Creates and run a child process.
	class UDJAT_API SubProcess : public NamedObject {
	private:

		/// @brief Parse input.
		// void parse(int id);

		/// @brief Read from pipe.
		/// @return true if the pipe is still valid.
		// bool read(int id);

#ifdef _WIN32

		class Watcher;
		friend class Watcher;

		struct Pipe {

			HANDLE hRead = 0;
			HANDLE hWrite = 0;

			size_t length = 0;
			char buffer[1024];

			Pipe();
			~Pipe();

			inline operator bool() const noexcept {
				return hRead != 0;
			}

		} pipes[2];

		PROCESS_INFORMATION piProcInfo;

		int exitcode = -1;

		inline bool running() const noexcept {
			return pipes[0] || pipes[1] || piProcInfo.hProcess;
		}

		/// @brief Initialize.
		void init();

#else

		/// @brief Subprocess controller.
		class Controller;
		friend class Controller;

		/// @brief I/O handler
		class Handler;

		/// @brief Pid of the subprocess.
		pid_t pid = -1;

		struct {
			bool failed = false;
			int exit = 0;
			int termsig = 0;
		} status;

		inline bool running() const noexcept {
			return this->pid != -1;
		}

#endif // _WIN32

		/// @brief The command line to start.
		std::string command;

	protected:

		/// @brief Called on subprocess output.
		virtual void onStdOut(const char *line);

		/// @brief Called on subprocess stderr (redirects to onStdOut if not overrided).
		virtual void onStdErr(const char *line);

		/// @brief Called on subprocess normal exit (redirects to onStdOut if not overrided).
		virtual void onExit(int rc);

		/// @brief Called on subprocess abnormal exit (redirects to onStdOut if not overrided).
		virtual void onSignal(int sig);

		/// @brief Start sub process in background; the object will be removed when it exits.
		void start();

	public:
		SubProcess(const SubProcess &) = delete;
		SubProcess(const SubProcess *) = delete;

		SubProcess(const char *name, const char *command);

		SubProcess(const NamedObject *obj, const char *command) : SubProcess(obj->name(),command) {
		}

		/// @brief Create a sub-process with the default name.
		SubProcess(const char *command);

		virtual ~SubProcess();

		/// @brief Get command line
		inline const char * c_str() const noexcept {
			return command.c_str();
		}

		/// @brief Run subprocess in foreground.
		/// @return Sub process return code.
		int run();

		/// @brief Start sub process in background using the default object.
		static void start(const char *command);

		static void start(const NamedObject *obj, const char *command);

		/// @brief Start sub process in foreground using the default object.
		/// @return Sub process return code.
		static int run(const char *command);

		static int run(const NamedObject *obj, const char *command);
	};

 }
