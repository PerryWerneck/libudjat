#ifndef UDJAT_REQUEST_H_INCLUDED

	#define UDJAT_REQUEST_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <string>
	#include <cstring>
	#include <functional>
	#include <vector>
	#include <udjat/tools/timestamp.h>
	#include <udjat/tools/value.h>
	#include <memory>
	#include <udjat/tools/http/mimetype.h>
	#include <udjat/tools/method.h>

	namespace Udjat {

		/// @brief Report in the format row/col.
		class UDJAT_API Report {
		protected:

			struct {

				/// @brief Column names.
				std::vector<std::string> names;

				/// @brief Next column name.
				std::vector<std::string>::iterator current;

			} columns;

			/// @brief Start a new row.
			/// @return true if the row was open.
			virtual bool open();

			/// @brief End row.
			/// @return true if the row have columns.
			virtual bool close();

			/// @brief Get next column title (close and open row if needed).
			std::string next();

			Report();
			void set(const char *column_name, va_list args);

		public:

			/// @brief Open report, define column names.
			/// @param name	Report	name.
			/// @param column_name	First column name.
			/// @param ...			Subsequent column names.
			void start(const char *name, const char *column_name, ...) __attribute__ ((sentinel));

			/// @brief Open report, define column names.
			/// @param name	Report	name.
			/// @param column_names	The column names.
			void start(const char *name, const std::vector<std::string> &column_names);

			virtual ~Report();

			virtual Report & push_back(const char *str) = 0;

			virtual Report & push_back(const std::string &value);

			virtual Report & push_back(const short value);
			virtual Report & push_back(const unsigned short value);

			virtual Report & push_back(const int value);
			virtual Report & push_back(const unsigned int value);

			virtual Report & push_back(const long value);
			virtual Report & push_back(const unsigned long value);

			virtual Report & push_back(const TimeStamp value);
			virtual Report & push_back(const bool value);

			virtual Report & push_back(const float value);
			virtual Report & push_back(const double value);

			template <typename T>
			Report & push_back(const T &value) {
				return this->push_back(std::to_string(value));
			}

		};

		class UDJAT_API Response : public Udjat::Value {
		public:
		protected:

			/// @brief Expiration timestamp (For cache headers)
			time_t expiration = 0;

			/// @brief Timestamp of data.
			time_t modification = 0;

			/// @brief Response type.
			MimeType type = MimeType::custom;

			/// @brief is the response valid?
			bool valid = true;

		public:

			constexpr Response(const MimeType m = MimeType::custom)
			: expiration(0), modification(0), type(m) { }

			/// @brief Set timestamp for cache the response.
			void setExpirationTimestamp(const time_t time);

			/// @brief Set timestamp for data.
			void setModificationTimestamp(const time_t time);

			inline bool operator ==(const MimeType type) const noexcept {
				return this->type == type;
			}

			inline bool operator !=(const MimeType type) const noexcept {
				return this->type != type;
			}

			inline MimeType getType() const noexcept {
				return this->type;
			}

			inline operator MimeType() const noexcept {
				return this->type;
			}

		};

		class UDJAT_API Request {
		private:
			HTTP::Method type = HTTP::Get;

		protected:

			/// @brief Method name.
			std::string method;

			/// @brief Request path.
			std::string path;

		public:
			Request() {
			}

			Request(HTTP::Method t) : type(t) {
			}

			Request(const char *type) : type(HTTP::MethodFactory(type)) {
			}

			/// @brief Get the request method.
			inline const char * getMethod() const noexcept {
				return method.c_str();
			}

			/// @brief Get the requested action.
			virtual const std::string getAction();

			/// @brief Select action from list.
			/// @return Index of the selected action.
			size_t getAction(const char *name, ...) __attribute__ ((sentinel));

			/// @brief Get the request path.
			inline const char * getPath() const noexcept {
				return path.c_str();
			}

			inline bool operator==(HTTP::Method type) const noexcept {
				return this->type == type;
			}

			inline bool operator!=(HTTP::Method type) const noexcept {
				return this->type != type;
			}

			inline HTTP::Method as_type() const noexcept {
				return type;
			}

			bool operator ==(const char *key) const noexcept;

			/// @brief Pop one element from path, scan the list.
			/// @return Index of the 'popped' element.
			size_t pop(const char *name, ...) __attribute__ ((sentinel));

			/// @brief Pop one element from path.
			virtual std::string pop();

			virtual Request & pop(std::string &value);
			virtual Request & pop(int &value);
			virtual Request & pop(unsigned int &value);

		};

	}

	namespace std {

		template <typename T>
		inline Udjat::Report & operator<<(Udjat::Report &out, T value) {
			return out.push_back(value);
		}

		template <typename T>
		inline Udjat::Request & operator>>(Udjat::Request &in, T &value) {
			return in.pop(value);
		}

	}


#endif // UDJAT_REQUEST_H_INCLUDED
