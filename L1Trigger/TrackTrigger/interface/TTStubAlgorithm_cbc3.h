/*! \class   TTStubAlgorithm_cbc3
 *  \brief   Class for "cbc3" algorithm to be used
 *           in TTStubBuilder
 *  \details HW emulation.
 *
 *  \author Ivan Reid
 *  \date   2013, Oct 16
 *
 */

#ifndef L1_TRACK_TRIGGER_STUB_ALGO_CBC3_H
#define L1_TRACK_TRIGGER_STUB_ALGO_CBC3_H

#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/Framework/interface/ESProducer.h"

#include "L1Trigger/TrackTrigger/interface/TTStubAlgorithm.h"
#include "L1Trigger/TrackTrigger/interface/TTStubAlgorithmRecord.h"

#include "DataFormats/GeometryCommonDetAlgo/interface/MeasurementPoint.h"
#include "Geometry/CommonTopologies/interface/Topology.h"

#include <memory>
#include <string>
#include <map>
#include <typeinfo>

template< typename T >
class TTStubAlgorithm_cbc3 : public TTStubAlgorithm< T >
{
  private:
    /// Data members
    bool        mPerformZMatching2S;
    std::string className_;

  public:
    /// Constructor
    TTStubAlgorithm_cbc3( const TrackerGeometry* const theTrackerGeom, const TrackerTopology* const theTrackerTopo, bool aPerformZMatching2S ): 
                     TTStubAlgorithm< T >( theTrackerGeom, theTrackerTopo, __func__ )
    {
      mPerformZMatching2S = aPerformZMatching2S;
    }

    /// Destructor
    ~TTStubAlgorithm_cbc3() override{}

    /// Matching operations
    void PatternHitCorrelation( bool &aConfirmation,
                                int &aDisplacement,
                                int &anOffset,
				float &anROffset,
				float &anHardBend,
                                const TTStub< T > &aTTStub ) const override;

}; /// Close class

/*! \brief   Implementation of methods
 *  \details Here, in the header file, the methods which do not depend
 *           on the specific type <T> that can fit the template.
 *           Other methods, with type-specific features, are implemented
 *           in the source file.
 */

/// Matching operations
template< >
void TTStubAlgorithm_cbc3< Ref_Phase2TrackerDigi_ >::PatternHitCorrelation( bool &aConfirmation,
                                                                    int &aDisplacement,
                                                                    int &anOffset,
								    float &anROffset,
								    float &anHardBend,
                                                                    const TTStub< Ref_Phase2TrackerDigi_ > &aTTStub ) const;

/*! \class   ES_TTStubAlgorithm_cbc3
 *  \brief   Class to declare the algorithm to the framework
 *
 *  \author Nicola Pozzobon
 *  \date   2013, Jul 18
 *
 */

template< typename T >
class ES_TTStubAlgorithm_cbc3 : public edm::ESProducer
{
  private:
    /// Data members
    std::shared_ptr< TTStubAlgorithm< T > > _theAlgo;

    /// Z-matching
    bool  mPerformZMatching2S;

  public:
    /// Constructor
    ES_TTStubAlgorithm_cbc3( const edm::ParameterSet & p )
    {
      mPerformZMatching2S =  p.getParameter< bool >("zMatching2S");
      setWhatProduced( this );
    }

    /// Destructor
    ~ES_TTStubAlgorithm_cbc3() override{}

    /// Implement the producer
    std::shared_ptr< TTStubAlgorithm< T > > produce( const TTStubAlgorithmRecord & record )
    { 
      edm::ESHandle< TrackerGeometry > tGeomHandle;
      record.getRecord< TrackerDigiGeometryRecord >().get( tGeomHandle );
      const TrackerGeometry* const theTrackerGeom = tGeomHandle.product();
      edm::ESHandle<TrackerTopology> tTopoHandle;
      record.getRecord<TrackerTopologyRcd>().get(tTopoHandle);
      const TrackerTopology* const theTrackerTopo = tTopoHandle.product();

      TTStubAlgorithm< T >* TTStubAlgo = new TTStubAlgorithm_cbc3< T >( theTrackerGeom, theTrackerTopo, mPerformZMatching2S );
      _theAlgo = std::shared_ptr< TTStubAlgorithm< T > >( TTStubAlgo );
      return _theAlgo;
    } 

};

#endif

