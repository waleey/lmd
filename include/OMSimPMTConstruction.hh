#ifndef OMSimPMTConstruction_h
#define OMSimPMTConstruction_h 1

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4VisAttributes.hh"
#include "G4SubtractionSolid.hh"
#include "G4PVPlacement.hh"
#include "OMSimInputData.hh"
#include "G4Tubs.hh"
#include <tuple>
#include <map>
namespace pt = boost::property_tree;

class OMSimPMTConstruction
{
public:
    OMSimPMTConstruction(OMSimInputData *pData);

protected:
    // Abstract base class
    class PMT
    {
    public:
        PMT(OMSimInputData *pDataSource, G4String pSelectedPMT);
        void ConstructIt();
        void PlaceIt(G4ThreeVector pPosition, G4RotationMatrix *pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");
        void PlaceIt(G4Transform3D pTransform, G4LogicalVolume *&pMother, G4String pNameExtension = "");
        G4String mSelectedPMT;
        G4bool mDynodeSystem = false;
        G4bool mInternalReflections = false;
        G4double mPMTCenterToTip;
        G4double mPMTMaxRad;
        G4UnionSolid *mPMTSolid;

    protected:
        void BasicShape();
        std::tuple<G4UnionSolid *, G4SubtractionSolid *> BulbConstructionSimple(G4String pSide);
        std::tuple<G4UnionSolid *, G4SubtractionSolid *> BulbConstructionFull(G4String pSide);
        void CathodeBackShield(G4LogicalVolume *pPMTIinner);
        void DynodeSystemConstruction(G4LogicalVolume *pMother);
        void ReadParameters(G4String pSide);

        virtual G4UnionSolid *FrontalBulbConstruction() = 0; // abstract method

        G4LogicalVolume *mPMTlogical;
        G4UnionSolid *mGlassInside;
        G4bool mSimpleBulb;
        G4double mMissingTubeLength ;
        G4PVPlacement *mVacuumPhotocathodePlacement;
        G4SubtractionSolid *mVacuumPhotocathodeSolid;
        G4Tubs *mBulkSolid;
        G4PVPlacement *mVacuumTubePlacement;

        bool mCheckOverlaps = true;

        OMSimInputData *mData;

        //Variables from json files are saved in the following members
        G4double mTotalLenght;
        G4double mTubeWidth;
        G4double mOutRad;
        G4double mEllipseXYaxis;
        G4double mEllipseZaxis;
        G4double mSphereEllipseTransition_r;
        G4double mSpherePos_y;
        G4double mEllipsePos_y;
        G4double mLineFitSlope;
        G4double mEllipseConeTransition_x;
        G4double mEllipseConeTransition_y;
        G4double mConeTorusTransition_x;
        G4double mTorusCircleR;
        G4double mTorusCirclePos_x;
        G4double mTorusCirclePos_y;
        G4double mTorusTubeTransition_y;
        G4double mEllipseXYaxis_2;
        G4double mEllipsePos_y_2;
        G4double mEllipseZaxis_2;

        //Visual attributes
        const G4VisAttributes *mAluVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.9, 1.0));
        const G4VisAttributes *mGlassVis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.2));
        const G4VisAttributes *mPhotocathodeVis = new G4VisAttributes(false, G4Colour(1.0, 0, 0.0));
        const G4VisAttributes *mSteelVis = new G4VisAttributes(G4Colour(0.75, 0.75, 0.85, 1.0));
        const G4VisAttributes *mAirVis = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 1.0));
        const G4VisAttributes mInvisibleVis = G4VisAttributes::GetInvisible();
        //const G4VisAttributes *mPhotocathodeVis = new G4VisAttributes(G4Colour(1.0, 0, 0.0));
    };

    // Derived classes
    class SphereEllipsePhotocathode : public PMT
    {
    public:
        SphereEllipsePhotocathode(OMSimInputData *pDataSource, G4String pSelectedPMT) : PMT(pDataSource, pSelectedPMT){};

    public:
        G4UnionSolid *FrontalBulbConstruction();
    };
    class SphereDoubleEllipsePhotocathode : public PMT
    {
    public:
        SphereDoubleEllipsePhotocathode(OMSimInputData *pDataSource, G4String pSelectedPMT) : PMT(pDataSource, pSelectedPMT){};

    public:
        G4UnionSolid *FrontalBulbConstruction();
    };

public:
    G4double GetDistancePMTCenterToPMTtip();
    G4double GetMaxPMTMaxRadius();
    G4UnionSolid *GetPMTSolid();

    void PlaceIt(G4ThreeVector pPosition, G4RotationMatrix *pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    void PlaceIt(G4Transform3D pTransform, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    // K.H this function will be override so add virtual
    virtual void SelectPMT(G4String pPMTtoSelect);
    void SimulateInternalReflections();

protected:
    OMSimInputData *mData;
    PMT *mPMT;
    G4String mSelectedPMT;
};

#endif
//
