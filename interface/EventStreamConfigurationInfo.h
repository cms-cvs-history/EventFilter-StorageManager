// $Id: EventStreamConfigurationInfo.h,v 1.10 2010/12/17 18:21:04 mommsen Exp $
/// @file: EventStreamConfigurationInfo.h

#ifndef EventFilter_StorageManager_EventStreamConfigurationInfo_h
#define EventFilter_StorageManager_EventStreamConfigurationInfo_h

#include "IOPool/Streamer/interface/HLTInfo.h"
#include "EventFilter/StorageManager/interface/StreamID.h"

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>
#include <iosfwd>

namespace stor
{

  /**
     Configuration information for the event stream

     $Author: mommsen $
     $Revision: 1.10 $
     $Date: 2010/12/17 18:21:04 $
  */

  class EventStreamConfigurationInfo
  {

  public:

    // Constructor:
    EventStreamConfigurationInfo( const std::string& streamLabel,
                                  const int maxFileSizeMB,
                                  const std::string& triggerSelection,
                                  const Strings& eventSelection,
                                  const std::string& outputModuleLabel,
                                  double fractionToDisk ):
      _streamLabel( streamLabel ),
      _maxFileSizeMB( maxFileSizeMB ),
      _triggerSelection( triggerSelection ),
      _eventSelection( eventSelection ),
      _outputModuleLabel( outputModuleLabel ),
      _fractionToDisk( fractionToDisk ),
      _streamId(0)
    {}

    // Destructor:
    ~EventStreamConfigurationInfo() {}

    // Accessors:
    const std::string& streamLabel() const { return _streamLabel; }
    const int maxFileSizeMB() const { return _maxFileSizeMB; }
    const std::string& triggerSelection() const { return _triggerSelection; }
    const Strings& eventSelection() const { return _eventSelection; }
    const std::string& outputModuleLabel() const { return _outputModuleLabel; }
    double fractionToDisk() const { return _fractionToDisk; }
    StreamID streamId() const { return _streamId; }

    // Comparison:
    bool operator<(const EventStreamConfigurationInfo&) const;

    // Set stream Id:
    void setStreamId( StreamID sid ) { _streamId = sid; }

    // Output:
    friend std::ostream& operator <<
      ( std::ostream&, const EventStreamConfigurationInfo& );

  private:

    std::string _streamLabel;
    int _maxFileSizeMB;
    std::string _triggerSelection;
    Strings _eventSelection;
    std::string _outputModuleLabel;
    double _fractionToDisk;
    StreamID _streamId;

  };

  typedef std::vector<EventStreamConfigurationInfo> EvtStrConfigList;
  typedef boost::shared_ptr<EvtStrConfigList> EvtStrConfigListPtr;

  std::ostream& operator << ( std::ostream&, const EventStreamConfigurationInfo& );

} // namespace stor

#endif // EventFilter_StorageManager_EventStreamConfigurationInfo_h


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
