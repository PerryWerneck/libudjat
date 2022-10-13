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
 #include <private/protocol.h>
 #include <cstring>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/base64.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 namespace Udjat {

	Protocol::Worker::Worker(const char *url, const HTTP::Method method, const char *payload) : args(url, method), out(payload) {

		auto scheme = args.url.scheme();

		timeout.connect = Config::Value<time_t>(scheme.c_str(),"ConnectTimeout",timeout.connect);
		timeout.recv = Config::Value<time_t>(scheme.c_str(),"ReceiveTimeout",timeout.recv);
		timeout.send = Config::Value<time_t>(scheme.c_str(),"SendTimeout",timeout.send);

#ifdef _WIN32
		timeout.resolv = Config::Value<time_t>(scheme.c_str(),"ResolveTimeout",timeout.resolv);
#endif // _WIN32

		Protocol::Controller::getInstance().insert(this);
	}

	Protocol::Worker::Worker(const URL &url, const HTTP::Method method, const char *payload) : args(url, method), out(payload) {

		auto scheme = args.url.scheme();

		timeout.connect = Config::Value<time_t>(scheme.c_str(),"ConnectTimeout",timeout.connect);
		timeout.recv = Config::Value<time_t>(scheme.c_str(),"ReceiveTimeout",timeout.recv);
		timeout.send = Config::Value<time_t>(scheme.c_str(),"SendTimeout",timeout.send);

#ifdef _WIN32
		timeout.resolv = Config::Value<time_t>(scheme.c_str(),"ResolveTimeout",timeout.resolv);
#endif // _WIN32

		Protocol::Controller::getInstance().insert(this);
	}

	Protocol::Worker::~Worker() {
		Protocol::Controller::getInstance().remove(this);
	}

	Protocol::Worker & Protocol::Worker::credentials(const char UDJAT_UNUSED(*user), const char UDJAT_UNUSED(*passwd)) {
		throw system_error(ENOTSUP,system_category(),"No credentials support on selected worker");
	}

	unsigned short Protocol::Worker::test() {
		throw system_error(ENOTSUP,system_category(),"No test support on selected worker");
	}

	Protocol::Header & Protocol::Worker::header(const char UDJAT_UNUSED(*name)) {
		throw system_error(ENOTSUP,system_category(),string{"The selected worker was unable do create header '"} + name + "'");
	}

	static const std::function<bool(double current, double total)> dummy_progress([](double UDJAT_UNUSED(current), double UDJAT_UNUSED(total)) {
		return true;
	});

	Protocol::Worker & Protocol::Worker::url(const char *url) noexcept {

		const char *scheme = strstr(url,"://");
		if(scheme) {
			for(const char *ptr = url; *ptr && ptr < scheme; ptr++) {
				if(*ptr == '+') {
					url = ptr+1;
					break;
				}
			}
		}

		args.url = url;
		return *this;
	}

	String Protocol::Worker::get() {
		return get(dummy_progress);
	}

	void Protocol::Worker::get(const std::function<void(int code, const char *response)> &call) {
		call(200,get().c_str());
	}

	bool Protocol::Worker::save(const char *filename, bool replace) {
		return save(filename, dummy_progress, replace);
	}

	string Protocol::Worker::save() {
		return save(dummy_progress);
	}

	string Protocol::Worker::save(const std::function<bool(double current, double total)> &progress) {
		std::string filename = File::Temporary::create();
		save(filename.c_str(),progress,true);
		return filename;
	}

	bool Protocol::Worker::save(const char *filename, const std::function<bool(double current, double total)> &progress, bool replace) {

		File::Temporary{filename}
			.write(get(progress))
			.save(filename,replace);

		/*
		String text = get(progress);

		// TODO: Make backup.

		size_t length = text.size();
		int fd = open(filename,O_WRONLY|O_CREAT|O_TRUNC,0777);
		if(fd < 0) {
			throw system_error(errno,system_category(),filename);
		}

		const char *ptr = text.c_str();
		while(length) {
			ssize_t bytes = write(fd,ptr,length);
			if(bytes < 0) {
				int err = errno;
				::close(fd);
				throw system_error(err,system_category(),filename);
			} else if(bytes == 0) {
				::close(fd);
				throw runtime_error(string{"Unexpected error writing "} + filename);
			}
			ptr += bytes;
			length -= bytes;
		}
		::close(fd);
		*/

		return true;
	}

	std::string Protocol::Worker::filename(const std::function<bool(double current, double total)> &progress) {
		Application::CacheDir name{"urls"};
		name += Base64::encode(url());
		save(name.c_str(),progress);
		return name;
	}

	std::string Protocol::Worker::filename() {
		return filename(dummy_progress);
	}

 }


