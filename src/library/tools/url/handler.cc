/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 #include <udjat/tools/url.h>
 #include <list>

 #include <sstream>
 #include <udjat/tools/file.h>
 #include <udjat/tools/file/handler.h>
 #include <udjat/tools/file/temporary.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/exception.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/module/abstract.h>
 #include <private/url.h>
 #include <uriparser/Uri.h>
 #include <private/urlparser.h>
 #include <udjat/tools/container.h>

 using namespace std;

 namespace  Udjat {

 	Container<URL::Handler::Factory> & factories() {
		static Container<URL::Handler::Factory> factories;
		return factories;
	}

	URL::Handler::Factory::Factory(const char *n, const char *description) : name{n} {
		if(description && *description && Logger::enabled(Logger::Trace)) {
			Logger::String{"Registering URL handler '",name,"' - ",description}.trace();
		} else {
			Logger::String{"Registering URL handler '",name,"'"}.trace();
		}
		factories().push_back(this);
	}

	URL::Handler::Factory::~Factory() {
		Logger::String{"Removing URL handler '",name,"'"}.write(Logger::Debug);
		factories().remove(this);
	}

	std::shared_ptr<URL::Handler> URL::handler(bool allow_default, bool autoload) const {

		auto scheme = this->scheme();
		auto mark = scheme.find('+');

		if(mark != string::npos) {
			//
			// It's a combined scheme, remove the first part.
			//
			URL url{(this->c_str() + mark + 1)};
			scheme.resize(mark);

			if(!strcasecmp(url.scheme().c_str(),scheme.c_str())) {
				throw logic_error(Logger::String{"Circular reference on ",this->c_str()});
			}

			for(const auto factory : factories()) {
				if(*factory == scheme.c_str()) {
					return factory->HandlerFactory(url);
				}
			}

			throw runtime_error(Logger::String{"Unable to find url handler for '",scheme.c_str(),"'"});
		}

		for(const auto factory : factories()) {
			if(*factory == scheme.c_str()) {
				return factory->HandlerFactory(*this);
			}
		}

		if(!strcasecmp(scheme.c_str(),"file")) {
			return make_shared<FileURLHandler>(*this);
		}

		if(!strcasecmp(scheme.c_str(),"script")) {
			return make_shared<ScriptURLHandler>(*this);
		}

		if(autoload) {

			// Try to load a module using the handler name.
			try {

				if(Module::load(scheme.c_str(),false)) {

					Logger::String{"Module for ",scheme.c_str()," handler loaded"}.trace();

					for(const auto factory : factories()) {
						if(*factory == scheme.c_str()) {
							return factory->HandlerFactory(*this);
						}
					}

				}

			} catch(const std::exception &e) {

				Logger::String{"Failed to load module for ",scheme.c_str()," handler: ",e.what()}.trace();
			
			}


		}

		// Get default handler
		if(allow_default) {
			for(const auto factory : factories()) {
				if(*factory == "default") {
					return factory->HandlerFactory(*this);
				}
			}
		}

		throw invalid_argument(String{"Cant handle ",c_str()});
	}

	URL::Handler::~Handler() {
	}

	URL::Handler & URL::Handler::header(const char *, const char *) {
		return *this;
	}

	const char * URL::Handler::header(const char *) const {
		return "";
	}

	URL::Handler & URL::Handler::set(const MimeType mimetype) {
		// https://www.rfc-editor.org/rfc/rfc7231#section-5.3.2
		header("Accept",std::to_string(mimetype));
		return *this;
	}

	int URL::Handler::test(const HTTP::Method method, const char *payload) {

		if(method > HTTP::Patch || method == HTTP::Delete || method == HTTP::Connect || method == HTTP::Options || method == HTTP::Trace) {
			return -EINVAL;
		}

		return -ENOTSUP;
	}

	int URL::Handler::except(int code, const char *message) {

		if(code >= 200 && code <= 299) {
			return code;
		}

		if(code == 304) {        // Not modified.
			return 304;
		}

		if(code > 300 && code <= 399) { // Redirected.
			return code;
		}

		debug("code=",code," message='",message,"'");
		throw HTTP::Exception((unsigned int) code, message);

	}

	URL::Handler & URL::Handler::credentials(const char *, const char *) {
		return *this;
	}

	bool URL::Handler::get(Udjat::Value &value, const HTTP::Method method, const char *payload) {
		errno = ENOTSUP;
		return false;
	}

	String URL::Handler::get(const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress) {
		stringstream str;
		int rc = perform(
			method, 
			payload, 
			[&str,&progress](uint64_t current, uint64_t total, const void *data, size_t len){
				str.write((const char *) data,len);
				return progress(current,total);
			}
		);

		debug("rc=",rc);
		except(rc);
		debug("str=\n",str.str());

		return String{str.str()};
	}

	String URL::Handler::get(const HTTP::Method method, const char *payload) {
		return get(method,payload,[](uint64_t,uint64_t){ return false; });
	}

	bool URL::Handler::get(File::Handler &file, const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress) {

		{
			time_t mtime = file.mtime();
			if(mtime) {
				header("If-Modified-Since",HTTP::TimeStamp(mtime).to_string().c_str());
			}
		}

		int rc = perform(
			method, 
			payload, 
			[&file,&progress](uint64_t current, uint64_t total, const void *data, size_t len){

				if(len && data) {
					file.write(current,data,len);
				} else if(current == 0 && total) {
					file.allocate(total);
				}
				
				return progress(current,total);
			}
		);

		debug("File updated exits with ",rc);

		except(rc);

		if(rc != 304) {
			return true;
		}

		return false;
	}

	bool URL::Handler::get(File::Handler &file, const HTTP::Method method, const char *payload) {
		return get(file,method,payload,[](uint64_t,uint64_t){ return false; });
	}

	bool URL::Handler::get(const char *filename, const HTTP::Method method, const char *payload, const std::function<bool(uint64_t current, uint64_t total)> &progress) {
		
		// Download to temporary file.
		File::Temporary file{filename};

		{
			time_t mtime = file.mtime();
			if(mtime) {
				header("If-Modified-Since",HTTP::TimeStamp(mtime).to_string().c_str());
			}
		}

		status.code = perform(
			method, 
			payload, 
			[&file,&progress](uint64_t current, uint64_t total, const void *data, size_t len){

				if(len && data) {
					debug("Writing ",len," bytes");
					file.write(current,data,len);
				} else if(current == 0 && total) {
					debug("Allocating ",total," bytes");
					file.allocate(total);
				}
				
				debug("Calling progress(",current,",",total,")");
				return progress(current,total);
			}
		);

		debug("Update of ",filename," exits with ",status.code);
		if(status.code == 304) {
			return false;
		}

		except(status.code);

		if(status.code >= 200 && status.code <= 299) {

			// Save temporary file.
			file.save(filename,true);

			// Set file modification time.
			HTTP::TimeStamp timestamp{header("Last-Modified")};
			debug("timestamp=",timestamp.to_string());
			if(timestamp) {
				Logger::String{"Timestamp of ",filename," set to ",timestamp.to_string()}.trace();
				File::mtime(filename, (time_t) timestamp);
			}

			return true;
		}

		return false;

	}

	bool URL::Handler::get(const char *filename, const HTTP::Method method, const char *payload) {
		return get(filename,method,payload,[](uint64_t,uint64_t){ return false; });
	}

 }

