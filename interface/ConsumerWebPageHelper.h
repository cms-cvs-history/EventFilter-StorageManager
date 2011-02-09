// $Id: ConsumerWebPageHelper.h,v 1.1.2.3 2011/01/26 16:04:39 mommsen Exp $
/// @file: ConsumerWebPageHelper.h

#ifndef EventFilter_StorageManager_ConsumerWebPageHelper_h
#define EventFilter_StorageManager_ConsumerWebPageHelper_h

#include "xdaq/ApplicationDescriptor.h"
#include "xgi/Output.h"

#include "EventFilter/StorageManager/interface/DQMEventQueueCollection.h"
#include "EventFilter/StorageManager/interface/EventQueueCollection.h"
#include "EventFilter/StorageManager/interface/RegistrationCollection.h"
#include "EventFilter/StorageManager/interface/StatisticsReporter.h"
#include "EventFilter/StorageManager/interface/WebPageHelper.h"
#include "EventFilter/StorageManager/interface/XHTMLMaker.h"

#include <boost/function.hpp>

namespace stor
{

  /**
   * Helper class to handle consumer web page requests
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.3 $
   * $Date: 2011/01/26 16:04:39 $
   */

  template<typename WebPageHelper_t, typename EventQueueCollection_t, typename StatisticsReporter_t>
  class ConsumerWebPageHelper : public WebPageHelper<WebPageHelper_t>
  {
  public:

    ConsumerWebPageHelper
    (
      xdaq::ApplicationDescriptor* appDesc,
      const std::string& cvsVersion,
      WebPageHelper_t* webPageHelper,
      void (WebPageHelper_t::*addHyperLinks)(XHTMLMaker&, XHTMLMaker::Node*)
    );

    /**
       Generates consumer statistics page
    */
    void consumerStatistics
    (
      xgi::Output*,
      const std::string& externallyVisibleState,
      const std::string& innerStateName,
      const std::string& errorMsg,
      boost::shared_ptr<StatisticsReporter_t>,
      RegistrationCollectionPtr,
      boost::shared_ptr<EventQueueCollection_t>,
      DQMEventQueueCollectionPtr
    );
    
    
  private:

    /**
     * Adds statistics for event consumers
     */
    void addDOMforEventConsumers
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node* parent,
      RegistrationCollectionPtr registrationCollection,
      boost::shared_ptr<EventQueueCollection_t> eventQueueCollection,
      const EventConsumerMonitorCollection& eventConsumerCollection
    );

    /**
     * Adds statistics for DQM event consumers
     */
    void addDOMforDQMEventConsumers
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node* parent,
      RegistrationCollectionPtr registrationCollection,
      DQMEventQueueCollectionPtr dqmEventQueueCollection,
      const DQMConsumerMonitorCollection& dqmConsumerCollection
    );

    void addEntryForMaxRequestRate
    (
      XHTMLMaker& maker,
      XHTMLMaker::Node* tableRow,
      const utils::duration_t& interval
    );

    //Prevent copying of the ConsumerWebPageHelper
    ConsumerWebPageHelper(ConsumerWebPageHelper const&);
    ConsumerWebPageHelper& operator=(ConsumerWebPageHelper const&);

    xdaq::ApplicationDescriptor* _appDescriptor;

  };

} // namespace stor

#endif // EventFilter_StorageManager_ConsumerWebPageHelper_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
