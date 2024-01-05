/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2023 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements linux icon methods.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/ui/icon.h>
 #include <udjat/tools/configuration.h>
 #include <udjat/tools/file.h>
 #include <udjat/tools/logger.h>
 #include <string>
 #include <cstdint>
 #include <sys/mman.h>
 #include <arpa/inet.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <iostream>
 #include <iomanip>

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;
 using namespace Udjat;

 /// @brief Icon Cache file.
 class UDJAT_PRIVATE Cache {
 private:

 	// https://github.com/GNOME/gtk/blob/main/docs/iconcache.txt

 	size_t length = 0;
 	uint8_t *data = nullptr;

 public:
	Cache(const char *filename) {

		int fd = open(filename,O_RDONLY);
		if(fd < 0) {
			throw std::system_error(errno,std::system_category(),filename);
		}

		struct stat st;
		if(fstat(fd,&st) < 0) {
			int err = errno;
			close(fd);
			throw std::system_error(err,std::system_category(),filename);
		}

		length = st.st_size;

		data = (uint8_t *) mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
		if(data == MAP_FAILED) {
			int err = errno;
			close(fd);
			throw std::system_error(err,std::system_category(),filename);
		}

		close(fd);

		debug(filename,": ",get_u16(0),".",get_u16(2));

	}

 	uint16_t get_u16(uint32_t offset) const {
		return htons(*((uint16_t *) (data + offset)));
 	}

  	uint32_t get_u32(uint32_t offset) const {
		return htonl(*((uint32_t *) (data + offset)));
 	}

	template <typename T>
	inline const T * get_ptr(size_t offset) const {
		return (T *) (data+offset);
	}

	/*
	const char * directory(uint32_t index) const {
		uint32_t offset = get_u32(8);
		uint32_t items = get_u32(offset);
		if(index >= items) {
			return nullptr;
		}
		offset += 4;
		return get_ptr(get_u32(offset+(index*4)));
	}
	*/

	/*
	uint32_t bucket(uint32_t index) const {
		uint32_t offset = get_u32(4);
		uint32_t items = get_u32(offset);
		if(index >= items) {
			return 0;
		}
		offset += 4;
		return get_u32(offset+(index*4));
	}
	*/

	~Cache() {
		if(data) {
			munmap(data,length);
		}
	}
 };

 namespace Udjat {

	std::string Icon::filename() const {

		debug("Getting file for icon '",c_str(),"'");

		static const char * defpaths =
				"/usr/share/icons/Adwaita/," \
				"/usr/share/icons/," \
				"/usr/share/icons/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/," \
				"/usr/share/icons/gnome/," \
				"/usr/share/icons/hicolor/," \
				"/usr/share/icons/HighContrast/";

		Config::Value<std::vector<string>> paths("theme","iconpath",defpaths);

#ifdef DEBUG
		//
		// Search on cached themes
		//
		for(string &path : paths) {

			string filename{path + "icon-theme.cache"};

			if(access(filename.c_str(),R_OK) == 0) {

				Cache cache{filename.c_str()};

//#ifdef DEBUG
//				cout << "Hash offset: " << hex << cache.get_u32(4) << " " << cache.get_u32(cache.get_u32(4)) << dec << " items" << endl;
//				cout << "Directory offset: " << hex << cache.get_u32(8) << " " << cache.get_u32(cache.get_u32(8)) << dec << " items" << endl;
//

/*
 00 00 02 2d <-- Tamanho
 00 00 08 c4
 00 00 09 d8
 00 00 0a 58
 00 00 0a e4
 00 00 0c b0
 00 00 0d cc
 00 00 0e 48
 00 00 0f 10
 00 00 0f 6c
 00 00 10 2c
 00 00 12 30
 00 00 13 1c
 00 00 13 60
 00 00 13 f0
 00 00 15 08
 00 00 15 54
 00 00 17 3c
 00 00 17 a4
 00 00 18 14
 00 00 18 50
 00 00 19 08
 00 00 19 50
 00 00 19 8c
 00 00 1a 50
 1b 24 00 00
 1c bc 00 00
 1e 54 00 00
 1f d0 00 00
 20 1c ff ff
*/

//#endif // DEBUG

				{
					const uint32_t *hashptr = cache.get_ptr<uint32_t>(cache.get_u32(4));
					uint32_t items = htonl(*(hashptr++));

					cout << "Items: " << items << hex << "  first offset=" << cache.get_u32(cache.get_u32(4)) << dec << endl;

					uint32_t offset = cache.get_u32(4);
					if(cache.get_u32(offset) != items) {
						throw runtime_error("Items não bate");
					}

					offset += 4;

					//items = 2;
					for(uint32_t item = 0; item < items; item++) {
						const uint32_t *recptr = cache.get_ptr<uint32_t>(htonl(*(hashptr++)));

						const uint32_t recoffset = cache.get_u32(offset);
						cout << item << hex << " offset=" << offset << " [ ";

						auto cptr = cache.get_ptr<uint8_t>(offset);
						for(int x = 0; x < 4; x++) {
							cout << " " << setfill('0') << setw(2) << (int) *(cptr++);
						}
						cout << " ]";


						cout << dec << endl;
						offset += 4;
					}


					/*

01/04/24 20:03:06                Hash offset: c 22d items
01/04/24 20:03:06                Directory offset: 1c2f4 60 items
01/04/24 20:03:06                Hash[0]: 8c8
01/04/24 20:03:06                0 = 16x16/actions
01/04/24 20:03:06                1 = 16x16/apps
01/04/24 20:03:06                2 = 16x16/categories
01/04/24 20:03:06                3 = 16x16/devices
01/04/24 20:03:06                4 = 16x16/emblems

					uint32_t items = cache.get_u32(offset);
					items = 2;
					while(items-- > 0) {
						offset += 4;
						const uint32_t *item = cache.get_ptr<uint32_t>(cache.get_u32(offset));
						cout 	<< "item: " << items << hex << item[1] << dec << endl;
					}
					*/
				}



				/*
				for(size_t ix = 0; cache.directory(ix);ix++) {
					cout << ix << " = " << cache.directory(ix) << endl;
				}
				*/

				/*
				uint32_t offset = cache.get_u32(4);
				uint32_t items = cache.get_u32(offset);
				debug("items=",items);
				while(items--) {
					offset += 4;

					debug("offset=",cache.get_u32(offset));

				}
				debug("--");
				*/


			}
		}

		return "";
#endif // DEBUG

		string name{c_str()};
		if(!strchr(name.c_str(),'.')) {
			name += ".svg";
		}

		//
		// Search for filenames.
		//
		for(string &path : paths) {

			// Search for file.
			string filename{path + name};

			if(access(filename.c_str(),R_OK) == 0) {
				debug("Found '",c_str(),"'");
				return filename;
			}
#ifdef DEBUG
			else {
				debug(filename.c_str()," is invalid");
			}
#endif // DEBUG

		}

		//
		// Search paths
		//
		string filter{"*/"};
		filter += name;

		for(string &path : paths) {

			try {
				File::Path folder{path};
				if(folder && folder.find(filter.c_str(),true)) {
					debug("Found '",folder.c_str(),"'");
					return folder;
				}
#ifdef DEBUG
				else {
					debug("Cant find ",name," at ",folder.c_str());
				}
#endif // DEBUG
			} catch(const std::exception &e) {
				Logger::String{"Icon '",c_str(),"': ",e.what()}.error("ui");
			}

		}

		Logger::String{"Cant find icon '",c_str(),"'"}.error("ui");
		return name;

	}

 }


