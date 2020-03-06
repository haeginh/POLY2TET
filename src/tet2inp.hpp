#pragma once
#include <vector>
#include <map>
#include <math.h>
#include <string>
#include <fstream>
#include <iostream>
#include <time.h>
#include <tuple>
#include "ThreeVector.h"

using namespace std;
typedef tuple<int, int, int, int> ELE;
class tet2inp
{
public:
	tet2inp(string inp_filename, string out_filename, vector<ThreeVector> node_vec);
	~tet2inp(void);
	
	template <typename T> vector<size_t> sort_indexes(const vector<T> &v);
	void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
	void PrintCell(string filename, map<int, double> densityMap);
	void PrintTally(string filename, map<int, string> matNameMap);
//	void makeMCNPinp(string out_filename, string mat_filename);

private:
	void convertToINP(string filename);
    const string currentDateTime();

	vector<ThreeVector> node_vector;
	vector<int> region_vector;
	map<int, vector<ThreeVector> > node_map;
	map<int, vector<ELE> > ele_map;
	map<int, vector<ELE> > newele_map;
	map<int, vector<int> > idx_map;
	map<int, double > vol_map;
};

