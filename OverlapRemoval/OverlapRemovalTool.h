#ifndef OVERLAPREMOVAL_OVERLAPREMOVALTOOL_H
#define OVERLAPREMOVAL_OVERLAPREMOVALTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"

// EDM includes
#include "xAODBase/IParticle.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"

// Local includes
#include "OverlapRemoval/IOverlapRemovalTool.h"

// Put the tool in a namespace?

/// Overlap removal tool
///
/// This tool implements the harmonized object overlap removal
/// recommendations from the harmonization study group 5, given in
/// https://cds.cern.ch/record/1700874
///
/// @author Steve Farrell <steven.farrell@cern.ch>
///
class OverlapRemovalTool : public virtual IOverlapRemovalTool,
                           public asg::AsgTool
{

    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS(OverlapRemovalTool, IOverlapRemovalTool)

  public:

    /// Constructor for standalone usage
    OverlapRemovalTool(const std::string& name);

    /// @name Methods implementing the asg::IAsgTool interface
    /// @{

    /// Initialize the tool
    virtual StatusCode initialize();

    /// @}

    /// @name Methods implementing the IOverlapRemovalTool interface
    /// @{

    /// Remove overlapping electrons and jets
    /// This method will decorate both the electrons and jets according to
    /// both the e-jet and jet-e overlap removal prescriptions.
    virtual void removeEleJetOverlap(const xAOD::ElectronContainer* electrons,
                                     const xAOD::JetContainer* jets);

    /// Remove overlapping muons and jets
    virtual void removeMuonJetOverlap(const xAOD::MuonContainer* muons,
                                      const xAOD::JetContainer* jets);

    /// Remove overlapping electrons and muons
    /// TODO: make it possible to veto event based on this. Maybe the
    /// return value should just be a bool.
    virtual void removeEleMuonOverlap(const xAOD::ElectronContainer* electrons,
                                      const xAOD::MuonContainer* muons);

    /// TODO: the other ORs

    /// @}

  protected:

    /// Determine if objects overlap by a simple dR comparison
    bool objectsOverlap(const xAOD::IParticle* p1, const xAOD::IParticle* p2,
                        double dRMax, double dRMin = -1.);

    /// Recommended calculation of overlap distance parameter, delta R.
    /// delta R = sqrt((y1-y2)^2 + (phi1-phi2)^2).
    /// This is in contrast to what is returned by TLorentzVector::DeltaR,
    /// which uses the pseudorapidity eta rather than the rapidity y.
    double deltaR(const xAOD::IParticle* p1, const xAOD::IParticle* p2);

    /// Check if object is flagged as input for OR
    bool isInputObject(const xAOD::IParticle* obj);

    /// Check if object has been rejected by decoration
    bool isRejectedObject(const xAOD::IParticle* obj);

    /// Set output decoration on object, pass or fail
    void setOutputDecoration(const xAOD::IParticle* obj, int pass);

  private:

    /// TODO: add configurable properties for
    /// - OR parameters (dR, etc.)
    /// - OR scheme choices

    /// Input object decoration which specifies which objects to look at
    std::string m_inputLabel;
    /// Output object decoration which specifies passing objects
    std::string m_outputLabel;

}; // class OverlapRemovalTool

#endif
