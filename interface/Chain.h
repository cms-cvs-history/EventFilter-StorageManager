// $Id: Chain.h,v 1.1.2.1 2009/01/19 18:12:16 mommsen Exp $

#ifndef StorageManager_Chain_h
#define StorageManager_Chain_h

#include "toolbox/mem/Reference.h"


namespace stor {
  
  class Chain
  {
  public:

    Chain();
    
    ~Chain();

    bool empty();

    
  private:
    
    int id;
    toolbox::mem::Reference ref;

  };
  
} // namespace stor

#endif // StorageManager_Chain_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
