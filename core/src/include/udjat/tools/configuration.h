#ifndef CONFIGURABLE_H_INCLUDED

	#define CONFIGURATION_H_INCLUDED

	#include <string>
	#include <mutex>
	#include <udjat/defs.h>

	namespace Udjat {

		namespace Config {

			class UDJAT_API File {
			private:

				static std::recursive_mutex guard;

				void *hFile;	///< @brief Configuration file handle.

				void open();
				void close();

#ifndef _WIN32
				static void handle_reload(int sig) noexcept;
#endif // !_WIN32

				File();

			public:

				File(const File &src) = delete;
				File(const File *src) = delete;

				static File & getInstance();
				~File();

				void reload();

				int32_t get(const std::string &group, const std::string &name, const int32_t def) const;
				int64_t get(const std::string &group, const std::string &name, const int64_t def) const;
				uint32_t get(const std::string &group, const std::string &name, const uint32_t def) const;
				uint64_t get(const std::string &group, const std::string &name, const uint64_t def) const;
				float get(const std::string &group, const std::string &name, const float def) const;
				double get(const std::string &group, const std::string &name, const double def) const;
				std::string get(const std::string &group, const std::string &name, const char *def) const;
				std::string get(const std::string &group, const std::string &name, const std::string &def) const;
				bool get(const std::string &group, const std::string &name, const bool def) const;

			};

			template <typename T>
			class Value {
			private:
				T def;

				std::string group;
				std::string name;

			public:
				Value(const char *g, const char *n, const T d) : def(d),group(g),name(n) {
				}

				const T get() const {
					return Config::File::getInstance().get(group,name,def);
				}

				operator T() const {
					return get();
				}

				const std::string to_string() const {
					return Config::File::getInstance().get(group,name,std::to_string(def));
				}

			};

			template <>
			class Value<std::string> : public std::string {
			private:
				std::string group;
				std::string name;

			public:
				Value(const char *g, const char *n, const char *d)
					: std::string(Config::File::getInstance().get(g,n,d)),group(g),name(n) {
				}

				// const std::string & get() const;

			};
		}

	}



#endif // CONFIGURATION_H_INCLUDED
