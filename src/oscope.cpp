#include "stdUSB.h"
#include "Scope.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>
#include <limits>

using namespace std;

int main(int argc, char *argv[])
{
	Scope scp;
	int first = 0;
	while(true)
	{
		if(first == 0)
		{	
			scp.plot(false);
			std::cout << "Or here?" << std::endl;
			first++;
		}else
		{
			//std::cout << "Do we get here?" << std::endl;
		}
	}
}