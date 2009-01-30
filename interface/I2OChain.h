// $Id: I2OChain.h,v 1.1.2.1 2009/01/20 10:54:04 mommsen Exp $

#ifndef StorageManager_I2OChain_h
#define StorageManager_I2OChain_h

#include "toolbox/mem/Reference.h"


namespace stor {

  /**
   * List of one or multiple I2O messages representing event fragments. 
   *
   * It wraps several toolbox::mem::Reference chained together and 
   * assures that the corresponding release methods are called when 
   * the last instance of I2OChain goes out of scope.
   *
   * $Author:$
   * $Revision:$
   * $Date:$
   */
  
  class I2OChain
  {
  public:

    I2OChain();
    
    ~I2OChain();

    /**
     * Returns true if there are no I2O messages (fragments)
     */
    bool empty();

    /**
     * Returns true if all fragments of an event are available
     */
    bool complete();

    /**
     * Adds fragments from another chain to the current chain
     * taking care that all fragments are chained in the right order
     */
    void addToChain(I2OChain&);

    /**
     * Returns a copy of the current chain
     */
    I2OChain copy();

    
  private:
    
    int id;
    toolbox::mem::Reference ref;

  };
  
} // namespace stor

#endif // StorageManager_I2OChain_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
