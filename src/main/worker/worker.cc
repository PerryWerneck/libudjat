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
 #include <private/worker.h>
 #include <udjat/worker.h>
 #include <udjat/tools/logger.h>
 #include <udjat/moduleinfo.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	Worker::Worker(const char *n, const ModuleInfo &i) : name(n), module(i) {
		Controller::getInstance().insert(this);
	}

	Worker::~Worker() {
		Controller::getInstance().remove(this);
	}

	bool Worker::for_each(const std::function<bool(const Worker &worker)> &method) {
		return Controller::getInstance().for_each(method);
	}

	Value & Worker::getProperties(Value &properties) const {
		properties["name"] = name;
		return module.getProperties(properties);
	}

	const Worker * Worker::find(const char *path) {

		const Worker *response = nullptr;

		if(!Worker::Controller::getInstance().for_each([&response,path](const Worker &worker) {

			if(worker.probe(path)) {
				response = &worker;
				return true;
			}

			return false;

		})) {
			throw std::system_error(ENOENT,std::system_category(),Logger::Message("Cant find a worker for '{}'",path));
		}

		return response;
	}

	bool Worker::work(const char *path, Request &request, Response &response) {
		debug("--------------> request.path='",path,"'");
		return find(path)->work(request,response);
	}

	bool Worker::work(const char *path, Request &request, Report &response) {
		debug("--------------> request.path='",path,"'");
		return find(path)->work(request,response);
	}

	bool Worker::get(Request UDJAT_UNUSED(&request), Response UDJAT_UNUSED(&response)) const {
		return false;
	}

	bool Worker::head(Request UDJAT_UNUSED(&request), Response UDJAT_UNUSED(&response)) const {
		return false;
	}


	bool Worker::work(Request &request, Response &response) const {

		auto type = request.as_type();
		switch(type) {
		case HTTP::Get:
			debug("HTTP GET");
			return get(request,response);

		case HTTP::Head:
			debug("HTTP HEAD");
			return head(request,response);

		default:
			throw system_error(ENOENT,system_category(),Logger::Message("'{}' request are unavailable here",type));
		}

		return false;

	}

	bool Worker::work(Request UDJAT_UNUSED(&request), Report UDJAT_UNUSED(&response)) const {
		return false;
	}

	/*
	const char * Worker::path(const char *path) const {

		while(*path && isspace(*path)) {
			path++;
		}

		if(*path == '/') {
			path++;
		}

		path += strlen(name);
		while(*path && *path == '/') {
			path++;
		}

		return path;
	}
	*/

	bool Worker::probe(const char *path) const noexcept {

		if(!name && *name) {
			return false;
		}

		while(*path && *path == '/') {
			path++;
		}

		debug("-------> Probing path '",path,"' on '",name,"'");

		size_t szname = strlen(name);
		size_t szpath = strlen(path);

		if(szpath < szname) {
			return false;
		}

		return strncasecmp(this->name,path,szname) == 0 && (path[szname] == 0 || path[szname] == '/');
	}

	std::ostream & Worker::info() const {
		return cout << name << "\t";
	}

	std::ostream & Worker::warning() const {
		return clog << name << "\t";
	}

	std::ostream & Worker::error() const {
		return cerr << name << "\t";
	}

	std::ostream & Worker::trace() const {
		return Logger::trace() << name << "\t";
	}

 }

