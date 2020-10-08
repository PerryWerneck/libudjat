#ifndef UDJAT_LOGGER_H_INCLUDED

	#define UDJAT_LOGGER_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/atom.h>
	#include <vector>
	#include <string>

	namespace Udjat {

		/// @brief Manage log files.
		/// Reference: https://www.youtube.com/watch?v=IdM0Z2a4fjU
		class UDJAT_API Logger {
		public:
			enum Type : uint8_t {
				Info,
				Warning,
				Error,
				Debug
			};

		private:

			class Writer : public std::string {
			public:
				template<typename T, typename... Targs>
				Writer(Type type, const char *format, T &value, Targs... Fargs) {
					add(value);
					add(Fargs...);
				}

				Writer & add(const char *value);
				Writer & add();

				template<typename T, typename... Targs>
				Writer & add(T &value, Targs... Fargs) {
					add(value);
					return add(Fargs...);
				}


			};

			/// @brief Write log line
			/// @param format String with placeholders ( '{}', '{0}', etc. )
			//void write(Type type, const char *format, const ArgList &args) const noexcept;

		protected:
			Atom name;	///< @brief Object name.

		public:

			/// @brief Redirect std::cout, std::clog and std::cerr to log file.
			/// @param filename The log file name.
			/// @param console If true send log output to standard out.
			static void redirect(const char *filename = nullptr, bool console = false);

			Logger(const char *name);
			Logger(const Atom &name);
			~Logger();

			/// @brief Get Agent name
			const char * getName() const noexcept {
				return this->name.c_str();
			}

			template<typename... Targs>
			void info(const char *format, const Targs... args) const noexcept {
				std::cout << Writer(Info, format, args...).c_str() << std::endl;
			}

		};
	}

	namespace std {

		inline string to_string(const Udjat::Logger &logger) {
			return logger.getName();
		}

		inline ostream& operator<< (ostream& os, const Udjat::Logger &logger) {
			return os << logger.getName();
		}

	}

#endif // UDJAT_LOGGER_H_INCLUDED
