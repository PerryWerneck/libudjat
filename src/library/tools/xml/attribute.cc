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

 #ifdef HAVE_VMDETECT
	#include <vmdetect/virtualmachine.h>
 #endif // HAVE_VMDETECT

 #ifndef _WIN32
	#include <grp.h>
	#include <sys/types.h>
	#include <pwd.h>
 #endif // _WIN32

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

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

		// Search parents
		{
			String key{node.name(),"-",attrname};
			// debug("Searching for '",key,"' in parents of node '",node.name(),"'");
			
			for(XML::Node parent = node.parent();parent;parent = parent.parent()) {

				// Search on parent node attributes
				{
					XML::Attribute attribute{node.attribute(attrname)};
					if(attribute) {
						debug("Found '",attrname,"' in node '",node.name(),"'");
						return attribute;
					}
				}

				{
					XML::Attribute attribute{node.attribute(key.c_str())};
					if(attribute) {
						debug("Found '",attrname,"' in node '",node.name(),"'");
						return attribute;
					}
				}

				// Search on <attribute> nodes.
				for(XML::Node child = parent.child("attribute"); child; child = child.next_sibling("attribute")) {
					
					if(!strcasecmp(child.attribute("name").as_string(""),key.c_str()) && is_allowed(child)) {
						return child.attribute("value");
					}

					if(!strcasecmp(child.attribute("name").as_string(""),attrname) && is_allowed(child)) {
						return child.attribute("value");
					}
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

		bool allow = true;

		XML::Attribute attr{AttributeFactory(node,attrname)};
		if(!attr) {
			return defvalue;
		}

		const char *str = attr.as_string("");
		if(!str && *str) {
			Logger::String{"Invalid XML filter on attribute '",attrname,"', ignoring"}.warning(PACKAGE_NAME);
			return defvalue;
		}

		if(*str == '!') {
			str++;
			allow = !allow;
		} else if(!strncasecmp(str,"not ",4)) {
			str += 4;
			allow = !allow;
			while(*str && isspace(*str)) {
				str++;
			}
		}

		if(strstr(str,"://")) {
			// It's an URL, test it.
			return (URL{str}.test() == 200) ? allow : !allow;
		}

		if(!(strcasecmp(str,"only-on-virtual-machine") && strcasecmp(str,"virtual-machine"))) {
#ifdef HAVE_VMDETECT
			return VirtualMachine{Logger::enabled(Logger::Debug)} ? allow : !allow;
#else
			Logger::String{"Library built without virtual machine support, ignoring '",str,"' attribute"}.warning(PACKAGE_NAME);
			return defvalue;
#endif
		}

#ifdef _WIN32
		if(!(strcasecmp(str,"only-on-windows") && strcasecmp(str,"windows"))) {
			debug("Got windows filter (",(allow ? "allow" : "deny"));
			return allow;
		}
		if(!(strcasecmp(str,"only-on-linux") && strcasecmp(str,"linux"))){
			debug("Got linux filter (",(allow ? "allow" : "deny"));
			return !allow;
		}
#else
		if(!(strcasecmp(str,"only-on-windows") && strcasecmp(str,"windows"))) {
			debug("Got windows filter (",(allow ? "allow" : "deny"));
			return !allow;
		}
		if(!(strcasecmp(str,"only-on-linux") && strcasecmp(str,"linux"))){
			debug("Got linux filter (",(allow ? "allow" : "deny"));
			return allow;
		}

		if(!strncasecmp(str,"groups:",6)) {

			auto names=String{str+6}.split(",");

			struct passwd *pw = getpwuid(getuid());
			if(!pw) {
				Logger::String{"Cant get current user groups"}.warning(PACKAGE_NAME);
				return false;
			}

			int ngroups = 0;
			getgrouplist(pw->pw_name, pw->pw_gid, NULL, &ngroups);
			if(!ngroups) {
				Logger::String{"User group list is empty"}.warning(PACKAGE_NAME);
				return false;
			}

			gid_t groups[ngroups];
			getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);

			for (int i = 0; i < ngroups; i++){
				struct group* gr = getgrgid(groups[i]);
				if(gr == NULL){
					Logger::String{"getgrgid error: ",strerror(errno)}.warning(PACKAGE_NAME);
					continue;
				}
				for(const auto &name : names) {
					if(!strcasecmp(name.c_str(),gr->gr_name)) {
						return allow;
					}
				}
			}

			return !allow;
		}

#endif // _WIN32

		return attr.as_bool(defvalue);
	}

	const char * XML::QuarkFactory(const XML::Node &node, const char *attrname, const char *def) {
		return String{node,attrname,def}.as_quark();
	}

 }

