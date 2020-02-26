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

FileGen::FileGen(string _phantomName, bool _g4_gen, bool _mcnp6_gen, bool _phits_gen)
:phantomName(_phantomName), g4_gen(_g4_gen), mcnp6_gen(_mcnp6_gen), phits_gen(_phits_gen),
 numNode(0), numEle(0)
{
	ReadTetNode();

	if(mcnp6_gen||phits_gen)
		ReadTetEle();

	if(g4_gen)   {GenerateG4(); cout<<"  -->Geant4 generated"<<endl;}
	if(mcnp6_gen){GenerateM6(); cout<<"  -->MCNP6 generated"<<endl;}
	if(phits_gen){GeneratePH(); cout<<"  -->PHITS generated"<<endl;}
}

FileGen::~FileGen() {}

void FileGen::ReadTetNode(){
	//read node file
	ifstream ifs(phantomName+".node");
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
	beamHZ = floor((box_max.getZ()-box_min.getZ())*0.5)+1;
}

void FileGen::ReadTetEle(){

}

void FileGen::GenerateG4(){
    system(("./.Geant4_gen.sh "+phantomName + " " + to_string(beamHX) + " " + to_string(beamHZ)).c_str());
}

void FileGen::GenerateM6(){

}
