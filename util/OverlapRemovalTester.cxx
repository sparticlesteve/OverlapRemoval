// System includes
#include <memory>
#include <cstdlib>

// ROOT includes
#include "TFile.h"
#include "TError.h"
#include "TString.h"

// Infrastructure includes
#ifdef ROOTCORE
#   include "xAODRootAccess/Init.h"
#   include "xAODRootAccess/TEvent.h"
#endif // ROOTCORE

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODEgamma/PhotonContainer.h"

// Local includes
#include "OverlapRemoval/OverlapRemovalTool.h"

// Other includes

// Error checking macro
#define CHECK( ARG )                                 \
  do {                                               \
    const bool result = ARG;                         \
    if(!result) {                                    \
      ::Error(APP_NAME, "Failed to execute: \"%s\"", \
              #ARG );                                \
      return 1;                                      \
    }                                                \
  } while( false )


void printObj(const char* APP_NAME, const char* type,
              const xAOD::IParticle* obj)
{
  static SG::AuxElement::ConstAccessor<int> overlapAcc("overlaps");
  Info(APP_NAME, "  %s pt %6.2f eta %5.2f phi %5.2f overlaps %i",
                 type, obj->pt()/1000., obj->eta(), obj->phi(),
                 overlapAcc.isAvailable(*obj)? overlapAcc(*obj) : -1);
  //static SG::AuxElement::Accessor<int> passAcc("passesOR");
  //Info(APP_NAME, "  %s pt %6.2f eta %5.2f phi %5.2f passesOR %i",
  //               type, obj->pt()/1000., obj->eta(), obj->phi(),
  //               passAcc.isAvailable(*obj)? passAcc(*obj) : -1);
}


int main( int argc, char* argv[] )
{

  // The application's name
  const char* APP_NAME = argv[ 0 ];

  // Check if we received a file name
  if(argc < 2) {
    Error( APP_NAME, "No file name received!" );
    Error( APP_NAME, "  Usage: %s [xAOD file name]", APP_NAME );
    return 1;
  }

  // Initialise the application
  CHECK( xAOD::Init(APP_NAME) );
  StatusCode::enableFailure();

  // Open the input file
  const TString fileName = argv[ 1 ];
  Info(APP_NAME, "Opening file: %s", fileName.Data());
  std::auto_ptr<TFile> ifile(TFile::Open(fileName, "READ"));
  CHECK( ifile.get() );

  // Create a TEvent object
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  CHECK( event.readFrom(ifile.get()) );
  Info(APP_NAME, "Number of events in the file: %i",
       static_cast<int>(event.getEntries()));

  // Decide how many events to run over
  Long64_t entries = event.getEntries();
  if(argc > 2) {
    const Long64_t e = atoll(argv[2]);
    if(e < entries) {
      entries = e;
    }
  }

  // Create and configure the tool
  OverlapRemovalTool orTool("OverlapRemovalTool");
  CHECK( orTool.setProperty("InputLabel", "") );
  orTool.msg().setLevel(MSG::DEBUG);

  // Initialize the tool
  CHECK( orTool.initialize() );

  // Loop over the events
  std::cout << "Starting loop" << std::endl;
  for(Long64_t entry = 0; entry < entries; ++entry){

    event.getEntry(entry);

    // Print some event information for fun
    const xAOD::EventInfo* ei = 0;
    CHECK( event.retrieve(ei, "EventInfo") );
    Info(APP_NAME,
         "===>>>  start processing event #%i, "
         "run #%i %i events processed so far  <<<===",
         static_cast<int>(ei->eventNumber()),
         static_cast<int>(ei->runNumber()),
         static_cast<int>(entry));

    // Get electrons
    const xAOD::ElectronContainer* electrons = 0;
    CHECK( event.retrieve(electrons, "ElectronCollection") );
    // Get muons
    const xAOD::MuonContainer* muons = 0;
    CHECK( event.retrieve(muons, "Muons") );
    // Get jets
    const xAOD::JetContainer* jets = 0;
    CHECK( event.retrieve(jets, "AntiKt4LCTopoJets") );
    // Get taus
    const xAOD::TauJetContainer* taus = 0;
    CHECK( event.retrieve(taus, "TauRecContainer") );
    // Get photons
    const xAOD::PhotonContainer* photons = 0;
    CHECK( event.retrieve(photons, "PhotonCollection") );

    Info(APP_NAME,
         "  nEle %lu, nMuo %lu, nJet %lu, nTau %lu, nPho %lu",
         electrons->size(), muons->size(),
         jets->size(), taus->size(),
         photons->size());

    // Apply the overlap removal to all objects (dumb example)
    CHECK( orTool.removeOverlaps(electrons, muons, jets, taus, photons) );

    //
    // Now, dump all of the results
    //

    // electrons
    Info(APP_NAME, "Now dumping the electrons");
    for(auto electron : *electrons)
      printObj(APP_NAME, "ele", electron);

    // muons
    Info(APP_NAME, "Now dumping the muons");
    for(auto muon : *muons)
      printObj(APP_NAME, "muo", muon);

    // jets
    Info(APP_NAME, "Now dumping the jets");
    for(auto jet : *jets)
      printObj(APP_NAME, "jet", jet);

    // taus
    Info(APP_NAME, "Now dumping the taus");
    for(auto tau : *taus)
      printObj(APP_NAME, "tau", tau);

    // photons
    Info(APP_NAME, "Now dumping the photons");
    for(auto photon : *photons)
      printObj(APP_NAME, "pho", photon);

  }

  return 0;

}
