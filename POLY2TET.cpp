#include "POLY2TET.h"
#include "FileGen.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

void PrintUsage(){
	cout<<"./POLY2TET [options] PHANTOM.obj"<<endl;
	cout<<"  options:"<<endl;
	cout<<"-g : generate Geant4 code (incompatible with -o option)"<<endl;
	cout<<"-x : use previously generate TET phantom files"<<endl;
	cout<<"-c : positive integer number to replace zero-region number (and not defined regions, when used as -rc)"<<endl;
}

int main(int argc, char *argv[])
{
	int maxVolID(-1);
	bool gFlag(false), mFlag(false), pFlag(false), xFlag(false), rFlag(false);

	for (int i = 1; i < argc; i++) {
		if (string(argv[i]) == "-g")
			gFlag = true;
		else if (string(argv[i]) == "-x")
			xFlag = true;
		else if (string(argv[i]) == "-c") {
			maxVolID = atoi(argv[i + 1]);
			i++;
		}
		else if (string(argv[i]) == "-rc") {
			maxVolID = atoi(argv[i + 1]);
			i++;
			rFlag = true;
		}
	}

	string fileName = argv[argc-1];

	if (StringSplitterLast(fileName, ".") != "obj") {
		cerr << "File format should be obj" << endl;
		getchar();
		return 1;
	}

	if(!xFlag){
		//folder generation
		if(mkdir("phantoms", 0776) == -1 && errno != EEXIST)
		{ cout<< "phantom directory create error: " + string(strerror(errno)) << endl; return -1; }

		/****For the OBJ files with the group names starting with "O_"****/
		//	FixObj(fileName);
		//	fileName = fileName.substr(0, fileName.size() - 4) + "_2.obj";
		int largestR = ConvertOBJ(fileName);
		if (largestR < 0) return -1;
		string smeshName = fileName.substr(0, fileName.size() - 4) + ".smesh";

		char** p_argv = new char*[3];
		for(int i=0;i<3;i++) p_argv[i]=new char[50];

		strcpy(p_argv[0], string("tetgen").c_str());
		strcpy(p_argv[1], string("-p/0.0001YAFT0.000000001").c_str());
		strcpy(p_argv[2], smeshName.c_str());

		bool repeat(true);

		while(repeat){
			repeat = false;
			try {
				TetgenCall(3, p_argv);
			}
			catch (int input) {
				repeat=true;
			}
		}

		string tetFileName = fileName.substr(0, fileName.size() - 3) + "1";
		//int maxVolID = PrintVolume(tetFileName, cout);

		map<int, double> volMap = AnalyzeTet(tetFileName);
		double maxVol(-1.);
		int tempID;
		ofstream ofsVol(fileName.substr(0, fileName.size() - 4) + "_vol.txt");
		for (auto vol : volMap) {
			if (maxVol < vol.second) {
				maxVol = vol.second;
				tempID = vol.first;
			}
			ofsVol << vol.first << "\t" << vol.second << endl;
		}

		if (maxVolID < 0) maxVolID = tempID;
		bool allDone = ConvertZeroMat(maxVolID, tetFileName + ".ele", largestR, rFlag);
		if(!allDone) {gFlag = false; mFlag = false; pFlag = false;}

		//file renaming
		int eleRename = rename((tetFileName + ".ele").c_str(),
				("./phantoms/"+tetFileName.substr(0, tetFileName.size()-2)+".ele").c_str());
		int nodeRename= rename((tetFileName + ".node").c_str(),
				("./phantoms/"+tetFileName.substr(0, tetFileName.size()-2)+".node").c_str());
		if(eleRename||nodeRename)	cerr<<"WARNING-rename failed."<<endl;

		cout<<endl<<"--Conversion was done: "<<fileName.substr(0, fileName.size()-3)+"1.node/ele were exported"<<endl;
	}
	
	//file generations
	if(!(gFlag+mFlag+pFlag)) return 0;
	
	FileGen fileGen(fileName.substr(0, fileName.size()-4), gFlag, mFlag, pFlag);

	return 0;
}
