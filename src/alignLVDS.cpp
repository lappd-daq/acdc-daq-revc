#include "ACC.h"
#include <iostream>

using namespace std;

int main()
{
	
	//create an ACC object.
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	acc.alignLVDS();
	sleep(1);

	return 0;
}