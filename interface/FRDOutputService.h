#ifndef FRDOUTPUTSERVICE_H
#define FRDOUTPUTSERVICE_H

// $Id: FRDOutputService.h,v 1.1.12.1 2009/03/12 14:38:19 mommsen Exp $

#include <EventFilter/StorageManager/interface/OutputService.h>
#include <IOPool/Streamer/interface/FRDEventFileWriter.h>

namespace edm {

  class FRDOutputService : public OutputService
  {
    public:
      FRDOutputService(boost::shared_ptr<stor::FileRecord>);
      ~FRDOutputService();

      void   writeEvent(const uint8 * const);
      void   report(std::ostream &os, int indentation) const;

    private:
      void   closeFile();

      boost::shared_ptr<FRDEventFileWriter> writer_;
  };

} // edm namespace
#endif
