// -*- c++ -*-

#ifndef ENABLED_H
#define ENABLED_H

// Enabled, inner state of Normal

#include <boost/statechart/state.hpp>
#include <boost/mpl/list.hpp>

#include <string>

#include "EventFilter/StorageManager/interface/TransitionEvents.h"
#include "EventFilter/StorageManager/interface/Operations.h"

class Ready;
class Halted;
class Enabled;
class Normal;

namespace bsc = boost::statechart;

class Enabled: public bsc::state<Enabled,Normal>, public Operations
{

public:

  typedef bsc::transition<Stop,Ready> ST;
  typedef bsc::transition<Halt,Halted> HT;
  typedef boost::mpl::list<ST,HT> reactions;

  Enabled( my_context );
  virtual ~Enabled();

  virtual std::string state_name() const;
  virtual void handle_I2O_event_message() const;

private:

  static unsigned int _counter;

};

#endif
