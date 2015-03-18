//============================================================================
// Name        : PlaygroundCpp.cpp
// Author      : Chloe Lu
// Version     :
// Copyright   : 
// Description : Hello World in C++, Ansi-style
//============================================================================
#include "PlaygroundCpp.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
using namespace std;

void readData(string infileStr, vector<vector<int> >& data) {
	// assume data cannot be over than 1000 lines
	string line;
	ifstream myfile(infileStr);
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			size_t equaldelim = line.find_first_of('=');
			size_t commadelim = line.find_first_of(',');
			size_t colondelim = line.find_first_of(':');
			vector<int> v;
			v.push_back( atoi(line.substr(0, equaldelim).c_str())); // dest
			v.push_back( atoi(line.substr(equaldelim + 1, commadelim).c_str())); // src1
			v.push_back( atoi(line.substr(commadelim + 1, colondelim).c_str())); // src2
			v.push_back( atoi(line.substr(colondelim + 1, line.length()).c_str())); // latency
			data.push_back(v);
		}
		myfile.close();
	}
}

int main(int argc, char *argv[]) {
	string infileStr = argv[1];
	if (infileStr.empty()) {
		cout << "Argument needed! Exiting.. " << endl;
		exit(1);
	}
	vector<vector<int> > data;
	readData(infileStr, data);

	for (unsigned i=0; i<data.size(); i++) {
		vector<int> entry = data[i];
		cout << entry[0] << "," << entry[1] << "," << entry[2] << "," << entry[3] << endl;
	}

	return 0;
}
