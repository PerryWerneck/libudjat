#ifndef ATOM_H_INCLUDED

	#define ATOM_H_INCLUDED

	#include <udjat/defs.h>
	#include <pugixml.hpp>
	#include <udjat/tools/atom.h>
	#include <cstring>

	namespace Udjat {

		/// @brief Single instance string.
		class UDJAT_API Atom {
		private:
			class Controller;
			friend class Controller;

			const char *value;

		public:

			static Atom getFromStatic(const char *str);

			Atom() : value(nullptr) {}

			Atom(const char *str);
			Atom(const std::string &str);
			Atom(const Atom &src);
			Atom(const Atom *src);
			Atom(const pugi::xml_attribute &attribute);

			Atom & operator=(const char *str);
			Atom & operator=(const std::string &str);
			Atom & operator=(const pugi::xml_attribute &attribute);

			const char * c_str() const;

			operator bool() const {
				return value != nullptr && *value;
			}

			bool operator==(const Atom &src) const {
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

	namespace std {

		template <>
		struct hash<Udjat::Atom> {
			inline size_t operator() (const Udjat::Atom &atom) const {
				return std::hash<std::string>{}(atom.c_str());
			}
		};

		inline ostream& operator<< (ostream& os, const Udjat::Atom &atom ) {
			return os << atom.c_str();
		}

	}

#endif // ATOM_H_INCLUDED
