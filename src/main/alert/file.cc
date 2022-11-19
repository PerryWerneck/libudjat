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
 #include <udjat/alert/file.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/logger.h>
 #include <sys/stat.h>
 #include <fstream>

 using namespace std;

 namespace Udjat {

	Alert::File::File(const pugi::xml_node &node, const char *defaults) : Abstract::Alert(node) {

		// Get filename
		{
			auto filename = node.attribute("filename");
			if(!filename) {
				filename = getAttribute(node,"alert-filename",false);
			}

			if(!filename) {
				throw runtime_error(string{"Required attribute 'filename' is missing on alert '"} + name() + "'");
			}

			this->filename = Quark(filename.as_string()).c_str();

			if(!*this->filename) {
				throw runtime_error(string{"Required attribute 'filename' is empty on alert '"} + name() + "'");
			}

		}

		debug("Alert file set to '",filename,"'");

		payload = getPayload(node);

	}

	std::shared_ptr<Udjat::Alert::Activation> Alert::File::ActivationFactory() const {
		return make_shared<Activation>(this);
	}

	Value & Alert::File::getProperties(Value &value) const noexcept {
		Abstract::Alert::getProperties(value);
		value["filename"] = filename;
		return value;
	}

	Value & Alert::File::Activation::getProperties(Value &value) const noexcept {
		Udjat::Alert::Activation::getProperties(value);
		value["filename"] = filename.c_str();
		return value;
	}

	Alert::File::Activation::Activation(const Udjat::Alert::File *alert) : Udjat::Alert::Activation(alert), filename(alert->filename), maxage(alert->maxage), payload(alert->payload) {

		payload.expand(*alert,true,false);

		if(strchr(filename.c_str(),'%')) {
			// Expand filename.
			filename = TimeStamp().to_string(filename);
		}

		filename.expand(*alert,true,false);
	}

	void Alert::File::Activation::emit() {

		filename.expand();
		payload.expand();

		if(verbose()) {
			if(description.empty()) {
				info() << "Emitting " << filename << endl;
			} else {
				info() << description << ": " << filename << endl;
			}
		}

		debug("File=",filename);
		debug("Payload='",payload,"'");

		struct stat st;
		if(!stat(filename.c_str(),&st) && (time(nullptr) - st.st_mtime) > maxage) {
			// Its an old file, remove it
			info() << "Removing " << filename << endl;
			remove(filename.c_str());
		}

		std::ofstream ofs;
		ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);

		ofs.open(filename, ofstream::out | ofstream::app);
		ofs << payload << endl;
		ofs.close();

	}

	Alert::Activation & Alert::File::Activation::set(const Abstract::Object &object) {
		filename.expand(object);
		return *this;
	}

	Alert::Activation & Alert::File::Activation::expand(const std::function<bool(const char *key, std::string &value)> &expander) {
		filename.expand(expander);
		return *this;
	}

 }


