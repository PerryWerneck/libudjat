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

/*
 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/http/client.h>
 #include <string>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/timestamp.h>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	namespace HTTP {

		static void setup_cache(std::shared_ptr<Protocol::Worker> worker, const char *filename) {

			struct stat st;

			if(stat(filename,&st) < 0) {

				if(errno != ENOENT) {
					throw system_error(errno,system_category(),Logger::Message("Can't stat '{}'",filename));
				}

			} else {

				worker->request("If-Modified-Since") = TimeStamp(st.st_mtime);
				debug("Last modification time: ",worker->request("If-Modified-Since").c_str());

			}
		}

		String get(const char *url) {
			return Client(url).get();
		}

		bool save(const char *url, const char *filename) {
			return Client(url).save(filename);
		}

		Client::Client(const XML::Node &node) : Client(node.attribute("src").as_string()) {
		}

		Client::Client(const URL &url, bool load) : worker{Protocol::WorkerFactory(url.c_str(),true,load)} {
			worker->url(url);
		}

		String Client::get(const std::function<bool(double current, double total)> &progress) {
			worker->payload(payload.str());
			worker->method(Get);
			return worker->get(progress);
		}

		String Client::get() {
			Protocol::Watcher::getInstance().set_url(worker->url().c_str());
			return get(Protocol::Watcher::progress);
		}

		String Client::post(const char *payload, const std::function<bool(double current, double total)> &progress) {
			worker->payload(payload);
			worker->method(Post);
			return worker->get(progress);
		}

		String Client::post(const char *payload) {
			return post(payload,[](double UDJAT_UNUSED(current),double UDJAT_UNUSED(total)){return true;});
		}

		void Client::cache(const char *filename) {
			setup_cache(worker,filename);
		}

		bool Client::save(const char *filename, const std::function<bool(double current, double total)> &progress) {
			worker->payload(payload.str());
			worker->method(Get);
			setup_cache(worker,filename);
			return worker->save(filename,progress);
		}

		bool Client::save(const char *filename,const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer) {
			worker->payload(payload.str());
			worker->method(Get);
			setup_cache(worker,filename);
			return worker->save(filename,writer);
		}

		bool Client::save(const char *filename) {
			Protocol::Watcher::getInstance().set_url(worker->url().c_str());
			return save(filename,Protocol::Watcher::progress);
		}

		bool Client::save(const XML::Node &node, const char *filename, const std::function<bool(double current, double total)> &progress) {

			Client client(node);

			if(node.attribute("cache").as_bool(true)) {
				setup_cache(client.worker,filename);
			} else {
				cout << "http\tCache for '" << filename << "' disabled by XML definition" << endl;
			}

			Protocol::Watcher::getInstance().set_url(client.worker->url().c_str());
			return client.worker->save(filename,progress);
		}

		bool Client::save(const XML::Node &node, const char *filename) {

			Client client(node);

			if(node.attribute("cache").as_bool(true)) {
				setup_cache(client.worker,filename);
			} else {
				cout << "http\tCache for '" << filename << "' disabled by XML definition" << endl;
			}

			Protocol::Watcher::getInstance().set_url(client.worker->url().c_str());
			return client.worker->save(filename,Protocol::Watcher::progress);
		}

		std::string Client::filename(const std::function<bool(double current, double total)> &progress) {
			return worker->filename(progress);
		}

		std::string Client::filename() {
			return worker->filename();
		}


	}

 }
*/
