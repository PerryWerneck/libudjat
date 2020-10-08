#ifndef UDJAT_LOGGER_H_INCLUDED

	#define UDJAT_LOGGER_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/atom.h>
	#include <vector>
	#include <string>
	#include <iostream>

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

			class UDJAT_API Writer {
			private:
				std::vector<std::string> args;
				std::string format;

				Writer & append(const char *value);
				Writer & append(const std::string &value);

				Writer & add();

				template<typename T>
				Writer & append(const T &value) {
					return append(std::to_string(value));
				}

				template<typename T, typename... Targs>
				Writer & add(T &value, Targs... Fargs) {
					append(value);
					return add(Fargs...);
				}

			public:
				template<typename T, typename... Targs>
				Writer(const char *format, T &value, Targs... Fargs) {
					this->format = format;
					append(value);
					add(Fargs...);
				}


				std::string to_string() const;

			};

		protected:
			Atom name;	///< @brief Object name.

			Logger();

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

			/// @brief Write informational message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void info(const char *format, const Targs... args) const noexcept {
				std::cout << name << "\t" << Writer(format, args...).to_string() << std::endl;
			}

			/// @brief Write warning message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void warning(const char *format, const Targs... args) const noexcept {
				std::clog << name << "\t" << Writer(format, args...).to_string() << std::endl;
			}

			/// @brief Write error message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void error(const char *format, const Targs... args) const noexcept {
				std::cerr << name << "\t" << Writer(format, args...).to_string() << std::endl;
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
