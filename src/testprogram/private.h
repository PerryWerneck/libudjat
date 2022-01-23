
#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <iostream>
	#include <memory>

	#ifdef HAVE_UNISTD_H
		#include <unistd.h>
	#endif // HAVE_UNISTD_H

	using namespace std;
	using namespace Udjat;

	#ifdef HAVE_CIVETWEB

		void run_civetweb();

	#endif // HAVE_CIVETWEB

#endif // PRIVATE_H_INCLUDED
