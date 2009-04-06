// $Id: StreamsMonitorCollection.h,v 1.1.2.6 2009/04/03 14:28:10 mommsen Exp $

#ifndef StorageManager_StreamsMonitorCollection_h
#define StorageManager_StreamsMonitorCollection_h

#include <sstream>
#include <iomanip>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "xdata/UnsignedInteger32.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"


namespace stor {

  /**
   * A collection of MonitoredQuantities of output streams
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.6 $
   * $Date: 2009/04/03 14:28:10 $
   */
  
  class StreamsMonitorCollection : public MonitorCollection
  {
  public:

    struct StreamRecord
    {
      std::string streamName;  // name of the stream
      MonitoredQuantity size;  // data in MBytes stored in this stream
    };

    // We do not know how many streams there will be.
    // Thus, we need a vector of them.
    typedef boost::shared_ptr<StreamRecord> StreamRecordPtr;
    typedef std::vector<StreamRecordPtr> StreamRecordList;


    explicit StreamsMonitorCollection(xdaq::Application*);

    const StreamRecordPtr getNewStreamRecord();

    const StreamRecordList& getStreamRecordsMQ() const {
      return _streamRecords;
    }
    StreamRecordList& getStreamRecordsMQ() {
      return _streamRecords;
    }


  private:

    //Prevent copying of the StreamsMonitorCollection
    StreamsMonitorCollection(StreamsMonitorCollection const&);
    StreamsMonitorCollection& operator=(StreamsMonitorCollection const&);

    virtual void do_calculateStatistics();
    
    virtual void do_updateInfoSpace();

    StreamRecordList _streamRecords;


    // InfoSpace items which were defined in the old SM


  };
  
} // namespace stor

#endif // StorageManager_StreamsMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
