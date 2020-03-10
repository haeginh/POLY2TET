#ifndef POLY2TETH
#define POLY2TETH

#include "tetgen.h" // Defined tetgenio, tetrahedralize().
#include "ThreeVector.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <algorithm>
#include <iomanip>
#include <float.h>

using namespace std;

void FixObj(string fileName); //for HUREL internal use.
int ConvertOBJ(string fileName);
vector<vector<int>> GetRepFaces(vector<vector<int>> faceVec, int numShells); // get representative faces for each shells
vector<pair<ThreeVector, int>> GetRegionList(vector<vector<int>> repFaces, vector<ThreeVector> vVec);
vector<vector<int>> SeparateShell(vector<vector<int>> facePool);
vector<vector<vector<int>>> SeparateAndGetShells(vector<vector<int>> facePool);
void PrintNode(string fileName, vector<ThreeVector>);
void PrintSmesh(string fileName, vector<vector<int>>, vector<pair<ThreeVector, int>>);
//void ReArrangeFaces(string tetFile, vector<ThreeVector>, vector<vector<int>>);
string StringSplitterFirst(string str, string del);
string StringSplitterLast(string str, string del);
void TetgenCall(int p_argc, char ** p_argv);
map<int, double> AnalyzeTet(string tetFile, bool oldVer=false);
bool ChkDegenTet(ThreeVector anchor, ThreeVector p2, ThreeVector p3, ThreeVector p4, double &vol6);
bool ChkDegenTet2(ThreeVector p0, ThreeVector p1, ThreeVector p2, ThreeVector p3, double& vol6);
double FixDegenTet(ThreeVector& anchor, ThreeVector& p2, ThreeVector& p3, ThreeVector& p4, double &vol6);
double FixDegenTet2(ThreeVector& anchor, ThreeVector& p2, ThreeVector& p3, ThreeVector& p4, double &vol6);
double FixDegenTet3(ThreeVector& anchor, ThreeVector& p2, ThreeVector& p3, ThreeVector& p4, double &vol6);
int  PrintVolume(string tetFile, ostream& os);
bool ConvertZeroMat(int rst, string eleFile, int largestR, bool forcedChange);

void FixObj(string fileName) {
	ifstream ifs(fileName);
	string str;
	stringstream ssOut;
	while (getline(ifs, str)) {
		stringstream ss(str);
		string dump;
		ss >> dump;
		if (dump == "g") {
			ss >> dump;
			ssOut << "g " << dump.substr(2, dump.size()) << endl;
		}
		else {
			ssOut << str << endl;
		}
	}
	ifs.close();
	ofstream ofs(fileName.substr(0, fileName.size() - 4) + "_2.obj");
	ofs << ssOut.str();
	ofs.close();
	cout << fileName.substr(0, fileName.size() - 4) + "_2.obj was exported" << endl;
}

int ConvertOBJ(string fileName) {
	ifstream ifs(fileName);
	if (!ifs.is_open()) {
		cerr << "There is no " << fileName << endl;
		exit(1);
	}
	cout << "Read " << fileName <<"..."<< endl;

	string dump, firstStr;
	ThreeVector point;
	vector<ThreeVector> pointVec;
	vector<vector<int>> faceVec;
	vector<int> regionVec;
	int a, b, c, id(0), shellCount(0);
	while (getline(ifs, dump)) {
		stringstream ss(dump);
		if (!(ss >> firstStr)) continue;
		if (firstStr == "v") {
			ss >> point;
			pointVec.push_back(point);
		}
		else if (firstStr == "f") {
			ss >> a >> b >> c;
			faceVec.push_back({ a-1, b-1, c-1, id });
		}
		else if (firstStr == "g") {
			ss >> dump;
			id = atoi(StringSplitterFirst(dump, "_").c_str());
			if (id <= 0) {
				cerr << "Wrong id number " << id << " exists" << endl;
				return -1;
			}
			regionVec.push_back(id);
			shellCount++;
		}
	}
	ifs.close();

	cout << endl<< "   " << pointVec.size() << " vertices" << endl
		<< "   " << faceVec.size() << " faces" << endl
		<< "   " << shellCount << " shells" << endl;

	vector<vector<int>>            repFaces = GetRepFaces(faceVec,shellCount);
	vector<pair<ThreeVector, int>> regions  = GetRegionList(repFaces, pointVec);
	cout <<'\r'<<"   separated into " << repFaces.size() << " shells" << endl << endl;
	PrintNode(fileName.substr(0, fileName.size() - 4) + ".node", pointVec);
	cout << fileName.substr(0, fileName.size() - 4) + ".node was exported" << endl;
//	PrintFace(fileName.substr(0, fileName.size() - 4) + ".face", faceVec);
//	cout << fileName.substr(0, fileName.size() - 4) + ".face was exported" << endl;
	PrintSmesh(fileName.substr(0, fileName.size() - 4) + ".smesh", faceVec, regions);
	cout << fileName.substr(0, fileName.size() - 4) + ".smesh was exported" << endl;

	sort(regionVec.begin(), regionVec.end());
	return regionVec.back();
}

vector<vector<int>> GetRepFaces(vector<vector<int>> faceVec, int numShells) {
	int prevID = faceVec[0][3];
	vector<vector<int>> facePool;
	vector<vector<int>> repFaces;
	int count(1);
	for (size_t i = 0; i < faceVec.size(); i++) {
		facePool.push_back({ faceVec[i][0], faceVec[i][1], faceVec[i][2] });

		if (i==faceVec.size()-1 || faceVec[i][3] != faceVec[i+1][3]) {
			cout<<'\r'<<"   Separating shells..."<<count++<<"/"<<numShells<<flush;
			vector<vector<int>> extracted = SeparateShell(facePool);
			for (vector<int> &extF : extracted) {
				extF.push_back(prevID);
			}
			repFaces.insert(repFaces.end(), extracted.begin(), extracted.end());
			facePool.clear();
		}
		prevID = faceVec[i][3];
	}cout<<endl;
	return repFaces;
}

vector<pair<ThreeVector, int>> GetRegionList(vector<vector<int>> repFaces, vector<ThreeVector> vVec) {
	vector<pair<ThreeVector, int>> regionList;
	for (vector<int> aRepF : repFaces) {
		ThreeVector trans = ((vVec[aRepF[2]] - vVec[aRepF[0]]).cross(vVec[aRepF[1]] - vVec[aRepF[0]])).unit() * 0.00001;
		ThreeVector center = (vVec[aRepF[0]] + vVec[aRepF[1]] + vVec[aRepF[2]]) / 3.;
		regionList.push_back(make_pair(center + trans, aRepF[3]));
	}

	return regionList;
}

vector<vector<int>> SeparateShell(vector<vector<int>> facePool) {
	vector<vector<int>> repFaces;
	
	set<int> vToSearch, vToSearchNext;
	vToSearch.insert(facePool.back().begin(), facePool.back().end());
	vector<vector<int>> rest = facePool;
	repFaces.push_back(rest.back());

	rest.pop_back();

	vector<vector<int>> restNext;
	while (1) { // loop to detect all sepearated shells
		while (1) { // loop for finding all faces for each separated shell
			for (vector<int> f : rest) {
				if (vToSearch.find(f[0]) != vToSearch.end()) {
					vToSearchNext.insert(f[1]);
					vToSearchNext.insert(f[2]);
				}
				else if (vToSearch.find(f[1]) != vToSearch.end()) {
					vToSearchNext.insert(f[0]);
					vToSearchNext.insert(f[2]);
				}
				else if (vToSearch.find(f[2]) != vToSearch.end()) {
					vToSearchNext.insert(f[0]);
					vToSearchNext.insert(f[1]);
				}
				else restNext.push_back(f);
			}
			if (vToSearchNext.size() == 0) break;

			vToSearch = vToSearchNext;
			vToSearchNext.clear();
			rest = restNext;
			restNext.clear();
		}
		if (restNext.size() == 0) break;
		rest = restNext; restNext.clear();

		vToSearch.clear();
		vToSearch.insert(rest.back().begin(), rest.back().end());
		repFaces.push_back(rest.back());
		rest.pop_back();
	}
	return repFaces;
}

vector<vector<int>> SeparateShell(vector<vector<int>> facePool, vector<int>& numFace) {
	vector<vector<int>> repFaces;
	numFace.clear();

	set<int> vToSearch, vToSearchNext;
	vToSearch.insert(facePool.back().begin(), facePool.back().end());
	vector<vector<int>> rest = facePool;
	repFaces.push_back(rest.back());

	rest.pop_back();

	vector<vector<int>> restNext;
	while (1) { // loop to detect all sepearated shells
		int count(0);
		while (1) { // loop for finding all faces for each separated shell
			for (vector<int> f : rest) {
				if (vToSearch.find(f[0]) != vToSearch.end()) {
					vToSearchNext.insert(f[1]);
					vToSearchNext.insert(f[2]);
					count++;
				}
				else if (vToSearch.find(f[1]) != vToSearch.end()) {
					vToSearchNext.insert(f[0]);
					vToSearchNext.insert(f[2]);
					count++;
				}
				else if (vToSearch.find(f[2]) != vToSearch.end()) {
					vToSearchNext.insert(f[0]);
					vToSearchNext.insert(f[1]);
					count++;
				}
				else restNext.push_back(f);
			}
			if (vToSearchNext.size() == 0) break;

			vToSearch = vToSearchNext;
			vToSearchNext.clear();
			rest = restNext;
			restNext.clear();
		}

		if (restNext.size() == 0) break;
		rest = restNext; restNext.clear();

		vToSearch.clear();
		vToSearch.insert(rest.back().begin(), rest.back().end());
		repFaces.push_back(rest.back());
		numFace.push_back(count);
		rest.pop_back();
	}
	return repFaces;
}

vector<vector<vector<int>>> SeparateAndGetShells(vector<vector<int>> facePool) {
	vector<vector<vector<int>>> saparatedShells;
	
	set<int> vToSearch, vToSearchNext;
	vToSearch.insert(facePool.back().begin(), facePool.back().end());
	vector<vector<int>> collected;
	vector<vector<int>> rest = facePool;

	collected.push_back(rest.back());
	rest.pop_back();

	vector<vector<int>> restNext;
	while (1) { // loop to detect all sepearated shells
		while (1) { // loop for finding all faces for each separated shell
			for (vector<int> f : rest) {
				if (vToSearch.find(f[0]) != vToSearch.end()) {
					collected.push_back(f);
					vToSearchNext.insert(f[1]);
					vToSearchNext.insert(f[2]);
				}
				else if (vToSearch.find(f[1]) != vToSearch.end()) {
					collected.push_back(f);
					vToSearchNext.insert(f[0]);
					vToSearchNext.insert(f[2]);
				}
				else if (vToSearch.find(f[2]) != vToSearch.end()) {
					collected.push_back(f);
					vToSearchNext.insert(f[0]);
					vToSearchNext.insert(f[1]);
				}
				else restNext.push_back(f);
			}
			if (vToSearchNext.size() == 0) break;

			vToSearch = vToSearchNext;
			vToSearchNext.clear();
			rest = restNext;
			restNext.clear();
		}
		if (restNext.size() == 0) break;
		rest = restNext; restNext.clear();

		vToSearch.clear();
		vToSearch.insert(rest.back().begin(), rest.back().end());
		saparatedShells.push_back(collected);
		collected.clear();
		collected.push_back(rest.back());
		rest.pop_back();
	}
	return saparatedShells;
}

void PrintNode(string fileName, vector<ThreeVector> pointVec) {
	stringstream ss;
	ss.precision(20);
	ss.setf(ios_base::fixed, ios_base::floatfield);
	ss << pointVec.size() << "  3  0  0" << endl;
	for (size_t i = 0; i < pointVec.size();i++) {
		ss <<setw(4)<< i <<"    "<< pointVec[i] << endl;
	}
	ofstream ofs(fileName);
	ofs << ss.str();
	ofs.close();
}

void PrintSmesh(string fileName, vector<vector<int>> faceVec, vector<pair<ThreeVector, int>> regions) {
	stringstream ss;
	ss << "# node section" << endl
		<< "0  3  0  0 # node list is found in.node file." << endl << endl
		<< "# facet section" << endl
		<< faceVec.size() << " 1" << endl;
	for (vector<int> f : faceVec) {
		ss << "3 " << f[0] << " " << f[1] << " " << f[2] << " " << f[3] << endl;
	}
	ss << endl
		<< "# hole section" << endl
		<< "0" << endl << endl
		<< "# region section" << endl
		<< regions.size() << endl;
	int regionID(1);
	ss.precision(10);
	ss.setf(ios_base::fixed, ios_base::floatfield);
	for (pair<ThreeVector, int> aRegioin : regions) {
		ss << regionID++<<" " << aRegioin.first << " " << aRegioin.second <<" 1"<< endl;
	}
	ofstream ofs(fileName);
	ofs << ss.str();
	ofs.close();
}

string StringSplitterFirst(string str, string del) {
	size_t pos = 0;
	string token = str;
	while ((pos = str.find(del)) != std::string::npos) {
		token = str.substr(0, pos);
		break;
	}
	return token;
}

void ReArrangeFaces(string tetFile) {
	ifstream eIfs(tetFile + ".ele");
	int numEle, a, b, c, d, id;
	string str;
	getline(eIfs, str);
	stringstream ss(str);
	ss >> numEle;
	vector<int> idVec;
	for (int i = 0; i < numEle; i++) {
		getline(eIfs, str);
		ss.clear(); ss.str(str);
		while (ss >> id) {}
		idVec.push_back(id);
	}
	sort(idVec.begin(), idVec.end());
	idVec.erase(unique(idVec.begin(), idVec.end()), idVec.end());
	for (auto i : idVec) {
		cout << i << endl;
	}
	getchar(); getchar();

}


string StringSplitterLast(string str, string del) {
	size_t pos = 0;
	string token = str;
	while ((pos = str.find(del)) != std::string::npos) {
		str.erase(0, pos + del.length());
	}
	return str;
}

void TetgenCall(int p_argc, char ** p_argv)
{
	tetgenbehavior b;

	tetgenio in, addin, bgmin;

	if (!b.parse_commandline(p_argc, p_argv)) {
		terminatetetgen(NULL, 10);
	}

	// Read input files.
	if (b.refine) { // -r
		if (!in.load_tetmesh(b.infilename, (int)b.object)) {
			terminatetetgen(NULL, 10);
		}
	}
	else { // -p
		if (!in.load_plc(b.infilename, (int)b.object)) {
			terminatetetgen(NULL, 10);
		}
	}
	if (b.insertaddpoints) { // -i
							 // Try to read a .a.node file.
		addin.load_node(b.addinfilename);
	}
	if (b.metric) { // -m
					// Try to read a background mesh in files .b.node, .b.ele.
		bgmin.load_tetmesh(b.bgmeshfilename, (int)b.object);
	}

	tetrahedralize(&b, &in, NULL, &addin, &bgmin);
}

map<int, double> AnalyzeTet(string tetFile, bool oldVer) {
	cout << "Reading " + tetFile + ".node ... " << flush;
	ifstream ifsNode(tetFile + ".node");
	string str;
	vector<ThreeVector> nodeVec;
	int numNode, id;
	ThreeVector point;
	ifsNode >> numNode;
	getline(ifsNode, str);
	for (int i = 0; i < numNode; i++) {
		getline(ifsNode, str);
		stringstream ss(str);
		ss >> id >> point;
		nodeVec.push_back(point);
	}ifsNode.close();
	cout << "done" << endl
		<< "Reading " + tetFile + ".ele ... " << flush;
	
	ifstream ifsEle(tetFile + ".ele");
	vector<vector<int>> eleVec;
	map<int, double> volumeMap;
	int numEle, a, b, c, d;
	ifsEle >> numEle;
	getline(ifsEle, str);
	for (int i = 0; i < numEle; i++) {
		getline(ifsEle, str);
		stringstream ss(str);
		ss >> id >> a >> b >> c >> d >> id;
		eleVec.push_back({ a, b, c, d, id });
	}
	cout << "done" << endl;
	
	map<int, double> volMap;
	int degenCount(1), chkCount(1);


	while (degenCount) {
		cout <<endl<< "Check Degeneraged Tetrahedron (" << chkCount++ <<")"<< flush;
		volMap.clear();
		degenCount = 0.;

		for (vector<int> aTet : eleVec) {
			double vol6;
			if(oldVer){
				if (ChkDegenTet(nodeVec[aTet[0]], nodeVec[aTet[1]], nodeVec[aTet[2]], nodeVec[aTet[3]], vol6)) {
					cout << endl <<"   degenerated tet " << ++degenCount << " ("+to_string(aTet[4])+") "<< setw(6) <<"--> " <<flush;
					double move;
					move = FixDegenTet2(nodeVec[aTet[0]], nodeVec[aTet[1]], nodeVec[aTet[2]], nodeVec[aTet[3]], vol6);
					cout << "fixed (moved "<<move<<" cm)" ;
				}
			}else{
				if (ChkDegenTet2(nodeVec[aTet[0]], nodeVec[aTet[1]], nodeVec[aTet[2]], nodeVec[aTet[3]], vol6)) {
					cout << endl <<"   degenerated tet " << ++degenCount << " ("+to_string(aTet[4])+") "<< setw(6) <<"--> " <<flush;
					double move;
					move = FixDegenTet3(nodeVec[aTet[0]], nodeVec[aTet[1]], nodeVec[aTet[2]], nodeVec[aTet[3]], vol6);
					cout << "fixed (moved "<<move<<" cm)" ;
				}
			}
			volMap[aTet[4]] += vol6;
		}

		if (degenCount == 0)
			cout << endl << "   => not detected" << endl;
		else {
			cout << endl << "   => " << degenCount << " tetrahedrons are degenerated" << endl;
			PrintNode(tetFile + ".node", nodeVec);
		}
	}
	cout << endl;
	
	for (pair<const int, double>& volPair : volMap) 
		volPair.second /= 6.;
	
	return volMap;
}

bool ChkDegenTet(ThreeVector anchor, ThreeVector p2, ThreeVector p3, ThreeVector p4, double& vol6) {
	// fV<x><y> is vector from vertex <y> to vertex <x>
	//
	ThreeVector fV21 = p2 - anchor;
	ThreeVector fV31 = p3 - anchor;
	ThreeVector fV41 = p4 - anchor;

	// make sure this is a correctly oriented set of points for the tetrahedron
	//
	double signed_vol = fV21.cross(fV31).dot(fV41);
	vol6 = std::fabs(signed_vol);

	double fXMin = std::min(std::min(std::min(anchor.x(), p2.x()), p3.x()), p4.x());
	double fXMax = std::max(std::max(std::max(anchor.x(), p2.x()), p3.x()), p4.x());
	double fYMin = std::min(std::min(std::min(anchor.y(), p2.y()), p3.y()), p4.y());
	double fYMax = std::max(std::max(std::max(anchor.y(), p2.y()), p3.y()), p4.y());
	double fZMin = std::min(std::min(std::min(anchor.z(), p2.z()), p3.z()), p4.z());
	double fZMax = std::max(std::max(std::max(anchor.z(), p2.z()), p3.z()), p4.z());

	ThreeVector fMiddle = ThreeVector(fXMax + fXMin, fYMax + fYMin, fZMax + fZMin)*0.5;
	double fMaxSize = std::max(std::max(std::max((anchor - fMiddle).mag(),
		(p2 - fMiddle).mag()),
		(p3 - fMiddle).mag()),
		(p4 - fMiddle).mag());

	return (vol6 < 1e-9*fMaxSize*fMaxSize*fMaxSize);
}

bool ChkDegenTet2(ThreeVector p0, ThreeVector p1, ThreeVector p2, ThreeVector p3, double& vol6) {
	double hmin = 4.e-9;

	//Calculate volume
	vol6 = std::abs((p1 - p0).cross(p2 - p0).dot(p3 - p0));

	//calculate face areas squared
	double ss[4];
	ss[0] = ((p1 - p0).cross(p2 - p0)).mag2();
	ss[1] = ((p2 - p0).cross(p3 - p0)).mag2();
	ss[2] = ((p3 - p0).cross(p1 - p0)).mag2();
	ss[3] = ((p2 - p1).cross(p3 - p1)).mag2();

	//find face with max area
	int k=0;
	for(int i=0;i<4;++i){if(ss[i]>ss[k]) k=i;}

	//check: vol^2 / s^2 <= hmin^2
	return (vol6*vol6 <= ss[k]*hmin*hmin);
}

double FixDegenTet(ThreeVector& anchor, ThreeVector& p2, ThreeVector& p3, ThreeVector& p4, double& vol6){
	double fXMin = std::min(std::min(std::min(anchor.x(), p2.x()), p3.x()), p4.x());
	double fXMax = std::max(std::max(std::max(anchor.x(), p2.x()), p3.x()), p4.x());
	double fYMin = std::min(std::min(std::min(anchor.y(), p2.y()), p3.y()), p4.y());
	double fYMax = std::max(std::max(std::max(anchor.y(), p2.y()), p3.y()), p4.y());
	double fZMin = std::min(std::min(std::min(anchor.z(), p2.z()), p3.z()), p4.z());
	double fZMax = std::max(std::max(std::max(anchor.z(), p2.z()), p3.z()), p4.z());

	ThreeVector middle = ThreeVector(fXMax + fXMin, fYMax + fYMin, fZMax + fZMin)*0.5;
	
	int closestP(1);
	vector<ThreeVector> pointVec = { anchor, p2, p3, p4 };

	bool degen = true;
	double totalMove(0.);
	double move = pow(vol6, 1. / 3.)*0.0001;
	if (move > 0.0001) move = 0.00001;

	vector<ThreeVector> upVector;
	for (int i = 0; i < 4; i++) {
		ThreeVector up = (pointVec[(i + 2)%4] - pointVec[(i + 1) % 4]).cross(pointVec[(i + 3) % 4] - pointVec[(i + 1) % 4]);
		up = up.unit();
		if (up.dot(pointVec[i] - pointVec[(i + 1) % 4])<0) {
			up = -up;
		}
		upVector.push_back(up);
	}

	while (degen) {
		double minDist2(DBL_MAX);
		int minP(-1);
		for (int i = 0; i < 4; i++) {
			double dist2 = (middle - pointVec[i]).mag2();
			if (dist2 > minDist2) continue;
			minDist2 = dist2;
			minP = i;
		}

		pointVec[minP] += upVector[minP] * move;
		degen = ChkDegenTet(pointVec[0], pointVec[1], pointVec[2], pointVec[3], vol6);
		totalMove += move;
	}
//	cout << "moved " << totalMove << " cm" << endl;
	
	anchor = pointVec[0];
	p2 = pointVec[1];
	p3 = pointVec[2];
	p4 = pointVec[3];
	return totalMove;
}

double FixDegenTet2(ThreeVector& anchor, ThreeVector& p2, ThreeVector& p3, ThreeVector& p4, double &vol6) {
	vector<ThreeVector> pointVec = { anchor, p2, p3, p4 };
	vector<ThreeVector> upVector;
	for (int i = 0; i < 4; i++) {
		ThreeVector up = (pointVec[(i + 2) % 4] - pointVec[(i + 1) % 4]).cross(pointVec[(i + 3) % 4] - pointVec[(i + 1) % 4]);
		up = up.unit();
		if (up.dot(pointVec[i] - pointVec[(i + 1) % 4]) < 0) {
			up = -up;
		}
		upVector.push_back(up);
	}
	double minHeight(DBL_MAX);
	vector<ThreeVector> midVector;
	vector<ThreeVector> cenVector;
	int minP(-1);
	for (int i = 0; i < 4; i++) {
		ThreeVector u = pointVec[(i + 1) % 4] - pointVec[i];
		double height = -upVector[i].dot(u);
		ThreeVector proj = pointVec[i] - upVector[i] * height;
		ThreeVector mid = (pointVec[(i + 1) % 4] + pointVec[(i + 2) % 4] + pointVec[(i + 3) % 4]) / 3.;
		midVector.push_back(mid - proj);
		cenVector.push_back(mid + upVector[i] * height);
		if (height > minHeight) continue;
		minHeight = height;
		minP = i;
	}
	
	bool degen = true;
	double totalMove(0.);
	double move = pow(vol6, 1. / 3.)*0.0001;
	if (move > 0.0001) move = 0.00001;
	while (degen) {
		pointVec[minP] += upVector[minP] * move;
		if ((cenVector[minP] - pointVec[minP]).dot(midVector[minP]) > 0) {
			pointVec[minP] += midVector[minP] * move;
			totalMove += move * 1.41421356237;
		}
		else totalMove += move;
		degen = ChkDegenTet(pointVec[0], pointVec[1], pointVec[2], pointVec[3], vol6);
	}
	anchor = pointVec[0];
	p2 = pointVec[1];
	p3 = pointVec[2];
	p4 = pointVec[3];
	return totalMove;
}

double FixDegenTet3(ThreeVector& anchor, ThreeVector& p2, ThreeVector& p3, ThreeVector& p4, double &vol6) {
	vector<ThreeVector> pointVec = { anchor, p2, p3, p4 };
	vector<ThreeVector> upVector;
	for (int i = 0; i < 4; i++) {
		ThreeVector up = (pointVec[(i + 2) % 4] - pointVec[(i + 1) % 4]).cross(pointVec[(i + 3) % 4] - pointVec[(i + 1) % 4]);
		up = up.unit();
		if (up.dot(pointVec[i] - pointVec[(i + 1) % 4]) < 0) {
			up = -up;
		}
		upVector.push_back(up);
	}
	double minHeight(DBL_MAX);
	vector<ThreeVector> midVector;
	vector<ThreeVector> cenVector;
	int minP(-1);
	for (int i = 0; i < 4; i++) {
		ThreeVector u = pointVec[(i + 1) % 4] - pointVec[i];
		double height = -upVector[i].dot(u);
		ThreeVector proj = pointVec[i] - upVector[i] * height;
		ThreeVector mid = (pointVec[(i + 1) % 4] + pointVec[(i + 2) % 4] + pointVec[(i + 3) % 4]) / 3.;
		midVector.push_back(mid - proj);
		cenVector.push_back(mid + upVector[i] * height);
		if (height > minHeight) continue;
		minHeight = height;
		minP = i;
	}

	bool degen = true;
	double totalMove(0.);
	double move = pow(vol6, 1. / 3.)*0.0001;
	if (move > 0.0001) move = 0.00001;
	while (degen) {
		pointVec[minP] += upVector[minP] * move;
		if ((cenVector[minP] - pointVec[minP]).dot(midVector[minP]) > 0) {
			pointVec[minP] += midVector[minP] * move;
			totalMove += move * 1.41421356237;
		}
		else totalMove += move;
		degen = ChkDegenTet2(pointVec[0], pointVec[1], pointVec[2], pointVec[3], vol6);
	}
	anchor = pointVec[0];
	p2 = pointVec[1];
	p3 = pointVec[2];
	p4 = pointVec[3];
	return totalMove;
}

int PrintVolume(string tetFile, ostream& os) {
	cout << "Reading " + tetFile + ".node ... " << flush;
	ifstream ifsNode(tetFile + ".node");
	string str;
	vector<ThreeVector> nodeVec;
	int numNode, id;
	ThreeVector point;
	ifsNode >> numNode;
	getline(ifsNode, str);
	for (int i = 0; i < numNode;i++) {
		getline(ifsNode, str);
		stringstream ss(str);
		ss >> id >> point;
		nodeVec.push_back(point);
	}ifsNode.close();
	cout << "done" << endl
		<< "Reading " + tetFile + ".ele ... " << flush;
	ifstream ifsEle(tetFile + ".ele");
	map<int, double> volumeMap;
	int numEle, a, b, c, d;
	ifsEle >> numEle;
	getline(ifsEle, str);
	
	for (int i = 0; i < numEle; i++) {
		getline(ifsEle, str);
		stringstream ss(str);
		ss >> id >> a >> b >> c >> d >> id;
		ThreeVector v21 = nodeVec[b] - nodeVec[a];
		ThreeVector v31 = nodeVec[c] - nodeVec[a];
		ThreeVector v41 = nodeVec[d] - nodeVec[a];
		double volume6 = v21.cross(v31).dot(v41);
		volumeMap[id] += volume6;
	}
	cout << "done" << endl;
	os << fixed;
	os.precision(7);
	int maxVolID(-1);
	double maxVol(-1.);
	for (auto vol : volumeMap) {
		if (maxVol < vol.second) {
			maxVol = vol.second;
			maxVolID = vol.first;
		}
		os << setw(7) << vol.first << setw(15) << vol.second / 6. << endl;
	}
	return maxVolID;
}

bool ConvertZeroMat(int rst, string eleFile, int largestR, bool forcedChange) {
	cout << "Modifying " + eleFile + "..." << flush;
	ifstream ifs(eleFile);
	int numEle, tempInt, a, c, b, d, id, count(0);
	map<int, vector<int>> notDefinedR;
	string str;
	stringstream ssOut;
	ifs >> numEle;
	getline(ifs, str);
	ssOut << numEle << "  4  1" << endl;
	for (int i = 0; i < numEle; i++) {
		getline(ifs, str);
		stringstream ss(str);
		ss >> tempInt >> a >> b >> c >> d >> id;
		if (id == 0) {
			ssOut << setw(5) << tempInt 
				  << "   " << setw(5) << a 
				  << " " << setw(5) << b 
				  << " " << setw(5) << c 
				  << " " << setw(5) << d 
				  << "    " << rst << endl;
			count++;
		}
		else if (id > largestR) {
			notDefinedR[id] = { a, b, c, d };
			if(forcedChange)
				ssOut << setw(5) << tempInt
					  << "   " << setw(5) << a
					  << " " << setw(5) << b
					  << " " << setw(5) << c
					  << " " << setw(5) << d
					  << "    " << rst << endl;
			else
				ssOut << str << endl;
		}
		else 
			ssOut << str << endl;
	}
	ifs.close();

	ofstream ofs(eleFile);
	ofs << ssOut.str();
	ofs.close();
	cout << "done" << endl;
	cout << count << " elements have attribute 0, and were changed into " << rst << endl;
	cout << "There are " << notDefinedR.size() << " not defined regions" << endl;
	if (notDefinedR.size() > 0) {
		ifstream ifsNode(eleFile.substr(0, eleFile.size() - 3) + "node");
		string str;
		vector<ThreeVector> nodeVec;
		int numNode, id;
		ThreeVector point;
		ifsNode >> numNode;
		getline(ifsNode, str);
		for (int i = 0; i < numNode; i++) {
			getline(ifsNode, str);
			stringstream ss(str);
			ss >> id >> point;
			nodeVec.push_back(point);
		}ifsNode.close();
		cout << "-> Below is the list of the points in not defined regions" << endl;
		for (pair<int, vector<int>> aPair : notDefinedR) {
			ThreeVector center = (nodeVec[aPair.second[0]] +
								  nodeVec[aPair.second[1]] +
								  nodeVec[aPair.second[2]] +
								  nodeVec[aPair.second[3]]) * 0.25;
			cout << "   " << center << endl;
		}
		if(!forcedChange){
			cout<<"-> MC codes will not be generated because it may not work due to the undefined regions"<<endl;
			return false;
		}
	}
	return true;
}

#endif
