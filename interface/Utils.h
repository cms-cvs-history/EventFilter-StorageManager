// $Id: Utils.h,v 1.1.2.2 2009/01/30 20:34:34 paterno Exp $

#ifndef StorageManager_Utils_h
#define StorageManager_Utils_h

#include <sys/time.h>

namespace stor {

  namespace utils {

    /**
     * Collection of utility functions used in the storage manager
     *
     * $Author: paterno $
     * $Revision: 1.1.2.2 $
     * $Date: 2009/01/30 20:34:34 $
     */
    
    
    /**
     * Returns the current time as a double.  The value corresponds to the
     * number of seconds since the epoch (including a fractional part good to
     * the microsecond level).  A negative value indicates that an error
     * occurred when fetching the time from the operating system.
     */
    static double getCurrentTime()
    {
      double now = -1.0;
      struct timeval timeStruct;
      int status = gettimeofday(&timeStruct, 0);
      if (status == 0) {
        now = static_cast<double>(timeStruct.tv_sec) +
          (static_cast<double>(timeStruct.tv_usec) / 1000000.0);
      }
      return now;
    }


  } // namespace utils
  
} // namespace stor

#endif // StorageManager_Utils_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
