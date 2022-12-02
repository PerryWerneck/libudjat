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
 #include <udjat/tools/protocol.h>
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
 #include <udjat/tools/ip.h>

 #ifndef _WIN32
	#include <unistd.h>
 #endif // _WIN32

 namespace Udjat {

	Protocol::Worker::Worker(const URL &url, const HTTP::Method method, const char *payload) : args(url, method), out(payload) {

		if(!args.url.empty()) {

			size_t pos = args.url.find("://");
			if(pos != string::npos) {
				timeout.setup(URL::Scheme{args.url.c_str(),pos}.c_str());
			}

		}

		Protocol::Controller::getInstance().insert(this);
	}

	Protocol::Worker::Worker(const char *url, const HTTP::Method method, const char *payload) : Worker(URL(url),method,payload) {
	}

	Protocol::Worker::~Worker() {
		Protocol::Controller::getInstance().remove(this);
	}

	void Protocol::Worker::Timeouts::setup(const char *scheme) noexcept {

		if(scheme && *scheme) {

			debug("Setting worker timeouts for scheme '",scheme,"'");

			connect = Config::Value<time_t>(scheme,"ConnectTimeout",connect);
			recv = Config::Value<time_t>(scheme,"ReceiveTimeout",recv);
			send = Config::Value<time_t>(scheme,"SendTimeout",send);

			debug("connect=",connect,", recv=",recv,", send=",send);

#ifdef _WIN32
			resolv = Config::Value<time_t>(scheme,"ResolveTimeout",resolv);
#endif // _WIN32

		}

	}

	void Protocol::Worker::set_local(const sockaddr_storage &addr) noexcept {

		out.payload.expand([this,addr](const char *key, std::string &value){

			if(strcasecmp(key,"ipaddr") == 0) {
				value = to_string(addr);
				return true;
			}

			if(strcasecmp(key,"network-interface") == 0) {
				getnic(addr,value);
				return true;
			}

			if(strcasecmp(key,"macaddress") == 0) {
				getmac(addr,value);
				return true;
			}

			return false;

		},false,false);

	}

	/// @brief Set remote addr.
	void Protocol::Worker::set_remote(const sockaddr_storage &addr) noexcept {

		out.payload.expand([addr](const char *key, std::string &value){

			if(strcasecmp(key,"hostip") == 0) {
				value = to_string(addr);
				return true;
			}

			return false;

		},false,false);

	}

	Protocol::Worker & Protocol::Worker::credentials(const char UDJAT_UNUSED(*user), const char UDJAT_UNUSED(*passwd)) {
		throw system_error(ENOTSUP,system_category(),"No credentials support on selected worker");
	}

	int Protocol::Worker::test(const std::function<bool(double current, double total)> UDJAT_UNUSED(&progress)) noexcept {
		return ENOTSUP;
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

		if(!args.url.empty()) {

			size_t pos = args.url.find("://");
			if(pos != string::npos) {
				timeout.setup(URL::Scheme{args.url.c_str(),pos}.c_str());
			}

		}

		return *this;
	}

	String Protocol::Worker::get() {
		return get(dummy_progress);
	}

	int Protocol::Worker::test() noexcept {
		return test(dummy_progress);
	}

	void Protocol::Worker::get(const std::function<void(int code, const char *response)> UDJAT_UNUSED(&call)) {
		throw system_error(ENOTSUP,system_category());
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

	std::ostream & Protocol::Worker::info() const {
		return cout << name << "\t";
	}

	std::ostream & Protocol::Worker::warning() const {
		return clog << name << "\t";
	}

	std::ostream & Protocol::Worker::error() const {
		return cerr << name << "\t";
	}

	std::ostream & Protocol::Worker::trace() const {
		return Logger::trace() << name << "\t";
	}


 }


