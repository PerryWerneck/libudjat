/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
 
		// Check it using IBMTSS
		TSS_CONTEXT *tssContext = NULL;
		TPM_RC rc = 0;

		// 1. Create the TSS Context
		debug("Creating context");
		rc = TSS_Create(&tssContext);
		if (rc != 0) {

			failed = _("Unable to initialize IBM TSS context");

		} else {

			static const struct {
				unsigned int id;
				const char *name;
			} capabilities[] {
				 { 0x0000020D, "phEnable" },
			};

			for(auto capability : capabilities) {

				GetCapability_In in;
				GetCapability_Out out;

				in.capability = TPM_CAP_TPM_PROPERTIES;
				in.property = capability.id;	// Specific ID for required capability
				in.propertyCount = 1;     		// We only need this one

				rc = TSS_Execute(tssContext,
								(RESPONSE_PARAMETERS *)&out,
								(COMMAND_PARAMETERS *)&in,
								NULL,
								TPM_CC_GetCapability,
								TPM_RH_NULL, NULL, 0);
				if(rc) {

					const char *msg, *submsg, *num;
					TSS_ResponseCode_toString(&msg,&submsg,&num,rc);

					Logger::String {
						"IBMTSS error getting ",capability.name,": ",msg," ",submsg
					}.error();

					failed = _("Unable to get current TPM state");
					break;

				} else {

					uint32_t phEnableVal = out.capabilityData.data.tpmProperties.tpmProperty[0].value;

					if(!phEnableVal) {
						Logger::String{"TPM option '",capability.name,"' is disabled"}.error();
						failed = _("TPM is disabled");
						break;
					} else {
						Logger::String{"TPM option '",capability.name,"' is enabled"}.info();
					}
				}

			}

			/*
			// 2. Set up Input (Request) Structure
			GetCapability_In in;
			GetCapability_Out out;

			// We want TPM Properties
			in.capability = TPM_CAP_TPM_PROPERTIES;
			// Starting point: the first variable property (0x200)
			in.property = PT_VAR; 
			// How many properties to return in one call
			in.propertyCount = 10; 

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
				
				printf("Found %u variable properties:\n", props->count);
				for (uint32_t i = 0; i < props->count; i++) {
					printf("Property: 0x%08x | Value: %u\n", 
							props->tpmProperty[i].property, 
							props->tpmProperty[i].value);
				}
			} else {

				const char *msg, *submsg, *num;
				TSS_ResponseCode_toString(&msg,&submsg,&num,rc);

				Logger::String {
					"IBMTSS error: ",msg," ",submsg
				}.error();

				failed = _("Unable to get current TPM state");

			}
			*/

			TSS_Delete(tssContext);
		}


 #endif // HAVE_IBMTSS


		// TPM2_PT_STARTUP_CLEAR:
  		// phEnable:                  1
  		// shEnable:                  1
  		// ehEnable:                  1
		// phEnableNV                 1

		/*
		ESYS_CONTEXT *ctx = nullptr;
		Esys_Initialize(&ctx, nullptr, nullptr);

		TPMS_CAPABILITY_DATA *capabilityData = nullptr;
		TPMI_YES_NO moreData;

		// We request CAP_TPM_PROPERTIES starting from the first VARIABLE property
		TSS2_RC rc = Esys_GetCapability(ctx,
									ESYS_TR_NONE, ESYS_TR_NONE, ESYS_TR_NONE,
									TPM2_CAP_TPM_PROPERTIES, 
									TPM2_PT_VAR_PROPERTIES_FIRST, 
									TPM2_MAX_TPM_PROPERTIES, // Get up to 20 variable properties
									&moreData, 
									&capabilityData);

		if (rc == TSS2_RC_SUCCESS) {
			auto props = capabilityData->data.tpmProperties;
			for (uint32_t i = 0; i < props.count; i++) {
				printProp(props.tpmProperty[i].property, props.tpmProperty[i].value);
			}
			Esys_Free(capabilityData);
		}

		Esys_Finalize(&ctx);
		*/

		/*
		capability_string = "properties-variable",
        .capability        = TPM2_CAP_TPM_PROPERTIES,
        .property          = TPM2_PT_VAR,
        .count             = TPM2_MAX_TPM_PROPERTIES,		
		*/

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