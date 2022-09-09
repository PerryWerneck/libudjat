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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/protocol.h>
 #include <iostream>
 #include <list>
 #include <mutex>
 #include <udjat/tools/logger.h>
 #include <udjat/factory.h>
 #include <udjat/request.h>

 using namespace std;

 namespace Udjat {

	class Protocol::Controller {
	private:
		static mutex guard;
		list<Protocol *> protocols;
		list<Protocol::Worker *> workers;

		Controller();

		/// @brief Internal protocol for file://
		class File : public Udjat::Protocol {
		public:
			File();
			virtual ~File();

			String call(const URL &url, const HTTP::Method method, const char *payload = "") const override;

			std::shared_ptr<Protocol::Worker> WorkerFactory() const;

		};

		/// @brief Internal protocol for script://
		class Script : public Udjat::Protocol {
		public:
			Script();
			virtual ~Script();

			std::shared_ptr<Protocol::Worker> WorkerFactory() const;

		};

	public:
		static Controller & getInstance();
		~Controller();

		void insert(Protocol *protocol);
		void remove(Protocol *protocol);

		void insert(Protocol::Worker *worker);
		void remove(Protocol::Worker *worker);

		const Protocol * find(const char *name);
		void getInfo(Response &response) noexcept;

	};

 }
