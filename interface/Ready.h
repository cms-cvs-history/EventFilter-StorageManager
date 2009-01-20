// -*- c++ -*-

#ifndef READY_H
#define READY_H

// Ready, inner state of Normal

#include <boost/statechart/simple_state.hpp>

#include <string>

#include "EventFilter/StorageManager/interface/TransitionEvents.h"
#include "EventFilter/StorageManager/interface/Operations.h"

class Normal;
class Enabled;
class Halted;
class Ready;

namespace bsc = boost::statechart;

class Ready: public bsc::simple_state<Ready,Normal>, public Operations
{

public:

  typedef bsc::transition<Enable,Enabled> ET;
  typedef bsc::transition<Halt,Halted> HT;
  typedef boost::mpl::list<ET,HT> reactions;

  Ready();
  virtual ~Ready();

  virtual std::string state_name() const;

  virtual void handle_I2O_event_message() const;

};

#endif
