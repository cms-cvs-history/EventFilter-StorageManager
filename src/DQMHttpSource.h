// $Id: DQMHttpSource.h,v 1.22.8.4 2011/01/18 15:56:37 mommsen Exp $
/// @file: DQMHttpSource.h

#ifndef StorageManager_DQMHttpSource_h
#define StorageManager_DQMHttpSource_h

#include "DQMServices/Core/interface/DQMStore.h"
#include "EventFilter/StorageManager/interface/DQMEventServerProxy.h"
#include "FWCore/Framework/interface/InputSourceDescription.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Sources/interface/RawInputSource.h"
#include "IOPool/Streamer/interface/DQMEventMessage.h"

#include <memory>


namespace edm
{
  /**
    An input source for DQM consumers using cmsRun that connect to
    the StorageManager or SMProxyServer to get DQM (histogram) data.
    
    $Author: mommsen $
    $Revision: 1.22.8.4 $
    $Date: 2011/01/18 15:56:37 $
  */

  class DQMHttpSource : public edm::RawInputSource
  {
  public:
    DQMHttpSource
    (
      const edm::ParameterSet&, 
      const edm::InputSourceDescription&
    );
    virtual ~DQMHttpSource() {};

  private:
    virtual std::auto_ptr<edm::Event> readOneEvent();
    void addEventToDQMBackend(const DQMEventMsgView&);
    void initializeDQMStore();

    DQMStore *_dqmStore;
    stor::DQMEventServerProxy _dqmEventServerProxy;

  };

} // namespace edm

#endif // StorageManager_DQMHttpSource_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
