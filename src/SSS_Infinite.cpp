//============================================================================
// Name        : SSS_Infinite.cpp
// Author      : Chloe Lu
// Version     :
// Copyright   :
// Description : SSS assuming infinite resource
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

struct Tuple {
	int src1d;
	int src2d;
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

vector<map<int, string>*> constructRenamingMap(vector<vector<int> > data) {

	// create and init the map
	vector<map<int, string>*> rrm;
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

map<int, Tuple*> constructDependencyMap(vector<vector<int> > data) {
	// create and init the map
	map<int, Tuple*> dMap;
	for (unsigned i = 0; i < data.size(); i++) {
		dMap[i] = new Tuple();
		dMap[i]->src1d=-1;
		dMap[i]->src2d=-1;
	}

	for (unsigned i = 0; i < data.size(); i++) {
		// detect RAW dependencies on following instructions
		int dest = data[i][0];
		for (unsigned j = i + 1; (j < data.size()); j++) {
			if (dest == data[j][1]) {
				cout<< "instruction " <<j << " src1d is dependent on instr "<<i<<endl;
				dMap[j]->src1d = i;
			}
			if (dest == data[j][2]) {
				cout<< "instruction " <<j << " src2d is dependent on instr "<<i<<endl;
				dMap[j]->src2d = i;
			}
		}
	}
	return dMap;
}

unsigned int run(map<int, Tuple* > dMap, vector<vector<int> > instr) {
	unsigned int timeElapsed = 0;
	while (!dMap.empty()) {
		// find instructions to execute
		set<int> instrToExecute;
		for (map<int, Tuple* >::iterator it = dMap.begin(); it != dMap.end(); it++) {
			int instrId = it->first;
			cout<<"instr id=" << instrId << "dependencies: " << it->second->src1d << " " << it->second->src2d << endl;
			if (it->second->src1d==-1 && it->second->src2d==-1) {
				instrToExecute.insert(instrId);
				cout << "cycle #" << timeElapsed << "; executing instr #" << instrId << endl;
			}
		}

		// execute
		for (int instrId : instrToExecute) {
			if (instr.at(instrId)[3] > 1) {
				instr.at(instrId)[3]--;
			} else {
				cout<<"erasing:"<<instrId<<endl;
				dMap.erase(dMap.find(instrId));

			}
		}

		//update dependencies in dMap
		for (map<int, Tuple* >::iterator it = dMap.begin(); it != dMap.end(); it++) {
			Tuple* dependency = it->second;

			if (dependency->src1d != -1 && dMap.find(dependency->src1d) == dMap.end()) {
				dependency->src1d = -1;
			}
			if (dependency->src2d != -1 && dMap.find(dependency->src2d) == dMap.end()) {
				dependency->src2d = -1;
			}
		}

		timeElapsed++;
		cout << "time elapsed:" << timeElapsed << endl;
		if (timeElapsed>10){
			return timeElapsed;
		}
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

	map<int, Tuple* > dMap = constructDependencyMap(instructions);

	cout << "# cycles needed:" << endl;
	cout << run(dMap, instructions) << endl;

	return 0;
}
