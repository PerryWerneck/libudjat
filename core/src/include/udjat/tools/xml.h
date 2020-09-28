#ifndef UDJAT_XML_H_INCLUDED

	#define UDJAT_XML_H_INCLUDED

	#include <pugixml.hpp>

	namespace Udjat {

		pugi::xml_attribute getAttribute(const pugi::xml_node &node, const char *name);

	};


#endif // UDJAT_XML_H_INCLUDED
