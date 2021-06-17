

 #include "private.h"
 #include <ctime>
// #include <cstring>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void Response::setExpirationTimestamp(const time_t time) {

		if(expiration) {
			expiration = min(expiration,time);
		} else {
			expiration = time;
		}

	}

	void Response::setModificationTimestamp(const time_t time) {

		if(modification)
			modification = min(modification,time);
		else
			modification = time;

	}

}

