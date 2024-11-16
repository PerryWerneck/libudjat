/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements the API Call convenience class.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/method.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/intl.h>
 #include <list>

 using namespace std;

 namespace Udjat {

	class Method::Controller {
	private:
		std::list<Method *> methods;
		static std::mutex guard;

		Controller() {
		}

	public:
		static Controller & getInstance();

		inline const Method & find(const char *name) const {
			std::lock_guard<std::mutex> lock(guard);
			for(const Method *method : methods) {
				if(!strcasecmp(method->_name,name)) {
					return *method;
				}
			}
			throw system_error(ENOENT,system_category(),_( "Unable to handle request, no method"));		
		}

		inline void push_back(Method * method) noexcept {
			std::lock_guard<std::mutex> lock(guard);
			methods.push_back(method);
		}

		inline void remove(Method * method) noexcept {
			std::lock_guard<std::mutex> lock(guard);
			methods.remove(method);
		}

	};

	std::mutex Method::Controller::guard;
	Method::Controller & Method::Controller::getInstance() {
		std::lock_guard<std::mutex> lock(guard);
		static Controller instance;
		return instance;
	}

	const Method & Method::find(const char *name) {
		return Controller::getInstance().find(name);		
	}

	Method::Method(const char *n) : _name{n} {
		Controller::getInstance().push_back(this);
	}

	Method::Method(const XML::Node &node) : Method{String{node,"name",true}.as_quark()} {
	}

	Method::~Method() {
		Controller::getInstance().remove(this);
	}

	void Method::call(const char *, Udjat::Value &) {
		throw system_error(ENOTSUP,system_category(),_( "Unable to handle request, no backend"));		
	}

	void Method::introspect(Udjat::Value &input, Udjat::Value &output) {

		for_each([&input,&output](const size_t, bool in, const char *name, const Value::Type type){

			if(input) {
				input.append(name,type);
			} else {
				output.append(name,type);
			}

			return false;
		});
		
	}

	/*
	Method::Method(const XML::Node &node) {

		for(XML::Node child = node.child("input"); child; child = child.next_sibling("input")) {
			args.emplace_back(child,true);
		}

		for(XML::Node child = node.child("output"); child; child = child.next_sibling("output")) {
			args.emplace_back(child,false);
		}

	}

	Method::Argument::Argument(const XML::Node &node, bool input)
		: Argument{String{node,"name"}.as_quark(),Value::TypeFactory(node),input} {
	}

	Method::~Method() {

	}

	size_t Method::index(const char *name, bool insert, bool input) {

		for(size_t ix = 0; ix < args.size();ix++) {
			if(!strcasecmp(name,args[ix].name)) {
				return ix;
			}
		}

		if(insert) {
			args.emplace_back(name,Value::Type::Undefined,input);
			return args.size()-1;
		}

		throw logic_error(Logger::Message{"Cant find argument '{}'",name});

	}

	size_t Method::index(const char *name) const {

		for(size_t ix = 0; ix < args.size();ix++) {
			if(!strcasecmp(name,args[ix].name)) {
				return ix;
			}
		}

		throw logic_error(Logger::Message{"Cant find argument '{}'",name});

	}

	bool Method::for_each(const std::function<bool(const size_t index, const char *name, const Value::Type type)> &call) const {
		for(size_t ix = 0; ix < args.size();ix++) {
			if(call(ix,args[ix].name,args[ix].type)) {
				return true;
			}
		}
		return false;
	}

	bool Method::for_each_input(const std::function<bool(const size_t index, const char *name, const Value::Type type)> &call) const {
		for(size_t ix = 0; ix < args.size();ix++) {
			if(args[ix].input && call(ix,args[ix].name,args[ix].type)) {
				return true;
			}
		}
		return false;
	}

	bool Method::for_each_output(const std::function<bool(const size_t index, const char *name, const Value::Type type)> &call) const {
		for(size_t ix = 0; ix < args.size();ix++) {
			if(!args[ix].input && call(ix,args[ix].name,args[ix].type)) {
				return true;
			}
		}
		return false;
	}
	*/

 }
