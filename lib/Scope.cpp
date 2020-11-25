#include <cmath>
#include <sstream>
using std::ostringstream;

#include "Scope.h" 
#include "unistd.h"

Scope::Scope()
{
	init();
}

Scope::~Scope()
{
	
}

int Scope::init()
{  
	gp_cmd = popen("gnuplot -p", "w"); // open gnuplot
	if(!gp_cmd)
	{
		std::cout << "scope failed to initialize!" << std::endl;
		return (1);
	}else
	{
		std::cout << "opening gnuplot..." << std::endl;
	}
	send_cmd("load \"./oscilloscope/settings.gnu\"");
	//time to let gnuplot window open
	usleep(1000000);

	std::cout << " Init done" << std::endl;

	return (0);
}

int Scope::plot(bool rawMode, int bNum)
{	
	if(rawMode)
	{
		send_cmd("load \"./oscilloscope/settings_raw.gnu\"");
	}

	if(bNum == 0)
	{
		send_cmd("load \"./oscilloscope/liveplot.gnu\"");
	}else
	{
		send_cmd("load \"./oscilloscope/liveplot_b2.gnu\"");
	}
	return (0);
}

int Scope::send_cmd(const string &plot_cmd)
{
	ostringstream cmd_stream;
	cmd_stream << plot_cmd << std::endl;
	//std::cout << cmd_stream.str();
	fprintf(gp_cmd, "%s", cmd_stream.str().c_str());   
	fflush(gp_cmd);

	return (0);
}