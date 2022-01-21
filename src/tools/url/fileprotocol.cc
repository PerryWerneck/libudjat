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
 #include "private.h"
 #include <udjat/tools/file.h>

 namespace Udjat {

	/*
	static const ModuleInfo fileprotocolinfo {
		PACKAGE_NAME,									// The module name.
		"File protocol module",	 						// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	URL::Controller::FileProtocol::FileProtocol() : URL::Protocol{"file", "", &fileprotocolinfo} {
	}

	URL::Controller::FileProtocol::~FileProtocol() {
	}

	std::shared_ptr<URL::Response> URL::Controller::FileProtocol::call(const URL &url, const Method method, const char *mimetype, const char *payload) {

		class Response : public URL::Response {
		private:
			string text;

		public:
			Response() = default;

			void set(const char *text) {
				this->text = text;
				this->response.length = this->text.size();
				this->response.payload = this->text.c_str();
			}

		};

		std::shared_ptr<Response> response = make_shared<Response>();

		switch(method) {
		case URL::Method::Get:	// Get method, load the file.
			response->set(File::Text(url.getDomainName()).c_str());
			break;

		case URL::Method::Put:	// Put method, save the payload.
		case URL::Method::Post:	// Post method, save the payload.
			if(!payload) {
				throw runtime_error("PUT and POST methods requires a payload");
			}

			throw system_error(ENOTSUP,system_category(),"PUT and POST methods for file:// are not implemented (yet)");

			break;

		case URL::Method::Head:
		case URL::Method::Delete:
		case URL::Method::Connect:
		case URL::Method::Options:
		case URL::Method::Trace:
		case URL::Method::Patch:
			throw system_error(ENOTSUP,system_category(),"Unsuported method");

		}

		return response;

	}
	*/

 }

