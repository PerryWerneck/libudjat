/**
 * @file src/core/state/state.cc
 *
 * @brief Implements the abstract state methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include "private.h"
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <iostream>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void parse_range(const pugi::xml_node &node, int &from, int &to) {
		int value = node.attribute("value").as_int();
		from = node.attribute("from-value").as_int(value);
		to = node.attribute("to-value").as_int(value);
	}

	void parse_range(const pugi::xml_node &node, unsigned int &from, unsigned int &to) {
		unsigned int value = node.attribute("value").as_uint();
		from = node.attribute("from-value").as_uint(value);
		to = node.attribute("to-value").as_uint(value);
	}

	void parse_range(const pugi::xml_node &node, unsigned short &from, unsigned short &to) {
		unsigned int value = node.attribute("value").as_uint();
		from = (unsigned short) node.attribute("from-value").as_uint(value);
		to = (unsigned short) node.attribute("to-value").as_uint(value);
	}


}
