#ifndef OUTPUTSERVICE_H
#define OUTPUTSERVICE_H

// $Id: OutputService.h,v 1.4.12.1 2009/03/12 14:38:19 mommsen Exp $

#include <EventFilter/StorageManager/interface/FileRecord.h>
#include <IOPool/Streamer/interface/MsgHeader.h>

#include <boost/shared_ptr.hpp>
#include <string>

namespace edm {

  class OutputService
  {
    public:
      virtual ~OutputService();

      virtual void writeEvent(const uint8 * const) = 0;
      double lastEntry()   const { return file_->lastEntry(); }
      double lumiSection() const { return file_->lumiSection(); }
      virtual void report(std::ostream &os, int indentation) const = 0;

    protected:
      double getTimeStamp() const;

      boost::shared_ptr<stor::FileRecord> file_;
  };

} // edm namespace
#endif
