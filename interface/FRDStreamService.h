#ifndef FRDSTREAMSERVICE_H
#define FRDSTREAMSERVICE_H

// $Id: FRDStreamService.h,v 1.2 2008/09/04 17:44:15 biery Exp $

// - handling output files per stream make the problem 1-dimensional 
// - allows to use different file handling rules per stream

// functionality:
// - create and delete output service
// - do accounting
// - enforce file management rules

// needs:
// - filename, rules, etc.

#include "IOPool/Streamer/interface/FRDEventMessage.h"

#include "EventFilter/StorageManager/interface/StreamService.h"  

namespace edm {

  class FRDStreamService : public StreamService
  {
    public:
      FRDStreamService(ParameterSet const&);
      ~FRDStreamService() { stop(); }
      
      bool   nextEvent(const uint8 * const);
      void   stop();
      void   report(std::ostream &os, int indentation) const;

      void   closeTimedOutFiles(int lumi, double timeoutstart);
      void   closeTimedOutFiles();
 
    private:
      boost::shared_ptr<OutputService>  newOutputService();
      boost::shared_ptr<OutputService>  getOutputService(FRDEventMsgView const&);
      boost::shared_ptr<FileRecord>     generateFileRecord();  

      bool   checkEvent(boost::shared_ptr<FileRecord>, FRDEventMsgView const&) const;

  };

} // edm namespace
#endif
/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
