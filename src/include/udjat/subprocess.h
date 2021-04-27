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

 /***
  * @brief Implement the SubProcess object.
  *
  * References:
  *
  * <https://opensource.apple.com/source/Libc/Libc-167/gen.subproj/popen.c.auto.html>
  *
  */

 #pragma once
 #include <udjat/defs.h>
 #include <sys/types.h>
 #include <string>

 namespace Udjat {

	/// @brief Creates and run a child process.
	class UDJAT_API SubProcess {
	private:

		/// @brief Subprocess controller.
		class Controller;
		friend class Controller;

		/// @brief The command line to start.
		std::string command;

	protected:

		/// @brief Destroy subprocess (automatic called by controller).
		virtual ~SubProcess();

		/// @brief Called on subprocess stdout.
		virtual void onStdOut(const char *line);

		/// @brief Called on subprocess stderr.
		virtual void onStdErr(const char *line);

		/// @brief Called on subprocess normal exit.
		virtual void onTerminate(int rc);

		/// @brief Called on subprocess abnormal exit.
		virtual void onSignal(int sig);

	public:
		SubProcess(const SubProcess &) = delete;
		SubProcess(const SubProcess *) = delete;

		/// @brief Create a sub-process (Allways use with 'new' since the controller keeps a reference to it).
		SubProcess(const char *command);

		/// @brief Start sub process; this object will be removed when it exits.
		void start();

	};

 }
