#include "stdUSB.h"
#include "Scope.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[])
{
	Scope scp;
	int c2=0;
	string filename;

		filename = "/home/pipc/Results/Data_b0_evno" + to_string(c2) + ".txt";
		scp.plot(filename);

	while(c2<100){
		scp.send_cmd("pause 0.1");
		scp.send_cmd("reread");
		c2++;
	}
}