/** @file OMSimDetectorConstruction.cc
 *  @brief User defined detector.
 *
 * You should define and construct the detector here...this template is an example for a single arg module.
 *
 *  @author Martin Unland - modified by Markus Dittmer
 *  @date October 2021 - modified at February 2022
 *
 *  @version Geant4 10.7
 *
 *  @todo
 */

#include "OMSimDetectorConstruction.hh"

#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"

#include "OMSimInputData.hh"
#include "OMSimPMTConstruction.hh"

#include "OMSimMDOM.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"



extern G4double gworldsize;
extern G4int gDOM;
extern G4bool gCADImport;
extern G4bool gPlaceHarness;

OMSimDetectorConstruction::OMSimDetectorConstruction()
    : mWorldSolid(0), mWorldLogical(0), mWorldPhysical(0)
{
}

OMSimDetectorConstruction::~OMSimDetectorConstruction()
{
}

/**
 * Construct the world volume
 */

void OMSimDetectorConstruction::ConstructWorldMat()
{
    //elements
    H = new G4Element("Hydrogen", "H", z = 1, a = 1.01 * g / mole);
    O = new G4Element("Oxygen", "O", z = 8, a = 16.00 * g / mole);

    ice = new G4Material("Ice", density = 0.9 * g / cm3, nelements = 2);
    ice -> AddElement(H, 2);
    ice -> AddElement(O, 1);

    //ice properties
    numEntries = 45;

    energyice = { 1.73*eV, 1.77143*eV, 1.7971*eV, 1.82353*eV, 1.85075*eV,
           1.87879*eV, 1.90769*eV, 1.9375*eV, 1.96825*eV, 2*eV,
           2.03279*eV, 2.06667*eV, 2.10169*eV, 2.13793*eV, 2.17544*eV,
           2.21429*eV, 2.25455*eV, 2.2963*eV, 2.33962*eV, 2.38462*eV,
           2.43137*eV, 2.48*eV, 2.53061*eV, 2.58333*eV, 2.6383*eV,
           2.69565*eV, 2.75556*eV, 2.81818*eV, 2.88372*eV, 2.95238*eV,
           3.02439*eV, 3.1*eV, 3.17949*eV, 3.26316*eV, 3.35135*eV,
           3.44444*eV, 3.54286*eV, 3.64706*eV, 3.75758*eV, 3.875*eV,
           4*eV, 4.13333*eV, 4.27586*eV, 4.42857*eV, 4.59259*eV };

    rindexice = { 1.30815, 1.30803, 1.30797, 1.30797, 1.30802,
           1.30812, 1.30826, 1.30843, 1.30863, 1.30887,
           1.30912, 1.30939, 1.30969, 1.31, 1.31032,
           1.31067, 1.31102, 1.31139, 1.31178, 1.31218,
           1.3126, 1.31304, 1.3135, 1.314, 1.31452,
           1.31507, 1.31567, 1.31631, 1.31699, 1.31774,
           1.31855, 1.31943, 1.32039, 1.32143, 1.32257,
           1.32381, 1.32517, 1.32665, 1.32827, 1.33003,
           1.33195, 1.33404, 1.33632, 1.33879, 1.34147 };

    absorptionice = { 1.53548*m, 1.75477*m, 2.01271*m, 2.31728*m, 2.67831*m,
           3.10791*m, 3.62113*m, 4.23659*m, 4.97741*m, 5.87229*m,
           6.95675*m, 8.2746*m, 9.87943*m, 11.836*m, 14.2211*m,
           17.1234*m, 20.6402*m, 24.8713*m, 29.9055*m, 35.8002*m,
           42.5523*m, 50.0649*m, 58.1194*m, 66.3693*m, 74.3702*m,
           81.65*m, 87.8004*m, 92.5553*m, 95.8262*m, 97.6856*m,
           98.3168*m, 97.9538*m, 96.8329*m, 95.1628*m, 93.1123*m,
           90.8086*m, 88.3431*m, 85.7786*m, 83.1565*m, 80.5033*m,
	   77.8352*m, 75.162*m, 72.4893*m, 69.8202*m, 67.1563*m };

    mieice = {
    43.34304558 *m, 42.42963752 *m, 41.88378269 *m, 41.33703248*m, 40.78945806*m,
       40.24116094*m, 39.69208334*m, 39.14203288*m, 38.59123555*m, 38.03942305*m,
       37.48673822*m, 36.93319642*m, 36.37886334*m, 35.82339849*m, 35.26700007*m,
       34.70962076*m, 34.15128373*m, 33.59194528*m, 33.03163909*m, 32.47010205*m,
       31.90766098*m, 31.34399899*m, 30.77926321*m, 30.21335953*m, 29.64620844*m,
       29.0779486*m , 28.50834676*m, 27.93759786*m, 27.3654842*m , 26.7920465*m ,
       26.21723599*m, 25.64102564*m, 25.0633547*m , 24.48422634*m, 23.90358884*m,
       23.32137519*m, 22.73747976*m, 22.15196722*m, 21.5647042*m , 20.97569605*m,
       20.38482208*m, 19.792053*m  , 19.19728392*m, 18.60046301*m, 18.00151303*m
   }; //using water scattering length for now.

    G4double mieconst[3] = { 0.99, 0.99, 0.8 };

    mptice = new G4MaterialPropertiesTable();
    mptice -> AddProperty("RINDEX", energyice, rindexice, numEntries);
    mptice -> AddProperty("ABSLENGTH", energyice, absorptionice,numEntries);
    mptice -> AddProperty("MIEHG", energyice, mieice,numEntries);
    mptice -> AddConstProperty("MIEHG_FORWARD", mieconst[0]);
	mptice -> AddConstProperty("MIEHG_BACKWARD", mieconst[1]);
	mptice -> AddConstProperty("MIEHG_FORWARD_RATIO", mieconst[2]);

	ice -> SetMaterialPropertiesTable(mptice);

	G4cout << "::::::::::::::::::World Material Constructed Sucessfully:::::::::::::::::::" << G4endl;

}
void OMSimDetectorConstruction::ConstructWorld()
{
    ConstructWorldMat();

    mWorldSolid = new G4Box("World", 20 * m, 20 * m, 20 * m);
    //mWorldSolid = new G4Box("World", 2 * m, 2 * m, 2 * m);
    /*mWorldLogical = new G4LogicalVolume(mWorldSolid, ice, "World_log", 0, 0, 0);
    mWorldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), mWorldLogical, "World_phys", 0, false, 0, true);*/
    mWorldLogical = new G4LogicalVolume(mWorldSolid, mData->GetMaterial("argWorld"), "World_log", 0, 0, 0);
    mWorldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), mWorldLogical, "World_phys", 0, false, 0);
    G4VisAttributes* World_vis = new G4VisAttributes(G4Colour(0.45,0.5,0.35,0.2));
    //G4VisAttributes* World_vis = new G4VisAttributes(G4Colour(0,0,0,0));
    mWorldLogical -> SetVisAttributes(World_vis);

    G4cout << "::::::::::::::::::World Volume Constructed Sucessfully:::::::::::::::::::" << G4endl;

}

// DONE UP TO THIS POINT

/**
 * Construct all solids needed for your study. Call OMSimOMConstruction for simulations with optical modules
 * and OMSimPMTConstruction for simulations with only PMTs.
 * @return World physical for the main
 */
G4VPhysicalVolume *OMSimDetectorConstruction::Construct()
{

    mData = new OMSimInputData();
    mData->SearchFolders("/home/waly/bulkice_doumeki/mdom/build");

    ConstructWorld();

    /*G4cout << "GDOM +++++++++++++++++++" << gDOM << G4endl;

    G4cout << "Constructing LOM18" << G4endl;
    LOM18 *lOpticalModule = new LOM18(mData,gPlaceHarness);
    lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
    G4cout << "::::::::::::::LOM18 successfully constructed::::::::::::" << G4endl;
    */




    if (gDOM == 0){ //Single PMT
        G4cout << "Constructing single PMT" << G4endl;
        mPMTManager = new OMSimPMTConstruction(mData);
        mPMTManager->SelectPMT("argPMT");

        G4RotationMatrix* lRot = new G4RotationMatrix();
        G4Transform3D lTransformers = G4Transform3D(*lRot, G4ThreeVector(0,0,0));
        mPMTManager->PlaceIt(lTransformers, mWorldLogical, "PMT");
    }
    else if (gDOM == 1){ //mDOM
        G4cout << "Constructing mDOM" << G4endl;
        mDOM *lOpticalModule = new mDOM(mData,gPlaceHarness);
        lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
    }
    else if (gDOM == 2){ //PDOM
        G4cout << "Constructing PDOM" << G4endl;
        pDOM *lOpticalModule = new pDOM(mData,gPlaceHarness);
        lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
    }
    else if (gDOM == 3){ //LOM16
        G4cout << "Constructing LOM16" << G4endl;
        LOM16 *lOpticalModule = new LOM16(mData,gPlaceHarness);
        lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
    }
    else if (gDOM == 4){ //LOM18
        G4cout << "Constructing LOM18" << G4endl;
        LOM18 *lOpticalModule = new LOM18(mData,gPlaceHarness);
        lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
        G4cout << "::::::::::::::LOM18 successfully constructed::::::::::::" << G4endl;
    }
     else if (gDOM == 5){ //DEGG
        G4cout << "Constructing DEGG" << G4endl;
        dEGG *lOpticalModule = new dEGG(mData,gPlaceHarness);
        lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
     }
    else{ //Add your costume detector contruction here and call it with -m 6 (or greater)
        G4cout << "Constructing custome detector construction" << G4endl;
    }





    return mWorldPhysical;
}
