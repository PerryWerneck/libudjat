/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

 #pragma once

/*
 #include <udjat/defs.h>
 #include <udjat/tools/url.h>
 #include <udjat/tools/http/mimetype.h>
 #include <udjat/tools/file/handler.h>
 #include <memory>

 #ifndef _WIN32
	#include <sys/socket.h>
 #endif // !_WIN32

 namespace Udjat {

	/// @brief Network protocol module.
	class UDJAT_API Protocol {
	private:
		class Controller;
		friend class Controller;

		/// @brief The protocol name.
		const char * name;

		/// @brief Module information.
		const ModuleInfo &module;

	protected:

		/// @brief Set current protocol as the default one.
		void setDefault() noexcept;

	public:

		/// @brief Get handler for file:///
		static Protocol & FileHandlerFactory();

		/// @brief Get handler for script:///
		static Protocol & ScriptHandlerFactory();

		/// @brief Protocol watcher.
		class UDJAT_API Watcher {
		private:
			static Watcher *instance;
			Watcher * parent = nullptr;

		protected:
			Watcher();

		public:
			~Watcher();

			static Watcher & getInstance();

			virtual void set_url(const char *url);
			virtual void set_progress(double current, double total);

			static const std::function<bool(double current, double total)> progress;

		};

		/// @brief Request/response header.
		class UDJAT_API Header : public std::string {
		private:
			std::string field_name;

		public:
			Header(const char *n) : field_name(n) {
			}

			inline const char * name() const noexcept {
				return field_name.c_str();
			}

			inline const char * value() const noexcept {
				return this->c_str();
			}

			inline bool operator == (const char *name) const noexcept {
				return strcasecmp(name,this->field_name.c_str()) == 0;
			}

			virtual Header & assign(const TimeStamp &value);

			Header & assign(const char *value);
			Header & assign(const std::string &value);

			template <typename T>
			Header assign(const T value) {
				return assign(std::to_string(value));
			}

			Header & operator = (const TimeStamp &value) {
				return assign(value);
			}

			template <typename T>
			Header & operator = (const T value) {
				return assign(std::to_string(value));
			}

		};

		/// @brief Protocol Worker.
		class UDJAT_API Worker {
		private:
			struct Args{
				URL url;
				HTTP::Method method;

				Args(const URL &u, HTTP::Method m) : url(u), method(m) {
				}

			} args;

			/// @brief Get network interface name from IP.
			/// @return true if the interface was found.
			bool getnic(const sockaddr_storage &addr, std::string &nic);

			/// @brief Get mac address from IP.
			void getmac(const sockaddr_storage &addr, std::string &mac);

		protected:

			/// @brief Worker name.
			const char *name = "";

			/// @brief Status of last operation.
			unsigned int status_code = 0;

			/// @brief Timeouts
			struct Timeouts {
				time_t connect = 30;		///< @brief Connect timeout (in seconds).
				time_t recv = 30;			///< @brief Receive timeout (in seconds).
				time_t send = 30;			///< @brief Send timeout (in seconds).
#ifdef _WIN32
				time_t resolv = 30;			///< @brief Resolv timeout (in seconds).
#endif // _WIN32

				/// @brief Setup timeout values from scheme.
				void setup(const char *scheme) noexcept;

			} timeout;

			/// @brief Output data (To host)
			struct Out {
				Udjat::String payload;	///< @brief Request payload.

				Out(const char *p) : payload(p) {
				}

			} out;

			/// @brief Connected to host, expand network variables in payload string.
			/// @param sock The connected socket used to get network info.
			void set_socket(int sock);

			/// @brief Set local addr.
			void set_local(const sockaddr_storage &addr) noexcept;

			/// @brief Set remote addr.
			void set_remote(const sockaddr_storage &addr) noexcept;

			/// @brief Expand payload.
			/// @return String with expanded payload.
			const char * get_payload() noexcept;

		public:

			Worker(const char *url = "", const HTTP::Method method = HTTP::Get, const char *payload = "");
			Worker(const URL &url, const HTTP::Method method = HTTP::Get, const char *payload = "");
			virtual ~Worker();

			/// @brief Set request credentials.
			virtual Worker & credentials(const char *user, const char *passwd);

			/// @brief Set request payload.
			inline Worker & payload(const char *payload) noexcept {
				out.payload = payload;
				return *this;
			}

			/// @brief Set request payload.
			inline Worker & payload(const std::string &payload) noexcept {
				out.payload = payload;
				return *this;
			}

			/// @brief Get request payload.
			inline const char * payload() const {
				return out.payload.c_str();
			}

			/// @brief Set request url.
			Worker & url(const char *url) noexcept;

			/// @brief Set request url.
			inline Worker & url(const URL &url) noexcept {
				return this->url(url.c_str());
			}

			inline const URL & url() const noexcept {
				return args.url;
			}

			inline Worker & method(const HTTP::Method method) noexcept {
				args.method = method;
				return *this;
			}

			inline HTTP::Method method() const noexcept {
				return args.method;
			}

			/// @brief Get status of last operation.
			/// @retval 0 No status.
			/// @retval 200 Ok.
			inline unsigned int result_code() const noexcept {
				return status_code;
			}

			/// @brief Get/Create request header.
			/// @param name Header name.
			/// @return The header object.
			virtual Header & request(const char *name);

			/// @brief Get/Create response header.
			/// @param name Header name.
			/// @return The header object.
			virtual const Header & response(const char *name);

			/// @brief Get header.
			/// @param key The header name.
			/// @return The header.
			inline const Header & operator[](const char *name) {
				return response(name);
			}

			/// @brief Set request mimetype.
			/// @param type The mimetype for the request.
			/// @return 0 if ok, errno if not.
			virtual int mimetype(const MimeType type);

			/// @brief Set request header.
			/// @param name Header name.
			/// @param value Header value;
			inline void header(const char *name, const char *value) {
				request(name).assign(value);
			}

			/// @brief Set request header.
			/// @param name Header name.
			/// @param value Header value;
			template <typename T>
			inline void header(const char *name, const T value) {
				request(name).assign(value);
			}

			/// @brief Call URL, return response as string.
			/// @param progress The download progress notifier.
			/// @return String with the URL contents.
			virtual String get(const std::function<bool(double current, double total)> &progress) = 0;

			/// @brief Test URL access (do a 'method' request with 'payload').
			/// @return URL return code.
			/// @retval 200 Got response.
			/// @retval 401 Acess denied.
			/// @retval 404 Not found.
			/// @retval EINVAL Invalid method.
			/// @retval ENOTSUP No support for test in protocol handler.
			virtual int test(const std::function<bool(double current, double total)> &progress) noexcept;

			/// @brief Test URL access (do a 'method' request with 'payload').
			/// @return URL return code.
			/// @retval 200 Got response.
			/// @retval 401 Acess denied.
			/// @retval 404 Not found.
			/// @retval EINVAL Invalid method.
			/// @retval ENOTSUP No support for test in protocol handler.
			int test() noexcept;

			/// @brief Set file properties using the http response header.
			/// @param filename The filename to update.
			/// @return 0 if ok, errno if not.
			//int set_file_properties(const char *filename);

			/// @brief Call URL, save response to file.
			/// @param File The file handler.
			/// @param progress The download progress notifier.
			virtual bool save(File::Handler &file, const std::function<bool(double current, double total)> &progress);

			/// @brief Call URL, save response as filename.
			/// @param filename	The file name to save.
			/// @param progress The download progress notifier.
			/// @param replace If true the file will be replaced (if updated); if false a '.bak' file will be keep with the old contents.
			/// @return true if the file was updated or replaced.
			virtual bool save(const char *filename, const std::function<bool(double current, double total)> &progress, bool replace = false);

			/// @brief Download filename if changed, call secondary writer with downloaded or cached info.
			/// @param filename	The cache filename.
			/// @param writer The secondary file writer.
			/// @return true if the file was updated.
			virtual bool save(const char *filename,const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer);

			/// @brief Call URL, save response with custom writer.
			/// @param writer The custom writer.
			virtual void save(const std::function<bool(unsigned long long current, unsigned long long total, const void *buf, size_t length)> &writer);

			/// @brief Get URL, save response to temporary file.
			/// @return The temporary filename.
			std::string save(const std::function<bool(double current, double total)> &progress);

			/// @brief Get URL, save response to cache file.
			/// @param progress The download progress notifier.
			/// @return The cached filename.
			virtual std::string filename(const std::function<bool(double current, double total)> &progress);

			/// @brief Get URL, save response to cache file.
			/// @return The cached filename.
			std::string filename();

			/// @brief Get URL, return response as string.
			String get();

			/// @brief Get URL (asyncronous when protocol handler can do it).
			virtual void get(const std::function<void(int code, const char *response)> &call);

			/// @brief Get URL, save response as filename.
			/// @return true if the file was updated.
			/// @param replace If true the file will be replaced (if updated); if false a '.bak' file will be keep with the old contents.
			bool save(const char *filename, bool replace = false);

			/// @brief Call URL, save response to temporary file.
			/// @return The temporary filename.
			std::string save();

			std::ostream & info() const;
			std::ostream & warning() const;
			std::ostream & error() const;
			std::ostream & trace() const;

		};

		virtual std::shared_ptr<Worker> WorkerFactory() const;

		static std::shared_ptr<Worker> WorkerFactory(const char *url, bool allow_default = true, bool autoload = true);

		Protocol(const Protocol &) = delete;
		Protocol(const Protocol *) = delete;

		Protocol(const char *name, const ModuleInfo &module);
		virtual ~Protocol();

		static bool for_each(const std::function<bool(const Protocol &protocol)> &method);

		inline const char * c_str() const noexcept {
			return name;
		}

		inline bool operator==(const char *name) const noexcept {
			return strcasecmp(name,this->name) == 0;
		}

		std::ostream & info() const;
		std::ostream & warning() const;
		std::ostream & error() const;
		std::ostream & trace() const;

		virtual Value & getProperties(Value &properties) const;

		/// @brief Find protocol based on URL.
		/// @param url The url to search for.
		/// @param allow_default If true returns the default protocol when not found.
		/// @param autoload If true tries to load a module using the protocol name.
		/// @return Pointer to selected protocol or nullptr.
		static const Protocol * find(const URL &url, bool allow_default = true, bool autoload = false);

		/// @brief Find protocol based on protocol name.
		/// @param url The url to search for.
		/// @param allow_default If true returns the default protocol when not found.
		/// @return Pointer to selected protocol or nullptr.
		// static const Protocol * find(const char *name, bool allow_default = true, bool autoload = false);

		/// @brief Verify protocol pointer.
		/// @param protocol Pointer to protocol to confirm.
		/// @return nullptr if protocol is not valid.
		static const Protocol * verify(const void *protocol);

		/// @brief Find protocol and call.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		static String call(const char *url, const HTTP::Method method = HTTP::Get, const char *payload = "");

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		virtual String call(const URL &url, const HTTP::Method method, const char *payload = "") const;

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @param value Value for response.
		/// @return true if the value was updated.
		virtual bool call(const URL &url, Udjat::Value &value, const HTTP::Method method = HTTP::Get, const char *payload = "") const;

		/// @brief Call protocol method.
		/// @param url The URL to call.
		/// @param method Required method.
		/// @param payload request payload.
		/// @return Host response.
		String call(const URL &url, const char *method, const char *payload = "") const;

		/// @brief Download/update file.
		/// @param url the file URL.
		/// @param filename The fullpath for the file.
		/// @param progress The progress callback.
		/// @return true if the file was updated.
		virtual bool get(const URL &url, const char *filename, const std::function<bool(double current, double total)> &progress) const;


	};

 }

*/

