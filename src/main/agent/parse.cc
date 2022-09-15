/**
 * @file src/core/state/state.cc
 *
 * @brief Implements the abstract state methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/agent.h>
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/converters.h>
 #include <iostream>
 #include <limits.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	/*
	void to_value(const pugi::xml_node &node, int &value) {
		value = node.attribute("value").as_int();
	}

	void to_value(const pugi::xml_node &node, unsigned int &value) {
		value = node.attribute("value").as_uint();
	}

	void to_value(const pugi::xml_node &node, unsigned short &value) {
		value = node.attribute("value").as_uint();
	}

	void to_value(const pugi::xml_node &node, float &value) {
		value = node.attribute("value").as_float();
	}

	void to_value(const pugi::xml_node &node, double &value) {
		value = node.attribute("value").as_double();
	}

	void to_value(const pugi::xml_node &node, unsigned long &value) {
		value = node.attribute("value").as_int();
	}

	void to_value(const pugi::xml_node &node, long &value) {
		value = node.attribute("value").as_uint();
	}
	*/

}
