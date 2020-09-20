#ifndef UDJAT_DMI_H_INCLUDED

	#define UDJAT_DMI_H_INCLUDED

	#include <udjat/defs.h>
	#include <string.h>

	namespace Udjat {

		namespace Dmi {

			enum Type : uint8_t {
				BIOS_INFO,				///< @brief 00 "BIOS"
				SYSTEM_INFO,			///< @brief 01 "System",
				BASE_BOARD,				///< @brief 02 "Base Board",
				CHASSIS_INFO,			///< @brief 03 "Chassis",
				PROCESSOR_INFO,			///< @brief 04 "Processor",
				MEMORY_CONTROLLER,		///< @brief 05 "Memory Controller",
				MEMORY_MODULE,			///< @brief 06 "Memory Module",
				DMI_007,				///< @brief 07 "Cache",
				DMI_008,				///< @brief 08 "Port Connector",
				DMI_009,				///< @brief 09 "System Slots",
				DMI_010,				///< @brief 10 "On Board Devices",
				DMI_011,				///< @brief 11 "OEM Strings",
				DMI_012,				///< @brief 12 "System Configuration Options",
				DMI_013,				///< @brief 13 "BIOS Language",
				DMI_014,				///< @brief 14 "Group Associations",
				DMI_015,				///< @brief 15 "System Event Log",
				DMI_016,				///< @brief 16 "Physical Memory Array",
				DMI_MEMORY_DEVICE,		///< @brief 17 "Memory Device",
										// 18 "32-bit Memory Error",
										// 19 "Memory Array Mapped Address",
										// 20 "Memory Device Mapped Address",
										// 21 "Built-in Pointing Device",
										// 22 "Portable Battery",
										// 23 "System Reset",
										// 24 "Hardware Security",
										// 25 "System Power Controls",
										// 26 "Voltage Probe",
										// 27 "Cooling Device",
										// 28 "Temperature Probe",
										// 29 "Electrical Current Probe",
										// 30 "Out-of-band Remote Access",
										// 31 "Boot Integrity Services",
										// 32 "System Boot",
										// 33 "64-bit Memory Error",
										// 34 "Management Device",
										// 35 "Management Device Component",
										// 36 "Management Device Threshold Data",
										// 37 "Memory Channel",
										// 38 "IPMI Device",
										// 39 "Power Supply"
			};

			class UDJAT_API Value {
			public:
				Value(Type type, int offset);

				std::string as_string();


			};

		}


	}

#endif // UDJAT_DMI_H_INCLUDED
