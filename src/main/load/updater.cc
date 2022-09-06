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
 #include <udjat/tools/logger.h>

 #ifdef HAVE_SYSTEMD
	#include <systemd/sd-daemon.h>
 #endif // HAVE_SYSTEMD

 #ifdef _WIN32
	#include <udjat/win32/registry.h>
 #else
	#include <unistd.h>
 #endif // _WIN32

 using namespace std;

 namespace Udjat {

	Updater::Updater(const char *pathname) : path{pathname} {

		// First scan for modules.
		if(Config::Value<bool>("modules","preload-from-xml",true)) {
			cout << "modules\tPreloading from " << path << endl;
			for_each([](const char UDJAT_UNUSED(*filename), const pugi::xml_document &doc){
				for(pugi::xml_node node = doc.document_element().child("module"); node; node = node.next_sibling("module")) {
					Module::load(node);
				}
			});
		}

		// Then check for file updates.
		for_each([this](const char *filename, const pugi::xml_document &doc){

			auto node = doc.document_element();

			const char *url = node.attribute("src").as_string();
			if(url && *url) {

				time_t refresh = node.attribute("update-timer").as_uint(0);

				try {

					info() << "Updating " << filename << endl;
					if(HTTP::Client::save(node,filename)) {
						changed = true;
					}

				} catch(const std::exception &e) {

					error() << "Error '" << e.what() << "' updating " << filename << endl;
					refresh = node.attribute("update-when-failed").as_uint(refresh);

				} catch(...) {

					error() << "Unexpected error updating " << filename << endl;
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
	time_t Updater::set(std::shared_ptr<Abstract::Agent> agent) noexcept {

		agent->warning() << "Reconfiguring from " << path << endl;

#ifdef HAVE_SYSTEMD
		sd_notify(0,"RELOADING=1");
#endif // HAVE_SYSTEMD

		for_each([agent](const char *filename, const pugi::xml_document &doc){

			agent->info() << "Loading '" << filename << "'" << endl;

			// First setup agent, load modules, etc.
			try {

				auto node = doc.document_element();
				const char *path = node.attribute("path").as_string();

				if(path && *path) {

					// Has defined root path, find agent.
					agent->find(path,true,true)->setup(node);

				} else {

					// No path, load here.
					agent->setup(node);

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

		setRootAgent(agent);

#if defined(HAVE_SYSTEMD)
		sd_notifyf(0,"READY=1\nSTATUS=%s",agent->state()->to_string().c_str());
#elif defined(_WIN32)
		try {

			Win32::Registry registry("service",true);

			registry.set("status",agent->state()->to_string().c_str());
			registry.set("status_time",TimeStamp().to_string().c_str());

		} catch(const std::exception &e) {

			error() << "Cant update windows registry: " << e.what() << endl;

		}
#endif

		return next;

	}

	bool for_each(const char *path, const std::function<void(const char *filename, const pugi::xml_document &document)> &call) {

		bool rc = true;

		struct stat pathstat;
		if(stat(path, &pathstat) == -1) {
			throw system_error(errno,system_category(),Logger::Message("Can't load '{}'",path));
		}

		if((pathstat.st_mode & S_IFMT) == S_IFDIR) {
			//
			// It's a folder.
			//
			File::Path(path).for_each("*.xml",false,[&call,&rc](const char *filename){

				try {

					pugi::xml_document doc;
					auto result = doc.load_file(filename);
					if(result.status != pugi::status_ok) {
						cerr << "xml\t" << filename << ": " << result.description() << endl;
						rc = false;
						return true;
					}

					call(filename,doc);

				} catch(const std::exception &e) {

					cerr << "xml\t" << filename << ": " << e.what() << endl;
					rc = false;

				} catch(...) {

					cerr << "xml\t" << filename << ": Unexpected error" << endl;
					rc = false;

				}

				return true;

			});

		} else {
			//
			// It's a single file.
			//
			try {

				pugi::xml_document doc;
				auto result = doc.load_file(path);
				if(result.status != pugi::status_ok) {
					cerr << "xml\t" << path << ": " << result.description() << endl;
					return false;
				}

				call(path,doc);

			} catch(const std::exception &e) {

				cerr << "xml\t" << path << ": " << e.what() << endl;
				return false;

			} catch(...) {

				cerr << "xml\t" << path << ": Unexpected error" << endl;
				return false;

			}
		}

		return rc;

	}

 }
