/**
 * @file
 *
 * @brief Test service manager.
 *
 * @author perry.werneck@gmail.com
 *
 */

#include <iostream>
#include <udjat/tools/mainloop.h>

using namespace std;
using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	MainLoop::getInstance().insert(nullptr, 500L, []() {

		cout << "Timer was activated!" << endl;

		return true;
	});

	MainLoop::getInstance().run();

	cout << "Test program ends normally" << endl;
	return 0;

}
