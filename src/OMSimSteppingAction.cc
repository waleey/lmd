#include "OMSimSteppingAction.hh"

#include "G4RunManager.hh"
#include "G4SteppingManager.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
//since Geant4.10: include units manually
#include "G4SystemOfUnits.hh"

#include "OMSimAnalysisManager.hh"

extern OMSimAnalysisManager gAnalysisManager;
extern G4String	gHittype;
extern G4int gcounter;
extern G4String gQEFile;


OMSimSteppingAction::OMSimSteppingAction()
{

}


void OMSimSteppingAction::UserSteppingAction(const G4Step* aStep)
{    G4Track* aTrack = aStep->GetTrack();

    //kill particles that are stuck... e.g. doing a loop in the pressure vessel
    if ( aTrack-> GetCurrentStepNumber() > 100000) {
        G4cout << "Particle stuck   " <<  aTrack->GetDefinition()->GetParticleName()  << " " << 1239.84193/(aTrack->GetKineticEnergy()/eV)<< G4endl;
        // gAnalysisManager.infiniteLoop = true;
        //gAnalysisManager.SaveThisEvent = true;
        if ( aTrack->GetTrackStatus() != fStopAndKill ) {
            aTrack->SetTrackStatus(fStopAndKill);
        }
    }
        //just to find the source of the weird positrons!
        /*if(aTrack -> GetDefinition() -> GetParticleName() == "e+")
        {
            if(aTrack -> GetTrackID() > 6647)
            {
                G4String processName = aTrack -> GetCreatorProcess() -> GetProcessName();
                G4cout << "**********Weird pos are created by " << processName << " ***********" << G4endl;
            }
        }*/
        //find out source of weird gammas
        /*if(aTrack -> GetDefinition() -> GetParticleName() == "gamma")
        {
                G4String processName = aTrack -> GetCreatorProcess() -> GetProcessName();
                G4cout << "**********Weird gammas are created by " << processName << " ***********" << G4endl;
        }*/

    //	Check if optical photon is about to hit a photocathode, if so, destroy it and save the hit
    if ( aTrack->GetDefinition()->GetParticleName() == "opticalphoton" ) {
        gcounter ++;

        //G4cout << "++++++++++ I CAN IDENTIFY OPTICAL PHOTONS! ++++++++++" << G4endl;
        if ( aTrack->GetTrackStatus() != fStopAndKill ) {

        //G4cout << "++++++++++++++++++++ Gets Here +++++++++++++++++" << G4endl;

            //G4cout << "++++++++++++++++++++ Materials in +++++++++++++++++" << aStep -> GetPostStepPoint() -> GetPhysicalVolume() -> GetName() << " ++++++++++ " << G4endl;

            if ( aStep->GetPostStepPoint()->GetMaterial()->GetName() == "RiAbs_Photocathode") {

                G4ThreeVector vertex_pos;
                vertex_pos = aTrack -> GetVertexPosition();

           //G4cout << "+++++++++++++++++++ I HIT A PHOTO CATHODE! +++++++++++" << G4endl;
                G4double Ekin;
                G4double t1, t2;
                //G4Track* aTrack = aStep->GetTrack();

               // G4double h = 4.136E-15*eV*s;
                //G4double c = 2.99792458E17*nm/s;
                G4double hc = 1240 * nm;
                G4double lambda;

                Ekin = aTrack->GetKineticEnergy() ;
                lambda = (hc/Ekin) * nm;
                //std::cout << "Lambda : " << lambda / nm<< std::endl;
                pmt_qe -> ReadQeTable();
                double qe = (pmt_qe -> GetQe(lambda)) / 100;
                //double random = pmt_qe -> RandomGen();
                double random = CLHEP::RandFlat::shoot(0.0, 1.0);
                std::cout << "++++++++++QE : " << qe << "++++" << std::endl;
                bool survived = (random < (qe)) ? true : false;
                if(survived) ///taking QE into consideration
                {
                std::cout << "+++++++++++++I Survived!! ++++++++++" << std::endl;




                //G4cout << "+++++++++++++++++++ I HIT A PHOTO CATHODE! +++++++++++" << G4endl;
                std::vector<G4String> n;
                extern std::vector<G4String> explode (G4String s, char d);
                G4ThreeVector deltapos;


                n = explode(aStep->GetPreStepPoint()->GetPhysicalVolume()->GetName(),'_');
                gAnalysisManager.stats_PMT_hit.push_back(atoi(n.at(1)));
                //G4cout << "+++++++++++++ The Fuck Is " << atoi(n.at(1)) << " ++++++++" << G4endl;

                if (gHittype == "individual") {
                    deltapos = aTrack->GetVertexPosition() - aTrack->GetPosition();
                    t1 = aTrack->GetGlobalTime() /ns;
                    t2 = aTrack->GetLocalTime();
                    Ekin = aTrack->GetKineticEnergy();
                    gAnalysisManager.stats_photon_direction.push_back(aTrack->GetMomentumDirection());
                    gAnalysisManager.stats_photon_position.push_back(aTrack->GetPosition());
                    gAnalysisManager.stats_event_id.push_back(gAnalysisManager.current_event_id);
                    gAnalysisManager.stats_photon_flight_time.push_back(t2);
                    gAnalysisManager.stats_photon_track_length.push_back(aTrack->GetTrackLength()/m);
                    gAnalysisManager.stats_hit_time.push_back(t1);
                    gAnalysisManager.stats_photon_energy.push_back(Ekin/eV);
                    gAnalysisManager.stats_event_distance.push_back(deltapos.mag()/m);
                    gAnalysisManager.stats_vertex_position.push_back(vertex_pos);
                    gAnalysisManager.stats_positron_id.push_back(aTrack -> GetParentID());
                }

                aTrack->SetTrackStatus(fStopAndKill);
               } 		// kills counted photon to prevent scattering and double-counting
            }
        }
    }
}
