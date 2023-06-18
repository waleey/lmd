#ifndef OMSimAnalysisManager_h
#define OMSimAnalysisManager_h 1

#include "G4Types.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"

#include <vector>
#include <fstream>

class OMSimAnalysisManager
{
	public:
		OMSimAnalysisManager();
		~OMSimAnalysisManager();

		void Reset();
		void Write();
		void WriteAccept();
		void Debug() { std::cerr << "OMSimAnalysisManager is alive" << std::endl; }

		// run quantities
		G4String outputFilename;
		std::fstream datafile;
		G4long current_event_id;
		std::vector<G4long>	stats_event_id;
		std::vector<G4double>	stats_hit_time;
		std::vector<G4double>	stats_photon_flight_time;
		std::vector<G4double>	stats_photon_track_length;
		std::vector<G4double>	stats_photon_energy;
		std::vector<G4int>	stats_PMT_hit;
		std::vector<G4ThreeVector>	stats_photon_direction;
		std::vector<G4ThreeVector>	stats_photon_position;
		std::vector<G4ThreeVector> stats_vertex_position;
		std::vector<G4double>	stats_event_distance;
		std::vector<G4int> stats_positron_id;



	private:

};

#endif
