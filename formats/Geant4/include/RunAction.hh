//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// \file   ./include/RunAction.hh
// \generated by POLY2TET (author: Haegin Han)

#ifndef RunAction_h
#define RunAction_h 1

#include <ostream>
#include <fstream>
#include <map>

#include "G4RunManager.hh"
#include "G4UnitsTable.hh"
#include "G4UserRunAction.hh"
#include "G4SystemOfUnits.hh"

#include "Run.hh"
#include "TetModelImport.hh"

class RunActionMessenger;
class RunAction : public G4UserRunAction
{
public:
	RunAction(TetModelImport* tetData, G4String output);
	virtual ~RunAction();

public:
	virtual G4Run* GenerateRun();
	virtual void BeginOfRunAction(const G4Run*);
	virtual void EndOfRunAction(const G4Run*);

	void PrintResult(std::ostream &out);
	void SetBeamArea(G4double _area){beamArea=_area;}
  
private:
	TetModelImport* tetData;
	Run*            fRun;
	G4int           numOfEvent;
	G4int           runID;
	G4String        outputFile;
	G4double        beamArea;
	RunActionMessenger* fMessenger;
};

//simple messenger for RunAction
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

class RunActionMessenger: public G4UImessenger
{
public:
	RunActionMessenger(RunAction* _runAction)
	:G4UImessenger(), fRunAction(_runAction)
	{
		fRunActionDir = new G4UIdirectory("/result/");
		fBeamAreaCmd  = new G4UIcmdWithADoubleAndUnit("/result/beamArea", this);
	}
	virtual ~RunActionMessenger(){
		delete fRunActionDir;
		delete fBeamAreaCmd;
	}

	virtual void SetNewValue(G4UIcommand* command, G4String newValue){
		if(command==fBeamAreaCmd){
			fRunAction->SetBeamArea(fBeamAreaCmd->GetNewDoubleValue(newValue));
		}
	}

private:
	RunAction*                   fRunAction;
	G4UIdirectory*               fRunActionDir;
	G4UIcmdWithADoubleAndUnit*   fBeamAreaCmd;
};

#endif




