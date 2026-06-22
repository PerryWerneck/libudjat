/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2026 Perry Werneck <perry.werneck@gmail.com>
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
#include <udjat/tools/loader.h>
#include <udjat/tools/argumentparser.h>
#include <udjat/tools/application.h>
#include <udjat/tools/intl.h>
#include <stdexcept>
#include <udjat/module.h>
#include <private/module.h>
#include <udjat/tools/unit-test.h>
#include <udjat/tools/logger.h>

#ifdef HAVE_PUGIXML
	#include <pugixml.hpp>
	#include <udjat/tools/xml.h>
#endif // HAVE_PUGIXML

#ifndef _WIN32
	#include <dlfcn.h>
	#include <link.h>
#endif // !_WIN32

using namespace std;

namespace Udjat {

#ifdef HAVE_PUGIXML
	static void load_modules(const char *filename) {
		debug("Loading ",filename);
		XML::Document document{filename};
		for(const auto &node : document) {
			for(auto child = node.child("module"); child; child = child.next_sibling("module")) {
				Module::load(child);
			}
		}
	}
#endif // HAVE_PUGIXML

	int UDJAT_API loader(const int argc, const char *argv[], const char *path) {
		return Udjat::loader(argc,argv,[](const LoaderMode, Application &, const char *) {return false;},path);
	}

	int UDJAT_API loader(const int argc, const char *argv[], const std::function<bool(const LoaderMode mode, Application &app, const char *arg)> &init, const char *path) {

		class Loader : public Udjat::Application {
		private:
			const std::string filename;
			const std::function<bool(const LoaderMode mode, Application &app, const char *arg)> &callback;

		protected:
			ArgumentParser & load(ArgumentParser &parser) noexcept override {

				parser.append(
					ArgumentParser::Argument{
						't', "run-tests", _("Run all unit tests"),
						[this](const char *arg, char) {
#ifdef HAVE_PUGIXML
							load_modules(filename.c_str());
#endif // HAVE_PUGIXML							
							if(callback(LOADER_MODE_RUN_TESTS,*this,arg)) {
								return true;
							}
							UnitTests tests;
							tests.load();
							tests.run(arg);
#ifdef HAVE_PUGIXML
							Module::unload();
#endif // HAVE_PUGIXML							
							return true;
						}
					},
					ArgumentParser::Argument{
						'i', "interactive", _("Interactive mode"),
						[this](const char *arg, char) {
#ifdef HAVE_PUGIXML
							load_modules(filename.c_str());
#endif // HAVE_PUGIXML							
							UnitTests tests;
							tests.load();
							tests.interactive();
#ifdef HAVE_PUGIXML
							Module::unload();
#endif // HAVE_PUGIXML							
							return true;
						}
					},
					ArgumentParser::Argument{
						'M', "load-module", _("Load module file"), _("path"),
						[](const char *path, char) {

							if(!(path && *path)) {
								throw runtime_error("Load module requires the module path as argument");
							}

							// TODO: Implement

							return false;
						}
					}
				);

				// Load unit-tests
				{
					UnitTests tests;
					tests.load();

					auto &group = parser.add_group(_("Test options"));

					tests.for_each([&group](const char *option, const char *label){

						if(option && *option) {
							group.emplace_back(
								option, label,
								[option](const char *, char) {
									debug("Calling option '",option,"'");
									UnitTests tests;
									tests.load();
									tests.run(option);
									return true;
								}
							);
						}

					});

				}

				return parser;
			}

		public:
			Loader(const int argc, const char *argv[], const char *path, const std::function<bool(const LoaderMode mode, Application &app, const char *arg)> &cbk) : Udjat::Application{argc,argv}, filename{path}, callback{cbk} {
			}

			std::shared_ptr<Abstract::Agent> RootFactory() override {
				callback(LOADER_MODE_INIT,*this,"");
				return Udjat::Application::RootFactory();
			}

			int run() {
				return Application::run(filename.c_str());
			}

		};

		Logger::verbosity(9);
		Logger::console(true);

		return Loader{argc,argv,path,init}.run();

	}

	int UDJAT_API loader(const int argc, const char *argv[], const std::function<int(Application &app)> &init, const char *path) {

		class Loader : public Udjat::Application {
		private:
			const std::string filename;
			const std::function<int(Application &app)> &callback;

		protected:
			ArgumentParser & load(ArgumentParser &parser) noexcept override {

				parser.append(
					ArgumentParser::Argument{
						'r', "run-tests", _("Run all unit tests"),
						[this](const char *arg, char) {
#ifdef HAVE_PUGIXML
							load_modules(filename.c_str());
#endif // HAVE_PUGIXML							
							UnitTests tests;
							tests.load();
							tests.run(arg);
							return true;
						}
					},
					ArgumentParser::Argument{
						'i', "interactive", _("Interactive mode"),
						[this](const char *arg, char) {
#ifdef HAVE_PUGIXML
							load_modules(filename.c_str());
#endif // HAVE_PUGIXML							
							UnitTests tests;
							tests.load();
							tests.interactive();
							return true;
						}
					},
					ArgumentParser::Argument{
						'M', "load-module", _("Load module from file"), _("path"),
						[](const char *path, char) {

							if(!(path && *path)) {
								throw runtime_error("Load module requires the module path as argument");
							}

							// TODO: Implement

							return false;
						}
					}
				);

				return parser;
			}

		public:
			Loader(const int argc, const char *argv[], const char *path, const std::function<int(Application &app)> &cbk) : Udjat::Application{argc,argv}, filename{path}, callback{cbk} {
			}

			std::shared_ptr<Abstract::Agent> RootFactory() override {
				if(callback(*this)) {
					throw runtime_error{"Initialization failed"};
				}
				return Udjat::Application::RootFactory();
			}

			int run() {
				return Application::run(filename.c_str());
			}

		};

		Logger::verbosity(9);
		Logger::console(true);

		return Loader{argc,argv,path,init}.run();

	}

}