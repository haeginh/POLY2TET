#include "./src/POLY2TET.h"
#include "./src/FileGen.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

void PrintUsage(){
	cout<<"./POLY2TET [options] MRCP_AM_2.obj"<<endl;
	cout<<"  options:"<<endl;
	cout<<"-g : generate Geant4 code"<<endl;
	cout<<"-m : generate MCNP6 input file"<<endl;
	cout<<"-p : generate PHITS input file"<<endl;
	cout<<"-c : positive integer number to replace zero-region number (and not defined regions, when used as -rc)"<<endl;
	cout<<"-s : without Y option. allow Steiner point on the polygon"<<endl;
	cout<<"-mat/m : material file in MCNP6 format"<<endl;
	cout<<"-mat/p : material file in PHITS format"<<endl;
}

int main(int argc, char *argv[])
{
	int maxVolID(-1);
	bool gFlag(false), mFlag(false), pFlag(false), rFlag(false), oFlag(false), oldFlag(false), sFlag;
	MAT matFormat; string matFile;

	// Argument parser
	for (int i = 1; i < argc-1; i++) {
		string argStr = string(argv[i]);
		if(argStr.substr(0,1)!="-") {PrintUsage();return -1;}
		if (argStr == "-c") {
			maxVolID = atoi(argv[++i]);
			continue;
		}
		else if (argStr == "-s") {
			sFlag = true;
			continue;
		}
		else if (argStr == "-rc") {
			maxVolID = atoi(argv[++i]);
			rFlag = true;
			continue;
		}
		else if (argStr == "-mat/m") {
			matFormat = MCNP6;
			matFile = string(argv[++i]);
			continue;
		}
		else if (argStr == "-mat/p") {
			matFormat = PHITS;
			matFile = string(argv[++i]);
			continue;
		}
		else if (argStr == "-o") {
			oFlag = true;
			continue;
		}
		else if (argStr == "-old") {
			oldFlag = true;
			continue;
		}


		const char* options = argStr.substr(1,argStr.size()-1).c_str();
		for(int j=0;j<argStr.size()-1;j++){
			if (options[j] == 'g')
				gFlag = true;
			else if (options[j] == 'm')
				mFlag = true;
			else if (options[j] == 'p')
				pFlag = true;
			else
				{PrintUsage();return -1;}
		}
	}

	string fileName = argv[argc-1];

	if (StringSplitterLast(fileName, ".") != "obj") {
		cerr << "File format should be obj" << endl;
		getchar();
		return 1;
	}

	// Check folder
	string baseDir = fileName.substr(0, fileName.size() - 4);
	bool genDir(true);
	while(access(baseDir.c_str(), F_OK)==0){
		int ans(0);
		while(ans==0){
			cout<< "<" + baseDir + "> directory already exists. Overwrite?"<<endl;
			cout<< " [1] Yes (erase all previous files)." <<endl;
			cout<< " [2] No (leave previous files)." <<endl;
			cout<< " [3] No (create a new folder with a different name)." <<endl;
			cout<< "--> input 1/2/3: "<<flush;
			string temp; cin>>temp;
			if(temp=="1") ans = 1; if(temp=="2") ans=2; if(temp=="3") ans = 3;

			if(ans==1) system(("rm -rf "+baseDir).c_str());
			else if(ans==2) genDir =false;
			else if(ans==3){
				cout<<"New directory name: ";
				cin>>baseDir;
			}
		}
		if(!genDir) break;
	}
	if(mkdir(baseDir.c_str(), 0776) == -1 && genDir)
	{ cout<< "phantom directory create error: " + string(strerror(errno)) << endl; return -1; }
	else cout<<"Files will be generated in <" + baseDir + ">"<<endl;
	baseDir = "./" + baseDir + "/";
	string phantomName = StringSplitterLast(fileName.substr(0,fileName.size()-4),"/");

	// Check if there are TET ELE files already
	bool tetFlag = true;
	if((access((baseDir+phantomName+".ele").c_str(), F_OK)!= -1)&&
			(access((baseDir+phantomName+".node").c_str(), F_OK)!= -1)){
		int ans(0);
		while(ans==0){
			cout<<"ELE and NODE file already exists. Overwrite? (y/n): ";
			string temp; cin>>temp;
			if(temp=="y") ans=1; if(temp=="n") ans=2;
		}
		if(ans==1){
			system(("rm -rf "+baseDir+fileName.substr(0, fileName.size()-3)+"ele").c_str());
			system(("rm -rf "+baseDir+fileName.substr(0, fileName.size()-3)+"node").c_str());
		}
		if(ans==2) tetFlag = false;
	}
	if(tetFlag){
		/****For the OBJ files with the group names starting with "O_"****/
		if(oFlag){
			FixObj(fileName);
			fileName = fileName.substr(0, fileName.size() - 4) + "_2.obj";
		}
		int largestR = ConvertOBJ(fileName);
		if (largestR < 0) return -1;
		string smeshName = fileName.substr(0, fileName.size() - 4) + ".smesh";

		char** p_argv = new char*[3];
		for(int i=0;i<3;i++) p_argv[i]=new char[50];

		strcpy(p_argv[0], string("tetgen").c_str());
		if(sFlag) strcpy(p_argv[1], string("-p/0.0001YAFT0.000000001").c_str());
		else      strcpy(p_argv[1], string("-pAFT0.000000001").c_str());
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

		map<int, double> volMap = AnalyzeTet(tetFileName, oldFlag);
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
				(baseDir+phantomName+".ele").c_str());
		int nodeRename= rename((tetFileName + ".node").c_str(),
				(baseDir+phantomName+".node").c_str());
		int volRename= rename((tetFileName.substr(0,tetFileName.size()-2) + "_vol.txt").c_str(),
				(baseDir+phantomName+".vol").c_str());
		if(eleRename||nodeRename||volRename)	cerr<<"WARNING-rename failed."<<endl;
		system(("rm "+fileName.substr(0, fileName.size()-3)+"node").c_str());
		system(("rm "+fileName.substr(0, fileName.size()-3)+"smesh").c_str());

		cout<<endl<<"--Conversion was done: "<<fileName.substr(0, fileName.size()-3)+"1.node/ele were exported"<<endl;
	}
	
	//file generations
	if(!(gFlag+mFlag+pFlag)) return 0;
	if(matFile.empty()) {
		cout<<"Please provide material file for the generation of MC files"<<endl;
		return -1;
	}
	
	FileGen fileGen(phantomName, baseDir);
	fileGen.ReadMatFile(matFile, matFormat);

	if(gFlag)   {
		cout<<"-----------------------GEANT4 file generation-----------------------"<<endl;
		fileGen.GenerateG4(); cout<<endl;}
	if(mFlag){
		cout<<"------------------------MCNP6 file generation-----------------------"<<endl;
		fileGen.GenerateM6(); cout<<endl;}
	if(pFlag){
		cout<<"------------------------PHITS file generation-----------------------"<<endl;
		fileGen.GeneratePH(); cout<<endl;}

	return 0;
}
