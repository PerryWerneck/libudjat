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
 #include <ostream>
 #include <string>
 #include <udjat/tools/properties.h>
 #include <udjat/tools/logger.h>
 #include <cstring>
 #include <functional>
 #include <memory>

 namespace Udjat {

	namespace Abstract {

		/// @brief Abstract object with properties.
		class UDJAT_API Object {
		protected:
			typedef Abstract::Object Super;

		public:

			class UDJAT_API Factory {
			private:
				const char *name;

			public:
				Factory(const char *name);
				virtual ~Factory();

				inline bool operator==(const char *n) const noexcept {
					return strcasecmp(n,name) == 0;
				}

				inline const char *c_str() const noexcept {
					return name;
				}

				virtual std::shared_ptr<Abstract::Object> ObjectFactory(const Udjat::Properties &props) const = 0;

			};

			/// @brief Merge several objects propertie into a single one.
			/// @details This method is used to merge the properties several objects into a single one.
			/// @param object first object to merge.
			/// @param  ... The other objects to merge.
			/// @return Pointer to new object combining all properties.
			/// @note The first object is used as the name for the new object.
			static std::shared_ptr<Object> merge(const Object *object, ...) noexcept __attribute__ ((sentinel));

			virtual ~Object();

			/// @brief Parse file(s), build children.
			/// @param path The path for a folder or file, nullptr for default.
			/// @return timestamp for next refresh.
			time_t load(const char *path = nullptr);

			/// @brief Append child object from properties.
			/// @details This method is called by parse_children() for every child node.
			/// @param props The child properties.
			/// @return true if the node was parsed and should be ignored by the caller.
			virtual bool append_child(const Properties &props);

			/// @brief Enumarate children from props, call append_child for every one.
			/// @param props The root property.
			virtual void append_children(const Properties &props);

			/// @brief Add child object (if supported).
			/// @return True if the object was inserted.
			/// @retval true The object was inserted.
			/// @retval false The object type is not supported.	
			virtual bool push_back(std::shared_ptr<Abstract::Object> child);

			/// @brief Add child object with properties (if supported).
			/// @return True if the object was inserted.
			/// @retval true The object was inserted.
			/// @retval false The object type is not supported.	
			virtual bool push_back(const Properties &props, std::shared_ptr<Abstract::Object> child);

			/// @brief Retrieves the schema definition for interface HTTP request.
			/// @param[in] path The path for required object on interface.
			/// @param[out] s Object populated with the interface http schema details.
			/// @return True if the interface defines an http schema; false otherwise (schema remains unmodified).
			virtual bool schema(const char *path, HTTPSchema &s) const noexcept;

			/// @brief Retrieves the schema definition for the interface inputs.
			/// @param[in] path The path for required object on interface.
			/// @param[out] s Object populated with the interface input schema details.
			/// @return True if the interface defines an input schema; false otherwise (schema remains unmodified).
			virtual bool schema(const char *path, InputSchema &s) const noexcept;

			/// @brief Retrieves the schema definition for the object outputs.
			/// @param path The request path for schema.
			/// @param[out] schema Object populated with the output schema details.
			/// @return True if the object defines an output schema; false otherwise (schema remains unmodified).
			virtual bool schema(const char *path, OutputSchema &s) const noexcept;

			virtual const char * name() const noexcept;

#if __cplusplus >= 202002L

			inline auto operator <=>(const char *obj) const noexcept {
				return strcasecmp(name(),obj);
			}

			inline auto operator <=>(const Object &object) const noexcept {
				return strcasecmp(name(),object.name());
			}

			inline auto operator <=>(const Object *object) const noexcept {
				return strcasecmp(name(),object->name());
			}

#else

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

#endif

			virtual std::string to_string() const noexcept;

			/// @brief Set property
			/// @param key The property name.
			/// @param value The property value.
			/// @return true if the property is valid.
			/// @retval true The property is valid and was updated.
			/// @retval false The property was not found.
			virtual bool set_property(const char *key, const char *value);

			/// @brief Get property as string.
			/// @param key The property name.
			/// @param value String to update with the property value.
			/// @return true if the property is valid.
			virtual bool get_property(const char *key, std::string &value) const;

			/// @brief Get property value.
			/// @param key The property name.
			/// @param value Object to receive the value.
			/// @return true if the property is valid and value was updated.
			virtual bool get_property(const char *key, Udjat::Value &value) const;

			/// @brief Get property value.
			/// @param key The property name.
			/// @param def Default value (nullptr if the property is required).
			/// @return The property value or def.
			String get_property(const char *key, const char *def = nullptr) const;

			/// @brief Get property.
			/// @param key The property name.
			/// @return The property value (empty if unable to get the propery).
			inline String operator[](const char *key) const {
				return get_property(key,"");
			}

			/// @brief Add object properties to the value.
			virtual Value & get_properties(Value &value) const;

			virtual int process(const Request &request, Response &response);

		};

	}

	/// @brief An object with name.
	class UDJAT_API NamedObject : public Abstract::Object {
	private:
		const char *objectName = "";

	protected:

		NamedObject(const char *name, const Properties &props);
		NamedObject(const Udjat::Properties &props);

		inline void rename(const char *name) {
			objectName = name;
		}

		typedef Abstract::Object Super;

	public:

		constexpr NamedObject(const char *name = "") : objectName{name} {}

		bool get_property(const char *key, std::string &value) const override;
		bool get_property(const char *key, Value &value) const override;

		/// @brief This object has a name?
		/// @return true if the object is named.
		inline bool named() const noexcept {
			return objectName && *objectName;
		}

		/// @brief Push a background task.
		/// @param callback Task method.
		size_t push(std::function<void()> callback);

		int compare(const NamedObject &object ) const;

		inline bool empty() const {
			return !(objectName && *objectName);
		}

		const char * name() const noexcept override;

		bool operator==(const char *name) const noexcept;
		bool operator==(const Properties &props) const noexcept;

		const char * c_str() const noexcept;

		std::string to_string() const noexcept override;

		Value & get_properties(Value &value) const override;

		template<typename... Targs>
		inline void notice(const char *fmt, Targs... Fargs) const {
			Logger::Message{fmt, Fargs...}.notice(objectName);
		}

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

		/// @brief Retrieves the schema definition for interface HTTP request.
		/// @param[in] path The path for required object on interface.
		/// @param[out] s Object populated with the interface http schema details.
		/// @return True if the interface defines an http schema; false otherwise (schema remains unmodified).
		virtual bool schema(const char *path, HTTPSchema &s) const noexcept override;

		/// @brief Retrieves the schema definition for the interface inputs.
		/// @param[in] path The path for required object on interface.
		/// @param[out] s Object populated with the interface input schema details.
		/// @return True if the interface defines an input schema; false otherwise (schema remains unmodified).
		virtual bool schema(const char *path, InputSchema &s) const noexcept override;

		/// @brief Retrieves the schema definition for the object outputs.
		/// @param path The request path for schema.
		/// @param[out] schema Object populated with the output schema details.
		/// @return True if the object defines an output schema; false otherwise (schema remains unmodified).
		virtual bool schema(const char *path, OutputSchema &s) const noexcept override;

	};

	/// @brief An object with common properties.
	class UDJAT_API Object : public NamedObject {
	protected:

		typedef NamedObject Super;

		struct {

			/// @brief Object label.
			const char * label = "";

			/// @brief Object summary.
			const char * summary = "";

			/// @brief URL associated with the object.
			const char * url = "";

			/// @brief Name of the object icon (https://specifications.freedesktop.org/icon-naming-spec/latest/)
			const char * icon = "";

		} properties;

		Object(const Properties &props);

	public:

		constexpr Object(const char *name) : NamedObject(name) {
		}

		inline time_t load(const char *path) {
			return Abstract::Object::load(path);
		}

		bool get_property(const char *key, std::string &value) const override;
		bool get_property(const char *key, Value &value) const override;

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
		Value & get_properties(Value &value) const override;

		/// @brief Retrieves the schema definition for interface HTTP request.
		/// @param[in] path The path for required object on interface.
		/// @param[out] s Object populated with the interface http schema details.
		/// @return True if the interface defines an http schema; false otherwise (schema remains unmodified).
		virtual bool schema(const char *path, HTTPSchema &s) const noexcept override;

		/// @brief Retrieves the schema definition for the interface inputs.
		/// @param[in] path The path for required object on interface.
		/// @param[out] s Object populated with the interface input schema details.
		/// @return True if the interface defines an input schema; false otherwise (schema remains unmodified).
		virtual bool schema(const char *path, InputSchema &s) const noexcept override;

		/// @brief Retrieves the schema definition for the object outputs.
		/// @param path The request path for schema.
		/// @param[out] schema Object populated with the output schema details.
		/// @return True if the object defines an output schema; false otherwise (schema remains unmodified).
		virtual bool schema(const char *path, OutputSchema &s) const noexcept override;

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
			return std::hash<const char *>{}(object.name());
		}
	};

	template <>
	struct hash<Udjat::Abstract::Object> {
		inline size_t operator() (const Udjat::Abstract::Object &obj) const {
			return std::hash<const char *>{}(obj.name());
		}
	};

 }

