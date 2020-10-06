#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <iostream>
	#include <udjat/agent.h>
	#include <unordered_map>
	#include <udjat/tools/atom.h>
	#include <pugixml.hpp>

	#ifdef HAVE_UNISTD_H
		#include <unistd.h>
	#endif // HAVE_UNISTD_H

	using namespace std;

	namespace Udjat {

		const char * check_for_reserved_name(const char *);

	}



#endif // PRIVATE_H_INCLUDED
