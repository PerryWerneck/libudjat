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
 #include <udjat/tools/file/path.h>
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
	//
	// References:
	//
 	// https://github.com/GNOME/gtk/blob/main/docs/iconcache.txt
	// https://github.com/GNOME/gtk/blob/main/gtk/gtkiconcache.c
	//
 public:

	enum Flag {
		NONE = 0,
		XPM_SUFFIX = 1 << 0,
		SVG_SUFFIX = 1 << 1,
		PNG_SUFFIX = 1 << 2,
		HAS_ICON_FILE = 1 << 3,
		SYMBOLIC_PNG_SUFFIX = 1 << 4,
	};

 private:

	size_t length = 0;
 	uint8_t *data = nullptr;
 	std::string path;

 	inline uint32_t get_uint32(uint32_t offset) const {
		return htonl(*((uint32_t *)(data+offset)));
 	}

  	inline uint32_t get_uint16(uint32_t offset) const {
		return htons(*((uint16_t *)(data+offset)));
 	}

  	inline const char * get_string_ptr(uint32_t offset) const {
		return (const char *) (data+offset);
 	}

  public:

	/// @brief Build cache from index file.
	Cache(const char *filename) : path{filename} {

		{
			auto lptr = path.rfind('/');
			if(lptr != string::npos) {
				path.resize(lptr+1);
			}
		}

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

		debug(filename,": ",get_uint16(0),".",get_uint16(2));

	}

	~Cache() {
		if(data) {
			munmap(data,length);
		}
	}

	/// @brief Get directory from index.
	const char * dirname(const uint16_t index) const {

		auto dir_list_offset = get_uint32(8);
		auto n_dirs = get_uint32(dir_list_offset);
		if(index > n_dirs) {
			throw runtime_error("Unexpected directory index");
		}

		auto dir_name_offset = get_uint32(dir_list_offset + 4 + (index*4));
		return get_string_ptr(dir_name_offset);

	}

	std::string find(const char *id, Flag required_flags = SVG_SUFFIX) const {

		const char *name = strchr(id,'/');
		if(name) {
			name++;
		} else {
			name = id;
		}

		debug("Searching for icon '",name,"'");

		auto hash_offset = get_uint32(4);
		auto n_buckets = get_uint32(hash_offset);

		for (uint32_t i = 0; i < n_buckets; i++) {

			auto chain_offset = get_uint32(hash_offset + 4 + 4 * i);
			while (chain_offset != 0xffffffff) {

				auto name_offset = get_uint32(chain_offset + 4);

				debug("- ",get_string_ptr(name_offset));
				if(!strcasecmp(name,get_string_ptr(name_offset))) {

					auto image_list_offset = get_uint32(chain_offset + 8);
					auto n_images = get_uint32(image_list_offset);
					for(uint32_t j = 0; j < n_images; j++) {
						auto flags = get_uint16(image_list_offset + 4 + 8 * j + 2);
#ifdef DEBUG
						cout 	<< "-    Flags: "
								<< (flags & XPM_SUFFIX ? "xpm " : "")
								<< (flags & SVG_SUFFIX ? "svg " : "")
								<< (flags & PNG_SUFFIX ? "png " : "")
								<< (flags & HAS_ICON_FILE ? "icon " : "")
								<< (flags & SYMBOLIC_PNG_SUFFIX ? "symbolicpng " : "")
								<< "(" << flags << ")" << endl
								<< "-    Path: " << dirname(get_uint16(image_list_offset + 4 + 8 * j)) << endl;
#endif // DEBUG

						if(flags & required_flags) {
							string filename{this->path};
							filename += dirname(get_uint16(image_list_offset + 4 + 8 * j));
							filename += "/";
							filename += get_string_ptr(name_offset);
							filename += '.';
							if(flags & SVG_SUFFIX) {
								filename += "svg";
							} else if(flags & PNG_SUFFIX) {
								filename += "png";
							} else if(flags & XPM_SUFFIX) {
								filename += "xpm";
							} else {
								throw runtime_error("Unsupported image format");
							}
							if(access(filename.c_str(),R_OK) == 0) {
								return filename;
							} else {
								cerr << "Cant find '" << filename << "'" << endl;
							}
						}

					}

					return "";
				}

				chain_offset = get_uint32(chain_offset);
			}

		}

		return "";
	}

 };


 namespace Udjat {

	std::string Icon::filename() const {

		debug("Getting file for icon '",c_str(),"'");

		static const char * defpaths =
				"/usr/share/icons/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/," \
				"/usr/share/icons/Adwaita/," \
				"/usr/share/icons/," \
				"/usr/share/icons/gnome/," \
				"/usr/share/icons/hicolor/," \
				"/usr/share/icons/HighContrast/";

		Config::Value<std::vector<string>> paths("theme","iconpath",defpaths);

		//
		// Search cache files
		//
		for(string &path : paths) {

			// Search for file.
			string filename{path + "icon-theme.cache"};

			if(access(filename.c_str(),R_OK) == 0) {
				filename = Cache{filename.c_str()}.find(c_str());
				if(!filename.empty()) {
					return filename;
				}
			}

		}

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


