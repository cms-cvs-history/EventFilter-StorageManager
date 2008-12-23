#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>
#include <iostream>

namespace sc = boost::statechart;


// Forward definition of the states and base class.
class SMOperations;
class Normal;
class Failed;
class Halted;
class Ready;
class Enabled;

// Definition of the events.
class Configure : public sc::event< Configure > {};
class Enable    : public sc::event< Enable > {};
class Stop      : public sc::event< Stop > {};
class Halt      : public sc::event< Halt > {};
class Fail      : public sc::event< Fail > {};

class SMOperations
{
public:

  // SMOperations is a base class, so it should have a virtual
  // destructor. Furthermore, it should be an abstract class; one
  // should not make instances of type SMOperations. Thus it is best
  // to have it be a pure interface, containing only abstract
  // functions and no data.

  virtual ~SMOperations() = 0;
  virtual void handleI2OEventMessage() const = 0;
  virtual const char* stateName() const = 0;
};

// Even though the destructor is pure virtual, we still need an
// implementation.
SMOperations::~SMOperations()
{
  // nothing to do.
}


// These are examples of the I2O message handling functions. This is
// not a full set, nor are the implementations at all interesting.
namespace I2OMessageHandlingFunctions
{
  void logError(SMOperations const& me)
  {
    std::cout << "ERROR: illegal call made while in state "
	      << me.stateName() << std::endl;
  }

  void logSuccess(SMOperations const& me)
  {
    std::cout << "SUCCESS: legal call made while in state "
	      << me.stateName() << std::endl;
  }
}


// Define the state machine with its initial state.
class SMStateMachine : public sc::state_machine< SMStateMachine, Normal >
{
public:

  void handleI2OEventMessage()
  {
    //  Use the reference form to throw an exception in case of a
    //  failed cast.
    SMOperations const& ref = state_cast<SMOperations const&>();
    ref.handleI2OEventMessage();
  }
};

// The NORMAL state is the default outer state of the SMStateMachine.
// It's inner initial state is HALTED
class Normal : public sc::simple_state< Normal, SMStateMachine, Halted >,
               public SMOperations
{
public:

  // The NORMAL state can only fail
  typedef boost::mpl::list<
    sc::transition< Fail, Failed > 
  > reactions;

  Normal() 
  { 
    std::cout << "Entering '" << stateName() << "' outer state" << std::endl;
  }
    
  virtual ~Normal() 
  { 
    std::cout << "Exiting 'NORMAL' outer state" << std::endl;
  }

  virtual const char* stateName() const { return "Normal"; }
  virtual void handleI2OEventMessage () const 
  {
    I2OMessageHandlingFunctions::logError(*this);
  }
};

// The FAILED state is an outer state
class Failed : public sc::simple_state< Failed, SMStateMachine >,
               public SMOperations
{
public:

  // There's no way out of the FAILED state.

  Failed() 
  { 
    std::cout << "Entering 'FAILED' outer state" << std::endl;
  }
    
  virtual ~Failed() 
  { 
    std::cout << "Exiting 'FAILED' outer state" << std::endl;
  }

  virtual const char* stateName() const { return "Failed"; }

  virtual void handleI2OEventMessage () const 
  {
    I2OMessageHandlingFunctions::logError(*this);
  }
};


// The HALTED state is a inner state of NORMAL
class Halted : public sc::simple_state< Halted, Normal >,
               public SMOperations
{
public:

  typedef boost::mpl::list<
    sc::transition< Configure, Ready > 
  > reactions;

  Halted() 
  { 
    std::cout << "    Entering 'HALTED' inner state" << std::endl;
  }
    
  virtual ~Halted() 
  { 
    std::cout << "    Exiting 'HALTED' inner state" << std::endl;
  }

  virtual const char* stateName() const { return "Halted"; }

  virtual void handleI2OEventMessage() const 
  {
    I2OMessageHandlingFunctions::logError(*this);
  }
};


// The READY state is a inner state of NORMAL
class Ready : public sc::simple_state< Ready, Normal >,
              public SMOperations
{
public:


  typedef boost::mpl::list<
    sc::transition< Enable, Enabled >,
    sc::transition< Halt, Halted > 
  > reactions;

  Ready() 
  { 
    std::cout << "    Entering 'READY' inner state" << std::endl;
  }

  virtual ~Ready() 
  { 
    std::cout << "    Exiting 'READY' inner state" << std::endl;
  }

  virtual const char* stateName() const { return "Ready"; }

  virtual void handleI2OEventMessage() const 
  { 
    I2OMessageHandlingFunctions::logError(*this); 
  }
};


// The ENABLED state is a inner state of NORMAL
class Enabled : public sc::state< Enabled, Normal >,
                public SMOperations
{
public:

  typedef boost::mpl::list<
    sc::transition< Stop, Ready >,
    sc::transition< Halt, Halted > 
  > reactions;

  Enabled(my_context ctx) : my_base(ctx)
  { 
    ++creationCounter_;
    if (creationCounter_ >= 3) {
      std::cout << "  'ENABLED' entry action encountered an error"
		<< std::endl;
      post_event( Fail() );
      return;
    }
    std::cout << "    Entering 'ENABLED' inner state" << std::endl;
  }

  ~Enabled() 
  { 
    std::cout << "  Closing files gracefully" << std::endl;
    std::cout << "    Exiting 'ENABLED' inner state" << std::endl;
  }

  const char* stateName() const { return "Enabled"; }


  virtual void handleI2OEventMessage() const 
  {
    I2OMessageHandlingFunctions::logSuccess(*this); 
  }

private:

  // simple-minded way to get the entry action to fail after N entries
  static int creationCounter_;
};

int Enabled::creationCounter_ = 0;



int main()
{
  SMStateMachine stateMachine;

  stateMachine.initiate();

  std::string transitionList[4] = {"Configure", "Enable",
				   "Stop", "Halt"};

  for (int jdx = 0; jdx < 3; ++jdx) {
    for (int idx = 0; idx < 4; ++idx) {
      std::string requestedTransition = transitionList[idx];

      // go directly from Enabled to Halted in one case
      if (jdx == 1 && idx == 2) {continue;}

      // this code would look like what would be in
      // the SM SOAP receiving method, correct?
      if (requestedTransition == "Configure") {
	stateMachine.process_event( Configure() );
      }
      else if (requestedTransition == "Enable") {
	stateMachine.process_event( Enable() );
      }
      else if (requestedTransition == "Stop") {
	stateMachine.process_event( Stop() );
      }
      else if (requestedTransition == "Halt") {
	stateMachine.process_event( Halt() );
      }

      // simulate an event fragment being received
      // while in a particular state
      stateMachine.handleI2OEventMessage();
      //::sleep(1);
    }
  }

  return 0;
}
