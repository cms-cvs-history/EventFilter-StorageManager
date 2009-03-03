// $Id: Utils.h,v 1.1.2.1 2009/02/18 14:58:47 mommsen Exp $

#ifndef StorageManager_Utils_h
#define StorageManager_Utils_h

#include <sys/time.h>

namespace stor {

  namespace utils {

    /**
     * Collection of utility functions used in the storage manager
     *
     * $Author: mommsen $
     * $Revision: 1.1.2.1 $
     * $Date: 2009/02/18 14:58:47 $
     */

    /**
       time_point_t is used to represent a specific point in time,
       measured by some specific clock. We rely on the "system
       clock". The value is represented by the number of seconds
       (including a fractional part that depends on the resolution of
       the system clock) since the beginning of the "epoch" (as defined
       by the system clock).
    */
    typedef double time_point_t;

    /**
       durtion_t is used to represent a duration (the "distance" between
       two points in time). The value is represented as a number of
       seconds (including a fractional part).
    */
    typedef double duration_t;

    /**
       Returns the current point in time. A negative value indicates
       that an error occurred when fetching the time from the operating
       system.
    */
    time_point_t getCurrentTime();

  } // namespace utils
  
} // namespace stor

#endif // StorageManager_Utils_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
