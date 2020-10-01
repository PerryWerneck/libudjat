#ifndef UDJAT_REQUEST_H_INCLUDED

	#define UDJAT_REQUEST_H_INCLUDED

	#include <udjat/defs.h>
	#include <string>
	#include <cstring>
	#include <functional>

	namespace Udjat {

		class UDJAT_API Request {
		protected:

			class Controller;

			/// @brief Request name (command)
			std::string name;

			/// @brief Object path.
			std::string path;

			/// @brief Expiration timestamp (For cache headers)
			time_t expiration;

			/// @brief Timestamp of data.
			time_t modification;

		public:
			Request();
			Request(const char *path);
			Request(const char *name, const char *path);

			/// @brief Register a request processor.
			static void insert(const char *name, std::function<void(Request &request)> method);

			virtual ~Request();

			/// @brief Get Request name.
			inline const char * getName() const noexcept {
				return name.c_str();
			}

			bool operator==(const char *name) const noexcept {
				return strcasecmp(this->name.c_str(),name) == 0;
			}

			/// @brief Get Request path.
			inline const char * getPath() const noexcept {
				return path.c_str();
			}

			/// @brief Execute request.
			void call();

			/// @brief Set timestamp for cache the response.
			void setExpirationTimestamp(time_t time);

			/// @brief Set timestamp for data.
			void setModificationTimestamp(time_t time);

			virtual Request & pop(int32_t &value);
			virtual Request & pop(uint32_t &value);
			virtual Request & pop(std::string &value);

			virtual Request & push(const int32_t value);
			virtual Request & push(const uint32_t value);
			virtual Request & push(const char *value);

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

#endif // UDJAT_REQUEST_H_INCLUDED
