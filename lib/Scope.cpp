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
  // need proper destructor at some point..
}

int Scope::init()
{  
  
  gp_cmd = popen("gnuplot -background black " 
		 "-xrm \'gnuplot*borderColor:white\' "
		 , "w"); // open gnuplot
  if(!gp_cmd){
    std::cout << "scope failed to initialize!" << std::endl;
    return (1);
  }
  else{
    std::cout << "opening gnuplot...";
  }
  //time to let gnuplot window open
  usleep(1000000);

  //set up the  graph
  //send_cmd("set timestamp");
  send_cmd("set pointsize .8");

  send_cmd("set autoscale y");
  send_cmd("set xrange [0:256]");

  send_cmd("set grid ls 1");
  send_cmd("set border ls 10");
  send_cmd("set size 1, 1"); 
  send_cmd("set term wxt size 3000, 2000");

  send_cmd("set xlabel \'Sample Number\' ");
  send_cmd("set ylabel \'voltage [mV]\' ");

  std::cout << " Init done" << std::endl;

  return (0);
}

int Scope::plot(string filename)
{
  //check that plot file was successfully written:
  FILE* fcheck = fopen(filename.c_str(), "r");
  if (fcheck == NULL){
    std::cout << "no data to for scope!" << std::endl;
    return (1);
  }
        
	ostringstream cmd_stream;
	
	char chan[50];
	int chan_num[6];
	send_cmd("set multiplot layout 3,3");

	  cmd_stream.str("");
	  cmd_stream << "plot \"" << filename
		     << "\" using ($1):2 ls 2 smooth csplines title \'CHAN 1\',\""
		     << filename
		     << "\" using ($1):3 ls 3 smooth csplines title \'CHAN 2\',\""
		     << filename
		     << "\" using ($1):4 ls 4 smooth csplines title \'CHAN 3\',\""
		     << filename
		     << "\" using ($1):5 ls 5 smooth csplines title \'CHAN 4\',\""
		     << filename
		     << "\" using ($1):6 ls 6 smooth csplines title \'CHAN 5\',\""
		     << filename
		     << "\" using ($1):7 ls 7 smooth csplines title \'CHAN 6\'";  
	  plot_cmd = cmd_stream.str();
	  send_cmd(plot_cmd);

	  cmd_stream.str("");
	  cmd_stream << "plot \"" << filename
		     << "\" using ($1):8 ls 2 smooth csplines title \'CHAN 1\',\""
		     << filename
		     << "\" using ($1):9 ls 3 smooth csplines title \'CHAN 2\',\""
		     << filename
		     << "\" using ($1):10 ls 4 smooth csplines title \'CHAN 3\',\""
		     << filename
		     << "\" using ($1):11 ls 5 smooth csplines title \'CHAN 4\',\""
		     << filename
		     << "\" using ($1):12 ls 6 smooth csplines title \'CHAN 5\',\""
		     << filename
		     << "\" using ($1):13 ls 7 smooth csplines title \'CHAN 6\'";  
	  plot_cmd = cmd_stream.str();
	  send_cmd(plot_cmd);

	  cmd_stream.str("");
	  cmd_stream << "plot \"" << filename
		     << "\" using ($1):14 ls 2 smooth csplines title \'CHAN 1\',\""
		     << filename
		     << "\" using ($1):15 ls 3 smooth csplines title \'CHAN 2\',\""
		     << filename
		     << "\" using ($1):16 ls 4 smooth csplines title \'CHAN 3\',\""
		     << filename
		     << "\" using ($1):17 ls 5 smooth csplines title \'CHAN 4\',\""
		     << filename
		     << "\" using ($1):18 ls 6 smooth csplines title \'CHAN 5\',\""
		     << filename
		     << "\" using ($1):19 ls 7 smooth csplines title \'CHAN 6\'";  
	  plot_cmd = cmd_stream.str();
	  send_cmd(plot_cmd);

	  cmd_stream.str("");
	  cmd_stream << "plot \"" << filename
		     << "\" using ($1):20 ls 2 smooth csplines title \'CHAN 1\',\""
		     << filename
		     << "\" using ($1):21 ls 3 smooth csplines title \'CHAN 2\',\""
		     << filename
		     << "\" using ($1):22 ls 4 smooth csplines title \'CHAN 3\',\""
		     << filename
		     << "\" using ($1):23 ls 5 smooth csplines title \'CHAN 4\',\""
		     << filename
		     << "\" using ($1):24 ls 6 smooth csplines title \'CHAN 5\',\""
		     << filename
		     << "\" using ($1):25 ls 7 smooth csplines title \'CHAN 6\'";  
	  plot_cmd = cmd_stream.str();
	  send_cmd(plot_cmd);
	  
	  cmd_stream.str("");
	  cmd_stream << "plot \"" << filename
		     << "\" using ($1):26 ls 2 smooth csplines title \'CHAN 1\',\""
		     << filename
		     << "\" using ($1):27 ls 3 smooth csplines title \'CHAN 2\',\""
		     << filename
		     << "\" using ($1):28 ls 4 smooth csplines title \'CHAN 3\',\""
		     << filename
		     << "\" using ($1):29 ls 5 smooth csplines title \'CHAN 4\',\""
		     << filename
		     << "\" using ($1):30 ls 6 smooth csplines title \'CHAN 5\',\""
		     << filename
		     << "\" using ($1):31 ls 7 smooth csplines title \'CHAN 6\'";  
	  plot_cmd = cmd_stream.str();
	  send_cmd(plot_cmd);
	  send_cmd("unset multiplot");
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