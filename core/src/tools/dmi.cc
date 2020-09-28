/**
 * @file
 *
 * @brief
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <iostream>
 #include <cstring>
 #include <udjat/tools/dmi.h>

 using namespace std;

//---[ Implement ]------------------------------------------------------------------------------------------

 namespace Udjat {

	static const char *typeNames[] = {
		"bios",
		"system",
		"board",
		"chassis",
		"processor",
		"memory-controller",
		"memory-module"
	};

	static Dmi::Type getTypeFromName(const char *name) {

		if(!(name && *name))
			throw runtime_error("Empty DMI name");

		for(uint8_t ix = 0; ix < (sizeof(typeNames)/sizeof(typeNames[0])); ix++) {
			if(!strcasecmp(typeNames[ix],name))
				return (Dmi::Type) ix;

		}

		throw runtime_error("Invalid DMI type");
	}


	Dmi::Value::Value(const char *t, const uint16_t o) : Value(getTypeFromName(t),o) {
	}

 	std::string Dmi::Value::as_string() {

		std::string value;

		Dmi::query(this->type,[this,&value](const Dmi::Header &header, uint8_t *data) {

			char *bp = (char *) header.data;
			size_t i, len;
			uint8_t s = data[this->offset];	// Offset 0x10

			if(s == 0) {
				throw runtime_error("Invalid index");
			}

			bp+=header.length;
			while(s>1 && *bp)
			{
				bp+=strlen(bp);
				bp++;
				s--;
			}

			if(!*bp)
				throw runtime_error("Bad Index");

			// ASCII filtering
			len=strlen(bp);
			for(i=0; i<len; i++) {
				if(bp[i]<32 || bp[i]==127)
					bp[i]='.';
			}

			value.assign(bp);

		});

		return value;
 	}

 }


