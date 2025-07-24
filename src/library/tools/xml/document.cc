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
 #include <udjat/defs.h>

 #ifdef LOG_DOMAIN
	#undef LOG_DOMAIN
 #endif
 #define LOG_DOMAIN "xml"
 #include <udjat/tools/logger.h>

 #include <udjat/tools/xml.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/url/handler.h>
 #include <stdexcept>
 #include <private/logger.h>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/container.h>

 using namespace std;

 namespace Udjat {

	static Container<XML::Parser> & Factories() {
		static Container<XML::Parser> instance;
		return instance;
	}

	XML::Parser::Parser(const char *n) : parser_name{n} {
		Factories().push_back(this);
	}

	XML::Parser::~Parser() {
		Factories().remove(this);
	}

	/// @brief Load XML file, check if it's valid.
 	static void load(XML::Document *document, const char *filename) {

		auto result = document->load_file(filename);
		if(result.status != pugi::status_ok) {
			throw runtime_error(Logger::String{filename,": ",result.description()});
		}

		Config::Value<string> tagname{"xml","tagname",Application::Name().c_str()};
		Config::Value<bool> allow_unsafe{"xml","allow-unsafe-updates",true};

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

		Udjat::load(this,filename);

		// Preload
		{
			auto root = document_element();
			Logger::setup(root);
			for(const XML::Node &node : root) {
				if(node.attribute("preload").as_bool(false)) {
					Logger::String{"Preloading module '",node.attribute("name").as_string(),"'"}.trace();
					XML::parse(node);
				}
			}

		}

		/*
		// Preload modules
		for(XML::Node child = document_element().child("module"); child; child = child.next_sibling("module")) {
			if(child.attribute("preload").as_bool(false)) {
				Logger::String{"Preloading module '",child.attribute("name").as_string(),"'"}.trace();
				Module::load(child);
			}		
		}
		*/

		// Check for update.
		const XML::Node &node = document_element();
		URL url{node};
		url.expand();

		if(!url.empty() && File::outdated(filename,TimeStamp{node,"update-timer"})) {

			try {
				
				bool updated = url.handler()->set(MimeType::xml).get(filename);

				if(updated) {
					Logger::String{filename," was updated from ",url.c_str()}.info("xml");
					reset();
					Udjat::load(this,filename);
					File::mtime(filename,time(0)); // Mark file as updated.
				}

			} catch(const std::exception &e) {

				Logger::String{"Error updating '",filename,"' from '",url.c_str(),"' - ",e.what()}.warning("xml");

			}

		}

	}

	time_t XML::Document::parse() const {

		auto root = document_element();
		Logger::setup(root);

		for(const XML::Node &node : root) {
			if(!node.attribute("preload").as_bool(false)) {
				XML::parse(node);
			}
		}

		time_t next = TimeStamp{root,"update-timer"};
		if(next) {
			next += time(0);
		}

		return next;
	}

	XML::Node & XML::Document::copy_to(XML::Node &node) const {
		for(auto &child : document_element()) {
			node.append_copy(child);			
		}
		return node;
	}

	void XML::parse_children(const XML::Node &node) {
		for(const auto &child : node) {
			if(XML::parse(child)) {
				parse_children(child); // Child was handled, parse its children.
			}
		}	
	}

	bool XML::parse(const XML::Node &node) {

		// It's an attribute?
		if(is_reserved(node) || !is_allowed(node)) {
			return true; // Ignore reserved nodes.
		}

		const char *name = node.name();
		if(!strcasecmp(name,"module")) {

			if(node.attribute("preload").as_bool(false)) {
				return true; // Preload modules are handled by Document constructor.
			}

			Logger::String{"Loading module '",node.attribute("name").as_string(),"'"}.trace();
			Module::load(node);
			return true;

		}
	
		for(const auto factory : Factories()) {
			if(*factory == name) {
				return factory->parse(node); // Handled by factory.
			}
		}

		return false; // Not handled.

	}

 }


