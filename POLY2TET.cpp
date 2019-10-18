#include "POLY2TET.h"

int main(int argc, char *argv[])
{
	int maxVolID(-1);
	for (int i = 1; i < argc; i++) {
		if (string(argv[i]) == "-c") {
			maxVolID = atoi(argv[i + 1]);
			i++;
		}
	}

	string fileName = argv[argc-1];

/****For the OBJ files with the group names starting with "O_"****/
//	string fileName;
//	cout << "filename: "; cin >> fileName;


	if (StringSplitterLast(fileName, ".") != "obj") {
		cerr << "File format should be obj" << endl;
		getchar();
		return 1;
	}

//	FixObj(fileName);
//	fileName = fileName.substr(0, fileName.size() - 4) + "_2.obj";
	int largestR = ConvertOBJ(fileName);
	if (largestR < 0) return -1;
	string smeshName = fileName.substr(0, fileName.size() - 4) + ".smesh";

	char* tetgen_name = new char[7];
	tetgen_name = "tetgen";
	char** p_argv = new char*[50];

	p_argv[0] = tetgen_name;
	p_argv[1] = "-p/0.0001YAFT0.000000001";
	p_argv[2] = (char*)smeshName.c_str();

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
	ofstream ofsVol(fileName.substr(0, fileName.size() - 3) + "_vol.txt");
	for (auto vol : volMap) {
		if (maxVol < vol.second) {
			maxVol = vol.second;
			tempID = vol.first;
		}
		ofsVol << vol.first << "\t" << vol.second << endl;
	}

	if (maxVolID < 0) maxVolID = tempID;
	ConvertZeroMat(maxVolID, fileName.substr(0, fileName.size() - 3) + "1.ele", largestR);

	cout<<endl<<"--Conversion was done: "<<fileName.substr(0, fileName.size()-3)+"1.node/ele were exported"<<endl;
	return 0;
}
