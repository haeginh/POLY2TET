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

#ifndef TetParameterisation_h
#define TetParameterisation_h 1

#include "TetModelImport.hh"

#include "globals.hh"
#include "G4VPVParameterisation.hh"
#include "G4VSolid.hh"
#include "G4Material.hh"
#include "G4VisAttributes.hh"

#include <map>

class G4VPhysicalVolume;

class TetParameterisation : public G4VPVParameterisation
{
  public:
    TetParameterisation(TetModelImport* tetData);
    virtual ~TetParameterisation();
    
    virtual G4VSolid* ComputeSolid(
    		       const G4int copyNo, G4VPhysicalVolume* );
    
    virtual void ComputeTransformation(
                   const G4int,G4VPhysicalVolume*) const;

    virtual G4Material* ComputeMaterial(const G4int copyNo,
                                        G4VPhysicalVolume* phy,
                                        const G4VTouchable*);

  private:
    TetModelImport*                    tetData;
    std::map<G4int, G4VisAttributes*>  visAttMap;
    G4bool                             isforVis;
};

#endif
