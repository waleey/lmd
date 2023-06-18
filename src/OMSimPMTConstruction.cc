/** @file OMSimPMTConstruction.cc
 *  @brief Construction of the PMTs.
 *
 *  This class creates the solids of the PMTs and place them in the detector/OMs.
 *  Methods are order as in the header. Please take a look there first!
 *  @author Lew Classen, Martin Unland
 *  @date October 2021
 *
 *  @version Geant4 10.7
 *
 *  @todo
 *
 */

#include <dirent.h>

#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4Ellipsoid.hh>
#include "G4LogicalVolume.hh"
#include "G4LogicalBorderSurface.hh"
#include <G4PVPlacement.hh>
#include <G4Sphere.hh>
#include <G4SubtractionSolid.hh>
#include "G4SystemOfUnits.hh"
#include <G4Torus.hh>
#include <G4Tubs.hh>
#include <G4UnionSolid.hh>
#include <G4UnitsTable.hh>
#include "G4VisAttributes.hh"

#include "OMSimPMTConstruction.hh"
#include "OMSimLogger.hh"

extern G4bool gVisual;
extern G4int gPMT;

/**
 * Constructor of the class. The InputData instance has to be passed here in order to avoid loading the input data twice and redifining the same materials.
 * @param pData OMSimInputData instance
 */
OMSimPMTConstruction::OMSimPMTConstruction(OMSimInputData *pData)
{
    mData = pData;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                               Methods of abstract base class "PMT"
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */
/**
 * Constructor of the PMT base class. The InputData instance has to be passed here in order to avoid loading the input data twice and redifining the same materials.
 * @param pData OMSimInputData instance
 */
OMSimPMTConstruction::PMT::PMT(OMSimInputData *pData, G4String pSelectedPMT)
{
    mData = pData;
    mSelectedPMT = pSelectedPMT;
}

/**
 * Constructs the PMT Solid.
 */
void OMSimPMTConstruction::PMT::ConstructIt()
{
    std::cerr<< "******************************************" << std::endl;
    std::cerr<< " OMSimPMTConstruction::PMT::ConstructIt is called " << std::endl;
    BasicShape();
    mPMTlogical = new G4LogicalVolume(mPMTSolid, mData->GetMaterial("RiAbs_Glass_Tube"), "PMT tube logical");
    mPMTlogical->SetVisAttributes(mGlassVis);

    if (mInternalReflections)
    {
        //G4cout << "++++++++++++++Internal Reflection Activated +++++++++++++++" << G4endl;
        G4SubtractionSolid *lVacuumTubeSolid = new G4SubtractionSolid("Vacuum Tube solid", mGlassInside, mVacuumPhotocathodeSolid, 0, G4ThreeVector(0, 0, 0));

        G4LogicalVolume *lVacuumPhotocathodeLogical = new G4LogicalVolume(mVacuumPhotocathodeSolid, mData->GetMaterial("Ri_Vacuum"), "Photocathode area vacuum");
        G4LogicalVolume *lVacuumTubeLogical = new G4LogicalVolume(lVacuumTubeSolid, mData->GetMaterial("Ri_Vacuum"), "Tube vacuum");

        mVacuumPhotocathodePlacement = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lVacuumPhotocathodeLogical, "Vacuum_1", mPMTlogical, false, 0, mCheckOverlaps);
        mVacuumTubePlacement = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lVacuumTubeLogical, "Vacuum_3", mPMTlogical, false, 0, mCheckOverlaps);

        if (mDynodeSystem)
        {
            DynodeSystemConstruction(lVacuumTubeLogical);
        }
        lVacuumPhotocathodeLogical->SetVisAttributes(mPhotocathodeVis);
        //lVacuumTubeLogical->SetVisAttributes(mAirVis);
        //lVacuumMirrorLogical->SetVisAttributes(mInvisibleVis);
    }

    else
    {
        /*
        G4LogicalVolume *lTubeVacuum = new G4LogicalVolume(mGlassInside, mData->GetMaterial("Ri_Vacuum"), "PMTvacuum");
        G4LogicalVolume *lPhotocathode = new G4LogicalVolume(mVacuumPhotocathodeSolid, mData->GetMaterial("RiAbs_Photocathode"), "Photocathode");

        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lTubeVacuum, "VacuumTube", mPMTlogical, false, 0, mCheckOverlaps);
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lPhotocathode, "VacuumPhoto", lTubeVacuum, false, 0, mCheckOverlaps);
        */
        //G4cout << "++++++++++++++No Internal Reflection Activated +++++++++++++++" << G4endl;
        G4SubtractionSolid *lVacuumTubeSolid_BackBulb = new G4SubtractionSolid("Vacuum Tube solid", mGlassInside, mVacuumPhotocathodeSolid, 0, G4ThreeVector(0, 0, 0));
        G4SubtractionSolid *lVacuumTubeSolid = new G4SubtractionSolid("Vacuum Tube solid", mBulkSolid, mVacuumPhotocathodeSolid, 0, G4ThreeVector(0, 0, mMissingTubeLength));
        G4SubtractionSolid *lBackBulbSolid = new G4SubtractionSolid("Vacuum Tube solid", lVacuumTubeSolid_BackBulb, lVacuumTubeSolid, 0, G4ThreeVector(0, 0, -mMissingTubeLength));

        G4LogicalVolume *lVacuumPhotocathodeLogical = new G4LogicalVolume(mVacuumPhotocathodeSolid, mData->GetMaterial("RiAbs_Photocathode"), "Photocathode area vacuum");

        //G4cout << "++++++++++++++Sensitive Guy++++++++++ " << lVacuumPhotocathodeLogical -> GetMaterial() -> GetName() << " " << G4endl;
        G4LogicalVolume *lVacuumTubeLogical = new G4LogicalVolume(lVacuumTubeSolid, mData->GetMaterial("NoOptic_Absorber"), "Fully absorber"); //I guess this is not trully fully absorber though...
        G4LogicalVolume *lBackBulbLogical = new G4LogicalVolume(lBackBulbSolid, mData->GetMaterial("RiAbs_Glass_Tube"), "Reflective mirror");

        mVacuumPhotocathodePlacement = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lVacuumPhotocathodeLogical, "Photocathode_pv_OMSIM", mPMTlogical, false, 0, mCheckOverlaps);
        mVacuumTubePlacement = new G4PVPlacement(0, G4ThreeVector(0, 0, -mMissingTubeLength), lVacuumTubeLogical, "BackTube", mPMTlogical, false, 0, mCheckOverlaps);

        mVacuumTubePlacement = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lBackBulbLogical, "BackBulb", mPMTlogical, false, 0, mCheckOverlaps);

        CathodeBackShield(lBackBulbLogical);

        //lVacuumPhotocathodeLogical->SetVisAttributes(mPhotocathodeVis);
        //lVacuumPhotocathodeLogical->SetVisAttributes(mInvisibleVis);
        const G4VisAttributes *lBackBulbVis = new G4VisAttributes(G4Colour(0.0, 0.0, 0.0, 1));
        const G4VisAttributes *lBackTubeVis = new G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 1));
        lVacuumTubeLogical->SetVisAttributes(lBackTubeVis);
        //lVacuumTubeLogical->SetVisAttributes(mInvisibleVis);
        lBackBulbLogical->SetVisAttributes(lBackBulbVis);
        //lBackBulbLogical->SetVisAttributes(mInvisibleVis);

    }
}

/**
 * Placement of the PMT and definition of LogicalBorderSurfaces in case internal reflections are needed.
 * @param pPosition G4ThreeVector with position of the module (as in G4PVPlacement())
 * @param pRotation G4RotationMatrix with rotation of the module (as in G4PVPlacement())
 * @param pMother G4LogicalVolume where the module is going to be placed (as in G4PVPlacement())
 * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name
 */
void OMSimPMTConstruction::PMT::PlaceIt(G4ThreeVector pPosition, G4RotationMatrix *pRotation, G4LogicalVolume *&pMother, G4String pNameExtension)
{
    G4PVPlacement *lPMTPhysical = new G4PVPlacement(pRotation, pPosition, mPMTlogical, "PMT_" + pNameExtension, pMother, false, 0, mCheckOverlaps);
    if (mInternalReflections)
    {
        G4OpticalSurface *Photocathode_opsurf = new G4OpticalSurface("Photocathode_opsurf");
        new G4LogicalBorderSurface("Photocathode_out", mVacuumPhotocathodePlacement, lPMTPhysical, Photocathode_opsurf);
        new G4LogicalBorderSurface("Photocathode_in", lPMTPhysical, mVacuumPhotocathodePlacement, Photocathode_opsurf);                  //*/
        new G4LogicalBorderSurface("PMT_mirrorglass", mVacuumTubePlacement, lPMTPhysical, mData->GetOpticalSurface("Refl_100polished")); //*/
        new G4LogicalBorderSurface("PMT_mirrorglass", lPMTPhysical, mVacuumTubePlacement, mData->GetOpticalSurface("Refl_100polished")); //*/
    } else {
        new G4LogicalBorderSurface("PMT_mirrorglass", mVacuumTubePlacement, lPMTPhysical, mData->GetOpticalSurface("Refl_100polished")); //*/
        new G4LogicalBorderSurface("PMT_mirrorglass", lPMTPhysical, mVacuumTubePlacement, mData->GetOpticalSurface("Refl_100polished")); //*/
    }
}
/**
 * @see PMT::PlaceIt
 * @param pTransform G4Transform3D with position & rotation of PMT
 */
void OMSimPMTConstruction::PMT::PlaceIt(G4Transform3D pTransform, G4LogicalVolume *&pMother, G4String pNameExtension)
{
    G4PVPlacement *lPMTPhysical = new G4PVPlacement(pTransform, mPMTlogical, "PMT_" + pNameExtension, pMother, false, 0, mCheckOverlaps);

    if (mInternalReflections)
    {
        G4OpticalSurface *Photocathode_opsurf = new G4OpticalSurface("Photocathode_opsurf");
        new G4LogicalBorderSurface("Photocathode_out", mVacuumPhotocathodePlacement, lPMTPhysical, Photocathode_opsurf);
        new G4LogicalBorderSurface("Photocathode_in", lPMTPhysical, mVacuumPhotocathodePlacement, Photocathode_opsurf);                  //*/
        new G4LogicalBorderSurface("PMT_mirrorglass", mVacuumTubePlacement, lPMTPhysical, mData->GetOpticalSurface("Refl_100polished")); //*/
        new G4LogicalBorderSurface("PMT_mirrorglass", lPMTPhysical, mVacuumTubePlacement, mData->GetOpticalSurface("Refl_100polished")); //*/
    } else {
        new G4LogicalBorderSurface("PMT_mirrorglass", mVacuumTubePlacement, lPMTPhysical, mData->GetOpticalSurface("Refl_PMTSideMirror")); //*/
        new G4LogicalBorderSurface("PMT_mirrorglass", lPMTPhysical, mVacuumTubePlacement, mData->GetOpticalSurface("Refl_PMTSideMirror")); //*/
    }
}

/**
 * The basic shape of the PMT is constructed twice, once for the external solid and once for the internal. A subtraction of these two shapes would yield the glass envelope of the PMT. The function calls either BulbConstructionSimple or BulbConstructionFull, depending on the data provided and simulation type. In case only the frontal curvate of the photocathode has to be well constructed, it calls BulbConstructionSimple. BulbConstructionFull constructs the neck of the PMT precisely, but it needs to have the fit data of the PMT type and is only needed if internal reflections are simulated.
 * @see BulbConstructionSimple
 * @see BulbConstructionFull
 */
void OMSimPMTConstruction::PMT::BasicShape()
{
    std::cerr << "OMSimPMTConstruction::PMT::BasicShape is called" << std::endl;
    G4SubtractionSolid *lVacuumPhotocathodeSolid;

    G4String lBulbBackShape = mData->GetString(mSelectedPMT,"jBulbBackShape");
    if (lBulbBackShape == "Simple")
        mSimpleBulb = true;

    if (mSimpleBulb || !mInternalReflections)
    {
        std::cerr << "simple bulb or not internal reflection " << std::endl;
        std::tie(mPMTSolid, lVacuumPhotocathodeSolid) = BulbConstructionSimple("jOuterShape");
        std::tie(mGlassInside, mVacuumPhotocathodeSolid) = BulbConstructionSimple("jInnerShape");
    }
    else
    {
        std::cerr << "lBulbBackShape is " << lBulbBackShape << std::endl;
        std::cerr << "mInternalReflections is " << mInternalReflections << std::endl;
        std::tie(mPMTSolid, lVacuumPhotocathodeSolid) = BulbConstructionFull("jOuterShape");
        std::tie(mGlassInside, mVacuumPhotocathodeSolid) = BulbConstructionFull("jInnerShape");
    }
}

/**
 * Construction of the basic shape of the PMT.
 * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
 */
std::tuple<G4UnionSolid *, G4SubtractionSolid *> OMSimPMTConstruction::PMT::BulbConstructionSimple(G4String pSide)
{
    std::cerr << "OMSimPMTConstruction::PMT::BulbConstructionSimple is called for pside " << pSide << std::endl;
    ReadParameters(pSide);
    G4UnionSolid *lBulbSolid = FrontalBulbConstruction();
    std::cerr << "OMSimPMTConstruction::PMT::BulbConstructionSimple bulb solid is generated " << std::endl;
    // Defining volume with boundaries of photocathode volume
    G4Tubs *lLargeTube = new G4Tubs("LargeTube", 0, mEllipseXYaxis, 50 * cm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid *lPhotocathodeSide = new G4SubtractionSolid("SubstractionPhotocathodeSide", lBulbSolid, lLargeTube, 0, G4ThreeVector(0, 0, -50 * cm));

    // Rest of tube
    G4double lFrontToEllipse_y = mOutRad + mSpherePos_y - mEllipsePos_y;

    G4double lMissingTubeLength = (mTotalLenght - lFrontToEllipse_y) * 0.5 * mm;
    G4Tubs *lBulkSolid = new G4Tubs("Bulb bulk solid", 0.0, 0.5 * mTubeWidth, lMissingTubeLength, 0, 2 * CLHEP::pi);
    if (pSide == "jOuterShape")
        mPMTCenterToTip = lFrontToEllipse_y;
    if (pSide == "jInnerShape")
        mBulkSolid = lBulkSolid;
        mMissingTubeLength = lMissingTubeLength;
    lBulbSolid = new G4UnionSolid("Bulb tube solid", lBulbSolid, lBulkSolid, 0, G4ThreeVector(0, 0, -lMissingTubeLength));
    return std::make_tuple(lBulbSolid, lPhotocathodeSide);
}

/**
 * Construction of the basic shape of the PMT for a full paramterised PMT. This is needed if internal reflections are simulated.
 * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
 */
std::tuple<G4UnionSolid *, G4SubtractionSolid *> OMSimPMTConstruction::PMT::BulbConstructionFull(G4String pSide)
{
    ReadParameters(pSide);
    G4UnionSolid *lBulbSolid = FrontalBulbConstruction();
    // Defining volume with boundaries of photocathode volume
    G4Tubs *lLargeTube = new G4Tubs("LargeTube", 0, mEllipseXYaxis, 50 * cm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid *lPhotocathodeSide = new G4SubtractionSolid("SubstractionPhotocathodeSide", lBulbSolid, lLargeTube, 0, G4ThreeVector(0, 0, -50 * cm));

    // Rest of tube
    G4double lFrontToEllipse_y = mOutRad + mSpherePos_y - mEllipsePos_y;

    G4double lMissingTubeLength = (mTotalLenght - lFrontToEllipse_y) * 0.5 * mm;
    G4Tubs *lBulkSolid = new G4Tubs("Bulb bulk solid", 0.0, 0.5 * mTubeWidth, lMissingTubeLength, 0, 2 * CLHEP::pi);

    if (pSide == "jOuterShape")
        mPMTCenterToTip = lFrontToEllipse_y;

    // Creating Cone
    G4double lConeLength_x = mEllipseConeTransition_x - mConeTorusTransition_x;
    G4double lConeHalfHeight = mLineFitSlope * lConeLength_x * 0.5;
    G4Cons *lCone = new G4Cons("Solid substraction cone", mConeTorusTransition_x, mConeTorusTransition_x + mTubeWidth, mEllipseConeTransition_x, mEllipseConeTransition_x + mTubeWidth, lConeHalfHeight, 0, 2 * CLHEP::pi);

    // Cone is substracted from frontal volume
    G4double lConeEllipse_y = mEllipseConeTransition_y - mEllipsePos_y - lConeHalfHeight;
    G4SubtractionSolid *lBulbSolidSubstractions = new G4SubtractionSolid("Substracted solid bulb", lBulbSolid,
                                                                         lCone, 0, G4ThreeVector(0, 0, lConeEllipse_y));

    // Creating Torus
    G4Torus *lTorus = new G4Torus("Solid substraction torus", 0.0, mTorusCircleR, mTorusCirclePos_x, 0, 2 * CLHEP::pi);
    G4double lTorusToEllipse = mTorusCirclePos_y - mEllipsePos_y;
    G4Tubs *lTubeEdge = new G4Tubs("Solid edge of torus", mTorusCirclePos_x, mEllipseConeTransition_x + mTubeWidth, mTorusCirclePos_x * 0.5, 0, 2 * CLHEP::pi);

    G4UnionSolid *lTorusTubeEdge = new G4UnionSolid("Solid torus with cylindrical edges", lTorus, lTubeEdge, 0, G4ThreeVector(0, 0, 0));

    // Create Tube for substracting cone and torus
    G4double lSubstractionTubeLength = mEllipseConeTransition_y - mTorusTubeTransition_y;
    G4Tubs *lSubstractionTube = new G4Tubs("substracion_tube", 0.0, mEllipseConeTransition_x, 0.5 * lSubstractionTubeLength, 0, 2 * CLHEP::pi);

    G4double lSTubeEllipse_y = mEllipseConeTransition_y - mEllipsePos_y - lSubstractionTubeLength * 0.5;

    G4SubtractionSolid *lBulbBack = new G4SubtractionSolid("Solid back of PMT", lSubstractionTube, lCone, 0, G4ThreeVector(0, 0, lConeEllipse_y - lSTubeEllipse_y));
    lBulbBack = new G4SubtractionSolid("Solid back of PMT", lBulbBack, lTorusTubeEdge, 0, G4ThreeVector(0, 0, lTorusToEllipse - lSTubeEllipse_y));

    // For some parameters this shape is too complicated for the visualizer, so I don't use the boolean in case of visualization
    // if you wanna see the PMT, negate this condition and use RayTracerX
    if (gVisual)
    {
        lBulbSolid = new G4UnionSolid("Bulb tube solid", lBulbSolidSubstractions, lBulbBack, 0, G4ThreeVector(0, 0, lSTubeEllipse_y));
        lBulbSolid = new G4UnionSolid("Bulb tube solid", lBulbSolid, lBulkSolid, 0, G4ThreeVector(0, 0, -lMissingTubeLength));
    }
    else
    {
        lBulbSolid = new G4UnionSolid("Bulb tube solid", lBulbSolid, lBulkSolid, 0, G4ThreeVector(0, 0, -lMissingTubeLength));
    }
    return std::make_tuple(lBulbSolid, lPhotocathodeSide);
}

/**
 * Creates and positions a thin disk behind the photocathode volume in order to shield photons coming from behind the PMT. Only used when internal reflections are turned off.
 */
void OMSimPMTConstruction::PMT::CathodeBackShield(G4LogicalVolume *pPMTinner)
{
    ReadParameters("jInnerShape");
    G4double lPCDiameter = 2 * mEllipseXYaxis;
    G4double lShieldBottomcut = mEllipseZaxis / mEllipseXYaxis * std::sqrt(std::pow(mEllipseXYaxis, 2.) - std::pow(0.5 * lPCDiameter, 2.));
    G4double lShieldRad = -0.01 * mm + mEllipseXYaxis / mEllipseZaxis * std::sqrt(std::pow(mEllipseZaxis, 2.) - std::pow(0.5 * lShieldBottomcut, 2.));
    G4Tubs *lShieldSolid = new G4Tubs("Shield solid", 0, lShieldRad, 0.1 * mm, 0, 2 * CLHEP::pi);
    G4LogicalVolume *lShieldLogical = new G4LogicalVolume(lShieldSolid, mData->GetMaterial("NoOptic_Absorber"), "Shield logical");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -0.1 * mm), lShieldLogical, "Shield physical", pPMTinner, false, 0, mCheckOverlaps);
    lShieldLogical->SetVisAttributes(mSteelVis);
}

/**
 * Construction & placement of the dynode system entrance for internal reflections. Currently only geometry for Hamamatsu R15458.
 * @param pMother LogicalVolume of the mother, where the dynode system entrance is placed (vacuum volume)
 */
void OMSimPMTConstruction::PMT::DynodeSystemConstruction(G4LogicalVolume *pMother)
{

    const G4double lEntranceH = 19.5 * mm;
    const G4double lEntranceW = 21.9 * mm;
    const G4double lPlateWidth = 0.7 * mm;
    const G4double lPlateRad = 46. * 0.5 * mm;
    const G4double lFirstDynodeH = 18.06 * mm;
    const G4double lFirstDynodeCurvatureR = 14.6439 * mm;

    const G4double lFurthestX = 13.8593179 * mm;
    const G4double lFurthestY = 2.0620 * mm;

    G4Tubs *lFirstDynode = new G4Tubs("FirstDynode", 0, lFirstDynodeCurvatureR, lEntranceW * 0.5, 8.46 * degree, 106.64 * degree - 8.46 * degree);
    G4Tubs *lSubstTube = new G4Tubs("FirstDynode", 0, lFirstDynodeCurvatureR, lEntranceW * 1, 0, 114.39 * degree);
    G4Box *lSubBox = new G4Box("box", 30 * mm, lFurthestY, lEntranceW);

    const G4double lleftBoxH = 14.102 * mm;
    const G4double lleftBoxW = 4.216 * mm;
    G4Box *lleftBox = new G4Box("box", 0.5 * lleftBoxW, 0.5 * lleftBoxH, lEntranceW * 0.5);

    const G4double lThirdDynodeHeight = 6.2 * mm;

    const G4double lLargeBoxH = lleftBoxH - lThirdDynodeHeight - lFurthestY;
    const G4double lLargeBoxW = 7 * mm;
    G4Box *lLargeBox = new G4Box("box", 0.5 * lLargeBoxW, 0.5 * lLargeBoxH, lEntranceW * 0.5);

    G4SubtractionSolid *newSubst = new G4SubtractionSolid("Subst", lFirstDynode, lSubstTube, 0, G4ThreeVector(lFurthestX, lFurthestY, 0));
    newSubst = new G4SubtractionSolid("Subst", newSubst, lSubBox, 0, G4ThreeVector(0, 0, 0));

    G4UnionSolid *lUnion = new G4UnionSolid("curvature leftbox", newSubst, lleftBox, 0, G4ThreeVector(-lleftBoxW * 0.5, lleftBoxH * 0.5, 0));
    lUnion = new G4UnionSolid("curvature leftbox", lUnion, lLargeBox, 0, G4ThreeVector(-lLargeBoxW, lLargeBoxH * 0.5 + lFurthestY, 0));

    G4Tubs *lDynodeEntrancePlate = new G4Tubs("DynodeEntrancePlate", 0, lPlateRad, lPlateWidth, 0, 360. * degree);
    G4Box *lEntranceWindowSubst = new G4Box("Entrance window", lEntranceW * 0.5, lEntranceH * 0.5, 30 * mm);
    G4SubtractionSolid *lDynodeEntrance = new G4SubtractionSolid("Substracted Plate", lDynodeEntrancePlate, lEntranceWindowSubst);

    G4Tubs *lDynodeSystem = new G4Tubs("DynodeEntrancePlate", 0, lPlateRad - 2 * mm, 0.5 * 15 * mm, 0, 360. * degree);
    G4Tubs *lAbsorber = new G4Tubs("DynodeEntrancePlate", lPlateRad - 1.99 * mm, (52.2 * mm - 2 * 1.351 * mm) * 0.5, 0.5 * 10 * mm, 0, 360. * degree);
    G4Tubs *lBackGlass = new G4Tubs("DynodeEntrancePlate", lPlateRad - 1.99 * mm, (52.2 * mm - 2 * 1.351 * mm) * 0.5, 0.5 * 2 * mm, 0, 360. * degree);

    G4RotationMatrix *myRotation1 = new G4RotationMatrix();
    myRotation1->rotateX(90. * deg);
    myRotation1->rotateY(90. * deg);
    G4SubtractionSolid *lSubstractedDynodeSystem = new G4SubtractionSolid("Substracteddynode", lDynodeSystem, lUnion, myRotation1, G4ThreeVector(0, -(lFurthestX - lEntranceH * 0.5), +lFurthestY + 15 * 0.5 * mm + 0.1 * mm));

    G4LogicalVolume *lDynodeSystemLog = new G4LogicalVolume(lSubstractedDynodeSystem, mData->GetMaterial("NoOptic_Reflector"), "PMT_12199 tube logical");
    G4LogicalVolume *lAbsorberLog = new G4LogicalVolume(lAbsorber, mData->GetMaterial("NoOptic_Absorber"), "PMT_12199_absorber");
    G4LogicalVolume *lBackGlassLog = new G4LogicalVolume(lBackGlass, mData->GetMaterial("RiAbs_Glass_Tube"), "PMT_12199_absorber");
    G4LogicalVolume *lDynodeEntranceLog = new G4LogicalVolume(lDynodeEntrance, mData->GetMaterial("NoOptic_Reflector"), "PMT_12199 tube logical");

    G4double lDistanceDynodeFrontalEllipsoid = -(29.0 - 7.9 - lPlateWidth) * mm;
    G4PVPlacement *lDynodePlatePhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, lDistanceDynodeFrontalEllipsoid), lDynodeEntranceLog, "Frontal_plate", pMother, false, 0, mCheckOverlaps);
    G4PVPlacement *lDynodeSystemPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, -15 * 0.5 * mm - lPlateWidth + lDistanceDynodeFrontalEllipsoid), lDynodeSystemLog, "FirstDynode", pMother, false, 0, mCheckOverlaps);
    G4PVPlacement *lAbsorberPhys = new G4PVPlacement(0, G4ThreeVector(0, 0, -40 * 0.5 * mm - lPlateWidth + lDistanceDynodeFrontalEllipsoid), lAbsorberLog, "Absorber", pMother, false, 0, mCheckOverlaps);
    G4PVPlacement *lBackGlassPhys = new G4PVPlacement(0, G4ThreeVector(0, 0, -18 * 0.5 * mm - lPlateWidth + lDistanceDynodeFrontalEllipsoid), lBackGlassLog, "BackGlass", pMother, false, 0, mCheckOverlaps);
    new G4LogicalBorderSurface("PMT_platemirror", mVacuumTubePlacement, lDynodePlatePhysical, mData->GetOpticalSurface("Refl_100polished"));
    new G4LogicalBorderSurface("PMT_platemirror", lDynodePlatePhysical, mVacuumTubePlacement, mData->GetOpticalSurface("Refl_100polished"));
    new G4LogicalBorderSurface("PMT_1dynmirror", mVacuumTubePlacement, lDynodeSystemPhysical, mData->GetOpticalSurface("Refl_100polished"));
    new G4LogicalBorderSurface("PMT_1dynmirror", lDynodeSystemPhysical, mVacuumTubePlacement, mData->GetOpticalSurface("Refl_100polished"));
}

/**
 * Reads the parameter table and assigns the value and dimension of member variables.
 */
void OMSimPMTConstruction::PMT::ReadParameters(G4String pSide)
{
    mTotalLenght = mData->GetValue(mSelectedPMT, pSide + ".jTotalLenght");
    mTubeWidth = mData->GetValue(mSelectedPMT, pSide + ".jTubeWidth");
    mOutRad = mData->GetValue(mSelectedPMT, pSide + ".jOutRad");
    mEllipseXYaxis = mData->GetValue(mSelectedPMT, pSide + ".jEllipseXYaxis");
    mEllipseZaxis = mData->GetValue(mSelectedPMT, pSide + ".jEllipseZaxis");
    mSphereEllipseTransition_r = mData->GetValue(mSelectedPMT, pSide + ".jSphereEllipseTransition_r");
    mSpherePos_y = mData->GetValue(mSelectedPMT, pSide + ".jSpherePos_y");
    mEllipsePos_y = mData->GetValue(mSelectedPMT, pSide + ".jEllipsePos_y");

    G4String lFrontalShape = mData->GetString(mSelectedPMT, "jFrontalShape");

    if (!mSimpleBulb)
    {
        mLineFitSlope = mData->GetValue(mSelectedPMT, pSide + ".jLineFitSlope");
        mEllipseConeTransition_x = mData->GetValue(mSelectedPMT, pSide + ".jEllipseConeTransition_x");
        mEllipseConeTransition_y = mData->GetValue(mSelectedPMT, pSide + ".jEllipseConeTransition_y");
        mConeTorusTransition_x = mData->GetValue(mSelectedPMT, pSide + ".jConeTorusTransition_x");
        mTorusCircleR = mData->GetValue(mSelectedPMT, pSide + ".jTorusCircleR");
        mTorusCirclePos_x = mData->GetValue(mSelectedPMT, pSide + ".jTorusCirclePos_x");
        mTorusCirclePos_y = mData->GetValue(mSelectedPMT, pSide + ".jTorusCirclePos_y");
        mTorusTubeTransition_y = mData->GetValue(mSelectedPMT, pSide + ".jTorusTubeTransition_y");
    }

    if (lFrontalShape == "2Ellipses")
    {
        mEllipseXYaxis_2 = mData->GetValue(mSelectedPMT, pSide + ".jEllipseXYaxis_2");
        mEllipseZaxis_2 = mData->GetValue(mSelectedPMT, pSide + ".jEllipseZaxis_2");
        mEllipsePos_y_2 = mData->GetValue(mSelectedPMT, pSide + ".jEllipsePos_y_2");
    }

    if (pSide == "jOuterShape")
    {
        mPMTMaxRad = mEllipseXYaxis;
    }
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Derived classes from ::PMT::
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */
/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with SphereEllipsePhotocathode were fitted with a sphere and an ellipse.
 * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
 */
G4UnionSolid *OMSimPMTConstruction::SphereEllipsePhotocathode::FrontalBulbConstruction()
{
    G4double lSphereAngle = asin(mSphereEllipseTransition_r / mOutRad);
    // PMT frontal glass envelope as union of sphere and ellipse
    G4Ellipsoid *lBulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", mEllipseXYaxis, mEllipseXYaxis, mEllipseZaxis);
    G4Sphere *lBulbSphere = new G4Sphere("Solid Bulb Ellipsoid", 0.0, mOutRad, 0, 2 * CLHEP::pi, 0, lSphereAngle);
    G4UnionSolid *lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbEllipsoid, lBulbSphere, 0, G4ThreeVector(0, 0, mSpherePos_y - mEllipsePos_y));
    return lBulbSolid;
}

/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with SphereDoubleEllipsePhotocathode were fitted with a sphere and two ellipses.
 * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
 */
G4UnionSolid *OMSimPMTConstruction::SphereDoubleEllipsePhotocathode::FrontalBulbConstruction()
{
    G4double lSphereAngle = asin(mSphereEllipseTransition_r / mOutRad);
    // PMT frontal glass envelope as union of sphere and ellipse
    G4Ellipsoid *lBulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", mEllipseXYaxis, mEllipseXYaxis, mEllipseZaxis);
    G4Sphere *lBulbSphere = new G4Sphere("Solid Bulb Ellipsoid", 0.0, mOutRad, 0, 2 * CLHEP::pi, 0, lSphereAngle);
    G4UnionSolid *lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbEllipsoid, lBulbSphere, 0, G4ThreeVector(0, 0, mSpherePos_y - mEllipsePos_y));
    G4Ellipsoid *lBulbEllipsoid_2 = new G4Ellipsoid("Solid Bulb Ellipsoid 2", mEllipseXYaxis_2, mEllipseXYaxis_2, mEllipseZaxis_2);
    G4double lExcess = mEllipsePos_y - mEllipsePos_y_2;
    G4Tubs *lSubtractionTube = new G4Tubs("substracion_tube_large_ellipsoid", 0.0, mEllipseXYaxis_2 * 2, 0.5 * mTotalLenght, 0, 2 * CLHEP::pi);
    G4SubtractionSolid *lSubstractedLargeEllipsoid = new G4SubtractionSolid("Substracted Bulb Ellipsoid 2", lBulbEllipsoid_2, lSubtractionTube, 0, G4ThreeVector(0, 0, lExcess - mTotalLenght * 0.5));
    lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbSolid, lSubstractedLargeEllipsoid, 0, G4ThreeVector(0, 0, mEllipsePos_y_2 - mEllipsePos_y));
    return lBulbSolid;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Main class methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 * Returns the distance between the 0.0 position of the PMT solid volume and the plane normal to the PMT frontal tip.
 * @return G4double
 */
G4double OMSimPMTConstruction::GetDistancePMTCenterToPMTtip()
{
    return mPMT->mPMTCenterToTip;
}
/**
 * Returns the maximal radius of the frontal part of the PMT.
 * @return G4double
 */
G4double OMSimPMTConstruction::GetMaxPMTMaxRadius()
{
    return mPMT->mPMTMaxRad;
}
/**
 * Returns the solid of the constructed PMT.
 * @return G4UnionSolid of the PMT
 */
G4UnionSolid *OMSimPMTConstruction::GetPMTSolid()
{
    return mPMT->mPMTSolid;
}
/**
 * @see PMT::PlaceIt
 */
void OMSimPMTConstruction::PlaceIt(G4ThreeVector pPosition, G4RotationMatrix *pRotation, G4LogicalVolume *&pMother, G4String pNameExtension)
{
    mPMT->PlaceIt(pPosition, pRotation, pMother, pNameExtension);
}

/**
 * @see PMT::PlaceIt
 */
void OMSimPMTConstruction::PlaceIt(G4Transform3D pTransform, G4LogicalVolume *&pMother, G4String pNameExtension)
{
    mPMT->PlaceIt(pTransform, pMother, pNameExtension);
}
/**
 * Select PMT model to use and assigns mPMT class.
 * @param G4String pPMTtoSelect string with the name of the PMT model
 */
void OMSimPMTConstruction::SelectPMT(G4String pPMTtoSelect)
{
    std::cerr << "***************************************" << std::endl;
    std::cerr << "OMSimPMTConstruction::SelectPMT is called " << std::endl;
    std::cerr << "***************************************" << std::endl;

    if (pPMTtoSelect.substr(0, 6) == "argPMT")
    {
        const G4String lPMTTypes[] = {"pmt_Hamamatsu_R15458", "pmt_ETEL_9320KFL-KFB", "pmt_HZC_XP82B2F", "pmt_Hamamatsu_4inch", "pmt_dEGG"};
        pPMTtoSelect = lPMTTypes[gPMT];
    }
    mSelectedPMT = pPMTtoSelect;

    //Check if requested PMT is in the table of PMTs
    if (mData->CheckIfKeyInTable(pPMTtoSelect))
    {//if found
        G4String mssg = pPMTtoSelect + " selected.";
        notice(mssg);

        const G4String lFrontalShape = mData->GetString(mSelectedPMT, "jFrontalShape");

        //Check what kind of fit was done on the PMT and assign class accordingly.
        if (lFrontalShape == "SphereEllipse")
            mPMT = new SphereEllipsePhotocathode(mData, mSelectedPMT);
        else if (lFrontalShape == "2Ellipses")
            mPMT = new SphereDoubleEllipsePhotocathode(mData, mSelectedPMT);

        mPMT->ConstructIt();
    }
    else
    {
        critical("Selected PMT not in PMT tree, please check that requested PMT exists in data folder.");
    }
}

/**
 * The PMT is constructed allowing internal reflections (in the mirrored part opposite to the photocathode). Do not use it if you really don't need it!!
 */
void OMSimPMTConstruction::SimulateInternalReflections()
{
    mPMT->mInternalReflections = true;
    if (mSelectedPMT == "pmt_Hamamatsu_R15458")
        mPMT->mDynodeSystem = true;
    else
    {
        warning("Internal reflections inside PMT is on, but this PMT has no Dynode system defined, only vacuum! I will continue with internal reflections anyway...");
        mPMT->mDynodeSystem = false;
    }
    mPMT->ConstructIt();
}
