
 #include <config.h>
 #include <cstring>
 #include <udjat/tools/xml.h>

 namespace Udjat {

	pugi::xml_attribute getAttribute(const pugi::xml_node &node, const char *name) {

		pugi::xml_attribute rc = node.attribute(name);

		if(!rc) {

			for(pugi::xml_node child = node.child("attribute"); child; child = child.next_sibling("attribute")) {

				if(strcasecmp(name,child.attribute("name").as_string()) == 0)
					return child.attribute("value");

			}


		}

		return rc;

	}

 };
