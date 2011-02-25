// $Id: DQMHttpSource.h,v 1.9.16.1 2011/02/22 11:28:41 mommsen Exp $
/// @file: DQMHttpSource.h

#ifndef StorageManager_DQMHttpSource_h
#define StorageManager_DQMHttpSource_h

#include "DQMServices/Core/interface/DQMStore.h"
#include "EventFilter/StorageManager/interface/DQMEventConsumerRegistrationInfo.h"
#include "EventFilter/StorageManager/interface/EventServerProxy.h"
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
    $Revision: 1.9.16.1 $
    $Date: 2011/02/22 11:28:41 $
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
    stor::EventServerProxy<stor::DQMEventConsumerRegistrationInfo> _dqmEventServerProxy;

  };

} // namespace edm

#endif // StorageManager_DQMHttpSource_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
