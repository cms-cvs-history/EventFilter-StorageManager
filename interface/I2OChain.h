// $Id: I2OChain.h,v 1.1.2.2 2009/01/30 10:49:40 mommsen Exp $

#ifndef StorageManager_I2OChain_h
#define StorageManager_I2OChain_h

#include <vector>

//#include "toolbox/mem/Reference.h"
#include "EventFilter/StorageManager/interface/Types.h"

namespace toolbox
{
  namespace mem
  {
    class Reference;
  } // namespace mem
} // namespace toolbox

namespace stor {

  /**
   * List of one or multiple I2O messages representing event fragments. 
   *
   * It wraps several toolbox::mem::Reference chained together and 
   * assures that the corresponding release methods are called when 
   * the last instance of I2OChain goes out of scope.
   *
   * $Author: mommsen $
   * $Revision: 1.1.2.2 $
   * $Date: 2009/01/30 10:49:40 $
   */
  
  class I2OChain
  {
  public:

    /**
       Default constructor, copy, assignment, and destructor are all
       compiler-generated.
    */

    /**
     * Returns true if there are no I2O messages (fragments)
     */
    bool empty() const;

    /**
     * Returns true if all fragments of an event are available
     */
    bool complete() const;

    /**
     * Adds fragments from another chain to the current chain
     * taking care that all fragments are chained in the right order
     */
    void addToChain(I2OChain&);

    
  private:
    
    std::vector<QueueID> _streamTags;
    std::vector<QueueID> _dqmEventConsumerTags;
    std::vector<QueueID> _eventConsumerTags;

    toolbox::mem::Reference* ref;
  };
  
} // namespace stor

#endif // StorageManager_I2OChain_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
