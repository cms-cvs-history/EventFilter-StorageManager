// -*- c++ -*-

#ifndef HALTED_H
#define HALTED_H

// Halted, inner state of Normal

#include <boost/statechart/simple_state.hpp>
#include <boost/mpl/list.hpp>

#include <string>

#include "EventFilter/StorageManager/interface/TransitionEvents.h"
#include "EventFilter/StorageManager/interface/Operations.h"

class Normal;
class Ready;
class Halted;

namespace bsc = boost::statechart;

class Halted: public bsc::simple_state<Halted,Normal>, public Operations
{

public:

  typedef bsc::transition<Configure,Ready> RT;
  typedef boost::mpl::list<RT> reactions;

  Halted();
  virtual ~Halted();

  virtual std::string state_name() const;
  virtual void handle_I2O_event_message() const;

};

#endif
