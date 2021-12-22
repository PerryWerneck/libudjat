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
 #include <udjat/tools/value.h>

 #ifdef HAVE_WINHTTP
	#include <udjat/tools/http.h>
 #endif // HAVE_WINHTTP

 namespace Udjat {

	URL::Controller::Controller() {

		insert(make_shared<FileProtocol>());

#ifdef HAVE_WINHTTP

	static const ModuleInfo winhttpinfo {
		PACKAGE_NAME,									// The module name.
		"WinHTTP protocol module",	 					// The module description.
		PACKAGE_VERSION, 								// The module version.
		PACKAGE_URL, 									// The package URL.
		PACKAGE_BUGREPORT 								// The bugreport address.
	};

	class WinHttpProtocol : public URL::Protocol {
	public:

		WinHttpProtocol() : URL::Protocol{"http", "", &winhttpinfo} {
		}

		~WinHttpProtocol() {
		}

		std::shared_ptr<URL::Response> call(const URL &url, const Method method, const char *mimetype, const char *payload) override {

			class Response : public URL::Response {
			private:
				std::string text;

			public:
				Response() = default;

				void set(const std::string &text) {
					this->text = text;
					this->status.code = 0;
					this->status.text = "Success";
					this->response.payload = text.c_str();
					this->response.length = text.size();
				}

			};

			std::shared_ptr<Response> response = make_shared<Response>();

			response->call([response,method,url,payload]() {

				HTTP::Client client(url.to_string().c_str());

				switch(method) {
				case URL::Method::Get:
					response->set(client.get());
					break;

				case URL::Method::Post:
					response->set(client.post(payload));
					break;

				default:
					throw system_error(ENOTSUP,system_category(),"Unsupported http method");

				}

			});

			return response;

		}

	};

	insert(make_shared<WinHttpProtocol>());

#endif // HAVE_WINHTTP

	}

	URL::Controller::~Controller() {

	}

	URL::Controller & URL::Controller::getInstance() {
		static Controller instance;
		return instance;
	}

	void URL::Controller::insert(std::shared_ptr<Protocol> protocol) {
		protocols.push_back(protocol);
	}

	void URL::Controller::getInfo(Udjat::Response &response) noexcept {

		response.reset(Value::Array);

		for(auto protocol : this->protocols) {

			Value &object = response.append(Value::Object);

			object["id"] = protocol->c_str();
			object["portname"] = protocol->getDefaultPortName();
			protocol->getModuleInfo()->get(object);

		}

	}

	shared_ptr<URL::Protocol> URL::Controller::find(const char *name) {

		for(auto protocol : protocols) {
			if(!strcmp(name,protocol->c_str())) {
				return protocol;
			}
		}

		throw runtime_error(Logger::Message("No available protocol manager for '{}://'",name).c_str());

	}


 }
