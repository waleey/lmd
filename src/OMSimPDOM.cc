#include "OMSimPDOM.hh"

#include "G4Ellipsoid.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"
#include "G4Polycone.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"
#include "G4Tubs.hh"

pDOM::pDOM(OMSimInputData* pData, G4bool pPlaceHarness) {
    mPlaceHarness = pPlaceHarness;
    mData = pData;
    mPMTManager = new OMSimPMTConstruction(mData);
    Construction();
}

/**
 * Construction of the pDOM, as in the older code.
 */
void pDOM::Construction()
{
    G4UnionSolid* lPMTsolid;

    mPMTManager->SelectPMT("pmt_Hamamatsu_R7081");
    lPMTsolid = mPMTManager->GetPMTSolid();

    G4double mGelThickness = 10 * mm;

    G4double lPMTz = 0.5 * 12 * 25.4 * mm - mPMTManager->GetDistancePMTCenterToPMTtip() - mGelThickness;

    G4Orb* lGlassSphereSolid = new G4Orb("PDOM_GlassSphere solid", 0.5 * 13 * 25.4 * mm);
    G4Orb* lGelSphereSolid = new G4Orb("PDOM_GelSphere solid", 0.5 * 12 * 25.4 * mm);

    G4Ellipsoid* lAirAuxSolid = new G4Ellipsoid("PDOM_AirAux solid", 0.5 * 12 * 25.4 * mm, 0.5 * 12 * 25.4 * mm, 0.5 * 12 * 25.4 * mm, -0.5 * 13 * 25.4 * mm, 50 * mm);

    G4SubtractionSolid* lAirSolid = new G4SubtractionSolid("PDOM_Air solid", lAirAuxSolid, lPMTsolid, 0, G4ThreeVector(0, 0, lPMTz));
    G4Tubs* lBoardSolid = new G4Tubs("PDOM_Board solid", 52 * mm, 0.5 * 11 * 25.4 * mm, 2 * mm, 0, 2 * CLHEP::pi);
    G4Tubs* lBaseSolid = new G4Tubs("PDOM_Board solid", 0 * mm, 6 * cm, 2 * mm, 0, 2 * CLHEP::pi);

    //Harness
    G4double lHarnessInner[] = { 0, 0, 0, 0 };
    G4double lHarnessRadii[] = { (0.5 * 365.76 - 8.3) * mm, 0.5 * 365.76 * mm, 0.5 * 365.76 * mm, (0.5 * 365.76 - 8.3) * mm };
    G4double lHarnessZplanes[] = { -31.75 * mm, -10 * mm, 10 * mm, 31.75 * mm };
    G4Polycone* lHarnessAuxSolid = new G4Polycone("PDOM_HarnessAux solid", 0, 2 * CLHEP::pi, 4, lHarnessZplanes, lHarnessInner, lHarnessRadii);
    G4SubtractionSolid* lHarnessSolid = new G4SubtractionSolid("PDOM_Harness solid", lHarnessAuxSolid, lGlassSphereSolid);

    //Logicals mData
    G4LogicalVolume* lGlassSphereLogical = new G4LogicalVolume(lGlassSphereSolid,
        mData->GetMaterial("argVesselGlass"),
        "PDOM_Glass logical");
    G4LogicalVolume* lHarnessLogical = new G4LogicalVolume(lHarnessSolid,
        mData->GetMaterial("NoOptic_Stahl"),
        "PDOM_Harness logical");

    G4LogicalVolume* lGelLogical = new G4LogicalVolume(lGelSphereSolid,
        mData->GetMaterial("argGel"),
        "PDOM_Gel logical");

    G4LogicalVolume* lBoardLogical = new G4LogicalVolume(lBoardSolid,
        mData->GetMaterial("NoOptic_Absorber"),
        "PDOM_Board logical");

    G4LogicalVolume* lBaseLogical = new G4LogicalVolume(lBaseSolid,
        mData->GetMaterial("NoOptic_Absorber"),
        "PDOM_Base logical");

    G4LogicalVolume* lAirLogical = new G4LogicalVolume(lAirSolid,
        mData->GetMaterial("Ri_Vacuum"),
        "PDOM_Air logical");

    G4PVPlacement* lBoardPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, -40 * mm), lBoardLogical, "pDOMBoardPhys", lAirLogical, false, 0);
    G4PVPlacement* lBasePhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, -105 * mm), lBaseLogical, "pDOMBasePhys", lAirLogical, false, 0);

    G4PVPlacement* lAirPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lAirLogical, "pDOMAirPhys ", lGelLogical, false, 0);

    mPMTManager->PlaceIt(G4ThreeVector(0, 0, lPMTz), new G4RotationMatrix(), lGelLogical);

    G4PVPlacement* lGelPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lGelLogical, "pDOMGelPhys", lGlassSphereLogical, false, 0);

    if (mPlaceHarness) AppendComponent(lHarnessSolid, lHarnessLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "pDOM_Harness");
    
    AppendComponent(lGlassSphereSolid, lGlassSphereLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "pDOM");

    // ------------------- optical border surfaces --------------------------------------------------------------------------------
    new G4LogicalSkinSurface("PDOM_Harness_skin", lHarnessLogical, mData->GetOpticalSurface("Refl_V95Gel"));

    lGlassSphereLogical->SetVisAttributes(mGlassVis);
    lHarnessLogical->SetVisAttributes(mSteelVis);
    lGelLogical->SetVisAttributes(mGelVis);
    lAirLogical->SetVisAttributes(mAirVis);
    lBoardLogical->SetVisAttributes(mBoardVis);
    lBaseLogical->SetVisAttributes(mBoardVis);
}
