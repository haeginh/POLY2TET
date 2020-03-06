/*
 * FileGen.cpp
 *
 *  Created on: Feb 24, 2020
 *      Author: haeginh
 */

#include "FileGen.h"
#include "tet2inp.hpp"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <unistd.h>
#include <iomanip>

FileGen::FileGen(string _phantomName, string _baseDir)
:phantomName(_phantomName), baseDir(_baseDir), numNode(0)
{
	cout<<"Read node file..."<<flush;
	ReadTetNode(); cout<<"done"<<endl;
}

FileGen::~FileGen() {}

void FileGen::ReadMatFile(string matFile, MAT matFormat){
	ifstream ifs(matFile);
	// exception for the case when there is no *.material file
	if(!ifs.is_open()) {
		int ans(0);
		while(ans==0){
			cout<<"There is no material (" + matFile<<"). Continue? (y/n): ";
			string temp; cin>>temp;
			if(temp=="y") ans=1; if(temp=="n") ans=2;
		}
		if(ans==1) return;
		if(ans==2) exit(10);
	}

	cout << "Reading material file (" << matFile << ")..." <<flush;
	if(matFormat==MCNP6){
		char read_data[50];
		char* token;
		int zaid;
		double fraction;
		string MaterialName;
		double density;

		while(!ifs.eof())
		{
			ifs >> read_data;                   //ex) 'C' RBM
			ifs >> MaterialName;                //ex)  C 'RBM'
			ifs >> read_data;
			density = std::atof(read_data);     //ex) 1.30
			ifs >> read_data;                   //ex) g/cm3
			ifs >> read_data;
			token = std::strtok(read_data,"m");
			int matID = std::atoi(token);        //ex) m'10'
			matNameMap[matID]= MaterialName;
			densityMap[matID]= density;

			for(int i=0 ;  ; i++)
			{
				ifs >> read_data;
				if(std::strcmp(read_data, "C")==0 || ifs.eof()) break;

				zaid = std::atoi(read_data);
				ifs >> read_data;
				fraction = -1.0 * std::atof(read_data);
				matMap[matID].push_back(make_pair(int(zaid), fraction));
			}
		}
		ifs.close();
		cout<<"done"<<endl;
//		if(matFile=="./phantoms/"+phantomName+".material"||matFile=="phantoms/"+phantomName+".material") continue;

		cout<<"Generate material file for MCNP6 in "+baseDir+phantomName+".material"<<endl;
		system(("cp "+ matFile + " " + baseDir+phantomName+".material").c_str());
	}
	else if(matFormat==PHITS){
		string read_data;
		int zaid;
		double fraction;
		string MaterialName;
		double density;

		while(!ifs.eof())
		{
			ifs >> read_data;                       //ex) '$' RBM
			ifs >> MaterialName;                    //ex)  $ 'RBM'
			ifs >> read_data;
			density = std::atof(read_data.c_str()); //ex) 1.30
			ifs >> read_data;                       //ex) g/cm3
			ifs >> read_data;
			int matID = std::atoi(read_data.substr(4,read_data.size()-5).c_str()); //ex) MAT['10']
			matNameMap[matID]= MaterialName;
			densityMap[matID]= density;

			for(int i=0 ;  ; i++)
			{
				ifs >> read_data;
				if(read_data.substr(0,2)=="mt" ||read_data.empty() || ifs.eof()) break;

				zaid = std::atoi(read_data.c_str());
				ifs >> read_data;
				fraction = -1.0 * std::atof(read_data.c_str());
				matMap[matID].push_back(make_pair(int(zaid), fraction));
			}
		}
		ifs.close();

		cout<<"Generate material file for MCNP6 in "+baseDir+phantomName+".material"<<endl;
		if(access((baseDir+phantomName+".material").c_str(), F_OK)==0){
			int ans(0);
			while(ans==0){
				cout<<baseDir+phantomName+".material already exists. Overwrite? (y/n)";
				string temp; cin>>temp;
				if(temp=="y") ans=1; if(temp=="n") ans=2;
			}
			if(ans==1){
				rmdir((baseDir+phantomName+".material").c_str());
				GenerateMCNP6Mat(baseDir+phantomName+".material");
			}
			if(ans==2){
				cout<<"Rename existing file into (press just Enter to skip MCNP6 material file generation): "<<flush;
				string newName; cin>>newName;
				if(!newName.empty()){
					rename((baseDir+phantomName+".material").c_str(), (baseDir+newName+".material").c_str());
					GenerateMCNP6Mat(baseDir+phantomName+".material");
				}
			}
		}
	}
	cout<<endl;
	return;
}

void FileGen::GenerateMCNP6Mat(string fileName){
	ofstream ofs(fileName);
	int w=floor(log10(matNameMap.rbegin()->first))+1;
	ofs<<left;
	for(auto matN:matNameMap){
		ofs<<"C "<<matN.second<<" "<<densityMap[matN.first]<<" g/cm3"<<endl;
		bool firstLine(true);
		for(auto elements:matMap[matN.first]){
			if(firstLine)
				ofs<<setw(w+8)<<"m"+to_string(matN.first)<<setw(10)<<elements.first<<-elements.second<<endl;
			else{
				ofs<<setw(w+8)<<" "<<setw(10)<<elements.first<<-elements.second<<endl;
				firstLine=false;
			}
		}
		ofs<<"C"<<endl;
	}
	ofs.close();
}

void FileGen::GeneratePhitsMat(string fileName){
	ofstream ofs(fileName);
	int w=floor(log10(matNameMap.rbegin()->first))+1;
	ofs<<left;
	for(auto matN:matNameMap){
		ofs<<"$ "<<matN.second<<" "<<densityMap[matN.first]<<" g/cm3"<<endl;
		ofs<<"MAT["<<matN.first<<"]"<<endl;
		for(auto elements:matMap[matN.first])
				ofs<<setw(5)<<" "<<setw(10)<<elements.first<<-elements.second<<endl;
		ofs<<"mt"<<matN.first<<endl;
	}
	ofs.close();
}

void FileGen::ReadTetNode(){
	//read node file
	ifstream ifs(baseDir+phantomName+".node");
	ifs>>numNode;
	string buffer;
	getline(ifs, buffer);

	for(int i=0;i<numNode;i++){
		ThreeVector point;
		ifs>>buffer>>point;
		nodeVec.push_back(point);
	}ifs.close();

	//verify boundary box
	box_max=nodeVec[0];
	box_min=nodeVec[0];
	for(auto node:nodeVec){
		if(box_max.getX()<node.getX()) box_max.setX(node.getX());
		else if(box_min.getX()>node.getX()) box_min.setX(node.getX());

		if(box_max.getY()<node.getY()) box_max.setY(node.getY());
		else if(box_min.getY()>node.getY()) box_min.setY(node.getY());

		if(box_max.getZ()<node.getZ()) box_max.setZ(node.getZ());
		else if(box_min.getZ()>node.getZ()) box_min.setZ(node.getZ());
	}
	beamHX = floor((box_max.getX()-box_min.getX())*0.5)+1;
	beamHZ = floor((box_max.getZ()-box_min.getZ()*0.5))+1;
	ThreeVector center = (box_max + box_min)*0.5;
	for(auto &node:nodeVec)	node -= center;
}

void FileGen::ReadTetEle(){
	ifstream ifs(baseDir+phantomName+".ele");
	int numEle, a, b, c, d, id;
	ifs>>numEle;
	string buffer;
	getline(ifs, buffer);

	for(int i=0;i<numEle;i++){
		getline(ifs, buffer);
		stringstream ss(buffer);
		ss>>buffer>>a>>b>>c>>d>>id;

		ThreeVector fV21 = nodeVec[b] - nodeVec[a];
		ThreeVector fV31 = nodeVec[c] - nodeVec[a];
		ThreeVector fV41 = nodeVec[d] - nodeVec[a];
		double signed_vol = fV21.cross(fV31).dot(fV41);
		volMap[id] = volMap[id] + std::fabs(signed_vol);
	}ifs.close();
}

void FileGen::GenerateG4(){
	//directory selection
	string dir_G4 = "Geant4";
	while(access((baseDir+dir_G4).c_str(),F_OK)==0){
		int ans(0);
		while(ans==0){
			cout<<baseDir+dir_G4 + " directory already exists. Overwrite? (y/n): ";
			string temp; cin>>temp;
			if(temp=="y") ans = 1; if(temp=="n") ans=2;
		}
		if(ans==1) system(("rm -rf " + baseDir + dir_G4).c_str());
		if(ans==2){
			cout<<"New name for Geant4 files (0 to skip Geant4 file generation): ";
			string temp; cin>>temp;
			if(temp=="0") return;
			dir_G4=temp;
		}
	}

	//generate Geant4 files
        system(("./.Geant4_gen.sh "+phantomName + " " + to_string(beamHX) + " " + to_string(beamHZ)
               +" "+ baseDir + dir_G4).c_str());
}

void FileGen::GenerateM6(){
	//directory selection
	string dir_M6 = "MCNP6";
	while(access((baseDir+dir_M6).c_str(),F_OK)==0){
		int ans(0);
		while(ans==0){
			cout<<baseDir+dir_M6 + " directory already exists. Overwrite? (y/n): ";
			string temp; cin>>temp;
			if(temp=="y") ans = 1; if(temp=="n") ans=2;
		}
		if(ans==1) system(("rm -rf " + baseDir + dir_M6).c_str());
		if(ans==2){
			cout<<"New name for MCNP6 files (0 to skip MCNP6 file generation): ";
			string temp; cin>>temp;
			if(temp=="0") return;
			dir_M6=temp;
		}
	}
	if(mkdir((baseDir+dir_M6).c_str(), 0776) == -1)
	{ cout<< "phantom directory create error: " + string(strerror(errno)) << endl; return; }

	//phantom data generation
	string loweredPhan = phantomName;
	transform(loweredPhan.begin(), loweredPhan.end(), loweredPhan.begin(), ::tolower);
	bool abaqusFlag(true);
	cout<<"Start generating ABAQUS file..."<<endl;
	string inpFileName = baseDir + dir_M6 + "/" + loweredPhan + ".inp";
	tet2inp tet(baseDir +phantomName+".ele", inpFileName, nodeVec);
	tet.PrintCell(baseDir + dir_M6 + "/" + loweredPhan+".cell", densityMap);
	tet.PrintTally(baseDir + dir_M6 + "/" + loweredPhan+".tally", matNameMap);

	//	read cell file
	ifstream ifs(baseDir + dir_M6 + "/" + loweredPhan+".cell");
	string dump;
	getline(ifs, dump);
	int psuedo, mat;
	vector<int> pCellVec;
	while(getline(ifs, dump)){
		stringstream ss(dump);
		ss>>psuedo>>mat;
		if(mat!=0) pCellVec.push_back(psuedo);
	}
	ifs.close();

//folder generation & print mcnp6 input file
	ThreeVector halfXYZ = (box_max - box_min)*0.5 + ThreeVector(1, 1, 1);
	stringstream ss; ss.precision(4); ss<<fixed;
	ss<<halfXYZ<<" "<<beamHX<<" "<<beamHZ;
	int largest = *pCellVec.rbegin();
	auto fp = popen(("./.MCNP6_gen.sh "+phantomName + " "
			         + ss.str() + " "
					 + to_string(pCellVec.size()) + " " + to_string(largest)
			         + " " + baseDir + dir_M6).c_str(), "r");
	pclose(fp);
	ofstream ofs(baseDir+dir_M6+"/"+phantomName+"_MCNP6.i", ios::out | ios::app);
	int w = floor(log10((double)largest))+2;
	ofs<<setw(w)<<1<<setw(w)<<pCellVec[0]<<endl;
	for(size_t i=1;i<pCellVec.size()-1;i++)
		ofs<<setw(w+15)<<i+1<<setw(w)<<pCellVec[i]<<endl;
	ofs.close();
	cout<<"MCNP6 input file was generated as "+baseDir+dir_M6+"/"+phantomName+"_MCNP6.i"<<endl;
}

void FileGen::GeneratePH(){
	//directory selection
	string dir_PH = "PHITS";
	while(access((baseDir+dir_PH).c_str(),F_OK)==0){
		int ans(0);
		while(ans==0){
			cout<<baseDir+dir_PH + " directory already exists. Overwrite? (y/n): ";
			string temp; cin>>temp;
			if(temp=="y") ans = 1; if(temp=="n") ans=2;
		}
		if(ans==1) system(("rm -rf "+baseDir + dir_PH).c_str());
		if(ans==2){
			cout<<"New name for PHITS files (0 to skip PHITS file generation): ";
			string temp; cin>>temp;
			if(temp=="0") return;
			dir_PH=temp;
		}
	}
	if(mkdir((baseDir+dir_PH).c_str(), 0776) == -1)
	{ cout<< "phantom directory create error: " + string(strerror(errno)) << endl; return; }


	cout<<"Read ele file..."<<flush;
	ReadTetEle();cout<<"done"<<endl;

	//material file generation
	GeneratePhitsMat(baseDir+dir_PH+"/"+phantomName+".material");

	//	generate cell file
	int w = floor(log10(matNameMap.rbegin()->first))+1;
	if(w<5) w=5;
	ofstream ofsCell(baseDir + dir_PH + "/" +phantomName+".cell");
	ofsCell<<"$ CELLS FOR {PHANTOM}"<<endl;
	ofsCell<<" "<<setfill('0')<<setw(w)<<"99999"<<setw(w+2)<<"0"<<setw(9)<<"-20"<<setw(5)<<" "<<"  "<<"U=15000 LAT=3 tfile=../{PHANTOM}"<<endl;
	ofsCell<<" "<<setfill('0')<<setw(w)<<"99998"<<setw(w+2)<<"0"<<setw(9)<<"-10"<<setw(5)<<" "<<"  "<<"FILL=15000"<<endl;
	ofsCell<<" "<<setfill('0')<<setw(w)<<"99997"<<setw(w+2)<<"0"<<setw(9)<<"-20"<<setw(5)<<"10"<<endl;
	ofsCell<<" "<<setfill('0')<<setw(w)<<"99996"<<setw(w+2)<<"0"<<setw(9)<<"-90"<<setw(5)<<"20"<<endl;
	ofsCell<<" "<<setfill('0')<<setw(w)<<"99995"<<setw(w+2)<<"-1"<<setw(9)<<"90"<<endl;
	for(auto volM:volMap){
		ofsCell<<" "<<setfill('0')<<setw(w)<<volM.first<<setw(w+2)<<volM.first<<setw(9)<<densityMap[volM.first]
		       <<setw(5)<<"-90"<<"  "<<left<<setw(w+4)<<"u="+to_string(volM.first)<<"VOL="<<volM.second<<endl;
	}
	ofsCell.close();

//folder generation & print mcnp6 input file
	ThreeVector halfXYZ = (box_max - box_min)*0.5 + ThreeVector(1, 1, 1);
	stringstream ss; ss.precision(4); ss<<fixed;
	ss<<halfXYZ<<" "<<beamHX<<" "<<beamHZ;
	auto fp = popen(("./.PHITS_gen.sh "+phantomName + " "
			         + ss.str() + " " + baseDir+dir_PH).c_str(), "r");
	char buff[50];
	fgets(buff, 50, fp); pclose(fp);
	string newFile(buff);
	cout<<"PHITS input file was generated as "+newFile<<endl;
}
