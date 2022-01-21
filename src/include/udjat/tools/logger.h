#ifndef UDJAT_LOGGER_H_INCLUDED

	#define UDJAT_LOGGER_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <vector>
	#include <string>
	#include <iostream>

	namespace Udjat {

		/// @brief Manage log files.
		/// Reference: https://www.youtube.com/watch?v=IdM0Z2a4fjU
		class UDJAT_API Logger {
		private:
			  const char * name;	///< @brief Object name.

		public:
			enum Type : uint8_t {
				Info,
				Warning,
				Error,
				Debug
			};

			/// @brief Log message formatter.
			class UDJAT_API Message : public std::string {
			public:
				template<typename T, typename... Targs>
				Message(const char *fmt, T &value, Targs... Fargs) : std::string(fmt) {
					append(value);
					add(Fargs...);
				}

				Message & append(const char *value);

				Message & append(const std::string &value) {
					return append(value.c_str());
				}

				Message & append(const std::exception &e);

				Message & add() {
					return *this;
				}

				template<typename T>
				Message & append(const T &value) {
					return append(std::to_string(value));
				}

				template<typename T, typename... Targs>
				Message & add(T &value, Targs... Fargs) {
					append(value);
					return add(Fargs...);
				}

			};

		public:

			/// @brief Redirect std::cout, std::clog and std::cerr to log file.
			/// @param filename The log file name.
			/// @param console If true send log output to standard out.
#ifdef DEBUG
			static void redirect(const char *filename = nullptr, bool console = true);
#else
			static void redirect(const char *filename = nullptr, bool console = false);
#endif // DEBUG

			constexpr Logger(const char *n = STRINGIZE_VALUE_OF(PRODUCT_NAME)) : name(n) {
			}

			~Logger();

			void set(const pugi::xml_node &name);

			operator const char *() const noexcept {
				return this->name;
			}

			/// @brief Get Logger name.
			const char * getName() const noexcept {
				return this->name;
			}

			inline const char * c_str() const noexcept {
				return this->name;
			}

			/// @brief Write informational message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void info(const char *format, const Targs... args) const noexcept {
				std::cout << name << "\t" << Message(format, args...) << std::endl;
			}

			/// @brief Write warning message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void warning(const char *format, const Targs... args) const noexcept {
				std::clog << name << "\t" << Message(format, args...) << std::endl;
			}

			/// @brief Write error message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void error(const char *format, const Targs... args) const noexcept {
				std::cerr << name << "\t" << Message(format, args...) << std::endl;
			}

			/// @brief Write Error message.
			/// @param e exception to write.
			void error(const std::exception &e) {
				std::cerr << name << "\t" << e.what() << std::endl;
			}

			/// @brief Write error message.
			/// @param e exception to write.
			/// @param format String with '{}' placeholders (the last one with be the exception message)
			template<typename... Targs>
			void error(const std::exception &e, const char *format, const Targs... args) const noexcept {
				std::cerr << name << "\t" << Message(format, args...).append(e) << std::endl;
			}

		};
	}

	namespace std {

		inline string to_string(const Udjat::Logger &logger) {
			return logger.getName();
		}

		inline string to_string(const Udjat::Logger *logger) {
			return logger->getName();
		}

		inline ostream& operator<< (ostream& os, const Udjat::Logger &logger) {
			return os << logger.getName();
		}

	}

#endif // UDJAT_LOGGER_H_INCLUDED
