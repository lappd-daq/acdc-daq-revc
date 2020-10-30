/////////////////////////////////////////////////////
//  file: ScopePipe.h
//
//  Header for ScopePipe c++ class
//
//  Revision History:
//          01/2012 original version 
//               (created 'self-standing' code, 
//                to separate from main program)
//  Author: ejo
////////////////////////////////////////////////////// 
#pragma once

#include <iostream>
#include <fstream>
#include <string>

using std::string;

class Scope {
public:
    Scope();

    ~Scope();

    int init();

    int plot(string filename);

    int send_cmd(const string &plot_cmd);

private:
    FILE *gp_cmd;
    string plot_cmd;
};
