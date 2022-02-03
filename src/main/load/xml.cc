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

/**
 * @file main/load/xml.cc
 *
 * @brief Implements the application settings loader.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <udjat-internals.h>
 #include <sys/stat.h>
 #include <udjat/module.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

using namespace std;

namespace Udjat {

	/// @brief Load modules from node definition.
	static void load_modules(const pugi::xml_node &root) {
		for(pugi::xml_node node = root.child("module"); node; node = node.next_sibling("module")) {
			Module::load(node.attribute("name").as_string(),node.attribute("required").as_bool(true));
		}
	}

	static void load(std::shared_ptr<Abstract::Agent> root, const pugi::xml_node &node) {

		const char *path = node.attribute("root-path").as_string();

		if(path && *path) {

			// Has defined root path, find agent.
			root->find(path,true,true)->load(node);

		} else {

			// No path, load here.
			root->load(node);

		}

	}

	UDJAT_API std::shared_ptr<Abstract::Agent> load(const char *pathname) {

		struct stat pathstat;
		if(stat(pathname, &pathstat) == -1) {
			throw system_error(errno,system_category(),Logger::Message("Can't load '{}'",pathname));
		}

		/// @brief The new root agent.
		std::shared_ptr<Abstract::Agent> root;

		if((pathstat.st_mode & S_IFMT) == S_IFDIR) {
			//
			// Load all XML files from pathname
			//
			File::List files((string{pathname} + "/*.xml").c_str());

			// First load modules.
			files.forEach([](const char *filename){
				pugi::xml_document doc;
				doc.load_file(filename);
				load_modules(doc.document_element());
			});

			// Then load agents
			clog << STRINGIZE_VALUE_OF(PRODUCT_NAME) << "\tCreating the new root agent" << endl;
			root = getDefaultRootAgent();

			files.forEach([root](const char *filename){
				pugi::xml_document doc;
				doc.load_file(filename);
				load(root, doc.document_element());
			});

		} else {

			//
			// Load a single XML file.
			//
			cout << STRINGIZE_VALUE_OF(PRODUCT_NAME) << "\tLoading '" << pathname << "'" << endl;

			// First load the modules.
			{
				pugi::xml_document doc;
				doc.load_file(pathname);
				load_modules(doc.document_element());
			}



			// Create new root agent.
			{
				pugi::xml_document doc;
				doc.load_file(pathname);

				root = getDefaultRootAgent();

				// Then load agents
				load(root, doc.document_element());
			}

		}

		setRootAgent(root);

		return root;
	}

	/*
	void Abstract::Agent::load(const pugi::xml_node &root, bool name) {

		// Translate method
		auto translate = [root](const char *key) {
			return (const char *) root.attribute(key).as_string();
		};

		//bool upsearch = root.attribute("upsearch").as_bool(true);

		// Load my attributes
		struct Attr {
			/// @brief The attribute name.
			const char *name;
			const char **value;
		} attributes[] = {
			{ "summary",	&this->summary	},
			{ "label",		&this->label	},
			{ "icon",		&this->icon		},
			{ "uri",		&this->uri		}
		};

		for(size_t ix = 0; ix < (sizeof(attributes)/sizeof(attributes[0])); ix++) {
			const char * value = Quark().set(root,attributes[ix].name,false,translate).c_str();
			if(value && *value) {
				*attributes[ix].value = value;
			}
		}

		if(name) {
			Logger::set(root);
		}

		this->update.timer = root.attribute("update-timer").as_uint(this->update.timer);
		this->update.on_demand = root.attribute("update-on-demand").as_bool(this->update.timer == 0);

		time_t delay = root.attribute("delay-on-startup").as_uint(0);
		if(delay)
			this->update.next = time(nullptr) + delay;

		// Load children
		for(pugi::xml_node node : root) {

			// Skip reserved names.
			if(!strcasecmp(node.name(),"attribute")) {
				continue;
			}

			// Check for module tag.
			if(!strcasecmp(node.name(),"module")) {
#ifdef DEBUG
				cout << "\tChecking for module '" << node.attribute("name").as_string() << "'" << endl;
#endif // DEBUG
				Module::load(node.attribute("name").as_string(),node.attribute("required").as_bool(true));

				continue;
			}

			// Process factory methods.
			try {

				Factory::parse(node.name(), *this, node);

			} catch(const std::exception &e) {

				error("Error '{}' loading node '{}'",e.what(),node.name());

			} catch(...) {

				error("Unexpected error loading node '{}'",node.name());

			}

		}

	}

	void Abstract::Agent::load(const pugi::xml_document &doc) {

		for(pugi::xml_node root = doc.child("config"); root; root = root.next_sibling("config")) {

			const char *path = root.attribute("root-path").as_string();

			if(path && *path) {

				// Has defined root path, find agent.
				Abstract::Agent * agent = this;
				while(agent->parent) {
					agent = agent->parent;
				}

				agent->find(path,true,true)->load(root);

			} else {

				// No path, load here.
				load(root);

			}

		}

	}
	*/

}
