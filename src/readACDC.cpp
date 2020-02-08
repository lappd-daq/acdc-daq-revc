#include <iostream>
#include <thread>
#include <chrono>
#include "ACC.h"

using namespace std;

//reads ACDC metadata on all detectable
//boards and prints it to stdout
int main() {

	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	acc.softwareTrigger(); //trigger all boards. 
	return 1;
}