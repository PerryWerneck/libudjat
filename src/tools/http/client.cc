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
 #include <udjat/defs.h>
 #include <udjat/tools/http/client.h>
 #include <string>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/logger.h>

#ifndef _WIN32
	#include <unistd.h>
#endif // _WIN32

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		static const std::function<bool(double current, double total)> dummy_progress([](double UDJAT_UNUSED(current), double UDJAT_UNUSED(total)) {
			return true;
		});

		static void setup_cache(std::shared_ptr<Protocol::Worker> worker, const char *filename) {

			struct stat st;

			if(stat(filename,&st) < 0) {

				if(errno != ENOENT) {
					throw system_error(errno,system_category(),Logger::Message("Can't stat '{}'",filename));
				}

				cout << "http\tDownloading '" << filename << "'" << endl;

			} else {

				worker->header("If-Modified-Since") = TimeStamp(st.st_mtime);

#ifdef DEBUG
				cout << "Last modification time: " << worker->header("If-Modified-Since") << endl;
#endif // DEBUG

			}
		}

		String get(const char *url) {
			return Client(url).get();
		}

		bool save(const char *url, const char *filename) {
			return Client(url).save(filename);
		}

		Client::Client(const pugi::xml_node &node) : Client(node.attribute("src").as_string()) {
		}

		Client::Client(const URL &url) {

			// Find a protocol handler for this URL.
			const Protocol * protocol = Protocol::find(url);
			if(!protocol) {
				throw runtime_error(string{"Cant find a protocol handler for "} + url);
			}

			worker = protocol->WorkerFactory();
			if(worker) {
				worker->url(url);
				return;
			}

			//
			// The protocol was unable to create a worker, use a proxy to the 'old' API.
			//
			protocol->warning() << "No worker factory (old version?) using proxy worker" << endl;

			class Proxy : public Protocol::Worker {
			public:
				Proxy(const URL &url) : Protocol::Worker(url) {
				}

				virtual ~Proxy() {
				}

				String get(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) override {
					return Udjat::Protocol::call(url().c_str(),method(),out.payload.c_str());
				}

				bool save(const char UDJAT_UNUSED(*filename), const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress), bool UDJAT_UNUSED(replace)) override {
					throw runtime_error("The selected protocol is unable to save files");
				}

			};

			worker = make_shared<Proxy>(url);

		}

		String Client::get(const std::function<bool(double current, double total)> &progress) {
			worker->payload(payload.str());
			worker->method(Get);
			return worker->get(progress);
		}

		String Client::get() {
			return get(dummy_progress);
		}

		String Client::post(const char *payload, const std::function<bool(double current, double total)> &progress) {
			worker->payload(payload);
			worker->method(Post);
			return worker->get(progress);
		}

		String Client::post(const char *payload) {
			return post(payload,[](double UDJAT_UNUSED(current),double UDJAT_UNUSED(total)){return true;});
		}

		bool Client::save(const char *filename, const std::function<bool(double current, double total)> &progress) {
			worker->payload(payload.str());
			worker->method(Get);
			setup_cache(worker,filename);
			return worker->save(filename,progress);
		}

		bool Client::save(const char *filename) {
			return save(filename,dummy_progress);
		}

		bool Client::save(const pugi::xml_node &node, const char *filename, const std::function<bool(double current, double total)> &progress) {

			Client client(node);

			if(node.attribute("cache").as_bool(true)) {
				setup_cache(client.worker,filename);
			} else {
				cout << "http\tCache for '" << filename << "' disabled by XML definition" << endl;
			}

			return client.worker->save(filename,progress);
		}

		bool Client::save(const pugi::xml_node &node, const char *filename) {
			return save(node,filename,dummy_progress);
		}

		std::string Client::filename(const std::function<bool(double current, double total)> &progress) {
			return worker->filename(progress);
		}

		std::string Client::filename() {
			return worker->filename();
		}


	}

 }
