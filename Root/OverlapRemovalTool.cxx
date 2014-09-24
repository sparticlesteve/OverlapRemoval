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
  // input/output labels
  declareProperty("InputLabel", m_inputLabel = "selected");
  //declareProperty("OutputLabel", m_outputLabel = "passesOR");
  declareProperty("OverlapLabel", m_overlapLabel = "overlaps",
                  "Decoration given to objects that fail OR");
  // dR cones for defining overlap
  declareProperty("ElectronJetDRCone",    m_electronJetDR    = 0.2);
  declareProperty("JetElectronDRCone",    m_jetElectronDR    = 0.4);
  declareProperty("MuonJetDRCone",        m_muonJetDR        = 0.4);
  declareProperty("TauJetDRCone",         m_tauJetDR         = 0.2);
  declareProperty("TauElectronDRCone",    m_tauElectronDR    = 0.2);
  declareProperty("TauMuonDRCone",        m_tauMuonDR        = 0.2);
  declareProperty("PhotonElectronDRCone", m_photonElectronDR = 0.4);
  declareProperty("PhotonMuonDRCone",     m_photonMuonDR     = 0.4);
  declareProperty("PhotonPhotonDRCone",   m_photonPhotonDR   = 0.4);
  declareProperty("PhotonJetDRCone",      m_photonJetDR      = 0.4);
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
  return removeOverlaps(electrons, muons, jets, taus,
                        electrons, muons, photons);
}
//-----------------------------------------------------------------------------
StatusCode OverlapRemovalTool::
removeOverlaps(const xAOD::ElectronContainer* electrons,
               const xAOD::MuonContainer* muons,
               const xAOD::JetContainer* jets,
               const xAOD::TauJetContainer* taus,
               const xAOD::ElectronContainer* looseElectrons,
               const xAOD::MuonContainer* looseMuons,
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
    ATH_CHECK( removeTauEleOverlap(taus, looseElectrons) );
    ATH_CHECK( removeTauMuonOverlap(taus, looseMuons) );
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
      // Use the generic OR method
      if(objectOverlaps<xAOD::ElectronContainer>(jet, electrons, m_electronJetDR))
        setObjectFail(jet);
      else setObjectPass(jet);
    }
  }
  // Remove electrons that overlap with surviving jets in dR < 0.4.
  // Maybe this should get its own method.
  for(const auto electron : *electrons){
    // Check that this electron passes the input selection
    if(isSurvivingObject(electron)){
      // Use the generic OR method
      if(objectOverlaps<xAOD::JetContainer>(electron, jets, m_jetElectronDR))
        setObjectFail(electron);
      else setObjectPass(electron);
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
  // Is there any faster way to access this information?
  std::vector<int> nTrkVec;
  //static SG::AuxElement::ConstAccessor<int> nTrkAcc("NumTrkPt1000");

  // Loop over jets
  for(const auto jet : *jets){
    if(isSurvivingObject(jet)){
      //int nTrk = nTrkAcc(*jet);
      jet->getAttribute(xAOD::JetAttribute::NumTrkPt500, nTrkVec);
      int nTrk = nTrkVec[0];
      // Loop over muons
      for(const auto muon : *muons){
        // Check for overlap
        if(isSurvivingObject(muon)){
          if(objectsOverlap(jet, muon, m_muonJetDR)){
            bool tossMuon = nTrk > 2;
            setOverlapDecoration(muon, tossMuon);
            setOverlapDecoration(jet, !tossMuon);
            //setOutputDecoration(jet, keepJet);
            //setOutputDecoration(muon, !keepJet);
            // Move on to next jet if we're tossing it
            if(!tossMuon) break;
          } // objects overlap
          // muon passes
          setObjectPass(muon);
        } // is surviving muon
      } // muon loop
      // if still surviving, mark jet as pass
      if(isSurvivingObject(jet)) setObjectPass(jet);
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
      int eleOverlaps = 0;
      //int elePass = 1;
      const xAOD::TrackParticle* elTrk = electron->trackParticle();
      // Loop over muons
      for(const auto muon : *muons){
        const xAOD::TrackParticle* muTrk =
          muon->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
        // Discard electron if they share an ID track
        if(isSurvivingObject(muon) && (elTrk == muTrk)){
          eleOverlaps = 1;
          //elePass = 0;
          break;
        }
      }
      setOverlapDecoration(electron, eleOverlaps);
      //setOutputDecoration(electron, elePass);
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
      if(objectOverlaps<xAOD::TauJetContainer>(jet, taus, m_tauJetDR))
        setObjectFail(jet);
      else setObjectPass(jet);
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
      int tauOverlaps = 0;
      //int tauPass = 1;
      for(const auto electron : *electrons){
        if(isSurvivingObject(electron)){
          // TODO: use faster method. This is slow.
          // TODO: check ID string
          bool passID = false;
          electron->passSelection(passID, "VeryLooseLH");
          if(passID && objectsOverlap(tau, electron, m_tauElectronDR)){
            tauOverlaps = 1;
            //tauPass = 0;
            break;
          } // electron overlaps
        } // is surviving electron
      } // electron loop
      setOverlapDecoration(tau, tauOverlaps);
      //setOutputDecoration(tau, tauPass);
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
      int tauOverlaps = 0;
      //int tauPass = 1;
      for(const auto muon : *muons){
        // No specific criteria on this muon?
        // TODO: get clarification on this
        if(isSurvivingObject(muon) && objectsOverlap(tau, muon, m_tauMuonDR)){
          tauOverlaps = 1;
          //tauPass = 0;
          break;
        } // muon overlaps
      } // muon loop
      setOverlapDecoration(tau, tauOverlaps);
      //setOutputDecoration(tau, tauPass);
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
      if(objectOverlaps<xAOD::ElectronContainer>(photon, electrons, m_photonElectronDR))
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
      if(objectOverlaps<xAOD::MuonContainer>(photon, muons, m_photonMuonDR))
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
      if(objectOverlaps<xAOD::PhotonContainer>(photon, photons, m_photonPhotonDR))
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
      if(objectOverlaps<xAOD::PhotonContainer>(jet, photons, m_photonJetDR))
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
  return (dR2 < (dRMax*dRMax) && dR2 > (dRMin*dRMin));
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
  // Reversing the logic
  static SG::AuxElement::Decorator<int> overlapDec(m_overlapLabel);
  if(overlapDec(*obj) == 1) return true;
  //static SG::AuxElement::Accessor<int> overlapAcc(m_overlapLabel);
  //if(overlapAcc.isAvailable(*obj) && overlapAcc(*obj) == 1)
  //  return true;
  /*
  static SG::AuxElement::Accessor<int> passAcc(m_outputLabel);
  //static SG::AuxElement::Decorator<int> passAcc(m_outputLabel);
  if(passAcc.isAvailable(*obj)){
    std::cout << "decoration is available" << std::endl;
    if(passAcc(*obj) == 0){
      std::cout << "decoration is FAIL" << std::endl;
      return true;
    }
  }*/
  /*if(passAcc.isAvailable(*obj) && passAcc(*obj) == 0){
    return true;
  }*/
  return false;
}

//-----------------------------------------------------------------------------
// Set output decoration on object
//-----------------------------------------------------------------------------
/*void OverlapRemovalTool::setOutputDecoration(const xAOD::IParticle* obj,
                                             int pass)
{
  static SG::AuxElement::Decorator<int> passAcc(m_outputLabel);
  passAcc(*obj) = pass;
}*/
//-----------------------------------------------------------------------------
void OverlapRemovalTool::setOverlapDecoration(const xAOD::IParticle* obj,
                                              int overlaps)
{
  static SG::AuxElement::Decorator<int> overlapDec(m_overlapLabel);
  overlapDec(*obj) = overlaps;
}

