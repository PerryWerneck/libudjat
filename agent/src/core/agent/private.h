#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <iostream>
	#include <udjat/agent.h>

	#ifdef HAVE_UNISTD_H
		#include <unistd.h>
	#endif // HAVE_UNISTD_H

	using namespace std;

	namespace Udjat {

		class Abstract::Agent::Controller : public Abstract::Agent {
		public:
			Controller();
			~Controller();

			void refresh() override;

		};


	}



#endif // PRIVATE_H_INCLUDED
