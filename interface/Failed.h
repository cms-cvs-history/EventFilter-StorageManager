// -*- c++ -*-

#ifndef FAILED_H
#define FAILED_H

// Failed state

#include <boost/statechart/simple_state.hpp>

#include <string>

#include "EventFilter/StorageManager/interface/Operations.h"

class Failed;
class StateMachine;

namespace bsc = boost::statechart;

class Failed: public bsc::simple_state<Failed,StateMachine>, public Operations
{

public:

  Failed();
  virtual ~Failed();

  virtual const std::string& state_name() const;
  virtual void handle_I2O_event_message() const;

};

#endif
