// -*- c++ -*-

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <boost/statechart/state_machine.hpp>

class Normal;

namespace bsc = boost::statechart;

class StateMachine: public bsc::state_machine<StateMachine,Normal>
{

public:

  void handle_I2O_event_message();

};

#endif
