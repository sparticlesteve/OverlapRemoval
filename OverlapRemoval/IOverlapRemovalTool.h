#ifndef OVERLAPREMOVAL_IOVERLAPREMOVALTOOL_H
#define OVERLAPREMOVAL_IOVERLAPREMOVALTOOL_H

// Framework includes
#include "AsgTools/IAsgTool.h"

// Put the tool in a namespace?

/// Interface for the overlap removal tool
///
/// @author Steve Farrell <steven.farrell@cern.ch>
///
class IOverlapRemovalTool : public virtual asg::IAsgTool
{

    /// Declare the interface
    ASG_TOOL_INTERFACE(IOverlapRemovalTool)

  public:

    /// Remove overlapping electrons and jets.
    /// This method will decorate both the electrons and jets according to
    /// both the e-jet and jet-e overlap removal prescriptions
    virtual void removeEleJetOverlap(const xAOD::ElectronContainer* electrons,
                                     const xAOD::JetContainer* jets) = 0;

    /// Remove overlapping muons and jets
    virtual void removeMuonJetOverlap(const xAOD::MuonContainer* muons,
                                      const xAOD::JetContainer* jets) = 0;

    /// Remove overlapping electrons and muons
    virtual void removeEleMuonOverlap(const xAOD::ElectronContainer* electrons,
                                      const xAOD::MuonContainer* muons) = 0;

    /// Remove jets overlapping with taus
    virtual void removeTauJetOverlap(const xAOD::TauJetContainer* taus,
                                     const xAOD::JetContainer* jets) = 0;

    /// Remove overlapping taus and electrons
    virtual void removeTauEleOverlap(const xAOD::TauJetContainer* taus,
                                     const xAOD::ElectronContainer* electrons) = 0;

    /// Remove overlapping taus and muons
    virtual void removeTauMuonOverlap(const xAOD::TauJetContainer* taus,
                                      const xAOD::MuonContainer* muons) = 0;

}; // class IOverlapRemovalTool

#endif
