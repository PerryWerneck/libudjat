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
 #include <udjat/tools/response.h>
 #include <udjat/tools/exception.h>
 #include <udjat/tools/intl.h>
 #include <ctime>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Response::~Response() {
	}

	Response::operator Value::Type() const noexcept {
		return (Value::Type) data;
	}

	bool Response::empty() const noexcept {
		return data.empty();
	}

	bool Response::isNull() const {
		return data.isNull();
	}

	Udjat::Value & Response::reset(const Udjat::Value::Type type) {
		return data.reset(type);
	}

	bool Response::for_each(const std::function<bool(const char *name, const Udjat::Value &value)> &call) const {
		return data.for_each(call);
	}

	time_t Response::expires(const time_t tm) noexcept {
		if(!timestamp.expires || timestamp.expires > tm) {
			timestamp.expires = tm;
		}
		return timestamp.expires;
	}

	const char * Response::message() const noexcept {
		if(status.message.empty()) {
			return "Ok";
		}
		return status.message.c_str();
	}

	Udjat::Value & Response::operator[](const char *name) {
		return data[name];
	}

	std::string Response::to_string() const noexcept {
		return data.to_string();
	}

	Response & Response::failed(const char *message, const char *details) noexcept {
		return failed("",message,details);
	}

	Response & Response::failed(const char *title,  const char *message, const char *details) noexcept {

		status.value = State::Failure;
		data.reset(Value::Object);

		status.message = message;
		status.code = 0;

		if(title && *title) {
			status.title = title;
			data["title"] = status.title;
		} else {
			status.title.clear();
		}

		if(details && *details) {
			data["details"] = status.details;
			status.details = details;
		} else {
			status.details.clear();
		}

		return *this;

	}

	Response & Response::failed(const std::exception &e) noexcept {

		status.value = State::Failure;
		data.reset(Value::Object);

		status.message = e.what();
		status.title.clear();
		status.details.clear();
		status.code = 0;

		{
			const Udjat::Exception *except = dynamic_cast<const Udjat::Exception *>(&e);
			if(except) {

				status.title = except->title();
				status.details = except->body();
				status.code = except->syscode();

				data["title"] = status.title;
				data["details"] = status.details;
				data["domain"] = except->domain();
				data["url"] = except->url();

				return *this;
			}
		}

		{
			const std::system_error *except = dynamic_cast<const std::system_error *>(&e);
			if(except) {

				status.code = except->code().value();
				status.title = _("System error");
				status.details = except->code().message();

				data["title"] = status.title;
				data["details"] = status.details;
				data["category"] = except->code().category().name();
				
				return *this;

			}
		}

		return *this;
	}

	void Response::serialize(std::ostream &stream) const {

		// https://github.com/omniti-labs/jsend

		switch(mimetype) {
		case Udjat::Value::Undefined:
			throw runtime_error("Unable to serialize undefined value");
			break;

		case Udjat::MimeType::xml:
			stream << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><response><status type='String'>";
			stream << status.value << "</status>";
			if(status.code) {
				stream << "<code>" << status.code << "</code>";
			}
			if(!status.message.empty()) {
				stream << "<message>" << status.message << "</message>";
			}

			stream << "<data>";
			data.to_xml(stream);
			stream << "</data></response>";
			break;

		case Udjat::MimeType::json:
			stream << "{\"status\":\"" << status.value << "\",\"data\":";
			data.to_json(stream);
			stream << "}";
			break;

		case Udjat::MimeType::yaml:
			stream << "status: \"" << status.value << "\"" << endl << "data:" << endl;
			data.to_yaml(stream,4);
			break;

		case Udjat::MimeType::html:
			stream << "<!doctype html xmlns=\"http://www.w3.org/1999/xhtml\">" \
						"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>"
					<< (status.message.empty() ? "Response" : status.message)
					<< "</title></head><body>";
			data.to_html(stream);
			stream << "</body></html>";
			break;

		case MimeType::sh:
			stream << "status=\"" << status.value << "\"" << endl;
			data.to_sh(stream);
			break;

		default:
			data.serialize(stream,mimetype);
		}

	}

 }

namespace std {

	UDJAT_API const char * to_string(const Udjat::Response::State state) {

		static const char * names[] = {
			"success",
			"error",
			"failure"
		};

		if( ((size_t) state) >= N_ELEMENTS(names)) {
			return "failure";
		}

		return names[state];

	}

 }
