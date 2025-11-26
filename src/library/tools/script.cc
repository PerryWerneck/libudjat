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
 #include <udjat/tools/intl.h>
 #include <udjat/tools/xml.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/script.h>
 #include <udjat/tools/quark.h>
 #include <udjat/tools/string.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/file/temporary.h>
 #include <system_error>
 #include <sys/stat.h>

 #ifndef _WIN32
	#include <pwd.h>
	#include <grp.h>
	#include <unistd.h>
 #endif // !_WIN32

 using namespace std;

 namespace Udjat {

#ifdef _WIN32

	Script::Script(const XML::Node &node, const char *msg)
		: 	Action{node},
#ifdef _WIN32
			cmdline{Quark(node,"cmdline","").c_str()} {
#else
			uid{getuid(node)},
			gid{getgid(node)},
			shell{node.attribute("shell").as_bool(false)} {
#endif // !WIN32

		if(!(cmdline && *cmdline)) {
			throw runtime_error(_("The required attribute 'cmdline' is missing"));
		}
	}

	// TODO
	int Script::run(const char *cmdline) const {

		// https://learn.microsoft.com/en-us/windows/win32/procthread/child-processes

		throw system_error(ENOTSUP,system_category(),"Cant run scripts in windows");
	}

#else

 	static int getuid(const XML::Node &node) {

 		const char *user = node.attribute("user").as_string("");

 		if(!(user && *user)) {
			return -1;
 		}

		size_t szBuffer = sysconf(_SC_GETPW_R_SIZE_MAX);
		if(szBuffer == (size_t) -1) {
			szBuffer = 16384;
		}

		char buffer[szBuffer+1];
		memset(buffer,0,szBuffer+1);

		struct passwd pwd;
		struct passwd *result;

		if(getpwnam_r(user, &pwd, buffer, szBuffer, &result) != 0) {
			throw system_error(errno,system_category(),user);
		};

		if(!result) {
			throw system_error(ENOENT,system_category(),user);
		}

		return (int) result->pw_uid;

 	}

 	static int getgid(const XML::Node &node) {

 		const char *group = node.attribute("group").as_string("");

 		if(!(group && *group)) {
			return -1;
 		}

		size_t szBuffer = sysconf(_SC_GETPW_R_SIZE_MAX);
		if(szBuffer == (size_t) -1) {
			szBuffer = 16384;
		}

		char buffer[szBuffer+1];
		memset(buffer,0,szBuffer+1);

		struct group grp;
		struct group *result;

		if(getgrnam_r(group, &grp, buffer, szBuffer, &result) != 0) {
			throw system_error(errno,system_category(),group);
		};

		if(!result) {
			throw system_error(ENOENT,system_category(),group);
		}

		return (int) result->gr_gid;

 	}

	Script::Script(const XML::Node &node, const char *title)
		: 	Action{node},
			cmdline{String{node,"cmdline",""}.as_quark()},
			title{String{node,"title",""}.as_quark()},
			uid{getuid(node)},
			gid{getgid(node)},
			shell{node.attribute("shell").as_bool(false)},
			sudo{node.attribute("sudo").as_bool(false)} {

		if(!(cmdline && *cmdline)) {

			String text{node.child_value()};
			text.strip();

			if(!text.empty()) {
                
				if(text[0] != '#' && text[1] != '!') {
#ifdef BUILD_LEGACY
					throw runtime_error(_("Missing shebang attribute"));
#else	
					throw runtime_error(Logger::Message{_("Missing shebang attribute on {}"),node.path()});
#endif 
				}

				Logger::String{"Using script from XML node"}.trace(name());
				if(node.attribute("strip-lines").as_bool(true)) {
					String stripped;
					text.for_each("\n",[&stripped](const String &value) {
						stripped += const_cast<String &>(value).strip();
						stripped += "\n";
						return false;
					});

					cmdline = stripped.as_quark();

				} else {

					cmdline = text.as_quark();
				}

			} else {

#ifdef BUILD_LEGACY
				throw runtime_error(_("Required attribute 'cmdline' missing"));
#else		
				throw runtime_error(Logger::Message{_("Required attribute 'cmdline' missing on {}"),node.path()});
#endif 			
			}

		}
	}

	int Script::run(const char *cmdline) const {

		class SubProcess : public Udjat::SubProcess {
		private:
			int uid = -1;
			int gid = -1;

		protected:

			void pre() override {

				Udjat::SubProcess::pre();

				if(uid != -1) {
					if(setuid(uid) != 0) {
						throw system_error(errno,system_category(),_("Cant set subprocess user id"));
					}
				}

				if(gid != -1) {
					if(setgid(gid) != 0) {
						throw system_error(errno,system_category(),_("Cant set subprocess group id"));
					}
				}

			}

		public:
			SubProcess(int u, int g, const char *name, const char *command,Logger::Level out, Logger::Level err)
				: Udjat::SubProcess{name,command,out,err}, uid{u}, gid{g} {
			}
		};

		if(title && *title) {
			Logger::String{title}.info(name());
		}

		if(cmdline[0] == '#' && cmdline[1] == '!') {

			// Has shebang, create temporary file
			int rc = -1;

			auto filename = File::Temporary::create(strlen(cmdline));

			try {

				File::Handler{filename.c_str(),true}.write(cmdline,strlen(cmdline));
				chmod(filename.c_str(),0755);

				if(sudo) {

					rc = SubProcess{
						uid,
						gid,
						name(),
						String{
							"/usr/bin/env -S /usr/bin/sudo --login -- ",
							filename.c_str()
						}.c_str(),
						out,
						err
					}.run();

				} else {

					rc = SubProcess{uid,gid,name(),filename.c_str(),out,err}.run();

				}


			} catch(...) {
				unlink(filename.c_str());
				throw;
			}

			unlink(filename.c_str());

			return rc;
		}

		if(sudo) {
			return SubProcess{
				uid,
				gid,
				name(),
				String{
					"/usr/bin/env -S /usr/bin/sudo --login -- ",
					cmdline
				}.c_str(),
				out,
				err
			}.run();
		}

		return SubProcess{uid,gid,name(),cmdline,out,err}.run();

	}

#endif // _WIN32

	Script::~Script() {
	}

	std::string Script::to_string() const noexcept {
		return c_str();
	}

	const char * Script::c_str() const noexcept {
		if(title && *title) {
			return title;
		}
		return cmdline;
	}

	int Script::run(const Udjat::NamedObject &object, bool except) const {
		int rc = run(String{cmdline}.expand(object).c_str());
		if(rc && except) {
			throw runtime_error(Logger::Message{"Script failed with rc {}",rc});
		}
		return rc;
	}

	int Script::call(Udjat::Request &request, Udjat::Response &response, bool except) {
		int rc = run(String{cmdline}.expand(request).c_str());
		response[name()] = rc;
		if(rc) {
			Logger::Message error_message{"Script failed with rc {}",rc};
			if(except) {
				throw runtime_error(error_message);
			}
			error_message.error(name());
		}
		return rc;
	}

 }

