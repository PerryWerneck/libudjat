/**
 * @file virtualmachine.cc
 *
 * @brief Detect virtual machine.
 *
 * Referênces:
 *
 * <https://www.codeproject.com/Articles/9823/Detect-if-your-program-is-running-inside-a-Virtual>
 * <https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html>
 * <https://people.redhat.com/~rjones/virt-what/>
 * <http://artemonsecurity.com/vmde.pdf>
 * <http://git.annexia.org/?p=virt-what.git;a=tree>
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <udjat/tools/virtualmachine.h>
 #include <cstring>

/*---[ Implement ]----------------------------------------------------------------------------------*/

#if defined(__i386__) || defined(__x86_64__)

// http://git.annexia.org/?p=virt-what.git;a=tree
static unsigned int cpuid(unsigned int eax, char *sig) {

	unsigned int *sig32 = (unsigned int *) sig;

	asm volatile (
		"xchgl %%ebx,%1; xor %%ebx,%%ebx; cpuid; xchgl %%ebx,%1"
		: "=a" (eax), "+r" (sig32[0]), "=c" (sig32[1]), "=d" (sig32[2])
		: "0" (eax)
	);
	sig[12] = 0;

	return eax;

}

static VirtualMachine::CpuID translate(const char *sig) {

	static const struct Key {
		VirtualMachine::CpuID	  id;
		const char				* sig;
	} keys[] = {
		{ VirtualMachine::VMWARE,	"VMwareVMware"	},
		{ VirtualMachine::VPC,		"Microsoft Hv"	},
		{ VirtualMachine::BHIVE,	"bhyve bhyve"	},
		{ VirtualMachine::XEN,		"XenVMMXenVMM"	},
		{ VirtualMachine::KVM,		"KVMKVMKVM"		},
		{ VirtualMachine::QEMU,		"TCGTCGTCGTCG"	},
		{ VirtualMachine::LKVM,		"LKVMLKVMLKVM"	},
		{ VirtualMachine::VMM,		"OpenBSDVMM58"	}
	};

	for(size_t ix = 0; ix < (sizeof(keys)/sizeof(keys[0])); ix++) {
		if(!strcmp(sig,keys[ix].sig)) {
			return keys[ix].id;
		}
	}

	return VirtualMachine::BARE_METAL;
}

VirtualMachine::CpuID VirtualMachine::getID() const {

	CpuID rc = BARE_METAL;
	char sig[13];

	unsigned int base = 0x40000000, leaf = base;
	unsigned int max_entries;

	memset (sig, 0, sizeof sig);
	max_entries = cpuid (leaf, sig);
	rc = translate(sig);

	//
	// Most hypervisors only have information in leaf 0x40000000, but
	// upstream Xen contains further leaf entries (in particular when
	// used with Viridian [HyperV] extensions).  CPUID is supposed to
	// return the maximum leaf offset in %eax, so that's what we use,
	// but only if it looks sensible.
	//
	if (rc == BARE_METAL && max_entries > 3 && max_entries < 0x10000) {
		for (leaf = base + 0x100; leaf <= base + max_entries && rc == BARE_METAL; leaf += 0x100) {
			memset (sig, 0, sizeof sig);
			cpuid (leaf, sig);
			rc = translate(sig);
		}
	}

	return rc;

}

#else // !i386, !x86_64

#endif // !i386, !x86_64

VirtualMachine::VirtualMachine() {
}

VirtualMachine::operator bool() const {
	return getID() != BARE_METAL;
}

const char * VirtualMachine::c_str() const {

	static const struct Key {
		VirtualMachine::CpuID	  id;
		const char				* name;
	} keys[] = {
		{ VirtualMachine::VMWARE,	"VMware"		},
		{ VirtualMachine::VPC,		"Microsoft Hv"	},
		{ VirtualMachine::BHIVE,	"bhyve"			},
		{ VirtualMachine::XEN,		"Xen"			},
		{ VirtualMachine::KVM,		"KVM"			},
		{ VirtualMachine::QEMU,		"QEMU"			},
		{ VirtualMachine::LKVM,		"LKVM"			},
		{ VirtualMachine::VMM,		"OpenBSDVMM58"	}
	};

	VirtualMachine::CpuID id = getID();
	if(id == BARE_METAL)
		return "Bare metal";

	for(size_t ix = 0; ix < (sizeof(keys)/sizeof(keys[0])); ix++) {
		if(id == keys[ix].id) {
			return keys[ix].name;
		}
	}

	return "Unknown";
}

const std::string VirtualMachine::to_string() const {
	return std::string(c_str());
}


