/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
#include <config.h>
#include <udjat/defs.h>
#include <udjat/tools/logger.h>
#include <udjat/tools/string.h>
#include <mutex>
#include <functional>

// https://github.com/openSUSE/libeconf
#if defined(HAVE_ECONF)
 extern "C" {
	#include <libeconf.h>
 }
#elif defined(HAVE_INIPARSER)
	#include <iniparser.h>
#endif // HAVE_ECONF

#if defined(_WIN32)

 namespace Udjat {
	namespace Config {

		// Controller with win32 registry as backend.
		class UDJAT_PRIVATE Controller {
		private:
			static std::recursive_mutex guard;

			Controller();

		public:

			static Controller & getInstance();

			~Controller();

			void open();
			void close();

			void reload();

			bool hasGroup(const char *group);
			bool hasKey(const char *group, const char *key);

			int32_t get(const char *group, const char *name, const int32_t def) const;
			int64_t get(const char *group, const char *name, const int64_t def) const;
			uint32_t get(const char *group, const char *name, const uint32_t def) const;
			uint64_t get(const char *group, const char *name, const uint64_t def) const;
			float get(const char *group, const char *name, const float def) const;
			double get(const char *group, const char *name, const double def) const;
			bool get(const char *group, const char *name, const bool def) const;

			Udjat::String get_string(const char *group, const char *name, const char *def) const;

			bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call);

		};

	}
 }

#elif defined(HAVE_ECONF)

 namespace Udjat {
	namespace Config {

		// Controller with libeconf as backend.
		class UDJAT_PRIVATE Controller {
		private:
			static std::recursive_mutex guard;
			static bool allow_user_config;
			econf_file *hFile = nullptr;

			Controller();
		
		public:

			static Controller & getInstance();

			inline operator bool() const noexcept {
				return (bool) hFile;
			}

			void allow_user_homedir(bool allow);

			~Controller();

			void open();
			void close();

			void reload();

			bool hasGroup(const char *group);
			bool hasKey(const char *group, const char *key);

			int32_t get(const char *group, const char *name, const int32_t def) const;
			int64_t get(const char *group, const char *name, const int64_t def) const;
			uint32_t get(const char *group, const char *name, const uint32_t def) const;
			uint64_t get(const char *group, const char *name, const uint64_t def) const;
			float get(const char *group, const char *name, const float def) const;
			double get(const char *group, const char *name, const double def) const;
			bool get(const char *group, const char *name, const bool def) const;

			Udjat::String get_string(const char *group, const char *name, const char *def) const;

			bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call);

		};

	}
 }

#elif defined(HAVE_INIPARSER)

 namespace Udjat {
	namespace Config {

		// Controller with libiniparser as backend.
		class UDJAT_PRIVATE Controller {
		private:
			static std::recursive_mutex guard;
			static bool allow_user_config;
			dictionary  *ini;

			static int error_callback(const char *format, ...);

			static Udjat::String key(const char *group, const char *key);

			Controller();

		public:

			static Controller & getInstance();

			inline operator bool() const noexcept {
				return (bool) ini;
			}

			~Controller();

			void allow_user_homedir(bool allow) noexcept;
			void open();
			void close();

			void reload();

			bool hasGroup(const char *group);
			bool hasKey(const char *group, const char *key);

			int32_t get(const char *group, const char *name, const int32_t def) const;
			int64_t get(const char *group, const char *name, const int64_t def) const;
			uint32_t get(const char *group, const char *name, const uint32_t def) const;
			uint64_t get(const char *group, const char *name, const uint64_t def) const;
			float get(const char *group, const char *name, const float def) const;
			double get(const char *group, const char *name, const double def) const;
			bool get(const char *group, const char *name, const bool def) const;

			Udjat::String get_string(const char *group, const char *name, const char *def) const;

			bool for_each(const char *group,const std::function<bool(const char *key, const char *value)> &call);

		};

	}
 }

#else

 namespace Udjat {
	namespace Config {

		// Controller without backend.
		class UDJAT_PRIVATE Controller {
		private:
			bool allow_user_config = false;

		public:

			static Controller & getInstance() {
				static Controller instance;
				return instance;
			}

			inline operator bool() const noexcept {
				return false;
			}

			Controller() {
			}

			~Controller() {				
			}

			inline void allow_user_homedir(bool allow) const noexcept {
			}

			inline void open() const noexcept {
			}

			inline void close() const noexcept {
			}

			inline void reload() const noexcept {
			}

			inline bool hasGroup(const char *) const noexcept {
				return false;
			}

			inline bool hasKey(const char *, const char *) const noexcept {
				return false;
			}

			template <typename T>
			inline T get(const char *, const char *, const T def) const noexcept {
				return def;
			}

			inline Udjat::String get(const char *, const char *, const char *def) const noexcept {
				return Udjat::String{def};
			}

			inline  bool for_each(const char *,const std::function<bool(const char *, const char *)> &) const noexcept {
				return false;
			}

		};

	}
 }

#endif 
