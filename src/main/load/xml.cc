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
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/threadpool.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 using namespace std;

 namespace Udjat {

	/// @brief Load modules from xml file.
	static bool prepare(const char *filename, bool download) {

		string url;
		Application::Name appname;

		{
			pugi::xml_document doc;
			auto result = doc.load_file(filename);
			if(result.status != pugi::status_ok) {
				cerr << appname << "\tError parsing '" << filename << "' (" << result.description() << "'" << endl;
				return false;
			}

			// Load modules.
			for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
				Module::load(node);
//				Module::load(node.attribute("name").as_string(),node.attribute("required").as_bool(false));
			}
			url = doc.document_element().attribute("src").as_string();
		}

		if(url.empty() || !download) {
			return true;
		}

		// Check for file update.
		try {

			if(URL(url.c_str()).get(filename)) {
				cout << appname << "\tFile '" << filename << "' was updated, reloading modules" << endl;

				// Reload modules.
				pugi::xml_document doc;
				auto result = doc.load_file(filename);
				if(result.status != pugi::status_ok) {
					cerr << appname << "\tError parsing '" << filename << "' (" << result.description() << "'" << endl;
					return false;
				}

				// Reload modules.
				for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
					Module::load(node.attribute("name").as_string(),node.attribute("required").as_bool(false));
				}

			}

		} catch(const exception &e) {

			cerr << appname << "\t" << e.what() << endl;

		}

		return true;

	}

	static time_t load(std::shared_ptr<Abstract::Agent> root, const char *filename) {

		Application::Name appname;

		pugi::xml_document doc;
		auto result = doc.load_file(filename);
		if(result.status != pugi::status_ok) {
			cerr << appname << "\tError parsing '" << filename << "' (" << result.description() << "'" << endl;
			return 0;
		}

		auto node = doc.document_element();

		const char *path = node.attribute("path").as_string();

		if(path && *path) {

			// Has defined root path, find agent.
			root->find(path,true,true)->load(node);

		} else {

			// No path, load here.
			root->load(node);

		}

		return node.attribute("update-timer").as_uint(0);

	}

	/// @brief Activate root agent from application definitions.
	/// @param pathname Path to a single xml file or a folder with xml files.
	static time_t activate(const char *pathname) {
		auto root = RootAgentFactory();
		time_t timer = load(root,pathname);
		setRootAgent(root);
		return timer;
	}

	UDJAT_API void load(const char *pathname) {

		Application::Name application;

		struct stat pathstat;
		if(stat(pathname, &pathstat) == -1) {
			throw system_error(errno,system_category(),Logger::Message("Can't load '{}'",pathname));
		}

		if((pathstat.st_mode & S_IFMT) == S_IFDIR) {
			//
			// Load all XML files from pathname
			//
			File::List files((string{pathname} + "/*.xml").c_str());

			// First load modules.
			files.forEach([](const char *filename){
				prepare(filename,false);
			});

			// Then load agents
			clog << application << "\tCreating the new root agent" << endl;
			auto root = RootAgentFactory();

			files.forEach([root](const char *filename){
				load(root, filename);
			});

			setRootAgent(root);

		} else {

			//
			// Load a single XML file.
			//
			cout << application << "\tLoading '" << pathname << "'" << endl;

			// First load the modules and check for update url.
			if(!prepare(pathname,true)) {
				throw runtime_error("Can't prepare definition file");
			}

			// Create new root agent.
			{

				// Then load agents
				time_t timer = activate(pathname);
				if(timer) {

					clog << Application::Name() << "\tReconfiguration scheduled to " << TimeStamp(time(0)+timer) << endl;

					const char *name = Quark(pathname).c_str();
					MainLoop::getInstance().insert(name,timer*1000L,[name](){
						//
						// Reconfigure time, do we need reload?
						//
						ThreadPool::getInstance().push([name]{

							Application::Name appname;
							try {

								string url;

								{
									pugi::xml_document doc;
									auto result = doc.load_file(name);
									if(result.status != pugi::status_ok) {
										cerr << appname << "\tError parsing '" << name << "' (" << result.description() << "'" << endl;
										return;
									}
									url = doc.document_element().attribute("src").as_string();
								}

								if(url.empty()) {
									clog << appname << "\tUpdating from '" << name << "' was disabled" << endl;
									MainLoop::getInstance().remove(name);
									return;
								}

								if(!URL(url.c_str()).get(name)) {
									cout << appname << "\tFile '" << name << "' was not updated, will keep current settings" << endl;
									return;
								}

								if(prepare(name,false)) {

									// Then load agents
									time_t timer = activate(name);

									if(!timer) {
										clog << appname << "\tUpdating from '" << name << "' was disabled" << endl;
										MainLoop::getInstance().remove(name);
									} else {
										clog << appname << "\tNext update of '" << name << "' will be on " << TimeStamp(time(0) + timer) << endl;
										// MainLoop::getInstance().reset(name,timer*1000L);
									}

								}

							} catch(const exception &e) {

								cerr << appname << "\tError '" << e.what() << "' reloading '" << name << "'" << endl;

							} catch(...) {

								cerr << appname << "\tUnexpected error reloading '" << name << "'" << endl;

							}

						});
						return true;
					});
				}


			}

		}

	}

 }
