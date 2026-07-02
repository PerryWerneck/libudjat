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
 
 namespace Udjat {

		/// @brief Authentication token
		class UDJAT_API Authentication {
			public:
				
				/// @brief Reset authentication tokens.
				static void reset();

				enum Level : uint16_t {
					None,	///< @brief Non authenticated user.
					Guest,  ///< @brief Guest/Viewer: Read-only access to specific resources.
					User,	///< @brief User/Member: Can create, edit, and view their own data, but cannot see global settings.
					Admin,	///< @brief Admin/Manager: Can invite/remove regular users, change application settings, and manage content.Standard 
					Owner,	///< @brief Owner/Super Admin: Full system control, billing management, and account deletion. (Strictly 1 or 2 users).
				
					Count	///< @brief How many authentication levels we have?
				};
			
				Authentication(Level level = None);
				Authentication(const char *username, Level level = Guest);

				/// @brief Encrypt token, return base64.
				/// @param token The token to encrypt.
				/// @return base64 encrypted token.
				static std::string encrypt(const std::string &token);

				/// @brief Decript base64, return token.
				/// @param b64 The Base64 encrypted token 
				/// @return The decrypted token.
				static std::string decrypt(const std::string &b64);

				virtual ~Authentication();

#if __cplusplus >= 202002L

				inline int operator <=>(const Level level) const noexcept {
					return current_level - level;
				}

#else

				inline bool operator ==(const Level level) const noexcept {
					return current_level == level;
				}

				inline bool operator>(const Level level) const noexcept {
					return current_level > level;
				}

				inline bool operator<(const Level level) const noexcept {
					return current_level < level;
				}

				inline bool operator>=(const Level level) const noexcept {
					return current_level >= level;
				}

				inline bool operator<=(const Level level) const noexcept {
					return current_level <= level;
				}

#endif

				inline const char *c_str() const noexcept {
					return username.c_str();
				}

			private:
				Level current_level = None;
				std::string username;


		};

 }
