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

	int exit_in;

  	system("gnuplot ./Results/liveplot.gnu");

}