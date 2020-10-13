
#ifndef UDJAT_INTERNALS_H_INCLUDED

	#define UDJAT_INTERNALS_H_INCLUDED

	#include <config.h>
	#include <udjat/defs.h>

	#ifdef HAVE_LIBINTL
		#include <locale.h>
		#include <libintl.h>
		#define _( x )                  dgettext(PACKAGE_NAME,x)
		#define N_( x )                 x
	#else
		#define _( x )                  x
		#define N_( x )                 x
	#endif // HAVE_LIBINTL


#endif // UDJAT_H_INCLUDED
