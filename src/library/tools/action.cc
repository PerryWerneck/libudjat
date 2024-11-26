/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
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
  * @brief Implements the abstract action.
  */

 #include <config.h>
 #include <udjat/defs.h>
 #include <udjat/tools/action.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/value.h>
 #include <udjat/tools/container.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/script.h>
 #include <udjat/tools/timestamp.h>
 #include <udjat/tools/protocol.h>
 #include <udjat/tools/http/exception.h>
 #include <list>
 #include <sys/stat.h>
 #include <fstream>
 #include <stdexcept>

 using namespace std;

 namespace Udjat {

	static Container<Action::Factory> & Factories() {
		static Container<Action::Factory> instance;
		return instance;
	}

	Action::Factory::Factory(const char *n) : name{n} {
		Factories().push_back(this);
	}

	Action::Factory::~Factory() {
		Factories().remove(this);
	}

	const std::list<Action::Factory *>::const_iterator Action::Factory::begin() {
		return Factories().begin();
	}

	const std::list<Action::Factory *>::const_iterator Action::Factory::end() {
		return Factories().end();
	}

	bool Action::Factory::for_each(const std::function<bool(Action::Factory &factory)> &func) noexcept {

		for(auto factory : Factories()) {

			try {

				if(func(*factory)) {
					return true;
				}

			} catch(const std::exception &e) {

				Logger::String{e.what()}.error(factory->name);

			} catch(...) {

				Logger::String{"Unexpected error"}.error(factory->name);

			}


		}

		return false;
	}

	std::shared_ptr<Action> Action::Factory::build(const XML::Node &node, const char *attrname, bool except) {

		const char *type = node.attribute(attrname).as_string();

		if(!(type && *type)) {
			Logger::String message{"Required attribute '",attrname,"' is missing or empty"};
			if(except) {
				throw logic_error(message);
			}
			message.warning(node.attribute("name").as_string(PACKAGE_NAME));
			return std::shared_ptr<Action>();
		}

		debug("Searching for action backend '",type,"'");

		try {

			//
			// Check for factories
			//
			for(const auto factory : Factories()) {
				if(strcasecmp(type,factory->name)) {
					continue;
				}
				
				auto action = factory->ActionFactory(node);
				if(action) {
					return action;
				}
			}

			//
			// Build internal actions
			//
			if(strcasecmp(type,"shell") == 0 || strcasecmp(type,"script") == 0 || strcasecmp(type,"shell-script") == 0) {
				// Script action.
				return make_shared<Script>(node);
			}

			if(strcasecmp(type,"file") == 0) {

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
							filename{String{node,"path"}.as_quark()},
							text{payload(node)}, 
							mimetype{MimeTypeFactory(String{node,"output-format","text"}.c_str())},
							maxage{(time_t) TimeStamp{node,"max-age",(time_t) 0}}  {

						if(!(filename && *filename)) {
							throw runtime_error("Required attribute 'filename' is missing or empty");
						}
					}

					int call(const Udjat::Value &request, Udjat::Value &response, bool except) override {

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
								info() << "Removing " << name << endl;
								remove(name.c_str());
							}

							std::ofstream ofs;
							ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
							ofs.open(name, ofstream::out | ofstream::app);
							if(text && *text) {
								ofs << String{text}.expand(request) << endl;
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

			if(strcasecmp(type,"url") == 0) {

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

					int call(const Udjat::Value &request, Udjat::Value &response, bool except) override {

						return exec(response,except,[&](){

							// Get payload
							String payload{text};
							if(payload.empty()) {
								payload = request.to_string(mimetype);
							} else {
								payload.expand(request);
							}

							String response = Protocol::call(
													String{url}.expand(request).c_str(),
													method,
													payload.c_str()
												);

							if(!response.empty()) {
								Logger::String{response.c_str()}.write(Logger::Trace,name());
							}

							return 0;
						});

					}

				};
				return make_shared<URLAction>(node);
			}

		} catch(const std::exception &e) {

			if(except) {
				throw;
			}
			Logger::Message{e.what()}.warning(node.attribute("name").as_string(PACKAGE_NAME));
			return std::shared_ptr<Action>();

		} catch(...) {

			Logger::Message message{"Unexpected error building action"};
			if(except) {
				throw logic_error(message);
			}
			message.warning(node.attribute("name").as_string(PACKAGE_NAME));
			return std::shared_ptr<Action>();

		}

		//
		// Cant find factory, return empty action.
		//
		Logger::String message{"Cant find backend for action type '",type,"'"};
		
		if(except) {
			throw logic_error(message);
		}
		message.warning(node.attribute("name").as_string(PACKAGE_NAME));
		return std::shared_ptr<Action>(); // Legacy

	}

	Action::Action(const XML::Node &node)
		: NamedObject{node}, title{String{node,"title"}.as_quark()} {
	}
	
	Action::~Action() {
	}

	void Action::call(Udjat::Request &, Udjat::Response &response) {
		call(response,response);
	}

	void Action::call(const XML::Node &node) {
		Udjat::Value value;
		call(value,value);
	}

	const char * Action::payload(const XML::Node &node, const char *attrname) {
		String child(node.child_value());
		if(child.empty()) {
			child = node.attribute("payload").as_string();
		}
		child.expand(node);
		if(getAttribute(node,"strip-payload").as_bool(true)) {
			child.strip();
		}
		return child.as_quark();
	}

	int Action::exec(Udjat::Value &value, bool except, const std::function<int()> &func) {

		try {

			int rc = func();
			value[name()] = rc;
			return rc;

		} catch(const HTTP::Exception &e) {

			value[name()] = e.code(); 
			if(except) {
				throw;
			}
			error() << e.what() << endl;
			return e.code();

		} catch(const std::system_error &e) {

			value[name()] = e.code().value(); 
			if(except) {
				throw;
			}
			error() << e.what() << endl;
			return e.code().value();

		} catch(const std::exception &e) {

			value[name()] = -1; 
			if(except) {
				throw;
			}
			return -1;

		} catch(...) {

			value[name()] = -1; 
			if(except) {
				throw;
			}
			return -1;

		}

	}

 }
