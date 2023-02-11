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
 #include <sys/types.h>
 #include <udjat/defs.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/handler.h>
 #include <private/event.h>
 #include <mutex>
 #include <list>
 #include <mutex>
 #include <memory>

 using namespace std;

 namespace Udjat {

	class UDJAT_PRIVATE SubProcess::Handler : public MainLoop::Handler {
	private:
		size_t length = 0;
		char buffer[256];

	protected:
		unsigned short id;

		void handle_event(const Event event) override;
		void parse();

		virtual void on_error(const char *reason) = 0;
		virtual void on_input(const char *line) = 0;

	public:
		Handler(unsigned short i) : MainLoop::Handler(-1, (Event) (oninput|onerror|onhangup)), id(i) {
		}

	};

	class UDJAT_PRIVATE SubProcess::Controller {
	public:

		struct Entry {
			std::shared_ptr<SubProcess> proc;				///< @brief The process object.
			std::shared_ptr<SubProcess::Handler> out;		///< @brief The output stream.
			std::shared_ptr<SubProcess::Handler> err;		///< @brief The error stream.
		};

	private:
		Controller();

		static mutex guard;

		list<Entry> entries;

		static void handle_signal(int sig) noexcept;

		void child_ended(pid_t pid, int status) noexcept;

	public:

		~Controller();
		static Controller & getInstance();

		/// @brief Initialize subprocess.
		static void init(SubProcess &proc, Handler &out, Handler &err);

		void push_back(Entry &entry);

 	};


 }
