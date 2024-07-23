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

 #pragma once

 #include <udjat/defs.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/string.h>
 #include <memory>

 namespace Udjat {

	namespace Abstract {

		/// @brief Abstract object with properties.
		class UDJAT_API Object {
		public:

			/// @brief Setup object.
			/// @param node The XML node with the object definitions.
			virtual void setup(const XML::Node &node);

			/// @brief Add child object (if supported).
			virtual void push_back(std::shared_ptr<Abstract::Object> child);

			/// @brief Add child object with XML definitions (if supported).
			virtual void push_back(const XML::Node &node, std::shared_ptr<Abstract::Object> child);

			/// @brief Get configuration file group.
			static const char * settings_from(const XML::Node &node,bool upstream = true,const char *def = "");

			/// @brief Call method on every ocorrence of 'tagname' until method returns 'true'.
			/// @param node The xml node.
			/// @param tagname The tagname.
			/// @return true if method has returned 'true'.
			static bool for_each(const XML::Node &node, const char *tagname, const std::function<bool (const XML::Node &node)> &call);

			/// @brief Navigate thru XML nodes, including groups.
			/// @param node The XML node to start search.
			/// @param name The child node name.
			/// @param group The child group node name, usually the plural of name (optional).
			/// @param handler The handler for children.
			static void for_each(const XML::Node &node, const char *name, const char *group, const std::function<void(const XML::Node &node)> &handler);

			/// @brief Navigate thru <tagname> nodes on current and parent nodes until lambda returns 'true'.
			/// @param node The starting point.
			/// @param tagname the xml tag to scan.
			/// @param call lambda to be called on every node.
			/// @return true if the lambda has returned true.
			static bool search(const XML::Node &node, const char *tagname, const std::function<bool(const XML::Node &node)> &call);

			/// @brief Get property from xml node and convert to const string.
			/// @param node The xml node.
			/// @param name The property name.
			/// @param change If true add the node name as prefix on the attribute name for upsearch.
			/// @return XML Attribute.
			static const XML::Attribute getAttribute(const XML::Node &n, const char *name);

			/// @brief Get property from xml node and convert to const string.
			/// @param node The xml node.
			/// @param name The property name.
			/// @param def The default value (should be constant).
			/// @return Attribute value converted to quark or def
			static const char * getAttribute(const XML::Node &node, const char *name, const char *def);

			/// @brief Get child value from xml node and convert to const string.
			/// @param node The xml node.
			/// @param group The configuration group name.
			/// @return child value converted to quark.
			static const char * getChildValue(const XML::Node &node, const char *group);

			/// @brief Get property from xml node with fallback to configuration file.
			/// @param node The xml node.
			/// @param group The configuration group name.
			/// @param name The property name.
			/// @param def The default value (should be constant).
			/// @return Attribute value converted to quark or def
			static const char * getAttribute(const XML::Node &node, const char *group, const char *name, const char *def);

			/// @brief Get property from xml node.
			/// @param node The xml node.
			/// @param name The property name.
			/// @param def The default value.
			/// @return Attribute value.
			static unsigned int getAttribute(const XML::Node &node, const char *name, unsigned int def);

			/// @brief Get property from xml node with fallback to configuration file.
			/// @param node The xml node.
			/// @param group The configuration group name.
			/// @param name The property name.
			/// @param def The default value.
			/// @return Attribute value.
			static unsigned int getAttribute(const XML::Node &node, const char *group, const char *name, unsigned int def);
			static bool getAttribute(const XML::Node &node, const char *group, const char *name, bool def);

			static inline unsigned int getAttribute(const XML::Node &node, const std::string &group, const char *name, unsigned int def) {
				return getAttribute(node,group.c_str(),name,def);
			}

			/// @brief Expand string using XML definitions and configuration file.
			/// @param node Reference node.
			/// @param group Configuration file group to get values.
			/// @param value String to expand.
			/// @return 'quarked' string with the expanded value.
			static const char * expand(const XML::Node &node, const char *group, const char *value);

			static inline const char * expand(const XML::Node &node, const std::string &group, const char *value) {
				return expand(node,group.c_str(),value);
			}

			virtual const char * name() const noexcept;

			inline bool operator ==(const char * obj) const noexcept {
				return strcasecmp(name(),obj) == 0;
			}

			inline bool operator ==(const Object &object) const noexcept {
				return strcasecmp(name(),object.name()) == 0;
			}

			inline bool operator ==(const Object *object) const noexcept {
				return strcasecmp(name(),object->name()) == 0;
			}

			inline bool operator < (const Object &object) const noexcept {
				return strcasecmp(name(),object.name()) < 0;
			}

			inline bool operator < (const Object *object) const noexcept {
				return strcasecmp(name(),object->name()) < 0;
			}

			inline bool operator > (const Object &object) const noexcept {
				return strcasecmp(name(),object.name()) > 0;
			}

			inline bool operator > (const Object *object) const noexcept {
				return strcasecmp(name(),object->name()) > 0;
			}

			virtual std::string to_string() const noexcept;

			/// @brief Get property value.
			/// @param key The property name.
			/// @param value String to update with the property value.
			/// @return true if the property is valid.
			virtual bool getProperty(const char *key, std::string &value) const;

			/// @brief Get property value.
			/// @param key The property name.
			/// @param value Object to receive the value.
			/// @return true if the property is valid and value was updated.
			virtual bool getProperty(const char *key, Udjat::Value &value) const;

			/// @brief Get property value.
			/// @param key The property name.
			/// @param def Default value (nullptr if the property is required).
			/// @return The property value or def.
			String getProperty(const char *key, const char *def = nullptr) const;

			/// @brief Get property.
			/// @param key The property name.
			/// @return The property value (empty if unable to get the propery).
			inline String operator[](const char *key) const {
				return getProperty(key,"");
			}

			/// @brief Expand ${} tags using object properties.
			/// @param text Text to expand.
			/// @param dynamic if true expands the dynamic values like ${timestamp(format)}.
			/// @param cleanup if true put an empty string in the non existant attributes.
			/// @return String with the known ${} tags expanded.
			std::string expand(const char *text, bool dynamic = false, bool cleanup = false) const;

			/// @brief Expand ${} tags using object properties.
			/// @param text Text to expand.
			/// @param dynamic if true expands the dynamic values like ${timestamp(format)}.
			/// @param cleanup if true put an empty string in the non existant attributes.
			void expand(std::string &text, bool dynamic = false, bool cleanup = false) const;

			/// @brief Add object properties to the value.
			virtual Value & getProperties(Value &value) const;

			std::ostream & info() const;
			std::ostream & warning() const;
			std::ostream & error() const;
			std::ostream & trace() const;

		};


	}

 }

 namespace std {

	template <>
	struct hash<Udjat::Abstract::Object> {
		inline size_t operator() (const Udjat::Abstract::Object &obj) const {
			return std::hash<const char *>{}(obj.name());
		}
	};

 }


