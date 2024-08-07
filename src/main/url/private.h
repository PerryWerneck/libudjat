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
 #include <udjat/url.h>
 #include <iostream>
 #include <memory>
 #include <list>

 using namespace std;

 namespace Udjat {

	class URL::Controller {
		Controller();

		class FileProtocol : public URL::Protocol {
		public:

			FileProtocol();

			virtual ~FileProtocol();
			virtual std::shared_ptr<URL::Response> call(const URL &url, const Method method, const char *mimetype, const char *payload) override;

		};

		list<shared_ptr<Protocol>> protocols;

	public:
		~Controller();
		static Controller & getInstance();

		void getInfo(Udjat::Response &response) noexcept;
		void insert(std::shared_ptr<Protocol> protocol);

		shared_ptr<Protocol> find(const char *name);

	};

 }
