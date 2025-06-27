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
 #include <udjat/tools/protocol.h>
 #include <private/protocol.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/base64.h>
 #include <udjat/net/ip/address.h>

 #ifdef _WIN32
	#include <sys/utime.h>
 #else
	#include <unistd.h>
	#include <sys/types.h>
	#include <utime.h>
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

	int Protocol::Worker::mimetype(const MimeType type) {
		request("Content-Type") = std::to_string(type);
		return 0;
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

	Protocol::Header & Protocol::Worker::request(const char *name) {
		throw system_error(ENOTSUP,system_category(),string{"The selected worker was unable to handle '"} + name + "' request");
	}

	const Protocol::Header & Protocol::Worker::response(const char *name) {
		throw system_error(ENOTSUP,system_category(),string{"The selected worker was unable to handle '"} + name + "' response");
	}

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
		Protocol::Watcher::getInstance().set_url(args.url.c_str());
		return get(Protocol::Watcher::progress);
	}

	int Protocol::Worker::test() noexcept {
		Protocol::Watcher::getInstance().set_url(args.url.c_str());
		return test(Protocol::Watcher::progress);
	}

	void Protocol::Worker::get(const std::function<void(int code, const char *response)> UDJAT_UNUSED(&call)) {
		throw system_error(ENOTSUP,system_category());
	}

	bool Protocol::Worker::save(const char *filename, bool replace) {
		Protocol::Watcher::getInstance().set_url(args.url.c_str());
		return save(filename, Protocol::Watcher::progress, replace);
	}

	string Protocol::Worker::save() {
		Protocol::Watcher::getInstance().set_url(args.url.c_str());
		return save(Protocol::Watcher::progress);
	}

	string Protocol::Worker::save(const std::function<bool(double current, double total)> &progress) {
		std::string filename = File::Temporary::create();
		save(filename.c_str(),progress,true);
		return filename;
	}

	bool Protocol::Worker::save(File::Handler &file, const std::function<bool(double current, double total)> &progress) {

		save([&file,&progress](unsigned long long current, unsigned long long total, const void *buf, size_t length){
			file.write(buf,length);
			return progress(current,total);
		});

		return true;
	}

	bool Protocol::Worker::save(const char *filename, const std::function<bool(double current, double total)> &progress, bool replace) {

		File::Temporary tmpfile{filename};

		debug("Saving ",filename);
		if(save(tmpfile,progress)) {
			debug("File '",filename,"' was updated");
			tmpfile.save(filename,replace);
			return true;
		}

		return false;

	}

	void Protocol::Worker::save(const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &) {
		throw system_error(ENOTSUP,system_category(),"The available backend is unable to manage custom writers");
	}

	bool Protocol::Worker::save(const char *filename,const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer) {

		File::Temporary tmpfile{filename};

		bool saved = true;
		bool alloc = true;

		save([&tmpfile,&writer,&saved,&alloc](unsigned long long current, unsigned long long total, const void *buf, size_t length){

			if(total && !alloc) {
				tmpfile.allocate(total);
				alloc = false;
			}

			tmpfile.write(current,buf,length);
			if(!writer(current,total,buf,length)) {
				saved = false;
			}

			return saved;

		});

		if(saved) {
			tmpfile.save(filename,true);
		}

		return saved;
	}

	std::string Protocol::Worker::filename(const std::function<bool(double current, double total)> &progress) {
		Application::CacheDir name{"urls"};
		name += Base64::encode(url());
		save(name.c_str(),progress);
		return name;
	}

	std::string Protocol::Worker::filename() {
		Protocol::Watcher::getInstance().set_url(args.url.c_str());
		return filename(Protocol::Watcher::progress);
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
*/

