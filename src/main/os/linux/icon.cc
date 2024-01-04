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

 #ifdef HAVE_UNISTD_H
	#include <unistd.h>
 #endif // HAVE_UNISTD_H

 using namespace std;

 namespace Udjat {

	std::string Icon::filename() const {

		debug("Getting file for icon '",c_str(),"'");

		static const char * defpaths =
				"/usr/share/icons/," \
				"/usr/share/icons/" STRINGIZE_VALUE_OF(PRODUCT_NAME) "/," \
				"/usr/share/icons/Adwaita/," \
				"/usr/share/icons/gnome/," \
				"/usr/share/icons/hicolor/," \
				"/usr/share/icons/HighContrast/";

		Config::Value<std::vector<string>> paths("theme","iconpath",defpaths);

		string name{c_str()};
		if(!strchr(name.c_str(),'.')) {
			name += ".svg";
		}

		// First search for filenames.
		for(string &path : paths) {

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

		// Then search paths
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
				cerr << "icon\t" << e.what() << endl;
			}

		}

		Logger::String{"Cant find icon '",c_str(),"'"}.error("ui");
		return name;

	}

 }


