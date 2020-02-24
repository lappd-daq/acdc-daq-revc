#include "Config.h"
#include "ACC.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	if(argc != 2 && argc != 3)
	{
		cout << "Usage: ./bin/setConfig <filename>.yml [-v (verbose mode)]" << endl;
		return 0;
	}
	else
	{
		bool verbose = false;
		string conffile = string(argv[1]);
		//this is the verbose flag
		if(argc == 3)
		{
			string flag = string(argv[2]);
			if(flag == "-v"){verbose = true;}
		}

		//create an ACC object.
		ACC acc;
		acc.createAcdcs(); //detect ACDCs and create ACDC objects

		//create a configuration object
		//using the filename argument of constructor
		Config conf(conffile, verbose); //automatically parses the file
		bool success = false;
		success = conf.writeConfigToAcc(&acc);
		if(!success)
		{
			cout << "Failed setting configuration variables when sending them down USB line" << endl;
			return 0;
		}
		cout << "Done setting config" << endl;
	}


	return 0;
}