#ifndef ATOM_H_INCLUDED

	#define ATOM_H_INCLUDED

	#include <udjat/defs.h>
	#include <pugixml.hpp>
	#include <udjat/tools/atom.h>

	namespace Udjat {

		/// @brief Single instance string.
		class UDJAT_API Atom {
		private:
			class Controller;
			friend class Controller;

			std::string *value;

		public:

			Atom() : value(nullptr) {}

			Atom(const char *str);
			Atom(const std::string &str);
			Atom(const Atom &src);
			Atom(const Atom *src);

			Atom & operator=(const char *str);
			Atom & operator=(const std::string &str);
			Atom & operator=(const pugi::xml_attribute &attribute);

			const char * c_str() const {
				return value ? value->c_str() : "";
			}

			const std::string & to_string() const {
				return *value;
			}

			operator bool() const {
				return (this->value && !this->value->empty());
			}

			bool operator==(const Atom &src) const {
				return this->value == src.value;
			}

			int compare(const char *str) const {
				return this->value->compare(str);
			}

			int compare(const std::string &str) const {
				return this->value->compare(str);
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

		inline const string & to_string(const Udjat::Atom atom) {
			return atom.to_string();
		}

		inline ostream& operator<< (ostream& os, const Udjat::Atom &atom ) {
			return os << atom.to_string();
		}

	}

#endif // ATOM_H_INCLUDED
