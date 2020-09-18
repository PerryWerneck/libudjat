
#include <config.h>
#include <udjat.h>
#include <iostream>
#include <udjat/tools/atom.h>

using namespace std;

static void atom_test() {

	Udjat::Atom t1("test_value");
	Udjat::Atom t2("test_value");
	Udjat::Atom t3("Another value");

	cout	<< "T1 has pointer " << ((void *) t1.c_str()) << endl
			<< "T2 has pointer " << ((void *) t2.c_str()) << endl
			<< "T3 has pointer " << ((void *) t3.c_str()) << endl
			<< "T1 and T2 values are " << (t1 == t2 ? "the same" : "Not the same") << endl
			<< "T1 and T3 values are " << (t1 == t3 ? "the same" : "Not the same") << endl;



}

int main() {

	atom_test();

	return 0;

}
