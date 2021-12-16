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
 #include <limits.h>
 #include <udjat/state.h>

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

	void parse_range(const pugi::xml_node &node, float &from, float &to) {
		float value = node.attribute("value").as_float();
		from = node.attribute("from-value").as_float(value);
		to = node.attribute("to-value").as_float(value);
	}

	void parse_range(const pugi::xml_node &node, double &from, double &to) {
		double value = node.attribute("value").as_double();
		from = node.attribute("from-value").as_double(value);
		to = node.attribute("to-value").as_double(value);
	}

	void parse_range(const pugi::xml_node &node, unsigned long &from, unsigned long &to) {
		int value = node.attribute("value").as_int();
		from = node.attribute("from-value").as_int(value);
		to = node.attribute("to-value").as_int(value);
	}

	void parse_range(const pugi::xml_node &node, long &from, long &to) {
		unsigned int value = node.attribute("value").as_uint();
		from = node.attribute("from-value").as_uint(value);
		to = node.attribute("to-value").as_uint(value);
	}

	unsigned long long parse_byte_value(const pugi::xml_attribute &attr) {
		unsigned long long rc = 0;
		const char *str = attr.as_string();

		int bytes = sscanf(str,"%llu",&rc);
		if(bytes == EOF) {
			throw runtime_error("Unexpected byte value");
		}

		str += bytes;
		while(*str && isspace(*str))
			str++;

		if(*str) {

			static const char * names[] = { "B", "KB", "GB", "MB", "TB" };
			unsigned long long value = 1;

			for(size_t ix = 0; ix < N_ELEMENTS(names);ix++) {

				if(!strcasecmp(str,names[ix])) {
					return rc * value;
				}

				value *= 1024;

			}

			throw runtime_error("Unexpected byte unit");

		}

		return rc;
	}

	void parse_byte_range(const pugi::xml_node &node, unsigned long long &from, unsigned long long &to) {

		auto value = node.attribute("value");

		if(value) {
			from = to = parse_byte_value(value);
			return;
		}

		auto f = node.attribute("from-value");
		if(f) {
			from = parse_byte_value(f);
		} else {
			from  = 0;
		}

		auto t = node.attribute("to-value");
		if(t) {
			to = parse_byte_value(t);
		} else {
			to = ULLONG_MAX;
		}

	}


}
