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
 #include <udjat/tools/value.h>
 #include <ostream>
 #include <string>
 #include <udjat/tools/xml.h>
 #include <cstring>
 #include <functional>

 namespace Udjat {

	namespace Abstract {

		/// @brief Abstract object with properties.
		class UDJAT_API Object {
		public:

			/// @brief Setup object.
			/// @param node The XML node with the object definitions.
			virtual void setup(const pugi::xml_node &node);

			/// @brief Get configuration file group.
			static const char * settings_from(const XML::Node &node,bool upstream = true,const char *def = "");

			/// @brief Call method on every ocorrence of 'tagname' until method returns 'true'.
			/// @param node The xml node.
			/// @param tagname The tagname.
			/// @return true if method has returned 'true'.
			static bool for_each(const pugi::xml_node &node, const char *tagname, const std::function<bool (const pugi::xml_node &node)> &call);

			/// @brief Navigate thru XML nodes, including groups.
			/// @param node The XML node to start search.
			/// @param name The child node name.
			/// @param group The child group node name, usually the plural of name (optional).
			/// @param handler The handler for children.
			static void for_each(const pugi::xml_node &node, const char *name, const char *group, const std::function<void(const pugi::xml_node &node)> &handler);

			/// @brief Get property from xml node and convert to const string.
			/// @param node The xml node.
			/// @param name The property name.
			/// @param change If true add the node name as prefix on the attribute name for upsearch.
			/// @return XML Attribute.
			static const pugi::xml_attribute getAttribute(const pugi::xml_node &n, const char *name, bool change = true);

			/// @brief Get property from xml node and convert to const string.
			/// @param node The xml node.
			/// @param name The property name.
			/// @param def The default value (should be constant).
			/// @return Attribute value converted to quark or def
			static const char * getAttribute(const pugi::xml_node &node, const char *name, const char *def);

			/// @brief Get property from xml node with fallback to configuration file.
			/// @param node The xml node.
			/// @param group The configuration group name.
			/// @param name The property name.
			/// @param def The default value (should be constant).
			/// @return Attribute value converted to quark or def
			static const char * getAttribute(const pugi::xml_node &node, const char *group, const char *name, const char *def);

			/// @brief Get property from xml node.
			/// @param node The xml node.
			/// @param name The property name.
			/// @param def The default value.
			/// @return Attribute value.
			static unsigned int getAttribute(const pugi::xml_node &node, const char *name, unsigned int def);

			/// @brief Get property from xml node with fallback to configuration file.
			/// @param node The xml node.
			/// @param group The configuration group name.
			/// @param name The property name.
			/// @param def The default value.
			/// @return Attribute value.
			static unsigned int getAttribute(const pugi::xml_node &node, const char *group, const char *name, unsigned int def);
			static bool getAttribute(const pugi::xml_node &node, const char *group, const char *name, bool def);

			static inline unsigned int getAttribute(const pugi::xml_node &node, const std::string &group, const char *name, unsigned int def) {
				return getAttribute(node,group.c_str(),name,def);
			}

			/// @brief Expand string using XML definitions and configuration file.
			/// @param node Reference node.
			/// @param group Configuration file group to get values.
			/// @param value String to expand.
			/// @return 'quarked' string with the expanded value.
			static const char * expand(const pugi::xml_node &node, const char *group, const char *value);

			static inline const char * expand(const pugi::xml_node &node, const std::string &group, const char *value) {
				return expand(node,group.c_str(),value);
			}

			virtual std::string to_string() const = 0;

			/// @brief Get property value.
			/// @param key The property name.
			/// @param value String to update with the property value.
			/// @return true if the property is valid.
			virtual bool getProperty(const char *key, std::string &value) const noexcept = 0;

			/// @brief Get property.
			/// @param key The property name.
			/// @return The property value (empty if invalid key).
			std::string operator[](const char *key) const noexcept;

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
			/// @return String with the known ${} tags expanded.
			void expand(std::string &text, bool dynamic = false, bool cleanup = false) const;

			/// @brief Add object properties to the value.
			virtual Value & getProperties(Value &value) const noexcept;

		};


	}

	/// @brief An object with name.
	class UDJAT_API NamedObject : public Abstract::Object {
	private:
		const char *objectName = "";

	protected:

		constexpr NamedObject(const char *name = "") : objectName(name) {}
		NamedObject(const pugi::xml_node &node);

		/// @brief Set object properties from XML node.
		/// @param node XML node for the object properties
		/// @return true if the value was updated.
		bool set(const pugi::xml_node &node);

		inline void rename(const char *name) {
			objectName = name;
		}

		typedef NamedObject Super;

	public:

		bool getProperty(const char *key, std::string &value) const noexcept override;

		/// @brief Push a background task.
		/// @param callback Task method.
		size_t push(std::function<void()> callback);

		int compare(const NamedObject &object ) const;

		inline bool empty() const {
			return !(objectName && *objectName);
		}

		inline const char * name() const noexcept {
			return objectName;
		}

		bool operator==(const char *name) const noexcept;
		bool operator==(const pugi::xml_node &node) const noexcept;
		size_t hash() const noexcept;

		const char * c_str() const noexcept;

		std::string to_string() const override;

		Value & getProperties(Value &value) const noexcept override;

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;
		std::ostream & trace() const;

	};

	/// @brief An object with common properties.
	class UDJAT_API Object : public NamedObject {
	protected:

		struct Properties {

			/// @brief Object label.
			const char * label = "";

			/// @brief Object summary.
			const char * summary = "";

			/// @brief URL associated with the object.
			const char * url = "";

			/// @brief Name of the object icon (https://specifications.freedesktop.org/icon-naming-spec/latest/)
			const char * icon = "";

		} properties;

		constexpr Object(const char *name) : NamedObject(name) {
		}

		Object(const pugi::xml_node &node);
		void set(const pugi::xml_node &node);

	public:

		bool getProperty(const char *key, std::string &value) const noexcept override;

		inline const char * label() const noexcept {
			return properties.label;
		}

		/// @brief Object summary.
		inline const char * summary() const noexcept {
			return properties.summary;
		}

		/// @brief URL associated with the object.
		inline const char * url() const noexcept {
			return properties.url;
		}

		/// @brief Name of the object icon (https://specifications.freedesktop.org/icon-naming-spec/latest/)
		virtual const char * icon() const noexcept;

		/// @brief Export all object properties.
		/// @param Value to receive the properties.
		/// @return Pointer to value (for reference).
		Value & getProperties(Value &value) const noexcept override;
	};

 }

 namespace std {

	inline string to_string(const Udjat::Abstract::Object &object) {
		return object.to_string();
	}

	inline ostream& operator<< (ostream& os, const Udjat::Abstract::Object &object) {
			return os << object.to_string();
	}

	template<>
	struct hash<Udjat::NamedObject> {
		size_t operator() (const Udjat::NamedObject &object) const {
			return object.hash();
		}
	};

 }

