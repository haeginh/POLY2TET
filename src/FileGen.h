/*
 * FileGen.h
 *
 *  Created on: Feb 24, 2020
 *      Author: haeginh
 */

#ifndef FILEGEN_H_
#define FILEGEN_H_

#include <string>
#include <vector>
#include <map>
#include "ThreeVector.h"

enum MAT{MCNP6, PHITS};
using namespace std;
class FileGen {
public:
	FileGen(string phantomName, string baseDir);
	virtual ~FileGen();
	void ReadMatFile(string matFile, MAT matFormat);
	void GenerateG4();
	void GenerateM6();
	void GeneratePH();

private:
	void ReadTetNode();
	void ReadTetEle();
	void GenerateMCNP6Mat(string fileName);
	void GeneratePhitsMat(string fileName);

private:
	string phantomName;
	string baseDir;

	int numNode;
	ThreeVector box_max, box_min, center;
	int beamHX, beamHZ;
	vector<ThreeVector> nodeVec;

	map<int, double> densityMap;
	map<int, string> matNameMap;
	map<int, vector<pair<int, double>>> matMap;
	map<int, double> volMap;
};

#endif /* FILEGEN_H_ */
