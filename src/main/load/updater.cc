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
 #include <private/updater.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/module.h>
 #include <iostream>
 #include <udjat/tools/http/client.h>
 #include <private/misc.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <private/logger.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file.h>

 using namespace std;

 namespace Udjat {

	Updater::Updater(const char *pathname, bool force) : update{force} {

		if(pathname && *pathname) {

			// Has pathname, use it.
			Logger::String{"Loading xml definitions from '",pathname,"'"}.trace("settings");
			File::Path{pathname}.for_each("*.xml",[this](const File::Path &path){
				push_back(path);
				return false;
			});

		} else {

			// No pathname, use the default one.
			Config::Value<string> config("application","definitions","");
			if(config.empty()) {

				// No configuration, scan standard paths.
				std::string options[] = {
					string{Application::DataDir{nullptr,false} + "settings.xml" },
					Application::DataDir{"xml.d",false},
#ifndef _WIN32
					string{ string{"/etc/"} + Application::name() + ".xml" },
					string{ string{"/etc/"} + Application::name() + ".xml.d" },
#endif // _WIN32
				};

				for(size_t ix=0;ix < (sizeof(options)/sizeof(options[0]));ix++) {

					File::Path path{options[ix].c_str()};

					if(path) {
						Logger::String{"Loading xml definitions from '",path.c_str(),"'"}.trace("settings");
						path.for_each("*.xml",[this](const File::Path &path){
							push_back(path);
							return false;
						});
					}
#ifdef DEBUG
					else {
						debug("Cant find ",options[ix].c_str());
					}
#endif // DEBUG

				}

			} else {

				// Scan only the configured path.
				Logger::String{"Loading xml definitions from '",config.c_str(),"'"}.trace("settings");
				File::Path{config.c_str()}.for_each("*.xml",[this](const File::Path &path){
					push_back(path);
					return false;
				});

			}

		}

		// Check for extra files.
		{
			File::Path path{Config::Value<std::string>{"paths","xml",""}};
			if(!path.empty()) {

				path.mkdir();

				info() << "Loading extended definitions from '" << path << "'" << endl;

				path.for_each("*.xml",[this](const File::Path &path){
					push_back(path);
					return false;
				});

			}
		}

	}

	bool Updater::refresh() {

		size_t changed = 0;
		size_t loaded = 0;

		Logger::String{"Checking ",size()," setup file(s) for update"}.write(Logger::Trace,name().c_str());
		for(std::string &filename : *this) {

			debug("Checking '",filename,"'");

			try {

				pugi::xml_document doc;
				auto result = doc.load_file(filename.c_str());
				if(result.status != pugi::status_ok) {
					warning() << filename << ": " << result.description() << endl;
					continue;
				}

				loaded++;
				auto node = doc.document_element();

				/// Setup logger.
				Logger::setup(node);

				// Check for modules.
				for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
					if(node.attribute("preload").as_bool(false)) {
						Module::load(node);
					}
				}

				// Check for update timer.
				const char *url = node.attribute("src").as_string();
				if(url && *url) {

					time_t refresh = node.attribute("update-timer").as_uint(0);

					info() << "Updating " << filename << endl;

					try {

						if(HTTP::Client::save(node,filename.c_str())) {
							changed++;
						}

					} catch(const std::exception &e) {

						error() << "Error '" << e.what() << "' updating " << filename << endl;
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

			} catch(const std::exception &e) {

				error() << filename << ": " << e.what() << endl;

			}

		}

		if(!loaded) {
			next = Config::Value<time_t>("application","update-when-failed",3600);
			error() << "Unable to load xml definitions, setting refresh timer to " << next << " seconds" << endl;
			return false;
		}

		if(changed) {
			Logger::String(changed, " file(s) changed, requesting full update").write(Logger::Trace,name().c_str());
			update = true;
		}

		return update;

	}

	bool Updater::load(std::shared_ptr<Abstract::Agent> root) const noexcept {

		for(const std::string &filename : *this) {

			Logger::String{"Loading '",filename,"'"}.write(Logger::Trace,name().c_str());

			try {

				pugi::xml_document doc;
				auto result = doc.load_file(filename.c_str());
				if(result.status != pugi::status_ok) {
					error() << filename << ": " << result.description() << endl;
					return false;
				}

				auto node = doc.document_element();

				Logger::setup(node);

				const char *path = node.attribute("agent-path").as_string();

				if(path && *path) {

					// Has defined root path, find agent.
					root->find(path,true,true)->setup(node);

				} else {

					// No path, load here.
					root->setup(node);

				}

			} catch(const std::exception &e) {

				error() << filename << ": " << e.what() << endl;
				return false;

			}

		}

		// Activate new root agent.
		Udjat::setRootAgent(root);

		return true;

	}

 }
