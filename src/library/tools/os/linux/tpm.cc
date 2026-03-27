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

 #define LOG_DOMAIN "tpm"

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/crypto.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <string>

 #ifdef HAVE_IBMTSS
 	#define TPM_POSIX  // Tells IBM TSS to use Linux/Unix headers
	extern "C" {
		#include <ibmtss/tss.h>
		#include <ibmtss/tssutils.h>
		#include <ibmtss/tssresponsecode.h>
	}
 #endif // HAVE_IBMTSS
 
 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 #define TPMA_STARTUP_CLEAR_PHENABLE		0x00000001
 #define TPMA_STARTUP_CLEAR_SHENABLE		0x00000002
 #define TPMA_STARTUP_CLEAR_EHENABLE		0x00000004
 #define TPMA_STARTUP_CLEAR_PHENABLENV		0x00000008
 #define TPMA_STARTUP_CLEAR_RESERVED		0x7ffffff0
 #define TPMA_STARTUP_CLEAR_ORDERLY			0x80000000

 using namespace std;

 namespace Udjat {

	UDJAT_API bool TPM::probe(const bool except) {

		if(access("/dev/tpm0",F_OK) != 0) {
			// No TPM device, just return false.
			return false;
		}

		string failed;

		// Found TPM device is it available?

 #if defined(HAVE_IBMTSS)
 
		// Use IBMTSS

		// TODO: Check if access("/dev/tpm0",R_OK) or access("/dev/tpm0",W_OK) works.
		if(!getuid()) {
		
			// Root user, check TPM state
			// Check it using IBMTSS
			TSS_CONTEXT *tssContext = NULL;
			TPM_RC rc = 0;

			// 1. Create the TSS Context
			debug("Creating context");
			rc = TSS_Create(&tssContext);
			if (rc != 0) {

				failed = _("Unable to initialize IBM TSS context");

			} else {

				// 2. Set up Input (Request) Structure
				GetCapability_In in;
				GetCapability_Out out;

				// We want TPM Properties
				in.capability = TPM_CAP_TPM_PROPERTIES;
				// Starting point: the first variable property
				in.property = TPM_PT_STARTUP_CLEAR; 
				// How many properties to return in one call
				in.propertyCount = 1; 

				// 3. Execute the Command
				// NULL handles indicate no specific sessions/auth required for this
				debug("Getting capabilities");
				rc = TSS_Execute(tssContext,
									(RESPONSE_PARAMETERS *)&out,
									(COMMAND_PARAMETERS *)&in,
									NULL,
									TPM_CC_GetCapability,
									TPM_RH_NULL, NULL, 0);

				if (rc == 0) {
					// 4. Parse the output union
					// The data is inside the tpmProperties member of the capabilityData union
					TPML_TAGGED_TPM_PROPERTY *props = &out.capabilityData.data.tpmProperties;

						if(props->tpmProperty[0].property != TPM_PT_STARTUP_CLEAR) {
							
							// Found property, split contents
							failed = _("Unexpected response asking for tpm properties");

						} else {
							
							// Check properties.
							static const struct {
								unsigned int mask;
								const char *name;
							} properties[] = {
								{ TPMA_STARTUP_CLEAR_PHENABLE,		"phEnable"		},
								{ TPMA_STARTUP_CLEAR_SHENABLE,		"shEnable"		},
								{ TPMA_STARTUP_CLEAR_EHENABLE,		"ehEnable"		},
								{ TPMA_STARTUP_CLEAR_PHENABLENV,	"phEnableNV"	},
							};

							for(const auto property : properties) {
								if(props->tpmProperty[0].value & property.mask) {
									Logger::String{"Property '",property.name,"' is ok"}.info();	
								} else {
									Logger::String{"Property '",property.name,"' is disabled"}.error();	
									failed = _("TPM is disabled");
								}
							}

						}

				} else {

					const char *msg, *submsg, *num;
					TSS_ResponseCode_toString(&msg,&submsg,&num,rc);

					Logger::String {
						"IBMTSS error: ",msg," ",submsg
					}.error();

					failed = _("Unable to get current TPM state");

				}

				TSS_Delete(tssContext);

			}

		} else {

			Logger::String{"Non root user, ignoring TPM state checks"}.warning();

		}


 #endif // HAVE_IBMTSS

		if(failed.empty()) {
			return true;
		}

		if(except) {
			throw runtime_error(failed);
		} else {
			Logger::String{failed.c_str()}.error();
		}

		return false;

	}


 }