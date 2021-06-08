#ifndef VIRTUAL_MACHINE_H_INCLUDED

	#define VIRTUAL_MACHINE_H_INCLUDED

	#include <udjat/defs.h>
	#include <string>

	class UDJAT_API VirtualMachine {
	public:
		operator bool() const;
		const std::string to_string() const;

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
