/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Brief description of this source.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <algorithm>
 #include <private/request.h>
 #include <udjat/tools/abstract/response.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/http/timestamp.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/application.h>
 #include <udjat/tools/configuration.h>

 using namespace std;

 namespace Udjat {

	Abstract::Response & Abstract::Response::failed(int code, const char *message, const char *body, const char *url) noexcept {
		if(!status.code) {
			status.code = code;
			status.message = message;
			status.body = body;
			status.url = url;
		}
		return *this;
	}

	Abstract::Response & Abstract::Response::failed(int code) noexcept {
		return failed(code,strerror(code));
	}

	time_t Abstract::Response::expires(const time_t tm) noexcept {

		if(tm && tm > time(0)) {
			if(!timestamp.expires || timestamp.expires > tm)
				timestamp.expires = tm;
		}

		return (time_t) timestamp.expires;

	}

	time_t Abstract::Response::last_modified(const time_t tm) noexcept {

		if(tm && tm <= time(0)) {
			if(!timestamp.last_modified || timestamp.last_modified > tm)
				timestamp.last_modified = tm;
		}

		return (time_t) timestamp.last_modified;

	}

	void Abstract::Response::for_each(const std::function<void(const char *property_name, const char *property_value)> &call) const noexcept {

		if(status.code == 0) {

			// No error.

			time_t now = time(0);

			time_t modtime = last_modified();
			if(!modtime) {
				modtime = now;
			}

			call("Last-Modified", HTTP::TimeStamp{modtime}.to_string().c_str());

			time_t expires = this->expires();
			if(expires >= now) {

				// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control
				call("Cache-Control", Udjat::String{"max-age=",(unsigned int) (expires-now),", private"}.c_str());
				call("Expires", HTTP::TimeStamp{expires}.to_string().c_str());

			} else {

				// https://stackoverflow.com/questions/18148884/difference-between-no-cache-and-must-revalidate-for-cache-control
				if(last_modified()) {
					call("Cache-Control", "must-revalidate, private, max-age=0");
				} else {
					call("Cache-Control", "no-cache, no-store, must-revalidate, private, max-age=0");
				}

				call("Expires", "0");

			}

		} else {

			// Error response, setup cache.

			call("Cache-Control", "no-cache, no-store, must-revalidate, private, max-age=0");
			call("Expires", "0");

		}

		call("Content-Type",std::to_string(mimetype));

		// Add standard headers from configuration.
		Config::for_each("response-headers",[&call](const char *key, const char *value) {
			call(key,value);
			return false;
		});

	}

	void Abstract::Response::count(size_t) noexcept {
		// https://stackoverflow.com/questions/3715981/what-s-the-best-restful-method-to-return-total-number-of-items-in-an-object
	}

	void Abstract::Response::content_range(size_t, size_t, size_t) noexcept {
		// https://stackoverflow.com/questions/3715981/what-s-the-best-restful-method-to-return-total-number-of-items-in-an-object
	}

	Abstract::Response & Abstract::Response::failed(const std::system_error &e) noexcept {
		return failed(e.code().value(),e.what());
	}

	Abstract::Response & Abstract::Response::failed(const HTTP::Exception &e) noexcept {
		return failed(e.syscode(),e.what());
	}

	Abstract::Response & Abstract::Response::failed(const std::exception &e) noexcept {

		// Is it an Udjat::Exception?
		{
			const Udjat::Exception *err = dynamic_cast<const Udjat::Exception *>(&e);
			if(err) {
				return failed(err->syscode(), err->what(), err->body(), err->url());
			}
		}

		// Is it a HTTP:Exception?
		{
			const HTTP::Exception *err = dynamic_cast<const HTTP::Exception *>(&e);
			if(err) {
				return failed(err->syscode(),err->what());
			}
		}

		// Is it a system error?
		{
			const std::system_error *err = dynamic_cast<const std::system_error *>(&e);
			if(err) {
				return failed(err->code().value(),err->what());
			}
		}

		// Regular exception.
		return failed(-1,e.what());
	}

	Abstract::Response & Abstract::Response::failed(const char *message, const char *body, const char *url) noexcept {
		return failed(-1,message,body,url);
	}

	std::string Abstract::Response::to_string() const {
		Logger::String{"Unable to convert abstract response to string, using an empty one"}.trace(Application::Name().c_str());
		return "";
	}

	void Abstract::Response::serialize(std::ostream &stream) const {

		debug(__FUNCTION__,": Serializing response");

		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wswitch"
		switch(mimetype) {
		case MimeType::xml:
			stream	<< "<?xml version=\"1.0\" encoding=\"UTF-8\"?><response status-code='"
					<< status.code << "'";

			if(!status.message.empty()) {
				stream << " status-message='" << status.message << "'";
			}

			if(timestamp.last_modified) {
				stream << " last-modified='" << timestamp.last_modified.to_string() << "'";
			}

			if(timestamp.expires) {
				stream << " expiration-time='" << timestamp.expires.to_string() << "'";
			}

			stream << ">";

			break;

		case Udjat::MimeType::json:
			{
				// Format as json
				// https://stackoverflow.com/questions/12806386/is-there-any-standard-for-json-api-response-format

				// https://github.com/omniti-labs/jsend
				stream << "{\"status\":\"";

				if(status.code == 0) {
					stream << "success\"";
				} else if(status.message.empty()) {
					stream << "fail\"";
				} else {
					stream << "error\",\"message\":\"" << status.message << "\"";
					if(status.code) {
						stream << ",\"code\":" << status.code;
					}
				}

				stream << ",\"data\":";

			}
			break;

		}
		#pragma GCC diagnostic pop

	}

 }

