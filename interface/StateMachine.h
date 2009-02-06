// -*- c++ -*-

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "toolbox/lang/Class.h"
#include "toolbox/task/WorkLoop.h"

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>

#include <string>
#include <vector>
#include <ctime>
#include <sys/time.h>

namespace bsc = boost::statechart;

namespace stor
{

  class I2OChain;
  class DiskWriter;
  class EventDistributor;
  class FragmentProcessor;
  class FragmentStore;

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

    Operations();
    virtual ~Operations() = 0;
    void processI2OFragment( I2OChain& frag,
			     EventDistributor& ed,
                             FragmentStore& fs ) const;

    std::string stateName() const;

  protected:

    virtual void do_processI2OFragment( I2OChain& frag,
				        EventDistributor& ed,
				        FragmentStore& fs ) const;
    virtual std::string do_stateName() const = 0;

  };

  ///////////////////////////
  //// TransitionRecord: ////
  ///////////////////////////

  class TransitionRecord
  {

  public:

    TransitionRecord( const std::string& state_name, bool is_entry );

    const std::string& stateName() const { return _stateName; }
    bool isEntry() const { return _isEntry; }
    const struct timeval& timeStamp() const { return _timestamp; }

  private:

    std::string _stateName;
    bool _isEntry;
    struct timeval _timestamp;

  };

  ///////////////////////
  //// StateMachine: ////
  ///////////////////////

  class StateMachine: public bsc::state_machine<StateMachine,Normal>
  {

  public:

    StateMachine( DiskWriter* dw,
                  EventDistributor* ed,
                  FragmentProcessor* fp );

    //void processI2OFragment();
    std::string getCurrentStateName();
    Operations const& getCurrentState();

    void updateHistory( const TransitionRecord& tr );

    typedef std::vector<TransitionRecord> History;
    const History& history() const { return _history; }

    DiskWriter* getDiskWriter() { return _diskWriter; }
    EventDistributor* getEventDistributor() { return _eventDistributor; }
    FragmentProcessor* getFragmentProcessor() { return _fragmentProcessor; }

  private:

    History _history;

    DiskWriter* _diskWriter;
    EventDistributor* _eventDistributor;
    FragmentProcessor* _fragmentProcessor;

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

  private:

    virtual std::string do_stateName() const;

  };

  // Normal:
  class Normal: public bsc::state<Normal,StateMachine,Halted>, public Operations
  {

  public:

    typedef bsc::transition<Fail,Failed> FT;
    typedef boost::mpl::list<FT> reactions;

    Normal( my_context );
    virtual ~Normal();

  private:

    virtual std::string do_stateName() const;
  };

  // Halted:
  class Halted: public bsc::state<Halted,Normal>, public Operations
  {

  public:

    typedef bsc::transition<Configure,Ready> RT;
    typedef boost::mpl::list<RT> reactions;

    Halted( my_context );
    virtual ~Halted();

  private:

    virtual std::string do_stateName() const;

  };

  // Ready:
  class Ready: public bsc::state<Ready,Normal,Stopped>, public Operations
  {

  public:

    typedef bsc::transition<Halt,Halted> HT;
    typedef boost::mpl::list<HT> reactions;

    Ready( my_context );
    virtual ~Ready();

  private:

    virtual std::string do_stateName() const;

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
    
  private:

    virtual std::string do_stateName() const;

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

  private:

    virtual std::string do_stateName() const;

  };

  // Processing:
  class Processing: public bsc::state<Processing,Enabled>, public Operations
  {

  public:

    typedef bsc::transition<Stop,DrainingQueues> DT;
    typedef boost::mpl::list<DT> reactions;

    Processing( my_context );
    virtual ~Processing();

  private:

    virtual std::string do_stateName() const;
    virtual void do_processI2OFragment( I2OChain& frag,
				        EventDistributor& ed,
				        FragmentStore& fs ) const;
    static unsigned int _counter;

  };

  // DrainingQueues:
  class DrainingQueues: public bsc::state<DrainingQueues,Enabled>, public Operations,
    public toolbox::lang::Class
  {

  public:

    DrainingQueues( my_context );
    virtual ~DrainingQueues();
    void emergencyStop( const EmergencyStop& );

  private:
    virtual std::string do_stateName() const;
    bool action( toolbox::task::WorkLoop* );

    toolbox::task::WorkLoop *_workloop;
    bool _doDraining;

  };

} // end namespace stor

#endif

/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
