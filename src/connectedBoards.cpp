#include "ACC.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>

using namespace std;

int main()
{
	ACC acc;

	int retval = acc.whichAcdcsConnected(); 
	if(retval==-1)
	{
		std::cout << "Trying to reset ACDC boards" << std::endl;
		acc.resetACDC();
		int retval = acc.whichAcdcsConnected();
		if(retval==-1)
		{
			std::cout << "After ACDC reset no changes, still no boards found" << std::endl;
		}
	}
}
