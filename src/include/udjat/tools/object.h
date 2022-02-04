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
 #include <pugixml.hpp>

 namespace Udjat {

	namespace Abstract {

		/// @brief Abstract object with properties.
		class UDJAT_API Object {

		public:
			/// @brief Get property from xml node and convert to const string;
			/// @param node The xml node.
			/// @param name The property name.
			/// @param def The default value.
			/// @return Attribute value converted to quark.
			static const char * getAttribute(const pugi::xml_node &node, const char *name, const char *def);

			virtual std::string to_string() const = 0;

			/// @brief Get object as Udjat::Value.
			// Udjat::Value as_value() const noexcept;

			/// @brief Get property.
			/// @param key The property name.
			/// @param value String to update with the property value.
			/// @return true if the property is valid.
			virtual bool getProperty(const char *key, std::string &value) const noexcept = 0;

			/// @brief Get property.
			/// @param key The property name.
			/// @return The property value (empty if invalid key).
			std::string operator[](const char *key) const noexcept;

			/// @brief Expand ${} tags on string.
			std::string expand(const char *text) const;

			/// @brief Add object properties to the value.
			virtual Value & getProperties(Value &value) const noexcept;

		};


	}

	/// @brief An object with name.
	class UDJAT_API NamedObject : public Abstract::Object {
	private:
		const char *objectName = "";

	protected:
		constexpr NamedObject(const char *name) : objectName(name) {}
		NamedObject(const pugi::xml_node &node);

		void set(const pugi::xml_node &node);

	public:
		bool getProperty(const char *key, std::string &value) const noexcept override;

		inline const char * name() const noexcept {
			return objectName;
		}

		Value & getProperties(Value &value) const noexcept override;

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

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
			const char * icon = STRINGIZE_VALUE_OF(PRODUCT_NAME);

		} properties;

		constexpr Object(const char *name) : NamedObject(name) {
		}

		Object(const pugi::xml_node &node);
		void set(const pugi::xml_node &node);

	public:

		bool getProperty(const char *key, std::string &value) const noexcept override;

		inline const char * label() const {
			return properties.label;
		}

		/// @brief Object summary.
		inline const char * summary() const {
			return properties.summary;
		}

		/// @brief URL associated with the object.
		inline const char * url() const {
			return properties.url;
		}

		/// @brief Name of the object icon (https://specifications.freedesktop.org/icon-naming-spec/latest/)
		inline const char * icon() const {
			return properties.icon;
		}

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

 }

