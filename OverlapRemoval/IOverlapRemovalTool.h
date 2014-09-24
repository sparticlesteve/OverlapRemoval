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

    /// Top-level method for performing full overlap-removal.
    /// The individual OR methods will be called in the recommended order,
    /// and the considered objects will be decorated with the output result.
    /// Use this method form when the electron and muon containers are
    /// sufficiently loose for the tau-lep overlap removal.
    virtual StatusCode removeOverlaps(const xAOD::ElectronContainer* electrons,
                                      const xAOD::MuonContainer* muons,
                                      const xAOD::JetContainer* jets,
                                      const xAOD::TauJetContainer* taus = 0,
                                      const xAOD::PhotonContainer* photons = 0) = 0;

    /// Top-level method for performing full overlap-removal.
    /// The individual OR methods will be called in the recommended order,
    /// and the considered objects will be decorated with the output result.
    /// Use this method form when you're using view-containers or subset
    /// containers in order to provide the loose electrons and muons for the
    /// tau-lep overlap removal.
    virtual StatusCode removeOverlaps(const xAOD::ElectronContainer* electrons,
                                      const xAOD::MuonContainer* muons,
                                      const xAOD::JetContainer* jets,
                                      const xAOD::TauJetContainer* taus,
                                      const xAOD::ElectronContainer* looseElectrons,
                                      const xAOD::MuonContainer* looseMuons,
                                      const xAOD::PhotonContainer* photons = 0) = 0;

    /// Remove overlapping electrons and jets.
    /// This method will decorate both the electrons and jets according to
    /// both the e-jet and jet-e overlap removal prescriptions
    virtual StatusCode removeEleJetOverlap(const xAOD::ElectronContainer* electrons,
                                           const xAOD::JetContainer* jets) = 0;

    /// Remove overlapping muons and jets
    virtual StatusCode removeMuonJetOverlap(const xAOD::MuonContainer* muons,
                                            const xAOD::JetContainer* jets) = 0;

    /// Remove overlapping electrons and muons
    virtual StatusCode removeEleMuonOverlap(const xAOD::ElectronContainer* electrons,
                                            const xAOD::MuonContainer* muons) = 0;

    /// Remove jets overlapping with taus
    virtual StatusCode removeTauJetOverlap(const xAOD::TauJetContainer* taus,
                                           const xAOD::JetContainer* jets) = 0;

    /// Remove overlapping taus and electrons
    virtual StatusCode removeTauEleOverlap(const xAOD::TauJetContainer* taus,
                                           const xAOD::ElectronContainer* electrons) = 0;

    /// Remove overlapping taus and muons
    virtual StatusCode removeTauMuonOverlap(const xAOD::TauJetContainer* taus,
                                            const xAOD::MuonContainer* muons) = 0;

    /// Remove overlapping photons and electrons
    virtual StatusCode removePhotonEleOverlap(const xAOD::PhotonContainer* photons,
                                              const xAOD::ElectronContainer* electrons) = 0;

    /// Remove overlapping photons and muons
    virtual StatusCode removePhotonMuonOverlap(const xAOD::PhotonContainer* photons,
                                               const xAOD::MuonContainer* muons) = 0;

    /// Remove overlapping photons
    virtual StatusCode removePhotonPhotonOverlap(const xAOD::PhotonContainer* photons) = 0;

    /// Remove overlapping photons and jets
    virtual StatusCode removePhotonJetOverlap(const xAOD::PhotonContainer* photons,
                                              const xAOD::JetContainer* jets) = 0;

}; // class IOverlapRemovalTool

#endif
