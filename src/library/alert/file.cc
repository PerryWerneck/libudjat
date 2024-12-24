/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/alert.h>
 #include <udjat/alert/file.h>
 #include <udjat/tools/logger.h>
 #include <sys/stat.h>
 #include <fstream>
 #include <udjat/tools/string.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/timestamp.h>

 using namespace std;

 namespace Udjat {

	FileAlert::FileAlert(const XML::Node &node) : Alert{node},
		filename{String{node,"filename"}.as_quark()}, maxage{node.attribute("maxage").as_uint(86400)}, 
		payload{Activatable::payload(node)} {

		if(!(filename && *filename)) {
			throw runtime_error(String{"Required attribute 'filename' is empty on alert '",name(),"'"});
		}

		if(!(payload.tmpl && *payload.tmpl)) {
			throw runtime_error(String{"Required payload is empty on alert '",name(),"'"});
		}

		debug("Alert file set to '",filename,"'");

	}

	FileAlert::~FileAlert() {
	}	

	void FileAlert::reset(bool active) noexcept {
		if(!active) {
			payload.value.clear();
		}
		super::reset(active);
	}

	bool FileAlert::activate() noexcept {
		payload.value = payload.tmpl;
		payload.value.expand();
		return super::activate();
	}

	bool FileAlert::activate(const Udjat::Abstract::Object &object) noexcept {
		payload.value = payload.tmpl;
		payload.value.expand(object,true,false);
		return super::activate();
	}

	int FileAlert::emit() {

		String name{filename};
		name.expand();

		if(strchr(name.c_str(),'%')) {
			name = TimeStamp().to_string(name.c_str());
		}

		if(maxage) {
			struct stat st;
			if(!stat(name.c_str(),&st) && (time(nullptr) - st.st_mtime) > maxage) {
				// Its an old file, remove it
				Logger::String{"Removing ",name.c_str()}.info(this->name());
				remove(name.c_str());
			}
		}

		std::ofstream ofs;
		ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

		ofs.open(name, ofstream::out | ofstream::app);
		ofs << payload.value << endl;
		ofs.close();

		return 0;

	}

 }