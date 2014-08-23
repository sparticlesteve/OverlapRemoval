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

    /// Top-level method for performing full overlap-removal
    /// It might be best to specify input containers by TStore key,
    /// but it's not clear how that will work for systematic variations.
    /// Consider this explicit approach a placeholder, for now.
    virtual void removeOverlaps(const xAOD::ElectronContainer* electrons,
                                const xAOD::MuonContainer* muons,
                                const xAOD::JetContainer* jets,
                                const xAOD::TauJetContainer* taus = 0,
                                const xAOD::PhotonContainer* photons = 0) = 0;

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

    /// Remove overlapping photons and electrons
    virtual void removePhotonEleOverlap(const xAOD::PhotonContainer* photons,
                                        const xAOD::ElectronContainer* electrons) = 0;

    /// Remove overlapping photons and muons
    virtual void removePhotonMuonOverlap(const xAOD::PhotonContainer* photons,
                                         const xAOD::MuonContainer* muons) = 0;

    /// Remove overlapping photons
    virtual void removePhotonPhotonOverlap(const xAOD::PhotonContainer* photons) = 0;

    /// Remove overlapping photons and jets
    virtual void removePhotonJetOverlap(const xAOD::PhotonContainer* photons,
                                        const xAOD::JetContainer* jets) = 0;

}; // class IOverlapRemovalTool

#endif
