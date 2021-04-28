#ifndef UDJAT_REQUEST_H_INCLUDED

	#define UDJAT_REQUEST_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <string>
	#include <cstring>
	#include <functional>
	#include <json/value.h>
	#include <vector>
	#include <udjat/tools/timestamp.h>
	#include <memory>

	namespace Udjat {

		class UDJAT_API Response : public Json::Value {
		protected:

			/// @brief Expiration timestamp (For cache headers)
			time_t expiration;

			/// @brief Timestamp of data.
			time_t modification;

			Response(const time_t expiration, const time_t modification);

		public:

			/// @brief Report with fixed columns.
			class UDJAT_API Report {
			protected:

				struct {

					/// @brief Column names.
					std::vector<std::string> names;

					/// @brief Next column name.
					std::vector<std::string>::iterator current;

				} columns;

				/// @brief Start a new row.
				virtual void open();

				/// @brief End row.
				virtual void close();

				/// @brief Get next column title (close and open row if needed).
				std::string next();

				Report();
				void setColumns(const char *column_name, va_list args);

			public:

				// Report(const char *name, const char *column_name, ...) __attribute__ ((sentinel));

				virtual ~Report();

				virtual Report & push_back(const char *str) = 0;
				virtual std::string to_string() = 0;

				virtual Report & push_back(const std::string &str);
				virtual Report & push_back(const bool value);
				virtual Report & push_back(const TimeStamp &timestamp);
				virtual Report & push_back(const int8_t value);
				virtual Report & push_back(const int16_t value);
				virtual Report & push_back(const int32_t value);
				virtual Report & push_back(const uint8_t value);
				virtual Report & push_back(const uint16_t value);
				virtual Report & push_back(const uint32_t value);

				template <typename T>
				Report & push_back(const T &value) {
					return this->push_back(std::to_string(value));
				}

			};

			Response();

			/// @brief Set timestamp for cache the response.
			void setExpirationTimestamp(const time_t time);

			/// @brief Set timestamp for data.
			void setModificationTimestamp(const time_t time);

			/// @brief Open a row/column report.
			virtual std::shared_ptr<Report> open(const char *name, const char *column_name, ...) __attribute__ ((sentinel));

		};

		class UDJAT_API Request : public Json::Value {
		private:
			std::string path;

		public:
			Request(const char *path);
			Request(const std::string &path);

			const char *c_str() const {
				return path.c_str();
			}

		};

	}

	template <typename T>
	inline Udjat::Response::Report & operator<<(Udjat::Response::Report &out, T value) {
		return out.push_back(value);
	}


#endif // UDJAT_REQUEST_H_INCLUDED
