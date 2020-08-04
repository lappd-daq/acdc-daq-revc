#include "ACC.h"
#include <iostream>
#include <unistd.h>
using namespace std;

int main(int argc, char *argv[])
{
	
	//create an ACC object.
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	acc.alignLVDS();
	usleep(1000000);

	return 0;
}