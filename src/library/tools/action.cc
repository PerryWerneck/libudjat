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

	static std::shared_ptr<Action> build_action(const XML::Node &node, const char *type, bool except) {

		try {

			if(!(type && *type)) {
				throw runtime_error("Action type is missing or empty");
			}

			//
			// Check for factories
			//
			for(const auto factory : Factories()) {
				
				if(*factory == type) {

					try {

						auto action = factory->ActionFactory(node);
						if(action) {
							return action;
						}

						Logger::String{"Empty response from module while building action '",type,"', ignoring"}.warning();

					} catch(const std::exception &e) {

						Logger::String{"External module failed building action '",type,"': ",e.what()}.error();

					} catch(...) {

						Logger::String{"Unexpected error building action '",type,"'"}.error();

					}
				}
				
			}

			if(strcasecmp(type,"shell") == 0 || strcasecmp(type,"shell-script") == 0) {
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
								remove(name.c_str());
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

					int call(Udjat::Request &request, Udjat::Response &response, bool except) override {

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
		Logger::String error_message{"Cant find backend for action type '",type,"'"};
		if(except) {
			throw logic_error(error_message);
		}
		error_message.warning();
		return std::shared_ptr<Action>();

	}

	std::shared_ptr<Action> Action::Factory::build(const XML::Node &node, bool except) {

		// Check for action/script children
		if(node.child("script") || node.child("action")) {

			/// @brief Execute multiple actions.
			class MultiAction : public Action {
			private:
 				std::vector<std::shared_ptr<Action>> actions;

			public:
				MultiAction(const XML::Node &node) : Action{node} {
					for(const char *nodename : { "action", "script" }) {
						for(auto action = node.child(nodename); action; action = action.next_sibling(nodename)) {
							actions.push_back(build_action(action,String{action,"type"}.c_str(),true));
						}
					}
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

			return make_shared<MultiAction>(node);

		}

		// Check for action-type attribute.
		{
			String type{node,"action-type"};
			if(!type.empty()) {
				return build_action(node,type.c_str(),except);
			}
		}

		// Check for action-name attribute.
		{
			String type{node,"action-name"};
			if(!type.empty()) {
				return build_action(node,type.c_str(),except);
			}
		}

		if(!strcasecmp(node.name(),"action")) {
			String type{node,"name"};
			if(!type.empty()) {
				return build_action(node,type.c_str(),except);
			}
		}

		// Check for 'type' attribute.
		{
			String type{node,"type"};
			if(!type.empty()) {
				return build_action(node,type.c_str(),except);
			}
		}
	
		const char *type = nullptr;		
		for(XML::Node nd = node; nd && !(type && *type);nd = nd.parent()) {
			type = nd.attribute("action-type").as_string();
		}

		if(!(type && *type)) {
			
			const char *name = node.attribute("name").as_string(PACKAGE_NAME);

			if(node.attribute("url")) {

				type = "url";

			} else if(node.attribute("cmdline")) {

				type = "shell";

			} else if(node.attribute("filename")) {

				type = "file";

			} else {

				Logger::String message{"Required attribute 'type' is missing or empty"};
				if(except) {
					throw logic_error(message);
				}
				message.warning(name);
				return std::shared_ptr<Action>();

			}

			Logger::String{"Required type attribute is missing or empty, using '",type,"'"}.warning(name);
		}

		debug("Searching for action backend '",type,"'");

		return build_action(node,type,except);

	}

	Action::Action(const XML::Node &node)
		: Activatable{node}, title{String{node,"title"}.as_quark()} {
	}
	
	Action::~Action() {
	}

	int Action::call(bool except) {
		Udjat::Request request;
		Udjat::Response response;
		return call(request,response,except);
	}

	bool Action::activate(const Udjat::Abstract::Object &object) noexcept {

		try {

			Udjat::Request request;
			object.getProperties(request);

			Udjat::Response response;
			int rc = call(request,response,true);
			if(rc) {
				Logger::String{"Action failed with code ",rc}.error(name());
				return false;
			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(name());
			return false;

		} catch(...) {

			Logger::String{"Unexpected error"}.error(name());
			return false;

		}

		return true;

	}

	bool Action::activate() noexcept {

		try {

			Udjat::Request request;
			Udjat::Response response;
			int rc = call(request,response,true);
			if(rc) {
				Logger::String{"Action failed with code ",rc}.error(name());
				return false;
			}

		} catch(const std::exception &e) {

			Logger::String{e.what()}.error(name());
			return false;

		} catch(...) {

			Logger::String{"Unexpected error"}.error(name());
			return false;

		}

		return true;

	}

	void Action::introspect(const std::function<void(const char *name, const Value::Type type, bool in)> &) const {
	}

	int Action::exec(Udjat::Value &value, bool except, const std::function<int()> &func) {

		try {

			return func();

		} catch(const HTTP::Exception &e) {

			if(except) {
				throw;
			}
			Logger::String{e.what()}.error(name());
			return e.code();

		} catch(const std::system_error &e) {

			if(except) {
				throw;
			}
			Logger::String{e.what()}.error(name());
			return e.code().value();

		} catch(const std::exception &e) {

			if(except) {
				throw;
			}
			Logger::String{e.what()}.error(name());
			return -1;

		} catch(...) {

			if(except) {
				throw;
			}
			Logger::String{"Unexpected error"}.error(name());
			return -1;

		}

	}

 }
