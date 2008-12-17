#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/mpl/list.hpp>
#include <iostream>

namespace sc = boost::statechart;


// Forward definition of the states
class Normal;
class Failed;
class Halted;
class Ready;
class Enabled;

// Definition of the state transitions
class Configure : public sc::event< Configure > {};
class Enable    : public sc::event< Enable > {};
class Stop      : public sc::event< Stop > {};
class Halt      : public sc::event< Halt > {};
class Fail      : public sc::event< Fail > {};

class SMOperations
{
public:

    virtual void handleI2OEventMessage() const
    {
        std::cout << "ERROR: handleI2OEventMessage not supported in "
                  << stateName_ << " state!" << std::endl;
    }

protected:

    std::string stateName_;
};

// Define the state machine with its initial state
class SMStateMachine : public sc::state_machine< SMStateMachine, Normal >
{
public:

    void handleI2OEventMessage()
    {
        const SMOperations *ptr = state_cast< const SMOperations * >();
        ptr->handleI2OEventMessage();
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
        stateName_ = "Normal";
        std::cout << "Entering '" << stateName_ << "' outer state" << std::endl;
    }
    
    ~Normal() 
    { 
        std::cout << "Exiting 'NORMAL' outer state" << std::endl;
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
        stateName_ = "Failed";
        std::cout << "Entering 'FAILED' outer state" << std::endl;
    }
    
    ~Failed() 
    { 
        std::cout << "Exiting 'FAILED' outer state" << std::endl;
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
        stateName_ = "Halted";
        std::cout << "    Entering 'HALTED' inner state" << std::endl;
    }
    
    ~Halted() 
    { 
        std::cout << "    Exiting 'HALTED' inner state" << std::endl;
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
        stateName_ = "Ready";
        std::cout << "    Entering 'READY' inner state" << std::endl;
    }

    ~Ready() 
    { 
        std::cout << "    Exiting 'READY' inner state" << std::endl;
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
        stateName_ = "Enabled";
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

    void handleI2OEventMessage() const
    {
        std::cout << "SUCCESS: handleI2OEventMessage correctly called in "
                  << stateName_ << " state!" << std::endl;
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
            ::sleep(1);
        }
    }

    return 0;
}
