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
// Determine if object is currently surviving OR.
// Surviving means the object either hasn't been seen yet
// or the 'pass' decoration is set to 1
//-----------------------------------------------------------------------------
bool OverlapRemovalTool::isSurvivingObject(const xAOD::IParticle* obj)
{
  static SG::AuxElement::ConstAccessor<int> passAcc("passOR");
  if(!passAcc.isAvailable(*obj) || passAcc(*obj) == 1){
    return true;
  }
  return false;
}
