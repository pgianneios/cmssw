//*****************************************************************************
// File:      EgammaHcalIsolation.cc
// ----------------------------------------------------------------------------
// OrigAuth:  Matthias Mozer
// Institute: IIHE-VUB
//=============================================================================
//*****************************************************************************
//ROOT includes
#include <Math/VectorUtil.h>

//CMSSW includes
#include "RecoEgamma/EgammaIsolationAlgos/interface/EgammaHcalIsolation.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "Geometry/CommonDetUnit/interface/TrackingGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "DataFormats/RecoCandidate/interface/RecoCandidate.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/Math/interface/deltaR.h"

double scaleToE(const double& eta) { return 1.; }
double scaleToEt(const double& eta) { return std::sin(2. * std::atan(std::exp(-eta))); }

EgammaHcalIsolation::EgammaHcalIsolation(InclusionRule extIncRule,
                                         double extRadius,
                                         InclusionRule intIncRule,
                                         double intRadius,
                                         const std::array<double, 4> &eThresHB,
                                         const std::array<double, 4> &etThresHB,
                                         int maxSeverityHB,
                                         const std::array<double, 7> &eThresHE,
                                         const std::array<double, 7> &etThresHE,
                                         int maxSeverityHE,
                                         const std::array<double, 7> &eThresHF,
                                         const std::array<double, 7> &etThresHF,
                                         int maxSeverityHF,
                                         const HBHERecHitCollection& mhbhe,
                                         const HFRecHitCollection& mhf,
                                         edm::ESHandle<CaloGeometry> caloGeometry,
                                         edm::ESHandle<HcalTopology> hcalTopology,
                                         edm::ESHandle<HcalChannelQuality> hcalChStatus,
                                         edm::ESHandle<HcalSeverityLevelComputer> hcalSevLvlComputer,
                                         edm::ESHandle<CaloTowerConstituentsMap> towerMap) : extIncRule_(extIncRule),
                                                                                             extRadius_(extRadius * extRadius),
                                                                                             intIncRule_(intIncRule),
                                                                                             intRadius_(intRadius * intRadius),
                                                                                             maxSeverityHB_(maxSeverityHB),
                                                                                             maxSeverityHE_(maxSeverityHE),
                                                                                             maxSeverityHF_(maxSeverityHF),
                                                                                             mhbhe_(mhbhe),
                                                                                             mhf_(mhf),
                                                                                             caloGeometry_(*caloGeometry.product()),
                                                                                             hcalTopology_(*hcalTopology.product()),
                                                                                             hcalChStatus_(*hcalChStatus.product()),
                                                                                             hcalSevLvlComputer_(*hcalSevLvlComputer.product()),
                                                                                             towerMap_(*towerMap.product())
{
  eThresHB_ = eThresHB;
  etThresHB_ = etThresHB;
  eThresHE_ = eThresHE;
  etThresHE_ = etThresHE;
  eThresHF_ = eThresHF;
  etThresHF_ = etThresHF;

  // make some adjustments for the BC rules
  if (extIncRule_ == InclusionRule::isBehindClusterSeed and intIncRule_ == InclusionRule::withinConeAroundCluster) {
    extRadius_ = 0.;
    intRadius_ = 0.;
  }
  else if (extIncRule_ == InclusionRule::withinConeAroundCluster and intIncRule_ == InclusionRule::isBehindClusterSeed) {
    intRadius_ = 0.;
  }
  else if (extIncRule_ == InclusionRule::isBehindClusterSeed and intIncRule_ == InclusionRule::isBehindClusterSeed) {
    edm::LogWarning("EgammaHcalIsolation") << " external and internal rechit inclusion rules can't both be isBehindClusterSeed."
                                           << " Setting both to withinConeAroundCluster!";
    extIncRule_ = InclusionRule::withinConeAroundCluster;
    intIncRule_ = InclusionRule::withinConeAroundCluster;
  }
}

EgammaHcalIsolation::EgammaHcalIsolation(InclusionRule extIncRule,
                                         double extRadius,
                                         InclusionRule intIncRule,
                                         double intRadius,
                                         const std::array<double, 4> &eThresHB,
                                         const std::array<double, 4> &etThresHB,
                                         int maxSeverityHB,
                                         const std::array<double, 7> &eThresHE,
                                         const std::array<double, 7> &etThresHE,
                                         int maxSeverityHE,
                                         const std::array<double, 7> &eThresHF,
                                         const std::array<double, 7> &etThresHF,
                                         int maxSeverityHF,
                                         const HBHERecHitCollection& mhbhe,
                                         const HFRecHitCollection& mhf,
                                         const CaloGeometry &caloGeometry,
                                         const HcalTopology &hcalTopology,
                                         const HcalChannelQuality &hcalChStatus,
                                         const HcalSeverityLevelComputer &hcalSevLvlComputer,
                                         const CaloTowerConstituentsMap &towerMap) : extIncRule_(extIncRule),
                                                                                     extRadius_(extRadius * extRadius),
                                                                                     intIncRule_(intIncRule),
                                                                                     intRadius_(intRadius * intRadius),
                                                                                     maxSeverityHB_(maxSeverityHB),
                                                                                     maxSeverityHE_(maxSeverityHE),
                                                                                     maxSeverityHF_(maxSeverityHF),
                                                                                     mhbhe_(mhbhe),
                                                                                     mhf_(mhf),
                                                                                     caloGeometry_(caloGeometry),
                                                                                     hcalTopology_(hcalTopology),
                                                                                     hcalChStatus_(hcalChStatus),
                                                                                     hcalSevLvlComputer_(hcalSevLvlComputer),
                                                                                     towerMap_(towerMap)
{
  eThresHB_ = eThresHB;
  etThresHB_ = etThresHB;
  eThresHE_ = eThresHE;
  etThresHE_ = etThresHE;
  eThresHF_ = eThresHF;
  etThresHF_ = etThresHF;

  // make some adjustments for the BC rules
  if (extIncRule_ == InclusionRule::isBehindClusterSeed and intIncRule_ == InclusionRule::withinConeAroundCluster) {
    extRadius_ = 0.;
    intRadius_ = 0.;
  }
  else if (extIncRule_ == InclusionRule::withinConeAroundCluster and intIncRule_ == InclusionRule::isBehindClusterSeed) {
    intRadius_ = 0.;
  }
  else if (extIncRule_ == InclusionRule::isBehindClusterSeed and intIncRule_ == InclusionRule::isBehindClusterSeed) {
    edm::LogWarning("EgammaHcalIsolation") << " external and internal rechit inclusion rules can't both be isBehindClusterSeed."
                                           << " Setting both to withinConeAroundCluster!";
    extIncRule_ = InclusionRule::withinConeAroundCluster;
    intIncRule_ = InclusionRule::withinConeAroundCluster;
  }
}

template <typename HcalRecHit>
double EgammaHcalIsolation::goodHitEnergy(const GlobalPoint &pclu, const HcalRecHit &hit, int depth, int ieta, int iphi, int include_or_exclude,
                                          double (*scale)(const double&)) const {
  const auto phit = caloGeometry_.getPosition(hit.detid());

  if ((extIncRule_ == InclusionRule::withinConeAroundCluster and deltaR2(pclu, phit) > extRadius_) or
      (intIncRule_ == InclusionRule::withinConeAroundCluster and deltaR2(pclu, phit) < intRadius_))
    return 0.;

  const HcalDetId hid(hit.detid());
  const int hd = (hid.subdet() != HcalForward) ? hid.depth() : hid.hfdepth(), he = hid.ieta(), hp = hid.iphi();
  const int h1 = hd - 1;

  if ((hid.subdet() == HcalBarrel and (hd < 1 or hd > 4)) or
      (hid.subdet() == HcalEndcap and (hd < 1 or hd > 7)) or
      (hid.subdet() == HcalForward and (hd < 1 or hd > 2)))
    std::cout << "hit in subdet " << hid.subdet() << " has unaccounted-for depth of " << hd << "!!" << std::endl;

  //if (hid.subdet() == HcalForward)
  //  std::cout << "HF hit has a depth of " << hd << "!!" << std::endl; // depth is 1 or 2

  if (include_or_exclude == -1 and (he != ieta or hp != iphi))
    return 0.;

  if (include_or_exclude == 1 and (he == ieta and hp == iphi))
    return 0.;

  const bool right_depth = (depth == 0 or hd == depth);
  if (!right_depth)
    return 0.;

  DetId did = hcalTopology_.idFront(hid);
  const uint32_t flag = hit.flags();
  const uint32_t dbflag = hcalChStatus_.getValues(did)->getValue();
  int severity = hcalSevLvlComputer_.getSeverityLevel(did, flag, dbflag);
  bool recovered = hcalSevLvlComputer_.recoveredRecHit(did, flag);

  const double het = hit.energy() * scaleToEt(phit.eta());
  const bool goodHB = hid.subdet() == HcalBarrel  and (severity <= maxSeverityHB_ or recovered) and hit.energy() > eThresHB_[h1] and het > etThresHB_[h1];
  const bool goodHE = hid.subdet() == HcalEndcap  and (severity <= maxSeverityHE_ or recovered) and hit.energy() > eThresHE_[h1] and het > etThresHE_[h1];
  const bool goodHF = hid.subdet() == HcalForward and (severity <= maxSeverityHF_ or recovered) and hit.energy() > eThresHF_[h1] and het > etThresHF_[h1];

  if (goodHB or goodHE or goodHF)
    return hit.energy() * scale(phit.eta());

  return 0.;
}

double EgammaHcalIsolation::getHcalSum(const GlobalPoint &pclu,
                                       int depth, int ieta, int iphi, int include_or_exclude,
                                       double (*scale)(const double&)) const {
  double sum = 0.;
  for (const auto &hit : mhbhe_)
    sum += goodHitEnergy(pclu, hit, depth, ieta, iphi, include_or_exclude, scale);

  for (const auto &hit : mhf_)
    sum += goodHitEnergy(pclu, hit, depth, ieta, iphi, include_or_exclude, scale);

  return sum;
}
