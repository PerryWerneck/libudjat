/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2025 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements the abstract action controller.
  */

 #define LOG_DOMAIN "action"

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/abstract/object.h>
 #include <udjat/tools/activatable.h>
 #include <udjat/tools/actions/abstract.h>
 #include <udjat/tools/script.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <private/action.h>
 #include <udjat/tools/url.h>
 #include <memory>
 #include <sys/stat.h>
 #include <fstream>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	Action::Controller::Controller() : Abstract::Object::Factory{"action"} {
		Logger::String{"Initializing factories"}.trace();
	}

	Action::Controller & Action::Controller::getInstance() {
		static Action::Controller instance;
		return instance;
	}

	static String TypeFactory(const XML::Node &node) {

		static const char *attributes[] = { "action-type", "action-name", "action", "type", "container", "multiple" }; 

		if(node.child("script") || node.child("action")) {
			return "multiple";
		}

		for(const char *attribute : attributes) {

			const char *type = node.attribute(attribute).as_string();
			if(type && *type) {
				return type;
			}

		}

		for(const char *attribute : attributes) {
			String type{node,attribute};
			if(!type.empty()) {
				return type;
			}
		}

		// Convenience attributes
		if(node.attribute("url")) {
			return "url";

		} else if(node.attribute("cmdline")) {
			return "shell";

		} else if(node.attribute("filename")) {
			return "file";

		} else if(!strcasecmp(node.name(),"action")) {
			String type{node,"name"};
			if(!type.empty()) {
				return type;
			}

		} else if(node.child("action")) {
			return "multiple";
			
		}

		throw runtime_error(Logger::String{"Required attribute 'type' is missing at ",node.path()});

	}

	std::shared_ptr<Abstract::Object> Action::Controller::ObjectFactory(const XML::Node &node) const {

		// Get action type
		auto type = TypeFactory(node);

		// Check factories
		for(const auto factory : *this) {
			
			if(*factory == type.c_str()) {
				auto action = factory->ActionFactory(node);
				if(action) {
					return action;
				}
			}

		}

		// Check for internal actions.
		switch(type.select("url","shell","shell-script","file","multiple",nullptr)) {
		case 0: // URL
			{
				/// @brief Call URL when activated.
				class URLAction : public Action {
				private:
					const char *url;
					const HTTP::Method method;
					const char *text = "";
					const MimeType mimetype;

				public:
					URLAction(const XML::Node &node) 
						: 	Action{node}, 
							url{String{node,"url"}.as_quark()},
							method{HTTP::MethodFactory(node,"get")},
							text{payload(node)}, 
							mimetype{MimeTypeFactory(String{node,"payload-format","json"}.c_str())} {

						if(!url && *url) {
							throw runtime_error("Required attribute 'url' is missing or empty");
						}
					}

					int call(Udjat::Request &request, Udjat::Response &response, bool except) override {

						return exec(response,except,[&](){

							// Get payload
							String payload{text};
							if(payload.empty()) {
								payload = request.to_string(mimetype);
							} else {
								payload.expand(request);
							}

							URL url{this->url};
							url.expand(request);

							String response = url.call(method,payload.c_str());

							if(!response.empty()) {
								Logger::String{response.c_str()}.write(Logger::Trace,name());
							}

							return 0;
						});

					}

				};

				return make_shared<URLAction>(node);
			}

		case 1: // shell
		case 2: // shell-script
			{
				// Script action.
				return make_shared<Script>(node);
			}

		case 3: // File
			{
				/// @brief Update filename when activated.
				class FileAction : public Action {
				private:
					const char *filename;
					const char *text = "";
					const MimeType mimetype;
					const time_t maxage;

				public:
					FileAction(const XML::Node &node) 
						: 	Action{node}, 
							filename{String{node,"filename"}.as_quark()},
							text{payload(node)}, 
							mimetype{MimeTypeFactory(String{node,"output-format","text"}.c_str())},
							maxage{(time_t) TimeStamp{node,"max-age",(time_t) 0}}  {

						if(!(filename && *filename)) {
							throw runtime_error("Required attribute 'filename' is missing or empty");
						}
					}

					int call(Udjat::Request &request, Udjat::Response &response, bool except) override {

						return exec(response,except,[&]() {

							// Get filename
							String name{filename};
							if(strchr(name.c_str(),'%')) {
								// Expand filename.
								name = TimeStamp().to_string(name);
							}
							name.expand(request).strip();
							if(name.empty()) {
								throw runtime_error("The target filename is empty");
							}

							// Check filename age, if necessary
							struct stat st;
							if(maxage && !stat(name.c_str(),&st) && (time(nullptr) - st.st_mtime) > maxage) {
								// Its an old file, remove it
								Logger::String{"Removing ",name}.info(Activatable::name());
								::remove(name.c_str());
							}

							std::ofstream ofs;
							ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
							ofs.open(name, ofstream::out | ofstream::app);
							if(text && *text) {
								ofs << String{text}.expand(request,true,false) << endl;
							} else {
								request.serialize(ofs,mimetype);
								ofs << endl;
							}
							ofs.close();
							return 0;
							
						});
					}

				};

				return make_shared<FileAction>(node);

			}

		case 4: // multiple
			{
				/// @brief Container with multiple actions.
				class ActionContainer : public Action {
				private:
					std::vector<std::shared_ptr<Action>> actions;

				public:
					ActionContainer(const Controller *cntrl, const XML::Node &node) : Action{node} {
						
						// Parse standard children
						parse(node);

						// Legacy support for <script> children
						for(auto action = node.child("script"); action; action = action.next_sibling("script")) {
							push_back(cntrl->ObjectFactory(action));
						}
					}

					bool push_back(std::shared_ptr<Abstract::Object> child) override {
						auto action = std::dynamic_pointer_cast<Action>(child);
						if(action) {
							actions.push_back(action);
							return true;
						}
						return Action::push_back(child);
					}

					int call(bool except) override {
						for(auto action : actions) {
							int rc = action->call(except);
							if(rc) {
								return rc;
							}
						}
						return 0;
					}

					int call(Udjat::Request &request, Udjat::Response &response, bool except) override {
						for(auto action : actions) {
							int rc = action->call(request,response,except);
							if(rc) {
								return rc;
							}
						}
						return 0;
					}

				};

				return make_shared<ActionContainer>(this,node);
			}

		default:
			throw runtime_error(Logger::String{"Unexpected or invalid type at ",node.path()});
		}


	}

 }


