#include "ACC.h"
#include <iostream>
#include <chrono>
#include <string>

using namespace std;

//currently written to test timing
//of reading the ACDC buffer. the
//only argument is the number of iterations
//in the loop that reads the acdc buffer
//and then throws it away. 

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		cout << "Usage: ./bin/testFunction <number of test iterations> <filename for output data>" << endl;
		return 0;
	}
	//create an ACC object.
	ACC acc;
	acc.createAcdcs(); //detect ACDCs and create ACDC objects

	int niter = atoi(argv[1]);


	//usb wait statements are still in for the moment. 


	//duration variables
	vector<chrono::microseconds> looptimes; 
	auto start = chrono::high_resolution_clock::now();
	auto end = chrono::high_resolution_clock::now(); //just for initialization
	int corruptBufferCount = 0;
	for(int i = 0; i < niter; i++)
	{
		start = chrono::high_resolution_clock::now();	
		int cbc = acc.testFunction();
		corruptBufferCount = corruptBufferCount + cbc;
		end = chrono::high_resolution_clock::now();	
		looptimes.push_back(chrono::duration_cast<chrono::microseconds>(end - start));
	}

	
	cout << "Got " << corruptBufferCount << " corrupt buffers of " << niter << " iterations " << endl;

	//make a little python file with a list of the times. 
	string fn = string(argv[2]) + ".py";
	cout << "Finished, saving times to file: " << endl;
	ofstream dataofs(fn.c_str(), ios_base::trunc); 
	dataofs << "times = ["; 
	for(int i = 0; i < (int)looptimes.size(); i++)
	{
		dataofs << looptimes.at(i).count();
		if(i < (int)(looptimes.size() - 1))
		{
			dataofs << ",";
		} 
	}
	dataofs << "]";
	dataofs.close();

	return 0;
}