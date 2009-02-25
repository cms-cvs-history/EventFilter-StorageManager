// -*- c++ -*-
// $Id: $

#ifndef EVENTSTREAMCONFIGURATIONINFO_H
#define EVENTSTREAMCONFIGURATIONINFO_H

#include <string>
#include <vector>
#include <iostream>

namespace stor
{

  class EventStreamConfigurationInfo
  {

  public:

    typedef std::vector<std::string> FilterList;

    // Constructor:
    EventStreamConfigurationInfo( const std::string& streamLabel,
				  unsigned int maxSize,
				  const FilterList& selEvents,
				  const std::string& selHLTOut ):
      _streamLabel( streamLabel ),
      _maxSize( maxSize ),
      _selEvents( selEvents ),
      _selHLTOut( selHLTOut )
    {}

    // Destructor:
    ~EventStreamConfigurationInfo() {}

    // Accessors:
    const std::string& streamLabel() const { return _streamLabel; }
    unsigned int maxSize() const { return _maxSize; }
    const FilterList& selEvents() const { return _selEvents; }
    const std::string& selHLTOut() const { return _selHLTOut; }

    // Output:
    friend std::ostream& operator <<
      ( std::ostream&, const EventStreamConfigurationInfo& );

  private:

    std::string _streamLabel;
    unsigned int _maxSize;
    FilterList _selEvents;
    std::string _selHLTOut;

  };

}

#endif
