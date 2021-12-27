#ifndef CONFIGURABLE_H_INCLUDED

	#define CONFIGURATION_H_INCLUDED

	#include <string>
	#include <mutex>
	#include <udjat/defs.h>
	#include <vector>

	namespace Udjat {

		namespace Config {

			UDJAT_API int32_t get(const std::string &group, const std::string &name, const int32_t def);
			UDJAT_API int64_t get(const std::string &group, const std::string &name, const int64_t def);
			UDJAT_API uint32_t get(const std::string &group, const std::string &name, const uint32_t def);
			UDJAT_API uint64_t get(const std::string &group, const std::string &name, const uint64_t def);
			UDJAT_API float get(const std::string &group, const std::string &name, const float def);
			UDJAT_API double get(const std::string &group, const std::string &name, const double def);
			UDJAT_API std::string get(const std::string &group, const std::string &name, const char *def);
			UDJAT_API std::string get(const std::string &group, const std::string &name, const std::string &def);
			UDJAT_API bool get(const std::string &group, const std::string &name, const bool def);

			UDJAT_API bool hasGroup(const std::string &group);


			template <typename T>
			class Value {
			private:
				T def;

				std::string group;
				std::string name;

			public:
				constexpr Value(const char *g, const char *n, const T d) : def(d),group(g),name(n) {
				}

				T get() const {
					return Config::get(group,name,def);
				}

				operator T() const {
					return get();
				}

				const std::string to_string() const {
					return Config::get(group,name,std::to_string(def));
				}

			};

			template <>
			class Value<std::string> : public std::string {
			private:
				std::string group;
				std::string name;

			public:
				Value(const char *g, const char *n, const char *d)
					: std::string(Config::get(g,n,d)),group(g),name(n) {
				}

				/// @brief Translate block ${name} in the string to *value.
				/// @param name Name of the block inside the string to substitute.
				/// @param value Value to substitute.
				Value<std::string> & set(const char *name, const char *value);

				/// @brief Translate block ${name} in the string with the value of group/key.
				/// @param name Name of the block inside the string to substitute.
				/// @param group Group of the configuration to get the value to substitute
				/// @param key Key with the value to substitute.
				/// @param def dafault value if the group/key doesn't exists in the configuration.
				Value<std::string> & set(const char *name, const char *group, const char *key, const char *def = "");

				inline operator const char *() const noexcept {
					return c_str();
				}

			};

			template <>
			class Value<std::vector<std::string>> : public std::vector<std::string> {
			public:
				Value(const char *group, const char *name, const char *def, const char *delim = ",");

			};


		}

	}



#endif // CONFIGURATION_H_INCLUDED
