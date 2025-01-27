/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/xml.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	/// @brief Load XML file, check if it's valid.
 	static void load(XML::Document *document, const char *filename) {

		auto result = document->load_file(filename);
		if(result.status != pugi::status_ok) {
			throw runtime_error(Logger::String{filename,": ",result.description()});
		}

		Config::Value<string> tagname{"xml","tagname",Application::Name().c_str()};
		Config::Value<bool> allow_unsafe{"xml","allow-unsafe-updates",false};

		bool safe = (strcasecmp(document->document_element().name(),tagname.c_str()) == 0);

		if(!(safe || allow_unsafe)) {
			throw runtime_error(
				Logger::String {
					"The first node on document is <",document->document_element().name(),">, expecting <",tagname.c_str(),">"
				}
			);
		}

 	}

	XML::Document::Document(const char *filename) {

		Logger::String{"Loading '",filename,"'"}.info("xml");

		Udjat::load(this,filename);

		// TODO: Check file's last write and and update-timer attribute to see if an url check is required.

		URL url{document_element()};
		url.expand();

		if(!url.empty()) {

			bool updated = url.handler()->set(MimeType::xml).get(filename);

			if(updated) {
				Logger::String{filename," was updated from ",url.c_str()}.info("xml");
				reset();
				Udjat::load(this,filename);
			}

			document_element().append_attribute("updated").set_value(updated);

		}

		// TODO: Parse <include> nodes.

	}


 }


