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
 #include <udjat/module.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/threadpool.h>
 #include <list>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 using namespace std;

 namespace Udjat {

	static void loader(const char *pathname, const std::function<void(const char *filename, const pugi::xml_document &document)> &call) {

		struct stat pathstat;
		if(stat(pathname, &pathstat) == -1) {
			throw system_error(errno,system_category(),Logger::Message("Can't load '{}'",pathname));
		}

		if((pathstat.st_mode & S_IFMT) == S_IFDIR) {
			//
			// It's a folder.
			//
			File::List((string{pathname} + "/*.xml").c_str()).forEach([&call,&pathname](const char *filename){

				try {

					pugi::xml_document doc;
					auto result = doc.load_file(filename);
					if(result.status != pugi::status_ok) {
						Application::error() << filename << ": " << result.description() << endl;
						return;
					}

					call(filename,doc);

				} catch(const std::exception &e) {

					Application::error() << pathname << ": " << e.what() << endl;

				} catch(...) {

					Application::error() << pathname << ": Unexpected error" << endl;

				}

			});

		} else {
			//
			// It's a single file.
			//
			try {

				pugi::xml_document doc;
				auto result = doc.load_file(pathname);
				if(result.status != pugi::status_ok) {
					Application::error() << pathname << ": " << result.description() << endl;
					return;
				}

				call(pathname,doc);

			} catch(const std::exception &e) {

				Application::error() << pathname << ": " << e.what() << endl;

			} catch(...) {

				Application::error() << pathname << ": Unexpected error" << endl;

			}
		}
	}

	void load_modules(const char *pathname) {

		loader(pathname,[](const char UDJAT_UNUSED(*filename), const pugi::xml_document &doc){
			for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
				Module::load(node);
			}
		});

	}

	UDJAT_PRIVATE time_t refresh_definitions(const char *pathname) {

		time_t next = 0;

		struct FileDefinition {
			std::string filename;
			std::string url;

			FileDefinition(const char *f, const char *u) : filename(f), url(u) {
			}

		};

		std::list<FileDefinition> definitions;

		loader(pathname,[&next,&definitions](const char *filename, const pugi::xml_document &doc){

			auto node = doc.document_element();

			const char *url = node.attribute("src").as_string();
			if(url && *url) {

				time_t refresh = node.attribute("update-timer").as_uint(0);
				if(refresh) {
					if(next) {
						next = std::min(next,refresh);
					} else {
						next = refresh;
					}

					// TODO: Check for the file timestamps to see if an update is required.
					definitions.emplace_back(filename,url);

				} else {
					definitions.emplace_back(filename,url);
				}

			}

		});

		// Update file(s)
		if(!definitions.empty()) {
			Application::info() << "Updating configuration files" << endl;
			for(auto definition : definitions) {
				try {
					Application::info() << "Updating from '" << definition.url << "'" << endl;
					URL(definition.url).get(definition.filename.c_str());
				} catch(const std::exception &e) {
					Application::error() << e.what() << endl;
				} catch(...) {
					Application::error() << "Unexpected error getting " << definition.url << endl;
				}
			}
			Application::info() << "Updating complete" << endl;
		}

		return next;
	}

	void load_agent_definitions(std::shared_ptr<Abstract::Agent> agent, const char *pathname) {

		loader(pathname,[agent](const char *filename, const pugi::xml_document &doc){

			agent->info() << "Loading '" << filename << "'" << endl;

			try {

				auto node = doc.document_element();
				const char *path = node.attribute("path").as_string();

				if(path && *path) {

					// Has defined root path, find agent.
					agent->find(path,true,true)->load(node);

				} else {

					// No path, load here.
					agent->load(node);

				}

			} catch(const std::exception &e) {

				agent->error() << filename << ": " << e.what() << endl;

			} catch(...) {

				agent->error() << filename << ": Unexpected error" << endl;

			}
		});

	}

	UDJAT_API time_t load(std::shared_ptr<Abstract::Agent> agent, const char *pathname) {

		Udjat::load_modules(pathname);
		time_t next = Udjat::refresh_definitions(pathname);
		load_agent_definitions(agent,pathname);
		setRootAgent(agent);
		return next;
	}

	UDJAT_API time_t load(const char *pathname) {
		Udjat::load_modules(pathname);
		time_t next = Udjat::refresh_definitions(pathname);
		auto agent = RootAgentFactory();
		load_agent_definitions(agent,pathname);
		setRootAgent(agent);
		return next;
	}

 }
