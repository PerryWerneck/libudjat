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
 #include <udjat/tools/worker.h>
 #include <udjat/tools/logger.h>
 #include <udjat/module/info.h>

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

	const Worker * Worker::find(const char *name) {

		const Worker *response = nullptr;

		if(!Worker::Controller::getInstance().for_each([&response,name](const Worker &worker) {

			if(!strcasecmp(worker.name,name)) {
				response = &worker;
				return true;
			}

			return false;

		})) {
			throw std::system_error(ENOENT,std::system_category(),Logger::Message("Cant find worker '{}'",name));
		}

		return response;
	}

	/*
	bool Worker::work(const char *path, Request &request, Response &response) {

		debug("--------------> request.path='",path,"'");

		return Worker::for_each([path,&request,&response](const Worker &worker) {

			if(worker.probe(path)) {
				worker.work(request,response);
			}

			return false;
		});

		return false;
	}

	bool Worker::work(const char *path, Request &request, Report &response) {
		debug("--------------> request.path='",path,"'");

		return Worker::for_each([path,&request,&response](const Worker &worker) {

			if(worker.probe(path)) {
				worker.work(request,response);
			}

			return false;
		});

		return false;
	}
	*/

	bool Worker::get(Request &, Response::Value &) const {
		return false;
	}

	bool Worker::head(Request &, Response::Value &) const {
		return false;
	}

	bool Worker::work(Request &request, Response::Value &response) const {

		switch((HTTP::Method) request) {
		case HTTP::Get:
			debug("HTTP GET");
			return get(request,response);

		case HTTP::Head:
			debug("HTTP HEAD");
			return head(request,response);

		default:
			throw system_error(ENOENT,system_category(),Logger::String{"Unable to handle '",(const char *) request,"'"});
		}

		return false;

	}

	bool Worker::work(Request &, Response::Table &) const {
		return false;
	}

	const char * Worker::check_path(const char *path) const noexcept {

		if(!name && *name) {
			return nullptr;
		}

		while(*path && *path == '/') {
			path++;
		}

		size_t szname = strlen(name);

		if(strncasecmp(name,path,szname)) {
			debug("Rejecting '",path,"' on worker ",name);
			return nullptr;
		}

		debug("Found '",path,"' on worker ",name);

		path += szname;

		debug("Path fixed to '",path,"'");
		if(*path && *path != '/') {
			return nullptr;
		}

		debug("Using path '",path,"' on worker ",name);
		return path;
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

