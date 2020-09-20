

#include <udjat/tools/dmi.h>
#include <stdexcept>
#include <cstring>
#include <sys/mman.h>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <system_error>

#define BIOS_BASE_ADDRESS 0xF0000
#define BIOS_STRUCT_LENGTH 0x10000

#define WORD(x) (uint16_t)(*(const uint16_t *)(x))
#define DWORD(x) (uint32_t)(*(const uint32_t *)(x))
#define QWORD(x) (*(const uint64_t *)(x))

using namespace std;

namespace Udjat {

	static const char *MEM_DEVICE = "/dev/mem";
	static const char *SMBIOS_STRING_MAGIC = "_SM_";
	static const char *DMI_STRING_MAGIC = "_DMI_";

	/// @brief Copy a memory chunk from /dev/mem into a memory buffer.
	static void * copy_physmem_to_buffer(size_t start, size_t lenght, const char *mem_dev) {
		void *p;
		int fd;
		size_t mmoffset;
		void *mmp;

		fd = open(mem_dev, O_RDONLY);

		if(fd == (-1) ) {
			throw runtime_error("Can't open memory device");
		}

		if((p=malloc(lenght))==NULL) {
			throw runtime_error("Can't allocate memory");
		}

		mmoffset= start % getpagesize();

		mmp=mmap(0, mmoffset+lenght, PROT_READ, MAP_SHARED, fd, start-mmoffset);
		if(mmp==MAP_FAILED) {
			free(p);
			throw runtime_error("MMap error");
		}

		memcpy(p, (uint8_t *)mmp+mmoffset, lenght);

		if(munmap(mmp, mmoffset+lenght)==-1) {
			clog << "munmap error" << endl;
		}

		// No need to check for the return value of 'close', because we've opened the file with O_RDOLNY.
		close(fd);

		return p;
	}

	static int checksum(const uint8_t *buf, size_t len) {

		uint8_t sum=0;
		size_t a;

		for(a=0; a<len; a++)
			sum+=buf[a];

		return (sum==0);

	}

	static void dmi_table(uint32_t base, uint16_t len, uint16_t num, uint16_t ver, const char *mem_dev, uint8_t type, function<void(const Dmi::Header &, uint8_t *)> response) {

		uint8_t *buf;
		uint8_t *data;
		int i=0;

		if((buf= (uint8_t *) copy_physmem_to_buffer(base, len, mem_dev)) == NULL) {
			throw runtime_error("DMI table is unreachable.");
		}

		data=buf;

		// 4 is the length of an SMBIOS structure header
		while(i<num && data+4<=buf+len) {
			uint8_t *next;
			Dmi::Header h;

			{
				h.type=data[0];
				h.length=data[1];
				h.handle=WORD(data+2);
				h.data=data;
			};

			if(h.length<4) {
				clog << "Invalid entry length " << ((int) h.length) << " DMI table is broken!" << endl;
				break;
			}

			// look for the next handle
			next=data+h.length;
			while(next-buf+1<len && (next[0]!=0 || next[1]!=0))
				next++;

			next+=2;

			if(h.type == type) {
				if(next-buf<=len) {
					response(h,data);
				} else {
					clog << "DMI: <TRUNCATED>" << endl;
				}
			}
			data=next;
			i++;
		}
		free(buf);
	}

	static int dmibios_decode(uint8_t *buf, const char *devmem, uint8_t type, function<void(const Dmi::Header &, uint8_t *)> response) {

		if(checksum(buf, 0x0F)) {
			dmi_table(
				DWORD(buf+0x08),
				WORD(buf+0x06),
				WORD(buf+0x0C),
				((buf[0x0E]&0xF0)<<4)+(buf[0x0E]&0x0F),
				devmem,
				type,
				response
			);
			return 1;
		}

		return 0;
	}

	static int smbios_decode(uint8_t *buf, const char *mem_dev, uint8_t type, function<void(const Dmi::Header &, uint8_t *)> response) {

		if(checksum(buf, buf[0x05]) && memcmp(buf+0x10, DMI_STRING_MAGIC, 5)==0 && checksum(buf+0x10, 0x0F)) {
			dmi_table(
				DWORD(buf+0x18),
				WORD(buf+0x16),
				WORD(buf+0x1C),
				(buf[0x06]<<8)+buf[0x07],
				mem_dev,
				type,
				response
			);
			return 1;
		}

		return 0;
	}

	int Dmi::query(Dmi::Type type, function<void(const Header &, uint8_t *)> response) {

		int fp;
		uint8_t *buf;

		if(sizeof(uint8_t)!=1 || sizeof(uint16_t)!=2 || sizeof(uint32_t)!=4 || '\0'!=0)
			throw runtime_error("Invalid binary format");

		buf = (uint8_t *) copy_physmem_to_buffer(BIOS_BASE_ADDRESS, BIOS_STRUCT_LENGTH, MEM_DEVICE);

		for(fp=0; fp<=0xFFF0; fp+=16) {

			if(memcmp(buf+fp, SMBIOS_STRING_MAGIC, 4)==0 && fp<=0xFFE0) {

				if(smbios_decode(buf+fp, MEM_DEVICE, type, response))
					fp+=16;

			} else if(memcmp(buf+fp, DMI_STRING_MAGIC, 5)==0) {

				dmibios_decode(buf+fp, MEM_DEVICE, type, response);

			}

		}

		free(buf);

		return 0;

	}

}

