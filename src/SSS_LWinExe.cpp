//============================================================================
// Name        : SSS_Infinite.cpp
// Author      : Chloe Lu
// Version     :
// Copyright   :
// Description : Simulates CPU execution with true data dependency.
// 				 Args: <executable> <inFilePath> <windowSize>(optional) <numExecUnits>(optional)
//				 The program assumes unlimited windowSize and numExecutionUnits if the args are missing
//============================================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <array>
#include <map>
#include <limits>
using namespace std;

const int INVALID_OPT = -1;

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

// Util function to read and parse the input data
void readData(string infileStr, vector<vector<int> >& data) {
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

/**
 * Dependency Map: a structure that contains <instructinId, dependencyTuple>
 * instructionId: the id of instr, starting fron 0
 * dependencyTuple: a tuple that stores the id of previous instructions that the current instruction has RAW dependency on
 */
map<int, Tuple*> constructDependencyMap(vector<vector<int> > data, const int windowSize) {
	// create and init the map
	map<int, Tuple*> dMap;
	int dMapSize = windowSize == INVALID_OPT ? data.size() : windowSize;
	for (unsigned i = 0; i < dMapSize; i++) {
		dMap[i] = new Tuple();
		dMap[i]->src1d = -1;
		dMap[i]->src2d = -1;
	}

	for (unsigned i = 0; i < dMapSize; i++) {
		// detect RAW dependencies on following instructions
		int dest = data[i][0];
		for (unsigned j = i + 1; j < dMapSize; j++) {
			if (dest == data[j][1]) {
				dMap[j]->src1d = i;
			}
			if (dest == data[j][2]) {
				dMap[j]->src2d = i;
			}
		}
	}
	return dMap;
}

/**
 * After each cycle, this method is called to update the dMap, reading new set of <windowSize> instructions, and assign the correct dependency tuple
 */
void updateDependencyMap(vector<vector<int> > instructions, map<int, Tuple*>& dMap, const int windowSize, int& sIndex) {
	if (sIndex >= instructions.size()) {
		return; // no need to read more instructions
	}

	int eIndex = sIndex + windowSize;
	for (unsigned i = sIndex; (i < eIndex && i < instructions.size()); i++) {

		dMap[i] = new Tuple();
		dMap[i]->src1d = -1;
		dMap[i]->src2d = -1;
		// update dependency of new instructions
		for (map<int, Tuple*>::iterator it = dMap.begin(); it != dMap.end(); it++) {
			int instrId = it->first;
			if (instrId == i) {
				break;
			}
			int prevDest = instructions[instrId][0];
			if (prevDest == instructions[i][1]) {
				dMap[i]->src1d = instrId;
//				cout << "instruction #" << i << " is dependent on #" << instrId << endl;
			}
			if (prevDest == instructions[i][2]) {
				dMap[i]->src2d = instrId;
//				cout << "instruction #" << i << " is dependent on #" << instrId << endl;
			}
		}
	}
	sIndex = eIndex;
}

unsigned int run(vector<vector<int> > instr, const int windowSize, const int numExecUnitIn) {
	unsigned int timeElapsed = 0;
	map<int, Tuple*> dMap = constructDependencyMap(instr, windowSize);
	int sIndex = windowSize;

	int numExecUnit;
	if (numExecUnitIn==INVALID_OPT){
		numExecUnit = windowSize == INVALID_OPT ? numeric_limits<int>::max() : windowSize;
	} else {
		numExecUnit = numExecUnitIn;
	}

//	cout << "num execution unit:" << numExecUnit << endl;

	set<int> instrToExecute;
	while (!dMap.empty()) {
		// find instructions to execute
		int numNewInstr = 0;
		for (map<int, Tuple*>::iterator it = dMap.begin(); it != dMap.end(); it++) {
			if (numNewInstr >= numExecUnit) {
				break;
			}
			int instrId = it->first;
			if (it->second->src1d == -1 && it->second->src2d == -1) {
				if (instrToExecute.find(instrId) == instrToExecute.end()) {
					instrToExecute.insert(instrId);
					numNewInstr++;
				}
			}
		}

		set<int>::iterator it = instrToExecute.begin();
		while (it != instrToExecute.end()) {
			// copy the current iterator then increment it
			std::set<int>::iterator current = it++;
			int instrId = *current;
			if (instr.at(instrId)[3] > 1) {
				instr.at(instrId)[3]--;
			} else {
				dMap.erase(dMap.find(instrId));
				instrToExecute.erase(current);
			}
		}

		//update dependencies in dMap
		for (map<int, Tuple*>::iterator it = dMap.begin(); it != dMap.end(); it++) {
			Tuple* dependency = it->second;

			if (dependency->src1d != -1 && dMap.find(dependency->src1d) == dMap.end()) {
				dependency->src1d = -1;
			}
			if (dependency->src2d != -1 && dMap.find(dependency->src2d) == dMap.end()) {
				dependency->src2d = -1;
			}
		}

		if (windowSize != INVALID_OPT) {
			updateDependencyMap(instr, dMap, windowSize, sIndex);
		}
//		cout << "map size: " << dMap.size() << endl;
		timeElapsed++;

		if (timeElapsed > 10) {
			return timeElapsed;
		}
	}
	return timeElapsed;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "Argument needed! Exiting.. " << endl;
		exit(1);
	}
	string infileStr = argv[1];
	vector<vector<int> > instructions;
	readData(infileStr, instructions);

	int windowSize;
	int numExe;

	switch (argc) {
	case 2:
		windowSize = INVALID_OPT;
		numExe = INVALID_OPT;
		break;
	case 3:
		windowSize = atoi(argv[2]);
		numExe = INVALID_OPT;
		break;
	case 4:
		windowSize = atoi(argv[2]);
		numExe = atoi(argv[3]);
		break;

	}
	cout << run(instructions, windowSize, numExe) << endl;

	return 0;
}
