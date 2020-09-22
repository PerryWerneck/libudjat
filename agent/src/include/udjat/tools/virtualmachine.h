#ifndef VIRTUAL_MACHINE_H_INCLUDED

	#define VIRTUAL_MACHINE_H_INCLUDED

	#include <udjat/defs.h>
	#include <string>

	class UDJAT_API VirtualMachine {
	public:

		enum CpuID : uint8_t {
			BARE_METAL,			///< @brief Running on bare metal
			VMWARE,				///< @brief Running on VMWare
			VPC,				///< @brief Running on Virtual PC
			BHIVE,				///< @brief Running on BHIVE
			XEN,				///< @brief Running on XEN
			KVM,				///< @brief Running on KVM
			QEMU,				///< @brief Running on QEMU
			LKVM,				///< @brief Running on LKVM
			VMM					///< @brief Running on VMM
		};

		VirtualMachine();

		CpuID getID() const;

		operator bool() const;

		const std::string to_string() const;
		const char * c_str() const;

	};

namespace std {

	inline string to_string(const VirtualMachine &vm) {
		return vm.to_string();
	}

	inline ostream& operator<< (ostream& os, const VirtualMachine &vm) {
		return os << vm.to_string();
	}

}


#endif // VIRTUAL_MACHINE_H_INCLUDED
