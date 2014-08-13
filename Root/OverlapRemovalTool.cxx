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
// Remove overlapping electrons and jets
//-----------------------------------------------------------------------------
void OverlapRemovalTool::removeEleJetOverlap
(const xAOD::ElectronContainer* electrons, const xAOD::JetContainer* jets)
{
  // Remove jets that overlap with electrons in dR < 0.2
  for(const auto jet : *jets){
    // Check that this jet passes the input selection
    if(isSurvivingObject(jet)){
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
  // Need two steps so as to avoid using rejected jets.
  // Maybe this should get its own method.
  for(const auto electron : *electrons){
    // Check that this electron passes the input selection
    if(isSurvivingObject(electron)){
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
}

//-----------------------------------------------------------------------------
// Remove overlapping muons and jets
//-----------------------------------------------------------------------------
void OverlapRemovalTool::removeMuonJetOverlap
(const xAOD::MuonContainer* muons, const xAOD::JetContainer* jets)
{
  // Accessor to jet.nTrack
  // TODO: figure out the correct aux key
  static SG::AuxElement::ConstAccessor<int> nTrkAcc("numTracks");

  // Loop over jets
  for(const auto jet : *jets){
    // Check that this jet is still OK
    if(isSurvivingObject(jet)){
      int nTrk = nTrkAcc(*jet);
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
}

//-----------------------------------------------------------------------------
// Remove overlapping electrons and muons
//-----------------------------------------------------------------------------
void OverlapRemovalTool::removeEleMuonOverlap
(const xAOD::ElectronContainer* electrons, const xAOD::MuonContainer* muons)
{
  // TODO: remove electrons that share a track with a muon
}

//-----------------------------------------------------------------------------
// Check if two objects overlap in a dR window
//-----------------------------------------------------------------------------
bool OverlapRemovalTool::objectsOverlap(const xAOD::IParticle* p1,
                                        const xAOD::IParticle* p2,
                                        double dRMax, double dRMin)
{
  double dR = deltaR(p1, p2);
  return (dR < dRMax && dR < dRMin);
}

//-----------------------------------------------------------------------------
// Calculate delta R between two particles
//-----------------------------------------------------------------------------
double OverlapRemovalTool::deltaR(const xAOD::IParticle* p1,
                                  const xAOD::IParticle* p2)
{
  double dY = p1->rapidity() - p2->rapidity();
  double dPhi = TVector2::Phi_mpi_pi(p1->phi() - p2->phi());
  return sqrt(dY*dY + dPhi*dPhi);
}

//-----------------------------------------------------------------------------
// Determine if object is currently OK for input to OR
//-----------------------------------------------------------------------------
bool OverlapRemovalTool::isInputObject(const xAOD::IParticle* obj)
{
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
