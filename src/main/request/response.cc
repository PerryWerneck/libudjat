

 #include "private.h"
 #include <ctime>
// #include <cstring>

//---[ Implement ]------------------------------------------------------------------------------------------

namespace Udjat {

	void ResponseInfo::setExpirationTimestamp(const time_t time) {

		if(expiration) {
			expiration = min(expiration,time);
		} else {
			expiration = time;
		}

	}

	void ResponseInfo::setModificationTimestamp(const time_t time) {

		if(modification)
			modification = min(modification,time);
		else
			modification = time;

	}

}

