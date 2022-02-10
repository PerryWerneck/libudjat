#ifndef UDJAT_LOGGER_H_INCLUDED

	#define UDJAT_LOGGER_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	//#include <vector>
	#include <string>
	#include <iostream>
	#include <mutex>

	namespace Udjat {

		/// @brief Manage log files.
		/// Reference: https://www.youtube.com/watch?v=IdM0Z2a4fjU
		class UDJAT_API Logger {
		public:
			enum Level : uint8_t {
				Info,
				Warning,
				Error
			};

		private:

			static std::mutex guard;

			class Writer : public std::basic_streambuf<char, std::char_traits<char> > {
			private:
				class Buffer : public std::string {
				private:
					Buffer() : std::string() {
					}

				public:
					Buffer(const Buffer &src) = delete;
					Buffer(const Buffer *src) = delete;

					static Buffer & getInstance(Level id);
					bool push_back(int c);

				};


				/// @brief The buffer id.
				Level id = Info;

				/// @brief Send output to console?
				bool console = true;

				void write(int fd, const std::string &str);
				void write(Buffer &buffer);

			protected:

				/// @brief Writes characters to the associated file from the put area
				int sync() override;

				/// @brief Writes characters to the associated output sequence from the put area.
				int overflow(int c) override;

			public:
				Writer(Logger::Level i, bool c) : id(i), console(c) {
				}

			};

		protected:

			static Level level;

			struct Properties {
				const char * name;	///< @brief Object name.

				constexpr Properties(const char *n) : name(n) {
				}

			} properties;

		public:

			static Level LevelFactory(const char *name) noexcept;

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
			/// @param console If true send log output to standard out.
#ifdef DEBUG
			static void redirect(bool console = true);
#else
			static void redirect(bool console = false);
#endif // DEBUG

			constexpr Logger(const char *name = STRINGIZE_VALUE_OF(PRODUCT_NAME)) : properties(name) {
			}

			void set(const pugi::xml_node &name);

			operator const char *() const noexcept {
				return this->properties.name;
			}

			/// @brief Get Logger name.
			const char * name() const noexcept {
				return this->properties.name;
			}

			inline const char * c_str() const noexcept {
				return this->properties.name;
			}

			/// @brief Write informational message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void info(const char *format, const Targs... args) const noexcept {
				std::cout << name() << "\t" << Message(format, args...) << std::endl;
			}

			/// @brief Write warning message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void warning(const char *format, const Targs... args) const noexcept {
				std::clog << name() << "\t" << Message(format, args...) << std::endl;
			}

			/// @brief Write error message.
			/// @param format String with '{}' placeholders
			template<typename... Targs>
			void error(const char *format, const Targs... args) const noexcept {
				std::cerr << name() << "\t" << Message(format, args...) << std::endl;
			}

			/// @brief Write Error message.
			/// @param e exception to write.
			void error(const std::exception &e) {
				std::cerr << name() << "\t" << e.what() << std::endl;
			}

			/// @brief Write error message.
			/// @param e exception to write.
			/// @param format String with '{}' placeholders (the last one with be the exception message)
			template<typename... Targs>
			void error(const std::exception &e, const char *format, const Targs... args) const noexcept {
				std::cerr << name() << "\t" << Message(format, args...).append(e) << std::endl;
			}

		};
	}

	namespace std {

		inline string to_string(const Udjat::Logger &logger) {
			return logger.name();
		}

		inline string to_string(const Udjat::Logger *logger) {
			return logger->name();
		}

		inline ostream& operator<< (ostream& os, const Udjat::Logger &logger) {
			return os << logger.name();
		}

	}

#endif // UDJAT_LOGGER_H_INCLUDED
