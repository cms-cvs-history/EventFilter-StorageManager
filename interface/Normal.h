// -*- c++ -*-

#ifndef NORMAL_H
#define NORMAL_H

// Normal outer state

#include <boost/statechart/simple_state.hpp>

#include <string>

#include "EventFilter/StorageManager/interface/TransitionEvents.h"
#include "EventFilter/StorageManager/interface/Operations.h"

class Failed;
class StateMachine;
class Halted;
class Normal;

namespace bsc = boost::statechart;

class Normal: public bsc::simple_state<Normal,StateMachine,Halted>, public Operations
{

public:

  typedef bsc::transition<Fail,Failed> FT;
  typedef boost::mpl::list<FT> reactions;

  Normal();
  virtual ~Normal();

  virtual std::string state_name() const;
  virtual void handle_I2O_event_message() const;

};

#endif
