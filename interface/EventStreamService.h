#ifndef EVENTSTREAMSERVICE_H
#define EVENTSTREAMSERVICE_H

// $Id: EventStreamService.h,v 1.2.10.3 2009/03/13 20:11:10 biery Exp $

// - handling output files per stream make the problem 1-dimensional 
// - allows to use different file handling rules per stream

// functionality:
// - create and delete output service
// - pass init and event message to correct output service
// - do accounting
// - enforce file management rules

// needs:
// - event selector
// - copy of init message to create a new file
// - filename, rules, etc.

#include <FWCore/Framework/interface/EventSelector.h>

#include <IOPool/Streamer/interface/InitMessage.h>
#include <IOPool/Streamer/interface/EventMessage.h>

#include <EventFilter/StorageManager/interface/StreamService.h>  
#include <EventFilter/StorageManager/interface/Configuration.h>

namespace edm {

  class EventStreamService : public StreamService
  {
    public:
      EventStreamService(ParameterSet const&, InitMsgView const&,
                         stor::DiskWritingParams dwParams);
      ~EventStreamService() { stop(); }
      
      bool   nextEvent(const uint8 * const);
      void   stop();
      void   report(std::ostream &os, int indentation) const;

      void   closeTimedOutFiles();
 
    private:
      boost::shared_ptr<OutputService>  newOutputService();
      boost::shared_ptr<OutputService>  getOutputService(EventMsgView const&);
      boost::shared_ptr<stor::FileRecord> generateFileRecord();  

      void   saveInitMessage(InitMsgView const&);
      void   initializeSelection(InitMsgView const&);
      bool   acceptEvent(EventMsgView const&);
      bool   checkEvent(boost::shared_ptr<stor::FileRecord>, EventMsgView const&) const;

      // variables
      boost::shared_ptr<edm::EventSelector>  eventSelector_;

      // set from init message ( is init message )
      std::vector<unsigned char> saved_initmsg_;

      stor::DiskWritingParams diskWritingParams_;
  };

} // edm namespace
#endif
/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
