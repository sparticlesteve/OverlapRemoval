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
  declareProperty("OutputLabel", m_outputLabel = "passOR");
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
  // TODO: remove jets that overlap with electrons in dR < 0.2
  // TODO: remove electrons that overlap with surviving jets in dR < 0.4
}

//-----------------------------------------------------------------------------
// Remove overlapping muons and jets
//-----------------------------------------------------------------------------
void OverlapRemovalTool::removeMuonJetOverlap
(const xAOD::MuonContainer* muons, const xAOD::JetContainer* jets)
{
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
void OverlapRemovalTool::setOutputDecoration(xAOD::IParticle* obj, int pass)
{
  static SG::AuxElement::Accessor<int> passAcc(m_outputLabel);
  passAcc(*obj) = pass;
}
