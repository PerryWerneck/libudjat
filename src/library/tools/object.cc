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
 #include <udjat/tools/quark.h>
 #include <udjat/tools/object.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/expander.h>
 #include <udjat/tools/factory.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <cstdarg>
 #include <udjat/module/abstract.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/interface.h>

 #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  
 using namespace std;

 namespace Udjat {

	static const char * NameFactory(const XML::Node &node) noexcept {

		const char *name = node.attribute("name").as_string();

		if(!(name && *name)) {
			name = node.name();
			Logger::String{"<",node.name(),"> doesn't have the required attribute 'name', using default '",name,"'"}.trace("xml");
		}

		return Quark(name).c_str();
	}

	NamedObject::NamedObject(const XML::Node &node) : NamedObject{NameFactory(node)} {
	}

	NamedObject::NamedObject(const char *name, const XML::Node &) : NamedObject{name} {
	}

	const char * NamedObject::name() const noexcept {
		return objectName;
	}

	void NamedObject::parse(const XML::Node &node) {
		if(!(objectName && *objectName)) {
			objectName = NameFactory(node);
		}
		Abstract::Object::parse(node);
	}

	const char * NamedObject::c_str() const noexcept {
		return (this->objectName ? this->objectName : "" );
	}

	size_t NamedObject::push(std::function<void()> callback) {
		return ThreadPool::getInstance().push(objectName,callback);
	}

	int NamedObject::compare(const NamedObject &object ) const {
		return strcasecmp(this->c_str(),object.c_str());
	}

	bool NamedObject::operator==(const char *name) const noexcept {
		return strcasecmp(c_str(), name) == 0;
	}

	bool NamedObject::operator==(const XML::Node &node) const noexcept {
		return strcasecmp(c_str(),node.attribute("name").as_string()) == 0;
	}

	Value & NamedObject::getProperties(Value &value) const {
		value["name"] = objectName;
		return value;
	}

	std::string NamedObject::to_string() const noexcept {
		return c_str();
	}

	std::ostream & NamedObject::trace() const {
		return Logger::trace() << objectName << "\t";
	}

	std::ostream & NamedObject::info() const {
		return cout << objectName << "\t";
	}

	std::ostream & NamedObject::warning() const {
		return clog << objectName << "\t";
	}

	std::ostream & NamedObject::error() const {
		return cerr << objectName << "\t";
	}

	std::shared_ptr<Abstract::Object> Abstract::Object::Factory(const Object *object, ...) noexcept {

		class Object : public Udjat::NamedObject {
		public:
			vector<const Udjat::Abstract::Object *> items;

			Object(const Udjat::Abstract::Object *object) : NamedObject{object->name()} {
			}

			Value & getProperties(Value &value) const override {
				for(const auto item : items) {
					item->getProperties(value);
				}
				return value;
			}

			bool getProperty(const char *key, Udjat::Value &value) const override {
				for(const auto item : items) {
					if(item->getProperty(key,value)) {
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

	void Abstract::Object::push_back(std::shared_ptr<Abstract::Object>) {
		throw logic_error("Object is unable to handle children");
	}

	void Abstract::Object::push_back(const XML::Node &, std::shared_ptr<Abstract::Object> child) {
		push_back(child);
	}

	bool Abstract::Object::parse_child(const XML::Node &node) {
		
		if(is_reserved(node) || !is_allowed(node)) {
			return true; // Ignore reserved nodes.
		}

		// It's a module?
		if(strcasecmp(node.name(),"module") == 0) {
			Module::load(node);
			return true; // Handled by module.
		}

		// It's an interface?
		if(strcasecmp(node.name(),"interface") == 0) {
			Interface::Factory::build(node);
			return true; // Handled by interface.
		}

		if(strcasecmp(node.name(),"init") == 0) {
			auto action = Action::Factory::build(node,true);
			if(action) {
				action->call(node);
			}
			return true; // Handled by action.
		}

		// Run node based factories.
		if(Udjat::Factory::for_each(node,[this,&node](Udjat::Factory &factory) {

			if(factory.NodeFactory(*this,node)) {
				return true;
			}

			return factory.NodeFactory(node);

		})) {

			return true; // Handled by factory.

		}

		// Run factories.
		if(Udjat::Factory::for_each([this,&node](Udjat::Factory &factory) {

			if(factory == node.attribute("type").as_string("default") && factory.CustomFactory(*this,node)) {
				return true;
			}

			return false;

		})) {

			return true; // Handled by factory.

		}

		return false;	// Not handled, maybe the caller can handle it.
	}

	void Abstract::Object::parse(const XML::Node &node) {

		for(XML::Node child : node) {

			if(parse_child(child)) {
				continue; // Child was handled.
			}

			const char *type = child.attribute("type").as_string("default");

			if(Factory::for_each([this,type,&child](Udjat::Factory &factory){

				if(factory == child.name()) {

					auto object = factory.ObjectFactory(*this,child);
					if(object) {
						push_back(child,object);
						return true;
					}

					if(factory.NodeFactory(*this,child)) {
						return true;
					}

					if(factory.NodeFactory(child)) {
						return true;
					}

				}

				if(factory == type && factory.CustomFactory(child)) {
					return true;
				}

				return false;

			})) {
				continue; // Handled by factory.
			};

			if(Logger::enabled(Logger::Debug)) {
				Logger::String{"Ignoring node <",node.name(),">"}.write(Logger::Debug,PACKAGE_NAME);
			}

		}

	}

	void Object::parse(const XML::Node &node) {

		properties.label = String{node,"label",properties.label}.as_quark();
		properties.summary = String{node,"summary",properties.summary}.as_quark();
		properties.url = String{node,"url",properties.url}.as_quark();
		properties.icon = String{node,"icon",properties.icon}.as_quark();

		NamedObject::parse(node);

	}

	Value & Abstract::Object::getProperties(Value &value) const {
		return value;
	}

	const char * Object::label() const noexcept {
		if(properties.label && *properties.label) {
			return properties.label;
		}
		return name();
	}

	const char * Object::icon() const noexcept {
		if(properties.icon && *properties.icon) {
			return properties.icon;
		}
		return "image-missing";
	}

	const char * Object::summary() const noexcept {
		return properties.summary;
	}

	const char * Abstract::Object::name() const noexcept {
		return "";
	}

	std::string Abstract::Object::to_string() const noexcept {
		return name();
	}

	String Abstract::Object::getProperty(const char *key, const char *def) const {

		String value;
		if(getProperty(key,value)) {
			return value;
		}

		if(def) {
			return def;
		}

		throw runtime_error(Logger::Message{_("Unable to get value of '{}'"),key});

	}

	bool Abstract::Object::getProperty(const char *, std::string &) const {
		return false;
	}

	bool Abstract::Object::getProperty(const char *key, Udjat::Value &value) const {
		std::string str;
		if(getProperty(key,str)) {
			value = str;
			return true;
		}
		return false;
	}

	Value & Object::getProperties(Value &value) const {

		NamedObject::getProperties(value);

		value["summary"] = summary();
		value["label"] = label();
		value["url"] = url();
		value["icon"].set(icon(),Value::Icon);

		return value;
	}

	bool Object::getProperty(const char *key, std::string &value) const {

		if(NamedObject::getProperty(key,value)) {
			return true;
		}

		if(!strcasecmp(key,"label")) {
			value = properties.label;
		} else if(!strcasecmp(key,"summary")) {
			value = summary();
		} else if(!strcasecmp(key,"url")) {
			value = properties.url;
		} else if(!strcasecmp(key,"icon")) {
			value = icon();
		} else {
			return false;
		}
		return true;

	}

	bool NamedObject::getProperty(const char *key, std::string &value) const {
		if(!strcasecmp(key,"name")) {
			value = objectName;
			return true;
		}
		return false;
	}

	size_t NamedObject::hash() const noexcept {
		size_t seed = 0x19670123;
		size_t hash = 0;
		for(const char *ptr = objectName;*ptr;ptr++) {
			hash = (hash * seed) + ((size_t) *ptr);
		}
		return hash;
	}

	std::ostream & Abstract::Object::info() const {
		return std::cout << name() << "\t";
	}

	std::ostream & Abstract::Object::warning() const {
		return std::clog << name() << "\t";
	}

	std::ostream & Abstract::Object::error() const {
		return std::cerr << name() << "\t";
	}

	std::ostream & Abstract::Object::trace() const {
		return Logger::trace() << name() << "\t";
	}

	void Abstract::Object::for_each(const XML::Node &root, const char *name, const char *group, const std::function<void(const XML::Node &node)> &handler) {

		for(XML::Node node = root.child(name); node; node = node.next_sibling(name)) {
			handler(node);
		}

		if(group && *group) {

			string group_name{root.name()};
			group_name += '-';
			group_name += group;

			string node_name{root.name()};
			node_name += '-';
			node_name += name;

			for(XML::Node parent = root.parent(); parent; parent = parent.parent()) {

				// Scan for nodes.
				for(XML::Node node = parent.child(node_name.c_str()); node; node = node.next_sibling(node_name.c_str())) {
					handler(node);
				}

				// Scan for groups.
				for(XML::Node grp = parent.child(group_name.c_str()); grp; grp = grp.next_sibling(group_name.c_str())) {

					for(XML::Node node = grp.child(name); node; node = node.next_sibling(name)) {
						handler(node);
					}

				}

			}

		}

	}

	const char * Abstract::Object::settings_from(const XML::Node &node, bool upstream, const char *def) {

		auto attribute = node.attribute("settings-from");
		if(attribute) {
			return attribute.as_string(def);
		}

		string attrname{node.name()};
		attrname += "-defaults-from";
		attribute = node.attribute(attrname.c_str());
		if(attribute) {
			return attribute.as_string(def);
		}

		if(upstream) {
			for(XML::Node parent = node.parent(); parent; parent = parent.parent()) {
				attribute = parent.attribute(attrname.c_str());
				if(attribute) {
					return attribute.as_string(def);
				}
			}
		}

		if(*def) {
			return def;
		}

		return Quark( (string{node.name()} + "-defaults").c_str() ).c_str();
	}

	bool Abstract::Object::for_each(const XML::Node &node, const char *tagname, const std::function<bool (const XML::Node &node)> &call) {

		bool rc = false;

		for(XML::Node n = node; n && !rc; n = n.parent()) {

			for(XML::Node child = n.child(tagname); child && !rc; child = child.next_sibling(tagname)) {

				if(is_allowed(child)) {
					rc = call(child);
				}

			}

		}

		return rc;
	}

	const XML::Attribute Abstract::Object::getAttribute(const XML::Node &n, const char *name) {
		return XML::AttributeFactory(n,name);
	}

	unsigned int Abstract::Object::getAttribute(const XML::Node &node, const char *name, unsigned int def) {
		return getAttribute(node,name).as_uint(def);
	}

	unsigned int Abstract::Object::getAttribute(const XML::Node &node, const char *group, const char *name, unsigned int def) {
		auto attribute = getAttribute(node,name);
		if(attribute) {
			return attribute.as_uint(def);
		}
		return Config::Value<unsigned int>(group,name,def);
	}

	bool Abstract::Object::getAttribute(const XML::Node &node, const char *group, const char *name, bool def) {
		auto attribute = getAttribute(node,name);
		if(attribute) {
			return attribute.as_bool(def);
		}
		return Config::Value<bool>(group,name,def);
	}

	const char * Abstract::Object::getAttribute(const XML::Node &node, const char *name, const char *def) {
		auto attribute = getAttribute(node,name);
		if(attribute) {
			return Quark(Udjat::expand(node,attribute,def)).c_str();
		}
		return def;
	}

	const char * Abstract::Object::getAttribute(const XML::Node &node, const char *group, const char *name, const char *def) {

		auto attribute = getAttribute(node,name);
		if(attribute) {
			return Quark(Udjat::String(attribute.as_string(def)).expand(node)).c_str();
		}

		if(Config::hasKey(group,name)) {
			return Quark(Udjat::String(Config::get(group,name,def)).expand(node)).c_str();
		}

		return def;
	}

	/*
	const char * Abstract::Object::expand(const XML::Node &node, const char *group, const char *value) {

		String text{value};
		text.expand([node,group](const char *key, string &value) {

			auto attribute = getAttribute(node,key);
			if(attribute) {
				value = Udjat::expand(node,attribute,"");
				return true;
			}

			if(Config::hasKey(group,key)) {
				value = Config::Value<string>(group,key,"");
				return true;
			}

			return false;

		});

		return Quark(text).c_str();

	}
	*/

	const char * Abstract::Object::getChildValue(const XML::Node &node, const char *group) {
		String text{node.child_value()};
		text.expand(node,group);
		text.strip();
		if(text.empty()) {
			return "";
		}
		return text.as_quark();
	}

	bool Abstract::Object::search(const XML::Node &node, const char *tagname, const std::function<bool(const XML::Node &node)> &call) {

		for(XML::Node nd = node; nd; nd = nd.parent()) {
			for(XML::Node child = nd.child(tagname); child; child = child.next_sibling(tagname)) {
				if(call(child)) {
					return true;
				}
			}
		}

		return false;

	}

 }
