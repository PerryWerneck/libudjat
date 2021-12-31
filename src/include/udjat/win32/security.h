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
 #include <udjat/defs.h>

 #ifndef _WIN32
	#error Service objects requires win32
 #endif // _WIN32

 #define SECURITY_WIN32

 #include <aclapi.h>
 #include <vector>
 #include <string>

 namespace Udjat {

	namespace Win32 {

		/// @brief Windows security identifier (SID)
		class UDJAT_API SecurityIdentifier {
		protected:
			PSID sid;

		public:

			// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ne-winnt-well_known_sid_type
			SecurityIdentifier(WELL_KNOWN_SID_TYPE type);
			SecurityIdentifier(SID_IDENTIFIER_AUTHORITY identifier, const std::vector<DWORD> &dwSubAuthority);

			~SecurityIdentifier();

			void set(PSID sid);

			inline PSID getSID() const {
				return sid;
			}

			std::string to_string() const;

		};

		/// @brief Permission info.
		struct Permission {
			SecurityIdentifier	identifier;

			// https://docs.microsoft.com/en-us/windows/win32/api/accctrl/ne-accctrl-trustee_type
			TRUSTEE_TYPE		TrusteeType;

			// https://docs.microsoft.com/en-us/windows/win32/secauthz/access-mask
			DWORD				grfAccessPermissions;

			// https://docs.microsoft.com/en-us/windows/win32/api/accctrl/ne-accctrl-access_mode
			ACCESS_MODE			grfAccessMode;
		};

		/// @brief Windows Generic security descriptor.
		class UDJAT_API SecurityDescriptor : public SecurityIdentifier {
		private:
			PSECURITY_DESCRIPTOR sd;
			EXPLICIT_ACCESS ea;
			PACL acl;

		public:
			SecurityDescriptor(WELL_KNOWN_SID_TYPE type = WinWorldSid);
			~SecurityDescriptor();

			LPSECURITY_ATTRIBUTES get(SECURITY_ATTRIBUTES &sa);

		};


		/// @brief Windows security descriptor for an object specified by name.
		class UDJAT_API NamedObjectSecurityDescriptor {
		private:
			SE_OBJECT_TYPE			ObjectType;
			std::string				ObjectName;

		public:

			NamedObjectSecurityDescriptor(const char *pObjectName, const SE_OBJECT_TYPE ObjectType);
			~NamedObjectSecurityDescriptor();

			inline const char * c_str() const noexcept {
				return ObjectName.c_str();
			}

			/// @brief Aplica ACLs no objeto.
			void set(PEXPLICIT_ACCESS acl, size_t cCountOfExplicitEntries);

			/// @brief Apply ACLs to object.
			/// @param permissions	Array with the permissions to set.
			/// @param length		Length of the permissions array.
			void set(const Permission *permissions, size_t length);

			/// @brief Set Owner.
			void setOwner(SecurityIdentifier &sid);

			/// @brief Set group.
			void setGroup(SecurityIdentifier &sid);

		};

		/// @brief Windows security descriptor for a file.
		class UDJAT_API FileSecurityDescriptor : public NamedObjectSecurityDescriptor {
		public:
			inline FileSecurityDescriptor(const char *filename) : NamedObjectSecurityDescriptor(filename, SE_FILE_OBJECT) {
			}

			/// @brief Apply file permissions.
			void setProtected();

		};

	}

 }
