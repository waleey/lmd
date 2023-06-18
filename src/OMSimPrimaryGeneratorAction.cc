#include "OMSimPrimaryGeneratorAction.hh"


#include "G4Event.hh"
//#include "G4GeneralParticleSource.hh"
#include "G4ParticleTypes.hh"

#include <iostream>
#include <random>
#include <cmath>
#include <fstream>
#include <vector>
#include <stdlib.h>

extern G4double gworldsize;


OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	/*particleSource = new G4GeneralParticleSource ();
	particleSource->SetParticleDefinition(G4GenericIon::GenericIonDefinition());*/

	fParticleGun = new G4ParticleGun(1);
	G4cout << ":::::::::::::::Particle Gun Created:::::::::::" << G4endl;
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
	//delete particleSource;

	delete fParticleGun;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	//particleSource->GeneratePrimaryVertex(anEvent);

	using namespace std;
	SetUpEnergyAndPosition();

	G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
	G4String particleName = "e+";
	G4ParticleDefinition *particle = particleTable -> FindParticle(particleName);

	for(G4int i = 0; i < numParticles; i++)
	{
		G4ThreeVector particlePosition(data[X][i] * m, data[Y][i] * m, data[Z][i] * m);
		//G4cout << "particle x position:::::::::::::::::::" << particlePosition.x() / m<< G4endl;
		G4ThreeVector particleOrientation(data.at(AX)[i], data.at(AY)[i], data.at(AZ)[i]);

		G4double particleEnergy = data[ENERGY][i] * CLHEP::MeV;
		//cout << "This particle energy : " << particleEnergy << endl; //comment this out
		G4double particleInTime = data[TIME][i] *ms;
        //cout << "THis particle time : " << particleInTime << endl;
		//G4double particleInTime = 0 *CLHEP::ns;
		fParticleGun -> SetParticlePosition(particlePosition);
		fParticleGun -> SetParticleMomentumDirection(particleOrientation);
		fParticleGun -> SetParticleEnergy(particleEnergy);
		fParticleGun -> SetParticleDefinition(particle);
		fParticleGun -> SetParticleTime(particleInTime);
		fParticleGun -> GeneratePrimaryVertex(anEvent);

	}
	/*


	//dummy generator
	G4int numParticles = 1;
    G4double particleEnergy = 1.0 *CLHEP::MeV;

	G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
	G4String particleName = "e+";
	G4ParticleDefinition *particle = particleTable->FindParticle(particleName);

	G4ThreeVector pos(-20 * CLHEP::cm,0. ,0. ); //change photon generating position later.
	G4ThreeVector mom(1.0, 0. ,0.); //direction of momentum

    fParticleGun->SetParticlePosition(pos);
	fParticleGun->SetParticleMomentumDirection(mom);
	fParticleGun->SetParticleEnergy(particleEnergy); //change photon energy as necessary
	fParticleGun->SetParticleDefinition(particle);

	for(int i = 0; i < numParticles; i++)
	{
        fParticleGun->GeneratePrimaryVertex(anEvent);
	}
	*/
	for(auto& vals : data)
	{
        vals.clear();
	}
	//data.clear();
	/*if(data.empty())
	{
        std::cout << "Successfully Cleared the vectors!!" << std::endl;
	}
	else
	{
        std::cout << "Vectors cleaning not successful. Aborting...." << std::endl;
        exit(0);
	}*/
}


void OMSimPrimaryGeneratorAction::SetUpEnergyAndPosition()
{
    using namespace std;
    G4double temp;
    string fileName;

    for(int i = 0; i < data.size(); i++)
    {
        temp = 0;
        fileName = filePath + dtypes.at(i) + ".dat";

        ifstream file(fileName, std::ios::binary);

	if(!file.is_open())
	{
		std::cout << "Failed to open" + fileName << std::endl;

	}

	while(file.read((char*)&temp, sizeof(temp)))
	{
		data.at(i).push_back(temp);
	}

	file.close();
    }

    numParticles = data.at(ENERGY).size();

    G4cout << "Particles Information are set up for " << numParticles << " particles!" << G4endl;
}
