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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/http/client.h>
 #include <string>

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		Client::Client(const char *u) {

			URL url{u};

			const Protocol * protocol = Protocol::find(u);
			if(!protocol) {
				throw runtime_error(string{"Unable to handle "} + u);
			}

			worker = protocol->WorkerFactory();
			if(!worker) {
				throw runtime_error(string{"Unable to find a worker for "} + u);
			}

			worker->url(url);

		}

		String Client::get(const std::function<bool(double current, double total)> &progress) {
			// worker->payload(payload.str());
			worker->method(Get);
			return worker->get(progress);
		}

		String Client::get() {
			return get([](double UDJAT_UNUSED(current),double UDJAT_UNUSED(total)){return true;});
		}

		bool Client::save(const char *filename, const std::function<bool(double current, double total)> &progress) {
			// worker->payload(payload.str());
			worker->method(Get);
			return worker->save(filename,progress);
		}

		bool Client::save(const char *filename) {
			return save(filename,[](double UDJAT_UNUSED(current),double UDJAT_UNUSED(total)){return true;});
		}

	}

 }
