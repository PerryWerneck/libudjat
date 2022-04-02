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
 #include <udjat/tools/http/client.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/systemservice.h>
 #include <udjat/tools/configuration.h>
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


	struct Updater {

		bool changed = false;
		time_t next = 0;
		Application::DataFile path;

		Updater(const char *pathname) : path{pathname} {

			// First scan for modules.
			if(Config::Value<bool>("modules","preload-from-xml",true)) {
				cout << "modules\tPreloading from " << path << endl;
				loader(path.c_str(),[](const char UDJAT_UNUSED(*filename), const pugi::xml_document &doc){
					for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
						Module::load(node);
					}
				});
			}

			// Then check for file updates.
			loader(path.c_str(),[this](const char *filename, const pugi::xml_document &doc){

					auto node = doc.document_element();

					const char *url = node.attribute("src").as_string();
					if(url && *url) {

						time_t refresh = node.attribute("update-timer").as_uint(0);

						try {

							Application::info() << "Updating " << filename << endl;
							if(HTTP::Client::save(node,filename)) {
								changed = true;
							}

						} catch(const std::exception &e) {

							Application::error() << "Error '" << e.what() << "' updating " << filename << endl;
							refresh = node.attribute("update-when-failed").as_uint(refresh);

						} catch(...) {

							Application::error() << "Unexpected error updating " << filename << endl;
							refresh = node.attribute("update-when-failed").as_uint(refresh);

						}

						if(refresh) {
							if(next) {
								next = std::min(next,refresh);
							} else {
								next = refresh;
							}
						}

					}

				});

		}


		/// @brief Update agent, set it as a new root.
		void set(std::shared_ptr<Abstract::Agent> agent, const char *pathname) const noexcept {

			loader(pathname,[agent](const char *filename, const pugi::xml_document &doc){

				agent->info() << "Loading '" << filename << "'" << endl;

				// First setup agent, load modules, etc.
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

				// Then setup modules.
				Module::for_each([&doc](Module &module) {

					try {

						module.set(doc);

					} catch(const std::exception &e) {

						cerr << "modules\tError '" << e.what() << "' on module setup" << endl;

					} catch(...) {

						cerr << "modules\tUnexpected error on module setup" << endl;

					}

				});

			});

		}

	};

	UDJAT_API time_t reconfigure(const char *pathname, bool force) {

		Updater update(pathname);
		if(!(update.changed || force)) {
			Application::info() << "No changes, reconfiguration is not necessary" << endl;
			return update.next;
		}

		Application::warning() << "Reconfiguring from '" << update.path << "'" << endl;

		auto agent = RootAgentFactory();
		update.set(agent,update.path.c_str());
		setRootAgent(agent);

		return update.next;

	}

	UDJAT_API time_t reconfigure(std::shared_ptr<Abstract::Agent> agent, const char *pathname, bool force) {

		Updater update(pathname);
		if(!(update.changed || force)) {
			Application::info() << "No changes, reconfiguration is not necessary" << endl;
			return update.next;
		}

		Application::warning() << "Reconfiguring from '" << update.path << "'" << endl;

		update.set(agent,update.path.c_str());
		setRootAgent(agent);

		return update.next;

	}

	void SystemService::reconfigure(const char *pathname, bool force) noexcept {

		try {

			Updater update(pathname);

			if(!(update.changed || force)) {

				info() << "No changes, reconfiguration is not necessary" << endl;

			} else {

				auto agent = RootFactory();
				update.set(agent,update.path.c_str());
				setRootAgent(agent);

			}

			if(update.next) {

				Udjat::MainLoop::getInstance().insert(definitions,update.next*1000,[]{
					ThreadPool::getInstance().push([]{
						if(instance) {
							instance->reconfigure(instance->definitions,false);
						}
					});
					return false;
				});

				Application::info() << "Next reconfiguration set to " << TimeStamp(time(0)+update.next) << endl;

			}

		} catch(const std::exception &e ) {

			Application::error() << "Reconfiguration has failed: " << e.what() << endl;

		} catch(...) {

			Application::error() << "Unexpected error during reconfiguration" << endl;

		}

	}


 }
