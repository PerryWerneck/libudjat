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

		/// @brief I/O handler
		class Handler;

#ifdef _WIN32

		/// @brief Subprocess watcher.
		class Watcher;
		friend class Watcher;

		PROCESS_INFORMATION piProcInfo;

		/// @brief The process exit code.
		DWORD exitcode = -1;

		/// @brief Initialize.
		void init(Handler &outpipe, Handler &errpipe);

#else

		/// @brief Subprocess controller.
		class Controller;
		friend class Controller;

		/// @brief Pid of the subprocess.
		pid_t pid = -1;

		inline bool running() const noexcept {
			return this->pid != -1;
		}

		void init(Handler &outpipe, Handler &errpipe);

#endif // _WIN32

		/// @brief The command line to start.
		std::string command;

		/// @brief Parse command-line argument.
		static char * get_next_argument(char **txtptr);

	protected:

		/// @brief Default outputs (if onStdOut and onStdErr are not overrided).
		struct LogLevels {
			Logger::Level out = Logger::Trace;
			Logger::Level err = Logger::Error;

			constexpr LogLevels(Logger::Level o, Logger::Level e) : out{o}, err{e} {
			}

		} loglevels;

		virtual void pre();

		virtual void post(int status);

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

		class UDJAT_API Arguments {
		private:

			struct {
				size_t count = 0;
				char **values = nullptr;
			} args;

			void append(char *arg);

		public:

#ifdef _WIN32

#else

			/// @brief Get from application command line.
			static Arguments from_pid();

			/// @brief Get from application command line from pid.
			static Arguments from_pid(int pid);

#endif // _WIN32

			constexpr Arguments() {
			}

			Arguments(const char *cmdline);

			~Arguments();

			Arguments & push_back(const char *arg) noexcept;
			Arguments & push_back(const std::string &value) noexcept;

			/// @brief Get argument.
			/// @param argname The argname without '--' or '-'.
			/// @return The argument value os nullptr if not found.
			const char * operator[](const char *argname) const;

			/// @brief Get argument.
			/// @param argname The argname without '--' or '-'.
			/// @return The argument value os nullptr if not found.
			const char * operator[](const char argname) const;

			template <typename T>
			Arguments & push_back(const T &value) {
				return push_back(std::to_string(value));
			}

			inline size_t size() const noexcept {
				return args.count;
			}

			inline size_t argc() const noexcept {
				return args.count;
			}

			inline const char ** argv() const noexcept {
				return (const char **) args.values;
			}

		};

		SubProcess(const SubProcess &) = delete;
		SubProcess(const SubProcess *) = delete;

		SubProcess(const char *name, const char *command, Logger::Level out = Logger::Trace, Logger::Level err = Logger::Error);

		SubProcess(const NamedObject *obj, const char *command, Logger::Level out = Logger::Trace, Logger::Level err = Logger::Error)
			: SubProcess(obj->name(),command,out,err) {
		}

		SubProcess(const NamedObject &obj, const char *command, Logger::Level out = Logger::Trace, Logger::Level err = Logger::Error)
			: SubProcess(obj.name(),command,out,err) {
		}

		/// @brief Create a sub-process with the default name.
		SubProcess(const char *command, Logger::Level out = Logger::Trace, Logger::Level err = Logger::Error);

		virtual ~SubProcess();

		/// @brief Get command line
		inline const char * c_str() const noexcept {
			return command.c_str();
		}

		/// @brief Run subprocess in foreground.
		/// @return Sub process return code.
		int run();

#ifndef _WIN32
		/// @brief Run subprocess with popen.
		int prun();
#endif // _WIN32

		/// @brief Start sub process in background using the default object.
		static void start(const char *command);

		static void start(const NamedObject *obj, const char *command);

		/// @brief Start sub process in foreground using the default object.
		/// @return Sub process return code.
		static int run(const char *command, Logger::Level out = Logger::Trace, Logger::Level err = Logger::Error);

		static int run(const char *name, const char *command, Logger::Level out = Logger::Trace, Logger::Level err = Logger::Error);

		static int run(const NamedObject *obj, const char *command, Logger::Level out = Logger::Trace, Logger::Level err = Logger::Error);
	};

 }

 namespace std {

	inline Udjat::SubProcess::Arguments & operator<< (Udjat::SubProcess::Arguments &opt, const char *value ) {
		return opt .push_back(value);
	}

	template <typename T>
	inline Udjat::SubProcess::Arguments & operator<< (Udjat::SubProcess::Arguments &opt, const T &value ) {
			return opt.push_back(value);
	}

	string to_string(const Udjat::SubProcess::Arguments &opt, const char *sep = " ");

	inline ostream& operator<< (ostream& os, Udjat::SubProcess::Arguments &opt) {
		return os << to_string(opt);
	}

 }
