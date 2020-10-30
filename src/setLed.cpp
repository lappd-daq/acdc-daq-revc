#include "ACC.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		cout << "Usage: ./bin/setLed <1/0>" << endl;
		cout << "1 means LEDs go on, 0 means LEDS go off" << endl;
		return 0;
	}
	

	int tog = atoi(argv[1]);
	bool toggle = (bool)tog; 
	//create an ACC object.
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects
	acc.setLed(toggle);

	return 0;
}