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

 /***
  * @brief Implement the SubProcess common methods.
  *
  */

 #include <config.h>
 #include <udjat/tools/subprocess.h>
 #include <udjat/tools/logger.h>
 #include <udjat/tools/intl.h>
 #include <iostream>
 #include <udjat/tools/threadpool.h>
 #include <ctype.h>

 using namespace std;

 namespace Udjat {

	SubProcess::Arguments::Arguments(const char *cmdline) {

		while(isspace(*cmdline) && *cmdline) {
			cmdline++;
		}

		while(*cmdline) {

			const char *mark = cmdline+1;
			switch(*cmdline) {
			case '\'':
				mark = strchr(++cmdline,'\'');
				if(!mark) {
					throw runtime_error("Missing required \'");
				}
				break;

			case '\"':
				mark = strchr(++cmdline,'\"');
				if(!mark) {
					throw runtime_error("Missing required \'");
				}
				break;

			default:
				while(*mark && !isspace(*mark)) {
					mark++;
				}
			}

			size_t length = (mark-cmdline);
			char *value = (char *) malloc(length+1);
			strncpy(value,cmdline,length);
			value[length] = 0;
			append(value);

			if(*mark) {
				cmdline = mark+1;
				while(*cmdline && isspace(*cmdline)) {
					cmdline++;
				}
			} else {
				break;
			}

		}

	}

	void SubProcess::Arguments::append(char *arg) {
		if(args.values) {
			args.values = (char **) realloc(args.values,(args.count+2) * sizeof(const char *));
		} else {
			args.values = (char **) malloc(sizeof(const char *) * 2);
			args.count = 0;
		}
		debug("Value(",args.count,")=[",arg,"]");
		args.values[args.count++] = arg;
		args.values[args.count] = NULL;
	}

	SubProcess::Arguments & SubProcess::Arguments::push_back(const char *arg) noexcept {
		append(strdup(arg));
		return *this;
	}

	SubProcess::Arguments & SubProcess::Arguments::push_back(const std::string &value) noexcept {
		push_back(value.c_str());
		return *this;
	}

	SubProcess::Arguments::~Arguments() {
		for(size_t ix = 0; ix < args.count; ix++) {
			free(args.values[ix]);
		}
		free(args.values);
	}

	const char * SubProcess::Arguments::operator[](const char *argname) const {

		while(*argname && *argname == '-') {
			argname++;
		}

		if(!*argname) {
			throw system_error(EINVAL,system_category(),"Empty argument");
		}

		for(const char **arg = argv(); *arg; arg++) {

			const char *ptr = *arg;
			while(*ptr && *ptr == '-')
				ptr++;

			if(*ptr && !strcasecmp(ptr,argname)) {
				return *(arg+1);
			}

		}

		return nullptr;
	}

	const char * SubProcess::Arguments::operator[](const char argname) const {

		if(!argname) {
			throw system_error(EINVAL,system_category(),"Empty argument");
		}

		for(const char **arg = argv(); *arg; arg++) {

			const char *ptr = *arg;
			while(*ptr && *ptr == '-')
				ptr++;

			if(*ptr && *ptr == argname && !(*(ptr+1))) {
				return *(arg+1);
			}

		}

		return nullptr;
	}

 }

 namespace std {

 	string to_string(const Udjat::SubProcess::Arguments &opt, const char *sep) {
		const char **arg = opt.argv();
		string result{*(arg++)};

		while(*arg) {
			result += sep;
			result += *arg;
			arg++;
		}
		return result;
	}

 }
