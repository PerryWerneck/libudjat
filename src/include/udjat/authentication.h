/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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

 #include <udjat/defs.h>
 #include <cstdint>
 #include <udjat/tools/string.h>
 #include <udjat/tools/properties.h>
 
 namespace Udjat {

		/// @brief Authentication token
		class UDJAT_API Authentication {
			public:

				/// @brief Check for authentication engine availability.
				/// @return true if the authentication engine is available.
				static bool available() noexcept;

				/// @brief Clear authentication, set it to default state.
				virtual void clear() noexcept;

				/// @brief Reset authentication engine, generate a new key.
				static void reset() noexcept;

				enum Level : uint8_t {
					None,	///< @brief Non authenticated user.
					Guest,  ///< @brief Guest/Viewer: Read-only access to specific resources.
					User,	///< @brief User/Member: Can create, edit, and view their own data, but cannot see global settings.
					Admin,	///< @brief Admin/Manager: Can invite/remove regular users, change application settings, and manage content.Standard 

					// Owner is allways the higher one.
					Owner,	///< @brief Owner/Super: Full system control, billing management, and account deletion. (Strictly 1 or 2 users).
				};

				static Level LevelFactory(const char *name = nullptr);
				static Level LevelFactory(const Properties &props);
				static Level LevelFactory(const Properties &props, Level level);
			
				Authentication(Level level = None);
				Authentication(const char *username, Level level = Guest);

				/// @brief Encrypt token, return base64.
				/// @param token The token to encrypt.
				/// @param sz The length of the token
				/// @return base64 encrypted token.
				static String encrypt(const void *token, size_t sz);

				template <typename T>
				inline static String encrypt(const T &token) {
					return encrypt(&token,sizeof(token));
				}

				/// @brief Decript base64, return token.
				/// @param b64 The Base64 encrypted token. 
				/// @param token The destination token.
				/// @param maxlen The max lenght for token.
				/// @return The size of decripted token.
				static size_t decrypt(const char *b64, void *token, size_t maxlen);

				/// @brief Decript base64, return string.
				/// @param b64 The Base64 encrypted token. 
				/// @return The string with decripted token.
				static String decrypt(const char *b64);

				inline static String decrypt(const std::string &b64) {
					return decrypt(b64.c_str());
				}

				template <typename T>
				inline static size_t decrypt(const char *b64, const T &token) {
					return decrypt(b64,&token,sizeof(token));
				}

				template <typename T>
				inline static size_t decrypt(const std::string &b64, const T &token) {
					return decrypt(b64.c_str(),&token,sizeof(token));
				}

				virtual ~Authentication();

#if __cplusplus >= 202002L

				inline int operator <=>(const Level level) const noexcept {
					return current_level - level;
				}

#else

				inline bool operator ==(const Level level) const noexcept {
					return user.level == level;
				}

				inline bool operator>(const Level level) const noexcept {
					return user.level > level;
				}

				inline bool operator<(const Level level) const noexcept {
					return user.level < level;
				}

				inline bool operator>=(const Level level) const noexcept {
					return user.level >= level;
				}

				inline bool operator<=(const Level level) const noexcept {
					return user.level <= level;
				}

#endif

				inline const char *c_str() const noexcept {
					return user.name.c_str();
				}

				inline bool allow(Level level) const noexcept {
					return user.level >= level;
				}

				inline Level level() const noexcept {
					return user.level;
				}

				inline const char *name() const noexcept {
					return user.name.c_str();
				}

			protected:
				inline void name(const char *name) noexcept {
					user.name = name;
				}

				/// @brief Set login name to 'email', update access level.
				/// @param email The user e-mail.
				/// @return The new access level.
				virtual Level login(const char *email) noexcept;
				
			private:

				struct {
					Level level = None;
					std::string name;
				} user;


		};

 }

 namespace std {

	UDJAT_API const char * to_string(const Udjat::Authentication::Level level);

	inline ostream & operator<< (ostream& os, const Udjat::Authentication::Level level) {
		return os << to_string(level);
	}


 }