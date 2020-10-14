#ifndef QUARK_H_INCLUDED

	#define QUARK_H_INCLUDED

	#include <udjat/defs.h>
	#include <pugixml.hpp>
	#include <udjat/tools/quark.h>
	#include <cstring>

	namespace Udjat {

		/// @brief Single instance string.
		class UDJAT_API Quark {
		private:
			class Controller;
			friend class Controller;

			const char *value;

		public:

			static Quark getFromStatic(const char *str);

			Quark() : value(nullptr) {}

			Quark(const char *str);
			Quark(const std::string &str);
			Quark(const Quark &src);
			Quark(const Quark *src);
			Quark(const pugi::xml_attribute &attribute);

			Quark & operator=(const char *str);
			Quark & operator=(const std::string &str);
			Quark & operator=(const pugi::xml_attribute &attribute);

			const char * c_str() const;

			operator bool() const {
				return value != nullptr && *value;
			}

			bool operator==(const Quark &src) const {
				return this->value == src.value;
			}

			bool operator==(const char *str) const {
				return compare(str);
			}

			int compare(const char *str) const {
				return strcmp(c_str(),str);
			}

		};
	}

	#define I_(str) Udjat::Quark::getFromStatic(str)

	namespace std {

		template <>
		struct hash<Udjat::Quark> {
			inline size_t operator() (const Udjat::Quark &q) const {
				return std::hash<std::string>{}(q.c_str());
			}
		};

		inline ostream& operator<< (ostream& os, const Udjat::Quark &q ) {
			return os << q.c_str();
		}

	}

#endif // QUARK_H_INCLUDED
