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
// $Id: B4cCalorimeterSD.cc 100946 2016-11-03 11:28:08Z gcosmo $
//
/// \file B4cCalorimeterSD.cc
/// \brief Implementation of the B4cCalorimeterSD class

#include "B4cCalorimeterSD.hh"
#include "B4cReadoutGeometry.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"
#include "B4cTrackInformation.hh"
#include "B4cTrackingAction.hh"

#include "B4cDetParams.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4cCalorimeterSD::B4cCalorimeterSD(const G4String& name,const G4String& hitsCollectionName)
        : G4VSensitiveDetector(name),
        fHitsCollection(nullptr),
        cellcounter(0),
        hitcounter(0)
{
        collectionName.insert(hitsCollectionName);

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

B4cCalorimeterSD::~B4cCalorimeterSD()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4cCalorimeterSD::Initialize(G4HCofThisEvent* hce)
{
        eges=0;
        // Create hits collection
        fHitsCollection
                = new B4cCalorHitsCollection(SensitiveDetectorName, collectionName[0]);

        // Add this collection in hce
        auto hcID
                = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
        hce->AddHitsCollection( hcID, fHitsCollection );

        // if(this->GetName()=="GapSD") {
        //         fHitsCollection->insert(new B4cCalorHit());
        // }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4bool B4cCalorimeterSD::ProcessHits(G4Step* step,
                                     G4TouchableHistory* ROhist)
{
        G4int photNR=0;

        B4cTrackInformation* info = (B4cTrackInformation*)(step->GetTrack()->GetUserInformation());
        //G4cout << "Photon number: "<<info->GetOriginalPhotonNumber()<< G4endl;
        if(info) {

                photNR=info->GetOriginalPhotonNumber();
        }

        // energy deposit
        auto edep = step->GetTotalEnergyDeposit();

        auto touchable = (step->GetPreStepPoint()->GetTouchable());

        G4ThreeVector steppos=step->GetPreStepPoint()->GetPosition();

        std::string CalorPart=touchable->GetVolume()->GetLogicalVolume()->GetName();

        G4double stepLength = 0.;
        if ( step->GetTrack()->GetDefinition()->GetPDGCharge() != 0. ) {
                stepLength = step->GetStepLength();
        }

        if ( edep==0. && stepLength == 0. ) return false;
        eges+=edep/MeV;
        hitcounter++;

        // Get calorimeter cell id
        auto layerNumber = touchable->GetReplicaNumber(1);

        //auto LogicalVolume= ROhist->GetSolid()->GetName();

        G4int X;
        G4int Y;
        G4int Layer;
        G4int CalorSeg;

        // //Get copynumbers to specify cell
        // //
        // Cell=ROhist->GetReplicaNumber();
        // //  auto CellV=ROhist-> GetVolume()->GetName();
        //
        // Strip=ROhist->GetReplicaNumber(1);
        // //  auto StripV=ROhist-> GetVolume(1)->GetName();
        //
        // Layer=ROhist->GetReplicaNumber(3);
        // //  auto LayerV=ROhist-> GetVolume(3)->GetName();
        // std::cout<<"Z:"<<Layer /*<<" Y: "<<Strip<<" X: "<<Cell*/<<std::endl;
        //
        //CalorSeg=ROhist->GetReplicaNumber(4);
        // //std::cout<<"CalorSeg: "<<CalorSeg<<std::endl;

        //Calculate CellID
        G4ThreeVector st = touchable->GetTranslation(0);
        //G4int ROCellID=(Layer)*StilesPerLayer+(Strip)*ScellsPerStrip+Cell;
        //G4int ROLayerID=fNofCells*StilesPerLayer+Layer;
        if(CalorPart=="InnerGapLV") {

                X=floor(steppos.x()/GetInst().GetInnertileLenX())+GetInst().GetnofInnerTilesX()/2;
                Y=floor(steppos.y()/GetInst().GetInnertileLenY())+GetInst().GetnofInnerTilesY()/2;
                Layer=layerNumber;
                //std::cout<<"X: "<<X<<"Y: "<<Y<<"Z: "<<Layer<<std::endl;

                HitID=Layer*GetInst().GetInnertilesPerLayer()+X*GetInst().GetnofInnerTilesY()+Y;
        }
        else if(CalorPart=="OuterGapLV") {
          
                X=floor(steppos.x()/GetInst().GetOutertileLenX())+GetInst().GetnofOuterTilesX()/2;
                Y=floor(steppos.y()/GetInst().GetOutertileLenY())+GetInst().GetnofOuterTilesY()/2;
                Layer=layerNumber;

                //std::cout<<"X: "<<X<<"Y: "<<Y<<"Z: "<<Layer+GetInst().GetfNofInnerLayers()<<std::endl;

                HitID=GetInst().GetfNofInnerLayers()*GetInst().GetInnertilesPerLayer() + Layer*GetInst().GetOutertilesPerLayer() +X*GetInst().GetnofInnerTilesY() + Y;
        }
        else{

                std::cout<<"leck mich am oasch"<<std::endl;
                return false;
        }
        //std::cout<<"Z:"<<Layer <<" Y: "<<Strip<<" X: "<<Cell<<std::endl;

        B4cCalorHit * hit;

        it=cellmap.find(HitID);

        if(it==cellmap.end()) {
                cellmap.insert(std::pair<G4int,G4int>(HitID,cellcounter));
                fHitsCollection->insert(new B4cCalorHit());
                //std::cout<<"HitInserted"<<std::endl;
                hit=(*fHitsCollection)[cellcounter];

                cellcounter++;
        }
        else if(it!=cellmap.end()) {

                hit=(*fHitsCollection)[it->second];

        }
        // auto hit=(*fHitsCollection)[ROHitID];
        // if ( !hit ) {
        //         G4ExceptionDescription msg;
        //         msg << "Cannot access Gap hit " << layerNumber;
        //         G4Exception("B4cCalorimeterSD::ProcessHits()",
        //                     "MyCode0004", FatalException, msg);
        // }
        //

//        auto hitLayer = (*fHitsCollection)[ROLayerID];
        // Get hit for total accounting
        auto hitTotal = (*fHitsCollection)[fHitsCollection->entries()-1];

        // Add values to cell information
        hit->Add(edep, stepLength);
        hit->SetPhotonNumber(photNR);
        //G4cout<<hit->GetPhotonNumber()<<G4endl;
        if(hit->GetTouch()==false) {
                hit->SetCalorPart(CalorPart);
                hit->SetTouch();
                hit->SetCellInfo();
                hit->SetX(X);
                hit->SetY(Y);
                hit->SetZ(Layer);
                hit->SetCalorSeg(1);

        }
        //Add energydeposition for layered accounting
        // hitLayer->Add(edep,stepLength);
        // if(hitLayer->GetTouch()==false) {
        //         hitLayer->SetTouch();
        //         hitLayer->SetZ(Layer);
        //         hitLayer->SetX(0.); //Set X and Y to zero to prevent random coordinates
        //         hitLayer->SetY(0.); //
        // }
        //Add energydepositon for total accounting
        hitTotal->Add(edep, stepLength);
        if(hitTotal->GetTouch()==false) {
                hitTotal->SetZ(0.); //
                hitTotal->SetX(0.); //Set X, Y, Z to zero to prevent random coordinates
                hitTotal->SetY(0.); //
                hitTotal->SetTouch();
        }


        return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void B4cCalorimeterSD::EndOfEvent(G4HCofThisEvent*)
{
  fHitsCollection->insert(new B4cCalorHit());

  auto hitTotal = (*fHitsCollection)[fHitsCollection->entries()-1];
  hitTotal->Add(eges, 1.);
  if(hitTotal->GetTouch()==false) {
          hitTotal->SetZ(0.); //
          hitTotal->SetX(0.); //Set X, Y, Z to zero to prevent random coordinates
          hitTotal->SetY(0.); //
          hitTotal->SetTouch();
  }
  if ( verboseLevel>1 ) {
          auto nofHits = fHitsCollection->entries();
          G4cout
                  << G4endl
                  << "-------->Hits Collection: in this event they are " << nofHits
                  << " hits in the tracker chambers: " << G4endl;
          for ( G4int i=0; i<nofHits; i++ ) (*fHitsCollection)[i]->Print();
  }
  // std::cout<<"Run produced "<<hitcounter<<" Hits in "<<cellcounter<<" cells"<<std::endl;
  // std::cout<<"Hitcollection has "<<fHitsCollection->entries()<<" entries"<<std::endl;
  hitcounter=0;
  cellcounter=0;
  eges=0;
  cellmap.clear();

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
