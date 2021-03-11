#ifndef UDJAT_REQUEST_H_INCLUDED

	#define UDJAT_REQUEST_H_INCLUDED

	#include <udjat/defs.h>
	#include <udjat/tools/quark.h>
	#include <string>
	#include <cstring>
	#include <functional>
	#include <json/value.h>

	namespace Udjat {

		class UDJAT_API Response : public Json::Value {

		protected:

			/// @brief Expiration timestamp (For cache headers)
			time_t expiration;

			/// @brief Timestamp of data.
			time_t modification;

			Response(const time_t expiration, const time_t modification);

		public:
			Response();

			/// @brief Set timestamp for cache the response.
			void setExpirationTimestamp(const time_t time);

			/// @brief Set timestamp for data.
			void setModificationTimestamp(const time_t time);

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

#endif // UDJAT_REQUEST_H_INCLUDED
