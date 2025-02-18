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
 #include <udjat/tools/application.h>
 #include <udjat/module/abstract.h>
 #include <iostream>
 #include <udjat/tools/url.h>
 #include <udjat/tools/url/handler.h>
 #include <private/misc.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <private/logger.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/file/text.h>
 #include <private/agent.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/timestamp.h>

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
#ifndef _WIN32
					string{ string{"/etc/"} + name + ".xml" },
					string{ string{"/etc/"} + name + ".xml.d" },
#endif // _WIN32
					string{Application::DataDir{nullptr,false} + "settings.xml" },
					Application::DataDir{"xml.d",false},
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

				path.mkdir(true);

				info() << "Loading extended definitions from '" << path << "'" << endl;

				path.for_each("*.xml",[this](const File::Path &path){
					push_back(path);
					return false;
				});

			}
		}

	}

	void Updater::push_back(const std::string &filename) {
		pugi::xml_document doc;
		auto result = doc.load_file(filename.c_str());
		if(result.status != pugi::status_ok) {
			error() << filename << ": " << result.description() << endl;
			return;
		}

		auto node = doc.document_element();

		/// Setup logger.
		Logger::setup(node);

		// Check for modules.
		for(XML::Node child = node.child("module"); child; child = child.next_sibling("module")) {
			if(child.attribute("preload").as_bool(false)) {
				Module::load(child);
			}
		}

		files.emplace_back(filename,node);

	}

	bool Updater::refresh() {
		size_t changed = 0;
		Config::Value<string> xmlname{"settings","tagname",Application::Name().c_str()};
		Config::Value<bool> allow_unsafe{"settings","allow-unsafe-updates",false};

		Logger::String{"Checking ",size()," setup file(s) for update"}.write(Logger::Trace,name.c_str());
		for(const Settings &descr : *this) {

			debug("Checking '",descr.filename,"'");

			try {

				// Check for update timer.
				if(!descr.url.empty()) {

					info() << "Updating " << descr.filename << endl;

					time_t refresh = descr.ifsuccess;
					try {


						auto handler = descr.url.handler();
						handler->set(MimeType::xml);

						try {

							handler->get(descr.filename.c_str());
							File::Text text{descr.filename};

							pugi::xml_document doc;
							auto result = doc.load_string(text.c_str());
							if(result.status == pugi::status_ok) {

								bool safe = (strcasecmp(doc.document_element().name(),xmlname.c_str()) == 0);

								if(safe || allow_unsafe) {

									if(safe) {
										Logger::String{
											"Got valid response from ",handler->c_str()," updating ",descr.filename
										}.trace("xml");
									} else {
										Logger::String {
											"The first node on ",handler->c_str()," is <",doc.document_element().name(),">, expecting <",xmlname.c_str(),">, doing an unsafe update"
										}.warning("xml");
									}

									text.save();

									{
										time_t timestamp = HTTP::TimeStamp{handler->header("Last-Modified")}.as_value();
										if(!timestamp) {
											Logger::String{"No timestamp on ",handler->c_str()}.warning("xml");
										} else if(File::mtime(descr.filename.c_str(),timestamp)) {
											Logger::String{"Cant set timestamp on '",descr.filename,"': ",strerror(errno)}.error("xml");
										} else if(Logger::enabled(Logger::Trace)) {
											Logger::String{"Timestamp of '", descr.filename, "' set to ",TimeStamp{timestamp}.to_string()}.trace("xml");
										}
									}

									// Count changed file.
									changed++;

								} else {

									Logger::String {
										"The first node on ",handler->c_str()," is <",doc.document_element().name(),">, expecting <",xmlname.c_str(),">, update is unsafe"
									}.error("xml");

								}

							} else {

								error() << "Error parsing " << handler->c_str() << ": " << result.description() << endl;

							}

						} catch(HTTP::Exception &e) {

							if(e.code() != 304) {
								throw;
							}

							cout << "http\t" << descr.filename << " was not modified" << endl;
						}


					} catch(const std::exception &e) {

						error() << "Error '" << e.what() << "' updating from " << descr.filename << endl;
						refresh = descr.iffailed;

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

				error() << descr.filename << ": " << e.what() << endl;

			}

		}

		if(changed) {
			Logger::String(changed, " file(s) changed, requesting full update").write(Logger::Trace,name.c_str());
			update = true;
		}

		return update;

	}

	bool Updater::load(std::shared_ptr<Abstract::Agent> root) noexcept {

		files.sort([](const Settings &a, const Settings &b){

			const char *names[] = {
				a.filename.c_str(),
				b.filename.c_str()
			};

			for(size_t ix = 0; ix < 2; ix++) {
				const char *ptr = strrchr(names[ix],'/');
				if(ptr) {
					names[ix] = ptr+1;
				}
				ptr = strrchr(names[ix],'\\');
				if(ptr) {
					names[ix] = ptr+1;
				}
			}

			return strcasecmp(names[0],names[1]) < 0;

		});

		for(const Settings &descr : *this) {

			Logger::String{"Loading '",descr.filename,"'"}.write(Logger::Trace,name.c_str());

			try {

				pugi::xml_document doc;
				auto result = doc.load_file(descr.filename.c_str());
				if(result.status != pugi::status_ok) {
					error() << descr.filename << ": " << result.description() << endl;
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

				error() << descr.filename << ": " << e.what() << endl;
				return false;

			}

		}

		// Activate new root agent.
		// Udjat::setRootAgent(root);
		Logger::String{"Activating new root agent"}.trace(Application::Name().c_str());
		Abstract::Agent::Controller::getInstance().set(root);

		Module::for_each([root](Module &module){
			module.set(root);
			return false;
		});

		return true;

	}

 }
