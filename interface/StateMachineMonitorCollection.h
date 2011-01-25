// $Id: StateMachineMonitorCollection.h,v 1.7.8.1 2011/01/24 12:18:39 mommsen Exp $
/// @file: StateMachineMonitorCollection.h 

#ifndef EventFilter_StorageManager_StateMachineMonitorCollection_h
#define EventFilter_StorageManager_StateMachineMonitorCollection_h

#include <ostream>
#include <string>

#include "xdata/String.h"

#include "EventFilter/StorageManager/interface/MonitorCollection.h"
#include "EventFilter/StorageManager/interface/TransitionRecord.h"


namespace stor {

  /**
   * A collection of monitored quantities related to the state machine
   *
   * $Author: mommsen $
   * $Revision: 1.7.8.1 $
   * $Date: 2011/01/24 12:18:39 $
   */
  
  class StateMachineMonitorCollection : public MonitorCollection
  {

  public:

    explicit StateMachineMonitorCollection(const utils::duration_t& updateInterval);

    /**
     * Add the TransitionRecord to the state machine history
     */
    void updateHistory(const TransitionRecord&);

    /**
     * Copy the state machine history in the given History vector
     */
    typedef std::vector<TransitionRecord> History;
    void getHistory(History&) const;

    /**
     * Dump the state machine history into the stream
     */
    void dumpHistory(std::ostream&) const;

    /**
     * Set the externally visible state name
     */
    void setExternallyVisibleState( const std::string& );

    /**
     * Retrieve the externally visible state name
     */
    const std::string& externallyVisibleState() const;

    /**
     * Set status message
     */
    void setStatusMessage( const std::string& );

    /**
     * Clear status message
     */
    void clearStatusMessage();

    /**
     * Get status message
     */
    bool statusMessage( std::string& msg ) const;

    /**
     * Retrieve the current internal state name
     */
    std::string innerStateName() const;

  private:

    //Prevent copying of the StateMachineMonitorCollection
    StateMachineMonitorCollection(StateMachineMonitorCollection const&);
    StateMachineMonitorCollection& operator=(StateMachineMonitorCollection const&);

    virtual void do_calculateStatistics();
    virtual void do_reset();
    virtual void do_appendInfoSpaceItems(InfoSpaceItems&);
    virtual void do_updateInfoSpaceItems();

    History _history;
    std::string _externallyVisibleState;
    mutable boost::mutex _stateMutex;

    std::string _statusMessage;

    xdata::String _stateName;

  };
  
} // namespace stor

#endif // EventFilter_StorageManager_StateMachineMonitorCollection_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
