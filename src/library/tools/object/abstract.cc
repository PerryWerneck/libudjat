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
 #include <udjat/tools/response.h>
 #include <udjat/tools/file/path.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/schema.h>

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

	bool Abstract::Object::output_schema(const char *, Schema &) const noexcept {
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

			bool get_property(const char *key, Udjat::Value &value) const override {
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
		debug("Node name for '",props.path()," is '",name,"'");

		// TODO: Rewrite init actions to use Object::Factory.
		// if(strcasecmp(props.node_name(),"init") == 0) {
		// 	Action::Factory::build(props)->call(node);
		// 	return true; // Handled by action.
		// }

		// Is it a factory?
		for(const auto factory : Factories()) {

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
		return value;
	}

	int Abstract::Object::call(const Request &, Response &response) {
		get_properties(response);
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

	bool Abstract::Object::get_property(const char *, std::string &) const {
		return false;
	}

	bool Abstract::Object::get_property(const char *key, Udjat::Value &value) const {
		std::string str;
		if(get_property(key,str)) {
			value = str;
			return true;
		}
		return false;
	}

	const char * Abstract::Object::settings_from(const Properties &node, bool upstream, const char *def) {

		throw runtime_error("Refactor incomplete");

		// auto attribute = node.pugi::xml_node::attribute("settings-from");
		// if(attribute) {
		// 	return attribute.as_string(def);
		// }

		// string attrname{node.name()};
		// attrname += "-defaults-from";
		// attribute = node.pugi::xml_node::attribute(attrname.c_str());
		// if(attribute) {
		// 	return attribute.as_string(def);
		// }

		// if(upstream) {
		// 	for(XML::Node parent = node.parent(); parent; parent = parent.parent()) {
		// 		attribute = parent.pugi::xml_node::attribute(attrname.c_str());
		// 		if(attribute) {
		// 			return attribute.as_string(def);
		// 		}
		// 	}
		// }

		// if(*def) {
		// 	return def;
		// }

		// return Quark( (string{node.name()} + "-defaults").c_str() ).c_str();
	}

	// bool Abstract::Object::for_each(const Properties &props, const char *tagname, const std::function<bool (const Properties &props)> &call) {

	// 	bool rc = false;

	// 	for(XML::Node n = node; n && !rc; n = n.parent()) {

	// 		for(XML::Node child = n.child(tagname); child && !rc; child = child.next_sibling(tagname)) {

	// 			if(is_allowed(child)) {
	// 				rc = call(child);
	// 			}

	// 		}

	// 	}

	// 	return rc;
	// }

	time_t Abstract::Object::load(const char *p) {

		time_t next = 0;

		File::Path path{XML::PathFactory(p)};
		if(path.dir()) {

			// Is a directory, scan for files
			Logger::String{"Loading xml definitions from directory '",path.c_str(),"'"}.trace(name());

			std::vector<std::string> files;
			path.for_each("*.xml",[&files](const File::Path &path) -> bool {
				files.emplace_back(path.c_str());
				return false;
			});

			std::sort(files.begin(), files.end());

			for(const auto &file : files) {

				// Recursive call to parse document.
				time_t expires = load(file.c_str());
				if(expires) {
					expires += time(0);
					if(expires < next || next == 0) {
						next = expires;
					}
				}

			}

		} else {

			// Is a file, load it
			Logger::String{"Loading xml definitions from file '",path.c_str(),"'"}.info(name());

			XML::Document document{path.c_str()};
			XML::Node root{document.document_element()};
			
			next = TimeStamp{root,"update-timer"};
			if(next) {
				next += time(0);
			}

			append_children(root);

		}

#ifdef DEBUG 
		if(next) {
			debug("Next update in ",TimeStamp{next}.to_string());
		} else {
			debug("No next update defined");
		}
#endif // DEBUG		

		return next;

	}

 }
