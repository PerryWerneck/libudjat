/**
 * @file src/core/state/state.cc
 *
 * @brief Implements the abstract state methods
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <config.h>
 #include <private/state.h>
 #include <cstring>
 #include <udjat/tools/xml.h>
 #include <iostream>
 #include <limits.h>
 #include <udjat/tools/parse.h>

 using namespace std;

 namespace Udjat {

	namespace XML {

		void parse(const XML::Node &node, int &from, int &to) {
			int value = node.attribute("value").as_int();
			from = node.attribute("from-value").as_int(value);
			to = node.attribute("to-value").as_int(value);
		}

		void parse(const XML::Node &node, unsigned int &from, unsigned int &to) {
			unsigned int value = node.attribute("value").as_uint();
			from = node.attribute("from-value").as_uint(value);
			to = node.attribute("to-value").as_uint(value);
		}

		void parse(const XML::Node &node, unsigned short &from, unsigned short &to) {
			unsigned int value = node.attribute("value").as_uint();
			from = (unsigned short) node.attribute("from-value").as_uint(value);
			to = (unsigned short) node.attribute("to-value").as_uint(value);
		}

		void parse(const XML::Node &node, float &from, float &to) {
			float value = node.attribute("value").as_float();
			from = node.attribute("from-value").as_float(value);
			to = node.attribute("to-value").as_float(value);
		}

		void parse(const XML::Node &node, double &from, double &to) {
			double value = node.attribute("value").as_double();
			from = node.attribute("from-value").as_double(value);
			to = node.attribute("to-value").as_double(value);
		}

		void parse(const XML::Node &node, unsigned long &from, unsigned long &to) {
			int value = node.attribute("value").as_int();
			from = node.attribute("from-value").as_int(value);
			to = node.attribute("to-value").as_int(value);
		}

		void parse(const XML::Node &node, long &from, long &to) {
			unsigned int value = node.attribute("value").as_uint();
			from = node.attribute("from-value").as_uint(value);
			to = node.attribute("to-value").as_uint(value);
		}

		void parse(const XML::Node &node, int &value) {
			value = node.attribute("value").as_int();
		}

		void parse(const XML::Node &node, unsigned int &value) {
			value = node.attribute("value").as_uint();
		}

		void parse(const XML::Node &node, unsigned short &value) {
			value = node.attribute("value").as_uint();
		}

		void parse(const XML::Node &node, float &value) {
			value = node.attribute("value").as_float();
		}

		void parse(const XML::Node &node, double &value) {
			value = node.attribute("value").as_double();
		}

		void parse(const XML::Node &node, unsigned long &value) {
			value = node.attribute("value").as_int();
		}

		void parse(const XML::Node &node, long &value) {
			value = node.attribute("value").as_uint();
		}

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

	void parse_byte_range(const XML::Node &node, unsigned long long &from, unsigned long long &to) {

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
