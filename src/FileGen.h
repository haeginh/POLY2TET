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
#include "ThreeVector.h"

using namespace std;
class FileGen {
public:
	FileGen(string phantomName, bool g4_gen, bool mcnp6_gen, bool phits_gen);
	virtual ~FileGen();

private:
	void ReadTetNode();
	void ReadTetEle();
	void GenerateG4();
	void GenerateM6();
	void GeneratePH(){};

private:
	string phantomName;

	bool g4_gen;
	bool mcnp6_gen;
	bool phits_gen;

	int numNode, numEle;
	ThreeVector box_max, box_min, center;
	int beamHX, beamHZ;
	vector<ThreeVector> nodeVec;
};

#endif /* FILEGEN_H_ */
