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

 #include <config.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <udjat/tools/url.h>
 #include <cstring>

 using namespace std;
 using namespace pugi;

 namespace Udjat {

	XML::Attribute XML::AttributeFactory(const XML::Node &node, const char *attrname) {

		XML::Attribute attribute{node.attribute(attrname)};
		if(attribute) {
			debug("Found '",attrname,"' in node '",node.name(),"'");
			return attribute;
		}

		// Search on node children for <attribute name='${attrname}' value= />
		for(XML::Node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {
			if(!strcasecmp(child.attribute("name").as_string(""),attrname) && is_allowed(child)) {
				return child.attribute("value");
			}
		}

		// Search parents from <attribute name='${node.name()}-${attrname}' value= />
		{
			String key{node.name(),"-",attrname};
			for(XML::Node parent = node.parent();parent;parent = parent.parent()) {
				for(XML::Node child = parent.child("attribute"); child; child = child.next_sibling("attribute")) {
					if(!strcasecmp(child.attribute("name").as_string(""),key.c_str()) && is_allowed(child)) {
						return child.attribute("value");
					}
				}
			}
		}

		// Search parents for <attribute name='${attrname}' value= />
		for(XML::Node parent = node.parent();parent;parent = parent.parent()) {
			for(XML::Node child = parent.child("attribute"); child; child = child.next_sibling("attribute")) {
				if(!strcasecmp(child.attribute("name").as_string(""),attrname) && is_allowed(child)) {
					return child.attribute("value");
				}
			}
		}

		return XML::Attribute{};

	}

	const char * XML::StringFactory(const XML::Node &node, const char *attrname, const char *def) {

		XML::Attribute attr{AttributeFactory(node,attrname)};
		if(attr) {
			return attr.as_string();
		}

		if(!def) {
			throw runtime_error(Logger::Message(_("Required attribute '{}' is missing"),attrname));
		}

		return def;
	}

	UDJAT_API bool XML::test(const XML::Node &node, const char *attrname, bool defvalue) {

		XML::Attribute attr{AttributeFactory(node,attrname)};
		if(!attr) {
			return defvalue;
		}

		const char *str = attr.as_string("");
		if(str && *str && strstr(str,"://")) {
			// It's an URL, test it.
			return URL{str}.test();
		}

		if(!strcasecmp(str,"only-on-virtual-machine")) {
#ifdef HAVE_VMDETECT
		return VirtualMachine{Logger::enabled(Logger::Debug)};
#else
		Logger::String{"Library built without virtual machine support, ignoring '",str,"' attribute"}.warning(PACKAGE_NAME);
		return defvalue;
#endif
		}

#ifdef _WIN32
		if(!strcasecmp(str,"only-on-windows")) {
			return true;
		}
		if(!strcasecmp(str,"only-on-linux")) {
			return false;
		}
#else
		if(!strcasecmp(str,"only-on-windows")) {
			return false;
		}
		if(!strcasecmp(str,"only-on-linux")) {
			return true;
		}
#endif // _WIN32

		return attr.as_bool(defvalue);
	}

	const char * XML::QuarkFactory(const XML::Node &node, const char *attrname, const char *def) {
		return Quark{StringFactory(node,attrname,def)}.c_str();
	}

 }

