// -*- C++ -*-
//
// Package:     SiPixelPhase1TrackResiduals
// Class:       SiPixelPhase1TrackResiduals
//

// Original Author: Marcel Schneider

#include "DQM/SiPixelPhase1TrackResiduals/interface/SiPixelPhase1TrackResiduals.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"
#include "Geometry/CommonTopologies/interface/PixelTopology.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"

SiPixelPhase1TrackResiduals::SiPixelPhase1TrackResiduals(const edm::ParameterSet& iConfig) :
  SiPixelPhase1Base(iConfig),
  validator(iConfig, consumesCollector())
{
 
  applyVertexCut_=iConfig.getUntrackedParameter<bool>("VertexCut",true);

  offlinePrimaryVerticesToken_= consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"));
			       
}

void SiPixelPhase1TrackResiduals::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if( !checktrigger(iEvent,iSetup,DCS) ) return;

  edm::ESHandle<TrackerGeometry> tracker;
  iSetup.get<TrackerDigiGeometryRecord>().get(tracker);
  assert(tracker.isValid());

  edm::Handle<reco::VertexCollection> vertices;
  if(applyVertexCut_) {
    iEvent.getByToken(offlinePrimaryVerticesToken_, vertices);
    if (!vertices.isValid() || vertices->empty()) return;
  }

  std::vector<TrackerValidationVariables::AVTrackStruct> vtracks;

  validator.fillTrackQuantities(iEvent, iSetup, 
    // tell the validator to only look at good tracks
    [&](const reco::Track& track) -> bool { 
	return (!applyVertexCut_ || (track.pt() > 0.75
				     && std::abs( track.dxy(vertices->at(0).position()) ) < 5*track.dxyError())) ;
    }, vtracks);

  for (auto& track : vtracks) {
    for (auto& it : track.hits) {
      auto id = DetId(it.rawDetId);
      auto isPixel = id.subdetId() == 1 || id.subdetId() == 2;
      if (!isPixel) continue; 
      
      histo[RESIDUAL_X].fill(it.resXprime, id, &iEvent);
      histo[RESIDUAL_Y].fill(it.resYprime, id, &iEvent);
    }
  }

}

DEFINE_FWK_MODULE(SiPixelPhase1TrackResiduals);

