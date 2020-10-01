#ifndef UDJAT_REQUEST_H_INCLUDED

	#define UDJAT_REQUEST_H_INCLUDED

	#include <udjat/defs.h>
	#include <string>
	#include <functional>

	namespace Udjat {

		namespace Abstract {

			class UDJAT_API Request {
			protected:

				/// @brief Request name (command)
				std::string name;

				/// @brief Object path.
				std::string path;

				/// @brief Expiration timestamp (For cache headers)
				time_t expiration;

				/// @brief Timestamp of data.
				time_t modification;

				Request();
				Request(const char *name, const char *path);

			public:
				class Controller;

				virtual ~Request();

				/// @brief Get Agent path.
				inline const char * getPath() const noexcept {
					return path.c_str();
				}

				/// @brief Execute request.
				void call();

				/// @brief Set timestamp for cache the response.
				void setExpirationTimestamp(time_t time);

				/// @brief Set timestamp for data.
				void setModificationTimestamp(time_t time);

				virtual Request & pop(int32_t &value) = 0;
				virtual Request & pop(uint32_t &value) = 0;
				virtual Request & pop(std::string &value) = 0;

				virtual Request & push(const int32_t value) = 0;
				virtual Request & push(const uint32_t value) = 0;
				virtual Request & push(const char *value) = 0;

				inline Request & push(const std::string &value) {
					return push(value.c_str());
				}

				virtual Request & pop(const char *name, int32_t &value);
				virtual Request & pop(const char *name, uint32_t &value);
				virtual Request & pop(const char *name, std::string &value);

				virtual Request & push(const char *name, const int32_t value);
				virtual Request & push(const char *name, const uint32_t value);
				virtual Request & push(const char *name, const char *value);

				inline Request & push(const char *name, const std::string &value) {
					return push(name, value.c_str());
				}


			};

		}

		/// @brief Register a request processor.
		void insert(const char *name, std::function<void(Abstract::Request &request)> method);

	}

#endif // UDJAT_REQUEST_H_INCLUDED
