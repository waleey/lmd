#include <iostream>
#include <sstream>

#include "G4RunManager.hh"
#include "G4UIterminal.hh"
#include "G4UItcsh.hh"
#include "FTFP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4OpticalPhysics.hh"
#include "G4SystemOfUnits.hh"

#define G4VIS_USE 1
#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#endif

#include "OMSimDetectorConstruction.hh"
#include "OMSimPhysicsList.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimEventAction.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimSteppingAction.hh"
#include "OMSimSteppingVerbose.hh"
#include "OMSimAnalysisManager.hh"
//#include "OMSimPMTQE.hh"

//setting up the external variables
G4int           gGlass = 1;
G4int           gGel = 1;
G4double        gRefCone_angle = 51;
G4int           gConeMat = 1;
G4int           gHolderColor = 1;
//G4int           gDOM = 0; // 0 : single PMT
G4int           gDOM = 1; // 1 : mdom
//G4int           gDOM = 2; // 2 : pdom
G4int           gPMT = 3; // i don't know what is it
G4bool          gPlaceHarness = true;
G4int           gHarness = 1;
G4int           gRopeNumber = 1;
G4double        gworldsize = 40.*m;

G4bool          gCADImport = false;
G4String        gHittype = "individual"; // seems like individual records each hit per pmt
G4bool          gVisual = true; // may be visualization on?
G4int           gEnvironment = 1; // I don't know what is it
G4String        ghitsfilename = "/mnt/c/Users/Waly/bulkice_doumeki/hit.dat";
G4int           gcounter = 0;
G4String        gQEFile = "/home/waly/bulkice_doumeki/mdom/InputFile/TA0001_HamamatsuQE.data";

//G4String base_name = "/mnt/c/Users/Waly/bulkice_doumeki/" ;

/*if(gDOM == 0)
    ghitsfilename = base_name + "hit_PMT.dat";
else if(gDOM == 1)
    ghitsfilename = base_name + "hit_MDOM.dat";
else if(gDOM == 2)
    ghitsfilename = base_name + "hit_DOM.dat";
else if(gDOM == 3)
    ghitsfilename = base_name + "hit_LOM16.dat";
else if(gDOM == 4)
    ghitsfilename = base_name + "hit_LOM18.dat";
else if(gDOM == 5)
    ghitsfilename = base_name + "hit_DEGG.dat";
else
    G4cout << "Invalid PMT model selected" << G4endl;

*/

/*switch(gDOM)
{
    case 0 :
        ghitsfilename = base_name + "hit_PMT.dat";
        break;
    case 1 :
    ghitsfilename = base_name + "hit_MDOM.dat";
        break;
    case 2 :
    ghitsfilename = base_name + "hit_DOM.dat";
        break;
    case 3 :
    ghitsfilename = base_name + "hit_LOM16.dat";
        break;
    case 4 :
    ghitsfilename = base_name + "hit_LOM18.dat";
        break;
    case 5 :
    ghitsfilename = base_name + "hit_DEGG.dat";
        break;
    default :
        G4cout << "Invalid PMT model selected" << G4endl;
        exit(0);
}*/

OMSimAnalysisManager gAnalysisManager;

std::vector<G4String> explode(G4String s, char d) {
        std::vector<G4String> o;
        int i,j;
        i = s.find_first_of("#");
        if (i == 0) return o;
        while (s.size() > 0) {
                i = s.find_first_of(d);
                j = s.find_last_of(d);
                o.push_back(s.substr(0, i));
                if (i == j) {
                        o.push_back(s.substr(j+1));
                        break;
                }
                s.erase(0,i+1);
        }
        return o;
    }

std::vector<G4String> explode(char* cs, char d) {
        std::vector<G4String> o;
        G4String s = cs;
        return explode(s,d);
}

int main(int argc, char** argv)
{
    G4String macroname;
    if(argc != 1)
    {
        G4cout << ":::::::::::::::::::Batch Mode Called:::::::::::::::::" << G4endl;
        macroname = G4String(argv[1]);
    }

    G4RunManager* runmanager = new G4RunManager();

    //OMSimDetectorConstruction* detector = new OMSimDetectorConstruction();
    //Generating Physics List
    G4VModularPhysicsList* physicsList = new FTFP_BERT;
    physicsList->ReplacePhysics(new G4EmStandardPhysics_option4());
    G4OpticalPhysics* opticalPhysics = new G4OpticalPhysics();
    physicsList->RegisterPhysics(opticalPhysics);

    runmanager -> SetUserInitialization(new OMSimDetectorConstruction);
    runmanager -> SetUserInitialization(physicsList);
    //runmanager -> SetUserInitialization(new OMSimPhysicsList);
    runmanager -> SetUserAction(new OMSimPrimaryGeneratorAction);
    runmanager -> SetUserAction(new OMSimEventAction);
    runmanager -> SetUserAction(new OMSimRunAction);
    runmanager -> SetUserAction(new OMSimSteppingAction);
    runmanager -> SetUserAction(new OMSimTrackingAction);

    #ifdef G4VIS_USE
  // initialize visualization package
    G4VisManager* vismanager = new G4VisExecutive();
  //G4VisManager* visManager= new J4VisManager;
    vismanager -> Initialize();
    std::cerr << " ------------------------------- " << std::endl
         << " ---- VisManager created! ---- " << std::endl
         << " ------------------------------- " << std::endl;
    std::cerr << std::endl;
    #endif

    std::cerr << "about to initialize runManager" << std::endl;
    /*OMSimPMTQE* pmt_qe = new OMSimPMTQE();
    pmt_qe -> ReadQeTable();*/
    runmanager-> Initialize();
    std::cerr << "initialize runManager succeed" << std::endl;

    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    G4UIExecutive* ui = 0;

    if ( argc!=1 ) {
    // batch mode
        std::cerr << ":::::::::::::::::::Batch Mode Called:::::::::::::::::" << std::endl;
        G4String command = "/control/execute " + macroname;
        std::cout << "command " << command << std::endl;
        UImanager->ApplyCommand(command);
        }
    else {
    // interactive mode
        std::cerr << "interactive mode called" << std::endl;
        ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute vis.mac");
        /*UImanager ->ApplyCommand("/vis/open OGL");
        UImanager ->ApplyCommand("/vis/drawVolume");
        UImanager ->ApplyCommand("/vis/scene/add/trajectories smooth");
        UImanager ->ApplyCommand("/vis/viewer/set/autoRefresh true");*/
        ui-> SessionStart();
        delete ui;
    }

  //-----------------------
  // terminating...
  //-----------------------

    #ifdef G4VIS_USE
    delete vismanager;
    #endif

    delete runmanager;
    std::cout << "::::::::::::::this is the end:::::::::::::"<< std::endl;
    return 0;

}
