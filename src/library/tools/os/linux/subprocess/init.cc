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

 //
 // References:
 //
 // https://android.googlesource.com/platform/bionic/+/3884bfe9661955543ce203c60f9225bbdf33f6bb/libc/unistd/system.c
 //

 #include <config.h>
 #include <udjat/defs.h>
 #include <private/linux/subprocess.h>

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <unistd.h>
 #include <udjat/tools/mainloop.h>
 #include <udjat/tools/handler.h>
 #include <udjat/tools/string.h>
 #include <iostream>
 #include <cstring>
 #include <csignal>
 #include <sys/wait.h>
 #include <sys/poll.h>
 #include <udjat/tools/threadpool.h>
 #include <udjat/tools/logger.h>
 #include <ctype.h>

 namespace Udjat {

	void SubProcess::Controller::init(SubProcess &proc, Handler &outpipe, Handler &errpipe) {

		if(proc.pid != -1) {
			throw system_error(EBUSY,system_category(),Logger::Message{"The child process is active on pid {}",proc.pid});
		}

		//
		// Start process.
		//
		int errcode;

		int out[2] = {-1, -1};
		int err[2] = {-1, -1};

		// Create sockets.
		if(socketpair(AF_UNIX, SOCK_STREAM, 0, out) < 0) {
			throw system_error(errno,system_category(),Logger::Message{"Can't create stdout pipes for '{}'",proc.command});
		}

		if(socketpair(AF_UNIX, SOCK_STREAM, 0, err) < 0) {
			errcode = errno;
			::close(out[0]);
			::close(out[1]);
			throw system_error(errno,system_category(),Logger::Message{"Can't create stderr pipes for '{}'",proc.command});
		}

		// Parse arguments
		// http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html
		char * buffer = strdup(proc.command.c_str());
		char * line = chug(buffer);
		char * argv[64];
		size_t argc = 0;

		while(*line) {

			if(argc > 63) {
				throw runtime_error("Too many arguments");
			}

			argv[argc++] = get_next_argument(&line);

		}

		argv[argc] = NULL;

		{
			Logger::String logmsg{"Command line: ["};
			for(size_t ix=0;ix < argc; ix++) {
				strip(argv[ix]);
				if(ix) {
					logmsg.append(" ");
				}
				logmsg.append(argv[ix]);
			}
			logmsg.append("]");
			logmsg.trace(proc.name());
		}

		// Fork new proccess
		sigset_t mask, omask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGCHLD);
		sigprocmask(SIG_BLOCK, &mask, &omask);

		switch (proc.pid = vfork()) {
		case -1: // Error
			sigprocmask(SIG_SETMASK, &omask, NULL);
			errcode = errno;
			::close(out[0]);
			::close(out[1]);
			::close(err[0]);
			::close(err[1]);
			throw system_error(errcode,system_category(),string{"Can't start child for '"} + proc.c_str() + "'");

		case 0:	// child

			sigprocmask(SIG_SETMASK, &omask, NULL);
			if(out[1] != STDOUT_FILENO) {
				(void)dup2(out[1], STDOUT_FILENO);
				(void)close(out[1]);
				out[1] = STDOUT_FILENO;
			}

			if(err[1] != STDERR_FILENO) {
				(void)dup2(err[1], STDERR_FILENO);
				(void)close(err[1]);
				err[1] = STDERR_FILENO;
			}

			try {

				proc.pre();

			} catch(const std::exception &e) {

				cerr << "Pre script has failed on pid " << proc.pid << ": " << e.what() << endl;
				_exit(127);

			} catch(...) {

				cerr << "Unexpected error on pre script for pid " << proc.pid << endl;
				_exit(127);

			}

			execvp(*argv,argv);
			_exit(127);

		}

		sigprocmask(SIG_SETMASK, &omask, NULL);
		free(buffer);

		// Child started, capture pipes.
		outpipe.set(out[0]);
		errpipe.set(err[0]);

	}


 }
