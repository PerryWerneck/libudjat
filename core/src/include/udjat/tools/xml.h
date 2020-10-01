/**
 * @file src/include/udjat/agent.h
 *
 * @brief Declare the agent classes
 *
 * @author perry.werneck@gmail.com
 *
 */

#ifndef UDJAT_TOOLS_XML_H_INCLUDED

	#define UDJAT_TOOLS_XML_H_INCLUDED

	#include <string>
	#include <pugixml.hpp>
	#include <memory>
	#include <vector>
	#include <mutex>
	#include <functional>
	#include <udjat/defs.h>
	#include <udjat/state.h>
	#include <udjat/tools/atom.h>
	#include <udjat/request.h>
	#include <json/value.h>
	#include <cstring>

	namespace Udjat {

		pugi::xml_attribute getAttribute(const pugi::xml_node &node, const char *name);

		/// @brief Wrapper for XML attribute
		class UDJAT_API Attribute : public pugi::xml_attribute {
		public:
			Attribute(const pugi::xml_node &node, const char *name) : pugi::xml_attribute(node.attribute(name)) {
			}

			operator uint32_t() const {
				return as_uint();
			}

			operator int32_t() const {
				return as_int();
			}

			operator bool() const {
				return as_bool();
			}

		};


	}


#endif // UDJAT_TOOLS_XML_H_INCLUDED
