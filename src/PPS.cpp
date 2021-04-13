#include "ACC.h"
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <bitset>
#include <unistd.h>
#include <limits>
#include <chrono> 
#include <iomanip>
#include <numeric>
#include <ctime>
#include <vector>

string getTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y%d%m_%H%M%S");
    return ss.str();
}

int main()
{
	int retval;
	unsigned int boardmask;
	int calibMode;
	bool rawBool;
	int eventCounter;
	int eventNumber;
  vector<int> boards = {};
	string timestamp;

	ACC acc;

	system("mkdir -p Results");
  
  boardmask = 0xFF;
  calibMode = 0;
  rawBool = false;
  eventNumber = 100;
  
  retval = acc.initializeForDataReadout(0, boardmask, calibMode);
  if(retval != 0)
	{
		cout << "Initialization failed!" << endl;
		return 0;
	}
  acc.emptyUsbLine();
	acc.dumpData();
  
  //--------------------------------------------------------------------
  acc.setSoftwareTrigger(boards);
  timestamp = getTime();
  while(eventCounter<eventNumber)
	{
    acc.softwareTrigger();
    retval = acc.listenForAcdcData(1, rawBool, timestamp);
    if(retval==0){eventCounter++;}
    else{acc.dumpData();}
  }
  
  //--------------------------------------------------------------------
  acc.setHardwareTrigSrc(9, boardmask)
  eventCounter=0;
	timestamp = getTime();
  while(eventCounter<eventNumber)
	{
    acc.softwareTrigger();
    retval = acc.listenForAcdcData(9, rawBool, timestamp);
    if(retval==0){eventCounter++;}
    else{acc.dumpData();}
  } 
  
	return 1;
}
