#include "ACC.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;

int main()
{
	
	//create an ACC object.
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	acc.alignLVDS();
	std::this_thread::sleep_for(1s);


	return 0;
}