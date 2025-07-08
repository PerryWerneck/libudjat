/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/url.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/value.h>
 #include <errno.h>
 #include <private/url.h>

 #ifdef HAVE_SMBIOS
 	
 #include <smbios/node.h>
 #include <smbios/value.h>

 namespace Udjat {

	SMBiosURLHandler::SMBiosURLHandler(const URL &u) : url{u} {
	
	}

	const char * SMBiosURLHandler::c_str() const noexcept {
		return url.c_str();
	}

	bool SMBiosURLHandler::get(Udjat::Value &response, const HTTP::Method, const char *) {

		auto elements = url.path(true).split("/");

		switch(elements.size()) {
		case 0: // Empty
			throw std::invalid_argument{"SMBiosURLHandler: Invalid URL path, must contain 'type' or 'type/entry'."};

		case 1: // Only node, enumerate all entries
			{
				debug("Enumerating SMBios node: ", elements[0].c_str());
				response.clear(Value::Array);

				SMBios::Node::for_each([&response,&elements](const SMBios::Node &node) {

					if(strcasecmp(node.name(), elements[0].c_str()) != 0) {
						return false; // Keep iterating
					}

					debug("Found SMBios node: ", node.name(), " (", node.type(), ")");

					auto &item = response.append(Value::Object);
					node.for_each([&item](const SMBios::Value &v) -> bool {
						auto &row = item[v.name()];
						row["description"] = v.description();
						row["value"] = v.as_string();
						return false;
					});

					return false; // Keep iterating
				});

			}
			break;

		case 3: // Node and entry, get specific entry
			break;

		default:
			throw std::invalid_argument{"SMBiosURLHandler: Invalid URL path, must contain 'type' or 'type/entry'."};
		}

		return true;

	} 

	URL::Handler & SMBiosURLHandler::set(const MimeType mimetype) {
		this->mimetype = mimetype;
		return *this;
	}

	int SMBiosURLHandler::perform(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total, const void *data, size_t len)> &progress) {

		try {

			Value response;
			if(!get(response, method, payload)) {
				return 404;
			}

			auto text = response.to_string(mimetype);
			debug("Response text: ", text.c_str());

			if(progress(0, text.size(), text.c_str(), text.size())) {
				return ECANCELED;
			}

			return 200;

		} catch(const std::exception &e) {

			Logger::String{url.c_str(),": ",e.what()}.error();

		} catch(...) {

			Logger::String{url.c_str(),": Unexpected error"}.error();

		}

		return 500;

	}

	int SMBiosURLHandler::test(const HTTP::Method method, const char *) {

		try {

			// TODO: check if the URL is valid and can be accessed.

			return 200;

		} catch(const std::exception &e) {

			Logger::String{url.c_str(),": ",e.what()}.error();

		} catch(...) {

			Logger::String{url.c_str(),": Unexpected error"}.error();

		}

		return 500;

	}

 }

 #endif // HAVE_SMBIOS