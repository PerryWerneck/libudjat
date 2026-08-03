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
 #include <udjat/tools/object.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/request.h>
 #include <udjat/tools/response.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/schema.h>
 #include <udjat/tools/http/schema.h>

 #include <cstdarg>

 using namespace std;

 namespace Udjat {

	static Container<Abstract::Object::Factory> & Factories() {
		static Container<Abstract::Object::Factory> instance;
		return instance;
	}

	Abstract::Object::Factory::Factory(const char *n) : name{n} {
		Factories().push_back(this);
	}

	Abstract::Object::Factory::~Factory() {
		Factories().remove(this);
	}

	bool Abstract::Object::schema(const char *, Schema::Method &schema) const noexcept {
		schema.add(
			Schema::Method::Item{HTTP::Get, Authentication::None}
		);
		return true;
	}

	bool Abstract::Object::schema(const HTTP::Method, const char *, Schema::Input &) const noexcept {
		return false;
	}

	bool Abstract::Object::schema(const HTTP::Method, const char *, Schema::Output &) const noexcept {
		return false;
	}

	std::shared_ptr<Abstract::Object> Abstract::Object::merge(const Object *object, ...) noexcept {

		class Object : public Udjat::NamedObject {
		public:
			vector<const Udjat::Abstract::Object *> items;

			Object(const Udjat::Abstract::Object *object) : NamedObject{object->name()} {
			}

			Value & get_properties(Value &value) const override {
				for(const auto item : items) {
					item->get_properties(value);
				}
				return value;
			}

			bool get_property(const char *key, Udjat::Variant &value) const override {
				for(const auto item : items) {
					if(item->get_property(key,value)) {
						return true;
					}
				}
				return false;
			}

		};

		auto obj = make_shared<Object>(object);

		va_list args;
		va_start(args, object);
		while(object) {
			obj->items.push_back(object);
			object = va_arg(args, const Udjat::Abstract::Object *);
		}

		return obj;
		
	}

	Abstract::Object::~Object() {
	}

	bool Abstract::Object::push_back(std::shared_ptr<Abstract::Object>) {
		return false;
	}

	bool Abstract::Object::push_back(const Properties &, std::shared_ptr<Abstract::Object> child) {
		return push_back(child);
	}

	void Abstract::Object::append_children(const Properties &props) {
		props.for_each_child([this](const Properties &child){
			this->append_child(child);
			return false;
		});
	}
	
	bool Abstract::Object::append_child(const Properties &props) {

		if(!props.allowed()) {
			return true; // Ignore reserved nodes.
		}

		const char *name = props.node_name();	// Get property name, for factories.
		debug("Node name for '",props.path()," is '",name,"', searching factories");

		// TODO: Rewrite init actions to use Object::Factory.
		// if(strcasecmp(props.node_name(),"init") == 0) {
		// 	Action::Factory::build(props)->call(node);
		// 	return true; // Handled by action.
		// }

		// Is it a factory?
		for(const auto factory : Factories()) {

			debug("Testing factory '",factory->c_str(),"'");
			if(*factory == name) {

				// Apend object.
				auto object = factory->ObjectFactory(props);
				push_back(props,object);

				// Append children
				object->append_children(props);

				// Parsed, return true.
				return true; 
			}
		}

		return false;	// Not handled, maybe the caller can handle it.

	}

	Value & Abstract::Object::get_properties(Value &value) const {
		Schema::Output schema;
		if(this->schema(HTTP::Get,"",schema)) {
			for(const auto &item : schema) {
				get_property(item.name(),value);
			}
		}
		return value;
	}

	int Abstract::Object::process(const Request &request, Response &response) {

		response.set(this);

		Schema::Output schema;
		if(this->schema(request.method(),request.path(),schema)) {

			// Has schema, use it
			for(const auto &item : schema) {
				if(!this->get_property(item.name(),response[item.name()])) {
					throw logic_error(String{"Property '",item.name(),"' is not available"});
				}
			}

		} else {

			// No schema, just copy properties.
			get_properties(response);

		}

		return 0;
	}

	const char * Abstract::Object::name() const noexcept {
		return "";
	}

	std::string Abstract::Object::to_string() const noexcept {
		return name();
	}

	bool Abstract::Object::set_property(const char *, const char *) {
		return false;
	}

	String Abstract::Object::get_property(const char *key, const char *def) const {

		String value;
		if(get_property(key,value)) {
			return value;
		}

		if(def) {
			return def;
		}

		throw runtime_error(Logger::Message{_("Unable to get value of '{}'"),key});

	}

	bool Abstract::Object::get_property(const char *key, std::string &value) const {

		Value val;
		if(get_property(key, val)) {
			value = val.to_string().c_str();
			return true;
		}
		return false;

	}

	bool Abstract::Object::get_property(const char *key, Udjat::Variant &value) const {
		return false;
	}

 }
