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
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <array>
#include <map>
using namespace std;

class LabelGenerator {
	unsigned count;
public:
	LabelGenerator() {
		count = 0;
	}
	;
	string next() {
		stringstream sstm;
		sstm << "l" << count++;
		return (sstm.str());
	}
	;
};

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
			v.push_back(atoi(line.substr(0, equaldelim).c_str())); // dest
			v.push_back(atoi(line.substr(equaldelim + 1, commadelim).c_str())); // src1
			v.push_back(atoi(line.substr(commadelim + 1, colondelim).c_str())); // src2
			v.push_back(atoi(line.substr(colondelim + 1, line.length()).c_str())); // latency
			data.push_back(v);
		}
		myfile.close();
	}
}

vector<map<int, string>* > constructRenamingMap(vector<vector<int> > data) {

	// create and init the map
	vector<map<int, string>* > rrm ;
	for (unsigned i = 0; i < data.size(); i++) {
		map<int, string>* aMap = new map<int, string>();
		rrm.push_back(aMap);
	}

	LabelGenerator* lblGenerator = new LabelGenerator();
	for (unsigned i = 0; i < data.size(); i++) {
		// populate unfilled entries in current map
		map<int, string>* currMap = rrm[i];
		for (int regId : data[i]) {
			if (currMap->find(regId) == currMap->end()) {
				string nextFreeLbl = lblGenerator->next();
				(*currMap)[regId] = nextFreeLbl;
			}
		}

		// detect RAW dependencies on following maps
		int dest = data[i][0];
		int latency = data[i][3];
		for (unsigned j = i + 1; (j <= i + latency && j < data.size()); j++) {
			if (dest == data[j][1] || dest == data[j][2]) {
				rrm[j][dest] = currMap[dest];
			}
		}
	}

	return rrm;
}

unsigned int run(vector<vector<int> > instructions, vector<map<int, string>* > rrm) {
	unsigned int timeElapsed = 0;
	map<int, set<int> > dependencyMap;
}

int main(int argc, char *argv[]) {
	string infileStr = argv[1];
	if (infileStr.empty()) {
		cout << "Argument needed! Exiting.. " << endl;
		exit(1);
	}
	vector<vector<int> > instructions;
	readData(infileStr, instructions);

	vector<map<int, string>* > rrm = constructRenamingMap(instructions);

	int count = 1;
	for (map<int, string>* aMap : rrm) {
		cout << count++ << endl;
		for (map<int, string>::iterator iterator = aMap->begin(); iterator != aMap->end(); iterator++) {
			cout << "Register:" << iterator->first << "; Label:" << iterator->second << endl;
		}
	}

	return 0;
}
