//============================================================================
// Name        : SSS_Infinite.cpp
// Author      : Chloe Lu
// Version     :
// Copyright   :
// Description : SSS assuming limited instruction window
//============================================================================
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

map<int, set<int>*> constructDependencyMap(vector<vector<int> > instructions, const int windowSize) {
	// dMap should be of window size same as user input
	map<int, set<int>*> dMap;
	for (unsigned i = 0; i < windowSize; i++) {
		set<int>* v = new set<int>();
		dMap[i] = v;
	}

	for (unsigned i = 0; (i < windowSize && i < instructions.size()); i++) {
		int dest = instructions[i][0];
		int latency = instructions[i][3];
		for (unsigned j = i + 1; (j <= i + latency && j < windowSize && j < instructions.size()); j++) {
			if (dest == instructions[j][1] || dest == instructions[j][2]) {
				dMap[j]->insert(i);
			}
		}
	}

	return dMap;
}

void updateDependencyMap(vector<vector<int> > instructions, map<int, set<int>*>& dMap, const int windowSize,
		int& sIndex) {
	// dMap should be of window size same as user input
	int eIndex = sIndex + windowSize - dMap.size();
	for (unsigned i = sIndex; (i < eIndex && i < instructions.size()); i++) {
		dMap[i] = new set<int>();
		// update dependency of new instructions
		cout << "instruction #" << i << " is added to dMap" << endl;
		for (map<int, set<int>*>::iterator it = dMap.begin(); it != dMap.end(); it++) {
			int instrId = it->first;
			if (instrId == i) {
				break;
			}
			int prevDest = instructions[instrId][0];
			if (prevDest == instructions[i][1] || prevDest == instructions[i][2]) {
				dMap[i]->insert(instrId);
				cout << "dependent on #" << instrId << endl;
			}
		}
	}
	sIndex = eIndex;
}

unsigned int run(vector<vector<int> > instr, const int windowSize) {
	unsigned int timeElapsed = 0;
	map<int, set<int>*> dMap = constructDependencyMap(instr, windowSize);
	int sIndex = windowSize;

	while (!dMap.empty()) {
		// find instructions to execute
		set<int> instrToExecute;
		for (map<int, set<int>*>::iterator it = dMap.begin(); it != dMap.end(); it++) {
			int instrId = it->first;
			if (it->second->empty()) {
				instrToExecute.insert(instrId);
			}
		}

		// execute
		for (int instrId : instrToExecute) {
			if (instr.at(instrId)[3] > 1) {
				instr.at(instrId)[3]--;
			} else {
				dMap.erase(dMap.find(instrId));
			}
		}

		//update dependencies in dMap
		for (map<int, set<int>*>::iterator it = dMap.begin(); it != dMap.end(); it++) {
			set<int>* dependency = it->second;
			for (set<int>::iterator setIt = dependency->begin(); setIt != dependency->end();) {
				if (dMap.find(*setIt) == dMap.end()) {
					dependency->erase(setIt++);
				} else {
					++setIt;
				}
			}
		}

		updateDependencyMap(instr, dMap, windowSize, sIndex);

		timeElapsed++;
	}
	return timeElapsed;
}

int main(int argc, char *argv[]) {
	string infileStr = argv[1];
	if (infileStr.empty()) {
		cout << "Argument needed! Exiting.. " << endl;
		exit(1);
	}
	vector<vector<int> > instructions;
	readData(infileStr, instructions);

	string wSizeStr = argv[2];
	if (wSizeStr.empty()) {
		cout << "Window size needed! Exiting.. " << endl;
		exit(1);
	}
	const int windowSize = atoi(argv[2]);

	cout << "# cycles needed:" << endl;
	cout << run(instructions, windowSize) << endl;

	return 0;
}
