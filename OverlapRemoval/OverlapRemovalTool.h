#ifndef OVERLAPREMOVAL_OVERLAPREMOVALTOOL_H
#define OVERLAPREMOVAL_OVERLAPREMOVALTOOL_H

// Framework includes
#include "AsgTools/AsgTool.h"

// EDM includes
#include "xAODBase/IParticle.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"

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

    // TODO: add better documentation about the loose ele/mu requirements.

    /// Top-level method for performing full overlap-removal.
    /// The individual OR methods will be called in the recommended order,
    /// and the considered objects will be decorated with the output result.
    /// Use this method form when the electron and muon containers are
    /// sufficiently loose for the tau-lep overlap removal.
    virtual StatusCode removeOverlaps(const xAOD::ElectronContainer* electrons,
                                      const xAOD::MuonContainer* muons,
                                      const xAOD::JetContainer* jets,
                                      const xAOD::TauJetContainer* taus = 0,
                                      const xAOD::PhotonContainer* photons = 0);

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
                                      const xAOD::PhotonContainer* photons = 0);

    /// Remove overlapping electrons and jets
    /// This method will decorate both the electrons and jets according to
    /// both the e-jet and jet-e overlap removal prescriptions.
    virtual StatusCode removeEleJetOverlap(const xAOD::ElectronContainer* electrons,
                                           const xAOD::JetContainer* jets);

    /// Remove overlapping muons and jets
    virtual StatusCode removeMuonJetOverlap(const xAOD::MuonContainer* muons,
                                            const xAOD::JetContainer* jets);

    /// Remove overlapping electrons and muons
    /// TODO: make it possible to veto event based on this.
    /// Maybe the return value should just be a bool.
    virtual StatusCode removeEleMuonOverlap(const xAOD::ElectronContainer* electrons,
                                            const xAOD::MuonContainer* muons);

    /// Remove jets overlapping with taus
    virtual StatusCode removeTauJetOverlap(const xAOD::TauJetContainer* taus,
                                           const xAOD::JetContainer* jets);

    /// Remove overlapping taus and electrons
    virtual StatusCode removeTauEleOverlap(const xAOD::TauJetContainer* taus,
                                           const xAOD::ElectronContainer* electrons);

    /// Remove overlapping taus and muons
    virtual StatusCode removeTauMuonOverlap(const xAOD::TauJetContainer* taus,
                                            const xAOD::MuonContainer* muons);

    /// Remove overlapping photons and electrons
    virtual StatusCode removePhotonEleOverlap(const xAOD::PhotonContainer* photons,
                                              const xAOD::ElectronContainer* electrons);

    /// Remove overlapping photons and muons
    virtual StatusCode removePhotonMuonOverlap(const xAOD::PhotonContainer* photons,
                                               const xAOD::MuonContainer* muons);

    /// Remove overlapping photons
    virtual StatusCode removePhotonPhotonOverlap(const xAOD::PhotonContainer* photons);

    /// Remove overlapping photons and jets
    virtual StatusCode removePhotonJetOverlap(const xAOD::PhotonContainer* photons,
                                              const xAOD::JetContainer* jets);

    /// TODO: add the high-level overlap removal logic

    /// @}

  protected:

    /// Generic dR-based overlap check between one object and a container.
    /// NOTE: this doesn't have any check that the objects are equal.
    /// TODO: decide if generic overlap function is worth it.
    /// TODO: can I just use DataVector inheritance here???
    template<typename ContainerType> bool objectOverlaps
    (const xAOD::IParticle* obj, const ContainerType* container, double dR)
    {
      for(const auto contObj : *container){
        if(isSurvivingObject(contObj)){
          // Make sure these are not the same object
          if(obj == contObj) continue;
          if(objectsOverlap(obj, contObj, dR)) return true;
        }
      }
      return false;
    }

    /// Determine if objects overlap by a simple dR comparison
    bool objectsOverlap(const xAOD::IParticle* p1, const xAOD::IParticle* p2,
                        double dRMax, double dRMin = 0);

    /// Recommended calculation of overlap distance parameter, (delta R)^2.
    /// dR^2 = (y1-y2)^2 + (phi1-phi2)^2
    /// Note this is calculated with the rapidity rather than the
    /// pseudorapidity. TLorentzVector::DeltaR uses the latter.
    double deltaR2(const xAOD::IParticle* p1, const xAOD::IParticle* p2);
    /// deltaR = sqrt( deltaR2 )
    double deltaR(const xAOD::IParticle* p1, const xAOD::IParticle* p2);

    /// Check if object is flagged as input for OR
    bool isInputObject(const xAOD::IParticle* obj);

    /// Check if object has been rejected by decoration
    bool isRejectedObject(const xAOD::IParticle* obj);

    /// Check if object is surviving OR thus far
    bool isSurvivingObject(const xAOD::IParticle* obj)
    { return isInputObject(obj) && !isRejectedObject(obj); }

    /// Set output decoration on object, pass or fail
    void setOutputDecoration(const xAOD::IParticle* obj, int pass);

    /// Shorthand way to set an object as pass
    void setObjectPass(const xAOD::IParticle* obj)
    { setOutputDecoration(obj, 1); }

    /// Shorthand way to set an object as fail
    void setObjectFail(const xAOD::IParticle* obj)
    { setOutputDecoration(obj, 0); }

  private:

    /// TODO: add configurable properties for
    /// - all OR parameters (dR, etc.)
    /// - all OR scheme choices

    /// Input object decoration which specifies which objects to look at
    std::string m_inputLabel;
    /// Output object decoration which specifies passing objects
    std::string m_outputLabel;

    /// electron-jet overlap cone (removes electron)
    float m_electronJetDR;
    /// jet-electron overlap cone (removes jet)
    float m_jetElectronDR;
    /// muon-jet overlap cone
    float m_muonJetDR;
    /// tau-jet overlap cone
    float m_tauJetDR;
    /// tau-electron overlap cone
    float m_tauElectronDR;
    /// tau-muon overlap cone
    float m_tauMuonDR;
    /// photon-electron overlap cone
    float m_photonElectronDR;
    /// photon-muon overlap cone
    float m_photonMuonDR;
    /// photon-photon overlap cone
    float m_photonPhotonDR;
    /// photon-jet overlap cone
    float m_photonJetDR;

}; // class OverlapRemovalTool

#endif
