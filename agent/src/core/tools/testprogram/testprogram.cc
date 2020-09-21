/**
 * @file
 *
 * @brief Test tools.
 *
 * @author perry.werneck@gmail.com
 *
 */

 #include <iostream>
 #include <udjat/tools/dmi.h>

 using namespace std;
 using namespace Udjat;

//---[ Implement ]------------------------------------------------------------------------------------------

int main(int argc, char **argv) {

	cout 	<< "Processor by value: " << Dmi::Value(Dmi::PROCESSOR_INFO,0x10).as_string() << endl
			<< "Processor by name:  " << Dmi::Value("Processor",0x10).as_string() << endl;


	return 0;
}
