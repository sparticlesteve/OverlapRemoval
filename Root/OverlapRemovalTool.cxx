// EDM includes
#include "AthContainers/AuxElement.h"

// Local includes
#include "OverlapRemoval/OverlapRemovalTool.h"

//-----------------------------------------------------------------------------
// Standard constructor
//-----------------------------------------------------------------------------
OverlapRemovalTool::OverlapRemovalTool(const std::string& name)
        : asg::AsgTool(name)
{
  // declare configurable properties here
  declareProperty("InputLabel", m_inputLabel = "selected");
  declareProperty("OutputLabel", m_outputLabel = "passesOR");
}

//-----------------------------------------------------------------------------
// Initialize the tool
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::initialize()
{
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove all overlapping objects according to the official
// harmonization prescription
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::
removeOverlaps(const xAOD::ElectronContainer* electrons,
               const xAOD::MuonContainer* muons,
               const xAOD::JetContainer* jets,
               const xAOD::TauJetContainer* taus,
               const xAOD::PhotonContainer* photons)
{
  /*
    Recommended removal sequence

    1. Tau ID and OR with specific loose electrons and muons
    2. ID and isolation of electrons and muons
    3. e-µ OR
    4. lep-photon OR
    5. lep/photon - jet OR
  */

  // Tau and loose ele/mu OR
  if(taus){
    ATH_CHECK( removeTauEleOverlap(taus, electrons) );
    ATH_CHECK( removeTauMuonOverlap(taus, muons) );
  }
  // e-mu OR
  ATH_CHECK( removeEleMuonOverlap(electrons, muons) );
  // photon and e/mu OR
  // TODO: where does photon-photon fit in?
  if(photons){
    ATH_CHECK( removePhotonPhotonOverlap(photons) );
    ATH_CHECK( removePhotonEleOverlap(photons, electrons) );
    ATH_CHECK( removePhotonMuonOverlap(photons, muons) );
  }
  // lep/photon and jet OR
  ATH_CHECK( removeEleJetOverlap(electrons, jets) );
  ATH_CHECK( removeMuonJetOverlap(muons, jets) );
  if(photons) ATH_CHECK( removePhotonJetOverlap(photons, jets) );
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping electrons and jets
// Need two steps so as to avoid using rejected jets in the 2nd step.
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removeEleJetOverlap
(const xAOD::ElectronContainer* electrons, const xAOD::JetContainer* jets)
{
  // Remove jets that overlap with electrons in dR < 0.2
  for(const auto jet : *jets){
    // Check that this jet passes the input selection
    if(isSurvivingObject(jet)){
      // TODO: migrate to generic overlap method here?
      int jetPass = 1;
      // Loop over electrons
      for(const auto electron : *electrons){
        // Check for overlap
        // TODO: drop hardcoded cone in favor of member
        if(isSurvivingObject(electron) && objectsOverlap(electron, jet, 0.2)){
          jetPass = 0;
          break;
        }
      }
      setOutputDecoration(jet, jetPass);
    }
  }
  // Remove electrons that overlap with surviving jets in dR < 0.4.
  // Maybe this should get its own method.
  for(const auto electron : *electrons){
    // Check that this electron passes the input selection
    if(isSurvivingObject(electron)){
      // TODO: migrate to generic overlap method here?
      int elePass = 1;
      // Loop over jets
      for(const auto jet : *jets){
        // Check for overlap with surviving jets
        if(isSurvivingObject(jet) && objectsOverlap(electron, jet, 0.4)) {
          elePass = 0;
          break;
        }
      }
      setOutputDecoration(electron, elePass);
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping muons and jets
// Note that because of the numTrack requirement on the jet,
// we are able to do this in just one double loop, unlike ele-jet.
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removeMuonJetOverlap
(const xAOD::MuonContainer* muons, const xAOD::JetContainer* jets)
{
  // Accessor to jet.nTrack
  // TODO: figure out the correct aux key
  static SG::AuxElement::ConstAccessor<int> nTrkAcc("numTracks");

  // Loop over jets
  for(const auto jet : *jets){
    if(isSurvivingObject(jet)){
      int nTrk = nTrkAcc(*jet);
      // TODO: migrate to generic overlap method here?
      // Loop over muons
      for(const auto muon : *muons){
        // Check for overlap
        if(isSurvivingObject(muon) && objectsOverlap(jet, muon, 0.4)){
          // Toss muon
          if(nTrk > 2){
            setObjectFail(muon);
            setObjectPass(jet);
          }
          // Toss jet
          else{
            setObjectFail(jet);
            setObjectPass(muon);
            break;
          }
        } // objects overlap
      } // muon loop
    } // is surviving jet
  } // jet loop
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping electrons and muons
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removeEleMuonOverlap
(const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer* muons)
{
  // Loop over electrons
  for(const auto electron : *electrons){
    if(isSurvivingObject(electron)){
      int elePass = 1;
      const xAOD::TrackParticle* elTrk = electron->trackParticle();
      // Loop over muons
      for(const auto muon : *muons){
        const xAOD::TrackParticle* muTrk =
          muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        // Discard electron if they share an ID track
        if(isSurvivingObject(muon) && (elTrk == muTrk)){
          elePass = 0;
          break;
        }
      }
      setOutputDecoration(electron, elePass);
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping hadronic taus and jets
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removeTauJetOverlap(const xAOD::TauJetContainer* taus,
                                                   const xAOD::JetContainer* jets)
{
  // Loop over jets
  for(const auto jet : *jets){
    // Check that this jet passes the input selection
    if(isSurvivingObject(jet)){
      // TODO: migrate to generic overlap method here?
      int jetPass = 1;
      // Loop over taus
      for(const auto tau : *taus){
        // Check for overlap
        if(isSurvivingObject(tau) && objectsOverlap(tau, jet, 0.2)){
          jetPass = 0;
          break;
        }
      }
      setOutputDecoration(jet, jetPass);
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping hadronic taus and electrons
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removeTauEleOverlap
(const xAOD::TauJetContainer* taus, const xAOD::ElectronContainer* electrons)
{
  // Remove tau if overlaps with a VeryLooseLLH electron in dR < 0.2
  for(const auto tau : *taus){
    if(isSurvivingObject(tau)){
      int tauPass = 1;
      for(const auto electron : *electrons){
        if(isSurvivingObject(electron)){
          // TODO: use faster method. This is slow.
          // TODO: check ID string
          bool passID = false;
          electron->passSelection(passID, "VeryLooseLH");
          if(passID && objectsOverlap(tau, electron, 0.2)){
            tauPass = 0;
            break;
          } // electron overlaps
        } // is surviving electron
      } // electron loop
      setOutputDecoration(tau, tauPass);
    } // is surviving tau
  } // tau loop
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping hadronic taus and muons
// This function loop could be combined with the electron one above for speed.
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removeTauMuonOverlap
(const xAOD::TauJetContainer* taus, const xAOD::MuonContainer* muons)
{
  // Remove tau if overlaps with a muon in dR < 0.2
  for(const auto tau : *taus){
    if(isSurvivingObject(tau)){
      // TODO: migrate to generic overlap method here?
      int tauPass = 1;
      for(const auto muon : *muons){
        // No specific criteria on this muon?
        if(isSurvivingObject(muon) && objectsOverlap(tau, muon, 0.2)){
          tauPass = 0;
          break;
        } // muon overlaps
      } // muon loop
      setOutputDecoration(tau, tauPass);
    } // is surviving tau
  } // tau loop
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping photons and electrons
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removePhotonEleOverlap
(const xAOD::PhotonContainer* photons, const xAOD::ElectronContainer* electrons)
{
  for(const auto photon : *photons){
    if(isSurvivingObject(photon)){
      // This generic template method makes the code concise,
      // but is it now overly complicated? Need to decide.
      if(objectOverlaps<xAOD::ElectronContainer>(photon, electrons, 0.4))
        setObjectFail(photon);
      else setObjectPass(photon);
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping photons and muons
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removePhotonMuonOverlap
(const xAOD::PhotonContainer* photons, const xAOD::MuonContainer* muons)
{
  for(const auto photon : *photons){
    if(isSurvivingObject(photon)){
      // This generic template method makes the code concise,
      // but is it now overly complicated? Need to decide.
      if(objectOverlaps<xAOD::MuonContainer>(photon, muons, 0.4))
        setObjectFail(photon);
      else setObjectPass(photon);
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping photons and electrons
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removePhotonPhotonOverlap
(const xAOD::PhotonContainer* photons)
{
  for(const auto photon : *photons){
    if(isSurvivingObject(photon)){
      // This generic template method makes the code concise,
      // but is it now overly complicated? Need to decide.
      // TODO: what is the correct overlap cone here?
      if(objectOverlaps<xAOD::PhotonContainer>(photon, photons, 0.4))
        setObjectFail(photon);
      else setObjectPass(photon);
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Remove overlapping photons and jets
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::removePhotonJetOverlap
(const xAOD::PhotonContainer* photons, const xAOD::JetContainer* jets)
{
  for(const auto jet : *jets){
    if(isSurvivingObject(jet)){
      // This generic template method makes the code concise,
      // but is it now overly complicated? Need to decide.
      if(objectOverlaps<xAOD::PhotonContainer>(jet, photons, 0.4))
        setObjectFail(jet);
      else setObjectPass(jet);
    }
  }
  return StatusCode::SUCCESS;
}

//-----------------------------------------------------------------------------
// Check if two objects overlap in a dR window
//-----------------------------------------------------------------------------
bool OverlapRemovalTool::objectsOverlap(const xAOD::IParticle* p1,
                                        const xAOD::IParticle* p2,
                                        double dRMax, double dRMin)
{
  double dR2 = deltaR2(p1, p2);
  // TODO: use fpcompare utilities
  return (dR2 < (dRMax*dRMax) && dR2 < (dRMin*dRMin));
}

//-----------------------------------------------------------------------------
// Calculate delta R between two particles
//-----------------------------------------------------------------------------
double OverlapRemovalTool::deltaR2(const xAOD::IParticle* p1,
                                   const xAOD::IParticle* p2)
{
  double dY = p1->rapidity() - p2->rapidity();
  double dPhi = TVector2::Phi_mpi_pi(p1->phi() - p2->phi());
  return dY*dY + dPhi*dPhi;
}
double OverlapRemovalTool::deltaR(const xAOD::IParticle* p1,
                                  const xAOD::IParticle* p2)
{ return sqrt(deltaR2(p1, p2)); }

//-----------------------------------------------------------------------------
// Determine if object is currently OK for input to OR
//-----------------------------------------------------------------------------
bool OverlapRemovalTool::isInputObject(const xAOD::IParticle* obj)
{
  // Input label is turned off if empty string
  if(m_inputLabel.empty()) return true;
  static SG::AuxElement::ConstAccessor<int> inputAcc(m_inputLabel);
  return inputAcc(*obj);
}

//-----------------------------------------------------------------------------
// Determine if object is currently rejected by OR.
// Return false if object hasn't been seen yet;
// i.e., the decoration hasn't been set.
//-----------------------------------------------------------------------------
bool OverlapRemovalTool::isRejectedObject(const xAOD::IParticle* obj)
{
  static SG::AuxElement::ConstAccessor<int> passAcc(m_outputLabel);
  if(passAcc.isAvailable(*obj) && passAcc(*obj) == 0){
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
// Set output decoration on object
//-----------------------------------------------------------------------------
void OverlapRemovalTool::setOutputDecoration(const xAOD::IParticle* obj,
                                             int pass)
{
  static SG::AuxElement::Decorator<int> passAcc(m_outputLabel);
  passAcc(*obj) = pass;
}
