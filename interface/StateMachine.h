// -*- c++ -*-

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>

#include <string>

namespace bsc = boost::statechart;

////////////////////////////////////////////////
//// Forward declarations of state classes: ////
////////////////////////////////////////////////

class Failed;
class Normal;
class Halted;
class Ready;
class Stopped;
class Enabled;
class Processing;
class DrainingQueues;

////////////////////////////
//// Transition events: ////
////////////////////////////

class Configure : public bsc::event<Configure> {};
class Enable : public bsc::event<Enable> {};
class Stop : public bsc::event<Stop> {};
class Halt : public bsc::event<Halt> {};
class Fail : public bsc::event<Fail> {};
class Reconfigure : public bsc::event<Reconfigure> {};
class EmergencyStop : public bsc::event<EmergencyStop> {};
class StopDone : public bsc::event<StopDone> {};

////////////////////////////////////////////////////////
//// Operations -- abstract base for state classes: ////
////////////////////////////////////////////////////////

class Operations
{

public:

  virtual ~Operations() = 0;
  virtual void handleI2OEventMessage() const = 0;
  virtual std::string stateName() const = 0;

};

///////////////////////
//// StateMachine: ////
///////////////////////

class StateMachine: public bsc::state_machine<StateMachine,Normal>
{

public:

  void handleI2OEventMessage();
  std::string getCurrentStateName();

};

////////////////////////
//// State classes: ////
////////////////////////

// Failed:
class Failed: public bsc::state<Failed,StateMachine>, public Operations
{

public:

  Failed( my_context );
  virtual ~Failed();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

};

// Normal:
class Normal: public bsc::state<Normal,StateMachine,Halted>, public Operations
{

public:

  typedef bsc::transition<Fail,Failed> FT;
  typedef boost::mpl::list<FT> reactions;

  Normal( my_context );
  virtual ~Normal();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

};

// Halted:
class Halted: public bsc::state<Halted,Normal>, public Operations
{

public:

  typedef bsc::transition<Configure,Ready> RT;
  typedef boost::mpl::list<RT> reactions;

  Halted( my_context );
  virtual ~Halted();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

};

// Ready:
class Ready: public bsc::state<Ready,Normal,Stopped>, public Operations
{

public:

  typedef bsc::transition<Halt,Halted> HT;
  typedef boost::mpl::list<HT> reactions;

  Ready( my_context );
  virtual ~Ready();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

};

// Stopped:
class Stopped: public bsc::state<Stopped,Ready>, public Operations
{

public:

  typedef bsc::transition<Reconfigure,Stopped> ST;
  typedef bsc::transition<Enable,Enabled> ET;
  typedef boost::mpl::list<ST,ET> reactions;

  Stopped( my_context );
  virtual ~Stopped();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

};

// Enabled:
class Enabled: public bsc::state<Enabled,Ready,Processing>, public Operations
{

public:

  typedef bsc::transition<EmergencyStop,Stopped> ET;
  typedef bsc::transition<StopDone,Stopped> DT;
  typedef boost::mpl::list<ET,DT> reactions;

  Enabled( my_context );
  virtual ~Enabled();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

};

// Processing:
class Processing: public bsc::state<Processing,Enabled>, public Operations
{

public:

  typedef bsc::transition<Stop,DrainingQueues> DT;
  typedef boost::mpl::list<DT> reactions;

  Processing( my_context );
  virtual ~Processing();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

private:

  static unsigned int _counter;

};

// DrainingQueues:
class DrainingQueues: public bsc::state<DrainingQueues,Enabled>, public Operations
{

public:

  DrainingQueues( my_context );
  virtual ~DrainingQueues();

  virtual std::string stateName() const;
  virtual void handleI2OEventMessage() const;

};

#endif
