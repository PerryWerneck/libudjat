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
 #include <udjat/tools/abstract/object.h>
 #include <ostream>
 #include <string>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <cstring>
 #include <functional>

 namespace Udjat {

	/// @brief An object with name.
	class UDJAT_API NamedObject : public Abstract::Object {
	private:
		const char *objectName = "";

	protected:

		NamedObject(const char *name, const XML::Node &node);
		NamedObject(const XML::Node &node);

		/// @brief Set object properties from XML node.
		/// @param node XML node for the object properties
		/// @return true if the value was updated.
		void parse(const XML::Node &node) override;

		inline void rename(const char *name) {
			objectName = name;
		}

		typedef Abstract::Object Super;

	public:

		constexpr NamedObject(const char *name = "") : objectName(name) {}

		bool getProperty(const char *key, std::string &value) const override;

		/// @brief Push a background task.
		/// @param callback Task method.
		size_t push(std::function<void()> callback);

		int compare(const NamedObject &object ) const;

		inline bool empty() const {
			return !(objectName && *objectName);
		}

		const char * name() const noexcept override;

		bool operator==(const char *name) const noexcept;
		bool operator==(const XML::Node &node) const noexcept;
		size_t hash() const noexcept;

		const char * c_str() const noexcept;

		std::string to_string() const noexcept override;

		Value & getProperties(Value &value) const;

		std::ostream & trace() const;
		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;

		template<typename... Targs>
		inline void trace(const char *fmt, Targs... Fargs) const {
			Logger::Message{fmt, Fargs...}.trace(objectName);
		}

		template<typename... Targs>
		inline void info(const char *fmt, Targs... Fargs) const {
			Logger::Message{fmt, Fargs...}.info(objectName);
		}

		template<typename... Targs>
		inline void warning(const char *fmt, Targs... Fargs) const {
			Logger::Message{fmt, Fargs...}.warning(objectName);
		}

		template<typename... Targs>
		inline void error(const char *fmt, Targs... Fargs) const {
			Logger::Message{fmt, Fargs...}.error(objectName);
		}

	};

	/// @brief An object with common properties.
	class UDJAT_API Object : public NamedObject {
	protected:

		typedef NamedObject Super;

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

		Object(const XML::Node &node) : NamedObject(node) {
		}

		Object(const char *name, const XML::Node &node) : NamedObject(name, node) {
		}

		void parse(const XML::Node &node) override;

		inline time_t parse(const char *path) {
			return Abstract::Object::parse(path);
		}

	public:

		constexpr Object(const char *name) : NamedObject(name) {
		}

		bool getProperty(const char *key, std::string &value) const override;

		virtual const char * label() const noexcept;

		/// @brief Object summary.
		virtual const char * summary() const noexcept;

		/// @brief URL associated with the object.
		inline const char * url() const noexcept {
			return properties.url;
		}

		/// @brief Name of the object icon (https://specifications.freedesktop.org/icon-naming-spec/latest/)
		virtual const char * icon() const noexcept;

		/// @brief Export all object properties.
		/// @param Value to receive the properties.
		/// @return Pointer to value (for reference).
		Value & getProperties(Value &value) const;
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

