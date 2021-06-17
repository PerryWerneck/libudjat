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
	#include <udjat/tools/mimetype.h>

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
		public:
			enum class Type : uint8_t {
				Get,		///< @brief Requests a representation of the specified resource; only retrieve data.
				Head,		///< @brief Asks for a response identical to that of a GET without body.
				Post,		///< @brief Submit an entity to the specified resource.
				Put,		///< @brief Replace all current representations of the target resource.
				Delete,		///< @brief Delete the specified resource.
				Connect,	///< @brief Establishes a tunnel to the server identified by the target resource.
				Options,	///< @brief Describe the communication options for the target resource.
				Trace,		///< @brief Performs a message loop-back test along the path to the target resource.
				Patch,		///< @brief Apply partial modifications to a resource.

				Count		///< @brief Type count.
			};

			static const char *typeNames[(size_t) Type::Count];

		private:
			std::string path;
			Type type = Type::Get;

		public:
			Request(const char *path, Type type = Type::Get);
			Request(const char *path, const char *type);

			const char *c_str() const {
				return path.c_str();
			}

			inline bool operator==(Request::Type) const noexcept {
				return this->type == type;
			}

			inline bool operator!=(Request::Type) const noexcept {
				return this->type != type;
			}

			Type as_type(const char *type);

			inline Type as_type() const noexcept {
				return type;
			}

			bool operator ==(const char *key) const noexcept;

			/// @brief Pop one element from path.
			std::string pop();

			/// @brief Pop one element from path, scan the list.
			/// @return Index of the 'popped' element.
			size_t pop(const char *name, ...) __attribute__ ((sentinel));

			Request & pop(std::string &value);
			Request & pop(int &value);
			Request & pop(unsigned int &value);

		};

	}

	template <typename T>
	inline Udjat::Report & operator<<(Udjat::Report &out, T value) {
		return out.push_back(value);
	}

	template <typename T>
	inline Udjat::Request & operator>>(Udjat::Request &in, T &value) {
		return in.pop(value);
	}

#endif // UDJAT_REQUEST_H_INCLUDED
