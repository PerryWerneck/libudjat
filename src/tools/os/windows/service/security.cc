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

 /**
  * @brief Implements service security.
  */

 #include <config.h>
 #include <udjat/win32/service.h>
 #include <udjat/win32/exception.h>

 namespace Udjat {

	Win32::Service::Security::Security(SC_HANDLE h) : handle{h} {

		pSD = NULL;
		DWORD dwBytesNeeded	= 0;

		if(!QueryServiceObjectSecurity(handle, DACL_SECURITY_INFORMATION, NULL, 0, &dwBytesNeeded)) {

			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
				throw Win32::Exception("QueryServiceObjectSecurity");
			}

			DWORD dwSize = dwBytesNeeded;
			pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, dwSize);

			if (!QueryServiceObjectSecurity(handle, DACL_SECURITY_INFORMATION, pSD, dwSize, &dwBytesNeeded)) {
				LocalFree(pSD);
				throw Win32::Exception("QueryServiceObjectSecurity");
			}

		}

	}

	Win32::Service::Security::~Security() {
		LocalFree(pSD);
	}

	void Win32::Service::Security::set(PEXPLICIT_ACCESS acl, size_t cCountOfExplicitEntries) {

		PACL				pacl	= NULL;
		PACL				pNewAcl	= NULL;
		DWORD				dwError	= 0;
		SECURITY_DESCRIPTOR	sd;

		try {

			BOOL bDaclDefaulted	= FALSE;
			BOOL bDaclPresent = FALSE;
			if (!GetSecurityDescriptorDacl(pSD, &bDaclPresent, &pacl, &bDaclDefaulted)) {
				throw Win32::Exception("GetSecurityDescriptorDacl");
			}

			dwError = SetEntriesInAcl(cCountOfExplicitEntries, acl, pacl, &pNewAcl);
			if (dwError != ERROR_SUCCESS) {
				throw Win32::Exception("SetEntriesInAcl",dwError);
			}

			if (!InitializeSecurityDescriptor(&sd,  SECURITY_DESCRIPTOR_REVISION)) {
				throw Win32::Exception("InitializeSecurityDescriptor");
			}

			if (!SetSecurityDescriptorDacl(&sd, TRUE, pNewAcl, FALSE)) {
				throw Win32::Exception("SetSecurityDescriptorDacl");
			}

			if (!SetServiceObjectSecurity(handle, DACL_SECURITY_INFORMATION, &sd)) {
				throw Win32::Exception("SetServiceObjectSecurity");
			}

		} catch ( ... ) {

			if(pNewAcl)
				LocalFree((HLOCAL) pNewAcl);

			throw;
		}

		LocalFree((HLOCAL) pNewAcl);

	}

	void Win32::Service::Security::set(const Permission *permissions, size_t length) {

		size_t eaLen = sizeof(EXPLICIT_ACCESS) * sizeof(length);
		PEXPLICIT_ACCESS ea = (PEXPLICIT_ACCESS) malloc(eaLen);
		memset(ea,0,eaLen);

		try {

			for(size_t ix = 0; ix < length; ix++) {

				ea[ix].grfAccessPermissions		= permissions[ix].grfAccessPermissions;
				ea[ix].grfAccessMode			= permissions[ix].grfAccessMode;
				ea[ix].grfInheritance			= NO_INHERITANCE;

				ea[ix].Trustee.TrusteeForm		= TRUSTEE_IS_SID;
				ea[ix].Trustee.TrusteeType		= permissions[ix].TrusteeType;
				ea[ix].Trustee.ptstrName		= (LPSTR) permissions[ix].identifier.getSID();

			}

			this->set(
				ea,
				length
			);


		} catch(...) {

			free(ea);
			throw;

		}

		free(ea);

	}

	/// @brief Disable 'net stop' for all users.
	void Win32::Service::Security::setUnStoppable() {

		// https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights

		/*
 		#define DENY_OPTIONS	SERVICE_STOP|DELETE, \
								DENY_ACCESS
		*/

		#define ALLOW_OPTIONS	SERVICE_QUERY_STATUS|SERVICE_START, \
								SET_ACCESS


		Permission permissions[] = {

			//
			// 'everyone' group permissions.
			//
			/*
			{
				SecurityIdentifier(WinWorldSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				DENY_OPTIONS
			},
			*/

			{
				SecurityIdentifier(WinWorldSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				ALLOW_OPTIONS
			},

			//
			// 'users' group permissions
			//
			/*
			{
				SecurityIdentifier(WinBuiltinUsersSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				DENY_OPTIONS
			},
			*/

			{
				SecurityIdentifier(WinBuiltinUsersSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				ALLOW_OPTIONS
			},

			//
			// 'Authenticated users' group permissions
			//
			/*
			{
				SecurityIdentifier(WinAuthenticatedUserSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				DENY_OPTIONS
			},
			*/

			{
				SecurityIdentifier(WinAuthenticatedUserSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				ALLOW_OPTIONS
			},

			//
			// 'admins' group permissions
			//
			/*
			{
				SecurityIdentifier(WinBuiltinAdministratorsSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				DENY_OPTIONS
			},
			*/

			{
				SecurityIdentifier(WinBuiltinAdministratorsSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				STANDARD_RIGHTS_READ|ALLOW_OPTIONS
			},

			//
			// 'LocalSystem'  group permissions
			//
			{
				SecurityIdentifier(WinLocalSystemSid),
				TRUSTEE_IS_WELL_KNOWN_GROUP,
				GENERIC_ALL|SERVICE_ALL_ACCESS|SERVICE_STOP|DELETE, // GENERIC_ALL dshould be enough, but, 'just in case'
				SET_ACCESS
			},

		};

		this->set(
			permissions,
			sizeof(permissions)/sizeof(permissions[0])
		);

		/*
		EXPLICIT_ACCESS	ea[3];

		ZeroMemory(ea,sizeof(ea));

		// Define permissões do grupo "everyone"
		SecurityIdentifier everyone(SECURITY_WORLD_SID_AUTHORITY, { SECURITY_WORLD_RID });

		ea[0].grfAccessPermissions	= SERVICE_START;
		ea[0].grfAccessMode			= GRANT_ACCESS;
		ea[0].grfInheritance		= NO_INHERITANCE;
		ea[0].Trustee.TrusteeForm	= TRUSTEE_IS_SID;
		ea[0].Trustee.TrusteeType	= TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea[0].Trustee.ptstrName		= everyone.getName();

		// Define permissões do grupo administradores
		SecurityIdentifier admin(SECURITY_NT_AUTHORITY, { SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS });

#if defined(DEBUG)
		// DEBUG or DEVEL: Set full control for Administrators.
		ea[1].grfAccessPermissions	= GENERIC_ALL; // STANDARD_RIGHTS_READ|SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|DELETE;
#else
		// NO-DEBUG: Set read access for Administrators.
		ea[1].grfAccessPermissions	= STANDARD_RIGHTS_READ|SERVICE_QUERY_STATUS|SERVICE_START;
#endif // DEBUG

		ea[1].grfAccessMode			= SET_ACCESS;
		ea[1].grfInheritance		= NO_INHERITANCE;
		ea[1].Trustee.TrusteeForm	= TRUSTEE_IS_SID;
		ea[1].Trustee.TrusteeType	= TRUSTEE_IS_GROUP;
		ea[1].Trustee.ptstrName		= admin.getName();

		// Define permissões do usuário localsystem
		SecurityIdentifier system(SECURITY_NT_AUTHORITY, { SECURITY_LOCAL_SYSTEM_RID });

		ea[2].grfAccessPermissions	= GENERIC_ALL; // STANDARD_RIGHTS_READ|SERVICE_QUERY_STATUS|SERVICE_START|SERVICE_STOP|DELETE;
		ea[2].grfAccessMode			= SET_ACCESS;
		ea[2].grfInheritance		= NO_INHERITANCE;
		ea[2].Trustee.TrusteeForm	= TRUSTEE_IS_SID;
		ea[2].Trustee.TrusteeType	= TRUSTEE_IS_USER;
		ea[2].Trustee.ptstrName		= system.getName();

		// Atualiza ACLs
		this->set(ea, (sizeof(ea)/sizeof(ea[0])));
		*/

	}

 }
