// $Id: ConsumerWebPageHelper.h,v 1.12.2.1 2011/01/24 12:18:39 mommsen Exp $
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
   * $Revision: 1.12.2.1 $
   * $Date: 2011/01/24 12:18:39 $
   */

  template<typename EventQueueCollection_t, typename StatisticsReporter_t>
  class ConsumerWebPageHelper : public WebPageHelper
  {
  public:

    ConsumerWebPageHelper(xdaq::ApplicationDescriptor*);

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
    
    typedef boost::function<void (XHTMLMaker&, XHTMLMaker::Node*)> HyperLinks_t;
    
    
  private:

    /**
       Adds the links for the other hyperdaq webpages
     */
    void addDOMforHyperLinks(XHTMLMaker&, XHTMLMaker::Node* parent) {}; 

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


  private:

    //Prevent copying of the ConsumerWebPageHelper
    ConsumerWebPageHelper(ConsumerWebPageHelper const&);
    ConsumerWebPageHelper& operator=(ConsumerWebPageHelper const&);

    xdaq::ApplicationDescriptor* _appDescriptor;

  };

} // namespace stor

#include "EventFilter/StorageManager/src/ConsumerWebPageHelper.icc"

#endif // EventFilter_StorageManager_ConsumerWebPageHelper_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
