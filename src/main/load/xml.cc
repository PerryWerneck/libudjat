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
 #include <udjat/tools/application.h>
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

		const char *path = node.attribute("path").as_string();

		if(path && *path) {

			// Has defined root path, find agent.
			root->find(path,true,true)->load(node);

		} else {

			// No path, load here.
			root->load(node);

		}

	}

	UDJAT_API std::shared_ptr<Abstract::Agent> load(const char *pathname) {

		Application::Name application;

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
			clog << application << "\tCreating the new root agent" << endl;
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
			cout << application << "\tLoading '" << pathname << "'" << endl;

			// List of file to update
			string url;

			// First load the modules and check for update url.
			{
				pugi::xml_document doc;
				doc.load_file(pathname);
				url = doc.document_element().attribute("src").as_string();
				load_modules(doc.document_element());
			}

			if(!url.empty()) {

				// The file has an update url, use it.

				try {

					if(URL(url.c_str()).get(pathname)) {
						cout << application << "\tFile '" << pathname << "' was updated" << endl;

						// TODO: Reload modules.

					}

				} catch(const exception &e) {

					cerr << application << "\t" << e.what() << endl;
				}

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

 }
