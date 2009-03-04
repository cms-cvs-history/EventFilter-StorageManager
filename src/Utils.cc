/**
 * $Id: Utils.cc,v 1.1.2.1 2009/03/03 18:32:41 paterno Exp $
 */

#include "EventFilter/StorageManager/interface/Utils.h"
#include <sys/time.h>


namespace stor
{
  namespace utils
  {

    namespace
    {
      /**
	 Convert a (POSIX) timeval into a time_point_t.
      */
      inline void timeval_to_timepoint(timeval const& in, 
				       time_point_t& out)
      {
	// First set the seconds.
	out = static_cast<time_point_t>(in.tv_sec);
	
	// Then set the microseconds.
	out += static_cast<time_point_t>(in.tv_usec)/(1000*1000);
      }
    } // anonymous namespace
    

    time_point_t getCurrentTime()
    {
      time_point_t result = -1.0;
      timeval now;
      if (gettimeofday(&now, 0) == 0) timeval_to_timepoint(now, result);
      return result;
    }

  } // namespace utils

} // namespace stor
